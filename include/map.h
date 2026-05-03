/*
 * Stone Age Client - Map System
 * Reverse engineered from sa_9061.exe
 *
 * Protocol Analysis:
 * FUN_00404850 - Battle field loader (20x20 isometric grid)
 * FUN_00446df0 - World to screen isometric transform
 * FUN_00446e40 - Screen to world isometric transform
 * FUN_0047e210 - Sprite render queue add
 */

#ifndef MAP_H
#define MAP_H

#include "types.h"
#include "map_npc.h"  /* NPC management types and functions */

/* ========================================
 * Constants from binary analysis
 * ======================================== */

/* Battle field constants from FUN_00404850 */
#define MAX_BATTLE_FIELDS       220   /* battle000.sab - battle219.sab (0xDB = 219 max) */
#define BATTLE_FIELD_SIZE       20    /* 20x20 grid */
#define BATTLE_TILE_COUNT       400   /* 20 * 20 */

/* Map size constants */
#define MAP_MAX_SIZE            256   /* Maximum map dimension */

/* Render queue constants from FUN_0047e210 */
#define MAX_RENDER_QUEUE        4096  /* 0x1000 - max sprites in queue */
#define RENDER_ENTRY_SIZE       0x18  /* 24 bytes per render entry */

/* Isometric transformation constants from FUN_00446df0 */
#define ISO_TILE_WIDTH          64.0f   /* Tile width in pixels */
#define ISO_TILE_HEIGHT         32.0f   /* Tile height in pixels */
#define ISO_X_STEP              32      /* X increment per column (0x20) */
#define ISO_Y_STEP              24      /* Y increment per column (0x18) */
#define ISO_SCALE_FACTOR        (1.0f / 64.0f)  /* World to screen scale */

/* Screen center from DAT_004bb424, DAT_004bb428 */
#define SCREEN_CENTER_X         320   /* 0x140 */
#define SCREEN_CENTER_Y         240   /* 0xf0 */

/* ========================================
 * Battle Background Sprites - from FUN_00404850
 * ======================================== */

/* Default battle sprites based on terrain type */
#define BATTLE_SPRITE_DEFAULT   0x7149  /* Grass/default */
#define BATTLE_SPRITE_DESERT    0x715c  /* Desert terrain */
#define BATTLE_SPRITE_SNOW      0x715b  /* Snow terrain */
#define BATTLE_SPRITE_CAVE      0x7160  /* Cave terrain */
#define BATTLE_SPRITE_INDOOR    0x7161  /* Indoor terrain */
#define BATTLE_SPRITE_SPECIAL   0x718d  /* Special map (0x2147) */

/* Terrain-based sprite calculations */
#define BATTLE_SPRITE_TERRAIN_BASE  0x7177  /* Terrain variant base */
#define BATTLE_SPRITE_INDOOR_BASE   0x718e  /* Indoor variant base */
#define BATTLE_SPRITE_VARIANT_BASE  0x7192  /* Another variant base */

/* Special map IDs from FUN_00404850 checks */
#define MAP_ID_SPECIAL_331      0x331
#define MAP_ID_DUNGEON_1        0x1f47
#define MAP_ID_DUNGEON_2        0x1fa5
#define MAP_ID_DUNGEON_3        0x1fa4
#define MAP_ID_DUNGEON_4        0x1f5b
#define MAP_ID_DUNGEON_5        0x1f5c
#define MAP_ID_DUNGEON_6        0x1f5d
#define MAP_ID_DUNGEON_7        0x1f4f

/*
 * Battle field structure - from FUN_00404850
 * .sab files are loaded from data/battleMap/battle###.sab
 */
#pragma pack(push, 1)
typedef struct {
    u16 tiles[BATTLE_TILE_COUNT];   /* 400 tiles (20x20) read as byte pairs */
} BattleField;
#pragma pack(pop)

/* Map tile structure */
typedef struct {
    u16 ground_tile;     /* Ground layer tile ID */
    u16 object_tile;     /* Object layer tile ID */
    u8  flags;           /* Walk flags, etc */
    u8  height;          /* Tile height for pseudo-3D */
} MapTile;

/* Map layer types */
typedef enum {
    LAYER_GROUND = 0,
    LAYER_OBJECT = 1,
    LAYER_SKY = 2,
    LAYER_WEATHER = 3
} MapLayer;

/* Map object structure */
typedef struct {
    u32 id;
    u16 x;
    u16 y;
    u16 sprite_id;
    u8  direction;
    u8  action;
    u16 width;
    u16 height;
    u32 flags;
    u32 anim_state;     /* Animation state for rendering */
} MapObject;

/* Map header */
typedef struct {
    u32 magic;
    u16 width;
    u16 height;
    u16 tile_count;
    u16 object_count;
    char name[32];
    u32 checksum;
} MapHeader;

/* Map context */
typedef struct {
    u32 current_map_id;
    MapTile* tiles;
    MapObject* objects;
    u32 tile_count;
    u32 object_count;
    u16 width;
    u16 height;

    /* Collision map for pathfinding - DAT_04569b74 region */
    u8* collision_map;          /* 1 = walkable, 0 = blocked */

    /* Map offset for coordinate conversion - DAT_04560e44, DAT_04560e3c */
    s32 offset_x;
    s32 offset_y;

    /* Battle mode flag - DAT_046308b4 */
    int in_battle;

    /* Camera */
    int camera_x;
    int camera_y;
    int target_camera_x;
    int target_camera_y;

    /* Player position */
    int player_x;
    int player_y;
    int player_direction;

    /* Map data buffers */
    void* map_data;
    u32 map_data_size;

    /* Loaded flag */
    int loaded;
} MapContext;

/* Global map context */
extern MapContext g_map;

/* Map functions */
int map_init(void);
void map_shutdown(void);

/* Map loading */
int map_load(u32 map_id);
int map_load_from_file(const char* path);
void map_unload(void);

/* Map rendering - FUN_0047dc60 */
void map_render(void);
void map_render_layer(MapLayer layer);
void map_render_objects(void);
void map_render_isometric(void);  /* FUN_004412e0 - isometric diamond iteration */

/* Tile access */
MapTile* map_get_tile(int x, int y);
int map_is_walkable(int x, int y);
int map_get_height(int x, int y);

/* Object management */
void map_add_object(MapObject* obj);
void map_remove_object(u32 id);
MapObject* map_get_object(u32 id);
void map_update_objects(void);

/* Camera */
void map_set_camera(int x, int y);
void map_set_camera_target(int x, int y);
void map_update_camera(void);

/* Coordinate conversion */
void map_screen_to_world(int screen_x, int screen_y, int* world_x, int* world_y);
void map_world_to_screen(int world_x, int world_y, int* screen_x, int* screen_y);

/* Isometric coordinate conversion - FUN_00446df0, FUN_00446e40 */
void map_world_to_screen_iso(float world_x, float world_y, float* screen_x, float* screen_y);
void map_screen_to_world_iso(float screen_x, float screen_y, float* world_x, float* world_y);
void map_set_camera_iso(float world_x, float world_y);

/* Movement */
int map_move_player(int dx, int dy);
int map_can_move(int x, int y);
int map_check_collision(int x, int y, int mode);

/* Convenience function */
#define map_get_current_id() ((u16)map_get_current_map_id())

/* Battle map functions */
int map_load_battle(u32 battle_id);
void map_render_battle(void);
const char* map_get_battle_path(u32 battle_id);
u16 map_get_battle_sprite(void);
void map_set_battle_sprite(u16 sprite_id);
void map_set_current_map_id(u32 map_id);
u32 map_get_current_map_id(void);

/* Map entry functions - from binary packet dispatcher case 0x43 */
void map_set_map_id(u32 map_id);                        /* FUN_00440dd0 */
void map_set_coordinates(int x, int y);                 /* FUN_00440df0 */
void map_load_map_data(u32 map_id, int width, int height);  /* FUN_00440530 */
void map_clear_all_pets(void);                          /* FUN_0040f650 */

/* Isometric field rendering - FUN_004412e0 */
void map_render_field_iso(int camera_x, int camera_y, int map_offset_x, int map_offset_y,
                           int field_width, int field_height);
void map_init_field_buffers(int width, int height);
void map_set_field_tile(int x, int y, u16 ground_tile, u16 object_tile);
int map_get_tile_update_flag(void);
void map_clear_tile_update_flag(void);

#endif /* MAP_H */
