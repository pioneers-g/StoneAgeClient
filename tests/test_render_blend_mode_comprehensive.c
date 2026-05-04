/*
 * Stone Age Client - Render Blend Mode Functions Unit Tests
 * Tests for FUN_0047e640 (sprite render with blend modes)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;
typedef unsigned short u16;
typedef int s32;

/* ========================================
 * Test Data and Mock Implementations
 * ======================================== */

#define RENDER_QUEUE_MAX 4096

static u32 test_render_queue_count = 0;
static u32 test_render_queue_x[RENDER_QUEUE_MAX];
static u32 test_render_queue_y[RENDER_QUEUE_MAX];
static u32 test_render_queue_sprite[RENDER_QUEUE_MAX];
static u32 test_render_queue_layer[RENDER_QUEUE_MAX];

static int test_passed = 0;
static int test_failed = 0;

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    test_##name(); \
    printf("PASS\n"); \
    test_passed++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL at line %d: %s\n", __LINE__, #cond); \
        test_failed++; \
        return; \
    } \
} while(0)

/* Reset queue */
static void reset_queue(void) {
    test_render_queue_count = 0;
    memset(test_render_queue_x, 0, sizeof(test_render_queue_x));
    memset(test_render_queue_y, 0, sizeof(test_render_queue_y));
    memset(test_render_queue_sprite, 0, sizeof(test_render_queue_sprite));
    memset(test_render_queue_layer, 0, sizeof(test_render_queue_layer));
}

/* Mock FUN_0047e210 */
static void FUN_0047e210(u32 x, u32 y, int layer, u32 sprite_id, int flags) {
    if (test_render_queue_count >= RENDER_QUEUE_MAX) return;

    test_render_queue_x[test_render_queue_count] = x;
    test_render_queue_y[test_render_queue_count] = y;
    test_render_queue_layer[test_render_queue_count] = layer;
    test_render_queue_sprite[test_render_queue_count] = sprite_id;
    test_render_queue_count++;
    (void)flags;
}

/*
 * FUN_0047e640 - Add Sprite to Render Queue with Blend Mode
 */
static void FUN_0047e640(int x1, int y1, u32 x2, u32 y2, int layer, u32 sprite_id, int blend_mode) {
    u32 combined_x = ((u32)x1 << 16) | x2;
    u32 combined_y = ((u32)y1 << 16) | y2;

    switch (blend_mode) {
        case 0:  /* Additive */
            FUN_0047e210(combined_x, combined_y, layer, sprite_id | 0xa0000000, 0);
            break;
        case 1:  /* Subtractive */
            FUN_0047e210(combined_x, combined_y, layer, sprite_id | 0x90000000, 0);
            break;
        case 2:  /* Multiplicative */
            FUN_0047e210(combined_x, combined_y, layer, sprite_id | 0xc0000000, 0);
            break;
        default:  /* Normal */
            FUN_0047e210(combined_x, combined_y, layer, sprite_id, 0);
            break;
    }
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(normal_blend_mode) {
    reset_queue();

    FUN_0047e640(100, 200, 0, 0, 5, 0x1234, 99);

    ASSERT(test_render_queue_count == 1);
    ASSERT((test_render_queue_sprite[0] & 0xff000000) == 0);
    ASSERT((test_render_queue_sprite[0] & 0xffffff) == 0x1234);
}

TEST(additive_blend_mode) {
    reset_queue();

    FUN_0047e640(100, 200, 0, 0, 5, 0x1234, 0);

    ASSERT(test_render_queue_count == 1);
    ASSERT((test_render_queue_sprite[0] & 0xf0000000) == 0xa0000000);
}

TEST(subtractive_blend_mode) {
    reset_queue();

    FUN_0047e640(100, 200, 0, 0, 5, 0x1234, 1);

    ASSERT(test_render_queue_count == 1);
    ASSERT((test_render_queue_sprite[0] & 0xf0000000) == 0x90000000);
}

TEST(multiply_blend_mode) {
    reset_queue();

    FUN_0047e640(100, 200, 0, 0, 5, 0x1234, 2);

    ASSERT(test_render_queue_count == 1);
    ASSERT((test_render_queue_sprite[0] & 0xf0000000) == 0xc0000000);
}

TEST(position_combination) {
    reset_queue();

    FUN_0047e640(100, 200, 0x1234, 0x5678, 5, 0x9ABC, 0);

    ASSERT(test_render_queue_count == 1);
    /* X should be 100 << 16 | 0x1234 */
    ASSERT(test_render_queue_x[0] == ((100u << 16) | 0x1234));
    /* Y should be 200 << 16 | 0x5678 */
    ASSERT(test_render_queue_y[0] == ((200u << 16) | 0x5678));
}

TEST(layer_preserved) {
    reset_queue();

    FUN_0047e640(0, 0, 0, 0, 7, 0x1234, 0);

    ASSERT(test_render_queue_count == 1);
    ASSERT(test_render_queue_layer[0] == 7);
}

TEST(sprite_id_preserved_lower_bits) {
    reset_queue();

    FUN_0047e640(0, 0, 0, 0, 0, 0x00FFFFFF, 0);

    ASSERT(test_render_queue_count == 1);
    ASSERT((test_render_queue_sprite[0] & 0x00FFFFFF) == 0x00FFFFFF);
}

TEST(multiple_calls) {
    reset_queue();

    FUN_0047e640(0, 0, 0, 0, 0, 0x100, 0);
    FUN_0047e640(0, 0, 0, 0, 0, 0x200, 1);
    FUN_0047e640(0, 0, 0, 0, 0, 0x300, 2);

    ASSERT(test_render_queue_count == 3);
    ASSERT((test_render_queue_sprite[0] & 0xf0000000) == 0xa0000000);
    ASSERT((test_render_queue_sprite[1] & 0xf0000000) == 0x90000000);
    ASSERT((test_render_queue_sprite[2] & 0xf0000000) == 0xc0000000);
}

TEST(zero_position) {
    reset_queue();

    FUN_0047e640(0, 0, 0, 0, 0, 0x1234, 0);

    ASSERT(test_render_queue_count == 1);
    ASSERT(test_render_queue_x[0] == 0);
    ASSERT(test_render_queue_y[0] == 0);
}

TEST(negative_position) {
    reset_queue();

    FUN_0047e640(-10, -20, 0, 0, 0, 0x1234, 0);

    ASSERT(test_render_queue_count == 1);
    /* -10 as u16 becomes 0xfff6, shifted left 16 becomes 0xfff60000 */
    ASSERT((test_render_queue_x[0] >> 16) == 0xfff6);
    ASSERT((test_render_queue_y[0] >> 16) == 0xffec);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Render Blend Mode Functions Unit Tests ===\n\n");

    printf("Blend Mode Tests:\n");
    RUN_TEST(normal_blend_mode);
    RUN_TEST(additive_blend_mode);
    RUN_TEST(subtractive_blend_mode);
    RUN_TEST(multiply_blend_mode);

    printf("\nPosition Tests:\n");
    RUN_TEST(position_combination);
    RUN_TEST(zero_position);
    RUN_TEST(negative_position);

    printf("\nValue Preservation Tests:\n");
    RUN_TEST(layer_preserved);
    RUN_TEST(sprite_id_preserved_lower_bits);

    printf("\nMultiple Operations Tests:\n");
    RUN_TEST(multiple_calls);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
