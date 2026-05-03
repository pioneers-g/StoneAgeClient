/*
 * Stone Age Client - Asset File I/O
 * File reading and buffering functions
 */

#include <windows.h>
#include <stdlib.h>
#include "types.h"
#include "logger.h"

/* File handle structure - matches FUN_00492394 pattern */
typedef struct {
    HANDLE hFile;
    DWORD size;
    DWORD position;
    DWORD buffer_size;
    DWORD buffer_pos;
    u8* buffer;
    u16 flags;
    u16 reserved;
} FileHandle;

/*
 * Open file for reading - FUN_00492394 pattern
 */
void* file_open_read(const char* path) {
    FileHandle* fh;
    HANDLE hFile;
    DWORD size;

    hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        LOG_WARN("Failed to open file: %s (error: %lu)", path, GetLastError());
        return NULL;
    }

    fh = (FileHandle*)malloc(sizeof(FileHandle));
    if (!fh) {
        CloseHandle(hFile);
        return NULL;
    }

    size = GetFileSize(hFile, NULL);

    fh->hFile = hFile;
    fh->size = size;
    fh->position = 0;
    fh->buffer_size = 0x1000;  /* 4KB buffer */
    fh->buffer_pos = 0;
    fh->buffer = (u8*)malloc(fh->buffer_size);
    fh->flags = 0;

    return fh;
}

/*
 * Read from file
 */
int file_read(void* handle, void* buffer, u32 size) {
    FileHandle* fh = (FileHandle*)handle;
    DWORD bytes_read;

    if (!fh || !buffer || size == 0) {
        return 0;
    }

    if (!ReadFile(fh->hFile, buffer, size, &bytes_read, NULL)) {
        return 0;
    }

    fh->position += bytes_read;
    return (int)bytes_read;
}

/*
 * Close file
 */
void file_close(void* handle) {
    FileHandle* fh = (FileHandle*)handle;

    if (fh) {
        if (fh->hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(fh->hFile);
        }
        if (fh->buffer) {
            free(fh->buffer);
        }
        free(fh);
    }
}

/*
 * Check end of file
 */
int file_eof(void* handle) {
    FileHandle* fh = (FileHandle*)handle;

    if (!fh) {
        return 1;
    }

    return fh->position >= fh->size;
}

/*
 * Get file size
 */
u32 file_get_size(void* handle) {
    FileHandle* fh = (FileHandle*)handle;
    return fh ? fh->size : 0;
}

/*
 * Read structured entry - FUN_0049212c pattern
 */
int file_read_entry(void* handle, void* entry, u32 size) {
    return file_read(handle, entry, size);
}

/*
 * Seek in file
 */
int file_seek(void* handle, s32 offset, int origin) {
    FileHandle* fh = (FileHandle*)handle;
    DWORD result;

    if (!fh) return -1;

    result = SetFilePointer(fh->hFile, offset, NULL, origin);
    if (result == INVALID_SET_FILE_POINTER) {
        return -1;
    }

    fh->position = result;
    return 0;
}

/*
 * Get current position
 */
u32 file_tell(void* handle) {
    FileHandle* fh = (FileHandle*)handle;
    return fh ? fh->position : 0;
}
