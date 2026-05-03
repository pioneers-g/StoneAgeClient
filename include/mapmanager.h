/*
 * Stone Age Client - Map Manager Header
 * Reverse engineered from sa_9061.exe
 * Handles map loading, rendering, and collision
 */

#ifndef MAPMANAGER_H
#define MAPMANAGER_H

#include "types.h"

/* Map constants */
#define MAX_MAPS            300
#define MAX_MAP_WIDTH       256
#define MAX_MAP_HEIGHT      256
#define TILE_SIZE           32

/* Tile flags */
#define TILE_FLAG_WALKABLE      (1 << 0)
#define TILE_FLAG_WATER         (1 << 1)
#define TILE_FLAG_ENCOUNTER     (1 << 2)
#define TILE_FLAG_WARP          (1 << 3)
#define TILE_FLAG_NPC           (1 << 4)
#define TILE_FLAG_SAFE_ZONE     (1 << 5)

/* Map layer types */
typedef enum {
    LAYER_GROUND = 0,
    LAYER_OBJECT,
    LAYER_ROOF,
    LAYER_COUNT
} MapLayer;

/* Map tile structure */
typedef struct {
    u16 tile_id;        /* Tile graphic ID */
    u16 flags;          /* Tile properties */
    u8 x, y;            /* Tile position */
    u8 layer;           /* Layer index */
} MapTile;

/* Warp point */
typedef struct {
    int src_x, src_y;       /* Source position */
    int dest_map_id;        /* Destination map */
    int dest_x, dest_y;     /* Destination position */
} WarpPoint;

/* Map encounter */
typedef struct {
    u16 enemy_id;
    u8 level_min;
    u8 level_max;
    u8 encounter_rate;
} MapEncounter;

/* Map structure */
typedef struct {
    /* Identity */
    int map_id;
    char name[64];
    char file_path[256];

    /* Dimensions */
    int width;
    int height;

    /* Tiles */
    u16* ground_tiles;      /* Ground layer */
    u16* object_tiles;      /* Object layer */
    u8* tile_flags;         /* Collision/flags */

    /* Warps */
    WarpPoint* warps;
    int warp_count;

    /* Encounters */
    MapEncounter* encounters;
    int encounter_count;

    /* NPCs */
    int* npc_ids;
    int npc_count;

    /* Music */
    int bgm_id;

    /* Flags */
    int is_indoor;
    int is_dungeon;
    int is_safe_zone;

    /* State */
    int loaded;
    int data_size;
    u32 last_access;

} MapData;

/* Map manager context */
typedef struct {
    /* Current map */
    MapData* current_map;
    int current_map_id;

    /* Viewport */
    int view_x;
    int view_y;
    int view_width;
    int view_height;

    /* Map cache */
    MapData map_cache[32];
    int cache_count;

    /* Collision */
    u8* collision_map;

    /* Animation */
    u32 anim_timer;
    int anim_frame;

} MapManagerContext;

/* Global map manager */
extern MapManagerContext g_mapmgr;

/* Initialization */
int mapmgr_init(void);
void mapmgr_shutdown(void);

/* Map loading */
MapData* mapmgr_load(int map_id);
void mapmgr_unload(MapData* map);
void mapmgr_unload_all(void);

/* Map switching */
int mapmgr_change_map(int map_id, int x, int y);
int mapmgr_is_map_loaded(int map_id);

/* Current map access */
MapData* mapmgr_get_current(void);
int mapmgr_get_current_id(void);

/* Tile access */
u16 mapmgr_get_tile(int x, int y, int layer);
void mapmgr_set_tile(int x, int y, int layer, u16 tile_id);
u8 mapmgr_get_tile_flags(int x, int y);
int mapmgr_is_walkable(int x, int y);

/* Rendering */
void mapmgr_render(int screen_x, int screen_y);
void mapmgr_render_layer(int layer, int screen_x, int screen_y);
void mapmgr_render_tiles(int start_x, int start_y, int end_x, int end_y, int layer);

/* Viewport */
void mapmgr_set_viewport(int x, int y, int width, int height);
void mapmgr_center_view(int x, int y);
int mapmgr_get_view_x(void);
int mapmgr_get_view_y(void);

/* Collision */
int mapmgr_check_collision(int x, int y);
int mapmgr_find_path(int start_x, int start_y, int end_x, int end_y, int* path, int max_length);

/* Warps */
WarpPoint* mapmgr_find_warp(int x, int y);
int mapmgr_add_warp(MapData* map, int src_x, int src_y, int dest_map, int dest_x, int dest_y);

/* Encounters */
MapEncounter* mapmgr_get_encounter(void);
int mapmgr_check_encounter(int x, int y);

/* World to screen conversion */
void mapmgr_world_to_screen(int world_x, int world_y, int* screen_x, int* screen_y);
void mapmgr_screen_to_world(int screen_x, int screen_y, int* world_x, int* world_y);

/* Utilities */
int mapmgr_get_width(void);
int mapmgr_get_height(void);
const char* mapmgr_get_name(void);

/* Battle map loading */
int mapmgr_load_battle_map(int battle_id);
void mapmgr_unload_battle_map(void);

/* Animation */
void mapmgr_update_animation(void);
int mapmgr_get_anim_frame(void);

/* Debug */
void mapmgr_draw_collision_map(void);

#endif /* MAPMANAGER_H */
