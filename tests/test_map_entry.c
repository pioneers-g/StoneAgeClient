/*
 * Stone Age Client - Unit Tests for Map Entry Functions
 * Test file: test_map_entry.c
 *
 * Tests FUN_00440dd0, FUN_00440df0, FUN_00440530, FUN_0040f650
 * These functions handle map entry from binary packet dispatcher case 0x43
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/map.h"

/* ========================================
 * Test Cases for Map Entry Functions
 * ======================================== */

/*
 * Test 1: Map ID setting
 */
static void test_map_set_id(void) {
    TEST_BEGIN("Map set ID");

    /* Initialize map system */
    map_init();

    /* Set map ID */
    map_set_map_id(12345);

    /* Verify map ID was stored */
    TEST_ASSERT(g_map.current_map_id == 12345, "Map ID should be stored");

    /* Cleanup */
    map_shutdown();

    TEST_END();
}

/*
 * Test 2: Coordinate setting
 */
static void test_map_set_coordinates(void) {
    TEST_BEGIN("Map set coordinates");

    map_init();

    /* Set player coordinates */
    map_set_coordinates(100, 200);

    /* Verify coordinates */
    TEST_ASSERT(g_map.player_x == 100, "Player X should be 100");
    TEST_ASSERT(g_map.player_y == 200, "Player Y should be 200");

    /* Verify map is marked as loaded */
    TEST_ASSERT(g_map.loaded == 1, "Map should be marked as loaded");

    map_shutdown();

    TEST_END();
}

/*
 * Test 3: Coordinate initialization clears previous state
 */
static void test_map_coordinate_reset(void) {
    TEST_BEGIN("Map coordinate reset");

    map_init();

    /* Set initial coordinates */
    map_set_coordinates(50, 50);

    /* Change to new coordinates */
    map_set_coordinates(150, 250);

    /* Verify new coordinates */
    TEST_ASSERT(g_map.player_x == 150, "Player X should update to 150");
    TEST_ASSERT(g_map.player_y == 250, "Player Y should update to 250");

    map_shutdown();

    TEST_END();
}

/*
 * Test 4: Clear pets
 */
static void test_map_clear_pets(void) {
    TEST_BEGIN("Map clear pets");

    map_init();

    /* Set in_battle flag */
    g_map.in_battle = 1;

    /* Clear pets */
    map_clear_all_pets();

    /* Verify battle flag cleared */
    TEST_ASSERT(g_map.in_battle == 0, "Battle flag should be cleared");

    map_shutdown();

    TEST_END();
}

/*
 * Test 5: Map context initialization
 */
static void test_map_context_init(void) {
    TEST_BEGIN("Map context init");

    /* Initialize should zero all fields */
    map_init();

    TEST_ASSERT(g_map.tiles == NULL, "Tiles should be NULL after init");
    TEST_ASSERT(g_map.objects == NULL, "Objects should be NULL after init");
    TEST_ASSERT(g_map.tile_count == 0, "Tile count should be 0");
    TEST_ASSERT(g_map.width == 0, "Width should be 0");
    TEST_ASSERT(g_map.height == 0, "Height should be 0");

    map_shutdown();

    TEST_END();
}

/*
 * Test 6: Boundary coordinate values
 */
static void test_map_boundary_coords(void) {
    TEST_BEGIN("Map boundary coordinates");

    map_init();

    /* Test with zero coordinates */
    map_set_coordinates(0, 0);
    TEST_ASSERT(g_map.player_x == 0, "Should handle X=0");
    TEST_ASSERT(g_map.player_y == 0, "Should handle Y=0");

    /* Test with large coordinates */
    map_set_coordinates(10000, 10000);
    TEST_ASSERT(g_map.player_x == 10000, "Should handle large X");
    TEST_ASSERT(g_map.player_y == 10000, "Should handle large Y");

    map_shutdown();

    TEST_END();
}

/*
 * Test 7: Camera position from coordinates
 */
static void test_map_camera_from_coords(void) {
    TEST_BEGIN("Map camera from coordinates");

    map_init();

    /* Set coordinates and verify camera is set */
    map_set_coordinates(100, 100);

    /* Camera should be scaled version of coordinates */
    /* Expected: 100 * 32 = 3200 for X, 100 * 24 = 2400 for Y */
    TEST_ASSERT(g_map.camera_x == 3200, "Camera X should be scaled");
    TEST_ASSERT(g_map.camera_y == 2400, "Camera Y should be scaled");
    TEST_ASSERT(g_map.target_camera_x == 3200, "Target camera X should match");
    TEST_ASSERT(g_map.target_camera_y == 2400, "Target camera Y should match");

    map_shutdown();

    TEST_END();
}

/*
 * Test 8: Map load data with invalid path
 */
static void test_map_load_data_invalid(void) {
    TEST_BEGIN("Map load data invalid");

    map_init();

    /* Try to load non-existent map data */
    /* Should not crash, just return without error */
    map_load_map_data(99999, 100, 100);

    /* No assertion needed - just checking it doesn't crash */
    TEST_ASSERT(1, "Should handle invalid map data gracefully");

    map_shutdown();

    TEST_END();
}

/*
 * Test 9: Multiple map entries
 */
static void test_map_multiple_entries(void) {
    TEST_BEGIN("Map multiple entries");

    map_init();

    /* Simulate multiple map entries */
    for (int i = 0; i < 5; i++) {
        map_set_map_id(i * 100);
        map_set_coordinates(i * 10, i * 20);
    }

    /* Final values should be from last iteration */
    TEST_ASSERT(g_map.current_map_id == 400, "Map ID should be 400");
    TEST_ASSERT(g_map.player_x == 40, "Player X should be 40");
    TEST_ASSERT(g_map.player_y == 80, "Player Y should be 80");

    map_shutdown();

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(map_entry) {
    test_map_set_id();
    test_map_set_coordinates();
    test_map_coordinate_reset();
    test_map_clear_pets();
    test_map_context_init();
    test_map_boundary_coords();
    test_map_camera_from_coords();
    test_map_load_data_invalid();
    test_map_multiple_entries();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - Map Entry Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(map_entry);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
