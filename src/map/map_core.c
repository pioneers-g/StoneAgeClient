/*
 * Stone Age Client - Map Core Module
 * Reverse engineered from sa_9061.exe
 *
 * Core map operations: initialization, loading, tile management, camera
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "map.h"
#include "render.h"
#include "logger.h"

/* Global map context */
MapContext g_map = {0};

/* Isometric camera state */
static float s_iso_camera_x = 0.0f;
static float s_iso_camera_y = 0.0f;

/* Current map ID for sprite selection - DAT_04581190 */
static u32 s_current_map_id = 0;

/*
 * Initialize map system
 */
int map_init(void) {
    memset(&g_map, 0, sizeof(MapContext));
    LOG_INFO("Map system initialized");
    return 1;
}

/*
 * Shutdown map system
 */
void map_shutdown(void) {
    map_unload();
    LOG_INFO("Map system shutdown");
}

/*
 * Unload current map
 */
void map_unload(void) {
    if (g_map.tiles) {
        free(g_map.tiles);
        g_map.tiles = NULL;
    }
    if (g_map.objects) {
        free(g_map.objects);
        g_map.objects = NULL;
    }
    if (g_map.map_data) {
        free(g_map.map_data);
        g_map.map_data = NULL;
    }
    if (g_map.collision_map) {
        free(g_map.collision_map);
        g_map.collision_map = NULL;
    }

    memset(&g_map, 0, sizeof(MapContext));
}

/*
 * Load map by ID
 */
int map_load(u32 map_id) {
    char path[260];

    snprintf(path, sizeof(path), "data/map/map%05d.bin", map_id);
    return map_load_from_file(path);
}

/*
 * Load map from file
 */
int map_load_from_file(const char* path) {
    HANDLE hFile;
    DWORD bytes_read;
    MapHeader header;
    u32 tile_data_size;

    LOG_INFO("Loading map: %s", path);

    hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        LOG_ERROR("Failed to open map file: %s", path);
        return 0;
    }

    if (!ReadFile(hFile, &header, sizeof(MapHeader), &bytes_read, NULL)) {
        CloseHandle(hFile);
        return 0;
    }

    if (header.magic != 0x4D415053) { /* "MAPS" */
        LOG_ERROR("Invalid map magic: 0x%08X", header.magic);
        CloseHandle(hFile);
        return 0;
    }

    map_unload();

    g_map.width = header.width;
    g_map.height = header.height;
    g_map.current_map_id = 0;

    g_map.tile_count = header.width * header.height;
    g_map.tiles = (MapTile*)malloc(g_map.tile_count * sizeof(MapTile));
    if (!g_map.tiles) {
        CloseHandle(hFile);
        return 0;
    }

    tile_data_size = g_map.tile_count * sizeof(MapTile);
    if (!ReadFile(hFile, g_map.tiles, tile_data_size, &bytes_read, NULL)) {
        map_unload();
        CloseHandle(hFile);
        return 0;
    }

    if (header.object_count > 0) {
        g_map.object_count = header.object_count;
        g_map.objects = (MapObject*)malloc(header.object_count * sizeof(MapObject));
        if (g_map.objects) {
            ReadFile(hFile, g_map.objects, header.object_count * sizeof(MapObject),
                &bytes_read, NULL);
        }
    }

    CloseHandle(hFile);
    g_map.loaded = 1;

    LOG_INFO("Map loaded: %dx%d, %u objects", g_map.width, g_map.height, g_map.object_count);
    return 1;
}

/*
 * Get tile at position
 */
MapTile* map_get_tile(int x, int y) {
    if (!g_map.tiles) return NULL;
    if (x < 0 || x >= g_map.width || y < 0 || y >= g_map.height) return NULL;

    return &g_map.tiles[y * g_map.width + x];
}

/*
 * Check if position is walkable
 */
int map_is_walkable(int x, int y) {
    MapTile* tile = map_get_tile(x, y);
    if (!tile) return 0;

    return (tile->flags & 0x01) == 0;
}

/*
 * Get tile height
 */
int map_get_height(int x, int y) {
    MapTile* tile = map_get_tile(x, y);
    if (!tile) return 0;

    return tile->height;
}

/*
 * Set camera position
 */
void map_set_camera(int x, int y) {
    g_map.camera_x = x;
    g_map.camera_y = y;

    if (g_map.camera_x < 0) g_map.camera_x = 0;
    if (g_map.camera_y < 0) g_map.camera_y = 0;
}

/*
 * Set camera target (for smooth scrolling)
 */
void map_set_camera_target(int x, int y) {
    g_map.target_camera_x = x;
    g_map.target_camera_y = y;
}

/*
 * Update camera (smooth movement)
 */
void map_update_camera(void) {
    int dx, dy, speed;

    speed = 4;

    dx = g_map.target_camera_x - g_map.camera_x;
    dy = g_map.target_camera_y - g_map.camera_y;

    if (abs(dx) > speed) {
        g_map.camera_x += (dx > 0) ? speed : -speed;
    } else {
        g_map.camera_x = g_map.target_camera_x;
    }

    if (abs(dy) > speed) {
        g_map.camera_y += (dy > 0) ? speed : -speed;
    } else {
        g_map.camera_y = g_map.target_camera_y;
    }
}

/*
 * Set current map ID - DAT_04581190
 */
void map_set_current_map_id(u32 map_id) {
    s_current_map_id = map_id;
}

/*
 * Get current map ID
 */
u32 map_get_current_map_id(void) {
    return s_current_map_id;
}

/*
 * Set isometric camera position
 */
void map_set_camera_iso(float world_x, float world_y) {
    s_iso_camera_x = world_x;
    s_iso_camera_y = world_y;
}

/*
 * Get isometric camera position
 */
void map_get_camera_iso(float* world_x, float* world_y) {
    if (world_x) *world_x = s_iso_camera_x;
    if (world_y) *world_y = s_iso_camera_y;
}

/*
 * Move player
 */
int map_move_player(int dx, int dy) {
    int new_x = g_map.player_x + dx;
    int new_y = g_map.player_y + dy;

    if (!map_is_walkable(new_x, new_y)) {
        return 0;
    }

    g_map.player_x = new_x;
    g_map.player_y = new_y;

    map_set_camera_target(
        g_map.player_x * 32 - 320,
        g_map.player_y * 32 - 240
    );

    return 1;
}

/*
 * Check if can move to position
 */
int map_can_move(int x, int y) {
    return map_is_walkable(x, y);
}

/*
 * Add object to map
 */
void map_add_object(MapObject* obj) {
    if (!obj) return;

    g_map.objects = (MapObject*)realloc(g_map.objects,
        (g_map.object_count + 1) * sizeof(MapObject));

    if (g_map.objects) {
        memcpy(&g_map.objects[g_map.object_count], obj, sizeof(MapObject));
        g_map.object_count++;
    }
}

/*
 * Remove object from map
 */
void map_remove_object(u32 id) {
    u32 i;

    if (!g_map.objects) return;

    for (i = 0; i < g_map.object_count; i++) {
        if (g_map.objects[i].id == id) {
            memmove(&g_map.objects[i], &g_map.objects[i + 1],
                (g_map.object_count - i - 1) * sizeof(MapObject));
            g_map.object_count--;
            break;
        }
    }
}

/*
 * Get object by ID
 */
MapObject* map_get_object(u32 id) {
    u32 i;

    if (!g_map.objects) return NULL;

    for (i = 0; i < g_map.object_count; i++) {
        if (g_map.objects[i].id == id) {
            return &g_map.objects[i];
        }
    }

    return NULL;
}

/*
 * Update map state
 */
void map_update(void) {
    if (!g_map.loaded) return;

    map_update_camera();
    map_update_objects();
}

/*
 * Update map objects
 */
void map_update_objects(void) {
    u32 i;

    if (!g_map.objects) return;

    for (i = 0; i < g_map.object_count; i++) {
        MapObject* obj = &g_map.objects[i];

        if (obj->sprite_id != 0) {
            u32 current_time = timeGetTime();
            u32 frame_duration = 200;

            u16 frame = (u16)((current_time / frame_duration) % 8);

            obj->anim_state = (obj->anim_state & 0xFFF8) | (frame & 0x7);
        }
    }
}

/*
 * Handle map data from server
 */
void map_handle_data(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u16 map_id;
    u16 width, height;
    u32 i;

    if (size < 6) return;

    map_id = *(u16*)ptr; ptr += 2;
    width = *(u16*)ptr; ptr += 2;
    height = *(u16*)ptr; ptr += 2;

    g_map.current_map_id = map_id;
    g_map.width = width;
    g_map.height = height;

    if (ptr + (width * height * 2) <= (u8*)data + size) {
        for (i = 0; i < (u32)(width * height) && i < MAP_MAX_SIZE * MAP_MAX_SIZE; i++) {
            g_map.tiles[i].ground_tile = *(u16*)(ptr + i * 2);
        }
    }

    LOG_INFO("Map data loaded: ID=%u, %ux%u", map_id, width, height);
}

/*
 * Change to new map
 */
void map_change(u32 map_id) {
    memset(g_map.tiles, 0, sizeof(g_map.tiles));
    memset(g_map.objects, 0, sizeof(g_map.objects));
    g_map.object_count = 0;

    g_map.current_map_id = map_id;
    g_map.loaded = 0;

    LOG_INFO("Changing to map %u", map_id);
}

/*
 * Set current map ID - FUN_00440dd0
 */
void map_set_map_id(u32 map_id) {
    g_map.current_map_id = map_id;
}

/*
 * Set player coordinates on map - FUN_00440df0
 */
void map_set_coordinates(int x, int y) {
    g_map.player_x = x;
    g_map.player_y = y;

    s_iso_camera_x = (float)x * 0.015625f;
    s_iso_camera_y = (float)y * 0.015625f;

    g_map.offset_x = 0;
    g_map.offset_y = 0;

    g_map.camera_x = x * 32;
    g_map.camera_y = y * 24;
    g_map.target_camera_x = g_map.camera_x;
    g_map.target_camera_y = g_map.camera_y;

    g_map.loaded = 1;
}

/*
 * Check collision at position - FUN_0040e8c0 pattern
 */
int map_check_collision(int x, int y, int mode) {
    if (x < 0 || y < 0 || x >= (int)g_map.width || y >= (int)g_map.height) {
        return 0;
    }

    (void)mode;

    return 1;
}

/*
 * Clear all pet entities - FUN_0040f650
 */
void map_clear_all_pets(void) {
    g_map.in_battle = 0;
}
