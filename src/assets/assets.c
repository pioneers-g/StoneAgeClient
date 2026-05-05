/*
 * Stone Age Client - Asset Loading Implementation
 * Reverse engineered from sa_9061.exe
 * FUN_0041fb10 - Sprite data loading
 * FUN_0041fc90 - Extended sprite loading
 * FUN_0041f900 - Sprite dimension lookup
 * FUN_0048a550 - Sprite data decoding
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "assets.h"
#include "assets_file.h"
#include "assets_sprite.h"
#include "config.h"
#include "logger.h"

/* Global asset context */
AssetContext g_assets = {0};

/* Maximum counts from binary analysis */
#define MAX_SPRITE_ID           550000
#define MAX_STANDARD_SPRITE     500000
#define MAX_EXTENDED_SPRITE     50000
#define SPRITE_INFO_ENTRY_SIZE  0x14   /* 20 bytes per entry */
#define SPRITE_DIM_ENTRY_SIZE   0x50   /* 80 bytes per entry */
#define EXTENDED_INFO_SIZE      0x28   /* 40 bytes per entry */

/* Sprite frame entry - matches FUN_0041fe20 stack layout */
#pragma pack(push, 1)
typedef struct {
    u16 width;
    u16 height;
    u32 data_offset;    /* Combined from two u16 values */
    u32 data_size;      /* Combined from two u16 values */
} SpriteFrameEntry;

/* Sprite header entry - matches FUN_0041fe20 processing */
typedef struct {
    u32 id;
    u32 flags;
    u16 frame_count;
    u16 reserved;
} SpriteFileHeader;

/* Sprite info entry - DAT_00e8f228 format (0x14 bytes) */
typedef struct {
    u32 file_index;     /* +0x00: File index for sprite data */
    u32 data_offset;    /* +0x04: Offset into sprite file */
    u32 sprite_offset;  /* +0x08: Secondary offset */
    u32 data_size;      /* +0x0c: Compressed data size */
    u16 flags;          /* +0x10: Sprite flags */
    u16 reserved;       /* +0x12: Reserved */
} SpriteInfoEntry;

/*
 * Initialize assets - FUN_0041e260 pattern
 */
int assets_init(void) {
    memset(&g_assets, 0, sizeof(AssetContext));

    LOG_INFO("Loading game assets...");

    /* Allocate sprite arrays first */
    g_assets.sprites = (SpriteEntry*)malloc(MAX_SPRITE_ID * sizeof(SpriteEntry));
    if (!g_assets.sprites) {
        LOG_ERROR("Failed to allocate sprite array");
        return 0;
    }
    memset(g_assets.sprites, 0, MAX_SPRITE_ID * sizeof(SpriteEntry));

    /* Allocate index arrays */
    g_assets.real_index = (u32*)malloc(MAX_REAL_ID * sizeof(u32));
    g_assets.adrn_index = (u32*)malloc(MAX_ADDRESS_ID * sizeof(u32));

    if (!g_assets.real_index || !g_assets.adrn_index) {
        LOG_ERROR("Failed to allocate index arrays");
        return 0;
    }

    memset(g_assets.real_index, 0, MAX_REAL_ID * sizeof(u32));
    memset(g_assets.adrn_index, 0, MAX_ADDRESS_ID * sizeof(u32));

    /* Load main data files */
    if (!assets_load_real(g_config.real_bin_path)) {
        LOG_WARN("Failed to load real.bin, trying default path");
        if (!assets_load_real(PATH_REALTRUE_BIN)) {
            LOG_ERROR("Failed to load real.bin from all paths");
        }
    }

    if (!assets_load_adrn(g_config.adrn_bin_path)) {
        LOG_WARN("Failed to load adrn.bin, trying default path");
        if (!assets_load_adrn(PATH_ADRNTRUE_BIN)) {
            LOG_ERROR("Failed to load adrn.bin from all paths");
        }
    }

    if (!assets_load_spr(g_config.spr_bin_path)) {
        LOG_ERROR("Failed to load spr.bin");
        return 0;
    }

    if (!assets_load_spradrn(g_config.spradrn_bin_path)) {
        LOG_WARN("Failed to load spradrn.bin");
    }

    /* Load palette */
    assets_load_palette(PATH_PALETTE);

    LOG_INFO("Assets loaded: %u sprites, %u addresses, %u real entries",
        g_assets.sprite_count, g_assets.adrn_count, g_assets.real_count);

    return 1;
}

void assets_shutdown(void) {
    if (g_assets.real_data) {
        free(g_assets.real_data);
        g_assets.real_data = NULL;
    }
    if (g_assets.adrn_data) {
        free(g_assets.adrn_data);
        g_assets.adrn_data = NULL;
    }
    if (g_assets.spr_data) {
        free(g_assets.spr_data);
        g_assets.spr_data = NULL;
    }
    if (g_assets.spradrn_data) {
        free(g_assets.spradrn_data);
        g_assets.spradrn_data = NULL;
    }
    if (g_assets.sprite_cache) {
        sprite_cache_shutdown();
        g_assets.sprite_cache = NULL;
    }
    if (g_assets.sprites) {
        free(g_assets.sprites);
        g_assets.sprites = NULL;
    }
    if (g_assets.real_index) {
        free(g_assets.real_index);
        g_assets.real_index = NULL;
    }
    if (g_assets.adrn_index) {
        free(g_assets.adrn_index);
        g_assets.adrn_index = NULL;
    }
    if (g_assets.decode_buffer) {
        free(g_assets.decode_buffer);
        g_assets.decode_buffer = NULL;
    }

    memset(&g_assets, 0, sizeof(AssetContext));
    LOG_INFO("Assets unloaded");
}

/*
 * Load real.bin - FUN_0041fbb0 pattern
 */
int assets_load_real(const char* path) {
    void* fh;
    u32 i;

    fh = file_open_read(path);
    if (!fh) {
        return 0;
    }

    /* Allocate real data array - 500000 entries max from binary */
    g_assets.real_data = malloc(500000 * sizeof(RealEntry));
    if (!g_assets.real_data) {
        file_close(fh);
        return 0;
    }

    memset(g_assets.real_index, 0, MAX_REAL_ID * sizeof(u32));

    /* Read entries */
    g_assets.real_count = 0;

    while (!file_eof(fh) && g_assets.real_count < 500000) {
        RealEntry entry;
        int bytes_read = file_read(fh, &entry, sizeof(RealEntry));

        if (bytes_read != sizeof(RealEntry)) {
            break;
        }

        /* Store entry and update index */
        ((RealEntry*)g_assets.real_data)[g_assets.real_count] = entry;

        if (entry.id < MAX_REAL_ID) {
            g_assets.real_index[entry.id] = g_assets.real_count;
        }

        g_assets.real_count++;
    }

    file_close(fh);

    LOG_INFO("Loaded %u real entries from %s", g_assets.real_count, path);
    return 1;
}

/*
 * Load adrn.bin - FUN_0041f7d0 pattern
 */
int assets_load_adrn(const char* path) {
    void* fh;
    AddressEntry entry;
    int bytes_read;

    fh = file_open_read(path);
    if (!fh) {
        return 0;
    }

    /* Allocate address data */
    g_assets.adrn_data = malloc(0x8000 * sizeof(AddressEntry));
    if (!g_assets.adrn_data) {
        file_close(fh);
        return 0;
    }

    memset(g_assets.adrn_index, 0, MAX_ADDRESS_ID * sizeof(u32));

    g_assets.adrn_count = 0;

    /* Read entries matching FUN_0041f7d0 pattern */
    while (!file_eof(fh) && g_assets.adrn_count < 0x8000) {
        bytes_read = file_read(fh, &entry, sizeof(AddressEntry));

        if (bytes_read != sizeof(AddressEntry)) {
            break;
        }

        /* Store entry */
        ((AddressEntry*)g_assets.adrn_data)[g_assets.adrn_count] = entry;

        /* Update index - FUN_0041f7d0 pattern */
        if (entry.id < MAX_ADDRESS_ID) {
            g_assets.adrn_index[entry.id] = g_assets.adrn_count;
        }

        /* Special value adjustments from binary */
        if ((entry.id > 0x3201 && entry.id < 0x320c) ||
            (entry.id > 0x2793 && entry.id < 0x2799)) {
            ((AddressEntry*)g_assets.adrn_data)[g_assets.adrn_count].flags =
                (entry.flags % 100) + 300;
        }

        g_assets.adrn_count++;
    }

    file_close(fh);

    LOG_INFO("Loaded %u address entries from %s", g_assets.adrn_count, path);
    return 1;
}

/*
 * Load spr.bin - FUN_0041fe20 pattern
 */
int assets_load_spr(const char* path) {
    void* fh;
    u32 file_size;
    u32 sprite_id;
    u32 j;
    int bytes_read;
    u8* data_ptr;
    u32 data_offset;
    u32 sprite_count;

    fh = file_open_read(path);
    if (!fh) {
        return 0;
    }

    file_size = file_get_size(fh);

    /* Allocate sprite data buffer */
    g_assets.spr_data = malloc(file_size);
    g_assets.spr_size = file_size;

    if (!g_assets.spr_data) {
        file_close(fh);
        return 0;
    }

    /* Read entire file into memory */
    bytes_read = file_read(fh, g_assets.spr_data, file_size);
    if (bytes_read != (int)file_size) {
        free(g_assets.spr_data);
        g_assets.spr_data = NULL;
        file_close(fh);
        return 0;
    }

    /* Initialize sprite tracking structures */
    g_assets.sprite_count = 0;
    data_ptr = (u8*)g_assets.spr_data;
    data_offset = 0;
    sprite_count = 0;

    /* Parse sprites - FUN_0041fe20 pattern */
    for (sprite_id = 0; sprite_id < MAX_SPRITE_ID && data_offset < file_size; sprite_id++) {
        SpriteEntry* sprite = &g_assets.sprites[sprite_id];
        u8* sprite_header;
        u32 frame_size;
        u16 frame_count;
        u16 flags;

        /* Check if we have enough data for header */
        if (data_offset + 12 > file_size) break;

        sprite_header = data_ptr + data_offset;

        /* Read sprite info */
        sprite->id = *(u32*)(sprite_header);
        flags = *(u16*)(sprite_header + 4);
        frame_count = *(u16*)(sprite_header + 6);

        sprite->frame_count = frame_count;
        sprite->flags = flags;
        sprite->data_offset = data_offset;

        /* Calculate sprite dimensions from first frame */
        if (frame_count > 0 && data_offset + 12 + 16 <= file_size) {
            u8* frame_ptr = sprite_header + 12;
            sprite->width = *(u16*)(frame_ptr);
            sprite->height = *(u16*)(frame_ptr + 2);
            sprite->offset_x = *(s16*)(frame_ptr + 4);
            sprite->offset_y = *(s16*)(frame_ptr + 6);
        }

        /* Calculate total size for all frames */
        frame_size = 12 + (frame_count * 16);

        /* Special sprite ID adjustments */
        if ((sprite->id >= 0x0390f0f3 && sprite->id <= 0x0390f24d) ||
            (sprite->id >= 0x0390f273 && sprite->id <= 0x0390f27d)) {
            sprite->flags |= SPRITE_FLAG_ADJUSTED;
        }

        sprite->data_size = frame_size;
        data_offset += frame_size;
        sprite_count++;

        /* Skip frame pixel data */
        if (flags & 0x10) {
            u32 pixel_data_size = 0;
            for (j = 0; j < frame_count && data_offset - frame_size + 12 + j * 12 < file_size; j++) {
                u8* frame_info = data_ptr + data_offset - frame_size + 12 + j * 12;
                pixel_data_size += *(u16*)(frame_info + 8) * *(u16*)(frame_info + 10);
            }
            data_offset += pixel_data_size;
        }
    }

    g_assets.sprite_count = sprite_count;
    file_close(fh);

    LOG_INFO("Loaded spr.bin: %u bytes, %u sprites", file_size, sprite_count);
    return 1;
}

/*
 * Load spradrn.bin - sprite address index
 */
int assets_load_spradrn(const char* path) {
    void* fh;
    u32 file_size;

    fh = file_open_read(path);
    if (!fh) {
        return 0;
    }

    file_size = file_get_size(fh);

    g_assets.spradrn_data = malloc(file_size);
    if (!g_assets.spradrn_data) {
        file_close(fh);
        return 0;
    }

    g_assets.spradrn_count = file_size / 4;

    if (file_read(fh, g_assets.spradrn_data, file_size) != (int)file_size) {
        file_close(fh);
        return 0;
    }

    file_close(fh);

    LOG_INFO("Loaded spradrn.bin: %u entries", g_assets.spradrn_count);
    return 1;
}

/*
 * Load palette - FUN_00412220 pattern
 */
int assets_load_palette(const char* path) {
    void* fh;
    int i;
    u8 rgb[3];
    int bytes_read;

    fh = file_open_read(path);
    if (!fh) {
        /* Use default grayscale palette */
        for (i = 0; i < 256; i++) {
            g_assets.palette[i * 4 + 0] = (u8)i;  /* B */
            g_assets.palette[i * 4 + 1] = (u8)i;  /* G */
            g_assets.palette[i * 4 + 2] = (u8)i;  /* R */
            g_assets.palette[i * 4 + 3] = 0;      /* Reserved */
        }
        g_assets.palette_loaded = 1;
        LOG_INFO("Using default grayscale palette");
        return 1;
    }

    /* Read 256 RGB triplets */
    for (i = 0; i < 256; i++) {
        bytes_read = file_read(fh, rgb, 3);
        if (bytes_read != 3) {
            break;
        }
        /* Convert to BGRA format */
        g_assets.palette[i * 4 + 0] = rgb[2];  /* B */
        g_assets.palette[i * 4 + 1] = rgb[1];  /* G */
        g_assets.palette[i * 4 + 2] = rgb[0];  /* R */
        g_assets.palette[i * 4 + 3] = 0;       /* Reserved */
    }

    g_assets.palette_loaded = 1;
    file_close(fh);

    LOG_INFO("Loaded palette from %s", path);
    return 1;
}

/*
 * Get sprite by ID
 */
SpriteEntry* assets_get_sprite(u32 id) {
    if (id >= g_assets.sprite_count || !g_assets.sprites) {
        return NULL;
    }
    return &g_assets.sprites[id];
}

/*
 * Get sprite data by ID
 */
SpriteData* assets_get_sprite_data(u32 id) {
    if (id >= MAX_SPRITE_ID || !g_assets.spr_data) {
        return NULL;
    }

    SpriteEntry* entry = assets_get_sprite(id);
    if (!entry) return NULL;

    static SpriteData temp_data;
    temp_data.id = entry->id;
    temp_data.frame_count = entry->frame_count;
    temp_data.animation_count = 0;
    temp_data.total_size = entry->data_size;

    return &temp_data;
}

/*
 * Get sprite frame data
 */
void* assets_get_sprite_frame(u32 sprite_id, u32 frame_index) {
    SpriteEntry* sprite = assets_get_sprite(sprite_id);
    if (!sprite || !g_assets.spr_data) {
        return NULL;
    }
    if (frame_index >= sprite->frame_count) {
        frame_index = 0;
    }

    u32 frame_offset = sprite->data_offset + 8 + (frame_index * 16);
    if (frame_offset >= g_assets.spr_size) {
        return NULL;
    }

    return (u8*)g_assets.spr_data + frame_offset;
}

/*
 * Get sprite dimensions
 */
int assets_get_sprite_dimensions(u32 id, int* width, int* height) {
    SpriteEntry* sprite = assets_get_sprite(id);
    if (!sprite) {
        return 0;
    }
    if (width) *width = sprite->width;
    if (height) *height = sprite->height;
    return 1;
}

/*
 * Get address entry
 */
AddressEntry* assets_get_address(u32 id) {
    if (id >= MAX_ADDRESS_ID || !g_assets.adrn_index || !g_assets.adrn_data) {
        return NULL;
    }
    u32 index = g_assets.adrn_index[id];
    if (index >= g_assets.adrn_count) {
        return NULL;
    }
    return &((AddressEntry*)g_assets.adrn_data)[index];
}

/*
 * Get real entry
 */
RealEntry* assets_get_real(u32 id) {
    if (id >= MAX_REAL_ID || !g_assets.real_index || !g_assets.real_data) {
        return NULL;
    }
    u32 index = g_assets.real_index[id];
    return &((RealEntry*)g_assets.real_data)[index];
}

/* Extended sprite file handle */
static HANDLE s_extended_sprite_file = INVALID_HANDLE_VALUE;
static ExtendedSpriteInfo* s_extended_sprite_info = NULL;
static u32 s_extended_sprite_count = 0;

/*
 * Load sprite from standard sprite info - FUN_0041fb10 pattern
 */
static int load_sprite_from_info(u32 sprite_id, void** out_data, u32* out_width, u32* out_height) {
    SpriteEntry* sprite;
    u8* src_data;
    u32 src_size;
    u8* paletted_data;
    u32 paletted_size;
    void* rgb565_data;
    u32 rgb565_size;
    int width, height;

    if (sprite_id >= MAX_STANDARD_SPRITE) {
        return 0;
    }

    if (!g_assets.spr_data) {
        return 0;
    }

    sprite = assets_get_sprite(sprite_id);
    if (!sprite) {
        return 0;
    }

    /* Decode sprite as 8-bit paletted (width*height bytes) */
    paletted_size = sprite->width * sprite->height;
    paletted_data = (u8*)malloc(paletted_size);
    if (!paletted_data) {
        return 0;
    }

    src_data = (u8*)g_assets.spr_data + sprite->data_offset;
    src_size = sprite->data_size;

    if (!sprite_decode_from_data(src_data, src_size, paletted_data, paletted_size,
                                  &width, &height, &paletted_size)) {
        free(paletted_data);
        return 0;
    }

    /* Convert 8-bit paletted to 16-bit RGB565 using global palette */
    if (g_assets.palette_loaded) {
        rgb565_data = sprite_apply_palette(paletted_data, (u32)(width * height),
                                            g_assets.palette, &rgb565_size);
        free(paletted_data);
        if (!rgb565_data) {
            return 0;
        }
    } else {
        /* No palette: treat decoded data as raw 16-bit (fallback) */
        rgb565_data = paletted_data;
        rgb565_size = paletted_size;
    }

    *out_data = rgb565_data;
    *out_width = (u32)width;
    *out_height = (u32)height;
    return 1;
}

/*
 * Load extended sprite (500000+) - FUN_0041fc90 pattern
 */
static int load_extended_sprite(u32 sprite_id, void** out_data, u32* out_width, u32* out_height, int* has_extra) {
    u32 ext_index;
    ExtendedSpriteInfo* info;
    u8* sprite_data;
    DWORD bytes_read;
    BOOL success;
    u8* paletted_data;
    u32 paletted_size;
    void* rgb565_data;
    u32 rgb565_size;
    int width, height;

    ext_index = sprite_id - 500000;
    if (ext_index >= MAX_EXTENDED_SPRITE) {
        return 0;
    }

    /* Open extended sprite file if needed */
    if (s_extended_sprite_file == INVALID_HANDLE_VALUE) {
        char path[MAX_PATH];

        GetModuleFileNameA(NULL, path, MAX_PATH);
        {
            char* last_slash = strrchr(path, '\\');
            if (last_slash) {
                strcpy(last_slash + 1, "data\\extsprite.bin");
            } else {
                strcpy(path, "data\\extsprite.bin");
            }
        }

        s_extended_sprite_file = CreateFileA(path, GENERIC_READ,
            FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (s_extended_sprite_file == INVALID_HANDLE_VALUE) {
            LOG_WARN("Extended sprite file not found");
            return 0;
        }
    }

    /* Get extended sprite info */
    if (s_extended_sprite_info && ext_index < s_extended_sprite_count) {
        info = &s_extended_sprite_info[ext_index];
    } else {
        *out_width = 64;
        *out_height = 64;
        *has_extra = 0;
        return 0;
    }

    SetFilePointer(s_extended_sprite_file, info->file_offset, NULL, FILE_BEGIN);

    /* Read palette data first - FUN_0041fc90 reads palette_size+4 bytes */
    {
        u8 ext_palette[1024];
        u32 pal_size = info->palette_size + 4;
        if (pal_size > sizeof(ext_palette)) pal_size = sizeof(ext_palette);
        ReadFile(s_extended_sprite_file, ext_palette, pal_size, &bytes_read, NULL);
    }

    /* Read sprite data */
    sprite_data = (u8*)malloc(info->data_size);
    if (!sprite_data) {
        return 0;
    }

    success = ReadFile(s_extended_sprite_file, sprite_data, info->data_size, &bytes_read, NULL);
    if (!success || bytes_read == 0) {
        free(sprite_data);
        return 0;
    }

    /* Decode sprite as 8-bit paletted */
    paletted_size = 64 * 64;
    paletted_data = (u8*)malloc(paletted_size);
    if (!paletted_data) {
        free(sprite_data);
        return 0;
    }

    if (!sprite_decode_from_data(sprite_data, info->data_size, paletted_data, paletted_size,
                                  &width, &height, &paletted_size)) {
        free(sprite_data);
        free(paletted_data);
        return 0;
    }

    free(sprite_data);

    /* Convert to 16-bit RGB565 using global palette */
    if (g_assets.palette_loaded) {
        rgb565_data = sprite_apply_palette(paletted_data, (u32)(width * height),
                                            g_assets.palette, &rgb565_size);
        free(paletted_data);
        if (!rgb565_data) {
            return 0;
        }
    } else {
        rgb565_data = paletted_data;
        rgb565_size = paletted_size;
    }

    *out_data = rgb565_data;
    *out_width = (u32)width;
    *out_height = (u32)height;

    if (has_extra && info->extra_size > 0) {
        *has_extra = 1;
    }

    return 1;
}

/*
 * Load and decode sprite by ID
 */
void* assets_load_and_decode_sprite(u32 sprite_id, int* width, int* height) {
    void* decoded_data = NULL;
    u32 out_width = 0, out_height = 0;

    if (sprite_id < MAX_STANDARD_SPRITE) {
        if (!load_sprite_from_info(sprite_id, &decoded_data, &out_width, &out_height)) {
            return NULL;
        }
    } else if (sprite_id < MAX_SPRITE_ID) {
        int has_extra = 0;
        if (!load_extended_sprite(sprite_id, &decoded_data, &out_width, &out_height, &has_extra)) {
            return NULL;
        }
    } else {
        return NULL;
    }

    if (width) *width = (int)out_width;
    if (height) *height = (int)out_height;

    return decoded_data;
}
