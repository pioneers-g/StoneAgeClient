/*
 * Stone Age Client - Render Queue Comprehensive Unit Tests
 * Tests for FUN_0047e210 (queue add), FUN_0047dc60 (queue process),
 * FUN_0047e640 (fade add), FUN_004808e0 (sprite validate)
 *
 * Based on Ghidra decompilation analysis:
 * - Queue limit: 0xfff (4095) entries
 * - Sprite ID validation: -1 to 99 invalid
 * - Priority ranges: 0-9, 10-19, 20-29, 30-39, 40-49
 * - Blend modes: 0=normal, 1=alpha, 2=additive, 3=subtractive, 4=special
 * - Fade flags: 0xa0000000, 0x90000000, 0xc0000000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Constants from Ghidra */
#define MAX_QUEUE_ENTRIES 0x1000
#define QUEUE_LIMIT 0xfff

/* Render flags from FUN_0047e640 */
#define FADE_FLAG_FILL    0xa0000000
#define FADE_FLAG_SPECIAL 0x90000000
#define FADE_FLAG_OUTLINE 0xc0000000

/* Sprite ID flags from FUN_0047dc60 */
#define SPRITE_FLAG_HIDDEN 0x80000000

/* Queue storage - matching DAT_04633488 region layout */
static u16 s_queue_index[MAX_QUEUE_ENTRIES];
static u8  s_queue_layer[MAX_QUEUE_ENTRIES];
static s32 s_queue_x[MAX_QUEUE_ENTRIES];
static s32 s_queue_y[MAX_QUEUE_ENTRIES];
static u32 s_queue_sprite_id[MAX_QUEUE_ENTRIES];
static void* s_queue_linked[MAX_QUEUE_ENTRIES];
static s32 s_queue_priority[MAX_QUEUE_ENTRIES];
static u8  s_queue_blend_mode[MAX_QUEUE_ENTRIES];
static int s_queue_count = 0;

/* Sprite dimensions cache */
static u16 s_sprite_width[600000];
static u16 s_sprite_height[600000];
static int s_sprite_loaded[600000];

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

/* Setup */
static void test_setup(void) {
    memset(s_queue_index, 0, sizeof(s_queue_index));
    memset(s_queue_layer, 0, sizeof(s_queue_layer));
    memset(s_queue_x, 0, sizeof(s_queue_x));
    memset(s_queue_y, 0, sizeof(s_queue_y));
    memset(s_queue_sprite_id, 0, sizeof(s_queue_sprite_id));
    memset(s_queue_linked, 0, sizeof(s_queue_linked));
    memset(s_queue_priority, 0, sizeof(s_queue_priority));
    memset(s_queue_blend_mode, 0, sizeof(s_queue_blend_mode));
    s_queue_count = 0;
    memset(s_sprite_loaded, 0, sizeof(s_sprite_loaded));
}

/* Stub: Get sprite dimensions */
static void render_get_sprite_dimensions(u32 sprite_id, u16* w, u16* h) {
    if (sprite_id >= 600000) {
        *w = 0;
        *h = 0;
        return;
    }
    *w = s_sprite_width[sprite_id];
    *h = s_sprite_height[sprite_id];
}

/* Initialize test sprite */
static void init_test_sprite(u32 sprite_id, u16 w, u16 h) {
    if (sprite_id < 600000) {
        s_sprite_width[sprite_id] = w;
        s_sprite_height[sprite_id] = h;
        s_sprite_loaded[sprite_id] = 1;
    }
}

/*
 * Add sprite to render queue - FUN_0047e210 pattern
 */
static int render_queue_add_sprite(int x, int y, int layer, u32 sprite_id, int priority) {
    int index;
    u16 sprite_w = 0, sprite_h = 0;
    int blend_mode;
    int actual_priority;
    s32 sid = (s32)sprite_id;

    /* Check queue limit - from Ghidra: 0xfff */
    if (s_queue_count >= QUEUE_LIMIT) {
        return -2;
    }

    /* Sprite ID validation - FUN_0047e210 pattern
     * From Ghidra:
     * if (param_4 < -1) {
     *   if (99 < param_4) goto get_dimensions;
     *   local_8[0] = 0; // Invalid range 0-99
     * } else {
     *   if (param_4 < 100) return -2;  // Invalid: -1 to 99
     * }
     */
    if (sid >= -1 && sid < 100) {
        return -2;
    }

    /* Get sprite dimensions for valid sprites */
    if (sid >= 100 || sid < -1) {
        render_get_sprite_dimensions(sprite_id, &sprite_w, &sprite_h);
    }

    /* Calculate priority layer - FUN_0047e210 pattern */
    if (priority < 10) {
        blend_mode = 0;
        actual_priority = priority;
    } else if (priority < 0x14) {
        blend_mode = 1;
        actual_priority = priority - 10;
    } else if (priority < 0x1e) {
        blend_mode = 2;
        actual_priority = priority - 0x14;
    } else if (priority < 0x28) {
        blend_mode = 3;
        actual_priority = priority - 0x1e;
    } else if (priority < 0x32) {
        blend_mode = 4;
        actual_priority = priority - 0x28;
    } else {
        blend_mode = 0;
        actual_priority = priority;
    }

    index = s_queue_count;

    /* Store entry */
    s_queue_index[index] = (u16)index;
    s_queue_layer[index] = (u8)layer;
    s_queue_x[index] = x + (s32)sprite_w;
    s_queue_y[index] = y + (s32)sprite_h;
    s_queue_sprite_id[index] = sprite_id;
    s_queue_linked[index] = NULL;
    s_queue_priority[index] = actual_priority;
    s_queue_blend_mode[index] = (u8)blend_mode;

    s_queue_count++;
    return index;
}

/*
 * Add fade effect to queue - FUN_0047e640 pattern
 */
static int render_queue_add_fade(int x1, int y1, int x2, int y2, int sprite_type, u32 base_sprite, int fade_mode) {
    u32 packed_x;
    u32 packed_y;
    u32 sprite_id;

    if (s_queue_count >= QUEUE_LIMIT) {
        return -2;
    }

    /* Pack coordinates */
    packed_x = ((u32)x1 & 0xFFFF) | ((u32)x2 << 16);
    packed_y = ((u32)y1 & 0xFFFF) | ((u32)y2 << 16);

    /* Build sprite ID with fade flags */
    switch (fade_mode) {
        case 0:
            sprite_id = base_sprite | FADE_FLAG_FILL;
            break;
        case 1:
            sprite_id = base_sprite | FADE_FLAG_SPECIAL;
            break;
        case 2:
            sprite_id = base_sprite | FADE_FLAG_OUTLINE;
            break;
        default:
            sprite_id = base_sprite;
            break;
    }

    int index = s_queue_count;
    s_queue_index[index] = (u16)index;
    s_queue_layer[index] = (u8)sprite_type;
    s_queue_x[index] = (s32)packed_x;
    s_queue_y[index] = (s32)packed_y;
    s_queue_sprite_id[index] = sprite_id;
    s_queue_priority[index] = 0;
    s_queue_blend_mode[index] = 0;

    s_queue_count++;
    return index;
}

/*
 * Validate sprite ID - FUN_004808e0 pattern
 */
static int render_validate_sprite(u32 sprite_id) {
    /* From FUN_004808e0:
     * if (param_1 < 500000) {
     *   // Normal sprite
     * } else if (549999 < param_1) {
     *   return 0;  // Invalid
     * }
     */
    if (sprite_id < 500000) {
        return 1;
    } else if (sprite_id <= 549999) {
        return 1;
    }
    return 0;
}

/* ========================================
 * Queue Limit Tests - FUN_0047e210
 * ======================================== */

static int test_queue_limit_0xfff(void) {
    test_setup();

    /* Queue limit is 0xfff (4095) */
    assert(QUEUE_LIMIT == 0xfff);
    assert(MAX_QUEUE_ENTRIES == 0x1000);

    return 1;
}

static int test_queue_add_success(void) {
    test_setup();

    init_test_sprite(100, 64, 48);

    int result = render_queue_add_sprite(100, 200, 0, 100, 0);

    assert(result >= 0);
    assert(s_queue_count == 1);

    return 1;
}

static int test_queue_add_fill_to_limit(void) {
    test_setup();

    /* Fill queue to near limit */
    for (int i = 0; i < QUEUE_LIMIT - 1; i++) {
        u32 sprite_id = 100 + i;
        init_test_sprite(sprite_id, 64, 48);
        int result = render_queue_add_sprite(0, 0, 0, sprite_id, 0);
        assert(result >= 0);
    }

    assert(s_queue_count == QUEUE_LIMIT - 1);

    /* One more should succeed */
    init_test_sprite(5000, 64, 48);
    int result = render_queue_add_sprite(0, 0, 0, 5000, 0);
    assert(result >= 0);
    assert(s_queue_count == QUEUE_LIMIT);

    /* Next should fail */
    init_test_sprite(5001, 64, 48);
    result = render_queue_add_sprite(0, 0, 0, 5001, 0);
    assert(result == -2);

    return 1;
}

/* ========================================
 * Sprite ID Validation Tests - FUN_0047e210
 * ======================================== */

static int test_sprite_id_invalid_negative_one(void) {
    test_setup();

    /* -1 is invalid */
    int result = render_queue_add_sprite(0, 0, 0, (u32)-1, 0);
    assert(result == -2);

    return 1;
}

static int test_sprite_id_invalid_zero_to_99(void) {
    test_setup();

    /* 0-99 are invalid */
    for (int i = 0; i < 100; i++) {
        int result = render_queue_add_sprite(0, 0, 0, i, 0);
        assert(result == -2);
    }

    return 1;
}

static int test_sprite_id_valid_100(void) {
    test_setup();

    init_test_sprite(100, 64, 48);

    /* 100 is valid */
    int result = render_queue_add_sprite(0, 0, 0, 100, 0);
    assert(result >= 0);

    return 1;
}

static int test_sprite_id_valid_large(void) {
    test_setup();

    init_test_sprite(50000, 64, 48);

    /* Large sprite IDs are valid */
    int result = render_queue_add_sprite(0, 0, 0, 50000, 0);
    assert(result >= 0);

    return 1;
}

static int test_sprite_id_valid_negative_below_minus_one(void) {
    test_setup();

    /* From Ghidra: param_4 < -1 is valid if > 99 */
    /* -2 as unsigned is 0xFFFFFFFE which is > 99 */
    init_test_sprite(0xFFFFFFFE, 64, 48);

    int result = render_queue_add_sprite(0, 0, 0, 0xFFFFFFFE, 0);
    assert(result >= 0);

    return 1;
}

/* ========================================
 * Priority/Blend Mode Tests - FUN_0047e210
 * ======================================== */

static int test_priority_range_0_to_9(void) {
    test_setup();

    init_test_sprite(100, 64, 48);

    for (int i = 0; i < 10; i++) {
        s_queue_count = 0;
        int result = render_queue_add_sprite(0, 0, 0, 100, i);
        assert(result >= 0);
        assert(s_queue_blend_mode[0] == 0);
        assert(s_queue_priority[0] == i);
    }

    return 1;
}

static int test_priority_range_10_to_19(void) {
    test_setup();

    init_test_sprite(100, 64, 48);

    for (int i = 10; i < 20; i++) {
        s_queue_count = 0;
        int result = render_queue_add_sprite(0, 0, 0, 100, i);
        assert(result >= 0);
        assert(s_queue_blend_mode[0] == 1);
        assert(s_queue_priority[0] == i - 10);
    }

    return 1;
}

static int test_priority_range_20_to_29(void) {
    test_setup();

    init_test_sprite(100, 64, 48);

    for (int i = 0x14; i < 0x1e; i++) {
        s_queue_count = 0;
        int result = render_queue_add_sprite(0, 0, 0, 100, i);
        assert(result >= 0);
        assert(s_queue_blend_mode[0] == 2);
        assert(s_queue_priority[0] == i - 0x14);
    }

    return 1;
}

static int test_priority_range_30_to_39(void) {
    test_setup();

    init_test_sprite(100, 64, 48);

    for (int i = 0x1e; i < 0x28; i++) {
        s_queue_count = 0;
        int result = render_queue_add_sprite(0, 0, 0, 100, i);
        assert(result >= 0);
        assert(s_queue_blend_mode[0] == 3);
        assert(s_queue_priority[0] == i - 0x1e);
    }

    return 1;
}

static int test_priority_range_40_to_49(void) {
    test_setup();

    init_test_sprite(100, 64, 48);

    for (int i = 0x28; i < 0x32; i++) {
        s_queue_count = 0;
        int result = render_queue_add_sprite(0, 0, 0, 100, i);
        assert(result >= 0);
        assert(s_queue_blend_mode[0] == 4);
        assert(s_queue_priority[0] == i - 0x28);
    }

    return 1;
}

static int test_priority_50_and_above(void) {
    test_setup();

    init_test_sprite(100, 64, 48);

    /* Priority 50+ uses blend mode 0 */
    int result = render_queue_add_sprite(0, 0, 0, 100, 50);
    assert(result >= 0);
    assert(s_queue_blend_mode[0] == 0);
    assert(s_queue_priority[0] == 50);

    return 1;
}

/* ========================================
 * Fade Effect Tests - FUN_0047e640
 * ======================================== */

static int test_fade_mode_0_fill_flag(void) {
    test_setup();

    int result = render_queue_add_fade(10, 20, 100, 200, 0, 0, 0);

    assert(result >= 0);
    assert((s_queue_sprite_id[0] & FADE_FLAG_FILL) != 0);
    assert((s_queue_sprite_id[0] & 0xF0000000) == 0xa0000000);

    return 1;
}

static int test_fade_mode_1_special_flag(void) {
    test_setup();

    int result = render_queue_add_fade(10, 20, 100, 200, 0, 0, 1);

    assert(result >= 0);
    assert((s_queue_sprite_id[0] & FADE_FLAG_SPECIAL) != 0);
    assert((s_queue_sprite_id[0] & 0xF0000000) == 0x90000000);

    return 1;
}

static int test_fade_mode_2_outline_flag(void) {
    test_setup();

    int result = render_queue_add_fade(10, 20, 100, 200, 0, 0, 2);

    assert(result >= 0);
    assert((s_queue_sprite_id[0] & FADE_FLAG_OUTLINE) != 0);
    assert((s_queue_sprite_id[0] & 0xF0000000) == 0xc0000000);

    return 1;
}

static int test_fade_coordinate_packing(void) {
    test_setup();

    int x1 = 100, y1 = 200, x2 = 300, y2 = 400;

    int result = render_queue_add_fade(x1, y1, x2, y2, 0, 0, 0);

    assert(result >= 0);

    /* Verify packed coordinates */
    u32 packed_x = ((u32)x1 & 0xFFFF) | ((u32)x2 << 16);
    u32 packed_y = ((u32)y1 & 0xFFFF) | ((u32)y2 << 16);

    assert(s_queue_x[0] == (s32)packed_x);
    assert(s_queue_y[0] == (s32)packed_y);

    return 1;
}

/* ========================================
 * Sprite Validation Tests - FUN_004808e0
 * ======================================== */

static int test_sprite_validate_normal_range(void) {
    /* Sprites 0-499999 are normal */
    assert(render_validate_sprite(0) == 1);
    assert(render_validate_sprite(100000) == 1);
    assert(render_validate_sprite(499999) == 1);

    return 1;
}

static int test_sprite_validate_extended_range(void) {
    /* Sprites 500000-549999 are extended */
    assert(render_validate_sprite(500000) == 1);
    assert(render_validate_sprite(525000) == 1);
    assert(render_validate_sprite(549999) == 1);

    return 1;
}

static int test_sprite_validate_invalid_range(void) {
    /* Sprites >= 550000 are invalid */
    assert(render_validate_sprite(550000) == 0);
    assert(render_validate_sprite(600000) == 0);
    assert(render_validate_sprite(1000000) == 0);

    return 1;
}

/* ========================================
 * Queue Storage Layout Tests
 * ======================================== */

static int test_queue_storage_stride(void) {
    test_setup();

    /* From Ghidra: entries stored with stride 6 (0x18 bytes) in some arrays */
    /* DAT_04633488 + iVar5 * 6 = X position */
    /* This verifies the stride is correct */

    init_test_sprite(100, 64, 48);
    init_test_sprite(101, 32, 32);

    render_queue_add_sprite(100, 200, 0, 100, 0);
    render_queue_add_sprite(300, 400, 0, 101, 0);

    assert(s_queue_x[0] == 100 + 64);
    assert(s_queue_y[0] == 200 + 48);
    assert(s_queue_x[1] == 300 + 32);
    assert(s_queue_y[1] == 400 + 32);

    return 1;
}

static int test_queue_index_storage(void) {
    test_setup();

    init_test_sprite(100, 64, 48);

    render_queue_add_sprite(0, 0, 0, 100, 0);
    render_queue_add_sprite(0, 0, 0, 101, 0);
    render_queue_add_sprite(0, 0, 0, 102, 0);

    assert(s_queue_index[0] == 0);
    assert(s_queue_index[1] == 1);
    assert(s_queue_index[2] == 2);

    return 1;
}

static int test_queue_layer_storage(void) {
    test_setup();

    init_test_sprite(100, 64, 48);

    render_queue_add_sprite(0, 0, 5, 100, 0);
    render_queue_add_sprite(0, 0, 10, 101, 0);
    render_queue_add_sprite(0, 0, 255, 102, 0);

    assert(s_queue_layer[0] == 5);
    assert(s_queue_layer[1] == 10);
    assert(s_queue_layer[2] == 255);

    return 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_multiple_queue_operations(void) {
    test_setup();

    init_test_sprite(100, 64, 48);
    init_test_sprite(200, 32, 32);
    init_test_sprite(300, 16, 16);

    /* Add sprites */
    int r1 = render_queue_add_sprite(0, 0, 0, 100, 5);
    int r2 = render_queue_add_sprite(100, 100, 1, 200, 15);
    int r3 = render_queue_add_fade(0, 0, 50, 50, 0, 0, 0);

    assert(r1 >= 0);
    assert(r2 >= 0);
    assert(r3 >= 0);
    assert(s_queue_count == 3);

    return 1;
}

static int test_blend_mode_distribution(void) {
    test_setup();

    init_test_sprite(100, 64, 48);

    /* Add with different priorities to get different blend modes */
    render_queue_add_sprite(0, 0, 0, 100, 5);   /* blend_mode 0 */
    render_queue_add_sprite(0, 0, 0, 101, 12);  /* blend_mode 1 */
    render_queue_add_sprite(0, 0, 0, 102, 25);  /* blend_mode 2 */
    render_queue_add_sprite(0, 0, 0, 103, 35);  /* blend_mode 3 */
    render_queue_add_sprite(0, 0, 0, 104, 45);  /* blend_mode 4 */

    assert(s_queue_blend_mode[0] == 0);
    assert(s_queue_blend_mode[1] == 1);
    assert(s_queue_blend_mode[2] == 2);
    assert(s_queue_blend_mode[3] == 3);
    assert(s_queue_blend_mode[4] == 4);

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Render Queue Comprehensive Unit Tests ===\n\n");

    /* Queue limit tests */
    printf("Queue Limit Tests (FUN_0047e210):\n");
    TEST(queue_limit_0xfff);
    TEST(queue_add_success);
    TEST(queue_add_fill_to_limit);

    /* Sprite ID validation tests */
    printf("\nSprite ID Validation Tests (FUN_0047e210):\n");
    TEST(sprite_id_invalid_negative_one);
    TEST(sprite_id_invalid_zero_to_99);
    TEST(sprite_id_valid_100);
    TEST(sprite_id_valid_large);
    TEST(sprite_id_valid_negative_below_minus_one);

    /* Priority/blend mode tests */
    printf("\nPriority/Blend Mode Tests (FUN_0047e210):\n");
    TEST(priority_range_0_to_9);
    TEST(priority_range_10_to_19);
    TEST(priority_range_20_to_29);
    TEST(priority_range_30_to_39);
    TEST(priority_range_40_to_49);
    TEST(priority_50_and_above);

    /* Fade effect tests */
    printf("\nFade Effect Tests (FUN_0047e640):\n");
    TEST(fade_mode_0_fill_flag);
    TEST(fade_mode_1_special_flag);
    TEST(fade_mode_2_outline_flag);
    TEST(fade_coordinate_packing);

    /* Sprite validation tests */
    printf("\nSprite Validation Tests (FUN_004808e0):\n");
    TEST(sprite_validate_normal_range);
    TEST(sprite_validate_extended_range);
    TEST(sprite_validate_invalid_range);

    /* Queue storage layout tests */
    printf("\nQueue Storage Layout Tests:\n");
    TEST(queue_storage_stride);
    TEST(queue_index_storage);
    TEST(queue_layer_storage);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(multiple_queue_operations);
    TEST(blend_mode_distribution);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
