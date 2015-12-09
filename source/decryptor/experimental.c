#include "fs.h"
#include "draw.h"
#include "hid.h"
#include "decryptor/decryptor.h"
#include "decryptor/nand.h"
#include "decryptor/experimental.h"
#include "fatfs/sdmmc.h"

#define SD_MINFREE_SECTORS  ((1024 * 1024 * 1024) / 0x200)  // have at least 1GB free
#define PARTITION_ALIGN     ((4 * 1024 * 1024) / 0x200)     // align at 4MB
#define MAX_STARTER_SIZE    (2 * 1024 * 1024)               // allow to take over max 2MB boot.3dsx

u32 FormatSdCard(u32 param)
{
    MbrInfo* mbr_info = (MbrInfo*) 0x20316000;
    MbrPartitionInfo* part_info = mbr_info->partitions;
    u8* buffer = (u8*) 0x21000000;
    
    bool setup_emunand = (param & SD_SETUP_EMUNAND);
    u32 starter_size = (param & SD_USE_STARTER) ? 1 : 0;
    
    u32 nand_size_sectors  = getMMCDevice(0)->total_size;
    u32 sd_size_sectors    = 0;
    u32 sd_emunand_sectors = 0;
    u32 fat_offset_sectors = 0;
    u32 fat_size_sectors   = 0;
    
    // copy starter.3dsx to memory for autosetup
    if (starter_size) {
        if (!DebugFileOpen("starter.3dsx"))
            return 1;
        starter_size = FileGetSize();
        if (starter_size > MAX_STARTER_SIZE) {
            Debug("File is %ikB (max %ikB)", starter_size / 1024, MAX_STARTER_SIZE / 1024);
            FileClose();
            return 1;
        }
        if (!DebugFileRead(buffer, starter_size, 0)) {
            FileClose();
            return 1;
        }
        FileClose();
        Debug("File is stored in buffer");
    }
    
    // give the user one last chance to swap the SD card
    DeinitFS();
    Debug("You may insert the SD to format now");
    Debug("Press <A>+<L> when ready");
    while((InputWait() & (BUTTON_A|BUTTON_L1)) != (BUTTON_A|BUTTON_L1));
    InitFS();
    
    // check SD size
    sd_size_sectors = getMMCDevice(1)->total_size;
    if (!sd_size_sectors) {
        Debug("Could not detect SD card size");
        return 1;
    }
   
    // set FAT partition offset and size
    Debug("SD card size: %lluMB", ((u64) sd_size_sectors * 0x200) / (1024 * 1024));
    if (setup_emunand) {
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
        setup_emunand ? 16 : 0, setup_emunand ? 16 : 0,
        setup_emunand ? "GATEWAYNAND" : "", "DECRYPT9SD",
        "SDSIZE :", (unsigned int) sd_size_sectors,
        "NDSIZE :", (unsigned int) nand_size_sectors,
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
    DeinitFS();
    Debug("Writing new master boot record..");
    sdmmc_sdcard_writesectors(0, 1, (u8*) mbr_info);
    InitFS();
    Debug("Formatting FAT partition...");
    if (setup_emunand)
        Debug("This will take some time");
    if (!PartitionFormat("DECRYPT9SD"))
        return 1;
    
    if (starter_size) {
        if (!DebugFileCreate("//boot.3dsx", true))
            return 1;
        if (!DebugFileWrite(buffer, starter_size, 0)) {
            FileClose();
            return 1;
        }
        FileClose();
    }
    
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
        return 1;
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
