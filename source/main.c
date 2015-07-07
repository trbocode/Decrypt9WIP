#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "draw.h"
#include "fs.h"
#include "hid.h"
#include "i2c.h"
#include "decryptor/features.h"
#include "console.h"
#include "menu.h"

void Initialize()
{
    memset(TOP_SCREEN0, 0x00, SCREEN_SIZE);
    memset(TOP_SCREEN1, 0x00, SCREEN_SIZE);
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
    InitFS();
    DebugClearAll();
    Initialize();
    
    while (true)
    {
        MainMenu(); //Load main menu!
    }
}
