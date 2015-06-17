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
    DrawStringF(110, 60, "A: NCCH Padgen");
    DrawStringF(110, 70, "B: SD Padgen");
    DrawStringF(110, 80, "X: Titlekey Decryption");
    DrawStringF(110, 90, "Y: NAND Padgen");
    drawRect(6, 50, 394, 110, 102, 0, 255, TOP_SCREEN0); //middle rectangle
    drawRect(6, 50, 394, 110, 102, 0, 255, TOP_SCREEN1); //middle rectangle
}

void drawFreeSpace()
{
    drawRect(6, 234, 394, 214, 102, 0, 255, TOP_SCREEN0); //small rectangle bottom
    drawRect(6, 234, 394, 214, 102, 0, 255, TOP_SCREEN1); //small rectangle bottom
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
