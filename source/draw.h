// Copyright 2013 Normmatt
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "common.h"

#define BYTES_PER_PIXEL 3
#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 400

#define SCREEN_SIZE (BYTES_PER_PIXEL * SCREEN_HEIGHT * SCREEN_WIDTH)

#define RGB(r,g,b) (r<<24|b<<16|g<<8|r)

#ifdef EXEC_GATEWAY
    #define TOP_SCREEN0 (u8*)(*(u32*)((uint32_t)0x080FFFC0 + 4 * (*(u32*)0x080FFFD8 & 1)))
    #define BOT_SCREEN0 (u8*)(*(u32*)0x080FFFD0)
    #define TOP_SCREEN1 TOP_SCREEN0
    #define BOT_SCREEN1 (u8*)(*(u32*)0x080FFFD4)
#elif defined(EXEC_BOOTSTRAP)
    #define TOP_SCREEN0 (u8*)(0x20000000)
    #define TOP_SCREEN1 (u8*)(0x20046500)
    #define BOT_SCREEN0 (u8*)(0x2008CA00)
    #define BOT_SCREEN1 (u8*)(0x200C4E00)
#else
    #error "Unknown execution method"
#endif

//Colors Macros
#define BLACK 		RGB(0, 0, 0)
#define WHITE		RGB(255, 255, 255)
#define RED			RGB(255, 0, 0)
#define GREEN		RGB(0, 255, 0)
#define BLUE		RGB(0, 0, 255)
#define GREY		RGB(0x77, 0x77, 0x77)
#define TRANSPARENT RGB(255, 0, 255)
#define PURPLE		RGB(102, 0, 255)

void ClearScreen(unsigned char *screen, int color);
void DrawCharacter(unsigned char *screen, int character, int x, int y, int color, int bgcolor);

void DrawString(unsigned char *screen, const char *str, int x, int y, int color, int bgcolor);
void DrawStringF(int x, int y, const char *format, ...);
void DrawStringM(int x, int y, const char *format, ...);

void DebugClear();
void ConsoleClear();
void Debug(const char *format, ...);

void ShowProgress(u32 current, u32 total);

void DrawPixel(int x, int y, int color, int screen);

void DrawUI();
void DrawFreeSpace();
void DebugClearAll();
void DrawSplash(char* splash_file, u32 top_screen);
void DrawTopSplash(char splash_file[]);
void DrawBottomSplash(char splash_file[]);
