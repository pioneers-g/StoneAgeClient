/*
 * Stone Age Client - Unit Tests for Sprite Rendering System
 * Test file: test_sprite.c
 *
 * Tests for sprite rendering, blending, RLE decompression
 * Based on reverse engineering of sa_9061.exe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/sprite.h"

/* ========================================
 * Test Constants
 * ======================================== */

#define TRANSPARENT_COLOR_565   0xF81F
#define TRANSPARENT_COLOR_555   0x7C1F

/* ========================================
 * Test Cases for Color Conversion
 * ======================================== */

/*
 * Test 1: RGB to 565 color conversion
 */
static void test_color_565(void) {
    TEST_BEGIN("Color RGB to 565");

    /* Test pure red (255, 0, 0) */
    u16 red = sprite_color_565(255, 0, 0);
    TEST_ASSERT((red >> 11) == 0x1F, "Red channel should be 0x1F");
    TEST_ASSERT((red & 0x07E0) == 0, "Green channel should be 0");
    TEST_ASSERT((red & 0x001F) == 0, "Blue channel should be 0");

    /* Test pure green (0, 255, 0) */
    u16 green = sprite_color_565(0, 255, 0);
    TEST_ASSERT((green >> 11) == 0, "Red channel should be 0");
    TEST_ASSERT(((green >> 5) & 0x3F) == 0x3F, "Green channel should be 0x3F");
    TEST_ASSERT((green & 0x001F) == 0, "Blue channel should be 0");

    /* Test pure blue (0, 0, 255) */
    u16 blue = sprite_color_565(0, 0, 255);
    TEST_ASSERT((blue >> 11) == 0, "Red channel should be 0");
    TEST_ASSERT((blue & 0x07E0) == 0, "Green channel should be 0");
    TEST_ASSERT((blue & 0x001F) == 0x1F, "Blue channel should be 0x1F");

    /* Test white (255, 255, 255) */
    u16 white = sprite_color_565(255, 255, 255);
    TEST_ASSERT(white == 0xFFFF, "White should be 0xFFFF");

    /* Test black (0, 0, 0) */
    u16 black = sprite_color_565(0, 0, 0);
    TEST_ASSERT(black == 0, "Black should be 0");

    TEST_END();
}

/*
 * Test 2: RGB to 555 color conversion
 */
static void test_color_555(void) {
    TEST_BEGIN("Color RGB to 555");

    /* Test pure red (255, 0, 0) */
    u16 red = sprite_color_555(255, 0, 0);
    TEST_ASSERT((red >> 10) == 0x1F, "Red channel should be 0x1F");

    /* Test pure green (0, 255, 0) */
    u16 green = sprite_color_555(0, 255, 0);
    TEST_ASSERT(((green >> 5) & 0x1F) == 0x1F, "Green channel should be 0x1F");

    /* Test pure blue (0, 0, 255) */
    u16 blue = sprite_color_555(0, 0, 255);
    TEST_ASSERT((blue & 0x1F) == 0x1F, "Blue channel should be 0x1F");

    /* Test white (255, 255, 255) */
    u16 white = sprite_color_555(255, 255, 255);
    TEST_ASSERT(white == 0x7FFF, "White should be 0x7FFF in 555 format");

    TEST_END();
}

/*
 * Test 3: Color split 565
 */
static void test_color_split_565(void) {
    TEST_BEGIN("Color split 565");

    u8 r, g, b;

    /* Test known color */
    sprite_color_split_565(0xF800, &r, &g, &b);  /* Red */
    TEST_ASSERT(r == 0xF8, "Red should be 0xF8");
    TEST_ASSERT(g == 0, "Green should be 0");
    TEST_ASSERT(b == 0, "Blue should be 0");

    sprite_color_split_565(0x07E0, &r, &g, &b);  /* Green */
    TEST_ASSERT(r == 0, "Red should be 0");
    TEST_ASSERT(g == 0xFC, "Green should be 0xFC");
    TEST_ASSERT(b == 0, "Blue should be 0");

    sprite_color_split_565(0x001F, &r, &g, &b);  /* Blue */
    TEST_ASSERT(r == 0, "Red should be 0");
    TEST_ASSERT(g == 0, "Green should be 0");
    TEST_ASSERT(b == 0xF8, "Blue should be 0xF8");

    TEST_END();
}

/*
 * Test 4: Color split 555
 */
static void test_color_split_555(void) {
    TEST_BEGIN("Color split 555");

    u8 r, g, b;

    sprite_color_split_555(0x7C00, &r, &g, &b);  /* Red in 555 */
    TEST_ASSERT(r == 0xF8, "Red should be 0xF8");
    TEST_ASSERT(g == 0, "Green should be 0");
    TEST_ASSERT(b == 0, "Blue should be 0");

    sprite_color_split_555(0x03E0, &r, &g, &b);  /* Green in 555 */
    TEST_ASSERT(r == 0, "Red should be 0");
    TEST_ASSERT(g == 0xF8, "Green should be 0xF8");
    TEST_ASSERT(b == 0, "Blue should be 0");

    TEST_END();
}

/* ========================================
 * Test Cases for Alpha Blending
 * ======================================== */

/*
 * Test 5: Basic 565 alpha blending
 */
static void test_blend_565_basic(void) {
    TEST_BEGIN("Blend 565 basic");

    u16 dest[4] = {0x0000, 0xF800, 0x07E0, 0x001F};  /* Black, Red, Green, Blue */
    u16 src[4] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};   /* White */

    /* Blend with 50% alpha (128) */
    sprite_blend_565(dest, src, 4, 128);

    /* All colors should be modified (except transparent if any) */
    /* Result should be between source and destination */
    TEST_ASSERT(dest[0] != 0x0000, "Black should be blended");
    TEST_ASSERT(dest[1] != 0xF800, "Red should be blended");

    TEST_END();
}

/*
 * Test 6: 565 blend with transparent
 */
static void test_blend_565_transparent(void) {
    TEST_BEGIN("Blend 565 transparent");

    u16 dest[2] = {0xF800, 0x07E0};  /* Red, Green */
    u16 src[2] = {TRANSPARENT_COLOR_565, 0xFFFF};  /* Transparent, White */

    sprite_blend_565(dest, src, 2, 128);

    /* First pixel should remain unchanged (transparent source) */
    TEST_ASSERT(dest[0] == 0xF800, "Transparent pixel should not modify dest");

    /* Second pixel should be modified */
    TEST_ASSERT(dest[1] != 0x07E0, "Non-transparent pixel should modify dest");

    TEST_END();
}

/*
 * Test 7: Basic 555 alpha blending
 */
static void test_blend_555_basic(void) {
    TEST_BEGIN("Blend 555 basic");

    u16 dest[2] = {0x0000, 0x7C00};  /* Black, Red in 555 */
    u16 src[2] = {0x7FFF, 0x7FFF};   /* White in 555 */

    sprite_blend_555(dest, src, 2, 128);

    TEST_ASSERT(dest[0] != 0x0000, "Black should be blended");
    TEST_ASSERT(dest[1] != 0x7C00, "Red should be blended");

    TEST_END();
}

/*
 * Test 8: 555 blend with transparent
 */
static void test_blend_555_transparent(void) {
    TEST_BEGIN("Blend 555 transparent");

    u16 dest[2] = {0x7C00, 0x03E0};  /* Red, Green in 555 */
    u16 src[2] = {TRANSPARENT_COLOR_555, 0x7FFF};

    sprite_blend_555(dest, src, 2, 128);

    TEST_ASSERT(dest[0] == 0x7C00, "Transparent pixel should not modify dest");
    TEST_ASSERT(dest[1] != 0x03E0, "Non-transparent pixel should modify dest");

    TEST_END();
}

/*
 * Test 9: Additive blending
 */
static void test_blend_additive(void) {
    TEST_BEGIN("Blend additive");

    u16 dest[2] = {0x0000, 0xF800};  /* Black, Red */
    u16 src[2] = {0x07E0, 0x07E0};   /* Green */

    sprite_blend_additive(dest, src, 2, 128);

    /* First pixel should become greenish */
    TEST_ASSERT(dest[0] != 0x0000, "Black + Green should be greenish");

    /* Second pixel should become yellowish (Red + Green) */
    TEST_ASSERT(dest[1] != 0xF800, "Red + Green should be yellowish");

    TEST_END();
}

/*
 * Test 10: Additive blend clamping
 */
static void test_blend_additive_clamp(void) {
    TEST_BEGIN("Blend additive clamp");

    u16 dest[2] = {0xFFFF, 0xFFFF};  /* Both white */
    u16 src[2] = {0xFFFF, 0xFFFF};

    sprite_blend_additive(dest, src, 2, 255);

    /* Should clamp to max value */
    TEST_ASSERT(dest[0] == 0xFFFF, "White + White should clamp to white");
    TEST_ASSERT(dest[1] == 0xFFFF, "White + White should clamp to white");

    TEST_END();
}

/* ========================================
 * Test Cases for RLE Decompression
 * ======================================== */

/*
 * Test 11: RLE 8-bit literal
 */
static void test_rle_8bit_literal(void) {
    TEST_BEGIN("RLE 8-bit literal");

    /* Literal bytes: count < 0x80, followed by raw data */
    u8 src[] = {0x04, 0x11, 0x22, 0x33, 0x44};  /* 4 literal bytes */
    u8 dst[16] = {0};

    int result = sprite_decode_rle_8bit(src, sizeof(src), dst, sizeof(dst), 4, 1);

    TEST_ASSERT(result > 0, "RLE decode should succeed");
    TEST_ASSERT(dst[0] == 0x11, "First byte should be 0x11");
    TEST_ASSERT(dst[1] == 0x22, "Second byte should be 0x22");
    TEST_ASSERT(dst[2] == 0x33, "Third byte should be 0x33");
    TEST_ASSERT(dst[3] == 0x44, "Fourth byte should be 0x44");

    TEST_END();
}

/*
 * Test 12: RLE 8-bit run
 */
static void test_rle_8bit_run(void) {
    TEST_BEGIN("RLE 8-bit run");

    /* Run-length: count | 0x80, followed by byte to repeat */
    u8 src[] = {0x85, 0xAA};  /* Repeat 0xAA 5 times */
    u8 dst[16] = {0};

    int result = sprite_decode_rle_8bit(src, sizeof(src), dst, sizeof(dst), 5, 1);

    TEST_ASSERT(result > 0, "RLE decode should succeed");
    TEST_ASSERT(dst[0] == 0xAA, "First byte should be 0xAA");
    TEST_ASSERT(dst[1] == 0xAA, "Second byte should be 0xAA");
    TEST_ASSERT(dst[2] == 0xAA, "Third byte should be 0xAA");
    TEST_ASSERT(dst[3] == 0xAA, "Fourth byte should be 0xAA");
    TEST_ASSERT(dst[4] == 0xAA, "Fifth byte should be 0xAA");

    TEST_END();
}

/*
 * Test 13: RLE 8-bit mixed
 */
static void test_rle_8bit_mixed(void) {
    TEST_BEGIN("RLE 8-bit mixed");

    /* Mix of literal and run-length */
    u8 src[] = {0x02, 0x11, 0x22, 0x83, 0x33, 0x01, 0x44};  /* 2 literal, 3x0x33, 1 literal */
    u8 dst[16] = {0};

    int result = sprite_decode_rle_8bit(src, sizeof(src), dst, sizeof(dst), 6, 1);

    TEST_ASSERT(result > 0, "RLE decode should succeed");
    TEST_ASSERT(dst[0] == 0x11, "First byte should be 0x11");
    TEST_ASSERT(dst[1] == 0x22, "Second byte should be 0x22");
    TEST_ASSERT(dst[2] == 0x33, "Third byte should be 0x33");
    TEST_ASSERT(dst[3] == 0x33, "Fourth byte should be 0x33");
    TEST_ASSERT(dst[4] == 0x33, "Fifth byte should be 0x33");
    TEST_ASSERT(dst[5] == 0x44, "Sixth byte should be 0x44");

    TEST_END();
}

/*
 * Test 14: RLE 16-bit literal
 */
static void test_rle_16bit_literal(void) {
    TEST_BEGIN("RLE 16-bit literal");

    u8 src[] = {0x02, 0x11, 0x22, 0x33, 0x44};  /* 2 pixels (4 bytes) */
    u16 dst[16] = {0};

    int result = sprite_decode_rle_16bit(src, sizeof(src), dst, sizeof(dst) * 2, 2, 1);

    TEST_ASSERT(result > 0, "RLE 16-bit decode should succeed");
    TEST_ASSERT(dst[0] == 0x2211, "First pixel should be 0x2211");
    TEST_ASSERT(dst[1] == 0x4433, "Second pixel should be 0x4433");

    TEST_END();
}

/* ========================================
 * Test Cases for Sprite ID Ranges
 * ======================================== */

/*
 * Test 15: Sprite ID constants
 */
static void test_sprite_id_constants(void) {
    TEST_BEGIN("Sprite ID constants");

    TEST_ASSERT(SPRITE_ID_HIGHRES_BASE == 500000, "High-res base should be 500000");
    TEST_ASSERT(SPRITE_ID_HIGHRES_MAX == 549999, "High-res max should be 549999");
    TEST_ASSERT(SPRITE_QUEUE_MAX == 4096, "Queue max should be 4096");

    TEST_END();
}

/*
 * Test 16: Sprite ID range check
 */
static void test_sprite_id_range(void) {
    TEST_BEGIN("Sprite ID range check");

    /* Normal sprites: 0-499999 */
    u32 normal_id = 100000;
    TEST_ASSERT(normal_id < SPRITE_ID_HIGHRES_BASE, "Normal sprite ID should be < 500000");

    /* High-res sprites: 500000-549999 */
    u32 highres_id = 520000;
    TEST_ASSERT(highres_id >= SPRITE_ID_HIGHRES_BASE, "High-res sprite ID should be >= 500000");
    TEST_ASSERT(highres_id <= SPRITE_ID_HIGHRES_MAX, "High-res sprite ID should be <= 549999");

    /* Invalid sprite: >= 550000 */
    u32 invalid_id = 550000;
    TEST_ASSERT(invalid_id > SPRITE_ID_HIGHRES_MAX, "Invalid sprite ID should be > 549999");

    TEST_END();
}

/* ========================================
 * Test Cases for Render Queue
 * ======================================== */

/*
 * Test 17: Render mode constants
 */
static void test_render_mode_constants(void) {
    TEST_BEGIN("Render mode constants");

    TEST_ASSERT(RENDER_MODE_NORMAL == 0, "Normal mode should be 0");
    TEST_ASSERT(RENDER_MODE_ADDITIVE == 1, "Additive mode should be 1");
    TEST_ASSERT(RENDER_MODE_SUBTRACTIVE == 2, "Subtractive mode should be 2");
    TEST_ASSERT(RENDER_MODE_CUSTOM == 3, "Custom mode should be 3");

    TEST_END();
}

/*
 * Test 18: Sprite type constants
 */
static void test_sprite_type_constants(void) {
    TEST_BEGIN("Sprite type constants");

    TEST_ASSERT(SPRITE_TYPE_UI == 0x68, "UI type should be 0x68");
    TEST_ASSERT(SPRITE_TYPE_ICON == 0x69, "Icon type should be 0x69");
    TEST_ASSERT(SPRITE_TYPE_BUTTON == 0x6d, "Button type should be 0x6d");
    TEST_ASSERT(SPRITE_TYPE_EFFECT == 0x6e, "Effect type should be 0x6e");

    TEST_END();
}

/*
 * Test 19: Pixel format constants
 */
static void test_pixel_format_constants(void) {
    TEST_BEGIN("Pixel format constants");

    TEST_ASSERT(PIXEL_FORMAT_555 == 0, "555 format should be 0");
    TEST_ASSERT(PIXEL_FORMAT_565 == 2, "565 format should be 2");

    TEST_END();
}

/*
 * Test 20: Palette group constants
 */
static void test_palette_group_constants(void) {
    TEST_BEGIN("Palette group constants");

    TEST_ASSERT(PALETTE_GROUP_DEFAULT == 0, "Default palette should be 0");
    TEST_ASSERT(PALETTE_GROUP_1 == 1, "Palette group 1 should be 1");
    TEST_ASSERT(PALETTE_GROUP_2 == 2, "Palette group 2 should be 2");
    TEST_ASSERT(PALETTE_GROUP_3 == 3, "Palette group 3 should be 3");
    TEST_ASSERT(PALETTE_GROUP_4 == 4, "Palette group 4 should be 4");

    TEST_END();
}

/* ========================================
 * Test Cases for Sprite System Init
 * ======================================== */

/*
 * Test 21: Sprite system init
 */
static void test_sprite_init(void) {
    TEST_BEGIN("Sprite system init");

    int result = sprite_init();
    TEST_ASSERT(result == 1, "Sprite init should return 1");
    TEST_ASSERT(g_sprite_ctx.pixel_format == PIXEL_FORMAT_565, "Default pixel format should be 565");

    sprite_shutdown();

    TEST_END();
}

/*
 * Test 22: Sprite system shutdown
 */
static void test_sprite_shutdown(void) {
    TEST_BEGIN("Sprite system shutdown");

    sprite_init();
    g_sprite_ctx.dest_width = 800;
    g_sprite_ctx.dest_height = 600;

    sprite_shutdown();

    TEST_ASSERT(g_sprite_ctx.dest_width == 0, "Width should be 0 after shutdown");
    TEST_ASSERT(g_sprite_ctx.dest_height == 0, "Height should be 0 after shutdown");

    TEST_END();
}

/* ========================================
 * Test Cases for Animation System
 * ======================================== */

/*
 * Test 23: Animation initialization
 */
static void test_animation_init(void) {
    TEST_BEGIN("Animation init");

    SpriteAnimation anim;
    memset(&anim, 0xFF, sizeof(anim));  /* Fill with garbage */

    sprite_animation_init(&anim, 1000, 10);

    TEST_ASSERT(anim.sprite_id == 1000, "Sprite ID should be 1000");
    TEST_ASSERT(anim.frame_count == 10, "Frame count should be 10");
    TEST_ASSERT(anim.current_frame == 0, "Current frame should be 0");
    TEST_ASSERT(anim.is_playing == 0, "Animation should not be playing initially");

    TEST_END();
}

/*
 * Test 24: Animation play/stop
 */
static void test_animation_play_stop(void) {
    TEST_BEGIN("Animation play/stop");

    SpriteAnimation anim;
    sprite_animation_init(&anim, 1000, 10);

    sprite_animation_play(&anim);
    TEST_ASSERT(anim.is_playing == 1, "Animation should be playing");

    sprite_animation_stop(&anim);
    TEST_ASSERT(anim.is_playing == 0, "Animation should not be playing");

    TEST_END();
}

/*
 * Test 25: Animation update
 */
static void test_animation_update(void) {
    TEST_BEGIN("Animation update");

    SpriteAnimation anim;
    sprite_animation_init(&anim, 1000, 4);
    anim.animation_speed = 100;  /* 100ms per frame */

    sprite_animation_play(&anim);

    /* Update with less than frame duration */
    sprite_animation_update(&anim, 50);
    TEST_ASSERT(anim.current_frame == 0, "Frame should not advance yet");

    /* Update with full frame duration */
    sprite_animation_update(&anim, 100);
    TEST_ASSERT(anim.current_frame == 1, "Frame should advance to 1");

    /* Update multiple frames */
    sprite_animation_update(&anim, 250);
    TEST_ASSERT(anim.current_frame == 3, "Frame should advance to 3");

    TEST_END();
}

/*
 * Test 26: Animation loop
 */
static void test_animation_loop(void) {
    TEST_BEGIN("Animation loop");

    SpriteAnimation anim;
    sprite_animation_init(&anim, 1000, 3);
    anim.animation_speed = 100;
    anim.loop = 1;

    sprite_animation_play(&anim);

    /* Advance past last frame */
    sprite_animation_update(&anim, 400);

    TEST_ASSERT(anim.current_frame == 0, "Frame should loop back to 0");
    TEST_ASSERT(anim.is_playing == 1, "Animation should still be playing");

    TEST_END();
}

/*
 * Test 27: Animation no loop
 */
static void test_animation_no_loop(void) {
    TEST_BEGIN("Animation no loop");

    SpriteAnimation anim;
    sprite_animation_init(&anim, 1000, 3);
    anim.animation_speed = 100;
    anim.loop = 0;

    sprite_animation_play(&anim);

    /* Advance past last frame */
    sprite_animation_update(&anim, 400);

    TEST_ASSERT(anim.current_frame == 2, "Frame should stay at last frame");
    TEST_ASSERT(anim.is_playing == 0, "Animation should stop at end");

    TEST_END();
}

/*
 * Test 28: Animation get frame
 */
static void test_animation_get_frame(void) {
    TEST_BEGIN("Animation get frame");

    SpriteAnimation anim;
    sprite_animation_init(&anim, 1000, 10);
    anim.current_frame = 5;

    u32 frame = sprite_animation_get_frame(&anim);
    TEST_ASSERT(frame == 5, "Get frame should return current frame");

    TEST_END();
}

/*
 * Test 29: Null animation handling
 */
static void test_animation_null(void) {
    TEST_BEGIN("Animation null handling");

    /* These should not crash */
    sprite_animation_init(NULL, 1000, 10);
    sprite_animation_play(NULL);
    sprite_animation_stop(NULL);
    sprite_animation_update(NULL, 100);
    sprite_animation_get_frame(NULL);

    TEST_ASSERT(1, "Null animation operations should not crash");

    TEST_END();
}

/*
 * Test 30: Render flags
 */
static void test_render_flags(void) {
    TEST_BEGIN("Render flags");

    TEST_ASSERT(RENDER_FLAG_NONE == 0x00, "None flag should be 0x00");
    TEST_ASSERT(RENDER_FLAG_ALPHA == 0x10, "Alpha flag should be 0x10");
    TEST_ASSERT(RENDER_FLAG_TRANSPARENT == 0x11, "Transparent flag should be 0x11");
    TEST_ASSERT(RENDER_FLAG_ADDITIVE == 0x12, "Additive flag should be 0x12");

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(sprite) {
    /* Color conversion tests */
    test_color_565();
    test_color_555();
    test_color_split_565();
    test_color_split_555();

    /* Alpha blending tests */
    test_blend_565_basic();
    test_blend_565_transparent();
    test_blend_555_basic();
    test_blend_555_transparent();
    test_blend_additive();
    test_blend_additive_clamp();

    /* RLE decompression tests */
    test_rle_8bit_literal();
    test_rle_8bit_run();
    test_rle_8bit_mixed();
    test_rle_16bit_literal();

    /* Sprite ID tests */
    test_sprite_id_constants();
    test_sprite_id_range();

    /* Render queue tests */
    test_render_mode_constants();
    test_sprite_type_constants();
    test_pixel_format_constants();
    test_palette_group_constants();

    /* System tests */
    test_sprite_init();
    test_sprite_shutdown();

    /* Animation tests */
    test_animation_init();
    test_animation_play_stop();
    test_animation_update();
    test_animation_loop();
    test_animation_no_loop();
    test_animation_get_frame();
    test_animation_null();

    /* Render flags test */
    test_render_flags();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - Sprite Rendering Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(sprite);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
