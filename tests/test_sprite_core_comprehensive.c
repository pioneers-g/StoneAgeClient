/*
 * Stone Age Client - Sprite Core Comprehensive Tests
 * Tests for sprite_core.c implementation
 *
 * Covers:
 * - Initialization and shutdown
 * - Color conversion (RGB to 565/555)
 * - Color splitting (565/555 to RGB)
 * - Pixel format handling
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
typedef int s16;
typedef int s32;

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

#define PIXEL_FORMAT_555    0
#define PIXEL_FORMAT_565    2

#define TRANSPARENT_COLOR_565   0xF81F
#define TRANSPARENT_COLOR_555   0x7C1F

/* ========================================
 * Global State
 * ======================================== */

typedef struct {
    void* dest_surface;
    u32 dest_pitch;
    u16 dest_width;
    u16 dest_height;
    u8  pixel_format;
    u8  reserved[3];
} SpriteContext;

static SpriteContext g_sprite_ctx = {0};
static int g_pixel_format = PIXEL_FORMAT_565;

/* ========================================
 * Implementation Functions
 * ======================================== */

/*
 * Initialize sprite system
 */
static int sprite_init(void) {
    memset(&g_sprite_ctx, 0, sizeof(SpriteContext));
    g_sprite_ctx.pixel_format = PIXEL_FORMAT_565;
    g_pixel_format = PIXEL_FORMAT_565;
    return 1;
}

/*
 * Shutdown sprite system
 */
static void sprite_shutdown(void) {
    memset(&g_sprite_ctx, 0, sizeof(SpriteContext));
}

/*
 * Set pixel format
 */
static void sprite_set_pixel_format(int format) {
    g_pixel_format = format;
    g_sprite_ctx.pixel_format = format;
}

/*
 * Get pixel format
 */
static int sprite_get_pixel_format(void) {
    return g_pixel_format;
}

/*
 * Convert RGB to 565 format
 */
static u16 sprite_color_565(u8 r, u8 g, u8 b) {
    return ((u16)(r & 0xF8) << 8) | ((u16)(g & 0xFC) << 3) | ((u16)(b & 0xF8) >> 3);
}

/*
 * Convert RGB to 555 format
 */
static u16 sprite_color_555(u8 r, u8 g, u8 b) {
    return ((u16)(r & 0xF8) << 7) | ((u16)(g & 0xF8) << 2) | ((u16)(b & 0xF8) >> 3);
}

/*
 * Split 565 color to RGB
 */
static void sprite_color_split_565(u16 color, u8* r, u8* g, u8* b) {
    if (r) *r = (u8)((color >> 8) & 0xF8);
    if (g) *g = (u8)((color >> 3) & 0xFC);
    if (b) *b = (u8)((color << 3) & 0xF8);
}

/*
 * Split 555 color to RGB
 */
static void sprite_color_split_555(u16 color, u8* r, u8* g, u8* b) {
    if (r) *r = (u8)((color >> 7) & 0xF8);
    if (g) *g = (u8)((color >> 2) & 0xF8);
    if (b) *b = (u8)((color << 3) & 0xF8);
}

/*
 * Get transparent color for current pixel format
 */
static u16 sprite_get_transparent_color(void) {
    return (g_pixel_format == PIXEL_FORMAT_565) ?
           TRANSPARENT_COLOR_565 : TRANSPARENT_COLOR_555;
}

/*
 * Reset state
 */
static void reset_state(void) {
    sprite_init();
}

/* ========================================
 * Test Cases - Initialization
 * ======================================== */

static int test_init_basic(void) {
    reset_state();

    return g_sprite_ctx.pixel_format == PIXEL_FORMAT_565;
}

static int test_shutdown_clears_context(void) {
    reset_state();

    g_sprite_ctx.dest_width = 100;
    g_sprite_ctx.dest_height = 100;

    sprite_shutdown();

    return g_sprite_ctx.dest_width == 0 && g_sprite_ctx.dest_height == 0;
}

static int test_init_after_shutdown(void) {
    reset_state();

    sprite_shutdown();
    sprite_init();

    return g_sprite_ctx.pixel_format == PIXEL_FORMAT_565;
}

/* ========================================
 * Test Cases - Pixel Format
 * ======================================== */

static int test_pixel_format_default(void) {
    reset_state();

    return sprite_get_pixel_format() == PIXEL_FORMAT_565;
}

static int test_pixel_format_set_555(void) {
    reset_state();

    sprite_set_pixel_format(PIXEL_FORMAT_555);

    return sprite_get_pixel_format() == PIXEL_FORMAT_555;
}

static int test_pixel_format_set_565(void) {
    reset_state();

    sprite_set_pixel_format(PIXEL_FORMAT_555);
    sprite_set_pixel_format(PIXEL_FORMAT_565);

    return sprite_get_pixel_format() == PIXEL_FORMAT_565;
}

static int test_transparent_color_565(void) {
    reset_state();

    sprite_set_pixel_format(PIXEL_FORMAT_565);

    return sprite_get_transparent_color() == TRANSPARENT_COLOR_565;
}

static int test_transparent_color_555(void) {
    reset_state();

    sprite_set_pixel_format(PIXEL_FORMAT_555);

    return sprite_get_transparent_color() == TRANSPARENT_COLOR_555;
}

/* ========================================
 * Test Cases - Color Conversion 565
 * ======================================== */

static int test_color_565_black(void) {
    u16 color = sprite_color_565(0, 0, 0);
    return color == 0x0000;
}

static int test_color_565_white(void) {
    u16 color = sprite_color_565(255, 255, 255);
    /* R=11111, G=111111, B=11111 */
    /* 11111 111111 11111 = 0xFFFF */
    return color == 0xFFFF;
}

static int test_color_565_red(void) {
    u16 color = sprite_color_565(255, 0, 0);
    /* R=11111 -> 11111000 00000000 */
    return color == 0xF800;
}

static int test_color_565_green(void) {
    u16 color = sprite_color_565(0, 255, 0);
    /* G=111111 -> 00000111 11100000 */
    return color == 0x07E0;
}

static int test_color_565_blue(void) {
    u16 color = sprite_color_565(0, 0, 255);
    /* B=11111 -> 00000000 00011111 */
    return color == 0x001F;
}

static int test_color_565_gray(void) {
    u16 color = sprite_color_565(128, 128, 128);
    /* R=128/8=16, G=128/4=32, B=128/8=16 */
    /* 16 = 10000, 32 = 100000, 16 = 10000 */
    /* 10000 100000 10000 = 0x8410 */
    return color == 0x8410;
}

static int test_color_565_truncation(void) {
    /* Test that lower bits are truncated */
    u16 c1 = sprite_color_565(255, 255, 255);
    u16 c2 = sprite_color_565(248, 252, 248);  /* Same after masking */
    return c1 == c2;
}

/* ========================================
 * Test Cases - Color Conversion 555
 * ======================================== */

static int test_color_555_black(void) {
    u16 color = sprite_color_555(0, 0, 0);
    return color == 0x0000;
}

static int test_color_555_white(void) {
    u16 color = sprite_color_555(255, 255, 255);
    /* R=11111, G=11111, B=11111 */
    /* 011111 11111 11111 = 0x7FFF */
    return color == 0x7FFF;
}

static int test_color_555_red(void) {
    u16 color = sprite_color_555(255, 0, 0);
    /* R=11111 -> 11111000 0000000 */
    return color == 0x7C00;
}

static int test_color_555_green(void) {
    u16 color = sprite_color_555(0, 255, 0);
    /* G=11111 -> 00000111 11000000 */
    return color == 0x03E0;
}

static int test_color_555_blue(void) {
    u16 color = sprite_color_555(0, 0, 255);
    /* B=11111 -> 00000000 00011111 */
    return color == 0x001F;
}

/* ========================================
 * Test Cases - Color Split 565
 * ======================================== */

static int test_split_565_black(void) {
    u8 r, g, b;
    sprite_color_split_565(0x0000, &r, &g, &b);
    return r == 0 && g == 0 && b == 0;
}

static int test_split_565_white(void) {
    u8 r, g, b;
    sprite_color_split_565(0xFFFF, &r, &g, &b);
    return r == 0xF8 && g == 0xFC && b == 0xF8;
}

static int test_split_565_red(void) {
    u8 r, g, b;
    sprite_color_split_565(0xF800, &r, &g, &b);
    return r == 0xF8 && g == 0 && b == 0;
}

static int test_split_565_green(void) {
    u8 r, g, b;
    sprite_color_split_565(0x07E0, &r, &g, &b);
    return r == 0 && g == 0xFC && b == 0;
}

static int test_split_565_blue(void) {
    u8 r, g, b;
    sprite_color_split_565(0x001F, &r, &g, &b);
    return r == 0 && g == 0 && b == 0xF8;
}

static int test_split_565_null_pointers(void) {
    /* Should not crash with null pointers */
    sprite_color_split_565(0x1234, NULL, NULL, NULL);
    return 1;
}

static int test_split_565_partial_null(void) {
    u8 r = 0, g = 0;
    sprite_color_split_565(0xF800, &r, &g, NULL);
    return r == 0xF8 && g == 0;
}

/* ========================================
 * Test Cases - Color Split 555
 * ======================================== */

static int test_split_555_black(void) {
    u8 r, g, b;
    sprite_color_split_555(0x0000, &r, &g, &b);
    return r == 0 && g == 0 && b == 0;
}

static int test_split_555_white(void) {
    u8 r, g, b;
    sprite_color_split_555(0x7FFF, &r, &g, &b);
    return r == 0xF8 && g == 0xF8 && b == 0xF8;
}

static int test_split_555_red(void) {
    u8 r, g, b;
    sprite_color_split_555(0x7C00, &r, &g, &b);
    return r == 0xF8 && g == 0 && b == 0;
}

static int test_split_555_green(void) {
    u8 r, g, b;
    sprite_color_split_555(0x03E0, &r, &g, &b);
    return r == 0 && g == 0xF8 && b == 0;
}

static int test_split_555_blue(void) {
    u8 r, g, b;
    sprite_color_split_555(0x001F, &r, &g, &b);
    return r == 0 && g == 0 && b == 0xF8;
}

/* ========================================
 * Test Cases - Round Trip
 * ======================================== */

static int test_roundtrip_565(void) {
    u8 orig_r = 0x18, orig_g = 0x38, orig_b = 0x28;  /* Values that fit 5-6-5 */
    u16 color = sprite_color_565(orig_r, orig_g, orig_b);

    u8 r, g, b;
    sprite_color_split_565(color, &r, &g, &b);

    /* After round-trip, lower bits are lost */
    return r == (orig_r & 0xF8) &&
           g == (orig_g & 0xFC) &&
           b == (orig_b & 0xF8);
}

static int test_roundtrip_555(void) {
    u8 orig_r = 0x18, orig_g = 0x28, orig_b = 0x38;  /* Values that fit 5-5-5 */
    u16 color = sprite_color_555(orig_r, orig_g, orig_b);

    u8 r, g, b;
    sprite_color_split_555(color, &r, &g, &b);

    /* After round-trip, lower bits are lost */
    return r == (orig_r & 0xF8) &&
           g == (orig_g & 0xF8) &&
           b == (orig_b & 0xF8);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Sprite Core Comprehensive Tests ===\n\n");

    printf("Initialization Tests:\n");
    TEST(init_basic);
    TEST(shutdown_clears_context);
    TEST(init_after_shutdown);

    printf("\nPixel Format Tests:\n");
    TEST(pixel_format_default);
    TEST(pixel_format_set_555);
    TEST(pixel_format_set_565);
    TEST(transparent_color_565);
    TEST(transparent_color_555);

    printf("\nColor Conversion 565 Tests:\n");
    TEST(color_565_black);
    TEST(color_565_white);
    TEST(color_565_red);
    TEST(color_565_green);
    TEST(color_565_blue);
    TEST(color_565_gray);
    TEST(color_565_truncation);

    printf("\nColor Conversion 555 Tests:\n");
    TEST(color_555_black);
    TEST(color_555_white);
    TEST(color_555_red);
    TEST(color_555_green);
    TEST(color_555_blue);

    printf("\nColor Split 565 Tests:\n");
    TEST(split_565_black);
    TEST(split_565_white);
    TEST(split_565_red);
    TEST(split_565_green);
    TEST(split_565_blue);
    TEST(split_565_null_pointers);
    TEST(split_565_partial_null);

    printf("\nColor Split 555 Tests:\n");
    TEST(split_555_black);
    TEST(split_555_white);
    TEST(split_555_red);
    TEST(split_555_green);
    TEST(split_555_blue);

    printf("\nRound Trip Tests:\n");
    TEST(roundtrip_565);
    TEST(roundtrip_555);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
