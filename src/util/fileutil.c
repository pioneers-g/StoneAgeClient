/*
 * Stone Age Client - File Utility System Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "types.h"
#include "fileutil.h"
#include "logger.h"

/*
 * Check if file exists
 */
int file_exists(const char* path) {
    FILE* fp;

    if (!path) return 0;

    fp = fopen(path, "rb");
    if (fp) {
        fclose(fp);
        return 1;
    }

    return 0;
}

/*
 * Check if directory exists
 */
int dir_exists(const char* path) {
    DWORD attr;

    if (!path) return 0;

    attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES) {
        return 0;
    }

    return (attr & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
}

/*
 * Create directory
 */
int dir_create(const char* path) {
    if (!path) return 0;

    if (CreateDirectoryA(path, NULL)) {
        return 1;
    }

    return (GetLastError() == ERROR_ALREADY_EXISTS) ? 1 : 0;
}

/*
 * Create directory recursively
 */
int dir_create_recursive(const char* path) {
    char buffer[512];
    char* p;
    int len;

    if (!path) return 0;

    strncpy(buffer, path, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    len = strlen(buffer);
    if (len == 0) return 0;

    /* Remove trailing slash */
    if (buffer[len - 1] == '\\' || buffer[len - 1] == '/') {
        buffer[len - 1] = '\0';
    }

    /* Create each directory in path */
    for (p = buffer + 1; *p; p++) {
        if (*p == '\\' || *p == '/') {
            *p = '\0';
            dir_create(buffer);
            *p = '\\';
        }
    }

    return dir_create(buffer);
}

/*
 * Delete file
 */
int file_delete(const char* path) {
    if (!path) return 0;
    return DeleteFileA(path) ? 1 : 0;
}

/*
 * Delete directory
 */
int dir_delete(const char* path) {
    if (!path) return 0;
    return RemoveDirectoryA(path) ? 1 : 0;
}

/*
 * Delete directory recursively
 */
int dir_delete_recursive(const char* path) {
    WIN32_FIND_DATAA find_data;
    HANDLE hfind;
    char search_path[512];
    char file_path[512];
    int result = 1;

    if (!path) return 0;

    snprintf(search_path, sizeof(search_path), "%s\\*.*", path);
    hfind = FindFirstFileA(search_path, &find_data);

    if (hfind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(find_data.cFileName, ".") == 0 ||
                strcmp(find_data.cFileName, "..") == 0) {
                continue;
            }

            snprintf(file_path, sizeof(file_path), "%s\\%s", path, find_data.cFileName);

            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (!dir_delete_recursive(file_path)) {
                    result = 0;
                }
            } else {
                if (!DeleteFileA(file_path)) {
                    result = 0;
                }
            }
        } while (FindNextFileA(hfind, &find_data));

        FindClose(hfind);
    }

    if (result) {
        result = RemoveDirectoryA(path) ? 1 : 0;
    }

    return result;
}

/*
 * Copy file
 */
int file_copy(const char* src, const char* dst) {
    FILE* fp_src;
    FILE* fp_dst;
    char buffer[4096];
    size_t bytes;

    if (!src || !dst) return 0;

    fp_src = fopen(src, "rb");
    if (!fp_src) return 0;

    fp_dst = fopen(dst, "wb");
    if (!fp_dst) {
        fclose(fp_src);
        return 0;
    }

    while ((bytes = fread(buffer, 1, sizeof(buffer), fp_src)) > 0) {
        fwrite(buffer, 1, bytes, fp_dst);
    }

    fclose(fp_src);
    fclose(fp_dst);

    return 1;
}

/*
 * Move file
 */
int file_move(const char* src, const char* dst) {
    if (!src || !dst) return 0;
    return MoveFileA(src, dst) ? 1 : 0;
}

/*
 * Rename file
 */
int file_rename(const char* old_path, const char* new_path) {
    return file_move(old_path, new_path);
}

/*
 * Get file size
 */
u32 file_get_size(const char* path) {
    WIN32_FILE_ATTRIBUTE_DATA info;

    if (!path) return 0;

    if (GetFileAttributesExA(path, GetFileExInfoStandard, &info)) {
        return info.nFileSizeLow;
    }

    return 0;
}

/*
 * Get file extension
 */
const char* file_get_extension(const char* path) {
    const char* p;

    if (!path) return NULL;

    p = strrchr(path, '.');
    if (p && (strchr(p, '\\') || strchr(p, '/'))) {
        return NULL;
    }

    return p;
}

/*
 * Get file name (without path)
 */
const char* file_get_name(const char* path) {
    const char* p;

    if (!path) return NULL;

    p = strrchr(path, '\\');
    if (!p) p = strrchr(path, '/');

    return p ? p + 1 : path;
}

/*
 * Get directory from path
 */
int file_get_directory(const char* path, char* buffer, u32 size) {
    const char* p;

    if (!path || !buffer || size == 0) return 0;

    p = strrchr(path, '\\');
    if (!p) p = strrchr(path, '/');

    if (p) {
        u32 len = p - path;
        if (len >= size) return 0;
        memcpy(buffer, path, len);
        buffer[len] = '\0';
        return 1;
    }

    buffer[0] = '\0';
    return 1;
}

/*
 * Get file name without extension
 */
int file_get_basename(const char* path, char* buffer, u32 size) {
    const char* name;
    const char* ext;
    u32 len;

    if (!path || !buffer || size == 0) return 0;

    name = file_get_name(path);
    if (!name) {
        buffer[0] = '\0';
        return 0;
    }

    ext = strrchr(name, '.');

    if (ext) {
        len = ext - name;
    } else {
        len = strlen(name);
    }

    if (len >= size) return 0;

    memcpy(buffer, name, len);
    buffer[len] = '\0';

    return 1;
}

/*
 * Combine paths
 */
int file_path_combine(char* buffer, u32 size, const char* dir, const char* name) {
    u32 dir_len;

    if (!buffer || size == 0) return 0;

    if (!dir || !dir[0]) {
        if (!name) return 0;
        strncpy(buffer, name, size - 1);
        buffer[size - 1] = '\0';
        return 1;
    }

    if (!name || !name[0]) {
        strncpy(buffer, dir, size - 1);
        buffer[size - 1] = '\0';
        return 1;
    }

    dir_len = strlen(dir);

    if (dir_len > 0 && (dir[dir_len - 1] == '\\' || dir[dir_len - 1] == '/')) {
        snprintf(buffer, size, "%s%s", dir, name);
    } else {
        snprintf(buffer, size, "%s\\%s", dir, name);
    }

    return 1;
}

/*
 * Read entire file into memory
 */
void* file_read_all(const char* path, u32* size) {
    FILE* fp;
    u8* data;
    u32 file_size;

    if (!path) return NULL;

    fp = fopen(path, "rb");
    if (!fp) return NULL;

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    data = (u8*)malloc(file_size + 1);
    if (!data) {
        fclose(fp);
        return NULL;
    }

    fread(data, 1, file_size, fp);
    data[file_size] = '\0';

    fclose(fp);

    if (size) *size = file_size;

    return data;
}

/*
 * Write data to file
 */
int file_write_all(const char* path, const void* data, u32 size) {
    FILE* fp;

    if (!path || !data) return 0;

    fp = fopen(path, "wb");
    if (!fp) return 0;

    fwrite(data, 1, size, fp);
    fclose(fp);

    return 1;
}

/*
 * Append data to file
 */
int file_append(const char* path, const void* data, u32 size) {
    FILE* fp;

    if (!path || !data) return 0;

    fp = fopen(path, "ab");
    if (!fp) return 0;

    fwrite(data, 1, size, fp);
    fclose(fp);

    return 1;
}

/*
 * Get file modification time
 */
u32 file_get_mtime(const char* path) {
    WIN32_FILE_ATTRIBUTE_DATA info;
    FILETIME ft;
    ULARGE_INTEGER uli;

    if (!path) return 0;

    if (GetFileAttributesExA(path, GetFileExInfoStandard, &info)) {
        ft = info.ftLastWriteTime;
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        return (u32)(uli.QuadPart / 10000000 - 11644473600LL);
    }

    return 0;
}

/*
 * Touch file (update modification time)
 */
int file_touch(const char* path) {
    FILE* fp;

    if (!path) return 0;

    fp = fopen(path, "ab");
    if (fp) {
        fclose(fp);
        return 1;
    }

    return 0;
}

/*
 * List files in directory
 */
int file_list(const char* dir, FileInfo* files, u32 max_count, const char* pattern) {
    WIN32_FIND_DATAA find_data;
    HANDLE hfind;
    char search_path[512];
    u32 count = 0;

    if (!dir || !files) return 0;

    if (pattern) {
        snprintf(search_path, sizeof(search_path), "%s\\%s", dir, pattern);
    } else {
        snprintf(search_path, sizeof(search_path), "%s\\*.*", dir);
    }

    hfind = FindFirstFileA(search_path, &find_data);
    if (hfind == INVALID_HANDLE_VALUE) {
        return 0;
    }

    do {
        if (strcmp(find_data.cFileName, ".") == 0 ||
            strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }

        if (count < max_count) {
            strncpy(files[count].name, find_data.cFileName, 255);
            files[count].name[255] = '\0';
            files[count].size = find_data.nFileSizeLow;
            files[count].is_dir = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
            files[count].mtime = 0;
            count++;
        }
    } while (FindNextFileA(hfind, &find_data) && count < max_count);

    FindClose(hfind);

    return count;
}

/*
 * Get current working directory
 */
int file_get_cwd(char* buffer, u32 size) {
    return GetCurrentDirectoryA(size, buffer) ? 1 : 0;
}

/*
 * Set current working directory
 */
int file_set_cwd(const char* path) {
    return SetCurrentDirectoryA(path) ? 1 : 0;
}

/*
 * Get executable path
 */
int file_get_exe_path(char* buffer, u32 size) {
    return GetModuleFileNameA(NULL, buffer, size) ? 1 : 0;
}

/*
 * Get executable directory
 */
int file_get_exe_dir(char* buffer, u32 size) {
    char path[512];

    if (!GetModuleFileNameA(NULL, path, sizeof(path))) {
        return 0;
    }

    return file_get_directory(path, buffer, size);
}

/* ========================================
 * BMP File Writing - FUN_004150d0, FUN_00415250
 * For screenshot capture functionality
 * ======================================== */

/*
 * Write 8-bit paletted BMP file - FUN_004150d0
 * Matches original binary behavior:
 * - Creates file with GENERIC_WRITE, CREATE_ALWAYS
 * - Writes BITMAPFILEHEADER (14 bytes)
 * - Writes BITMAPINFOHEADER (40 bytes)
 * - Writes palette (num_colors * 4 bytes)
 * - Writes pixel data bottom-up with row padding
 */
int file_write_bmp_paletted(const char* path, const u8* pixels, int offset,
                            int height, int width, int num_rows, int stride,
                            const void* palette, u32 num_colors) {
    HANDLE hfile;
    BMPFileHeader file_header;
    BMPInfoHeader info_header;
    DWORD bytes_written;
    int row_padding;
    int padded_width;
    u32 palette_size;
    u32 pixel_data_size;
    u32 file_size;
    const u8* row_ptr;
    u8 padding_bytes[4] = {0};
    int i;

    if (!path || !pixels) return 0;

    /* Calculate row padding to 4-byte boundary */
    row_padding = (4 - (width & 3)) & 3;
    padded_width = width + row_padding;
    palette_size = num_colors * 4;
    pixel_data_size = padded_width * num_rows;
    file_size = 14 + 40 + palette_size + pixel_data_size;

    /* Create file */
    hfile = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hfile == INVALID_HANDLE_VALUE) {
        LOG_DEBUG("Failed to create BMP file: %s", path);
        return 0;
    }

    /* Setup file header */
    memset(&file_header, 0, sizeof(file_header));
    file_header.type = 0x4D42;  /* "BM" */
    file_header.file_size = file_size;
    file_header.reserved1 = 0;
    file_header.reserved2 = 0;
    file_header.offset = 14 + 40 + palette_size;

    /* Setup info header */
    memset(&info_header, 0, sizeof(info_header));
    info_header.size = 40;
    info_header.width = width;
    info_header.height = num_rows;
    info_header.planes = 1;
    info_header.bit_count = 8;
    info_header.compression = 0;
    info_header.size_image = pixel_data_size;
    info_header.x_ppm = 0;
    info_header.y_ppm = 0;
    info_header.clr_used = num_colors;
    info_header.clr_important = 0;

    /* Write file header */
    if (!WriteFile(hfile, &file_header, 14, &bytes_written, NULL)) {
        CloseHandle(hfile);
        return 0;
    }

    /* Write info header */
    if (!WriteFile(hfile, &info_header, 40, &bytes_written, NULL)) {
        CloseHandle(hfile);
        return 0;
    }

    /* Write palette */
    if (palette && num_colors > 0) {
        if (!WriteFile(hfile, palette, palette_size, &bytes_written, NULL)) {
            CloseHandle(hfile);
            return 0;
        }
    }

    /* Write pixel data - bottom-up (BMP native format) */
    /* Original: starts from (height-1+num_rows) * stride + offset */
    row_ptr = pixels + (offset + (height - 1) * stride);

    for (i = 0; i < num_rows; i++) {
        /* Write row */
        if (!WriteFile(hfile, row_ptr, width, &bytes_written, NULL)) {
            CloseHandle(hfile);
            return 0;
        }

        /* Write padding */
        if (row_padding > 0) {
            if (!WriteFile(hfile, padding_bytes, row_padding, &bytes_written, NULL)) {
                CloseHandle(hfile);
                return 0;
            }
        }

        /* Move to previous row (bottom-up, so subtract stride) */
        row_ptr -= stride;
    }

    CloseHandle(hfile);
    LOG_DEBUG("Wrote paletted BMP: %s (%dx%d)", path, width, num_rows);
    return 1;
}

/*
 * Write 24-bit BMP file - FUN_00415250
 * Simple 24-bit BMP without palette
 */
int file_write_bmp_24bit(const char* path, const void* pixels, int width, int height) {
    HANDLE hfile;
    BMPFileHeader file_header;
    BMPInfoHeader info_header;
    DWORD bytes_written;
    u32 pixel_data_size;
    u32 file_size;

    if (!path || !pixels) return 0;

    /* 24-bit has no padding needed if width is aligned, but calculate anyway */
    pixel_data_size = width * height * 3;
    file_size = 14 + 40 + pixel_data_size;

    /* Create file */
    hfile = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hfile == INVALID_HANDLE_VALUE) {
        LOG_DEBUG("Failed to create BMP file: %s", path);
        return 0;
    }

    /* Setup file header */
    memset(&file_header, 0, sizeof(file_header));
    file_header.type = 0x4D42;  /* "BM" */
    file_header.file_size = file_size;
    file_header.reserved1 = 0;
    file_header.reserved2 = 0;
    file_header.offset = 54;  /* 14 + 40 */

    /* Setup info header */
    memset(&info_header, 0, sizeof(info_header));
    info_header.size = 40;
    info_header.width = width;
    info_header.height = height;
    info_header.planes = 1;
    info_header.bit_count = 24;
    info_header.compression = 0;
    info_header.size_image = pixel_data_size;
    info_header.x_ppm = 0;
    info_header.y_ppm = 0;
    info_header.clr_used = 0;
    info_header.clr_important = 0;

    /* Write file header */
    if (!WriteFile(hfile, &file_header, 14, &bytes_written, NULL)) {
        CloseHandle(hfile);
        return 0;
    }

    /* Write info header */
    if (!WriteFile(hfile, &info_header, 40, &bytes_written, NULL)) {
        CloseHandle(hfile);
        return 0;
    }

    /* Write pixel data */
    if (!WriteFile(hfile, pixels, pixel_data_size, &bytes_written, NULL)) {
        CloseHandle(hfile);
        return 0;
    }

    CloseHandle(hfile);
    LOG_DEBUG("Wrote 24-bit BMP: %s (%dx%d)", path, width, height);
    return 1;
}

/*
 * Write screenshot to BMP file
 * Handles both 8-bit paletted and 24-bit modes
 */
int file_write_screenshot(const char* path, const void* pixels, int width, int height,
                          int bpp, const void* palette, u32 num_colors) {
    if (!path || !pixels) return 0;

    if (bpp == 8) {
        /* 8-bit paletted mode */
        return file_write_bmp_paletted(path, (const u8*)pixels, 0, height,
                                       width, height, width,
                                       palette, num_colors);
    } else if (bpp == 24) {
        /* 24-bit mode */
        return file_write_bmp_24bit(path, pixels, width, height);
    }

    LOG_WARN("Unsupported BMP bit depth: %d", bpp);
    return 0;
}
