/*
 * Stone Age Client - Asset Loading Module
 * Reverse engineered from sa_9061.exe
 * FUN_0041fb10 - Sprite data loading
 * FUN_0041fc90 - Extended sprite loading
 * FUN_0041f900 - Sprite dimension lookup
 * FUN_0048a550 - Sprite data decoding
 */

#ifndef ASSETS_H
#define ASSETS_H

#include "types.h"

/* Asset file paths */
#define PATH_REAL_BIN       "data/real.bin"
#define PATH_ADRN_BIN       "data/adrn.bin"
#define PATH_SPR_BIN        "data/spr.bin"
#define PATH_SPRADRN_BIN    "data/spradrn.bin"
#define PATH_REALTRUE_BIN   "data/realtrue.bin"
#define PATH_ADRNTRUE_BIN   "data/adrntrue.bin"
#define PATH_EXTSPRITE_BIN  "data/extsprite.bin"
#define PATH_TOKYOHOT_DAT   "tokyohot.dat"
#define PATH_PALETTE        "data/pal/Palet_1.sap"

/* Sprite ID ranges - from FUN_0041f900, FUN_0041fb10, FUN_0041fc90 */
#define SPRITE_ID_STANDARD_MAX  500000   /* Standard sprites: 0-499999 */
#define SPRITE_ID_EXTENDED_MAX  550000   /* Extended sprites: 500000-549999 */
#define SPRITE_ID_INVALID       550000   /* Invalid: >= 550000 */

/* Maximum counts for index arrays */
#define MAX_REAL_ID             100000   /* Maximum real.bin entries */
#define MAX_ADDRESS_ID          100000   /* Maximum adrn.bin entries */

/* Sprite flags */
#define SPRITE_FLAG_COMPRESSED  0x01
#define SPRITE_FLAG_ALPHA       0x02
#define SPRITE_FLAG_RLE         0x04
#define SPRITE_FLAG_ADJUSTED    0x10  /* Special sprite needing ID adjustment */

/* Sprite info entry size - DAT_00e8f228 */
#define SPRITE_INFO_SIZE        0x14   /* 20 bytes */

/* Sprite dimension entry size - DAT_00e8f234 */
#define SPRITE_DIM_SIZE         0x50   /* 80 bytes */
#define SPRITE_DIM_ENTRY_SIZE   SPRITE_DIM_SIZE  /* Alias for tests */

/* Extended sprite info size - DAT_0081c7e4 */
#define EXTENDED_INFO_SIZE      0x28   /* 40 bytes */

/* Sprite frame structure */
typedef struct {
    u16 width;
    u16 height;
    s16 offset_x;
    s16 offset_y;
    u32 data_offset;
    u32 data_size;
    u8  flags;
    u8  compression;
    u16 reserved;
} SpriteFrame;

/* Sprite structure - matches binary format */
typedef struct {
    u32 id;
    u16 frame_count;
    u16 animation_count;
    SpriteFrame frames[16];
    u32 total_size;
} SpriteData;

/* Sprite entry for cache - used by render system */
typedef struct {
    u32 id;
    u16 width;
    u16 height;
    s16 offset_x;
    s16 offset_y;
    u32 data_offset;
    u32 data_size;
    u8  flags;
    u8  compression;
    u16 frame_count;
} SpriteEntry;

/* Address entry - matches binary format */
#pragma pack(push, 1)
typedef struct {
    u32 id;
    u32 sprite_id;
    u16 x;
    u16 y;
    u16 width;
    u16 height;
    u8  direction;
    u8  frame;
    u16 flags;
    u32 extra[4];
} AddressEntry;
#pragma pack(pop)

/* Real data entry */
#pragma pack(push, 1)
typedef struct {
    u32 id;
    u32 sprite_id;
    u16 x;
    u16 y;
    u16 width;
    u16 height;
    u8  direction;
    u8  frame_index;
    u16 animation_id;
    u16 palette_id;
    u16 flags;
    u8  alpha;
    u8  reserved[5];
} RealEntry;
#pragma pack(pop)

/* Extended sprite info - DAT_0081c7e4 format (0x28 bytes) */
#pragma pack(push, 1)
typedef struct {
    u32 file_offset;    /* +0x00: Offset in extended sprite file */
    u32 palette_size;   /* +0x04: Palette data size + 4 */
    u32 data_size;      /* +0x08: Sprite data size */
    u32 extra_offset;   /* +0x0c: Extra data offset */
    u32 extra_size;     /* +0x10: Extra data size */
    u8  reserved[20];   /* +0x14: Reserved padding */
} ExtendedSpriteInfo;
#pragma pack(pop)

/* Extended sprite dimension - DAT_0081c7f4 format (40 bytes) */
#pragma pack(push, 1)
typedef struct {
    u16 width;          /* +0x00 */
    u16 height;         /* +0x02 */
    u8  reserved[36];   /* +0x04 */
} ExtendedSpriteDim;
#pragma pack(pop)

/* Decoded sprite cache entry (in-memory cache) */
typedef struct {
    u32 sprite_id;
    void* decoded_data;
    u32 data_size;
    u16 width;
    u16 height;
    u8  is_valid;
    u8  flags;
    u32 last_used;
} DecodedSpriteCacheEntry;

/* Asset context */
typedef struct {
    /* real.bin data */
    void* real_data;
    u32 real_count;
    u32* real_index;

    /* adrn.bin data */
    void* adrn_data;
    u32 adrn_count;
    u32* adrn_index;

    /* spr.bin data */
    void* spr_data;
    u32 spr_size;
    u32 sprite_count;

    /* Sprites */
    SpriteEntry* sprites;
    u32 sprites_size;

    /* spradrn.bin data */
    void* spradrn_data;
    u32 spradrn_count;

    /* Extended sprite info */
    void* extended_info;
    u32 extended_count;

    /* Palette */
    u8 palette[256 * 4];
    int palette_loaded;

    /* Sprite cache */
    DecodedSpriteCacheEntry* sprite_cache;
    u32 cache_size;
    u32 cache_used;

    /* Decoded sprite buffer */
    void* decode_buffer;
    u32 decode_buffer_size;

} AssetContext;

/* Global asset context */
extern AssetContext g_assets;

/* Asset loading functions */
int assets_init(void);
void assets_shutdown(void);

/* Load data files */
int assets_load_real(const char* path);
int assets_load_adrn(const char* path);
int assets_load_spr(const char* path);
int assets_load_spradrn(const char* path);
int assets_load_palette(const char* path);

/* Sprite access */
SpriteEntry* assets_get_sprite(u32 id);
SpriteData* assets_get_sprite_data(u32 id);
void* assets_get_sprite_frame(u32 sprite_id, u32 frame_index);
int assets_get_sprite_dimensions(u32 id, int* width, int* height);

/* Address lookup */
AddressEntry* assets_get_address(u32 id);
RealEntry* assets_get_real(u32 id);

/* Sprite decoding */
int sprite_decode_rle(void* src, u32 src_size, void* dst, u32 dst_size, int width, int height);
int sprite_decode_raw(void* src, u32 src_size, void* dst, u32 dst_size);
int sprite_decode_alpha(void* src, u32 src_size, void* dst, u32 dst_size, int width, int height, u8 alpha);
int sprite_decode_from_data(void* src_data, u32 src_size, void* dst_buffer,
                            u32 dst_size, int* out_width, int* out_height, u32* out_data_size);
void* assets_load_and_decode_sprite(u32 sprite_id, int* width, int* height);

/* Sprite cache */
int sprite_cache_init(u32 max_sprites);
void sprite_cache_shutdown(void);
DecodedSpriteCacheEntry* decoded_sprite_cache_get(u32 sprite_id);
void sprite_cache_invalidate(u32 sprite_id);

/* File I/O helpers - from FUN_00492394 */
void* file_open_read(const char* path);
int file_read(void* handle, void* buffer, u32 size);
void file_close(void* handle);
int file_read_entry(void* handle, void* entry, u32 size);
int file_eof(void* handle);
u32 file_get_size(void* handle);

#endif /* ASSETS_H */
