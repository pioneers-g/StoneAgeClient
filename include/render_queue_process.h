/*
 * Stone Age Client - Render Queue Processing Header
 * Reverse engineered from sa_9061.exe
 *
 * Core rendering pipeline functions for sprite and text rendering.
 * FUN_0047dc60 - Main render queue processor
 * FUN_0047d8e0 - Field background rendering
 * FUN_0047e970 - Alpha blending
 * FUN_0047f170 - Additive blending
 * FUN_0047f710 - Special blending
 */

#ifndef RENDER_QUEUE_PROCESS_H
#define RENDER_QUEUE_PROCESS_H

#include "types.h"
#include <ddraw.h>

/* Render modes - DAT_005ab6fc */
typedef enum {
    RENDER_MODE_NORMAL   = 0,
    RENDER_MODE_LOADING  = 2,
    RENDER_MODE_BATTLE   = 3
} RenderMode;

/* Blend modes - DAT_0463349c values */
typedef enum {
    BLEND_MODE_NORMAL    = 0,   /* Standard blit with transparency */
    BLEND_MODE_ALPHA     = 1,   /* Alpha blending (FUN_0047e970) */
    BLEND_MODE_ADDITIVE  = 2,   /* Additive blending (FUN_0047f170) */
    BLEND_MODE_SPECIAL   = 3    /* Special/custom blend (FUN_0047f710) */
} BlendMode;

/* Sprite priority layers - from queue entry byte 1 */
#define PRIORITY_LAYER_GROUND     0x00
#define PRIORITY_LAYER_CHARACTER  0x68
#define PRIORITY_LAYER_UI         0x6C
#define PRIORITY_LAYER_EFFECT     0x6E

/* Render queue entry flags (sprite_id high bits) */
#define RENDER_FLAG_HIDDEN      0x80000000
#define RENDER_FLAG_SPECIAL_1   0x10000000
#define RENDER_FLAG_SPECIAL_2   0x20000000
#define RENDER_FLAG_SPECIAL_3   0x40000000

/* Render queue entry - DAT_04633488 region (24 bytes each, stride 6 DWORDs) */
typedef struct {
    s32 x;               /* +0x00: Screen X position */
    s32 y;               /* +0x04: Screen Y position */
    u32 sprite_id;       /* +0x08: Sprite ID (may have flags in high bits) */
    u32 entity_ptr;      /* +0x0C: Entity pointer or 0 */
    s32 blend_type;      /* +0x10: Blend/render type */
    s32 extra;           /* +0x14: Extra parameter */
} RenderQueueEntry;

/* Render queue sprite index - DAT_0464b488 region (4 bytes each) */
typedef struct {
    u16 queue_index;     /* Index into DAT_04633488 array */
    u8  priority;        /* Render priority/layer (byte 1) */
    u8  reserved;
} RenderQueueIndex;

/* Sprite data entry for linked list traversal - 28 bytes */
typedef struct {
    IDirectDrawSurface* surface;    /* +0x00: Surface pointer */
    void* alpha_surface;            /* +0x04: Alpha surface */
    void* reserved;                 /* +0x08: Reserved */
    int sprite_id;                  /* +0x0C: Sprite ID */
    void* data;                     /* +0x10: Sprite data */
    s16 x_offset;                   /* +0x14: X offset */
    s16 y_offset;                   /* +0x16: Y offset */
    int ref_count;                  /* +0x18: Reference count */
    struct SpriteChain* next;       /* +0x1C: Next in chain */
} SpriteChain;

/* Global render state */
typedef struct {
    u32 queue_count;              /* DAT_0464f488 */
    u32 queue_skip;               /* DAT_0464f48c */
    RenderQueueIndex* queue_ptr;  /* DAT_0464f64c */
    RenderMode mode;              /* DAT_005ab6fc */
    int scroll_x;                 /* DAT_045829b4 */
    int scroll_y;                 /* DAT_045829b8 */
    int map_id;                   /* DAT_04581190 */
    int screen_width;             /* Derived from resolution mode */
    int screen_height;
} RenderState;

extern RenderState g_render;

/* ========================================
 * Core Render Functions - FUN_0047dc60
 * ======================================== */

/**
 * Main render queue processing function
 * Processes all queued sprites and text for the current frame
 */
void render_queue_process(void);

/**
 * Clear back buffer - FUN_00412a40
 */
void render_clear_back_buffer(void);

/**
 * Update animation timer - FUN_0047d850
 * Updates random animation values for dynamic backgrounds
 */
void render_update_animation_timer(void);

/**
 * Render field background - FUN_0047d8e0
 * Handles scrolling and renders background layer
 * @return 1 if rendered, 0 otherwise
 */
int render_field_background(void);

/**
 * Render sprites for field mode - part of FUN_0047d8e0
 */
int render_sprites_field(void);

/**
 * Render sprites for normal mode - FUN_0047e720
 */
void render_sprites_normal(void);

/**
 * Render dynamic background - FUN_0047d5b0
 * Handles minimap cursor and animated elements
 */
void render_dynamic_background(void);

/**
 * Render text queue - FUN_00414820
 * @param layer Text layer to render (0-3)
 */
void render_text_queue(int layer);

/* ========================================
 * Sprite Blitting Functions
 * ======================================== */

/**
 * Blit sprite without alpha - FUN_004142f0
 */
int render_sprite_blit(int x, int y, void* sprite_data);

/**
 * Blit sprite with alpha surface - FUN_00414190
 */
int render_sprite_blit_alpha(int x, int y, void* sprite_data, void* alpha_data);

/**
 * Scaled sprite rendering - FUN_0047fae0
 */
void render_sprite_scaled(void* alpha_surface, void* sprite_surface,
                          int dest_x, int dest_y,
                          int src_x, int src_y,
                          int src_w, int src_h,
                          int scale_w, int scale_h,
                          int flip);

/* ========================================
 * Blend Mode Functions
 * ======================================== */

/**
 * Alpha blending - FUN_0047e970
 * Blends source onto destination using alpha value
 */
void render_blend_alpha(u16* dest, const u16* src,
                        int dest_pitch, int src_pitch,
                        int width, int height,
                        u8 alpha, int flags);

/**
 * Additive blending - FUN_0047f170
 * Adds source color to destination (for lighting effects)
 */
void render_blend_additive(u16* dest, const u16* src,
                           int dest_pitch, int src_pitch,
                           int width, int height, int flags);

/**
 * Special blending - FUN_0047f710
 * Custom blend mode based on blend_type parameter
 */
void render_blend_special(u16* dest, const u16* src,
                          int dest_pitch, int src_pitch,
                          int width, int height,
                          int blend_type, int flags);

/* ========================================
 * Render Queue Management
 * ======================================== */

/**
 * Load sprite into cache - FUN_004808e0
 */
int sprite_cache_load(u32 sprite_id);

/**
 * Get render queue count
 */
u32 render_get_queue_count(void);

/**
 * Get current render mode
 */
int render_get_mode(void);

/**
 * Set render mode
 */
void render_set_mode(int mode);

/**
 * Clear render queue after processing
 */
void render_queue_clear(void);

#endif /* RENDER_QUEUE_PROCESS_H */
