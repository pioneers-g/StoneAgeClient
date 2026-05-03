/*
 * Stone Age Client - Map NPC Management
 * Reverse engineered from sa_9061.exe
 *
 * FUN_0040ee70 - NPC creation
 * FUN_0040f310 - Find free NPC slot
 * FUN_0040f3f0 - Get NPC type value
 * FUN_0040f3c0 - Get render flags
 * FUN_0040e830 - Find NPC by ID
 *
 * Note: This module handles NPCs on the game map (field entities).
 * The npc.c module handles NPC dialog interactions.
 */

#ifndef MAP_NPC_H
#define MAP_NPC_H

#include "types.h"

/* ========================================
 * Constants from binary analysis
 * ======================================== */

/* NPC/Object constants from FUN_0040ee70, FUN_0040f310 */
#define MAX_MAP_OBJECTS         1500  /* 0x5dc - max NPC/object count */
#define NPC_STRUCT_SIZE         0x10c /* 268 bytes per NPC */
#define MAX_OBJ_NAME_LEN        16    /* Max object name length */
#define MAX_OBJ_TITLE_LEN       32    /* Max object title length */

/*
 * Object type values from FUN_0040f3f0
 * Type determines animation state value
 */
typedef enum {
    OBJ_TYPE_NORMAL     = 0,    /* Returns 1 */
    OBJ_TYPE_NPC        = 1,    /* Returns 8 */
    OBJ_TYPE_PLAYER     = 3,    /* Returns 0x200 render flags */
    OBJ_TYPE_PET        = 13,   /* Returns 17 (0x11) */
    OBJ_TYPE_ENEMY      = 27,   /* Returns 17 (0x11) */
    OBJ_TYPE_EFFECT     = 32,   /* Returns 32 (0x21) */
} ObjectType;

/* Type values returned by FUN_0040f3f0 */
#define NPC_TYPEVAL_NORMAL      1
#define NPC_TYPEVAL_NPC         8
#define NPC_TYPEVAL_CHARACTER   17    /* Player, pet, enemy */
#define NPC_TYPEVAL_EFFECT      32

/*
 * Object flags from FUN_0040ee70
 */
#define OBJ_FLAG_HIDDEN     0x40    /* Object is hidden */
#define OBJ_FLAG_INVISIBLE  0x80    /* Object is invisible */

/*
 * NPC/Object structure - from FUN_0040ee70 analysis
 * Size: 0x10c (268) bytes
 * Array at DAT_004e2b20, max 1500 entries
 * Byte offset = index * 0x10c
 * Word offset = index * 0x86 (for u16 access at start)
 * Dword offset = index * 0x43 (for u32 access)
 */
#pragma pack(push, 1)
typedef struct {
    /* Offset 0x00-0x03 */
    u16 type;           /* +0x00: Object type (OBJ_TYPE_*) */
    u16 anim_state;     /* +0x02: Animation state/frame */

    /* Offset 0x04-0x1b */
    u32 id;             /* +0x04: Object ID */
    u32 x;              /* +0x08: World X coordinate */
    u32 y;              /* +0x0c: World Y coordinate */
    u32 param1;         /* +0x10: Parameter 1 (direction/action) */
    u32 param2;         /* +0x14: Parameter 2 */
    u32 param3;         /* +0x18: Parameter 3 */

    /* Offset 0x1c-0x23 */
    u32 extra_param;    /* +0x1c: Extra parameter */
    u8  direction;      /* +0x20: Facing direction */
    u8  action;         /* +0x21: Current action */
    u16 sprite_id;      /* +0x22: Sprite ID */

    /* Offset 0x24-0x37 - Name string (up to 16 chars) */
    char name[MAX_OBJ_NAME_LEN + 1];    /* +0x24: Object name */

    /* Offset 0x35-0x56 - Title string (up to 32 chars) */
    char title[MAX_OBJ_TITLE_LEN + 1];  /* +0x35: Object title */

    /* Offset 0x57-0x5b */
    u16 width;          /* +0x57: Sprite width */
    u16 height;         /* +0x59: Sprite height */
    u8  obj_flags;      /* +0x5b: Object flags */

    /* Offset 0x5c-0x5f */
    u32 render_flags;   /* +0x5c: Render/sprite flags */

    /* Offset 0x60-0x10b */
    u8  reserved[172];  /* +0x60: Reserved data */
} MapNPC;
#pragma pack(pop)

/*
 * Map NPC context - matches DAT_004e2bxx region
 * Named MapNPCContext to avoid conflict with NPCContext in npc.h
 */
typedef struct {
    MapNPC objects[MAX_MAP_OBJECTS];    /* Array at DAT_004e2b20 */
    u32 count;                          /* DAT_004e2b14 - active count */
    u32 high_water;                     /* DAT_00544d70 - highest used index */
    u32 next_index;                     /* DAT_004e2b10 - next free index */
    u32 search_index;                   /* DAT_004e2b0c - search optimization */
} MapNPCContext;

/* Global map NPC context */
extern MapNPCContext g_map_npcs;

/* ========================================
 * Map NPC Functions (prefixed with map_npc_)
 * ======================================== */

/* Initialization */
int map_npc_init(void);
void map_npc_shutdown(void);
void map_npc_clear(void);

/* NPC creation - FUN_0040ee70 */
int map_npc_create(u32 id, u32 x, u32 y, u32 param1, u32 param2, u32 param3,
                   const char* name, const char* title, u32 extra_param,
                   u16 sprite_id, int hidden, int invisible);

/* NPC lookup - FUN_0040e830 */
int map_npc_find_by_id(u32 id);
MapNPC* map_npc_get_by_id(u32 id);
MapNPC* map_npc_get_by_index(u32 index);

/* NPC update */
void map_npc_remove(u32 id);
void map_npc_update_all(void);

/* NPC rendering */
void map_npc_render_all(void);

/* Object type helpers - FUN_0040f3f0, FUN_0040f3c0 */
u16 map_npc_get_type_value(u32 obj_type);
u32 map_npc_get_render_flags(u32 obj_type);

#endif /* MAP_NPC_H */
