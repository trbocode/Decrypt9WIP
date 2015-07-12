#pragma once

#include "common.h"

#define WORKDIR "/Decrypt9"

bool InitFS();
void DeinitFS();

/** Opens existing files */
bool FileOpen(const char* path);

/** Opens new files (and creates them if they don't already exist) */
bool FileCreate(const char* path, bool truncate);

/** Reads contents of the opened file */
size_t FileRead(void* buf, size_t size, size_t foffset);

/** Writes to the opened file */
size_t FileWrite(void* buf, size_t size, size_t foffset);

/** Gets the size of the opened file */
size_t FileGetSize();

/** Creates a directory */
bool DirMake(const char* path);

/** Opens an existing directory */
bool DirOpen(const char* path);

/** Opens first / next file in directory */
bool DirFileOpen();

/** Gets remaining space on SD card in bytes */
uint64_t RemainingStorageSpace();

void FileClose();

void DirClose();

bool FileOpenSplash(const char* path);
size_t FileReadSplash(void* buf, size_t size, size_t foffset);
