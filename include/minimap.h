/*
 * Stone Age Client - Minimap/Radar System Header
 * Reverse engineered from sa_9061.exe
 *
 * Provides a small overhead map showing:
 * - Player position (center, green arrow)
 * - NPCs (yellow dots)
 * - Monsters (red dots)
 * - Other players (white dots)
 * - Party members (blue dots)
 * - Warp points (magenta dots)
 * - Ground items (orange dots)
 */

#ifndef MINIMAP_H
#define MINIMAP_H

#include "types.h"

/* Constants */
#define MINIMAP_WIDTH           128
#define MINIMAP_HEIGHT          128
#define MINIMAP_SCALE           4       /* World pixels per minimap pixel */
#define MINIMAP_DOT_SIZE        2       /* Size of entity dots */
#define MAX_MINIMAP_DOTS        256     /* Maximum tracked entities */

/* Minimap dot colors (16-bit RGB565 format) */
#define MINIMAP_COLOR_PLAYER        0x07E0  /* Green */
#define MINIMAP_COLOR_NPC           0xFFE0  /* Yellow */
#define MINIMAP_COLOR_MONSTER       0xF800  /* Red */
#define MINIMAP_COLOR_PARTY         0x001F  /* Blue */
#define MINIMAP_COLOR_PLAYER_OTHER  0xFFFF  /* White */
#define MINIMAP_COLOR_WARP          0xF81F  /* Magenta */
#define MINIMAP_COLOR_ITEM          0xFD20  /* Orange */

/* Dot types - matches color table indices */
typedef enum {
    DOT_NONE = 0,
    DOT_PLAYER,
    DOT_NPC,
    DOT_MONSTER,
    DOT_PARTY_MEMBER,
    DOT_WARP,
    DOT_ITEM,
    DOT_PLAYER_OTHER,
    DOT_BACKGROUND,
    DOT_BORDER
} MinimapDotType;

/* Minimap dot entry */
typedef struct {
    u16 x;              /* World X coordinate */
    u16 y;              /* World Y coordinate */
    MinimapDotType type; /* Dot type for coloring */
    u32 id;             /* Entity ID for tracking */
} MinimapDot;

/* Minimap context */
typedef struct {
    /* Position on screen */
    u16 screen_x;
    u16 screen_y;
    u16 width;
    u16 height;

    /* Map view center (player position) */
    u16 center_x;
    u16 center_y;

    /* Scale factor (world pixels per minimap pixel) */
    u8 scale;

    /* Visibility flags */
    u8 visible;
    u8 dirty;
    u8 background_enabled;

    /* Dot list */
    MinimapDot dots[MAX_MINIMAP_DOTS];
    int dot_count;

    /* Background surface for rendering */
    void* surface;
    u32 surface_size;

    /* Transparency (0-255, not fully implemented) */
    u8 alpha;

    /* Current map info */
    u32 current_map_id;
    u16 map_width;
    u16 map_height;

} MinimapContext;

/* Global minimap context */
extern MinimapContext g_minimap;

/* Initialization */
int minimap_init(void);
void minimap_shutdown(void);

/* Rendering */
void minimap_render(void* surface);
void minimap_draw_background(void* surface);
void minimap_draw_dots(void* surface);
void minimap_draw_border(void* surface);
void minimap_draw_player(void* surface);

/* Update */
void minimap_update(void);
void minimap_set_center(u16 x, u16 y);
void minimap_set_map(u32 map_id, u16 width, u16 height);
void minimap_refresh(void);

/* Dot management */
void minimap_clear_dots(void);
void minimap_add_dot(u16 x, u16 y, MinimapDotType type, u32 id);
void minimap_remove_dot(u32 id);
void minimap_update_dot(u32 id, u16 x, u16 y);

/* Position conversion */
void minimap_world_to_minimap(u16 world_x, u16 world_y, u16* minimap_x, u16* minimap_y);
void minimap_minimap_to_world(u16 minimap_x, u16 minimap_y, u16* world_x, u16* world_y);

/* Visibility */
void minimap_show(void);
void minimap_hide(void);
void minimap_toggle(void);
int minimap_is_visible(void);

/* Position on screen */
void minimap_set_position(u16 x, u16 y);
void minimap_get_position(u16* x, u16* y);

/* Scale */
void minimap_set_scale(u8 scale);
u8 minimap_get_scale(void);

/* Click handling */
int minimap_handle_click(u16 screen_x, u16 screen_y);

#endif /* MINIMAP_H */
