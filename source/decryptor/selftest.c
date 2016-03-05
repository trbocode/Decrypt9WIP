#include "fs.h"
#include "draw.h"
#include "decryptor/aes.h"
#include "decryptor/sha.h"
#include "decryptor/selftest.h"
#include "decryptor/decryptor.h"
#include "decryptor/game.h"
#include "decryptor/titlekey.h"
#include "fatfs/sdmmc.h"

// keys to test (requirements in comments)
// 0x11 -> test key (needs various normalKeys)
// 0x2C -> standard NCCH crypto
// 0x25 -> 7x NCCH crypto
// 0x18 -> Secure3 crypto (needs N3DS)
// 0x34 -> 'SD' key (needs movable keyY)
// 0x03 -> TWL key (console unique)
// 0x04 -> CTRNAND O3DS key (console unique, O3DS only)
// 0x05 -> CTRNAND N3DS key (console unique, N3DS only)
// 0x06 -> FIRM key (console unique)
// 0x07 -> AGBSAVE key (console unique)

// other stuff to test
// Getting the NAND CID -> from memory / Normatts function
// TitleKey decryption -> all 6 * 16 commonKey indices
// various AES modes

#define ST_NAND_CID_HARD    1
#define ST_NAND_CID_MEM     2
#define ST_SHA              3
#define ST_AES_MODE         4
#define ST_AES_KEYSLOT      5
#define ST_AES_KEYSLOT_Y    6
#define ST_TITLEKEYS        7

typedef struct {
    char name[16];
    u32 size;
    u32 type;
    u32 param;
} SubTestInfo;

SubTestInfo TestList[] = {
    { "nand_cid_hard", 16, ST_NAND_CID_HARD, 0 },
    { "nand_cid_mem", 16, ST_NAND_CID_MEM, 0 },
    { "sha256", 32, ST_SHA, SHA256_MODE },
    { "sha1", 20, ST_SHA, SHA1_MODE },
    { "aes_cnt_ctr", 16, ST_AES_MODE, AES_CNT_CTRNAND_MODE },
    { "aes_cnt_twl", 16, ST_AES_MODE, AES_CNT_TWLNAND_MODE },
    { "aes_ttk_enc", 16, ST_AES_MODE, AES_CNT_TITLEKEY_DECRYPT_MODE },
    { "aes_ttk_dec", 16, ST_AES_MODE, AES_CNT_TITLEKEY_ENCRYPT_MODE },
    { "ncch_std_key", 16, ST_AES_KEYSLOT_Y, 0x2C },
    { "ncch_7x_key", 16, ST_AES_KEYSLOT_Y, 0x25 },
    { "ncch_sec3_key", 16, ST_AES_KEYSLOT_Y, 0x18 },
    { "ncch_sec4_key", 16, ST_AES_KEYSLOT_Y, 0x1B },
    { "nand_twl_key", 16, ST_AES_KEYSLOT, 0x03 },
    { "nand_ctro_key", 16, ST_AES_KEYSLOT, 0x04 },
    { "nand_ctrn_key", 16, ST_AES_KEYSLOT, 0x05 },
    { "nand_agb_key", 16, ST_AES_KEYSLOT, 0x06 },
    { "nand_frm_key", 16, ST_AES_KEYSLOT, 0x07 },
    { "titlekey", 6*16, ST_TITLEKEYS, 0 },
};

u32 SelfTest(u32 param)
{
    u8* test_data = (u8*) 0x20316000;
    const u8 teststr[16] = { 'D', '9', ' ', 'S', 'E', 'L', 'F', 'T', 'E', 'S', 'T', ' ', ' ', ' ', ' ' };
    const u8 zeroes[16] = { 0x00 };
    bool selftest = !(param & ST_REFERENCE);
       
    Debug((selftest) ? "Running selftest..." : "Creating selftest reference data...");
    
    // process all subtests
    u32 num_tests = sizeof(TestList) / sizeof(SubTestInfo);
    u8* test_ptr = test_data;
    u32 fsize_test = 0;
    for (u32 i = 0; i < num_tests; i++) {
        u32 size = TestList[i].size;
        u32 size_a = align(size, 16);
        u32 type = TestList[i].type;
        u32 param = TestList[i].param;
        
        memset(test_ptr, 0x00, 16 + size_a);
        strncpy((char*) test_ptr, TestList[i].name, 16);
        test_ptr += 16;
        
        if (type == ST_NAND_CID_HARD) {
            sdmmc_get_cid(1, (uint32_t*) test_ptr);
        } else if (type == ST_NAND_CID_MEM) {
            memcpy(test_ptr, (void*) 0x01FFCD84, 16);
        } else if (type == ST_SHA) {
            sha_init(param);
            sha_update(teststr, 16);
            sha_get(test_ptr);
        } else if ((type == ST_AES_MODE) || (type == ST_AES_KEYSLOT) || (type == ST_AES_KEYSLOT_Y)) {
            CryptBufferInfo info = {.setKeyY = 0, .size = 16, .buffer = test_ptr};
            if (type == ST_AES_MODE) {
                info.mode = param;
                info.keyslot = 0x11;
                setup_aeskey(0x11, (void*) zeroes);
            } else {
                if (type == ST_AES_KEYSLOT_Y) {
                    info.setKeyY = 1;
                    memcpy(info.keyY, zeroes, 16);
                }
                info.mode = AES_CNT_CTRNAND_MODE;
                info.keyslot = param;
            }
            memset(info.ctr, 0x00, 16);
            memcpy(test_ptr, teststr, 16);
            CryptBuffer(&info);
        } else if (type == ST_TITLEKEYS) {
            TitleKeyEntry titlekey;
            memset(&titlekey, 0x00, sizeof(TitleKeyEntry));
            for (titlekey.commonKeyIndex = 0; titlekey.commonKeyIndex < 6; titlekey.commonKeyIndex++) {
                memset(titlekey.titleId, 0x00, 8);
                memset(titlekey.encryptedTitleKey, 0x00, 16);
                DecryptTitlekey(&titlekey);
                memcpy(test_ptr + (titlekey.commonKeyIndex * 16), titlekey.encryptedTitleKey, 16);
            }     
        }
        
        test_ptr += size_a;
        fsize_test += 16 + size_a;
    }
    
    // run actual self test
    char filename[32];
    snprintf(filename, 31, "d9_selftest.ref");
    if (selftest) {
        u8* cmp_ptr = test_data + fsize_test;
        if (!DebugFileOpen(filename)) {
            Debug("No reference data available!");
            return 1;
        }
        if (!DebugFileRead(cmp_ptr, fsize_test, 0)) {
            FileClose();
            return 1;
        }
        FileClose();
        for (u32 chk = 0; chk < 2; chk++) {
            u32 count = 0;
            Debug("");
            Debug((chk) ? "Failed tests:" : "Passed tests:");
            test_ptr = test_data;
            cmp_ptr = test_data + fsize_test;
            for (u32 i = 0; i < num_tests; i++) {
                u32 size = TestList[i].size;
                u32 size_a = align(size, 16);
                test_ptr += 16;
                cmp_ptr += 16;
                if ((!chk && memcmp(test_ptr, cmp_ptr, size) == 0) || (chk && memcmp(test_ptr, cmp_ptr, size) != 0)) {
                    Debug(TestList[i].name);
                    count++;
                }
                test_ptr += size_a;
                cmp_ptr += size_a;
            }
            Debug("%u of %u tests %s", count, num_tests, (chk) ? "failed" : "passed");
        }
        snprintf(filename, 31, "d9_selftest.lst");
    }
    
    // write test data to file
    Debug("");
    if (!DebugFileCreate(filename, true))
        return 1;
    if (!DebugFileWrite(test_data, fsize_test, 0)) {
        FileClose();
        return 1;
    }
    FileClose();
    
    return 0;
}
