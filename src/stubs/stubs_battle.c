/*
 * Stone Age Client - Battle Stub Functions
 * Split from stubs.c to reduce file size
 */

#include <windows.h>
#include <stdlib.h>
#include "types.h"

/* External globals */
extern void* s_battle_units;
extern u32 s_battle_unit_count;

/* Battle unit management stubs */
void* battle_get_unit(int index) {
    (void)index;
    return NULL;
}

void* battle_get_unit_by_index(int index) {
    (void)index;
    return NULL;
}

void* battle_get_player_unit(void) {
    return NULL;
}

int battle_add_unit(void* unit) {
    (void)unit;
    return 0;
}

/* Battle render stubs */
void battle_render_units(void) {}
void battle_render_ui(void) {}
void battle_render_effects(void) {}
void battle_render_field(void) {}

/* Action effect stubs */
void action_show_effect(int effect_type, int x, int y) {
    (void)effect_type; (void)x; (void)y;
}

void action_play_sound(int sound_id) {
    (void)sound_id;
}

/* Battle display functions */
void battle_update_unit_display(int unit_index) { (void)unit_index; }
void battle_update_unit_displays(void) {}

/* Battle utility functions */
int battle_find_rideable_pet(void) { return -1; }
int battle_check_end_condition(void) { return 0; }

/* Sound stub */
void sound_stop_bgm(void) {}

/* Battle state stubs */

/*
 * FUN_0040a1a0 - Battle State Machine
 *
 * Binary analysis:
 * - Main battle state machine with 9 states
 * - State dispatch via DAT_04630df0
 *
 * States (DAT_04630df0):
 * - 0: Initialize - Load battle map, init units, play BGM
 * - 1: Fade in transition
 * - 2: Battle start - show intro text, check for events
 * - 3: Action execution phase
 * - 4: Process actions (normal or special mode)
 * - 5: Turn end processing
 * - 6: Turn transition
 * - 7: Battle end fade out
 * - 8: Exit transition, return to field
 *
 * BGM selection based on map ID (DAT_04581190):
 * - Special maps (0x331, 0x1f47, 0x1fa5, etc.): BGM 0x18
 * - Normal maps: BGM based on encounter type
 *
 * Key functions called:
 * - FUN_00404850: Load battle field
 * - FUN_00401170: Update entities
 * - FUN_0047dc60: Render
 * - FUN_0047bde0: Fade effects
 * - FUN_00405160: Action processing
 */
void FUN_0040a1a0(void) {}

/*
 * FUN_00424b70 - Battle Action Dispatcher
 *
 * Binary analysis:
 * - Dispatches battle actions based on action type
 * - param_1: action type (0-0x68)
 * - param_2: action parameter 1
 * - param_3: action parameter 2
 * - param_4: action parameter 3
 * - param_5: action data string
 *
 * Action types:
 * - 0: Normal attack
 * - 1: Guard
 * - 2: Escape
 * - 6: Use item
 * - 7: Use skill
 * - 8: Pet skill
 * - 9: Capture pet
 * - 10-11: Team actions
 * - 12: Summon pet
 * - 13: Change formation
 * - 14: Pet command
 * - 15: Pet AI toggle
 * - 18: Get gold reward
 * - 21: PVP action
 * - 24-27: Combined actions
 * - 28: Counter attack
 * - 29: Automatic action
 * - 30: Request action
 * - 33-45: Various special actions
 * - 101: Battle text
 * - 103-104: System actions
 * - 520: Pet summon (0x208)
 *
 * Sets globals:
 * - DAT_004b83ec: Current action type
 * - DAT_0455ef94: Action parameter
 * - DAT_04558c34: Action target
 * - DAT_0455b5ac: Action data
 */
void FUN_00424b70(int param_1, int param_2, int param_3, int param_4, char* param_5) {
    (void)param_1; (void)param_2; (void)param_3; (void)param_4; (void)param_5;
}
void FUN_00424f50(void) {}
void FUN_00426cc0(void) {}
void FUN_00427190(void) {}
void FUN_004276a0(void) {}
void FUN_00426380(int mode) { (void)mode; }
void FUN_00426850(void) {}
void FUN_0042ce40(void) {}
void FUN_0042acf0(void) {}
void FUN_0042e8f0(void) {}
void FUN_00430b50(void) {}
void FUN_00431560(void) {}
void FUN_004338d0(void) {}
void FUN_00434d60(void) {}
void FUN_004354f0(void) {}
void FUN_00432a10(int param_1) { (void)param_1; }
void FUN_00438080(void) {}
void FUN_00438880(void) {}
void FUN_00439310(char* param_1, int param_2) { (void)param_1; (void)param_2; }
void FUN_00438f70(int param_1, void* param_2) { (void)param_1; (void)param_2; }
void FUN_0044aff0(void) {}
void FUN_0044ac50(void) {}
void FUN_0044adc0(void) {}
/*
 * FUN_00405080 - Battle Unit Array Reset
 *
 * Binary analysis:
 * - Initializes battle unit arrays to default state (-2 = empty)
 * - Clears DAT_004e10e8 (10 entries) - player party units
 * - Clears DAT_004e214c (6 entries) - enemy units
 * - Clears DAT_004d9054 (6 entries) - reserve units
 * - Clears DAT_004e211c (7 entries) - pet units
 * - Clears DAT_004d7f80 (9 entries) - formation data
 * - Sets various battle state flags:
 *   - DAT_004e21e4: 0 (no action pending)
 *   - DAT_004e21e8: 1 (battle active)
 *   - DAT_004a1308: -1 (no selection)
 *   - DAT_0049e090: 1 (normal mode)
 *   - DAT_004e21f0: 1 if DAT_004a12fc == -2 (PVP mode check)
 * - Calls FUN_00405300 to initialize additional state
 */
void FUN_00405080(void) {}

/*
 * FUN_004051c0 - Clear Unit Action Flags
 *
 * Binary analysis:
 * - Clears action flag (bit 6, 0x40) from all units
 * - Iterates through unit array at DAT_04ebe428
 * - Modifies offset 0xa0: flags &= ~0x40
 * - Used after action processing completes
 */
void FUN_004051c0(void) {}

/*
 * FUN_00405120 - Check Multiple Active Units
 *
 * Binary analysis:
 * - Counts units with active pets/companions
 * - Iterates through unit array at DAT_04ebe428 (max 20 entries)
 * - Checks: unit->offset8 != 0 AND unit->companion->offset0x120 == 1
 * - Returns:
 *   - 1 if exactly 2 units have active companions
 *   - 2 if more than 2 units have active companions
 *   - Used to determine combined attack availability
 */
int FUN_00405120(void) { return 1; }

/*
 * FUN_00405160 - Mark Units for Action
 *
 * Binary analysis:
 * - Sets action flag (0x40) on units
 * - If DAT_004e1110 != DAT_004d903c: mark all units
 * - Otherwise: mark only units where bit is set in DAT_004dd06c bitmask
 * - Iterates through DAT_04ebe428 array (up to 0x4ebe478)
 * - Flag 0x40 at offset 0xa0 indicates unit needs action processing
 */
void FUN_00405160(void) {}

/*
 * FUN_00405370 - Battle Grid Position Setup
 *
 * Binary analysis:
 * - Sets up battle grid positions for units
 * - param_1: base X coordinate
 * - param_2: base Y coordinate
 * - Creates grid entries at DAT_04582f30 array
 * - Each entry has 8 fields: x, y, prev_x, prev_y, sprite_id, surface_type, ...
 * - Grid spacing: 0x30 (48) X, 0x33 (51) per cell
 * - Y offset increases by 0x30 (48) every 5 cells (0xef threshold)
 * - Sprite IDs: 20000, 20001, 20002... (offset 20000 base)
 */
void FUN_00405370(int base_x, int base_y) { (void)base_x; (void)base_y; }

void FUN_0040daf0(void) {}
void FUN_00419ac0(void) {}
void FUN_00419a40(void) {}
int FUN_00404850(int param_1) { (void)param_1; return 0; }
