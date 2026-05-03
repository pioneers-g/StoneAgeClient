/*
 * Stone Age Client - Pet AI System Header
 * Reverse engineered from sa_9061.exe
 *
 * FUN_004017a0 - AI settings binary loader
 * FUN_00401ab0 - AI settings validator
 * FUN_0042f130 - Pet skill data parser
 * FUN_00425dc0 - Capture data parser
 */

#ifndef PET_AI_H
#define PET_AI_H

#include "types.h"

/* Constants from binary analysis */
#define MAX_AI_PRESETS          1       /* Only 1 preset used in original */
#define MAX_PRIMARY_SKILLS      10      /* 0x14 bytes / 2 = 10 entries */
#define MAX_BATTLE_SKILLS       20      /* Max 20 skill entries from FUN_0042f130 */

/* AI action types - for AIDecision */
typedef enum {
    AI_ACTION_MANUAL = 0,       /* Player-controlled */
    AI_ACTION_ATTACK = 1,       /* Normal attack */
    AI_ACTION_SKILL = 2,        /* Use skill */
    AI_ACTION_DEFEND = 3,       /* Defend/guard */
    AI_ACTION_ESCAPE = 4,       /* Try to escape */
    AI_ACTION_ITEM = 5,         /* Use item */
    AI_ACTION_WAIT = 6          /* Do nothing */
} AIActionType;

/*
 * AI settings structure - from FUN_004017a0
 * Total size: 69 bytes (0x45)
 * Storage: DAT_004d9050 region
 */
#pragma pack(push, 1)
typedef struct {
    u32 ai_mode;                          /* 0x00: AI mode (0=off, 3=on) */
    s8  primary_skill[20];                /* 0x04: Primary skill slots (0-9) */
    s8  primary_skill_level[20];          /* 0x18: Primary skill levels (0-100) */
    s8  secondary_skill_level[20];        /* 0x2c: Secondary skill levels */
    s32 tertiary_skill_level;             /* 0x40: Tertiary skill level */
    u8  auto_battle_flag;                 /* 0x44: Auto battle enabled */
    u8  padding[3];                       /* Align to 4 bytes */
} PetAISettings;
#pragma pack(pop)

/*
 * Pet battle skill structure - from FUN_0042f130
 * Entry size: 0x418 (1048) bytes in original
 * Stored at DAT_0454ffe0 region
 */
typedef struct {
    u16 id;                    /* Skill ID */
    u16 level;                 /* Skill level */
    u16 mp_cost;               /* MP cost */
    u16 power;                 /* Skill power */
    u16 element;               /* Element type */
    u16 cooldown;              /* Cooldown in turns */
    u16 type;                  /* Skill type */
    u16 target_type;           /* Target type */
    u16 flags;                 /* Skill flags */
    u16 padding;
    char name[256];            /* Skill name */
    char description[256];     /* Skill description */
    char animation[256];       /* Animation data */
} PetBattleSkill;

/*
 * AI decision result - from petai_decide_action
 * Used by battle system to execute pet actions
 */
typedef struct {
    AIActionType action;       /* Action to perform */
    u16 skill_id;              /* Skill ID if AI_ACTION_SKILL */
    u32 target_id;             /* Target entity ID */
    u32 item_id;               /* Item ID if AI_ACTION_ITEM */
    u16 priority;              /* Action priority */
    u16 flags;                 /* Decision flags */
} AIDecision;

/*
 * Initialize pet AI system
 * Loads default AI settings from data/AISetting.dat
 */
int petai_init(void);

/*
 * Shutdown pet AI system
 */
void petai_shutdown(void);

/*
 * Load AI settings from binary file - FUN_004017a0
 * File format:
 *   - Offset 0x10: AI mode (4 bytes)
 *   - Primary skill array (20 bytes)
 *   - Primary skill level array (20 bytes)
 *   - Secondary skill level array (20 bytes)
 *   - Tertiary skill level (4 bytes)
 *   - Auto battle flag (1 byte)
 */
int petai_load_settings(const char* filename);

/*
 * Validate and fix AI settings - FUN_00401ab0
 * Ensures:
 *   - AI mode is 0 or 3
 *   - Skill levels are 0-100 and divisible by 5
 *   - Primary skill IDs are 0-9
 *   - Auto battle flag is 0 or 1
 */
void petai_validate_settings(void);

/*
 * Save AI settings to file
 */
int petai_save_settings(const char* filename);

/*
 * Update AI settings in memory
 */
void petai_update_settings(int primary_skill, int primary_level, int secondary_level);

/*
 * Send AI update to server
 */
void petai_send_update(void);

/*
 * Parse pet skill data from packet - FUN_0042f130
 * Packet format: field3|field4|count|skill1_data|skill2_data|...
 * Each skill_data: id name desc mp power element cooldown type ...
 */
void petai_parse_skill_data(const char* packet_data);

/*
 * Get AI settings for preset
 */
PetAISettings* petai_get_settings(int preset);

/*
 * Check if AI is enabled
 */
int petai_is_enabled(void);

/*
 * Set AI mode
 */
void petai_set_mode(int enabled);

/*
 * Get battle skill by index
 */
PetBattleSkill* petai_get_battle_skill(int index);

/*
 * Get battle skill count
 */
int petai_get_battle_skill_count(void);

/*
 * AI decision engine - decide pet action in battle
 * Returns AIDecision with action type and parameters
 */
AIDecision petai_decide_action(PetData* pet, void* battle_ctx);

/*
 * Helper: Check if pet has heal skill
 */
int petai_has_heal_skill(PetData* pet);

/*
 * Helper: Find heal skill in pet skills
 */
u16 petai_find_heal_skill(PetData* pet);

/*
 * Helper: Find target in battle
 */
u32 petai_find_target(void* battle_ctx, int enemy_side);

/*
 * Helper: Parse field from delimited string
 */
void petai_parse_field(const char* str, char delimiter, int field_num, char* buffer, int buf_size);

/*
 * Helper: Unescape string (convert escape sequences)
 */
void petai_unescape_string(char* str);

#endif /* PET_AI_H */
