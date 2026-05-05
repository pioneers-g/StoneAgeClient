/*
 * Stone Age Client - Sprite Decoding Interface
 */

#ifndef ASSETS_SPRITE_H
#define ASSETS_SPRITE_H

#include "types.h"
#include "assets.h"  /* For DecodedSpriteCacheEntry */

/* ========================================
 * Sprite Decoding Functions
 * ======================================== */

int sprite_decode_rle(void* src, u32 src_size, void* dst, u32 dst_size, int width, int height);
int sprite_decode_raw(void* src, u32 src_size, void* dst, u32 dst_size);
int sprite_decode_alpha(void* src, u32 src_size, void* dst, u32 dst_size, int width, int height, u8 alpha);
int sprite_decode_from_data(void* src_data, u32 src_size, void* dst_buffer,
                            u32 dst_size, int* out_width, int* out_height, u32* out_data_size);

/**
 * Convert 8-bit paletted sprite data to 16-bit RGB565.
 * @param palette_8bit Decoded 8-bit sprite data (1 byte per pixel)
 * @param pixel_count Number of pixels (width * height)
 * @param palette_bgra Palette in BGRA format (256 * 4 bytes)
 * @param out_size Output: size of returned buffer in bytes
 * @return Newly allocated RGB565 buffer, or NULL on failure
 */
void* sprite_apply_palette(const u8* palette_8bit, u32 pixel_count,
                            const u8* palette_bgra, u32* out_size);

/* ========================================
 * Sprite Cache Functions
 * ======================================== */

int sprite_cache_init(u32 max_sprites);
void sprite_cache_shutdown(void);
DecodedSpriteCacheEntry* decoded_sprite_cache_get(u32 sprite_id);
DecodedSpriteCacheEntry* sprite_cache_add(u32 sprite_id, void* data, u32 size, int width, int height);
void sprite_cache_invalidate(u32 sprite_id);
void sprite_cache_clear(void);

/* ========================================
 * Sprite Lookup Functions - FUN_0041f900, FUN_0041fad0
 * ======================================== */

/**
 * Get sprite dimensions by ID - FUN_0041f900
 * @param sprite_id Sprite ID (0-549999)
 * @param width Output: sprite width
 * @param height Output: sprite height
 * @return 1 on success, 0 on failure
 */
int sprite_get_dimensions(u32 sprite_id, u16* width, u16* height);

/**
 * Get sprite data offset - FUN_0041fad0
 * @param sprite_id Sprite ID (0-549999)
 * @param offset Output: data offset in spr.bin
 * @return 1 on success, 0 on failure
 */
int sprite_get_data_offset(u32 sprite_id, u32* offset);

/**
 * Load sprite data into buffer - FUN_0041fb10 helper
 * @param sprite_id Sprite ID (0-499999 for standard sprites)
 * @param buffer Destination buffer
 * @param buffer_size Buffer size in bytes
 * @param out_width Output: sprite width
 * @param out_height Output: sprite height
 * @return Decoded data size, or 0 on failure
 */
u32 sprite_load_data(u32 sprite_id, void* buffer, u32 buffer_size, u16* out_width, u16* out_height);

/**
 * Load extended sprite data - FUN_0041fc90 helper
 * @param sprite_id Sprite ID (500000-549999 for extended sprites)
 * @param buffer Destination buffer
 * @param buffer_size Buffer size in bytes
 * @param out_width Output: sprite width
 * @param out_height Output: sprite height
 * @param out_extra Output: extra data flag
 * @return Decoded data size, or 0 on failure
 */
u32 sprite_load_extended(u32 sprite_id, void* buffer, u32 buffer_size,
                          u16* out_width, u16* out_height, u32* out_extra);

#endif /* ASSETS_SPRITE_H */
