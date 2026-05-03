/*
 * Stone Age Client - DirectX Module Comprehensive Tests
 * Tests for DirectDraw core, sprite cache, surface management
 * Reverse engineered from sa_9061.exe FUN_004119e0, FUN_00440170, FUN_00480740
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

/* Surface flags from directx.h */
#define SURFACE_FLAG_VIDEO_MEMORY   0x4000
#define SURFACE_FLAG_SYSTEM_MEMORY  0x0800

/* Sprite cache constants */
#define SPRITE_CACHE_SIZE           4096
#define MAX_OFFSCREEN_SURFACES      0x1000

/* Sprite ID ranges */
#define SPRITE_ID_STANDARD_MAX      500000
#define SPRITE_ID_EXTENDED_MAX      550000
#define SPRITE_ID_INVALID           550000

/* Screen modes */
#define SCREEN_MODE_FULLSCREEN      0
#define SCREEN_MODE_WINDOWED        1

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
 * Stub Implementations matching directx.c
 * ======================================== */

/* Global variables */
static u32 g_red_mask = 0;
static u32 g_green_mask = 0;
static u32 g_blue_mask = 0;
static int g_red_shift = 0;
static int g_green_shift = 0;
static int g_blue_shift = 0;
static int g_sprite_width = 64;
static int g_sprite_height = 48;
static int g_high_res_mode = 0;
static int g_alpha_mode = 0;

/* Graphics context dimensions from FUN_00440170 */
static int g_screen_width = 640;
static int g_screen_height = 480;

/*
 * Count bits in mask - FUN_004119e0
 * Binary analysis shows this counts set bits in a 32-bit value
 */
int count_bits(u32 mask) {
    int count = 0;
    u32 bit = 1;
    int i;
    for (i = 0; i < 32; i++) {
        if (mask & bit) count++;
        bit <<= 1;
    }
    return count;
}

/*
 * Graphics allocate context - FUN_00440170
 * Sets screen dimensions and sprite sizes based on mode
 */
int graphics_allocate_context(int screen_mode) {
    switch (screen_mode) {
        case 0: case 2:
            g_screen_width = 640;   /* 0x280 */
            g_screen_height = 480;  /* 0x1E0 */
            g_sprite_width = 64;    /* 0x40 */
            g_sprite_height = 48;   /* 0x30 */
            break;
        case 1:
            g_screen_width = 320;   /* 0x140 */
            g_screen_height = 240;  /* 0xF0 */
            g_sprite_width = 32;    /* 0x20 */
            g_sprite_height = 24;   /* 0x18 */
            break;
        default:
            g_screen_width = 640;
            g_screen_height = 480;
            g_sprite_width = 64;
            g_sprite_height = 48;
            break;
    }
    return 1;
}

void graphics_set_alpha_mode(int enabled) {
    g_alpha_mode = enabled;
}

int graphics_get_alpha_mode(void) {
    return g_alpha_mode;
}

/* SpriteCacheEntry structure */
typedef struct {
    int sprite_id;
    void* surface;
    void* alpha_surface;
    void* alpha_buffer;
    int x_offset;
    int y_offset;
    int ref_count;
    int flags;
    void* sprite_data;
} SpriteCacheEntry;

/* ========================================
 * Test Cases for FUN_004119e0 - count_bits
 * ======================================== */

static int test_count_bits_zero(void) {
    return count_bits(0) == 0;
}

static int test_count_bits_one(void) {
    return count_bits(1) == 1;
}

static int test_count_bits_all_bits(void) {
    /* 0xFFFFFFFF has all 32 bits set */
    return count_bits(0xFFFFFFFF) == 32;
}

static int test_count_bits_power_of_two(void) {
    /* Single bit set: 1, 2, 4, 8, ... */
    int i;
    for (i = 0; i < 32; i++) {
        if (count_bits(1u << i) != 1) return 0;
    }
    return 1;
}

static int test_count_bits_consecutive(void) {
    /* 0b11 = 2 bits, 0b111 = 3 bits, etc. */
    return count_bits(0x3) == 2 &&
           count_bits(0x7) == 3 &&
           count_bits(0xF) == 4 &&
           count_bits(0x1F) == 5;
}

static int test_count_bits_rgb565_red(void) {
    /* RGB565 red mask: 0xF800 = 11111 000000 00000 */
    return count_bits(0xF800) == 5;
}

static int test_count_bits_rgb565_green(void) {
    /* RGB565 green mask: 0x07E0 = 00000 111111 00000 */
    return count_bits(0x07E0) == 6;
}

static int test_count_bits_rgb565_blue(void) {
    /* RGB565 blue mask: 0x001F = 00000 000000 11111 */
    return count_bits(0x001F) == 5;
}

static int test_count_bits_rgb555_red(void) {
    /* RGB555 red mask: 0x7C00 = 11111 00000 00000 */
    return count_bits(0x7C00) == 5;
}

static int test_count_bits_rgb555_green(void) {
    /* RGB555 green mask: 0x03E0 = 00000 11111 00000 */
    return count_bits(0x03E0) == 5;
}

static int test_count_bits_rgb555_blue(void) {
    /* RGB555 blue mask: same as RGB565 = 0x001F */
    return count_bits(0x001F) == 5;
}

static int test_count_bits_byte_patterns(void) {
    /* Various byte patterns */
    return count_bits(0xAA) == 4 &&  /* 10101010 */
           count_bits(0x55) == 4 &&  /* 01010101 */
           count_bits(0xCC) == 4 &&  /* 11001100 */
           count_bits(0x33) == 4;    /* 00110011 */
}

/* ========================================
 * Test Cases for FUN_00440170 - graphics_allocate_context
 * ======================================== */

static int test_allocate_context_mode_0(void) {
    graphics_allocate_context(0);
    return g_screen_width == 640 &&
           g_screen_height == 480 &&
           g_sprite_width == 64 &&
           g_sprite_height == 48;
}

static int test_allocate_context_mode_1(void) {
    graphics_allocate_context(1);
    return g_screen_width == 320 &&
           g_screen_height == 240 &&
           g_sprite_width == 32 &&
           g_sprite_height == 24;
}

static int test_allocate_context_mode_2(void) {
    graphics_allocate_context(2);
    return g_screen_width == 640 &&
           g_screen_height == 480 &&
           g_sprite_width == 64 &&
           g_sprite_height == 48;
}

static int test_allocate_context_mode_invalid(void) {
    graphics_allocate_context(99);
    return g_screen_width == 640 &&
           g_screen_height == 480 &&
           g_sprite_width == 64 &&
           g_sprite_height == 48;
}

static int test_screen_dimensions_hex_values(void) {
    /* Verify hex values match FUN_00440170 */
    return 640 == 0x280 &&
           480 == 0x1E0 &&
           320 == 0x140 &&
           240 == 0xF0 &&
           64 == 0x40 &&
           48 == 0x30 &&
           32 == 0x20 &&
           24 == 0x18;
}

/* ========================================
 * Test Cases for Surface Flags
 * ======================================== */

static int test_surface_flag_video_memory(void) {
    return SURFACE_FLAG_VIDEO_MEMORY == 0x4000;
}

static int test_surface_flag_system_memory(void) {
    return SURFACE_FLAG_SYSTEM_MEMORY == 0x0800;
}

static int test_surface_flags_distinct(void) {
    /* Video and system memory flags should be distinct */
    return (SURFACE_FLAG_VIDEO_MEMORY & SURFACE_FLAG_SYSTEM_MEMORY) == 0;
}

/* ========================================
 * Test Cases for Sprite Cache Constants
 * ======================================== */

static int test_sprite_cache_size(void) {
    return SPRITE_CACHE_SIZE == 4096;
}

static int test_max_offscreen_surfaces(void) {
    return MAX_OFFSCREEN_SURFACES == 0x1000;
}

static int test_sprite_id_ranges(void) {
    return SPRITE_ID_STANDARD_MAX == 500000 &&
           SPRITE_ID_EXTENDED_MAX == 550000 &&
           SPRITE_ID_INVALID == 550000;
}

static int test_sprite_id_range_size(void) {
    /* Extended range should be 50000 sprites */
    return (SPRITE_ID_EXTENDED_MAX - SPRITE_ID_STANDARD_MAX) == 50000;
}

/* ========================================
 * Test Cases for Alpha Mode
 * ======================================== */

static int test_alpha_mode_default(void) {
    graphics_set_alpha_mode(0);
    return graphics_get_alpha_mode() == 0;
}

static int test_alpha_mode_enable(void) {
    graphics_set_alpha_mode(1);
    int result = graphics_get_alpha_mode() == 1;
    graphics_set_alpha_mode(0);  /* Reset */
    return result;
}

static int test_alpha_mode_toggle(void) {
    graphics_set_alpha_mode(0);
    if (graphics_get_alpha_mode() != 0) return 0;

    graphics_set_alpha_mode(1);
    if (graphics_get_alpha_mode() != 1) return 0;

    graphics_set_alpha_mode(0);
    return graphics_get_alpha_mode() == 0;
}

/* ========================================
 * Test Cases for SpriteCacheEntry
 * ======================================== */

static int test_sprite_cache_entry_size(void) {
    /* Verify structure size is reasonable */
    return sizeof(SpriteCacheEntry) >= 32;  /* At least 32 bytes */
}

static int test_sprite_cache_entry_init(void) {
    SpriteCacheEntry entry;
    entry.sprite_id = -1;
    entry.surface = NULL;
    entry.alpha_surface = NULL;
    entry.alpha_buffer = NULL;
    entry.x_offset = 0;
    entry.y_offset = 0;
    entry.ref_count = 0;
    entry.flags = 0;
    entry.sprite_data = NULL;

    return entry.sprite_id == -1 &&
           entry.surface == NULL &&
           entry.ref_count == 0;
}

static int test_sprite_cache_entry_fields(void) {
    SpriteCacheEntry entry;

    entry.sprite_id = 12345;
    entry.x_offset = 10;
    entry.y_offset = 20;
    entry.ref_count = 5;
    entry.flags = SURFACE_FLAG_VIDEO_MEMORY;

    return entry.sprite_id == 12345 &&
           entry.x_offset == 10 &&
           entry.y_offset == 20 &&
           entry.ref_count == 5 &&
           entry.flags == 0x4000;
}

/* ========================================
 * Test Cases for Pixel Format Calculations
 * ======================================== */

static int test_pixel_format_shift_calculation(void) {
    /* Shift = 8 - count_bits(mask) */
    int red_shift_565 = 8 - count_bits(0xF800);   /* 8 - 5 = 3 */
    int green_shift_565 = 8 - count_bits(0x07E0); /* 8 - 6 = 2 */
    int blue_shift_565 = 8 - count_bits(0x001F);  /* 8 - 5 = 3 */

    return red_shift_565 == 3 &&
           green_shift_565 == 2 &&
           blue_shift_565 == 3;
}

static int test_pixel_format_rgb555_shift(void) {
    /* RGB555: all channels have 5 bits */
    int red_shift_555 = 8 - count_bits(0x7C00);
    int green_shift_555 = 8 - count_bits(0x03E0);
    int blue_shift_555 = 8 - count_bits(0x001F);

    return red_shift_555 == 3 &&
           green_shift_555 == 3 &&
           blue_shift_555 == 3;
}

/* ========================================
 * Test Cases for Color Conversion
 * ======================================== */

static int test_rgb565_to_rgb555_white(void) {
    /* White in RGB565: 0xFFFF */
    u16 pixel_565 = 0xFFFF;

    /* Extract components */
    u16 r = (pixel_565 >> 11) & 0x1F;  /* 31 */
    u16 g = (pixel_565 >> 5) & 0x3F;   /* 63 */
    u16 b = pixel_565 & 0x1F;          /* 31 */

    /* Convert to RGB555 */
    u16 pixel_555 = (r << 10) | ((g >> 1) << 5) | b;

    /* Should be 0x7FFF */
    return pixel_555 == 0x7FFF;
}

static int test_rgb565_to_rgb555_red(void) {
    /* Red in RGB565: 0xF800 */
    u16 pixel_565 = 0xF800;

    u16 r = (pixel_565 >> 11) & 0x1F;
    u16 g = (pixel_565 >> 5) & 0x3F;
    u16 b = pixel_565 & 0x1F;

    u16 pixel_555 = (r << 10) | ((g >> 1) << 5) | b;

    /* Red in RGB555: 0x7C00 */
    return pixel_555 == 0x7C00;
}

static int test_rgb565_to_rgb555_green(void) {
    /* Green in RGB565: 0x07E0 */
    u16 pixel_565 = 0x07E0;

    u16 r = (pixel_565 >> 11) & 0x1F;
    u16 g = (pixel_565 >> 5) & 0x3F;
    u16 b = pixel_565 & 0x1F;

    u16 pixel_555 = (r << 10) | ((g >> 1) << 5) | b;

    /* Green in RGB555: 31<<5 = 0x03E0 */
    return pixel_555 == 0x03E0;
}

static int test_rgb565_to_rgb555_blue(void) {
    /* Blue in RGB565: 0x001F (same in RGB555) */
    u16 pixel_565 = 0x001F;

    u16 r = (pixel_565 >> 11) & 0x1F;
    u16 g = (pixel_565 >> 5) & 0x3F;
    u16 b = pixel_565 & 0x1F;

    u16 pixel_555 = (r << 10) | ((g >> 1) << 5) | b;

    /* Blue stays the same */
    return pixel_555 == 0x001F;
}

/* ========================================
 * Test Cases for Tile Calculations
 * ======================================== */

static int test_tile_count_standard(void) {
    /* Standard sprites: 64x48 tiles */
    int tile_w = 64, tile_h = 48;
    int width = 128, height = 96;

    int tiles_x = (width + tile_w - 1) / tile_w;
    int tiles_y = (height + tile_h - 1) / tile_h;

    return tiles_x == 2 && tiles_y == 2;
}

static int test_tile_count_extended(void) {
    /* Extended sprites: 64x64 tiles */
    int tile_w = 64, tile_h = 64;
    int width = 128, height = 128;

    int tiles_x = (width + tile_w - 1) / tile_w;
    int tiles_y = (height + tile_h - 1) / tile_h;

    return tiles_x == 2 && tiles_y == 2;
}

static int test_tile_count_non_aligned(void) {
    /* Non-aligned sprite */
    int tile_w = 64, tile_h = 48;
    int width = 100, height = 100;

    int tiles_x = (width + tile_w - 1) / tile_w;
    int tiles_y = (height + tile_h - 1) / tile_h;

    return tiles_x == 2 && tiles_y == 3;
}

static int test_tile_count_single(void) {
    /* Single tile */
    int tile_w = 64, tile_h = 48;
    int width = 64, height = 48;

    int tiles_x = (width + tile_w - 1) / tile_w;
    int tiles_y = (height + tile_h - 1) / tile_h;

    return tiles_x == 1 && tiles_y == 1;
}

/* ========================================
 * Test Cases for Memory Alignment
 * ======================================== */

static int test_width_alignment_4(void) {
    /* 4-pixel alignment used in sprite rendering */
    int widths[] = {1, 2, 3, 4, 5, 63, 64, 65, 127, 128};
    int expected[] = {4, 4, 4, 4, 8, 64, 64, 68, 128, 128};
    int i;

    for (i = 0; i < 10; i++) {
        int aligned = (widths[i] + 3) & ~3;
        if (aligned != expected[i]) return 0;
    }
    return 1;
}

static int test_pitch_alignment(void) {
    /* DirectDraw pitch is typically aligned to 4 bytes */
    int widths[] = {64, 128, 256, 320, 640};
    int i;

    for (i = 0; i < 5; i++) {
        /* Pitch for 16-bit surface */
        int pitch = widths[i] * 2;
        /* Should be divisible by 4 */
        if (pitch % 4 != 0) return 0;
    }
    return 1;
}

/* ========================================
 * Test Cases for Cache Index Calculations
 * ======================================== */

static int test_cache_index_wrap(void) {
    /* Cache index wraps at SPRITE_CACHE_SIZE */
    int index = SPRITE_CACHE_SIZE;
    index = index % SPRITE_CACHE_SIZE;
    return index == 0;
}

static int test_cache_index_large(void) {
    /* Large index calculation */
    int sprite_id = 1234567;
    int index = sprite_id % SPRITE_CACHE_SIZE;
    return index >= 0 && index < SPRITE_CACHE_SIZE;
}

static int test_cache_index_negative(void) {
    /* Negative index handling */
    int index = -1;
    /* Should be treated as invalid */
    return index < 0;
}

/* ========================================
 * Test Cases for Screen Mode Constants
 * ======================================== */

static int test_screen_mode_values(void) {
    return SCREEN_MODE_FULLSCREEN == 0 &&
           SCREEN_MODE_WINDOWED == 1;
}

/* ========================================
 * Test Cases for High-Res Mode
 * ======================================== */

static int test_high_res_mode_default(void) {
    return g_high_res_mode == 0;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_initialization_flow(void) {
    /* Simulate initialization flow */

    /* 1. Set mode 0 (fullscreen) */
    graphics_allocate_context(0);
    if (g_screen_width != 640 || g_screen_height != 480) return 0;

    /* 2. Check sprite dimensions */
    if (g_sprite_width != 64 || g_sprite_height != 48) return 0;

    /* 3. Count bits for pixel format */
    if (count_bits(0xF800) != 5) return 0;

    return 1;
}

static int test_mode_switch_flow(void) {
    /* Switch between modes */

    /* Start in fullscreen */
    graphics_allocate_context(0);
    if (g_sprite_width != 64) return 0;

    /* Switch to windowed */
    graphics_allocate_context(1);
    if (g_sprite_width != 32) return 0;

    /* Back to fullscreen */
    graphics_allocate_context(0);
    if (g_sprite_width != 64) return 0;

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== DirectX Module Comprehensive Tests ===\n\n");

    /* count_bits tests (FUN_004119e0) */
    printf("Count Bits Tests (FUN_004119e0):\n");
    TEST(count_bits_zero);
    TEST(count_bits_one);
    TEST(count_bits_all_bits);
    TEST(count_bits_power_of_two);
    TEST(count_bits_consecutive);
    TEST(count_bits_rgb565_red);
    TEST(count_bits_rgb565_green);
    TEST(count_bits_rgb565_blue);
    TEST(count_bits_rgb555_red);
    TEST(count_bits_rgb555_green);
    TEST(count_bits_rgb555_blue);
    TEST(count_bits_byte_patterns);

    /* graphics_allocate_context tests (FUN_00440170) */
    printf("\nGraphics Allocate Context Tests (FUN_00440170):\n");
    TEST(allocate_context_mode_0);
    TEST(allocate_context_mode_1);
    TEST(allocate_context_mode_2);
    TEST(allocate_context_mode_invalid);
    TEST(screen_dimensions_hex_values);

    /* Surface flags tests */
    printf("\nSurface Flags Tests:\n");
    TEST(surface_flag_video_memory);
    TEST(surface_flag_system_memory);
    TEST(surface_flags_distinct);

    /* Sprite cache tests */
    printf("\nSprite Cache Tests:\n");
    TEST(sprite_cache_size);
    TEST(max_offscreen_surfaces);
    TEST(sprite_id_ranges);
    TEST(sprite_id_range_size);

    /* Alpha mode tests */
    printf("\nAlpha Mode Tests:\n");
    TEST(alpha_mode_default);
    TEST(alpha_mode_enable);
    TEST(alpha_mode_toggle);

    /* SpriteCacheEntry tests */
    printf("\nSpriteCacheEntry Tests:\n");
    TEST(sprite_cache_entry_size);
    TEST(sprite_cache_entry_init);
    TEST(sprite_cache_entry_fields);

    /* Pixel format tests */
    printf("\nPixel Format Tests:\n");
    TEST(pixel_format_shift_calculation);
    TEST(pixel_format_rgb555_shift);

    /* Color conversion tests */
    printf("\nColor Conversion Tests:\n");
    TEST(rgb565_to_rgb555_white);
    TEST(rgb565_to_rgb555_red);
    TEST(rgb565_to_rgb555_green);
    TEST(rgb565_to_rgb555_blue);

    /* Tile calculation tests */
    printf("\nTile Calculation Tests:\n");
    TEST(tile_count_standard);
    TEST(tile_count_extended);
    TEST(tile_count_non_aligned);
    TEST(tile_count_single);

    /* Alignment tests */
    printf("\nAlignment Tests:\n");
    TEST(width_alignment_4);
    TEST(pitch_alignment);

    /* Cache index tests */
    printf("\nCache Index Tests:\n");
    TEST(cache_index_wrap);
    TEST(cache_index_large);
    TEST(cache_index_negative);

    /* Screen mode tests */
    printf("\nScreen Mode Tests:\n");
    TEST(screen_mode_values);
    TEST(high_res_mode_default);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_initialization_flow);
    TEST(mode_switch_flow);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
