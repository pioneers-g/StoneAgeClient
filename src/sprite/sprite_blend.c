/*
 * Stone Age Client - Sprite Alpha Blending
 * Split from sprite.c for code organization
 *
 * Handles alpha blending for 565 and 555 pixel formats
 * Based on FUN_0047e970 analysis
 */

#include <windows.h>
#include "types.h"
#include "sprite.h"

/* Transparent color key (magic pink) */
#define TRANSPARENT_COLOR_565   0xF81F
#define TRANSPARENT_COLOR_555   0x7C1F

/*
 * Alpha blend for 565 format - FUN_0047e970 pattern
 * This is the core blending function used for semi-transparent sprites
 *
 * Binary analysis notes:
 * - Uses SIMD-like operations (psubusw, paddsw, psllw) for parallel processing
 * - The exact formula: result = dest + ((src - dest) * alpha) >> 5
 * - Binary handles odd widths separately (local_5 flag in decompiled code)
 * - Alpha factor calculation: param_8 in binary
 */
void sprite_blend_565(u16* dest, const u16* src, u32 count, u8 alpha) {
    u32 i;
    u16 src_pixel, dest_pixel;
    u32 src_r, src_g, src_b;
    u32 dest_r, dest_g, dest_b;
    u32 result_r, result_g, result_b;

    /* Alpha factor: 0-31 range for 565 */
    u32 alpha_factor = (alpha * 32) / 256;

    for (i = 0; i < count; i++) {
        src_pixel = src[i];
        dest_pixel = dest[i];

        /* Skip transparent pixels */
        if (src_pixel == TRANSPARENT_COLOR_565) {
            continue;
        }

        /* Extract source RGB - 565 format */
        src_r = (src_pixel >> 11) & 0x1F;
        src_g = (src_pixel >> 5) & 0x3F;
        src_b = src_pixel & 0x1F;

        /* Extract destination RGB */
        dest_r = (dest_pixel >> 11) & 0x1F;
        dest_g = (dest_pixel >> 5) & 0x3F;
        dest_b = dest_pixel & 0x1F;

        /* Blend: result = src + (dest - src) * alpha / 32 */
        result_r = dest_r + (((src_r - dest_r) * alpha_factor) >> 5);
        result_g = dest_g + (((src_g - dest_g) * alpha_factor) >> 5);
        result_b = dest_b + (((src_b - dest_b) * alpha_factor) >> 5);

        /* Clamp */
        if (result_r > 0x1F) result_r = 0x1F;
        if (result_g > 0x3F) result_g = 0x3F;
        if (result_b > 0x1F) result_b = 0x1F;

        /* Combine */
        dest[i] = (u16)((result_r << 11) | (result_g << 5) | result_b);
    }
}

/*
 * Alpha blend for 555 format
 *
 * Binary analysis notes:
 * - When DAT_0054bdec != 2, uses 555 format with different bit masks:
 *   - Red: bits 10-14 (mask 0x7C00)
 *   - Green: bits 5-9 (mask 0x03E0)
 *   - Blue: bits 0-4 (mask 0x001F)
 */
void sprite_blend_555(u16* dest, const u16* src, u32 count, u8 alpha) {
    u32 i;
    u16 src_pixel, dest_pixel;
    u32 src_r, src_g, src_b;
    u32 dest_r, dest_g, dest_b;
    u32 result_r, result_g, result_b;

    /* Alpha factor: 0-31 range for 555 */
    u32 alpha_factor = (alpha * 32) / 256;

    for (i = 0; i < count; i++) {
        src_pixel = src[i];
        dest_pixel = dest[i];

        /* Skip transparent pixels */
        if (src_pixel == TRANSPARENT_COLOR_555) {
            continue;
        }

        /* Extract source RGB - 555 format */
        src_r = (src_pixel >> 10) & 0x1F;
        src_g = (src_pixel >> 5) & 0x1F;
        src_b = src_pixel & 0x1F;

        /* Extract destination RGB */
        dest_r = (dest_pixel >> 10) & 0x1F;
        dest_g = (dest_pixel >> 5) & 0x1F;
        dest_b = dest_pixel & 0x1F;

        /* Blend */
        result_r = dest_r + (((src_r - dest_r) * alpha_factor) >> 5);
        result_g = dest_g + (((src_g - dest_g) * alpha_factor) >> 5);
        result_b = dest_b + (((src_b - dest_b) * alpha_factor) >> 5);

        /* Clamp */
        if (result_r > 0x1F) result_r = 0x1F;
        if (result_g > 0x1F) result_g = 0x1F;
        if (result_b > 0x1F) result_b = 0x1F;

        /* Combine */
        dest[i] = (u16)((result_r << 10) | (result_g << 5) | result_b);
    }
}

/*
 * Additive blend for 565 format
 * Used for light effects and glows
 */
void sprite_blend_additive(u16* dest, const u16* src, u32 count, u8 alpha) {
    u32 i;
    u16 src_pixel, dest_pixel;
    u32 src_r, src_g, src_b;
    u32 dest_r, dest_g, dest_b;
    u32 result_r, result_g, result_b;

    u32 alpha_factor = (alpha * 32) / 256;

    for (i = 0; i < count; i++) {
        src_pixel = src[i];
        dest_pixel = dest[i];

        if (src_pixel == TRANSPARENT_COLOR_565) {
            continue;
        }

        src_r = (src_pixel >> 11) & 0x1F;
        src_g = (src_pixel >> 5) & 0x3F;
        src_b = src_pixel & 0x1F;

        dest_r = (dest_pixel >> 11) & 0x1F;
        dest_g = (dest_pixel >> 5) & 0x3F;
        dest_b = dest_pixel & 0x1F;

        /* Additive blend */
        result_r = dest_r + ((src_r * alpha_factor) >> 5);
        result_g = dest_g + ((src_g * alpha_factor) >> 5);
        result_b = dest_b + ((src_b * alpha_factor) >> 5);

        /* Clamp */
        if (result_r > 0x1F) result_r = 0x1F;
        if (result_g > 0x3F) result_g = 0x3F;
        if (result_b > 0x1F) result_b = 0x1F;

        dest[i] = (u16)((result_r << 11) | (result_g << 5) | result_b);
    }
}

/*
 * Subtractive blend for 565 format
 * Used for shadow effects
 */
void sprite_blend_subtractive(u16* dest, const u16* src, u32 count, u8 alpha) {
    u32 i;
    u16 src_pixel, dest_pixel;
    s32 src_r, src_g, src_b;
    s32 dest_r, dest_g, dest_b;
    s32 result_r, result_g, result_b;

    u32 alpha_factor = (alpha * 32) / 256;

    for (i = 0; i < count; i++) {
        src_pixel = src[i];
        dest_pixel = dest[i];

        if (src_pixel == TRANSPARENT_COLOR_565) {
            continue;
        }

        src_r = (src_pixel >> 11) & 0x1F;
        src_g = (src_pixel >> 5) & 0x3F;
        src_b = src_pixel & 0x1F;

        dest_r = (dest_pixel >> 11) & 0x1F;
        dest_g = (dest_pixel >> 5) & 0x3F;
        dest_b = dest_pixel & 0x1F;

        /* Subtractive blend */
        result_r = dest_r - ((src_r * alpha_factor) >> 5);
        result_g = dest_g - ((src_g * alpha_factor) >> 5);
        result_b = dest_b - ((src_b * alpha_factor) >> 5);

        /* Clamp */
        if (result_r < 0) result_r = 0;
        if (result_g < 0) result_g = 0;
        if (result_b < 0) result_b = 0;

        dest[i] = (u16)((result_r << 11) | (result_g << 5) | result_b);
    }
}
