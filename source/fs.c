#include "fs.h"

#include <stdio.h>
#include <string.h>
#include "fatfs/ff.h"

static FATFS fs;
static FIL file;
static DIR dir;

bool InitFS()
{
#ifndef EXEC_GATEWAY
    // TODO: Magic?
    *(u32*)0x10000020 = 0;
    *(u32*)0x10000020 = 0x340;
#endif
    return f_mount(&fs, "0:", 0) == FR_OK;
}

void DeinitFS()
{
    f_mount(NULL, "0:", 0);
}

bool FileOpen(const char* path)
{
    unsigned flags = FA_READ | FA_WRITE | FA_OPEN_EXISTING;
    #ifdef WORK_DIR
    char workpath[256];
    snprintf(workpath, 256, "%s/%s", WORK_DIR, (path[0] == '/') ? path + 1 : path);
    bool ret = (f_open(&file, workpath, flags) == FR_OK);
    #else
    bool ret = (f_open(&file, path, flags) == FR_OK);
    #endif
    f_lseek(&file, 0);
    f_sync(&file);
    return ret;
}

bool FileCreate(const char* path, bool truncate)
{
    unsigned flags = FA_READ | FA_WRITE;
    flags |= truncate ? FA_CREATE_ALWAYS : FA_OPEN_ALWAYS;
    #ifdef WORK_DIR
    f_mkdir(WORK_DIR);
    char workpath[256];
    snprintf(workpath, 256, "%s/%s", WORK_DIR, (path[0] == '/') ? path + 1 : path);
    bool ret = (f_open(&file, workpath, flags) == FR_OK);
    #else
    bool ret = (f_open(&file, path, flags) == FR_OK);
    #endif
    f_lseek(&file, 0);
    f_sync(&file);
    return ret;
}

size_t FileRead(void* buf, size_t size, size_t foffset)
{
    UINT bytes_read = 0;
    f_lseek(&file, foffset);
    f_read(&file, buf, size, &bytes_read);
    return bytes_read;
}

size_t FileWrite(void* buf, size_t size, size_t foffset)
{
    UINT bytes_written = 0;
    f_lseek(&file, foffset);
    f_write(&file, buf, size, &bytes_written);
    f_sync(&file);
    return bytes_written;
}

size_t FileGetSize()
{
    return f_size(&file);
}

void FileClose()
{
    f_close(&file);
}

bool DirMake(const char* path)
{
    #ifdef WORK_DIR
    f_mkdir(WORK_DIR);
    char workpath[256];
    snprintf(workpath, 256, "%s/%s", WORK_DIR, (path[0] == '/') ? path + 1 : path);
    FRESULT res = f_mkdir(workpath);
    #else
    FRESULT res = f_mkdir(path);
    #endif
    bool ret = (res == FR_OK) || (res == FR_EXIST);
    return ret;
}

bool DirOpen(const char* path)
{
    #ifdef WORK_DIR
    char workpath[256];
    snprintf(workpath, 256, "%s/%s", WORK_DIR, (path[0] == '/') ? path + 1 : path);
    bool ret = (f_opendir(&dir, workpath) == FR_OK);
    #else
    bool ret = (f_opendir(&dir, path) == FR_OK);
    #endif
    return ret;
}

bool DirRead(char* filename)
{
    FILINFO fno;
    bool ret = false;
    while (f_readdir(&dir, &fno) == FR_OK) {
        if (fno.fname[0] != 0) break;
        if ((fno.fname[0] != '.') && !(fno.fattrib & AM_DIR)) {
            ret = true;
            break;
        }
    }
    if (ret) strcpy(filename, fno.fname); // only short filenames
    return ret;
}

void DirClose()
{
    f_closedir(&dir);
}

static uint64_t ClustersToBytes(FATFS* fs, DWORD clusters)
{
    uint64_t sectors = clusters * fs->csize;
#if _MAX_SS != _MIN_SS
    return sectors * fs->ssize;
#else
    return sectors * _MAX_SS;
#endif
}

uint64_t RemainingStorageSpace()
{
    DWORD free_clusters;
    FATFS *fs2;
    FRESULT res = f_getfree("0:", &free_clusters, &fs2);
    if (res)
        return -1;

    return ClustersToBytes(&fs, free_clusters);
}
