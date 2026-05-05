/*
 * Stone Age Client - Battle Render Simple Actions
 * Split from battle_render.c
 *
 * Contains simple action render stubs for various battle actions.
 * These functions primarily set fade palette for battle transitions.
 */

#include <windows.h>
#include "types.h"
#include "battle_render.h"
#include "logger.h"

/* Set render area for battle fade effects - FUN_00488190 */
extern void FUN_00488190(int r, int g, int b);

/* Counter render - FUN_00426cc0 - Action 3 */
void battle_render_action_3(void) {
    LOG_DEBUG("Render counter attack");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Defend render - FUN_00427190 - Action 4 */
void battle_render_action_4(void) {
    LOG_DEBUG("Render defend");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Charge render - FUN_004276a0 - Action 5 */
void battle_render_action_5(void) {
    LOG_DEBUG("Render charge");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Combo render - FUN_00428280 - Actions 6,7,8 */
void battle_render_action_678(void) {
    LOG_DEBUG("Render combo");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Item render - FUN_0042acf0 - Action 9 */
void battle_render_action_9(void) {
    LOG_DEBUG("Render item use");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Pet attack render - FUN_0042e8f0 - Action 0xe */
void battle_render_action_e(void) {
    LOG_DEBUG("Render pet attack");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Pet skill render - FUN_0042f370 - Action 0xf */
void battle_render_action_f(void) {
    LOG_DEBUG("Render pet skill");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Special skill 1 - FUN_00430b50 - Action 0x12 */
void battle_render_action_12(void) {
    LOG_DEBUG("Render special skill 1");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Special skill 2/3 - FUN_00431560 - Actions 0x13, 0x14 */
void battle_render_action_1314(void) {
    LOG_DEBUG("Render special skill 2/3");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Special skill 4 - FUN_00431d60 - Action 0x15 */
void battle_render_action_15(void) {
    LOG_DEBUG("Render special skill 4");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Summon pet - FUN_00432a10 - Actions 0x16, 0x2c */
void battle_render_action_16(int variant) {
    LOG_DEBUG("Render summon pet variant %d", variant);
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Recall pet - FUN_00433030 - Actions 0x17, 0x2b */
void battle_render_action_17(int variant) {
    LOG_DEBUG("Render recall pet variant %d", variant);
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Gold display - FUN_00436af0 - Action 0x18 */
void battle_render_action_18(void) {
    LOG_DEBUG("Render gold display");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Combo setup - FUN_004338d0 - Action 0x19 */
void battle_render_action_19(int param) {
    LOG_DEBUG("Render combo setup param=%d", param);
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Combo follow-up - FUN_004340a0 - Action 0x1a */
void battle_render_action_1a(int param) {
    LOG_DEBUG("Render combo follow-up param=%d", param);
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Combo finish - FUN_00434610 - Action 0x1b */
void battle_render_action_1b(int param) {
    LOG_DEBUG("Render combo finish param=%d", param);
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Action 1d - FUN_00434d60 */
void battle_render_action_1d(void) {
    LOG_DEBUG("Render action 1d");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Revive - FUN_004354f0 - Action 0x1e */
void battle_render_action_1e(void) {
    LOG_DEBUG("Render revive");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Berserk - FUN_004366b0 - Action 0x1f */
void battle_render_action_1f(void) {
    LOG_DEBUG("Render berserk");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Transform - FUN_00435b40 - Action 0x20 */
void battle_render_action_20(void) {
    LOG_DEBUG("Render transform");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Status effect - FUN_00435f70 - Action 0x21 */
void battle_render_action_21(void) {
    LOG_DEBUG("Render status effect");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Chain attack - FUN_00436220 - Action 0x22 */
void battle_render_action_22(void) {
    LOG_DEBUG("Render chain attack");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Action 23 - FUN_00437200 */
void battle_render_action_23(void) {
    LOG_DEBUG("Render action 23");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Target select A - FUN_00438080 - Actions 0x24, 0x27 */
void battle_render_action_2427(void) {
    LOG_DEBUG("Render target select A");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Target select B - FUN_00438880 - Actions 0x25, 0x26 */
void battle_render_action_2526(void) {
    LOG_DEBUG("Render target select B");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Action 28 - FUN_00427cc0 */
void battle_render_action_28(void) {
    LOG_DEBUG("Render action 28");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Defend stance - FUN_0042bb40 - Action 0x29 */
void battle_render_action_29(void) {
    LOG_DEBUG("Render defend stance");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Escape - FUN_0042b150 - Action 0x2a */
void battle_render_action_2a(void) {
    LOG_DEBUG("Render escape");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Pet escape - FUN_00439a00 - Action 0x67 */
void battle_render_action_67(void) {
    LOG_DEBUG("Render pet escape");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Pet swap - FUN_0043a100 - Action 0x68 */
void battle_render_action_68(void) {
    LOG_DEBUG("Render pet swap");
    FUN_00488190(0xca, 0x140, 0xf0);
}

/* Pet action - FUN_0043a420 - Action 0x69 */
void battle_render_action_69(void) {
    LOG_DEBUG("Render pet action 69");
    FUN_00488190(0xca, 0x140, 0xf0);
}
