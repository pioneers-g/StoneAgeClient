/*
 * Stone Age Client - Sprite Decoding and Cache
 * RLE decompression and sprite cache management
 * Reverse engineered from FUN_0048a550, FUN_0041fb10, FUN_0041fc90
 */

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "assets.h"
#include "assets_sprite.h"
#include "logger.h"

/* External reference to main asset context */
extern AssetContext g_assets;

/*
 * Decode sprite from compressed data - RLE variant
 */
int sprite_decode_rle(void* src, u32 src_size, void* dst, u32 dst_size, int width, int height) {
    u8* src_ptr = (u8*)src;
    u16* dst_ptr = (u16*)dst;
    u32 src_pos = 0;
    u32 dst_pos = 0;
    int x, y;
    u8 count;
    u16 pixel;

    /* 16-bit RLE decompression */
    for (y = 0; y < height; y++) {
        x = 0;

        while (x < width && src_pos + 1 < src_size) {
            count = src_ptr[src_pos++];

            if (count & 0x80) {
                /* Run-length encoded: repeat pixel */
                count &= 0x7F;
                if (src_pos + 1 >= src_size) break;

                pixel = src_ptr[src_pos] | (src_ptr[src_pos + 1] << 8);
                src_pos += 2;

                while (count > 0 && x < width && dst_pos < dst_size / 2) {
                    dst_ptr[dst_pos++] = pixel;
                    x++;
                    count--;
                }
            } else {
                /* Literal pixels */
                while (count > 0 && x < width && src_pos + 1 < src_size && dst_pos < dst_size / 2) {
                    pixel = src_ptr[src_pos] | (src_ptr[src_pos + 1] << 8);
                    dst_ptr[dst_pos++] = pixel;
                    src_pos += 2;
                    x++;
                    count--;
                }
            }
        }

        /* Pad remaining row with transparent */
        while (x < width && dst_pos < dst_size / 2) {
            dst_ptr[dst_pos++] = 0xF81F;  /* Transparent pink in 565 format */
            x++;
        }
    }

    return (int)(dst_pos * 2);
}

/*
 * Decode raw sprite data
 */
int sprite_decode_raw(void* src, u32 src_size, void* dst, u32 dst_size) {
    if (src_size > dst_size) {
        src_size = dst_size;
    }
    memcpy(dst, src, src_size);
    return (int)src_size;
}

/*
 * Decode sprite with alpha channel
 */
int sprite_decode_alpha(void* src, u32 src_size, void* dst, u32 dst_size, int width, int height, u8 alpha) {
    /* For now, just do raw copy - alpha can be applied during rendering */
    return sprite_decode_raw(src, src_size, dst, dst_size);
}

/*
 * Decode sprite from spr.bin format - FUN_0048a550 exact pattern
 *
 * Sprites are 8-bit paletted: each decoded pixel is 1 byte (palette index).
 * Total decoded size = width * height bytes (not width * height * 2).
 *
 * Header format (16 bytes):
 *   [0-1]: "RD" magic
 *   [2]:   compression flag (0=raw, nonzero=RLE)
 *   [4-7]: width (u32)
 *   [8-11]: height (u32)
 *   [12-15]: compressed data size (u32)
 *
 * RLE control byte format (from FUN_0048a550 decompilation):
 *   Bit 7: 0=literal, 1=RLE run
 *   Bit 6 (RLE only): 0=pixel byte follows, 1=zero fill
 *   Bits 5-4: count encoding (00=4bit, 01=12bit, 1x=20bit)
 *   Bits 3-0: count bits (low nibble)
 *
 * Literal: copy count bytes (1 byte per pixel, 8-bit paletted)
 * RLE: fill count bytes with single pixel byte
 */
int sprite_decode_from_data(void* src_data, u32 src_size, void* dst_buffer,
                            u32 dst_size, int* out_width, int* out_height, u32* out_data_size) {
    u8* src = (u8*)src_data;
    u8* dst = (u8*)dst_buffer;
    u32 src_pos = 0;
    u32 dst_pos = 0;
    u32 width, height;
    u8 compression;
    u32 total_pixels;

    if (src_size < 16) {
        return 0;
    }

    /* Check magic - "RD" from FUN_0048a550 */
    if (src[0] != 'R' || src[1] != 'D') {
        if (src_size > dst_size) src_size = dst_size;
        memcpy(dst, src, src_size);
        if (out_width) *out_width = 0;
        if (out_height) *out_height = 0;
        if (out_data_size) *out_data_size = src_size;
        return 1;
    }

    /* Read header */
    compression = src[2];
    width = *(u32*)(src + 4);
    height = *(u32*)(src + 8);

    if (out_width) *out_width = (int)width;
    if (out_height) *out_height = (int)height;

    total_pixels = width * height;

    /* Destination buffer holds 8-bit paletted data: 1 byte per pixel */
    if (total_pixels > dst_size) {
        return 0;
    }

    src_pos = 16;

    if (compression == 0) {
        /* Raw: copy width*height bytes directly */
        u32 copy_size = total_pixels;
        if (src_pos + copy_size > src_size) {
            copy_size = src_size - src_pos;
        }
        memcpy(dst, src + src_pos, copy_size);
        if (out_data_size) *out_data_size = copy_size;
        return 1;
    }

    /* RLE decompression - matching FUN_0048a550 byte-level operations */
    while (src_pos < src_size && dst_pos < total_pixels) {
        u8 ctrl = src[src_pos++];
        u32 count;
        u8 pixel_byte;
        u32 i;

        if (ctrl & 0x80) {
            /* RLE run: fill count bytes with single pixel byte */

            if ((ctrl & 0x40) == 0) {
                if (src_pos >= src_size) break;
                pixel_byte = src[src_pos++];
            } else {
                pixel_byte = 0;
            }

            /* Decode count from bits 5-4 and low nibble */
            if ((ctrl & 0x20) == 0) {
                if (ctrl & 0x10) {
                    /* 12-bit: (nibble << 8) | next */
                    if (src_pos >= src_size) break;
                    count = ((ctrl & 0x0F) << 8) | src[src_pos++];
                } else {
                    count = ctrl & 0x0F;
                }
            } else {
                /* 20-bit: (nibble << 16) | (next << 8) | next2 */
                if (src_pos + 2 > src_size) break;
                count = ((ctrl & 0x0F) << 16) | (src[src_pos] << 8) | src[src_pos + 1];
                src_pos += 2;
            }

            /* Write run: fill count bytes with pixel_byte */
            for (i = 0; i < count && dst_pos < total_pixels; i++) {
                dst[dst_pos++] = pixel_byte;
            }
        } else {
            /* Literal run: copy count bytes from source */

            if (ctrl & 0x10) {
                if (src_pos >= src_size) break;
                count = ((ctrl & 0x0F) << 8) | src[src_pos++];
            } else {
                count = ctrl & 0x0F;
            }

            if (count > 0xFFFFE) break;

            /* Copy count bytes (1 byte per pixel) */
            for (i = 0; i < count && src_pos < src_size && dst_pos < total_pixels; i++) {
                dst[dst_pos++] = src[src_pos++];
            }
        }
    }

    if (out_data_size) *out_data_size = dst_pos;
    return 1;
}

/*
 * Convert 8-bit paletted sprite data to 16-bit RGB565 using global palette.
 * Called after sprite_decode_from_data which outputs 8-bit palette indices.
 * Returns newly allocated buffer of width*height*2 bytes, or NULL on failure.
 */
void* sprite_apply_palette(const u8* palette_8bit, u32 pixel_count,
                            const u8* palette_bgra, u32* out_size) {
    u16* rgb565;
    u32 i;

    if (!palette_8bit || pixel_count == 0) return NULL;

    rgb565 = (u16*)malloc(pixel_count * 2);
    if (!rgb565) return NULL;

    for (i = 0; i < pixel_count; i++) {
        u8 idx = palette_8bit[i];
        u8 b = palette_bgra[idx * 4 + 0];
        u8 g = palette_bgra[idx * 4 + 1];
        u8 r = palette_bgra[idx * 4 + 2];
        rgb565[i] = (u16)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
    }

    if (out_size) *out_size = pixel_count * 2;
    return rgb565;
}

/*
 * Sprite cache initialization
 */
int sprite_cache_init(u32 max_sprites) {
    if (g_assets.sprite_cache) {
        free(g_assets.sprite_cache);
    }

    g_assets.cache_size = max_sprites;
    g_assets.cache_used = 0;
    g_assets.sprite_cache = (DecodedSpriteCacheEntry*)malloc(max_sprites * sizeof(DecodedSpriteCacheEntry));

    if (!g_assets.sprite_cache) {
        return 0;
    }

    memset(g_assets.sprite_cache, 0, max_sprites * sizeof(DecodedSpriteCacheEntry));
    return 1;
}

/*
 * Sprite cache shutdown
 */
void sprite_cache_shutdown(void) {
    u32 i;

    if (g_assets.sprite_cache) {
        /* Free cached sprite data */
        for (i = 0; i < g_assets.cache_size; i++) {
            if (g_assets.sprite_cache[i].decoded_data) {
                free(g_assets.sprite_cache[i].decoded_data);
            }
        }
        free(g_assets.sprite_cache);
        g_assets.sprite_cache = NULL;
    }

    g_assets.cache_size = 0;
    g_assets.cache_used = 0;
}

/*
 * Get sprite from cache
 */
DecodedSpriteCacheEntry* decoded_sprite_cache_get(u32 sprite_id) {
    u32 i;

    if (!g_assets.sprite_cache) {
        return NULL;
    }

    for (i = 0; i < g_assets.cache_used; i++) {
        if (g_assets.sprite_cache[i].sprite_id == sprite_id && g_assets.sprite_cache[i].is_valid) {
            return &g_assets.sprite_cache[i];
        }
    }

    return NULL;
}

/*
 * Add sprite to cache
 */
DecodedSpriteCacheEntry* sprite_cache_add(u32 sprite_id, void* data, u32 size, int width, int height) {
    DecodedSpriteCacheEntry* entry;

    if (!g_assets.sprite_cache || g_assets.cache_used >= g_assets.cache_size) {
        return NULL;
    }

    entry = &g_assets.sprite_cache[g_assets.cache_used];
    entry->sprite_id = sprite_id;
    entry->decoded_data = data;
    entry->data_size = size;
    entry->width = width;
    entry->height = height;
    entry->is_valid = 1;
    entry->last_used = GetTickCount();

    g_assets.cache_used++;

    return entry;
}

/*
 * Invalidate sprite cache entry
 */
void sprite_cache_invalidate(u32 sprite_id) {
    DecodedSpriteCacheEntry* entry = decoded_sprite_cache_get(sprite_id);
    if (entry) {
        if (entry->decoded_data) {
            free(entry->decoded_data);
        }
        memset(entry, 0, sizeof(DecodedSpriteCacheEntry));
    }
}

/*
 * Clear entire sprite cache
 */
void sprite_cache_clear(void) {
    u32 i;

    if (g_assets.sprite_cache) {
        for (i = 0; i < g_assets.cache_used; i++) {
            if (g_assets.sprite_cache[i].decoded_data) {
                free(g_assets.sprite_cache[i].decoded_data);
            }
        }
        memset(g_assets.sprite_cache, 0, g_assets.cache_size * sizeof(DecodedSpriteCacheEntry));
    }

    g_assets.cache_used = 0;
}

/*
 * Get sprite dimensions - FUN_0041f900
 * Looks up width and height for a sprite ID
 * Returns 1 on success, 0 on failure
 */
int sprite_get_dimensions(u32 sprite_id, u16* width, u16* height) {
    SpriteEntry* sprite;

    if (!width || !height) {
        return 0;
    }

    /* Standard sprites: 0-499999 */
    if (sprite_id < 500000) {
        sprite = assets_get_sprite(sprite_id);
        if (!sprite) {
            *width = 0;
            *height = 0;
            return 0;
        }
        *width = sprite->width;
        *height = sprite->height;
        return 1;
    }

    /* Extended sprites: 500000-549999 */
    if (sprite_id <= 549999) {
        /* Extended sprites are typically 64x64 */
        /* The original uses DAT_0081c7f4 lookup table */
        u32 ext_index = sprite_id - 500000;

        /* Check if we have extended sprite info */
        if (ext_index < g_assets.extended_count && g_assets.extended_info) {
            ExtendedSpriteDim* dim = (ExtendedSpriteDim*)((u8*)g_assets.extended_info + ext_index * 40);
            *width = dim->width;
            *height = dim->height;
        } else {
            /* Default to 64x64 for extended sprites */
            *width = 64;
            *height = 64;
        }
        return 1;
    }

    /* Invalid sprite ID */
    *width = 0;
    *height = 0;
    return 0;
}

/*
 * Get sprite data offset - FUN_0041fad0
 * Returns the data offset for a sprite ID
 * Returns 1 on success, 0 on failure
 */
int sprite_get_data_offset(u32 sprite_id, u32* offset) {
    SpriteEntry* sprite;

    if (!offset) {
        return 0;
    }

    /* Standard sprites: 0-499999 */
    if (sprite_id < 500000) {
        sprite = assets_get_sprite(sprite_id);
        if (!sprite) {
            *offset = 0;
            return 0;
        }
        *offset = sprite->data_offset;
        return 1;
    }

    /* Extended sprites: 500000-549999 */
    if (sprite_id <= 549999) {
        /* Extended sprites use their ID as offset index */
        /* The original returns sprite_id directly for extended sprites */
        *offset = sprite_id;
        return 1;
    }

    /* Invalid sprite ID */
    *offset = 0;
    return 0;
}

/*
 * Load sprite data into buffer - FUN_0041fb10 helper
 * Loads sprite from spr.bin into the provided buffer
 * Returns decoded data size, or 0 on failure
 */
u32 sprite_load_data(u32 sprite_id, void* buffer, u32 buffer_size, u16* out_width, u16* out_height) {
    SpriteEntry* sprite;
    u8* src_data;
    u32 src_size;
    u8* paletted_data;
    u32 paletted_size;
    int width, height;

    if (!buffer || buffer_size == 0) {
        return 0;
    }

    if (sprite_id >= 550000) {
        return 0;
    }

    sprite = assets_get_sprite(sprite_id);
    if (!sprite || !g_assets.spr_data) {
        return 0;
    }

    src_data = (u8*)g_assets.spr_data + sprite->data_offset;
    src_size = sprite->data_size;

    /* Decode as 8-bit paletted */
    paletted_size = sprite->width * sprite->height;
    paletted_data = (u8*)malloc(paletted_size);
    if (!paletted_data) return 0;

    if (!sprite_decode_from_data(src_data, src_size, paletted_data, paletted_size,
                                  &width, &height, &paletted_size)) {
        free(paletted_data);
        return 0;
    }

    /* Convert to RGB565 */
    if (g_assets.palette_loaded) {
        u32 rgb565_size;
        void* rgb565_data = sprite_apply_palette(paletted_data, (u32)(width * height),
                                                   g_assets.palette, &rgb565_size);
        free(paletted_data);
        if (!rgb565_data) return 0;

        if (rgb565_size > buffer_size) rgb565_size = buffer_size;
        memcpy(buffer, rgb565_data, rgb565_size);
        free(rgb565_data);

        if (out_width) *out_width = (u16)width;
        if (out_height) *out_height = (u16)height;
        return rgb565_size;
    }

    free(paletted_data);
    return 0;
}

/*
 * Load extended sprite data - FUN_0041fc90 helper
 * Loads extended sprite from extsprite.bin
 * Returns decoded data size, or 0 on failure
 */
u32 sprite_load_extended(u32 sprite_id, void* buffer, u32 buffer_size,
                          u16* out_width, u16* out_height, u32* out_extra) {
    void* decoded_data;
    int width, height;
    u32 copy_size;

    if (!buffer || buffer_size == 0) {
        return 0;
    }

    decoded_data = assets_load_and_decode_sprite(sprite_id, &width, &height);
    if (!decoded_data) {
        return 0;
    }

    /* assets_load_and_decode_sprite now returns RGB565 data (width*height*2) */
    copy_size = width * height * 2;
    if (copy_size > buffer_size) {
        copy_size = buffer_size;
    }

    memcpy(buffer, decoded_data, copy_size);
    free(decoded_data);

    if (out_width) *out_width = (u16)width;
    if (out_height) *out_height = (u16)height;
    if (out_extra) *out_extra = 0;

    return copy_size;
}

/*
 * Load sprite and create surface - FUN_0041fb10 wrapper
 * Loads sprite data, decodes RLE, creates DirectDraw surface
 */
int assets_load_sprite(int sprite_id, void** surface, int* width, int* height) {
    u16 w, h;
    u8* paletted_data;
    u32 paletted_size;
    void* rgb565_data;
    u32 rgb565_size;
    SpriteEntry* sprite;
    int dec_w, dec_h;

    if (sprite_id < 0 || sprite_id >= 550000) return 0;

    sprite = assets_get_sprite(sprite_id);
    if (!sprite || !g_assets.spr_data) return 0;

    w = sprite->width;
    h = sprite->height;
    if (w == 0 || h == 0) return 0;

    /* Decode as 8-bit paletted */
    paletted_size = w * h;
    paletted_data = (u8*)malloc(paletted_size);
    if (!paletted_data) return 0;

    if (!sprite_decode_from_data(
            (u8*)g_assets.spr_data + sprite->data_offset,
            sprite->data_size, paletted_data, paletted_size,
            &dec_w, &dec_h, &paletted_size)) {
        free(paletted_data);
        return 0;
    }

    /* Convert to RGB565 */
    if (g_assets.palette_loaded) {
        rgb565_data = sprite_apply_palette(paletted_data, (u32)(dec_w * dec_h),
                                            g_assets.palette, &rgb565_size);
        free(paletted_data);
        if (!rgb565_data) return 0;
    } else {
        rgb565_data = paletted_data;
        rgb565_size = paletted_size;
    }

    if (width) *width = dec_w > 0 ? dec_w : (int)w;
    if (height) *height = dec_h > 0 ? dec_h : (int)h;
    if (surface) *surface = rgb565_data;

    return 1;
}

/*
 * Load high-res sprite - FUN_0041fc90 wrapper
 */
int assets_load_sprite_hires(unsigned int sprite_index, void** surface,
                              int* width, int* height, void** alpha_surface, int* has_alpha) {
    u32 sprite_id = sprite_index + 500000;
    u16 w, h;

    if (sprite_id > 549999) return 0;

    w = 64;
    h = 64;

    void* decoded_data = malloc(w * h * 2);
    if (!decoded_data) return 0;

    u32 decoded_size = sprite_load_extended(sprite_id, decoded_data, w * h * 2, &w, &h, NULL);
    if (decoded_size == 0) {
        free(decoded_data);
        return 0;
    }

    if (width) *width = w;
    if (height) *height = h;
    if (surface) *surface = decoded_data;
    if (alpha_surface) *alpha_surface = NULL;
    if (has_alpha) *has_alpha = 0;

    return 1;
}
