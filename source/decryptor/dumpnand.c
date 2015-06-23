#include "dumpnand.h"
#include "fs.h"
#include "draw.h"
#include "platform.h"

#define BLOCK_SIZE (1*1024*1024)
#define NAND_SECTOR_SIZE 0x200
#define SECTORS_PER_READ 0x200
#define BUF1 ((volatile u8*)0x21000000)

u32 NandDumper() {
	u32 nand_size = 0;
	switch (GetUnitPlatform()) {
        case PLATFORM_3DS:
            nand_size = 0x3AF;
            break;
        case PLATFORM_N3DS:
            nand_size = 0x4D8;
            break;
    }
	
	Debug("Dumping NAND.bin. Size (MB): %u", nand_size);
    Debug("Filename: NAND.bin");

	if (!FileCreate("/NAND.bin", true)) {
		Debug("Failed to create the dump.");
        return 1;
	}
	
	Debug("Dumping...");
	
	u32 size_bytes = nand_size * 1024*1024;
	u32 n_sectors = size_bytes / NAND_SECTOR_SIZE;
	u32 n_sectors_100 = n_sectors / 100;
	for (u32 i = 0; i < n_sectors; i += SECTORS_PER_READ) {
		sdmmc_nand_readsectors(i, SECTORS_PER_READ, BUF1);
		FileWrite(BUF1, NAND_SECTOR_SIZE * SECTORS_PER_READ, i * NAND_SECTOR_SIZE);
		DrawStringF(SCREEN_HEIGHT - 40, SCREEN_WIDTH - 20, "%i%%", (i + SECTORS_PER_READ) / n_sectors_100);
	}
	
	DrawStringF(SCREEN_HEIGHT - 40, SCREEN_WIDTH - 20, "    ");
	FileClose();
	
	Debug("Finished dumping!");
	return 0;
}
