#include "fs.h"
#include "draw.h"
#include "platform.h"
#include "decryptor/hashfile.h"
#include "decryptor/nand.h"
#include "decryptor/nandfat.h"
#include "fatfs/sdmmc.h"
#include "NCSD_header_o3ds_hdr.h"


u32 NandTransfer(u32 param) {
    PartitionInfo* p_info = GetPartitionInfo(P_CTRFULL);
    u8* buffer = BUFFER_ADDRESS;
    char filename[64] = { 0 };
    char secnfoname[64] = { 0 };
    char hashname[64] = { 0 };
    bool a9lh = ((*(u32*) 0x101401C0) == 0);
    u32 region = GetRegion();
    u32 imgsize = 0;
    
    
    // developer screwup protection
    if (!(param & N_NANDWRITE))
        return 1;
    
    // check free space
    if (!DebugCheckFreeSpace(128 * 1024 * 1024)) {
        Debug("You need at least 128MB free for this operation");
        return 1;
    }
    
    // check crypto type
    if ((GetUnitPlatform() == PLATFORM_N3DS) && (p_info->keyslot == 0x04)) {
        Debug("This does not work on N3DS with O3DS FW");
        Debug("Restore a NAND backup first");
        return 1;
    }
    
    // select CTRNAND image for transfer
    Debug("Select CTRNAND transfer image");
    if (InputFileNameSelector(filename, "ctrtransfer", "bin", NULL, 0, 0x2F5D0000, true) != 0) // use O3DS size as minimum
        return 1;
    // check size of image
    if (!FileOpen(filename) || !(imgsize = FileGetSize()))
        return 1;
    FileClose();
    if (((GetUnitPlatform() == PLATFORM_3DS) && (imgsize != 0x2F5D0000)) || // only O3DS size allowed on O3DS
        ((GetUnitPlatform() == PLATFORM_N3DS) && (imgsize != 0x2F5D0000) && (imgsize != 0x41ED0000))) {
        Debug("Image has wrong size");
        return 1;
    }
    
    // SHA / region check
    u8 sha256[0x21]; // this needs a 0x20 + 0x01 byte .SHA file
    snprintf(hashname, 64, "%s.sha", filename);
    if (FileGetData(hashname, sha256, 0x21, 0) != 0x21) {
        Debug(".SHA file not found or too small");
        return 1;
    }
    // region check
    if (region != sha256[0x20]) {
        if (!a9lh) {
            Debug("Region does not match");
            return 1;
        } else {
            u8 secureinfo[0x111];
            do {
                Debug("Region does not match, select SecureInfo_A file");
                if (InputFileNameSelector(secnfoname, "SecureInfo_A", NULL, NULL, 0, 0x111, false) != 0)
                    return 1;
                if (FileGetData(secnfoname, secureinfo, 0x111, 0) != 0x111)
                    return 1;
            } while (region != (u32) secureinfo[0x100]);
        }
    }
    
    Debug("");
    Debug("Step #1: .SHA verification of CTRNAND image...");
    Debug("Checking hash from .SHA file...");
    if (CheckHashFromFile(filename, 0, 0, sha256) != 0) {
        Debug("Failed, image corrupt or modified!");
        return 1;
    }
    Debug("Step #1 success!");
    
    Debug("");
    Debug("Step #2: Dumping transfer files...");
    if ((DumpNandFile(FF_AUTONAME | F_MOVABLE) != 0) ||
        (DumpNandFile(FF_AUTONAME | F_TICKET) != 0) ||
        (DumpNandFile(FF_AUTONAME | F_CONFIGSAVE) != 0) ||
        (DumpNandFile(FF_AUTONAME | F_LOCALFRIEND) != 0) ||
        (DumpNandFile(FF_AUTONAME | F_SECUREINFO) != 0))
        return 1;
    Debug("Step #2 success!");
        
    // check NAND header, restore if required (!!!)
    
    Debug("");
    Debug("Step #3: Injecting CTRNAND transfer image...");
    if (p_info->size != imgsize) {
        if (GetUnitPlatform() != PLATFORM_N3DS) // extra safety
            return 1;
        Debug("Switching out NAND header first...");
        if ((NCSD_header_o3ds_hdr_size != 0x200) || (PutNandHeader((u8*) NCSD_header_o3ds_hdr) != 0))
            return 1;
        p_info = GetPartitionInfo(P_CTRFULL);
        if (p_info->size != imgsize)
            return 1;
    }
    Debug("Injecting %s (%lu MB)...", filename, p_info->size / (1024 * 1024));
    if (EncryptFileToNand(filename, p_info->offset, p_info->size, p_info) != 0)
        return 1;
    Debug("Step #3 success!");
    
    Debug("");
    Debug("Step #4: Injecting transfer files...");
    if ((InjectNandFile(N_NANDWRITE | FF_AUTONAME | F_MOVABLE) != 0) ||
        (InjectNandFile(N_NANDWRITE | FF_AUTONAME | F_TICKET) != 0) ||
        (InjectNandFile(N_NANDWRITE | FF_AUTONAME | F_CONFIGSAVE) != 0) ||
        (InjectNandFile(N_NANDWRITE | FF_AUTONAME | F_LOCALFRIEND) != 0) ||
        (*secnfoname && (InjectNandFile(N_NANDWRITE | FF_AUTONAME | F_SECUREINFO) != 0)))
        return 1;
    if (*secnfoname) {
        u32 offset;
        u32 size;
        if (DebugSeekFileInNand(&offset, &size, "SecureInfo_A", "RW         SYS        SECURE~?   ", p_info) != 0)
            return 1;
        if (EncryptFileToNand(secnfoname, offset, size, p_info) != 0)
            return 1;
    }
    Debug("Step #4 success!");
    
    Debug("");
    Debug("Step #5: Fixing CMACs and paths...");
    if (AutoFixCtrnand(N_NANDWRITE) != 0)
        return 1;
    Debug("Step #5 success!");
    
    if (a9lh) { // done at this step if running from a9lh
        Debug("");
        return 0;
    }
    
    Debug("");
    Debug("Step #6: Dumping and injecting NATIVE_FIRM...");
    PartitionInfo* p_firm0 = GetPartitionInfo(P_FIRM0);
    PartitionInfo* p_firm1 = GetPartitionInfo(P_FIRM1);
    u8* firm = buffer;
    u32 firm_size = 0;
    if (DumpNcchFirm((p_info->keyslot == 0x4) ? 4 : 0, false, false) == 0) {
        Debug("NATIVE_FIRM found, injecting...");
        firm_size = FileGetData((p_info->keyslot == 0x4) ? "NATIVE_FIRM.bin" : "NATIVE_FIRM_N3DS.bin", firm, 0x400000, 0);
    } else {
        Debug("NATIVE_FIRM not found, failure!");
        return 1;
    }
    firm_size = CheckFirmSize(firm, firm_size);
    if (!firm_size)
        return 0;
    if ((EncryptMemToNand(firm, p_firm0->offset, firm_size, p_firm0) != 0) ||
        (EncryptMemToNand(firm, p_firm1->offset, firm_size, p_firm1) != 0))
        return 1;
    Debug("Step #6 success!");
    
    
    return 0;
}

u32 DumpTransferable(u32 param) {
    (void) param;
    PartitionInfo* p_info;
    char filename[64];
    char hashname[64];
    u8 magic[0x200];
    u8 sha256[0x21];
    
    p_info = GetPartitionInfo(P_CTRNAND);
    if ((DecryptNandToMem(magic, p_info->offset, 16, p_info) != 0) || (memcmp(p_info->magic, magic, 8) != 0)) {
        Debug("Corrupt partition or decryption error");
        if (p_info->keyslot == 0x05)
            Debug("(or slot0x05keyY not set up)");
        return 1;
    }
    
    if ((CheckNandFile(F_MOVABLE) != 0) ||
        (CheckNandFile(F_TICKET) != 0) ||
        (CheckNandFile(F_CONFIGSAVE) != 0) ||
        (CheckNandFile(F_LOCALFRIEND) != 0) ||
        (CheckNandFile(F_SECUREINFO) != 0)) {
        Debug("CTRNAND is fragmented or corrupt");
        return 1;
    }
    
    Debug("");
    Debug("Creating transferable CTRNAND, size (MB): %u", p_info->size / (1024 * 1024));
    Debug("Select name for transfer file");
    if (OutputFileNameSelector(filename, "ctrtransfer", "bin") != 0)
        return 1;
    p_info = GetPartitionInfo(P_CTRFULL);
    if (DecryptNandToFile(filename, p_info->offset, p_info->size, p_info, sha256) != 0)
        return 1;
    
    sha256[0x20] = (u8) GetRegion();
    snprintf(hashname, 64, "%s.sha", filename);
    if ((sha256[0x20] > 6) || (FileDumpData(hashname, sha256, 0x21) != 0x21)) {
        Debug("Failed creating hashfile");
        return 1;
    }
    
    return 0;
}