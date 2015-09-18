#include "nandio.h"
#include "fatfs/sdmmc.h"

static u32 emunand_offset_sectors = 0;


u32 GetNandSize()
{
    return getMMCDevice(0)->total_size * 0x200;
}

u32 CheckEmuNand()
{
    u8 buffer[0x200];
    u32 nand_size_sectors = getMMCDevice(0)->total_size;
    bool found = false;
    
    // check for EmuNAND NCSD header after full NAND size
    sdmmc_sdcard_readsectors(nand_size_sectors, 1, buffer);
    if (buffer[0x100] == 'N' && buffer[0x101] == 'C' && buffer[0x102] == 'S' && buffer[0x103] == 'D')
        found = true;
    
    return (found) ? nand_size_sectors : 0;
}

u32 SetEmuNand(bool use_emunand)
{
    emunand_offset_sectors = (use_emunand) ? CheckEmuNand() : 0;
    return emunand_offset_sectors;
}

int ReadNandSectors(u32 sector_no, u32 numsectors, u8 *out)
{
    if (emunand_offset_sectors) {
        if (sector_no == 0) {
            int errorcode = sdmmc_sdcard_readsectors(emunand_offset_sectors, 1, out);
            if (errorcode) return errorcode;
            sector_no = 1;
            numsectors--;
            out += 0x200;
        }
        return sdmmc_sdcard_readsectors(sector_no, numsectors, out);
    } else return sdmmc_nand_readsectors(sector_no, numsectors, out);
}

int WriteNandSectors(u32 sector_no, u32 numsectors, u8 *in)
{
    if (emunand_offset_sectors) {
        if (sector_no == 0) {
            int errorcode = sdmmc_sdcard_writesectors(emunand_offset_sectors, 1, in);
            if (errorcode) return errorcode;
            sector_no = 1;
            numsectors--;
            in += 0x200;
        }
        return sdmmc_sdcard_writesectors(sector_no, numsectors, in);
    } else return sdmmc_nand_writesectors(sector_no, numsectors, in);
}
