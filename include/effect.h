/*
 * Stone Age Client - Status Effect System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef EFFECT_H
#define EFFECT_H

#include "types.h"

/* Constants */
#define MAX_EFFECTS         20
#define MAX_PET_EFFECTS     8       /* Max effects per pet */
#define MAX_PET_SLOTS       5       /* Max pets */
#define MAX_EFFECT_DURATION 300000  /* 5 minutes max */

/* Effect types */
typedef enum {
    EFFECT_NONE = 0,

    /* Positive effects (buffs) */
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

    /* Negative effects (debuffs) */
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

    /* Special effects */
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
    EFFECT_FLAG_TICK = (1 << 4)      /* Has periodic effect */
} EffectFlags;

/* Active effect */
typedef struct {
    u16 effect_id;
    u16 level;
    s32 value;              /* Effect power/value */
    u32 duration;           /* Total duration in ms */
    u32 remaining;          /* Remaining time in ms */
    u32 tick_interval;      /* For periodic effects */
    u32 last_tick;          /* Last tick timestamp */
    u32 source_id;          /* Who applied the effect */
    u16 flags;
    u8  category;
    u8  reserved;
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
    /* Active effects on player */
    ActiveEffect effects[MAX_EFFECTS];
    int effect_count;

    /* Pet effects - [pet_slot][effect_slot] */
    ActiveEffect pet_effects[MAX_PET_SLOTS][MAX_PET_EFFECTS];

    /* Effect database */
    EffectData* effect_db;
    int db_count;

    /* UI state */
    u8 show_buffs;
    u8 show_debuffs;
    u8 effect_update;

    /* Summary stats */
    s32 attack_mod;
    s32 defense_mod;
    s32 speed_mod;
    s32 magic_mod;

} EffectContext;

/* Global effect context */
extern EffectContext g_effect;

/* Initialization */
int effect_init(void);
void effect_shutdown(void);

/* Effect database */
int effect_load_database(void);
EffectData* effect_get_data(u16 effect_id);

/* Add/Remove effects */
int effect_add(u16 effect_id, u16 level, u32 duration, u32 source_id);
int effect_remove(u16 effect_id);
void effect_remove_all(void);
void effect_remove_by_category(EffectCategory category);
void effect_remove_buffs(void);
void effect_remove_debuffs(void);
int effect_dispel(u8 priority);  /* Remove lowest priority effects */

/* Effect queries */
int effect_has(u16 effect_id);
int effect_get_level(u16 effect_id);
s32 effect_get_value(u16 effect_id);
u32 effect_get_remaining(u16 effect_id);
ActiveEffect* effect_get_active(u16 effect_id);
ActiveEffect* effect_get_by_index(int index);
int effect_get_count(void);
int effect_get_buff_count(void);
int effect_get_debuff_count(void);

/* Update */
void effect_update(u32 delta_time);
void effect_tick(ActiveEffect* effect);

/* Calculations */
s32 effect_get_attack_modifier(void);
s32 effect_get_defense_modifier(void);
s32 effect_get_speed_modifier(void);
s32 effect_get_magic_modifier(void);
int effect_can_act(void);
int effect_can_cast(void);
int effect_can_move(void);
int effect_is_stunned(void);
int effect_is_immobilized(void);

/* Packet handlers */
void effect_handle_add(void* data, u32 size);
void effect_handle_remove(void* data, u32 size);
void effect_handle_list(void* data, u32 size);

/* Utility */
const char* effect_get_name(u16 effect_id);
EffectCategory effect_get_category(u16 effect_id);
int effect_is_buff(u16 effect_id);
int effect_is_debuff(u16 effect_id);
u16 effect_get_icon(u16 effect_id);

/* Pet effects */
int pet_effect_add(u32 pet_slot, u16 effect_id, u16 level, u32 duration);
int pet_effect_remove(u32 pet_slot, u16 effect_id);
void pet_effect_remove_all(u32 pet_slot);
int pet_effect_has(u32 pet_slot, u16 effect_id);
int pet_effect_get_level(u32 pet_slot, u16 effect_id);
void pet_effect_update(u32 pet_slot, u32 delta_time);
int pet_effect_can_act(u32 pet_slot);
int pet_effect_can_cast(u32 pet_slot);

/* Extended queries */
int effect_is_silenced(void);
int effect_is_blind(void);
int effect_is_confused(void);
int effect_is_asleep(void);
int effect_has_immunity(void);
int effect_can_dispel(u16 effect_id);

/* Protocol handlers */
void effect_handle_pet_add(void* data, u32 size);
void effect_handle_pet_remove(void* data, u32 size);

#endif /* EFFECT_H */
