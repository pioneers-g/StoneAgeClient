/*
 * Stone Age Client - Sprite Cache Module Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef RENDER_SPRITE_H
#define RENDER_SPRITE_H

#include "types.h"
#include "directx.h"

/* Sprite surface cache size - from original binary
 * Regular sprites: 0-499999, Extended sprites: 500000-549999
 */
#define SPRITE_SURFACE_CACHE_SIZE   550000

/* Sprite surface entry - matching DAT_0466b7e0 structure */
typedef struct SpriteSurfaceEntry {
    IDirectDrawSurface* surface;        /* +0x00: Primary surface */
    IDirectDrawSurface* alpha_surface;  /* +0x04: Alpha surface */
    u16 width;                          /* +0x08: Width */
    u16 height;                         /* +0x0a: Height */
    u32 timestamp;                      /* +0x0c: Last access timestamp */
    struct SpriteSurfaceEntry* next;    /* +0x10: Next in chain */
} SpriteSurfaceEntry;

/* Sprite cache initialization */
void render_sprite_cache_init(void);

/* Sprite cache management */
SpriteSurfaceEntry* render_get_sprite_surface(u32 sprite_id);
int render_load_sprite(u32 sprite_id);
int render_load_extended_sprite(u32 sprite_id);
void render_unload_sprite(u32 sprite_id);
void render_clear_sprite_cache(void);

/* Sprite dimension lookup - FUN_0041f900 */
int render_get_sprite_dimensions(u32 sprite_id, u16* width, u16* height);

/* Sprite pointer lookup - FUN_0041fad0 */
u32 render_get_sprite_pointer(u32 sprite_id);

/* Alpha mode */
void render_sprite_set_alpha_mode(int enabled);
int render_sprite_get_alpha_mode(void);

/* Sprite rendering */
int render_sprite(u32 sprite_id, int x, int y);
int render_sprite_frame(u32 sprite_id, int frame, int x, int y);
int render_sprite_scaled(u32 sprite_id, int x, int y, int w, int h);

#endif /* RENDER_SPRITE_H */
