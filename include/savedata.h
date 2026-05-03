/*
 * Stone Age Client - Save Data System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef SAVEDATA_H
#define SAVEDATA_H

#include "types.h"

/* Constants */
#define SAVE_DATA_FILE      "data\\savedata.dat"
#define SAVE_BACKUP_FILE    "data\\savebak.dat"
#define MAX_SAVE_SLOTS      3
#define SAVE_MAGIC          0x53415645  /* "SAVE" */
#define SAVE_VERSION        0x0100

/* Save slot info */
typedef struct {
    u32 magic;
    u16 version;
    u16 reserved;
    u32 checksum;
    u32 timestamp;
    u32 play_time;
    char char_name[24];
    u16 level;
    u16 map_id;
    u32 gold;
    u32 data_size;
} SaveSlotHeader;

/* Save data structure */
typedef struct {
    /* Header */
    SaveSlotHeader header;

    /* Character data */
    char name[24];
    u32 id;
    u16 level;
    u16 exp;
    u16 job;
    u16 job_level;
    u16 hp;
    u16 max_hp;
    u16 mp;
    u16 max_mp;
    u16 str;
    u16 vit;
    u16 dex;
    u16 agi;
    u16 intel;
    u16 luck;
    u16 stat_points;
    u16 skill_points;

    /* Position */
    u32 map_id;
    u16 x;
    u16 y;
    u8 dir;

    /* Stats */
    u32 gold;
    u32 total_kills;
    u32 total_deaths;
    u32 quests_completed;
    u32 achievements;

    /* Timestamps */
    u32 created_time;
    u32 last_save_time;
    u32 total_play_time;

    /* Inventory checksum */
    u32 inventory_checksum;
    u32 equipment_checksum;
    u32 skill_checksum;
    u32 quest_checksum;

} SaveData;

/* Auto-save settings */
typedef struct {
    u8 enabled;
    u8 interval_minutes;
    u8 on_level_up;
    u8 on_quest_complete;
    u8 on_death;
    u8 backup_count;
    u8 reserved[2];
} AutoSaveSettings;

/* Save context */
typedef struct {
    /* Current save data */
    SaveData current_save;
    u8 has_pending_save;
    u8 dirty;

    /* Auto-save */
    AutoSaveSettings auto_save;
    u32 last_save_time;
    u32 last_auto_save;

    /* Slots */
    SaveSlotHeader slots[MAX_SAVE_SLOTS];
    u8 slot_valid[MAX_SAVE_SLOTS];

    /* Status */
    u8 saving;
    u8 loading;
    char last_error[64];

    /* Statistics */
    u32 total_saves;
    u32 total_loads;
    u32 failed_saves;
    u32 failed_loads;

} SaveContext;

/* Global save context */
extern SaveContext g_save;

/* Initialization */
int savedata_init(void);
void savedata_shutdown(void);

/* Save operations */
int savedata_save(int slot);
int savedata_save_async(int slot);
int savedata_quick_save(void);
int savedata_auto_save(void);
int savedata_is_saving(void);

/* Load operations */
int savedata_load(int slot);
int savedata_load_async(int slot);
int savedata_quick_load(void);
int savedata_is_loading(void);

/* Slot management */
int savedata_get_slot_count(void);
int savedata_is_slot_valid(int slot);
SaveSlotHeader* savedata_get_slot_info(int slot);
int savedata_delete_slot(int slot);
int savedata_copy_slot(int from, int to);

/* Data management */
void savedata_mark_dirty(void);
int savedata_has_unsaved_changes(void);
void savedata_update_current(SaveData* data);
SaveData* savedata_get_current(void);

/* Checksum */
u32 savedata_calculate_checksum(void* data, u32 size);
int savedata_verify_checksum(SaveData* save);

/* Backup */
int savedata_create_backup(void);
int savedata_restore_backup(void);
int savedata_get_backup_count(void);
int savedata_delete_backup(int index);

/* Auto-save */
void savedata_set_auto_save(u8 enabled, u8 interval);
void savedata_check_auto_save(void);
int savedata_should_auto_save(void);

/* File operations */
int savedata_write_file(const char* path, SaveData* save);
int savedata_read_file(const char* path, SaveData* save);
int savedata_file_exists(const char* path);

/* Import/Export */
int savedata_export(const char* path);
int savedata_import(const char* path);

/* Validation */
int savedata_validate(SaveData* save);
int savedata_fix_corruption(SaveData* save);

/* Error handling */
const char* savedata_get_last_error(void);
void savedata_clear_error(void);

/* Utility */
u32 savedata_get_play_time(void);
void savedata_update_play_time(void);
const char* savedata_format_play_time(u32 seconds);

#endif /* SAVEDATA_H */
