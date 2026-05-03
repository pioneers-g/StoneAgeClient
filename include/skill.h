/*
 * Stone Age Client - Skill System Header
 * Reverse engineered from sa_9061.exe
 *
 * Module Split:
 * - skill.h - Main skill system header (types and core functions)
 * - skill_protocol.h - Protocol parsing interface
 * - skill_damage.h - Damage calculation interface
 */

#ifndef SKILL_H
#define SKILL_H

#include "types.h"

/* Constants */
#define MAX_SKILLS          100
#define MAX_SKILL_LEVEL     10
#define MAX_SKILL_NAME      24
#define MAX_SKILL_DESC      128
#define MAX_PLAYER_SKILLS   20
#define MAX_SKILL_TARGETS   40      /* 0x28 from FUN_00425380 */
#define MAX_PET_SKILL_ENTRIES 80    /* 0x50 from FUN_00425bb0 */

/* Skill entry size from FUN_0045ffb0 case 0x53 */
#define SKILL_ENTRY_SIZE    0x60    /* 96 bytes per skill entry */
#define MAX_SKILL_NAME_FIELD 17     /* 0x11 max from case 0x53 */
#define MAX_SKILL_DESC_FIELD 85     /* 0x55 max from case 0x53 */

/* Protocol commands - from FUN_00424b70 analysis */
#define SKILL_CMD_PLAYER        "PS"    /* Player skill use */
#define SKILL_CMD_ENEMY         "PE"    /* Enemy skill (NPC) */
#define SKILL_CMD_PET           "PT"    /* Pet skill use */
#define SKILL_CMD_ACTION        "J"     /* Battle action */
#define SKILL_CMD_LIST          "SL"    /* Request skill list */
#define SKILL_CMD_LEARN         "SLR"   /* Learn skill request */
#define SKILL_CMD_FORGET        "SF"    /* Forget skill */
#define SKILL_CMD_LEVEL_UP      "SLU"   /* Level up skill */
#define SKILL_CMD_SET_SLOT      "SS"    /* Set skill slot (hotkey) */

/* Battle action types - from FUN_00424b70 */
/* Note: These are protocol action codes, different from BattleAction enum in battle.h */
typedef enum {
    SKILL_ACTION_ATTACK = 0,       /* Normal attack - FUN_00425380 */
    SKILL_ACTION_DEFEND = 1,       /* Defend - FUN_00425380 */
    SKILL_ACTION_SKILL_ITEM = 2,   /* Use skill/item - FUN_004253d0 */
    SKILL_ACTION_ESCAPE = 6,       /* Escape - FUN_00425420 */
    SKILL_ACTION_CAPTURE = 7,      /* Capture pet - FUN_004254e0 */
    SKILL_ACTION_CAPTURE2 = 8,     /* Special capture - FUN_004254e0 */
    SKILL_ACTION_SUMMON = 9,       /* Summon pet - FUN_00425bb0 */
    SKILL_ACTION_PET_ATTACK = 10,  /* Pet attack - FUN_00425b50 */
    SKILL_ACTION_PET_ATTACK2 = 11, /* Pet attack variant */
    SKILL_ACTION_WAIT = 12,        /* Pass turn - FUN_004262f0 */
    SKILL_ACTION_FIELD = 13,       /* Field action - FUN_00425dc0 */
    SKILL_ACTION_PET_SKILL = 14,   /* Pet skill - FUN_0042e870 */
    SKILL_ACTION_PET_SKILL2 = 15,  /* Pet skill special - FUN_0042f130 */
    SKILL_ACTION_SPECIAL1 = 18,    /* Special action - FUN_00430700 */
    SKILL_ACTION_STATUS1 = 19,     /* Status action - FUN_00431390 */
    SKILL_ACTION_STATUS2 = 20,     /* Status action 2 */
    SKILL_ACTION_BUFF = 21,        /* Buff action - FUN_00431ad0 */
    SKILL_ACTION_TRANSFORM = 22,   /* Transform - FUN_004327b0 */
    SKILL_ACTION_GOLD_EXP = 24,    /* Gold/Exp gain */
    SKILL_ACTION_ANIMATION = 30,   /* Animation - FUN_00435450 */
    SKILL_ACTION_DIALOG = 31,      /* Dialog - FUN_004364e0 */
    SKILL_ACTION_SPECIAL_TEXT = 32,/* Special text - FUN_00435a00 */
    SKILL_ACTION_TEXT1 = 36,       /* Text display */
    SKILL_ACTION_TEXT2 = 37,       /* Text display variant */
    SKILL_ACTION_TEXT3 = 38,       /* Text display variant */
    SKILL_ACTION_TEXT4 = 39,       /* Text display variant */
    SKILL_ACTION_CAPTURE_RESULT = 42, /* Capture result - FUN_0042af40 */
    SKILL_ACTION_TRANSFORM2 = 43,  /* Transform variant - FUN_00432ec0 */
    SKILL_ACTION_TRANSFORM3 = 44,  /* Transform variant 2 */
    SKILL_ACTION_COMBO = 45,       /* Combo attack - FUN_00425380 */
    SKILL_ACTION_SPECIAL = 101     /* Special skill action */
} SkillActionType;

/* Skill types */
typedef enum {
    SKILL_TYPE_ATTACK = 0,      /* Attack skill */
    SKILL_TYPE_HEAL = 1,        /* Healing skill */
    SKILL_TYPE_BUFF = 2,        /* Buff skill */
    SKILL_TYPE_DEBUFF = 3,      /* Debuff skill */
    SKILL_TYPE_PASSIVE = 4,     /* Passive skill */
    SKILL_TYPE_SPECIAL = 5,     /* Special skill */
    SKILL_TYPE_SUMMON = 6       /* Summon skill */
} SkillType;

/* Skill targets */
typedef enum {
    SKILL_TARGET_SINGLE = 0,    /* Single target */
    SKILL_TARGET_ALL_ENEMY = 1, /* All enemies */
    SKILL_TARGET_ALL_ALLY = 2,  /* All allies */
    SKILL_TARGET_SELF = 3,      /* Self only */
    SKILL_TARGET_RANDOM = 4,    /* Random target */
    SKILL_TARGET_ROW = 5        /* Entire row */
} SkillTargetType;

/* Skill element */
typedef enum {
    SKILL_ELEM_NONE = 0,
    SKILL_ELEM_FIRE = 1,
    SKILL_ELEM_WATER = 2,
    SKILL_ELEM_EARTH = 3,
    SKILL_ELEM_WIND = 4,
    SKILL_ELEM_LIGHT = 5,
    SKILL_ELEM_DARK = 6
} SkillElement;

/* Skill flags */
typedef enum {
    SKILL_FLAG_NONE = 0,
    SKILL_FLAG_COMBO = (1 << 0),
    SKILL_FLAG_COUNTER = (1 << 1),
    SKILL_FLAG_CHARGE = (1 << 2),
    SKILL_FLAG_INSTANT = (1 << 3)
} SkillFlags;

/* Skill data (static) */
typedef struct {
    u32 id;
    char name[MAX_SKILL_NAME];
    char desc[MAX_SKILL_DESC];
    SkillType type;
    SkillTargetType target_type;
    SkillElement element;
    u16 base_power;
    u16 mp_cost;
    u16 range;
    u8 max_level;
    u8 flags;
    u16 animation_id;
    u16 sound_id;
    u32 requirement_skill;
    u8 requirement_level;
} SkillData;

/*
 * Skill entry from text protocol - FUN_0045ffb0 case 0x53
 * Entry size: 0x60 (96) bytes
 * Base address: DAT_04624042
 * Fields parsed from pipe-delimited text (9 fields per skill)
 */
#pragma pack(push, 1)
typedef struct {
    u16 skill_id;               /* Offset 0x00: Field 1 - Skill ID */
    u16 level;                  /* Offset 0x02: Field 2 - Skill level */
    u16 type;                   /* Offset 0x04: Field 3 - Skill type */
    u16 target_type;            /* Offset 0x06: Field 4 - Target type */
    u16 padding_08;
    u16 padding_0a;
    u16 padding_0c;
    u16 padding_0e;

    u32 field_10;               /* Offset 0x10: Field 5 - Additional data */
    u32 field_14;               /* Offset 0x14: Field 6 - Additional data */
    u32 field_18;               /* Offset 0x18: Field 7 - Additional data */
    u32 padding_1c;

    char name[17];              /* Offset 0x20: Field 8 - Name (max 0x11) */
    char padding_31[15];        /* Pad to 0x40 */

    char description[85];       /* Offset 0x40: Field 9 - Description (max 0x55) */
    char padding_95[11];        /* Pad to 0x60 */
} SkillListEntry;
#pragma pack(pop)

/* Global skill list from server */
extern SkillListEntry g_skill_list[MAX_PLAYER_SKILLS];
extern int g_skill_list_count;

/* Learned skill */
typedef struct {
    u32 skill_id;
    u8 level;
    u8 slot;
    u16 exp;
    u16 exp_next;
    u32 last_used;
} LearnedSkill;

/* Skill effect result */
typedef struct {
    u32 target_id;
    s32 damage;
    s32 heal;
    u8 critical;
    u8 miss;
    u8 status_effect;
    u8 status_duration;
} SkillEffect;

/* Skill context */
typedef struct {
    /* Learned skills */
    LearnedSkill skills[MAX_PLAYER_SKILLS];
    int skill_count;

    /* Skill slots (hotkeys) */
    u32 skill_slots[10];
    int slot_count;

    /* Available skill points */
    u16 skill_points;
    u16 total_points_used;

    /* Current casting */
    u32 casting_skill;
    u32 casting_target;
    u32 cast_start_time;
    u16 cast_time;
    u8 is_casting;

    /* Cooldowns */
    u32 cooldowns[MAX_PLAYER_SKILLS];

    /* Skill database */
    SkillData* skill_db;
    int skill_db_count;

    /* Flags */
    u8 dirty;
    u8 loaded;

} SkillContext;

/* Global skill context */
extern SkillContext g_skill;

/* Initialization */
int skill_init(void);
void skill_shutdown(void);

/* Skill database */
int skill_load_database(void);
SkillData* skill_get_data(u32 skill_id);
SkillData* skill_get_by_name(const char* name);

/* Learning */
int skill_learn(u32 skill_id);
int skill_forget(u32 skill_id);
int skill_has_skill(u32 skill_id);
int skill_can_learn(u32 skill_id);
int skill_get_level(u32 skill_id);
int skill_set_level(u32 skill_id, u8 level);

/* Leveling */
int skill_level_up(u32 skill_id);
int skill_gain_exp(u32 skill_id, u16 exp);
int skill_get_exp_for_level(u8 level);

/* Points */
void skill_set_points(u16 points);
u16 skill_get_points(void);
int skill_use_point(u32 skill_id);

/* Slots */
int skill_set_slot(int slot, u32 skill_id);
u32 skill_get_slot(int slot);
int skill_clear_slot(int slot);
void skill_swap_slots(int slot1, int slot2);

/* Casting */
int skill_start_cast(u32 skill_id, u32 target_id);
void skill_update_cast(u32 current_time);
int skill_is_casting(void);
void skill_cancel_cast(void);
int skill_finish_cast(void);
int skill_get_cast_progress(void);

/* Cooldowns */
void skill_start_cooldown(u32 skill_id);
int skill_is_on_cooldown(u32 skill_id);
u32 skill_get_cooldown_remaining(u32 skill_id);

/* Usage */
int skill_use(u32 skill_id, u32 target_id);
int skill_use_slot(int slot, u32 target_id);
void skill_handle_result(void* data, u32 size);
void skill_handle_effect(void* data, u32 size);

/* Packet handlers */
void skill_handle_list(void* data, u32 size);
void skill_handle_learn_result(void* data, u32 size);
void skill_handle_level_up(void* data, u32 size);

/* Text protocol parser - FUN_0045ffb0 case 0x53 */
void skill_parse_list_text(const char* packet_data);

/* Query */
LearnedSkill* skill_get_learned(u32 skill_id);
LearnedSkill* skill_get_learned_by_index(int index);
int skill_get_count(void);
int skill_get_mp_cost(u32 skill_id);

/* Pet skills */
int pet_skill_use(u32 pet_slot, u32 skill_id, u32 target_id);
int pet_skill_get_count(u32 pet_slot);
u32 pet_skill_get(u32 pet_slot, int skill_index);

/* Network requests */
void skill_request_list(void);
int skill_request_learn(u32 skill_id);
int skill_request_forget(u32 skill_id);
int skill_request_level_up(u32 skill_id);
int skill_set_slot_network(int slot, u32 skill_id);

/* Utility */
const char* skill_get_name(u32 skill_id);
const char* skill_type_to_string(SkillType type);
u32 skill_calculate_damage(u32 skill_id, u32 attacker_id, u32 target_id);
int skill_is_valid_target(u32 skill_id, u32 caster_id, u32 target_id);

/* Damage calculation */
typedef struct {
    u32 attacker_id;
    u32 target_id;
    u32 skill_id;
    u8 skill_level;
    u16 attacker_atk;
    u16 attacker_matk;
    u16 target_def;
    u16 target_mdef;
    u8 attacker_element;
    u8 target_element;
    u8 attacker_level;
    u8 target_level;
} SkillDamageParams;

typedef struct {
    s32 base_damage;
    s32 final_damage;
    s32 heal_amount;
    float element_multiplier;
    u8 is_critical;
    u8 is_miss;
    u8 is_blocked;
    u8 status_effect;
    u8 status_duration;
} SkillDamageResult;

SkillDamageResult skill_calculate_damage_ex(SkillDamageParams* params);
u32 skill_calculate_heal(u32 skill_id, u8 skill_level, u16 matk);
float skill_get_element_multiplier(u8 attack_elem, u8 defend_elem);
int skill_roll_critical(u16 luck, u8 skill_level);
int skill_roll_hit(u16 attacker_hit, u16 target_dodge);

/* Status effects */
typedef enum {
    STATUS_NONE = 0,
    STATUS_POISON = 1,
    STATUS_PARALYZE = 2,
    STATUS_SLEEP = 3,
    STATUS_STUN = 4,
    STATUS_BLIND = 5,
    STATUS_SILENCE = 6,
    STATUS_CONFUSE = 7,
    STATUS_FEAR = 8,
    STATUS_FREEZE = 9,
    STATUS_BURN = 10,
    STATUS_SLOW = 11,
    STATUS_HASTE = 12,
    STATUS_SHIELD = 13,
    STATUS_REFLECT = 14,
    STATUS_INVISIBLE = 15,
    STATUS_STONE = 16,
    STATUS_CURSE = 17,
    STATUS_BLEED = 18
} StatusEffect;

typedef struct {
    StatusEffect effect;
    u8 duration;
    u8 power;
    u8 source_id;
} ActiveStatus;

/* Buff/Debuff system */
typedef enum {
    BUFF_TYPE_ATTACK_UP = 1,
    BUFF_TYPE_DEFENSE_UP = 2,
    BUFF_TYPE_SPEED_UP = 3,
    BUFF_TYPE_MAGIC_UP = 4,
    BUFF_TYPE_ATTACK_DOWN = 5,
    BUFF_TYPE_DEFENSE_DOWN = 6,
    BUFF_TYPE_SPEED_DOWN = 7,
    BUFF_TYPE_MAGIC_DOWN = 8,
    BUFF_TYPE_REGEN = 9,
    BUFF_TYPE_RESTORE = 10
} BuffType;

typedef struct {
    BuffType type;
    u8 duration;
    u16 power;
    u8 stack_count;
} ActiveBuff;

void skill_apply_status(u32 target_id, StatusEffect status, u8 duration, u8 power);
void skill_apply_buff(u32 target_id, BuffType buff, u8 duration, u16 power);
void skill_remove_status(u32 target_id, StatusEffect status);
void skill_remove_buff(u32 target_id, BuffType buff);
int skill_has_status(u32 target_id, StatusEffect status);
int skill_has_buff(u32 target_id, BuffType buff);

/* Effect parsing */
void skill_parse_effect_string(const char* effect_str, SkillEffect* effects, int* count);
void skill_apply_effect(SkillEffect* effect);

#endif /* SKILL_H */
