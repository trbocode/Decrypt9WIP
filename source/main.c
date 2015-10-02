#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
        "XORpad Options", 4,
        {
            { "NCCH Padgen", &NcchPadgen, 0, 0 },
            { "SD Padgen", &SdPadgen, 0, 0 },
            { "CTRNAND Padgen", &CtrNandPadgen, 0, 0 },
            { "TWLNAND Padgen", &TwlNandPadgen, 0, 0 }
        }
    },
    {
        "NAND Options 1", 4,
        {
            { "NAND Backup", &DumpNand, 0, 0 },
            { "All Partitions Dump", &DecryptAllNandPartitions, 0, 0 },
            { "TWLNAND Partition Dump", &DecryptTwlNandPartition, 0, 0 },
            { "CTRNAND Partition Dump", &DecryptCtrNandPartition, 0, 0 }
        }
    },
    {
        "NAND Options 2", 4,
        {
            { "NAND Restore", &RestoreNand, 1, 0 },
            { "All Partitions Inject", &InjectAllNandPartitions, 1, 0 },
            { "TWLNAND Partition Inject", &InjectTwlNandPartition, 1, 0 },
            { "CTRNAND Partition Inject", &InjectCtrNandPartition, 1, 0 }
        }
    },
    {
        "Titles Options", 4,
        {
            { "Titlekey Decrypt (file)", &DecryptTitlekeysFile, 0, 0 },
            { "Titlekey Decrypt (NAND)", &DecryptTitlekeysNand, 0, 0 },
            { "Ticket Dump", &DumpTicket, 0, 0 },
            { "NCCH/CIA Decryptor", &DecryptNcsdNcchBatch, 0, 0 }
        }
    },
    {
        "EmuNAND Options", 4,
        {
            { "Titlekey Decrypt", &DecryptTitlekeysNand, 0, 1 },
            { "Ticket Dump", &DumpTicket, 0, 1 },
            { "Update SeedDB", &UpdateSeedDb, 0, 1 },
            { "Seedsave Dump", &DumpSeedsave, 0, 1 }
        }
    },
    #ifdef EXPERIMENTAL
    {
        "EmuNAND Read Options", 4,
        {
            { "EmuNAND Backup", &DumpNand, 0, 1 },
            { "All Partitions Dump", &DecryptAllNandPartitions, 0, 1 },
            { "TWLNAND Partition Dump", &DecryptTwlNandPartition, 0, 1 },
            { "CTRNAND Partition Dump", &DecryptCtrNandPartition, 0, 1 }
        }
    },
    {
        "EmuNAND Write Options", 4,
        {
            { "EmuNAND Restore", &RestoreNand, 1, 1 },
            { "All Partitions Inject", &InjectAllNandPartitions, 1, 1 },
            { "TWLNAND Partition Inject", &InjectTwlNandPartition, 1, 1 },
            { "CTRNAND Partition Inject", &InjectCtrNandPartition, 1, 1 }
        }
    }
    #endif
};


void Reboot()
{
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 2);
    while(true);
}

int main()
{
    DebugClear();
    InitFS();

    ProcessMenu(menu, sizeof(menu) / sizeof(MenuInfo));
    
    DeinitFS();
    Reboot();
    return 0;
}
