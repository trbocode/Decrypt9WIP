#pragma once

#include "common.h"

typedef struct {
    u32 isDangerous;
    char* longTitle;
    char* shortTitle;
    u32 (*function)(void);
    char* gfxMenu;
} MenuEntry;

typedef struct {
    char* name;
    MenuEntry entries[10];
} MenuInfo;

u32 ProcessMenu(MenuInfo* info, u32 nEntries);
