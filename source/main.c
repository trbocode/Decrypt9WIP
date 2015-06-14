#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "draw.h"
#include "fs.h"
#include "hid.h"
#include "i2c.h"
#include "decryptor/padgen.h"
#include "decryptor/titlekey.h"

void Reboot()
{
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 2);
    while(true);
}

void drawMenu()
{
    DrawString(TOP_SCREEN0, "A: NCCH Padgen", 110, 60, 65280, 0);
    DrawString(TOP_SCREEN1, "A: NCCH Padgen", 110, 60, 65280, 0);
    DrawString(TOP_SCREEN0, "B: SD Padgen", 110, 70, 65280, 0);
    DrawString(TOP_SCREEN1, "B: SD Padgen", 110, 70, 65280, 0);
    DrawString(TOP_SCREEN0, "X: Titlekey Decryption", 110, 80, 65280, 0);
    DrawString(TOP_SCREEN1, "X: Titlekey Decryption", 110, 80, 65280, 0);
    DrawString(TOP_SCREEN0, "Y: NAND Padgen", 110, 90, 65280, 0);
    DrawString(TOP_SCREEN1, "Y: NAND Padgen", 110, 90, 65280, 0);
    drawRect(6, 50, 394, 110, 102, 0, 255, TOP_SCREEN0);
    drawRect(6, 50, 394, 110, 102, 0, 255, TOP_SCREEN1);
}

void drawFreeSpace()
{
    drawRect(6, 234, 394, 214, 102, 0, 255, TOP_SCREEN0);
    drawRect(6, 234, 394, 214, 102, 0, 255, TOP_SCREEN1);
    DrawStringF(10, 220, "     Remaining SD storage space: %llu MiB", RemainingStorageSpace() / 1024 / 1024);
}

int main()
{
    InitFS();
    DebugClear();
    drawMenu();
    drawFreeSpace();
    
    while (true) {
        u32 pad_state = InputWait();
        if (pad_state & BUTTON_A) {
            DebugClear();
            Debug("NCCH Padgen: %s!", NcchPadgen() == 0 ? "succeeded" : "failed");
            break;
        } else if (pad_state & BUTTON_B) {
            DebugClear();
            Debug("SD Padgen: %s!", SdPadgen() == 0 ? "succeeded" : "failed");
            break;
        } else if (pad_state & BUTTON_X) {
            DebugClear();
            Debug("Titlekey Decryption: %s!", DecryptTitlekeys() == 0 ? "succeeded" : "failed");
            break;
        } else if (pad_state & BUTTON_Y) {
            DebugClear();
            Debug("NAND Padgen: %s!", NandPadgen() == 0 ? "succeeded" : "failed");
            break;
        } else if (pad_state & BUTTON_START) {
            goto reboot;
        }
    }

    Debug("");
    Debug("Press START to reboot to home.");
    while(true) {
        if (InputWait() & BUTTON_START)
            break;
    }

reboot:
    DeinitFS();
    Reboot();
    return 0;
}
