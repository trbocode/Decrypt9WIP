#pragma once

#include "common.h"

typedef struct {
    u32 nSub;
    char* longTitle;
    char* shortTitle;
    u32 (*function)(void);
    char* gfxMenu;
    char* gfxSelect;
    char* gfxWarning;
    char* gfxProcess;
	char* gfxDone;
	char* gfxFailed;
} MenuEntry;

u32 ProcessMenu(MenuEntry* menu, u32 nEntries);
