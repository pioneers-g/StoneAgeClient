/*
 * Stone Age Client - Battle Transition Header
 * Reverse engineered from sa_9061.exe
 *
 * Functions for battle state transitions and initialization.
 */

#ifndef BATTLE_TRANSITION_H
#define BATTLE_TRANSITION_H

#include "types.h"

/*
 * Initialize battle transition state
 * FUN_0047a3f0
 */
void battle_transition_init(void);

/*
 * Clear battle UI state
 * FUN_0044aff0
 */
void battle_clear_ui_state(void);

/*
 * Clear battle UI windows - part 1
 * FUN_0044ac50
 */
void battle_clear_windows_1(void);

/*
 * Clear battle UI windows - part 2
 * FUN_0044adc0
 */
void battle_clear_windows_2(void);

/*
 * Build entity render queue
 * FUN_0047e440
 */
void battle_build_entity_queue(void);

/*
 * Display battle transition message
 * FUN_0041b870
 */
void battle_transition_message(void);

/*
 * Render queue count accessor
 */
u32 battle_get_render_queue_count(void);

#endif /* BATTLE_TRANSITION_H */
