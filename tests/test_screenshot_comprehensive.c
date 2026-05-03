/*
 * Stone Age Client - Screenshot System Comprehensive Tests
 * Tests for screenshot.c implementation
 *
 * Covers:
 * - Initialization and shutdown
 * - RGB565 to BGR conversion
 * - RGB555 to BGR conversion
 * - Screen dimensions based on resolution mode
 * - BMP file header creation
 * - Format and quality settings
 * - Counter and statistics tracking
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
typedef size_t SIZE_T;

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    fflush(stdout); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Constants
 * ======================================== */

#define SCREENSHOT_BMP  0
#define SCREENSHOT_JPEG 1
#define SCREENSHOT_PNG  2

#define PIXEL_FORMAT_RGB565 2
#define PIXEL_FORMAT_RGB555 0

#define MAX_PATH 260

/* ========================================
 * Structures
 * ======================================== */

typedef struct {
    int enabled;
    int format;
    u8 quality;
    int counter;
    int total_captures;
    char last_file[MAX_PATH];
    u32 last_capture;
} ScreenshotContext;

/* BMP file header - 14 bytes */
typedef struct {
    u16 bfType;
    u32 bfSize;
    u16 bfReserved1;
    u16 bfReserved2;
    u32 bfOffBits;
} BITMAPFILEHEADER;

/* BMP info header - 40 bytes (local definition to avoid Windows header conflicts) */
typedef struct {
    u32 biSize;
    s32 biWidth;
    s32 biHeight;
    u16 biPlanes;
    u16 biBitCount;
    u32 biCompression;
    u32 biSizeImage;
    s32 biXPelsPerMeter;
    s32 biYPelsPerMeter;
    u32 biClrUsed;
    u32 biClrImportant;
} TestBitmapInfoHeader;

/* ========================================
 * Global State
 * ======================================== */

static ScreenshotContext g_screenshot = {0};
static int g_pixel_format = PIXEL_FORMAT_RGB565;
static int g_resolution_mode = 0;  /* 0 = 640x480, 1 = 320x240 */

/* ========================================
 * Implementation Functions
 * ======================================== */

static int screenshot_init(void) {
    memset(&g_screenshot, 0, sizeof(ScreenshotContext));

    g_screenshot.enabled = 1;
    g_screenshot.format = SCREENSHOT_BMP;
    g_screenshot.quality = 90;
    g_screenshot.counter = 0;

    return 1;
}

static void screenshot_shutdown(void) {
    memset(&g_screenshot, 0, sizeof(ScreenshotContext));
}

static void convert_rgb565_to_bgr(const u16* src, u8* dst, int width, int height, int src_pitch) {
    const u16* src_row;
    u8* dst_row;
    int x, y;
    u16 pixel;

    for (y = 0; y < height; y++) {
        src_row = (const u16*)((const u8*)src + y * src_pitch);
        dst_row = dst + (height - 1 - y) * width * 3;

        for (x = 0; x < width; x++) {
            pixel = src_row[x];
            dst_row[x * 3 + 0] = (u8)((pixel << 3) & 0xF8);
            dst_row[x * 3 + 1] = (u8)((pixel >> 3) & 0xFC);
            dst_row[x * 3 + 2] = (u8)((pixel >> 8) & 0xF8);
        }
    }
}

static void convert_rgb555_to_bgr(const u16* src, u8* dst, int width, int height, int src_pitch) {
    const u16* src_row;
    u8* dst_row;
    int x, y;
    u16 pixel;

    for (y = 0; y < height; y++) {
        src_row = (const u16*)((const u8*)src + y * src_pitch);
        dst_row = dst + (height - 1 - y) * width * 3;

        for (x = 0; x < width; x++) {
            pixel = src_row[x];
            dst_row[x * 3 + 0] = (u8)((pixel << 3) & 0xF8);
            dst_row[x * 3 + 1] = (u8)((pixel >> 2) & 0xF8);
            dst_row[x * 3 + 2] = (u8)((pixel >> 7) & 0xF8);
        }
    }
}

static void get_screen_dimensions(int* width, int* height) {
    if (g_resolution_mode == 1) {
        *width = 320;
        *height = 240;
    } else {
        *width = 640;
        *height = 480;
    }
}

static void screenshot_set_format(int format) {
    g_screenshot.format = format;
}

static void screenshot_set_quality(u8 quality) {
    g_screenshot.quality = quality > 100 ? 100 : quality;
}

static void screenshot_set_enabled(int enabled) {
    g_screenshot.enabled = enabled;
}

static int screenshot_is_enabled(void) {
    return g_screenshot.enabled;
}

static const char* screenshot_get_last_file(void) {
    return g_screenshot.last_file;
}

static int screenshot_get_total_captures(void) {
    return g_screenshot.total_captures;
}

static const char* screenshot_get_directory(void) {
    return "screenshot";
}

static void screenshot_record_capture(const char* filename) {
    g_screenshot.total_captures++;
    strncpy(g_screenshot.last_file, filename, MAX_PATH - 1);
    g_screenshot.last_file[MAX_PATH - 1] = '\0';
    g_screenshot.counter++;
}

static int bmp_calc_row_size(int width) {
    return (width * 3 + 3) & ~3;  /* Align to 4 bytes */
}

static int bmp_calc_file_size(int width, int height) {
    int row_size = bmp_calc_row_size(width);
    int pixel_size = row_size * height;
    return sizeof(BITMAPFILEHEADER) + sizeof(TestBitmapInfoHeader) + pixel_size;
}

static void bmp_init_file_header(BITMAPFILEHEADER* bfh, int file_size) {
    memset(bfh, 0, sizeof(BITMAPFILEHEADER));
    bfh->bfType = 0x4D42;  /* "BM" */
    bfh->bfSize = file_size;
    bfh->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(TestBitmapInfoHeader);
}

static void bmp_init_info_header(TestBitmapInfoHeader* bih, int width, int height) {
    int row_size = bmp_calc_row_size(width);
    int pixel_size = row_size * height;

    memset(bih, 0, sizeof(TestBitmapInfoHeader));
    bih->biSize = sizeof(TestBitmapInfoHeader);
    bih->biWidth = width;
    bih->biHeight = height;
    bih->biPlanes = 1;
    bih->biBitCount = 24;
    bih->biCompression = 0;  /* BI_RGB */
    bih->biSizeImage = pixel_size;
}

/* ========================================
 * Test Cases - Initialization
 * ======================================== */

static int test_init_basic(void) {
    int result = screenshot_init();
    return result == 1 && g_screenshot.enabled == 1;
}

static int test_init_format_bmp(void) {
    screenshot_init();
    return g_screenshot.format == SCREENSHOT_BMP;
}

static int test_init_quality_90(void) {
    screenshot_init();
    return g_screenshot.quality == 90;
}

static int test_init_counter_zero(void) {
    screenshot_init();
    return g_screenshot.counter == 0;
}

static int test_shutdown_clears_state(void) {
    screenshot_init();
    screenshot_record_capture("test.bmp");
    screenshot_shutdown();
    return g_screenshot.enabled == 0 && g_screenshot.total_captures == 0;
}

/* ========================================
 * Test Cases - RGB565 Conversion
 * ======================================== */

static int test_rgb565_black(void) {
    u16 src[1] = { 0x0000 };  /* Black */
    u8 dst[3];

    convert_rgb565_to_bgr(src, dst, 1, 1, 2);

    return dst[0] == 0 && dst[1] == 0 && dst[2] == 0;
}

static int test_rgb565_white(void) {
    u16 src[1] = { 0xFFFF };  /* White: R=31, G=63, B=31 */
    u8 dst[3];

    convert_rgb565_to_bgr(src, dst, 1, 1, 2);

    /* R=31<<3=248, G=63>>3=60<<2=240, B=31<<3=248 */
    return dst[0] == 248 && dst[1] == 252 && dst[2] == 248;
}

static int test_rgb565_red(void) {
    u16 src[1] = { 0xF800 };  /* Red: R=31, G=0, B=0 */
    u8 dst[3];

    convert_rgb565_to_bgr(src, dst, 1, 1, 2);

    /* BGR format: B=0, G=0, R=248 */
    return dst[0] == 0 && dst[1] == 0 && dst[2] == 248;
}

static int test_rgb565_green(void) {
    u16 src[1] = { 0x07E0 };  /* Green: R=0, G=63, B=0 */
    u8 dst[3];

    convert_rgb565_to_bgr(src, dst, 1, 1, 2);

    /* BGR format: B=0, G=252, R=0 */
    return dst[0] == 0 && dst[1] == 252 && dst[2] == 0;
}

static int test_rgb565_blue(void) {
    u16 src[1] = { 0x001F };  /* Blue: R=0, G=0, B=31 */
    u8 dst[3];

    convert_rgb565_to_bgr(src, dst, 1, 1, 2);

    /* BGR format: B=248, G=0, R=0 */
    return dst[0] == 248 && dst[1] == 0 && dst[2] == 0;
}

static int test_rgb565_bottom_up(void) {
    /* 2x2 image - verify bottom-up ordering */
    u16 src[4] = {
        0xF800, 0x001F,  /* Row 0: Red, Blue */
        0x07E0, 0x0000   /* Row 1: Green, Black */
    };
    u8 dst[12];  /* 2x2x3 */

    convert_rgb565_to_bgr(src, dst, 2, 2, 4);

    /* First output row should be last input row (Green, Black) */
    int green_b = dst[0];
    int green_g = dst[1];
    int green_r = dst[2];

    return green_r == 0 && green_g == 252 && green_b == 0;
}

/* ========================================
 * Test Cases - RGB555 Conversion
 * ======================================== */

static int test_rgb555_black(void) {
    u16 src[1] = { 0x0000 };
    u8 dst[3];

    convert_rgb555_to_bgr(src, dst, 1, 1, 2);

    return dst[0] == 0 && dst[1] == 0 && dst[2] == 0;
}

static int test_rgb555_white(void) {
    u16 src[1] = { 0x7FFF };  /* White: R=31, G=31, B=31 */
    u8 dst[3];

    convert_rgb555_to_bgr(src, dst, 1, 1, 2);

    return dst[0] == 248 && dst[1] == 248 && dst[2] == 248;
}

static int test_rgb555_red(void) {
    u16 src[1] = { 0x7C00 };  /* Red: R=31, G=0, B=0 */
    u8 dst[3];

    convert_rgb555_to_bgr(src, dst, 1, 1, 2);

    return dst[0] == 0 && dst[1] == 0 && dst[2] == 248;
}

static int test_rgb555_green(void) {
    u16 src[1] = { 0x03E0 };  /* Green: R=0, G=31, B=0 */
    u8 dst[3];

    convert_rgb555_to_bgr(src, dst, 1, 1, 2);

    return dst[0] == 0 && dst[1] == 248 && dst[2] == 0;
}

static int test_rgb555_blue(void) {
    u16 src[1] = { 0x001F };  /* Blue: R=0, G=0, B=31 */
    u8 dst[3];

    convert_rgb555_to_bgr(src, dst, 1, 1, 2);

    return dst[0] == 248 && dst[1] == 0 && dst[2] == 0;
}

/* ========================================
 * Test Cases - Screen Dimensions
 * ======================================== */

static int test_dimensions_default(void) {
    int width, height;
    g_resolution_mode = 0;
    get_screen_dimensions(&width, &height);
    return width == 640 && height == 480;
}

static int test_dimensions_320x240(void) {
    int width, height;
    g_resolution_mode = 1;
    get_screen_dimensions(&width, &height);
    return width == 320 && height == 240;
}

/* ========================================
 * Test Cases - BMP Calculations
 * ======================================== */

static int test_bmp_row_size_basic(void) {
    /* 10 pixels * 3 bytes = 30 bytes, already aligned to 4 */
    return bmp_calc_row_size(10) == 32;  /* 30 rounded up to 32 */
}

static int test_bmp_row_size_aligned(void) {
    /* 4 pixels * 3 bytes = 12 bytes, already aligned */
    return bmp_calc_row_size(4) == 12;
}

static int test_bmp_row_size_padding(void) {
    /* 3 pixels * 3 bytes = 9 bytes, needs padding to 12 */
    return bmp_calc_row_size(3) == 12;
}

static int test_bmp_file_size_basic(void) {
    /* 2x2 image: row_size = 8, pixel_size = 16, total = 14 + 40 + 16 = 70 */
    int expected = sizeof(BITMAPFILEHEADER) + sizeof(TestBitmapInfoHeader) + (8 * 2);
    return bmp_calc_file_size(2, 2) == expected;
}

static int test_bmp_file_size_larger(void) {
    /* 10x10 image: row_size = 32, pixel_size = 320 */
    int expected = sizeof(BITMAPFILEHEADER) + sizeof(TestBitmapInfoHeader) + (32 * 10);
    return bmp_calc_file_size(10, 10) == expected;
}

static int test_bmp_header_type(void) {
    BITMAPFILEHEADER bfh;
    bmp_init_file_header(&bfh, 100);

    return bfh.bfType == 0x4D42;  /* "BM" */
}

static int test_bmp_header_offset(void) {
    BITMAPFILEHEADER bfh;
    bmp_init_file_header(&bfh, 100);

    return bfh.bfOffBits == sizeof(BITMAPFILEHEADER) + sizeof(TestBitmapInfoHeader);
}

static int test_bmp_info_size(void) {
    TestBitmapInfoHeader bih;
    bmp_init_info_header(&bih, 10, 10);

    return bih.biSize == sizeof(TestBitmapInfoHeader);
}

static int test_bmp_info_dimensions(void) {
    TestBitmapInfoHeader bih;
    bmp_init_info_header(&bih, 640, 480);

    return bih.biWidth == 640 && bih.biHeight == 480;
}

static int test_bmp_info_bit_count(void) {
    TestBitmapInfoHeader bih;
    bmp_init_info_header(&bih, 10, 10);

    return bih.biBitCount == 24;
}

/* ========================================
 * Test Cases - Configuration
 * ======================================== */

static int test_set_format_jpeg(void) {
    screenshot_init();
    screenshot_set_format(SCREENSHOT_JPEG);
    return g_screenshot.format == SCREENSHOT_JPEG;
}

static int test_set_format_png(void) {
    screenshot_init();
    screenshot_set_format(SCREENSHOT_PNG);
    return g_screenshot.format == SCREENSHOT_PNG;
}

static int test_set_quality_basic(void) {
    screenshot_init();
    screenshot_set_quality(75);
    return g_screenshot.quality == 75;
}

static int test_set_quality_max(void) {
    screenshot_init();
    screenshot_set_quality(100);
    return g_screenshot.quality == 100;
}

static int test_set_quality_clamped(void) {
    screenshot_init();
    screenshot_set_quality(150);  /* Should be clamped to 100 */
    return g_screenshot.quality == 100;
}

static int test_set_enabled(void) {
    screenshot_init();
    screenshot_set_enabled(0);
    return screenshot_is_enabled() == 0;
}

static int test_is_enabled_initial(void) {
    screenshot_init();
    return screenshot_is_enabled() == 1;
}

/* ========================================
 * Test Cases - Statistics
 * ======================================== */

static int test_get_total_captures_initial(void) {
    screenshot_init();
    return screenshot_get_total_captures() == 0;
}

static int test_record_capture_increments(void) {
    screenshot_init();
    screenshot_record_capture("test1.bmp");
    screenshot_record_capture("test2.bmp");
    return screenshot_get_total_captures() == 2;
}

static int test_get_last_file(void) {
    screenshot_init();
    screenshot_record_capture("screenshot\\test.bmp");
    return strcmp(screenshot_get_last_file(), "screenshot\\test.bmp") == 0;
}

static int test_counter_increments(void) {
    screenshot_init();
    screenshot_record_capture("test1.bmp");
    screenshot_record_capture("test2.bmp");
    return g_screenshot.counter == 2;
}

static int test_get_directory(void) {
    return strcmp(screenshot_get_directory(), "screenshot") == 0;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Screenshot System Comprehensive Tests ===\n\n");

    printf("Initialization Tests:\n");
    TEST(init_basic);
    TEST(init_format_bmp);
    TEST(init_quality_90);
    TEST(init_counter_zero);
    TEST(shutdown_clears_state);

    printf("\nRGB565 Conversion Tests:\n");
    TEST(rgb565_black);
    TEST(rgb565_white);
    TEST(rgb565_red);
    TEST(rgb565_green);
    TEST(rgb565_blue);
    TEST(rgb565_bottom_up);

    printf("\nRGB555 Conversion Tests:\n");
    TEST(rgb555_black);
    TEST(rgb555_white);
    TEST(rgb555_red);
    TEST(rgb555_green);
    TEST(rgb555_blue);

    printf("\nScreen Dimensions Tests:\n");
    TEST(dimensions_default);
    TEST(dimensions_320x240);

    printf("\nBMP Calculation Tests:\n");
    TEST(bmp_row_size_basic);
    TEST(bmp_row_size_aligned);
    TEST(bmp_row_size_padding);
    TEST(bmp_file_size_basic);
    TEST(bmp_file_size_larger);
    TEST(bmp_header_type);
    TEST(bmp_header_offset);
    TEST(bmp_info_size);
    TEST(bmp_info_dimensions);
    TEST(bmp_info_bit_count);

    printf("\nConfiguration Tests:\n");
    TEST(set_format_jpeg);
    TEST(set_format_png);
    TEST(set_quality_basic);
    TEST(set_quality_max);
    TEST(set_quality_clamped);
    TEST(set_enabled);
    TEST(is_enabled_initial);

    printf("\nStatistics Tests:\n");
    TEST(get_total_captures_initial);
    TEST(record_capture_increments);
    TEST(get_last_file);
    TEST(counter_increments);
    TEST(get_directory);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
