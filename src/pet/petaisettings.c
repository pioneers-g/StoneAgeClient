/*
 * Stone Age Client - Pet AI Settings System Implementation
 * Pet AI configuration loaded from data\AISetting.dat
 * Reverse engineered from sa_9061.exe FUN_00401300
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "petaisettings.h"
#include "logger.h"

/* Global AI settings context */
AISettingsContext g_ai_settings = {0};

/* XOR encryption key from DAT_004c10bc in sa_9061.exe */
const u8 g_ai_xor_key[16] = {
    0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
    0x21, 0x43, 0x65, 0x87, 0xa9, 0xcb, 0xed, 0x0f
};

/* Default AI presets */
static const PetAISettings s_default_presets[] = {
    {
        .name = "Balanced",
        .id = 0,
        .priorities = {
            .attack_priority = {50, 50, 50, 50, 50},
            .skill_priority = {30, 30, 30, 30, 30},
            .defense_priority = {40, 40, 40, 40, 40},
            .support_priority = {20, 20, 20, 20, 20}
        },
        .active = 1,
        .auto_skill = 1,
        .auto_item = 0,
        .retreat_hp = 20
    },
    {
        .name = "Aggressive",
        .id = 1,
        .priorities = {
            .attack_priority = {80, 80, 80, 80, 80},
            .skill_priority = {60, 60, 60, 60, 60},
            .defense_priority = {20, 20, 20, 20, 20},
            .support_priority = {10, 10, 10, 10, 10}
        },
        .active = 0,
        .auto_skill = 1,
        .auto_item = 0,
        .retreat_hp = 10
    },
    {
        .name = "Defensive",
        .id = 2,
        .priorities = {
            .attack_priority = {30, 30, 30, 30, 30},
            .skill_priority = {40, 40, 40, 40, 40},
            .defense_priority = {70, 70, 70, 70, 70},
            .support_priority = {50, 50, 50, 50, 50}
        },
        .active = 0,
        .auto_skill = 1,
        .auto_item = 1,
        .retreat_hp = 40
    }
};
#define DEFAULT_PRESET_COUNT (sizeof(s_default_presets) / sizeof(s_default_presets[0]))

/*
 * XOR encrypt/decrypt - matches FUN_00401300 pattern
 */
void ai_settings_decrypt(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 i;

    /* XOR with key - iterating backwards as in FUN_00401300 */
    for (i = 0; i < size; i++) {
        ptr[i] ^= g_ai_xor_key[i & 0x0f];
    }
}

void ai_settings_encrypt(void* data, u32 size) {
    ai_settings_decrypt(data, size);
}

/*
 * Initialize AI settings system
 */
int ai_settings_init(void) {
    memset(&g_ai_settings, 0, sizeof(AISettingsContext));

    /* Try to load from file */
    if (ai_settings_load()) {
        LOG_INFO("AI settings loaded from file");
    } else {
        /* Use default presets */
        u32 i;
        for (i = 0; i < DEFAULT_PRESET_COUNT && i < AI_SETTINGS_MAX; i++) {
            memcpy(&g_ai_settings.presets[i], &s_default_presets[i], sizeof(PetAISettings));
            g_ai_settings.valid[i] = 1;
        }
        g_ai_settings.active_preset = 0;
        LOG_INFO("Using default AI settings");
    }

    g_ai_settings.loaded = 1;
    return 1;
}

/*
 * Shutdown AI settings system
 */
void ai_settings_shutdown(void) {
    if (g_ai_settings.dirty) {
        ai_settings_save();
    }

    memset(&g_ai_settings, 0, sizeof(AISettingsContext));
    LOG_INFO("AI settings shutdown");
}

/*
 * Load AI settings from file
 * Matches FUN_00401300 structure
 */
int ai_settings_load(void) {
    FILE* fp;
    AISettingsHeader header;
    PetAISettings entry;
    char name_buffer[AI_NAME_LENGTH];
    u32 entry_count;
    u32 i;
    int found;

    fp = fopen(AI_SETTINGS_FILE, "rb");
    if (!fp) {
        LOG_WARN("AI settings file not found: %s", AI_SETTINGS_FILE);
        return 0;
    }

    /* Read header (0x10 bytes) - matches FUN_00401300 */
    if (fread(&header, sizeof(AISettingsHeader), 1, fp) != 1) {
        fclose(fp);
        LOG_ERROR("Failed to read AI settings header");
        return 0;
    }

    /* Decrypt header */
    ai_settings_decrypt(&header, sizeof(AISettingsHeader));

    /* Validate header */
    if (header.version > 0x0100) {
        fclose(fp);
        LOG_WARN("AI settings version mismatch: %u", header.version);
        return 0;
    }

    /* Store character name */
    strncpy(g_ai_settings.character_name, header.name, AI_NAME_LENGTH - 1);

    /* Read entries - each entry is 0x50 bytes */
    entry_count = header.entry_count;
    if (entry_count > AI_SETTINGS_MAX) {
        entry_count = AI_SETTINGS_MAX;
    }

    for (i = 0; i < entry_count; i++) {
        if (fread(&entry, sizeof(PetAISettings), 1, fp) != 1) {
            break;
        }

        /* Decrypt entry */
        ai_settings_decrypt(&entry, sizeof(PetAISettings));

        /* Check if name matches - matches FUN_00401300 string comparison */
        if (entry.name[0] != '\0') {
            memcpy(&g_ai_settings.presets[i], &entry, sizeof(PetAISettings));
            g_ai_settings.valid[i] = 1;

            if (entry.active) {
                g_ai_settings.active_preset = i;
            }
        }
    }

    fclose(fp);
    LOG_INFO("Loaded %u AI settings presets", entry_count);
    return 1;
}

/*
 * Save AI settings to file
 */
int ai_settings_save(void) {
    FILE* fp;
    AISettingsHeader header;
    PetAISettings entry;
    u32 entry_count;
    u32 i;

    if (!g_ai_settings.dirty) {
        return 1;
    }

    fp = fopen(AI_SETTINGS_FILE, "wb");
    if (!fp) {
        strcpy(g_ai_settings.last_error, "Cannot create file");
        LOG_ERROR("Failed to create AI settings file");
        return 0;
    }

    /* Count valid entries */
    entry_count = 0;
    for (i = 0; i < AI_SETTINGS_MAX; i++) {
        if (g_ai_settings.valid[i]) {
            entry_count++;
        }
    }

    /* Prepare header */
    memset(&header, 0, sizeof(AISettingsHeader));
    strncpy(header.name, g_ai_settings.character_name, AI_NAME_LENGTH - 1);
    header.version = 0x0100;
    header.entry_count = entry_count;

    /* Encrypt header */
    ai_settings_encrypt(&header, sizeof(AISettingsHeader));

    if (fwrite(&header, sizeof(AISettingsHeader), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }

    /* Write entries */
    for (i = 0; i < AI_SETTINGS_MAX; i++) {
        if (g_ai_settings.valid[i]) {
            memcpy(&entry, &g_ai_settings.presets[i], sizeof(PetAISettings));
            ai_settings_encrypt(&entry, sizeof(PetAISettings));

            if (fwrite(&entry, sizeof(PetAISettings), 1, fp) != 1) {
                fclose(fp);
                return 0;
            }
        }
    }

    fclose(fp);
    g_ai_settings.dirty = 0;

    LOG_INFO("Saved AI settings: %u presets", entry_count);
    return 1;
}

/*
 * Add new preset
 */
int ai_settings_add_preset(PetAISettings* preset) {
    u32 i;

    if (!preset) return -1;

    /* Find empty slot */
    for (i = 0; i < AI_SETTINGS_MAX; i++) {
        if (!g_ai_settings.valid[i]) {
            memcpy(&g_ai_settings.presets[i], preset, sizeof(PetAISettings));
            g_ai_settings.valid[i] = 1;
            g_ai_settings.dirty = 1;
            return i;
        }
    }

    return -1;
}

/*
 * Update existing preset
 */
int ai_settings_update_preset(int index, PetAISettings* preset) {
    if (index < 0 || index >= AI_SETTINGS_MAX || !preset) {
        return 0;
    }

    if (!g_ai_settings.valid[index]) {
        return 0;
    }

    memcpy(&g_ai_settings.presets[index], preset, sizeof(PetAISettings));
    g_ai_settings.dirty = 1;

    return 1;
}

/*
 * Get preset by index
 */
PetAISettings* ai_settings_get_preset(int index) {
    if (index < 0 || index >= AI_SETTINGS_MAX) {
        return NULL;
    }

    if (!g_ai_settings.valid[index]) {
        return NULL;
    }

    return &g_ai_settings.presets[index];
}

/*
 * Find preset by name
 */
int ai_settings_find_preset(const char* name) {
    u32 i;

    if (!name) return -1;

    for (i = 0; i < AI_SETTINGS_MAX; i++) {
        if (g_ai_settings.valid[i]) {
            if (strcmp(g_ai_settings.presets[i].name, name) == 0) {
                return i;
            }
        }
    }

    return -1;
}

/*
 * Delete preset
 */
int ai_settings_delete_preset(int index) {
    if (index < 0 || index >= AI_SETTINGS_MAX) {
        return 0;
    }

    if (!g_ai_settings.valid[index]) {
        return 0;
    }

    memset(&g_ai_settings.presets[index], 0, sizeof(PetAISettings));
    g_ai_settings.valid[index] = 0;
    g_ai_settings.dirty = 1;

    /* Update active preset if needed */
    if (g_ai_settings.active_preset == index) {
        g_ai_settings.active_preset = 0;
    }

    return 1;
}

/*
 * Set active preset
 */
void ai_settings_set_active(int index) {
    u32 i;

    if (index < 0 || index >= AI_SETTINGS_MAX) {
        return;
    }

    if (!g_ai_settings.valid[index]) {
        return;
    }

    /* Deactivate all */
    for (i = 0; i < AI_SETTINGS_MAX; i++) {
        if (g_ai_settings.valid[i]) {
            g_ai_settings.presets[i].active = 0;
        }
    }

    /* Activate selected */
    g_ai_settings.presets[index].active = 1;
    g_ai_settings.active_preset = index;
    g_ai_settings.dirty = 1;
}

/*
 * Get active preset
 */
PetAISettings* ai_settings_get_active(void) {
    if (g_ai_settings.active_preset >= AI_SETTINGS_MAX) {
        return NULL;
    }

    if (!g_ai_settings.valid[g_ai_settings.active_preset]) {
        return NULL;
    }

    return &g_ai_settings.presets[g_ai_settings.active_preset];
}

/*
 * Get last error
 */
const char* ai_settings_get_last_error(void) {
    return g_ai_settings.last_error;
}
