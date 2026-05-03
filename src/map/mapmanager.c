/*
 * Stone Age Client - Map Manager Implementation
 * Reverse engineered from sa_9061.exe
 * Handles map loading, rendering, and collision
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "mapmanager.h"
#include "logger.h"
#include "resource.h"

/* Global map manager */
MapManagerContext g_mapmgr = {0};

/*
 * Initialize map manager
 */
int mapmgr_init(void) {
    memset(&g_mapmgr, 0, sizeof(MapManagerContext));

    g_mapmgr.view_width = 640;
    g_mapmgr.view_height = 480;
    g_mapmgr.current_map_id = -1;

    LOG_INFO("Map manager initialized");
    return 1;
}

/*
 * Shutdown map manager
 */
void mapmgr_shutdown(void) {
    mapmgr_unload_all();

    if (g_mapmgr.collision_map) {
        free(g_mapmgr.collision_map);
        g_mapmgr.collision_map = NULL;
    }

    memset(&g_mapmgr, 0, sizeof(MapManagerContext));
    LOG_INFO("Map manager shutdown");
}

/*
 * Load map data
 */
MapData* mapmgr_load(int map_id) {
    MapData* map;
    char path[256];
    FILE* fp;
    int i;

    /* Check if already cached */
    for (i = 0; i < g_mapmgr.cache_count; i++) {
        if (g_mapmgr.map_cache[i].map_id == map_id && g_mapmgr.map_cache[i].loaded) {
            g_mapmgr.map_cache[i].last_access = timeGetTime();
            return &g_mapmgr.map_cache[i];
        }
    }

    /* Find free cache slot */
    if (g_mapmgr.cache_count >= 32) {
        /* Evict oldest */
        u32 oldest_time = 0xFFFFFFFF;
        int oldest_index = -1;

        for (i = 0; i < g_mapmgr.cache_count; i++) {
            if (g_mapmgr.map_cache[i].last_access < oldest_time &&
                &g_mapmgr.map_cache[i] != g_mapmgr.current_map) {
                oldest_time = g_mapmgr.map_cache[i].last_access;
                oldest_index = i;
            }
        }

        if (oldest_index >= 0) {
            mapmgr_unload(&g_mapmgr.map_cache[oldest_index]);
        }
    }

    /* Allocate new slot */
    map = &g_mapmgr.map_cache[g_mapmgr.cache_count];

    /* Build path - map files in data\map\ directory */
    snprintf(path, sizeof(path), "data\\map\\map_%d.dat", map_id);

    fp = fopen(path, "rb");
    if (!fp) {
        LOG_WARN("Failed to load map: %s", path);
        return NULL;
    }

    /* Read map header */
    fread(&map->map_id, sizeof(int), 1, fp);
    fread(&map->width, sizeof(int), 1, fp);
    fread(&map->height, sizeof(int), 1, fp);
    fread(&map->bgm_id, sizeof(int), 1, fp);
    fread(&map->is_indoor, sizeof(int), 1, fp);
    fread(map->name, 1, sizeof(map->name), fp);

    /* Allocate tiles */
    map->ground_tiles = (u16*)malloc(map->width * map->height * sizeof(u16));
    map->object_tiles = (u16*)malloc(map->width * map->height * sizeof(u16));
    map->tile_flags = (u8*)malloc(map->width * map->height);

    if (!map->ground_tiles || !map->object_tiles || !map->tile_flags) {
        LOG_ERROR("Failed to allocate map tiles");
        fclose(fp);
        return NULL;
    }

    /* Read tiles */
    fread(map->ground_tiles, sizeof(u16), map->width * map->height, fp);
    fread(map->object_tiles, sizeof(u16), map->width * map->height, fp);
    fread(map->tile_flags, sizeof(u8), map->width * map->height, fp);

    fclose(fp);

    map->loaded = 1;
    map->last_access = timeGetTime();
    strncpy(map->file_path, path, sizeof(map->file_path) - 1);

    g_mapmgr.cache_count++;

    LOG_INFO("Loaded map %d: %s (%dx%d)", map_id, map->name, map->width, map->height);
    return map;
}

/*
 * Unload map data
 */
void mapmgr_unload(MapData* map) {
    if (!map || !map->loaded) return;

    if (map->ground_tiles) {
        free(map->ground_tiles);
        map->ground_tiles = NULL;
    }

    if (map->object_tiles) {
        free(map->object_tiles);
        map->object_tiles = NULL;
    }

    if (map->tile_flags) {
        free(map->tile_flags);
        map->tile_flags = NULL;
    }

    if (map->warps) {
        free(map->warps);
        map->warps = NULL;
    }

    if (map->encounters) {
        free(map->encounters);
        map->encounters = NULL;
    }

    map->loaded = 0;
    map->warp_count = 0;
    map->encounter_count = 0;
}

/*
 * Unload all maps
 */
void mapmgr_unload_all(void) {
    int i;

    for (i = 0; i < g_mapmgr.cache_count; i++) {
        mapmgr_unload(&g_mapmgr.map_cache[i]);
    }

    g_mapmgr.cache_count = 0;
    g_mapmgr.current_map = NULL;
    g_mapmgr.current_map_id = -1;
}

/*
 * Change to new map
 */
int mapmgr_change_map(int map_id, int x, int y) {
    MapData* map;

    map = mapmgr_load(map_id);
    if (!map) {
        LOG_ERROR("Failed to change to map %d", map_id);
        return 0;
    }

    g_mapmgr.current_map = map;
    g_mapmgr.current_map_id = map_id;

    /* Set viewport */
    mapmgr_center_view(x, y);

    LOG_INFO("Changed to map %d at (%d, %d)", map_id, x, y);
    return 1;
}

/*
 * Check if map is loaded
 */
int mapmgr_is_map_loaded(int map_id) {
    int i;

    for (i = 0; i < g_mapmgr.cache_count; i++) {
        if (g_mapmgr.map_cache[i].map_id == map_id && g_mapmgr.map_cache[i].loaded) {
            return 1;
        }
    }

    return 0;
}

/*
 * Get current map
 */
MapData* mapmgr_get_current(void) {
    return g_mapmgr.current_map;
}

/*
 * Get current map ID
 */
int mapmgr_get_current_id(void) {
    return g_mapmgr.current_map_id;
}

/*
 * Get tile at position
 */
u16 mapmgr_get_tile(int x, int y, int layer) {
    MapData* map = g_mapmgr.current_map;

    if (!map || !map->loaded) return 0;
    if (x < 0 || x >= map->width || y < 0 || y >= map->height) return 0;

    if (layer == LAYER_GROUND) {
        return map->ground_tiles[y * map->width + x];
    } else if (layer == LAYER_OBJECT) {
        return map->object_tiles[y * map->width + x];
    }

    return 0;
}

/*
 * Set tile at position
 */
void mapmgr_set_tile(int x, int y, int layer, u16 tile_id) {
    MapData* map = g_mapmgr.current_map;

    if (!map || !map->loaded) return;
    if (x < 0 || x >= map->width || y < 0 || y >= map->height) return;

    if (layer == LAYER_GROUND) {
        map->ground_tiles[y * map->width + x] = tile_id;
    } else if (layer == LAYER_OBJECT) {
        map->object_tiles[y * map->width + x] = tile_id;
    }
}

/*
 * Get tile flags
 */
u8 mapmgr_get_tile_flags(int x, int y) {
    MapData* map = g_mapmgr.current_map;

    if (!map || !map->loaded) return 0;
    if (x < 0 || x >= map->width || y < 0 || y >= map->height) return 0;

    return map->tile_flags[y * map->width + x];
}

/*
 * Check if tile is walkable
 */
int mapmgr_is_walkable(int x, int y) {
    u8 flags = mapmgr_get_tile_flags(x, y);
    return (flags & TILE_FLAG_WALKABLE) != 0;
}

/*
 * Render map
 */
void mapmgr_render(int screen_x, int screen_y) {
    /* Render layers in order */
    mapmgr_render_layer(LAYER_GROUND, screen_x, screen_y);
    mapmgr_render_layer(LAYER_OBJECT, screen_x, screen_y);
}

/*
 * Render single layer
 */
void mapmgr_render_layer(int layer, int screen_x, int screen_y) {
    MapData* map = g_mapmgr.current_map;
    int start_tile_x, start_tile_y;
    int end_tile_x, end_tile_y;
    int tile_x, tile_y;
    int pixel_x, pixel_y;

    if (!map || !map->loaded) return;

    /* Calculate visible tile range */
    start_tile_x = g_mapmgr.view_x / TILE_SIZE;
    start_tile_y = g_mapmgr.view_y / TILE_SIZE;
    end_tile_x = (g_mapmgr.view_x + g_mapmgr.view_width) / TILE_SIZE + 1;
    end_tile_y = (g_mapmgr.view_y + g_mapmgr.view_height) / TILE_SIZE + 1;

    /* Clamp to map bounds */
    if (start_tile_x < 0) start_tile_x = 0;
    if (start_tile_y < 0) start_tile_y = 0;
    if (end_tile_x > map->width) end_tile_x = map->width;
    if (end_tile_y > map->height) end_tile_y = map->height;

    /* Render tiles */
    for (tile_y = start_tile_y; tile_y < end_tile_y; tile_y++) {
        for (tile_x = start_tile_x; tile_x < end_tile_x; tile_x++) {
            u16 tile_id = mapmgr_get_tile(tile_x, tile_y, layer);

            if (tile_id > 0) {
                /* Calculate screen position */
                pixel_x = tile_x * TILE_SIZE - g_mapmgr.view_x + screen_x;
                pixel_y = tile_y * TILE_SIZE - g_mapmgr.view_y + screen_y;

                /* Draw tile sprite */
                /* sprite_draw_tile(tile_id, pixel_x, pixel_y); */
            }
        }
    }
}

/*
 * Render tiles in range
 */
void mapmgr_render_tiles(int start_x, int start_y, int end_x, int end_y, int layer) {
    MapData* map = g_mapmgr.current_map;
    int x, y;

    if (!map || !map->loaded) return;

    for (y = start_y; y < end_y; y++) {
        for (x = start_x; x < end_x; x++) {
            u16 tile_id = mapmgr_get_tile(x, y, layer);

            if (tile_id > 0) {
                int pixel_x = x * TILE_SIZE - g_mapmgr.view_x;
                int pixel_y = y * TILE_SIZE - g_mapmgr.view_y;

                /* sprite_draw_tile(tile_id, pixel_x, pixel_y); */
            }
        }
    }
}

/*
 * Set viewport
 */
void mapmgr_set_viewport(int x, int y, int width, int height) {
    g_mapmgr.view_x = x;
    g_mapmgr.view_y = y;
    g_mapmgr.view_width = width;
    g_mapmgr.view_height = height;
}

/*
 * Center view on world position
 */
void mapmgr_center_view(int x, int y) {
    MapData* map = g_mapmgr.current_map;

    if (!map || !map->loaded) return;

    /* Calculate centered view position */
    g_mapmgr.view_x = x * TILE_SIZE - g_mapmgr.view_width / 2;
    g_mapmgr.view_y = y * TILE_SIZE - g_mapmgr.view_height / 2;

    /* Clamp to map bounds */
    if (g_mapmgr.view_x < 0) g_mapmgr.view_x = 0;
    if (g_mapmgr.view_y < 0) g_mapmgr.view_y = 0;

    if (g_mapmgr.view_x > map->width * TILE_SIZE - g_mapmgr.view_width) {
        g_mapmgr.view_x = map->width * TILE_SIZE - g_mapmgr.view_width;
    }
    if (g_mapmgr.view_y > map->height * TILE_SIZE - g_mapmgr.view_height) {
        g_mapmgr.view_y = map->height * TILE_SIZE - g_mapmgr.view_height;
    }
}

/*
 * Get viewport X
 */
int mapmgr_get_view_x(void) {
    return g_mapmgr.view_x;
}

/*
 * Get viewport Y
 */
int mapmgr_get_view_y(void) {
    return g_mapmgr.view_y;
}

/*
 * Check collision at position
 */
int mapmgr_check_collision(int x, int y) {
    return !mapmgr_is_walkable(x, y);
}

/*
 * Find warp at position
 */
WarpPoint* mapmgr_find_warp(int x, int y) {
    MapData* map = g_mapmgr.current_map;
    int i;

    if (!map || !map->warps) return NULL;

    for (i = 0; i < map->warp_count; i++) {
        if (map->warps[i].src_x == x && map->warps[i].src_y == y) {
            return &map->warps[i];
        }
    }

    return NULL;
}

/*
 * Add warp to map
 */
int mapmgr_add_warp(MapData* map, int src_x, int src_y, int dest_map, int dest_x, int dest_y) {
    WarpPoint* warp;

    if (!map) return 0;

    /* Reallocate warps array */
    map->warps = (WarpPoint*)realloc(map->warps, (map->warp_count + 1) * sizeof(WarpPoint));
    if (!map->warps) return 0;

    warp = &map->warps[map->warp_count];
    warp->src_x = src_x;
    warp->src_y = src_y;
    warp->dest_map_id = dest_map;
    warp->dest_x = dest_x;
    warp->dest_y = dest_y;

    map->warp_count++;
    return 1;
}

/*
 * Get random encounter
 */
MapEncounter* mapmgr_get_encounter(void) {
    MapData* map = g_mapmgr.current_map;

    if (!map || !map->encounters || map->encounter_count == 0) {
        return NULL;
    }

    /* Random encounter selection */
    int index = rand() % map->encounter_count;
    return &map->encounters[index];
}

/*
 * Check for encounter at position
 */
int mapmgr_check_encounter(int x, int y) {
    u8 flags = mapmgr_get_tile_flags(x, y);

    if (!(flags & TILE_FLAG_ENCOUNTER)) {
        return 0;
    }

    /* Random chance for encounter */
    return (rand() % 100) < 5;  /* 5% chance */
}

/*
 * Convert world to screen coordinates
 */
void mapmgr_world_to_screen(int world_x, int world_y, int* screen_x, int* screen_y) {
    if (screen_x) *screen_x = world_x * TILE_SIZE - g_mapmgr.view_x;
    if (screen_y) *screen_y = world_y * TILE_SIZE - g_mapmgr.view_y;
}

/*
 * Convert screen to world coordinates
 */
void mapmgr_screen_to_world(int screen_x, int screen_y, int* world_x, int* world_y) {
    if (world_x) *world_x = (screen_x + g_mapmgr.view_x) / TILE_SIZE;
    if (world_y) *world_y = (screen_y + g_mapmgr.view_y) / TILE_SIZE;
}

/*
 * Get map width
 */
int mapmgr_get_width(void) {
    MapData* map = g_mapmgr.current_map;
    return map ? map->width : 0;
}

/*
 * Get map height
 */
int mapmgr_get_height(void) {
    MapData* map = g_mapmgr.current_map;
    return map ? map->height : 0;
}

/*
 * Get map name
 */
const char* mapmgr_get_name(void) {
    MapData* map = g_mapmgr.current_map;
    return map ? map->name : "Unknown";
}

/*
 * Load battle map
 */
int mapmgr_load_battle_map(int battle_id) {
    char path[256];
    FILE* fp;

    snprintf(path, sizeof(path), "data\\battleMap\\battle%d.sab", battle_id);

    fp = fopen(path, "rb");
    if (!fp) {
        LOG_WARN("Failed to load battle map: %s", path);
        return 0;
    }

    /* Battle map loading logic */
    /* Would read battle map format specific to Stone Age */

    fclose(fp);
    LOG_INFO("Loaded battle map: %s", path);
    return 1;
}

/*
 * Unload battle map
 */
void mapmgr_unload_battle_map(void) {
    /* Cleanup battle map resources */
}

/*
 * Update map animation
 */
void mapmgr_update_animation(void) {
    u32 current_time = timeGetTime();

    if (current_time - g_mapmgr.anim_timer >= 200) {  /* 200ms per frame */
        g_mapmgr.anim_timer = current_time;
        g_mapmgr.anim_frame = (g_mapmgr.anim_frame + 1) % 4;
    }
}

/*
 * Get current animation frame
 */
int mapmgr_get_anim_frame(void) {
    return g_mapmgr.anim_frame;
}

/*
 * Draw collision map (debug)
 */
void mapmgr_draw_collision_map(void) {
    MapData* map = g_mapmgr.current_map;
    int x, y;

    if (!map || !map->tile_flags) return;

    for (y = 0; y < map->height; y++) {
        for (x = 0; x < map->width; x++) {
            u8 flags = map->tile_flags[y * map->width + x];
            int screen_x = x * TILE_SIZE - g_mapmgr.view_x;
            int screen_y = y * TILE_SIZE - g_mapmgr.view_y;

            if (flags & TILE_FLAG_WALKABLE) {
                /* Draw green */
            } else {
                /* Draw red */
            }
        }
    }
}
