/*
 * Stone Age Client - Effect Manager
 * Effect add, remove, query functions
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "effect.h"
#include "logger.h"

/* External global */
extern EffectContext g_effect;

/*
 * Add effect
 */
int effect_add(u16 effect_id, u16 level, u32 duration, u32 source_id) {
    EffectData* data;
    ActiveEffect* effect;
    int slot = -1;
    int i;

    data = effect_get_data(effect_id);
    if (!data) {
        LOG_WARN("Unknown effect: %d", effect_id);
        return 0;
    }

    /* Check if already has effect */
    for (i = 0; i < MAX_EFFECTS; i++) {
        if (g_effect.effects[i].effect_id == effect_id) {
            /* Refresh duration if stackable */
            if (data->flags & EFFECT_FLAG_STACKABLE) {
                g_effect.effects[i].duration = duration;
                g_effect.effects[i].remaining = duration;
                g_effect.effects[i].level = level;
                LOG_DEBUG("Effect %d refreshed", effect_id);
                return 1;
            }
            /* Otherwise replace */
            slot = i;
            break;
        }
        if (g_effect.effects[i].effect_id == 0 && slot < 0) {
            slot = i;
        }
    }

    if (slot < 0) {
        LOG_WARN("No free effect slot");
        return 0;
    }

    effect = &g_effect.effects[slot];
    effect->effect_id = effect_id;
    effect->level = level;
    effect->value = data->base_value * level;
    effect->duration = duration;
    effect->remaining = duration;
    effect->tick_interval = data->tick_interval;
    effect->last_tick = timeGetTime();
    effect->source_id = source_id;
    effect->flags = data->flags;
    effect->category = data->category;

    if (slot >= g_effect.effect_count) {
        g_effect.effect_count = slot + 1;
    }

    g_effect.effect_update = 1;

    LOG_INFO("Effect added: %s (level %d, duration %ums)",
             data->name, level, duration);
    return 1;
}

/*
 * Remove effect
 */
int effect_remove(u16 effect_id) {
    int i;

    for (i = 0; i < MAX_EFFECTS; i++) {
        if (g_effect.effects[i].effect_id == effect_id) {
            memset(&g_effect.effects[i], 0, sizeof(ActiveEffect));

            /* Compact array */
            if (i == g_effect.effect_count - 1) {
                g_effect.effect_count--;
                while (g_effect.effect_count > 0 &&
                       g_effect.effects[g_effect.effect_count - 1].effect_id == 0) {
                    g_effect.effect_count--;
                }
            }

            g_effect.effect_update = 1;
            LOG_DEBUG("Effect removed: %d", effect_id);
            return 1;
        }
    }

    return 0;
}

/*
 * Remove all effects
 */
void effect_remove_all(void) {
    memset(g_effect.effects, 0, sizeof(g_effect.effects));
    g_effect.effect_count = 0;
    g_effect.effect_update = 1;
}

/*
 * Remove effects by category
 */
void effect_remove_by_category(EffectCategory category) {
    int i;

    for (i = 0; i < MAX_EFFECTS; i++) {
        if (g_effect.effects[i].effect_id != 0 &&
            g_effect.effects[i].category == category) {
            effect_remove(g_effect.effects[i].effect_id);
        }
    }
}

/*
 * Remove all buffs
 */
void effect_remove_buffs(void) {
    effect_remove_by_category(EFFECT_CAT_BUFF);
}

/*
 * Remove all debuffs
 */
void effect_remove_debuffs(void) {
    effect_remove_by_category(EFFECT_CAT_DEBUFF);
}

/*
 * Dispel effects by priority
 */
int effect_dispel(u8 priority) {
    int i;
    int removed = 0;

    for (i = 0; i < g_effect.effect_count; i++) {
        ActiveEffect* effect = &g_effect.effects[i];
        if (effect->effect_id == 0) continue;

        EffectData* data = effect_get_data(effect->effect_id);
        if (data && (data->flags & EFFECT_FLAG_DISPELLABLE) &&
            data->dispel_priority <= priority) {
            effect_remove(effect->effect_id);
            removed++;
        }
    }

    return removed;
}

/*
 * Check if has effect
 */
int effect_has(u16 effect_id) {
    int i;

    for (i = 0; i < g_effect.effect_count; i++) {
        if (g_effect.effects[i].effect_id == effect_id) {
            return 1;
        }
    }

    return 0;
}

/*
 * Get effect level
 */
int effect_get_level(u16 effect_id) {
    ActiveEffect* effect = effect_get_active(effect_id);
    return effect ? effect->level : 0;
}

/*
 * Get effect value
 */
s32 effect_get_value(u16 effect_id) {
    ActiveEffect* effect = effect_get_active(effect_id);
    return effect ? effect->value : 0;
}

/*
 * Get remaining time
 */
u32 effect_get_remaining(u16 effect_id) {
    ActiveEffect* effect = effect_get_active(effect_id);
    return effect ? effect->remaining : 0;
}

/*
 * Get active effect
 */
ActiveEffect* effect_get_active(u16 effect_id) {
    int i;

    for (i = 0; i < g_effect.effect_count; i++) {
        if (g_effect.effects[i].effect_id == effect_id) {
            return &g_effect.effects[i];
        }
    }

    return NULL;
}

/*
 * Get effect by index
 */
ActiveEffect* effect_get_by_index(int index) {
    if (index < 0 || index >= g_effect.effect_count) {
        return NULL;
    }
    return &g_effect.effects[index];
}

/*
 * Get effect count
 */
int effect_get_count(void) {
    return g_effect.effect_count;
}

/*
 * Get buff count
 */
int effect_get_buff_count(void) {
    int i, count = 0;

    for (i = 0; i < g_effect.effect_count; i++) {
        if (g_effect.effects[i].category == EFFECT_CAT_BUFF) {
            count++;
        }
    }

    return count;
}

/*
 * Get debuff count
 */
int effect_get_debuff_count(void) {
    int i, count = 0;

    for (i = 0; i < g_effect.effect_count; i++) {
        if (g_effect.effects[i].category == EFFECT_CAT_DEBUFF) {
            count++;
        }
    }

    return count;
}
