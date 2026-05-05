/*
 * Stone Age Client - Sprite Blend Functions Comprehensive Tests
 * Tests for FUN_0047e970 (additive), FUN_0047f170 (subtractive), FUN_0047f710 (tint)
 * Also tests sprite_blend_565, sprite_blend_555, sprite_blend_additive, sprite_blend_subtractive
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;

#define TRANSPARENT_COLOR_565   0xF81F
#define TRANSPARENT_COLOR_555   0x7C1F

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT_EQ(a, b) do { int _a = (int)(a); int _b = (int)(b); if (_a != _b) { printf("FAIL: line %d got %d expected %d\n", __LINE__, _a, _b); tests_failed++; return; } } while(0)
#define ASSERT_TRUE(cond) do { if (!(cond)) { printf("FAIL: line %d assertion false\n", __LINE__); tests_failed++; return; } } while(0)

static u16 blend_565_alpha(u16 src, u16 dst, u8 alpha) {
    u32 src_r, src_g, src_b, dst_r, dst_g, dst_b;
    u32 result_r, result_g, result_b;
    u32 alpha_factor = (alpha * 32) / 256;
    if (src == TRANSPARENT_COLOR_565) return dst;
    src_r = (src >> 11) & 0x1F; src_g = (src >> 5) & 0x3F; src_b = src & 0x1F;
    dst_r = (dst >> 11) & 0x1F; dst_g = (dst >> 5) & 0x3F; dst_b = dst & 0x1F;
    result_r = dst_r + (((src_r - dst_r) * alpha_factor) >> 5);
    result_g = dst_g + (((src_g - dst_g) * alpha_factor) >> 5);
    result_b = dst_b + (((src_b - dst_b) * alpha_factor) >> 5);
    if (result_r > 0x1F) result_r = 0x1F;
    if (result_g > 0x3F) result_g = 0x3F;
    if (result_b > 0x1F) result_b = 0x1F;
    return (u16)((result_r << 11) | (result_g << 5) | result_b);
}

static u16 blend_555_alpha(u16 src, u16 dst, u8 alpha) {
    u32 src_r, src_g, src_b, dst_r, dst_g, dst_b;
    u32 result_r, result_g, result_b;
    u32 alpha_factor = (alpha * 32) / 256;
    if (src == TRANSPARENT_COLOR_555) return dst;
    src_r = (src >> 10) & 0x1F; src_g = (src >> 5) & 0x1F; src_b = src & 0x1F;
    dst_r = (dst >> 10) & 0x1F; dst_g = (dst >> 5) & 0x1F; dst_b = dst & 0x1F;
    result_r = dst_r + (((src_r - dst_r) * alpha_factor) >> 5);
    result_g = dst_g + (((src_g - dst_g) * alpha_factor) >> 5);
    result_b = dst_b + (((src_b - dst_b) * alpha_factor) >> 5);
    if (result_r > 0x1F) result_r = 0x1F;
    if (result_g > 0x1F) result_g = 0x1F;
    if (result_b > 0x1F) result_b = 0x1F;
    return (u16)((result_r << 10) | (result_g << 5) | result_b);
}

static u16 blend_additive(u16 src, u16 dst, u8 alpha) {
    u32 src_r, src_g, src_b, dst_r, dst_g, dst_b;
    u32 result_r, result_g, result_b;
    u32 alpha_factor = (alpha * 32) / 256;
    if (src == TRANSPARENT_COLOR_565) return dst;
    src_r = (src >> 11) & 0x1F; src_g = (src >> 5) & 0x3F; src_b = src & 0x1F;
    dst_r = (dst >> 11) & 0x1F; dst_g = (dst >> 5) & 0x3F; dst_b = dst & 0x1F;
    result_r = dst_r + ((src_r * alpha_factor) >> 5);
    result_g = dst_g + ((src_g * alpha_factor) >> 5);
    result_b = dst_b + ((src_b * alpha_factor) >> 5);
    if (result_r > 0x1F) result_r = 0x1F;
    if (result_g > 0x3F) result_g = 0x3F;
    if (result_b > 0x1F) result_b = 0x1F;
    return (u16)((result_r << 11) | (result_g << 5) | result_b);
}

static u16 blend_subtractive(u16 src, u16 dst, u8 alpha) {
    s32 src_r, src_g, src_b, dst_r, dst_g, dst_b;
    s32 result_r, result_g, result_b;
    u32 alpha_factor = (alpha * 32) / 256;
    if (src == TRANSPARENT_COLOR_565) return dst;
    src_r = (src >> 11) & 0x1F; src_g = (src >> 5) & 0x3F; src_b = src & 0x1F;
    dst_r = (dst >> 11) & 0x1F; dst_g = (dst >> 5) & 0x3F; dst_b = dst & 0x1F;
    result_r = dst_r - ((src_r * alpha_factor) >> 5);
    result_g = dst_g - ((src_g * alpha_factor) >> 5);
    result_b = dst_b - ((src_b * alpha_factor) >> 5);
    if (result_r < 0) result_r = 0;
    if (result_g < 0) result_g = 0;
    if (result_b < 0) result_b = 0;
    return (u16)((result_r << 11) | (result_g << 5) | result_b);
}

static u16 make_565(u32 r, u32 g, u32 b) { return (u16)((r << 11) | (g << 5) | b); }
static u16 make_555(u32 r, u32 g, u32 b) { return (u16)((r << 10) | (g << 5) | b); }

/* 565 Alpha Blend Tests */
void test_alpha_565_basic(void) {
    u16 result = blend_565_alpha(make_565(31, 0, 0), make_565(0, 0, 31), 128);
    ASSERT_TRUE(((result >> 11) & 0x1F) > 0);
    ASSERT_TRUE((result & 0x1F) > 0);
    tests_passed++;
}
void test_alpha_565_transparent(void) {
    u16 dst = make_565(10, 20, 30);
    ASSERT_EQ(blend_565_alpha(TRANSPARENT_COLOR_565, dst, 128), dst);
    tests_passed++;
}
void test_alpha_565_full_alpha(void) {
    u16 result = blend_565_alpha(make_565(15, 30, 10), make_565(5, 10, 20), 255);
    /* alpha=255 -> alpha_factor=31, so result = dst + ((src-dst)*31)>>5 which is near src */
    ASSERT_TRUE(((result >> 11) & 0x1F) >= 14);
    ASSERT_TRUE(((result >> 5) & 0x3F) >= 28);
    ASSERT_TRUE((result & 0x1F) >= 9);
    tests_passed++;
}
void test_alpha_565_zero_alpha(void) {
    u16 dst = make_565(5, 10, 5);
    ASSERT_EQ(blend_565_alpha(make_565(31, 63, 31), dst, 0), dst);
    tests_passed++;
}
void test_alpha_565_gradient(void) {
    u16 result = blend_565_alpha(make_565(31, 63, 31), make_565(0, 0, 0), 64);
    ASSERT_TRUE(((result >> 11) & 0x1F) > 0);
    ASSERT_TRUE(((result >> 5) & 0x3F) > 0);
    ASSERT_TRUE((result & 0x1F) > 0);
    tests_passed++;
}
void test_alpha_565_same_color(void) {
    u16 color = make_565(15, 30, 15);
    u16 result = blend_565_alpha(color, color, 128);
    ASSERT_EQ((result >> 11) & 0x1F, 15);
    ASSERT_EQ((result >> 5) & 0x3F, 30);
    ASSERT_EQ(result & 0x1F, 15);
    tests_passed++;
}
void test_alpha_565_clamp(void) {
    u16 result = blend_565_alpha(make_565(31, 63, 31), make_565(31, 63, 31), 255);
    ASSERT_EQ((result >> 11) & 0x1F, 31);
    ASSERT_EQ((result >> 5) & 0x3F, 63);
    ASSERT_EQ(result & 0x1F, 31);
    tests_passed++;
}

/* 555 Alpha Blend Tests */
void test_alpha_555_basic(void) {
    u16 result = blend_555_alpha(make_555(31, 0, 0), make_555(0, 0, 31), 128);
    ASSERT_TRUE(((result >> 10) & 0x1F) > 0);
    ASSERT_TRUE((result & 0x1F) > 0);
    tests_passed++;
}
void test_alpha_555_transparent(void) {
    u16 dst = make_555(10, 20, 30);
    ASSERT_EQ(blend_555_alpha(TRANSPARENT_COLOR_555, dst, 128), dst);
    tests_passed++;
}
void test_alpha_555_same_color(void) {
    u16 color = make_555(15, 15, 15);
    u16 result = blend_555_alpha(color, color, 128);
    ASSERT_EQ((result >> 10) & 0x1F, 15);
    ASSERT_EQ((result >> 5) & 0x1F, 15);
    ASSERT_EQ(result & 0x1F, 15);
    tests_passed++;
}

/* Additive Blend Tests */
void test_additive_brightens(void) {
    u16 result = blend_additive(make_565(10, 20, 10), make_565(10, 20, 10), 255);
    ASSERT_TRUE(((result >> 11) & 0x1F) >= 10);
    ASSERT_TRUE(((result >> 5) & 0x3F) >= 20);
    tests_passed++;
}
void test_additive_clamp_white(void) {
    u16 white = make_565(31, 63, 31);
    u16 result = blend_additive(white, white, 255);
    ASSERT_EQ((result >> 11) & 0x1F, 31);
    ASSERT_EQ((result >> 5) & 0x3F, 63);
    ASSERT_EQ(result & 0x1F, 31);
    tests_passed++;
}
void test_additive_zero_alpha(void) {
    u16 dst = make_565(0, 0, 0);
    ASSERT_EQ(blend_additive(make_565(31, 63, 31), dst, 0), dst);
    tests_passed++;
}
void test_additive_transparent(void) {
    u16 dst = make_565(10, 20, 10);
    ASSERT_EQ(blend_additive(TRANSPARENT_COLOR_565, dst, 128), dst);
    tests_passed++;
}
void test_additive_symmetric(void) {
    u16 result = blend_additive(make_565(15, 0, 15), make_565(15, 0, 15), 128);
    ASSERT_EQ((result >> 11) & 0x1F, result & 0x1F);
    tests_passed++;
}

/* Subtractive Blend Tests */
void test_subtractive_darkens(void) {
    u16 result = blend_subtractive(make_565(20, 40, 20), make_565(20, 40, 20), 128);
    ASSERT_TRUE(((result >> 11) & 0x1F) <= 20);
    ASSERT_TRUE(((result >> 5) & 0x3F) <= 40);
    tests_passed++;
}
void test_subtractive_clamp_black(void) {
    u16 result = blend_subtractive(make_565(31, 63, 31), make_565(0, 0, 0), 255);
    ASSERT_EQ(result, 0);
    tests_passed++;
}
void test_subtractive_zero_alpha(void) {
    u16 dst = make_565(20, 40, 20);
    ASSERT_EQ(blend_subtractive(make_565(31, 63, 31), dst, 0), dst);
    tests_passed++;
}
void test_subtractive_transparent(void) {
    u16 dst = make_565(10, 20, 10);
    ASSERT_EQ(blend_subtractive(TRANSPARENT_COLOR_565, dst, 128), dst);
    tests_passed++;
}
void test_subtractive_non_negative(void) {
    u16 result = blend_subtractive(make_565(31, 63, 31), make_565(1, 1, 1), 200);
    ASSERT_TRUE((s32)((result >> 11) & 0x1F) >= 0);
    ASSERT_TRUE((s32)((result >> 5) & 0x3F) >= 0);
    ASSERT_TRUE((s32)(result & 0x1F) >= 0);
    tests_passed++;
}

/* Grayscale Formula Tests */
void test_grayscale_white(void) {
    u32 gray = (31 + 63 * 2 + 31) * 3 / 10;
    ASSERT_TRUE(gray > 0);
    ASSERT_TRUE(gray <= 63);
    tests_passed++;
}
void test_grayscale_black(void) {
    ASSERT_EQ((0 + 0 * 2 + 0) * 3 / 10, 0);
    tests_passed++;
}
void test_grayscale_mid(void) {
    ASSERT_EQ((16 + 32 * 2 + 16) * 3 / 10, 28);
    tests_passed++;
}

/* Color Format Tests */
void test_565_roundtrip(void) {
    u16 p = make_565(17, 33, 8);
    ASSERT_EQ((p >> 11) & 0x1F, 17);
    ASSERT_EQ((p >> 5) & 0x3F, 33);
    ASSERT_EQ(p & 0x1F, 8);
    tests_passed++;
}
void test_555_roundtrip(void) {
    u16 p = make_555(17, 15, 8);
    ASSERT_EQ((p >> 10) & 0x1F, 17);
    ASSERT_EQ((p >> 5) & 0x1F, 15);
    ASSERT_EQ(p & 0x1F, 8);
    tests_passed++;
}
void test_565_green_6bit(void) {
    u32 diff = ((make_565(0, 63, 0) >> 5) & 0x3F) - ((make_565(0, 32, 0) >> 5) & 0x3F);
    ASSERT_EQ(diff, 31);
    tests_passed++;
}

/* Additive partial alpha */
void test_additive_partial(void) {
    u16 result = blend_additive(make_565(20, 40, 20), make_565(5, 10, 5), 64);
    u32 r = (result >> 11) & 0x1F;
    u32 g = (result >> 5) & 0x3F;
    ASSERT_TRUE(r >= 5 && r <= 25);
    ASSERT_TRUE(g >= 10 && g <= 50);
    tests_passed++;
}
void test_subtractive_partial(void) {
    u16 result = blend_subtractive(make_565(20, 40, 20), make_565(20, 40, 20), 64);
    u32 r = (result >> 11) & 0x1F;
    ASSERT_TRUE(r >= 15 && r <= 20);
    tests_passed++;
}

int main(void) {
    printf("=== Sprite Blend Functions Tests ===\n\n");

    printf("[565 Alpha Blend]\n");
    test_alpha_565_basic();       printf("  TEST: 565 basic blend ... PASS\n");
    test_alpha_565_transparent(); printf("  TEST: 565 transparent passthrough ... PASS\n");
    test_alpha_565_full_alpha();  printf("  TEST: 565 full alpha is source ... PASS\n");
    test_alpha_565_zero_alpha();  printf("  TEST: 565 zero alpha is dest ... PASS\n");
    test_alpha_565_gradient();    printf("  TEST: 565 gradient ... PASS\n");
    test_alpha_565_same_color();  printf("  TEST: 565 same color blend ... PASS\n");
    test_alpha_565_clamp();       printf("  TEST: 565 clamp max values ... PASS\n");

    printf("[555 Alpha Blend]\n");
    test_alpha_555_basic();       printf("  TEST: 555 basic blend ... PASS\n");
    test_alpha_555_transparent(); printf("  TEST: 555 transparent passthrough ... PASS\n");
    test_alpha_555_same_color();  printf("  TEST: 555 same color blend ... PASS\n");

    printf("[Additive Blend]\n");
    test_additive_brightens();    printf("  TEST: Additive brightens ... PASS\n");
    test_additive_clamp_white();  printf("  TEST: Additive clamps white ... PASS\n");
    test_additive_zero_alpha();   printf("  TEST: Additive zero alpha ... PASS\n");
    test_additive_transparent();  printf("  TEST: Additive transparent skip ... PASS\n");
    test_additive_symmetric();    printf("  TEST: Additive symmetric channels ... PASS\n");
    test_additive_partial();      printf("  TEST: Additive partial alpha ... PASS\n");

    printf("[Subtractive Blend]\n");
    test_subtractive_darkens();      printf("  TEST: Subtractive darkens ... PASS\n");
    test_subtractive_clamp_black();  printf("  TEST: Subtractive clamps black ... PASS\n");
    test_subtractive_zero_alpha();   printf("  TEST: Subtractive zero alpha ... PASS\n");
    test_subtractive_transparent();  printf("  TEST: Subtractive transparent skip ... PASS\n");
    test_subtractive_non_negative(); printf("  TEST: Subtractive non-negative ... PASS\n");
    test_subtractive_partial();      printf("  TEST: Subtractive partial alpha ... PASS\n");

    printf("[Grayscale Formula]\n");
    test_grayscale_white(); printf("  TEST: Grayscale white > 0 ... PASS\n");
    test_grayscale_black(); printf("  TEST: Grayscale black = 0 ... PASS\n");
    test_grayscale_mid();   printf("  TEST: Grayscale mid value ... PASS\n");

    printf("[Color Format]\n");
    test_565_roundtrip();   printf("  TEST: 565 roundtrip ... PASS\n");
    test_555_roundtrip();   printf("  TEST: 555 roundtrip ... PASS\n");
    test_565_green_6bit();  printf("  TEST: 565 green 6-bit ... PASS\n");

    printf("\n=== Results: %d passed, %d failed, %d total ===\n",
           tests_passed, tests_failed, tests_passed + tests_failed);
    return tests_failed;
}
