/*
 * Stone Age Client - Skill Damage/Effect System
 * Split from skill.c for code organization
 *
 * Handles damage calculation, status effects, and buffs
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "skill.h"
#include "skill_damage.h"
#include "character.h"
#include "battle.h"
#include "pet.h"
#include "logger.h"

/* External skill context */
extern SkillContext g_skill;

/* External skill functions */
extern SkillData* skill_get_data(u32 skill_id);
extern LearnedSkill* skill_get_learned(u32 skill_id);

/* Element advantage table - from game data analysis */
static const float s_element_matrix[7][7] = {
    /* None, Fire, Water, Earth, Wind, Light, Dark */
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},  /* None */
    {1.0f, 0.5f, 0.5f, 1.5f, 1.5f, 1.0f, 1.0f},  /* Fire strong vs Earth, Wind */
    {1.0f, 1.5f, 0.5f, 0.5f, 1.5f, 1.0f, 1.0f},  /* Water strong vs Fire, Wind */
    {1.0f, 0.5f, 1.5f, 0.5f, 0.5f, 1.0f, 1.0f},  /* Earth strong vs Water */
    {1.0f, 0.5f, 0.5f, 1.5f, 0.5f, 1.0f, 1.0f},  /* Wind strong vs Earth */
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.5f},  /* Light strong vs Dark */
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.5f, 0.5f}   /* Dark strong vs Light */
};

/*
 * Get element multiplier
 */
float skill_get_element_multiplier(u8 attack_elem, u8 defend_elem) {
    if (attack_elem > 6 || defend_elem > 6) {
        return 1.0f;
    }
    return s_element_matrix[attack_elem][defend_elem];
}

/*
 * Roll critical hit
 */
int skill_roll_critical(u16 luck, u8 skill_level) {
    /* Base crit chance = luck / 10, max 30% */
    u16 base_chance = luck / 10;
    if (base_chance > 30) base_chance = 30;

    /* Skills with higher level have slightly better crit */
    base_chance += skill_level;

    return (rand() % 100) < base_chance;
}

/*
 * Roll hit chance
 */
int skill_roll_hit(u16 attacker_hit, u16 target_dodge) {
    /* Base hit chance = 90% */
    s16 hit_chance = 90;

    /* Adjust by hit vs dodge */
    hit_chance += (attacker_hit - target_dodge) / 10;

    /* Clamp between 5% and 99% */
    if (hit_chance < 5) hit_chance = 5;
    if (hit_chance > 99) hit_chance = 99;

    return (rand() % 100) < hit_chance;
}

/*
 * Calculate skill damage - integrates with character/pet stats
 */
u32 skill_calculate_damage(u32 skill_id, u32 attacker_id, u32 target_id) {
    SkillData* data = skill_get_data(skill_id);
    LearnedSkill* skill = skill_get_learned(skill_id);
    Character* player;
    Pet* pet;
    u32 base;
    u16 attack = 0;
    u8 attacker_elem = 0;
    u8 target_elem = 0;
    float elem_mult;

    if (!data || !skill) return 0;

    /* Get attacker stats from character system */
    player = character_get_player();
    if (player && player->id == attacker_id) {
        attack = player->stats.attack;
        attacker_elem = 0;  /* Player element from equipment/skills */
    }

    /* Get attacker stats from pet system */
    pet = pet_get_by_id(attacker_id);
    if (pet) {
        attack = pet->attack;
        attacker_elem = pet->element;
    }

    /* Base damage formula: skill_power * level + attack */
    base = data->base_power * skill->level + attack;

    /* Get target element for element multiplier */
    if (player && player->id == target_id) {
        target_elem = 0;
    }
    pet = pet_get_by_id(target_id);
    if (pet) {
        target_elem = pet->element;
    }

    /* Apply element multiplier */
    elem_mult = skill_get_element_multiplier(
        data->element > 0 ? data->element : attacker_elem,
        target_elem
    );
    base = (u32)(base * elem_mult);

    return base;
}

/*
 * Calculate damage with full parameters
 * Integrates with character and pet stats
 */
SkillDamageResult skill_calculate_damage_ex(SkillDamageParams* params) {
    SkillDamageResult result = {0};
    SkillData* skill;
    s32 base_damage;
    s32 defense;
    float element_mult;
    int crit_roll;
    int hit_roll;
    u16 attacker_hit = params->attacker_atk;  /* Default from params */
    u16 target_dodge = 0;
    u16 attacker_luck = 50;

    if (!params) return result;

    skill = skill_get_data(params->skill_id);
    if (!skill) return result;

    /* Get actual stats from character system if available */
    {
        Character* player = character_get_player();
        if (player && player->id == params->attacker_id) {
            attacker_hit = player->stats.hit_rate;
            attacker_luck = player->stats.luck;
        }
        if (player && player->id == params->target_id) {
            target_dodge = player->stats.dodge_rate;
        }
    }

    /* Get stats from pet system if target is a pet */
    {
        Pet* pet = pet_get_by_id(params->attacker_id);
        if (pet) {
            attacker_hit = pet->agility * 2;  /* Pet hit rate from agility */
            attacker_luck = 50;  /* Default pet luck */
        }
    }

    /* Calculate base damage */
    if (skill->type == SKILL_TYPE_ATTACK) {
        /* Physical skill - use ATK */
        base_damage = skill->base_power + params->attacker_atk;
        defense = params->target_def;
    } else {
        /* Magical skill - use MATK */
        base_damage = skill->base_power + params->attacker_matk;
        defense = params->target_mdef;
    }

    /* Level scaling */
    base_damage = base_damage * (100 + params->skill_level * 10) / 100;

    /* Element multiplier */
    element_mult = skill_get_element_multiplier(
        skill->element > 0 ? skill->element : params->attacker_element,
        params->target_element
    );

    result.element_multiplier = element_mult;
    base_damage = (s32)(base_damage * element_mult);

    /* Apply defense */
    base_damage = base_damage - defense / 2;
    if (base_damage < 1) base_damage = 1;

    result.base_damage = base_damage;

    /* Check for miss - use actual hit/dodge values */
    hit_roll = skill_roll_hit(attacker_hit, target_dodge);
    if (!hit_roll) {
        result.is_miss = 1;
        result.final_damage = 0;
        return result;
    }

    /* Check for critical - use actual luck value */
    crit_roll = skill_roll_critical(attacker_luck, params->skill_level);
    if (crit_roll) {
        result.is_critical = 1;
        base_damage = base_damage * 150 / 100;  /* 1.5x crit multiplier */
    }

    /* Add variance (+/- 10%) */
    {
        int variance = base_damage / 10;
        if (variance > 0) {
            base_damage += (rand() % (variance * 2 + 1)) - variance;
        }
    }

    result.final_damage = base_damage;
    return result;
}

/*
 * Calculate healing amount
 */
u32 skill_calculate_heal(u32 skill_id, u8 skill_level, u16 matk) {
    SkillData* skill = skill_get_data(skill_id);
    u32 heal_amount;

    if (!skill) return 0;

    /* Base heal = skill power + MATK */
    heal_amount = skill->base_power + matk / 2;

    /* Level scaling */
    heal_amount = heal_amount * (100 + skill_level * 5) / 100;

    return heal_amount;
}

/*
 * Apply skill effect - integrates with character/pet damage system
 */
void skill_apply_effect(SkillEffect* effect) {
    Character* player;
    Pet* pet;

    if (!effect) return;

    /* Apply damage */
    if (effect->damage != 0) {
        /* Check if target is player */
        player = character_get_player();
        if (player && player->id == effect->target_id) {
            /* Apply damage to player */
            s32 new_hp = (s32)player->stats.hp - effect->damage;
            if (new_hp < 0) new_hp = 0;
            player->stats.hp = (u16)new_hp;
            LOG_DEBUG("Applied damage %d to player, HP now %u", effect->damage, player->stats.hp);
        }

        /* Check if target is a pet */
        pet = pet_get_by_id(effect->target_id);
        if (pet) {
            s32 new_hp = (s32)pet->current_hp - effect->damage;
            if (new_hp < 0) new_hp = 0;
            pet->current_hp = (u32)new_hp;
            LOG_DEBUG("Applied damage %d to pet %u, HP now %u", effect->damage, pet->id, pet->current_hp);
        }

        /* Check battle system for unit */
        {
            BattleUnit* unit = battle_get_unit(effect->target_id);
            if (unit) {
                s32 new_hp = (s32)unit->hp - effect->damage;
                if (new_hp < 0) {
                    new_hp = 0;
                    unit->is_alive = 0;
                }
                unit->hp = (u16)new_hp;
                LOG_DEBUG("Applied damage %d to battle unit %u, HP now %u", effect->damage, unit->id, unit->hp);
            }
        }
    }

    /* Apply healing */
    if (effect->heal != 0) {
        /* Check if target is player */
        player = character_get_player();
        if (player && player->id == effect->target_id) {
            s32 new_hp = (s32)player->stats.hp + effect->heal;
            if (new_hp > player->stats.max_hp) new_hp = player->stats.max_hp;
            player->stats.hp = (u16)new_hp;
            LOG_DEBUG("Applied heal %d to player, HP now %u", effect->heal, player->stats.hp);
        }

        /* Check if target is a pet */
        pet = pet_get_by_id(effect->target_id);
        if (pet) {
            s32 new_hp = (s32)pet->current_hp + effect->heal;
            if (new_hp > (s32)pet->max_hp) new_hp = pet->max_hp;
            pet->current_hp = (u32)new_hp;
            LOG_DEBUG("Applied heal %d to pet %u, HP now %u", effect->heal, pet->id, pet->current_hp);
        }

        /* Check battle system for unit */
        {
            BattleUnit* unit = battle_get_unit(effect->target_id);
            if (unit) {
                s32 new_hp = (s32)unit->hp + effect->heal;
                if (new_hp > unit->max_hp) new_hp = unit->max_hp;
                unit->hp = (u16)new_hp;
                LOG_DEBUG("Applied heal %d to battle unit %u, HP now %u", effect->heal, unit->id, unit->hp);
            }
        }
    }

    /* Apply status effect */
    if (effect->status_effect != 0) {
        skill_apply_status(effect->target_id,
                          (StatusEffect)effect->status_effect,
                          effect->status_duration,
                          1);
    }
}

/*
 * Apply status effect to target - integrates with character/pet status system
 */
void skill_apply_status(u32 target_id, StatusEffect status, u8 duration, u8 power) {
    Character* player = character_get_player();
    Pet* pet;
    BattleUnit* unit;

    LOG_DEBUG("Apply status %d to target %u for %d turns", status, target_id, duration);

    /* Apply to player */
    if (player && player->id == target_id) {
        player->buff_flags |= (1 << (status - 1));  /* Status effects start at bit 0 */
        /* Store duration in first available buff slot */
        for (int i = 0; i < 8; i++) {
            if (player->buff_end_time[i] == 0) {
                player->buff_end_time[i] = GetTickCount() + duration * 1000;
                break;
            }
        }
    }

    /* Apply to pet */
    pet = pet_get_by_id(target_id);
    if (pet) {
        /* Pet status would be stored in pet context */
        LOG_DEBUG("Status %d applied to pet %u", status, target_id);
    }

    /* Apply to battle unit */
    unit = battle_get_unit(target_id);
    if (unit) {
        battle_apply_buff(unit, status - 1, duration);
    }
}

/*
 * Remove status effect - integrates with character/pet status system
 */
void skill_remove_status(u32 target_id, StatusEffect status) {
    Character* player = character_get_player();
    Pet* pet;
    BattleUnit* unit;

    LOG_DEBUG("Remove status %d from target %u", status, target_id);

    /* Remove from player */
    if (player && player->id == target_id) {
        player->buff_flags &= ~(1 << (status - 1));
    }

    /* Remove from pet */
    pet = pet_get_by_id(target_id);
    if (pet) {
        LOG_DEBUG("Status %d removed from pet %u", status, target_id);
    }

    /* Remove from battle unit */
    unit = battle_get_unit(target_id);
    if (unit) {
        battle_remove_buff(unit, status - 1);
    }
}

/*
 * Check if target has status - integrates with character/pet status system
 */
int skill_has_status(u32 target_id, StatusEffect status) {
    Character* player = character_get_player();
    BattleUnit* unit;

    /* Check player */
    if (player && player->id == target_id) {
        return (player->buff_flags & (1 << (status - 1))) != 0;
    }

    /* Check battle unit */
    unit = battle_get_unit(target_id);
    if (unit) {
        return (unit->buff_flags & (1 << (status - 1))) != 0;
    }

    return 0;
}

/*
 * Apply buff to target - integrates with character/pet buff system
 */
void skill_apply_buff(u32 target_id, BuffType buff, u8 duration, u16 power) {
    Character* player = character_get_player();
    BattleUnit* unit;

    LOG_DEBUG("Apply buff %d to target %u for %d turns, power %d", buff, target_id, duration, power);

    /* Apply to player */
    if (player && player->id == target_id) {
        player->buff_flags |= (1 << (buff - 1));
        /* Store duration */
        for (int i = 0; i < 8; i++) {
            if (player->buff_end_time[i] == 0) {
                player->buff_end_time[i] = GetTickCount() + duration * 1000;
                break;
            }
        }
        /* Apply buff effect to stats */
        switch (buff) {
            case BUFF_ATTACK_UP:
                player->stats.attack = player->stats.attack * 120 / 100;
                break;
            case BUFF_DEFENSE_UP:
                player->stats.defense = player->stats.defense * 120 / 100;
                break;
            case BUFF_SPEED_UP:
                player->stats.agility = player->stats.agility * 120 / 100;
                break;
            case BUFF_ATTACK_DOWN:
                player->stats.attack = player->stats.attack * 80 / 100;
                break;
            case BUFF_DEFENSE_DOWN:
                player->stats.defense = player->stats.defense * 80 / 100;
                break;
            default:
                break;
        }
        return;
    }

    /* Apply to battle unit */
    unit = battle_get_unit(target_id);
    if (unit) {
        battle_apply_buff(unit, buff - 1, duration);
    }
}

/*
 * Remove buff - integrates with character/pet buff system
 */
void skill_remove_buff(u32 target_id, BuffType buff) {
    Character* player = character_get_player();
    BattleUnit* unit;

    LOG_DEBUG("Remove buff %d from target %u", buff, target_id);

    /* Remove from player */
    if (player && player->id == target_id) {
        player->buff_flags &= ~(1 << (buff - 1));
        /* Recalculate stats would be done by a separate function */
        return;
    }

    /* Remove from battle unit */
    unit = battle_get_unit(target_id);
    if (unit) {
        battle_remove_buff(unit, buff - 1);
    }
}

/*
 * Check if target has buff - integrates with character/pet buff system
 */
int skill_has_buff(u32 target_id, BuffType buff) {
    Character* player = character_get_player();
    BattleUnit* unit;

    /* Check player */
    if (player && player->id == target_id) {
        return (player->buff_flags & (1 << (buff - 1))) != 0;
    }

    /* Check battle unit */
    unit = battle_get_unit(target_id);
    if (unit) {
        return (unit->buff_flags & (1 << (buff - 1))) != 0;
    }

    return 0;
}
