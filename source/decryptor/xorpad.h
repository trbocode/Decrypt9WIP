#pragma once

#include "common.h"

// force slot 0x04 for CTRNAND padgen
#define PG_FORCESLOT4 (1<<0)

typedef struct {
    u32  keyslot;
    u32  setKeyY;
    u8   ctr[16];
    u8   keyY[16];
    u32  size_mb;
    u32  mode;
    char filename[180];
} __attribute__((packed, aligned(16))) PadInfo;

typedef struct {
    u8   ctr[16];
    u32  size_mb;
    char filename[180];
} __attribute__((packed)) SdInfoEntry;

typedef struct {
    u32 n_entries;
    SdInfoEntry entries[MAX_ENTRIES];
} __attribute__((packed, aligned(16))) SdInfo;

typedef struct {
    u8   ctr[16];
    u8   keyY[16];
    u32  size_mb;
    u8   reserved[4];
    u32  ncchFlag7;
    u32  ncchFlag3;
    u64  titleId;
    char filename[112];
} __attribute__((packed)) NcchInfoEntry;

typedef struct {
    u32 padding;
    u32 ncch_info_version;
    u32 n_entries;
    u8  reserved[4];
    NcchInfoEntry entries[MAX_ENTRIES];
} __attribute__((packed, aligned(16))) NcchInfo;


u32 CreatePad(PadInfo *info);
u32 SdInfoGen(SdInfo* info, const char* base_path);

// --> FEATURE FUNCTIONS <--
u32 NcchPadgen(u32 param);
u32 SdPadgen(u32 param);
u32 SdPadgenDirect(u32 param);

u32 CtrNandPadgen(u32 param);
u32 TwlNandPadgen(u32 param);
u32 Firm0Firm1Padgen(u32 param);
