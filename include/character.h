/*
 * Stone Age Client - Character System
 * Reverse engineered from sa_9061.exe (FUN_00464670, FUN_00464190, FUN_00446df0)
 */

#ifndef CHARACTER_H
#define CHARACTER_H

#include "types.h"

/* Character directions - 8 directional movement */
typedef enum {
    DIR_NORTH = 0,
    DIR_NORTHEAST = 1,
    DIR_EAST = 2,
    DIR_SOUTHEAST = 3,
    DIR_SOUTH = 4,
    DIR_SOUTHWEST = 5,
    DIR_WEST = 6,
    DIR_NORTHWEST = 7
} Direction;

/* Character actions */
typedef enum {
    ACTION_STAND = 0,
    ACTION_WALK = 1,
    ACTION_RUN = 2,
    ACTION_ATTACK = 3,
    ACTION_SIT = 4,
    ACTION_DIE = 5,
    ACTION_CAST = 6,
    ACTION_HURT = 7
} CharacterAction;

/* Map CharacterAction to AnimationType */
static inline int character_action_to_anim_type(CharacterAction action) {
    switch (action) {
        case ACTION_STAND: return 0;  /* ANIM_TYPE_IDLE */
        case ACTION_WALK:   return 1;  /* ANIM_TYPE_WALK */
        case ACTION_RUN:    return 2;  /* ANIM_TYPE_RUN */
        case ACTION_ATTACK: return 3;  /* ANIM_TYPE_ATTACK */
        case ACTION_CAST:   return 5;  /* ANIM_TYPE_CAST */
        case ACTION_HURT:   return 6;  /* ANIM_TYPE_DAMAGE */
        case ACTION_DIE:    return 7;  /* ANIM_TYPE_DEATH */
        case ACTION_SIT:    return 8;  /* ANIM_TYPE_SIT */
        default: return 0;
    }
}

/*
 * Character entry structure - 0x61 (97) bytes per entry
 * From FUN_00464670 analysis (map enter handler)
 * Array at DAT_0462bf4c with stride 0x61 for integer fields
 * Array at DAT_0462c028 with stride 0xc2 for full entries (including strings)
 */
#pragma pack(push, 1)
typedef struct {
    /* Offset 0x00 - from DAT_0462bf4c indexing */
    s32 some_field;              /* Field from param 4 in FUN_00464670 */

    /* Offset 0x04 - from DAT_0462bf50 indexing */
    u16 x;                       /* World X coordinate */
    u16 padding1;

    /* Offset 0x08 - from DAT_0462bf54 indexing */
    u16 y;                       /* World Y coordinate */
    u16 padding2;

    /* Offset 0x0c - from DAT_0462bf58 indexing */
    u32 field_0x0c;              /* Some integer field */

    /* Offset 0x10 - from DAT_0462bf5c indexing */
    char field_0x10[16];         /* String field */

    /* Offset 0x20 */
    u8  padding3[17];

    /* Offset 0x31 - from DAT_0462c02a indexing (0xc2 stride) */
    u16 direction;               /* Facing direction */

    /* Offset 0x33 - from DAT_0462c02c indexing */
    u16 level;                   /* Character level (mod 100) */

    /* Offset 0x35 - from DAT_0462c02e indexing */
    u16 level_over_100;          /* Flag: level >= 100 */

    /* Offset 0x37 - from DAT_0462c030 indexing */
    u16 sprite_id;               /* Character sprite */
    u16 padding4;

    /* Offset 0x3b - from DAT_0462c032 indexing (name) */
    char name[29];               /* Character name (max 0x1d = 29 bytes) */
    u8  padding5;

    /* Offset 0x59 - from DAT_0462c04f indexing */
    char field_0x3a[17];         /* Short string field (max 0x11 = 17) */

    /* Offset 0x6a - from DAT_0462c060 indexing */
    char description[85];        /* Description (max 0x55 = 85) */

    /* Offset 0xbf */
    u8  padding6[32];

} CharacterEntry;  /* 0xc2 = 194 bytes with string data */
#pragma pack(pop)

/*
 * Player position structure
 * From FUN_00440df0 analysis
 */
typedef struct {
    u16 x;                       /* DAT_04581d3c, DAT_0458118c */
    u16 y;                       /* DAT_04581d40, DAT_04581184 */
    float fx;                    /* DAT_0456a644 - float version for rendering */
    float fy;                    /* DAT_0456a648 */
} PlayerPosition;

/* Character stats */
typedef struct {
    u16 level;
    u16 hp;
    u16 max_hp;
    u16 mp;
    u16 max_mp;
    u16 stamina;
    u16 max_stamina;

    /* Primary stats */
    u16 strength;
    u16 dexterity;
    u16 vitality;
    u16 agility;
    u16 intelligence;
    u16 luck;

    /* Combat stats */
    u16 attack;
    u16 defense;
    u16 magic_attack;
    u16 magic_defense;
    u16 hit_rate;
    u16 dodge_rate;

    /* Experience */
    u32 experience;
    u32 next_level_exp;
} CharacterStats;

/*
 * Detailed character stats from FUN_0045ffb0 case 0x50
 * 30+ fields parsed from server
 * Uses bitmask (field 1) to determine which fields are present
 */
#pragma pack(push, 1)
typedef struct {
    /* Player ID from case 0x45 */
    s32 player_id;             /* DAT_04581180 */
    s32 field_00;              /* DAT_045811b0 */

    /* Field 2-29: Integer fields (28 total) */
    u32 field_02;              /* DAT_0462be98 - hp? */
    u32 field_03;              /* DAT_0462be9c - max_hp? */
    u32 field_04;              /* DAT_0462bea0 - mp? */
    u32 field_05;              /* DAT_0462bea4 - max_mp? */
    u32 field_06;              /* DAT_0462bea8 - stamina? */
    u32 field_07;              /* DAT_0462beac - max_stamina? */
    u32 field_08;              /* DAT_0462beb0 - strength? */
    u32 field_09;              /* DAT_0462beb4 - dexterity? */
    u32 field_10;              /* DAT_0462beb8 - vitality? */
    u32 field_11;              /* DAT_0462bebc - agility? */
    u32 field_12;              /* DAT_0462bec0 - intelligence? */
    u32 field_13;              /* DAT_0462bec4 - luck? */
    u32 field_14;              /* DAT_0462bec8 - attack */
    u32 field_15;              /* DAT_0462becc - defense */
    u32 field_16;              /* DAT_0462bed0 - magic_attack */
    u32 field_17;              /* DAT_0462bed4 - magic_defense */
    u32 field_18;              /* DAT_0462bed8 - hit_rate */
    u32 field_19;              /* DAT_0462bedc - dodge_rate */
    u32 field_20;              /* DAT_0462bee0 - experience */
    u32 field_21;              /* DAT_0462bee4 - next_exp */
    u32 field_22;              /* DAT_0462bee8 - level */
    u32 field_23;              /* DAT_0462bef0 - gold? */
    u32 field_24;              /* DAT_0462bef4 - fame? */
    u32 field_25;              /* DAT_0462e3b4 - karma? */
    u32 field_26;              /* Used internally (DAT_0462e3d8) */
    u32 field_27;              /* Used internally (DAT_0462e3dc) */
    u32 field_28;              /* Used internally (DAT_0462e410) */
    u32 field_29;              /* Used internally (DAT_0462e3e0) */

    /* Field 30: String field (max 0x11 = 17 bytes) - DAT_0462bef8 */
    char title[17];

    /* Field 31: String field (max 0x21 = 33 bytes) - DAT_0462bf09 */
    char guild_name[33];

    /* Bitmask from field 1 */
    u32 field_mask;
} DetailedCharacterStats;
#pragma pack(pop)

/* Global detailed stats */
extern DetailedCharacterStats g_detailed_stats;

/* Character appearance */
typedef struct {
    u16 base_sprite;      /* Base character sprite */
    u16 hair_sprite;      /* Hair style */
    u16 face_sprite;      /* Face */
    u16 weapon_sprite;    /* Equipped weapon */
    u16 armor_sprite;     /* Equipped armor */
    u16 accessory_sprite; /* Accessory */
    u8  color_hair;       /* Hair color */
    u8  color_cloth;      /* Cloth color */
    u8  color_skin;       /* Skin color */
    u8  padding;
} CharacterAppearance;

/* Character structure */
typedef struct {
    u32 id;
    u16 sprite_id;                 /* Sprite ID for rendering */
    u16 padding;
    char name[24];
    u32 account_id;

    /* Position */
    u16 x;
    u16 y;
    u16 dest_x;
    u16 dest_y;
    u8  direction;
    u8  action;
    u16 speed;

    /* Movement */
    s16 move_dx;
    s16 move_dy;
    u32 move_end_time;
    u8  is_moving;

    /* Animation - integrated with animation system */
    u32 anim_entity_id;     /* Entity ID for animation system */
    u16 frame;              /* Current frame (cached from animation) */
    u16 frame_time;
    u8  animation_loop;
    u8  anim_playing;       /* Is animation active */

    /* Appearance */
    CharacterAppearance appearance;

    /* Stats */
    CharacterStats stats;

    /* Pet */
    u32 pet_id;

    /* Flags */
    u32 flags;
    u8  visible;
    u8  in_battle;

    /* Buffs/Debuffs */
    u32 buff_flags;
    u32 buff_end_time[8];

} Character;

/* Pet structure */
typedef struct {
    u32 id;
    u32 owner_id;
    char name[24];
    u16 sprite_id;
    u16 level;

    /* Position relative to owner */
    s16 offset_x;
    s16 offset_y;

    /* Stats */
    u16 current_hp;
    u16 max_hp;
    u16 attack;
    u16 defense;
    u16 speed;
    u16 agility;
    u16 spirit;
    u8  element;
    u8  padding;

    /* Skills */
    u16 skill_id[4];
    u8  skill_level[4];

} Pet;

/*
 * NPC structure - 0x10c (268) bytes per entry
 * From FUN_0040ee70 analysis
 * Array at DAT_004e2b20 with stride 0x86 for main fields
 */
#pragma pack(push, 1)
typedef struct {
    /* Offset 0x00 - from DAT_004e2b20 indexing (stride 0x86) */
    u16 type;                    /* 0x02 for NPC? */
    u16 direction;               /* From FUN_0040f3f0 */

    /* Offset 0x04 - from DAT_004e2b24 indexing (stride 0x43) */
    u32 id;                      /* NPC unique ID */
    u32 param1;                  /* DAT_004e2b28 */
    u32 param2;                  /* DAT_004e2b2c */
    u32 param3;                  /* DAT_004e2b30 */
    u32 param4;                  /* DAT_004e2b38 */

    /* Offset 0x18 - from DAT_004e2b34 indexing */
    u32 extra_param;             /* DAT_004e2b34 */

    /* Offset 0x1c */
    u16 field_0x1c;              /* DAT_004e2b48 */
    u16 padding1;

    /* Offset 0x20 - flags from DAT_004e2b4c */
    u16 flags;                   /* Bit 6 = param10, Bit 7 = param11 */

    /* Offset 0x22 - name (max 0x11 = 17 bytes) - DAT_004e2b58 */
    char name[17];

    /* Offset 0x33 - description (max 0x21 = 33 bytes) - DAT_004e2b69 */
    char description[33];

    /* Offset 0x54 */
    u16 x;                       /* Position X */
    u16 y;                       /* Position Y */

    /* Offset 0x58 - sprite field from DAT_004e2bd4 */
    u16 sprite_id;
    u8  state;                   /* DAT_004e2bdc - 0 = inactive */

    u8  padding2[15];

} NPCEntry;  /* 0x10c = 268 bytes */
#pragma pack(pop)

/* Legacy NPC structure for simple use */
typedef struct {
    u32 id;
    u16 sprite_id;
    u16 x;
    u16 y;
    u8  direction;
    u8  action;
    char name[32];
    u16 function;      /* NPC function type */
    u16 shop_id;       /* Shop ID if vendor */
    u16 quest_id;      /* Quest ID if quest giver */
    u32 flags;
} NPC;

/* Character list */
#define MAX_CHARACTERS 100
#define MAX_PETS 5
#define MAX_NPCS 200

/* Global character context */
typedef struct {
    Character player;
    Character characters[MAX_CHARACTERS];
    Pet pets[MAX_PETS];
    NPC npcs[MAX_NPCS];
    u32 character_count;
    u32 npc_count;
} CharacterContext;

extern CharacterContext g_chars;

/* Character system functions */
int character_init(void);
void character_shutdown(void);

/* Player functions */
Character* character_get_player(void);
void character_set_player_info(Character* info);
void character_set_position(u16 x, u16 y);
void character_set_direction(Direction dir);
void character_set_action(CharacterAction action);

/* Movement */
int character_move_to(u16 x, u16 y);
void character_move(int direction);
void character_update_movement(void);
void character_stop_movement(void);

/* Animation */
void character_update_animation(void);
void character_play_action(CharacterAction action);

/* Rendering */
void character_render_player(void);
void character_render_all(void);
void character_render(Character* ch, int screen_x, int screen_y);

/* Other characters */
Character* character_find_by_id(u32 id);
void character_add(Character* ch);
void character_remove(u32 id);
void character_update_all(void);

/* NPC functions - declared in npc.h */
/* npc_find_by_id, npc_find_at_position, npc_add, npc_remove, npc_render_all */


/* Pet functions */
Pet* pet_get_by_index(int index);
void pet_update_all(void);

/* Character creation */
int character_create(Character* ch, const char* name, int job, CharacterAppearance* app);
void character_delete(u32 char_id);

/* Stats calculation */
void character_recalculate_stats(Character* ch);
int character_can_level_up(Character* ch);
void character_level_up(Character* ch);

/* Isometric coordinate transformation - FUN_00446df0, FUN_00446e40 */
void world_to_screen(float world_x, float world_y, float* screen_x, float* screen_y);
void screen_to_world(float screen_x, float screen_y, float* world_x, float* world_y);

/* Character position setup - FUN_00440df0 */
void character_setup_position(u16 x, u16 y);


/* Include sub-module headers */
#include "character_npc.h"
#include "character_protocol.h"

#endif /* CHARACTER_H */
