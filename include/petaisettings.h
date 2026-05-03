/*
 * Stone Age Client - Pet AI Settings System Header
 * Pet AI configuration loaded from data\AISetting.dat
 * Reverse engineered from sa_9061.exe FUN_00401300
 */

#ifndef PETAISETTINGS_H
#define PETAISETTINGS_H

#include "types.h"

/* Constants */
#define AI_SETTINGS_MAX         20
#define AI_NAME_LENGTH          16
#define AI_SETTINGS_FILE        "data\\AISetting.dat"

/* AI action priorities - 5 sets of 5 values (0x14 bytes each) */
typedef struct {
    u8 attack_priority[5];       /* Attack priority settings */
    u8 skill_priority[5];        /* Skill priority settings */
    u8 defense_priority[5];      /* Defense priority settings */
    u8 support_priority[5];      /* Support priority settings */
    u8 reserved[4];
} AIActionPriorities;

/* Pet AI settings entry - matches DAT_004d7ea4 structure */
typedef struct {
    char name[AI_NAME_LENGTH];   /* AI preset name */
    u32 id;                       /* AI preset ID */
    AIActionPriorities priorities;
    u8 active;                    /* Is this AI active */
    u8 auto_skill;                /* Auto-use skills */
    u8 auto_item;                 /* Auto-use items */
    u8 retreat_hp;                /* Retreat when HP below % */
    u8 reserved[3];
} PetAISettings;

/* AI settings file header */
typedef struct {
    char name[AI_NAME_LENGTH];    /* Character name for these settings */
    u32 checksum;
    u32 entry_count;
    u32 version;
} AISettingsHeader;

/* AI settings context */
typedef struct {
    PetAISettings presets[AI_SETTINGS_MAX];
    u8 valid[AI_SETTINGS_MAX];
    u32 active_preset;
    u8 dirty;
    char character_name[AI_NAME_LENGTH];
    u32 loaded;
    char last_error[64];
} AISettingsContext;

/* Global AI settings context */
extern AISettingsContext g_ai_settings;

/* XOR encryption key - from DAT_004c10bc in sa_9061.exe */
extern const u8 g_ai_xor_key[16];

/* Initialization */
int ai_settings_init(void);
void ai_settings_shutdown(void);

/* Load/Save */
int ai_settings_load(void);
int ai_settings_save(void);

/* Preset management */
int ai_settings_add_preset(PetAISettings* preset);
int ai_settings_update_preset(int index, PetAISettings* preset);
PetAISettings* ai_settings_get_preset(int index);
int ai_settings_find_preset(const char* name);
int ai_settings_delete_preset(int index);

/* Active preset */
void ai_settings_set_active(int index);
PetAISettings* ai_settings_get_active(void);

/* Utility */
void ai_settings_encrypt(void* data, u32 size);
void ai_settings_decrypt(void* data, u32 size);
const char* ai_settings_get_last_error(void);

#endif /* PETAISETTINGS_H */
