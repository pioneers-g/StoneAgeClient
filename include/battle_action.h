/*
 * Stone Age Client - Battle Action Handler
 * Reverse engineered from sa_9061.exe (FUN_00424b70)
 *
 * Battle action dispatcher that handles all battle actions
 * including attacks, skills, items, escape, capture, etc.
 *
 * Module split (1392 lines -> 6 files):
 * - battle_action_core.c: Core dispatcher, init, parsing (~350 lines)
 * - battle_action_attack.c: Attack actions (~200 lines)
 * - battle_action_skill.c: Skill actions (~200 lines)
 * - battle_action_item.c: Item actions (~150 lines)
 * - battle_action_pet.c: Pet actions (~300 lines)
 * - battle_action_special.c: Special actions (~350 lines)
 * - battle_action_render.c: Execute/render (~250 lines)
 */

#ifndef BATTLE_ACTION_H
#define BATTLE_ACTION_H

#include "types.h"

/* Action handler result codes */
typedef enum {
    ACTION_RESULT_CONTINUE = 0,     /* Continue with action */
    ACTION_RESULT_SKIP = 1,         /* Skip this action */
    ACTION_RESULT_END_BATTLE = 2,   /* Battle should end */
    ACTION_RESULT_ERROR = -1,       /* Error occurred */
    ACTION_RESULT_INVALID = -2      /* Invalid action */
} ActionResult;

/* Action context - parameters passed to action handlers */
typedef struct {
    u32 action_type;        /* Action type from FUN_00424b70 param_1 */
    u32 param1;             /* param_2 - Usually target info */
    u32 param2;             /* param_3 - Usually x/position */
    u32 param3;             /* param_4 - Usually y/position */
    const char* message;    /* param_5 - Message string */

    /* Parsed data from message */
    int parsed_int[16];     /* Parsed integers from pipe-delimited message */
    char parsed_str[8][64]; /* Parsed strings */
    int parse_count;        /* Number of parsed elements */
} ActionContext;

/* Action handler function type */
typedef void (*ActionHandler)(const ActionContext* ctx);

/* Action info structure for dispatcher table */
typedef struct {
    u32 action_type;
    const char* name;
    ActionHandler handler;
    u32 flags;
} ActionInfo;

/* Battle effect structure for visual effects */
#define MAX_BATTLE_EFFECTS  32

typedef struct {
    u16 id;             /* Effect ID */
    u16 x;              /* Screen X position */
    u16 y;              /* Screen Y position */
    u16 timer;          /* Duration in frames */
    u16 frame;          /* Current animation frame */
    u8  active;         /* Is effect active */
    u8  reserved;
} BattleEffect;

/* Global action state - matches DAT_004b83ec region */
typedef struct {
    /* Current action being processed */
    u32 current_action;         /* DAT_004b83ec */
    u32 action_param1;          /* DAT_0455ef94 */
    u32 action_param2;          /* DAT_04558c34 */
    u32 action_param3;          /* DAT_0455b5ac */

    /* Action flags */
    int is_combo;               /* DAT_0454f108 - Combo follow-up flag */
    int action_ended;           /* Action has ended */

    /* Player position during action */
    u16 player_x;               /* DAT_045528c8 */
    u16 player_y;               /* DAT_0454fe98 */

    /* Escape attempt data - from FUN_0042af40 */
    int escape_attempt;         /* DAT_0455b0f0 */
    int escape_count;           /* DAT_0454f400 */
    int escape_max;             /* DAT_0455b370 (0x50 = 80) */

    /* Capture data - from FUN_004262f0 */
    char capture_name[26];      /* DAT_045529ec */
    char capture_target[26];    /* DAT_0454e000 */

    /* Message buffer - from DAT_04554270 */
    char message_buffer[256];

    /* Skill data */
    int skill_type;             /* DAT_0454f5b0 */

    /* Death message */
    int death_flag;             /* DAT_0455efa0 */

    /* Pet battle state */
    u8  summoned_pet_slot;      /* Currently summoned pet slot (0xFF = none) */
    u8  counter_unit;           /* Unit in counter stance */
    u8  combo_unit;             /* Unit initiating combo */

    /* Visual effects queue */
    BattleEffect effects[MAX_BATTLE_EFFECTS];
    u32 effect_count;

} BattleActionState;

/* Global state */
extern BattleActionState g_battle_action;

/* Initialization */
int battle_action_init(void);
void battle_action_shutdown(void);

/* Main dispatcher - FUN_00424b70 */
void battle_action_dispatch(u32 action, u32 param1, u32 param2, u32 param3, const char* message);

/* Action handlers - implemented from FUN_00424b70 switch cases */
void battle_action_attack(const ActionContext* ctx);          /* Action 0, 1, 0x2d */
void battle_action_skill(const ActionContext* ctx);           /* Action 2 */
void battle_action_counter(const ActionContext* ctx);         /* Action 6 */
void battle_action_combo(const ActionContext* ctx);           /* Action 7, 8 */
void battle_action_item(const ActionContext* ctx);            /* Action 9 */
void battle_action_defend(const ActionContext* ctx);          /* Action 0x29 */
void battle_action_escape(const ActionContext* ctx);          /* Action 0x2a */
void battle_action_wait(const ActionContext* ctx);            /* Action 10, 0x0b */
void battle_action_capture(const ActionContext* ctx);         /* Action 0x0c */
void battle_action_end(const ActionContext* ctx);             /* Action 0x0d */
void battle_action_pet_attack(const ActionContext* ctx);      /* Action 0x0e */
void battle_action_pet_skill(const ActionContext* ctx);       /* Action 0x0f */
void battle_action_berserk(const ActionContext* ctx);         /* Action 0x1f */
void battle_action_skill_type1(const ActionContext* ctx);     /* Action 0x12 */
void battle_action_skill_type2(const ActionContext* ctx);     /* Action 0x13 */
void battle_action_skill_type3(const ActionContext* ctx);     /* Action 0x14 */
void battle_action_skill_type4(const ActionContext* ctx);     /* Action 0x15 */
void battle_action_summon(const ActionContext* ctx);          /* Action 0x16, 0x2c */
void battle_action_recall(const ActionContext* ctx);          /* Action 0x17, 0x2b */
void battle_action_special1(const ActionContext* ctx);        /* Action 0x19 */
void battle_action_special2(const ActionContext* ctx);        /* Action 0x1a */
void battle_action_special3(const ActionContext* ctx);        /* Action 0x1b */
void battle_action_death(const ActionContext* ctx);           /* Action 0x1c */
void battle_action_revive(const ActionContext* ctx);          /* Action 0x1e */
void battle_action_transform(const ActionContext* ctx);       /* Action 0x20 */
void battle_action_chain(const ActionContext* ctx);           /* Action 0x22 */
void battle_action_target_select(const ActionContext* ctx);   /* Action 0x24, 0x25, 0x26, 0x27 */
void battle_action_pet_escape(const ActionContext* ctx);      /* Action 0x67 */
void battle_action_pet_swap(const ActionContext* ctx);        /* Action 0x68 */

/* Message parsing - FUN_00489f70 pattern */
int battle_action_parse_message(const char* message, ActionContext* ctx);

/* Helper functions */
void battle_action_set_player_pos(u16 x, u16 y);
void battle_action_clear_state(void);
int battle_action_is_active(void);

/* Query functions */
u32 battle_action_get_current(void);
const char* battle_action_get_message(void);
int battle_action_is_combo(void);

/* Action execution - FUN_00424f50 */
void battle_action_execute(void);

/* Position check for action */
int battle_action_check_position(void);

#endif /* BATTLE_ACTION_H */
