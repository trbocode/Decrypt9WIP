#include "fs.h"
#include "draw.h"

bool DebugFileOpen(const char* path) {
    Debug("Opening %s ...", path);
    if (!FileOpen(path)) {
        Debug("Could not open %s!", path);
        return false;
    }
    
    return true;
}

bool DebugFileCreate(const char* path, bool truncate) {
    Debug("Creating %s ...", path);
    if (!FileCreate(path, truncate)) {
        Debug("Could not create %s!", path);
        return false;
    }

    return true;
}

bool DebugFileRead(void* buf, size_t size, size_t foffset) {
    size_t bytesRead = FileRead(buf, size, foffset);
    if (bytesRead != size) {
        Debug("ERROR, file is too small!");
        FileClose(); // <- need to keep this in mind!
        return false;
    }
    
    return true;
}

bool DebugFileWrite(void* buf, size_t size, size_t foffset) {
    size_t bytesWritten = FileWrite(buf, size, foffset);
    if (bytesWritten != size) {
        Debug("ERROR, SD card may be full!");
        FileClose(); // <- need to keep this in mind!
        return false;
    }
    
    return true;
}

bool DebugDirMake(const char* path) {
    if (!DirMake(path)) {
        Debug("ERROR, could not create folder!");
        return false;
    }
    
    return true;
}
