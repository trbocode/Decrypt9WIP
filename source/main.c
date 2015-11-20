#include "common.h"
#include "draw.h"
#include "fs.h"
#include "hid.h"
#include "menu.h"
#include "i2c.h"
#include "decryptor/game.h"
#include "decryptor/nand.h"
#include "decryptor/nandfat.h"
#include "decryptor/titlekey.h"


MenuInfo menu[] =
{
    {
        "XORpad Generator Options", 4,
        {
            { "NCCH Padgen", &NcchPadgen, 0, 0, 0 },
            { "SD Padgen", &SdPadgen, 0, 0, 0 },
            { "CTRNAND Padgen", &CtrNandPadgen, 0, 0, 0 },
            { "TWLNAND Padgen", &TwlNandPadgen, 0, 0, 0 }
        }
    },
    {
        "Titlekey Decrypt Options", 3,
        {
            { "Titlekey Decrypt (file)", &DecryptTitlekeysFile, 0, 0, 0 },
            { "Titlekey Decrypt (NAND)", &DecryptTitlekeysNand, 0, 0, 0 },
            { "Titlekey Decrypt (EMU)", &DecryptTitlekeysNand, 0, 1, 0 }
        }
    },
    {
        "SysNAND Dump Options", 4,
        {
            { "NAND Backup", &DumpNand, 0, 0, 0 },
            { "All Partitions Dump", &DecryptNandPartitions, 0, 0, P_ALL },
            { "TWLNAND Partition Dump", &DecryptNandPartitions, 0, 0, P_TWLN },
            { "CTRNAND Partition Dump", &DecryptNandPartitions, 0, 0, P_CTRNAND }
        }
    },
    {
        "SysNAND Inject Options", 4,
        {
            { "NAND Restore", &RestoreNand, 1, 0 },
            { "All Partitions Inject", &InjectNandPartitions, 1, 0, P_ALL },
            { "TWLNAND Partition Inject", &InjectNandPartitions, 1, 0, P_TWLN },
            { "CTRNAND Partition Inject", &InjectNandPartitions, 1, 0, P_CTRNAND }
        }
    },
    #ifdef EXPERIMENTAL
    {
        "EmuNAND Dump Options", 4,
        {
            { "EmuNAND Backup", &DumpNand, 0, 1, 0 },
            { "All Partitions Dump", &DecryptNandPartitions, 0, 1, P_ALL },
            { "TWLNAND Partition Dump", &DecryptNandPartitions, 0, 1, P_TWLN },
            { "CTRNAND Partition Dump", &DecryptNandPartitions, 0, 1, P_CTRNAND }
        }
    },
    {
        "EmuNAND Inject Options", 4,
        {
            { "EmuNAND Restore", &RestoreNand, 1, 1, 0 },
            { "All Partitions Inject", &InjectNandPartitions, 1, 1, P_ALL },
            { "TWLNAND Partition Inject", &InjectNandPartitions, 1, 1, P_TWLN },
            { "CTRNAND Partition Inject", &InjectNandPartitions, 1, 1, P_CTRNAND }
        }
    },
    #endif
    {
        "SysNAND File Options", 7,
        {
            { "Dump ticket.db", &DumpFile, 0, 0, F_TICKET },
            { "Dump movable.sed", &DumpFile, 0, 0, F_MOVABLE },
            { "Dump SecureInfo_A", &DumpFile, 0, 0, F_SECUREINFO },
            { "Dump Health&Safety", &DumpHealthAndSafety, 0, 0, 0 },
            { "Inject movable.sed", &InjectFile, 1, 0, F_MOVABLE },
            { "Inject SecureInfo_A", &InjectFile, 1, 0, F_SECUREINFO },
            { "Inject Health&Safety", &InjectHealthAndSafety, 1, 0 }
        }
    },
    {
        "EmuNAND File Options", 9,
        {
            { "Dump ticket_emu.db", &DumpFile, 0, 1, F_TICKET },
            { "Dump movable.sed", &DumpFile, 0, 1, F_MOVABLE },
            { "Dump SecureInfo_A", &DumpFile, 0, 1, F_SECUREINFO },
            { "Dump seedsave.bin", &DumpFile, 0, 1, F_SEEDSAVE },
            { "Dump Health&Safety", &DumpHealthAndSafety, 0, 1, 0 },
            { "Inject movable.sed", &InjectFile, 1, 1, F_MOVABLE },
            { "Inject SecureInfo_A", &InjectFile, 1, 1, F_SECUREINFO },
            { "Inject Health&Safety", &InjectHealthAndSafety, 1, 1, 0 },
            { "Update SeedDB", &UpdateSeedDb, 0, 1, 0 }
        }
    },
    {
        "Game Decryptor Options", 6,
        {
            { "NCCH/NCSD Decryptor", &DecryptNcsdNcch, 0, 0, 0 },
            { "NCCH/NCSD Encryptor", &EncryptNcsdNcchStandard, 0, 0, 0 },
            { "CIA Decryptor (shallow)", &DecryptCiaShallow, 0, 0, 0 },
            { "CIA Decryptor (deep)", &DecryptCiaDeep, 0, 0, 0 },
            { "CIA Decryptor (for GW)", &DecryptCiaGateway, 0, 0, 0 },
            { "SD Decryptor/Encryptor", &CryptSdFiles, 0, 0, 0 }
        }
    }
};


void Reboot()
{
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 2);
    while(true);
}


void PowerOff()
{
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 0);
    while (true);
}


int main()
{
    ClearScreenFull(false);
    ClearScreenFull(true);
    InitFS();

    u32 menu_exit = ProcessMenu(menu, sizeof(menu) / sizeof(MenuInfo));
    
    DeinitFS();
    (menu_exit == MENU_EXIT_REBOOT) ? Reboot() : PowerOff();
    return 0;
}
