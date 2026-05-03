/*
 * Stone Age Client - Pet Battle Handlers Header
 * Split from pet.h for code organization
 */

#ifndef PET_BATTLE_H
#define PET_BATTLE_H

#include "types.h"

/*
 * Battle handlers - from Ghidra analysis
 * All functions parse pipe-delimited protocol data
 */

/* Summon pet to battle - FUN_004327b0 pattern */
void pet_summon_battle(const char* pet_data, int is_field_skill);

/* Recall pet from battle - FUN_00432ec0 pattern */
void pet_recall_battle(const char* pet_data);

/* Pet attack handler - FUN_0042e870 pattern */
void pet_attack_handler(const char* target_data);

/* Pet skill handler - FUN_0042f130 pattern */
void pet_skill_handler(const char* skill_data);

/* Pet escape handler - FUN_0042af40 pattern */
void pet_escape_handler(const char* escape_data);

/* Handle capture text result */
void pet_handle_capture_text(const char* pet_data);

#endif /* PET_BATTLE_H */
