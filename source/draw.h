// Copyright 2013 Normmatt
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "common.h"

#define BYTES_PER_PIXEL 3
#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH_TOP 400
#define SCREEN_WIDTH_BOT 320

#define RGB(r,g,b) (r<<24|b<<16|g<<8|r)

#ifdef EXEC_GATEWAY
	#define TOP_SCREEN0 (u8*)(*(u32*)((uint32_t)0x080FFFC0 + 4 * (*(u32*)0x080FFFD8 & 1)))
	#define BOT_SCREEN0 (u8*)(*(u32*)0x080FFFD4)
	#define TOP_SCREEN1 TOP_SCREEN0
	#define BOT_SCREEN1 BOT_SCREEN0
#elif defined(EXEC_BOOTSTRAP)
	#define TOP_SCREEN0 (u8*)(0x20000000)
	#define TOP_SCREEN1 (u8*)(0x20046500)
	#define BOT_SCREEN0 (u8*)(0x2008CA00)
	#define BOT_SCREEN1 (u8*)(0x200C4E00)
#else
	#error "Unknown execution method"
#endif

void ClearScreen(unsigned char *screen, int width, int color);
void ClearScreenFull(bool use_top);
void DrawCharacter(unsigned char *screen, int character, int x, int y, int color, int bgcolor);
void DrawString(unsigned char *screen, const char *str, int x, int y, int color, int bgcolor);
void DrawStringF(int x, int y, bool use_top, const char *format, ...);

void DebugClear();
void Debug(const char *format, ...);

void ShowProgress(u64 current, u64 total);
