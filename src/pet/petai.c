/*
 * Stone Age Client - Pet AI Core
 * Main AI implementation with element system and damage estimation
 *
 * Split into modules:
 * - petai_settings.c: Settings loading and preset management
 * - petai_decision.c: AI decision engine
 * - petai.c: Core functions, element system, utility
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "petai.h"
#include "random.h"
#include "logger.h"

/* Global AI context - matches DAT_004d9050 region */
PetAIContext g_petai = {0};

/* Element advantage table - from game data analysis
 * Fire > Wind > Earth > Water > Fire
 * Light <-> Dark (mutual 1.5x)
 */
static const float s_element_table[8][8] = {
    /* None, Fire, Water, Earth, Wind, Light, Dark, Special */
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},  /* None */
    {1.0f, 0.5f, 0.5f, 1.5f, 1.5f, 1.0f, 1.0f, 1.0f},  /* Fire */
    {1.0f, 1.5f, 0.5f, 0.5f, 1.5f, 1.0f, 1.0f, 1.0f},  /* Water */
    {1.0f, 0.5f, 1.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f},  /* Earth */
    {1.0f, 0.5f, 0.5f, 1.5f, 0.5f, 1.0f, 1.0f, 1.0f},  /* Wind */
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.5f, 1.0f},  /* Light */
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.5f, 0.5f, 1.0f},  /* Dark */
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f}   /* Special */
};

/*
 * Get element advantage multiplier
 */
float petai_get_element_advantage(u8 attacker_element, u8 defender_element) {
    if (attacker_element > 7 || defender_element > 7) {
        return 1.0f;
    }
    return s_element_table[attacker_element][defender_element];
}

/*
 * Advanced skill evaluation with element and status considerations
 */
int petai_evaluate_skill_advanced(PetSkill* skill, BattleContext* battle,
                                   AIPreset* preset, BattleUnit* target) {
    int priority;
    float element_mult;
    int hp_factor;
    int mp_factor;
    int status_factor;

    if (!skill) return 0;

    priority = petai_evaluate_skill(skill, battle, preset);

    /* Element advantage */
    if (target && skill->element > 0) {
        element_mult = petai_get_element_advantage(skill->element, target->element);
        if (element_mult > 1.0f) {
            priority += 20 * (int)((element_mult - 1.0f) * 10);
        } else if (element_mult < 1.0f) {
            priority -= 15 * (int)((1.0f - element_mult) * 10);
        }
    }

    /* HP factor for healing */
    hp_factor = 0;
    if (skill->type == SKILL_TYPE_HEAL) {
        if (battle && battle->pet_hp_percent < 30) {
            hp_factor = 30;
        } else if (battle && battle->pet_hp_percent < 50) {
            hp_factor = 15;
        }
    }
    priority += hp_factor;

    /* MP cost factor */
    mp_factor = 0;
    if (battle && battle->pet_mp_percent < 30) {
        mp_factor = -(skill->mp_cost / 2);
    }
    priority += mp_factor;

    /* Status effect factor */
    status_factor = 0;
    if (skill->type == SKILL_TYPE_DEBUFF) {
        if (target && (target->flags & 0x0F)) {
            status_factor = 15;
        }
    }
    if (skill->type == SKILL_TYPE_BUFF) {
        if (battle && !(battle->pet_flags & 0x0F)) {
            status_factor = 10;
        }
    }
    priority += status_factor;

    return priority;
}

/*
 * Check if pet can heal
 */
int petai_can_heal(PetData* pet) {
    int i;

    if (!pet) return 0;

    for (i = 0; i < pet->skill_count; i++) {
        if (pet->skills[i].type == SKILL_TYPE_HEAL &&
            pet->skills[i].mp_cost <= pet->mp) {
            return 1;
        }
    }

    return 0;
}

/*
 * Find heal skill
 */
u16 petai_find_heal_skill(PetData* pet) {
    int i;

    if (!pet) return 0;

    for (i = 0; i < pet->skill_count; i++) {
        if (pet->skills[i].type == SKILL_TYPE_HEAL &&
            pet->skills[i].mp_cost <= pet->mp) {
            return pet->skills[i].id;
        }
    }

    return 0;
}

/*
 * Find buff skill
 */
u16 petai_find_buff_skill(PetData* pet) {
    int i;

    if (!pet) return 0;

    for (i = 0; i < MAX_PET_SKILLS_PER_PET; i++) {
        if (pet->skills[i].type == SKILL_TYPE_BUFF &&
            pet->skills[i].mp_cost <= pet->mp) {
            return pet->skills[i].id;
        }
    }

    return 0;
}

/*
 * Find debuff skill
 */
u16 petai_find_debuff_skill(PetData* pet) {
    int i;

    if (!pet) return 0;

    for (i = 0; i < MAX_PET_SKILLS_PER_PET; i++) {
        if (pet->skills[i].type == SKILL_TYPE_DEBUFF &&
            pet->skills[i].mp_cost <= pet->mp) {
            return pet->skills[i].id;
        }
    }

    return 0;
}

/*
 * Estimate damage from skill
 */
int petai_estimate_damage(PetData* pet, PetSkill* skill, BattleUnit* target) {
    int base_damage;
    float element_mult;
    int final_damage;

    if (!pet || !skill || !target) return 0;

    base_damage = skill->power + pet->attack;
    base_damage -= target->defense / 2;

    element_mult = petai_get_element_advantage(skill->element, target->element);

    final_damage = (int)(base_damage * element_mult);
    final_damage += random_range(final_damage / 4) - (final_damage / 8);

    if (final_damage < 1) final_damage = 1;

    return final_damage;
}

/*
 * Check if skill can kill target
 */
int petai_can_kill_target(PetData* pet, PetSkill* skill, BattleUnit* target) {
    int estimated_damage;

    if (!pet || !skill || !target) return 0;

    estimated_damage = petai_estimate_damage(pet, skill, target);

    return estimated_damage >= target->hp;
}

/*
 * Select skill that can kill target (finisher)
 */
u16 petai_select_finisher_skill(PetData* pet, BattleContext* battle,
                                  BattleUnit* target) {
    int i;
    PetSkill* skill;
    u16 best_skill = 0;
    int lowest_cost = 9999;

    if (!pet || !battle || !target) return 0;

    for (i = 0; i < MAX_PET_SKILLS_PER_PET; i++) {
        skill = &pet->skills[i];

        if (skill->id == 0) continue;
        if (skill->mp_cost > pet->mp) continue;
        if (skill->type != SKILL_TYPE_ATTACK) continue;

        if (petai_can_kill_target(pet, skill, target)) {
            if (skill->mp_cost < lowest_cost) {
                lowest_cost = skill->mp_cost;
                best_skill = skill->id;
            }
        }
    }

    return best_skill;
}

/*
 * AI mode to string
 */
const char* petai_mode_to_string(AIMode mode) {
    static const char* mode_names[] = {
        "Aggressive", "Defensive", "Balanced", "Support", "Manual"
    };

    if (mode < sizeof(mode_names) / sizeof(mode_names[0])) {
        return mode_names[mode];
    }
    return "Unknown";
}

/*
 * AI action to string
 */
const char* petai_action_to_string(AIAction action) {
    static const char* action_names[] = {
        "None", "Attack", "Skill", "Defend", "Item", "Flee", "Wait"
    };

    if (action < sizeof(action_names) / sizeof(action_names[0])) {
        return action_names[action];
    }
    return "Unknown";
}
