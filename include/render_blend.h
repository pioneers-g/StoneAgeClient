/*
 * Stone Age Client - Render Blend Operations Header
 * Split from render_queue.c for better code organization
 */

#ifndef RENDER_BLEND_H
#define RENDER_BLEND_H

#include <ddraw.h>
#include "types.h"

/* Pixel format constants */
#define PIXEL_FORMAT_555    0
#define PIXEL_FORMAT_565    2

/*
 * Set pixel format for blend operations
 */
void render_blend_set_pixel_format(int format);

/*
 * Alpha blend single pixel helpers
 */
u16 alpha_blend_rgb565(u16 src, u16 dst, int alpha);
u16 alpha_blend_rgb555(u16 src, u16 dst, int alpha);

/*
 * Blend operation functions - FUN_0047e970, FUN_0047f170, FUN_0047f710
 */

/* Alpha blend blit - FUN_0047e970 */
void render_blit_alpha_blend(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                             int dst_x, int dst_y, int w, int h, int alpha);

/* Additive blend blit - glowing effects */
void render_blit_additive(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                          int dst_x, int dst_y, int w, int h);

/* Subtractive blend blit - shadow effects */
void render_blit_subtractive(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                             int dst_x, int dst_y, int w, int h);

/* Translucent blend blit - FUN_0047f170 */
void render_blit_translucent(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                             int dst_x, int dst_y, int w, int h);

/* Gray tint blend - FUN_0047f710 */
void render_blit_gray_tint(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                           int dst_x, int dst_y, int w, int h, int tint);

/* Per-pixel alpha blend blit with alpha channel - FUN_0047fae0 */
void render_blit_per_pixel_alpha(IDirectDrawSurface* src, IDirectDrawSurface* alpha_surf,
                                  IDirectDrawSurface* dst, int dst_x, int dst_y,
                                  int src_x, int src_y, int w, int h, int flip);

/* Fill rectangle with color */
void render_fill_rect(IDirectDrawSurface* surface, int x, int y, int w, int h, u32 color);

/* Blit with transparency (color key) */
int render_blit_transparent(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                             int src_x, int src_y, int w, int h, int dst_x, int dst_y, u32 color_key);

/* Primitive rectangle rendering - FUN_00412eb0 */
void render_primitive_rect(RECT* rect, u32 sprite_id, int mode);

/* Pixel format variable - DAT_0054bdec */
extern int s_pixel_format;

#endif /* RENDER_BLEND_H */
