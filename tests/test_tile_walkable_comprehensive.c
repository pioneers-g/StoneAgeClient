/*
 * Stone Age Client - Tile Walkable Check Function Unit Tests
 * Tests for FUN_00443510 (check if tile is walkable)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* ========================================
 * Test Data and Constants
 * ======================================== */

#define MAP_WIDTH 100
#define MAP_SIZE 1000

static u16 DAT_04581280[MAP_SIZE];
static s32 DAT_045602b0 = MAP_WIDTH;
static s32 DAT_04560e3c = 0;
static s32 DAT_04560e44 = 0;

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

/*
 * FUN_00443510 - Check if Tile is Walkable
 */
static int FUN_00443510(int x, int y) {
    int index = ((y - DAT_04560e3c) * DAT_045602b0 - DAT_04560e44) + x;
    u16 tile_data = DAT_04581280[index];
    return (tile_data & 0xfff) == 2;
}

/* ========================================
 * Helper Functions
 * ======================================== */

static void clear_map(void) {
    memset(DAT_04581280, 0, sizeof(DAT_04581280));
}

static void set_tile(int x, int y, u16 value) {
    int index = y * DAT_045602b0 + x;
    if (index >= 0 && index < MAP_SIZE) {
        DAT_04581280[index] = value;
    }
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(walkable_tile_type_2) {
    clear_map();
    set_tile(5, 5, 2);
    ASSERT(FUN_00443510(5, 5) == 1);
}

TEST(non_walkable_tile_type_0) {
    clear_map();
    set_tile(5, 5, 0);
    ASSERT(FUN_00443510(5, 5) == 0);
}

TEST(non_walkable_tile_type_1) {
    clear_map();
    set_tile(5, 5, 1);
    ASSERT(FUN_00443510(5, 5) == 0);
}

TEST(non_walkable_tile_type_3) {
    clear_map();
    set_tile(5, 5, 3);
    ASSERT(FUN_00443510(5, 5) == 0);
}

TEST(tile_with_high_bits) {
    clear_map();
    /* Tile type 2 with high bits set - should still be walkable */
    set_tile(5, 5, 0xF002);
    ASSERT(FUN_00443510(5, 5) == 1);
}

TEST(tile_with_flag_bits) {
    clear_map();
    /* Tile type 2 with flag bits in upper nibble */
    set_tile(5, 5, 0x1002);
    ASSERT(FUN_00443510(5, 5) == 1);
}

TEST(multiple_walkable_tiles) {
    clear_map();
    set_tile(0, 0, 2);
    set_tile(1, 0, 2);
    set_tile(0, 1, 2);
    set_tile(1, 1, 0);

    ASSERT(FUN_00443510(0, 0) == 1);
    ASSERT(FUN_00443510(1, 0) == 1);
    ASSERT(FUN_00443510(0, 1) == 1);
    ASSERT(FUN_00443510(1, 1) == 0);
}

TEST(tile_at_origin) {
    clear_map();
    set_tile(0, 0, 2);
    ASSERT(FUN_00443510(0, 0) == 1);
}

TEST(tile_at_edge) {
    clear_map();
    set_tile(99, 9, 2);
    ASSERT(FUN_00443510(99, 9) == 1);
}

TEST(tile_with_offset) {
    clear_map();
    DAT_04560e3c = 5;  /* Y offset */
    DAT_04560e44 = 3;  /* X offset */

    set_tile(8, 7, 2);  /* Physical position */

    /* Logical position should account for offset */
    /* index = (y - offsetY) * width - offsetX + x */
    int index = (7 - 5) * 100 - 3 + 8;
    ASSERT(index >= 0 && index < MAP_SIZE);

    /* Reset offsets */
    DAT_04560e3c = 0;
    DAT_04560e44 = 0;
}

TEST(empty_map_not_walkable) {
    clear_map();
    ASSERT(FUN_00443510(5, 5) == 0);
    ASSERT(FUN_00443510(0, 0) == 0);
    ASSERT(FUN_00443510(99, 9) == 0);
}

TEST(tile_type_masking) {
    clear_map();
    /* Test that only lower 12 bits are checked */
    set_tile(5, 5, 0xFFFF);
    ASSERT(FUN_00443510(5, 5) == 0);  /* 0xFFF = 4095, not 2 */

    set_tile(5, 5, 0x0002);
    ASSERT(FUN_00443510(5, 5) == 1);

    set_tile(5, 5, 0x1002);
    ASSERT(FUN_00443510(5, 5) == 1);
}

TEST(mixed_tile_types) {
    clear_map();
    int i, j;
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            /* Alternate between walkable and non-walkable */
            set_tile(i, j, (i + j) % 2 == 0 ? 2 : 1);
        }
    }

    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            int expected = (i + j) % 2 == 0 ? 1 : 0;
            ASSERT(FUN_00443510(i, j) == expected);
        }
    }
}

TEST(checkerboard_pattern) {
    clear_map();
    int i;
    for (i = 0; i < 100; i++) {
        set_tile(i, 0, (i % 2) + 1);  /* Types 1 and 2 alternating */
    }

    for (i = 0; i < 100; i++) {
        int expected = (i % 2 == 1) ? 1 : 0;  /* Odd indices have type 2 */
        ASSERT(FUN_00443510(i, 0) == expected);
    }
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Tile Walkable Check Function Unit Tests ===\n\n");

    printf("Basic Tile Type Tests:\n");
    RUN_TEST(walkable_tile_type_2);
    RUN_TEST(non_walkable_tile_type_0);
    RUN_TEST(non_walkable_tile_type_1);
    RUN_TEST(non_walkable_tile_type_3);

    printf("\nBit Masking Tests:\n");
    RUN_TEST(tile_with_high_bits);
    RUN_TEST(tile_with_flag_bits);
    RUN_TEST(tile_type_masking);

    printf("\nPosition Tests:\n");
    RUN_TEST(tile_at_origin);
    RUN_TEST(tile_at_edge);
    RUN_TEST(tile_with_offset);

    printf("\nPattern Tests:\n");
    RUN_TEST(multiple_walkable_tiles);
    RUN_TEST(empty_map_not_walkable);
    RUN_TEST(mixed_tile_types);
    RUN_TEST(checkerboard_pattern);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
