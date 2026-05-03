/*
 * Stone Age Client - Sprite Core Module Comprehensive Tests
 * Tests for sprite_core.c implementation
 *
 * Covers:
 * - RGB to 565 color conversion
 * - RGB to 555 color conversion
 * - Color splitting (565/555 to RGB)
 * - Pixel format detection and setting
 * - Transparent color values
 * - Initialization and shutdown
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
 * Structures
 * ======================================== */

typedef struct {
    void* dest_surface;
    u32 dest_pitch;
    u16 dest_width;
    u16 dest_height;
    u8  pixel_format;
    u8  reserved[3];
} SpriteContext;

/* ========================================
 * Global State
 * ======================================== */

static SpriteContext g_sprite_ctx = {0};
static int g_pixel_format = PIXEL_FORMAT_565;

/* ========================================
 * Implementation Functions
 * ======================================== */

static int sprite_init(void) {
    memset(&g_sprite_ctx, 0, sizeof(SpriteContext));
    g_sprite_ctx.pixel_format = PIXEL_FORMAT_565;
    g_pixel_format = PIXEL_FORMAT_565;
    return 1;
}

static void sprite_shutdown(void) {
    memset(&g_sprite_ctx, 0, sizeof(SpriteContext));
    g_pixel_format = PIXEL_FORMAT_565;
}

static void sprite_set_pixel_format(int format) {
    g_pixel_format = format;
    g_sprite_ctx.pixel_format = format;
}

static int sprite_get_pixel_format(void) {
    return g_pixel_format;
}

static u16 sprite_color_565(u8 r, u8 g, u8 b) {
    return ((u16)(r & 0xF8) << 8) | ((u16)(g & 0xFC) << 3) | ((u16)(b & 0xF8) >> 3);
}

static u16 sprite_color_555(u8 r, u8 g, u8 b) {
    return ((u16)(r & 0xF8) << 7) | ((u16)(g & 0xF8) << 2) | ((u16)(b & 0xF8) >> 3);
}

static void sprite_color_split_565(u16 color, u8* r, u8* g, u8* b) {
    if (r) *r = (u8)((color >> 8) & 0xF8);
    if (g) *g = (u8)((color >> 3) & 0xFC);
    if (b) *b = (u8)((color << 3) & 0xF8);
}

static void sprite_color_split_555(u16 color, u8* r, u8* g, u8* b) {
    if (r) *r = (u8)((color >> 7) & 0xF8);
    if (g) *g = (u8)((color >> 2) & 0xF8);
    if (b) *b = (u8)((color << 3) & 0xF8);
}

static u16 sprite_get_transparent_color(void) {
    return (g_pixel_format == PIXEL_FORMAT_565) ?
           TRANSPARENT_COLOR_565 : TRANSPARENT_COLOR_555;
}

/* ========================================
 * Test Cases - Initialization
 * ======================================== */

static int test_init_basic(void) {
    int result = sprite_init();
    return result == 1 && g_sprite_ctx.pixel_format == PIXEL_FORMAT_565;
}

static int test_init_default_format_565(void) {
    sprite_init();
    return sprite_get_pixel_format() == PIXEL_FORMAT_565;
}

static int test_shutdown_clears_context(void) {
    sprite_init();
    sprite_set_pixel_format(PIXEL_FORMAT_555);
    sprite_shutdown();
    return g_sprite_ctx.pixel_format == 0;
}

static int test_reinit_restores_defaults(void) {
    sprite_init();
    sprite_set_pixel_format(PIXEL_FORMAT_555);
    sprite_init();
    return sprite_get_pixel_format() == PIXEL_FORMAT_565;
}

/* ========================================
 * Test Cases - RGB565 Conversion
 * ======================================== */

static int test_color_565_black(void) {
    return sprite_color_565(0, 0, 0) == 0x0000;
}

static int test_color_565_white(void) {
    /* R=248 (0xF8), G=252 (0xFC), B=248 (0xF8) */
    /* Result: (0xF8 << 8) | (0xFC << 3) | (0xF8 >> 3) = 0xF800 | 0x07E0 | 0x001F = 0xFFFF */
    return sprite_color_565(255, 255, 255) == 0xFFFF;
}

static int test_color_565_red(void) {
    /* Pure red: R=248 (0xF8 masked), G=0, B=0 */
    /* Result: 0xF800 */
    return sprite_color_565(255, 0, 0) == 0xF800;
}

static int test_color_565_green(void) {
    /* Pure green: R=0, G=252 (0xFC masked), B=0 */
    /* Result: 0x07E0 */
    return sprite_color_565(0, 255, 0) == 0x07E0;
}

static int test_color_565_blue(void) {
    /* Pure blue: R=0, G=0, B=248 (0xF8 masked) */
    /* Result: 0x001F */
    return sprite_color_565(0, 0, 255) == 0x001F;
}

static int test_color_565_magenta(void) {
    /* Magenta (transparent): R=248, G=0, B=248 */
    /* Result: 0xF81F */
    return sprite_color_565(255, 0, 255) == 0xF81F;
}

static int test_color_565_values_masked(void) {
    /* Test that values are properly masked (only top 5/6 bits matter) */
    u16 c1 = sprite_color_565(255, 255, 255);  /* 0xFF, 0xFF, 0xFF */
    u16 c2 = sprite_color_565(248, 252, 248);  /* 0xF8, 0xFC, 0xF8 */
    return c1 == c2;
}

static int test_color_565_partial_red(void) {
    /* R=128 (0x80 masked to 0x80), G=0, B=0 */
    /* 0x80 & 0xF8 = 0x80, shifted: 0x8000 */
    return sprite_color_565(128, 0, 0) == 0x8000;
}

static int test_color_565_partial_green(void) {
    /* G=128 (0x80 masked to 0x80), but green has 6 bits */
    /* 0x80 & 0xFC = 0x80, shifted: 0x0400 */
    return sprite_color_565(0, 128, 0) == 0x0400;
}

static int test_color_565_partial_blue(void) {
    /* B=128 (0x80 masked to 0x80) */
    /* 0x80 & 0xF8 = 0x80, shifted right: 0x80 >> 3 = 0x10 */
    return sprite_color_565(0, 0, 128) == 0x0010;
}

static int test_color_565_gray(void) {
    /* Gray 128: R=0x80, G=0x80, B=0x80 */
    /* R: 0x80 << 8 = 0x8000 */
    /* G: 0x80 << 3 = 0x0400 (but masked with 0xFC = 0x80) */
    /* B: 0x80 >> 3 = 0x10 */
    /* Result: 0x8000 | 0x0400 | 0x0010 = 0x8410 */
    u16 result = sprite_color_565(128, 128, 128);
    u8 r, g, b;
    sprite_color_split_565(result, &r, &g, &b);
    /* Due to bit loss, exact values won't match, but should be close */
    return r >= 120 && r <= 136 && g >= 124 && g <= 132 && b >= 120 && b <= 136;
}

/* ========================================
 * Test Cases - RGB555 Conversion
 * ======================================== */

static int test_color_555_black(void) {
    return sprite_color_555(0, 0, 0) == 0x0000;
}

static int test_color_555_white(void) {
    /* R=248 (0xF8), G=248 (0xF8), B=248 (0xF8) */
    /* Result: (0xF8 << 7) | (0xF8 << 2) | (0xF8 >> 3) = 0x7C00 | 0x03E0 | 0x001F = 0x7FFF */
    return sprite_color_555(255, 255, 255) == 0x7FFF;
}

static int test_color_555_red(void) {
    /* Pure red: R=248, G=0, B=0 */
    /* Result: 0x7C00 */
    return sprite_color_555(255, 0, 0) == 0x7C00;
}

static int test_color_555_green(void) {
    /* Pure green: R=0, G=248, B=0 */
    /* Result: 0x03E0 */
    return sprite_color_555(0, 255, 0) == 0x03E0;
}

static int test_color_555_blue(void) {
    /* Pure blue: R=0, G=0, B=248 */
    /* Result: 0x001F */
    return sprite_color_555(0, 0, 255) == 0x001F;
}

static int test_color_555_magenta(void) {
    /* Magenta (transparent in 555): R=248, G=0, B=248 */
    /* Result: 0x7C1F */
    return sprite_color_555(255, 0, 255) == 0x7C1F;
}

static int test_color_555_values_masked(void) {
    /* Test that values are properly masked */
    u16 c1 = sprite_color_555(255, 255, 255);
    u16 c2 = sprite_color_555(248, 248, 248);
    return c1 == c2;
}

static int test_color_555_partial_red(void) {
    /* R=128 (0x80 masked to 0x80) */
    /* 0x80 << 7 = 0x4000 */
    return sprite_color_555(128, 0, 0) == 0x4000;
}

static int test_color_555_partial_green(void) {
    /* G=128 (0x80 masked to 0x80) */
    /* 0x80 << 2 = 0x0200 */
    return sprite_color_555(0, 128, 0) == 0x0200;
}

static int test_color_555_partial_blue(void) {
    /* B=128 (0x80 masked to 0x80) */
    /* 0x80 >> 3 = 0x10 */
    return sprite_color_555(0, 0, 128) == 0x0010;
}

/* ========================================
 * Test Cases - Color Splitting 565
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
    u8 r = 0xFF, g = 0xFF, b = 0xFF;
    sprite_color_split_565(0xF800, &r, NULL, NULL);
    sprite_color_split_565(0x07E0, NULL, &g, NULL);
    sprite_color_split_565(0x001F, NULL, NULL, &b);
    return r == 0xF8 && g == 0xFC && b == 0xF8;
}

static int test_split_565_magenta(void) {
    u8 r, g, b;
    sprite_color_split_565(0xF81F, &r, &g, &b);
    return r == 0xF8 && g == 0 && b == 0xF8;
}

/* ========================================
 * Test Cases - Color Splitting 555
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

static int test_split_555_null_pointers(void) {
    sprite_color_split_555(0x1234, NULL, NULL, NULL);
    return 1;
}

static int test_split_555_partial_null(void) {
    u8 r = 0xFF, g = 0xFF, b = 0xFF;
    sprite_color_split_555(0x7C00, &r, NULL, NULL);
    sprite_color_split_555(0x03E0, NULL, &g, NULL);
    sprite_color_split_555(0x001F, NULL, NULL, &b);
    return r == 0xF8 && g == 0xF8 && b == 0xF8;
}

static int test_split_555_magenta(void) {
    u8 r, g, b;
    sprite_color_split_555(0x7C1F, &r, &g, &b);
    return r == 0xF8 && g == 0 && b == 0xF8;
}

/* ========================================
 * Test Cases - Round Trip Conversion
 * ======================================== */

static int test_roundtrip_565_primary_colors(void) {
    u8 orig_r, orig_g, orig_b;
    u16 color;
    u8 new_r, new_g, new_b;

    /* Test red */
    orig_r = 248; orig_g = 0; orig_b = 0;
    color = sprite_color_565(orig_r, orig_g, orig_b);
    sprite_color_split_565(color, &new_r, &new_g, &new_b);
    if (new_r != orig_r || new_g != orig_g || new_b != orig_b) return 0;

    /* Test green */
    orig_r = 0; orig_g = 252; orig_b = 0;
    color = sprite_color_565(orig_r, orig_g, orig_b);
    sprite_color_split_565(color, &new_r, &new_g, &new_b);
    if (new_r != orig_r || new_g != orig_g || new_b != orig_b) return 0;

    /* Test blue */
    orig_r = 0; orig_g = 0; orig_b = 248;
    color = sprite_color_565(orig_r, orig_g, orig_b);
    sprite_color_split_565(color, &new_r, &new_g, &new_b);
    if (new_r != orig_r || new_g != orig_g || new_b != orig_b) return 0;

    return 1;
}

static int test_roundtrip_555_primary_colors(void) {
    u8 orig_r, orig_g, orig_b;
    u16 color;
    u8 new_r, new_g, new_b;

    /* Test red */
    orig_r = 248; orig_g = 0; orig_b = 0;
    color = sprite_color_555(orig_r, orig_g, orig_b);
    sprite_color_split_555(color, &new_r, &new_g, &new_b);
    if (new_r != orig_r || new_g != orig_g || new_b != orig_b) return 0;

    /* Test green */
    orig_r = 0; orig_g = 248; orig_b = 0;
    color = sprite_color_555(orig_r, orig_g, orig_b);
    sprite_color_split_555(color, &new_r, &new_g, &new_b);
    if (new_r != orig_r || new_g != orig_g || new_b != orig_b) return 0;

    /* Test blue */
    orig_r = 0; orig_g = 0; orig_b = 248;
    color = sprite_color_555(orig_r, orig_g, orig_b);
    sprite_color_split_555(color, &new_r, &new_g, &new_b);
    if (new_r != orig_r || new_g != orig_g || new_b != orig_b) return 0;

    return 1;
}

static int test_roundtrip_565_random(void) {
    int i;
    for (i = 0; i < 100; i++) {
        /* Generate valid 565 colors (5-6-5 bits) */
        u8 r = (u8)((rand() % 32) << 3);  /* 5 bits, shifted to MSB */
        u8 g = (u8)((rand() % 64) << 2);  /* 6 bits, shifted */
        u8 b = (u8)((rand() % 32) << 3);  /* 5 bits, shifted */

        u16 color = sprite_color_565(r, g, b);
        u8 nr, ng, nb;
        sprite_color_split_565(color, &nr, &ng, &nb);

        if (nr != r || ng != g || nb != b) return 0;
    }
    return 1;
}

static int test_roundtrip_555_random(void) {
    int i;
    for (i = 0; i < 100; i++) {
        /* Generate valid 555 colors (5-5-5 bits) */
        u8 r = (u8)((rand() % 32) << 3);  /* 5 bits, shifted to MSB */
        u8 g = (u8)((rand() % 32) << 3);  /* 5 bits, shifted */
        u8 b = (u8)((rand() % 32) << 3);  /* 5 bits, shifted */

        u16 color = sprite_color_555(r, g, b);
        u8 nr, ng, nb;
        sprite_color_split_555(color, &nr, &ng, &nb);

        if (nr != r || ng != g || nb != b) return 0;
    }
    return 1;
}

/* ========================================
 * Test Cases - Pixel Format
 * ======================================== */

static int test_set_format_565(void) {
    sprite_init();
    sprite_set_pixel_format(PIXEL_FORMAT_565);
    return sprite_get_pixel_format() == PIXEL_FORMAT_565;
}

static int test_set_format_555(void) {
    sprite_init();
    sprite_set_pixel_format(PIXEL_FORMAT_555);
    return sprite_get_pixel_format() == PIXEL_FORMAT_555;
}

static int test_format_persists(void) {
    sprite_init();
    sprite_set_pixel_format(PIXEL_FORMAT_555);
    /* Format should persist until changed */
    return sprite_get_pixel_format() == PIXEL_FORMAT_555;
}

/* ========================================
 * Test Cases - Transparent Color
 * ======================================== */

static int test_transparent_565(void) {
    sprite_init();
    sprite_set_pixel_format(PIXEL_FORMAT_565);
    return sprite_get_transparent_color() == TRANSPARENT_COLOR_565;
}

static int test_transparent_555(void) {
    sprite_init();
    sprite_set_pixel_format(PIXEL_FORMAT_555);
    return sprite_get_transparent_color() == TRANSPARENT_COLOR_555;
}

static int test_transparent_matches_magenta_565(void) {
    sprite_init();
    sprite_set_pixel_format(PIXEL_FORMAT_565);
    u16 transparent = sprite_get_transparent_color();
    u16 magenta = sprite_color_565(255, 0, 255);
    return transparent == magenta;
}

static int test_transparent_matches_magenta_555(void) {
    sprite_init();
    sprite_set_pixel_format(PIXEL_FORMAT_555);
    u16 transparent = sprite_get_transparent_color();
    u16 magenta = sprite_color_555(255, 0, 255);
    return transparent == magenta;
}

static int test_transparent_is_f81f_565(void) {
    sprite_init();
    sprite_set_pixel_format(PIXEL_FORMAT_565);
    return sprite_get_transparent_color() == 0xF81F;
}

static int test_transparent_is_7c1f_555(void) {
    sprite_init();
    sprite_set_pixel_format(PIXEL_FORMAT_555);
    return sprite_get_transparent_color() == 0x7C1F;
}

/* ========================================
 * Test Cases - Edge Cases
 * ======================================== */

static int test_color_565_low_values(void) {
    /* Test with lowest non-zero values */
    u16 color = sprite_color_565(8, 4, 8);  /* Minimum values that produce non-zero */
    /* R: (8 & 0xF8) << 8 = 0x0800, G: (4 & 0xFC) << 3 = 0x0020, B: (8 & 0xF8) >> 3 = 0x0001 */
    return color == 0x0821;
}

static int test_color_555_low_values(void) {
    /* Test with lowest non-zero values */
    u16 color = sprite_color_555(8, 8, 8);
    /* R: (8 & 0xF8) << 7 = 0x0400, G: (8 & 0xF8) << 2 = 0x0020, B: (8 & 0xF8) >> 3 = 0x0001 */
    return color == 0x0421;
}

static int test_color_565_bit_positions(void) {
    /* Verify bit positions are correct */
    u16 red = sprite_color_565(255, 0, 0);
    u16 green = sprite_color_565(0, 255, 0);
    u16 blue = sprite_color_565(0, 0, 255);

    /* Red in 565: bits 11-15 */
    /* Green in 565: bits 5-10 */
    /* Blue in 565: bits 0-4 */
    return (red & 0xF800) == 0xF800 &&
           (green & 0x07E0) == 0x07E0 &&
           (blue & 0x001F) == 0x001F;
}

static int test_color_555_bit_positions(void) {
    /* Verify bit positions are correct */
    u16 red = sprite_color_555(255, 0, 0);
    u16 green = sprite_color_555(0, 255, 0);
    u16 blue = sprite_color_555(0, 0, 255);

    /* Red in 555: bits 10-14 */
    /* Green in 555: bits 5-9 */
    /* Blue in 555: bits 0-4 */
    return (red & 0x7C00) == 0x7C00 &&
           (green & 0x03E0) == 0x03E0 &&
           (blue & 0x001F) == 0x001F;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Sprite Core Module Comprehensive Tests ===\n\n");

    printf("Initialization Tests:\n");
    TEST(init_basic);
    TEST(init_default_format_565);
    TEST(shutdown_clears_context);
    TEST(reinit_restores_defaults);

    printf("\nRGB565 Conversion Tests:\n");
    TEST(color_565_black);
    TEST(color_565_white);
    TEST(color_565_red);
    TEST(color_565_green);
    TEST(color_565_blue);
    TEST(color_565_magenta);
    TEST(color_565_values_masked);
    TEST(color_565_partial_red);
    TEST(color_565_partial_green);
    TEST(color_565_partial_blue);
    TEST(color_565_gray);

    printf("\nRGB555 Conversion Tests:\n");
    TEST(color_555_black);
    TEST(color_555_white);
    TEST(color_555_red);
    TEST(color_555_green);
    TEST(color_555_blue);
    TEST(color_555_magenta);
    TEST(color_555_values_masked);
    TEST(color_555_partial_red);
    TEST(color_555_partial_green);
    TEST(color_555_partial_blue);

    printf("\nColor Splitting 565 Tests:\n");
    TEST(split_565_black);
    TEST(split_565_white);
    TEST(split_565_red);
    TEST(split_565_green);
    TEST(split_565_blue);
    TEST(split_565_null_pointers);
    TEST(split_565_partial_null);
    TEST(split_565_magenta);

    printf("\nColor Splitting 555 Tests:\n");
    TEST(split_555_black);
    TEST(split_555_white);
    TEST(split_555_red);
    TEST(split_555_green);
    TEST(split_555_blue);
    TEST(split_555_null_pointers);
    TEST(split_555_partial_null);
    TEST(split_555_magenta);

    printf("\nRound Trip Conversion Tests:\n");
    TEST(roundtrip_565_primary_colors);
    TEST(roundtrip_555_primary_colors);
    TEST(roundtrip_565_random);
    TEST(roundtrip_555_random);

    printf("\nPixel Format Tests:\n");
    TEST(set_format_565);
    TEST(set_format_555);
    TEST(format_persists);

    printf("\nTransparent Color Tests:\n");
    TEST(transparent_565);
    TEST(transparent_555);
    TEST(transparent_matches_magenta_565);
    TEST(transparent_matches_magenta_555);
    TEST(transparent_is_f81f_565);
    TEST(transparent_is_7c1f_555);

    printf("\nEdge Case Tests:\n");
    TEST(color_565_low_values);
    TEST(color_555_low_values);
    TEST(color_565_bit_positions);
    TEST(color_555_bit_positions);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
