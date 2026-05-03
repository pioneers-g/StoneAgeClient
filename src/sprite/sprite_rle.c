/*
 * Stone Age Client - Sprite RLE Decompression
 * Split from sprite.c for code organization
 *
 * Handles RLE decompression for 8-bit and 16-bit sprites
 */

#include <windows.h>
#include "types.h"
#include "sprite.h"

/* Transparent color key (magic pink) */
#define TRANSPARENT_COLOR_565   0xF81F

/*
 * RLE decompression for 8-bit sprites
 * Format: count byte followed by pixel data
 * If count & 0x80: repeat next byte (count & 0x7F) times
 * Otherwise: copy (count) literal bytes
 */
int sprite_decode_rle_8bit(const u8* src, u32 src_size, u8* dst, u32 dst_size, u16 width, u16 height) {
    u32 src_pos = 0;
    u32 dst_pos = 0;
    u32 x, y;
    u8 count, pixel;

    for (y = 0; y < height; y++) {
        x = 0;

        while (x < width && src_pos < src_size) {
            count = src[src_pos++];

            if (count & 0x80) {
                /* Run-length encoded */
                count &= 0x7F;
                if (src_pos >= src_size) break;

                pixel = src[src_pos++];

                while (count > 0 && x < width && dst_pos < dst_size) {
                    dst[dst_pos++] = pixel;
                    x++;
                    count--;
                }
            } else {
                /* Literal bytes */
                while (count > 0 && x < width && src_pos < src_size && dst_pos < dst_size) {
                    dst[dst_pos++] = src[src_pos++];
                    x++;
                    count--;
                }
            }
        }

        /* Skip to next row if needed */
        while (x < width && dst_pos < dst_size) {
            dst[dst_pos++] = 0;  /* Pad with transparent */
            x++;
        }
    }

    return (int)dst_pos;
}

/*
 * RLE decompression for 16-bit sprites
 */
int sprite_decode_rle_16bit(const u8* src, u32 src_size, u16* dst, u32 dst_size, u16 width, u16 height) {
    u32 src_pos = 0;
    u32 dst_pos = 0;
    u32 x, y;
    u8 count;
    u16 pixel;

    dst_size /= 2;  /* Convert to pixel count */

    for (y = 0; y < height; y++) {
        x = 0;

        while (x < width && src_pos + 1 < src_size) {
            count = src[src_pos++];

            if (count & 0x80) {
                /* Run-length encoded */
                count &= 0x7F;
                if (src_pos + 1 >= src_size) break;

                pixel = src[src_pos] | (src[src_pos + 1] << 8);
                src_pos += 2;

                while (count > 0 && x < width && dst_pos < dst_size) {
                    dst[dst_pos++] = pixel;
                    x++;
                    count--;
                }
            } else {
                /* Literal pixels */
                while (count > 0 && x < width && src_pos + 1 < src_size && dst_pos < dst_size) {
                    pixel = src[src_pos] | (src[src_pos + 1] << 8);
                    dst[dst_pos++] = pixel;
                    src_pos += 2;
                    x++;
                    count--;
                }
            }
        }

        /* Pad row */
        while (x < width && dst_pos < dst_size) {
            dst[dst_pos++] = TRANSPARENT_COLOR_565;
            x++;
        }
    }

    return (int)(dst_pos * 2);
}

/*
 * RLE decompression with transparency check
 * Returns 1 if sprite has transparency, 0 if fully opaque
 */
int sprite_decode_rle_16bit_check_alpha(const u8* src, u32 src_size, u16* dst, u32 dst_size, u16 width, u16 height, int* has_alpha) {
    u32 src_pos = 0;
    u32 dst_pos = 0;
    u32 x, y;
    u8 count;
    u16 pixel;
    int alpha_found = 0;

    dst_size /= 2;

    for (y = 0; y < height; y++) {
        x = 0;

        while (x < width && src_pos + 1 < src_size) {
            count = src[src_pos++];

            if (count & 0x80) {
                count &= 0x7F;
                if (src_pos + 1 >= src_size) break;

                pixel = src[src_pos] | (src[src_pos + 1] << 8);
                src_pos += 2;

                /* Check for transparent color */
                if (pixel == TRANSPARENT_COLOR_565) {
                    alpha_found = 1;
                }

                while (count > 0 && x < width && dst_pos < dst_size) {
                    dst[dst_pos++] = pixel;
                    x++;
                    count--;
                }
            } else {
                while (count > 0 && x < width && src_pos + 1 < src_size && dst_pos < dst_size) {
                    pixel = src[src_pos] | (src[src_pos + 1] << 8);
                    if (pixel == TRANSPARENT_COLOR_565) {
                        alpha_found = 1;
                    }
                    dst[dst_pos++] = pixel;
                    src_pos += 2;
                    x++;
                    count--;
                }
            }
        }

        while (x < width && dst_pos < dst_size) {
            dst[dst_pos++] = TRANSPARENT_COLOR_565;
            x++;
        }
    }

    if (has_alpha) {
        *has_alpha = alpha_found;
    }

    return (int)(dst_pos * 2);
}
