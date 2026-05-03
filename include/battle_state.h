/*
 * Stone Age Client - Battle State Machine
 * Reverse engineered from sa_9061.exe FUN_0040a1a0
 *
 * Battle state management and transitions
 */

#ifndef BATTLE_STATE_H
#define BATTLE_STATE_H

#include "types.h"

/*
 * Battle state machine functions - FUN_0040a1a0
 * Each function handles one state of the battle system
 */

/* State 0 - Initialize battle systems */
void battle_state_init(u32* state, u32* action_result);

/* State 2 - Main battle loop (action selection) */
void battle_state_main(u32* state, u32* action_result);

/* State 3 - Action processing */
void battle_state_action(u32* state, u32* action_result);

/* State 4 - Execute action animations */
void battle_state_execute(u32* state);

/* State 5 - Result processing */
void battle_state_result(u32* state, u32* action_result);

/* State 6 - Message display */
void battle_state_message(u32* state, u32* action_result);

/* State 7 - Begin end sequence */
void battle_state_end_start(u32* state);

/* State 8 - Final cleanup and return to field */
void battle_state_end_complete(u32* state);

/*
 * Helper functions for state management
 */

/* Check if fade effect is complete */
int battle_fade_check(void);

/* Process render queue during state transitions */
void battle_render_queue_process(void);

/* Initialize battle UI components */
void battle_ui_init(void);

/* Initialize battle cursor */
void battle_cursor_init(void);

/* Render battle field */
void battle_field_render(void);

/* Check special battle conditions */
void battle_check_special_conditions(u32* state);

/* Draw dialog buttons for battle menu */
void battle_draw_dialog_buttons(u32 dialog_id);

/* Update battle menu */
void battle_menu_update(void);

/*
 * Internal state helper functions
 */

/* Initialize turn order for battle */
void battle_init_turn_order(void);

/* Reset all battle animations */
void battle_reset_animations(void);

/* Show battle message */
void battle_show_message(const char* msg, int type);

/* Show result animation */
void battle_show_result_animation(u32 p2, u32 p3, u32 p4, int result);

/* Show damage numbers */
void battle_show_damage(u32 attacker, u32 target, u16 damage, u16 flags, u32 skill_id);

/* Clear capture UI elements */
void battle_clear_capture_ui(void);

#endif /* BATTLE_STATE_H */
