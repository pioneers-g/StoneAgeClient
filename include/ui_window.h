/*
 * Stone Age Client - Window Widget Header
 * Reverse engineered from sa_9061.exe
 * FUN_00448610 - Window creation
 * FUN_00448270 - Window rendering with 9-sprite grid
 *
 * Window uses a 3x3 sprite grid for decoration:
 * - 4 corner sprites
 * - 4 edge sprites (top, bottom, left, right)
 * - 1 center sprite
 *
 * Total of 9 sprites per window, with optional animation.
 */

#ifndef UI_WINDOW_H
#define UI_WINDOW_H

#include "types.h"

/* Window states - matches FUN_00448270 switch cases */
typedef enum {
    WINDOW_STATE_OPENING = 0,   /* Case 0: Expanding animation */
    WINDOW_STATE_COMPLETE = 1,  /* Case 1: Fully rendered */
    WINDOW_STATE_DRAWING = 2,   /* Case 2: Drawing 9-sprite grid */
    WINDOW_STATE_BUTTONS = 3    /* Case 3: Button handling */
} WindowState;

/* Window styles - matches FUN_00448610 param_6 values */
typedef enum {
    WINDOW_STYLE_NORMAL = 0,       /* Standard window with close button */
    WINDOW_STYLE_ALT = 1,          /* Alternative style with close button */
    WINDOW_STYLE_NO_DECORATION = 2,/* No title bar buttons */
    WINDOW_STYLE_DIALOG = 3,       /* Dialog style */
    WINDOW_STYLE_MINIMAL = 4,      /* Minimal window (no borders) */
    WINDOW_STYLE_CUSTOM = -1       /* Custom sprite base */
} WindowStyle;

/* Click results */
typedef enum {
    WINDOW_CLICK_NONE = 0,
    WINDOW_CLICK_CLOSE = 1,
    WINDOW_CLICK_MINIMIZE = 2,
    WINDOW_CLICK_TITLEBAR = 3,
    WINDOW_CLICK_CONTENT = 4
} WindowClickResult;

/* Window internal data - matches FUN_00448610 structure */
typedef struct {
    int x;                    /* Base X position */
    int y;                    /* Base Y position */
    int width;                /* Width in tiles */
    int height;               /* Height in tiles */
    int center_x;             /* Center X for animation */
    int center_y;             /* Center Y for animation */
    u32 sprite_base;          /* Base sprite ID (DAT_0054b194 or DAT_0054c208) */
    u16 sprite_offset;        /* Sprite offset (0x68 for normal, 0x6d for minimal) */
    int layer;                /* Render layer (0 or 1) */
    int has_close_btn;        /* Has close button */
    int has_minimize_btn;     /* Has minimize button */
    int close_btn_sprite;     /* Close button sprite ID */
    int minimize_btn_sprite;  /* Minimize button sprite ID */
    int close_btn_x;          /* Close button X position */
    int close_btn_y;          /* Close button Y position */
    int close_btn_index;      /* Close button render queue index */
    int minimize_btn_index;   /* Minimize button render queue index */
    int close_btn_hovered;    /* Close button hover state */
    int minimize_btn_hovered; /* Minimize button hover state */
    int anim_current_w;       /* Current animation width */
    int anim_current_h;       /* Current animation height */
    int anim_step_x;          /* Animation step X for opening */
    int anim_step_y;          /* Animation step Y for opening */
    int anim_frame;           /* Animation frame counter */
    int title_sprite;         /* Title bar sprite ID */
} WindowInternal;

/* Window widget structure - matches FUN_00448610 allocation */
typedef struct {
    /* Public fields */
    int x;                          /* Window X position */
    int y;                          /* Window Y position */
    int width;                      /* Width in tiles */
    int height;                     /* Height in tiles */
    WindowState state;              /* Current state */
    WindowStyle style;              /* Window style */
    u32 flags;                      /* Window flags */
    char title[64];                 /* Window title */

    /* Rendering */
    int render_complete;            /* Rendering completed flag */
    int close_clicked;              /* Close button was clicked */
    int minimized;                  /* Window is minimized */

    /* Handler function */
    void (*handler)(struct WindowWidget* wnd);

    /* Internal data */
    WindowInternal internal;

    /* Custom data */
    void* user_data;

} WindowWidget;

/*
 * Create window widget - FUN_00448610
 * x, y: Position in pixels
 * width, height: Size in tiles (1 tile = 64x48 pixels)
 * style: Window style (WINDOW_STYLE_*)
 * Returns: WindowWidget pointer or NULL on failure
 */
WindowWidget* window_create(int x, int y, int width, int height, int style);

/*
 * Destroy window widget
 */
void window_destroy(WindowWidget* wnd);

/*
 * Window render handler - FUN_00448270
 * Handles opening animation, 9-sprite grid drawing, and button states
 */
void window_render_handler(WindowWidget* wnd);

/*
 * Update window animation
 */
void window_update(WindowWidget* wnd);

/*
 * Handle window click
 * Returns: WindowClickResult indicating what was clicked
 */
int window_handle_click(WindowWidget* wnd, int click_x, int click_y);

/*
 * Set window position
 */
void window_set_position(WindowWidget* wnd, int x, int y);

/*
 * Set window title
 */
void window_set_title(WindowWidget* wnd, const char* title);

/*
 * Check if point is inside window
 */
int window_contains_point(WindowWidget* wnd, int x, int y);

/*
 * Get window content area (inside borders)
 */
void window_get_content_rect(WindowWidget* wnd, int* x, int* y, int* w, int* h);

#endif /* UI_WINDOW_H */
