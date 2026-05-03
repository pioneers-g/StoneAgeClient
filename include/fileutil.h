/*
 * Stone Age Client - File Utility System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef FILEUTIL_H
#define FILEUTIL_H

#include "types.h"

/* File info structure */
typedef struct {
    char name[256];
    u32 size;
    u32 mtime;
    int is_dir;

} FileInfo;

/* File existence */
int file_exists(const char* path);
int dir_exists(const char* path);

/* Directory operations */
int dir_create(const char* path);
int dir_create_recursive(const char* path);
int dir_delete(const char* path);
int dir_delete_recursive(const char* path);

/* File operations */
int file_delete(const char* path);
int file_copy(const char* src, const char* dst);
int file_move(const char* src, const char* dst);
int file_rename(const char* old_path, const char* new_path);
int file_touch(const char* path);

/* File info */
u32 file_get_size(const char* path);
u32 file_get_mtime(const char* path);

/* Path parsing */
const char* file_get_extension(const char* path);
const char* file_get_name(const char* path);
int file_get_directory(const char* path, char* buffer, u32 size);
int file_get_basename(const char* path, char* buffer, u32 size);
int file_path_combine(char* buffer, u32 size, const char* dir, const char* name);

/* Read/Write */
void* file_read_all(const char* path, u32* size);
int file_write_all(const char* path, const void* data, u32 size);
int file_append(const char* path, const void* data, u32 size);

/* Directory listing */
int file_list(const char* dir, FileInfo* files, u32 max_count, const char* pattern);

/* Working directory */
int file_get_cwd(char* buffer, u32 size);
int file_set_cwd(const char* path);

/* Executable */
int file_get_exe_path(char* buffer, u32 size);
int file_get_exe_dir(char* buffer, u32 size);

/* ========================================
 * BMP File Writing - FUN_004150d0, FUN_00415250
 * For screenshot capture functionality
 * ======================================== */

/* BMP file header structure (14 bytes) */
#pragma pack(push, 1)
typedef struct {
    u16 type;           /* "BM" (0x4D42) */
    u32 file_size;      /* Total file size */
    u16 reserved1;      /* Reserved */
    u16 reserved2;      /* Reserved */
    u32 offset;         /* Offset to pixel data */
} BMPFileHeader;
#pragma pack(pop)

/* BMP info header (40 bytes) */
#pragma pack(push, 1)
typedef struct {
    u32 size;           /* Header size (40) */
    s32 width;          /* Image width */
    s32 height;         /* Image height */
    u16 planes;         /* Number of planes (1) */
    u16 bit_count;      /* Bits per pixel (8, 24) */
    u32 compression;    /* Compression (0 = none) */
    u32 size_image;     /* Image data size */
    s32 x_ppm;          /* X pixels per meter */
    s32 y_ppm;          /* Y pixels per meter */
    u32 clr_used;       /* Colors used */
    u32 clr_important;  /* Important colors */
} BMPInfoHeader;
#pragma pack(pop)

/* RGB palette entry (4 bytes) */
#pragma pack(push, 1)
typedef struct {
    u8 blue;
    u8 green;
    u8 red;
    u8 reserved;
} BMPPaletteEntry;
#pragma pack(pop)

/*
 * Write 8-bit paletted BMP file - FUN_004150d0
 * @param path: Output file path
 * @param pixels: Pixel data (top-down)
 * @param offset: Offset into pixel data
 * @param height: Image height (for row calculation)
 * @param width: Row width in bytes
 * @param num_rows: Number of rows to write
 * @param stride: Row stride (negative for bottom-up)
 * @param palette: Palette data (BGRA format)
 * @param num_colors: Number of palette entries
 * @return 1 on success, 0 on failure
 */
int file_write_bmp_paletted(const char* path, const u8* pixels, int offset,
                            int height, int width, int num_rows, int stride,
                            const void* palette, u32 num_colors);

/*
 * Write 24-bit BMP file - FUN_00415250
 * @param path: Output file path
 * @param pixels: Pixel data (BGR format, bottom-up)
 * @param width: Image width
 * @param height: Image height
 * @return 1 on success, 0 on failure
 */
int file_write_bmp_24bit(const char* path, const void* pixels, int width, int height);

/*
 * Write screenshot to BMP file
 * @param path: Output file path
 * @param pixels: Pixel data
 * @param width: Image width
 * @param height: Image height
 * @param bpp: Bits per pixel (8 or 24)
 * @param palette: Palette for 8-bit mode (NULL for 24-bit)
 * @param num_colors: Number of palette entries
 * @return 1 on success, 0 on failure
 */
int file_write_screenshot(const char* path, const void* pixels, int width, int height,
                          int bpp, const void* palette, u32 num_colors);

#endif /* FILEUTIL_H */
