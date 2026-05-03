/*
 * Stone Age Client - Battle Render Functions Header
 * Reverse engineered from sa_9061.exe
 *
 * Functions called by FUN_00424f50 (battle_action_execute) to render
 * battle action animations and effects.
 */

#ifndef BATTLE_RENDER_H
#define BATTLE_RENDER_H

#include "types.h"

/* Animation state structure */
typedef struct {
    int active;         /* Is animation active */
    int frame;          /* Current frame */
    int timer;          /* Timer counter */
    int effect_id;      /* Current effect ID */
    int target_x;       /* Target X position */
    int target_y;       /* Target Y position */
    int damage_value;   /* Damage to display */
    int heal_value;     /* Heal to display */
    int frame_delay;    /* Frames between updates */
    int total_frames;   /* Total frames in animation */
    int loop;           /* Loop animation flag */
} BattleAnimState;

/* Attack render - FUN_00426380 */
void battle_render_action_0(int variant);

/* Skill render - FUN_00426850 */
void battle_render_action_2(void);

/* Counter render - FUN_00426cc0 */
void battle_render_action_3(void);

/* Defend render - FUN_00427190 */
void battle_render_action_4(void);

/* Charge render - FUN_004276a0 */
void battle_render_action_5(void);

/* Combo render - FUN_00428280 */
void battle_render_action_678(void);

/* Item render - FUN_0042acf0 */
void battle_render_action_9(void);

/* Capture render - FUN_0042ce40 */
void battle_render_action_cd(void);

/* Pet attack render - FUN_0042e8f0 */
void battle_render_action_e(void);

/* Pet skill render - FUN_0042f370 */
void battle_render_action_f(void);

/* Special skill 1 - FUN_00430b50 */
void battle_render_action_12(void);

/* Special skill 2/3 - FUN_00431560 */
void battle_render_action_1314(void);

/* Special skill 4 - FUN_00431d60 */
void battle_render_action_15(void);

/* Summon pet - FUN_00432a10 */
void battle_render_action_16(int variant);

/* Recall pet - FUN_00433030 */
void battle_render_action_17(int variant);

/* Gold display - FUN_00436af0 */
void battle_render_action_18(void);

/* Combo setup - FUN_004338d0 */
void battle_render_action_19(int param);

/* Combo follow-up - FUN_004340a0 */
void battle_render_action_1a(int param);

/* Combo finish - FUN_00434610 */
void battle_render_action_1b(int param);

/* Unknown action 1d - FUN_00434d60 */
void battle_render_action_1d(void);

/* Revive - FUN_004354f0 */
void battle_render_action_1e(void);

/* Berserk - FUN_004366b0 */
void battle_render_action_1f(void);

/* Transform - FUN_00435b40 */
void battle_render_action_20(void);

/* Status effect - FUN_00435f70 */
void battle_render_action_21(void);

/* Chain attack - FUN_00436220 */
void battle_render_action_22(void);

/* Unknown action 23 - FUN_00437200 */
void battle_render_action_23(void);

/* Target select A - FUN_00438080 */
void battle_render_action_2427(void);

/* Target select B - FUN_00438880 */
void battle_render_action_2526(void);

/* Unknown action 28 - FUN_00427cc0 */
void battle_render_action_28(void);

/* Defend stance - FUN_0042bb40 */
void battle_render_action_29(void);

/* Escape - FUN_0042b150 */
void battle_render_action_2a(void);

/* Pet escape - FUN_00439a00 */
void battle_render_action_67(void);

/* Pet swap - FUN_0043a100 */
void battle_render_action_68(void);

/* Pet action - FUN_0043a420 */
void battle_render_action_69(void);

#endif /* BATTLE_RENDER_H */
