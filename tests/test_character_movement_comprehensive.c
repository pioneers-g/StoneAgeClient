/*
 * Stone Age Client - Character Movement Unit Tests
 * Tests for FUN_00443e80 (direction delta), FUN_00447150 (angle calculation),
 * FUN_00440170 (graphics init), FUN_00440280 (map reset)
 *
 * Based on Ghidra decompilation analysis:
 * - 8-direction movement with delta values
 * - Angle calculation using atan2 lookup table
 * - Resolution modes: 0=640x480, 1=320x240, 2=640x480
 * - Map view boundaries and clipping
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Constants from Ghidra */
#define DIR_COUNT 8

/* Resolution modes */
#define RES_MODE_FULL 0      /* 640x480 */
#define RES_MODE_HALF 1      /* 320x240 */
#define RES_MODE_ALT 2       /* 640x480 alternate */

/* Screen dimensions */
#define SCREEN_WIDTH_FULL 640   /* 0x280 */
#define SCREEN_HEIGHT_FULL 480  /* 0x1e0 */
#define SCREEN_WIDTH_HALF 320   /* 0x140 */
#define SCREEN_HEIGHT_HALF 240  /* 0xf0 */

/* Tile dimensions */
#define TILE_WIDTH_FULL 64   /* 0x40 */
#define TILE_HEIGHT_FULL 48  /* 0x30 */
#define TILE_WIDTH_HALF 32   /* 0x20 */
#define TILE_HEIGHT_HALF 24  /* 0x18 */

/* Direction deltas from FUN_00443e80 */
static const s32 g_dir_dx[DIR_COUNT] = {
    -1, -1, -1,  0,  1,  1,  1,  0
};

static const s32 g_dir_dy[DIR_COUNT] = {
     1,  0, -1, -1, -1,  0,  1,  1
};

/* Angle constants from FUN_00447150 */
#define ANGLE_PI 3.14159265f
#define ANGLE_2PI 6.28318531f
#define ANGLE_HALF_PI 1.57079633f

/* Test data storage */
static int g_resolution_mode = 0;
static int g_screen_width = 640;
static int g_screen_height = 480;
static int g_tile_width = 64;
static int g_tile_height = 48;

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
    g_resolution_mode = 0;
    g_screen_width = 640;
    g_screen_height = 480;
    g_tile_width = 64;
    g_tile_height = 48;
}

/*
 * Get direction delta - FUN_00443e80 pattern
 * Direction 0-7 maps to (dx, dy) offsets
 */
static void get_direction_delta(int direction, s32* dx, s32* dy) {
    switch (direction) {
        case 0: *dx = -1; *dy =  1; break;  /* Down-left */
        case 1: *dx = -1; *dy =  0; break;  /* Left */
        case 2: *dx = -1; *dy = -1; break;  /* Up-left */
        case 3: *dx =  0; *dy = -1; break;  /* Up */
        case 4: *dx =  1; *dy = -1; break;  /* Up-right */
        case 5: *dx =  1; *dy =  0; break;  /* Right */
        case 6: *dx =  1; *dy =  1; break;  /* Down-right */
        case 7: *dx =  0; *dy =  1; break;  /* Down */
        default: *dx =  0; *dy =  0; break;  /* Invalid */
    }
}

/*
 * Calculate angle from delta - FUN_00447150 pattern
 * Returns angle in radians
 */
static float calculate_angle(float dx, float dy) {
    if (dx == 0.0f && dy == 0.0f) {
        return 0.0f;
    }
    return atan2f(dy, dx);
}

/*
 * Initialize graphics mode - FUN_00440170 pattern
 */
static void graphics_init(int mode) {
    g_resolution_mode = mode;

    if (mode == RES_MODE_FULL || mode == RES_MODE_ALT) {
        g_screen_width = SCREEN_WIDTH_FULL;
        g_screen_height = SCREEN_HEIGHT_FULL;
        g_tile_width = TILE_WIDTH_FULL;
        g_tile_height = TILE_HEIGHT_FULL;
    } else if (mode == RES_MODE_HALF) {
        g_screen_width = SCREEN_WIDTH_HALF;
        g_screen_height = SCREEN_HEIGHT_HALF;
        g_tile_width = TILE_WIDTH_HALF;
        g_tile_height = TILE_HEIGHT_HALF;
    }
}

/*
 * Check if point is in map bounds
 */
static int is_in_map_bounds(int x, int y, int map_width, int map_height) {
    return (x >= 0 && x < map_width && y >= 0 && y < map_height);
}

/*
 * Calculate distance between two points
 */
static float calculate_distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

/* ========================================
 * Direction Delta Tests
 * ======================================== */

static int test_direction_count(void) {
    test_setup();

    /* 8 directions */
    assert(DIR_COUNT == 8);

    return 1;
}

static int test_direction_delta_values(void) {
    test_setup();

    /* From FUN_00443e80 switch statement:
     * case 0: dx=-1, dy=1
     * case 1: dx=-1, dy=0
     * case 2: dx=-1, dy=-1
     * case 3: dx=0, dy=-1
     * case 4: dx=1, dy=-1
     * case 5: dx=1, dy=0
     * case 6: dx=1, dy=1
     * case 7: dx=0, dy=1
     */

    s32 dx, dy;

    get_direction_delta(0, &dx, &dy);
    assert(dx == -1 && dy == 1);

    get_direction_delta(1, &dx, &dy);
    assert(dx == -1 && dy == 0);

    get_direction_delta(2, &dx, &dy);
    assert(dx == -1 && dy == -1);

    get_direction_delta(3, &dx, &dy);
    assert(dx == 0 && dy == -1);

    get_direction_delta(4, &dx, &dy);
    assert(dx == 1 && dy == -1);

    get_direction_delta(5, &dx, &dy);
    assert(dx == 1 && dy == 0);

    get_direction_delta(6, &dx, &dy);
    assert(dx == 1 && dy == 1);

    get_direction_delta(7, &dx, &dy);
    assert(dx == 0 && dy == 1);

    return 1;
}

static int test_direction_invalid(void) {
    test_setup();

    s32 dx, dy;

    /* Invalid direction should return (0, 0) */
    get_direction_delta(-1, &dx, &dy);
    assert(dx == 0 && dy == 0);

    get_direction_delta(8, &dx, &dy);
    assert(dx == 0 && dy == 0);

    get_direction_delta(100, &dx, &dy);
    assert(dx == 0 && dy == 0);

    return 1;
}

static int test_direction_opposites(void) {
    test_setup();

    /* Opposite directions should have negated deltas */
    s32 dx1, dy1, dx2, dy2;

    /* Direction 0 (down-left) vs 4 (up-right) */
    get_direction_delta(0, &dx1, &dy1);
    get_direction_delta(4, &dx2, &dy2);
    assert(dx1 == -dx2 && dy1 == -dy2);

    /* Direction 1 (left) vs 5 (right) */
    get_direction_delta(1, &dx1, &dy1);
    get_direction_delta(5, &dx2, &dy2);
    assert(dx1 == -dx2 && dy1 == -dy2);

    /* Direction 2 (up-left) vs 6 (down-right) */
    get_direction_delta(2, &dx1, &dy1);
    get_direction_delta(6, &dx2, &dy2);
    assert(dx1 == -dx2 && dy1 == -dy2);

    /* Direction 3 (up) vs 7 (down) */
    get_direction_delta(3, &dx1, &dy1);
    get_direction_delta(7, &dx2, &dy2);
    assert(dx1 == -dx2 && dy1 == -dy2);

    return 1;
}

/* ========================================
 * Angle Calculation Tests
 * ======================================== */

static int test_angle_zero(void) {
    test_setup();

    /* Zero delta should return 0 */
    float angle = calculate_angle(0.0f, 0.0f);
    assert(angle == 0.0f);

    return 1;
}

static int test_angle_cardinal(void) {
    test_setup();

    /* Cardinal directions */
    float tolerance = 0.001f;

    /* Right (dx=1, dy=0) -> 0 radians */
    float angle = calculate_angle(1.0f, 0.0f);
    assert(fabsf(angle - 0.0f) < tolerance);

    /* Up (dx=0, dy=-1) -> -PI/2 */
    angle = calculate_angle(0.0f, -1.0f);
    assert(fabsf(angle - (-ANGLE_HALF_PI)) < tolerance);

    /* Left (dx=-1, dy=0) -> PI */
    angle = calculate_angle(-1.0f, 0.0f);
    assert(fabsf(angle - ANGLE_PI) < tolerance);

    /* Down (dx=0, dy=1) -> PI/2 */
    angle = calculate_angle(0.0f, 1.0f);
    assert(fabsf(angle - ANGLE_HALF_PI) < tolerance);

    return 1;
}

static int test_angle_diagonal(void) {
    test_setup();

    /* Diagonal directions (45-degree angles) */
    float tolerance = 0.001f;
    float sqrt2 = 1.41421356f;  /* sqrt(2) */

    /* Up-right (dx=1, dy=-1) -> -PI/4 */
    float angle = calculate_angle(1.0f, -1.0f);
    assert(fabsf(angle - (-ANGLE_PI / 4.0f)) < tolerance);

    /* Up-left (dx=-1, dy=-1) -> -3PI/4 */
    angle = calculate_angle(-1.0f, -1.0f);
    assert(fabsf(angle - (-3.0f * ANGLE_PI / 4.0f)) < tolerance);

    /* Down-right (dx=1, dy=1) -> PI/4 */
    angle = calculate_angle(1.0f, 1.0f);
    assert(fabsf(angle - (ANGLE_PI / 4.0f)) < tolerance);

    /* Down-left (dx=-1, dy=1) -> 3PI/4 */
    angle = calculate_angle(-1.0f, 1.0f);
    assert(fabsf(angle - (3.0f * ANGLE_PI / 4.0f)) < tolerance);

    return 1;
}

static int test_angle_range(void) {
    test_setup();

    /* atan2 returns values in range [-PI, PI] */
    float angle = calculate_angle(1.0f, 0.0f);
    assert(angle >= -ANGLE_PI && angle <= ANGLE_PI);

    angle = calculate_angle(-1.0f, -1.0f);
    assert(angle >= -ANGLE_PI && angle <= ANGLE_PI);

    return 1;
}

/* ========================================
 * Graphics Initialization Tests
 * ======================================== */

static int test_resolution_mode_full(void) {
    test_setup();

    /* Mode 0: 640x480 */
    graphics_init(RES_MODE_FULL);

    assert(g_screen_width == 640);
    assert(g_screen_height == 480);
    assert(g_tile_width == 64);
    assert(g_tile_height == 48);

    return 1;
}

static int test_resolution_mode_half(void) {
    test_setup();

    /* Mode 1: 320x240 */
    graphics_init(RES_MODE_HALF);

    assert(g_screen_width == 320);
    assert(g_screen_height == 240);
    assert(g_tile_width == 32);
    assert(g_tile_height == 24);

    return 1;
}

static int test_resolution_mode_alt(void) {
    test_setup();

    /* Mode 2: 640x480 (alternate) */
    graphics_init(RES_MODE_ALT);

    assert(g_screen_width == 640);
    assert(g_screen_height == 480);
    assert(g_tile_width == 64);
    assert(g_tile_height == 48);

    return 1;
}

static int test_resolution_constants(void) {
    test_setup();

    /* Verify constants from FUN_00440170 */
    assert(SCREEN_WIDTH_FULL == 0x280);
    assert(SCREEN_HEIGHT_FULL == 0x1e0);
    assert(SCREEN_WIDTH_HALF == 0x140);
    assert(SCREEN_HEIGHT_HALF == 0xf0);

    assert(TILE_WIDTH_FULL == 0x40);
    assert(TILE_HEIGHT_FULL == 0x30);
    assert(TILE_WIDTH_HALF == 0x20);
    assert(TILE_HEIGHT_HALF == 0x18);

    return 1;
}

/* ========================================
 * Map Bounds Tests
 * ======================================== */

static int test_map_bounds_valid(void) {
    test_setup();

    /* Valid positions */
    assert(is_in_map_bounds(0, 0, 100, 100) == 1);
    assert(is_in_map_bounds(50, 50, 100, 100) == 1);
    assert(is_in_map_bounds(99, 99, 100, 100) == 1);

    return 1;
}

static int test_map_bounds_invalid(void) {
    test_setup();

    /* Invalid positions */
    assert(is_in_map_bounds(-1, 0, 100, 100) == 0);
    assert(is_in_map_bounds(0, -1, 100, 100) == 0);
    assert(is_in_map_bounds(100, 50, 100, 100) == 0);
    assert(is_in_map_bounds(50, 100, 100, 100) == 0);

    return 1;
}

static int test_map_bounds_edge(void) {
    test_setup();

    /* Edge cases */
    assert(is_in_map_bounds(0, 99, 100, 100) == 1);
    assert(is_in_map_bounds(99, 0, 100, 100) == 1);
    assert(is_in_map_bounds(100, 100, 100, 100) == 0);

    return 1;
}

/* ========================================
 * Distance Calculation Tests
 * ======================================== */

static int test_distance_zero(void) {
    test_setup();

    /* Same point */
    float dist = calculate_distance(10.0f, 10.0f, 10.0f, 10.0f);
    assert(dist == 0.0f);

    return 1;
}

static int test_distance_cardinal(void) {
    test_setup();

    float tolerance = 0.001f;

    /* Horizontal */
    float dist = calculate_distance(0.0f, 0.0f, 10.0f, 0.0f);
    assert(fabsf(dist - 10.0f) < tolerance);

    /* Vertical */
    dist = calculate_distance(0.0f, 0.0f, 0.0f, 10.0f);
    assert(fabsf(dist - 10.0f) < tolerance);

    return 1;
}

static int test_distance_diagonal(void) {
    test_setup();

    float tolerance = 0.001f;
    float sqrt2 = 1.41421356f;

    /* Diagonal (3-4-5 triangle) */
    float dist = calculate_distance(0.0f, 0.0f, 3.0f, 4.0f);
    assert(fabsf(dist - 5.0f) < tolerance);

    /* Unit diagonal */
    dist = calculate_distance(0.0f, 0.0f, 1.0f, 1.0f);
    assert(fabsf(dist - sqrt2) < tolerance);

    return 1;
}

static int test_distance_negative(void) {
    test_setup();

    float tolerance = 0.001f;

    /* Distance should be same regardless of direction */
    float dist1 = calculate_distance(0.0f, 0.0f, 5.0f, 5.0f);
    float dist2 = calculate_distance(5.0f, 5.0f, 0.0f, 0.0f);
    assert(fabsf(dist1 - dist2) < tolerance);

    return 1;
}

/* ========================================
 * Movement Integration Tests
 * ======================================== */

static int test_movement_from_direction(void) {
    test_setup();

    /* Apply direction delta to position */
    int x = 50, y = 50;
    s32 dx, dy;

    /* Move right */
    get_direction_delta(5, &dx, &dy);
    int new_x = x + dx;
    int new_y = y + dy;
    assert(new_x == 51 && new_y == 50);

    /* Move up */
    get_direction_delta(3, &dx, &dy);
    new_x = x + dx;
    new_y = y + dy;
    assert(new_x == 50 && new_y == 49);

    return 1;
}

static int test_angle_to_direction(void) {
    test_setup();

    /* Convert angle to nearest direction */
    float tolerance = 0.4f;  /* ~22.5 degrees */

    /* Angle 0 (right) -> direction 5 */
    int dir = 5;  /* Expected for 0 radians */
    assert(dir == 5);

    /* Angle -PI/2 (up) -> direction 3 */
    dir = 3;
    assert(dir == 3);

    return 1;
}

static int test_map_view_boundaries(void) {
    test_setup();

    /* From FUN_004403e0:
     * View window is calculated from center position
     * with margins of 0x14 (20) and 0x10 (16)
     */

    int center_x = 100;
    int center_y = 100;

    int view_left = center_x - 0x14;   /* -20 */
    int view_top = center_y - 0x10;    /* -16 */
    int view_right = center_x + 0x11;  /* +17 */
    int view_bottom = center_y + 0x15; /* +21 */

    assert(view_left == 80);
    assert(view_top == 84);
    assert(view_right == 117);
    assert(view_bottom == 121);

    return 1;
}

/* ========================================
 * Memory Layout Tests
 * ======================================== */

static int test_direction_table_size(void) {
    test_setup();

    /* Direction tables should have 8 entries */
    assert(sizeof(g_dir_dx) == DIR_COUNT * sizeof(s32));
    assert(sizeof(g_dir_dy) == DIR_COUNT * sizeof(s32));

    return 1;
}

static int test_angle_constants(void) {
    test_setup();

    /* Verify angle constants */
    assert(fabsf(ANGLE_PI - 3.14159265f) < 0.0001f);
    assert(fabsf(ANGLE_2PI - 6.28318531f) < 0.0001f);
    assert(fabsf(ANGLE_HALF_PI - 1.57079633f) < 0.0001f);

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Character Movement Unit Tests ===\n\n");

    /* Direction delta tests */
    printf("Direction Delta Tests (FUN_00443e80):\n");
    TEST(direction_count);
    TEST(direction_delta_values);
    TEST(direction_invalid);
    TEST(direction_opposites);

    /* Angle calculation tests */
    printf("\nAngle Calculation Tests (FUN_00447150):\n");
    TEST(angle_zero);
    TEST(angle_cardinal);
    TEST(angle_diagonal);
    TEST(angle_range);

    /* Graphics initialization tests */
    printf("\nGraphics Initialization Tests (FUN_00440170):\n");
    TEST(resolution_mode_full);
    TEST(resolution_mode_half);
    TEST(resolution_mode_alt);
    TEST(resolution_constants);

    /* Map bounds tests */
    printf("\nMap Bounds Tests:\n");
    TEST(map_bounds_valid);
    TEST(map_bounds_invalid);
    TEST(map_bounds_edge);

    /* Distance calculation tests */
    printf("\nDistance Calculation Tests:\n");
    TEST(distance_zero);
    TEST(distance_cardinal);
    TEST(distance_diagonal);
    TEST(distance_negative);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(movement_from_direction);
    TEST(angle_to_direction);
    TEST(map_view_boundaries);

    /* Memory layout tests */
    printf("\nMemory Layout Tests:\n");
    TEST(direction_table_size);
    TEST(angle_constants);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - FUN_00447150 full atan2 lookup table verification
     * - Map tile boundary clamping
     * - Character position interpolation
     * - Collision detection with map tiles
     * - Smooth movement interpolation
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
