/*
 * Stone Age Client - Battle Action Executor Header
 * Reverse engineered from sa_9061.exe (FUN_00424f50)
 *
 * Battle action execution dispatcher for rendering and processing
 * Handles 30+ different action types for combat UI
 */

#ifndef BATTLE_ACTION_EXEC_H
#define BATTLE_ACTION_EXEC_H

#include "types.h"

/* ========================================
 * Action Type Definitions
 * From FUN_00424f50 switch cases
 * ======================================== */

typedef enum {
    /* Basic actions */
    ACTION_TYPE_ATTACK_MELEE   = 0,     /* FUN_00426380(0) */
    ACTION_TYPE_ATTACK_RANGED  = 1,     /* FUN_00426380(1) */
    ACTION_TYPE_SKILL          = 2,     /* FUN_00426850 */
    ACTION_TYPE_COUNTER_SETUP  = 3,     /* FUN_00426cc0 */
    ACTION_TYPE_COMBO_START    = 4,     /* FUN_00427190 */
    ACTION_TYPE_COMBO_CONTINUE = 5,     /* FUN_004276a0 */
    ACTION_TYPE_COUNTER        = 6,     /* FUN_00428280 */
    ACTION_TYPE_COMBO          = 7,     /* FUN_00428280 */
    ACTION_TYPE_COMBO_END      = 8,     /* FUN_00428280 */
    ACTION_TYPE_ITEM           = 9,     /* FUN_0042acf0 */
    ACTION_TYPE_WAIT           = 10,    /* FUN_00426380(0) */
    ACTION_TYPE_WAIT_ALT       = 11,    /* FUN_00426380(1) */

    /* Pet actions */
    ACTION_TYPE_CAPTURE        = 12,    /* FUN_0042ce40 */
    ACTION_TYPE_BATTLE_END     = 13,    /* FUN_0042ce40 */
    ACTION_TYPE_PET_ATTACK     = 14,    /* FUN_0042e8f0 */
    ACTION_TYPE_PET_SKILL      = 15,    /* FUN_0042f370 */

    /* Skill variations */
    ACTION_TYPE_SKILL_CHAIN    = 18,    /* FUN_00430b50 */
    ACTION_TYPE_SKILL_AREA     = 19,    /* FUN_00431560 */
    ACTION_TYPE_SKILL_ALL      = 20,    /* FUN_00431560 */
    ACTION_TYPE_SKILL_SPECIAL  = 21,    /* FUN_00431d60 */

    /* Pet management */
    ACTION_TYPE_SUMMON         = 22,    /* FUN_00432a10(0) */
    ACTION_TYPE_RECALL         = 23,    /* FUN_00433030(0) */
    ACTION_TYPE_PET_WAIT       = 24,    /* FUN_00436af0 */
    ACTION_TYPE_PET_DEFEND     = 25,    /* FUN_004338d0(0) */
    ACTION_TYPE_PET_ESCAPE     = 26,    /* FUN_004340a0(0) */
    ACTION_TYPE_PET_ITEM       = 27,    /* FUN_00434610(0) */

    /* Special actions */
    ACTION_TYPE_DEATH          = 29,    /* FUN_00434d60 */
    ACTION_TYPE_REVIVE         = 30,    /* FUN_004354f0 */
    ACTION_TYPE_BERSERK        = 31,    /* FUN_004366b0 */
    ACTION_TYPE_TRANSFORM      = 32,    /* FUN_00435b40 */
    ACTION_TYPE_CHAIN_ATTACK   = 33,    /* FUN_00435f70 */
    ACTION_TYPE_COMBO_FINISH   = 34,    /* FUN_00436220 */
    ACTION_TYPE_SPECIAL_MOVE   = 35,    /* FUN_00437200 */

    /* Target selection */
    ACTION_TYPE_TARGET_SINGLE  = 36,    /* FUN_00438080 */
    ACTION_TYPE_TARGET_AREA    = 37,    /* FUN_00438880 */
    ACTION_TYPE_TARGET_ALL     = 38,    /* FUN_00438080 */
    ACTION_TYPE_TARGET_RANDOM  = 39,    /* FUN_00438880 */

    /* Defense and escape */
    ACTION_TYPE_DEFEND         = 40,    /* FUN_00427cc0 */
    ACTION_TYPE_ESCAPE         = 41,    /* FUN_0042bb40 */
    ACTION_TYPE_ESCAPE_FAILED  = 42,    /* FUN_0042b150 */

    /* Pet variations */
    ACTION_TYPE_RECALL_ALT     = 43,    /* FUN_00433030(1) */
    ACTION_TYPE_SUMMON_ALT     = 44,    /* FUN_00432a10(1) */
    ACTION_TYPE_ATTACK_SPECIAL = 45,    /* FUN_00426380(2) */

    /* Pet battle system */
    ACTION_TYPE_PET_BATTLE_END = 103,   /* FUN_00439a00 */
    ACTION_TYPE_PET_SWAP       = 104,   /* FUN_0043a100 */
    ACTION_TYPE_PET_BATTLE_ACT = 105,   /* FUN_0043a420 */

} BattleActionType;

/* ========================================
 * Action UI State
 * From DAT_0455ef98, DAT_004b83ec regions
 * ======================================== */

/* Button positions for action UI - from FUN_00426380 */
typedef struct {
    int x;              /* X position */
    int y;              /* Y position */
    int width;          /* Button width */
    int height;         /* Button height */
    u32 widget_id;      /* Widget ID for click detection */
    int enabled;        /* Is button enabled */
} ActionButton;

/* Skill display data */
typedef struct {
    char name[100];     /* Skill name buffer - DAT_04556678 region */
    int mp_cost;        /* MP cost */
    int skill_id;       /* Skill ID */
    int target_type;    /* Target type (single, area, all) */
} SkillDisplayEntry;

/* Action execution state - matches binary state machine */
typedef struct {
    /* Current action being executed */
    u32 current_action;         /* DAT_004b83ec */
    u32 action_phase;           /* Current phase of action */

    /* UI window state */
    u32 action_window_id;       /* DAT_0455ef98 - Window widget ID */
    u32 skill_window_id;        /* Skill selection window */

    /* Button state arrays */
    ActionButton action_buttons[6];  /* Main action buttons */
    ActionButton skill_buttons[6];   /* Skill selection buttons */

    /* Position tracking */
    u16 player_target_x;        /* DAT_045528c8 - Target X */
    u16 player_target_y;        /* DAT_0454fe98 - Target Y */
    u16 player_current_x;       /* DAT_04581d3c - Current X */
    u16 player_current_y;       /* DAT_04581d40 - Current Y */

    /* Action flags */
    u32 action_flags;           /* DAT_0455ef94 - Bitmask for available actions */
    u32 cancel_flag;            /* Cancel action flag */
    u32 scroll_active;          /* DAT_0455ef9c - Scroll/movement active */

    /* Skill selection state */
    int skill_count;            /* DAT_0454efcc - Number of skills */
    int skill_selected;         /* DAT_0454f5b0 - Selected skill index */
    int skill_scroll;           /* DAT_04552fb0 - Skill scroll position */

    /* Display data */
    SkillDisplayEntry skills[10];  /* DAT_04556678 - Skill name array */

    /* UI layout */
    int window_width;           /* Window dimensions */
    int window_height;
    int button_height;          /* DAT_0455b0dc = 0x14 (20) */

} BattleActionExecState;

/* Global state */
extern BattleActionExecState g_battle_exec;

/* ========================================
 * Action Execution Functions
 * ======================================== */

/*
 * Main action executor - FUN_00424f50
 * Dispatches to appropriate handler based on current action type
 */
void battle_action_execute(void);

/*
 * Initialize action execution state
 */
void battle_action_exec_init(void);

/*
 * Cleanup action execution
 */
void battle_action_exec_shutdown(void);

/*
 * Set current action type
 */
void battle_action_set_type(u32 action_type);

/*
 * Get current action type
 */
u32 battle_action_get_type(void);

/* ========================================
 * Individual Action Renderers
 * From FUN_00424f50 switch cases
 * ======================================== */

/* Attack/Wait UI - FUN_00426380 */
void battle_action_render_attack(int attack_type);

/* Skill selection UI - FUN_00426850 */
void battle_action_render_skill_select(void);

/* Counter setup UI - FUN_00426cc0 */
void battle_action_render_counter_setup(void);

/* Combo attack UI - FUN_00427190, FUN_004276a0 */
void battle_action_render_combo(int combo_phase);

/* Counter/Combo execution - FUN_00428280 */
void battle_action_render_counter_exec(void);

/* Item selection UI - FUN_0042acf0 */
void battle_action_render_item_select(void);

/* Capture pet UI - FUN_0042ce40 */
void battle_action_render_capture(void);

/* Pet attack UI - FUN_0042e8f0 */
void battle_action_render_pet_attack(void);

/* Pet skill UI - FUN_0042f370 */
void battle_action_render_pet_skill(void);

/* Skill chain UI - FUN_00430b50 */
void battle_action_render_skill_chain(void);

/* Area skill UI - FUN_00431560 */
void battle_action_render_skill_area(void);

/* Special skill UI - FUN_00431d60 */
void battle_action_render_skill_special(void);

/* Summon pet UI - FUN_00432a10 */
void battle_action_render_summon(int is_alt);

/* Recall pet UI - FUN_00433030 */
void battle_action_render_recall(int is_alt);

/* Pet wait UI - FUN_00436af0 */
void battle_action_render_pet_wait(void);

/* Pet defend UI - FUN_004338d0 */
void battle_action_render_pet_defend(void);

/* Pet escape UI - FUN_004340a0 */
void battle_action_render_pet_escape(void);

/* Pet item UI - FUN_00434610 */
void battle_action_render_pet_item(void);

/* Death UI - FUN_00434d60 */
void battle_action_render_death(void);

/* Revive UI - FUN_004354f0 */
void battle_action_render_revive(void);

/* Berserk UI - FUN_004366b0 */
void battle_action_render_berserk(void);

/* Transform UI - FUN_00435b40 */
void battle_action_render_transform(void);

/* Chain attack UI - FUN_00435f70 */
void battle_action_render_chain(void);

/* Combo finish UI - FUN_00436220 */
void battle_action_render_combo_finish(void);

/* Special move UI - FUN_00437200 */
void battle_action_render_special_move(void);

/* Target selection UI - FUN_00438080, FUN_00438880 */
void battle_action_render_target_select(int selection_type);

/* Defend UI - FUN_00427cc0 */
void battle_action_render_defend(void);

/* Escape UI - FUN_0042bb40, FUN_0042b150 */
void battle_action_render_escape(int success);

/* Pet battle end UI - FUN_00439a00 */
void battle_action_render_pet_battle_end(void);

/* Pet swap UI - FUN_0043a100 */
void battle_action_render_pet_swap(void);

/* Pet battle action UI - FUN_0043a420 */
void battle_action_render_pet_battle_action(void);

/* ========================================
 * Helper Functions
 * ======================================== */

/* Check position for action validity */
int battle_action_check_position(void);

/* Send action to server */
void battle_action_send_to_server(u32 action, u32 param1, u32 param2, u32 param3, const char* message);

/* Process button click */
int battle_action_process_click(int x, int y);

/* Update scroll state */
void battle_action_update_scroll(void);

/* Render action buttons */
void battle_action_render_buttons(void);

/* Render skill list */
void battle_action_render_skill_list(void);

#endif /* BATTLE_ACTION_EXEC_H */
