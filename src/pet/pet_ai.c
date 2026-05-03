/*
 * Stone Age Client - Pet AI System Implementation
 * Reverse engineered from sa_9061.exe
 *
 * FUN_004017a0 - AI settings binary loader
 * FUN_00401ab0 - AI settings validator
 * FUN_0042f130 - Pet skill data parser
 * FUN_00425dc0 - Capture data parser
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "pet.h"
#include "pet_ai.h"
#include "battle.h"
#include "network.h"
#include "logger.h"

/* AI settings data region - DAT_004d9050 area */
static PetAISettings g_ai_settings[MAX_AI_PRESETS];
static int g_ai_settings_loaded = 0;

/* Pet battle skill data - DAT_0454ffe0 region
 * Entry size: 0x418 (1048) bytes
 * Max 20 entries
 */
static PetBattleSkill g_pet_battle_skills[20];
static int g_pet_battle_skill_count = 0;

/* External functions from file I/O */
extern int file_open_read(const char* filename);
extern int file_read(int handle, void* buffer, u32 size);
extern void file_close(int handle);

/* External network function */
extern int network_send(const void* data, size_t len);

/*
 * Initialize pet AI system
 */
int petai_init(void) {
    memset(g_ai_settings, 0, sizeof(g_ai_settings));
    memset(g_pet_battle_skills, 0, sizeof(g_pet_battle_skills));
    g_ai_settings_loaded = 0;
    g_pet_battle_skill_count = 0;

    /* Load default AI settings */
    petai_load_settings("data/AISetting.dat");

    LOG_INFO("Pet AI system initialized");
    return 1;
}

/*
 * Shutdown pet AI system
 */
void petai_shutdown(void) {
    memset(g_ai_settings, 0, sizeof(g_ai_settings));
    memset(g_pet_battle_skills, 0, sizeof(g_pet_battle_skills));
    g_ai_settings_loaded = 0;
    g_pet_battle_skill_count = 0;

    LOG_INFO("Pet AI system shutdown");
}

/*
 * Load AI settings from binary file - FUN_004017a0
 * File: data/AISetting.dat
 */
int petai_load_settings(const char* filename) {
    int handle;
    u32 offset;
    int i;
    PetAISettings* settings;

    handle = file_open_read(filename);
    if (handle < 0) {
        LOG_WARN("Failed to open AI settings file: %s", filename);
        return 0;
    }

    /* Seek to offset: DAT_004d7f60 * 0x55 + 0x10
     * DAT_004d7f60 = 0 (file offset multiplier)
     */
    offset = 0x10;  /* Base offset */

    /* Read AI mode (4 bytes) - DAT_004d9050 */
    file_read(handle, &g_ai_settings[0].ai_mode, 4);

    /* Read primary skill array (0x14 = 20 bytes) - DAT_004d7ea4 */
    file_read(handle, g_ai_settings[0].primary_skill, 20);

    /* Read primary skill level array (0x14 = 20 bytes) - DAT_004d7f30 */
    file_read(handle, g_ai_settings[0].primary_skill_level, 20);

    /* Read secondary skill level array (0x14 = 20 bytes) - DAT_004d7f1c */
    file_read(handle, g_ai_settings[0].secondary_skill_level, 20);

    /* Read tertiary skill level (4 bytes) - DAT_004d7f18 */
    file_read(handle, &g_ai_settings[0].tertiary_skill_level, 4);

    /* Read auto battle flag (1 byte) - DAT_004d7f54 */
    file_read(handle, &g_ai_settings[0].auto_battle_flag, 1);

    file_close(handle);

    /* Validate settings */
    petai_validate_settings();

    g_ai_settings_loaded = 1;
    LOG_INFO("AI settings loaded from %s", filename);

    return 1;
}

/*
 * Validate and fix AI settings - FUN_00401ab0
 */
void petai_validate_settings(void) {
    PetAISettings* settings = &g_ai_settings[0];
    int i;

    /* Validate AI mode: must be 0 or 3 */
    if (settings->ai_mode != 0 && settings->ai_mode != 3) {
        settings->ai_mode = 0;  /* Default to manual */
    }

    /* Validate primary skill levels: must be 0-100 and divisible by 5 */
    for (i = 0; i < 10; i++) {
        if (settings->primary_skill_level[i] < 0 ||
            settings->primary_skill_level[i] > 100 ||
            settings->primary_skill_level[i] % 5 != 0) {
            settings->primary_skill_level[i] = 30;  /* Default value */
        }
    }

    /* Validate secondary skill levels */
    for (i = 0; i < 10; i++) {
        if (settings->secondary_skill_level[i] < 0 ||
            settings->secondary_skill_level[i] > 100 ||
            settings->secondary_skill_level[i] % 5 != 0) {
            settings->secondary_skill_level[i] = 30;
        }
    }

    /* Validate tertiary skill level */
    if (settings->tertiary_skill_level < 0 ||
        settings->tertiary_skill_level > 100 ||
        settings->tertiary_skill_level % 5 != 0) {
        settings->tertiary_skill_level = 30;
    }

    /* Validate primary skill: must be 0-9 */
    for (i = 0; i < 10; i++) {
        if (settings->primary_skill[i] < 0 || settings->primary_skill[i] > 9) {
            settings->primary_skill[i] = 0;
        }
    }

    /* Validate auto battle flag */
    if (settings->auto_battle_flag > 1) {
        settings->auto_battle_flag = 0;
    }

    LOG_DEBUG("AI settings validated");
}

/*
 * Parse pet skill data from packet - FUN_0042f130
 * Packet format: field3|field4|count|skill1_data|skill2_data|...
 *
 * Each skill_data has 9 space-separated fields:
 * skill_id name description mp power element cooldown type ...
 */
void petai_parse_skill_data(const char* packet_data) {
    const char* ptr = packet_data;
    char field_buf[256];
    char skill_buf[512];
    char name_buf[512];
    char desc_buf[512];
    int i;
    PetBattleSkill* skill;

    /* Clear existing skill data - DAT_0454ffe0 region */
    memset(g_pet_battle_skills, 0, sizeof(g_pet_battle_skills));
    g_pet_battle_skill_count = 0;

    /* Parse field 3: DAT_0455efd0 */
    petai_parse_field(ptr, '|', 3, field_buf, sizeof(field_buf));
    /* Value stored but not used in original */

    /* Parse field 4: DAT_0455efcc */
    petai_parse_field(ptr, '|', 4, field_buf, sizeof(field_buf));
    /* Value stored but not used in original */

    /* Parse field 5: skill count - DAT_0455efd4 */
    petai_parse_field(ptr, '|', 5, field_buf, sizeof(field_buf));
    g_pet_battle_skill_count = atoi(field_buf);

    /* Limit to max 20 skills */
    if (g_pet_battle_skill_count > 20) {
        g_pet_battle_skill_count = 20;
    }

    /* Parse each skill entry */
    for (i = 0; i < g_pet_battle_skill_count; i++) {
        skill = &g_pet_battle_skills[i];

        /* Get skill data field (field 6 + i) */
        petai_parse_field(ptr, '|', 6 + i, skill_buf, sizeof(skill_buf));

        /* Parse skill sub-fields (space-separated) */
        /* Field 1: skill_id */
        petai_parse_field(skill_buf, ' ', 1, field_buf, sizeof(field_buf));
        skill->id = (u16)atoi(field_buf);

        /* Field 2: name */
        petai_parse_field(skill_buf, ' ', 2, name_buf, sizeof(name_buf));
        petai_unescape_string(name_buf);
        strncpy(skill->name, name_buf, sizeof(skill->name) - 1);

        /* Field 3: description */
        petai_parse_field(skill_buf, ' ', 3, desc_buf, sizeof(desc_buf));
        petai_unescape_string(desc_buf);
        strncpy(skill->description, desc_buf, sizeof(skill->description) - 1);

        /* Field 5: mp_cost */
        petai_parse_field(skill_buf, ' ', 5, field_buf, sizeof(field_buf));
        skill->mp_cost = (u16)atoi(field_buf);

        /* Field 4: power */
        petai_parse_field(skill_buf, ' ', 4, field_buf, sizeof(field_buf));
        skill->power = (u16)atoi(field_buf);

        /* Field 6: element */
        petai_parse_field(skill_buf, ' ', 6, field_buf, sizeof(field_buf));
        skill->element = (u16)atoi(field_buf);

        /* Field 7: cooldown */
        petai_parse_field(skill_buf, ' ', 7, field_buf, sizeof(field_buf));
        skill->cooldown = (u16)atoi(field_buf);

        /* Field 8: skill_type */
        petai_parse_field(skill_buf, ' ', 8, field_buf, sizeof(field_buf));
        skill->type = (u16)atoi(field_buf);
    }

    LOG_DEBUG("Parsed %d pet battle skills", g_pet_battle_skill_count);
}

/*
 * Get AI settings for preset
 */
PetAISettings* petai_get_settings(int preset) {
    if (preset < 0 || preset >= MAX_AI_PRESETS) {
        return NULL;
    }
    return &g_ai_settings[preset];
}

/*
 * Check if AI is enabled
 */
int petai_is_enabled(void) {
    return g_ai_settings[0].ai_mode == 3;
}

/*
 * Set AI mode
 */
void petai_set_mode(int enabled) {
    g_ai_settings[0].ai_mode = enabled ? 3 : 0;
}

/*
 * Get battle skill by index
 */
PetBattleSkill* petai_get_battle_skill(int index) {
    if (index < 0 || index >= g_pet_battle_skill_count) {
        return NULL;
    }
    return &g_pet_battle_skills[index];
}

/*
 * Get battle skill count
 */
int petai_get_battle_skill_count(void) {
    return g_pet_battle_skill_count;
}

/*
 * AI decision engine - decide pet action in battle
 */
AIDecision petai_decide_action(PetData* pet, void* battle_ctx) {
    AIDecision decision = {0};
    PetAISettings* settings;
    int hp_percent;
    int mp_percent;

    if (!pet || !petai_is_enabled()) {
        decision.action = AI_ACTION_MANUAL;
        return decision;
    }

    settings = &g_ai_settings[0];

    /* Calculate HP and MP percentages */
    hp_percent = (pet->hp * 100) / pet->max_hp;
    mp_percent = (pet->mp * 100) / pet->max_mp;

    /* Decision priority based on AI settings */

    /* Priority 1: Heal if HP is low and we have heal skill */
    if (hp_percent < 30 && petai_has_heal_skill(pet)) {
        decision.action = AI_ACTION_SKILL;
        decision.skill_id = petai_find_heal_skill(pet);
        decision.target_id = 0;  /* Self */
        return decision;
    }

    /* Priority 2: Use skill if MP is sufficient and skill priority is set */
    if (mp_percent >= 30 && settings->primary_skill[0] > 0) {
        int skill_slot = settings->primary_skill[0] - 1;
        if (skill_slot >= 0 && skill_slot < pet->skill_count) {
            decision.action = AI_ACTION_SKILL;
            decision.skill_id = pet->skills[skill_slot].skill_id;
            decision.target_id = petai_find_target(battle_ctx, 1);  /* Find enemy target */
            return decision;
        }
    }

    /* Priority 3: Normal attack */
    decision.action = AI_ACTION_ATTACK;
    decision.target_id = petai_find_target(battle_ctx, 1);

    return decision;
}

/*
 * Check if pet has heal skill
 */
int petai_has_heal_skill(PetData* pet) {
    int i;

    for (i = 0; i < pet->skill_count; i++) {
        if (pet->skills[i].skill_id > 0) {
            /* Check if skill is heal type (would need skill database) */
            /* For now, assume skill ID 1 is heal */
            if (pet->skills[i].skill_id == 1) {
                return 1;
            }
        }
    }

    return 0;
}

/*
 * Find heal skill in pet skills
 */
u16 petai_find_heal_skill(PetData* pet) {
    int i;

    for (i = 0; i < pet->skill_count; i++) {
        if (pet->skills[i].skill_id == 1) {  /* Heal skill ID */
            return pet->skills[i].skill_id;
        }
    }

    return 0;
}

/*
 * Find target in battle
 */
u32 petai_find_target(void* battle_ctx, int enemy_side) {
    /* This would integrate with battle system */
    /* For now, return first enemy target */
    return 10;  /* Enemy side starts at index 10 */
}

/*
 * Parse field from delimited string - helper
 */
void petai_parse_field(const char* str, char delimiter, int field_num, char* buffer, int buf_size) {
    const char* ptr = str;
    const char* start;
    int current_field = 1;
    int len;

    buffer[0] = '\0';

    /* Skip to desired field */
    while (*ptr && current_field < field_num) {
        if (*ptr == delimiter) {
            current_field++;
        }
        ptr++;
    }

    if (current_field != field_num) {
        return;
    }

    start = ptr;

    /* Find end of field */
    while (*ptr && *ptr != delimiter) {
        ptr++;
    }

    /* Copy field to buffer */
    len = ptr - start;
    if (len >= buf_size) {
        len = buf_size - 1;
    }

    memcpy(buffer, start, len);
    buffer[len] = '\0';
}

/*
 * Unescape string (convert escape sequences)
 */
void petai_unescape_string(char* str) {
    char* src = str;
    char* dst = str;

    while (*src) {
        if (*src == '\\' && src[1]) {
            src++;
            switch (*src) {
                case 'n': *dst++ = '\n'; break;
                case 't': *dst++ = '\t'; break;
                case 'r': *dst++ = '\r'; break;
                case '\\': *dst++ = '\\'; break;
                case '|': *dst++ = '|'; break;
                default: *dst++ = *src; break;
            }
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

/*
 * Save AI settings to file
 */
int petai_save_settings(const char* filename) {
    FILE* fp;
    PetAISettings* settings = &g_ai_settings[0];

    fp = fopen(filename, "wb");
    if (!fp) {
        LOG_ERROR("Failed to open AI settings file for writing: %s", filename);
        return 0;
    }

    /* Write AI mode */
    fwrite(&settings->ai_mode, 4, 1, fp);

    /* Write primary skill array */
    fwrite(settings->primary_skill, 20, 1, fp);

    /* Write primary skill level array */
    fwrite(settings->primary_skill_level, 20, 1, fp);

    /* Write secondary skill level array */
    fwrite(settings->secondary_skill_level, 20, 1, fp);

    /* Write tertiary skill level */
    fwrite(&settings->tertiary_skill_level, 4, 1, fp);

    /* Write auto battle flag */
    fwrite(&settings->auto_battle_flag, 1, 1, fp);

    fclose(fp);

    LOG_INFO("AI settings saved to %s", filename);
    return 1;
}

/*
 * Update AI settings in memory
 */
void petai_update_settings(int primary_skill, int primary_level, int secondary_level) {
    PetAISettings* settings = &g_ai_settings[0];

    if (primary_skill >= 0 && primary_skill <= 9) {
        settings->primary_skill[0] = primary_skill;
    }

    if (primary_level >= 0 && primary_level <= 100 && primary_level % 5 == 0) {
        settings->primary_skill_level[0] = primary_level;
    }

    if (secondary_level >= 0 && secondary_level <= 100 && secondary_level % 5 == 0) {
        settings->secondary_skill_level[0] = secondary_level;
    }
}

/*
 * Send AI update to server
 */
void petai_send_update(void) {
    char packet[128];
    PetAISettings* settings = &g_ai_settings[0];

    _snprintf(packet, sizeof(packet), "AI|%d|%d|%d|%d",
              settings->ai_mode,
              settings->primary_skill[0],
              settings->primary_skill_level[0],
              settings->secondary_skill_level[0]);

    network_send(packet, strlen(packet));
    network_send("\n", 1);

    LOG_DEBUG("Sent AI update: %s", packet);
}
