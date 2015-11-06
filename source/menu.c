#include "menu.h"
#include "draw.h"
#include "hid.h"
#include "fs.h"
#include "decryptor/features.h"

#define TOP_SCREEN false


u32 UnmountSd()
{
    u32 pad_state;
    
    DebugClear();
    Debug("Unmounting SD card...");
    DeinitFS();
    Debug("SD is unmounted, you may remove it now.");
    Debug("Put the SD card back in before pressing B!");
    Debug("");
    Debug("(B to return, START to reboot)");
    while (true) {
        pad_state = InputWait();
        if (((pad_state & BUTTON_B) && InitFS()) || (pad_state & BUTTON_START))
            break;
    }
    
    return pad_state;
}

void DrawMenu(MenuInfo* currMenu, u32 index, bool fullDraw, bool subMenu)
{
    u32 menublock_y0 = 40;
    u32 menublock_y1 = menublock_y0 + currMenu->n_entries * 10;
    
    if (fullDraw) { // draw full menu
        if (!TOP_SCREEN)
            ClearScreenFull(true);
        ClearScreenFull(TOP_SCREEN);
        DrawStringF(10, menublock_y0 - 20, TOP_SCREEN, "%s", currMenu->name);
        DrawStringF(10, menublock_y0 - 10, TOP_SCREEN, "==========================");
        DrawStringF(10, menublock_y1 +  0, TOP_SCREEN, "==========================");
        DrawStringF(10, menublock_y1 + 10, TOP_SCREEN, (subMenu) ? "A: Choose  B: Return" : "A: Choose");
        DrawStringF(10, menublock_y1 + 20, TOP_SCREEN, "SELECT: Unmount SD");
        DrawStringF(10, menublock_y1 + 30, TOP_SCREEN, "START:  Reboot");
        DrawStringF(10, SCREEN_HEIGHT - 20, TOP_SCREEN, "Remaining SD storage space: %llu MiB", RemainingStorageSpace() / 1024 / 1024);
        DrawStringF(10, SCREEN_HEIGHT - 30, TOP_SCREEN, "Game directory: %s", GAME_DIR);
        #ifdef WORK_DIR
        if (DirOpen(WORK_DIR)) {
            DrawStringF(10, SCREEN_HEIGHT - 40, TOP_SCREEN, "Work directory: %s", WORK_DIR);
            DirClose();
        }
        #endif
    }
    
    if (!TOP_SCREEN)
        DrawStringF(10, 10, true, "Selected: %-*.*s", 32, 32, currMenu->entries[index].name);
        
    for (u32 i = 0; i < currMenu->n_entries; i++) { // draw menu entries / selection []
        char* name = currMenu->entries[i].name;
        DrawStringF(10, menublock_y0 + (i*10), TOP_SCREEN, (i == index) ? "[%s]" : " %s ", name);
    }
}

u32 ProcessEntry(MenuEntry* entry)
{
    u32 pad_state;
    
    // unlock sequence for dangerous features
    if (entry->dangerous) {
        u32 unlockSequence[] = { BUTTON_LEFT, BUTTON_RIGHT, BUTTON_DOWN, BUTTON_UP, BUTTON_A };
        u32 unlockLvlMax = sizeof(unlockSequence) / sizeof(u32);
        u32 unlockLvl = 0;
        DebugClear();
        Debug("You selected \"%s\".", entry->name);
        Debug("This feature is potentially dangerous!");
        Debug("If you understand and wish to proceed, enter:");
        Debug("<Left>, <Right>, <Down>, <Up>, <A>");
        Debug("");
        Debug("(B to return, START to reboot)");
        while (true) {
            ShowProgress(unlockLvl, unlockLvlMax);
            if (unlockLvl == unlockLvlMax)
                break;
            pad_state = InputWait();
            if (!(pad_state & BUTTON_ANY))
                continue;
            else if (pad_state & unlockSequence[unlockLvl])
                unlockLvl++;
            else if (pad_state & (BUTTON_B | BUTTON_START))
                break;
            else if (unlockLvl == 0 || !(pad_state & unlockSequence[unlockLvl-1]))
                unlockLvl = 0;
        }
        ShowProgress(0, 0);
        if (unlockLvl < unlockLvlMax)
            return pad_state;
    }
    
    // execute this entries function
    DebugClear();
    if (SetNand(entry->emunand) != 0) {
        Debug("%s: failed!", entry->name);
    } else {
        Debug("%s: %s!", entry->name, (*(entry->function))() == 0 ? "succeeded" : "failed");
    }
    Debug("");
    Debug("Press B to return, START to reboot.");
    while(!(pad_state = InputWait() & (BUTTON_B | BUTTON_START)));
    
    // returns the last known pad_state
    return pad_state;
}

u32 ProcessMenu(MenuInfo* info, u32 nMenus)
{
    MenuInfo mainMenu;
    MenuInfo* currMenu = &mainMenu;
    u32 index = 0;
    u32 result = MENU_EXIT_REBOOT;
    
    // build main menu structure from submenus
    memset(&mainMenu, 0x00, sizeof(MenuInfo));
    for (u32 i = 0; i < nMenus && i < 8; i++) {
        mainMenu.entries[i].name = info[i].name;
        mainMenu.entries[i].function = NULL;
    }
    #ifndef BUILD_NAME
    mainMenu.name = "Decrypt9 Main Menu";
    #else
    mainMenu.name = BUILD_NAME;
    #endif
    mainMenu.n_entries = nMenus;
    DrawMenu(&mainMenu, 0, true, false);
    
    // main processing loop
    while (true) {
        bool full_draw = true;
        u32 pad_state = InputWait();
        if ((pad_state & BUTTON_A) && (currMenu == &mainMenu)) {
            currMenu = info + index;
            index = 0;
        } else if (pad_state & BUTTON_A) {
            pad_state = ProcessEntry(currMenu->entries + index);
        } else if ((pad_state & BUTTON_B) && (currMenu != &mainMenu)) {
            index = currMenu - info;
            currMenu = &mainMenu;
        } else if (pad_state & BUTTON_DOWN) {
            index = (index == currMenu->n_entries - 1) ? 0 : index + 1;
            full_draw = false;
        } else if (pad_state & BUTTON_UP) {
            index = (index == 0) ? currMenu->n_entries - 1 : index - 1;
            full_draw = false;
        } else if ((pad_state & BUTTON_R1) && (currMenu != &mainMenu)) {
            if (++currMenu - info >= nMenus) currMenu = info;
            index = 0;
        } else if ((pad_state & BUTTON_L1) && (currMenu != &mainMenu)) {
            if (--currMenu < info) currMenu = info + nMenus - 1;
            index = 0;
        } else if (pad_state & BUTTON_SELECT) {
            pad_state = UnmountSd();
        } else {
            full_draw = false;
        }
        if (pad_state & BUTTON_START) {
            result = (pad_state & BUTTON_LEFT) ? MENU_EXIT_POWEROFF : MENU_EXIT_REBOOT;
            break;
        }
        DrawMenu(currMenu, index, full_draw, currMenu != &mainMenu);
    }
    
    return result;
}
