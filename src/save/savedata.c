/*
 * Stone Age Client - Save Data System Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "types.h"
#include "savedata.h"
#include "logger.h"

/* Global save context */
SaveContext g_save = {0};

/*
 * Initialize save system
 */
int savedata_init(void) {
    memset(&g_save, 0, sizeof(SaveContext));

    /* Default auto-save settings */
    g_save.auto_save.enabled = 1;
    g_save.auto_save.interval_minutes = 5;
    g_save.auto_save.on_level_up = 1;
    g_save.auto_save.on_quest_complete = 1;
    g_save.auto_save.backup_count = 3;

    /* Scan existing save slots */
    int i;
    for (i = 0; i < MAX_SAVE_SLOTS; i++) {
        char path[64];
        snprintf(path, sizeof(path), "data\\save%02d.dat", i + 1);

        FILE* fp = fopen(path, "rb");
        if (fp) {
            SaveSlotHeader header;
            if (fread(&header, sizeof(SaveSlotHeader), 1, fp) == 1) {
                if (header.magic == SAVE_MAGIC) {
                    g_save.slots[i] = header;
                    g_save.slot_valid[i] = 1;
                }
            }
            fclose(fp);
        }
    }

    LOG_INFO("Save system initialized");
    return 1;
}

/*
 * Shutdown save system
 */
void savedata_shutdown(void) {
    if (g_save.dirty) {
        savedata_auto_save();
    }

    memset(&g_save, 0, sizeof(SaveContext));
    LOG_INFO("Save system shutdown");
}

/*
 * Save to slot
 */
int savedata_save(int slot) {
    char path[64];
    FILE* fp;
    u32 checksum;

    if (slot < 0 || slot >= MAX_SAVE_SLOTS) {
        strcpy(g_save.last_error, "Invalid slot");
        return 0;
    }

    /* Update save data */
    g_save.current_save.header.magic = SAVE_MAGIC;
    g_save.current_save.header.version = SAVE_VERSION;
    g_save.current_save.header.timestamp = (u32)time(NULL);
    g_save.current_save.header.play_time = g_save.current_save.total_play_time;
    g_save.current_save.header.data_size = sizeof(SaveData);

    /* Copy important fields to header */
    strncpy(g_save.current_save.header.char_name, g_save.current_save.name, 23);
    g_save.current_save.header.level = g_save.current_save.level;
    g_save.current_save.header.map_id = g_save.current_save.map_id;
    g_save.current_save.header.gold = g_save.current_save.gold;

    /* Calculate checksum */
    checksum = savedata_calculate_checksum(&g_save.current_save, sizeof(SaveData));
    g_save.current_save.header.checksum = checksum;

    /* Write file */
    snprintf(path, sizeof(path), "data\\save%02d.dat", slot + 1);

    fp = fopen(path, "wb");
    if (!fp) {
        strcpy(g_save.last_error, "Cannot create save file");
        g_save.failed_saves++;
        LOG_ERROR("Failed to save: %s", path);
        return 0;
    }

    if (fwrite(&g_save.current_save, sizeof(SaveData), 1, fp) != 1) {
        strcpy(g_save.last_error, "Write error");
        fclose(fp);
        g_save.failed_saves++;
        return 0;
    }

    fclose(fp);

    /* Update slot info */
    g_save.slots[slot] = g_save.current_save.header;
    g_save.slot_valid[slot] = 1;
    g_save.dirty = 0;
    g_save.last_save_time = timeGetTime();
    g_save.total_saves++;

    LOG_INFO("Saved to slot %d: %s (Lv.%d)",
             slot + 1, g_save.current_save.name, g_save.current_save.level);
    return 1;
}

/*
 * Async save (simulated)
 */
int savedata_save_async(int slot) {
    /* In a real implementation, this would run on a separate thread */
    return savedata_save(slot);
}

/*
 * Quick save
 */
int savedata_quick_save(void) {
    /* Find first available slot or use slot 0 */
    int slot = 0;
    return savedata_save(slot);
}

/*
 * Auto save
 */
int savedata_auto_save(void) {
    if (!g_save.auto_save.enabled) {
        return 0;
    }

    /* Create backup first */
    savedata_create_backup();

    g_save.last_auto_save = timeGetTime();
    return savedata_quick_save();
}

/*
 * Check if saving
 */
int savedata_is_saving(void) {
    return g_save.saving;
}

/*
 * Load from slot
 */
int savedata_load(int slot) {
    char path[64];
    FILE* fp;
    u32 checksum;

    if (slot < 0 || slot >= MAX_SAVE_SLOTS) {
        strcpy(g_save.last_error, "Invalid slot");
        return 0;
    }

    snprintf(path, sizeof(path), "data\\save%02d.dat", slot + 1);

    fp = fopen(path, "rb");
    if (!fp) {
        strcpy(g_save.last_error, "Save file not found");
        g_save.failed_loads++;
        return 0;
    }

    if (fread(&g_save.current_save, sizeof(SaveData), 1, fp) != 1) {
        strcpy(g_save.last_error, "Read error");
        fclose(fp);
        g_save.failed_loads++;
        return 0;
    }

    fclose(fp);

    /* Verify magic */
    if (g_save.current_save.header.magic != SAVE_MAGIC) {
        strcpy(g_save.last_error, "Invalid save file");
        g_save.failed_loads++;
        return 0;
    }

    /* Verify checksum */
    checksum = g_save.current_save.header.checksum;
    if (savedata_calculate_checksum(&g_save.current_save, sizeof(SaveData)) != checksum) {
        LOG_WARN("Save file checksum mismatch, may be corrupted");
        /* Try to fix */
        if (!savedata_fix_corruption(&g_save.current_save)) {
            strcpy(g_save.last_error, "Save file corrupted");
            g_save.failed_loads++;
            return 0;
        }
    }

    g_save.dirty = 0;
    g_save.total_loads++;

    LOG_INFO("Loaded from slot %d: %s (Lv.%d)",
             slot + 1, g_save.current_save.name, g_save.current_save.level);
    return 1;
}

/*
 * Async load
 */
int savedata_load_async(int slot) {
    return savedata_load(slot);
}

/*
 * Quick load
 */
int savedata_quick_load(void) {
    /* Load from first valid slot */
    int i;
    for (i = 0; i < MAX_SAVE_SLOTS; i++) {
        if (g_save.slot_valid[i]) {
            return savedata_load(i);
        }
    }
    return 0;
}

/*
 * Check if loading
 */
int savedata_is_loading(void) {
    return g_save.loading;
}

/*
 * Get slot count
 */
int savedata_get_slot_count(void) {
    return MAX_SAVE_SLOTS;
}

/*
 * Check if slot is valid
 */
int savedata_is_slot_valid(int slot) {
    if (slot < 0 || slot >= MAX_SAVE_SLOTS) {
        return 0;
    }
    return g_save.slot_valid[slot];
}

/*
 * Get slot info
 */
SaveSlotHeader* savedata_get_slot_info(int slot) {
    if (slot < 0 || slot >= MAX_SAVE_SLOTS) {
        return NULL;
    }
    return g_save.slot_valid[slot] ? &g_save.slots[slot] : NULL;
}

/*
 * Delete slot
 */
int savedata_delete_slot(int slot) {
    char path[64];

    if (slot < 0 || slot >= MAX_SAVE_SLOTS) {
        return 0;
    }

    snprintf(path, sizeof(path), "data\\save%02d.dat", slot + 1);
    remove(path);

    g_save.slot_valid[slot] = 0;
    memset(&g_save.slots[slot], 0, sizeof(SaveSlotHeader));

    LOG_INFO("Deleted slot %d", slot + 1);
    return 1;
}

/*
 * Copy slot
 */
int savedata_copy_slot(int from, int to) {
    char from_path[64], to_path[64];
    FILE *fp_from, *fp_to;
    char buffer[4096];
    size_t bytes;

    if (from < 0 || from >= MAX_SAVE_SLOTS ||
        to < 0 || to >= MAX_SAVE_SLOTS || from == to) {
        return 0;
    }

    if (!g_save.slot_valid[from]) {
        return 0;
    }

    snprintf(from_path, sizeof(from_path), "data\\save%02d.dat", from + 1);
    snprintf(to_path, sizeof(to_path), "data\\save%02d.dat", to + 1);

    fp_from = fopen(from_path, "rb");
    if (!fp_from) return 0;

    fp_to = fopen(to_path, "wb");
    if (!fp_to) {
        fclose(fp_from);
        return 0;
    }

    while ((bytes = fread(buffer, 1, sizeof(buffer), fp_from)) > 0) {
        fwrite(buffer, 1, bytes, fp_to);
    }

    fclose(fp_from);
    fclose(fp_to);

    g_save.slots[to] = g_save.slots[from];
    g_save.slot_valid[to] = 1;

    LOG_INFO("Copied slot %d to slot %d", from + 1, to + 1);
    return 1;
}

/*
 * Mark dirty
 */
void savedata_mark_dirty(void) {
    g_save.dirty = 1;
}

/*
 * Has unsaved changes
 */
int savedata_has_unsaved_changes(void) {
    return g_save.dirty;
}

/*
 * Update current save data
 */
void savedata_update_current(SaveData* data) {
    memcpy(&g_save.current_save, data, sizeof(SaveData));
    g_save.dirty = 1;
}

/*
 * Get current save data
 */
SaveData* savedata_get_current(void) {
    return &g_save.current_save;
}

/*
 * Calculate checksum
 */
u32 savedata_calculate_checksum(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 checksum = 0;
    u32 i;

    /* Skip checksum field itself */
    for (i = 0; i < size; i++) {
        if (i >= 8 && i < 12) continue;  /* Skip checksum bytes */
        checksum = ((checksum << 5) + checksum) + ptr[i];
    }

    return checksum;
}

/*
 * Verify checksum
 */
int savedata_verify_checksum(SaveData* save) {
    u32 stored = save->header.checksum;
    u32 calculated = savedata_calculate_checksum(save, sizeof(SaveData));
    return stored == calculated;
}

/*
 * Create backup
 */
int savedata_create_backup(void) {
    char src_path[64], dst_path[64];
    int i;

    /* Rotate backups */
    for (i = g_save.auto_save.backup_count - 1; i > 0; i--) {
        snprintf(dst_path, sizeof(dst_path), "data\\savebak%d.dat", i);
        snprintf(src_path, sizeof(src_path), "data\\savebak%d.dat", i - 1);
        rename(src_path, dst_path);
    }

    /* Create new backup */
    snprintf(src_path, sizeof(src_path), "data\\save01.dat");
    snprintf(dst_path, sizeof(dst_path), "data\\savebak0.dat");

    FILE* fp_src = fopen(src_path, "rb");
    FILE* fp_dst = fopen(dst_path, "wb");

    if (fp_src && fp_dst) {
        char buffer[4096];
        size_t bytes;
        while ((bytes = fread(buffer, 1, sizeof(buffer), fp_src)) > 0) {
            fwrite(buffer, 1, bytes, fp_dst);
        }
    }

    if (fp_src) fclose(fp_src);
    if (fp_dst) fclose(fp_dst);

    return 1;
}

/*
 * Restore backup
 */
int savedata_restore_backup(void) {
    char src_path[64], dst_path[64];

    snprintf(src_path, sizeof(src_path), "data\\savebak0.dat");
    snprintf(dst_path, sizeof(dst_path), "data\\save01.dat");

    FILE* fp_src = fopen(src_path, "rb");
    FILE* fp_dst = fopen(dst_path, "wb");

    if (!fp_src || !fp_dst) {
        if (fp_src) fclose(fp_src);
        if (fp_dst) fclose(fp_dst);
        return 0;
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), fp_src)) > 0) {
        fwrite(buffer, 1, bytes, fp_dst);
    }

    fclose(fp_src);
    fclose(fp_dst);

    return 1;
}

/*
 * Get backup count
 */
int savedata_get_backup_count(void) {
    return g_save.auto_save.backup_count;
}

/*
 * Delete backup
 */
int savedata_delete_backup(int index) {
    char path[64];
    snprintf(path, sizeof(path), "data\\savebak%d.dat", index);
    return remove(path) == 0;
}

/*
 * Set auto-save settings
 */
void savedata_set_auto_save(u8 enabled, u8 interval) {
    g_save.auto_save.enabled = enabled;
    g_save.auto_save.interval_minutes = interval;
}

/*
 * Check auto-save
 */
void savedata_check_auto_save(void) {
    if (!g_save.auto_save.enabled) return;
    if (!g_save.dirty) return;

    u32 elapsed = (timeGetTime() - g_save.last_auto_save) / 60000;
    if (elapsed >= g_save.auto_save.interval_minutes) {
        savedata_auto_save();
    }
}

/*
 * Should auto-save
 */
int savedata_should_auto_save(void) {
    if (!g_save.auto_save.enabled) return 0;
    if (!g_save.dirty) return 0;

    u32 elapsed = (timeGetTime() - g_save.last_auto_save) / 60000;
    return elapsed >= g_save.auto_save.interval_minutes;
}

/*
 * Write save file
 */
int savedata_write_file(const char* path, SaveData* save) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;

    int result = fwrite(save, sizeof(SaveData), 1, fp) == 1;
    fclose(fp);

    return result;
}

/*
 * Read save file
 */
int savedata_read_file(const char* path, SaveData* save) {
    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;

    int result = fread(save, sizeof(SaveData), 1, fp) == 1;
    fclose(fp);

    return result;
}

/*
 * Check if file exists
 */
int savedata_file_exists(const char* path) {
    FILE* fp = fopen(path, "rb");
    if (fp) {
        fclose(fp);
        return 1;
    }
    return 0;
}

/*
 * Export save
 */
int savedata_export(const char* path) {
    return savedata_write_file(path, &g_save.current_save);
}

/*
 * Import save
 */
int savedata_import(const char* path) {
    SaveData temp;
    if (!savedata_read_file(path, &temp)) {
        return 0;
    }

    if (!savedata_validate(&temp)) {
        return 0;
    }

    memcpy(&g_save.current_save, &temp, sizeof(SaveData));
    g_save.dirty = 1;

    return 1;
}

/*
 * Validate save data
 */
int savedata_validate(SaveData* save) {
    if (!save) return 0;

    if (save->header.magic != SAVE_MAGIC) return 0;
    if (save->header.version > SAVE_VERSION) return 0;

    if (save->level < 1 || save->level > 200) return 0;
    if (save->name[0] == '\0') return 0;

    return 1;
}

/*
 * Fix corruption
 */
int savedata_fix_corruption(SaveData* save) {
    if (!save) return 0;

    /* Try to recover what we can */
    if (save->header.magic != SAVE_MAGIC) {
        save->header.magic = SAVE_MAGIC;
    }

    if (save->level < 1) save->level = 1;
    if (save->level > 200) save->level = 200;

    /* Recalculate checksum */
    save->header.checksum = savedata_calculate_checksum(save, sizeof(SaveData));

    return 1;
}

/*
 * Get last error
 */
const char* savedata_get_last_error(void) {
    return g_save.last_error;
}

/*
 * Clear error
 */
void savedata_clear_error(void) {
    g_save.last_error[0] = '\0';
}

/*
 * Get play time
 */
u32 savedata_get_play_time(void) {
    return g_save.current_save.total_play_time;
}

/*
 * Update play time
 */
void savedata_update_play_time(void) {
    static u32 last_update = 0;
    u32 current = timeGetTime();

    if (last_update > 0) {
        u32 delta = (current - last_update) / 1000;
        g_save.current_save.total_play_time += delta;
    }

    last_update = current;
}

/*
 * Format play time
 */
const char* savedata_format_play_time(u32 seconds) {
    static char buffer[32];
    u32 hours = seconds / 3600;
    u32 minutes = (seconds % 3600) / 60;
    u32 secs = seconds % 60;

    snprintf(buffer, sizeof(buffer), "%u:%02u:%02u", hours, minutes, secs);
    return buffer;
}
