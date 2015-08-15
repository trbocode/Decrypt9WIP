#include "draw.h"
#include "hid.h"
#include "menu.h"


void ProcessEntry(MenuEntry* entry)
{
    if (entry->gfxWarning != NULL) { // warning graphic (if available)
        DrawSplash(entry->gfxWarning, 0);
        while (true) {
            u32 pad_state = InputWait();
            if (pad_state & BUTTON_B)
                return;
            else if (pad_state & BUTTON_A)
                break;
        }
    }
    
    if (entry->gfxProcess != NULL) // process graphic (if available)
        DrawSplash(entry->gfxProcess, 0);
    
    DebugSetTitle(entry->longTitle);
    DebugClear();
    Debug("%s: %s!", entry->shortTitle, entry->function() == 0 ? "succeeded" : "failed");
    Debug("Press B to exit");
    while (!(InputWait() & BUTTON_B));
    DrawSplashLogo();
}

u32 ProcessMenu(MenuEntry* menu, u32 nEntries)
{
    u32 menu_idx = 0;
    u32 pad_state;
    
    DrawSplashLogo();
    
    while(true) {
        // draw top and bottom graphics
        DrawSplash(menu[menu_idx].gfxMenu, 0); // bottom
        
        pad_state = InputWait();
        if (pad_state & BUTTON_START) { 
            return 1;
        } else if (pad_state & BUTTON_SELECT) {
            return 2;
        } else if (pad_state & (BUTTON_DOWN | BUTTON_RIGHT | BUTTON_R1) && menu_idx != nEntries - 1) {
            menu_idx += (menu[menu_idx].nSub + 1); // move right
        } else if (pad_state & (BUTTON_UP | BUTTON_LEFT | BUTTON_L1) && menu_idx != 0) {
            menu_idx -= (menu[menu_idx-1].nSub + 1); // move left
        } else if (pad_state & BUTTON_A) { // process action
            if (menu[menu_idx].nSub == 0) { // if there are no subentries...
                ProcessEntry(&menu[menu_idx]);
            } else { // one ore more subentries (only one sub allowed at this time
                DrawSplash(menu[menu_idx].gfxSelect, 0); //selection
                while (true) {
                    pad_state = InputWait();
                    if (pad_state & BUTTON_B) {
                        break;
                    } else if (pad_state & BUTTON_Y) {
                        ProcessEntry(&menu[menu_idx]);
                        break;
                    } else if (pad_state & BUTTON_X) {
                        ProcessEntry(&menu[menu_idx+1]);
                        break;
                    }
                }
            }
        }
    }
    
    return 0;
}
