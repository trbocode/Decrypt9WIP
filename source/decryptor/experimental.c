#include "fs.h"
#include "draw.h"
#include "decryptor/decryptor.h"
#include "decryptor/nand.h"
#include "decryptor/experimental.h"
#include "fatfs/sdmmc.h"

#define SD_MINFREE_SECTORS  ((1024 * 1024 * 1024) / 0x200)  // have at least 1GB free
#define PARTITION_ALIGN     ((4 * 1024 * 1024) / 0x200)     // align at 4MB

u32 FormatSdCard(u32 param)
{
    MbrInfo* mbr_info = (MbrInfo*) 0x20316000;
    MbrPartitionInfo* part_info = mbr_info->partitions;
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
    sprintf(mbr_info->text, "%-*.*s%-16.16s%-8.8s%08X%-8.8s%08X%-8.8s%08X%-8.8s%08X",
        format_emunand ? 16 : 0, format_emunand ? 16 : 0,
        format_emunand ? "GATEWAYNAND" : "", "DECRYPT9SD",
        "SDSIZE:", (unsigned int) sd_size_sectors,
        "NDSIZE:", (unsigned int) nand_size_sectors,
        "FATOFFS:", (unsigned int) fat_offset_sectors,
        "FATSIZE:", (unsigned int) fat_size_sectors);
    mbr_info->magic         = 0xAA55;
    part_info->status       = 0x80;
    part_info->type         = 0x0C;
    memcpy(part_info->chs_start, "\x01\x01\x00", 3);
    memcpy(part_info->chs_end  , "\xFE\xFF\xFF", 3);
    part_info->offset       = fat_offset_sectors;
    part_info->size         = fat_size_sectors;
    
    // here the actual formatting takes place
    Debug("Unmounting SD card...");
    DeinitFS();
    Debug("Writing new master boot record..");
    sdmmc_sdcard_writesectors(0, 1, (u8*) mbr_info);
    Debug("Mounting SD card...");
    InitFS();
    Debug("Formatting FAT partition...");
    if (!PartitionFormat("DECRYPT9SD"))
        return 1;
    
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
