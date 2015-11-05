#pragma once

#include "common.h"

#define MENU_EXIT_REBOOT    0
#define MENU_EXIT_POWEROFF  1

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

u32 ProcessMenu(MenuInfo* info, u32 nMenus);
