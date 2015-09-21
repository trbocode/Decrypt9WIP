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
        "XORpad Options",
        {
            { "NCCH Padgen", &NcchPadgen, 0, 0 },
            { "SD Padgen", &SdPadgen, 0, 0 },
            { "CTRNAND Padgen", &CtrNandPadgen, 0, 0 },
            { "TWLNAND Padgen", &TwlNandPadgen, 0, 0 }
        }
    },
    {
        "NAND Options",
        {
            { "NAND Backup", &DumpNand, 0, 0 },
            { "All Partitions Dump", &DecryptAllNandPartitions, 0, 0 },
            { "TWLNAND Partition Dump", &DecryptTwlNandPartition, 0, 0 },
            { "CTRNAND Partition Dump", &DecryptCtrNandPartition, 0, 0 }
        }
    },
    {
        "Titles Options",
        {
            { "Titlekey Decrypt (file)", &DecryptTitlekeysFile, 0, 0 },
            { "Titlekey Decrypt (NAND)", &DecryptTitlekeysNand, 0, 0 },
            { "Ticket Dump", &DumpTicket, 0, 0 },
            { "Decrypt Titles", &DecryptTitles, 0, 0 }
        }
    },
    {
        "EmuNAND Options",
        {
            { "Titlekey Decrypt", &DecryptTitlekeysNand, 0, 1 },
            { "Ticket Dump", &DumpTicket, 0, 1 },
            { "Update SeedDB", &UpdateSeedDb, 0, 1 },
            { "Seedsave Dump", &DumpSeedsave, 0, 1 }
        }
    },
    #ifdef DANGER_ZONE
    {
        "!DANGER_ZONE!",
        {
            { "NAND Restore", &RestoreNand, 1, 0 },
            { "All Partitions Inject", &InjectAllNandPartitions, 1, 0 },
            { "TWLNAND Partition Inject", &InjectTwlNandPartition, 1, 0 },
            { "CTRNAND Partition Inject", &InjectCtrNandPartition, 1, 0 }
        }
    }
    #endif
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
    DebugClear();
    InitFS();

    ProcessMenu(menu, sizeof(menu) / sizeof(MenuInfo));
    
    DeinitFS();
    Reboot();
    return 0;
}
