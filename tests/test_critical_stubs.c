/*
 * Stone Age Client - Critical Stub Functions Tests
 * Tests for FUN_00488190, FUN_004011c0, FUN_00448610
 * These are the most-called FUN_* stub functions in the codebase
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

/* ========================================
 * Stubs for dependencies
 * ======================================== */

typedef uint32_t u32;

/* Graphics context */
typedef struct {
    int width;
    int height;
    void* primary_surface;
    void* back_buffer;
    void* offscreen_surface;
} GraphicsContext;

GraphicsContext g_graphics = {640, 480, NULL, NULL, NULL};
int g_sprite_width = 64;
int g_sprite_height = 48;

/* Test globals */
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) printf("  TEST: %s ... ", name);
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)
#define ASSERT_EQ(a, b) do { if ((a) != (b)) { FAIL(#a " != " #b); return; } } while(0)
#define ASSERT_NE(a, b) do { if ((a) == (b)) { FAIL(#a " == " #b); return; } } while(0)
#define ASSERT_NULL(p) do { if ((p) != NULL) { FAIL(#p " is not NULL"); return; } } while(0)
#define ASSERT_TRUE(c) do { if (!(c)) { FAIL(#c " is false"); return; } } while(0)

/* ========================================
 * Simplified implementations for testing
 * ======================================== */

/* Globals for FUN_00488190 state */
static u32 s_fade_mode = 0;
static u32 s_fade_width = 0;
static u32 s_fade_height = 0;

void FUN_00488190(int param_1, int param_2, int param_3) {
    s_fade_mode = (u32)param_1;
    s_fade_width = (u32)param_2;
    s_fade_height = (u32)param_3;
}

u32 fade_get_mode(void) { return s_fade_mode; }
u32 fade_get_width(void) { return s_fade_width; }
u32 fade_get_height(void) { return s_fade_height; }

/*
 * FUN_004011c0 - Entity/Window Cleanup
 * Uses intptr_t for portable pointer-as-int usage
 */
void FUN_004011c0(intptr_t param_1) {
    if (param_1 != 0) {
        *(int*)(param_1 + 0x24) = 1;
    }
}

/* Window render state */
static int s_last_window_x = 0;
static int s_last_window_y = 0;
static int s_last_window_w = 0;
static int s_last_window_h = 0;
static int s_last_window_sprite = 0;
static int s_last_window_style = 0;
static int s_window_draw_count = 0;

int FUN_00448610(int x, int y, int width, int height, int center_sprite, int style) {
    if (width <= 0 || height <= 0) {
        return 0;
    }

    s_last_window_x = x;
    s_last_window_y = y;
    s_last_window_w = width;
    s_last_window_h = height;
    s_last_window_sprite = center_sprite;
    s_last_window_style = style;
    s_window_draw_count++;

    return 1;
}

int window_get_draw_count(void) { return s_window_draw_count; }
int window_get_last_x(void) { return s_last_window_x; }
int window_get_last_y(void) { return s_last_window_y; }
int window_get_last_w(void) { return s_last_window_w; }
int window_get_last_h(void) { return s_last_window_h; }

/* ========================================
 * Test Functions
 * ======================================== */

void test_FUN_00488190_basic(void) {
    TEST("FUN_00488190 sets fade parameters");
    FUN_00488190(0xca, 0x140, 0xf0);
    ASSERT_EQ(fade_get_mode(), 0xca);
    ASSERT_EQ(fade_get_width(), 0x140);
    ASSERT_EQ(fade_get_height(), 0xf0);
    PASS();
}

void test_FUN_00488190_highres(void) {
    TEST("FUN_00488190 high-res mode (640x480)");
    FUN_00488190(0xca, 0x280, 0x1e0);
    ASSERT_EQ(fade_get_mode(), 0xca);
    ASSERT_EQ(fade_get_width(), 0x280);
    ASSERT_EQ(fade_get_height(), 0x1e0);
    PASS();
}

void test_FUN_00488190_zero_params(void) {
    TEST("FUN_00488190 with zero params");
    FUN_00488190(0, 0, 0);
    ASSERT_EQ(fade_get_mode(), 0);
    ASSERT_EQ(fade_get_width(), 0);
    ASSERT_EQ(fade_get_height(), 0);
    PASS();
}

void test_FUN_00488190_called_multiple_times(void) {
    TEST("FUN_00488190 called multiple times (last wins)");
    FUN_00488190(0, 100, 100);
    FUN_00488190(1, 200, 200);
    FUN_00488190(2, 320, 240);
    ASSERT_EQ(fade_get_mode(), 2);
    ASSERT_EQ(fade_get_width(), 320);
    ASSERT_EQ(fade_get_height(), 240);
    PASS();
}

void test_FUN_004011c0_null_pointer(void) {
    TEST("FUN_004011c0 with NULL pointer (no crash)");
    FUN_004011c0(0);
    PASS();
}

void test_FUN_004011c0_sets_release_flag(void) {
    TEST("FUN_004011c0 sets release flag at offset 0x24");
    char buffer[64];
    memset(buffer, 0, sizeof(buffer));
    intptr_t ptr = (intptr_t)buffer;
    ASSERT_EQ(*(int*)(ptr + 0x24), 0);
    FUN_004011c0(ptr);
    ASSERT_EQ(*(int*)(ptr + 0x24), 1);
    PASS();
}

void test_FUN_004011c0_called_twice(void) {
    TEST("FUN_004011c0 called twice on same pointer (idempotent)");
    char buffer[64];
    memset(buffer, 0, sizeof(buffer));
    intptr_t ptr = (intptr_t)buffer;
    FUN_004011c0(ptr);
    FUN_004011c0(ptr);
    ASSERT_EQ(*(int*)(ptr + 0x24), 1);
    PASS();
}

void test_FUN_004011c0_different_structures(void) {
    TEST("FUN_004011c0 on different structures");
    char buf1[64], buf2[64];
    memset(buf1, 0, sizeof(buf1));
    memset(buf2, 0, sizeof(buf2));
    intptr_t ptr1 = (intptr_t)buf1;
    intptr_t ptr2 = (intptr_t)buf2;
    FUN_004011c0(ptr1);
    ASSERT_EQ(*(int*)(ptr1 + 0x24), 1);
    ASSERT_EQ(*(int*)(ptr2 + 0x24), 0);
    FUN_004011c0(ptr2);
    ASSERT_EQ(*(int*)(ptr2 + 0x24), 1);
    PASS();
}

void test_FUN_00448610_basic_window(void) {
    TEST("FUN_00448610 basic 7x5 window");
    s_window_draw_count = 0;
    int result = FUN_00448610(0x60, 0x6c, 7, 5, 0, 1);
    ASSERT_EQ(result, 1);
    ASSERT_EQ(window_get_last_x(), 0x60);
    ASSERT_EQ(window_get_last_y(), 0x6c);
    ASSERT_EQ(window_get_last_w(), 7);
    ASSERT_EQ(window_get_last_h(), 5);
    ASSERT_EQ(window_get_draw_count(), 1);
    PASS();
}

void test_FUN_00448610_rejects_zero_width(void) {
    TEST("FUN_00448610 rejects zero width");
    s_window_draw_count = 0;
    int result = FUN_00448610(0, 0, 0, 5, 0, 1);
    ASSERT_EQ(result, 0);
    ASSERT_EQ(window_get_draw_count(), 0);
    PASS();
}

void test_FUN_00448610_rejects_zero_height(void) {
    TEST("FUN_00448610 rejects zero height");
    s_window_draw_count = 0;
    int result = FUN_00448610(0, 0, 7, 0, 0, 1);
    ASSERT_EQ(result, 0);
    ASSERT_EQ(window_get_draw_count(), 0);
    PASS();
}

void test_FUN_00448610_rejects_negative_dims(void) {
    TEST("FUN_00448610 rejects negative dimensions");
    s_window_draw_count = 0;
    int result = FUN_00448610(10, 20, -1, -1, 0, 1);
    ASSERT_EQ(result, 0);
    ASSERT_EQ(window_get_draw_count(), 0);
    PASS();
}

void test_FUN_00448610_multiple_windows(void) {
    TEST("FUN_00448610 multiple windows");
    s_window_draw_count = 0;
    FUN_00448610(10, 10, 7, 5, 0, 1);
    FUN_00448610(20, 20, 10, 8, 1, 0);
    FUN_00448610(50, 50, 3, 3, 2, 1);
    ASSERT_EQ(window_get_draw_count(), 3);
    ASSERT_EQ(window_get_last_x(), 50);
    ASSERT_EQ(window_get_last_y(), 50);
    ASSERT_EQ(window_get_last_w(), 3);
    ASSERT_EQ(window_get_last_h(), 3);
    PASS();
}

void test_FUN_00448610_various_sprites(void) {
    TEST("FUN_00448610 with different center sprites");
    s_window_draw_count = 0;
    FUN_00448610(0, 0, 5, 5, 0, 1);
    ASSERT_EQ(s_last_window_sprite, 0);
    FUN_00448610(0, 0, 5, 5, 3, 1);
    ASSERT_EQ(s_last_window_sprite, 3);
    PASS();
}

void test_FUN_00448610_style_flags(void) {
    TEST("FUN_00448610 preserves style flags");
    FUN_00448610(100, 200, 8, 6, 1, 0);
    ASSERT_EQ(s_last_window_style, 0);
    FUN_00448610(100, 200, 8, 6, 1, 1);
    ASSERT_EQ(s_last_window_style, 1);
    PASS();
}

/* ========================================
 * Main
 * ======================================== */

int main(void) {
    printf("\n=== Critical Stub Functions Tests ===\n\n");

    printf("[FUN_00488190 - Render Area/Fade Setup]\n");
    test_FUN_00488190_basic();
    test_FUN_00488190_highres();
    test_FUN_00488190_zero_params();
    test_FUN_00488190_called_multiple_times();

    printf("\n[FUN_004011c0 - Entity/Window Cleanup]\n");
    test_FUN_004011c0_null_pointer();
    test_FUN_004011c0_sets_release_flag();
    test_FUN_004011c0_called_twice();
    test_FUN_004011c0_different_structures();

    printf("\n[FUN_00448610 - Window Draw (9-Sprite Grid)]\n");
    test_FUN_00448610_basic_window();
    test_FUN_00448610_rejects_zero_width();
    test_FUN_00448610_rejects_zero_height();
    test_FUN_00448610_rejects_negative_dims();
    test_FUN_00448610_multiple_windows();
    test_FUN_00448610_various_sprites();
    test_FUN_00448610_style_flags();

    printf("\n=== Results: %d passed, %d failed, %d total ===\n",
           tests_passed, tests_failed, tests_passed + tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
