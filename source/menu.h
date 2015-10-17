#pragma once

#include "common.h"

typedef struct {
    char* name;
    u32 (*function)(void);
    u32 dangerous;
    u32 emunand;
} MenuEntry;

typedef struct {
    char* name;
    u32 n_entries;
    MenuEntry entries[12];
} MenuInfo;

void ProcessMenu(MenuInfo* info, u32 nMenus);
