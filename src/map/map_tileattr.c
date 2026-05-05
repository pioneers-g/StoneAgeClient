/*
 * Stone Age Client - Map Tile Attribute Processing
 * Reverse engineered from sa_9061.exe
 *
 * FUN_00441b90 - Map tile attribute processing for collision/walkability
 * FUN_00442070 - Find walkable tiles around a position
 */

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "map.h"

/* Tile attribute constants - from FUN_00441b90 binary analysis */
#define TILE_ATTR_EMPTY      0
#define TILE_ATTR_WALKABLE   1
#define TILE_ATTR_BLOCKED    2

/* Direction deltas for 8-directional search in FUN_00442070 */
static const int s_dir_dx[8] = { -1, -1,  0,  1,  1,  1,  0, -1 };
static const int s_dir_dy[8] = {  0, -1, -1, -1,  0,  1,  1,  1 };

/* Global storage for found walkable positions - DAT_04560290-9c region */
#define MAX_FOUND_TILES 8
static int s_found_tiles_x[MAX_FOUND_TILES];  /* DAT_04560290 */
static int s_found_tiles_y[MAX_FOUND_TILES];  /* DAT_04560294 */
static int s_found_tile_count = 0;            /* DAT_045602c4 */

/* Global tile attribute arrays - DAT regions from FUN_00441b90 */
static u16* s_tile_attr_data = NULL;   /* param_6 equivalent */
static int s_tile_attr_count = 0;      /* param_7 equivalent */

/*
 * FUN_00441b90 - Map Tile Attribute Processing
 *
 * Binary analysis:
 * - Processes map tiles for collision/attribute data
 * - param_1: start X coordinate
 * - param_2: start Y coordinate
 * - param_3: width of area to process
 * - param_4: height of area to process
 * - param_5: stride (row width) of tile data
 * - param_6: input tile data array (tile IDs)
 * - param_7: size of tile data
 * - param_8: output attribute array
 *
 * Tile type classification:
 * - 0: Empty (check adjacent tile flag)
 * - 1, 2, 5, 6, 9, 10: Walkable
 * - 4: Blocked
 * - >= 100: Sprite tile (look up attributes)
 */
void FUN_00441b90(int start_x, int start_y, int width, int height, int stride,
                  unsigned short* tile_data, int tile_count, short* attr_output) {
    int row, col;
    int idx;

    if (!tile_data || !attr_output) return;
    if (width <= 0 || height <= 0 || stride <= 0) return;

    for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
            int map_x = start_x + col;
            int map_y = start_y + row;
            u16 tile_id;
            short attr;

            idx = row * stride + col;
            if (idx < 0 || idx >= tile_count) {
                attr_output[row * width + col] = TILE_ATTR_BLOCKED;
                continue;
            }

            tile_id = tile_data[idx];

            if (tile_id == 0) {
                /* Empty tile - blocked by default */
                attr = TILE_ATTR_BLOCKED;
            } else if (tile_id == 1 || tile_id == 2 || tile_id == 5 ||
                       tile_id == 6 || tile_id == 9 || tile_id == 10) {
                /* Known walkable tile types */
                attr = TILE_ATTR_WALKABLE;
            } else if (tile_id == 4) {
                /* Explicitly blocked tile */
                attr = TILE_ATTR_BLOCKED;
            } else if (tile_id >= 100) {
                /* Sprite tile - check collision data from map */
                if (g_map.collision_map) {
                    int collision_idx = map_y * g_map.width + map_x;
                    if (collision_idx >= 0 && collision_idx < (int)(g_map.width * g_map.height)) {
                        attr = g_map.collision_map[collision_idx] ? TILE_ATTR_WALKABLE : TILE_ATTR_BLOCKED;
                    } else {
                        attr = TILE_ATTR_WALKABLE;
                    }
                } else {
                    /* No collision data - assume walkable for sprite tiles */
                    attr = TILE_ATTR_WALKABLE;
                }
            } else {
                /* Other special tile types - check environment tiles */
                if (tile_id >= 0x28 && tile_id <= 0x3b) {
                    /* Environment effect tiles - walkable */
                    attr = TILE_ATTR_WALKABLE;
                } else if (tile_id >= 0x14 && tile_id <= 0x27) {
                    /* Ground effect tiles - walkable */
                    attr = TILE_ATTR_WALKABLE;
                } else {
                    attr = TILE_ATTR_BLOCKED;
                }
            }

            attr_output[row * width + col] = attr;
        }
    }
}

/*
 * FUN_00442070 - Find Walkable Tile
 *
 * Binary analysis:
 * - Finds walkable tiles around a center position
 * - param_1: search direction mode (0-7)
 *   - 0/1/2: Search left side (west, northwest, southwest)
 *   - 3/4: Search bottom (south)
 *   - 5/6: Search right side (east, northeast, southeast)
 *   - 7: Search top (north)
 * - Returns 1 if walkable tile found, 0 otherwise
 * - Updates s_found_tile_count with count of found tiles
 * - Stores found positions in s_found_tiles_x/y arrays
 *
 * Search pattern:
 * - Starts from player position
 * - Checks adjacent tiles in the specified direction
 * - Tests up to MAX_FOUND_TILES positions
 * - Uses collision map or tile attributes for walkability check
 */
int FUN_00442070(int search_mode) {
    int player_x, player_y;
    int base_dir, scan_range, dir_offset;
    int dx, dy;
    int check_x, check_y;
    int found;

    if (search_mode < 0 || search_mode > 7) return 0;

    s_found_tile_count = 0;
    player_x = g_map.player_x;
    player_y = g_map.player_y;

    /* Determine primary search direction based on mode */
    base_dir = search_mode;

    /* Scan range around player position */
    scan_range = 3;

    for (dir_offset = -1; dir_offset <= 1; dir_offset++) {
        int dir_idx = (base_dir + dir_offset) & 7;
        int step;

        dx = s_dir_dx[dir_idx];
        dy = s_dir_dy[dir_idx];

        for (step = 1; step <= scan_range; step++) {
            check_x = player_x + dx * step;
            check_y = player_y + dy * step;

            /* Check bounds */
            if (check_x < 0 || check_y < 0) continue;
            if (check_x >= (int)g_map.width || check_y >= (int)g_map.height) continue;

            /* Check walkability */
            found = 0;

            if (g_map.collision_map) {
                int idx = check_y * g_map.width + check_x;
                found = g_map.collision_map[idx] != 0;
            } else if (g_map.tiles) {
                MapTile* tile = map_get_tile(check_x, check_y);
                if (tile) {
                    found = (tile->flags & 0x01) == 0;
                }
            } else {
                /* No map data loaded - treat as walkable */
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

/* Accessor functions for found tile positions */
int map_get_found_tile_count(void) {
    return s_found_tile_count;
}

void map_get_found_tile(int index, int* x, int* y) {
    if (index < 0 || index >= s_found_tile_count) {
        if (x) *x = -1;
        if (y) *y = -1;
        return;
    }
    if (x) *x = s_found_tiles_x[index];
    if (y) *y = s_found_tiles_y[index];
}
