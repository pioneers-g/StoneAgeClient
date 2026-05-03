/*
 * Stone Age Client - Save Data System
 * Reverse engineered from sa_9061.exe (FUN_00401300, FUN_004495c0)
 *
 * Note: Album system types are in album.h
 */

#ifndef SAVE_H
#define SAVE_H

#include "types.h"

/* Save file paths */
#define SAVE_DATA_PATH          "data\\savedata.dat"
#define AI_SETTING_PATH         "data\\AISetting.dat"
#define MAIL_DATA_PATH          "data\\mail.dat"
#define AUTO_DATA_PATH          "data\\auto.dat"
#define CHANNEL_DATA_PATH       "data\\channel.dat"

/* AI Setting constants - from FUN_00401300 analysis */
#define AI_SETTING_SIZE         0x14    /* 20 bytes per setting */
#define AI_SETTING_COUNT        5       /* 5 AI settings */

/* Mail save constants - from FUN_004495c0 analysis */
#define MAIL_ENTRY_SIZE         0xef0   /* 3824 bytes per mail entry */
#define MAIL_MAX_ENTRIES        80      /* 0x50 entries from FUN_00449660 */

/* XOR encryption key table - DAT_004c107c to DAT_004c10bc */
#define XOR_KEY_SIZE            16      /* 4 dwords = 16 bytes */

/* AI Setting structure - from FUN_00401300 analysis */
#pragma pack(push, 1)
typedef struct {
    u32 setting1;           /* Primary skill setting */
    u32 setting2;           /* Secondary setting */
    u32 setting3;           /* Tertiary setting */
    u32 setting4;           /* Quaternary setting */
    u32 setting5;           /* Quinary setting */
} AISetting;
#pragma pack(pop)

/* AI Settings context - matches DAT_004d7ea4 region */
typedef struct {
    AISetting settings[AI_SETTING_COUNT];
    u32 flags;              /* DAT_004d9050 - AI mode flags */
    u32 extra1;             /* DAT_004d7f18 */
    u32 extra2;             /* DAT_004d7f54 */
    u32 loaded;             /* Settings loaded flag */
} AISettingsContext;

/* Global AI settings context */
extern AISettingsContext g_ai_settings;

/* ========================================
 * File I/O Functions - FUN_00492374 pattern
 * ======================================== */

/* Open file for reading */
void* save_open_read(const char* path);

/* Open file for writing */
void* save_open_write(const char* path);

/* Close file */
void save_close(void* handle);

/* Read from file */
int save_read(void* handle, void* buffer, u32 size, u32 count);

/* Write to file */
int save_write(void* handle, const void* buffer, u32 size, u32 count);

/* Seek in file */
int save_seek(void* handle, u32 offset, int origin);

/* ========================================
 * XOR Encryption - DAT_004c107c pattern
 * ======================================== */

/* Encrypt/decrypt data buffer */
void save_xor_crypt(void* data, u32 size);

/* ========================================
 * AI Settings - FUN_00401300
 * ======================================== */

/* Initialize AI settings */
int ai_settings_init(void);

/* Shutdown AI settings */
void ai_settings_shutdown(void);

/* Load AI settings from file */
int ai_settings_load(void);

/* Save AI settings to file */
int ai_settings_save(void);

/* Get AI setting */
u32 ai_settings_get(int index, int setting);

/* Set AI setting */
void ai_settings_set(int index, int setting, u32 value);

/* ========================================
 * Mail Save System - FUN_004495c0, FUN_00449660
 * ======================================== */

/* Load mail data */
int mail_load(int index);

/* Save mail data */
int mail_save(int index);

/* ========================================
 * Auto Save System - FUN_0041f3d0
 * ======================================== */

/* Load auto save data */
int auto_save_load(void* data);

/* Save auto save data */
int auto_save_save(void* data);

/* ========================================
 * General Save/Load
 * ======================================== */

/* Initialize save system */
int save_init(void);

/* Shutdown save system */
void save_shutdown(void);

/* Save all data */
int save_all(void);

/* Load all data */
int load_all(void);

/* Check save exists */
int save_exists(const char* path);

/* Delete save file */
int save_delete(const char* path);

/* Get save file size */
u32 save_get_size(const char* path);

#endif /* SAVE_H */
