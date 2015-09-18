#pragma once

#include "common.h"


u32 GetNandSize();
u32 CheckEmuNand();
u32 SetEmuNand(bool use_emunand);
int ReadNandSectors(u32 sector_no, u32 numsectors, u8 *out);
int WriteNandSectors(u32 sector_no, u32 numsectors, u8 *in);
