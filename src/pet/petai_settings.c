/*
 * Stone Age Client - Pet AI Settings
 * Split from petai.c for code organization
 *
 * Handles AI settings loading, validation, and preset management
 * Reverse engineered from FUN_00401300, FUN_00401ab0
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "petai.h"
#include "logger.h"

/* XOR decryption key - from DAT_004c10bc region
 * Key is applied backwards (from index 15 down to 0)
 */
static const u8 s_xor_key[16] = {
    0x2b, 0x1c, 0x57, 0x57, 0xbd, 0x14, 0xeb, 0x1c,
    0x46, 0x43, 0x5c, 0x1d, 0x86, 0x17, 0x62, 0x9b
};

/* AI setting entry structure - 0x55 = 85 bytes each */
typedef struct {
    char name[16];              /* XOR encrypted name */
    u32 checksum;               /* Checksum or reserved */
    u32 ai_mode;                /* AI mode setting */
    u32 primary_skill;          /* Primary skill slot */
    u32 reserved1[5];           /* Reserved fields */
    u32 primary_skill_level;    /* Primary skill level/type */
    u32 secondary_skill_level;  /* Secondary skill level */
    u32 tertiary_skill_level;   /* Tertiary skill level */
    u32 reserved2[5];           /* More reserved */
    u32 skill_level_4;
    u32 skill_level_5;
    u8 auto_battle;             /* Auto battle flag */
    u8 padding[16];
} AISettingEntry;

/*
 * XOR decrypt data - FUN_00401300 pattern
 * Key is applied backwards from index 15 to 0
 */
static void petai_xor_decrypt(u8* data, size_t len) {
    size_t i;
    int key_idx = 15;

    for (i = 0; i < len && key_idx >= 0; i++) {
        data[i] ^= s_xor_key[key_idx];
        key_idx--;
        if (key_idx < 0) {
            key_idx = 15;
        }
    }
}

/*
 * Initialize pet AI system - matches FUN_004017a0 pattern
 */
int petai_init(void) {
    memset(&g_petai, 0, sizeof(PetAIContext));

    /* Set default values - from FUN_00401300 initialization */
    g_petai.ai_mode = 0;
    g_petai.primary_skill = 9;
    g_petai.primary_skill_level = 1;
    g_petai.secondary_skill_level = 30;
    g_petai.tertiary_skill_level = 0;
    g_petai.auto_battle_flag = 0;
    g_petai.auto_skill_target = -1;
    g_petai.secondary_skill = 9;
    g_petai.secondary_skill_mode = 2;
    g_petai.auto_item_target = 0x1a;
    g_petai.auto_item_type = 4;
    g_petai.settings_valid = 0;

    g_petai.default_mode = AI_MODE_BALANCED;
    g_petai.auto_skill = 1;
    g_petai.auto_item = 1;

    if (petai_load_settings_binary("data\\AISetting.dat")) {
        petai_validate_settings();
        LOG_INFO("Pet AI system initialized (mode=%d, primary_skill=%d)",
                 g_petai.ai_mode, g_petai.primary_skill);
        return 1;
    }

    if (!petai_load_settings("data\\AISetting.dat")) {
        petai_create_default_presets();
    }

    LOG_INFO("Pet AI system initialized with defaults");
    return 1;
}

/*
 * Shutdown pet AI system
 */
void petai_shutdown(void) {
    int i;

    for (i = 0; i < MAX_AI_PRESETS; i++) {
        if (g_petai.presets[i].name[0] != '\0') {
            memset(&g_petai.presets[i], 0, sizeof(AIPreset));
        }
    }

    memset(&g_petai, 0, sizeof(PetAIContext));
    LOG_INFO("Pet AI system shutdown");
}

/*
 * Load AI settings from binary file - FUN_00401300
 */
int petai_load_settings_binary(const char* filename) {
    FILE* fp;
    u8 name_buffer[16];
    u32 temp_values[5];
    int result = 0;
    long file_size;

    fp = fopen(filename, "rb");
    if (!fp) {
        LOG_DEBUG("AI settings file not found: %s", filename);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size < 85) {
        LOG_WARN("AI settings file too small: %ld bytes", file_size);
        fclose(fp);
        return 0;
    }

    if (fread(name_buffer, 16, 1, fp) == 1) {
        petai_xor_decrypt(name_buffer, 16);

        if (fread(&g_petai.ai_mode, 4, 1, fp) == 1) {
            petai_xor_decrypt((u8*)&g_petai.ai_mode, 4);

            if (fread(&g_petai.primary_skill, 4, 1, fp) == 1) {
                petai_xor_decrypt((u8*)&g_petai.primary_skill, 4);

                if (fread(temp_values, 4, 5, fp) == 5) {
                    petai_xor_decrypt((u8*)temp_values, 20);

                    g_petai.primary_skill_level = temp_values[0];
                    g_petai.secondary_skill_level = temp_values[1];
                    g_petai.tertiary_skill_level = temp_values[2];
                    g_petai.skill_level_4 = temp_values[3];
                    g_petai.skill_level_5 = temp_values[4];

                    if (fread(&g_petai.auto_battle_flag, 1, 1, fp) == 1) {
                        result = 1;
                    }
                }
            }
        }
    }

    fclose(fp);
    return result;
}

/*
 * Validate AI settings - FUN_00401ab0
 */
void petai_validate_settings(void) {
    int* skill_levels[5];
    int i;

    if (g_petai.ai_mode == 0) {
        g_petai.ai_mode = 3;
    }

    skill_levels[0] = &g_petai.secondary_skill_level;
    skill_levels[1] = &g_petai.tertiary_skill_level;
    skill_levels[2] = &g_petai.skill_level_4;
    skill_levels[3] = &g_petai.skill_level_5;
    skill_levels[4] = &g_petai.primary_skill_level;

    for (i = 0; i < 5; i++) {
        int level = *skill_levels[i];
        if (level < 0 || level > 100 || (level % 5) != 0) {
            *skill_levels[i] = 30;
        }
    }

    g_petai.has_heal_skill = 0;
    g_petai.heal_skill_slot = -1;

    if (g_petai.tertiary_skill_level < 0) {
        g_petai.tertiary_skill_level = 0;
    }

    if (g_petai.primary_skill < 0 || g_petai.primary_skill > 9) {
        g_petai.primary_skill = 9;
    }

    if (g_petai.primary_skill_level != 1 &&
        g_petai.primary_skill_level != 2 &&
        (g_petai.primary_skill_level < 7 || g_petai.primary_skill_level > 11)) {
        g_petai.primary_skill_level = 1;
    }

    if (!g_petai.has_heal_skill) {
        g_petai.auto_skill_target = -1;
    }

    g_petai.settings_valid = 1;

    if (g_petai.secondary_skill_mode != 2 &&
        g_petai.secondary_skill_mode != 3 &&
        (g_petai.secondary_skill_mode < 7 || g_petai.secondary_skill_mode > 11)) {
        g_petai.secondary_skill_mode = 2;
    }
}

/*
 * Load AI settings from text file
 */
int petai_load_settings(const char* filename) {
    FILE* fp;
    char line[256];
    char key[64], value[192];

    fp = fopen(filename, "r");
    if (!fp) {
        LOG_WARN("AI settings file not found: %s", filename);
        petai_create_default_presets();
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        if (sscanf(line, "%[^=]=%s", key, value) == 2) {
            petai_parse_setting(key, value);
        }
    }

    fclose(fp);
    return 1;
}

/*
 * Parse AI setting
 */
void petai_parse_setting(const char* key, const char* value) {
    if (strcmp(key, "default_mode") == 0) {
        g_petai.default_mode = (AIMode)atoi(value);
    } else if (strcmp(key, "auto_skill") == 0) {
        g_petai.auto_skill = atoi(value);
    } else if (strcmp(key, "auto_item") == 0) {
        g_petai.auto_item = atoi(value);
    } else if (strcmp(key, "hp_threshold") == 0) {
        g_petai.hp_threshold = (u8)atoi(value);
    } else if (strcmp(key, "mp_threshold") == 0) {
        g_petai.mp_threshold = (u8)atoi(value);
    }
}

/*
 * Create default AI presets
 */
void petai_create_default_presets(void) {
    AIPreset* preset;

    preset = &g_petai.presets[AI_PRESET_AGGRESSIVE];
    strcpy(preset->name, "Aggressive");
    preset->mode = AI_MODE_AGGRESSIVE;
    preset->attack_priority = 80;
    preset->skill_priority = 70;
    preset->defend_threshold = 20;
    preset->heal_threshold = 25;

    preset = &g_petai.presets[AI_PRESET_DEFENSIVE];
    strcpy(preset->name, "Defensive");
    preset->mode = AI_MODE_DEFENSIVE;
    preset->attack_priority = 30;
    preset->skill_priority = 40;
    preset->defend_threshold = 50;
    preset->heal_threshold = 60;

    preset = &g_petai.presets[AI_PRESET_BALANCED];
    strcpy(preset->name, "Balanced");
    preset->mode = AI_MODE_BALANCED;
    preset->attack_priority = 50;
    preset->skill_priority = 50;
    preset->defend_threshold = 35;
    preset->heal_threshold = 40;

    preset = &g_petai.presets[AI_PRESET_SUPPORT];
    strcpy(preset->name, "Support");
    preset->mode = AI_MODE_SUPPORT;
    preset->attack_priority = 20;
    preset->skill_priority = 60;
    preset->defend_threshold = 40;
    preset->heal_threshold = 50;
    preset->buff_priority = 70;

    g_petai.preset_count = 4;
}

/*
 * Save AI settings to file
 */
int petai_save_settings(const char* filename) {
    FILE* fp;

    fp = fopen(filename, "w");
    if (!fp) {
        LOG_ERROR("Failed to save AI settings: %s", filename);
        return 0;
    }

    fprintf(fp, "# Stone Age Pet AI Settings\n");
    fprintf(fp, "default_mode=%d\n", g_petai.default_mode);
    fprintf(fp, "auto_skill=%d\n", g_petai.auto_skill);
    fprintf(fp, "auto_item=%d\n", g_petai.auto_item);
    fprintf(fp, "hp_threshold=%d\n", g_petai.hp_threshold);
    fprintf(fp, "mp_threshold=%d\n", g_petai.mp_threshold);

    fclose(fp);
    return 1;
}

/*
 * Set AI mode for pet
 */
void petai_set_mode(u8 pet_slot, AIMode mode) {
    if (pet_slot >= 5) return;
    g_petai.pet_modes[pet_slot] = mode;
}

/*
 * Get AI mode for pet
 */
AIMode petai_get_mode(u8 pet_slot) {
    if (pet_slot >= 5) return AI_MODE_BALANCED;
    return g_petai.pet_modes[pet_slot];
}

/*
 * Set AI preset for pet
 */
void petai_set_preset(u8 pet_slot, u8 preset_id) {
    if (pet_slot >= 5 || preset_id >= MAX_AI_PRESETS) return;
    g_petai.pet_presets[pet_slot] = preset_id;
}

/*
 * Get AI preset for pet
 */
AIPreset* petai_get_preset(u8 pet_slot) {
    u8 preset_id;

    if (pet_slot >= 5) return NULL;

    preset_id = g_petai.pet_presets[pet_slot];
    if (preset_id >= MAX_AI_PRESETS) return NULL;

    return &g_petai.presets[preset_id];
}

/*
 * Get preset by ID
 */
AIPreset* petai_get_preset_by_id(u8 preset_id) {
    if (preset_id >= MAX_AI_PRESETS) return NULL;
    return &g_petai.presets[preset_id];
}

/*
 * Get preset count
 */
int petai_get_preset_count(void) {
    return g_petai.preset_count;
}

/*
 * Create custom preset
 */
int petai_create_preset(const char* name, AIMode mode, int attack_priority,
                         int skill_priority, int defend_threshold, int heal_threshold) {
    int i;
    AIPreset* preset;

    for (i = 0; i < MAX_AI_PRESETS; i++) {
        if (g_petai.presets[i].name[0] == '\0') {
            preset = &g_petai.presets[i];
            strncpy(preset->name, name, 31);
            preset->name[31] = '\0';
            preset->mode = mode;
            preset->attack_priority = (u8)attack_priority;
            preset->skill_priority = (u8)skill_priority;
            preset->defend_threshold = (u8)defend_threshold;
            preset->heal_threshold = (u8)heal_threshold;

            g_petai.preset_count++;
            return i;
        }
    }

    return -1;
}

/*
 * Delete custom preset
 */
int petai_delete_preset(u8 preset_id) {
    if (preset_id >= MAX_AI_PRESETS) return 0;
    if (preset_id < AI_PRESET_COUNT) return 0;

    memset(&g_petai.presets[preset_id], 0, sizeof(AIPreset));
    g_petai.preset_count--;

    return 1;
}

/*
 * Auto settings control
 */
void petai_set_auto_skill(int enabled) {
    g_petai.auto_skill = enabled;
}

void petai_set_auto_item(int enabled) {
    g_petai.auto_item = enabled;
}

int petai_is_auto_skill(void) {
    return g_petai.auto_skill;
}

int petai_is_auto_item(void) {
    return g_petai.auto_item;
}
