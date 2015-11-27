#ifdef USE_THEME
#include "theme.h"
#include "fs.h"

void LoadThemeGfxS(const char* filename, bool use_top) {
    char path[256];
    #ifdef APP_TITLE
    snprintf(path, 256, "//3ds/%s/UI/%s", APP_TITLE, filename);
    if (ImportFrameBuffer(path, use_top)) return;
    #endif
    snprintf(path, 256, "//%s/%s", USE_THEME, filename);
    if (!ImportFrameBuffer(path, use_top))
        DrawStringF(10, 10, true, "Not found: %s", filename);
}

void LoadThemeGfx(const char* filename) {
    LoadThemeGfxS(filename, false); // all standard GFX always go to the bottom
}

void LoadThemeGfxMenu(u32 index) {
    char filename[16];
    snprintf(filename, 16, "menu%04lu.bin", index);
    LoadThemeGfxS(filename, !LOGO_TOP); // this goes where the logo goes not
}

void LoadThemeGfxLogo(void) {
    LoadThemeGfxS(GFX_LOGO, LOGO_TOP);
    #if defined LOGO_TEXT_X && defined LOGO_TEXT_Y
    DrawStringF(LOGO_TEXT_X, LOGO_TEXT_Y -  0, LOGO_TOP, "Remaining SD storage space: %llu MiB", RemainingStorageSpace() / 1024 / 1024);
    DrawStringF(LOGO_TEXT_X, LOGO_TEXT_Y - 10, LOGO_TOP, "Game directory: %s", GAME_DIR);
    #ifdef WORK_DIR
    if (DirOpen(WORK_DIR)) {
        DrawStringF(LOGO_TEXT_X, LOGO_TEXT_Y - 20, LOGO_TOP, "Work directory: %s", WORK_DIR);
        DirClose();
    }
    #endif
    #endif
}
#endif
