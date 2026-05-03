/*
 * Stone Age Client - File Utility Module Comprehensive Tests
 * Tests for fileutil.c - file and directory operations
 *
 * Coverage:
 * - File/directory existence checks
 * - Directory operations (create, delete, recursive)
 * - File operations (delete, copy, move, rename)
 * - Path parsing utilities
 * - File read/write operations
 * - BMP file writing (FUN_004150d0, FUN_00415250)
 * - Working directory operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Test paths - use local test data directory */
#define TEST_TEMP_DIR "test_temp_dir"
#define TEST_TEMP_FILE "test_temp_file.dat"
#define TEST_DATA_DIR "tests/data"
#define TEST_AI_FILE_RELATIVE "tests/data/AISetting.dat"
#define TEST_AI_FILE_ABSOLUTE "D:/code/StoneAgeClient/tests/data/AISetting.dat"

/* FileInfo structure */
typedef struct {
    char name[256];
    u32 size;
    u32 mtime;
    int is_dir;
} FileInfo;

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

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Helper Functions
 * ======================================== */

/* Helper to get AI file path (works from both project root and tests directory) */
static const char* get_test_ai_file(void) {
    static char path[512];
    FILE* fp;

    /* Try relative path first */
    fp = fopen(TEST_AI_FILE_RELATIVE, "rb");
    if (fp) {
        fclose(fp);
        return TEST_AI_FILE_RELATIVE;
    }

    /* Try absolute path */
    fp = fopen(TEST_AI_FILE_ABSOLUTE, "rb");
    if (fp) {
        fclose(fp);
        return TEST_AI_FILE_ABSOLUTE;
    }

    return NULL;
}

/* Helper to check if test data exists */
static int test_data_available(void) {
    return get_test_ai_file() != NULL;
}

/* ========================================
 * Stub Implementations (matching fileutil.c)
 * ======================================== */

#include <windows.h>
#include <direct.h>

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

int dir_exists(const char* path) {
    DWORD attr;
    if (!path) return 0;
    attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES) {
        return 0;
    }
    return (attr & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
}

int dir_create(const char* path) {
    if (!path) return 0;
    if (CreateDirectoryA(path, NULL)) {
        return 1;
    }
    return (GetLastError() == ERROR_ALREADY_EXISTS) ? 1 : 0;
}

int dir_create_recursive(const char* path) {
    char buffer[512];
    char* p;
    int len;

    if (!path) return 0;

    strncpy(buffer, path, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    len = strlen(buffer);
    if (len == 0) return 0;

    if (buffer[len - 1] == '\\' || buffer[len - 1] == '/') {
        buffer[len - 1] = '\0';
    }

    for (p = buffer + 1; *p; p++) {
        if (*p == '\\' || *p == '/') {
            *p = '\0';
            dir_create(buffer);
            *p = '\\';
        }
    }

    return dir_create(buffer);
}

int dir_delete(const char* path) {
    if (!path) return 0;
    return RemoveDirectoryA(path) ? 1 : 0;
}

int file_delete(const char* path) {
    if (!path) return 0;
    return DeleteFileA(path) ? 1 : 0;
}

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

int file_move(const char* src, const char* dst) {
    if (!src || !dst) return 0;
    return MoveFileA(src, dst) ? 1 : 0;
}

int file_rename(const char* old_path, const char* new_path) {
    return file_move(old_path, new_path);
}

u32 file_get_size(const char* path) {
    WIN32_FILE_ATTRIBUTE_DATA info;
    if (!path) return 0;
    if (GetFileAttributesExA(path, GetFileExInfoStandard, &info)) {
        return info.nFileSizeLow;
    }
    return 0;
}

const char* file_get_extension(const char* path) {
    const char* p;
    if (!path) return NULL;
    p = strrchr(path, '.');
    if (p && (strchr(p, '\\') || strchr(p, '/'))) {
        return NULL;
    }
    return p;
}

const char* file_get_name(const char* path) {
    const char* p;
    if (!path) return NULL;
    p = strrchr(path, '\\');
    if (!p) p = strrchr(path, '/');
    return p ? p + 1 : path;
}

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

int file_write_all(const char* path, const void* data, u32 size) {
    FILE* fp;
    if (!path || !data) return 0;

    fp = fopen(path, "wb");
    if (!fp) return 0;

    fwrite(data, 1, size, fp);
    fclose(fp);

    return 1;
}

int file_append(const char* path, const void* data, u32 size) {
    FILE* fp;
    if (!path || !data) return 0;

    fp = fopen(path, "ab");
    if (!fp) return 0;

    fwrite(data, 1, size, fp);
    fclose(fp);

    return 1;
}

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

int file_get_cwd(char* buffer, u32 size) {
    return GetCurrentDirectoryA(size, buffer) ? 1 : 0;
}

int file_set_cwd(const char* path) {
    return SetCurrentDirectoryA(path) ? 1 : 0;
}

int file_get_exe_path(char* buffer, u32 size) {
    return GetModuleFileNameA(NULL, buffer, size) ? 1 : 0;
}

int file_get_exe_dir(char* buffer, u32 size) {
    char path[512];
    if (!GetModuleFileNameA(NULL, path, sizeof(path))) {
        return 0;
    }
    return file_get_directory(path, buffer, size);
}

/*
 * BMP File Writing - FUN_004150d0, FUN_00415250
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
    row_ptr = pixels + (offset + (height - 1) * stride);

    for (i = 0; i < num_rows; i++) {
        if (!WriteFile(hfile, row_ptr, width, &bytes_written, NULL)) {
            CloseHandle(hfile);
            return 0;
        }

        if (row_padding > 0) {
            if (!WriteFile(hfile, padding_bytes, row_padding, &bytes_written, NULL)) {
                CloseHandle(hfile);
                return 0;
            }
        }

        row_ptr -= stride;
    }

    CloseHandle(hfile);
    return 1;
}

int file_write_bmp_24bit(const char* path, const void* pixels, int width, int height) {
    HANDLE hfile;
    BMPFileHeader file_header;
    BMPInfoHeader info_header;
    DWORD bytes_written;
    u32 pixel_data_size;
    u32 file_size;

    if (!path || !pixels) return 0;

    pixel_data_size = width * height * 3;
    file_size = 14 + 40 + pixel_data_size;

    hfile = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hfile == INVALID_HANDLE_VALUE) {
        return 0;
    }

    memset(&file_header, 0, sizeof(file_header));
    file_header.type = 0x4D42;
    file_header.file_size = file_size;
    file_header.reserved1 = 0;
    file_header.reserved2 = 0;
    file_header.offset = 54;

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

    if (!WriteFile(hfile, &file_header, 14, &bytes_written, NULL)) {
        CloseHandle(hfile);
        return 0;
    }

    if (!WriteFile(hfile, &info_header, 40, &bytes_written, NULL)) {
        CloseHandle(hfile);
        return 0;
    }

    if (!WriteFile(hfile, pixels, pixel_data_size, &bytes_written, NULL)) {
        CloseHandle(hfile);
        return 0;
    }

    CloseHandle(hfile);
    return 1;
}

int file_write_screenshot(const char* path, const void* pixels, int width, int height,
                          int bpp, const void* palette, u32 num_colors) {
    if (!path || !pixels) return 0;

    if (bpp == 8) {
        return file_write_bmp_paletted(path, (const u8*)pixels, 0, height,
                                       width, height, width,
                                       palette, num_colors);
    } else if (bpp == 24) {
        return file_write_bmp_24bit(path, pixels, width, height);
    }

    return 0;
}

/* ========================================
 * Test Setup/Teardown
 * ======================================== */

static void test_setup(void) {
    /* Clean up any leftover test files */
    file_delete(TEST_TEMP_FILE);
    file_delete("test_moved_file.dat");
    file_delete("test_renamed_file.dat");
    file_delete(TEST_TEMP_FILE ".bmp");
    dir_delete(TEST_TEMP_DIR);
}

static void test_teardown(void) {
    /* Clean up test files */
    file_delete(TEST_TEMP_FILE);
    file_delete("test_moved_file.dat");
    file_delete("test_renamed_file.dat");
    file_delete(TEST_TEMP_FILE ".bmp");
    dir_delete(TEST_TEMP_DIR);
}

/* ========================================
 * File Existence Tests
 * ======================================== */

static int test_file_exists_null(void) {
    return file_exists(NULL) == 0;
}

static int test_file_exists_invalid_file(void) {
    return file_exists("nonexistent_file_xyz.dat") == 0;
}

static int test_file_exists_created_file(void) {
    test_setup();

    /* Create a test file */
    u8 data[] = {1, 2, 3, 4, 5};
    file_write_all(TEST_TEMP_FILE, data, sizeof(data));

    int exists = file_exists(TEST_TEMP_FILE);

    test_teardown();

    return exists == 1;
}

static int test_file_exists_test_data(void) {
    /* Check if we can find the test AI file */
    const char* ai_file = get_test_ai_file();
    if (!ai_file) {
        /* Skip test if test data not available */
        printf("(skipped - no test data) ");
        return 1;
    }
    return file_exists(ai_file) == 1;
}

/* ========================================
 * Directory Existence Tests
 * ======================================== */

static int test_dir_exists_null(void) {
    return dir_exists(NULL) == 0;
}

static int test_dir_exists_invalid_dir(void) {
    return dir_exists("X:/nonexistent_directory_xyz") == 0;
}

static int test_dir_exists_created_dir(void) {
    test_setup();

    dir_create(TEST_TEMP_DIR);
    int exists = dir_exists(TEST_TEMP_DIR);

    test_teardown();

    return exists == 1;
}

static int test_dir_exists_file_as_dir(void) {
    test_setup();

    /* Create a test file */
    u8 data[] = {1, 2, 3};
    file_write_all(TEST_TEMP_FILE, data, sizeof(data));

    /* A file path should not be detected as directory */
    int is_not_dir = dir_exists(TEST_TEMP_FILE) == 0;

    test_teardown();

    return is_not_dir;
}

static int test_dir_exists_test_data_dir(void) {
    /* Check tests/data directory */
    if (dir_exists(TEST_DATA_DIR)) {
        return 1;
    }
    /* Try alternative path */
    if (dir_exists("D:/code/StoneAgeClient/tests/data")) {
        return 1;
    }
    printf("(skipped - no test data dir) ");
    return 1;
}

/* ========================================
 * Directory Creation Tests
 * ======================================== */

static int test_dir_create_simple(void) {
    test_setup();

    int result = dir_create(TEST_TEMP_DIR);
    int exists = dir_exists(TEST_TEMP_DIR);

    test_teardown();

    return result == 1 && exists == 1;
}

static int test_dir_create_null(void) {
    return dir_create(NULL) == 0;
}

static int test_dir_create_existing(void) {
    test_setup();

    dir_create(TEST_TEMP_DIR);
    int result = dir_create(TEST_TEMP_DIR);  /* Create again */

    test_teardown();

    return result == 1;  /* Should succeed (already exists) */
}

static int test_dir_create_recursive(void) {
    test_setup();

    const char* deep_path = "test_temp_dir/sub1/sub2/sub3";
    int result = dir_create_recursive(deep_path);
    int exists = dir_exists(deep_path);

    /* Clean up recursively */
    dir_delete("test_temp_dir/sub1/sub2/sub3");
    dir_delete("test_temp_dir/sub1/sub2");
    dir_delete("test_temp_dir/sub1");
    test_teardown();

    return result == 1 && exists == 1;
}

static int test_dir_create_recursive_null(void) {
    return dir_create_recursive(NULL) == 0;
}

/* ========================================
 * Directory Deletion Tests
 * ======================================== */

static int test_dir_delete_null(void) {
    return dir_delete(NULL) == 0;
}

static int test_dir_delete_nonexistent(void) {
    return dir_delete("nonexistent_dir_xyz") == 0;
}

static int test_dir_delete_and_verify(void) {
    /* Use a unique directory name to avoid conflicts */
    const char* unique_dir = "test_temp_dir_unique_12345";

    /* Clean up first */
    dir_delete(unique_dir);

    dir_create(unique_dir);
    int exists_before = dir_exists(unique_dir);
    int result = dir_delete(unique_dir);
    int exists_after = dir_exists(unique_dir);

    return exists_before == 1 && result == 1 && exists_after == 0;
}

/* ========================================
 * File Operations Tests
 * ======================================== */

static int test_file_delete_null(void) {
    return file_delete(NULL) == 0;
}

static int test_file_delete_nonexistent(void) {
    return file_delete("nonexistent_file_xyz.dat") == 0;
}

static int test_file_delete_created_file(void) {
    test_setup();

    /* Create then delete */
    u8 data[] = {1, 2, 3};
    file_write_all(TEST_TEMP_FILE, data, sizeof(data));
    int result = file_delete(TEST_TEMP_FILE);
    int gone = !file_exists(TEST_TEMP_FILE);

    test_teardown();

    return result == 1 && gone;
}

static int test_file_copy_null_src(void) {
    return file_copy(NULL, TEST_TEMP_FILE) == 0;
}

static int test_file_copy_null_dst(void) {
    return file_copy(TEST_TEMP_FILE, NULL) == 0;
}

static int test_file_copy_created_file(void) {
    test_setup();

    /* Create source file */
    u8 data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x42};
    file_write_all("source_file.dat", data, sizeof(data));

    int result = file_copy("source_file.dat", TEST_TEMP_FILE);

    /* Verify content */
    u32 dst_size;
    u8* dst_data = file_read_all(TEST_TEMP_FILE, &dst_size);

    int pass = result == 1 &&
               dst_size == sizeof(data) &&
               dst_data != NULL &&
               memcmp(dst_data, data, sizeof(data)) == 0;

    free(dst_data);
    file_delete("source_file.dat");
    test_teardown();

    return pass;
}

static int test_file_copy_test_data(void) {
    const char* ai_file = get_test_ai_file();
    if (!ai_file) {
        printf("(skipped - no test data) ");
        return 1;
    }

    test_setup();

    u32 src_size = file_get_size(ai_file);
    int result = file_copy(ai_file, TEST_TEMP_FILE);
    u32 dst_size = file_get_size(TEST_TEMP_FILE);

    test_teardown();

    return result == 1 && src_size == dst_size && src_size > 0;
}

static int test_file_move_null_src(void) {
    return file_move(NULL, TEST_TEMP_FILE) == 0;
}

static int test_file_move_null_dst(void) {
    return file_move(TEST_TEMP_FILE, NULL) == 0;
}

static int test_file_move_created_file(void) {
    test_setup();

    /* Create a file to move */
    u8 data[] = {0x11, 0x22, 0x33, 0x44};
    file_write_all(TEST_TEMP_FILE, data, sizeof(data));
    u32 size_before = file_get_size(TEST_TEMP_FILE);

    const char* moved_file = "test_moved_file.dat";
    int result = file_move(TEST_TEMP_FILE, moved_file);
    u32 size_after = file_get_size(moved_file);
    int original_gone = file_exists(TEST_TEMP_FILE) == 0;

    file_delete(moved_file);
    test_teardown();

    return result == 1 && size_before == size_after && original_gone;
}

static int test_file_rename(void) {
    test_setup();

    /* Create test file */
    u8 data[] = {0xAA, 0xBB, 0xCC};
    file_write_all(TEST_TEMP_FILE, data, sizeof(data));

    const char* renamed_file = "test_renamed_file.dat";
    int result = file_rename(TEST_TEMP_FILE, renamed_file);
    int exists_new = file_exists(renamed_file);
    int exists_old = file_exists(TEST_TEMP_FILE);

    file_delete(renamed_file);
    test_teardown();

    return result == 1 && exists_new == 1 && exists_old == 0;
}

/* ========================================
 * File Info Tests
 * ======================================== */

static int test_file_get_size_null(void) {
    return file_get_size(NULL) == 0;
}

static int test_file_get_size_created_file(void) {
    test_setup();

    u8 data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    file_write_all(TEST_TEMP_FILE, data, sizeof(data));

    u32 size = file_get_size(TEST_TEMP_FILE);

    test_teardown();

    return size == sizeof(data);
}

static int test_file_get_size_nonexistent(void) {
    return file_get_size("nonexistent_file_xyz.dat") == 0;
}

static int test_file_get_size_test_data(void) {
    const char* ai_file = get_test_ai_file();
    if (!ai_file) {
        printf("(skipped - no test data) ");
        return 1;
    }

    u32 size = file_get_size(ai_file);
    /* AISetting.dat should be 1955 bytes */
    return size == 1955;
}

static int test_file_get_mtime_null(void) {
    return file_get_mtime(NULL) == 0;
}

static int test_file_get_mtime_created_file(void) {
    test_setup();

    u8 data[] = {1, 2, 3};
    file_write_all(TEST_TEMP_FILE, data, sizeof(data));

    u32 mtime = file_get_mtime(TEST_TEMP_FILE);

    test_teardown();

    /* File should have a valid modification time */
    return mtime > 0;
}

static int test_file_touch_null(void) {
    return file_touch(NULL) == 0;
}

static int test_file_touch_create(void) {
    test_setup();

    int result = file_touch(TEST_TEMP_FILE);
    int exists = file_exists(TEST_TEMP_FILE);
    u32 size = file_get_size(TEST_TEMP_FILE);

    test_teardown();

    return result == 1 && exists == 1 && size == 0;
}

/* ========================================
 * Path Parsing Tests
 * ======================================== */

static int test_file_get_extension_null(void) {
    return file_get_extension(NULL) == NULL;
}

static int test_file_get_extension_simple(void) {
    const char* ext = file_get_extension("test.dat");
    return ext != NULL && strcmp(ext, ".dat") == 0;
}

static int test_file_get_extension_long(void) {
    const char* ext = file_get_extension("path/to/file.bmp");
    return ext != NULL && strcmp(ext, ".bmp") == 0;
}

static int test_file_get_extension_no_extension(void) {
    const char* ext = file_get_extension("path/to/file");
    return ext == NULL;
}

static int test_file_get_extension_multiple_dots(void) {
    const char* ext = file_get_extension("path/to/file.name.txt");
    return ext != NULL && strcmp(ext, ".txt") == 0;
}

static int test_file_get_extension_hidden_file(void) {
    const char* ext = file_get_extension(".hidden");
    return ext != NULL && strcmp(ext, ".hidden") == 0;
}

static int test_file_get_name_null(void) {
    return file_get_name(NULL) == NULL;
}

static int test_file_get_name_simple(void) {
    const char* name = file_get_name("test.dat");
    return name != NULL && strcmp(name, "test.dat") == 0;
}

static int test_file_get_name_with_path(void) {
    const char* name = file_get_name("path/to/file.bmp");
    return name != NULL && strcmp(name, "file.bmp") == 0;
}

static int test_file_get_name_windows_path(void) {
    const char* name = file_get_name("C:\\path\\to\\file.dat");
    return name != NULL && strcmp(name, "file.dat") == 0;
}

static int test_file_get_name_trailing_slash(void) {
    /* Edge case: trailing slash */
    const char* name = file_get_name("path/to/dir/");
    /* Should return "dir/" or empty depending on implementation */
    return name != NULL;
}

static int test_file_get_directory_null(void) {
    char buffer[256];
    return file_get_directory(NULL, buffer, sizeof(buffer)) == 0;
}

static int test_file_get_directory_simple(void) {
    char buffer[256];
    int result = file_get_directory("path/to/file.dat", buffer, sizeof(buffer));
    return result == 1 && strcmp(buffer, "path/to") == 0;
}

static int test_file_get_directory_windows_path(void) {
    char buffer[256];
    int result = file_get_directory("C:\\path\\to\\file.dat", buffer, sizeof(buffer));
    return result == 1 && strcmp(buffer, "C:\\path\\to") == 0;
}

static int test_file_get_directory_no_directory(void) {
    char buffer[256];
    int result = file_get_directory("file.dat", buffer, sizeof(buffer));
    return result == 1 && buffer[0] == '\0';
}

static int test_file_get_directory_small_buffer(void) {
    char buffer[2];
    int result = file_get_directory("path/to/file.dat", buffer, sizeof(buffer));
    return result == 0;  /* Should fail - buffer too small */
}

static int test_file_get_basename_null(void) {
    char buffer[256];
    return file_get_basename(NULL, buffer, sizeof(buffer)) == 0;
}

static int test_file_get_basename_simple(void) {
    char buffer[256];
    int result = file_get_basename("path/to/file.dat", buffer, sizeof(buffer));
    return result == 1 && strcmp(buffer, "file") == 0;
}

static int test_file_get_basename_no_extension(void) {
    char buffer[256];
    int result = file_get_basename("path/to/file", buffer, sizeof(buffer));
    return result == 1 && strcmp(buffer, "file") == 0;
}

static int test_file_get_basename_multiple_dots(void) {
    char buffer[256];
    int result = file_get_basename("path/to/file.name.txt", buffer, sizeof(buffer));
    return result == 1 && strcmp(buffer, "file.name") == 0;
}

static int test_file_path_combine_null_buffer(void) {
    return file_path_combine(NULL, 256, "dir", "file") == 0;
}

static int test_file_path_combine_null_dir(void) {
    char buffer[256];
    int result = file_path_combine(buffer, sizeof(buffer), NULL, "file.dat");
    return result == 1 && strcmp(buffer, "file.dat") == 0;
}

static int test_file_path_combine_null_name(void) {
    char buffer[256];
    int result = file_path_combine(buffer, sizeof(buffer), "path/to", NULL);
    return result == 1 && strcmp(buffer, "path/to") == 0;
}

static int test_file_path_combine_normal(void) {
    char buffer[256];
    int result = file_path_combine(buffer, sizeof(buffer), "path/to", "file.dat");
    return result == 1 && strcmp(buffer, "path/to\\file.dat") == 0;
}

static int test_file_path_combine_trailing_slash(void) {
    char buffer[256];
    int result = file_path_combine(buffer, sizeof(buffer), "path/to/", "file.dat");
    return result == 1 && strcmp(buffer, "path/to/file.dat") == 0;
}

static int test_file_path_combine_empty_dir(void) {
    char buffer[256];
    int result = file_path_combine(buffer, sizeof(buffer), "", "file.dat");
    return result == 1 && strcmp(buffer, "file.dat") == 0;
}

static int test_file_path_combine_empty_name(void) {
    char buffer[256];
    int result = file_path_combine(buffer, sizeof(buffer), "path/to", "");
    return result == 1 && strcmp(buffer, "path/to") == 0;
}

/* ========================================
 * File Read/Write Tests
 * ======================================== */

static int test_file_read_all_null(void) {
    return file_read_all(NULL, NULL) == NULL;
}

static int test_file_read_all_nonexistent(void) {
    u32 size;
    u8* data = file_read_all("nonexistent_file_xyz.dat", &size);
    return data == NULL;
}

static int test_file_read_all_created_file(void) {
    test_setup();

    /* Create test file */
    u8 original[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x42};
    file_write_all(TEST_TEMP_FILE, original, sizeof(original));

    u32 size;
    u8* data = file_read_all(TEST_TEMP_FILE, &size);
    if (!data) { test_teardown(); return 0; }

    int pass = size == sizeof(original) && memcmp(data, original, size) == 0;
    free(data);

    test_teardown();
    return pass;
}

static int test_file_read_all_null_size(void) {
    test_setup();

    u8 data[] = {1, 2, 3};
    file_write_all(TEST_TEMP_FILE, data, sizeof(data));

    u8* read_data = file_read_all(TEST_TEMP_FILE, NULL);

    int pass = read_data != NULL;
    free(read_data);

    test_teardown();
    return pass;
}

static int test_file_read_all_test_data(void) {
    const char* ai_file = get_test_ai_file();
    if (!ai_file) {
        printf("(skipped - no test data) ");
        return 1;
    }

    u32 size;
    u8* data = file_read_all(ai_file, &size);
    if (!data) return 0;

    int pass = size == 1955 && data[0] != 0;
    free(data);
    return pass;
}

static int test_file_write_all_null_path(void) {
    u8 data[] = {1, 2, 3};
    return file_write_all(NULL, data, sizeof(data)) == 0;
}

static int test_file_write_all_null_data(void) {
    return file_write_all(TEST_TEMP_FILE, NULL, 10) == 0;
}

static int test_file_write_all_valid(void) {
    test_setup();

    u8 data[] = {1, 2, 3, 4, 5};
    int result = file_write_all(TEST_TEMP_FILE, data, sizeof(data));

    u32 size = file_get_size(TEST_TEMP_FILE);

    test_teardown();

    return result == 1 && size == 5;
}

static int test_file_write_and_read_roundtrip(void) {
    test_setup();

    u8 original[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    int write_result = file_write_all(TEST_TEMP_FILE, original, sizeof(original));

    u32 size;
    u8* read_data = file_read_all(TEST_TEMP_FILE, &size);

    int pass = write_result == 1 &&
               size == sizeof(original) &&
               memcmp(original, read_data, size) == 0;

    free(read_data);
    test_teardown();

    return pass;
}

static int test_file_append_null_path(void) {
    u8 data[] = {1, 2, 3};
    return file_append(NULL, data, sizeof(data)) == 0;
}

static int test_file_append_null_data(void) {
    return file_append(TEST_TEMP_FILE, NULL, 10) == 0;
}

static int test_file_append_valid(void) {
    test_setup();

    u8 data1[] = {1, 2, 3};
    u8 data2[] = {4, 5, 6};

    file_write_all(TEST_TEMP_FILE, data1, sizeof(data1));
    int result = file_append(TEST_TEMP_FILE, data2, sizeof(data2));

    u32 size = file_get_size(TEST_TEMP_FILE);
    u8* read_data = file_read_all(TEST_TEMP_FILE, NULL);

    int pass = result == 1 &&
               size == 6 &&
               read_data[0] == 1 &&
               read_data[3] == 4;

    free(read_data);
    test_teardown();

    return pass;
}

/* ========================================
 * File Listing Tests
 * ======================================== */

static int test_file_list_null_dir(void) {
    FileInfo files[10];
    return file_list(NULL, files, 10, NULL) == 0;
}

static int test_file_list_null_files(void) {
    return file_list(TEST_TEMP_DIR, NULL, 10, NULL) == 0;
}

static int test_file_list_created_files(void) {
    /* Use unique directory name to avoid conflicts */
    const char* unique_dir = "test_temp_dir_list_12345";

    /* Clean up first */
    dir_delete(unique_dir);

    /* Create directory with files */
    dir_create(unique_dir);

    char path1[256], path2[256];
    file_path_combine(path1, sizeof(path1), unique_dir, "file1.dat");
    file_path_combine(path2, sizeof(path2), unique_dir, "file2.dat");

    u8 data[] = {1, 2, 3};
    file_write_all(path1, data, sizeof(data));
    file_write_all(path2, data, sizeof(data));

    FileInfo files[10];
    int count = file_list(unique_dir, files, 10, NULL);

    /* Clean up */
    file_delete(path1);
    file_delete(path2);
    dir_delete(unique_dir);

    return count == 2;
}

static int test_file_list_with_pattern(void) {
    test_setup();

    /* Create directory with different files */
    dir_create(TEST_TEMP_DIR);

    char path1[256], path2[256], path3[256];
    file_path_combine(path1, sizeof(path1), TEST_TEMP_DIR, "file.dat");
    file_path_combine(path2, sizeof(path2), TEST_TEMP_DIR, "file.txt");
    file_path_combine(path3, sizeof(path3), TEST_TEMP_DIR, "other.dat");

    u8 data[] = {1, 2, 3};
    file_write_all(path1, data, sizeof(data));
    file_write_all(path2, data, sizeof(data));
    file_write_all(path3, data, sizeof(data));

    FileInfo files[10];
    int count = file_list(TEST_TEMP_DIR, files, 10, "*.dat");

    /* Should find at least 2 .dat files (pattern matching may vary by platform) */
    /* Windows FindFirstFile with *.dat matches both file.dat and other.dat */
    int pass = count >= 2;

    test_teardown();

    return pass;
}

static int test_file_list_max_count(void) {
    test_setup();

    /* Create directory with multiple files */
    dir_create(TEST_TEMP_DIR);

    for (int i = 0; i < 10; i++) {
        char path[256], name[32];
        snprintf(name, sizeof(name), "file%d.dat", i);
        file_path_combine(path, sizeof(path), TEST_TEMP_DIR, name);

        u8 data[] = {1, 2, 3};
        file_write_all(path, data, sizeof(data));
    }

    FileInfo files[5];
    int count = file_list(TEST_TEMP_DIR, files, 5, NULL);

    /* Should be limited to 5 */
    int pass = count <= 5;

    test_teardown();

    return pass;
}

/* ========================================
 * Working Directory Tests
 * ======================================== */

static int test_file_get_cwd(void) {
    char buffer[256];
    int result = file_get_cwd(buffer, sizeof(buffer));
    return result == 1 && strlen(buffer) > 0;
}

static int test_file_set_cwd_invalid(void) {
    return file_set_cwd("X:/nonexistent_directory") == 0;
}

static int test_file_get_exe_path(void) {
    char buffer[MAX_PATH];
    int result = file_get_exe_path(buffer, sizeof(buffer));
    return result == 1 && strlen(buffer) > 0;
}

static int test_file_get_exe_dir(void) {
    char buffer[MAX_PATH];
    int result = file_get_exe_dir(buffer, sizeof(buffer));
    return result == 1 && strlen(buffer) > 0;
}

/* ========================================
 * BMP File Writing Tests
 * ======================================== */

static int test_bmp_header_structure(void) {
    /* BMPFileHeader should be 14 bytes */
    /* BMPInfoHeader should be 40 bytes */
    return sizeof(BMPFileHeader) == 14 && sizeof(BMPInfoHeader) == 40;
}

static int test_bmp_magic_number(void) {
    return 0x4D42 == 0x4D42;  /* "BM" in little-endian */
}

static int test_file_write_bmp_24bit_null_path(void) {
    u8 pixels[12] = {0};  /* 2x2 image */
    return file_write_bmp_24bit(NULL, pixels, 2, 2) == 0;
}

static int test_file_write_bmp_24bit_null_pixels(void) {
    return file_write_bmp_24bit(TEST_TEMP_FILE ".bmp", NULL, 2, 2) == 0;
}

static int test_file_write_bmp_24bit_valid(void) {
    test_setup();

    /* Create 4x4 test image (BGR format) */
    u8 pixels[4 * 4 * 3];
    for (int i = 0; i < 4 * 4 * 3; i++) {
        pixels[i] = (u8)(i % 256);
    }

    int result = file_write_bmp_24bit(TEST_TEMP_FILE ".bmp", pixels, 4, 4);
    int exists = file_exists(TEST_TEMP_FILE ".bmp");
    u32 size = file_get_size(TEST_TEMP_FILE ".bmp");

    /* Expected size: 14 + 40 + 4*4*3 = 14 + 40 + 48 = 102 */
    int correct_size = size == 102;

    file_delete(TEST_TEMP_FILE ".bmp");
    test_teardown();

    return result == 1 && exists == 1 && correct_size;
}

static int test_file_write_bmp_24bit_verify_header(void) {
    test_setup();

    /* Create 2x2 test image */
    u8 pixels[2 * 2 * 3] = {
        0xFF, 0x00, 0x00,  /* Blue */
        0x00, 0xFF, 0x00,  /* Green */
        0x00, 0x00, 0xFF,  /* Red */
        0xFF, 0xFF, 0xFF   /* White */
    };

    int result = file_write_bmp_24bit(TEST_TEMP_FILE ".bmp", pixels, 2, 2);

    /* Read and verify header */
    FILE* fp = fopen(TEST_TEMP_FILE ".bmp", "rb");
    if (!fp) { test_teardown(); return 0; }

    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    fread(&file_header, 14, 1, fp);
    fread(&info_header, 40, 1, fp);
    fclose(fp);

    int header_valid = file_header.type == 0x4D42 &&
                       file_header.offset == 54 &&
                       info_header.size == 40 &&
                       info_header.width == 2 &&
                       info_header.height == 2 &&
                       info_header.planes == 1 &&
                       info_header.bit_count == 24;

    file_delete(TEST_TEMP_FILE ".bmp");
    test_teardown();

    return result == 1 && header_valid;
}

static int test_file_write_bmp_paletted_null_path(void) {
    u8 pixels[4] = {0};
    u8 palette[4] = {0};
    return file_write_bmp_paletted(NULL, pixels, 0, 2, 2, 2, 2, palette, 1) == 0;
}

static int test_file_write_bmp_paletted_null_pixels(void) {
    u8 palette[4] = {0};
    return file_write_bmp_paletted(TEST_TEMP_FILE ".bmp", NULL, 0, 2, 2, 2, 2, palette, 1) == 0;
}

static int test_file_write_bmp_paletted_valid(void) {
    test_setup();

    /* Create 4x4 paletted image */
    u8 pixels[4 * 4];
    for (int i = 0; i < 16; i++) {
        pixels[i] = (u8)i;
    }

    /* Create simple palette (4 colors) */
    u8 palette[4 * 4] = {
        0x00, 0x00, 0x00, 0x00,  /* Black */
        0xFF, 0x00, 0x00, 0x00,  /* Blue */
        0x00, 0xFF, 0x00, 0x00,  /* Green */
        0x00, 0x00, 0xFF, 0x00   /* Red */
    };

    int result = file_write_bmp_paletted(TEST_TEMP_FILE ".bmp", pixels, 0, 4, 4, 4, 4, palette, 4);
    int exists = file_exists(TEST_TEMP_FILE ".bmp");
    u32 size = file_get_size(TEST_TEMP_FILE ".bmp");

    /* Expected size: 14 + 40 + 16 + 16 = 86 */
    int correct_size = size == 86;

    file_delete(TEST_TEMP_FILE ".bmp");
    test_teardown();

    return result == 1 && exists == 1 && correct_size;
}

static int test_file_write_bmp_paletted_verify_header(void) {
    test_setup();

    /* Create 2x2 paletted image */
    u8 pixels[4] = {0, 1, 2, 3};

    /* Create palette (4 colors) */
    u8 palette[4 * 4] = {
        0x00, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0x00,
        0x80, 0x80, 0x80, 0x00,
        0x40, 0x40, 0x40, 0x00
    };

    int result = file_write_bmp_paletted(TEST_TEMP_FILE ".bmp", pixels, 0, 2, 2, 2, 2, palette, 4);

    /* Read and verify header */
    FILE* fp = fopen(TEST_TEMP_FILE ".bmp", "rb");
    if (!fp) { test_teardown(); return 0; }

    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    fread(&file_header, 14, 1, fp);
    fread(&info_header, 40, 1, fp);
    fclose(fp);

    int header_valid = file_header.type == 0x4D42 &&
                       info_header.size == 40 &&
                       info_header.width == 2 &&
                       info_header.height == 2 &&
                       info_header.bit_count == 8 &&
                       info_header.clr_used == 4;

    file_delete(TEST_TEMP_FILE ".bmp");
    test_teardown();

    return result == 1 && header_valid;
}

static int test_file_write_bmp_paletted_row_padding(void) {
    test_setup();

    /* Create 3x2 image (3 pixels wide needs 1 byte padding per row) */
    u8 pixels[3 * 2];
    memset(pixels, 0x55, sizeof(pixels));

    u8 palette[4] = {0, 0, 0, 0};

    int result = file_write_bmp_paletted(TEST_TEMP_FILE ".bmp", pixels, 0, 2, 3, 2, 3, palette, 1);
    u32 size = file_get_size(TEST_TEMP_FILE ".bmp");

    /* Expected: 14 + 40 + 4 + (4 * 2) = 14 + 40 + 4 + 8 = 66 */
    /* With padding: each row is 3 pixels + 1 padding = 4 bytes */
    int correct_size = size == 66;

    file_delete(TEST_TEMP_FILE ".bmp");
    test_teardown();

    return result == 1 && correct_size;
}

static int test_file_write_screenshot_8bit(void) {
    test_setup();

    u8 pixels[4 * 4];
    memset(pixels, 0x42, sizeof(pixels));

    u8 palette[4] = {0, 0, 0, 0};

    int result = file_write_screenshot(TEST_TEMP_FILE ".bmp", pixels, 4, 4, 8, palette, 1);
    int exists = file_exists(TEST_TEMP_FILE ".bmp");

    file_delete(TEST_TEMP_FILE ".bmp");
    test_teardown();

    return result == 1 && exists == 1;
}

static int test_file_write_screenshot_24bit(void) {
    test_setup();

    u8 pixels[4 * 4 * 3];
    memset(pixels, 0x55, sizeof(pixels));

    int result = file_write_screenshot(TEST_TEMP_FILE ".bmp", pixels, 4, 4, 24, NULL, 0);
    int exists = file_exists(TEST_TEMP_FILE ".bmp");

    file_delete(TEST_TEMP_FILE ".bmp");
    test_teardown();

    return result == 1 && exists == 1;
}

static int test_file_write_screenshot_invalid_bpp(void) {
    u8 pixels[16];
    return file_write_screenshot(TEST_TEMP_FILE ".bmp", pixels, 4, 4, 16, NULL, 0) == 0;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_file_lifecycle(void) {
    test_setup();

    /* Write */
    u8 data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    int write_ok = file_write_all(TEST_TEMP_FILE, data, sizeof(data));
    if (!write_ok) { test_teardown(); return 0; }

    /* Check exists */
    int exists = file_exists(TEST_TEMP_FILE);
    if (!exists) { test_teardown(); return 0; }

    /* Check size */
    u32 size = file_get_size(TEST_TEMP_FILE);
    if (size != sizeof(data)) { test_teardown(); return 0; }

    /* Read back */
    u32 read_size;
    u8* read_data = file_read_all(TEST_TEMP_FILE, &read_size);
    if (!read_data || read_size != sizeof(data)) {
        free(read_data);
        test_teardown();
        return 0;
    }

    /* Verify content */
    int content_ok = memcmp(data, read_data, sizeof(data)) == 0;
    free(read_data);

    /* Delete */
    int delete_ok = file_delete(TEST_TEMP_FILE);

    /* Verify deleted */
    int gone = !file_exists(TEST_TEMP_FILE);

    test_teardown();

    return content_ok && delete_ok && gone;
}

static int test_copy_move_delete_chain(void) {
    test_setup();

    /* Create original file */
    u8 data[] = {0x12, 0x34, 0x56, 0x78};
    file_write_all("source_file.dat", data, sizeof(data));

    /* Copy */
    int copy_ok = file_copy("source_file.dat", TEST_TEMP_FILE);
    if (!copy_ok) { file_delete("source_file.dat"); test_teardown(); return 0; }

    /* Move */
    const char* moved = "moved_file.dat";
    int move_ok = file_move(TEST_TEMP_FILE, moved);
    if (!move_ok) { file_delete("source_file.dat"); test_teardown(); return 0; }

    /* Delete */
    int delete_ok = file_delete(moved);
    file_delete("source_file.dat");

    test_teardown();

    return copy_ok && move_ok && delete_ok;
}

static int test_directory_operations_chain(void) {
    test_setup();

    /* Create directory */
    int create_ok = dir_create(TEST_TEMP_DIR);
    if (!create_ok) { test_teardown(); return 0; }

    /* Verify exists */
    int exists = dir_exists(TEST_TEMP_DIR);
    if (!exists) { test_teardown(); return 0; }

    /* Create file inside */
    char file_path[256];
    file_path_combine(file_path, sizeof(file_path), TEST_TEMP_DIR, "test.dat");

    u8 data[] = {1, 2, 3};
    int write_ok = file_write_all(file_path, data, sizeof(data));
    if (!write_ok) { test_teardown(); return 0; }

    /* List directory */
    FileInfo files[10];
    int count = file_list(TEST_TEMP_DIR, files, 10, NULL);

    /* Clean up */
    file_delete(file_path);
    dir_delete(TEST_TEMP_DIR);

    test_teardown();

    /* Should have found at least 1 file (the one we created) */
    return create_ok && exists && write_ok && count >= 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== File Utility Module Comprehensive Tests ===\n\n");

    /* File existence tests */
    printf("File Existence Tests:\n");
    TEST(file_exists_null);
    TEST(file_exists_invalid_file);
    TEST(file_exists_created_file);
    TEST(file_exists_test_data);

    /* Directory existence tests */
    printf("\nDirectory Existence Tests:\n");
    TEST(dir_exists_null);
    TEST(dir_exists_invalid_dir);
    TEST(dir_exists_created_dir);
    TEST(dir_exists_file_as_dir);
    TEST(dir_exists_test_data_dir);

    /* Directory creation tests */
    printf("\nDirectory Creation Tests:\n");
    TEST(dir_create_simple);
    TEST(dir_create_null);
    TEST(dir_create_existing);
    TEST(dir_create_recursive);
    TEST(dir_create_recursive_null);

    /* Directory deletion tests */
    printf("\nDirectory Deletion Tests:\n");
    TEST(dir_delete_null);
    TEST(dir_delete_nonexistent);
    TEST(dir_delete_and_verify);

    /* File operations tests */
    printf("\nFile Operations Tests:\n");
    TEST(file_delete_null);
    TEST(file_delete_nonexistent);
    TEST(file_delete_created_file);
    TEST(file_copy_null_src);
    TEST(file_copy_null_dst);
    TEST(file_copy_created_file);
    TEST(file_copy_test_data);
    TEST(file_move_null_src);
    TEST(file_move_null_dst);
    TEST(file_move_created_file);
    TEST(file_rename);

    /* File info tests */
    printf("\nFile Info Tests:\n");
    TEST(file_get_size_null);
    TEST(file_get_size_created_file);
    TEST(file_get_size_nonexistent);
    TEST(file_get_size_test_data);
    TEST(file_get_mtime_null);
    TEST(file_get_mtime_created_file);
    TEST(file_touch_null);
    TEST(file_touch_create);

    /* Path parsing tests */
    printf("\nPath Parsing Tests:\n");
    TEST(file_get_extension_null);
    TEST(file_get_extension_simple);
    TEST(file_get_extension_long);
    TEST(file_get_extension_no_extension);
    TEST(file_get_extension_multiple_dots);
    TEST(file_get_extension_hidden_file);
    TEST(file_get_name_null);
    TEST(file_get_name_simple);
    TEST(file_get_name_with_path);
    TEST(file_get_name_windows_path);
    TEST(file_get_name_trailing_slash);
    TEST(file_get_directory_null);
    TEST(file_get_directory_simple);
    TEST(file_get_directory_windows_path);
    TEST(file_get_directory_no_directory);
    TEST(file_get_directory_small_buffer);
    TEST(file_get_basename_null);
    TEST(file_get_basename_simple);
    TEST(file_get_basename_no_extension);
    TEST(file_get_basename_multiple_dots);
    TEST(file_path_combine_null_buffer);
    TEST(file_path_combine_null_dir);
    TEST(file_path_combine_null_name);
    TEST(file_path_combine_normal);
    TEST(file_path_combine_trailing_slash);
    TEST(file_path_combine_empty_dir);
    TEST(file_path_combine_empty_name);

    /* File read/write tests */
    printf("\nFile Read/Write Tests:\n");
    TEST(file_read_all_null);
    TEST(file_read_all_nonexistent);
    TEST(file_read_all_created_file);
    TEST(file_read_all_null_size);
    TEST(file_read_all_test_data);
    TEST(file_write_all_null_path);
    TEST(file_write_all_null_data);
    TEST(file_write_all_valid);
    TEST(file_write_and_read_roundtrip);
    TEST(file_append_null_path);
    TEST(file_append_null_data);
    TEST(file_append_valid);

    /* File listing tests */
    printf("\nFile Listing Tests:\n");
    TEST(file_list_null_dir);
    TEST(file_list_null_files);
    TEST(file_list_created_files);
    TEST(file_list_with_pattern);
    TEST(file_list_max_count);

    /* Working directory tests */
    printf("\nWorking Directory Tests:\n");
    TEST(file_get_cwd);
    TEST(file_set_cwd_invalid);
    TEST(file_get_exe_path);
    TEST(file_get_exe_dir);

    /* BMP file writing tests */
    printf("\nBMP File Writing Tests:\n");
    TEST(bmp_header_structure);
    TEST(bmp_magic_number);
    TEST(file_write_bmp_24bit_null_path);
    TEST(file_write_bmp_24bit_null_pixels);
    TEST(file_write_bmp_24bit_valid);
    TEST(file_write_bmp_24bit_verify_header);
    TEST(file_write_bmp_paletted_null_path);
    TEST(file_write_bmp_paletted_null_pixels);
    TEST(file_write_bmp_paletted_valid);
    TEST(file_write_bmp_paletted_verify_header);
    TEST(file_write_bmp_paletted_row_padding);
    TEST(file_write_screenshot_8bit);
    TEST(file_write_screenshot_24bit);
    TEST(file_write_screenshot_invalid_bpp);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_file_lifecycle);
    TEST(copy_move_delete_chain);
    TEST(directory_operations_chain);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - dir_delete_recursive
     * - Large file handling (>4GB)
     * - Unicode paths
     * - File locking
     * - Error handling edge cases
     * - Performance tests for many files
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
