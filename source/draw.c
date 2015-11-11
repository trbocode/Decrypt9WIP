// Copyright 2013 Normmatt
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "font.h"
#include "draw.h"
#include "fs.h"

#define BG_COLOR   (RGB(0x00, 0x00, 0x00))
#define FONT_COLOR (RGB(0xFF, 0xFF, 0xFF))

#define START_Y 10
#define END_Y   (SCREEN_HEIGHT - 10)
#define START_X 10
#define END_X   (SCREEN_WIDTH_TOP - 10)

#define STEP_Y      10
#define N_CHARS_Y   ((END_Y - START_Y) / STEP_Y)
#define N_CHARS_X   (((END_X - START_X) / 8) + 1)

static char debugstr[N_CHARS_X * N_CHARS_Y] = { 0 };

void ClearScreen(u8* screen, int width, int color)
{
    for (int i = 0; i < (width * SCREEN_HEIGHT); i++) {
        *(screen++) = color >> 16;  // B
        *(screen++) = color >> 8;   // G
        *(screen++) = color & 0xFF; // R
    }
}

void ClearScreenFull(bool use_top)
{
    if (use_top) {
        ClearScreen(TOP_SCREEN0, SCREEN_WIDTH_TOP, BG_COLOR);
        ClearScreen(TOP_SCREEN1, SCREEN_WIDTH_TOP, BG_COLOR);
    } else {
        ClearScreen(BOT_SCREEN0, SCREEN_WIDTH_BOT, BG_COLOR);
        ClearScreen(BOT_SCREEN1, SCREEN_WIDTH_BOT, BG_COLOR);
    }
}

void DrawCharacter(u8* screen, int character, int x, int y, int color, int bgcolor)
{
    for (int yy = 0; yy < 8; yy++) {
        int xDisplacement = (x * BYTES_PER_PIXEL * SCREEN_HEIGHT);
        int yDisplacement = ((SCREEN_HEIGHT - (y + yy) - 1) * BYTES_PER_PIXEL);
        u8* screenPos = screen + xDisplacement + yDisplacement;

        u8 charPos = font[character * 8 + yy];
        for (int xx = 7; xx >= 0; xx--) {
            if ((charPos >> xx) & 1) {
                *(screenPos + 0) = color >> 16;  // B
                *(screenPos + 1) = color >> 8;   // G
                *(screenPos + 2) = color & 0xFF; // R
            } else {
                *(screenPos + 0) = bgcolor >> 16;  // B
                *(screenPos + 1) = bgcolor >> 8;   // G
                *(screenPos + 2) = bgcolor & 0xFF; // R
            }
            screenPos += BYTES_PER_PIXEL * SCREEN_HEIGHT;
        }
    }
}

void DrawString(u8* screen, const char *str, int x, int y, int color, int bgcolor)
{
    for (int i = 0; i < strlen(str); i++)
        DrawCharacter(screen, str[i], x + i * 8, y, color, bgcolor);
}

void DrawStringF(int x, int y, bool use_top, const char *format, ...)
{
    char str[256] = {};
    va_list va;

    va_start(va, format);
    vsnprintf(str, 256, format, va);
    va_end(va);

    if (use_top) {
        DrawString(TOP_SCREEN0, str, x, y, FONT_COLOR, BG_COLOR);
        DrawString(TOP_SCREEN1, str, x, y, FONT_COLOR, BG_COLOR);
    } else {
        DrawString(BOT_SCREEN0, str, x, y, FONT_COLOR, BG_COLOR);
        DrawString(BOT_SCREEN1, str, x, y, FONT_COLOR, BG_COLOR);
    }
}

void DebugClear()
{
    memset(debugstr, 0x00, N_CHARS_X * N_CHARS_Y);
    ClearScreenFull(true);
    LogWrite("");
}

void Debug(const char *format, ...)
{
    char tempstr[128] = { 0 }; // 128 instead of N_CHARS_X for log file 
    va_list va;
    
    va_start(va, format);
    vsnprintf(tempstr, 128, format, va);
    va_end(va);
    LogWrite(tempstr);
    
    memmove(debugstr + N_CHARS_X, debugstr, N_CHARS_X * (N_CHARS_Y - 1));
    snprintf(debugstr, N_CHARS_X, "%-*.*s", N_CHARS_X - 1, N_CHARS_X - 1, tempstr);
    
    int pos_y = START_Y;
    for (char* str = debugstr + (N_CHARS_X * (N_CHARS_Y - 1)); str >= debugstr; str -= N_CHARS_X) {
        if (str[0] != '\0') {
            DrawString(TOP_SCREEN0, str, START_X, pos_y, FONT_COLOR, BG_COLOR);
            DrawString(TOP_SCREEN1, str, START_X, pos_y, FONT_COLOR, BG_COLOR);
            pos_y += STEP_Y;
        }
    }
}

void ShowProgress(u64 current, u64 total)
{
    if (total > 0)
        DrawStringF(SCREEN_WIDTH_TOP - 40, SCREEN_HEIGHT - 20, true, "%3llu%%", (current * 100) / total);
    else
        DrawStringF(SCREEN_WIDTH_TOP - 40, SCREEN_HEIGHT - 20, true, "    ");
}
