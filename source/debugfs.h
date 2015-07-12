#pragma once

#include "common.h"

/** Opens existing files */
bool DebugFileOpen(const char* path);

/** Opens new files (and creates them if they don't already exist) */
bool DebugFileCreate(const char* path, bool truncate);

/** Reads contents of the opened file */
bool DebugFileRead(void* buf, size_t size, size_t foffset);

/** Writes to the opened file */
bool DebugFileWrite(void* buf, size_t size, size_t foffset);

/** Creates a directory */
bool DebugDirMake(const char* path);
