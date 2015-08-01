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
            { "TWL / AGB Partitions Dump", &DecryptTwlAgbPartitions },
            { "CTR Partitions Dump", &DecryptCtrPartitions },
            { "Decrypt Titles", &DecryptTitles }
        }
    },
    {
        "Titlekey Options",
        {
            { "Titlekey Decrypt (file)", &DecryptTitlekeysFile },
            { "Titlekey Decrypt (NAND)", &DecryptTitlekeysNand },
            { "Ticket Dump", &DumpTicket },
            { NULL, NULL }
        }
    },
    #ifdef DANGER_ZONE
    {
        "!DANGER_ZONE!",
        {
            { "NAND Restore", &RestoreNand },
            { "TWL /AGB Partitions Inject", &InjectTwlAgbPartitions},
            { "CTR Partitions Inject", &InjectCtrPartitions},
            { NULL, NULL }
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
