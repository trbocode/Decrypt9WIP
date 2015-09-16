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
            { "NCCH Padgen", &NcchPadgen },
            { "SD Padgen", &SdPadgen },
            { "CTRNAND Padgen", &CtrNandPadgen },
            { "TWLNAND Padgen", &TwlNandPadgen }
        }
    },
    {
        "NAND Options",
        {
            { "NAND Backup", &DumpNand },
            { "All Partitions Dump", &DecryptAllNandPartitions },
            { "TWLN Partition Dump", &DecryptTwlNandPartition },
            { "CTRNAND Partition Dump", &DecryptCtrNandPartition }
        }
    },
    {
        "Titles Options",
        {
            { "Titlekey Decrypt (file)", &DecryptTitlekeysFile },
            { "Titlekey Decrypt (NAND)", &DecryptTitlekeysNand },
            { "Ticket Dump", &DumpTicket },
            { "Decrypt Titles", &DecryptTitles }
        }
    },
    #ifdef DANGER_ZONE
    {
        "!DANGER_ZONE!",
        {
            { "NAND Restore", &RestoreNand },
            { "Available Partitions Inject", &InjectAllNandPartitions},
            { "TWL Partition Inject", &InjectTwlNandPartition},
            { "CTRNAND Partition Inject", &InjectCtrNandPartition}
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
