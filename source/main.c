#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "draw.h"
#include "fs.h"
#include "hid.h"
#include "i2c.h"
#include "decryptor/features.h"

void Reboot()
{
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 2);
    while(true);
}

int main()
{
    DebugClear();
    InitFS();

    Debug("A: NCCH Padgen");
    Debug("B: SD Padgen");
    Debug("X: Titlekey Decryption from file");
    Debug("Y: NAND Padgen");
    Debug("L: NAND Partition Dump");
    Debug("R: NAND Dump");
    Debug("\x18: Ticket Dump");
    Debug("\x19: Titlekey Decryption from NAND");
    Debug("");
    Debug("START: Reboot");
    Debug("");
    Debug("");
    Debug("Remaining SD storage space: %llu MiB", RemainingStorageSpace() / 1024 / 1024);

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
            Debug("Titlekey Decryption (file): %s!", DecryptTitlekeysFile() == 0 ? "succeeded" : "failed");
            break;
        } else if (pad_state & BUTTON_Y) {
            DebugClear();
            Debug("NAND Padgen: %s!", NandPadgen() == 0 ? "succeeded" : "failed");
            break;
        } else if (pad_state & BUTTON_L1) {
            DebugClear();
            Debug("NAND Partition Dump: %s!", DecryptNandPartitions() == 0 ? "succeeded" : "failed");
            break;
        } else if (pad_state & BUTTON_R1) {
            DebugClear();
            Debug("NAND Dump: %s!", DumpNand() == 0 ? "succeeded" : "failed");
            break;
        } else if (pad_state & BUTTON_UP) {
            DebugClear();
            Debug("Ticket Dump: %s!", DumpTicket() == 0 ? "succeeded" : "failed");
            break;
        } else if (pad_state & BUTTON_DOWN) {
            DebugClear();
            Debug("Titlekey Decryption (NAND): %s!", DecryptTitlekeysNand() == 0 ? "succeeded" : "failed");
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
