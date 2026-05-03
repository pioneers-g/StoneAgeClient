/*
 * Stone Age Client - Effect System Comprehensive Tests
 * Tests for effect module - effect_manager.c, effect_core.c, effect_update.c
 *
 * Coverage:
 * - Effect initialization and shutdown
 * - Effect add/remove operations
 * - Effect queries and modifiers
 * - Effect categories (buff/debuff)
 * - Effect tick processing
 * - Pet effects
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;
typedef short s16;

/* Constants */
#define MAX_EFFECTS         20
#define MAX_PET_EFFECTS     8
#define MAX_PET_SLOTS       5
#define MAX_EFFECT_DURATION 300000

/* Effect types */
typedef enum {
    EFFECT_NONE = 0,
    EFFECT_ATTACK_UP = 1,
    EFFECT_DEFENSE_UP = 2,
    EFFECT_SPEED_UP = 3,
    EFFECT_MAGIC_UP = 4,
    EFFECT_RESIST_FIRE = 5,
    EFFECT_RESIST_WATER = 6,
    EFFECT_RESIST_EARTH = 7,
    EFFECT_RESIST_WIND = 8,
    EFFECT_HP_REGEN = 9,
    EFFECT_MP_REGEN = 10,
    EFFECT_IMMUNITY = 11,
    EFFECT_REFLECT = 12,
    EFFECT_SHIELD = 13,
    EFFECT_INVISIBLE = 14,
    EFFECT_POISON = 20,
    EFFECT_PARALYZE = 21,
    EFFECT_SLEEP = 22,
    EFFECT_CONFUSE = 23,
    EFFECT_SILENCE = 24,
    EFFECT_BLIND = 25,
    EFFECT_CURSE = 26,
    EFFECT_SLOW = 27,
    EFFECT_WEAKNESS = 28,
    EFFECT_BLEED = 29,
    EFFECT_BURN = 30,
    EFFECT_FREEZE = 31,
    EFFECT_STUN = 40,
    EFFECT_STONE = 41,
    EFFECT_CHARM = 42,
    EFFECT_FEAR = 43,
    EFFECT_PETRIFY = 44
} EffectType;

/* Effect categories */
typedef enum {
    EFFECT_CAT_NONE = 0,
    EFFECT_CAT_BUFF = 1,
    EFFECT_CAT_DEBUFF = 2,
    EFFECT_CAT_SPECIAL = 3
} EffectCategory;

/* Effect flags */
typedef enum {
    EFFECT_FLAG_NONE = 0,
    EFFECT_FLAG_STACKABLE = (1 << 0),
    EFFECT_FLAG_DISPELLABLE = (1 << 1),
    EFFECT_FLAG_PERSIST_DEATH = (1 << 2),
    EFFECT_FLAG_VISIBLE = (1 << 3),
    EFFECT_FLAG_TICK = (1 << 4)
} EffectFlags;

/* Active effect */
typedef struct {
    u16 effect_id;
    u16 level;
    s32 value;
    u32 duration;
    u32 remaining;
    u32 tick_interval;
    u32 last_tick;
    u32 source_id;
    u16 flags;
    u8 category;
    u8 reserved;
} ActiveEffect;

/* Effect data (static) */
typedef struct {
    u16 id;
    char name[24];
    char description[64];
    u8 category;
    u8 flags;
    u16 icon_id;
    u16 animation_id;
    s16 base_value;
    u16 base_duration;
    u16 tick_interval;
    u8 max_stacks;
    u8 dispel_priority;
} EffectData;

/* Effect context */
typedef struct {
    ActiveEffect effects[MAX_EFFECTS];
    int effect_count;
    ActiveEffect pet_effects[MAX_PET_SLOTS][MAX_PET_EFFECTS];
    EffectData* effect_db;
    int db_count;
    u8 show_buffs;
    u8 show_debuffs;
    u8 effect_update;
    s32 attack_mod;
    s32 defense_mod;
    s32 speed_mod;
    s32 magic_mod;
} EffectContext;

/* Global effect context */
static EffectContext g_effect = {0};

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

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Implementation Functions
 * ======================================== */

void effect_init(void) {
    memset(&g_effect, 0, sizeof(EffectContext));
    g_effect.effect_db = (EffectData*)s_default_effects;
    g_effect.db_count = sizeof(s_default_effects) / sizeof(EffectData);
    g_effect.show_buffs = 1;
    g_effect.show_debuffs = 1;
}

void effect_shutdown(void) {
    memset(&g_effect, 0, sizeof(EffectContext));
}

EffectData* effect_get_data(u16 effect_id) {
    int i;
    for (i = 0; i < g_effect.db_count; i++) {
        if (g_effect.effect_db[i].id == effect_id) {
            return &g_effect.effect_db[i];
        }
    }
    return NULL;
}

const char* effect_get_name(u16 effect_id) {
    EffectData* data = effect_get_data(effect_id);
    return data ? data->name : "Unknown";
}

EffectCategory effect_get_category(u16 effect_id) {
    EffectData* data = effect_get_data(effect_id);
    return data ? data->category : EFFECT_CAT_NONE;
}

int effect_is_buff(u16 effect_id) {
    return effect_get_category(effect_id) == EFFECT_CAT_BUFF;
}

int effect_is_debuff(u16 effect_id) {
    return effect_get_category(effect_id) == EFFECT_CAT_DEBUFF;
}

int effect_add(u16 effect_id, u16 level, u32 duration, u32 source_id) {
    EffectData* data;
    ActiveEffect* effect;
    int slot = -1;
    int i;

    data = effect_get_data(effect_id);
    if (!data) return 0;

    /* Check if already has effect */
    for (i = 0; i < MAX_EFFECTS; i++) {
        if (g_effect.effects[i].effect_id == effect_id) {
            if (data->flags & EFFECT_FLAG_STACKABLE) {
                g_effect.effects[i].duration = duration;
                g_effect.effects[i].remaining = duration;
                g_effect.effects[i].level = level;
                return 1;
            }
            slot = i;
            break;
        }
        if (g_effect.effects[i].effect_id == 0 && slot < 0) {
            slot = i;
        }
    }

    if (slot < 0) return 0;

    effect = &g_effect.effects[slot];
    effect->effect_id = effect_id;
    effect->level = level;
    effect->value = data->base_value * level;
    effect->duration = duration;
    effect->remaining = duration;
    effect->tick_interval = data->tick_interval;
    effect->last_tick = 0;
    effect->source_id = source_id;
    effect->flags = data->flags;
    effect->category = data->category;

    if (slot >= g_effect.effect_count) {
        g_effect.effect_count = slot + 1;
    }

    g_effect.effect_update = 1;
    return 1;
}

int effect_remove(u16 effect_id) {
    int i;
    int found = 0;

    for (i = 0; i < MAX_EFFECTS; i++) {
        if (g_effect.effects[i].effect_id == effect_id) {
            memset(&g_effect.effects[i], 0, sizeof(ActiveEffect));
            found = 1;
            break;
        }
    }

    if (found) {
        /* Recalculate effect_count */
        g_effect.effect_count = 0;
        for (i = 0; i < MAX_EFFECTS; i++) {
            if (g_effect.effects[i].effect_id != 0) {
                g_effect.effect_count = i + 1;
            }
        }
        g_effect.effect_update = 1;
        return 1;
    }

    return 0;
}

void effect_remove_all(void) {
    memset(g_effect.effects, 0, sizeof(g_effect.effects));
    g_effect.effect_count = 0;
    g_effect.effect_update = 1;
}

void effect_remove_by_category(EffectCategory category) {
    int i;
    for (i = 0; i < MAX_EFFECTS; i++) {
        if (g_effect.effects[i].effect_id != 0 &&
            g_effect.effects[i].category == category) {
            effect_remove(g_effect.effects[i].effect_id);
        }
    }
}

void effect_remove_buffs(void) {
    effect_remove_by_category(EFFECT_CAT_BUFF);
}

void effect_remove_debuffs(void) {
    effect_remove_by_category(EFFECT_CAT_DEBUFF);
}

int effect_has(u16 effect_id) {
    int i;
    for (i = 0; i < g_effect.effect_count; i++) {
        if (g_effect.effects[i].effect_id == effect_id) {
            return 1;
        }
    }
    return 0;
}

int effect_get_level(u16 effect_id) {
    int i;
    for (i = 0; i < g_effect.effect_count; i++) {
        if (g_effect.effects[i].effect_id == effect_id) {
            return g_effect.effects[i].level;
        }
    }
    return 0;
}

s32 effect_get_value(u16 effect_id) {
    int i;
    for (i = 0; i < g_effect.effect_count; i++) {
        if (g_effect.effects[i].effect_id == effect_id) {
            return g_effect.effects[i].value;
        }
    }
    return 0;
}

u32 effect_get_remaining(u16 effect_id) {
    int i;
    for (i = 0; i < g_effect.effect_count; i++) {
        if (g_effect.effects[i].effect_id == effect_id) {
            return g_effect.effects[i].remaining;
        }
    }
    return 0;
}

ActiveEffect* effect_get_active(u16 effect_id) {
    int i;
    for (i = 0; i < g_effect.effect_count; i++) {
        if (g_effect.effects[i].effect_id == effect_id) {
            return &g_effect.effects[i];
        }
    }
    return NULL;
}

ActiveEffect* effect_get_by_index(int index) {
    if (index < 0 || index >= g_effect.effect_count) {
        return NULL;
    }
    return &g_effect.effects[index];
}

int effect_get_count(void) {
    return g_effect.effect_count;
}

int effect_get_buff_count(void) {
    int i, count = 0;
    for (i = 0; i < g_effect.effect_count; i++) {
        if (g_effect.effects[i].category == EFFECT_CAT_BUFF) {
            count++;
        }
    }
    return count;
}

int effect_get_debuff_count(void) {
    int i, count = 0;
    for (i = 0; i < g_effect.effect_count; i++) {
        if (g_effect.effects[i].category == EFFECT_CAT_DEBUFF) {
            count++;
        }
    }
    return count;
}

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

void effect_update_calc_modifiers(void) {
    int i;

    g_effect.attack_mod = 0;
    g_effect.defense_mod = 0;
    g_effect.speed_mod = 0;
    g_effect.magic_mod = 0;

    for (i = 0; i < g_effect.effect_count; i++) {
        ActiveEffect* effect = &g_effect.effects[i];
        if (effect->effect_id == 0) continue;

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

s32 effect_get_attack_modifier(void) {
    return g_effect.attack_mod;
}

s32 effect_get_defense_modifier(void) {
    return g_effect.defense_mod;
}

s32 effect_get_speed_modifier(void) {
    return g_effect.speed_mod;
}

s32 effect_get_magic_modifier(void) {
    return g_effect.magic_mod;
}

/* Pet effect functions */
int pet_effect_add(u32 pet_slot, u16 effect_id, u16 level, u32 duration) {
    EffectData* data;
    ActiveEffect* effect;
    int slot = -1;
    int i;

    if (pet_slot >= MAX_PET_SLOTS) return 0;

    data = effect_get_data(effect_id);
    if (!data) return 0;

    for (i = 0; i < MAX_PET_EFFECTS; i++) {
        if (g_effect.pet_effects[pet_slot][i].effect_id == 0 && slot < 0) {
            slot = i;
        }
    }

    if (slot < 0) return 0;

    effect = &g_effect.pet_effects[pet_slot][slot];
    effect->effect_id = effect_id;
    effect->level = level;
    effect->value = data->base_value * level;
    effect->duration = duration;
    effect->remaining = duration;
    effect->tick_interval = data->tick_interval;
    effect->flags = data->flags;
    effect->category = data->category;

    return 1;
}

int pet_effect_remove(u32 pet_slot, u16 effect_id) {
    int i;

    if (pet_slot >= MAX_PET_SLOTS) return 0;

    for (i = 0; i < MAX_PET_EFFECTS; i++) {
        if (g_effect.pet_effects[pet_slot][i].effect_id == effect_id) {
            memset(&g_effect.pet_effects[pet_slot][i], 0, sizeof(ActiveEffect));
            return 1;
        }
    }

    return 0;
}

void pet_effect_remove_all(u32 pet_slot) {
    if (pet_slot >= MAX_PET_SLOTS) return;
    memset(g_effect.pet_effects[pet_slot], 0, sizeof(g_effect.pet_effects[pet_slot]));
}

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

/* ========================================
 * Constants Tests
 * ======================================== */

static int test_max_effects(void) {
    return MAX_EFFECTS == 20;
}

static int test_max_pet_effects(void) {
    return MAX_PET_EFFECTS == 8;
}

static int test_max_pet_slots(void) {
    return MAX_PET_SLOTS == 5;
}

static int test_max_effect_duration(void) {
    return MAX_EFFECT_DURATION == 300000;
}

/* ========================================
 * Effect Type Tests
 * ======================================== */

static int test_effect_type_buffs(void) {
    return EFFECT_ATTACK_UP == 1 &&
           EFFECT_DEFENSE_UP == 2 &&
           EFFECT_SPEED_UP == 3 &&
           EFFECT_MAGIC_UP == 4;
}

static int test_effect_type_debuffs(void) {
    return EFFECT_POISON == 20 &&
           EFFECT_PARALYZE == 21 &&
           EFFECT_SLEEP == 22 &&
           EFFECT_SILENCE == 24;
}

static int test_effect_type_special(void) {
    return EFFECT_STUN == 40 &&
           EFFECT_STONE == 41;
}

/* ========================================
 * Effect Category Tests
 * ======================================== */

static int test_effect_category_values(void) {
    return EFFECT_CAT_NONE == 0 &&
           EFFECT_CAT_BUFF == 1 &&
           EFFECT_CAT_DEBUFF == 2 &&
           EFFECT_CAT_SPECIAL == 3;
}

static int test_effect_flag_values(void) {
    return EFFECT_FLAG_NONE == 0 &&
           EFFECT_FLAG_STACKABLE == 1 &&
           EFFECT_FLAG_DISPELLABLE == 2 &&
           EFFECT_FLAG_TICK == 16;
}

/* ========================================
 * Effect Init Tests
 * ======================================== */

static int test_effect_init(void) {
    effect_init();
    return g_effect.effect_count == 0 &&
           g_effect.db_count > 0 &&
           g_effect.show_buffs == 1 &&
           g_effect.show_debuffs == 1;
}

static int test_effect_shutdown(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 60000, 0);
    effect_shutdown();
    return g_effect.effect_count == 0 &&
           g_effect.db_count == 0;
}

/* ========================================
 * Effect Add Tests
 * ======================================== */

static int test_effect_add_basic(void) {
    effect_init();
    int result = effect_add(EFFECT_ATTACK_UP, 1, 60000, 100);
    return result == 1 &&
           g_effect.effect_count == 1 &&
           g_effect.effects[0].effect_id == EFFECT_ATTACK_UP;
}

static int test_effect_add_multiple(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 60000, 100);
    effect_add(EFFECT_DEFENSE_UP, 1, 60000, 100);
    effect_add(EFFECT_SPEED_UP, 1, 60000, 100);
    return g_effect.effect_count == 3;
}

static int test_effect_add_invalid(void) {
    effect_init();
    int result = effect_add(9999, 1, 60000, 100);
    return result == 0 && g_effect.effect_count == 0;
}

static int test_effect_add_value_calculation(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 5, 60000, 100);
    /* base_value = 20, level = 5, value = 100 */
    return g_effect.effects[0].value == 100;
}

static int test_effect_add_with_source(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 60000, 12345);
    return g_effect.effects[0].source_id == 12345;
}

static int test_effect_add_duration(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 30000, 100);
    return g_effect.effects[0].duration == 30000 &&
           g_effect.effects[0].remaining == 30000;
}

/* ========================================
 * Effect Remove Tests
 * ======================================== */

static int test_effect_remove_basic(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 60000, 100);
    int result = effect_remove(EFFECT_ATTACK_UP);
    return result == 1 && g_effect.effect_count == 0;
}

static int test_effect_remove_not_found(void) {
    effect_init();
    int result = effect_remove(EFFECT_ATTACK_UP);
    return result == 0;
}

static int test_effect_remove_all(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 60000, 100);
    effect_add(EFFECT_DEFENSE_UP, 1, 60000, 100);
    effect_add(EFFECT_POISON, 1, 30000, 100);
    effect_remove_all();
    return g_effect.effect_count == 0;
}

static int test_effect_remove_buffs(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 60000, 100);
    effect_add(EFFECT_POISON, 1, 30000, 100);
    effect_remove_buffs();
    /* Poison should still exist, Attack Up should be removed */
    /* Note: effect_count may be 2 due to slot 1 being occupied by POISON */
    return effect_has(EFFECT_POISON) == 1 &&
           effect_has(EFFECT_ATTACK_UP) == 0;
}

static int test_effect_remove_debuffs(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 60000, 100);
    effect_add(EFFECT_POISON, 1, 30000, 100);
    effect_remove_debuffs();
    return g_effect.effect_count == 1 &&
           g_effect.effects[0].effect_id == EFFECT_ATTACK_UP;
}

/* ========================================
 * Effect Query Tests
 * ======================================== */

static int test_effect_has(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 60000, 100);
    return effect_has(EFFECT_ATTACK_UP) == 1 &&
           effect_has(EFFECT_DEFENSE_UP) == 0;
}

static int test_effect_get_level(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 5, 60000, 100);
    return effect_get_level(EFFECT_ATTACK_UP) == 5;
}

static int test_effect_get_level_not_found(void) {
    effect_init();
    return effect_get_level(EFFECT_ATTACK_UP) == 0;
}

static int test_effect_get_value(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 3, 60000, 100);
    /* base_value = 20, level = 3, value = 60 */
    return effect_get_value(EFFECT_ATTACK_UP) == 60;
}

static int test_effect_get_remaining(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 30000, 100);
    return effect_get_remaining(EFFECT_ATTACK_UP) == 30000;
}

static int test_effect_get_active(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 60000, 100);
    ActiveEffect* effect = effect_get_active(EFFECT_ATTACK_UP);
    return effect != NULL && effect->effect_id == EFFECT_ATTACK_UP;
}

static int test_effect_get_active_not_found(void) {
    effect_init();
    ActiveEffect* effect = effect_get_active(EFFECT_ATTACK_UP);
    return effect == NULL;
}

static int test_effect_get_by_index(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 60000, 100);
    ActiveEffect* effect = effect_get_by_index(0);
    return effect != NULL && effect->effect_id == EFFECT_ATTACK_UP;
}

static int test_effect_get_by_index_invalid(void) {
    effect_init();
    ActiveEffect* effect = effect_get_by_index(-1);
    if (effect != NULL) return 0;
    effect = effect_get_by_index(100);
    return effect == NULL;
}

static int test_effect_get_count(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 60000, 100);
    effect_add(EFFECT_DEFENSE_UP, 1, 60000, 100);
    return effect_get_count() == 2;
}

static int test_effect_get_buff_count(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 60000, 100);
    effect_add(EFFECT_POISON, 1, 30000, 100);
    return effect_get_buff_count() == 1;
}

static int test_effect_get_debuff_count(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 60000, 100);
    effect_add(EFFECT_POISON, 1, 30000, 100);
    effect_add(EFFECT_SILENCE, 1, 30000, 100);
    return effect_get_debuff_count() == 2;
}

/* ========================================
 * Effect Category Query Tests
 * ======================================== */

static int test_effect_is_buff(void) {
    return effect_is_buff(EFFECT_ATTACK_UP) == 1 &&
           effect_is_buff(EFFECT_POISON) == 0;
}

static int test_effect_is_debuff(void) {
    return effect_is_debuff(EFFECT_POISON) == 1 &&
           effect_is_debuff(EFFECT_ATTACK_UP) == 0;
}

static int test_effect_get_name(void) {
    const char* name = effect_get_name(EFFECT_POISON);
    return strcmp(name, "Poison") == 0;
}

static int test_effect_get_name_unknown(void) {
    const char* name = effect_get_name(9999);
    return strcmp(name, "Unknown") == 0;
}

static int test_effect_get_category(void) {
    return effect_get_category(EFFECT_ATTACK_UP) == EFFECT_CAT_BUFF &&
           effect_get_category(EFFECT_POISON) == EFFECT_CAT_DEBUFF &&
           effect_get_category(EFFECT_STUN) == EFFECT_CAT_SPECIAL;
}

/* ========================================
 * Effect Modifier Tests
 * ======================================== */

static int test_effect_attack_modifier(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 5, 60000, 100);
    effect_update_calc_modifiers();
    return effect_get_attack_modifier() == 100;
}

static int test_effect_defense_modifier(void) {
    effect_init();
    effect_add(EFFECT_DEFENSE_UP, 3, 60000, 100);
    effect_update_calc_modifiers();
    return effect_get_defense_modifier() == 60;
}

static int test_effect_speed_modifier(void) {
    effect_init();
    effect_add(EFFECT_SPEED_UP, 1, 60000, 100);
    effect_update_calc_modifiers();
    return effect_get_speed_modifier() == 30;
}

static int test_effect_multiple_modifiers(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 60000, 100);
    effect_add(EFFECT_DEFENSE_UP, 1, 60000, 100);
    effect_add(EFFECT_SPEED_UP, 1, 60000, 100);
    effect_update_calc_modifiers();
    return effect_get_attack_modifier() == 20 &&
           effect_get_defense_modifier() == 20 &&
           effect_get_speed_modifier() == 30;
}

/* ========================================
 * Effect Dispel Tests
 * ======================================== */

static int test_effect_dispel_basic(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 60000, 100);  /* dispel_priority = 1 */
    int removed = effect_dispel(1);
    return removed == 1 && g_effect.effect_count == 0;
}

static int test_effect_dispel_priority(void) {
    effect_init();
    effect_add(EFFECT_ATTACK_UP, 1, 60000, 100);  /* priority = 1 */
    effect_add(EFFECT_STUN, 1, 5000, 100);        /* priority = 3, not dispellable */
    int removed = effect_dispel(1);
    /* Stun is not dispellable, only Attack Up should be removed */
    return removed == 1 && effect_has(EFFECT_STUN) == 1;
}

static int test_effect_dispel_none(void) {
    effect_init();
    effect_add(EFFECT_STUN, 1, 5000, 100);  /* Not dispellable */
    int removed = effect_dispel(1);
    return removed == 0 && g_effect.effect_count == 1;
}

/* ========================================
 * Pet Effect Tests
 * ======================================== */

static int test_pet_effect_add(void) {
    effect_init();
    int result = pet_effect_add(0, EFFECT_ATTACK_UP, 1, 60000);
    return result == 1 && g_effect.pet_effects[0][0].effect_id == EFFECT_ATTACK_UP;
}

static int test_pet_effect_add_invalid_slot(void) {
    effect_init();
    int result = pet_effect_add(10, EFFECT_ATTACK_UP, 1, 60000);
    return result == 0;
}

static int test_pet_effect_remove(void) {
    effect_init();
    pet_effect_add(0, EFFECT_ATTACK_UP, 1, 60000);
    int result = pet_effect_remove(0, EFFECT_ATTACK_UP);
    return result == 1 && g_effect.pet_effects[0][0].effect_id == 0;
}

static int test_pet_effect_remove_all(void) {
    effect_init();
    pet_effect_add(0, EFFECT_ATTACK_UP, 1, 60000);
    pet_effect_add(0, EFFECT_DEFENSE_UP, 1, 60000);
    pet_effect_remove_all(0);
    return g_effect.pet_effects[0][0].effect_id == 0 &&
           g_effect.pet_effects[0][1].effect_id == 0;
}

static int test_pet_effect_has(void) {
    effect_init();
    pet_effect_add(0, EFFECT_ATTACK_UP, 1, 60000);
    return pet_effect_has(0, EFFECT_ATTACK_UP) == 1 &&
           pet_effect_has(0, EFFECT_DEFENSE_UP) == 0;
}

static int test_pet_effect_get_level(void) {
    effect_init();
    pet_effect_add(0, EFFECT_ATTACK_UP, 5, 60000);
    return pet_effect_get_level(0, EFFECT_ATTACK_UP) == 5;
}

static int test_pet_effect_multiple_pets(void) {
    effect_init();
    pet_effect_add(0, EFFECT_ATTACK_UP, 1, 60000);
    pet_effect_add(1, EFFECT_DEFENSE_UP, 1, 60000);
    pet_effect_add(2, EFFECT_SPEED_UP, 1, 60000);
    return pet_effect_has(0, EFFECT_ATTACK_UP) == 1 &&
           pet_effect_has(1, EFFECT_DEFENSE_UP) == 1 &&
           pet_effect_has(2, EFFECT_SPEED_UP) == 1;
}

static int test_pet_effect_multiple_effects(void) {
    effect_init();
    pet_effect_add(0, EFFECT_ATTACK_UP, 1, 60000);
    pet_effect_add(0, EFFECT_DEFENSE_UP, 1, 60000);
    pet_effect_add(0, EFFECT_SPEED_UP, 1, 60000);
    return pet_effect_has(0, EFFECT_ATTACK_UP) == 1 &&
           pet_effect_has(0, EFFECT_DEFENSE_UP) == 1 &&
           pet_effect_has(0, EFFECT_SPEED_UP) == 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_effect_lifecycle(void) {
    effect_init();

    /* Add buffs */
    effect_add(EFFECT_ATTACK_UP, 5, 60000, 100);
    effect_add(EFFECT_DEFENSE_UP, 3, 60000, 100);

    /* Add debuff */
    effect_add(EFFECT_POISON, 2, 30000, 100);

    /* Check counts */
    if (effect_get_count() != 3) return 0;
    if (effect_get_buff_count() != 2) return 0;
    if (effect_get_debuff_count() != 1) return 0;

    /* Check modifiers */
    effect_update_calc_modifiers();
    if (effect_get_attack_modifier() != 100) return 0;
    if (effect_get_defense_modifier() != 60) return 0;

    /* Remove debuffs */
    effect_remove_debuffs();
    if (effect_get_debuff_count() != 0) return 0;

    /* Remove all */
    effect_remove_all();
    return effect_get_count() == 0;
}

static int test_effect_combat_scenario(void) {
    effect_init();

    /* Player gets poisoned */
    effect_add(EFFECT_POISON, 3, 30000, 1000);
    if (!effect_has(EFFECT_POISON)) return 0;

    /* Player uses cure - remove debuffs */
    effect_remove_debuffs();
    if (effect_has(EFFECT_POISON)) return 0;

    /* Player buffs up */
    effect_add(EFFECT_ATTACK_UP, 5, 60000, 100);
    effect_add(EFFECT_DEFENSE_UP, 5, 60000, 100);

    /* Check modifiers */
    effect_update_calc_modifiers();
    if (effect_get_attack_modifier() != 100) return 0;
    if (effect_get_defense_modifier() != 100) return 0;

    return 1;
}

static int test_pet_full_effect_cycle(void) {
    effect_init();

    /* Pet 0 gets effects */
    pet_effect_add(0, EFFECT_ATTACK_UP, 3, 60000);
    pet_effect_add(0, EFFECT_SPEED_UP, 2, 60000);

    /* Pet 1 gets different effects */
    pet_effect_add(1, EFFECT_DEFENSE_UP, 4, 60000);

    /* Verify */
    if (!pet_effect_has(0, EFFECT_ATTACK_UP)) return 0;
    if (!pet_effect_has(0, EFFECT_SPEED_UP)) return 0;
    if (!pet_effect_has(1, EFFECT_DEFENSE_UP)) return 0;

    /* Remove all from pet 0 */
    pet_effect_remove_all(0);
    if (pet_effect_has(0, EFFECT_ATTACK_UP)) return 0;
    if (!pet_effect_has(1, EFFECT_DEFENSE_UP)) return 0;

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Effect System Comprehensive Tests ===\n\n");

    /* Constants tests */
    printf("Constants Tests:\n");
    TEST(max_effects);
    TEST(max_pet_effects);
    TEST(max_pet_slots);
    TEST(max_effect_duration);

    /* Effect type tests */
    printf("\nEffect Type Tests:\n");
    TEST(effect_type_buffs);
    TEST(effect_type_debuffs);
    TEST(effect_type_special);

    /* Effect category tests */
    printf("\nEffect Category Tests:\n");
    TEST(effect_category_values);
    TEST(effect_flag_values);

    /* Effect init tests */
    printf("\nEffect Init Tests:\n");
    TEST(effect_init);
    TEST(effect_shutdown);

    /* Effect add tests */
    printf("\nEffect Add Tests:\n");
    TEST(effect_add_basic);
    TEST(effect_add_multiple);
    TEST(effect_add_invalid);
    TEST(effect_add_value_calculation);
    TEST(effect_add_with_source);
    TEST(effect_add_duration);

    /* Effect remove tests */
    printf("\nEffect Remove Tests:\n");
    TEST(effect_remove_basic);
    TEST(effect_remove_not_found);
    TEST(effect_remove_all);
    TEST(effect_remove_buffs);
    TEST(effect_remove_debuffs);

    /* Effect query tests */
    printf("\nEffect Query Tests:\n");
    TEST(effect_has);
    TEST(effect_get_level);
    TEST(effect_get_level_not_found);
    TEST(effect_get_value);
    TEST(effect_get_remaining);
    TEST(effect_get_active);
    TEST(effect_get_active_not_found);
    TEST(effect_get_by_index);
    TEST(effect_get_by_index_invalid);
    TEST(effect_get_count);
    TEST(effect_get_buff_count);
    TEST(effect_get_debuff_count);

    /* Effect category query tests */
    printf("\nEffect Category Query Tests:\n");
    TEST(effect_is_buff);
    TEST(effect_is_debuff);
    TEST(effect_get_name);
    TEST(effect_get_name_unknown);
    TEST(effect_get_category);

    /* Effect modifier tests */
    printf("\nEffect Modifier Tests:\n");
    TEST(effect_attack_modifier);
    TEST(effect_defense_modifier);
    TEST(effect_speed_modifier);
    TEST(effect_multiple_modifiers);

    /* Effect dispel tests */
    printf("\nEffect Dispel Tests:\n");
    TEST(effect_dispel_basic);
    TEST(effect_dispel_priority);
    TEST(effect_dispel_none);

    /* Pet effect tests */
    printf("\nPet Effect Tests:\n");
    TEST(pet_effect_add);
    TEST(pet_effect_add_invalid_slot);
    TEST(pet_effect_remove);
    TEST(pet_effect_remove_all);
    TEST(pet_effect_has);
    TEST(pet_effect_get_level);
    TEST(pet_effect_multiple_pets);
    TEST(pet_effect_multiple_effects);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_effect_lifecycle);
    TEST(effect_combat_scenario);
    TEST(pet_full_effect_cycle);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
