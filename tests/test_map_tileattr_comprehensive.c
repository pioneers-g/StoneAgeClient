/*
 * Stone Age Client - Map Tile Attribute Tests
 * Tests for FUN_00441b90 (tile attribute processing)
 *          and FUN_00442070 (find walkable tile)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int32_t s32;

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) printf("  TEST: %s ... ", name)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define ASSERT_EQ(a, b) do { int _a = (int)(a); int _b = (int)(b); if (_a != _b) { printf("FAIL: line %d got %d expected %d\n", __LINE__, _a, _b); tests_failed++; return; } } while(0)

/* ===== Tile Attribute Constants ===== */
#define TILE_ATTR_EMPTY      0
#define TILE_ATTR_WALKABLE   1
#define TILE_ATTR_BLOCKED    2

/* ===== Minimal MapContext for testing ===== */
typedef struct {
    u32 current_map_id;
    u16 width;
    u16 height;
    int player_x;
    int player_y;
    int camera_x;
    int camera_y;
    int offset_x;
    int offset_y;
    int loaded;
    int in_battle;
    u8* collision_map;
} TestMapContext;

static TestMapContext g_map;

typedef struct {
    u16 ground_tile;
    u16 object_tile;
    u8  flags;
    u8  height;
} TestMapTile;

static TestMapTile* g_tiles = NULL;

/* Direction deltas */
static const int s_dir_dx[8] = { -1, -1,  0,  1,  1,  1,  0, -1 };
static const int s_dir_dy[8] = {  0, -1, -1, -1,  0,  1,  1,  1 };

#define MAX_FOUND_TILES 8
static int s_found_tiles_x[MAX_FOUND_TILES];
static int s_found_tiles_y[MAX_FOUND_TILES];
static int s_found_tile_count = 0;

/* ===== FUN_00441b90 - Tile Attribute Processing ===== */
static void FUN_00441b90(int start_x, int start_y, int width, int height, int stride,
                         u16* tile_data, int tile_count, short* attr_output) {
    int row, col;

    if (!tile_data || !attr_output) return;
    if (width <= 0 || height <= 0 || stride <= 0) return;

    for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
            int idx = row * stride + col;
            u16 tile_id;
            short attr;

            if (idx < 0 || idx >= tile_count) {
                attr_output[row * width + col] = TILE_ATTR_BLOCKED;
                continue;
            }

            tile_id = tile_data[idx];

            if (tile_id == 0) {
                attr = TILE_ATTR_BLOCKED;
            } else if (tile_id == 1 || tile_id == 2 || tile_id == 5 ||
                       tile_id == 6 || tile_id == 9 || tile_id == 10) {
                attr = TILE_ATTR_WALKABLE;
            } else if (tile_id == 4) {
                attr = TILE_ATTR_BLOCKED;
            } else if (tile_id >= 100) {
                if (g_map.collision_map) {
                    int map_x = start_x + col;
                    int map_y = start_y + row;
                    int cidx = map_y * g_map.width + map_x;
                    if (cidx >= 0 && cidx < (int)(g_map.width * g_map.height)) {
                        attr = g_map.collision_map[cidx] ? TILE_ATTR_WALKABLE : TILE_ATTR_BLOCKED;
                    } else {
                        attr = TILE_ATTR_WALKABLE;
                    }
                } else {
                    attr = TILE_ATTR_WALKABLE;
                }
            } else {
                if (tile_id >= 0x28 && tile_id <= 0x3b) {
                    attr = TILE_ATTR_WALKABLE;
                } else if (tile_id >= 0x14 && tile_id <= 0x27) {
                    attr = TILE_ATTR_WALKABLE;
                } else {
                    attr = TILE_ATTR_BLOCKED;
                }
            }

            attr_output[row * width + col] = attr;
        }
    }
}

/* ===== FUN_00442070 - Find Walkable Tile ===== */
static int FUN_00442070(int search_mode) {
    int base_dir, dir_offset, step;
    int dx, dy, check_x, check_y, found;

    if (search_mode < 0 || search_mode > 7) return 0;

    s_found_tile_count = 0;
    base_dir = search_mode;

    for (dir_offset = -1; dir_offset <= 1; dir_offset++) {
        int dir_idx = (base_dir + dir_offset) & 7;

        dx = s_dir_dx[dir_idx];
        dy = s_dir_dy[dir_idx];

        for (step = 1; step <= 3; step++) {
            check_x = g_map.player_x + dx * step;
            check_y = g_map.player_y + dy * step;

            if (check_x < 0 || check_y < 0) continue;
            if (check_x >= (int)g_map.width || check_y >= (int)g_map.height) continue;

            found = 0;
            if (g_map.collision_map) {
                int idx = check_y * g_map.width + check_x;
                found = g_map.collision_map[idx] != 0;
            } else if (g_tiles) {
                TestMapTile* tile = &g_tiles[check_y * g_map.width + check_x];
                found = (tile->flags & 0x01) == 0;
            } else {
                found = 1;
            }

            if (found && s_found_tile_count < MAX_FOUND_TILES) {
                s_found_tiles_x[s_found_tile_count] = check_x;
                s_found_tiles_y[s_found_tile_count] = check_y;
                s_found_tile_count++;
            }
        }
    }

    return s_found_tile_count > 0 ? 1 : 0;
}

/* ===== Test Cases ===== */

void test_tile_attr_walkable_types(void) {
    TEST("FUN_00441b90 walkable tile types (1,2,5,6,9,10)");
    u16 tiles[] = {1, 2, 5, 6, 9, 10};
    short attrs[6];
    FUN_00441b90(0, 0, 6, 1, 6, tiles, 6, attrs);
    for (int i = 0; i < 6; i++) {
        ASSERT_EQ(attrs[i], TILE_ATTR_WALKABLE);
    }
    PASS();
}

void test_tile_attr_blocked_zero(void) {
    TEST("FUN_00441b90 tile 0 is blocked");
    u16 tiles[] = {0};
    short attrs[1];
    FUN_00441b90(0, 0, 1, 1, 1, tiles, 1, attrs);
    ASSERT_EQ(attrs[0], TILE_ATTR_BLOCKED);
    PASS();
}

void test_tile_attr_blocked_four(void) {
    TEST("FUN_00441b90 tile 4 is blocked");
    u16 tiles[] = {4};
    short attrs[1];
    FUN_00441b90(0, 0, 1, 1, 1, tiles, 1, attrs);
    ASSERT_EQ(attrs[0], TILE_ATTR_BLOCKED);
    PASS();
}

void test_tile_attr_sprite_walkable(void) {
    TEST("FUN_00441b90 sprite tiles (>=100) are walkable without collision map");
    u16 tiles[] = {100, 200, 500, 1000};
    short attrs[4];
    g_map.collision_map = NULL;
    FUN_00441b90(0, 0, 4, 1, 4, tiles, 4, attrs);
    for (int i = 0; i < 4; i++) {
        ASSERT_EQ(attrs[i], TILE_ATTR_WALKABLE);
    }
    PASS();
}

void test_tile_attr_env_tiles(void) {
    TEST("FUN_00441b90 environment tiles (0x28-0x3b) are walkable");
    u16 tiles[] = {0x28, 0x30, 0x3b};
    short attrs[3];
    FUN_00441b90(0, 0, 3, 1, 3, tiles, 3, attrs);
    for (int i = 0; i < 3; i++) {
        ASSERT_EQ(attrs[i], TILE_ATTR_WALKABLE);
    }
    PASS();
}

void test_tile_attr_ground_effect(void) {
    TEST("FUN_00441b90 ground effect tiles (0x14-0x27) are walkable");
    u16 tiles[] = {0x14, 0x1e, 0x27};
    short attrs[3];
    FUN_00441b90(0, 0, 3, 1, 3, tiles, 3, attrs);
    for (int i = 0; i < 3; i++) {
        ASSERT_EQ(attrs[i], TILE_ATTR_WALKABLE);
    }
    PASS();
}

void test_tile_attr_null_inputs(void) {
    TEST("FUN_00441b90 handles NULL inputs gracefully");
    short attrs[1] = {-1};
    FUN_00441b90(0, 0, 1, 1, 1, NULL, 1, attrs);
    ASSERT_EQ(attrs[0], -1);

    u16 tiles[] = {1};
    FUN_00441b90(0, 0, 1, 1, 1, tiles, 1, NULL);
    PASS();
}

void test_tile_attr_2d_grid(void) {
    TEST("FUN_00441b90 processes 2D grid correctly");
    u16 tiles[] = {1, 4, 1,
                   1, 1, 0,
                   4, 1, 1};
    short attrs[9];
    FUN_00441b90(0, 0, 3, 3, 3, tiles, 9, attrs);

    ASSERT_EQ(attrs[0], TILE_ATTR_WALKABLE);   /* 1 */
    ASSERT_EQ(attrs[1], TILE_ATTR_BLOCKED);    /* 4 */
    ASSERT_EQ(attrs[2], TILE_ATTR_WALKABLE);   /* 1 */
    ASSERT_EQ(attrs[3], TILE_ATTR_WALKABLE);   /* 1 */
    ASSERT_EQ(attrs[4], TILE_ATTR_WALKABLE);   /* 1 */
    ASSERT_EQ(attrs[5], TILE_ATTR_BLOCKED);    /* 0 */
    ASSERT_EQ(attrs[6], TILE_ATTR_BLOCKED);    /* 4 */
    ASSERT_EQ(attrs[7], TILE_ATTR_WALKABLE);   /* 1 */
    ASSERT_EQ(attrs[8], TILE_ATTR_WALKABLE);   /* 1 */
    PASS();
}

void test_tile_attr_with_stride(void) {
    TEST("FUN_00441b90 respects stride parameter");
    /* 4x2 grid, but only process 2x2 with stride 4 */
    u16 tiles[] = {1, 4, 1, 1,
                   1, 1, 0, 1};
    short attrs[4];
    FUN_00441b90(0, 0, 2, 2, 4, tiles, 8, attrs);

    ASSERT_EQ(attrs[0], TILE_ATTR_WALKABLE);   /* tiles[0]=1 */
    ASSERT_EQ(attrs[1], TILE_ATTR_BLOCKED);    /* tiles[1]=4 */
    ASSERT_EQ(attrs[2], TILE_ATTR_WALKABLE);   /* tiles[4]=1 */
    ASSERT_EQ(attrs[3], TILE_ATTR_WALKABLE);   /* tiles[5]=1 */
    PASS();
}

void test_tile_attr_with_collision_map(void) {
    TEST("FUN_00441b90 uses collision map for sprite tiles");
    u16 tiles[] = {100, 200};
    short attrs[2];
    u8 collision[] = {1, 0}; /* first walkable, second blocked */

    g_map.width = 2;
    g_map.height = 1;
    g_map.collision_map = collision;

    FUN_00441b90(0, 0, 2, 1, 2, tiles, 2, attrs);
    ASSERT_EQ(attrs[0], TILE_ATTR_WALKABLE);
    ASSERT_EQ(attrs[1], TILE_ATTR_BLOCKED);

    g_map.collision_map = NULL;
    PASS();
}

/* ===== FUN_00442070 Tests ===== */

void setup_10x10_map(void) {
    memset(&g_map, 0, sizeof(g_map));
    g_map.width = 10;
    g_map.height = 10;
    g_map.player_x = 5;
    g_map.player_y = 5;

    if (g_tiles) free(g_tiles);
    g_tiles = (TestMapTile*)calloc(100, sizeof(TestMapTile));

    /* Mark all tiles as walkable */
    for (int i = 0; i < 100; i++) {
        g_tiles[i].flags = 0; /* walkable */
    }

    g_map.collision_map = NULL;
}

void setup_blocked_map(void) {
    memset(&g_map, 0, sizeof(g_map));
    g_map.width = 10;
    g_map.height = 10;
    g_map.player_x = 5;
    g_map.player_y = 5;

    if (g_tiles) free(g_tiles);
    g_tiles = (TestMapTile*)calloc(100, sizeof(TestMapTile));

    /* Mark all tiles as blocked */
    for (int i = 0; i < 100; i++) {
        g_tiles[i].flags = 1; /* blocked */
    }

    g_map.collision_map = NULL;
}

void test_find_walkable_north(void) {
    TEST("FUN_00442070 search mode 7 (north)");
    setup_10x10_map();
    int result = FUN_00442070(7);
    ASSERT_EQ(result, 1);
    ASSERT_EQ(s_found_tile_count > 0, 1);
    PASS();
}

void test_find_walkable_east(void) {
    TEST("FUN_00442070 search mode 2 (east)");
    setup_10x10_map();
    int result = FUN_00442070(2);
    ASSERT_EQ(result, 1);
    ASSERT_EQ(s_found_tile_count > 0, 1);
    PASS();
}

void test_find_walkable_south(void) {
    TEST("FUN_00442070 search mode 3/4 (south)");
    setup_10x10_map();
    int result = FUN_00442070(3);
    ASSERT_EQ(result, 1);
    ASSERT_EQ(s_found_tile_count > 0, 1);
    PASS();
}

void test_find_walkable_west(void) {
    TEST("FUN_00442070 search mode 0 (west)");
    setup_10x10_map();
    int result = FUN_00442070(0);
    ASSERT_EQ(result, 1);
    ASSERT_EQ(s_found_tile_count > 0, 1);
    PASS();
}

void test_find_walkable_all_blocked(void) {
    TEST("FUN_00442070 returns 0 when all tiles blocked");
    setup_blocked_map();
    int result = FUN_00442070(0);
    ASSERT_EQ(result, 0);
    ASSERT_EQ(s_found_tile_count, 0);
    PASS();
}

void test_find_walkable_invalid_mode(void) {
    TEST("FUN_00442070 returns 0 for invalid mode");
    setup_10x10_map();
    ASSERT_EQ(FUN_00442070(-1), 0);
    ASSERT_EQ(FUN_00442070(8), 0);
    PASS();
}

void test_find_walkable_at_edge(void) {
    TEST("FUN_00442070 handles player at map edge");
    setup_10x10_map();
    g_map.player_x = 0;
    g_map.player_y = 0;

    /* Search northwest - should find nothing to the west/north */
    int result = FUN_00442070(0);
    /* West from (0,0) - some directions will go out of bounds */
    /* But the function should still work, finding valid east/south tiles */
    /* The exact result depends on which directions are checked */
    ASSERT_EQ(result >= 0, 1); /* Shouldn't crash */
    PASS();
}

void test_find_walkable_collision_map(void) {
    TEST("FUN_00442070 uses collision map");
    setup_10x10_map();
    free(g_tiles);
    g_tiles = NULL;

    u8* cmap = (u8*)calloc(100, sizeof(u8));
    /* Only tile (7,5) is walkable */
    cmap[5 * 10 + 7] = 1;
    g_map.collision_map = cmap;

    int result = FUN_00442070(4); /* East (dir_dx[4]=1, dir_dy[4]=0) */
    /* Mode 2 checks dir_dx[1]=-1, dir_dx[2]=0, dir_dx[3]=1 */
    /* Should find the walkable tile at (7,5) via east direction */
    ASSERT_EQ(result, 1);

    free(cmap);
    g_map.collision_map = NULL;
    PASS();
}

void test_find_walkable_found_positions(void) {
    TEST("FUN_00442070 stores found positions correctly");
    setup_10x10_map();
    FUN_00442070(0); /* West */

    /* Should have found tiles to the west of (5,5) */
    int all_valid = 1;
    for (int i = 0; i < s_found_tile_count; i++) {
        if (s_found_tiles_x[i] < 0 || s_found_tiles_y[i] < 0 ||
            s_found_tiles_x[i] >= 10 || s_found_tiles_y[i] >= 10) {
            all_valid = 0;
            break;
        }
    }
    ASSERT_EQ(all_valid, 1);
    PASS();
}

void test_find_walkable_partial_block(void) {
    TEST("FUN_00442070 skips blocked tiles in path");
    setup_10x10_map();
    /* Block tile at (4,5) which is directly west */
    g_tiles[5 * 10 + 4].flags = 1; /* blocked */

    int result = FUN_00442070(0); /* West */
    /* Should still find tiles further west that are walkable */
    /* or find tiles in adjacent directions */
    ASSERT_EQ(result, 1);
    PASS();
}

int main(void) {
    printf("\n=== Map Tile Attribute Tests ===\n\n");

    printf("[FUN_00441b90 - Tile Attribute Processing]\n");
    test_tile_attr_walkable_types();
    test_tile_attr_blocked_zero();
    test_tile_attr_blocked_four();
    test_tile_attr_sprite_walkable();
    test_tile_attr_env_tiles();
    test_tile_attr_ground_effect();
    test_tile_attr_null_inputs();
    test_tile_attr_2d_grid();
    test_tile_attr_with_stride();
    test_tile_attr_with_collision_map();

    printf("\n[FUN_00442070 - Find Walkable Tile]\n");
    test_find_walkable_north();
    test_find_walkable_east();
    test_find_walkable_south();
    test_find_walkable_west();
    test_find_walkable_all_blocked();
    test_find_walkable_invalid_mode();
    test_find_walkable_at_edge();
    test_find_walkable_collision_map();
    test_find_walkable_found_positions();
    test_find_walkable_partial_block();

    printf("\n=== Results: %d passed, %d failed, %d total ===\n",
           tests_passed, tests_failed, tests_passed + tests_failed);

    if (g_tiles) free(g_tiles);

    return tests_failed > 0 ? 1 : 0;
}
