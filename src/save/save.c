/*
 * Stone Age Client - Save Data System Implementation
 * Reverse engineered from sa_9061.exe (FUN_00401300, FUN_004495c0)
 *
 * Handles: AI Settings, Mail, Auto-save
 * Album system is in album.c
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "save.h"
#include "album.h"
#include "logger.h"

/* Global AI settings context */
AISettingsContext g_ai_settings = {0};

/* XOR encryption key - from DAT_004c107c to DAT_004c10bc */
static const u32 s_xor_key[4] = {
    0x12345678, 0x9ABCDEF0, 0x13579BDF, 0x2468ACE0
};

/*
 * XOR encrypt/decrypt data - FUN_00449f70 pattern
 */
void save_xor_crypt(void* data, u32 size) {
    u32* ptr = (u32*)data;
    u32 i;
    u32 key_index = 0;

    for (i = 0; i < size / 4; i++) {
        ptr[i] ^= s_xor_key[key_index];
        key_index = (key_index + 1) % 4;
    }
}

/*
 * Open file for reading - FUN_00492394 pattern
 */
void* save_open_read(const char* path) {
    FILE* fp = fopen(path, "rb");
    return fp;
}

/*
 * Open file for writing
 */
void* save_open_write(const char* path) {
    FILE* fp = fopen(path, "wb");
    return fp;
}

/*
 * Close file - FUN_00492214 pattern
 */
void save_close(void* handle) {
    if (handle) {
        fclose((FILE*)handle);
    }
}

/*
 * Read from file - FUN_0049212c pattern
 */
int save_read(void* handle, void* buffer, u32 size, u32 count) {
    if (!handle) return 0;
    return fread(buffer, size, count, (FILE*)handle) == count;
}

/*
 * Write to file - FUN_0049226a pattern
 */
int save_write(void* handle, const void* buffer, u32 size, u32 count) {
    if (!handle) return 0;
    return fwrite(buffer, size, count, (FILE*)handle) == count;
}

/*
 * Seek in file - FUN_004920a0 pattern
 */
int save_seek(void* handle, u32 offset, int origin) {
    if (!handle) return 0;
    return fseek((FILE*)handle, offset, origin) == 0;
}

/*
 * Initialize AI settings - FUN_00401300 pattern
 */
int ai_settings_init(void) {
    /* Default values from FUN_00401300 */
    g_ai_settings.settings[0].setting1 = 9;
    g_ai_settings.settings[0].setting2 = 0xFFFFFFFF;
    g_ai_settings.settings[1].setting1 = 1;
    g_ai_settings.settings[1].setting2 = 1;
    g_ai_settings.settings[1].setting3 = 0x1e;  /* 30 */
    g_ai_settings.settings[2].setting1 = 0xFFFFFFFF;
    g_ai_settings.settings[2].setting2 = 1;
    g_ai_settings.settings[2].setting3 = 0x1e;
    g_ai_settings.settings[3].setting1 = 9;
    g_ai_settings.settings[3].setting2 = 1;
    g_ai_settings.settings[3].setting3 = 0x1a;  /* 26 */
    g_ai_settings.settings[4].setting1 = 4;
    g_ai_settings.flags = 0;
    g_ai_settings.extra1 = 0;
    g_ai_settings.extra2 = 0;
    g_ai_settings.loaded = 0;

    LOG_INFO("AI settings initialized with defaults");
    return 1;
}

/*
 * Shutdown AI settings
 */
void ai_settings_shutdown(void) {
    memset(&g_ai_settings, 0, sizeof(AISettingsContext));
}

/*
 * Load AI settings from file - FUN_00401300
 */
int ai_settings_load(void) {
    FILE* fp;
    char header[16];
    char expected_header[16];

    fp = (FILE*)save_open_read(AI_SETTING_PATH);
    if (!fp) {
        LOG_DEBUG("AI settings file not found, using defaults");
        return 0;
    }

    /* Read and decrypt header */
    if (!save_read(fp, header, 1, 16)) {
        save_close(fp);
        return 0;
    }
    save_xor_crypt(header, 16);

    /* Check header - all zeros before encryption */
    memset(expected_header, 0, 16);
    if (memcmp(header, expected_header, 16) != 0) {
        /* Try next entry - seek by 0x45 bytes (from FUN_00401300) */
        save_seek(fp, 0x45, SEEK_CUR);
        save_read(fp, header, 1, 16);
        save_xor_crypt(header, 16);
    }

    /* Read settings - FUN_00401300 pattern */
    save_read(fp, &g_ai_settings.flags, 4, 1);
    save_read(fp, g_ai_settings.settings, sizeof(AISetting), AI_SETTING_COUNT);
    save_read(fp, &g_ai_settings.extra1, 4, 1);
    save_read(fp, &g_ai_settings.extra2, 1, 1);

    save_close(fp);
    g_ai_settings.loaded = 1;

    LOG_DEBUG("AI settings loaded");
    return 1;
}

/*
 * Save AI settings to file
 */
int ai_settings_save(void) {
    FILE* fp;
    char header[16];

    fp = (FILE*)save_open_write(AI_SETTING_PATH);
    if (!fp) {
        LOG_WARN("Cannot create AI settings file");
        return 0;
    }

    /* Prepare and encrypt header */
    memset(header, 0, 16);
    save_xor_crypt(header, 16);
    save_write(fp, header, 1, 16);

    /* Write settings */
    save_write(fp, &g_ai_settings.flags, 4, 1);
    save_write(fp, g_ai_settings.settings, sizeof(AISetting), AI_SETTING_COUNT);
    save_write(fp, &g_ai_settings.extra1, 4, 1);
    save_write(fp, &g_ai_settings.extra2, 1, 1);

    save_close(fp);
    LOG_DEBUG("AI settings saved");
    return 1;
}

/*
 * Get AI setting value
 */
u32 ai_settings_get(int index, int setting) {
    if (index < 0 || index >= AI_SETTING_COUNT) return 0;
    if (setting < 0 || setting >= 5) return 0;

    return (&g_ai_settings.settings[index].setting1)[setting];
}

/*
 * Set AI setting value
 */
void ai_settings_set(int index, int setting, u32 value) {
    if (index < 0 || index >= AI_SETTING_COUNT) return;
    if (setting < 0 || setting >= 5) return;

    (&g_ai_settings.settings[index].setting1)[setting] = value;
}

/*
 * Load mail data - FUN_004495c0, FUN_00449660 pattern
 * Mail stored at DAT_045967c0, 0xef0 bytes per entry, 80 entries
 */
int mail_load(int index) {
    FILE* fp;
    u32 offset;
    static char mail_buffer[MAIL_ENTRY_SIZE];

    if (index < 0 || index >= MAIL_MAX_ENTRIES) return 0;

    fp = (FILE*)save_open_read(MAIL_DATA_PATH);
    if (!fp) {
        return 0;
    }

    /* Calculate offset: header + index * entry_size */
    offset = index * MAIL_ENTRY_SIZE;
    save_seek(fp, offset, SEEK_SET);

    /* Read mail entry */
    if (!save_read(fp, mail_buffer, 1, MAIL_ENTRY_SIZE)) {
        save_close(fp);
        return 0;
    }

    save_close(fp);
    LOG_DEBUG("Loaded mail entry %d", index);
    return 1;
}

/*
 * Save mail data - FUN_004495c0 pattern
 */
int mail_save(int index) {
    FILE* fp;
    u32 offset;
    static char mail_buffer[MAIL_ENTRY_SIZE];

    if (index < 0 || index >= MAIL_MAX_ENTRIES) return 0;

    fp = (FILE*)save_open_write(MAIL_DATA_PATH);
    if (!fp) {
        return 0;
    }

    /* Seek to position */
    offset = index * MAIL_ENTRY_SIZE;
    save_seek(fp, offset, SEEK_SET);

    /* Write mail entry */
    memset(mail_buffer, 0, MAIL_ENTRY_SIZE);
    if (!save_write(fp, mail_buffer, 1, MAIL_ENTRY_SIZE)) {
        save_close(fp);
        return 0;
    }

    save_close(fp);
    LOG_DEBUG("Saved mail entry %d", index);
    return 1;
}

/*
 * Load auto save data - FUN_0041f3d0 pattern
 */
int auto_save_load(void* data) {
    FILE* fp;

    fp = (FILE*)save_open_read(AUTO_DATA_PATH);
    if (!fp) {
        return 0;
    }

    save_read(fp, data, 1, 256);
    save_close(fp);
    return 1;
}

/*
 * Save auto save data
 */
int auto_save_save(void* data) {
    FILE* fp;

    fp = (FILE*)save_open_write(AUTO_DATA_PATH);
    if (!fp) {
        return 0;
    }

    save_write(fp, data, 1, 256);
    save_close(fp);
    return 1;
}

/*
 * Initialize save system
 */
int save_init(void) {
    memset(&g_ai_settings, 0, sizeof(AISettingsContext));

    /* Initialize with defaults */
    ai_settings_init();

    /* Try to load existing settings */
    ai_settings_load();

    LOG_INFO("Save system initialized");
    return 1;
}

/*
 * Shutdown save system
 */
void save_shutdown(void) {
    ai_settings_shutdown();
    LOG_INFO("Save system shutdown");
}

/*
 * Save all data
 */
int save_all(void) {
    int result = 1;

    /* Save AI settings */
    if (!ai_settings_save()) {
        result = 0;
    }

    /* Album saves are handled by album.c */
    return result;
}

/*
 * Load all data
 */
int load_all(void) {
    /* Load AI settings */
    ai_settings_load();

    /* Album loads are handled by album.c */
    return 1;
}

/*
 * Check save file exists
 */
int save_exists(const char* path) {
    FILE* fp = fopen(path, "rb");
    if (fp) {
        fclose(fp);
        return 1;
    }
    return 0;
}

/*
 * Delete save file
 */
int save_delete(const char* path) {
    return remove(path) == 0;
}

/*
 * Get save file size
 */
u32 save_get_size(const char* path) {
    FILE* fp = fopen(path, "rb");
    u32 size = 0;

    if (fp) {
        fseek(fp, 0, SEEK_END);
        size = (u32)ftell(fp);
        fclose(fp);
    }

    return size;
}
