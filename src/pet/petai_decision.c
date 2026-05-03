/*
 * Stone Age Client - Pet AI Decision Engine
 * Split from petai.c for code organization
 *
 * Handles AI decision making, target selection, and skill evaluation
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "petai.h"
#include "random.h"
#include "logger.h"

/*
 * Decide pet action in battle
 */
AIDecision petai_decide_action(u8 pet_slot, BattleContext* battle, PetData* pet) {
    AIDecision decision = {0};
    AIPreset* preset;
    int hp_percent;
    int mp_percent;

    if (!battle || !pet) {
        decision.action = AI_ACTION_NONE;
        return decision;
    }

    preset = petai_get_preset(pet_slot);
    if (!preset) {
        preset = &g_petai.presets[AI_PRESET_BALANCED];
    }

    hp_percent = (pet->hp * 100) / pet->max_hp;
    mp_percent = (pet->mp * 100) / pet->max_mp;

    /* Check if pet needs to heal */
    if (hp_percent < preset->heal_threshold && petai_can_heal(pet)) {
        decision.action = AI_ACTION_SKILL;
        decision.skill_id = petai_find_heal_skill(pet);
        decision.target_type = AI_TARGET_SELF;
        return decision;
    }

    /* Check if pet should defend */
    if (hp_percent < preset->defend_threshold && random_chance(50)) {
        decision.action = AI_ACTION_DEFEND;
        return decision;
    }

    /* Check if pet should use skill */
    if (preset->skill_priority > 0 && mp_percent > g_petai.mp_threshold) {
        if (random_chance(preset->skill_priority)) {
            decision.action = AI_ACTION_SKILL;
            decision.skill_id = petai_select_skill(pet, battle, preset);
            decision.target_type = AI_TARGET_ENEMY;
            decision.target_index = petai_select_target(battle, preset);
            return decision;
        }
    }

    /* Default to attack */
    decision.action = AI_ACTION_ATTACK;
    decision.target_type = AI_TARGET_ENEMY;
    decision.target_index = petai_select_target(battle, preset);

    return decision;
}

/*
 * Select best skill to use
 */
u16 petai_select_skill(PetData* pet, BattleContext* battle, AIPreset* preset) {
    int i;
    u16 best_skill = 0;
    int best_priority = -1;

    if (!pet) return 0;

    for (i = 0; i < pet->skill_count; i++) {
        PetSkill* skill = &pet->skills[i];
        int priority;

        if (skill->mp_cost > pet->mp) continue;
        if (skill->cooldown > 0) continue;

        priority = petai_evaluate_skill(skill, battle, preset);
        if (priority > best_priority) {
            best_priority = priority;
            best_skill = skill->id;
        }
    }

    return best_skill;
}

/*
 * Evaluate skill priority
 */
int petai_evaluate_skill(PetSkill* skill, BattleContext* battle, AIPreset* preset) {
    int priority = 50;

    if (!skill) return 0;

    switch (skill->type) {
        case SKILL_TYPE_ATTACK:
            priority += preset->attack_priority / 4;
            break;
        case SKILL_TYPE_HEAL:
            priority += (100 - preset->attack_priority) / 3;
            break;
        case SKILL_TYPE_BUFF:
            priority += preset->buff_priority / 4;
            break;
        case SKILL_TYPE_DEBUFF:
            priority += preset->skill_priority / 4;
            break;
    }

    priority += skill->power / 10;
    priority += random_range(20) - 10;

    return priority;
}

/*
 * Select target enemy
 */
u8 petai_select_target(BattleContext* battle, AIPreset* preset) {
    int i;
    u8 target = 0;
    int lowest_hp = 999999;
    int highest_threat = 0;

    if (!battle) return 0;

    switch (preset->mode) {
        case AI_MODE_AGGRESSIVE:
            for (i = 0; i < battle->enemy_count; i++) {
                if (battle->enemies[i].hp > 0 && battle->enemies[i].hp < lowest_hp) {
                    lowest_hp = battle->enemies[i].hp;
                    target = i;
                }
            }
            break;

        case AI_MODE_DEFENSIVE:
        case AI_MODE_BALANCED:
            for (i = 0; i < battle->enemy_count; i++) {
                if (battle->enemies[i].hp > 0) {
                    int threat = petai_calculate_threat(&battle->enemies[i]);
                    if (threat > highest_threat) {
                        highest_threat = threat;
                        target = i;
                    }
                }
            }
            break;

        default:
            do {
                target = random_range(battle->enemy_count);
            } while (battle->enemies[target].hp <= 0);
            break;
    }

    return target;
}

/*
 * Calculate threat level of enemy
 */
int petai_calculate_threat(BattleUnit* enemy) {
    int threat = 0;

    if (!enemy) return 0;

    threat += enemy->attack * 2;
    threat += enemy->magic_attack;
    threat += enemy->level * 5;

    if (enemy->hp < enemy->max_hp / 4) {
        threat += 50;
    }

    return threat;
}

/*
 * Team-based AI decision
 */
AIDecision petai_decide_team_action(u8 pet_slot, BattleContext* battle,
                                     PetData* pet, int party_size) {
    AIDecision decision = {0};
    AIPreset* preset;
    int lowest_ally_hp;
    int i;

    if (!battle || !pet) {
        decision.action = AI_ACTION_NONE;
        return decision;
    }

    preset = petai_get_preset(pet_slot);
    if (!preset) {
        preset = &g_petai.presets[AI_PRESET_BALANCED];
    }

    /* Find ally with lowest HP */
    lowest_ally_hp = 100;
    for (i = 0; i < battle->ally_count; i++) {
        if (battle->allies[i].hp > 0) {
            int ally_hp = (battle->allies[i].hp * 100) / battle->allies[i].max_hp;
            if (ally_hp < lowest_ally_hp) {
                lowest_ally_hp = ally_hp;
            }
        }
    }

    /* Check if we should heal ally */
    if (preset->mode == AI_MODE_SUPPORT && lowest_ally_hp < 40) {
        u16 heal_skill = petai_find_heal_skill(pet);
        if (heal_skill != 0) {
            decision.action = AI_ACTION_SKILL;
            decision.skill_id = heal_skill;
            decision.target_type = AI_TARGET_ALLY;
            decision.target_index = petai_find_lowest_hp_ally(battle);
            return decision;
        }
    }

    /* Check if we should buff team */
    if (preset->mode == AI_MODE_SUPPORT && party_size >= 2) {
        u16 buff_skill = petai_find_buff_skill(pet);
        if (buff_skill != 0 && random_chance(40)) {
            decision.action = AI_ACTION_SKILL;
            decision.skill_id = buff_skill;
            decision.target_type = AI_TARGET_ALLY;
            decision.target_index = 0;
            return decision;
        }
    }

    return petai_decide_action(pet_slot, battle, pet);
}

/*
 * Select skill for combo attack
 */
u16 petai_select_combo_skill(PetData* pet, BattleContext* battle,
                              AIPreset* preset, u8* combo_count) {
    u16 best_skill = 0;
    int best_priority = -1;
    int i;
    PetSkill* skill;

    if (!pet || !battle || !combo_count) {
        *combo_count = 0;
        return 0;
    }

    *combo_count = 0;

    for (i = 0; i < MAX_PET_SKILLS_PER_PET; i++) {
        skill = &pet->skills[i];

        if (skill->id == 0) continue;
        if (skill->mp_cost > pet->mp) continue;
        if (skill->cooldown > 0) continue;

        if (skill->type == SKILL_TYPE_ATTACK) {
            int priority = petai_evaluate_skill(skill, battle, preset);

            if (battle->last_action_type == AI_ACTION_SKILL) {
                priority += 10;
                *combo_count = 1;
            }

            if (priority > best_priority) {
                best_priority = priority;
                best_skill = skill->id;
            }
        }
    }

    return best_skill;
}

/*
 * Check if pet should use defensive action
 */
int petai_should_defend(PetData* pet, BattleContext* battle, AIPreset* preset) {
    int hp_percent;
    int enemy_count;
    int strong_enemy;
    int i;

    if (!pet || !battle) return 0;

    hp_percent = (pet->hp * 100) / pet->max_hp;
    enemy_count = battle->enemy_count;

    strong_enemy = 0;
    for (i = 0; i < enemy_count; i++) {
        if (battle->enemies[i].level > pet->level + 5) {
            strong_enemy = 1;
            break;
        }
    }

    if (hp_percent < preset->defend_threshold && strong_enemy) {
        return 1;
    }

    if (enemy_count >= 3 && hp_percent < 40) {
        return 1;
    }

    return 0;
}

/*
 * Check if pet should flee
 */
int petai_should_flee(PetData* pet, BattleContext* battle) {
    int hp_percent;
    int allies_alive;
    int i;

    if (!pet || !battle) return 0;

    hp_percent = (pet->hp * 100) / pet->max_hp;

    allies_alive = 0;
    for (i = 0; i < battle->ally_count; i++) {
        if (battle->allies[i].hp > 0) {
            allies_alive++;
        }
    }

    if (hp_percent < 10 && allies_alive == 0) {
        return 1;
    }

    if (hp_percent < 20 && battle->enemy_count >= 4 && allies_alive <= 1) {
        return 1;
    }

    return 0;
}

/*
 * Find ally with lowest HP percentage
 */
u8 petai_find_lowest_hp_ally(BattleContext* battle) {
    int i;
    u8 target = 0;
    int lowest_hp_percent = 100;

    if (!battle) return 0;

    for (i = 0; i < battle->ally_count; i++) {
        if (battle->allies[i].hp > 0) {
            int hp_percent = (battle->allies[i].hp * 100) / battle->allies[i].max_hp;
            if (hp_percent < lowest_hp_percent) {
                lowest_hp_percent = hp_percent;
                target = i;
            }
        }
    }

    return target;
}
