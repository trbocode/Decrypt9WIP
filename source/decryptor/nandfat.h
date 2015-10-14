#pragma once

#include "decryptor/nand.h"
#include "common.h"

#define MAX_ENTRIES 1024


u32 SeekFileInNand(u32* offset, u32* size, const char* path, PartitionInfo* partition);
u32 DebugSeekFileInNand(u32* offset, u32* size, const char* filename, const char* path, PartitionInfo* partition);
