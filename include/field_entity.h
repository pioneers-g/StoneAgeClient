/*
 * Stone Age Client - Field Entity System
 * Reverse engineered from sa_9061.exe
 *
 * Key functions:
 * - FUN_004779d0: Field UI initialization
 * - FUN_00477b20: Player entity spawn/update
 * - FUN_00477dc0: Player info display setup
 * - FUN_0040b5e0: Entity creation
 */

#ifndef FIELD_ENTITY_H
#define FIELD_ENTITY_H

#include "types.h"

/* Maximum entities on field */
#define MAX_FIELD_ENTITIES      500

/* Entity types */
typedef enum {
    ENTITY_TYPE_PLAYER = 0,
    ENTITY_TYPE_NPC = 1,
    ENTITY_TYPE_PET = 2,
    ENTITY_TYPE_MONSTER = 3,
    ENTITY_TYPE_ITEM = 4,
    ENTITY_TYPE_EFFECT = 5
} EntityType;

/* Entity flags */
#define ENTITY_FLAG_VISIBLE     0x01
#define ENTITY_FLAG_MOVING      0x02
#define ENTITY_FLAG_IN_BATTLE   0x04
#define ENTITY_FLAG_DEAD        0x08
#define ENTITY_FLAG_HIGHLIGHT   0x10
#define ENTITY_FLAG_MOUNTED     0x20
#define ENTITY_FLAG_SPECIAL     0x40
#define ENTITY_FLAG_GM          0x80

/*
 * Field entity structure
 * Based on FUN_0040b5e0 analysis
 * Entity size is 0x38 (56) base, with extended data
 */
typedef struct FieldEntity {
    /* Linked list pointers - offset 0x00, 0x04 */
    struct FieldEntity* prev;
    struct FieldEntity* next;

    /* Entity kind/type - offset 0x14 */
    u8  entity_kind;
    u8  padding1[3];

    /* Some index - offset 0x18 */
    s32 index;

    /* Screen position - offset 0x18-0x1c (set by FUN_00446df0) */
    s16 screen_x;
    s16 screen_y;

    /* World position - offset 0xb0, 0xb4 */
    s32 world_x;
    s32 world_y;

    /* Position for movement - offset 0xb8, 0xbc */
    s32 dest_x;
    s32 dest_y;

    /* Float position for rendering - offset 0x114, 0x118 */
    float render_x;
    float render_y;

    /* Entity data pointer - offset 0x11c */
    void* entity_data;

    /* Animation state - offset 0x110 */
    u16 anim_state;
    u16 padding2;

    /* Animation timer - offset 0x120 */
    u32 anim_timer;

    /* Sprite and display - offset 0x140 */
    u32 sprite_base;
    u32 sprite_flags;

    /* Entity-specific data - offset 0x148 */
    u32 entity_type;
    u32 entity_flags;

    /* Extended data - offset 0x150 */
    u32 ext_data;

    /* Type-specific data at offset 0x15 */
    u8  type_data;

} FieldEntity;

/*
 * Player entity extended data
 * From FUN_00477dc0 analysis
 * Offsets referenced: 0x38 (title), 0x55 (guild), 0x8c, 0x94, 0x194, 0x1a8, 0x1cc
 */
typedef struct {
    /* Title - offset 0x38, max 17 bytes (0x11) */
    char title[17];

    /* Guild name - offset 0x55, max 33 bytes (0x21) */
    char guild_name[33];

    /* Player flags - offset 0x8c */
    u32 player_flags;

    /* Sprite ID - offset 0x94 */
    u16 sprite_id;
    u16 padding1;

    /* Additional name/party - offset 0x194, max 33 bytes */
    char party_name[33];

    /* Guild rank or similar - offset 0x1a8 */
    u32 guild_data;

    /* Extended flags - offset 0x1cc */
    u32 ext_flags;

    /* Player stats reference */
    void* stats_ptr;

} PlayerEntityData;

/*
 * Field entity manager context
 * Manages all entities on the current map
 */
typedef struct {
    /* Entity pool */
    FieldEntity entities[MAX_FIELD_ENTITIES];
    u32 entity_count;

    /* Active entity list (linked list) */
    FieldEntity* active_head;
    FieldEntity* active_tail;

    /* Player entity pointer (DAT_0462e3ac) */
    FieldEntity* player_entity;

    /* Player info data (DAT_0462bef8 region) */
    char player_title[17];       /* DAT_0462bef8 */
    char player_guild[33];       /* DAT_0462bf09 */
    u32 player_field_0x0c;       /* DAT_0462bec0 */
    char player_party[33];       /* DAT_0462e3e4 */
    u32 player_sprite_id;        /* DAT_0462e408 */
    u16 player_level;            /* DAT_0462bf2a */
    u16 player_flags;            /* DAT_0462bf2c */

    /* Position references (DAT_04581d3c, DAT_04581d40) */
    u16* player_x_ref;
    u16* player_y_ref;

    /* Additional entity arrays (from FUN_004779d0) */
    u32 entity_slots_1[20];      /* DAT_0461b528 */
    u32 entity_slots_2[20];      /* DAT_0461b5e0 */
    u32 entity_slots_3[20];      /* DAT_0461b590 */
    u32 entity_slot_flags[5];    /* Various flags */

} FieldEntityContext;

/* Global context */
extern FieldEntityContext g_field_entities;

/* Initialization */
int field_entity_init(void);
void field_entity_shutdown(void);

/* Player entity - FUN_00477b20 */
FieldEntity* field_entity_create_player(u32 sprite_id, u16 x, u16 y, u32 param);
void field_entity_update_player(u32 sprite_id, u32 param);
void field_entity_set_player_position(u16 x, u16 y);

/* Player info - FUN_00477dc0 */
void field_entity_set_player_info(
    const char* title,
    const char* guild,
    u32 field_0x0c,
    const char* party,
    u32 sprite_id,
    u16 level,
    int flag_0x40,
    int flag_0x80,
    u32 param_9,
    u32 param_10,
    u32 param_11,
    u32 param_12
);

/* Field UI initialization - FUN_004779d0 */
void field_ui_init(void);
void field_ui_clear(void);

/* Entity management */
FieldEntity* field_entity_create(EntityType type, u16 x, u16 y);
void field_entity_destroy(FieldEntity* entity);
void field_entity_clear_all(void);

/* Entity update */
void field_entity_update_all(void);
void field_entity_render_all(void);

/* Entity lookup */
FieldEntity* field_entity_get_player(void);
FieldEntity* field_entity_find_by_id(u32 id);
FieldEntity* field_entity_find_at_position(u16 x, u16 y);

/* Entity rendering position */
void field_entity_calc_screen_pos(FieldEntity* entity);

#endif /* FIELD_ENTITY_H */
