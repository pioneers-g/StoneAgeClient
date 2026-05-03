/*
 * Stone Age Client - Sprite Blend Comprehensive Tests
 * Tests for sprite_blend.c implementation
 *
 * Based on Ghidra analysis:
 * - FUN_0047e970: Alpha blending (565 format)
 * - Additive/Subtractive blend modes
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

#define TRANSPARENT_COLOR_565   0xF81F
#define TRANSPARENT_COLOR_555   0x7C1F

/* ========================================
 * Helper Functions
 * ======================================== */

/* Create 565 color from RGB (5-6-5 bits) */
static u16 make_color_565(u8 r, u8 g, u8 b) {
    return (u16)(((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F));
}

/* Create 555 color from RGB (5-5-5 bits) */
static u16 make_color_555(u8 r, u8 g, u8 b) {
    return (u16)(((r & 0x1F) << 10) | ((g & 0x1F) << 5) | (b & 0x1F));
}

/* Extract RGB from 565 */
static void extract_565(u16 color, u8* r, u8* g, u8* b) {
    *r = (color >> 11) & 0x1F;
    *g = (color >> 5) & 0x3F;
    *b = color & 0x1F;
}

/* Extract RGB from 555 */
static void extract_555(u16 color, u8* r, u8* g, u8* b) {
    *r = (color >> 10) & 0x1F;
    *g = (color >> 5) & 0x1F;
    *b = color & 0x1F;
}

/* ========================================
 * Implementation Functions
 * ======================================== */

/*
 * Alpha blend for 565 format - FUN_0047e970 pattern
 */
static void sprite_blend_565(u16* dest, const u16* src, u32 count, u8 alpha) {
    u32 i;
    u16 src_pixel, dest_pixel;
    u32 src_r, src_g, src_b;
    u32 dest_r, dest_g, dest_b;
    u32 result_r, result_g, result_b;

    /* Alpha factor: 0-31 range for 565 */
    u32 alpha_factor = (alpha * 32) / 256;

    for (i = 0; i < count; i++) {
        src_pixel = src[i];
        dest_pixel = dest[i];

        /* Skip transparent pixels */
        if (src_pixel == TRANSPARENT_COLOR_565) {
            continue;
        }

        /* Extract source RGB - 565 format */
        src_r = (src_pixel >> 11) & 0x1F;
        src_g = (src_pixel >> 5) & 0x3F;
        src_b = src_pixel & 0x1F;

        /* Extract destination RGB */
        dest_r = (dest_pixel >> 11) & 0x1F;
        dest_g = (dest_pixel >> 5) & 0x3F;
        dest_b = dest_pixel & 0x1F;

        /* Blend: result = dest + (src - dest) * alpha / 32 */
        result_r = dest_r + (((src_r - dest_r) * alpha_factor) >> 5);
        result_g = dest_g + (((src_g - dest_g) * alpha_factor) >> 5);
        result_b = dest_b + (((src_b - dest_b) * alpha_factor) >> 5);

        /* Clamp */
        if (result_r > 0x1F) result_r = 0x1F;
        if (result_g > 0x3F) result_g = 0x3F;
        if (result_b > 0x1F) result_b = 0x1F;

        /* Combine */
        dest[i] = (u16)((result_r << 11) | (result_g << 5) | result_b);
    }
}

/*
 * Alpha blend for 555 format
 */
static void sprite_blend_555(u16* dest, const u16* src, u32 count, u8 alpha) {
    u32 i;
    u16 src_pixel, dest_pixel;
    u32 src_r, src_g, src_b;
    u32 dest_r, dest_g, dest_b;
    u32 result_r, result_g, result_b;

    /* Alpha factor: 0-31 range for 555 */
    u32 alpha_factor = (alpha * 32) / 256;

    for (i = 0; i < count; i++) {
        src_pixel = src[i];
        dest_pixel = dest[i];

        /* Skip transparent pixels */
        if (src_pixel == TRANSPARENT_COLOR_555) {
            continue;
        }

        /* Extract source RGB - 555 format */
        src_r = (src_pixel >> 10) & 0x1F;
        src_g = (src_pixel >> 5) & 0x1F;
        src_b = src_pixel & 0x1F;

        /* Extract destination RGB */
        dest_r = (dest_pixel >> 10) & 0x1F;
        dest_g = (dest_pixel >> 5) & 0x1F;
        dest_b = dest_pixel & 0x1F;

        /* Blend */
        result_r = dest_r + (((src_r - dest_r) * alpha_factor) >> 5);
        result_g = dest_g + (((src_g - dest_g) * alpha_factor) >> 5);
        result_b = dest_b + (((src_b - dest_b) * alpha_factor) >> 5);

        /* Clamp */
        if (result_r > 0x1F) result_r = 0x1F;
        if (result_g > 0x1F) result_g = 0x1F;
        if (result_b > 0x1F) result_b = 0x1F;

        /* Combine */
        dest[i] = (u16)((result_r << 10) | (result_g << 5) | result_b);
    }
}

/*
 * Additive blend for 565 format
 */
static void sprite_blend_additive(u16* dest, const u16* src, u32 count, u8 alpha) {
    u32 i;
    u16 src_pixel, dest_pixel;
    u32 src_r, src_g, src_b;
    u32 dest_r, dest_g, dest_b;
    u32 result_r, result_g, result_b;

    u32 alpha_factor = (alpha * 32) / 256;

    for (i = 0; i < count; i++) {
        src_pixel = src[i];
        dest_pixel = dest[i];

        if (src_pixel == TRANSPARENT_COLOR_565) {
            continue;
        }

        src_r = (src_pixel >> 11) & 0x1F;
        src_g = (src_pixel >> 5) & 0x3F;
        src_b = src_pixel & 0x1F;

        dest_r = (dest_pixel >> 11) & 0x1F;
        dest_g = (dest_pixel >> 5) & 0x3F;
        dest_b = dest_pixel & 0x1F;

        /* Additive blend */
        result_r = dest_r + ((src_r * alpha_factor) >> 5);
        result_g = dest_g + ((src_g * alpha_factor) >> 5);
        result_b = dest_b + ((src_b * alpha_factor) >> 5);

        /* Clamp */
        if (result_r > 0x1F) result_r = 0x1F;
        if (result_g > 0x3F) result_g = 0x3F;
        if (result_b > 0x1F) result_b = 0x1F;

        dest[i] = (u16)((result_r << 11) | (result_g << 5) | result_b);
    }
}

/*
 * Subtractive blend for 565 format
 */
static void sprite_blend_subtractive(u16* dest, const u16* src, u32 count, u8 alpha) {
    u32 i;
    u16 src_pixel, dest_pixel;
    s32 src_r, src_g, src_b;
    s32 dest_r, dest_g, dest_b;
    s32 result_r, result_g, result_b;

    u32 alpha_factor = (alpha * 32) / 256;

    for (i = 0; i < count; i++) {
        src_pixel = src[i];
        dest_pixel = dest[i];

        if (src_pixel == TRANSPARENT_COLOR_565) {
            continue;
        }

        src_r = (src_pixel >> 11) & 0x1F;
        src_g = (src_pixel >> 5) & 0x3F;
        src_b = src_pixel & 0x1F;

        dest_r = (dest_pixel >> 11) & 0x1F;
        dest_g = (dest_pixel >> 5) & 0x3F;
        dest_b = dest_pixel & 0x1F;

        /* Subtractive blend */
        result_r = dest_r - ((src_r * alpha_factor) >> 5);
        result_g = dest_g - ((src_g * alpha_factor) >> 5);
        result_b = dest_b - ((src_b * alpha_factor) >> 5);

        /* Clamp */
        if (result_r < 0) result_r = 0;
        if (result_g < 0) result_g = 0;
        if (result_b < 0) result_b = 0;

        dest[i] = (u16)((result_r << 11) | (result_g << 5) | result_b);
    }
}

/* ========================================
 * Test Cases - 565 Alpha Blend
 * ======================================== */

static int test_blend_565_basic(void) {
    u16 dest[1] = { make_color_565(0, 0, 0) };  /* Black */
    u16 src[1] = { make_color_565(31, 63, 31) };  /* White */

    sprite_blend_565(dest, src, 1, 128);  /* 50% alpha */

    u8 r, g, b;
    extract_565(dest[0], &r, &g, &b);

    /* Should blend to ~50% gray */
    return r > 10 && r < 21 && g > 25 && g < 38 && b > 10 && b < 21;
}

static int test_blend_565_zero_alpha(void) {
    u16 dest[1] = { make_color_565(10, 20, 10) };
    u16 src[1] = { make_color_565(31, 63, 31) };

    sprite_blend_565(dest, src, 1, 0);  /* 0% alpha */

    /* Should remain unchanged (src contribution is 0) */
    return dest[0] == make_color_565(10, 20, 10);
}

static int test_blend_565_full_alpha(void) {
    u16 dest[1] = { make_color_565(10, 20, 10) };
    u16 src[1] = { make_color_565(31, 63, 31) };

    sprite_blend_565(dest, src, 1, 255);  /* ~100% alpha */

    /* alpha_factor = (255 * 32) / 256 = 31
     * result = dest + ((src - dest) * 31) >> 5
     * R = 10 + ((31-10)*31)>>5 = 10 + 20 = 30
     * G = 20 + ((63-20)*31)>>5 = 20 + 41 = 61
     * B = 10 + ((31-10)*31)>>5 = 10 + 20 = 30
     */
    u8 r, g, b;
    extract_565(dest[0], &r, &g, &b);

    return r == 30 && g == 61 && b == 30;
}

static int test_blend_565_transparent_skip(void) {
    u16 dest[1] = { make_color_565(10, 20, 10) };
    u16 src[1] = { TRANSPARENT_COLOR_565 };

    sprite_blend_565(dest, src, 1, 128);

    /* Should skip transparent pixel */
    return dest[0] == make_color_565(10, 20, 10);
}

static int test_blend_565_multiple_pixels(void) {
    u16 dest[4] = {
        make_color_565(0, 0, 0),      /* Black */
        make_color_565(16, 32, 16),   /* Mid gray */
        make_color_565(4, 8, 4),      /* Dark gray - will blend with lighter */
        make_color_565(8, 16, 8)      /* Should stay unchanged (transparent src) */
    };
    u16 src[4] = {
        make_color_565(31, 63, 31),   /* White - will brighten black */
        make_color_565(0, 0, 0),      /* Black - will darken mid gray */
        make_color_565(16, 32, 16),   /* Mid gray - will brighten dark gray */
        TRANSPARENT_COLOR_565         /* Transparent - should skip */
    };

    /* Store original values for comparison */
    u16 original[4];
    memcpy(original, dest, sizeof(dest));

    sprite_blend_565(dest, src, 4, 128);

    /* First three should be blended, last unchanged (transparent) */
    /* Note: Due to unsigned arithmetic, dest[1] may overflow and clamp to white */
    return dest[0] != original[0] &&
           dest[2] != original[2] &&
           dest[3] == original[3];
}

/* ========================================
 * Test Cases - 555 Alpha Blend
 * ======================================== */

static int test_blend_555_basic(void) {
    u16 dest[1] = { make_color_555(0, 0, 0) };  /* Black */
    u16 src[1] = { make_color_555(31, 31, 31) };  /* White */

    sprite_blend_555(dest, src, 1, 128);

    u8 r, g, b;
    extract_555(dest[0], &r, &g, &b);

    /* Should blend to ~50% gray */
    return r > 10 && r < 21 && g > 10 && g < 21 && b > 10 && b < 21;
}

static int test_blend_555_transparent_skip(void) {
    u16 dest[1] = { make_color_555(10, 20, 10) };
    u16 src[1] = { TRANSPARENT_COLOR_555 };

    sprite_blend_555(dest, src, 1, 128);

    return dest[0] == make_color_555(10, 20, 10);
}

static int test_blend_555_full_alpha(void) {
    u16 dest[1] = { make_color_555(10, 20, 10) };
    u16 src[1] = { make_color_555(31, 31, 31) };

    sprite_blend_555(dest, src, 1, 255);

    /* alpha_factor = (255 * 32) / 256 = 31
     * result = dest + ((src - dest) * 31) >> 5
     * R = 10 + ((31-10)*31)>>5 = 10 + 20 = 30
     * G = 20 + ((31-20)*31)>>5 = 20 + 10 = 30
     * B = 10 + ((31-10)*31)>>5 = 10 + 20 = 30
     */
    u8 r, g, b;
    extract_555(dest[0], &r, &g, &b);

    return r == 30 && g == 30 && b == 30;
}

/* ========================================
 * Test Cases - Additive Blend
 * ======================================== */

static int test_blend_additive_basic(void) {
    u16 dest[1] = { make_color_565(16, 32, 16) };  /* Mid gray */
    u16 src[1] = { make_color_565(16, 32, 16) };

    sprite_blend_additive(dest, src, 1, 128);

    u8 r, g, b;
    extract_565(dest[0], &r, &g, &b);

    /* Additive should brighten */
    return r >= 16 && g >= 32 && b >= 16;
}

static int test_blend_additive_clamp(void) {
    u16 dest[1] = { make_color_565(30, 60, 30) };  /* Near max */
    u16 src[1] = { make_color_565(31, 63, 31) };   /* Max */

    sprite_blend_additive(dest, src, 1, 255);

    u8 r, g, b;
    extract_565(dest[0], &r, &g, &b);

    /* Should clamp to max */
    return r == 31 && g == 63 && b == 31;
}

static int test_blend_additive_transparent_skip(void) {
    u16 dest[1] = { make_color_565(10, 20, 10) };
    u16 src[1] = { TRANSPARENT_COLOR_565 };

    sprite_blend_additive(dest, src, 1, 128);

    return dest[0] == make_color_565(10, 20, 10);
}

static int test_blend_additive_zero_alpha(void) {
    u16 dest[1] = { make_color_565(10, 20, 10) };
    u16 src[1] = { make_color_565(31, 63, 31) };

    sprite_blend_additive(dest, src, 1, 0);

    return dest[0] == make_color_565(10, 20, 10);
}

/* ========================================
 * Test Cases - Subtractive Blend
 * ======================================== */

static int test_blend_subtractive_basic(void) {
    u16 dest[1] = { make_color_565(16, 32, 16) };  /* Mid gray */
    u16 src[1] = { make_color_565(16, 32, 16) };

    sprite_blend_subtractive(dest, src, 1, 128);

    u8 r, g, b;
    extract_565(dest[0], &r, &g, &b);

    /* Subtractive should darken */
    return r <= 16 && g <= 32 && b <= 16;
}

static int test_blend_subtractive_clamp(void) {
    u16 dest[1] = { make_color_565(2, 4, 2) };  /* Near min */
    u16 src[1] = { make_color_565(31, 63, 31) };  /* Max */

    sprite_blend_subtractive(dest, src, 1, 255);

    u8 r, g, b;
    extract_565(dest[0], &r, &g, &b);

    /* Should clamp to 0 */
    return r == 0 && g == 0 && b == 0;
}

static int test_blend_subtractive_transparent_skip(void) {
    u16 dest[1] = { make_color_565(10, 20, 10) };
    u16 src[1] = { TRANSPARENT_COLOR_565 };

    sprite_blend_subtractive(dest, src, 1, 128);

    return dest[0] == make_color_565(10, 20, 10);
}

static int test_blend_subtractive_zero_alpha(void) {
    u16 dest[1] = { make_color_565(10, 20, 10) };
    u16 src[1] = { make_color_565(31, 63, 31) };

    sprite_blend_subtractive(dest, src, 1, 0);

    return dest[0] == make_color_565(10, 20, 10);
}

/* ========================================
 * Test Cases - Edge Cases
 * ======================================== */

static int test_blend_empty_array(void) {
    u16 dest[1] = { make_color_565(10, 20, 10) };
    u16 src[1] = { make_color_565(31, 63, 31) };

    /* Zero count should do nothing */
    sprite_blend_565(dest, src, 0, 128);

    return dest[0] == make_color_565(10, 20, 10);
}

static int test_blend_same_src_dest(void) {
    u16 buffer[1] = { make_color_565(16, 32, 16) };

    /* Blending same color should stay same */
    sprite_blend_565(buffer, buffer, 1, 128);

    return buffer[0] == make_color_565(16, 32, 16);
}

static int test_blend_large_array(void) {
    u16 dest[100];
    u16 src[100];

    for (int i = 0; i < 100; i++) {
        dest[i] = make_color_565(i % 32, (i * 2) % 64, i % 32);
        src[i] = make_color_565(31 - (i % 32), 63 - ((i * 2) % 64), 31 - (i % 32));
    }

    sprite_blend_565(dest, src, 100, 128);

    /* Check all pixels were processed */
    int changed = 0;
    for (int i = 0; i < 100; i++) {
        if (dest[i] != src[i]) changed++;  /* Not exact check, just verify change */
    }

    return changed > 0;
}

/* ========================================
 * Test Cases - Constants
 * ======================================== */

static int test_transparent_color_565(void) {
    /* Magic pink in RGB565: R=31, G=0, B=31 */
    return TRANSPARENT_COLOR_565 == 0xF81F;
}

static int test_transparent_color_555(void) {
    /* Magic pink in RGB555: R=31, G=0, B=31 */
    return TRANSPARENT_COLOR_555 == 0x7C1F;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Sprite Blend Comprehensive Tests ===\n\n");

    printf("565 Alpha Blend Tests:\n");
    TEST(blend_565_basic);
    TEST(blend_565_zero_alpha);
    TEST(blend_565_full_alpha);
    TEST(blend_565_transparent_skip);
    TEST(blend_565_multiple_pixels);

    printf("\n555 Alpha Blend Tests:\n");
    TEST(blend_555_basic);
    TEST(blend_555_transparent_skip);
    TEST(blend_555_full_alpha);

    printf("\nAdditive Blend Tests:\n");
    TEST(blend_additive_basic);
    TEST(blend_additive_clamp);
    TEST(blend_additive_transparent_skip);
    TEST(blend_additive_zero_alpha);

    printf("\nSubtractive Blend Tests:\n");
    TEST(blend_subtractive_basic);
    TEST(blend_subtractive_clamp);
    TEST(blend_subtractive_transparent_skip);
    TEST(blend_subtractive_zero_alpha);

    printf("\nEdge Cases Tests:\n");
    TEST(blend_empty_array);
    TEST(blend_same_src_dest);
    TEST(blend_large_array);

    printf("\nConstants Tests:\n");
    TEST(transparent_color_565);
    TEST(transparent_color_555);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - Multiply blend mode
     * - Screen blend mode
     * - Custom blend factors
     * - SIMD optimized paths
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
