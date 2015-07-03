#include <string.h>
#include <stdio.h>

#include "fs.h"
#include "draw.h"
#include "debugfs.h"
#include "platform.h"
#include "decryptor/decryptor.h"
#include "decryptor/crypto.h"
#include "decryptor/features.h"
#include "sha256.h"

#define BUFFER_ADDRESS      ((u8*) 0x21000000)
#define BUFFER_MAX_SIZE     (1 * 1024 * 1024)

#define NAND_SECTOR_SIZE    0x200
#define SECTORS_PER_READ    (BUFFER_MAX_SIZE / NAND_SECTOR_SIZE)

#define TICKET_SIZE         0xD0000

// From https://github.com/profi200/Project_CTR/blob/master/makerom/pki/prod.h#L19
static const u8 common_keyy[6][16] = {
    {0xD0, 0x7B, 0x33, 0x7F, 0x9C, 0xA4, 0x38, 0x59, 0x32, 0xA2, 0xE2, 0x57, 0x23, 0x23, 0x2E, 0xB9} , // 0 - eShop Titles
    {0x0C, 0x76, 0x72, 0x30, 0xF0, 0x99, 0x8F, 0x1C, 0x46, 0x82, 0x82, 0x02, 0xFA, 0xAC, 0xBE, 0x4C} , // 1 - System Titles
    {0xC4, 0x75, 0xCB, 0x3A, 0xB8, 0xC7, 0x88, 0xBB, 0x57, 0x5E, 0x12, 0xA1, 0x09, 0x07, 0xB8, 0xA4} , // 2
    {0xE4, 0x86, 0xEE, 0xE3, 0xD0, 0xC0, 0x9C, 0x90, 0x2F, 0x66, 0x86, 0xD4, 0xC0, 0x6F, 0x64, 0x9F} , // 3
    {0xED, 0x31, 0xBA, 0x9C, 0x04, 0xB0, 0x67, 0x50, 0x6C, 0x44, 0x97, 0xA3, 0x5B, 0x78, 0x04, 0xFC} , // 4
    {0x5E, 0x66, 0x99, 0x8A, 0xB4, 0xE8, 0x93, 0x16, 0x06, 0x85, 0x0F, 0xD7, 0xA1, 0x6D, 0xD7, 0x55} , // 5
};

u32 DecryptBuffer(DecryptBufferInfo *info)
{
    u8* ctr = info->CTR;
    u8* buffer = info->buffer;
    u32 size = info->size;

    if (info->setKeyY) {
        setup_aeskey(info->keyslot, AES_BIG_INPUT | AES_NORMAL_INPUT, info->keyY);
        info->setKeyY = 0;
    }
    use_aeskey(info->keyslot);

    for (u32 i = 0; i < size; i += 0x10, buffer += 0x10) {
        set_ctr(AES_BIG_INPUT | AES_NORMAL_INPUT, ctr);
        aes_decrypt((void*) buffer, (void*) buffer, ctr, 1, AES_CTR_MODE);
        add_ctr(ctr, 0x1);
    }
    
    return 0;
}

u32 DecryptTitlekey(u8* titlekey, u8* titleId, u32 index)
{
    u8 ctr[16];
    u8 keyY[16];
    
    memset(ctr, 0, 16);
    memcpy(ctr, titleId, 8);
    memcpy(keyY, (void *)common_keyy[index], 16);
    
    set_ctr(AES_BIG_INPUT|AES_NORMAL_INPUT, ctr);
    setup_aeskey(0x3D, AES_BIG_INPUT|AES_NORMAL_INPUT, keyY);
    use_aeskey(0x3D);
    aes_decrypt(titlekey, titlekey, ctr, 1, AES_CBC_DECRYPT_MODE);
    
    return 0;
}

u32 GetTicketData(u8* buffer)
{
    u32 ctrnand_offset;
    u32 ctrnand_size;
    u32 offset[2];
    u32 keyslot;

    if(GetUnitPlatform() == PLATFORM_3DS) {
        ctrnand_offset = 0x0B95CA00;
        ctrnand_size = 0x2F3E3600;
        keyslot = 0x4;
    } else {
        ctrnand_offset = 0x0B95AE00;
        ctrnand_size = 0x41D2D200;
        keyslot = 0x5;
    }
    
    for(u32 i = 0; i < 2; i++) {
        offset[i] = (i) ? offset[i-1] + 0x11BE200 : ctrnand_offset; // 0x11BE200 from rxTools v2.4
        Debug("Seeking for 'TICK' (%u)...", i + 1);
        offset[i] = SeekNandMagic((u8*) "TICK", 4, offset[i], ctrnand_size  + (offset[i] - ctrnand_offset), keyslot);
        if(offset[i] == (u32) -1) {
            Debug("Failed!");
            return 1;
        }
        Debug("Found at 0x%08X", offset[i] - ctrnand_offset);
    }
    
    // this only works if there is no fragmentation in NAND (there should not be)
    DecryptNandToMem(buffer, offset[0], TICKET_SIZE, keyslot);
    DecryptNandToMem(buffer + TICKET_SIZE, offset[1], TICKET_SIZE, keyslot);
    
    return 0;
}

u32 DumpTicket() {
    u8* buffer = BUFFER_ADDRESS;
    
    if (GetTicketData(buffer) != 0)
        return 1;
    if (!DebugFileCreate("/ticket.bin", true))
        return 1;
    if (!DebugFileWrite(buffer, 2 * TICKET_SIZE, 0))
        return 1;
    FileClose();
   
    return 0;
}

u32 DecryptTitlekeysFile(void)
{
    EncKeysInfo *info = (EncKeysInfo*)0x20316000;

    if (!DebugFileOpen("/encTitleKeys.bin"))
        return 1;
    
    if (!DebugFileRead(info, 16, 0))
        return 1;

    if (!info->n_entries || info->n_entries > MAX_ENTRIES) {
        Debug("Too many/few entries specified: %i", info->n_entries);
        FileClose();
        return 1;
    }

    Debug("Number of entries: %i", info->n_entries);
    if (!DebugFileRead(info->entries, info->n_entries * sizeof(TitleKeyEntry), 16))
        return 1;
    
    FileClose();

    Debug("Decrypting Title Keys...");
    for (u32 i = 0; i < info->n_entries; i++)
        DecryptTitlekey(info->entries[i].encryptedTitleKey, info->entries[i].titleId, info->entries[i].commonKeyIndex);

    if (!DebugFileCreate("/decTitleKeys.bin", true))
        return 1;

    if (!DebugFileWrite(info, info->n_entries * sizeof(TitleKeyEntry) + 16, 0))
        return 1;
    FileClose();

    return 0;
}

u32 DecryptTitlekeysNand(void)
{
    u8* tick_buf = BUFFER_ADDRESS;
    u8* tkey_buf = BUFFER_ADDRESS + (2* TICKET_SIZE);
    u32 nKeys = 0;
    u8* titlekey;
    u8* titleId;
    u32 commonKeyIndex;
    
    if (GetTicketData(tick_buf) != 0)
        return 1;
    
    Debug("Searching for Title Keys...");
    
    memset(tkey_buf, 0, 0x10);
    for (u32 i = 0x158; i < (2 * TICKET_SIZE) - 0x200; i += 0x200) {
        if(memcmp(tick_buf + i, (u8*) "Root-CA00000003-XS0000000c", 26) == 0) {
            u32 exid;
            titleId = tick_buf + i + 0x9C;
            commonKeyIndex = *(tick_buf + i + 0xB1);
            titlekey = tick_buf + i + 0x7F; // <- will get written to
            for (exid = 0x18; exid < 0x10 + nKeys * 0x20; exid += 0x20)
                if (memcmp(titleId, tkey_buf + exid, 8) == 0)
                    break;
            if (exid < 0x10 + nKeys * 0x20)
                continue; // continue if already dumped
            memset(tkey_buf + 0x10 + nKeys * 0x20, 0x00, 0x20);
            memcpy(tkey_buf + 0x10 + nKeys * 0x20 + 0x00, &commonKeyIndex, 4);
            memcpy(tkey_buf + 0x10 + nKeys * 0x20 + 0x08, titleId, 8);
            memcpy(tkey_buf + 0x10 + nKeys * 0x20 + 0x10, titlekey, 16);
            nKeys++;
        }
    }
    memcpy(tkey_buf, &nKeys, 4);
    
    Debug("Found %u unique Title Keys", nKeys);
    
    if(nKeys > 0) {
        if (!DebugFileCreate("/encTitleKeys.bin", true))
            return 1;
        if (!DebugFileWrite(tkey_buf, 0x10 + nKeys * 0x20, 0))
            return 1;
        FileClose();
    } else {
        return 1;
    }

    return DecryptTitlekeysFile();
}

u32 NcchPadgen()
{
    u32 result;

    NcchInfo *info = (NcchInfo*)0x20316000;
    SeedInfo *seedinfo = (SeedInfo*)0x20400000;

    if (DebugFileOpen("/slot0x25KeyX.bin")) {
        u8 slot0x25KeyX[16] = {0};
        if (!DebugFileRead(&slot0x25KeyX, 16, 0))
            return 1;
        FileClose();
        setup_aeskeyX(0x25, slot0x25KeyX);
    } else {
        // Debug("Warning, not using slot0x25KeyX.bin");
        Debug("7.x game decryption will fail on less than 7.x!");
    }

    if (DebugFileOpen("/seeddb.bin")) {
        if (!DebugFileRead(seedinfo, 16, 0))
            return 1;
        if (!seedinfo->n_entries || seedinfo->n_entries > MAX_ENTRIES) {
            Debug("Too many/few seeddb entries.");
            return 1;
        }
        if (!DebugFileRead(seedinfo->entries, seedinfo->n_entries * sizeof(SeedInfoEntry), 16))
            return 1;
        FileClose();
    } else {
        // Debug("Warning, didn't open seeddb.bin");
        Debug("9.x seed crypto game decryption will fail!");
    }

    if (!DebugFileOpen("/ncchinfo.bin"))
        return 1;
    if (!DebugFileRead(info, 16, 0))
        return 1;

    if (!info->n_entries || info->n_entries > MAX_ENTRIES) {
        Debug("Too many/few entries in ncchinfo.bin");
        return 1;
    }
    if (info->ncch_info_version != 0xF0000004) {
        Debug("Wrong version ncchinfo.bin");
        return 1;
    }
    if (!DebugFileRead(info->entries, info->n_entries * sizeof(NcchInfoEntry), 16))
        return 1;
    FileClose();

    Debug("Number of entries: %i", info->n_entries);

    for(u32 i = 0; i < info->n_entries; i++) {
        Debug("Creating pad number: %i. Size (MB): %i", i+1, info->entries[i].size_mb);

        PadInfo padInfo = {.setKeyY = 1, .size_mb = info->entries[i].size_mb};
        memcpy(padInfo.CTR, info->entries[i].CTR, 16);
        memcpy(padInfo.filename, info->entries[i].filename, 112);
        if (info->entries[i].uses7xCrypto && info->entries[i].usesSeedCrypto) {
            u8 keydata[32];
            memcpy(keydata, info->entries[i].keyY, 16);
            u32 found_seed = 0;
            for (u32 j = 0; j < seedinfo->n_entries; j++) {
                if (seedinfo->entries[j].titleId == info->entries[i].titleId) {
                    found_seed = 1;
                    memcpy(&keydata[16], seedinfo->entries[j].external_seed, 16);
                    break;
                }
            }
            if (!found_seed)
            {
                Debug("Failed to find seed in seeddb.bin");
                return 0;
            }
            u8 sha256sum[32];
            sha256_context shactx;
            sha256_starts(&shactx);
            sha256_update(&shactx, keydata, 32);
            sha256_finish(&shactx, sha256sum);
            memcpy(padInfo.keyY, sha256sum, 16);
        }
        else
            memcpy(padInfo.keyY, info->entries[i].keyY, 16);

        if(info->entries[i].uses7xCrypto == 0xA) // won't work on an Old 3DS
            padInfo.keyslot = 0x18;
        else if(info->entries[i].uses7xCrypto)
            padInfo.keyslot = 0x25;
        else
            padInfo.keyslot = 0x2C;

        result = CreatePad(&padInfo);
        if (!result)
            Debug("Done!");
        else
            return 1;
    }

    return 0;
}

u32 SdPadgen()
{
    u32 result;

    SdInfo *info = (SdInfo*)0x20316000;

    u8 movable_seed[0x120] = {0};

    // Load console 0x34 keyY from movable.sed if present on SD card
    if (DebugFileOpen("/movable.sed")) {
        if (!DebugFileRead(&movable_seed, 0x120, 0))
            return 1;
        FileClose();
        if (memcmp(movable_seed, "SEED", 4) != 0) {
            Debug("movable.sed is too corrupt!");
            return 1;
        }
        setup_aeskey(0x34, AES_BIG_INPUT|AES_NORMAL_INPUT, &movable_seed[0x110]);
        use_aeskey(0x34);
    }

    if (!DebugFileOpen("/SDinfo.bin"))
        return 1;
    if (!DebugFileRead(info, 4, 0))
        return 1;

    if (!info->n_entries || info->n_entries > MAX_ENTRIES) {
        Debug("Too many/few entries!");
        return 1;
    }

    Debug("Number of entries: %i", info->n_entries);

    if (!DebugFileRead(info->entries, info->n_entries * sizeof(SdInfoEntry), 4))
        return 1;
    FileClose();

    for(u32 i = 0; i < info->n_entries; i++) {
        Debug ("Creating pad number: %i. Size (MB): %i", i+1, info->entries[i].size_mb);

        PadInfo padInfo = {.keyslot = 0x34, .setKeyY = 0, .size_mb = info->entries[i].size_mb};
        memcpy(padInfo.CTR, info->entries[i].CTR, 16);
        memcpy(padInfo.filename, info->entries[i].filename, 180);

        result = CreatePad(&padInfo);
        if (!result)
            Debug("Done!");
        else
            return 1;
    }

    return 0;
}

u32 GetNandCtr(u8* ctr, u32 offset)
{
    static const char* versions[] = {"4.x", "5.x", "6.x", "7.x", "8.x", "9.x"};
    static const u8* version_ctrs[] = {
        (u8*)0x080D7CAC,
        (u8*)0x080D858C,
        (u8*)0x080D748C,
        (u8*)0x080D740C,
        (u8*)0x080D74CC,
        (u8*)0x080D794C
    };
    static const u32 version_ctrs_len = sizeof(version_ctrs) / sizeof(u32);
    static u8* ctr_start = NULL; 

    if(ctr_start == NULL) {
        for (u32 i = 0; i < version_ctrs_len; i++) {
            if (*(u32*)version_ctrs[i] == 0x5C980) {
                Debug("System version: %s", versions[i]);
                ctr_start =(u8*)(version_ctrs[i] + 0x30);
                break;
            }
        }

        if(ctr_start == NULL) {
            // if value not in previous list start memory scanning (test range)
            for (u8* c = (u8*)0x080D8FFF; c > (u8*)0x08000000; c--) {
                if (*(u32*)c == 0x5C980 && *(u32*)(c + 1) == 0x800005C9) {
                    Debug("CTR Start: 0x%08X", c + 0x30);
                    ctr_start = c + 0x30;
                    break;
                }
            }
        }
        
        if(ctr_start == NULL) {
            Debug("CTR Start: not found!");
            return 1;
        }
    }
    
    for (u32 i = 0; i < 16; i++)
        ctr[i] = *(ctr_start + (0xF - i)); // The CTR is stored backwards in memory.
    add_ctr(ctr, offset / 0x10);

    return 0;
}

u32 DecryptNandToMem(u8* buffer, u32 offset, u32 size, u32 keyslot)
{
    DecryptBufferInfo info = {.keyslot = keyslot, .setKeyY = 0, .size = size, .buffer = buffer};
    if(GetNandCtr(info.CTR, offset) != 0)
        return 1;

    u32 n_sectors = size / NAND_SECTOR_SIZE;
    u32 start_sector = offset / NAND_SECTOR_SIZE;
    sdmmc_nand_readsectors(start_sector, n_sectors, buffer);
    DecryptBuffer(&info);

    return 0;
}

u32 DecryptNandToFile(char* filename, u32 offset, u32 size, u32 keyslot)
{
    u8* buffer = BUFFER_ADDRESS;

    if (!DebugFileCreate(filename, true))
        return 1;

    for (u32 i = 0; i < size; i += NAND_SECTOR_SIZE * SECTORS_PER_READ) {
        u32 read_bytes = min(NAND_SECTOR_SIZE * SECTORS_PER_READ, (size - i));
        ShowProgress(i, size);
        DecryptNandToMem(buffer, offset + i, read_bytes, keyslot);
        if(!DebugFileWrite(buffer, read_bytes, i))
            return 1;
    }

    ShowProgress(0, 0);
    FileClose();

    return 0;
}

u32 SeekNandMagic(u8* magic, u32 magiclen, u32 offset, u32 size, u32 keyslot)
{
    u8* buffer = BUFFER_ADDRESS;
    u32 found = (u32) -1;
    // move this to GetTicketData()?
    for (u32 i = 0; i < size; i += NAND_SECTOR_SIZE) {
        ShowProgress(i, size);
        DecryptNandToMem(buffer, offset + i, NAND_SECTOR_SIZE, keyslot);
        if(memcmp(buffer, magic, magiclen) == 0) {
            found = offset + i;
            break;
        }
    }

    ShowProgress(0, 0);

    return found;
}

u32 NandPadgen()
{
    u32 keyslot;
    u32 nand_size;

    if(GetUnitPlatform() == PLATFORM_3DS) {
        keyslot = 0x4;
        nand_size = 758;
    } else {
        keyslot = 0x5;
        nand_size = 1055;
    }

    Debug("Creating NAND FAT16 xorpad. Size (MB): %u", nand_size);
    Debug("Filename: nand.fat16.xorpad");

    PadInfo padInfo = {.keyslot = keyslot, .setKeyY = 0, .size_mb = nand_size, .filename = "/nand.fat16.xorpad"};
    if(GetNandCtr(padInfo.CTR, 0xB930000) != 0)
        return 1;

    return CreatePad(&padInfo);
}

u32 CreatePad(PadInfo *info)
{
    static const uint8_t zero_buf[16] __attribute__((aligned(16))) = {0};
    u8* buffer = BUFFER_ADDRESS;
    
    if (!FileCreate(info->filename, true)) // No DebugFileCreate() here - messages are already given
        return 1;

    if(info->setKeyY)
        setup_aeskey(info->keyslot, AES_BIG_INPUT | AES_NORMAL_INPUT, info->keyY);
    use_aeskey(info->keyslot);

    u8 ctr[16] __attribute__((aligned(32)));
    memcpy(ctr, info->CTR, 16);

    u32 size_bytes = info->size_mb * 1024*1024;
    for (u32 i = 0; i < size_bytes; i += BUFFER_MAX_SIZE) {
        u32 curr_block_size = min(BUFFER_MAX_SIZE, size_bytes - i);

        for (u32 j = 0; j < curr_block_size; j+= 16) {
            set_ctr(AES_BIG_INPUT | AES_NORMAL_INPUT, ctr);
            aes_decrypt((void*)zero_buf, (void*)buffer + j, ctr, 1, AES_CTR_MODE);
            add_ctr(ctr, 1);
        }

        ShowProgress(i, size_bytes);

        if (!DebugFileWrite((void*)buffer, curr_block_size, i))
            return 1;
    }

    ShowProgress(0, 0);
    FileClose();

    return 0;
}

u32 DumpNand()
{
    u8* buffer = BUFFER_ADDRESS;
    u32 nand_size = (GetUnitPlatform() == PLATFORM_3DS) ? 0x3AF00000 : 0x4D800000;

    Debug("Dumping System NAND. Size (MB): %u", nand_size / (1024 * 1024));

    if (!DebugFileCreate("/NAND.bin", true))
        return 1;

    u32 n_sectors = nand_size / NAND_SECTOR_SIZE;
    for (u32 i = 0; i < n_sectors; i += SECTORS_PER_READ) {
        ShowProgress(i, n_sectors);
        sdmmc_nand_readsectors(i, SECTORS_PER_READ, buffer);
        if(!DebugFileWrite(buffer, NAND_SECTOR_SIZE * SECTORS_PER_READ, i * NAND_SECTOR_SIZE))
            return 1;
    }

    ShowProgress(0, 0);
    FileClose();

    return 0;
}

u32 DecryptNandPartitions() {
    u32 result = 0;
    u32 ctrnand_offset;
    u32 ctrnand_size;
    u32 keyslot;

    if(GetUnitPlatform() == PLATFORM_3DS) {
        ctrnand_offset = 0x0B95CA00;
        ctrnand_size = 0x2F3E3600;
        keyslot = 0x4;
    } else {
        ctrnand_offset = 0x0B95AE00;
        ctrnand_size = 0x41D2D200;
        keyslot = 0x5;
    }

    // see: http://3dbrew.org/wiki/Flash_Filesystem
    Debug("Dumping & Decrypting FIRM0.bin, size: 4MB");
    result += DecryptNandToFile("/firm0.bin", 0x0B130000, 0x00400000, 0x6);
    Debug("Dumping & Decrypting FIRM1.bin, size: 4MB");
    result += DecryptNandToFile("/firm1.bin", 0x0B530000, 0x00400000, 0x6);
    Debug("Dumping & Decrypting CTRNAND.bin, size: %uMB", ctrnand_size / (1024 * 1024));
    result += DecryptNandToFile("/ctrnand.bin", ctrnand_offset, ctrnand_size, keyslot);

    return 0;
}

u32 DecryptNandSystemTitles() {
    u8* buffer = BUFFER_ADDRESS;
    char filename[256];
    u32 nTitles = 0;
    
    u32 ctrnand_offset;
    u32 ctrnand_size;
    u32 keyslot;

    if(GetUnitPlatform() == PLATFORM_3DS) {
        ctrnand_offset = 0x0B95CA00;
        ctrnand_size = 0x2F3E3600;
        keyslot = 0x4;
    } else {
        ctrnand_offset = 0x0B95AE00;
        ctrnand_size = 0x41D2D200;
        keyslot = 0x5;
    }
    
    Debug("Seeking for 'NCCH'...");
    for (u32 i = 0; i < ctrnand_size; i += NAND_SECTOR_SIZE) {
        ShowProgress(i, ctrnand_size);
        if (DecryptNandToMem(buffer, ctrnand_offset + i, NAND_SECTOR_SIZE, keyslot) != 0)
            return 1;
        if (memcmp(buffer + 0x100, (u8*) "NCCH", 4) == 0) {
            u32 size = NAND_SECTOR_SIZE * (buffer[0x104] | (buffer[0x105] << 8) | (buffer[0x106] << 16) | (buffer[0x107] << 24));
            if ((size == 0) || (size > ctrnand_size - i)) {
                Debug("Found at 0x%08x, but invalid size", ctrnand_offset + i + 0x100);
                continue;
            }
            snprintf(filename, 256, "/%08X%08X.app",  *((unsigned int*)(buffer + 0x10C)), *((unsigned int*)(buffer + 0x108)));
            if (FileOpen(filename)) {
                FileClose();
                Debug("Found duplicate at 0x%08X", ctrnand_offset + i + 0x100, size);
                i += size - NAND_SECTOR_SIZE;
                continue;
            }
            Debug("Found (%i) at 0x%08X, size: %ub", nTitles + 1, ctrnand_offset + i + 0x100, size);
            if (DecryptNandToFile(filename, ctrnand_offset + i, size, keyslot) != 0)
                return 1;
            i += size - NAND_SECTOR_SIZE;
            nTitles++;
        }
    }
    ShowProgress(0, 0);
    
    Debug("Done, decrypted %u unique Titles!", nTitles);
    
    return 0;    
}

u32 RestoreNand()
{
    u8* buffer = BUFFER_ADDRESS;
    u32 nand_size;

    if (!DebugFileOpen("/NAND.bin"))
        return 1;
    nand_size = FileGetSize();
    
    Debug("Restoring System NAND. Size (MB): %u", nand_size / (1024 * 1024));

    u32 n_sectors = nand_size / NAND_SECTOR_SIZE;
    for (u32 i = 0; i < n_sectors; i += SECTORS_PER_READ) {
        ShowProgress(i, n_sectors);
        if(!DebugFileRead(buffer, NAND_SECTOR_SIZE * SECTORS_PER_READ, i * NAND_SECTOR_SIZE))
            return 1;
        sdmmc_nand_writesectors(i, SECTORS_PER_READ, buffer);
    }

    ShowProgress(0, 0);
    FileClose();

    return 0;
}

