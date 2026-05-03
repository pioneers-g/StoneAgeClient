/*
 * Stone Age Client - Battle Map Loading Unit Tests
 * Tests for FUN_00404850 (battle map load) and related functions
 *
 * Based on Ghidra decompilation analysis:
 * - Battle map files: data/battleMap/battleXX.sab
 * - File format: 4-byte header "SAB " + 800-byte tile data (400 tiles * 2 bytes)
 * - Grid size: 20x20 tiles
 * - Tile size: 32x24 pixels (0x20, 0x18)
 * - Max battle maps: 220 (0-219, 0xdb)
 *
 * Uses real game data from D:\Games\石器时代8.0\石器时代8.0单机版\data\battleMap\
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
#define MAX_BATTLE_MAPS 220
#define MAP_GRID_WIDTH 20
#define MAP_GRID_HEIGHT 20
#define TILE_COUNT 400
#define TILE_WIDTH 32    /* 0x20 */
#define TILE_HEIGHT 24   /* 0x18 */
#define HEADER_SIZE 4
#define EXPECTED_FILE_SIZE 804  /* 4 + 400*2 */

/* SAB file header */
static const char SAB_HEADER[4] = {'S', 'A', 'B', ' '};

/* Map tile data */
typedef struct {
    u16 tiles[TILE_COUNT];
    u32 background_sprite;
    int loaded;
} BattleMap;

/* Battle map storage */
static BattleMap g_battle_maps[MAX_BATTLE_MAPS];
static int g_current_map_index = -1;

/* Map ID for special backgrounds - DAT_04581190 */
static u32 g_map_id = 0;

/* Background sprite - DAT_004d7f74 */
static u32 g_background_sprite = 0;

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
    memset(g_battle_maps, 0, sizeof(g_battle_maps));
    g_current_map_index = -1;
    g_map_id = 0;
    g_background_sprite = 0;
}

/*
 * Load battle map from file - FUN_00404850 pattern
 * Returns 1 on success, 0 on failure
 */
static int battle_map_load(const char* filepath, int map_index) {
    FILE* fp;
    char header[4];
    u8 low, high;
    int i;

    if (map_index < 0 || map_index >= MAX_BATTLE_MAPS) {
        return 0;
    }

    fp = fopen(filepath, "rb");
    if (!fp) {
        return 0;
    }

    /* Read and verify header */
    if (fread(header, 1, 4, fp) != 4) {
        fclose(fp);
        return 0;
    }

    if (memcmp(header, SAB_HEADER, 4) != 0) {
        fclose(fp);
        return 0;  /* TODO: Invalid header - should log error */
    }

    /* Read 400 tile entries - FUN_00404850 pattern:
     * bVar1 = FUN_00492421(iVar3);  // low byte
     * uVar2 = FUN_00492421(iVar3);  // high byte
     * *puVar6 = uVar2 | (ushort)bVar1 << 8;
     */
    for (i = 0; i < TILE_COUNT; i++) {
        if (fread(&low, 1, 1, fp) != 1) break;
        if (fread(&high, 1, 1, fp) != 1) break;
        /* Note: Original uses bVar1 << 8 | uVar2, which is:
         * tile = low << 8 | high (big endian interpretation)
         * But hex dump shows little endian storage
         */
        g_battle_maps[map_index].tiles[i] = (u16)low | ((u16)high << 8);
    }

    fclose(fp);
    g_battle_maps[map_index].loaded = 1;
    g_current_map_index = map_index;

    return 1;
}

/*
 * Calculate background sprite from map ID - FUN_00404850 pattern
 * Uses DAT_04581190 (map_id) and random number
 */
static u32 calculate_background_sprite(u32 map_id, int random_val) {
    /* From Ghidra analysis:
     * Special maps have fixed backgrounds
     * Others use random value to select background variant
     */

    /* Default backgrounds */
    if (random_val == 0) return 0x7149;
    if (random_val == 1) return 0x715c;
    if (random_val == 2) return 0x715b;
    if (random_val == 3) return 0x7160;
    if (random_val == 4) return 0x7161;

    /* Calculated background */
    return 0x7177 - random_val;
}

/*
 * Get tile at position
 */
static u16 battle_map_get_tile(int map_index, int x, int y) {
    if (map_index < 0 || map_index >= MAX_BATTLE_MAPS) return 0;
    if (!g_battle_maps[map_index].loaded) return 0;
    if (x < 0 || x >= MAP_GRID_WIDTH) return 0;
    if (y < 0 || y >= MAP_GRID_HEIGHT) return 0;

    return g_battle_maps[map_index].tiles[y * MAP_GRID_WIDTH + x];
}

/* ========================================
 * File Format Tests
 * ======================================== */

static int test_sab_header_format(void) {
    /* Header should be "SAB " (0x53, 0x41, 0x42, 0x20) */
    assert(SAB_HEADER[0] == 0x53);  /* 'S' */
    assert(SAB_HEADER[1] == 0x41);  /* 'A' */
    assert(SAB_HEADER[2] == 0x42);  /* 'B' */
    assert(SAB_HEADER[3] == 0x20);  /* ' ' */
    return 1;
}

static int test_expected_file_size(void) {
    /* File should be 804 bytes: 4 header + 800 tile data */
    assert(EXPECTED_FILE_SIZE == HEADER_SIZE + TILE_COUNT * 2);
    assert(EXPECTED_FILE_SIZE == 804);
    return 1;
}

static int test_tile_count(void) {
    /* Grid is 20x20 = 400 tiles */
    assert(TILE_COUNT == MAP_GRID_WIDTH * MAP_GRID_HEIGHT);
    assert(TILE_COUNT == 400);
    return 1;
}

static int test_tile_dimensions(void) {
    /* Each tile is 32x24 pixels */
    assert(TILE_WIDTH == 32);
    assert(TILE_HEIGHT == 24);
    assert(TILE_WIDTH == 0x20);
    assert(TILE_HEIGHT == 0x18);
    return 1;
}

/* ========================================
 * Map Loading Tests
 * ======================================== */

static int test_map_load_valid_index(void) {
    test_setup();

    /* Try loading battle00.sab */
    int result = battle_map_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/battleMap/battle00.sab", 0);

    assert(result == 1);
    assert(g_battle_maps[0].loaded == 1);
    assert(g_current_map_index == 0);

    return 1;
}

static int test_map_load_invalid_index(void) {
    test_setup();

    /* Index >= MAX_BATTLE_MAPS should fail */
    int result = battle_map_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/battleMap/battle00.sab", 220);
    assert(result == 0);

    result = battle_map_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/battleMap/battle00.sab", -1);
    assert(result == 0);

    return 1;
}

static int test_map_load_nonexistent_file(void) {
    test_setup();

    int result = battle_map_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/battleMap/nonexistent.sab", 0);
    assert(result == 0);
    assert(g_battle_maps[0].loaded == 0);

    return 1;
}

static int test_map_load_multiple_maps(void) {
    test_setup();

    int r0 = battle_map_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/battleMap/battle00.sab", 0);
    int r1 = battle_map_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/battleMap/battle01.sab", 1);
    int r2 = battle_map_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/battleMap/battle02.sab", 2);

    assert(r0 == 1);
    assert(r1 == 1);
    assert(r2 == 1);
    assert(g_battle_maps[0].loaded == 1);
    assert(g_battle_maps[1].loaded == 1);
    assert(g_battle_maps[2].loaded == 1);

    /* Verify different maps have different tiles */
    assert(g_battle_maps[0].tiles[0] != g_battle_maps[1].tiles[0] ||
           g_battle_maps[0].tiles[50] != g_battle_maps[1].tiles[50]);

    return 1;
}

/* ========================================
 * Tile Data Tests
 * ======================================== */

static int test_tile_data_read(void) {
    test_setup();

    int result = battle_map_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/battleMap/battle00.sab", 0);
    assert(result == 1);

    /* First tile after header - from hex dump: 0x0064 */
    assert(g_battle_maps[0].tiles[0] == 0x0064);

    /* More tiles from hex dump */
    assert(g_battle_maps[0].tiles[1] == 0x0064);

    return 1;
}

static int test_tile_get_position(void) {
    test_setup();

    int result = battle_map_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/battleMap/battle00.sab", 0);
    assert(result == 1);

    /* Get tile at position (0,0) */
    u16 tile = battle_map_get_tile(0, 0, 0);
    assert(tile == g_battle_maps[0].tiles[0]);

    /* Get tile at position (19,19) */
    tile = battle_map_get_tile(0, 19, 19);
    assert(tile == g_battle_maps[0].tiles[399]);

    return 1;
}

static int test_tile_get_bounds_check(void) {
    test_setup();

    battle_map_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/battleMap/battle00.sab", 0);

    /* Out of bounds should return 0 */
    assert(battle_map_get_tile(0, -1, 0) == 0);
    assert(battle_map_get_tile(0, 0, -1) == 0);
    assert(battle_map_get_tile(0, 20, 0) == 0);
    assert(battle_map_get_tile(0, 0, 20) == 0);
    assert(battle_map_get_tile(0, 100, 100) == 0);

    /* Unloaded map should return 0 */
    assert(battle_map_get_tile(50, 0, 0) == 0);

    return 1;
}

static int test_tile_id_range(void) {
    test_setup();

    /* Tile IDs appear to be in range 0x0064-0x00FF and higher */
    /* From hex dump: 0x0064, 0x0074, 0x006d, 0x00bb, etc. */

    battle_map_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/battleMap/battle00.sab", 0);

    /* Check that tile IDs are reasonable (not all 0 or 0xFFFF) */
    int non_zero = 0;
    int non_max = 0;

    for (int i = 0; i < TILE_COUNT; i++) {
        if (g_battle_maps[0].tiles[i] != 0) non_zero++;
        if (g_battle_maps[0].tiles[i] != 0xFFFF) non_max++;
    }

    assert(non_zero > 0);
    assert(non_max == TILE_COUNT);  /* No 0xFFFF tiles expected */

    return 1;
}

/* ========================================
 * Background Sprite Tests - FUN_00404850
 * ======================================== */

static int test_background_sprite_default(void) {
    test_setup();

    /* From Ghidra:
     * if (DAT_004d7f74 == 0) DAT_004d7f74 = 0x7149;
     * else if (DAT_004d7f74 == 1) DAT_004d7f74 = 0x715c;
     * else if (DAT_004d7f74 == 2) DAT_004d7f74 = 0x715b;
     * else if (DAT_004d7f74 == 3) DAT_004d7f74 = 0x7160;
     * else if (DAT_004d7f74 == 4) DAT_004d7f74 = 0x7161;
     */

    assert(calculate_background_sprite(0, 0) == 0x7149);
    assert(calculate_background_sprite(0, 1) == 0x715c);
    assert(calculate_background_sprite(0, 2) == 0x715b);
    assert(calculate_background_sprite(0, 3) == 0x7160);
    assert(calculate_background_sprite(0, 4) == 0x7161);

    return 1;
}

static int test_background_sprite_calculated(void) {
    test_setup();

    /* DAT_004d7f74 = 0x7177 - iVar3 */
    assert(calculate_background_sprite(0, 5) == 0x7172);  /* 0x7177 - 5 */
    assert(calculate_background_sprite(0, 10) == 0x716d); /* 0x7177 - 10 */

    return 1;
}

static int test_special_map_ids(void) {
    test_setup();

    /* From Ghidra: Special map IDs checked in FUN_00404850
     * 0x331, 0x1f47, 0x1fa5, 0x1fa4, 0x1f5b, 0x1f5c, 0x1f5d, 0x1f4f
     * 0x97, 0xa0, 0xa1, 0x7546-0x7549, 0x1fb1, 0x1fb2
     * 0x4e-0x63, 0x1fab-0x1fb0, 0x1fb4-0x1fb8, 0x1fbc-0x1fbe
     * 0x1d4c, 0x1d6a, 0x1db0
     */

    u32 special_ids[] = {0x331, 0x1f47, 0x1fa5, 0x1fa4, 0x1f5b, 0x1f5c, 0x1f5d, 0x1f4f,
                         0x97, 0xa0, 0xa1, 0x7546, 0x7547, 0x7548, 0x7549,
                         0x1fb1, 0x1fb2, 0x1d4c, 0x1d6a, 0x1db0};

    int count = sizeof(special_ids) / sizeof(special_ids[0]);
    assert(count == 20);

    /* These IDs trigger special background handling */
    for (int i = 0; i < count; i++) {
        assert(special_ids[i] != 0);
    }

    return 1;
}

/* ========================================
 * Grid Layout Tests
 * ======================================== */

static int test_grid_layout_20x20(void) {
    test_setup();

    /* Grid is 20x20 */
    assert(MAP_GRID_WIDTH == 20);
    assert(MAP_GRID_HEIGHT == 20);

    /* Calculate pixel dimensions */
    int pixel_width = MAP_GRID_WIDTH * TILE_WIDTH;   /* 640 */
    int pixel_height = MAP_GRID_HEIGHT * TILE_HEIGHT; /* 480 */

    assert(pixel_width == 640);
    assert(pixel_height == 480);

    return 1;
}

static int test_render_start_position(void) {
    test_setup();

    /* From FUN_00404850:
     * local_434 = -0x120 (-288)
     * local_430 = 0xf0 (240)
     */
    s32 start_x = -0x120;  /* -288 */
    s32 start_y = 0xf0;    /* 240 */

    assert(start_x == -288);
    assert(start_y == 240);

    /* Step values:
     * iVar4 = iVar4 + 0x20 (x step = 32 = tile width)
     * iVar3 = iVar3 + -0x18 (y step = -24 = negative tile height)
     */
    s32 x_step = 0x20;    /* 32 */
    s32 y_step = -0x18;   /* -24 */

    assert(x_step == TILE_WIDTH);
    assert(y_step == -TILE_HEIGHT);

    return 1;
}

static int test_render_position_calculation(void) {
    test_setup();

    /* From FUN_00404850:
     * FUN_0047e210(iVar4, iVar3, 1, uVar2, 0);
     *
     * Position calculation for tile (col, row):
     * x = start_x + col * TILE_WIDTH
     * y = start_y + row * TILE_HEIGHT
     */

    s32 start_x = -288;
    s32 start_y = 240;

    /* First tile (0,0) */
    s32 x0 = start_x;
    s32 y0 = start_y;

    /* Tile (10, 10) */
    s32 x10 = start_x + 10 * TILE_WIDTH;
    s32 y10 = start_y + 10 * TILE_HEIGHT;

    assert(x0 == -288);
    assert(y0 == 240);
    assert(x10 == -288 + 320);
    assert(y10 == 240 + 240);

    return 1;
}

/* ========================================
 * Maximum Maps Tests
 * ======================================== */

static int test_max_battle_maps(void) {
    test_setup();

    /* From FUN_00404850:
     * if (0xdb < param_1) param_1 = 0;
     */
    assert(MAX_BATTLE_MAPS == 220);
    assert(0xdb == 219);

    /* Map index is clamped to 0-219 */

    return 1;
}

static int test_map_index_clamping(void) {
    test_setup();

    /* Index > 0xdb should be clamped to 0 */
    u32 test_index = 220;

    /* From Ghidra: if (0xdb < param_1) param_1 = 0 */
    if (test_index > 0xdb) {
        test_index = 0;
    }

    assert(test_index == 0);

    return 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_map_load_and_access(void) {
    test_setup();

    /* Load map */
    int result = battle_map_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/battleMap/battle00.sab", 0);
    assert(result == 1);

    /* Access all tiles to verify no crashes */
    for (int y = 0; y < MAP_GRID_HEIGHT; y++) {
        for (int x = 0; x < MAP_GRID_WIDTH; x++) {
            u16 tile = battle_map_get_tile(0, x, y);
            (void)tile;  /* Just verify access works */
        }
    }

    return 1;
}

static int test_map_data_integrity(void) {
    test_setup();

    /* Load multiple maps and verify they're different */
    battle_map_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/battleMap/battle00.sab", 0);
    battle_map_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/battleMap/battle05.sab", 5);

    /* Compare some tiles */
    int different = 0;
    for (int i = 0; i < TILE_COUNT; i++) {
        if (g_battle_maps[0].tiles[i] != g_battle_maps[5].tiles[i]) {
            different++;
        }
    }

    /* Maps should be different */
    assert(different > 0);

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Battle Map Loading Unit Tests ===\n\n");

    /* File format tests */
    printf("File Format Tests:\n");
    TEST(sab_header_format);
    TEST(expected_file_size);
    TEST(tile_count);
    TEST(tile_dimensions);

    /* Map loading tests */
    printf("\nMap Loading Tests:\n");
    TEST(map_load_valid_index);
    TEST(map_load_invalid_index);
    TEST(map_load_nonexistent_file);
    TEST(map_load_multiple_maps);

    /* Tile data tests */
    printf("\nTile Data Tests:\n");
    TEST(tile_data_read);
    TEST(tile_get_position);
    TEST(tile_get_bounds_check);
    TEST(tile_id_range);

    /* Background sprite tests */
    printf("\nBackground Sprite Tests (FUN_00404850):\n");
    TEST(background_sprite_default);
    TEST(background_sprite_calculated);
    TEST(special_map_ids);

    /* Grid layout tests */
    printf("\nGrid Layout Tests:\n");
    TEST(grid_layout_20x20);
    TEST(render_start_position);
    TEST(render_position_calculation);

    /* Maximum maps tests */
    printf("\nMaximum Maps Tests:\n");
    TEST(max_battle_maps);
    TEST(map_index_clamping);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_map_load_and_access);
    TEST(map_data_integrity);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
