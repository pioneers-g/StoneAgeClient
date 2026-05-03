/*
 * Stone Age Client - Effect System Core
 * Core initialization, database, and utility functions
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "effect.h"
#include "logger.h"

/* Global effect context */
EffectContext g_effect = {0};

/* Default effect data */
static const EffectData s_default_effects[] = {
    {EFFECT_POISON, "Poison", "Deals damage over time", EFFECT_CAT_DEBUFF,
     EFFECT_FLAG_TICK | EFFECT_FLAG_DISPELLABLE, 10, 0, -10, 30000, 3000, 5, 1},
    {EFFECT_PARALYZE, "Paralyze", "Cannot move or act", EFFECT_CAT_DEBUFF,
     EFFECT_FLAG_DISPELLABLE, 11, 0, 0, 10000, 0, 1, 2},
    {EFFECT_SLEEP, "Sleep", "Asleep, wakes on damage", EFFECT_CAT_DEBUFF,
     EFFECT_FLAG_DISPELLABLE, 12, 0, 0, 15000, 0, 1, 2},
    {EFFECT_CONFUSE, "Confuse", "Random actions", EFFECT_CAT_DEBUFF,
     EFFECT_FLAG_DISPELLABLE, 13, 0, 0, 20000, 0, 1, 2},
    {EFFECT_SILENCE, "Silence", "Cannot use magic", EFFECT_CAT_DEBUFF,
     EFFECT_FLAG_DISPELLABLE, 14, 0, 0, 30000, 0, 1, 2},
    {EFFECT_BLIND, "Blind", "Reduced accuracy", EFFECT_CAT_DEBUFF,
     EFFECT_FLAG_DISPELLABLE, 15, 0, -50, 20000, 0, 1, 1},
    {EFFECT_ATTACK_UP, "Attack Up", "Increases attack power", EFFECT_CAT_BUFF,
     EFFECT_FLAG_DISPELLABLE, 20, 0, 20, 60000, 0, 5, 1},
    {EFFECT_DEFENSE_UP, "Defense Up", "Increases defense", EFFECT_CAT_BUFF,
     EFFECT_FLAG_DISPELLABLE, 21, 0, 20, 60000, 0, 5, 1},
    {EFFECT_SPEED_UP, "Speed Up", "Increases movement speed", EFFECT_CAT_BUFF,
     EFFECT_FLAG_DISPELLABLE, 22, 0, 30, 60000, 0, 1, 1},
    {EFFECT_HP_REGEN, "HP Regen", "Regenerates HP over time", EFFECT_CAT_BUFF,
     EFFECT_FLAG_TICK | EFFECT_FLAG_DISPELLABLE, 23, 0, 50, 60000, 5000, 1, 1},
    {EFFECT_MP_REGEN, "MP Regen", "Regenerates MP over time", EFFECT_CAT_BUFF,
     EFFECT_FLAG_TICK | EFFECT_FLAG_DISPELLABLE, 24, 0, 30, 60000, 5000, 1, 1},
    {EFFECT_STUN, "Stun", "Cannot act", EFFECT_CAT_SPECIAL,
     EFFECT_FLAG_NONE, 30, 0, 0, 5000, 0, 1, 3},
    {EFFECT_IMMUNITY, "Immunity", "Immune to debuffs", EFFECT_CAT_BUFF,
     EFFECT_FLAG_DISPELLABLE, 40, 0, 0, 30000, 0, 1, 1}
};

/*
 * Initialize effect system
 */
int effect_init(void) {
    memset(&g_effect, 0, sizeof(EffectContext));

    g_effect.effect_db = (EffectData*)s_default_effects;
    g_effect.db_count = sizeof(s_default_effects) / sizeof(EffectData);

    g_effect.show_buffs = 1;
    g_effect.show_debuffs = 1;

    LOG_INFO("Effect system initialized");
    return 1;
}

/*
 * Shutdown effect system
 */
void effect_shutdown(void) {
    memset(&g_effect, 0, sizeof(EffectContext));
    LOG_INFO("Effect system shutdown");
}

/*
 * Load effect database
 */
int effect_load_database(void) {
    /* Already using static data */
    return 1;
}

/*
 * Get effect data
 */
EffectData* effect_get_data(u16 effect_id) {
    int i;

    for (i = 0; i < g_effect.db_count; i++) {
        if (g_effect.effect_db[i].id == effect_id) {
            return &g_effect.effect_db[i];
        }
    }

    return NULL;
}

/*
 * Get effect name
 */
const char* effect_get_name(u16 effect_id) {
    EffectData* data = effect_get_data(effect_id);
    return data ? data->name : "Unknown";
}

/*
 * Get effect category
 */
EffectCategory effect_get_category(u16 effect_id) {
    EffectData* data = effect_get_data(effect_id);
    return data ? data->category : EFFECT_CAT_NONE;
}

/*
 * Check if effect is buff
 */
int effect_is_buff(u16 effect_id) {
    return effect_get_category(effect_id) == EFFECT_CAT_BUFF;
}

/*
 * Check if effect is debuff
 */
int effect_is_debuff(u16 effect_id) {
    return effect_get_category(effect_id) == EFFECT_CAT_DEBUFF;
}

/*
 * Get effect icon
 */
u16 effect_get_icon(u16 effect_id) {
    EffectData* data = effect_get_data(effect_id);
    return data ? data->icon_id : 0;
}
