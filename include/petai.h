/*
 * Stone Age Client - Pet AI System Header
 * Reverse engineered from sa_9061.exe
 * Based on data/AISetting.dat analysis
 */

#ifndef PETAI_H
#define PETAI_H

#include "types.h"
#include "pet.h"
#include "battle.h"

/* Constants */
#define MAX_AI_PRESETS      16
#define AI_PRESET_AGGRESSIVE 0
#define AI_PRESET_DEFENSIVE  1
#define AI_PRESET_BALANCED   2
#define AI_PRESET_SUPPORT    3
#define AI_PRESET_COUNT      4

/* AI modes */
typedef enum {
    AI_MODE_AGGRESSIVE = 0,
    AI_MODE_DEFENSIVE = 1,
    AI_MODE_BALANCED = 2,
    AI_MODE_SUPPORT = 3,
    AI_MODE_MANUAL = 4
} AIMode;

/* AI actions */
typedef enum {
    AI_ACTION_NONE = 0,
    AI_ACTION_ATTACK = 1,
    AI_ACTION_SKILL = 2,
    AI_ACTION_DEFEND = 3,
    AI_ACTION_ITEM = 4,
    AI_ACTION_FLEE = 5,
    AI_ACTION_WAIT = 6
} AIAction;

/* AI target types */
typedef enum {
    AI_TARGET_NONE = 0,
    AI_TARGET_SELF = 1,
    AI_TARGET_ALLY = 2,
    AI_TARGET_ENEMY = 3,
    AI_TARGET_ALL = 4
} AITargetType;

/* Skill types for AI evaluation */
typedef enum {
    SKILL_TYPE_ATTACK = 0,
    SKILL_TYPE_HEAL = 1,
    SKILL_TYPE_BUFF = 2,
    SKILL_TYPE_DEBUFF = 3,
    SKILL_TYPE_SPECIAL = 4
} SkillType;

/* Pet skill structure for AI evaluation */
typedef struct {
    u16 id;
    char name[32];
    SkillType type;
    u16 power;
    u16 mp_cost;
    u8 element;
    u8 cooldown;
    u8 current_cooldown;
    u8 priority;

} AIPetSkill;

/* AI preset */
typedef struct {
    char name[32];
    AIMode mode;
    u8 attack_priority;
    u8 skill_priority;
    u8 defend_threshold;    /* HP% to start defending */
    u8 heal_threshold;      /* HP% to start healing */
    u8 buff_priority;
    u8 reserved[2];

} AIPreset;

/* AI decision result */
typedef struct {
    AIAction action;
    AITargetType target_type;
    u8 target_index;
    u16 skill_id;
    u16 item_id;

} AIDecision;

/* Pet AI context - matches DAT_004d9050 region */
typedef struct {
    /* AI presets */
    AIPreset presets[MAX_AI_PRESETS];
    int preset_count;

    /* Per-pet settings */
    AIMode pet_modes[5];
    u8 pet_presets[5];

    /* Global settings - from DAT_004d9050 region */
    int ai_mode;                /* DAT_004d9050: AI mode (0=off, 3=on) */
    int primary_skill;          /* DAT_004d7ea4: Primary skill slot (0-9) */
    int primary_skill_level;    /* DAT_004d7f30: Primary skill level/type */
    int secondary_skill_level;  /* DAT_004d7f1c: Secondary skill level */
    int tertiary_skill_level;   /* DAT_004d7f18 + 0: Third skill level */
    int skill_level_4;          /* DAT_004d7f1c + 4: Fourth skill level */
    int skill_level_5;          /* DAT_004d7f1c + 8: Fifth skill level */
    int auto_battle_flag;       /* DAT_004d7f54: Auto battle enabled */
    int auto_skill_target;      /* DAT_004d7eac: Auto skill target */
    int auto_skill_type;        /* DAT_004d7f38: Auto skill type */
    int secondary_skill;        /* DAT_004d7eb0: Secondary skill slot */
    int secondary_skill_mode;    /* DAT_004d7f3c: Secondary skill mode */
    int auto_item_target;       /* DAT_004d7eb4: Auto item target */
    int auto_item_type;         /* DAT_004d7f40: Auto item type */

    /* Derived settings */
    AIMode default_mode;
    int auto_skill;
    int auto_item;
    u8 hp_threshold;
    u8 mp_threshold;

    /* Validation flags */
    int has_heal_skill;         /* DAT_004d7f68: Pet has heal skill */
    int heal_skill_slot;        /* DAT_0049e094: Heal skill slot index */
    int settings_valid;         /* DAT_004d7f34: Settings validated flag */

} PetAIContext;

/* Global AI context */
extern PetAIContext g_petai;

/* Initialization - FUN_004017a0, FUN_00401ab0 */
int petai_init(void);
void petai_shutdown(void);

/* Settings - FUN_004017a0 */
int petai_load_settings(const char* filename);
int petai_load_settings_binary(const char* filename);  /* Binary format loader */
void petai_validate_settings(void);  /* FUN_00401ab0 */
void petai_parse_setting(const char* key, const char* value);
void petai_create_default_presets(void);
int petai_save_settings(const char* filename);

/* AI mode */
void petai_set_mode(u8 pet_slot, AIMode mode);
AIMode petai_get_mode(u8 pet_slot);

/* AI preset */
void petai_set_preset(u8 pet_slot, u8 preset_id);
AIPreset* petai_get_preset(u8 pet_slot);
AIPreset* petai_get_preset_by_id(u8 preset_id);
int petai_get_preset_count(void);

/* Custom presets */
int petai_create_preset(const char* name, AIMode mode, int attack_priority,
                         int skill_priority, int defend_threshold, int heal_threshold);
int petai_delete_preset(u8 preset_id);

/* Decision making */
AIDecision petai_decide_action(u8 pet_slot, BattleContext* battle, PetData* pet);
u16 petai_select_skill(PetData* pet, BattleContext* battle, AIPreset* preset);
int petai_evaluate_skill(AIPetSkill* skill, BattleContext* battle, AIPreset* preset);
u8 petai_select_target(BattleContext* battle, AIPreset* preset);
int petai_calculate_threat(BattleUnit* enemy);

/* Helper functions */
int petai_can_heal(PetData* pet);
u16 petai_find_heal_skill(PetData* pet);

/* Auto settings */
void petai_set_auto_skill(int enabled);
void petai_set_auto_item(int enabled);
int petai_is_auto_skill(void);
int petai_is_auto_item(void);

/* Utility */
const char* petai_mode_to_string(AIMode mode);
const char* petai_action_to_string(AIAction action);

/* Element system */
float petai_get_element_advantage(u8 attacker_element, u8 defender_element);

/* Advanced skill evaluation */
int petai_evaluate_skill_advanced(AIPetSkill* skill, BattleContext* battle,
                                   AIPreset* preset, BattleUnit* target);

/* Defensive/Flee decisions */
int petai_should_defend(PetData* pet, BattleContext* battle, AIPreset* preset);
int petai_should_flee(PetData* pet, BattleContext* battle);

/* Combo system */
u16 petai_select_combo_skill(PetData* pet, BattleContext* battle,
                              AIPreset* preset, u8* combo_count);

/* Team AI */
AIDecision petai_decide_team_action(u8 pet_slot, BattleContext* battle,
                                     PetData* pet, int party_size);
u8 petai_find_lowest_hp_ally(BattleContext* battle);

/* Skill finding */
u16 petai_find_buff_skill(PetData* pet);
u16 petai_find_debuff_skill(PetData* pet);

/* Damage estimation */
int petai_estimate_damage(PetData* pet, AIPetSkill* skill, BattleUnit* target);
int petai_can_kill_target(PetData* pet, AIPetSkill* skill, BattleUnit* target);
u16 petai_select_finisher_skill(PetData* pet, BattleContext* battle,
                                  BattleUnit* target);

#endif /* PETAI_H */
