/*
 * Stone Age Client - Graphics Primitives Header
 * Reverse engineered from sa_9061.exe
 * Core rendering functions for 16-bit graphics
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "types.h"

/* Color formats */
typedef enum {
    COLOR_FORMAT_RGB555 = 0,
    COLOR_FORMAT_RGB565 = 1
} ColorFormat;

/* Graphics context */
typedef struct {
    /* Surface info */
    int width;
    int height;
    int bpp;
    int pitch;

    /* Color format */
    ColorFormat color_format;
    u16 mask_r, mask_g, mask_b;
    int shift_r, shift_g, shift_b;
    int loss_r, loss_g, loss_b;

} GraphicsContext;

/* Global graphics context */
extern GraphicsContext g_graphics;

/* Initialization */
int graphics_init(void);
void graphics_shutdown(void);

/* Color format */
void graphics_set_format(int is_565);

/* Color creation */
u16 graphics_make_color_565(u8 r, u8 g, u8 b);
u16 graphics_make_color_555(u8 r, u8 g, u8 b);
u16 graphics_make_color(u8 r, u8 g, u8 b);

/* Color extraction */
void graphics_extract_color_565(u16 color, u8* r, u8* g, u8* b);
void graphics_extract_color_555(u16 color, u8* r, u8* g, u8* b);

/* Alpha blending */
u16 graphics_alpha_blend_565(u16 src, u16 dst, int alpha);
u16 graphics_alpha_blend_555(u16 src, u16 dst, int alpha);
u16 graphics_alpha_blend(u16 src, u16 dst, int alpha);

/* Pixel operations */
void graphics_draw_pixel(u16* surface, int pitch, int x, int y, u16 color);
u16 graphics_get_pixel(u16* surface, int pitch, int x, int y);

/* Line drawing */
void graphics_draw_hline(u16* surface, int pitch, int x1, int x2, int y, u16 color);
void graphics_draw_vline(u16* surface, int pitch, int x, int y1, int y2, u16 color);
void graphics_draw_line(u16* surface, int pitch, int x1, int y1, int x2, int y2, u16 color);

/* Rectangle drawing */
void graphics_draw_rect(u16* surface, int pitch, int x, int y, int width, int height, u16 color);
void graphics_fill_rect(u16* surface, int pitch, int x, int y, int width, int height, u16 color);
void graphics_fill_gradient(u16* surface, int pitch, int x, int y, int width, int height,
                            u16 color1, u16 color2, int vertical);

/* Surface operations */
void graphics_clear(u16* surface, int pitch, u16 color);

/* Blitting */
void graphics_blit_transparent(u16* dst_surface, int dst_pitch,
                                u16* src_surface, int src_pitch,
                                int dst_x, int dst_y,
                                int src_x, int src_y,
                                int width, int height,
                                u16 transparent_color);

void graphics_blit_alpha(u16* dst_surface, int dst_pitch,
                          u16* src_surface, int src_pitch,
                          int dst_x, int dst_y,
                          int src_x, int src_y,
                          int width, int height,
                          int alpha, u16 transparent_color);

void graphics_blit_alpha_channel(u16* dst_surface, int dst_pitch,
                                  u8* alpha_channel, int alpha_pitch,
                                  u16* src_surface, int src_pitch,
                                  int dst_x, int dst_y,
                                  int src_x, int src_y,
                                  int width, int height);

void graphics_blit_scaled(u16* dst_surface, int dst_pitch,
                           u16* src_surface, int src_pitch,
                           int dst_x, int dst_y, int dst_w, int dst_h,
                           int src_x, int src_y, int src_w, int src_h);

/* Convenience macros */
#define RGB565(r, g, b) graphics_make_color_565(r, g, b)
#define RGB555(r, g, b) graphics_make_color_555(r, g, b)
#define RGB(r, g, b) graphics_make_color(r, g, b)

#endif /* GRAPHICS_H */
