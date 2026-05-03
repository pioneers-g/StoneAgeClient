/*
 * Stone Age Client - DirectX Module
 * Reverse engineered from sa_9061.exe (FUN_00411a00)
 */

#ifndef DIRECTX_H
#define DIRECTX_H

#include <ddraw.h>
#include <mmsystem.h>
#include <dsound.h>
#include <dinput.h>
#include "types.h"

/* Screen modes */
typedef enum {
    SCREEN_MODE_FULLSCREEN = 0,
    SCREEN_MODE_WINDOWED = 1
} ScreenMode;

/* Graphics context - matches DAT_0054a90c structure */
typedef struct {
    IDirectDraw*        ddraw;
    IDirectDraw4*       ddraw4;
    IDirectDrawSurface* primary_surface;
    IDirectDrawSurface* back_buffer;
    IDirectDrawSurface* offscreen_surface;
    IDirectDrawSurface* alpha_surface;     /* Alpha mode surface - at offset +0x10 */
    IDirectDrawPalette* palette;
    IDirectDrawClipper* clipper;
    int                 width;
    int                 height;
    int                 bpp;
    int                 pitch;
    void*               pixels;
    DDSURFACEDESC2      surface_desc;
} GraphicsContext;

/* Global graphics context */
extern GraphicsContext g_graphics;

/* High-res mode flag - DAT_0054c83c
 * 0 = standard sprites (0x6xxx series)
 * 1 = high-res sprites (0x7axxx series)
 */
extern int g_high_res_mode;

/* DirectX initialization - FUN_00411a00 */
int directx_init(HWND hWnd, int window_mode);
void directx_shutdown(void);

/* Graphics context allocation - FUN_00440170 */
int graphics_allocate_context(int screen_mode);

/* Graphics functions */
int graphics_init(HWND hWnd, int window_mode);
void graphics_shutdown(void);
int graphics_create_surfaces(int window_mode);
int graphics_set_mode(int width, int height, int bpp, int window_mode);
void graphics_flip(void);
void graphics_clear(int color);
void graphics_set_palette(void);
int graphics_load_palette(const char* path);

/* Surface management */
IDirectDrawSurface* graphics_create_offscreen(int width, int height, int flags);
void graphics_blit(IDirectDrawSurface* src, int src_x, int src_y, int src_w, int src_h,
                   IDirectDrawSurface* dst, int dst_x, int dst_y);

/* Surface locking - FUN_00412ab0 */
int graphics_lock_surface(IDirectDrawSurface* surface, void** pixels, int* pitch);
void graphics_unlock_surface(IDirectDrawSurface* surface);

/* Surface info */
int graphics_get_surface_pitch(IDirectDrawSurface* surface);
void* graphics_get_surface_pixels(IDirectDrawSurface* surface);

/* Sprite surface cache - FUN_00480740 */
#define SPRITE_CACHE_SIZE           0x1000
#define SPRITE_CACHE_ENTRY_SIZE     0x1c    /* 28 bytes per entry */

/* Sprite cache entry - matches DAT_0464f7c0 region layout
 * Array stride: 7 DWORDs (28 bytes) for primary
 * Extended stride: 14 WORDs (28 bytes) for offsets
 */
typedef struct {
    IDirectDrawSurface* surface;        /* Offset 0x00 - primary sprite surface */
    void* alpha_buffer;                 /* Offset 0x04 - alpha buffer (for alpha mode) */
    IDirectDrawSurface* alpha_surface;  /* Offset 0x08 - alpha surface (for alpha mode) */
    int sprite_id;                      /* Offset 0x0c - cached sprite ID (-1 if empty) */
    void* sprite_data;                  /* Offset 0x10 - sprite data pointer */
    short x_offset;                     /* Offset 0x14 - X offset in surface */
    short y_offset;                     /* Offset 0x16 - Y offset in surface */
    int ref_count;                      /* Offset 0x18 - reference count */
    int flags;                          /* Offset 0x1c - surface flags */
} SpriteCacheEntry;

/* Sprite cache manager state - matches DAT_0466b7xx region */
typedef struct {
    int current_index;          /* DAT_0466b7c8 - current cache index (circular) */
    int max_count;              /* DAT_0466b7cc - maximum cache entries */
    int sprite_width;           /* DAT_0466b7d0 - sprite surface width */
    int sprite_height;          /* DAT_0466b7d4 - sprite surface height */
    int total_surfaces;         /* DAT_04cb6d20 - total surfaces created */
    int alpha_surfaces;         /* DAT_0466b7d8 - alpha surfaces count */
} SpriteCacheState;

/* Sprite surface functions - FUN_00480740 */
int sprite_cache_init(int width, int height, int count, int alpha_mode);
void sprite_cache_shutdown(void);
SpriteCacheEntry* sprite_cache_get_entry(int index);
int sprite_cache_get_count(void);

/* Sprite surface cache manager - FUN_00480130 */
int sprite_cache_manager(uint32_t sprite_id, void* sprite_data);

/* Sprite loading and caching - FUN_004808e0 */
int sprite_load_and_cache(uint32_t sprite_id);

/* Surface data copy - FUN_00412d60 */
int sprite_copy_to_surface(IDirectDrawSurface* surface, int src_x, void* src_data,
                           uint32_t width, int height);

/* Dual surface copy (alpha) - FUN_00412b80 */
int sprite_copy_to_surfaces(IDirectDrawSurface* primary, IDirectDrawSurface* alpha,
                            int src_x, void* src_data, uint32_t width, int height);

/* Alpha mode */
void graphics_set_alpha_mode(int enabled);
int graphics_get_alpha_mode(void);

/* Offscreen surface creation - FUN_00412af0 */
void* offscreen_create(int width, int height, int flags);

/* Surface flags */
#define SURFACE_FLAG_VIDEO_MEMORY   0x4000
#define SURFACE_FLAG_SYSTEM_MEMORY  0x0800
#define SURFACE_FLAG_TEXTURE        0x1000

#endif /* DIRECTX_H */
