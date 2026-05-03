/*
 * Stone Age Client - Isometric Renderer Implementation
 * Reverse engineered from sa_9061.exe (FUN_004412e0, FUN_00446df0, FUN_00446e40)
 *
 * Implements isometric tile rendering with diamond iteration pattern
 */

#include <windows.h>
#include <math.h>
#include "isometric.h"
#include "render.h"
#include "directx.h"
#include "logger.h"

/* ========================================
 * Global Variables
 * ======================================== */

/* Camera instance - matches DAT_04581d3c region */
IsometricCamera g_camera = {0};

/* Map tile data */
MapTileData g_map_tiles = {0};

/* Environment state */
u32 g_environment_type = 0;     /* DAT_04633308 */
u32 g_environment_state = 0;    /* DAT_04ebe278 */
u32 g_render_flag = 0;          /* DAT_04ebe270 */

/* Coordinate transformation constants - from binary */
#define COORD_SCALE_X       0.5f    /* _DAT_0049c334 */
#define COORD_SCALE_Y       0.5f
#define SCREEN_CENTER_X     320.0f  /* DAT_004bb424 */
#define SCREEN_CENTER_Y     240.0f  /* DAT_004bb428 */
#define WORLD_OFFSET_X      0.0f    /* _DAT_04582998 */
#define WORLD_OFFSET_Y      0.0f    /* _DAT_04582994 */

/* Diamond iteration constants */
#define TILE_ITER_X         0x40    /* 64 - X increment per tile */
#define TILE_ITER_Y         0x18    /* 24 - Y increment per tile */
#define TILE_ITER_X_HALF    0x20    /* 32 - half X for diagonal */
#define TILE_ITER_Y_HALF    0x18    /* 24 - half Y for diagonal */

/* ========================================
 * Coordinate Transformations
 * ======================================== */

/*
 * World to screen coordinate transformation - FUN_00446df0
 *
 * Decompiled formula:
 *   fVar1 = (world_x - offset_x) * scale
 *   fVar2 = (world_y - offset_y) * scale
 *   screen_x = (fVar2 + fVar1) * iso_scale_x + center_x
 *   screen_y = (fVar2 - fVar1) * iso_scale_y + center_y
 */
void world_to_screen(float world_x, float world_y, float* screen_x, float* screen_y) {
    float dx, dy;

    /* Apply world offset */
    dx = (world_x - WORLD_OFFSET_X) * COORD_SCALE_X;
    dy = (world_y - WORLD_OFFSET_Y) * COORD_SCALE_Y;

    /* Isometric transformation */
    *screen_x = (dy + dx) * 0.5f + SCREEN_CENTER_X;
    *screen_y = (dy - dx) * 0.5f + SCREEN_CENTER_Y;
}

/*
 * Screen to world coordinate transformation - FUN_00446e40
 *
 * Inverse isometric transformation
 */
void screen_to_world(float screen_x, float screen_y, float* world_x, float* world_y) {
    float dx, dy;

    /* Remove screen center offset */
    dx = screen_x - SCREEN_CENTER_X;
    dy = screen_y - SCREEN_CENTER_Y;

    /* Inverse isometric transformation */
    *world_x = (dx - dy * 2.0f) * 0.5f + WORLD_OFFSET_X;
    *world_y = (dx + dy * 2.0f) * 0.5f + WORLD_OFFSET_Y;
}

/*
 * Tile to screen coordinate conversion
 * Each tile is 64x48 pixels in isometric view
 */
void tile_to_screen(int tile_x, int tile_y, int* screen_x, int* screen_y) {
    *screen_x = (tile_y - tile_x) * ISO_HALF_WIDTH + g_camera.screen_x;
    *screen_y = (tile_x + tile_y) * ISO_HALF_HEIGHT + g_camera.screen_y;
}

/*
 * Screen to tile coordinate conversion
 */
void screen_to_tile(int screen_x, int screen_y, int* tile_x, int* tile_y) {
    int rel_x, rel_y;

    rel_x = screen_x - g_camera.screen_x;
    rel_y = screen_y - g_camera.screen_y;

    *tile_x = (rel_y / ISO_HALF_HEIGHT - rel_x / ISO_HALF_WIDTH) / 2;
    *tile_y = (rel_y / ISO_HALF_HEIGHT + rel_x / ISO_HALF_WIDTH) / 2;
}

/* ========================================
 * Camera Control
 * ======================================== */

/*
 * Initialize camera
 */
void camera_init(void) {
    memset(&g_camera, 0, sizeof(IsometricCamera));
    g_camera.world_x = 0.0f;
    g_camera.world_y = 0.0f;
    g_camera.screen_x = 320;
    g_camera.screen_y = 240;
}

/*
 * Set camera position in world coordinates
 */
void camera_set_position(float world_x, float world_y) {
    g_camera.prev_world_x = (int)g_camera.world_x;
    g_camera.prev_world_y = (int)g_camera.world_y;
    g_camera.world_x = world_x;
    g_camera.world_y = world_y;

    /* Update screen coordinates */
    world_to_screen(world_x, world_y,
                    &g_camera.screen_x, &g_camera.screen_y);
}

/*
 * Update camera scroll state - part of FUN_004412e0
 */
void camera_update_scroll(void) {
    int current_x, current_y;

    current_x = (int)g_camera.world_x;
    current_y = (int)g_camera.world_y;

    /* Calculate scroll deltas */
    g_camera.scroll_dx = current_x - g_camera.prev_world_x;
    g_camera.scroll_dy = current_y - g_camera.prev_world_y;
    g_camera.scroll_prev_x = current_x - g_camera.prev_world_x;
    g_camera.scroll_prev_y = current_y - g_camera.prev_world_y;
}

/*
 * Get camera offset for rendering
 */
void camera_get_offset(int* offset_x, int* offset_y) {
    *offset_x = g_camera.screen_x;
    *offset_y = g_camera.screen_y;
}

/* ========================================
 * Map Data Access
 * ======================================== */

/*
 * Get tile at map position
 */
u16 map_get_tile(int x, int y, int layer) {
    int index;

    if (!map_is_valid_position(x, y)) {
        return 0;
    }

    index = g_map_tiles.stride * y + x;

    if (layer == 0) {
        return g_map_tiles.ground_layer[index];
    } else {
        return g_map_tiles.object_layer[index];
    }
}

/*
 * Set tile at map position
 */
void map_set_tile(int x, int y, u16 tile_id, int layer) {
    int index;

    if (!map_is_valid_position(x, y)) {
        return;
    }

    index = g_map_tiles.stride * y + x;

    if (layer == 0) {
        g_map_tiles.ground_layer[index] = tile_id;
    } else {
        g_map_tiles.object_layer[index] = tile_id;
    }
}

/*
 * Check if position is within map bounds
 */
int map_is_valid_position(int x, int y) {
    return (x >= 0 && x < g_map_tiles.width &&
            y >= 0 && y < g_map_tiles.height);
}

/* ========================================
 * Environment Effects
 * ======================================== */

/*
 * Process environment effect - FUN_00488680
 * Handles rain, snow, fog, sandstorm, leaves
 */
void environment_process(u32 effect_type) {
    /* Check if effect is in valid range */
    if (effect_type < ENV_EFFECT_RAIN || effect_type > ENV_EFFECT_LEAVES) {
        return;
    }

    /* Set environment state */
    g_environment_type = effect_type;
    g_render_flag = 1;

    /* TODO: Implement actual particle effects */
    switch (effect_type) {
        case ENV_EFFECT_RAIN:
            LOG_DEBUG("Environment: Rain effect");
            break;
        case ENV_EFFECT_SNOW:
            LOG_DEBUG("Environment: Snow effect");
            break;
        case ENV_EFFECT_FOG:
            LOG_DEBUG("Environment: Fog effect");
            break;
        case ENV_EFFECT_SANDSTORM:
            LOG_DEBUG("Environment: Sandstorm effect");
            break;
        case ENV_EFFECT_LEAVES:
            LOG_DEBUG("Environment: Leaves effect");
            break;
        default:
            break;
    }
}

/*
 * Check if tile is environment effect
 */
int is_environment_tile(u16 tile_id) {
    return (tile_id >= TILE_TYPE_ENV_MIN && tile_id <= TILE_TYPE_ENV_MAX);
}

/*
 * Check if tile is special render type
 */
int is_special_tile(u16 tile_id) {
    return (tile_id >= TILE_TYPE_SPECIAL_MIN && tile_id <= TILE_TYPE_SPECIAL_MAX);
}

/* ========================================
 * Diamond Iteration Rendering
 * ======================================== */

/*
 * Calculate diamond iteration bounds
 */
void calculate_diamond_bounds(int* start_row, int* end_row, int* start_col, int* end_col) {
    /* Based on visible screen area */
    *start_row = 0;
    *end_row = g_map_tiles.height - 1;
    *start_col = 0;
    *end_col = g_map_tiles.width - 1;
}

/*
 * Render single tile
 */
void isometric_render_tile(u16 tile_id, int screen_x, int screen_y, int layer) {
    /* Handle different tile types */
    if (tile_id < 100) {
        /* Special or environment tile */
        if (is_special_tile(tile_id)) {
            /* Render special tile - FUN_004888f0 */
            render_tile(tile_id, screen_x, screen_y);
        } else if (is_environment_tile(tile_id)) {
            /* Process environment effect */
            environment_process(tile_id);
        }
    } else {
        /* Regular sprite tile */
        if (layer == 0) {
            /* Ground layer */
            render_sprite(tile_id, screen_x, screen_y);
        } else {
            /* Object layer - needs sprite lookup */
            render_sprite(tile_id, screen_x, screen_y);
        }
    }
}

/*
 * Main isometric render function - FUN_004412e0
 *
 * Uses diamond iteration pattern:
 * - Iterates diagonally across visible tiles
 * - Renders ground layer first, then object layer
 * - Handles scrolling and camera updates
 */
void isometric_render(void) {
    int screen_x, screen_y;
    int tile_x, tile_y;
    int row, col;
    int start_row, end_row;
    int diag_offset;
    u16 ground_tile, object_tile;
    int scroll_active;
    int in_scroll_region;
    int render_sprite_flag;

    g_render_flag = 0;

    /* Check if camera moved */
    if ((int)g_camera.world_x != g_camera.prev_world_x ||
        (int)g_camera.world_y != g_camera.prev_world_y) {
        /* Camera moved - update scroll state */
        camera_update_scroll();

        /* TODO: Call FUN_00440aa0 for map update check */
        /* TODO: Call FUN_00441b90 for map rebuild */
    }

    /* Calculate screen offset based on battle state */
    if (g_environment_state == 0) {
        /* Normal field rendering */
        world_to_screen(g_camera.world_x, g_camera.world_y,
                        (float*)&screen_x, (float*)&screen_y);
    } else {
        /* Battle mode - different offset calculation */
        screen_x = (int)(SCREEN_CENTER_X + (g_camera.world_x + g_camera.world_y) * -ISO_HALF_WIDTH);
        screen_y = (int)(SCREEN_CENTER_Y + (g_camera.world_x - g_camera.world_y) * ISO_HALF_HEIGHT);
    }

    g_camera.screen_x = screen_x;
    g_camera.screen_y = screen_y;

    /* Calculate center offset for diamond iteration */
    screen_x = ((g_map_tiles.width + g_map_tiles.height) * ISO_TILE_WIDTH / 2 - ISO_TILE_WIDTH) / 2 + screen_x;
    screen_y = ((g_map_tiles.height - g_map_tiles.width - 1) * ISO_HALF_HEIGHT) / 2 + screen_y;

    /* Diamond iteration bounds */
    start_row = g_map_tiles.height - 1;
    end_row = 0;

    /* Check scroll state for optimized rendering */
    scroll_active = (g_camera.scroll_prev_x != 0 || g_camera.scroll_prev_y != 0);

    /* Main diamond iteration loop */
    for (row = start_row; row >= 0; row--) {
        int row_screen_x = screen_x;
        int row_screen_y = screen_y;

        /* Iterate through diagonal */
        for (col = 0; col <= row && col < g_map_tiles.stride; col++) {
            tile_x = col;
            tile_y = row - col;

            if (tile_y < 0) break;

            /* Get tiles from both layers */
            ground_tile = map_get_tile(tile_x, tile_y, 0);
            object_tile = map_get_tile(tile_x, tile_y, 1);

            /* Render ground layer */
            if (ground_tile < 100) {
                if (is_special_tile(ground_tile)) {
                    render_tile(ground_tile, row_screen_x, row_screen_y);
                } else if (is_environment_tile(ground_tile)) {
                    environment_process(ground_tile);
                    g_render_flag = 1;
                }
            } else {
                /* Check scroll optimization */
                if (scroll_active &&
                    row_screen_x > -32 && row_screen_x < 672 &&
                    row_screen_y > -24 && row_screen_y < 504) {

                    /* Check if in dirty region */
                    render_sprite_flag = 1;
                    if (g_camera.scroll_prev_x < 1) {
                        if (g_camera.scroll_prev_x < 0 &&
                            row_screen_x - g_camera.scroll_prev_x > 607) {
                            render_sprite_flag = 0;
                        }
                    } else if (row_screen_x - g_camera.scroll_prev_x < 33) {
                        render_sprite_flag = 0;
                    }

                    if (render_sprite_flag) {
                        render_sprite(ground_tile, row_screen_x, row_screen_y);
                    }
                }
            }

            /* Render object layer */
            if (object_tile < 100) {
                if (is_special_tile(object_tile)) {
                    render_tile(object_tile, row_screen_x, row_screen_y);
                } else if (is_environment_tile(object_tile)) {
                    environment_process(object_tile);
                    g_render_flag = 1;
                }
            } else {
                /* Object sprite with dimension check */
                int sprite_w, sprite_h;
                if (render_get_sprite_dimensions(object_tile, (u16*)&sprite_w, (u16*)&sprite_h) == 0) {
                    int draw_x = row_screen_x;
                    int draw_y = row_screen_y;

                    /* Check if sprite is visible on screen */
                    if (draw_x < 640 && draw_x + sprite_w > 0 &&
                        draw_y < 480 && draw_y + sprite_h > 0) {
                        render_sprite(object_tile, draw_x, draw_y);
                    }
                }
            }

            /* Move to next tile in diagonal */
            row_screen_x -= ISO_TILE_WIDTH;   /* -64 */
            row_screen_y -= ISO_HALF_HEIGHT;  /* -24 */
        }

        /* Adjust screen position for next row */
        if (row < g_map_tiles.stride - 1) {
            screen_x += ISO_HALF_WIDTH;  /* +32 */
        } else {
            screen_x -= ISO_HALF_WIDTH;  /* -32 */
            start_row--;
        }
        screen_y -= ISO_HALF_HEIGHT;  /* -24 */
    }

    /* Process environment effect if active */
    if (g_environment_type > ENV_EFFECT_NONE &&
        g_environment_type <= ENV_EFFECT_LEAVES &&
        g_environment_state == 2) {
        environment_process(g_environment_type);
        g_render_flag = 1;
        g_environment_type = 0;
    }

    /* Update previous camera position */
    g_camera.prev_world_x = (int)g_camera.world_x;
    g_camera.prev_world_y = (int)g_camera.world_y;
}

/* ========================================
 * Map Rendering Functions
 * ======================================== */

/*
 * Render map tiles for a specific area
 */
void isometric_render_tiles(int start_x, int start_y, int width, int height) {
    int x, y;
    int screen_x, screen_y;
    u16 tile_id;

    for (y = start_y; y < start_y + height && y < g_map_tiles.height; y++) {
        for (x = start_x; x < start_x + width && x < g_map_tiles.width; x++) {
            tile_to_screen(x, y, &screen_x, &screen_y);

            /* Render ground layer */
            tile_id = map_get_tile(x, y, 0);
            if (tile_id >= 100) {
                render_sprite(tile_id, screen_x, screen_y);
            }

            /* Render object layer */
            tile_id = map_get_tile(x, y, 1);
            if (tile_id >= 100) {
                render_sprite(tile_id, screen_x, screen_y);
            }
        }
    }
}
