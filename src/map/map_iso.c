/*
 * Stone Age Client - Map Isometric Field Renderer Module
 * Reverse engineered from sa_9061.exe (FUN_004412e0)
 *
 * Isometric field rendering with diamond pattern and depth sorting
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "map.h"
#include "render.h"
#include "assets.h"
#include "logger.h"

/* Field tile data buffers - from DAT_04581d48, DAT_0456a64c */
static u16 s_field_tiles_ground[MAP_MAX_SIZE * MAP_MAX_SIZE];
static u16 s_field_tiles_object[MAP_MAX_SIZE * MAP_MAX_SIZE];

/* Field dimensions */
static int s_field_width = 0;    /* DAT_045602b0 */
static int s_field_height = 0;

/* Camera tracking for change detection */
static int s_last_camera_x = 0;  /* DAT_004bb414 */
static int s_last_camera_y = 0;  /* DAT_004bb418 */

/* Tile rendering update flag */
static int s_tile_update_flag = 0;  /* DAT_04ebe270 */

/* Render queue entry for isometric tiles - from FUN_004445e0 */
typedef struct {
    void* sprite_data;
    int screen_x;
    int screen_y;
    int width;
    int height;
    int base_y;
    float world_x;
    float world_y;
    u16 render_type;
    u16 reserved;
    int sort_key;
} IsoRenderEntry;

/* Render queue for isometric tiles - DAT_0456b180 region */
#define MAX_ISO_QUEUE 2048
static IsoRenderEntry s_iso_queue[MAX_ISO_QUEUE];
static int s_iso_queue_count = 0;  /* DAT_0456b17c */

/*
 * Render tile effect - FUN_00488680
 */
static void map_render_tile_effect(u16 tile_id) {
    if (tile_id >= 0x28 && tile_id <= 0x3b) {
        LOG_DEBUG("Tile effect: 0x%04x", tile_id);
    }
}

/*
 * Render ground tile - FUN_004888f0
 */
static void map_render_ground_tile(u16 tile_id, int screen_x, int screen_y) {
    if (tile_id >= 0x14 && tile_id <= 0x27) {
        render_queue_add_sprite(screen_x, screen_y, 1, tile_id, 0);
    }
}

/*
 * Add entry to isometric render queue - FUN_004445e0
 */
static void map_add_iso_queue(void* sprite_data, int x, int y, int w, int h,
                               float world_x, float world_y, int priority) {
    IsoRenderEntry* entry;

    if (s_iso_queue_count >= MAX_ISO_QUEUE) {
        return;
    }

    entry = &s_iso_queue[s_iso_queue_count];
    entry->sprite_data = sprite_data;
    entry->screen_x = x;
    entry->screen_y = y;
    entry->width = w;
    entry->height = h;
    entry->base_y = y;
    entry->world_x = world_x;
    entry->world_y = world_y;
    entry->render_type = (priority == 0) ? 2 : 1;
    entry->sort_key = 0;

    s_iso_queue_count++;
}

/*
 * Check if tile is in screen bounds
 */
static int map_tile_on_screen(int screen_x, int screen_y, int width, int height) {
    if (screen_x + width < -32) return 0;
    if (screen_x > 640 + 32) return 0;
    if (screen_y + height < -24) return 0;
    if (screen_y > 480 + 24) return 0;
    return 1;
}

/*
 * Render isometric field tiles - FUN_004412e0
 */
void map_render_field_iso(int camera_x, int camera_y, int map_offset_x, int map_offset_y,
                           int field_width, int field_height) {
    float screen_x_f, screen_y_f;
    int screen_x, screen_y;
    int base_x, base_y;
    int delta_x, delta_y;
    int row, col;
    int tile_x, tile_y;
    u16 ground_tile, object_tile;
    int prev_screen_x = 0, prev_screen_y = 0;
    int render_start_row, render_end_row;
    int sprite_width, sprite_height;
    void* sprite_data;
    int i;

    s_tile_update_flag = 0;

    if (camera_x != s_last_camera_x || camera_y != s_last_camera_y) {
        s_last_camera_x = camera_x;
        s_last_camera_y = camera_y;
        s_tile_update_flag = 1;
    }

    if (camera_x == 0 && camera_y == 0) {
        map_world_to_screen_iso(0.0f, 0.0f, &screen_x_f, &screen_y_f);
        screen_x = (int)screen_x_f;
        screen_y = (int)screen_y_f;
    } else {
        screen_x = SCREEN_CENTER_X + (camera_y + camera_x) * -ISO_X_STEP / 2;
        screen_y = SCREEN_CENTER_Y + (camera_y - camera_x) * ISO_Y_STEP / 2;
    }

    delta_x = screen_x - prev_screen_x;
    delta_y = screen_y - prev_screen_y;

    base_x = ((map_offset_x + field_width) * ISO_X_STEP - ISO_X_STEP) / 2 + screen_x;
    base_y = ((field_height - map_offset_x - 1) * ISO_Y_STEP) / 2 + screen_y;

    render_start_row = 0;
    render_end_row = field_height;

    for (row = render_start_row; row < render_end_row; row++) {
        int row_base_x = base_x;
        int row_base_y = base_y;

        for (col = 0; col < field_width; col++) {
            int tile_idx = row * field_width + col;

            ground_tile = s_field_tiles_ground[tile_idx];
            object_tile = s_field_tiles_object[tile_idx];

            tile_x = row_base_x;
            tile_y = row_base_y;

            if (ground_tile < 100) {
                if (ground_tile < 0x14 || ground_tile > 0x27) {
                    if (ground_tile > 0x27 && ground_tile < 0x3c) {
                        map_render_tile_effect(ground_tile);
                        s_tile_update_flag = 1;
                    }
                } else {
                    map_render_ground_tile(ground_tile, tile_x, tile_y);
                }
            } else {
                if (map_tile_on_screen(tile_x, tile_y, 64, 32)) {
                    int visible = 1;

                    if (delta_x != 0 || delta_y != 0) {
                        if (tile_x - delta_x > 607 || tile_x - delta_x < 33) {
                            visible = 0;
                        }
                        if (tile_y - delta_y > 456 || tile_y - delta_y < 24) {
                            visible = visible != 0;
                        }
                    }

                    if (visible) {
                        render_queue_add_sprite(tile_x, tile_y, 1, ground_tile, 0);
                    }
                }
            }

            if (object_tile < 100) {
                if (object_tile < 0x14 || object_tile > 0x27) {
                    if (object_tile > 0x27 && object_tile < 0x3c) {
                        map_render_tile_effect(object_tile);
                        s_tile_update_flag = 1;
                    }
                } else {
                    map_render_ground_tile(object_tile, tile_x, tile_y);
                }
            } else {
                sprite_data = assets_get_sprite(object_tile);
                if (sprite_data) {
                    assets_get_sprite_dimensions(object_tile, &sprite_width, &sprite_height);

                    if (map_tile_on_screen(tile_x, tile_y, sprite_width, sprite_height)) {
                        float world_x = (float)(col + map_offset_x);
                        float world_y = (float)(row + map_offset_y);

                        map_add_iso_queue(sprite_data, tile_x, tile_y,
                                         sprite_width, sprite_height,
                                         world_x, world_y, 0);
                    }
                }
            }

            row_base_x += ISO_X_STEP;
            row_base_y -= ISO_Y_STEP;
        }

        if (row < field_width - 1) {
            base_x += ISO_X_STEP / 2;
        } else {
            base_x -= ISO_X_STEP / 2;
        }
        base_y += ISO_Y_STEP;
    }

    for (i = 0; i < s_iso_queue_count; i++) {
        IsoRenderEntry* entry = &s_iso_queue[i];
        render_sprite((u32)(uintptr_t)entry->sprite_data, entry->screen_x, entry->screen_y);
    }

    s_iso_queue_count = 0;

    if (s_tile_update_flag) {
        LOG_DEBUG("Field tile update triggered");
    }
}

/*
 * Initialize field tile buffers
 */
void map_init_field_buffers(int width, int height) {
    s_field_width = width;
    s_field_height = height;
    memset(s_field_tiles_ground, 0, sizeof(s_field_tiles_ground));
    memset(s_field_tiles_object, 0, sizeof(s_field_tiles_object));
}

/*
 * Set field tile data
 */
void map_set_field_tile(int x, int y, u16 ground_tile, u16 object_tile) {
    int idx = y * s_field_width + x;
    if (idx >= 0 && idx < MAP_MAX_SIZE * MAP_MAX_SIZE) {
        s_field_tiles_ground[idx] = ground_tile;
        s_field_tiles_object[idx] = object_tile;
    }
}

/*
 * Get field tile update flag
 */
int map_get_tile_update_flag(void) {
    return s_tile_update_flag;
}

/*
 * Clear field tile update flag
 */
void map_clear_tile_update_flag(void) {
    s_tile_update_flag = 0;
}

/*
 * Get field tile buffers
 */
u16* map_get_field_tiles_ground(void) {
    return s_field_tiles_ground;
}

u16* map_get_field_tiles_object(void) {
    return s_field_tiles_object;
}

/*
 * Get field dimensions
 */
void map_get_field_dimensions(int* width, int* height) {
    if (width) *width = s_field_width;
    if (height) *height = s_field_height;
}

/*
 * Load map data file - FUN_00440530
 */
void map_load_map_data(u32 map_id, int width, int height) {
    char path[260];
    HANDLE hFile;
    DWORD bytes_read;
    int layer, row, col;
    u16 tile_value;

    snprintf(path, sizeof(path), "data/map/%d.dat", map_id);

    hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    int file_width, file_height;
    ReadFile(hFile, &file_width, 4, &bytes_read, NULL);
    ReadFile(hFile, &file_height, 4, &bytes_read, NULL);

    if (file_width != width || file_height != height) {
        CloseHandle(hFile);
        return;
    }

    map_init_field_buffers(width, height);

    for (layer = 0; layer < 3; layer++) {
        for (row = 0; row < height; row++) {
            for (col = 0; col < width; col++) {
                if (!ReadFile(hFile, &tile_value, 2, &bytes_read, NULL)) {
                    break;
                }
                map_set_field_tile(col, row, tile_value, 0);
            }
        }
    }

    CloseHandle(hFile);
}
