#include "fs.h"
#include "draw.h"
#include "platform.h"
#include "decryptor/hashfile.h"
#include "decryptor/nand.h"
#include "decryptor/nandfat.h"


u32 NandTransfer(u32 param) {
    PartitionInfo* p_ctrnand = GetPartitionInfo(P_CTRNAND);
    PartitionInfo* p_firm0 = GetPartitionInfo(P_FIRM0);
    PartitionInfo* p_firm1 = GetPartitionInfo(P_FIRM1);
    char filename[64] = { 0 };
    char secnfoname[64] = { 0 };
    char hashname[64] = { 0 };
    bool a9lh = ((*(u32*) 0x101401C0) == 0);
    u32 region = GetRegion();
    
    
    // developer screwup protection
    if (!(param & N_NANDWRITE))
        return 1;
    
    // check free space
    if (!DebugCheckFreeSpace(128 * 1024 * 1024)) {
        Debug("You need at least 128MB free for this operation");
        return 1;
    }
    
    // select CTRNAND image for transfer
    Debug("Select CTRNAND image for transfer");
    if (InputFileNameSelector(filename, p_ctrnand->name, "bin", p_ctrnand->magic, 8, p_ctrnand->size, false) != 0)
        return 1;
    
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
    Debug("Step #3: Injecting CTRNAND image...");
    Debug("Injecting %s (%lu MB)...", filename, p_ctrnand->size / (1024 * 1024));
    if (EncryptFileToNand(filename, p_ctrnand->offset, p_ctrnand->size, p_ctrnand) != 0)
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
        if (DebugSeekFileInNand(&offset, &size, "SecureInfo_A", "RW         SYS        SECURE~?   ", p_ctrnand) != 0)
            return 1;
        if (EncryptFileToNand(secnfoname, offset, size, p_ctrnand) != 0)
            return 1;
    }
    Debug("Step #4 success!");
    
    Debug("");
    Debug("Step #5: Fixing CMACs and paths...");
    if (AutoFixCtrnand(N_NANDWRITE) != 0)
        return 1;
    Debug("Step #5 success!");
    
    return 0;
}

u32 DumpTransferable(u32 param) {
    (void) param;
    PartitionInfo* p_info = GetPartitionInfo(P_CTRNAND);
    char filename[64];
    char hashname[64];
    u8 magic[0x200];
    u8 sha256[0x21];
    
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
    if (OutputFileNameSelector(filename, p_info->name, "bin") != 0)
        return 1;
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