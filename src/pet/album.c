/*
 * Stone Age Client - Album System Implementation
 * Pet Album/Bestiary - Creature collection database
 * Reverse engineered from sa_9061.exe:
 *   FUN_0044a100 - Main album loader (tries album_4.dat through album_36.dat)
 *   FUN_00449e00 - Generic album file reader with XOR decryption
 *   FUN_00449f70 - data/album.dat loader (0x7c entries)
 *   FUN_00449910 - Single entry reader from album_37.dat
 *   FUN_00449a10 - Album initialization with random stats
 *   FUN_0044a690 - Sprite ID to album entry mapping
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "album.h"
#include "logger.h"

/* Global album context - matches DAT_045837c8 region */
AlbumContext g_album = {0};

/* XOR encryption key from DAT_004c107c in sa_9061.exe */
const u8 g_album_xor_key[16] = {
    0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
    0x0f, 0xed, 0xcb, 0xa9, 0x87, 0x65, 0x43, 0x21
};

/* "f;encor1c" string from DAT_0049e48c - checksum marker */
static const char s_encor_marker[] = "f;encor1c";

/* Album file paths and entry counts - from FUN_0044a100 */
typedef struct {
    const char* path;
    u16 entry_count;
} AlbumFileInfo;

static const AlbumFileInfo s_album_files[] = {
    { "data\\album.dat",      0x7c },   /* 124 entries - FUN_00449f70 */
    { "data\\album_4.dat",    0xe0 },   /* 224 entries */
    { "data\\album_5.dat",    0xe4 },   /* 228 entries */
    { "data\\album_6.dat",    0xe5 },   /* 229 entries */
    { "data\\album_7.dat",    0xe6 },   /* 230 entries */
    { "data\\album_8.dat",    0xed },   /* 237 entries */
    { "data\\album_9.dat",    0xf0 },   /* 240 entries */
    { "data\\album_10.dat",   0x129 },  /* 297 entries */
    { "data\\album_12.dat",   0x12d },  /* 301 entries */
    { "data\\album_13.dat",   0x131 },  /* 305 entries */
    { "data\\album_14.dat",   0x161 },  /* 353 entries */
    { "data\\album_15.dat",   0x165 },  /* 357 entries */
    { "data\\album_16.dat",   0x16c },  /* 364 entries */
    { "data\\album_17.dat",   0x185 },  /* 389 entries */
    { "data\\album_18.dat",   0x187 },  /* 391 entries */
    { "data\\album_19.dat",   0x18b },  /* 395 entries */
    { "data\\album_20.dat",   0x18e },  /* 398 entries */
    { "data\\album_21.dat",   0x191 },  /* 401 entries */
    { "data\\album_22.dat",   0x195 },  /* 405 entries */
    { "data\\album_23.dat",   0x197 },  /* 407 entries */
    { "data\\album_24.dat",   0x19e },  /* 414 entries */
    { "data\\album_25.dat",   0x19f },  /* 415 entries */
    { "data\\album_26.dat",   0x1a4 },  /* 420 entries */
    { "data\\album_27.dat",   0x1a6 },  /* 422 entries */
    { "data\\album_28.dat",   0x1a7 },  /* 423 entries */
    { "data\\album_29.dat",   0x1a8 },  /* 424 entries */
    { "data\\album_30.dat",   0x1a9 },  /* 425 entries */
    { "data\\album_31.dat",   0x1ad },  /* 429 entries */
    { "data\\album_32.dat",   0x1af },  /* 431 entries */
    { "data\\album_33.dat",   0x1b0 },  /* 432 entries */
    { "data\\album_34.dat",   0x1b4 },  /* 436 entries */
    { "data\\album_35.dat",   0x1ce },  /* 462 entries */
    { "data\\album_36.dat",   0x1e3 },  /* 483 entries */
    { "data\\album_37.dat",   0x1ee },  /* 494 entries - FUN_00449910 */
    { NULL, 0 }
};

#define ALBUM_FILE_COUNT (sizeof(s_album_files) / sizeof(s_album_files[0]) - 1)

/* Sprite-to-album mapping table - from DAT_004bd438 */
static AlbumSpriteMapping s_sprite_map[ALBUM_MAX_ENTRIES];
static u32 s_sprite_map_count = 0;

/*
 * XOR decrypt/encrypt - matches FUN_00449e00 pattern
 * XORs data with 16-byte key at offsets 0, 4, 8, 12 (dword aligned)
 */
void album_decrypt(void* data, u32 size) {
    u32* ptr = (u32*)data;
    const u32* key = (const u32*)g_album_xor_key;
    u32 i;

    /* XOR dwords - matches binary loop pattern */
    for (i = 0; i < (size >> 2); i++) {
        ptr[i] ^= key[i & 3];
    }
}

void album_encrypt(void* data, u32 size) {
    /* XOR is symmetric */
    album_decrypt(data, size);
}

/*
 * Random number in range - matches FUN_004472e0
 */
static u32 album_rand_range(u32 min_val, u32 max_val) {
    return min_val + (rand() % (max_val - min_val + 1));
}

/*
 * Initialize entry with random stats - FUN_00449a10
 */
void album_init_random_stats(AlbumEntry* entry) {
    int i;

    if (!entry) return;

    /* Initialize name with random bytes - matches FUN_00449a10 loop */
    for (i = 0; i < ALBUM_NAME_LENGTH; i++) {
        entry->name[i] = (char)album_rand_range(1, 255);
    }
    entry->name[ALBUM_NAME_LENGTH - 1] = '\0';

    /* Initialize stats with random values - from FUN_00449a10 */
    entry->stat_1 = album_rand_range(30000, 60000);  /* HP - higher range */
    entry->stat_2 = album_rand_range(1, 30000);
    entry->stat_3 = album_rand_range(1, 30000);
    entry->stat_4 = album_rand_range(1, 30000);
    entry->stat_5 = album_rand_range(1, 30000);
    entry->stat_6 = album_rand_range(1, 30000);
    entry->stat_7 = album_rand_range(1, 30000);
    entry->stat_8 = album_rand_range(1, 30000);
    entry->stat_9 = album_rand_range(1, 30000);
    entry->stat_10 = album_rand_range(1, 30000);

    /* Clear additional fields */
    entry->id = 0;
    entry->sprite_id = 0;
    entry->element = 0;
    entry->flags = 0;
    entry->caught = 0;
    entry->caught_count = 0;
    entry->battle_count = 0;
    entry->kill_count = 0;
}

/*
 * Initialize album system - FUN_0044a100, FUN_00449a10
 */
int album_init(void) {
    FILE* fp;
    char header_key[16];
    int i;

    memset(&g_album, 0, sizeof(AlbumContext));

    /* Initialize all entries with random stats - matches FUN_00449a10 */
    for (i = 0; i < ALBUM_MAX_ENTRIES; i++) {
        album_init_random_stats(&g_album.entries[i]);
    }

    /* Set initial count - DAT_045e7af4 = 0x227 */
    g_album.entry_count = ALBUM_MAX_ENTRIES;

    /* Delete old cache files - from FUN_00449a10 */
    DeleteFileA("data\\album_2.dat");
    DeleteFileA("data\\album_3.dat");
    DeleteFileA("data\\album_10.dat");

    /* Try to load album_37.dat first - FUN_00449a10 priority */
    fp = fopen("data\\album_37.dat", "rb");
    if (fp) {
        /* Read header key */
        if (fread(header_key, 1, 16, fp) == 16) {
            album_decrypt(header_key, 16);

            /* Compare with encor marker - FUN_0048bb90 pattern */
            if (memcmp(header_key, s_encor_marker, 9) == 0) {
                /* Load all entries from album_37.dat */
                if (fread(g_album.entries, ALBUM_ENTRY_SIZE, ALBUM_COUNT_37, fp) == ALBUM_COUNT_37) {
                    /* Mark entries as valid */
                    for (i = 0; i < ALBUM_COUNT_37; i++) {
                        g_album.valid[i] = 1;
                    }
                    g_album.entry_count = ALBUM_COUNT_37;
                }
            }
        }
        fclose(fp);
    }

    /* Load all album files - FUN_0044a100 */
    album_load_all();

    g_album.initialized = 1;
    LOG_INFO("Album system initialized, %u entries loaded", g_album.entry_count);
    return 1;
}

/*
 * Shutdown album system
 */
void album_shutdown(void) {
    if (g_album.initialized) {
        /* Could save dirty entries here */
    }

    memset(&g_album, 0, sizeof(AlbumContext));
    LOG_INFO("Album system shutdown");
}

/*
 * Load single album file - FUN_00449e00
 * Returns 1 on success, 0 on failure
 */
int album_load_file(const char* filepath, u32 entry_count) {
    FILE* fp;
    char header_key[16];
    AlbumEntry entry;
    u32 base_index;
    u32 i;

    if (!filepath || entry_count == 0) {
        return 0;
    }

    fp = fopen(filepath, "rb");
    if (!fp) {
        return 0;
    }

    /* Read header key - 16 bytes */
    if (fread(header_key, 1, 16, fp) != 16) {
        fclose(fp);
        return 0;
    }

    /* Decrypt header with XOR key */
    album_decrypt(header_key, 16);

    /* Compare with encor marker - FUN_00449e00 string comparison */
    if (memcmp(header_key, s_encor_marker, 9) != 0) {
        /* Try to skip and read anyway */
        fseek(fp, 0, SEEK_SET);
    }

    /* Calculate base index for this file */
    base_index = g_album.entry_count;

    /* Read entries - FUN_00449e00 loop pattern */
    for (i = 0; i < entry_count; i++) {
        if (fread(&entry, ALBUM_ENTRY_SIZE, 1, fp) != 1) {
            break;
        }

        /* Decrypt entry */
        album_decrypt(&entry, ALBUM_ENTRY_SIZE);

        /* Store entry if index in range */
        if (base_index + i < ALBUM_MAX_ENTRIES) {
            memcpy(&g_album.entries[base_index + i], &entry, sizeof(AlbumEntry));
            g_album.valid[base_index + i] = 1;
        }
    }

    fclose(fp);

    /* Update count - DAT_045e7af4 */
    if (base_index + entry_count > g_album.entry_count) {
        g_album.entry_count = base_index + entry_count;
        if (g_album.entry_count > ALBUM_MAX_ENTRIES) {
            g_album.entry_count = ALBUM_MAX_ENTRIES;
        }
    }

    LOG_INFO("Loaded album file: %s (%u entries)", filepath, entry_count);
    return 1;
}

/*
 * Load all album files - FUN_0044a100
 */
int album_load_all(void) {
    int loaded = 0;
    u32 i;

    /* Load each album file in order */
    for (i = 0; i < ALBUM_FILE_COUNT; i++) {
        if (s_album_files[i].path) {
            if (album_load_file(s_album_files[i].path, s_album_files[i].entry_count)) {
                loaded++;
            }
        }
    }

    return loaded;
}

/*
 * Load single entry by index - FUN_00449910
 */
int album_load_entry(u32 index) {
    FILE* fp;
    AlbumEntry entry;
    u32 offset;

    if (index >= g_album.entry_count) {
        return 0;
    }

    fp = fopen("data\\album_37.dat", "rb");
    if (!fp) {
        return 0;
    }

    /* Calculate offset - 0x10 header + index * 0x50 */
    offset = 0x10 + index * ALBUM_ENTRY_SIZE;
    fseek(fp, offset, SEEK_SET);

    if (fread(&entry, ALBUM_ENTRY_SIZE, 1, fp) == 1) {
        album_decrypt(&entry, ALBUM_ENTRY_SIZE);
        memcpy(&g_album.entries[index], &entry, sizeof(AlbumEntry));
        g_album.valid[index] = 1;
    }

    fclose(fp);
    return 1;
}

/*
 * Add new entry to album
 */
int album_add_entry(AlbumEntry* entry) {
    u32 i;

    if (!entry) {
        return -1;
    }

    /* Find empty slot */
    for (i = 0; i < g_album.entry_count; i++) {
        if (!g_album.valid[i] || g_album.entries[i].name[0] == '\0') {
            memcpy(&g_album.entries[i], entry, sizeof(AlbumEntry));
            g_album.valid[i] = 1;
            return (int)i;
        }
    }

    /* Expand if possible */
    if (g_album.entry_count < ALBUM_MAX_ENTRIES) {
        i = g_album.entry_count;
        memcpy(&g_album.entries[i], entry, sizeof(AlbumEntry));
        g_album.valid[i] = 1;
        g_album.entry_count++;
        return (int)i;
    }

    return -1;
}

/*
 * Update existing entry
 */
int album_update_entry(u32 index, AlbumEntry* entry) {
    if (index >= g_album.entry_count || !entry) {
        return 0;
    }

    memcpy(&g_album.entries[index], entry, sizeof(AlbumEntry));
    g_album.valid[index] = 1;
    return 1;
}

/*
 * Get entry by index
 */
AlbumEntry* album_get_entry(u32 index) {
    if (index >= g_album.entry_count) {
        return NULL;
    }

    if (!g_album.valid[index]) {
        return NULL;
    }

    return &g_album.entries[index];
}

/*
 * Find entry by name
 */
int album_find_entry(const char* name) {
    u32 i;

    if (!name) {
        return -1;
    }

    for (i = 0; i < g_album.entry_count; i++) {
        if (g_album.valid[i] && g_album.entries[i].name[0] != '\0') {
            if (strcmp(g_album.entries[i].name, name) == 0) {
                return (int)i;
            }
        }
    }

    return -1;
}

/*
 * Find entry by creature ID
 */
int album_find_entry_by_id(u16 creature_id) {
    u32 i;

    for (i = 0; i < g_album.entry_count; i++) {
        if (g_album.valid[i] && g_album.entries[i].id == creature_id) {
            return (int)i;
        }
    }

    return -1;
}

/*
 * Find entry by sprite ID - FUN_0044a690
 */
int album_find_by_sprite(u16 sprite_id) {
    u32 i;

    /* Check sprite mapping table first */
    for (i = 0; i < s_sprite_map_count; i++) {
        if (s_sprite_map[i].sprite_id == sprite_id) {
            if (s_sprite_map[i].album_index >= 0) {
                return s_sprite_map[i].album_index;
            }
        }
    }

    return -1;
}

/*
 * Map sprite ID to album entry - FUN_0044a690
 */
void album_map_sprite(u16 sprite_id, u32 album_index) {
    if (s_sprite_map_count < ALBUM_MAX_ENTRIES && album_index < g_album.entry_count) {
        s_sprite_map[s_sprite_map_count].sprite_id = sprite_id;
        s_sprite_map[s_sprite_map_count].album_index = (s32)album_index;
        s_sprite_map_count++;
    }
}

/*
 * Get statistics
 */
u32 album_get_total_caught(void) {
    u32 total = 0;
    u32 i;

    for (i = 0; i < g_album.entry_count; i++) {
        if (g_album.valid[i]) {
            total += g_album.entries[i].caught_count;
        }
    }

    return total;
}

u32 album_get_total_battles(void) {
    u32 total = 0;
    u32 i;

    for (i = 0; i < g_album.entry_count; i++) {
        if (g_album.valid[i]) {
            total += g_album.entries[i].battle_count;
        }
    }

    return total;
}

u32 album_get_total_kills(void) {
    u32 total = 0;
    u32 i;

    for (i = 0; i < g_album.entry_count; i++) {
        if (g_album.valid[i]) {
            total += g_album.entries[i].kill_count;
        }
    }

    return total;
}

u32 album_get_completion_rate(void) {
    u32 valid_count = 0;
    u32 i;

    for (i = 0; i < g_album.entry_count; i++) {
        if (g_album.valid[i] && g_album.entries[i].caught) {
            valid_count++;
        }
    }

    if (g_album.entry_count == 0) {
        return 0;
    }

    return (valid_count * 100) / g_album.entry_count;
}

u32 album_get_count(void) {
    return g_album.entry_count;
}
