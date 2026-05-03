/*
 * Stone Age Client - Surface Operations Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef RENDER_SURFACE_H
#define RENDER_SURFACE_H

#include "types.h"
#include "directx.h"

/* Surface creation flags */
#define SURFACE_FLAG_SYSTEM_MEM  0x800
#define SURFACE_FLAG_VIDEO_MEM   0x4000

/* Surface creation and management - FUN_00412af0 */
IDirectDrawSurface* render_create_surface(int width, int height, int flags);
void render_release_surface(IDirectDrawSurface* surface);
int render_lock_surface(IDirectDrawSurface* surface, void** pixels, int* pitch);
void render_unlock_surface(IDirectDrawSurface* surface);

/* Blit operations - FUN_004142f0, FUN_00414420, FUN_00414190 */
int render_blit(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                int src_x, int src_y, int src_w, int src_h,
                int dst_x, int dst_y);
int render_blit_scaled(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                       int src_x, int src_y, int src_w, int src_h,
                       int dst_x, int dst_y, int dst_w, int dst_h);
/* render_blit_transparent is declared in render_blend.h */
int render_blit_fast(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                     int src_x, int src_y, int src_w, int src_h,
                     int dst_x, int dst_y);

/* Fill operations */
void render_fill_rect(IDirectDrawSurface* surface, int x, int y, int w, int h, u32 color);

/* Surface check and restore */
int render_check_surface(IDirectDrawSurface* surface);

#endif /* RENDER_SURFACE_H */
