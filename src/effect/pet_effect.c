/*
 * Stone Age Client - Pet Effect System
 * Pet-specific effect management functions
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "effect.h"
#include "pet.h"
#include "logger.h"

/* External global */
extern EffectContext g_effect;

/*
 * Add pet effect
 */
int pet_effect_add(u32 pet_slot, u16 effect_id, u16 level, u32 duration) {
    PetData* pet;
    EffectData* data;
    int i, slot = -1;

    if (pet_slot >= MAX_PET_SLOTS) return 0;

    pet = pet_get_slot((int)pet_slot);
    if (!pet) return 0;

    data = effect_get_data(effect_id);
    if (!data) return 0;

    /* Check pet's effect slots */
    for (i = 0; i < MAX_PET_EFFECTS; i++) {
        if (g_effect.pet_effects[pet_slot][i].effect_id == effect_id) {
            /* Refresh existing */
            g_effect.pet_effects[pet_slot][i].duration = duration;
            g_effect.pet_effects[pet_slot][i].remaining = duration;
            g_effect.pet_effects[pet_slot][i].level = level;
            return 1;
        }
        if (g_effect.pet_effects[pet_slot][i].effect_id == 0 && slot < 0) {
            slot = i;
        }
    }

    if (slot < 0) return 0;

    /* Add new effect */
    {
        ActiveEffect* effect = &g_effect.pet_effects[pet_slot][slot];
        effect->effect_id = effect_id;
        effect->level = level;
        effect->value = data->base_value * level;
        effect->duration = duration;
        effect->remaining = duration;
        effect->tick_interval = data->tick_interval;
        effect->last_tick = timeGetTime();
        effect->flags = data->flags;
        effect->category = data->category;
    }

    LOG_DEBUG("Pet %d effect added: %s", pet_slot, data->name);
    return 1;
}

/*
 * Remove pet effect
 */
int pet_effect_remove(u32 pet_slot, u16 effect_id) {
    int i;

    if (pet_slot >= MAX_PET_SLOTS) return 0;

    for (i = 0; i < MAX_PET_EFFECTS; i++) {
        if (g_effect.pet_effects[pet_slot][i].effect_id == effect_id) {
            memset(&g_effect.pet_effects[pet_slot][i], 0, sizeof(ActiveEffect));
            LOG_DEBUG("Pet %d effect removed: %d", pet_slot, effect_id);
            return 1;
        }
    }

    return 0;
}

/*
 * Remove all pet effects
 */
void pet_effect_remove_all(u32 pet_slot) {
    if (pet_slot >= MAX_PET_SLOTS) return;
    memset(g_effect.pet_effects[pet_slot], 0, sizeof(g_effect.pet_effects[pet_slot]));
}

/*
 * Check if pet has effect
 */
int pet_effect_has(u32 pet_slot, u16 effect_id) {
    int i;

    if (pet_slot >= MAX_PET_SLOTS) return 0;

    for (i = 0; i < MAX_PET_EFFECTS; i++) {
        if (g_effect.pet_effects[pet_slot][i].effect_id == effect_id) {
            return 1;
        }
    }

    return 0;
}

/*
 * Get pet effect level
 */
int pet_effect_get_level(u32 pet_slot, u16 effect_id) {
    int i;

    if (pet_slot >= MAX_PET_SLOTS) return 0;

    for (i = 0; i < MAX_PET_EFFECTS; i++) {
        if (g_effect.pet_effects[pet_slot][i].effect_id == effect_id) {
            return g_effect.pet_effects[pet_slot][i].level;
        }
    }

    return 0;
}

/*
 * Update pet effects
 */
void pet_effect_update(u32 pet_slot, u32 delta_time) {
    PetData* pet;
    ActiveEffect* effect;
    u32 current_time;
    int i;

    if (pet_slot >= MAX_PET_SLOTS) return;

    pet = pet_get_slot((int)pet_slot);
    if (!pet) return;

    current_time = timeGetTime();

    for (i = 0; i < MAX_PET_EFFECTS; i++) {
        effect = &g_effect.pet_effects[pet_slot][i];
        if (effect->effect_id == 0) continue;

        /* Update remaining time */
        if (effect->remaining > delta_time) {
            effect->remaining -= delta_time;
        } else {
            /* Effect expired */
            memset(effect, 0, sizeof(ActiveEffect));
            continue;
        }

        /* Handle tick effects */
        if (effect->flags & EFFECT_FLAG_TICK) {
            if (current_time - effect->last_tick >= effect->tick_interval) {
                /* Apply effect to pet */
                switch (effect->effect_id) {
                    case EFFECT_POISON:
                        if (pet->hp > effect->value) {
                            pet->hp -= (u16)effect->value;
                        } else {
                            pet->hp = 1;
                        }
                        break;

                    case EFFECT_HP_REGEN:
                        pet->hp += (u16)effect->value;
                        if (pet->hp > pet->max_hp) {
                            pet->hp = pet->max_hp;
                        }
                        break;

                    case EFFECT_BURN:
                    case EFFECT_BLEED:
                        if (pet->hp > effect->value) {
                            pet->hp -= (u16)effect->value;
                        } else {
                            pet->hp = 0;
                        }
                        break;
                }

                effect->last_tick = current_time;
            }
        }
    }
}

/*
 * Check if pet can act
 */
int pet_effect_can_act(u32 pet_slot) {
    if (pet_slot >= MAX_PET_SLOTS) return 0;

    return !pet_effect_has(pet_slot, EFFECT_STUN) &&
           !pet_effect_has(pet_slot, EFFECT_PARALYZE) &&
           !pet_effect_has(pet_slot, EFFECT_SLEEP) &&
           !pet_effect_has(pet_slot, EFFECT_STONE);
}

/*
 * Check if pet can cast
 */
int pet_effect_can_cast(u32 pet_slot) {
    if (pet_slot >= MAX_PET_SLOTS) return 0;

    return !pet_effect_has(pet_slot, EFFECT_SILENCE) &&
           !pet_effect_has(pet_slot, EFFECT_STUN) &&
           !pet_effect_has(pet_slot, EFFECT_PARALYZE);
}
