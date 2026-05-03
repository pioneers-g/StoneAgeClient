/*
 * Stone Age Client - Effect Update System
 * Effect update and tick processing functions
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "effect.h"
#include "character.h"
#include "logger.h"

/* External global */
extern EffectContext g_effect;

/*
 * Update effects
 */
void effect_update(u32 delta_time) {
    int i;
    u32 current_time = timeGetTime();

    g_effect.attack_mod = 0;
    g_effect.defense_mod = 0;
    g_effect.speed_mod = 0;
    g_effect.magic_mod = 0;

    for (i = 0; i < g_effect.effect_count; i++) {
        ActiveEffect* effect = &g_effect.effects[i];
        if (effect->effect_id == 0) continue;

        /* Update remaining time */
        if (effect->remaining > delta_time) {
            effect->remaining -= delta_time;
        } else {
            /* Effect expired */
            LOG_DEBUG("Effect expired: %d", effect->effect_id);
            effect_remove(effect->effect_id);
            continue;
        }

        /* Handle tick effects */
        if (effect->flags & EFFECT_FLAG_TICK) {
            if (current_time - effect->last_tick >= effect->tick_interval) {
                effect_tick(effect);
                effect->last_tick = current_time;
            }
        }

        /* Calculate modifiers */
        switch (effect->effect_id) {
            case EFFECT_ATTACK_UP:
                g_effect.attack_mod += effect->value;
                break;
            case EFFECT_DEFENSE_UP:
                g_effect.defense_mod += effect->value;
                break;
            case EFFECT_SPEED_UP:
                g_effect.speed_mod += effect->value;
                break;
            case EFFECT_MAGIC_UP:
                g_effect.magic_mod += effect->value;
                break;
            case EFFECT_WEAKNESS:
                g_effect.attack_mod -= effect->value;
                break;
            case EFFECT_SLOW:
                g_effect.speed_mod -= effect->value;
                break;
        }
    }
}

/*
 * Process effect tick
 */
void effect_tick(ActiveEffect* effect) {
    Character* player;
    s32 value;

    if (!effect) return;

    player = character_get_player();
    value = effect->value;

    switch (effect->effect_id) {
        case EFFECT_POISON:
            /* Deal damage to character */
            if (player) {
                s32 damage = value;  /* Positive value = damage */
                if (player->stats.hp > damage) {
                    player->stats.hp -= (u16)damage;
                } else {
                    player->stats.hp = 1;  /* Poison doesn't kill */
                }
                LOG_DEBUG("Poison tick: %d damage (HP: %d/%d)",
                         damage, player->stats.hp, player->stats.max_hp);
            }
            break;

        case EFFECT_HP_REGEN:
            /* Restore HP to character */
            if (player) {
                u16 heal = (u16)value;
                player->stats.hp += heal;
                if (player->stats.hp > player->stats.max_hp) {
                    player->stats.hp = player->stats.max_hp;
                }
                LOG_DEBUG("HP Regen tick: +%d HP (HP: %d/%d)",
                         heal, player->stats.hp, player->stats.max_hp);
            }
            break;

        case EFFECT_MP_REGEN:
            /* Restore MP to character */
            if (player) {
                u16 restore = (u16)value;
                player->stats.mp += restore;
                if (player->stats.mp > player->stats.max_mp) {
                    player->stats.mp = player->stats.max_mp;
                }
                LOG_DEBUG("MP Regen tick: +%d MP (MP: %d/%d)",
                         restore, player->stats.mp, player->stats.max_mp);
            }
            break;

        case EFFECT_BURN:
            /* Fire damage to character */
            if (player) {
                s32 damage = value;
                if (player->stats.hp > damage) {
                    player->stats.hp -= (u16)damage;
                } else {
                    player->stats.hp = 0;
                }
                LOG_DEBUG("Burn tick: %d fire damage", damage);
            }
            break;

        case EFFECT_BLEED:
            /* Bleed damage to character */
            if (player) {
                s32 damage = value;
                if (player->stats.hp > damage) {
                    player->stats.hp -= (u16)damage;
                } else {
                    player->stats.hp = 0;
                }
                LOG_DEBUG("Bleed tick: %d damage", damage);
            }
            break;
    }
}

/*
 * Get attack modifier
 */
s32 effect_get_attack_modifier(void) {
    return g_effect.attack_mod;
}

/*
 * Get defense modifier
 */
s32 effect_get_defense_modifier(void) {
    return g_effect.defense_mod;
}

/*
 * Get speed modifier
 */
s32 effect_get_speed_modifier(void) {
    return g_effect.speed_mod;
}

/*
 * Get magic modifier
 */
s32 effect_get_magic_modifier(void) {
    return g_effect.magic_mod;
}
