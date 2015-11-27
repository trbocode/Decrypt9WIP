#pragma once
#ifdef USE_THEME
#include "common.h"
#include "draw.h"

#define GFX_PROGRESS  "progress.bin"
#define GFX_DONE      "done.bin"
#define GFX_FAILED    "failed.bin"
#define GFX_UNMOUNT   "unmount.bin"
#define GFX_DANGER_E  "danger_e.bin"
#define GFX_DANGER_S  "danger_s.bin"
#define GFX_DEBUG_BG  "debug_bg.bin"
#define GFX_LOGO      "logo.bin"

#define LOGO_TOP        true
#define LOGO_TEXT_X     10
#define LOGO_TEXT_Y     SCREEN_HEIGHT - 10
#define LOGO_COLOR_BG   COLOR_TRANSPARENT
#define LOGO_COLOR_FONT COLOR_WHITE

#define STD_COLOR_BG   LOGO_COLOR_BG
#define STD_COLOR_FONT LOGO_COLOR_FONT

#define DBG_COLOR_BG   COLOR_BLACK
#define DBG_COLOR_FONT COLOR_WHITE

#define DBG_START_Y 10
#define DBG_END_Y   (SCREEN_HEIGHT - 10)
#define DBG_START_X 10
#define DBG_END_X   (SCREEN_WIDTH_TOP - 10)
#define DBG_STEP_Y  10

void LoadThemeGfxS(const char* filename, bool use_top);
void LoadThemeGfx(const char* filename);
void LoadThemeGfxMenu(u32 index);
void LoadThemeGfxLogo(void);
#endif
