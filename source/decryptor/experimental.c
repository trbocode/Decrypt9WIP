#include "fs.h"
#include "draw.h"
#include "decryptor/decryptor.h"
#include "decryptor/nand.h"
#include "decryptor/experimental.h"
#include "fatfs/sdmmc.h"

#define SD_MINFREE_SECTORS  ((1024 * 1024 * 1024) / 0x200)  // have at least 1GB free
#define PARTITION_ALIGN     1 // ((4 * 1024 * 1024) / 0x200)     // align at 4MB


u32 GetDriveSizeSectors(u32 offset_sectors)
{
    u8* buffer = (u8*) 0x20316000;
    u32 part_size = 0;
    
    sdmmc_sdcard_readsectors(offset_sectors, 2, buffer);
    if (memcmp(&(buffer[0x1FE]), "\x55\xAA", 2) != 0)
        return 0;
    if (memcmp(&(buffer[0x52]), "FAT32   ", 8) == 0) { // FAT32 boot sector
        Fat32Info* fat_info = (Fat32Info*) buffer;
        FileSystemInfo* fs_info = (FileSystemInfo*) (buffer + 0x200);
        if ((fat_info->sct_size != 0x0200) || (fat_info->clr_size == 0x00) ||
            (fat_info->reserved0 != 0) || (fat_info->reserved1 != 0) || (fat_info->reserved2 != 0) ||
            (fat_info->clr_root != 0x2) || (fat_info->sct_fsinfo != 0x1) ||
            (fat_info->boot_sig != 0x29) || (fat_info->sct_total == 0))
            return 0; // FAT32 check not passed
        if ((fs_info->signature0 != 0x41615252) || (fs_info->signature1 != 0x61417272))
            return 0; // FS info check not passed
        Debug("Yay FS (%08X/%08X)", fs_info->signature0, fs_info->signature1);
        for (u32 i = 0; i < 480; i++)
            if (fs_info->reserved0[i]) return 0; // data in FS info reserved area
        Debug("Yay FS (2)");
        for (u32 i = 0; i < 14; i++)
            if (fs_info->reserved1[i]) return 0; // data in FS info reserved area
        Debug("Yay FS (3)");
        part_size = fat_info->sct_total + fat_info->sct_hidden;
    } else if (offset_sectors == 0) { // epecting MBR boot sector
        MbrInfo* mbr_info = (MbrInfo*) buffer;
        MbrPartitionInfo* part_info = mbr_info->partitions;
        for (u32 i = 0x1CE; i < 0x1FE; i++)
            if (buffer[i]) return 0; // only one partition allowed
        if ((part_info->type == 0) || (part_info->offset == 0) || (part_info->size == 0))
            return 0; // MBR partition check not passed
        part_size = part_info->size + part_info->offset;
        if (!GetDriveSizeSectors(part_info->offset))
            return 0; // second level check not passed
    }
    
    return part_size;
}

u32 FormatSdCard(u32 param)
{
    MbrInfo* mbr_info = (MbrInfo*) 0x20316000;
    MbrPartitionInfo* part_info = mbr_info->partitions;
    Fat32Info* fat_info = (Fat32Info*) 0x20316200;
    FileSystemInfo* fs_info = (FileSystemInfo*) 0x20316400;
    u8* buffer = BUFFER_ADDRESS;
    
    bool format_emunand = (param & SD_FORMAT_EMUNAND);
    u32 nand_size_sectors  = getMMCDevice(0)->total_size;
    u32 sd_size_sectors    = getMMCDevice(1)->total_size;
    u32 sd_emunand_sectors = 0;
    u32 fat_offset_sectors = 0;
    u32 fat_size_sectors   = 0;
    
    // check SD size
    if (!sd_size_sectors) {
        Debug("Could not detect SD card size");
        return 1;
    }
   
    // set FAT partition offset and size
    Debug("SD card size: %lluMB", ((u64) sd_size_sectors * 0x200) / (1024 * 1024));
    if (format_emunand) {
        Debug("3DS NAND size: %lluMB", ((u64) nand_size_sectors * 0x200) / (1024 * 1024));
        if (nand_size_sectors + SD_MINFREE_SECTORS + 1 > sd_size_sectors) {
            Debug("SD is too small for EmuNAND!");
            return 1;
        }
        sd_emunand_sectors = nand_size_sectors;
    }
    fat_offset_sectors = align(sd_emunand_sectors + 1, PARTITION_ALIGN);
    fat_size_sectors = sd_size_sectors - fat_offset_sectors;
    
    // make a new MBR
    memset(mbr_info, 0x00, 0x200);
    mbr_info->magic         = 0xAA55;
    sprintf(mbr_info->text, "%-*.*s%-16.16s%-8.8s%08X%-8.8s%08X%-8.8s%08X%-8.8s%08X",
        format_emunand ? 16 : 0, format_emunand ? 16 : 0,
        format_emunand ? "GATEWAYNAND" : "", "DECRYPT9SD",
        "SDSIZE:", (unsigned int) sd_size_sectors,
        "NDSIZE:", (unsigned int) nand_size_sectors,
        "FATOFFS:", (unsigned int) fat_offset_sectors,
        "FATSIZE:", (unsigned int) fat_size_sectors);
    part_info->status       = 0x80;
    part_info->type         = 0x0C;
    memcpy(part_info->chs_start, "\x01\x01\x00", 3);
    memcpy(part_info->chs_end  , "\xFE\xFF\xFF", 3);
    part_info->offset       = fat_offset_sectors;
    part_info->size         = fat_size_sectors;
    
    // make a new FAT32
    memset(fat_info, 0x00, 0x20 * 0x200);
    fat_info->magic         = 0xAA55;
    fat_info->sct_size      = 0x200;
    fat_info->clr_size      = 0x40;
    fat_info->sct_reserved  = 0x20;
    fat_info->fat_n         = 0x02;
    fat_info->mediatype     = 0xF8;
    fat_info->sct_track     = 0x3F; // unsure
    fat_info->sct_heads     = 0xFF; // unsure
    fat_info->sct_hidden    = fat_offset_sectors;
    fat_info->sct_total     = fat_size_sectors;
    fat_info->fat_size      =
        (((fat_size_sectors - fat_info->sct_reserved) * 4) /
        (fat_info->sct_size * fat_info->clr_size)) + 1;
    fat_info->clr_root      = 0x02;
    fat_info->sct_fsinfo    = 0x01;
    fat_info->sct_backup    = 0x06;
    fat_info->ndrive        = 0x80;
    fat_info->boot_sig      = 0x29;
    fat_info->vol_id        = 0xDEC9DEC9;
    memcpy(fat_info->jmp      , "\xEB\x00\x90", 3);
    memcpy(fat_info->oemname  , "DECRYPT9", 8);
    memcpy(fat_info->vol_label, "DECRYPT9SD ", 11);
    memcpy(fat_info->fs_type  , "FAT32   ", 8);
    
    // make a new FS info
    fs_info->magic      = 0xAA55;
    fs_info->signature0 = 0x41615252;
    fs_info->signature1 = 0x61417272;
    fs_info->clr_next   = 0xFFFFFFFF;
    fs_info->clr_free   = 0xFFFFFFFF;
    
    // set magic for third block
    memcpy((u8*) fat_info + 0x5FE, "\x55\xAA", 2);
    
    // make a backup copy of FAT32 info and FS info
    memcpy(((u8*) fat_info) + (fat_info->sct_backup * fat_info->sct_size), (u8*) fat_info, fat_info->sct_size * 3);
    
    // here the actual formatting takes place
    Debug("Unmounting SD card...");
    DeinitFS();
    Debug("Writing new master boot record..");
    sdmmc_sdcard_writesectors(0, 1, (u8*) mbr_info);
    Debug("Formatting FAT partition...");
    sdmmc_sdcard_writesectors(fat_offset_sectors, fat_info->sct_reserved, (u8*) fat_info);
    memset(buffer, 0x00, BUFFER_MAX_SIZE);
    for (u32 i = 0; i < (fat_info->fat_n * fat_info->fat_size) + 0x80; i += SECTORS_PER_READ)
        sdmmc_sdcard_writesectors(fat_offset_sectors + fat_info->sct_reserved + i, SECTORS_PER_READ, buffer);
    memcpy(buffer, "\xF8\xFF\xFF\xFF\xFF\xFF\xFF\xF0\xFF\xFF\xFF\xF0", 12);
    for (u32 i = 0; i < fat_info->fat_n; i++)
        sdmmc_sdcard_writesectors(fat_offset_sectors + fat_info->sct_reserved + (i * fat_info->fat_size), 1, buffer);
    memcpy(buffer, "DECRYPT9SD \x08", 12);
    sdmmc_sdcard_writesectors(fat_offset_sectors + fat_info->sct_reserved + (fat_info->fat_n * fat_info->fat_size), 1, buffer);
    Debug("Mounting SD card...");
    InitFS();
    
    return 0;
}

u32 CloneSysNand(u32 param)
{
    MbrInfo* mbr_info = (MbrInfo*) 0x20316000;
    MbrPartitionInfo* part_info = mbr_info->partitions;
    u8* buffer = BUFFER_ADDRESS;
    u32 nand_size_sectors = getMMCDevice(0)->total_size; 
    
    Debug("Checking SD card...");
    sdmmc_sdcard_readsectors(0, 1, (u8*) mbr_info);
    if (((memcmp(mbr_info->text, "GATEWAYNAND", 11) != 0) && (memcmp(mbr_info->text, "MTCARDNAND", 10) != 0) &&
        (memcmp(mbr_info->text, "3DSCARDNAND", 11) != 0)) || (nand_size_sectors > part_info->offset - 1)) {
        Debug("SD card is not formatted for EmuNAND!");
        return 0;
    }
    
    Debug("Cloning SysNAND to EmuNAND...");
    sdmmc_nand_readsectors(0, 1, buffer);
    sdmmc_sdcard_writesectors(nand_size_sectors, 1, buffer);
    for (u32 i = 1; i < nand_size_sectors; i += SECTORS_PER_READ) {
        u32 read_sectors = min(SECTORS_PER_READ, (nand_size_sectors - i));
        ShowProgress(i, nand_size_sectors);
        sdmmc_nand_readsectors(i, read_sectors, buffer);
        sdmmc_sdcard_writesectors(i, read_sectors, buffer);
    }
    ShowProgress(0, 0);
    
    return 0;
}
