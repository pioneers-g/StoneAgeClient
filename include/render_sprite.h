/*
 * Stone Age Client - Sprite Cache Module Header
 * Reverse engineered from sa_9061.exe
 * DAT_0466b7e0 (sprite table), DAT_0464f7c0 (node pool)
 */

#ifndef RENDER_SPRITE_H
#define RENDER_SPRITE_H

#include "types.h"
#include "directx.h"

/* Sprite surface cache size - from original binary
 * Regular sprites: 0-499999, Extended sprites: 500000-549999
 */
#define SPRITE_SURFACE_CACHE_SIZE   550000

/* Sprite node - matching DAT_0464f7c0 pool (28 bytes per node)
 * Each sprite_id can have a linked list of these nodes for multi-tile sprites.
 * In the original binary, sprites are loaded as 64x48 tiles and chained.
 */
typedef struct SpriteNode {
    IDirectDrawSurface* surface;        /* +0x00: Primary surface (IDirectDrawSurface*) */
    IDirectDrawSurface* alpha_data;     /* +0x04: Alpha surface or alpha channel data */
    u32 frame_type;                     /* +0x08: 0 = RLE, 1 = normal sprite */
    u32 sprite_id;                      /* +0x0c: Owning sprite ID */
    u32 timestamp;                      /* +0x10: Last access time (LRU eviction) */
    s16 x_offset;                       /* +0x14: X offset within sprite */
    s16 y_offset;                       /* +0x16: Y offset within sprite */
    struct SpriteNode* next;            /* +0x18: Next node in linked list */
} SpriteNode;

/* Sprite table entry - matching DAT_0466b7e0 (12 bytes per sprite_id)
 * Indexed by sprite_id, stride 3 dwords = 12 bytes.
 * Head pointer to linked list of SpriteNode for multi-tile sprites.
 */
typedef struct SpriteTableEntry {
    SpriteNode* head;                   /* +0x00: Head of sprite node linked list */
    SpriteNode* alpha_head;             /* +0x04: Head of alpha node linked list */
    u16 width;                          /* +0x08: Sprite width */
    u16 height;                         /* +0x0a: Sprite height */
} SpriteTableEntry;

/* Legacy surface entry for compatibility */
typedef struct SpriteSurfaceEntry {
    IDirectDrawSurface* surface;
    IDirectDrawSurface* alpha_surface;
    u16 width;
    u16 height;
    u32 timestamp;
    struct SpriteSurfaceEntry* next;
} SpriteSurfaceEntry;

/* Sprite cache initialization */
void render_sprite_cache_init(void);

/* Sprite cache management */
SpriteSurfaceEntry* render_get_sprite_surface(u32 sprite_id);
SpriteNode* render_get_sprite_node(u32 sprite_id);
SpriteNode* render_get_alpha_node(u32 sprite_id);
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
