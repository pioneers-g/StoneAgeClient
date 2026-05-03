/*
 * Stone Age Client - Map Render Module
 * Reverse engineered from sa_9061.exe
 *
 * Map rendering: regular and isometric transforms
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "types.h"
#include "map.h"
#include "render.h"
#include "assets.h"
#include "logger.h"

/* External map context */
extern MapContext g_map;

/* Isometric camera state */
static float s_iso_camera_x = 0.0f;
static float s_iso_camera_y = 0.0f;

/*
 * Render map - FUN_0047dc60 pattern
 */
void map_render(void) {
    if (!g_map.loaded) return;

    int start_x = g_map.camera_x / 32;
    int start_y = g_map.camera_y / 32;
    int end_x = start_x + (g_map.width / 32) + 2;
    int end_y = start_y + (g_map.height / 32) + 2;

    if (start_x < 0) start_x = 0;
    if (start_y < 0) start_y = 0;
    if (end_x > g_map.width) end_x = g_map.width;
    if (end_y > g_map.height) end_y = g_map.height;

    map_render_layer(LAYER_GROUND);
    map_render_objects();
    map_render_layer(LAYER_OBJECT);
}

/*
 * Render isometric map - FUN_004412e0 pattern
 */
void map_render_isometric(void) {
    int row, col;
    int screen_x, screen_y;
    int base_x, base_y;
    int start_row, end_row;
    int start_col, end_col;
    u16 tile_id;
    MapTile* tile;

    if (!g_map.loaded || !g_map.tiles) return;

    float fx, fy;
    map_world_to_screen_iso(0.0f, 0.0f, &fx, &fy);
    screen_x = (int)fx;
    screen_y = (int)fy;

    start_row = 0;
    end_row = g_map.height;
    start_col = 0;
    end_col = g_map.width;

    base_x = screen_x - ((g_map.width + g_map.height) * 32) / 2;
    base_y = screen_y - ((g_map.height - g_map.width) * 24) / 2;

    for (row = end_row - 1; row >= start_row; row--) {
        int row_x = base_x;
        int row_y = base_y;

        for (col = start_col; col < end_col; col++) {
            int map_x, map_y;

            map_y = row;
            map_x = col + (row - start_row);

            if (map_x < 0 || map_x >= g_map.width || map_y < 0 || map_y >= g_map.height) {
                row_x += 32;
                row_y -= 24;
                continue;
            }

            tile = map_get_tile(map_x, map_y);
            if (!tile) {
                row_x += 32;
                row_y -= 24;
                continue;
            }

            tile_id = tile->ground_tile;
            if (tile_id > 0 && tile_id < 100) {
                /* Special tile handling */
            } else if (tile_id >= 100) {
                if (row_x > -64 && row_x < 640 + 64 && row_y > -64 && row_y < 480 + 64) {
                    render_queue_add_sprite(row_x, row_y, 1, tile_id, 0);
                }
            }

            tile_id = tile->object_tile;
            if (tile_id >= 100) {
                if (row_x > -64 && row_x < 640 + 64 && row_y > -64 && row_y < 480 + 64) {
                    render_queue_add_sprite(row_x, row_y, 1, tile_id, 0);
                }
            }

            row_x += 32;
            row_y -= 24;
        }

        base_x += 32;
        base_y += 24;
    }
}

/*
 * Render specific layer
 */
void map_render_layer(MapLayer layer) {
    int x, y, tile_x, tile_y;
    MapTile* tile;
    u16 tile_id;

    if (!g_map.loaded || !g_map.tiles) return;

    for (y = 0; y < g_map.height; y++) {
        for (x = 0; x < g_map.width; x++) {
            tile = map_get_tile(x, y);
            if (!tile) continue;

            if (layer == LAYER_GROUND) {
                tile_id = tile->ground_tile;
            } else if (layer == LAYER_OBJECT) {
                tile_id = tile->object_tile;
            } else {
                continue;
            }

            if (tile_id == 0) continue;

            tile_x = x * 32 - g_map.camera_x;
            tile_y = y * 32 - g_map.camera_y;

            render_sprite(tile_id, tile_x, tile_y);
        }
    }
}

/*
 * Render map objects
 */
void map_render_objects(void) {
    u32 i;
    MapObject* obj;
    int screen_x, screen_y;

    if (!g_map.objects) return;

    for (i = 0; i < g_map.object_count; i++) {
        obj = &g_map.objects[i];

        screen_x = obj->x * 32 - g_map.camera_x;
        screen_y = obj->y * 32 - g_map.camera_y;

        render_sprite(obj->sprite_id, screen_x, screen_y);
    }
}

/*
 * Isometric coordinate transformation - FUN_00446df0
 */
void map_world_to_screen_iso(float world_x, float world_y, float* screen_x, float* screen_y) {
    float fvar1, fvar2;

    fvar1 = (world_x - s_iso_camera_x) * ISO_SCALE_FACTOR;
    fvar2 = (world_y - s_iso_camera_y) * ISO_SCALE_FACTOR;

    *screen_x = (fvar2 + fvar1) * ISO_TILE_WIDTH + (float)SCREEN_CENTER_X;
    *screen_y = (fvar2 - fvar1) * ISO_TILE_HEIGHT + (float)SCREEN_CENTER_Y;
}

/*
 * Screen to world coordinate transformation - FUN_00446e40
 */
void map_screen_to_world_iso(float screen_x, float screen_y, float* world_x, float* world_y) {
    float dx, dy;

    dx = screen_x - (float)SCREEN_CENTER_X;
    dy = screen_y - (float)SCREEN_CENTER_Y;

    *world_x = (dx / ISO_TILE_WIDTH + dy / ISO_TILE_HEIGHT) * 64.0f + s_iso_camera_x;
    *world_y = (dy / ISO_TILE_HEIGHT) + (dx / ISO_TILE_WIDTH) + s_iso_camera_y;
}

/*
 * Screen to world coordinate conversion
 */
void map_screen_to_world(int screen_x, int screen_y, int* world_x, int* world_y) {
    float wx, wy;
    map_screen_to_world_iso((float)screen_x, (float)screen_y, &wx, &wy);
    if (world_x) *world_x = (int)floorf(wx);
    if (world_y) *world_y = (int)floorf(wy);
}

/*
 * World to screen coordinate conversion
 */
void map_world_to_screen(int world_x, int world_y, int* screen_x, int* screen_y) {
    float sx, sy;
    map_world_to_screen_iso((float)world_x, (float)world_y, &sx, &sy);
    if (screen_x) *screen_x = (int)floorf(sx);
    if (screen_y) *screen_y = (int)floorf(sy);
}

/*
 * Update isometric camera from external state
 */
void map_render_set_camera(float x, float y) {
    s_iso_camera_x = x;
    s_iso_camera_y = y;
}
