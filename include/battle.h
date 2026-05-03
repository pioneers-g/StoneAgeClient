/*
 * Stone Age Client - Battle System
 * Reverse engineered from sa_9061.exe
 *
 * Main battle header - includes sub-modules:
 * - battle_calc.h: Damage and calculations
 * - battle_protocol.h: Text protocol handlers
 * - battle_action.h: Action dispatcher and handlers
 */

#ifndef BATTLE_H
#define BATTLE_H

#include "types.h"
#include "character.h"

/* Forward declarations for sub-modules */
struct BattleUnit;
struct BattleSkill;

/* Battle states - from FUN_00479c40 and FUN_0040a1a0 sub-states */
typedef enum {
    BATTLE_STATE_NONE = 0,
    BATTLE_STATE_INIT = 1,           /* FUN_00479c40 case 0,1 - Initialize battle */
    BATTLE_STATE_LOAD_FIELD = 2,     /* FUN_00479c40 case 2 - Load battle field */
    BATTLE_STATE_READY = 3,          /* FUN_00479c40 case 3 - Ready to start */
    BATTLE_STATE_WAIT_FIELD = 4,     /* FUN_00479c40 case 4 - Wait for field load */
    BATTLE_STATE_TRANSITION = 5,     /* FUN_00479c40 case 5 - Transition effect */
    BATTLE_STATE_START = 6,          /* FUN_0040a1a0 case 0 - Start battle */
    BATTLE_STATE_INTRO = 7,          /* FUN_0040a1a0 case 1 - Intro animation */
    BATTLE_STATE_MAIN = 8,           /* FUN_0040a1a0 case 2 - Main battle loop */
    BATTLE_STATE_ACTION = 9,         /* FUN_0040a1a0 case 3 - Process action */
    BATTLE_STATE_EXECUTE = 10,       /* FUN_0040a1a0 case 4 - Execute action */
    BATTLE_STATE_RESULT = 11,        /* FUN_0040a1a0 case 5 - Show result */
    BATTLE_STATE_NEXT_TURN = 12,     /* FUN_0040a1a0 case 6 - Next turn */
    BATTLE_STATE_END = 13,           /* FUN_0040a1a0 case 7 - End battle */
    BATTLE_STATE_EXIT = 14,          /* FUN_0040a1a0 case 8 - Exit battle */
    BATTLE_STATE_ANIMATION = 15,     /* Animation playback state */
    BATTLE_STATE_SELECT_ACTION = 16, /* Action selection state */

    /* Extended sub-states from DAT_04630df0 values */
    BATTLE_SUB_STATE_0X96 = 0x96,    /* Special state 150 */
    BATTLE_SUB_STATE_0X65 = 0x65,    /* Special state 101 */
    BATTLE_SUB_STATE_0X66 = 0x66,    /* Special state 102 */
    BATTLE_SUB_STATE_0X67 = 0x67,    /* Special state 103 */
    BATTLE_SUB_STATE_0XC9 = 0xC9,    /* Special state 201 */
    BATTLE_SUB_STATE_0XCA = 0xCA,    /* Special state 202 */
    BATTLE_SUB_STATE_0XCB = 0xCB,    /* Special state 203 */
    BATTLE_SUB_STATE_0XCC = 0xCC     /* Special state 204 */
} BattleState;

/* Battle action types - from FUN_00424b70 dispatcher */
typedef enum {
    BATTLE_ACTION_NONE = 0,
    BATTLE_ACTION_ATTACK = 1,        /* Normal attack */
    BATTLE_ACTION_SKILL = 2,         /* Use skill */
    BATTLE_ACTION_COUNTER = 6,       /* Counter attack */
    BATTLE_ACTION_COMBO = 7,         /* Combo attack */
    BATTLE_ACTION_COMBO_FOLLOW = 8,  /* Combo follow-up */
    BATTLE_ACTION_ITEM = 9,          /* Use item */
    BATTLE_ACTION_DEFEND = 0x29,     /* Defend/Guard */
    BATTLE_ACTION_ESCAPE = 0x2a,     /* Flee battle */
    BATTLE_ACTION_WAIT = 0x0b,       /* Wait/Skip turn */
    BATTLE_ACTION_CAPTURE = 0x0c,    /* Capture pet */
    BATTLE_ACTION_END = 0x0d,        /* End battle */
    BATTLE_ACTION_PET_ATTACK = 0x0e, /* Pet attack */
    BATTLE_ACTION_PET_SKILL = 0x0f,  /* Pet skill */

    /* Extended action types from FUN_00424b70 */
    BATTLE_ACTION_SKILL_1 = 0x12,    /* Skill type 1 */
    BATTLE_ACTION_SKILL_2 = 0x13,    /* Skill type 2 */
    BATTLE_ACTION_SKILL_3 = 0x14,    /* Skill type 3 */
    BATTLE_ACTION_SKILL_4 = 0x15,    /* Skill type 4 */
    BATTLE_ACTION_SUMMON = 0x16,     /* Summon pet */
    BATTLE_ACTION_RECALL = 0x17,     /* Recall pet */
    BATTLE_ACTION_PET_ITEM = 0x18,   /* Pet use item */
    BATTLE_ACTION_SPECIAL_1 = 0x19,  /* Special action 1 */
    BATTLE_ACTION_SPECIAL_2 = 0x1a,  /* Special action 2 */
    BATTLE_ACTION_SPECIAL_3 = 0x1b,  /* Special action 3 */
    BATTLE_ACTION_DEATH = 0x1c,      /* Death handler */
    BATTLE_ACTION_REVIVE = 0x1e,     /* Revive */
    BATTLE_ACTION_BERSERK = 0x1f,    /* Berserk mode */
    BATTLE_ACTION_TRANSFORM = 0x20,  /* Transform */
    BATTLE_ACTION_CHAIN = 0x22,      /* Chain skill */
    BATTLE_ACTION_SKILL_AOE = 0x2b,  /* AOE skill */
    BATTLE_ACTION_FIELD_SKILL = 0x2c,/* Field effect skill */
    BATTLE_ACTION_PET_ESCAPE = 0x67, /* Pet escape */
    BATTLE_ACTION_PET_SWAP = 0x68    /* Swap pet */
} BattleAction;

/* Battle position types */
typedef enum {
    BATTLE_POS_LEFT_FRONT = 0,
    BATTLE_POS_LEFT_BACK = 1,
    BATTLE_POS_RIGHT_FRONT = 2,
    BATTLE_POS_RIGHT_BACK = 3
} BattlePosition;

/* Additional buff flags not in types.h */
#define BUFF_BERSERK        (1 << 8)
#define BUFF_STONE          (1 << 9)
#define BUFF_DRUNK          (1 << 11)
#define BUFF_PARALYZE       (1 << 13)
#define BUFF_SILENCE        (1 << 14)
#define BUFF_BLIND          (1 << 15)

/* Element types - Stone Age element system */
typedef enum {
    ELEMENT_NONE = 0,
    ELEMENT_FIRE = 1,
    ELEMENT_WATER = 2,
    ELEMENT_EARTH = 3,
    ELEMENT_WIND = 4,
    ELEMENT_LIGHT = 5,
    ELEMENT_DARK = 6,
    ELEMENT_MAX = 7
} ElementType;

/* Battle unit structure */
typedef struct {
    u32 id;
    u32 char_id;          /* Character/Pet ID */
    u16 sprite_id;
    u16 x, y;             /* Battle position */
    u8  side;             /* 0 = player, 1 = enemy */
    u8  position;         /* Front/back row */
    u8  is_pet;           /* Is this a pet? */
    u8  is_alive;
    u8  is_active;        /* Can act this round */
    u8  level;            /* Unit level */

    /* Name - 16 chars + null */
    char name[17];

    /* Stats */
    u16 hp;
    u16 max_hp;
    u16 mp;
    u16 max_mp;
    u16 attack;
    u16 defense;
    u16 speed;
    u16 magic_attack;      /* Magic attack power */
    u16 magic_defense;     /* Magic defense */
    u8  element;           /* Unit element */
    u8  flags;             /* Unit flags */

    /* Buffs/Debuffs */
    u32 buff_flags;
    u8  buff_turns[16];

    /* Action for this round */
    BattleAction action;
    u32 target_id;
    u16 skill_id;
    u16 item_id;

    /* Animation */
    u16 animation;        /* Current animation ID */
    u16 frame;
    u16 effect_id;        /* Visual effect ID */
    u16 effect_timer;     /* Effect duration */
    s16 offset_x, offset_y;
    u8  shake_timer;

    /* Action state for network sync */
    u8  action_state;     /* Action state machine state */
    u8  action_result;    /* Action result code */

} BattleUnit;

/* Battle skill structure */
typedef struct {
    u16 id;
    char name[32];
    u16 mp_cost;
    u16 type;           /* Attack, heal, buff, etc */
    u16 power;
    u16 range;          /* Single, all, area */
    u16 element;
    u16 animation_id;
    u16 cooldown;
    u32 flags;
} BattleSkill;

/* Battle result */
typedef struct {
    u32 attacker_id;
    u32 target_id;
    u16 damage;
    u16 flags;
    u8  critical;
    u8  miss;
    u16 skill_id;
} BattleResult;

/* Damage number display structure */
typedef struct {
    s32 value;
    u16 x;
    u16 y;
    u16 timer;
    u8  flags;
    u8  is_critical;
    u8  is_heal;
    u8  reserved;
} DamageNumber;

/* Battle context - matching DAT_0455ef98 region */
typedef struct {
    BattleState state;
    u32 battle_id;
    u32 map_id;

    /* Units - DAT_0455ef94 bitmask for active units */
    BattleUnit units[20];      /* Max 10 per side */
    BattleUnit* enemies;       /* Pointer to enemy units */
    BattleUnit* allies;        /* Pointer to ally units */
    u32 unit_count;            /* DAT_0455b570 */
    u32 player_count;
    u32 enemy_count;
    u32 ally_count;            /* Number of allies */
    u32 unit_flags;            /* DAT_0455ef94 - bitmask of active units */

    /* Turn management */
    u32 current_turn;
    u32 turn_count;            /* DAT_045e184c from FUN_00465160 */
    u32 turn_order[20];        /* Sorted unit indices by speed */
    u32 action_queue[20];
    u32 action_count;
    u32 current_actor;
    u32 last_action_type;      /* Type of last action performed */

    /* Player position before battle - DAT_04581d3c, DAT_04581d40 */
    u16 player_x;
    u16 player_y;

    /* Pet status */
    u8  pet_hp_percent;        /* Pet HP percentage */
    u8  pet_mp_percent;        /* Pet MP percentage */
    u8  pet_flags;             /* Pet flags */
    u8  padding_pet;

    /* Battle field */
    u16 field_tiles[400];      /* 20x20 grid */
    u32 field_id;
    u16 field_sprite;          /* Battle background sprite */
    u8  field_loaded;
    u8  padding1;

    /* Current action - DAT_004b83ec */
    BattleAction current_action;
    u32 action_target;
    u32 action_param1;
    u32 action_param2;

    /* Selected targets for multi-target skills - DAT_004b87c4-DAT_004b87d4 */
    s32 selected_targets[6];
    u32 selected_count;        /* DAT_0455f054 */

    /* Rewards */
    u32 exp_reward;
    u32 gold_reward;
    u32 item_rewards[10];
    u32 capture_target;        /* Pet to capture */

    /* Capture result flags - DAT_04582a22, DAT_04582a28, DAT_04582a38, DAT_04582a34 */
    u16 capture_flags;
    u16 capture_result[3];
    u32 capture_pet_id;
    u16 capture_timer;

    /* Animation state */
    u32 animation_timer;
    u32 animation_id;
    u32 result_count;
    BattleResult results[20];

    /* Damage number display */
    DamageNumber damage_numbers[20];
    u32 damage_count;

    /* Battle message */
    char message[128];
    u8  message_type;
    u16 message_timer;

    /* Result animation */
    u8  result_animation;
    u8  result_timer;

    /* Battle menu position - DAT_04557a90, DAT_0455b100 */
    u16 menu_x;
    u16 menu_y;

    /* Flags */
    u8  active;                /* DAT_0455ef9c */
    u8  can_escape;
    u8  is_pvp;
    u8  is_boss;
    u8  ended;
    u8  victory;
    u8  is_combo;              /* DAT_0454f108 */
    u8  padding2;

    /* UI state - from FUN_00418330 */
    u32 ui_flag1;              /* DAT_005676a0 */
    u16 ui_flag2;              /* DAT_00564e62 */
    u32 ui_flag3;              /* DAT_005676b4 */
    u32 ui_flag4;              /* DAT_0056768c */
    u32 action_state;          /* DAT_004aa070 */
    u16 battle_coord_x;        /* DAT_00564e3c */
    u16 battle_coord_y;        /* DAT_00564e3e */
    u32 state_flag;            /* DAT_00567690 */

    /* FUN_0040a1a0 state machine fields */
    u32 entity_ref1;           /* DAT_0461c2c0 */
    u32 entity_ref2;           /* DAT_0461c680 */
    u32 entity_ref3;           /* DAT_0461c684 */
    u32 dungeon_entities[9];   /* Entity slots for special maps */
    u32 frame_counter;         /* DAT_04ebe31c */
    u32 message_index;         /* DAT_0464f488 */
    u32 action_queue_pos;      /* DAT_005ab6f8 */
    u32 render_phase;          /* DAT_004bb410 */
    u8  intro_phase;           /* Fade state */
    u8  fade_complete;         /* Fade effect complete flag */
    u8  fade_active;           /* Fade active flag */
    u8  copy_message_flag;     /* Message copy flag */
    u32 selected_index;        /* Current selection */
    u32 target_index;          /* Current target */
    u32 entity_queue[20];      /* Entity queue array */
    u32 wait_dialog_id;        /* Wait dialog window ID */
    u32 wait_dialog_button_count;
    u32 action_result_count;   /* Action result counter */
    u32 action_result_read;    /* Read position for action results */
    char action_results[4][4096]; /* Action result messages - 0x1000 each */
    char display_message[256]; /* Current display message */
    char source_message[256];  /* Source for copy */
    char current_message[256]; /* Current message buffer */
    char defeat_message[64];   /* Defeat message */
    u32 result_queue_count;    /* Result queue count */
    u32 result_queue_read;     /* Result queue read position */
    char result_queue[4][4096]; /* Result messages */
    u32 action_type;           /* Current action type */
    u32 current_unit_index;    /* Current unit being processed */
    u32 ai_mode;               /* AI mode (3 = on) */
    u32 party_update_pending;  /* Party update needed */
    u32 player_dest;           /* Player destination */
    u32 after_battle_delay;    /* After battle delay */
    u32 after_battle_state;    /* After battle state */
    u32 return_flag;           /* Return flag */
    u32 special_flags;         /* Special battle flags */
    u32 escape_request;        /* Escape request flag */

} BattleContext;

/* Global battle context */
extern BattleContext g_battle;

/* Battle system functions */
int battle_init(void);
void battle_shutdown(void);

/* UI state initialization - FUN_00418330 */
void battle_init_ui_state(void);

/* Battle start/end */
int battle_start(u32 battle_id, u32 enemy_count, u32* enemy_ids);
void battle_end(void);
void battle_victory(void);
void battle_defeat(void);

/* Battle update */
void battle_update(void);
void battle_render(void);

/* Unit management */
BattleUnit* battle_get_unit(u32 id);
BattleUnit* battle_get_unit_by_index(u32 index);
void battle_add_unit(BattleUnit* unit, u8 side);
void battle_remove_unit(u32 id);
void battle_update_units(void);

/* Action handling */
void battle_select_action(u32 unit_id, BattleAction action, u32 target_id, u16 skill_id);
void battle_execute_actions(void);
void battle_execute_action(BattleUnit* attacker);
void battle_process_result(BattleResult* result);

/* Turn management */
void battle_next_turn(void);
void battle_sort_actions(void);
void battle_end_turn(void);

/* Escape */
int battle_try_escape(void);

/* Capture */
int battle_try_capture(BattleUnit* target);

/* Animation */
void battle_play_animation(u32 animation_id, u32 source_id, u32 target_id);
void battle_render_background(void);
void battle_render_units(void);
void battle_render_ui(void);

/* Battle unit status protocol - case 0x4e handler */
#define BATTLE_UNIT_UPDATE_ID       (1 << 1)   /* Bit 1: char_id */
#define BATTLE_UNIT_UPDATE_HP       (1 << 2)   /* Bit 2: hp, max_hp */
#define BATTLE_UNIT_UPDATE_MP       (1 << 3)   /* Bit 3: mp, max_mp */
#define BATTLE_UNIT_UPDATE_LEVEL    (1 << 4)   /* Bit 4: level */
#define BATTLE_UNIT_UPDATE_STATUS   (1 << 5)   /* Bit 5: status flags */
#define BATTLE_UNIT_UPDATE_NAME     (1 << 6)   /* Bit 6: name string */

void battle_unit_status_update(int slot_index, u32 bitmask, const char* data);
void battle_unit_remove(int slot_index);
int battle_find_unit_slot(u32 char_id);

/* Rendering */
void battle_render_effects(void);
void battle_render_combatants(void);

/* AI */
void battle_ai_select_action(BattleUnit* unit);

/* Battle field - FUN_00404850 */
int battle_load_field(u32 field_id);
void battle_render_field(void);

/* Battle action handlers - FUN_00424b70 pattern */
void battle_handle_action(u32 action, u32 target_x, u32 target_y,
                          u32 param1, u32 param2, const char* message);

/* Battle result handlers - FUN_00464ac0, FUN_00464db0 */
void battle_handle_result(u32 attacker, u32 target, u16 damage, u16 flags, u32 skill_id);
void battle_handle_capture(u32 flags, u32 result);

/* Battle position setup - FUN_00440df0 */
void battle_setup_positions(u32 x, u32 y);

/* Battle start/end handlers - FUN_00465390, FUN_004653d0 */
void battle_on_start(u32 battle_id, u32 field_id);
void battle_on_end(u32 param1, u32 param2, u32 param3, u32 param4, int result);

/* Turn handler - FUN_00465160 */
void battle_set_turn(u32 turn);

/* Get battle unit by array index */
BattleUnit* battle_get_unit_at(u32 index);

/* Check if battle is active */
int battle_is_active(void);

/* Battle state helpers - FUN_0040a1a0 pattern */
void battle_clear_entity_queue(void);
void battle_reset_ai_state(void);
void battle_clear_unit_flags(void);
void battle_clear_render_queue(void);
int battle_is_special_map(u32 map_id);
u32 battle_create_dungeon_entity(u32 index);
void battle_init_render_queue(void);
void battle_menu_init(void);
u32 battle_get_bgm_for_map(u32 map_id);
void battle_start_fade(u32 fade_type);
void battle_send_party_update(void);
void battle_handle_unit_action_response(u32 unit_index);
u32 battle_get_action_type(void);
void battle_execute_pending_action(void);
void battle_update_state(void);
void battle_handle_action_results(void);
void battle_ai_execute_action(void);
void battle_ai_update(void);
void battle_player_execute_action(void);
void battle_update_unit_display(u32 unit_index);
void battle_draw_text(u32 x, u32 y, u32 param1, u32 param2, const char* text, u32 flags);
void battle_update_unit_displays(void);
void battle_quick_update(void);
void battle_process_network(void);
void battle_free_entity(u32 entity_id);
void battle_clear_state(void);
int battle_find_rideable_pet(void);
int battle_check_end_condition(void);

/* Include sub-modules after struct definitions */
#include "battle_calc.h"
#include "battle_protocol.h"
#include "battle_action.h"

#endif /* BATTLE_H */
