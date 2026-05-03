/*
 * Stone Age Client - Rendering Module Header
 * Reverse engineered from sa_9061.exe
 *
 * Module structure:
 * - render.c: Main module, drawing primitives, fonts, UI
 * - render_surface.c: Surface operations and blit functions
 * - render_queue.c: Render queue management
 * - render_sprite.c: Sprite cache and sprite rendering
 * - render_text.c: Text queue management
 */

#ifndef RENDER_H
#define RENDER_H

#include "types.h"
#include "directx.h"
#include "render_surface.h"
#include "render_sprite.h"
#include "render_text.h"

/* Color definitions - 16-bit RGB565 */
#define COLOR_BLACK       0x0000
#define COLOR_WHITE       0xFFFF
#define COLOR_RED         0xF800
#define COLOR_GREEN       0x07E0
#define COLOR_BLUE        0x001F
#define COLOR_YELLOW      0xFFE0
#define COLOR_CYAN        0x07FF
#define COLOR_MAGENTA     0xF81F
#define COLOR_ORANGE      0xFD20
#define COLOR_GRAY        0x8410

/* Sprite flags */
#define SPRITE_FLAG_ALPHA     0x01
#define SPRITE_FLAG_FLIP_X    0x02
#define SPRITE_FLAG_FLIP_Y    0x04
#define SPRITE_FLAG_ROTATE    0x08

/* Blend modes */
#define BLEND_NONE       0
#define BLEND_ALPHA      1
#define BLEND_ADDITIVE   2
#define BLEND_SUBTRACT   3

/* Render queue constants - from DAT_04633488 region */
#define MAX_RENDER_QUEUE_ENTRIES 0x7000  /* From original binary */

/* Render flags - from FUN_0047e640, FUN_0047dc60 */
#define RENDER_FLAG_HIDDEN       0x80000000
#define RENDER_FLAG_ALPHA_BLEND  0x10000000
#define RENDER_FLAG_ADDITIVE     0x20000000
#define RENDER_FLAG_SPECIAL      0x40000000

/* Render context */
typedef struct {
    IDirectDrawSurface* target;
    RECT clip_rect;
    int offset_x;
    int offset_y;
    int blend_mode;
    u32 color_key;
    int use_color_key;
} RenderContext;

/* Render queue entry - matching DAT_04633488 structure */
typedef struct {
    u32 pos_x;          /* +0x00: X position (packed for dual resolution) */
    u32 pos_y;          /* +0x04: Y position */
    u32 flags;          /* +0x08: Render flags and sprite ID */
    void* linked_obj;   /* +0x0c: Linked object pointer */
    u32 extra[3];       /* +0x10-0x18: Extra data */
} RenderQueueEntry;

/* Global render context */
extern RenderContext g_render;

/* ========================================
 * Initialization
 * ======================================== */

int render_init(void);
void render_shutdown(void);

/* ========================================
 * Drawing Primitives
 * ======================================== */

void render_fill_rect(IDirectDrawSurface* surface, int x, int y, int w, int h, u32 color);
void render_draw_rect(IDirectDrawSurface* surface, int x, int y, int w, int h, u32 color);
void render_draw_line(IDirectDrawSurface* surface, int x1, int y1, int x2, int y2, u32 color);
void render_draw_pixel(IDirectDrawSurface* surface, int x, int y, u32 color);

/* ========================================
 * Font Management - FUN_00414780, FUN_00415a70
 * ======================================== */

extern HFONT g_font;
extern HFONT g_font_custom;
extern HFONT g_font_custom2;
extern int g_font_size;

void render_set_font(int font_size);
HFONT render_create_font(int font_size);
void render_set_half_resolution(int enabled);
void render_set_alpha_mode(int enabled);
int render_get_alpha_mode(void);

/* ========================================
 * Text Rendering
 * ======================================== */

int render_text(int x, int y, const char* text, u32 color);
int render_text_centered(int x, int y, const char* text, u32 color);
int render_text_shadowed(int x, int y, const char* text, u32 color, u32 shadow_color);
int render_text_width(const char* text);
int render_text_width_n(const char* text, int n);

/* ========================================
 * Clipping
 * ======================================== */

void render_set_clip(int x, int y, int w, int h);
void render_reset_clip(void);

/* ========================================
 * UI Rendering
 * ======================================== */

void render_dialog(int x, int y, int w, int h);
void render_button(int x, int y, int w, int h, const char* text, int state);
void render_textbox(int x, int y, int w, int h, const char* text);

/* ========================================
 * Render Queue Functions
 * ======================================== */

void render_queue_init(void);
void render_queue_clear(void);
int render_queue_add(int x, int y, u32 sprite_id, u32 flags, void* linked_obj);
int render_queue_add_sprite(int x, int y, int layer, u32 sprite_id, int priority);
int render_queue_add_fade(int x1, int y1, int x2, int y2, int sprite_type, u32 base_sprite, int fade_mode);
void render_queue_process(int use_alpha);
void render_queue_process_full(void);
void render_queue_sort(void);
int render_queue_get_count(void);

/* Render state */
void render_process_background(void);
void render_state_reset(void);
void render_set_mode(int mode);
int render_is_surface_lost(void);

/* ========================================
 * Sprite Functions
 * ======================================== */

int render_sprite(u32 sprite_id, int x, int y);
int render_sprite_scaled(u32 sprite_id, int x, int y, int w, int h);
int render_sprite_rotated(u32 sprite_id, int x, int y, float angle);
int render_sprite_frame(u32 sprite_id, int frame, int x, int y);

/* Sprite cache */
SpriteSurfaceEntry* render_get_sprite_surface(u32 sprite_id);
int render_load_sprite(u32 sprite_id);
int render_load_extended_sprite(u32 sprite_id);
void render_unload_sprite(u32 sprite_id);
void render_clear_sprite_cache(void);

/* Sprite info */
u32 render_get_sprite_pointer(u32 sprite_id);
int render_get_sprite_dimensions(u32 sprite_id, u16* width, u16* height);

/* ========================================
 * UI Sprite IDs - FUN_004155e0
 * ======================================== */

void render_init_ui_sprites(void);
u32 render_get_ui_sprite_id(int index);

/* ========================================
 * Map/Character Rendering
 * ======================================== */

void render_tile(int tile_id, int x, int y);
void render_map_layer(int layer, int camera_x, int camera_y);
void render_map_all(int camera_x, int camera_y);
void render_character(int char_id, int x, int y, int direction, int action);
void render_npc(int npc_id, int x, int y);
void render_player(int x, int y, int direction, int action);

/* ========================================
 * Effects
 * ======================================== */

void render_fade(int alpha);
void render_flash(u32 color, int alpha);

#endif /* RENDER_H */
