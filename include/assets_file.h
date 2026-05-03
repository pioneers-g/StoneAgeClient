/*
 * Stone Age Client - Asset File I/O Interface
 */

#ifndef ASSETS_FILE_H
#define ASSETS_FILE_H

#include "types.h"

/* ========================================
 * File I/O Functions - FUN_00492394 pattern
 * ======================================== */

void* file_open_read(const char* path);
int file_read(void* handle, void* buffer, u32 size);
void file_close(void* handle);
int file_eof(void* handle);
u32 file_get_size(void* handle);
int file_read_entry(void* handle, void* entry, u32 size);
int file_seek(void* handle, s32 offset, int origin);
u32 file_tell(void* handle);

/* File seek origins */
#define FILE_SEEK_SET   0
#define FILE_SEEK_CUR   1
#define FILE_SEEK_END   2

#endif /* ASSETS_FILE_H */
