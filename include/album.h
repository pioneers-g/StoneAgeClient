/*
 * Stone Age Client - Album System Header
 * Creature/Pet collection system (Pet Album/Bestiary)
 * Reverse engineered from sa_9061.exe:
 *   FUN_0044a100 - Main album loader
 *   FUN_00449e00 - Album file reader with XOR decryption
 *   FUN_00449f70 - data/album.dat loader
 *   FUN_00449910 - Single entry reader from album_37.dat
 *   FUN_00449a10 - Album initialization with random stats
 *   FUN_0044a690 - Sprite ID to album entry mapping
 */

#ifndef ALBUM_H
#define ALBUM_H

#include "types.h"

/* Constants from binary analysis */
#define ALBUM_MAX_ENTRIES       551     /* 0x227 - total entries at DAT_045837c8 */
#define ALBUM_ENTRY_SIZE        80      /* 0x50 bytes per entry */
#define ALBUM_HEADER_SIZE       16      /* 0x10 bytes header */
#define ALBUM_NAME_LENGTH       17      /* 0x11 bytes for name field (from FUN_00449a10) */
#define ALBUM_MAX_FILES         38      /* album.dat + album_2.dat to album_37.dat */

/* Entry counts per album file - from FUN_0044a100 */
#define ALBUM_COUNT_37          0x1ee   /* 494 entries in album_37.dat */
#define ALBUM_COUNT_36          0x1e3   /* 483 entries */
#define ALBUM_COUNT_35          0x1ce   /* 462 entries */
#define ALBUM_COUNT_34          0x1b4   /* 436 entries */
#define ALBUM_COUNT_33          0x1b0   /* 432 entries */
#define ALBUM_COUNT_ALBUM_DAT   0x7c    /* 124 entries in album.dat */

/* Random stat ranges from FUN_00449a10 */
#define ALBUM_RAND_STAT_MIN     30000
#define ALBUM_RAND_STAT_MAX     60000   /* For HP */
#define ALBUM_RAND_OTHER_MIN    1
#define ALBUM_RAND_OTHER_MAX    30000   /* For other stats */

/*
 * Album entry structure - 0x50 (80) bytes
 * From FUN_00449a10 analysis:
 *   - 0x11 bytes for name (padded to align)
 *   - Random initialization for 10 stat fields
 *   - Entry valid flag at offset +0x10 from base
 *
 * Memory layout at DAT_045837c8:
 *   Each entry is 0x50 bytes, accessed as 0x14 dwords
 */
typedef struct {
    /* 0x00-0x10: Name and identification */
    char name[ALBUM_NAME_LENGTH];     /* 0x00: Pet/creature name (0x11 bytes) */
    u8  padding1[3];                  /* 0x11-0x13: Padding to align */

    /* 0x14-0x3c: Stats (10 values, randomly initialized in FUN_00449a10) */
    u32 stat_1;                       /* 0x14: Random 30000-60000 (HP base?) */
    u32 stat_2;                       /* 0x18: Random 1-30000 */
    u32 stat_3;                       /* 0x1c: Random 1-30000 */
    u32 stat_4;                       /* 0x20: Random 1-30000 */
    u32 stat_5;                       /* 0x24: Random 1-30000 */
    u32 stat_6;                       /* 0x28: Random 1-30000 */
    u32 stat_7;                       /* 0x2c: Random 1-30000 */
    u32 stat_8;                       /* 0x30: Random 1-30000 */
    u32 stat_9;                       /* 0x34: Random 1-30000 */
    u32 stat_10;                      /* 0x38: Random 1-30000 */

    /* 0x3c-0x4f: Additional data */
    u16 id;                           /* 0x3c: Creature ID */
    u16 sprite_id;                    /* 0x3e: Sprite ID for display */
    u8  element;                      /* 0x40: Element type */
    u8  flags;                        /* 0x41: Entry flags */
    u8  caught;                       /* 0x42: Has been caught flag */
    u8  padding2;                     /* 0x43: Padding */
    u32 caught_count;                 /* 0x44: Times caught */
    u32 battle_count;                 /* 0x48: Battles participated */
    u32 kill_count;                   /* 0x4c: Kills */
} AlbumEntry;

/* Album file header - 16 bytes from FUN_00449e00 */
typedef struct {
    u32 magic;                        /* Magic/checksum after XOR decrypt */
    u16 version;                      /* Version */
    u16 entry_count;                  /* Number of entries */
    u32 checksum;                     /* Checksum */
    u32 reserved;                     /* Reserved */
} AlbumHeader;

/* Sprite-to-album mapping - from DAT_004bd438 */
typedef struct {
    s32 album_index;                  /* -1 if unmapped, else index into album */
    u16 sprite_id;                    /* Sprite ID */
    u16 padding;
} AlbumSpriteMapping;

/* Album file context */
typedef struct {
    AlbumEntry entries[ALBUM_MAX_ENTRIES];
    u8 valid[ALBUM_MAX_ENTRIES];
    u32 entry_count;                  /* Actual count from file */
    u32 total_caught;
    u32 total_battles;
    u32 total_kills;
    u8 dirty;
    char filepath[64];
} AlbumFile;

/* Album system context - matches binary data locations */
typedef struct {
    AlbumEntry entries[ALBUM_MAX_ENTRIES];  /* DAT_045837c8 */
    u8 valid[ALBUM_MAX_ENTRIES];            /* Entry validity flags */
    u32 entry_count;                        /* DAT_045e7af4 - total entries */
    u32 retry_count;                        /* DAT_045f1a70 - retry counter */
    u8 initialized;
    char last_error[64];
} AlbumContext;

/* Global album context */
extern AlbumContext g_album;

/* XOR encryption key - from DAT_004c107c (16 bytes) */
extern const u8 g_album_xor_key[16];

/* Initialization - FUN_0044a100, FUN_00449a10 */
int album_init(void);
void album_shutdown(void);

/* Load functions - FUN_00449e00, FUN_00449f70, FUN_00449910 */
int album_load_file(const char* filepath, u32 entry_count);
int album_load_all(void);
int album_load_entry(u32 index);

/* Entry management */
int album_add_entry(AlbumEntry* entry);
int album_update_entry(u32 index, AlbumEntry* entry);
AlbumEntry* album_get_entry(u32 index);
int album_find_entry(const char* name);
int album_find_entry_by_id(u16 creature_id);
int album_find_by_sprite(u16 sprite_id);

/* Sprite mapping - FUN_0044a690 */
void album_map_sprite(u16 sprite_id, u32 album_index);

/* Random initialization - FUN_00449a10 */
void album_init_random_stats(AlbumEntry* entry);

/* XOR encryption - matches FUN_00449e00 pattern */
void album_encrypt(void* data, u32 size);
void album_decrypt(void* data, u32 size);

/* Statistics */
u32 album_get_total_caught(void);
u32 album_get_total_battles(void);
u32 album_get_total_kills(void);
u32 album_get_completion_rate(void);
u32 album_get_count(void);

#endif /* ALBUM_H */
