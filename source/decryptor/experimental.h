#pragma once

#include "common.h"

#define SD_SETUP_EMUNAND (1<<0)
#define SD_USE_STARTER     (1<<1)

typedef struct {
    u8  status;         // 0x80
    u8  chs_start[3];   // 0x01 0x01 0x00
    u8  type;           // 0x0C
    u8  chs_end[3];     // 0xFE 0xFF 0xFF
    u32 offset;         // 0x2000 (4MB offset, 512 byte sectors)
    u32 size;
} __attribute__((packed)) MbrPartitionInfo;

typedef struct {
    char text[446];
    MbrPartitionInfo partitions[4]; 
    u16  magic;         // 0xAA55
} __attribute__((packed)) MbrInfo;


// --> FEATURE FUNCTIONS <--
u32 FormatSdCard(u32 param);
u32 CloneSysNand(u32 param);
