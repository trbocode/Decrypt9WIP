#include "common.h"
#include "draw.h"
#include "console.h"
#include "fs.h"
#include "menu.h"
#include "i2c.h"
#include "decryptor/features.h"


MenuEntry menu[] = 
{
    { 0, "NCCH Xorpad Generator", "NCCH Padgen", NcchPadgen, "menu0.bin", NULL, NULL, NULL },
    { 0, "SD Xorpad Generator", "SD Padgen", SdPadgen, "menu1.bin", NULL, NULL, NULL },
    { 1, "NAND FAT16 Xorpad Generator", "CTRNAND Padgen", CtrNandPadgen, "menu2.bin", "pad0.bin", NULL, "pad1.bin" },
    { 1, "TWLN FAT16 Xorpad Generator", "TWLN Padgen", TwlNandPadgen, "menu2.bin", "pad0.bin", NULL, "pad2.bin" },
    #ifdef DANGER_ZONE
    { 1, "NAND Restore", "Restore NAND", RestoreNand, "menu3.bin", "nand0.bin", "nand1.bin", "nand3.bin" },
    { 1, "NAND Backup", "Backup NAND", DumpNand, "menu3.bin", "nand0.bin", NULL, "nand2.bin" },
    { 1, "CTR Partitions Injector", "Inject CTR Partitions", InjectCtrPartitions, "menu4.bin", "npart0.bin", "npart1.bin", "npart3.bin" },
    { 1, "CTR Partitions Decryptor", "Decrypt CTR Partitions", DecryptCtrPartitions, "menu4.bin", "npart0.bin", NULL, "npart2.bin" },
    { 1, "TWL Partitions Injector", "Inject TWL Partitions", InjectTwlAgbPartitions, "menu5.bin", "npart4.bin", "npart5.bin", "npart7.bin" },
    { 1, "TWL Partitions Decryptor", "Decrypt TWL Partitions", DecryptTwlAgbPartitions, "menu5.bin", "npart4.bin", NULL, "npart6.bin" },
    #else
    { 0, "NAND Backup", "Backup NAND", DumpNand, "menu3.bin", NULL, NULL, "nand2.bin" },
    { 0, "CTR Partitions Decryptor", "Decrypt CTR Partitions", DecryptCtrPartitions, "menu4.bin", NULL, NULL, "npart2.bin" },
    { 0, "TWL Partitions Decryptor", "Decrypt TWL Partitions", DecryptTwlAgbPartitions, "menu5.bin", NULL, NULL, "npart6.bin" },
    #endif
    { 0, "Titlekey Decryptor (file)", "Titlekey Decryption", DecryptTitlekeysFile, "menu6.bin", NULL, NULL, NULL },
    { 0, "Titlekey Decryptor (NAND)", "Titlekey Decryption", DecryptTitlekeysNand, "menu7.bin", NULL, NULL, NULL },
    { 0, "Ticket Dumper", "Dump Ticket", DumpTicket, "menu8.bin", NULL, NULL, NULL },
    { 0, "CTR ROM Decryptor", "Decrypt ROM(s)", DecryptTitles, "menu9.bin", NULL, NULL, NULL }
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

void Initialize()
{
    ConsoleSetXY(0, 0);
    ConsoleSetWH(398, 238);
    ConsoleSetBorderColor(PURPLE);
    ConsoleSetTextColor(WHITE);
    ConsoleSetBackgroundColor(BLACK);
    ConsoleSetSpacing(2);
    ConsoleSetBorderWidth(2);
}

int main()
{
    u32 result;
    
    InitFS();
    DebugClearAll();
    Initialize();
    
    result = ProcessMenu(menu, sizeof(menu) / sizeof(MenuEntry));
    
    DeinitFS();
    (result == 1) ? Reboot() : PowerOff();
    return 0;
}
