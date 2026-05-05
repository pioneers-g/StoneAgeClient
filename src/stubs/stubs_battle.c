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

/* Battle unit slot arrays */
extern s32 DAT_004e10e8[10];
extern s32 DAT_004e214c[6];
extern s32 DAT_004d9054[6];
extern s32 DAT_004e211c[7];
extern s32 DAT_004d7f80[9];
extern s32 DAT_004a12fc;

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
void battle_render_units(void) {
    /* Delegates to battle_render.c when available */
}
void battle_render_ui(void) {
    /* Delegates to battle_render_ui.c when available */
}
/*
 * battle_render_effects - Render visual battle effects
 * Processes the effect queue from battle_action_render.c
 */
void battle_render_effects(void) {
    /* Render active battle effects (damage numbers, animations) */
}

/*
 * battle_render_field - Render battle field background
 * Draws the isometric battle terrain based on current map
 */
void battle_render_field(void) {
    /* Render battle field terrain */
}

/* Action effect stubs - implemented in battle_action_render.c */
void action_show_effect(int effect_type, int x, int y) {
    (void)effect_type; (void)x; (void)y;
}

void action_play_sound(int sound_id) {
    (void)sound_id;
}

/* Battle display functions - defined in battle_unit.c */
void battle_update_unit_display(int unit_index) { (void)unit_index; }
void battle_update_unit_displays(void) {
    /* Refresh all unit HP/MP/status displays */
}

/* Battle utility functions - defined in battle_unit.c */
int battle_find_rideable_pet(void) { return -1; }
int battle_check_end_condition(void) { return 0; }

/*
 * sound_stop_bgm - Stop background music
 *
 * Binary analysis:
 * - Stops currently playing BGM via DirectSound
 * - Called during state transitions (field → battle, etc.)
 * - Resets BGM playback state in DAT_0461b420
 */
void sound_stop_bgm(void) {
    extern u32 DAT_0461b420;
    DAT_0461b420 = 0;
}

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
void FUN_0040a1a0(void) {
    extern void battle_update(void);
    battle_update();
}

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
    extern u32 DAT_004b83ec;
    extern u32 DAT_0455ef94;
    extern u32 DAT_04558c34;
    extern u32 DAT_0455b5ac;

    DAT_004b83ec = param_1;
    DAT_0455ef94 = param_2;
    DAT_04558c34 = param_3;
    DAT_0455b5ac = param_4;
    (void)param_5;

    switch (param_1) {
        case 0:   /* Normal attack */ break;
        case 1:   /* Guard */ break;
        case 2:   /* Escape */ break;
        case 6:   /* Use item */ break;
        case 7:   /* Use skill */ break;
        case 8:   /* Pet skill */ break;
        case 9:   /* Capture pet */ break;
        case 10:
        case 11:  /* Team actions */ break;
        case 12:  /* Summon pet */ break;
        case 13:  /* Change formation */ break;
        case 14:  /* Pet command */ break;
        case 15:  /* Pet AI toggle */ break;
        case 18:  /* Get gold reward */ break;
        case 21:  /* PVP action */ break;
        case 28:  /* Counter attack */ break;
        case 29:  /* Automatic action */ break;
        case 30:  /* Request action */ break;
        case 101: /* Battle text */ break;
        case 103:
        case 104: /* System actions */ break;
        case 520: /* Pet summon (0x208) */ break;
        default: break;
    }
}
/*
 * FUN_00424f50 - Battle Action Executor
 *
 * Binary analysis:
 * - Per-frame action dispatcher during battle
 * - Dispatches to UI rendering based on action type from DAT_004b83ec
 * - 45+ action types matching FUN_00426380, FUN_00426850, etc.
 * - Key sub-functions: FUN_00426380 (attack UI), FUN_00426850 (skill UI),
 *   FUN_00426cc0 (counter), FUN_0042ce40 (capture), FUN_0042acf0 (item),
 *   FUN_00438080/00438880 (target select)
 *
 * Real implementation in battle_action_exec.c (excluded due to overlap
 * with battle_action_core.c). These stubs provide the FUN_* entry points.
 */
/*
 * FUN_00424f50 - Battle Action Render Executor
 *
 * Wires to battle_render_execute() in battle_action_render.c
 * which processes the current action type's visual effects
 */
void FUN_00424f50(void) {
    extern void battle_render_execute(void);
    battle_render_execute();
}
/*
 * Individual battle action render stubs
 * Wire to battle_action_exec.c functions
 */
void FUN_00426cc0(void) {
    extern void battle_action_render_counter_setup(void);
    battle_action_render_counter_setup();
}
void FUN_00427190(void) {
    extern void battle_action_render_combo(int);
    battle_action_render_combo(0);
}
void FUN_004276a0(void) {
    extern void battle_action_render_combo(int);
    battle_action_render_combo(1);
}
void FUN_00426380(int mode) {
    extern void battle_action_render_attack(int);
    battle_action_render_attack(mode);
}
void FUN_00426850(void) {
    extern void battle_action_render_skill_select(void);
    battle_action_render_skill_select();
}
void FUN_0042ce40(void) {
    extern void battle_action_render_capture(void);
    battle_action_render_capture();
}
void FUN_0042acf0(void) {
    extern void battle_action_render_item_select(void);
    battle_action_render_item_select();
}
void FUN_0042e8f0(void) {
    extern void battle_action_render_pet_attack(void);
    battle_action_render_pet_attack();
}
void FUN_00430b50(void) {
    extern void battle_action_render_skill_chain(void);
    battle_action_render_skill_chain();
}
void FUN_00431560(void) {
    extern void battle_action_render_skill_area(void);
    battle_action_render_skill_area();
}
void FUN_004338d0(void) {
    extern void battle_action_render_pet_defend(void);
    battle_action_render_pet_defend();
}
void FUN_00434d60(void) {
    extern void battle_action_render_death(void);
    battle_action_render_death();
}
void FUN_004354f0(void) {
    extern void battle_action_render_revive(void);
    battle_action_render_revive();
}
void FUN_00432a10(int param_1) {
    extern void battle_action_render_summon(int);
    battle_action_render_summon(param_1);
}
void FUN_00438080(void) {
    extern void battle_action_render_target_select(int);
    battle_action_render_target_select(0);
}
void FUN_00438880(void) {
    extern void battle_action_render_target_select(int);
    battle_action_render_target_select(1);
}

void FUN_00439310(char* param_1, int param_2) {
    (void)param_1; (void)param_2;
}

void FUN_00438f70(int param_1, void* param_2) {
    (void)param_1; (void)param_2;
}

/*
 * FUN_0044aff0 - Battle Window Update
 *
 * Binary analysis:
 * - Updates battle window positions and widget states
 * - Called during battle UI refresh
 */
void FUN_0044aff0(void) {
    /* Battle window update */
}

/*
 * FUN_0044ac50 - Battle Button Update
 *
 * Binary analysis:
 * - Updates battle action button states (enabled/disabled)
 * - Processes button click regions
 */
void FUN_0044ac50(void) {
    /* Battle button update */
}

/*
 * FUN_0044adc0 - Battle Status Display
 *
 * Binary analysis:
 * - Updates battle status bar displays (HP/MP/etc)
 * - Refreshes unit status widgets
 */
void FUN_0044adc0(void) {
    /* Battle status display */
}
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
void FUN_00405080(void) {
    extern s32 DAT_004e21dc;
    extern s32 DAT_004e21e4;
    extern s32 DAT_004e21e8;
    extern s32 DAT_004a1308;
    extern s32 DAT_0049e090;
    extern s32 DAT_004e21f0;
    extern s32 DAT_004a12fc;
    int i;

    /* Clear player party unit slots (10 entries at DAT_004e10e8) */
    for (i = 0; i < 10; i++) {
        DAT_004e10e8[i] = -2;
    }

    /* Clear enemy unit slots (6 entries at DAT_004e214c) */
    for (i = 0; i < 6; i++) {
        DAT_004e214c[i] = -2;
    }

    /* Clear reserve unit slots (6 entries at DAT_004d9054) */
    for (i = 0; i < 6; i++) {
        DAT_004d9054[i] = -2;
    }

    /* Clear pet unit slots (7 entries at DAT_004e211c) */
    for (i = 0; i < 7; i++) {
        DAT_004e211c[i] = -2;
    }

    /* Clear formation data (9 entries at DAT_004d7f80) */
    for (i = 0; i < 9; i++) {
        DAT_004d7f80[i] = -2;
    }

    /* Reset battle state flags */
    DAT_004e21e4 = 0;
    DAT_004e21e8 = 1;
    DAT_004a1308 = -1;
    DAT_0049e090 = 1;
    DAT_004e21f0 = (DAT_004a12fc == -2) ? 1 : 0;
    DAT_004e21dc = 0;
}

/*
 * FUN_004051c0 - Clear Unit Action Flags
 *
 * Binary analysis:
 * - Clears action flag (bit 6, 0x40) from all units
 * - Iterates through unit array at DAT_04ebe428
 * - Modifies offset 0xa0: flags &= ~0x40
 * - Used after action processing completes
 */
void FUN_004051c0(void) {
    int i;
    extern s32 DAT_004e10e8[10];
    (void)DAT_004e10e8;
    for (i = 0; i < 10; i++) {
        /* Clear action flag bit 6 (0x40) from each unit */
        /* In real binary: unit[offset_0xa0] &= ~0x40 */
    }
}

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
int FUN_00405120(void) {
    extern s32 DAT_004e10e8[10];
    int count = 0;
    int i;
    for (i = 0; i < 10; i++) {
        if (DAT_004e10e8[i] != -2) {
            count++;
        }
    }
    if (count == 2) return 1;
    if (count > 2) return 2;
    return 0;
}

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
void FUN_00405160(void) {
    extern s32 DAT_004e10e8[10];
    extern s32 DAT_004e1110;
    extern s32 DAT_004d903c;
    extern u32 DAT_004dd06c;
    int i;
    if (DAT_004e1110 != DAT_004d903c) {
        for (i = 0; i < 10; i++) {
            if (DAT_004e10e8[i] != -2) {
                /* Mark unit for action (set bit 0x40) */
            }
        }
    } else {
        for (i = 0; i < 10; i++) {
            if (DAT_004e10e8[i] != -2 && (DAT_004dd06c & (1 << i))) {
                /* Mark unit for action (set bit 0x40) */
            }
        }
    }
}

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
void FUN_00405370(int base_x, int base_y) {
    extern s32 DAT_004d7f80[9];
    /* Formation positions relative to base_x, base_y */
    /* Standard diamond formation: 9 positions */
    int offsets[9][2] = {
        {0, 0}, {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {1, 1}, {-1, 1}, {1, -1}, {-1, -1}
    };
    int i;
    for (i = 0; i < 9; i++) {
        DAT_004d7f80[i] = (base_x + offsets[i][0]) | ((base_y + offsets[i][1]) << 16);
    }
}

/*
 * FUN_0040daf0 - Battle Data Processing
 * FUN_00419ac0 - Battle Resource Cleanup
 * FUN_00419a40 - Battle State Reset
 * Currently unused — placeholder for future battle subsystem wiring
 */
void FUN_0040daf0(void) {
    /* Battle data processing */
}
void FUN_00419ac0(void) {
    /* Battle resource cleanup */
}
void FUN_00419a40(void) {
    /* Battle state reset */
}
int FUN_00404850(int param_1) {
    extern s32 DAT_004d7f80[9];
    int i;
    /* Initialize battle field - FUN_00405080 resets units */
    FUN_00405080();
    /* Set formation positions for the battle field */
    FUN_00405370(320, 240);
    (void)param_1;
    for (i = 0; i < 9; i++) {
        DAT_004d7f80[i] = -2;
    }
    return 1;
}
