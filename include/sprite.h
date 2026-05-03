/*
 * Stone Age Client - Sprite Rendering System
 * Reverse engineered from sa_9061.exe (FUN_00414190, FUN_004142f0, FUN_00414420)
 */

#ifndef SPRITE_H
#define SPRITE_H

#include "types.h"
#include "directx.h"  /* For SpriteCacheEntry */

/* Pixel format flags */
#define PIXEL_FORMAT_555    0
#define PIXEL_FORMAT_565    2

/* Sprite render flags */
#define RENDER_FLAG_NONE        0x00
#define RENDER_FLAG_ALPHA       0x10
#define RENDER_FLAG_TRANSPARENT 0x11
#define RENDER_FLAG_ADDITIVE    0x12

/* Sprite structure for rendering */
typedef struct {
    u32 surface;        /* DirectDraw surface */
    u16 width;
    u16 height;
    s16 offset_x;
    s16 offset_y;
    u8  flags;
    u8  alpha;
} RenderSprite;

/* Sprite animation state */
typedef struct {
    u32 sprite_id;
    u32 current_frame;
    u32 frame_count;
    u32 animation_timer;
    u32 animation_speed;
    u8  is_playing;
    u8  loop;
    u8  direction;
    u8  reserved;
} SpriteAnimation;

/* Blend modes */
typedef enum {
    BLEND_NONE = 0,
    BLEND_ALPHA = 1,
    BLEND_ADDITIVE = 2,
    BLEND_MULTIPLY = 3
} BlendMode;

/* Sprite rendering context */
typedef struct {
    void* dest_surface;
    u32 dest_pitch;
    u16 dest_width;
    u16 dest_height;
    u8  pixel_format;
    u8  reserved[3];
} SpriteContext;

/* Global sprite context */
extern SpriteContext g_sprite_ctx;

/* Sprite system functions */
int sprite_init(void);
void sprite_shutdown(void);

/* Sprite rendering - FUN_00414190 pattern */
int sprite_draw(u32 sprite_id, s16 x, s16 y, u8 flags);
int sprite_draw_frame(u32 sprite_id, u32 frame, s16 x, s16 y, u8 flags);
int sprite_draw_scaled(u32 sprite_id, s16 x, s16 y, u16 scale_x, u16 scale_y, u8 flags);
int sprite_draw_rotated(u32 sprite_id, s16 x, s16 y, s16 angle, u8 flags);

/* Sprite blitting - FUN_004142f0, FUN_00414420 patterns */
int sprite_blit(void* src, s16 src_x, s16 src_y, u16 src_w, u16 src_h,
                void* dst, s16 dst_x, s16 dst_y);
int sprite_blit_transparent(void* src, s16 src_x, s16 src_y, u16 src_w, u16 src_h,
                            void* dst, s16 dst_x, s16 dst_y, u16 transparent_color);
int sprite_blit_alpha(void* src, s16 src_x, s16 src_y, u16 src_w, u16 src_h,
                      void* dst, s16 dst_x, s16 dst_y, u8 alpha);

/* Alpha blending - FUN_0047e970 pattern */
void sprite_blend_565(u16* dest, const u16* src, u32 count, u8 alpha);
void sprite_blend_555(u16* dest, const u16* src, u32 count, u8 alpha);
void sprite_blend_additive(u16* dest, const u16* src, u32 count, u8 alpha);

/* RLE decompression */
int sprite_decode_rle_8bit(const u8* src, u32 src_size, u8* dst, u32 dst_size, u16 width, u16 height);
int sprite_decode_rle_16bit(const u8* src, u32 src_size, u16* dst, u32 dst_size, u16 width, u16 height);

/* Animation */
void sprite_animation_init(SpriteAnimation* anim, u32 sprite_id, u32 frame_count);
void sprite_animation_update(SpriteAnimation* anim, u32 delta_time);
void sprite_animation_play(SpriteAnimation* anim);
void sprite_animation_stop(SpriteAnimation* anim);
u32 sprite_animation_get_frame(SpriteAnimation* anim);

/* Sprite cache */
SpriteCacheEntry* sprite_cache_get(u32 sprite_id);
SpriteCacheEntry* sprite_cache_add(u32 sprite_id, void* data, u32 size, u16 width, u16 height);
void sprite_cache_invalidate(u32 sprite_id);

/* Note: Sprite surface cache is implemented in directx.c - see directx.h for:
 * - SpriteCacheEntry structure (0x1c bytes)
 * - sprite_cache_init(), sprite_cache_shutdown()
 * - sprite_cache_get_entry(), sprite_cache_get_count()
 * These match FUN_00480740, FUN_00480870, FUN_004808c0
 */

/* Utility functions */
u16 sprite_color_565(u8 r, u8 g, u8 b);
u16 sprite_color_555(u8 r, u8 g, u8 b);
void sprite_color_split_565(u16 color, u8* r, u8* g, u8* b);
void sprite_color_split_555(u16 color, u8* r, u8* g, u8* b);

/* ========== Render Queue System (FUN_0047e210, FUN_0047e640) ========== */

/* Constants from binary analysis */
#define SPRITE_QUEUE_MAX        4096    /* 0x1000 max sprites in queue */
#define SPRITE_ID_HIGHRES_BASE  500000  /* High-res sprite ID start */
#define SPRITE_ID_HIGHRES_MAX   549999  /* High-res sprite ID max */

/* Sprite type constants (param_3 in FUN_0047e210) */
#define SPRITE_TYPE_UI          0x68    /* UI elements */
#define SPRITE_TYPE_ICON        0x69    /* Icons */
#define SPRITE_TYPE_BUTTON      0x6d    /* Buttons */
#define SPRITE_TYPE_EFFECT      0x6e    /* Effects */

/* Render modes for scaled sprites (FUN_0047e640 param_7) */
typedef enum {
    SPRITE_RENDER_MODE_NORMAL       = 0,      /* Normal rendering */
    SPRITE_RENDER_MODE_ADDITIVE     = 1,      /* Additive blending (0xa0000000) */
    SPRITE_RENDER_MODE_SUBTRACTIVE  = 2,      /* Subtractive blending (0x90000000) */
    SPRITE_RENDER_MODE_CUSTOM       = 3,      /* Custom mode (0xc0000000) */
} SpriteRenderMode;

/* Palette/style values (param_5 in FUN_0047e210) */
/* Values 0-9: default palette, 10-19: group 1, 20-29: group 2, etc. */
#define PALETTE_GROUP_DEFAULT   0
#define PALETTE_GROUP_1         1       /* Style group 1 (values 10-19) */
#define PALETTE_GROUP_2         2       /* Style group 2 (values 20-29) */
#define PALETTE_GROUP_3         3       /* Style group 3 (values 30-39) */
#define PALETTE_GROUP_4         4       /* Style group 4 (values 40-49) */

/* Sprite render queue entry - from DAT_04633488 region analysis */
typedef struct {
    u16 queue_index;     /* +0x00: Queue index */
    u8  sprite_type;     /* +0x02: Sprite type (0x68, 0x69, 0x6d, 0x6e) */
    u8  reserved;
    s32 x;               /* +0x04: X position */
    s32 y;               /* +0x08: Y position */
    s32 sprite_id;       /* +0x0C: Sprite ID (may include flags) */
    s32 flags;           /* +0x10: Additional flags */
    s32 palette;         /* +0x14: Palette/style index */
    s32 palette_group;   /* +0x18: Palette group (0-4) */
} SpriteRenderEntry;

/* Sprite dimension lookup result - FUN_0041f900 */
typedef struct {
    u16 width;
    u16 height;
} SpriteDimension;

/* Sprite render queue context */
typedef struct {
    SpriteRenderEntry entries[SPRITE_QUEUE_MAX];
    u32 count;                    /* Current queue count */
    u32 max_count;                /* Maximum processed */

    /* Statistics */
    u32 total_queued;
    u32 total_rendered;
} SpriteRenderQueue;

/* Global render queue */
extern SpriteRenderQueue g_sprite_queue;

/* Render queue functions - FUN_0047e210 implementation */
int sprite_queue_add(int x, int y, u8 type, int sprite_id, int palette);
void sprite_queue_clear(void);
u32 sprite_queue_get_count(void);

/* Scaled sprite queue - FUN_0047e640 implementation */
int sprite_queue_scaled(int x1, int y1, int x2, int y2,
                        u8 type, int sprite_id, SpriteRenderMode mode);

/* Sprite lookup functions - FUN_0041fad0, FUN_0041f900 */
int sprite_lookup_offset(u32 sprite_id, u32* offset);
int sprite_lookup_dimension(u32 sprite_id, u16* width, u16* height);

/* Process render queue */
void sprite_queue_process(void* surface, u32 pitch);

/* Utility */
int sprite_is_highres(u32 sprite_id);
int sprite_get_palette_group(int palette);

/* LCG Random Number Generator - FUN_00492403 */
u32 sprite_lcg_random(void);
void sprite_lcg_seed(u32 seed);
u32 sprite_lcg_get_state(void);
void sprite_lcg_set_state(u32 state);

/* Background rendering - FUN_0047cd80 */
void sprite_init_background(u32 sprite_id);
void sprite_shutdown_background(void);
IDirectDrawSurface* sprite_get_background_surface(int layer);
void sprite_get_background_dimension(u16* width, u16* height);
void sprite_get_background_offset(s32* x, s32* y);
void sprite_set_background_offset(s32 x, s32 y);

/* Pixel format */
void sprite_set_pixel_format(int format);
int sprite_get_pixel_format(void);
u16 sprite_get_transparent_color(void);

/* Cache utilities */
SpriteCacheEntry* sprite_cache_get_entry(u32 sprite_id);
void sprite_cache_clear(void);

/* LCG extended functions */
u32 sprite_lcg_random_range(u32 max);
u32 sprite_lcg_random_between(u32 min, u32 max);

/* Lookup table setters */
void sprite_set_offset_table(u32* table);
void sprite_set_dimension_table(SpriteDimension* table);
void sprite_set_dimension_table_hires(SpriteDimension* table);

/* Subtract blend mode */
void sprite_blend_subtractive(u16* dest, const u16* src, u32 count, u8 alpha);

/* RLE with alpha check */
int sprite_decode_rle_16bit_check_alpha(const u8* src, u32 src_size, u16* dst, u32 dst_size, u16 width, u16 height, int* has_alpha);

#endif /* SPRITE_H */
