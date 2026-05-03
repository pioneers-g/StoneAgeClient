/*
 * Stone Age Client - Network Protocol Data Structures
 * Split from network_protocol.c
 *
 * Contains all data structures and global state variables
 * for the network protocol system
 */

#include <winsock2.h>
#include <windows.h>
#include "types.h"
#include "network.h"

#ifndef NETWORK_PROTOCOL_DATA_H
#define NETWORK_PROTOCOL_DATA_H

/* ========================================
 * Constants
 * ======================================== */

#define MAX_BATTLE_UNITS 10
#define BATTLE_UNIT_SIZE 0x30
#define MAX_PARTY_MEMBERS 10
#define MAX_PETS 5
#define MAX_PET_SKILLS 7

/* ========================================
 * Battle Unit Structure - from DAT_046308f0 region
 * ======================================== */

typedef struct {
    u32 active;          /* Is unit slot active */
    u32 unit_id;         /* Unique unit identifier */
    u16 pos_x;           /* Battle grid X position */
    u16 pos_y;           /* Battle grid Y position */
    u16 direction;       /* Facing direction */
    u16 level;           /* Unit level */
    u16 level_display;   /* Display flag for level >= 100 */
    u16 hp;              /* Current HP */
    u16 max_hp;          /* Maximum HP */
    u16 mp;              /* Current MP */
    u16 sprite_id;       /* Sprite ID for rendering */
    u32 status;          /* Status effect flags */
    u32 action_type;     /* Current/last action type */
    char name[20];       /* Unit name */
    char title[20];      /* Unit title */
} BattleUnit;

/* ========================================
 * Pet Data Structure - from DAT_0462ac10 region
 * ======================================== */

typedef struct {
    u32 active;
    char name[26];
    char title[18];
    char description[74];
    u16 hp;
    u16 max_hp;
    u16 level;
    u16 level_display;
    u16 level_over_100;
    u16 sprite_id;
    s32 field1, field2, field3, field4, field5, field6;
    s32 field7, field8, field9, field10, field11, field12, field13, field14, field15;
} PetData;

/* ========================================
 * Pet Skill Entry - from DAT_0462b008 region
 * ======================================== */

typedef struct {
    u16 field1, field2, field3;
    char name[26];
    char memo[74];
} PetSkillEntry;

/* ========================================
 * Party Entry Structure
 * ======================================== */

typedef struct {
    u32 active;
    char name[30];
    char title[18];
    s32 field1, field2, field3;
    u16 level;
    u16 level_mod;
    u16 level_over_100;
    u16 sprite_id;
} PartyEntry;

/* ========================================
 * Send Queue System - FUN_004115e0
 * ======================================== */

/* ========================================
 * Global State Variables
 * ======================================== */

/* Connection state - DAT_04ebffd8 */
extern u32 s_connection_state;
extern u32 s_login_result;
extern char s_error_message[256];

/* Battle units - DAT_046308f0 */
extern BattleUnit s_battle_units[MAX_BATTLE_UNITS];
extern int s_battle_unit_count;
extern int s_selected_unit;     /* DAT_0462bf32 */
extern int s_target_unit;       /* DAT_0462bf3e */
extern u16 s_battle_unit_flags[MAX_BATTLE_UNITS];

/* Pet data - DAT_0462ac10 */
extern PetData s_pet_data[MAX_PETS];
extern PetSkillEntry s_pet_skills[MAX_PETS][MAX_PET_SKILLS];
extern int s_pet_skill_count[MAX_PETS];
extern int s_selected_pet;
extern int s_battle_pet;

/* Party data */
extern PartyEntry s_party_entries[MAX_PARTY_MEMBERS];
extern int s_party_count;
extern u32 s_party_flag;
extern int s_in_party;

/* Player state */
extern char s_player_name[32];
extern s32 s_player_field1;
extern s32 s_player_field2;
extern s32 s_player_field3;
extern s32 s_player_field4;

/* Movement sync state */
extern u32 s_last_move_time;
extern u32 s_move_target;
extern int s_move_timer_active;

/* Field state */
extern s32 s_current_field_id;
extern s32 s_field_sub_id;
extern s32 s_last_field_id;

/* Battle state */
extern s32 s_battle_field1;
extern s32 s_battle_field2;
extern s32 s_battle_field3;

/* Value update */
extern s32 s_update_value;

/* Map state */
extern int s_map_entry_flag;
extern int s_game_state;
extern int s_game_substate;
extern int s_battle_flag;
extern int s_battle_substate;
extern int s_battle_active_flag;
extern int s_player_clear_flag;

extern int s_map_id;
extern int s_map_width;
extern int s_map_height;
extern int s_map_pos_x;
extern int s_map_pos_y;
extern int s_last_map_id;
extern int s_map_param;

extern int s_view_min_x;
extern int s_view_min_y;
extern int s_view_max_x;
extern int s_view_max_y;
extern int s_view_width;
extern int s_view_height;

extern int s_entity_count;
extern int s_npc_count;
extern int s_render_queue_count;
extern int s_fade_active;

extern float s_iso_camera_x;
extern float s_iso_camera_y;

/* Isometric tile ratio constant */
#define ISO_TILE_RATIO 0.5f

#endif /* NETWORK_PROTOCOL_DATA_H */
