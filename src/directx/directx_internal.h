/*
 * Stone Age Client - DirectX Internal Header
 * Internal definitions for DirectX module
 */

#ifndef DIRECTX_INTERNAL_H
#define DIRECTX_INTERNAL_H

#include <ddraw.h>
#include "types.h"
#include "directx.h"

/* Maximum offscreen surfaces */
#define MAX_OFFSCREEN_SURFACES  0x1000

/* Surface flags */
#define SURFACE_FLAG_VIDEO_MEMORY   0x4000
#define SURFACE_FLAG_SYSTEM_MEMORY  0x0800

/* Sprite cache size */
#define SPRITE_CACHE_SIZE  4096

/* External graphics context */
extern GraphicsContext g_graphics;
extern int g_high_res_mode;
extern int g_sprite_width;
extern int g_sprite_height;

/* Pixel format globals */
extern u32 g_red_mask;
extern u32 g_green_mask;
extern u32 g_blue_mask;
extern int g_red_shift;
extern int g_green_shift;
extern int g_blue_shift;

/* Sprite cache globals */
extern int g_alpha_mode;
extern int g_cache_current_index;

/* Core functions - directx_core.c */
int load_directx_functions(void);
int graphics_init_pixel_format(void);
void graphics_init_default_palette(void);
void surface_set_color_key(IDirectDrawSurface* surface);
int count_bits(u32 mask);

/* Sprite functions - directx_sprite.c */
void sprite_set_data_buffer(void* buffer, int pitch, int height);
int sprite_copy_to_surface(IDirectDrawSurface* surface, int src_x, void* src_data,
                           uint32_t width, int height);
int sprite_copy_to_surfaces(IDirectDrawSurface* primary, IDirectDrawSurface* alpha,
                            int src_x, void* src_data, uint32_t width, int height);
int sprite_cache_init(int width, int height, int count, int alpha_mode);
void sprite_cache_shutdown(void);
SpriteCacheEntry* sprite_cache_get_entry(int index);
int sprite_cache_get_count(void);
int sprite_cache_manager(uint32_t sprite_id, void* sprite_data);
int sprite_load_and_cache(uint32_t sprite_id);

#endif /* DIRECTX_INTERNAL_H */
