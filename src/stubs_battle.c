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
void FUN_0040a1a0(void) {}
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
void FUN_0040daf0(void) {}
void FUN_00419ac0(void) {}
void FUN_00419a40(void) {}
int FUN_00404850(int param_1) { (void)param_1; return 0; }
