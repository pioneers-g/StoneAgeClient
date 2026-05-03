/*
 * Stone Age Client - Render Queue Unit Tests
 * Tests for FUN_0047e210, FUN_0047dc60, FUN_0047e640
 *
 * Based on Ghidra decompilation analysis:
 * - Queue max: 0xfff (4095) entries
 * - Priority/blend mode mapping
 * - Layer thresholds: 0x67 (103), 0x6c (108), 0x6e (110)
 * - Special sprite flags: 0x80000000
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Stub types for testing without Windows/DirectX */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Maximum queue entries - 0xfff from FUN_0047e210 */
#define MAX_QUEUE_ENTRIES 0x1000

/* Queue storage - matching DAT_04633488 region layout */
static u16 s_queue_index[MAX_QUEUE_ENTRIES];      /* DAT_0464b488 */
static u8  s_queue_layer[MAX_QUEUE_ENTRIES];      /* DAT_0464b48a */
static s32 s_queue_x[MAX_QUEUE_ENTRIES];          /* DAT_04633488 */
static s32 s_queue_y[MAX_QUEUE_ENTRIES];          /* DAT_0463348c */
static u32 s_queue_sprite_id[MAX_QUEUE_ENTRIES];  /* DAT_04633490 */
static void* s_queue_linked[MAX_QUEUE_ENTRIES];   /* DAT_04633494 */
static s32 s_queue_priority[MAX_QUEUE_ENTRIES];   /* DAT_04633498 */
static u8  s_queue_blend_mode[MAX_QUEUE_ENTRIES]; /* DAT_0463349c */

static int s_queue_count = 0;
static int s_render_mode = 0;

/* Stub functions for sprite dimension lookup */
static u16 g_stub_sprite_w = 0;
static u16 g_stub_sprite_h = 0;

int render_get_sprite_dimensions(u32 sprite_id, u16* width, u16* height) {
    /* Stub implementation - return preset values */
    (void)sprite_id;
    *width = g_stub_sprite_w;
    *height = g_stub_sprite_h;
    return 1;
}

/* Test helper to set stub sprite dimensions */
static void test_set_sprite_dimensions(u16 w, u16 h) {
    g_stub_sprite_w = w;
    g_stub_sprite_h = h;
}

/*
 * Initialize render queue
 */
void render_queue_init(void) {
    memset(s_queue_index, 0, sizeof(s_queue_index));
    memset(s_queue_layer, 0, sizeof(s_queue_layer));
    memset(s_queue_x, 0, sizeof(s_queue_x));
    memset(s_queue_y, 0, sizeof(s_queue_y));
    memset(s_queue_sprite_id, 0, sizeof(s_queue_sprite_id));
    memset(s_queue_linked, 0, sizeof(s_queue_linked));
    memset(s_queue_priority, 0, sizeof(s_queue_priority));
    memset(s_queue_blend_mode, 0, sizeof(s_queue_blend_mode));
    s_queue_count = 0;
}

/*
 * Clear render queue
 */
void render_queue_clear(void) {
    s_queue_count = 0;
}

/*
 * Get current render queue count
 */
int render_queue_get_count(void) {
    return s_queue_count;
}

/*
 * Set render mode
 */
void render_set_mode(int mode) {
    s_render_mode = mode;
}

/*
 * Add sprite to render queue - FUN_0047e210
 * Returns queue index on success, -2 if queue full or invalid sprite_id
 *
 * From Ghidra analysis:
 * - Sprite IDs -1 to 99 are invalid (return -2)
 * - Sprite IDs >= 100 or < -1 are valid
 */
int render_queue_add_sprite(int x, int y, int layer, u32 sprite_id, int priority) {
    int index;
    u16 sprite_w = 0, sprite_h = 0;
    int blend_mode;
    int actual_priority;
    s32 sid = (s32)sprite_id;

    /* Check queue limit (0xfff = 4095 from original) */
    if (s_queue_count >= 0xfff) {
        return -2;
    }

    /* Sprite ID validation - FUN_0047e210 pattern */
    if (sid >= -1 && sid < 100) {
        /* Invalid range: -1 to 99 */
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
    } else if (priority < 0x14) {  /* 10-19 */
        blend_mode = 1;
        actual_priority = priority - 10;
    } else if (priority < 0x1e) {  /* 20-29 */
        blend_mode = 2;
        actual_priority = priority - 0x14;
    } else if (priority < 0x28) {  /* 30-39 */
        blend_mode = 3;
        actual_priority = priority - 0x1e;
    } else if (priority < 0x32) {  /* 40-49 */
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
 * Add entry to render queue
 */
int render_queue_add(int x, int y, u32 sprite_id, u32 flags, void* linked_obj) {
    int index;
    u16 sprite_w = 0, sprite_h = 0;
    int priority = flags & 0xFF;
    int blend_mode = 0;

    if (s_queue_count >= 0xfff) {
        return -2;
    }

    index = s_queue_count;

    /* Check for special sprite flags */
    if ((sprite_id & 0x80000000) == 0) {
        s32 sid = (s32)sprite_id;
        if (sid >= -1 && sid < 100) {
            sprite_w = 0;
            sprite_h = 0;
        } else {
            render_get_sprite_dimensions(sprite_id, &sprite_w, &sprite_h);
        }
    }

    if (priority < 10) {
        blend_mode = 0;
    } else if (priority < 0x14) {
        blend_mode = 1;
        priority -= 10;
    } else if (priority < 0x1e) {
        blend_mode = 2;
        priority -= 0x14;
    } else if (priority < 0x28) {
        blend_mode = 3;
        priority -= 0x1e;
    } else if (priority < 0x32) {
        blend_mode = 4;
        priority -= 0x28;
    } else {
        blend_mode = 0;
    }

    s_queue_index[index] = (u16)index;
    s_queue_layer[index] = 0;
    s_queue_x[index] = x + sprite_w;
    s_queue_y[index] = y + sprite_h;
    s_queue_sprite_id[index] = sprite_id;
    s_queue_linked[index] = linked_obj;
    s_queue_priority[index] = priority;
    s_queue_blend_mode[index] = (u8)blend_mode;

    s_queue_count++;
    return index;
}

/*
 * Add fade effect to queue - FUN_0047e640
 */
int render_queue_add_fade(int x1, int y1, int x2, int y2, int sprite_type, u32 base_sprite, int fade_mode) {
    u32 packed_x;
    u32 packed_y;
    u32 sprite_id;

    if (s_queue_count >= 0xfff) {
        return -2;
    }

    packed_x = ((u32)x1 & 0xFFFF) | ((u32)x2 << 16);
    packed_y = ((u32)y1 & 0xFFFF) | ((u32)y2 << 16);

    switch (fade_mode) {
        case 0:
            sprite_id = base_sprite | 0xa0000000;
            break;
        case 1:
            sprite_id = base_sprite | 0x90000000;
            break;
        case 2:
            sprite_id = base_sprite | 0xc0000000;
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
    s_queue_linked[index] = NULL;
    s_queue_priority[index] = 0;
    s_queue_blend_mode[index] = 0;

    s_queue_count++;
    return index;
}

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

/* Test fixtures */
static void test_setup(void) {
    render_queue_init();
    test_set_sprite_dimensions(0, 0);
}

static void test_teardown(void) {
    render_queue_clear();
}

/* ========================================
 * Queue Initialization Tests
 * ======================================== */

static int test_queue_init_clear(void) {
    render_queue_init();
    assert(render_queue_get_count() == 0);
    return 1;
}

static int test_queue_count_after_add(void) {
    test_setup();
    test_set_sprite_dimensions(32, 48);

    int result = render_queue_add_sprite(100, 100, 0, 1000, 5);
    assert(result >= 0);
    assert(render_queue_get_count() == 1);

    render_queue_add_sprite(200, 200, 0, 1001, 10);
    assert(render_queue_get_count() == 2);

    test_teardown();
    return 1;
}

/* ========================================
 * Queue Limit Tests - FUN_0047e210
 * ======================================== */

static int test_queue_limit_fff(void) {
    test_setup();

    /* Max entries is 0xfff = 4095 */
    int i;
    for (i = 0; i < 4095; i++) {
        int result = render_queue_add_sprite(i * 10, i * 10, 0, 1000 + i, 0);
        if (result == -2) {
            assert(i >= 4094);
            break;
        }
    }

    int result = render_queue_add_sprite(0, 0, 0, 9999, 0);
    assert(result == -2);

    test_teardown();
    return 1;
}

static int test_queue_return_value(void) {
    test_setup();

    int idx0 = render_queue_add_sprite(10, 20, 0, 1000, 0);
    int idx1 = render_queue_add_sprite(30, 40, 0, 1001, 0);
    int idx2 = render_queue_add_sprite(50, 60, 0, 1002, 0);

    assert(idx0 == 0);
    assert(idx1 == 1);
    assert(idx2 == 2);

    test_teardown();
    return 1;
}

/* ========================================
 * Priority/Blend Mode Tests - FUN_0047e210
 * ======================================== */

static int test_priority_range_0_to_9(void) {
    test_setup();

    for (int p = 0; p <= 9; p++) {
        render_queue_clear();
        render_queue_add_sprite(0, 0, 0, 1000, p);

        assert(s_queue_blend_mode[0] == 0);
        assert(s_queue_priority[0] == p);
    }

    test_teardown();
    return 1;
}

static int test_priority_range_10_to_19(void) {
    test_setup();

    /* Priority 10-19 (0xa-0x13): blend_mode = 1, priority -= 10 */
    for (int p = 10; p < 0x14; p++) {
        render_queue_clear();
        render_queue_add_sprite(0, 0, 0, 1000, p);

        assert(s_queue_blend_mode[0] == 1);
        assert(s_queue_priority[0] == p - 10);
    }

    test_teardown();
    return 1;
}

static int test_priority_range_20_to_29(void) {
    test_setup();

    /* Priority 20-29 (0x14-0x1d): blend_mode = 2 */
    for (int p = 0x14; p < 0x1e; p++) {
        render_queue_clear();
        render_queue_add_sprite(0, 0, 0, 1000, p);

        assert(s_queue_blend_mode[0] == 2);
        assert(s_queue_priority[0] == p - 0x14);
    }

    test_teardown();
    return 1;
}

static int test_priority_range_30_to_39(void) {
    test_setup();

    /* Priority 30-39 (0x1e-0x27): blend_mode = 3 */
    for (int p = 0x1e; p < 0x28; p++) {
        render_queue_clear();
        render_queue_add_sprite(0, 0, 0, 1000, p);

        assert(s_queue_blend_mode[0] == 3);
        assert(s_queue_priority[0] == p - 0x1e);
    }

    test_teardown();
    return 1;
}

static int test_priority_range_40_to_49(void) {
    test_setup();

    /* Priority 40-49 (0x28-0x31): blend_mode = 4 */
    for (int p = 0x28; p < 0x32; p++) {
        render_queue_clear();
        render_queue_add_sprite(0, 0, 0, 1000, p);

        assert(s_queue_blend_mode[0] == 4);
        assert(s_queue_priority[0] == p - 0x28);
    }

    test_teardown();
    return 1;
}

static int test_priority_50_plus(void) {
    test_setup();

    render_queue_add_sprite(0, 0, 0, 1000, 50);
    assert(s_queue_blend_mode[0] == 0);

    render_queue_clear();
    render_queue_add_sprite(0, 0, 0, 1000, 100);
    assert(s_queue_blend_mode[0] == 0);

    test_teardown();
    return 1;
}

/* ========================================
 * Sprite ID Validation Tests
 * ======================================== */

static int test_sprite_id_invalid_range(void) {
    test_setup();

    int result;

    result = render_queue_add_sprite(0, 0, 0, (u32)-1, 0);
    assert(result == -2);

    result = render_queue_add_sprite(0, 0, 0, 0, 0);
    assert(result == -2);

    result = render_queue_add_sprite(0, 0, 0, 50, 0);
    assert(result == -2);

    result = render_queue_add_sprite(0, 0, 0, 99, 0);
    assert(result == -2);

    test_teardown();
    return 1;
}

static int test_sprite_id_valid_range(void) {
    test_setup();

    int result;

    result = render_queue_add_sprite(0, 0, 0, 100, 0);
    assert(result >= 0);

    render_queue_clear();
    result = render_queue_add_sprite(0, 0, 0, 1000, 0);
    assert(result >= 0);

    render_queue_clear();
    result = render_queue_add_sprite(0, 0, 0, 0x10000, 0);
    assert(result >= 0);

    /* Negative sprite IDs < -1 should also be valid */
    render_queue_clear();
    result = render_queue_add_sprite(0, 0, 0, (u32)-2, 0);
    assert(result >= 0);

    test_teardown();
    return 1;
}

/* ========================================
 * Position Storage Tests
 * ======================================== */

static int test_position_with_sprite_offset(void) {
    test_setup();

    /* Set sprite dimensions to 32x48 */
    test_set_sprite_dimensions(32, 48);

    /* Add at position 100, 200 */
    render_queue_add_sprite(100, 200, 0, 1000, 0);

    /* Position should be x + sprite_w, y + sprite_h */
    assert(s_queue_x[0] == 100 + 32);
    assert(s_queue_y[0] == 200 + 48);

    test_teardown();
    return 1;
}

static int test_negative_positions(void) {
    test_setup();

    int result = render_queue_add_sprite(-100, -200, 0, 1000, 0);
    assert(result >= 0);

    test_teardown();
    return 1;
}

static int test_large_positions(void) {
    test_setup();

    int result = render_queue_add_sprite(10000, 20000, 0, 1000, 0);
    assert(result >= 0);

    test_teardown();
    return 1;
}

/* ========================================
 * Layer Tests - FUN_0047dc60
 * ======================================== */

static int test_layer_storage(void) {
    test_setup();

    render_queue_add_sprite(0, 0, 100, 1000, 0);
    assert(s_queue_layer[0] == 100);

    render_queue_clear();
    render_queue_add_sprite(0, 0, 103, 1000, 0);
    assert(s_queue_layer[0] == 103);

    render_queue_clear();
    render_queue_add_sprite(0, 0, 110, 1000, 0);
    assert(s_queue_layer[0] == 110);

    test_teardown();
    return 1;
}

/* ========================================
 * Special Sprite Flags Tests
 * ======================================== */

static int test_special_sprite_flag_80000000(void) {
    test_setup();

    u32 special_id = 0x80000000 | 1000;
    int result = render_queue_add(0, 0, special_id, 0, NULL);

    assert(result >= 0);
    assert(s_queue_sprite_id[0] == special_id);

    test_teardown();
    return 1;
}

static int test_fade_mode_flags(void) {
    test_setup();

    int result;

    result = render_queue_add_fade(10, 20, 100, 200, 0, 0x1000, 0);
    assert(result >= 0);
    /* Fade mode 0 should have 0xa0000000 flag (which is 0x20000000) */
    assert((s_queue_sprite_id[0] & 0x20000000) != 0);

    render_queue_clear();
    result = render_queue_add_fade(10, 20, 100, 200, 0, 0x1000, 1);
    assert(result >= 0);
    /* Fade mode 1 should have 0x90000000 flag (which is 0x10000000) */
    assert((s_queue_sprite_id[0] & 0x10000000) != 0);

    render_queue_clear();
    result = render_queue_add_fade(10, 20, 100, 200, 0, 0x1000, 2);
    assert(result >= 0);
    /* Fade mode 2 should have 0xc0000000 flag (which is 0x40000000) */
    assert((s_queue_sprite_id[0] & 0x40000000) != 0);

    test_teardown();
    return 1;
}

/* ========================================
 * Coordinate Packing Tests - FUN_0047e640
 * ======================================== */

static int test_coordinate_packing(void) {
    test_setup();

    int x1 = 100, y1 = 200, x2 = 300, y2 = 400;

    render_queue_add_fade(x1, y1, x2, y2, 0, 0x1000, 0);

    u32 packed_x = (u32)s_queue_x[0];
    u32 packed_y = (u32)s_queue_y[0];

    assert((packed_x & 0xFFFF) == (x1 & 0xFFFF));
    assert((packed_x >> 16) == (x2 & 0xFFFF));
    assert((packed_y & 0xFFFF) == (y1 & 0xFFFF));
    assert((packed_y >> 16) == (y2 & 0xFFFF));

    test_teardown();
    return 1;
}

/* ========================================
 * Queue Clear Tests
 * ======================================== */

static int test_queue_clear(void) {
    test_setup();

    for (int i = 0; i < 100; i++) {
        render_queue_add_sprite(i * 10, i * 10, 0, 1000 + i, i % 50);
    }

    assert(render_queue_get_count() == 100);

    render_queue_clear();
    assert(render_queue_get_count() == 0);

    test_teardown();
    return 1;
}

/* ========================================
 * Render Mode Tests
 * ======================================== */

static int test_render_mode_normal(void) {
    render_set_mode(0);
    assert(s_render_mode == 0);
    return 1;
}

static int test_render_mode_battle(void) {
    render_set_mode(3);
    assert(s_render_mode == 3);
    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Render Queue Unit Tests ===\n\n");

    /* Initialization tests */
    printf("Initialization Tests:\n");
    TEST(queue_init_clear);
    TEST(queue_count_after_add);

    /* Queue limit tests */
    printf("\nQueue Limit Tests:\n");
    TEST(queue_limit_fff);
    TEST(queue_return_value);

    /* Priority/blend mode tests */
    printf("\nPriority/Blend Mode Tests:\n");
    TEST(priority_range_0_to_9);
    TEST(priority_range_10_to_19);
    TEST(priority_range_20_to_29);
    TEST(priority_range_30_to_39);
    TEST(priority_range_40_to_49);
    TEST(priority_50_plus);

    /* Sprite ID validation tests */
    printf("\nSprite ID Validation Tests:\n");
    TEST(sprite_id_invalid_range);
    TEST(sprite_id_valid_range);

    /* Position tests */
    printf("\nPosition Tests:\n");
    TEST(position_with_sprite_offset);
    TEST(negative_positions);
    TEST(large_positions);

    /* Layer tests */
    printf("\nLayer Tests:\n");
    TEST(layer_storage);

    /* Special sprite tests */
    printf("\nSpecial Sprite Tests:\n");
    TEST(special_sprite_flag_80000000);
    TEST(fade_mode_flags);
    TEST(coordinate_packing);

    /* Clear tests */
    printf("\nClear Tests:\n");
    TEST(queue_clear);

    /* Render mode tests */
    printf("\nRender Mode Tests:\n");
    TEST(render_mode_normal);
    TEST(render_mode_battle);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
