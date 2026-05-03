/*
 * Stone Age Client - Graphics Primitives Implementation
 * Reverse engineered from sa_9061.exe
 * Core rendering functions for 16-bit graphics
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "graphics.h"
#include "logger.h"

/* Global graphics context */
GraphicsContext g_graphics = {0};

/* 16-bit color format masks */
#define MASK_RGB565_R    0xF800
#define MASK_RGB565_G    0x07E0
#define MASK_RGB565_B    0x001F

#define MASK_RGB555_R    0x7C00
#define MASK_RGB555_G    0x03E0
#define MASK_RGB555_B    0x001F

/* Transparent color key */
#define TRANSPARENT_COLOR 0x0000

/*
 * Initialize graphics context
 */
int graphics_init(void) {
    memset(&g_graphics, 0, sizeof(GraphicsContext));

    g_graphics.width = 640;
    g_graphics.height = 480;
    g_graphics.bpp = 16;
    g_graphics.color_format = COLOR_FORMAT_RGB565;

    LOG_INFO("Graphics context initialized");
    return 1;
}

/*
 * Shutdown graphics context
 */
void graphics_shutdown(void) {
    memset(&g_graphics, 0, sizeof(GraphicsContext));
    LOG_INFO("Graphics context shutdown");
}

/*
 * Set pixel format
 */
void graphics_set_format(int is_565) {
    g_graphics.color_format = is_565 ? COLOR_FORMAT_RGB565 : COLOR_FORMAT_RGB555;

    if (is_565) {
        g_graphics.mask_r = MASK_RGB565_R;
        g_graphics.mask_g = MASK_RGB565_G;
        g_graphics.mask_b = MASK_RGB565_B;
        g_graphics.shift_r = 11;
        g_graphics.shift_g = 5;
        g_graphics.shift_b = 0;
        g_graphics.loss_r = 3;
        g_graphics.loss_g = 2;
        g_graphics.loss_b = 3;
    } else {
        g_graphics.mask_r = MASK_RGB555_R;
        g_graphics.mask_g = MASK_RGB555_G;
        g_graphics.mask_b = MASK_RGB555_B;
        g_graphics.shift_r = 10;
        g_graphics.shift_g = 5;
        g_graphics.shift_b = 0;
        g_graphics.loss_r = 3;
        g_graphics.loss_g = 3;
        g_graphics.loss_b = 3;
    }
}

/*
 * Make 16-bit color (RGB565)
 */
u16 graphics_make_color_565(u8 r, u8 g, u8 b) {
    return ((u16)(r >> 3) << 11) |
           ((u16)(g >> 2) << 5) |
           ((u16)(b >> 3));
}

/*
 * Make 16-bit color (RGB555)
 */
u16 graphics_make_color_555(u8 r, u8 g, u8 b) {
    return ((u16)(r >> 3) << 10) |
           ((u16)(g >> 3) << 5) |
           ((u16)(b >> 3));
}

/*
 * Make color based on current format
 */
u16 graphics_make_color(u8 r, u8 g, u8 b) {
    if (g_graphics.color_format == COLOR_FORMAT_RGB565) {
        return graphics_make_color_565(r, g, b);
    } else {
        return graphics_make_color_555(r, g, b);
    }
}

/*
 * Extract RGB components from 16-bit color (565)
 */
void graphics_extract_color_565(u16 color, u8* r, u8* g, u8* b) {
    if (r) *r = (u8)((color >> 11) << 3);
    if (g) *g = (u8)(((color >> 5) & 0x3F) << 2);
    if (b) *b = (u8)((color & 0x1F) << 3);
}

/*
 * Extract RGB components from 16-bit color (555)
 */
void graphics_extract_color_555(u16 color, u8* r, u8* g, u8* b) {
    if (r) *r = (u8)((color >> 10) << 3);
    if (g) *g = (u8)(((color >> 5) & 0x1F) << 3);
    if (b) *b = (u8)((color & 0x1F) << 3);
}

/*
 * Alpha blend two 16-bit colors (RGB565)
 * alpha: 0-32 (0 = fully dst, 32 = fully src)
 */
u16 graphics_alpha_blend_565(u16 src, u16 dst, int alpha) {
    int src_r, src_g, src_b;
    int dst_r, dst_g, dst_b;
    int result_r, result_g, result_b;

    /* Extract source components */
    src_r = (src >> 11) & 0x1F;
    src_g = (src >> 5) & 0x3F;
    src_b = src & 0x1F;

    /* Extract destination components */
    dst_r = (dst >> 11) & 0x1F;
    dst_g = (dst >> 5) & 0x3F;
    dst_b = dst & 0x1F;

    /* Blend */
    result_r = ((src_r - dst_r) * alpha >> 5) + dst_r;
    result_g = ((src_g - dst_g) * alpha >> 5) + dst_g;
    result_b = ((src_b - dst_b) * alpha >> 5) + dst_b;

    /* Clamp */
    if (result_r > 31) result_r = 31;
    if (result_g > 63) result_g = 63;
    if (result_b > 31) result_b = 31;

    return (u16)((result_r << 11) | (result_g << 5) | result_b);
}

/*
 * Alpha blend two 16-bit colors (RGB555)
 * alpha: 0-32 (0 = fully dst, 32 = fully src)
 */
u16 graphics_alpha_blend_555(u16 src, u16 dst, int alpha) {
    int src_r, src_g, src_b;
    int dst_r, dst_g, dst_b;
    int result_r, result_g, result_b;

    /* Extract source components */
    src_r = (src >> 10) & 0x1F;
    src_g = (src >> 5) & 0x1F;
    src_b = src & 0x1F;

    /* Extract destination components */
    dst_r = (dst >> 10) & 0x1F;
    dst_g = (dst >> 5) & 0x1F;
    dst_b = dst & 0x1F;

    /* Blend */
    result_r = ((src_r - dst_r) * alpha >> 5) + dst_r;
    result_g = ((src_g - dst_g) * alpha >> 5) + dst_g;
    result_b = ((src_b - dst_b) * alpha >> 5) + dst_b;

    /* Clamp */
    if (result_r > 31) result_r = 31;
    if (result_g > 31) result_g = 31;
    if (result_b > 31) result_b = 31;

    return (u16)((result_r << 10) | (result_g << 5) | result_b);
}

/*
 * Alpha blend based on current format
 */
u16 graphics_alpha_blend(u16 src, u16 dst, int alpha) {
    if (g_graphics.color_format == COLOR_FORMAT_RGB565) {
        return graphics_alpha_blend_565(src, dst, alpha);
    } else {
        return graphics_alpha_blend_555(src, dst, alpha);
    }
}

/*
 * Draw pixel to surface
 */
void graphics_draw_pixel(u16* surface, int pitch, int x, int y, u16 color) {
    if (x >= 0 && y >= 0 && x < g_graphics.width && y < g_graphics.height) {
        surface[y * (pitch >> 1) + x] = color;
    }
}

/*
 * Get pixel from surface
 */
u16 graphics_get_pixel(u16* surface, int pitch, int x, int y) {
    if (x >= 0 && y >= 0 && x < g_graphics.width && y < g_graphics.height) {
        return surface[y * (pitch >> 1) + x];
    }
    return 0;
}

/*
 * Draw horizontal line
 */
void graphics_draw_hline(u16* surface, int pitch, int x1, int x2, int y, u16 color) {
    int x;

    if (y < 0 || y >= g_graphics.height) return;

    if (x1 > x2) {
        int tmp = x1;
        x1 = x2;
        x2 = tmp;
    }

    if (x1 < 0) x1 = 0;
    if (x2 >= g_graphics.width) x2 = g_graphics.width - 1;

    for (x = x1; x <= x2; x++) {
        surface[y * (pitch >> 1) + x] = color;
    }
}

/*
 * Draw vertical line
 */
void graphics_draw_vline(u16* surface, int pitch, int x, int y1, int y2, u16 color) {
    int y;

    if (x < 0 || x >= g_graphics.width) return;

    if (y1 > y2) {
        int tmp = y1;
        y1 = y2;
        y2 = tmp;
    }

    if (y1 < 0) y1 = 0;
    if (y2 >= g_graphics.height) y2 = g_graphics.height - 1;

    for (y = y1; y <= y2; y++) {
        surface[y * (pitch >> 1) + x] = color;
    }
}

/*
 * Draw rectangle outline
 */
void graphics_draw_rect(u16* surface, int pitch, int x, int y, int width, int height, u16 color) {
    graphics_draw_hline(surface, pitch, x, x + width - 1, y, color);
    graphics_draw_hline(surface, pitch, x, x + width - 1, y + height - 1, color);
    graphics_draw_vline(surface, pitch, x, y, y + height - 1, color);
    graphics_draw_vline(surface, pitch, x + width - 1, y, y + height - 1, color);
}

/*
 * Fill rectangle
 */
void graphics_fill_rect(u16* surface, int pitch, int x, int y, int width, int height, u16 color) {
    int row;

    /* Clip to surface */
    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }
    if (x + width > g_graphics.width) {
        width = g_graphics.width - x;
    }
    if (y + height > g_graphics.height) {
        height = g_graphics.height - y;
    }

    if (width <= 0 || height <= 0) return;

    /* Fill rows */
    for (row = y; row < y + height; row++) {
        u16* dst = surface + row * (pitch >> 1) + x;
        int col;

        for (col = 0; col < width; col++) {
            dst[col] = color;
        }
    }
}

/*
 * Clear surface with color
 */
void graphics_clear(u16* surface, int pitch, u16 color) {
    int i;
    int pixel_count = g_graphics.height * (pitch >> 1);

    for (i = 0; i < pixel_count; i++) {
        surface[i] = color;
    }
}

/*
 * Blit with transparency (color key)
 */
void graphics_blit_transparent(u16* dst_surface, int dst_pitch,
                                u16* src_surface, int src_pitch,
                                int dst_x, int dst_y,
                                int src_x, int src_y,
                                int width, int height,
                                u16 transparent_color) {
    int y, x;

    /* Clip to destination */
    if (dst_x < 0) {
        width += dst_x;
        src_x -= dst_x;
        dst_x = 0;
    }
    if (dst_y < 0) {
        height += dst_y;
        src_y -= dst_y;
        dst_y = 0;
    }
    if (dst_x + width > g_graphics.width) {
        width = g_graphics.width - dst_x;
    }
    if (dst_y + height > g_graphics.height) {
        height = g_graphics.height - dst_y;
    }

    if (width <= 0 || height <= 0) return;

    /* Blit with transparency */
    for (y = 0; y < height; y++) {
        u16* src = src_surface + (src_y + y) * (src_pitch >> 1) + src_x;
        u16* dst = dst_surface + (dst_y + y) * (dst_pitch >> 1) + dst_x;

        for (x = 0; x < width; x++) {
            u16 pixel = src[x];
            if (pixel != transparent_color) {
                dst[x] = pixel;
            }
        }
    }
}

/*
 * Blit with alpha
 */
void graphics_blit_alpha(u16* dst_surface, int dst_pitch,
                          u16* src_surface, int src_pitch,
                          int dst_x, int dst_y,
                          int src_x, int src_y,
                          int width, int height,
                          int alpha, u16 transparent_color) {
    int y, x;

    /* Clip to destination */
    if (dst_x < 0) {
        width += dst_x;
        src_x -= dst_x;
        dst_x = 0;
    }
    if (dst_y < 0) {
        height += dst_y;
        src_y -= dst_y;
        dst_y = 0;
    }
    if (dst_x + width > g_graphics.width) {
        width = g_graphics.width - dst_x;
    }
    if (dst_y + height > g_graphics.height) {
        height = g_graphics.height - dst_y;
    }

    if (width <= 0 || height <= 0) return;

    /* Blit with alpha blending */
    for (y = 0; y < height; y++) {
        u16* src = src_surface + (src_y + y) * (src_pitch >> 1) + src_x;
        u16* dst = dst_surface + (dst_y + y) * (dst_pitch >> 1) + dst_x;

        for (x = 0; x < width; x++) {
            u16 src_pixel = src[x];
            if (src_pixel != transparent_color) {
                dst[x] = graphics_alpha_blend(src_pixel, dst[x], alpha);
            }
        }
    }
}

/*
 * Blit with per-pixel alpha (source alpha channel)
 */
void graphics_blit_alpha_channel(u16* dst_surface, int dst_pitch,
                                  u8* alpha_channel, int alpha_pitch,
                                  u16* src_surface, int src_pitch,
                                  int dst_x, int dst_y,
                                  int src_x, int src_y,
                                  int width, int height) {
    int y, x;

    /* Clip to destination */
    if (dst_x < 0) {
        width += dst_x;
        src_x -= dst_x;
        dst_x = 0;
    }
    if (dst_y < 0) {
        height += dst_y;
        src_y -= dst_y;
        dst_y = 0;
    }
    if (dst_x + width > g_graphics.width) {
        width = g_graphics.width - dst_x;
    }
    if (dst_y + height > g_graphics.height) {
        height = g_graphics.height - dst_y;
    }

    if (width <= 0 || height <= 0) return;

    /* Blit with per-pixel alpha */
    for (y = 0; y < height; y++) {
        u16* src = src_surface + (src_y + y) * (src_pitch >> 1) + src_x;
        u16* dst = dst_surface + (dst_y + y) * (dst_pitch >> 1) + dst_x;
        u8* alpha = alpha_channel + (src_y + y) * alpha_pitch + src_x;

        for (x = 0; x < width; x++) {
            if (alpha[x] > 0) {
                if (alpha[x] >= 255) {
                    dst[x] = src[x];
                } else {
                    int a = alpha[x] >> 3; /* Convert to 0-31 range */
                    dst[x] = graphics_alpha_blend(src[x], dst[x], a);
                }
            }
        }
    }
}

/*
 * Scale blit
 */
void graphics_blit_scaled(u16* dst_surface, int dst_pitch,
                           u16* src_surface, int src_pitch,
                           int dst_x, int dst_y, int dst_w, int dst_h,
                           int src_x, int src_y, int src_w, int src_h) {
    int y, x;
    int src_x_pos, src_y_pos;
    int x_step, y_step;

    if (src_w == 0 || src_h == 0) return;

    x_step = (src_w << 16) / dst_w;
    y_step = (src_h << 16) / dst_h;

    src_y_pos = 0;

    for (y = 0; y < dst_h; y++) {
        u16* dst = dst_surface + (dst_y + y) * (dst_pitch >> 1) + dst_x;
        int sy = src_y + (src_y_pos >> 16);
        u16* src_row = src_surface + sy * (src_pitch >> 1) + src_x;

        src_x_pos = 0;

        for (x = 0; x < dst_w; x++) {
            int sx = src_x_pos >> 16;
            dst[x] = src_row[sx];
            src_x_pos += x_step;
        }

        src_y_pos += y_step;
    }
}

/*
 * Fill rectangle with gradient
 */
void graphics_fill_gradient(u16* surface, int pitch,
                             int x, int y, int width, int height,
                             u16 color1, u16 color2, int vertical) {
    int i;
    u8 r1, g1, b1, r2, g2, b2;
    float dr, dg, db;

    /* Extract colors */
    if (g_graphics.color_format == COLOR_FORMAT_RGB565) {
        graphics_extract_color_565(color1, &r1, &g1, &b1);
        graphics_extract_color_565(color2, &r2, &g2, &b2);
    } else {
        graphics_extract_color_555(color1, &r1, &g1, &b1);
        graphics_extract_color_555(color2, &r2, &g2, &b2);
    }

    /* Calculate deltas */
    if (vertical) {
        dr = (float)(r2 - r1) / height;
        dg = (float)(g2 - g1) / height;
        db = (float)(b2 - b1) / height;

        for (i = 0; i < height; i++) {
            u8 r = (u8)(r1 + dr * i);
            u8 g = (u8)(g1 + dg * i);
            u8 b = (u8)(b1 + db * i);
            u16 color = graphics_make_color(r, g, b);

            graphics_draw_hline(surface, pitch, x, x + width - 1, y + i, color);
        }
    } else {
        dr = (float)(r2 - r1) / width;
        dg = (float)(g2 - g1) / width;
        db = (float)(b2 - b1) / width;

        for (i = 0; i < width; i++) {
            u8 r = (u8)(r1 + dr * i);
            u8 g = (u8)(g1 + dg * i);
            u8 b = (u8)(b1 + db * i);
            u16 color = graphics_make_color(r, g, b);

            graphics_draw_vline(surface, pitch, x + i, y, y + height - 1, color);
        }
    }
}
