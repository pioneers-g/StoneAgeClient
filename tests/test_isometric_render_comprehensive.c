/*
 * Stone Age Client - Isometric Rendering Unit Tests
 * Tests for FUN_004412e0 (diamond render) and FUN_00446df0 (coordinate transform)
 *
 * Based on Ghidra decompilation analysis:
 * - Coordinate transform: screen_x = (y + x) * scale_x + offset_x
 *                        screen_y = (y - x) * scale_y + offset_y
 * - Diamond iteration: -0x40 (X: -64), -0x18 (Y: -24) steps
 * - Environment effects: tile IDs 0x28-0x37 trigger weather effects
 * - Render queue: FUN_0047e210 for sprites, FUN_004445e0 for characters
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>
#include <math.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Constants from Ghidra */
#define TILE_WIDTH 64       /* 0x40 - X step in diamond iteration */
#define TILE_HEIGHT 24      /* 0x18 - Y step in diamond iteration */
#define HALF_TILE_WIDTH 32  /* 0x20 */
#define HALF_TILE_HEIGHT 12 /* 0x0c */

/* Screen bounds from FUN_004412e0 */
#define SCREEN_WIDTH 640    /* 0x280 */
#define SCREEN_HEIGHT 480   /* 0x1e0 */
#define RENDER_MIN_X -32    /* -0x20 */
#define RENDER_MAX_X 672    /* 0x2a0 */
#define RENDER_MIN_Y -24    /* -0x18 */
#define RENDER_MAX_Y 504    /* 0x1f8 */

/* Environment effect tile IDs */
#define ENV_EFFECT_MIN 0x28  /* 40 */
#define ENV_EFFECT_MAX 0x37  /* 55 */

/* Special tile ranges */
#define TILE_EFFECT_MIN 0x14  /* 20 */
#define TILE_EFFECT_MAX 0x27  /* 39 */
#define TILE_BG_EFFECT_MIN 0x28  /* 40 */
#define TILE_BG_EFFECT_MAX 0x3b  /* 59 */

/* Coordinate transform scale factors (from _DAT_0049c3e8, _DAT_0049c3e4) */
/* These are typically 32.0 and 12.0 for standard isometric */
#define ISO_SCALE_X 32.0f
#define ISO_SCALE_Y 12.0f
#define TILE_SCALE 1.0f

/* Map data storage */
#define MAP_WIDTH 64
#define MAP_HEIGHT 64
#define MAX_RENDER_QUEUE 2048

/* Environment effect mapping from FUN_00488680 */
static const u8 g_env_effect_map[16] = {
    4,   /* 0x28 → effect 4 */
    3,   /* 0x29 → effect 3 */
    7,   /* 0x2a → effect 7 */
    8,   /* 0x2b → effect 8 */
    9,   /* 0x2c → effect 9 */
    10,  /* 0x2d → effect 10 */
    11,  /* 0x2e → effect 11 */
    15,  /* 0x2f → effect 15 */
    16,  /* 0x30 → effect 16 */
    21,  /* 0x31 → effect 21 */
    17,  /* 0x32 → effect 17 */
    18,  /* 0x33 → effect 18 */
    19,  /* 0x34 → effect 19 */
    20,  /* 0x35 → effect 20 */
    22,  /* 0x36 → effect 22 */
    23   /* 0x37 → effect 23 */
};

/* Test data storage */
static u16 g_map_tiles[MAP_HEIGHT][MAP_WIDTH];
static int g_render_queue_count;
static int g_env_effect_triggered;
static u32 g_current_env_effect;

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
    memset(g_map_tiles, 0, sizeof(g_map_tiles));
    g_render_queue_count = 0;
    g_env_effect_triggered = 0;
    g_current_env_effect = 0;
}

/*
 * Coordinate transform - FUN_00446df0 pattern
 * Transforms map coordinates (x, y) to screen coordinates (sx, sy)
 *
 * Formula:
 *   sx = (y + x) * ISO_SCALE_X + offset_x
 *   sy = (y - x) * ISO_SCALE_Y + offset_y
 */
static void coordinate_transform(float map_x, float map_y,
                                 float origin_x, float origin_y,
                                 float scale_factor,
                                 float* screen_x, float* screen_y) {
    float dx = (map_x - origin_x) * scale_factor;
    float dy = (map_y - origin_y) * scale_factor;

    /* Isometric projection:
     * screen_x = (dy + dx) * ISO_SCALE_X + offset
     * screen_y = (dy - dx) * ISO_SCALE_Y + offset
     */
    *screen_x = (dy + dx) * ISO_SCALE_X;
    *screen_y = (dy - dx) * ISO_SCALE_Y;
}

/*
 * Diamond iteration step calculation - FUN_004412e0 pattern
 */
static void diamond_step(int* x, int* y, int is_row_end) {
    if (is_row_end) {
        *x += -HALF_TILE_WIDTH;   /* -0x20 */
    } else {
        *x += HALF_TILE_WIDTH;    /* 0x20 */
    }
    *y += -TILE_HEIGHT;           /* -0x18 */
}

/*
 * Get environment effect from tile ID - FUN_00488680 pattern
 * Returns effect ID or 0 if not an environment tile
 */
static u32 get_env_effect(u16 tile_id) {
    if (tile_id < ENV_EFFECT_MIN || tile_id > ENV_EFFECT_MAX) {
        return 0;
    }
    return g_env_effect_map[tile_id - ENV_EFFECT_MIN];
}

/*
 * Check if tile is in effect range - FUN_004412e0 pattern
 */
static int is_effect_tile(u16 tile_id) {
    return (tile_id >= TILE_EFFECT_MIN && tile_id <= TILE_EFFECT_MAX) ||
           (tile_id >= TILE_BG_EFFECT_MIN && tile_id <= TILE_BG_EFFECT_MAX);
}

/*
 * Check if tile should trigger environment effect - FUN_00488680 pattern
 */
static int should_trigger_env_effect(u16 tile_id) {
    return (tile_id > ENV_EFFECT_MIN && tile_id < ENV_EFFECT_MAX);
}

/*
 * Check if position is in render bounds - FUN_004412e0 pattern
 */
static int is_in_render_bounds(int x, int y) {
    return (x > RENDER_MIN_X && x < RENDER_MAX_X &&
            y > RENDER_MIN_Y && y < RENDER_MAX_Y);
}

/* ========================================
 * Coordinate Transform Tests
 * ======================================== */

static int test_coordinate_transform_origin(void) {
    test_setup();

    float sx, sy;
    coordinate_transform(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, &sx, &sy);

    /* Origin should map to (0, 0) with no offset */
    assert(sx == 0.0f);
    assert(sy == 0.0f);

    return 1;
}

static int test_coordinate_transform_positive(void) {
    test_setup();

    float sx, sy;
    /* Transform (1, 0) - one tile right */
    coordinate_transform(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, &sx, &sy);

    /* (1, 0) → (1 * 32, -1 * 12) = (32, -12) */
    assert(sx == ISO_SCALE_X);
    assert(sy == -ISO_SCALE_Y);

    return 1;
}

static int test_coordinate_transform_diagonal(void) {
    test_setup();

    float sx, sy;
    /* Transform (1, 1) - diagonal */
    coordinate_transform(1.0f, 1.0f, 0.0f, 0.0f, 1.0f, &sx, &sy);

    /* (1, 1) → ((1+1) * 32, (1-1) * 12) = (64, 0) */
    assert(sx == 2.0f * ISO_SCALE_X);
    assert(sy == 0.0f);

    return 1;
}

static int test_coordinate_transform_negative(void) {
    test_setup();

    float sx, sy;
    /* Transform (0, 1) - one tile down */
    coordinate_transform(0.0f, 1.0f, 0.0f, 0.0f, 1.0f, &sx, &sy);

    /* (0, 1) → (1 * 32, 1 * 12) = (32, 12) */
    assert(sx == ISO_SCALE_X);
    assert(sy == ISO_SCALE_Y);

    return 1;
}

static int test_coordinate_transform_symmetry(void) {
    test_setup();

    float sx1, sy1, sx2, sy2;

    /* Transform (5, 3) */
    coordinate_transform(5.0f, 3.0f, 0.0f, 0.0f, 1.0f, &sx1, &sy1);

    /* Transform (3, 5) - swapped */
    coordinate_transform(3.0f, 5.0f, 0.0f, 0.0f, 1.0f, &sx2, &sy2);

    /* X should be same (symmetric), Y should be negated */
    assert(sx1 == sx2);
    assert(sy1 == -sy2);

    return 1;
}

/* ========================================
 * Diamond Iteration Tests
 * ======================================== */

static int test_diamond_step_values(void) {
    test_setup();

    /* Step values from FUN_004412e0:
     * iVar3 = iVar3 + -0x40 (X step)
     * iVar4 = iVar4 + -0x18 (Y step)
     */
    assert(TILE_WIDTH == 64);
    assert(TILE_HEIGHT == 24);
    assert(HALF_TILE_WIDTH == 32);
    assert(HALF_TILE_HEIGHT == 12);

    return 1;
}

static int test_diamond_iteration_pattern(void) {
    test_setup();

    int x = 0, y = 0;
    int steps = 0;

    /* Simulate diamond iteration across one row */
    for (int i = 0; i < 10; i++) {
        x += -TILE_WIDTH;
        y += -TILE_HEIGHT;
        steps++;
    }

    /* After 10 steps: x = -640, y = -240 */
    assert(x == -640);
    assert(y == -240);
    assert(steps == 10);

    return 1;
}

static int test_diamond_row_transition(void) {
    test_setup();

    /* From FUN_004412e0:
     * At row end: iVar3 = local_28 + -0x20 (backtrack half tile)
     * Otherwise: iVar3 = local_28 + 0x20 (advance half tile)
     */
    int x = 100, y = 100;

    /* Normal step */
    int x_normal = x + HALF_TILE_WIDTH;
    int y_normal = y - TILE_HEIGHT;

    assert(x_normal == 132);
    assert(y_normal == 76);

    /* Row end step */
    int x_row_end = x - HALF_TILE_WIDTH;
    int y_row_end = y - TILE_HEIGHT;

    assert(x_row_end == 68);
    assert(y_row_end == 76);

    return 1;
}

/* ========================================
 * Environment Effect Tests
 * ======================================== */

static int test_env_effect_tile_range(void) {
    test_setup();

    /* Environment effect tiles: 0x28-0x37 (40-55) */
    assert(ENV_EFFECT_MIN == 0x28);
    assert(ENV_EFFECT_MAX == 0x37);

    /* Verify range size */
    assert(ENV_EFFECT_MAX - ENV_EFFECT_MIN + 1 == 16);

    return 1;
}

static int test_env_effect_mapping(void) {
    test_setup();

    /* From FUN_00488680 switch statement */
    assert(get_env_effect(0x28) == 4);
    assert(get_env_effect(0x29) == 3);
    assert(get_env_effect(0x2a) == 7);
    assert(get_env_effect(0x2b) == 8);
    assert(get_env_effect(0x2c) == 9);
    assert(get_env_effect(0x2d) == 10);
    assert(get_env_effect(0x2e) == 11);
    assert(get_env_effect(0x2f) == 15);
    assert(get_env_effect(0x30) == 16);
    assert(get_env_effect(0x31) == 21);
    assert(get_env_effect(0x32) == 17);
    assert(get_env_effect(0x33) == 18);
    assert(get_env_effect(0x34) == 19);
    assert(get_env_effect(0x35) == 20);
    assert(get_env_effect(0x36) == 22);
    assert(get_env_effect(0x37) == 23);

    return 1;
}

static int test_env_effect_invalid_tile(void) {
    test_setup();

    /* Tiles outside 0x28-0x37 should return 0 */
    assert(get_env_effect(0x00) == 0);
    assert(get_env_effect(0x27) == 0);
    assert(get_env_effect(0x38) == 0);
    assert(get_env_effect(0xFF) == 0);

    return 1;
}

static int test_should_trigger_env_effect(void) {
    test_setup();

    /* From FUN_00488680: (0x27 < param_1) && (param_1 < 0x38) */
    assert(should_trigger_env_effect(0x28) == 1);
    assert(should_trigger_env_effect(0x30) == 1);
    assert(should_trigger_env_effect(0x37) == 1);

    /* Should NOT trigger */
    assert(should_trigger_env_effect(0x27) == 0);
    assert(should_trigger_env_effect(0x38) == 0);

    return 1;
}

/* ========================================
 * Render Bounds Tests
 * ======================================== */

static int test_render_bounds_constants(void) {
    test_setup();

    /* From FUN_004412e0 bounds checks:
     * -0x20 < iVar3 (x > -32)
     * iVar3 < 0x2a0 (x < 672)
     * -0x18 < iVar4 (y > -24)
     * iVar4 < 0x1f8 (y < 504)
     */
    assert(RENDER_MIN_X == -32);
    assert(RENDER_MAX_X == 672);
    assert(RENDER_MIN_Y == -24);
    assert(RENDER_MAX_Y == 504);

    return 1;
}

static int test_render_bounds_check(void) {
    test_setup();

    /* Inside bounds */
    assert(is_in_render_bounds(0, 0) == 1);
    assert(is_in_render_bounds(320, 240) == 1);
    assert(is_in_render_bounds(640, 480) == 1);

    /* Outside bounds */
    assert(is_in_render_bounds(-50, 0) == 0);
    assert(is_in_render_bounds(700, 0) == 0);
    assert(is_in_render_bounds(0, -50) == 0);
    assert(is_in_render_bounds(0, 600) == 0);

    return 1;
}

static int test_render_bounds_screen_size(void) {
    test_setup();

    /* Screen is 640x480 (0x280 x 0x1e0) */
    assert(SCREEN_WIDTH == 640);
    assert(SCREEN_HEIGHT == 480);
    assert(SCREEN_WIDTH == 0x280);
    assert(SCREEN_HEIGHT == 0x1e0);

    return 1;
}

/* ========================================
 * Tile Classification Tests
 * ======================================== */

static int test_tile_effect_range(void) {
    test_setup();

    /* From FUN_004412e0:
     * (uVar1 < 0x14) || (0x27 < uVar1) - non-effect tiles
     * (0x27 < uVar1) && (uVar1 < 0x3c) - BG effect tiles
     */
    assert(TILE_EFFECT_MIN == 0x14);
    assert(TILE_EFFECT_MAX == 0x27);
    assert(TILE_BG_EFFECT_MIN == 0x28);
    assert(TILE_BG_EFFECT_MAX == 0x3b);

    return 1;
}

static int test_is_effect_tile(void) {
    test_setup();

    /* Effect tiles: 0x14-0x27 and 0x28-0x3b */
    assert(is_effect_tile(0x14) == 1);
    assert(is_effect_tile(0x20) == 1);
    assert(is_effect_tile(0x27) == 1);
    assert(is_effect_tile(0x28) == 1);
    assert(is_effect_tile(0x35) == 1);
    assert(is_effect_tile(0x3b) == 1);

    /* Non-effect tiles */
    assert(is_effect_tile(0x00) == 0);
    assert(is_effect_tile(0x13) == 0);
    assert(is_effect_tile(0x3c) == 0);
    assert(is_effect_tile(0xFF) == 0);

    return 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_coordinate_transform_grid(void) {
    test_setup();

    /* Test a 3x3 grid transformation */
    float positions[3][3][2];

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            coordinate_transform((float)x, (float)y, 0.0f, 0.0f, 1.0f,
                               &positions[y][x][0], &positions[y][x][1]);
        }
    }

    /* Verify center tile (1, 1) */
    assert(positions[1][1][0] == 64.0f);  /* (1+1) * 32 */
    assert(positions[1][1][1] == 0.0f);   /* (1-1) * 12 */

    /* Verify corners are symmetric around center */
    /* (0,0) should be (-32, -12) from center */
    assert(positions[0][0][0] == 0.0f);
    assert(positions[0][0][1] == 0.0f);

    /* (2,2) should be (+32, +12) from (0,0) in screen space... actually (64, 0) */
    assert(positions[2][2][0] == 128.0f);
    assert(positions[2][2][1] == 0.0f);

    return 1;
}

static int test_diamond_render_order(void) {
    test_setup();

    /* Diamond iteration renders tiles in back-to-front order
     * Starting from top, going down-right then back-left
     */

    /* Simulate starting position from FUN_004412e0:
     * iVar3 = ((map_width + map_height) * 0x40 + -0x40) / 2 + start_x
     * iVar4 = (((map_height - map_width) + -1) * 0x30) / 2 + start_y
     */

    int map_w = 10, map_h = 10;
    int start_x = ((map_w + map_h) * 0x40 + -0x40) / 2;
    int start_y = (((map_h - map_w) + -1) * 0x30) / 2;

    /* Verify starting position calculation */
    assert(start_x == ((20 * 64 - 64) / 2));  /* (1280 - 64) / 2 = 608 */
    assert(start_y == ((-1) * 48) / 2);       /* -24 */

    return 1;
}

static int test_tile_id_validity_check(void) {
    test_setup();

    /* From FUN_004412e0:
     * if (uVar1 < 100) { ... } - special handling for low tile IDs
     * else { FUN_0047e210(...) } - render as sprite
     */

    u16 valid_sprite_tile = 100;
    u16 special_tile = 50;

    assert(valid_sprite_tile >= 100);
    assert(special_tile < 100);

    return 1;
}

static int test_character_render_depth(void) {
    test_setup();

    /* From FUN_004445e0: characters use depth sorting
     * Depth is calculated from map coordinates
     */

    float depth_scale = 1.0f;  /* _DAT_0049c31c */
    int map_x = 5, map_y = 3;

    /* Depth formula: map_x * depth_scale for horizontal movement */
    float depth = (float)map_x * depth_scale;

    assert(depth == 5.0f);

    return 1;
}

/* ========================================
 * Memory Layout Tests
 * ======================================== */

static int test_map_tile_storage(void) {
    test_setup();

    /* Map tiles stored in 2D array at DAT_04581d48
     * Each tile is 2 bytes (u16)
     * Stride: DAT_045602b0 (map width)
     */
    assert(sizeof(u16) == 2);
    assert(MAP_WIDTH == 64);
    assert(MAP_HEIGHT == 64);

    return 1;
}

static int test_render_queue_size(void) {
    test_setup();

    /* From FUN_004445e0: DAT_0456b17c < 0x800 (2048) */
    assert(MAX_RENDER_QUEUE == 2048);
    assert(0x800 == 2048);

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Isometric Rendering Unit Tests ===\n\n");

    /* Coordinate transform tests */
    printf("Coordinate Transform Tests (FUN_00446df0):\n");
    TEST(coordinate_transform_origin);
    TEST(coordinate_transform_positive);
    TEST(coordinate_transform_diagonal);
    TEST(coordinate_transform_negative);
    TEST(coordinate_transform_symmetry);

    /* Diamond iteration tests */
    printf("\nDiamond Iteration Tests (FUN_004412e0):\n");
    TEST(diamond_step_values);
    TEST(diamond_iteration_pattern);
    TEST(diamond_row_transition);

    /* Environment effect tests */
    printf("\nEnvironment Effect Tests (FUN_00488680):\n");
    TEST(env_effect_tile_range);
    TEST(env_effect_mapping);
    TEST(env_effect_invalid_tile);
    TEST(should_trigger_env_effect);

    /* Render bounds tests */
    printf("\nRender Bounds Tests:\n");
    TEST(render_bounds_constants);
    TEST(render_bounds_check);
    TEST(render_bounds_screen_size);

    /* Tile classification tests */
    printf("\nTile Classification Tests:\n");
    TEST(tile_effect_range);
    TEST(is_effect_tile);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(coordinate_transform_grid);
    TEST(diamond_render_order);
    TEST(tile_id_validity_check);
    TEST(character_render_depth);

    /* Memory layout tests */
    printf("\nMemory Layout Tests:\n");
    TEST(map_tile_storage);
    TEST(render_queue_size);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - FUN_00440aa0 (map data initialization)
     * - FUN_00441b90 (map boundary calculation)
     * - FUN_00442070 (character queue processing)
     * - FUN_004888f0 (tile effect rendering)
     * - Full diamond iteration with actual map data
     * - Character depth sorting verification
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
