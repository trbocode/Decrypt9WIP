#include "common.h"
#include "draw.h"
#include "fs.h"
#include "hid.h"
#include "menu.h"
#include "i2c.h"
#include "decryptor/features.h"


MenuInfo menu[] =
{
    {
        "XORpad Generator Options", 4,
        {
            { "NCCH Padgen", &NcchPadgen, 0, 0 },
            { "SD Padgen", &SdPadgen, 0, 0 },
            { "CTRNAND Padgen", &CtrNandPadgen, 0, 0 },
            { "TWLNAND Padgen", &TwlNandPadgen, 0, 0 }
        }
    },
    {
        "Titlekey Decrypt Options", 3,
        {
            { "Titlekey Decrypt (file)", &DecryptTitlekeysFile, 0, 0 },
            { "Titlekey Decrypt (NAND)", &DecryptTitlekeysNand, 0, 0 },
            { "Titlekey Decrypt (EMU)", &DecryptTitlekeysNand, 0, 1 }
        }
    },
    {
        "SysNAND Dump Options", 4,
        {
            { "NAND Backup", &DumpNand, 0, 0 },
            { "All Partitions Dump", &DecryptAllNandPartitions, 0, 0 },
            { "TWLNAND Partition Dump", &DecryptTwlNandPartition, 0, 0 },
            { "CTRNAND Partition Dump", &DecryptCtrNandPartition, 0, 0 },
        }
    },
    {
        "SysNAND Inject Options", 4,
        {
            { "NAND Restore", &RestoreNand, 1, 0 },
            { "All Partitions Inject", &InjectAllNandPartitions, 1, 0 },
            { "TWLNAND Partition Inject", &InjectTwlNandPartition, 1, 0 },
            { "CTRNAND Partition Inject", &InjectCtrNandPartition, 1, 0 }
        }
    },
    #ifdef EXPERIMENTAL
    {
        "EmuNAND Dump Options", 4,
        {
            { "EmuNAND Backup", &DumpNand, 0, 1 },
            { "All Partitions Dump", &DecryptAllNandPartitions, 0, 1 },
            { "TWLNAND Partition Dump", &DecryptTwlNandPartition, 0, 1 },
            { "CTRNAND Partition Dump", &DecryptCtrNandPartition, 0, 1 }
        }
    },
    {
        "EmuNAND Inject Options", 4,
        {
            { "EmuNAND Restore", &RestoreNand, 1, 1 },
            { "All Partitions Inject", &InjectAllNandPartitions, 1, 1 },
            { "TWLNAND Partition Inject", &InjectTwlNandPartition, 1, 1 },
            { "CTRNAND Partition Inject", &InjectCtrNandPartition, 1, 1 }
        }
    },
    #endif
    {
        "SysNAND File Options", 7,
        {
            { "Dump ticket.db", &DumpTicket, 0, 0 },
            { "Dump movable.sed", &DumpMovableSed, 0, 0 },
            { "Dump SecureInfo_A", &DumpSecureInfoA, 0, 0 },
            { "Dump Health&Safety", &DumpHealthAndSafety, 0, 0 },
            { "Inject movable.sed", &InjectMovableSed, 1, 0 },
            { "Inject SecureInfo_A", &InjectSecureInfoA, 1, 0 },
            { "Inject Health&Safety", &InjectHealthAndSafety, 1, 0 }
        }
    },
    {
        "EmuNAND File Options", 9,
        {
            { "Dump ticket_emu.db", &DumpTicket, 0, 1 },
            { "Dump movable.sed", &DumpMovableSed, 0, 1 },
            { "Dump SecureInfo_A", &DumpSecureInfoA, 0, 1 },
            { "Dump seedsave.bin", &DumpSeedsave, 0, 1 },
            { "Dump Health&Safety", &DumpHealthAndSafety, 0, 1 },
            { "Inject movable.sed", &InjectMovableSed, 1, 1 },
            { "Inject SecureInfo_A", &InjectSecureInfoA, 1, 1 },
            { "Inject Health&Safety", &InjectHealthAndSafety, 1, 1 },
            { "Update SeedDB", &UpdateSeedDb, 0, 1 }
        }
    },
    {
        "Game Decryptor Options", 6,
        {
            { "NCCH/NCSD Decryptor", &DecryptNcsdNcch, 0, 0 },
            { "CIA Decryptor (shallow)", &DecryptCiaShallow, 0, 0 },
            { "CIA Decryptor (deep)", &DecryptCiaDeep, 0, 0 },
            { "SD Decryptor/Encryptor", &DecryptSdFiles, 0, 0 },
            { "NCCH/NCSD Encryptor", &EncryptNcsdNcch, 0, 0 },
            { "CIA Encryptor (shallow)", &EncryptCiaShallow, 0, 0 }
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
