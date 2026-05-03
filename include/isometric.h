/*
 * Stone Age Client - Isometric Renderer
 * Reverse engineered from sa_9061.exe (FUN_004412e0, FUN_00446df0, FUN_00446e40)
 *
 * Handles isometric tile rendering with diamond iteration pattern
 */

#ifndef ISOMETRIC_H
#define ISOMETRIC_H

#include "types.h"

/* Isometric tile dimensions - from binary analysis */
#define ISO_TILE_WIDTH      64      /* 0x40 - tile width in pixels */
#define ISO_TILE_HEIGHT     48      /* 0x30 - tile height in pixels */
#define ISO_HALF_WIDTH      32      /* 0x20 - half tile width */
#define ISO_HALF_HEIGHT     24      /* 0x18 - half tile height */

/* Map dimensions */
#define MAP_MAX_WIDTH       256
#define MAP_MAX_HEIGHT      256

/* Camera structure - from DAT_04581d3c region */
typedef struct {
    float world_x;          /* DAT_04581d3c - world X coordinate */
    float world_y;          /* DAT_04581d40 - world Y coordinate */
    int screen_x;           /* DAT_04560e48 - screen X offset */
    int screen_y;           /* DAT_045602ac - screen Y offset */
    int prev_world_x;       /* DAT_004bb414 - previous world X */
    int prev_world_y;       /* DAT_004bb418 - previous world Y */
    int scroll_dx;          /* DAT_045829b4 - scroll delta X */
    int scroll_dy;          /* DAT_045829b8 - scroll delta Y */
    int scroll_prev_x;      /* DAT_045829bc - previous scroll X */
    int scroll_prev_y;      /* DAT_045829c0 - previous scroll Y */
} IsometricCamera;

/* Map tile data - from DAT_04581d48 region */
typedef struct {
    u16* ground_layer;      /* DAT_04581d48 - ground tiles */
    u16* object_layer;      /* DAT_0456a64c - object/overlay tiles */
    int width;              /* DAT_04560e44 - map width */
    int height;             /* DAT_04560e38 - map height */
    int stride;             /* DAT_045602b0 - map stride for indexing */
} MapTileData;

/* Environment effect types - from FUN_00488680 */
typedef enum {
    ENV_EFFECT_NONE = 0,
    ENV_EFFECT_RAIN = 0x28,
    ENV_EFFECT_SNOW = 0x29,
    ENV_EFFECT_FOG = 0x2A,
    ENV_EFFECT_SANDSTORM = 0x2B,
    ENV_EFFECT_LEAVES = 0x2C
} EnvironmentEffect;

/* Tile types - from FUN_004412e0 conditional checks */
typedef enum {
    TILE_TYPE_EMPTY = 0,
    TILE_TYPE_WALKABLE = 100,       /* >= 100: sprite tiles */
    TILE_TYPE_SPECIAL_MIN = 0x14,   /* 20-39: special rendering */
    TILE_TYPE_SPECIAL_MAX = 0x27,   /* 39: end of special range */
    TILE_TYPE_ENV_MIN = 0x28,       /* 40: environment effects start */
    TILE_TYPE_ENV_MAX = 0x3B        /* 59: environment effects end */
} TileType;

/* Global camera instance */
extern IsometricCamera g_camera;

/* Global map data reference */
extern MapTileData g_map_tiles;

/* Environment state */
extern u32 g_environment_type;      /* DAT_04633308 */
extern u32 g_environment_state;     /* DAT_04ebe278 */
extern u32 g_render_flag;           /* DAT_04ebe270 */

/* ========================================
 * Coordinate Transformations
 * ======================================== */

/*
 * World to screen coordinate transformation - FUN_00446df0
 * Converts (world_x, world_y) to (screen_x, screen_y)
 *
 * Formula from binary:
 *   screen_x = ((world_y - offset_y) + (world_x - offset_x)) * scale_x + center_x
 *   screen_y = ((world_y - offset_y) - (world_x - offset_x)) * scale_y + center_y
 */
void world_to_screen(float world_x, float world_y, float* screen_x, float* screen_y);

/*
 * Screen to world coordinate transformation - FUN_00446e40
 * Converts (screen_x, screen_y) to (world_x, world_y)
 *
 * Inverse of world_to_screen
 */
void screen_to_world(float screen_x, float screen_y, float* world_x, float* world_y);

/*
 * Tile to screen coordinate conversion
 * Converts tile coordinates to screen position
 */
void tile_to_screen(int tile_x, int tile_y, int* screen_x, int* screen_y);

/*
 * Screen to tile coordinate conversion
 */
void screen_to_tile(int screen_x, int screen_y, int* tile_x, int* tile_y);

/* ========================================
 * Map Rendering
 * ======================================== */

/*
 * Main isometric render function - FUN_004412e0
 * Renders the visible map area using diamond iteration
 */
void isometric_render(void);

/*
 * Render map tiles - part of FUN_004412e0
 * Renders ground and object layers
 */
void isometric_render_tiles(int start_x, int start_y, int width, int height);

/*
 * Render single tile
 */
void isometric_render_tile(u16 tile_id, int screen_x, int screen_y, int layer);

/* ========================================
 * Camera Control
 * ======================================== */

/*
 * Initialize camera
 */
void camera_init(void);

/*
 * Set camera position in world coordinates
 */
void camera_set_position(float world_x, float world_y);

/*
 * Update camera scroll state
 */
void camera_update_scroll(void);

/*
 * Get camera offset for rendering
 */
void camera_get_offset(int* offset_x, int* offset_y);

/* ========================================
 * Environment Effects
 * ======================================== */

/*
 * Process environment effect - FUN_00488680
 * Handles rain, snow, fog, etc.
 */
void environment_process(u32 effect_type);

/*
 * Check if tile is environment effect
 */
int is_environment_tile(u16 tile_id);

/*
 * Check if tile is special render type
 */
int is_special_tile(u16 tile_id);

/* ========================================
 * Map Data Access
 * ======================================== */

/*
 * Get tile at map position
 */
u16 map_get_tile(int x, int y, int layer);

/*
 * Set tile at map position
 */
void map_set_tile(int x, int y, u16 tile_id, int layer);

/*
 * Check if position is within map bounds
 */
int map_is_valid_position(int x, int y);

/* ========================================
 * Diamond Iteration
 * ======================================== */

/*
 * Calculate diamond iteration bounds
 * Returns visible tile range for rendering
 */
void calculate_diamond_bounds(int* start_row, int* end_row, int* start_col, int* end_col);

/*
 * Get diamond iteration starting position
 */
void get_diamond_start(int row, int* screen_x, int* screen_y);

#endif /* ISOMETRIC_H */
