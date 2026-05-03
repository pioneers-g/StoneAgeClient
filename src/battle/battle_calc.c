/*
 * Stone Age Client - Battle Calculations
 * Reverse engineered from sa_9061.exe
 * Split from battle.c for code organization
 *
 * Consolidated from battlesystem.c - element system and damage formulas
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "battle.h"
#include "item.h"
#include "logger.h"

/* Element advantage table: Fire > Wind > Earth > Water > Fire */
/* Light <-> Dark (mutual weakness) */
static const float s_element_table[ELEMENT_MAX][ELEMENT_MAX] = {
    /* Target: None, Fire, Water, Earth, Wind, Light, Dark */
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},  /* None attacks */
    {1.0f, 1.0f, 0.5f, 1.0f, 1.5f, 1.0f, 1.0f},  /* Fire attacks */
    {1.0f, 1.5f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f},  /* Water attacks */
    {1.0f, 1.0f, 1.5f, 1.0f, 0.5f, 1.0f, 1.0f},  /* Earth attacks */
    {1.0f, 0.5f, 1.0f, 1.5f, 1.0f, 1.0f, 1.0f},  /* Wind attacks */
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.5f},  /* Light attacks */
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.5f, 1.0f},  /* Dark attacks */
};

/* External battle context */
extern BattleContext g_battle;

/* Skill database (placeholder) */
static BattleSkill g_skills[100] = {0};

/*
 * Calculate damage - FUN_00425380 derived
 */
u16 battle_calc_damage(BattleUnit* attacker, BattleUnit* target, BattleSkill* skill) {
    s32 damage;
    s32 attack, defense;

    if (!attacker || !target) return 0;

    attack = attacker->attack;
    defense = target->defense;

    /* Apply attack buffs */
    if (attacker->buff_flags & BUFF_ATTACK_UP) {
        attack = attack * 12 / 10;  /* +20% */
    }
    if (attacker->buff_flags & BUFF_ATTACK_DOWN) {
        attack = attack * 8 / 10;   /* -20% */
    }

    /* Apply defense buffs */
    if (target->buff_flags & BUFF_DEFENSE_UP) {
        defense = defense * 12 / 10;
    }
    if (target->buff_flags & BUFF_DEFENSE_DOWN) {
        defense = defense * 8 / 10;
    }

    /* Basic damage formula from Ghidra analysis */
    if (skill) {
        damage = skill->power + attack / 2 - defense / 4;
    } else {
        damage = attack - defense / 2;
    }

    /* Minimum damage */
    if (damage < 1) damage = 1;

    /* Random variance 90-110% */
    damage = damage * (90 + rand() % 21) / 100;

    return (u16)damage;
}

/*
 * Calculate skill damage
 */
u16 battle_calc_skill_damage(BattleUnit* attacker, BattleUnit* target, BattleSkill* skill) {
    s32 damage;

    if (!skill) return 0;
    if (!attacker || !target) return 0;

    /* Skill damage formula */
    damage = skill->power + attacker->attack / 2 - target->defense / 4;
    if (damage < 1) damage = 1;

    return (u16)damage;
}

/*
 * Calculate hit chance
 */
int battle_calc_hit(BattleUnit* attacker, BattleUnit* target) {
    int hit_chance = 90;  /* Base 90% */

    if (!attacker || !target) return 0;

    /* Always hit stunned targets */
    if (target->buff_flags & BUFF_STUN) {
        return 1;
    }

    /* Blind reduces hit chance */
    if (attacker->buff_flags & BUFF_BLIND) {
        hit_chance = hit_chance * 7 / 10;  /* -30% */
    }

    return (rand() % 100) < hit_chance;
}

/*
 * Calculate critical hit
 */
int battle_calc_critical(BattleUnit* attacker, BattleUnit* target) {
    int crit_chance = 5;  /* Base 5% */

    if (!attacker || !target) return 0;

    /* Higher crit chance against debuffed targets */
    if (target->buff_flags & (BUFF_STUN | BUFF_SLEEP | BUFF_CONFUSE)) {
        crit_chance += 10;
    }

    return (rand() % 100) < crit_chance;
}

/*
 * Calculate capture rate
 */
int battle_calc_capture_rate(int enemy_hp_percent, int enemy_level, int capture_item_id) {
    int base_rate;

    /* Base capture rate from HP percentage */
    base_rate = 100 - enemy_hp_percent;
    base_rate = base_rate / 2;  /* Max 50% */

    /* Level difference penalty */
    if (enemy_level > 50) {
        base_rate -= (enemy_level - 50);
    }

    /* Item bonus */
    if (capture_item_id > 0) {
        base_rate += 10;  /* Capture item bonus */
    }

    /* Clamp */
    if (base_rate < 5) base_rate = 5;
    if (base_rate > 80) base_rate = 80;

    return base_rate;
}

/*
 * Get skill by ID
 */
BattleSkill* battle_get_skill(u16 skill_id) {
    if (skill_id >= 100) return NULL;
    return &g_skills[skill_id];
}

/*
 * Use skill in battle
 */
int battle_use_skill(BattleUnit* user, BattleUnit* target, u16 skill_id) {
    BattleSkill* skill = battle_get_skill(skill_id);
    u16 damage;

    if (!skill || !user) return 0;
    if (user->mp < skill->mp_cost) return 0;

    /* Deduct MP */
    user->mp -= skill->mp_cost;

    /* Apply skill effect */
    if (target && target->is_alive) {
        switch (skill->type) {
            case 0:  /* Attack skill */
                damage = battle_calc_skill_damage(user, target, skill);
                target->hp -= damage;
                if (target->hp <= 0) {
                    target->hp = 0;
                    target->is_alive = 0;
                }
                break;

            case 1:  /* Heal skill */
                target->hp += skill->power;
                if (target->hp > target->max_hp) {
                    target->hp = target->max_hp;
                }
                break;

            case 2:  /* Buff skill */
                target->buff_flags |= skill->flags;
                /* Set buff duration */
                if (skill->element < 16) {
                    target->buff_turns[skill->element] = 3;
                }
                break;

            default:
                LOG_DEBUG("Unknown skill type: %d", skill->type);
                break;
        }
    }

    LOG_DEBUG("Skill %d used by unit %d", skill_id, user->id);
    return 1;
}

/*
 * Check if skill can be used
 */
int battle_can_use_skill(BattleUnit* unit, u16 skill_id) {
    BattleSkill* skill = battle_get_skill(skill_id);

    if (!skill || !unit) return 0;
    return unit->mp >= skill->mp_cost;
}

/*
 * Use item in battle
 */
int battle_use_item(BattleUnit* user, BattleUnit* target, u16 item_id) {
    Item* item;
    u16 effect_type;
    s16 effect_value;

    if (!user) return 0;

    /* Get item data */
    item = item_get(item_id);
    if (!item) {
        LOG_WARN("Unknown item %d", item_id);
        return 0;
    }

    /* Default target to self if not specified */
    if (!target) target = user;

    effect_type = item->effect_type;
    effect_value = item->effect_value;

    /* Apply item effect */
    switch (effect_type) {
        case EFFECT_HEAL_HP:
            /* Heal HP */
            target->hp += effect_value;
            if (target->hp > target->max_hp) {
                target->hp = target->max_hp;
            }
            LOG_DEBUG("Item %d healed %d HP to unit %d", item_id, effect_value, target->id);
            break;

        case EFFECT_HEAL_MP:
            /* Heal MP */
            target->mp += effect_value;
            if (target->mp > target->max_mp) {
                target->mp = target->max_mp;
            }
            LOG_DEBUG("Item %d restored %d MP to unit %d", item_id, effect_value, target->id);
            break;

        case EFFECT_BUFF_ATTACK:
            /* Buff attack - set buff flag */
            target->buff_flags |= BUFF_ATTACK_UP;
            if (target->buff_turns[0] < 3) target->buff_turns[0] = 3;
            LOG_DEBUG("Item %d buffed attack for unit %d", item_id, target->id);
            break;

        case EFFECT_BUFF_DEFENSE:
            /* Buff defense - set buff flag */
            target->buff_flags |= BUFF_DEFENSE_UP;
            if (target->buff_turns[2] < 3) target->buff_turns[2] = 3;
            LOG_DEBUG("Item %d buffed defense for unit %d", item_id, target->id);
            break;

        case EFFECT_BUFF_SPEED:
            /* Buff speed - set buff flag */
            target->buff_flags |= BUFF_SPEED_UP;
            if (target->buff_turns[4] < 3) target->buff_turns[4] = 3;
            LOG_DEBUG("Item %d buffed speed for unit %d", item_id, target->id);
            break;

        case EFFECT_CURE_POISON:
            /* Cure poison status */
            target->buff_flags &= ~BUFF_POISON;
            LOG_DEBUG("Item %d cured poison on unit %d", item_id, target->id);
            break;

        case EFFECT_CURE_STUN:
            /* Cure stun status */
            target->buff_flags &= ~BUFF_STUN;
            LOG_DEBUG("Item %d cured stun on unit %d", item_id, target->id);
            break;

        default:
            LOG_DEBUG("Item %d used by unit %d (effect type %d)", item_id, user->id, effect_type);
            break;
    }

    return 1;
}

/*
 * Apply buff to unit
 */
void battle_apply_buff(BattleUnit* unit, u32 buff_id, u8 turns) {
    if (!unit || buff_id >= 16) return;

    unit->buff_flags |= (1 << buff_id);
    unit->buff_turns[buff_id] = turns;

    LOG_DEBUG("Buff %d applied to unit %d for %d turns", buff_id, unit->id, turns);
}

/*
 * Remove buff from unit
 */
void battle_remove_buff(BattleUnit* unit, u32 buff_id) {
    if (!unit || buff_id >= 16) return;

    unit->buff_flags &= ~(1 << buff_id);
    unit->buff_turns[buff_id] = 0;
}

/*
 * Update all buffs (call at end of turn)
 */
void battle_update_buffs(void) {
    u32 i, j;

    for (i = 0; i < g_battle.unit_count; i++) {
        for (j = 0; j < 16; j++) {
            if (g_battle.units[i].buff_turns[j] > 0) {
                g_battle.units[i].buff_turns[j]--;
                if (g_battle.units[i].buff_turns[j] == 0) {
                    /* Remove expired buff */
                    g_battle.units[i].buff_flags &= ~(1 << j);
                    LOG_DEBUG("Buff %d expired on unit %d", j, g_battle.units[i].id);
                }
            }
        }
    }
}

/*
 * Calculate escape chance
 */
int battle_calc_escape_chance(BattleUnit* player, BattleUnit* fastest_enemy) {
    int escape_chance = 50;  /* Base 50% */

    if (!player) return escape_chance;

    /* Speed difference bonus */
    if (fastest_enemy) {
        int speed_diff = player->speed - fastest_enemy->speed;
        escape_chance += speed_diff;
    }

    /* Clamp */
    if (escape_chance < 10) escape_chance = 10;
    if (escape_chance > 90) escape_chance = 90;

    return escape_chance;
}
