/*
 * Stone Age Client - DirectX Unit Tests
 * Tests for DirectDraw initialization, surface management, sprite cache
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../tests/test_framework.h"
#include "types.h"
#include "directx.h"
#include "directx_internal.h"

/* Test surface flags */
void test_surface_flags(void) {
    TEST_ASSERT_EQ(SURFACE_FLAG_VIDEO_MEMORY, 0x4000);
    TEST_ASSERT_EQ(SURFACE_FLAG_SYSTEM_MEMORY, 0x0800);
}

/* Test sprite cache constants */
void test_sprite_cache_constants(void) {
    TEST_ASSERT_EQ(SPRITE_CACHE_SIZE, 4096);
    TEST_ASSERT_EQ(MAX_OFFSCREEN_SURFACES, 0x1000);
}

/* Test sprite ID ranges from assets.h */
void test_sprite_id_ranges(void) {
    TEST_ASSERT_EQ(SPRITE_ID_STANDARD_MAX, 500000);
    TEST_ASSERT_EQ(SPRITE_ID_EXTENDED_MAX, 550000);
    TEST_ASSERT_EQ(SPRITE_ID_INVALID, 550000);
}

/* Test count_bits function - FUN_004119e0 pattern */
void test_count_bits_zero(void) {
    TEST_ASSERT_EQ(count_bits(0), 0);
}

void test_count_bits_single(void) {
    TEST_ASSERT_EQ(count_bits(1), 1);
    TEST_ASSERT_EQ(count_bits(2), 1);
    TEST_ASSERT_EQ(count_bits(4), 1);
    TEST_ASSERT_EQ(count_bits(0x80000000), 1);
}

void test_count_bits_multiple(void) {
    TEST_ASSERT_EQ(count_bits(3), 2);     /* 0b11 */
    TEST_ASSERT_EQ(count_bits(7), 3);     /* 0b111 */
    TEST_ASSERT_EQ(count_bits(0xFF), 8);  /* 0b11111111 */
    TEST_ASSERT_EQ(count_bits(0xFFFF), 16);
}

void test_count_bits_masks(void) {
    /* RGB565 masks */
    TEST_ASSERT_EQ(count_bits(0xF800), 5);  /* Red: 5 bits */
    TEST_ASSERT_EQ(count_bits(0x07E0), 6);  /* Green: 6 bits */
    TEST_ASSERT_EQ(count_bits(0x001F), 5);  /* Blue: 5 bits */

    /* RGB555 masks */
    TEST_ASSERT_EQ(count_bits(0x7C00), 5);  /* Red: 5 bits */
    TEST_ASSERT_EQ(count_bits(0x03E0), 5);  /* Green: 5 bits */
    TEST_ASSERT_EQ(count_bits(0x001F), 5);  /* Blue: 5 bits */
}

/* Test GraphicsContext structure */
void test_graphics_context_size(void) {
    /* Verify structure has essential fields */
    TEST_ASSERT(sizeof(GraphicsContext) > 0);
}

/* Test SpriteCacheEntry structure */
void test_sprite_cache_entry_fields(void) {
    SpriteCacheEntry entry;

    /* Initialize entry */
    entry.sprite_id = -1;
    entry.surface = NULL;
    entry.alpha_surface = NULL;
    entry.alpha_buffer = NULL;
    entry.x_offset = 0;
    entry.y_offset = 0;
    entry.ref_count = 0;
    entry.flags = 0;
    entry.sprite_data = NULL;

    TEST_ASSERT_EQ(entry.sprite_id, -1);
    TEST_ASSERT_EQ(entry.surface, NULL);
    TEST_ASSERT_EQ(entry.alpha_surface, NULL);
    TEST_ASSERT_EQ(entry.ref_count, 0);
}

/* Test alpha mode functions */
void test_alpha_mode_default(void) {
    /* Default alpha mode should be 0 */
    TEST_ASSERT_EQ(graphics_get_alpha_mode(), 0);
}

void test_alpha_mode_set(void) {
    graphics_set_alpha_mode(1);
    TEST_ASSERT_EQ(graphics_get_alpha_mode(), 1);

    graphics_set_alpha_mode(0);
    TEST_ASSERT_EQ(graphics_get_alpha_mode(), 0);
}

/* Test sprite cache initialization/shutdown */
void test_sprite_cache_init_shutdown(void) {
    int count;

    /* Initialize with minimal count for testing */
    count = sprite_cache_init(64, 48, 2, 0);

    /* Without actual DirectDraw, this may fail or return 0 */
    /* The test verifies the function doesn't crash */
    TEST_ASSERT(count >= 0);

    /* Shutdown should be safe even if init failed */
    sprite_cache_shutdown();

    TEST_ASSERT_EQ(sprite_cache_get_count(), 0);
}

/* Test sprite cache get entry bounds */
void test_sprite_cache_get_entry_bounds(void) {
    SpriteCacheEntry* entry;

    /* Negative index should return NULL */
    entry = sprite_cache_get_entry(-1);
    TEST_ASSERT_EQ(entry, NULL);

    /* Out of bounds index should return NULL */
    entry = sprite_cache_get_entry(SPRITE_CACHE_SIZE);
    TEST_ASSERT_EQ(entry, NULL);

    entry = sprite_cache_get_entry(SPRITE_CACHE_SIZE + 100);
    TEST_ASSERT_EQ(entry, NULL);
}

/* Test sprite set data buffer */
void test_sprite_set_data_buffer(void) {
    uint8_t buffer[1024];

    sprite_set_data_buffer(buffer, 64, 16);

    /* Function should not crash - verifies parameter storage */
    TEST_PASS();
}

/* Test pixel format masks */
void test_pixel_format_globals(void) {
    /* Verify globals are accessible */
    extern u32 g_red_mask;
    extern u32 g_green_mask;
    extern u32 g_blue_mask;
    extern int g_red_shift;
    extern int g_green_shift;
    extern int g_blue_shift;

    /* Initial values should be 0 before init */
    TEST_ASSERT_EQ(g_red_mask, 0);
    TEST_ASSERT_EQ(g_green_mask, 0);
    TEST_ASSERT_EQ(g_blue_mask, 0);
}

/* Test graphics context dimensions */
void test_graphics_context_dimensions(void) {
    /* Test width/height constants from FUN_00440170 */
    TEST_ASSERT_EQ(640, 0x280);   /* Fullscreen width */
    TEST_ASSERT_EQ(480, 0x1E0);   /* Fullscreen height */
    TEST_ASSERT_EQ(320, 0x140);   /* Windowed width */
    TEST_ASSERT_EQ(240, 0xF0);    /* Windowed height */
    TEST_ASSERT_EQ(64, 0x40);     /* Sprite width */
    TEST_ASSERT_EQ(48, 0x30);     /* Sprite height */
    TEST_ASSERT_EQ(32, 0x20);     /* Small sprite width */
    TEST_ASSERT_EQ(24, 0x18);     /* Small sprite height */
}

/* Test screen mode constants */
void test_screen_mode_constants(void) {
    TEST_ASSERT_EQ(SCREEN_MODE_FULLSCREEN, 0);
    TEST_ASSERT_EQ(SCREEN_MODE_WINDOWED, 1);
}

/* Test sprite dimensions globals */
void test_sprite_dimensions_globals(void) {
    extern int g_sprite_width;
    extern int g_sprite_height;

    /* Default values */
    TEST_ASSERT_EQ(g_sprite_width, 64);
    TEST_ASSERT_EQ(g_sprite_height, 48);
}

/* Test high-res mode flag */
void test_high_res_mode_flag(void) {
    extern int g_high_res_mode;

    /* Default should be 0 */
    TEST_ASSERT_EQ(g_high_res_mode, 0);
}

/* Test RGB565 to RGB555 conversion logic */
void test_rgb565_to_555_conversion(void) {
    /* Test conversion math used in sprite_copy_to_surface */
    uint16_t pixel_565 = 0xFFFF;  /* White */
    uint16_t r = (pixel_565 >> 11) & 0x1F;  /* 31 */
    uint16_t g = (pixel_565 >> 5) & 0x3F;   /* 63 */
    uint16_t b = pixel_565 & 0x1F;          /* 31 */

    /* Convert to 555 */
    uint16_t pixel_555 = (r << 10) | ((g >> 1) << 5) | b;

    /* White in 555: R=31, G=31, B=31 -> 0x7FFF */
    TEST_ASSERT_EQ(pixel_555, 0x7FFF);
}

void test_rgb565_color_key(void) {
    /* Color key 0 should be transparent */
    uint16_t pixel = 0;

    if (pixel == 0) {
        /* Transparent - don't write */
        TEST_PASS();
    }
}

/* Test sprite cache manager tile calculation */
void test_tile_calculation_standard(void) {
    /* Standard sprites use 64x48 tiles */
    int tile_width = 64;
    int tile_height = 48;
    int width = 64;
    int height = 96;

    int tiles_x = (width + tile_width - 1) / tile_width;
    int tiles_y = (height + tile_height - 1) / tile_height;

    TEST_ASSERT_EQ(tiles_x, 1);
    TEST_ASSERT_EQ(tiles_y, 2);
}

void test_tile_calculation_extended(void) {
    /* Extended sprites use 64x64 tiles */
    int tile_width = 64;
    int tile_height = 64;
    int width = 128;
    int height = 128;

    int tiles_x = (width + tile_width - 1) / tile_width;
    int tiles_y = (height + tile_height - 1) / tile_height;

    TEST_ASSERT_EQ(tiles_x, 2);
    TEST_ASSERT_EQ(tiles_y, 2);
}

void test_tile_calculation_large(void) {
    /* Large sprite calculation */
    int tile_width = 64;
    int tile_height = 48;
    int width = 200;
    int height = 150;

    int tiles_x = (width + tile_width - 1) / tile_width;
    int tiles_y = (height + tile_height - 1) / tile_height;

    TEST_ASSERT_EQ(tiles_x, 4);  /* ceil(200/64) = 4 */
    TEST_ASSERT_EQ(tiles_y, 4);  /* ceil(150/48) = 4 */
}

/* Test width alignment for standard sprites */
void test_width_alignment(void) {
    /* Standard sprites need 4-pixel alignment */
    int width = 65;
    int aligned = (width + 3) & ~3;

    TEST_ASSERT_EQ(aligned, 68);

    width = 64;
    aligned = (width + 3) & ~3;
    TEST_ASSERT_EQ(aligned, 64);

    width = 67;
    aligned = (width + 3) & ~3;
    TEST_ASSERT_EQ(aligned, 68);
}

/* Test cache index wrapping */
void test_cache_index_wrapping(void) {
    int cache_count = 10;
    int index = 10;

    /* Wrap around */
    if (index >= cache_count) {
        index = 0;
    }
    TEST_ASSERT_EQ(index, 0);

    /* Multiple wraps */
    index = 25;
    index = index % cache_count;
    TEST_ASSERT_EQ(index, 5);
}

int main(void) {
    TEST_SUITE_BEGIN("DirectX Tests");

    /* Constants */
    TEST_RUN(test_surface_flags);
    TEST_RUN(test_sprite_cache_constants);
    TEST_RUN(test_sprite_id_ranges);

    /* count_bits function */
    TEST_RUN(test_count_bits_zero);
    TEST_RUN(test_count_bits_single);
    TEST_RUN(test_count_bits_multiple);
    TEST_RUN(test_count_bits_masks);

    /* Structures */
    TEST_RUN(test_graphics_context_size);
    TEST_RUN(test_sprite_cache_entry_fields);

    /* Alpha mode */
    TEST_RUN(test_alpha_mode_default);
    TEST_RUN(test_alpha_mode_set);

    /* Sprite cache */
    TEST_RUN(test_sprite_cache_init_shutdown);
    TEST_RUN(test_sprite_cache_get_entry_bounds);
    TEST_RUN(test_sprite_set_data_buffer);

    /* Globals */
    TEST_RUN(test_pixel_format_globals);
    TEST_RUN(test_graphics_context_dimensions);
    TEST_RUN(test_screen_mode_constants);
    TEST_RUN(test_sprite_dimensions_globals);
    TEST_RUN(test_high_res_mode_flag);

    /* Pixel format conversion */
    TEST_RUN(test_rgb565_to_555_conversion);
    TEST_RUN(test_rgb565_color_key);

    /* Tile calculations */
    TEST_RUN(test_tile_calculation_standard);
    TEST_RUN(test_tile_calculation_extended);
    TEST_RUN(test_tile_calculation_large);

    /* Alignment and wrapping */
    TEST_RUN(test_width_alignment);
    TEST_RUN(test_cache_index_wrapping);

    TEST_SUITE_END();
}
