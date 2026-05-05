/*
 * Stone Age Client - Window Widget with 9-Sprite Grid
 * Reverse engineered from sa_9061.exe
 * FUN_00448610 - Window creation (91 callers)
 * FUN_00448270 - Window rendering with 9-sprite grid
 *
 * Memory layout and logic extracted from binary analysis.
 * The window uses a 3x3 grid of sprites for decoration.
 */

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "ui_window.h"
#include "render.h"
#include "logger.h"

/* Sprite base addresses from binary */
#define SPRITE_BASE_NORMAL   0x6591    /* DAT_0054b194 */
#define SPRITE_BASE_ALPHA    0x7a121   /* DAT_0054c208 */

/* Close button sprites */
#define CLOSE_BTN_BG         0x6d      /* Background sprite offset */
#define CLOSE_BTN_NORMAL     0x65eb    /* Normal close button */
#define CLOSE_BTN_HOVER      0x65ec    /* Hover close button */
#define MINIMIZE_BTN         0x65ed    /* Minimize button */

/* Sprite offsets for 9-grid */
#define SPRITE_OFFSET_NORMAL   0x68    /* Normal window sprites */
#define SPRITE_OFFSET_MINIMAL  0x6d    /* Minimal window sprites */

/* Tile dimensions */
#define TILE_WIDTH   64    /* 0x40 */
#define TILE_HEIGHT  48    /* 0x30 */

/* Animation constants */
#define ANIM_FRAMES  10
#define ANIM_DIVISOR 10

/* Alpha mode flag - DAT_0054c83c */
extern int g_alpha_mode;

/* Mouse state globals - DAT_045f1bc4, DAT_045f1bf0 */
extern u32 g_mouse_state_flags;
extern int g_mouse_hover_target;

/* Sprite base globals */
static u32 s_sprite_base_normal = SPRITE_BASE_NORMAL;
static u32 s_sprite_base_alpha = SPRITE_BASE_ALPHA;

/* Close button background sprite - DAT_0054b174 */
static u32 s_close_btn_bg = 0;

/*
 * Create window widget - FUN_00448610
 *
 * Binary analysis:
 * - Calls FUN_004010a0(3, 0x40) for allocation
 * - Sets flags |= 6 at offset 0xa0
 * - Handler function at offset 8 is FUN_00448270
 * - Sprite offset 0x68 stored at offset 0x15
 *
 * Style mapping from param_6:
 * 0: DAT_0054b194 (normal)
 * 1: DAT_0054c208 (alpha)
 * 2: DAT_0054b194
 * 3: DAT_0054c208
 * 4: offset 0x6d, sprite_base = -2 (minimal)
 */
WindowWidget* ui_widget_window_create(int x, int y, int width, int height, int style) {
    WindowWidget* wnd;
    WindowInternal* wi;
    int half_w;
    int half_h;

    /* Allocate structure - FUN_004010a0(3, 0x40) in binary */
    wnd = (WindowWidget*)malloc(sizeof(WindowWidget));
    if (!wnd) {
        return NULL;
    }

    memset(wnd, 0, sizeof(WindowWidget));

    /* Set up window structure */
    wnd->x = x;
    wnd->y = y;
    wnd->width = width;
    wnd->height = height;
    wnd->style = style;
    wnd->state = WINDOW_STATE_OPENING;
    wnd->flags = 0x06;  /* Flags OR with 6 from FUN_00448610 */

    /* Set handler function */
    wnd->handler = window_render_handler;

    /* Get internal data pointer */
    wi = &wnd->internal;

    /* Calculate half dimensions for centering */
    half_w = (width << 6) / 2;  /* (width * 64) / 2 */
    half_h = (height << 6) / 2;

    /* Set up internal structure - matching FUN_00448610 offsets */
    wi->x = x;
    wi->y = y + 0x1b;  /* Y offset from binary */
    wi->width = width;
    wi->height = height;

    /* Set sprite base and offset based on style - from FUN_00448610 switch */
    switch (style) {
        case 0:  /* Normal mode */
            wi->sprite_base = s_sprite_base_normal;
            wi->sprite_offset = SPRITE_OFFSET_NORMAL;
            wi->has_close_btn = 1;
            break;
        case 1:  /* Alpha mode */
            wi->sprite_base = s_sprite_base_alpha;
            wi->sprite_offset = SPRITE_OFFSET_NORMAL;
            wi->has_close_btn = 1;
            break;
        case 2:  /* Normal mode variant */
            wi->sprite_base = s_sprite_base_normal;
            wi->sprite_offset = SPRITE_OFFSET_NORMAL;
            wi->has_close_btn = 0;
            break;
        case 3:  /* Alpha mode variant */
            wi->sprite_base = s_sprite_base_alpha;
            wi->sprite_offset = SPRITE_OFFSET_NORMAL;
            wi->has_close_btn = 0;
            break;
        case 4:  /* Minimal window */
            wi->sprite_offset = SPRITE_OFFSET_MINIMAL;
            wi->sprite_base = (u32)-2;  /* -2 in binary */
            wi->has_close_btn = 0;
            wi->has_minimize_btn = 0;
            break;
        default:
            wi->sprite_base = (u32)-1;  /* -1 for invalid */
            wi->sprite_offset = SPRITE_OFFSET_NORMAL;
            break;
    }

    /* Calculate center position */
    if (style == -1 || style == 4) {
        /* Special case: use width/2 directly */
        wi->center_x = x + width / 2;
    } else {
        wi->center_x = x + half_w;
    }
    wi->center_y = y + half_h;

    /* Calculate animation steps */
    wi->anim_step_x = (wi->center_x - x) / ANIM_DIVISOR;
    wi->anim_step_y = (wi->center_y - y) / ANIM_DIVISOR;

    /* Set render layer */
    wi->layer = (style < 2) ? 1 : 0;

    /* Initialize animation state */
    wi->anim_current_w = 0;
    wi->anim_current_h = 0;
    wi->anim_frame = 0;

    LOG_DEBUG("Window created: x=%d, y=%d, w=%d, h=%d, style=%d, base=0x%x",
              x, y, width, height, style, wi->sprite_base);

    return wnd;
}

/*
 * Destroy window widget
 */
void ui_widget_window_destroy(WindowWidget* wnd) {
    if (wnd) {
        free(wnd);
    }
}

/*
 * Window render handler - FUN_00448270
 *
 * State machine:
 * 0: Opening animation - expand from center using FUN_0047e640
 * 1: Complete - set render_complete flag
 * 2: Drawing - render 9-sprite grid using FUN_0047e210
 * 3: Buttons - render close/minimize buttons with hover state
 */
void window_render_handler(WindowWidget* wnd) {
    WindowInternal* wi;
    u32 sprite_base;
    u16 sprite_offset;
    int layer;
    int x, y, w, h;
    int row, col;
    int tile_x, tile_y;
    u32 sprite_id;
    int y_offset;

    if (!wnd) return;

    wi = &wnd->internal;

    switch (wnd->state) {
        case WINDOW_STATE_OPENING:
            /* Case 0: Opening animation - expand from center */
            /* FUN_0047e640 in binary: render expanding rectangle */
            render_queue_add_fade(
                wi->center_x - wi->anim_current_w,
                wi->center_y - wi->anim_current_h,
                wi->center_x + wi->anim_current_w,
                wi->center_y + wi->anim_current_h,
                wi->sprite_offset,
                0, 0
            );

            /* Update animation */
            wi->anim_current_w += wi->anim_step_x;
            wi->anim_current_h += wi->anim_step_y;
            wi->anim_frame++;

            /* Check if animation complete (10 frames) */
            if (wi->anim_frame >= ANIM_FRAMES) {
                if (wi->sprite_base == (u32)-2 || wi->sprite_base == (u32)-1) {
                    /* Special case: no sprites, go directly to complete */
                    wnd->state = WINDOW_STATE_COMPLETE;
                } else {
                    /* Start drawing the actual window */
                    wnd->state = WINDOW_STATE_DRAWING;
                }
            }
            break;

        case WINDOW_STATE_DRAWING:
            /* Case 2: Draw the 9-sprite grid */
            sprite_base = wi->sprite_base;
            sprite_offset = wi->sprite_offset;
            layer = wi->layer;

            w = wi->width;
            h = wi->height;

            /* Starting Y position with offset */
            y_offset = wi->y + 0x18;  /* 0x17/0x18 in binary */

            /* Draw 3x3 grid of sprites */
            for (row = 0; row < h; row++) {
                /* Y position for this row */
                tile_y = y_offset + row * TILE_HEIGHT;

                for (col = 0; col < w; col++) {
                    /* X position for this column */
                    tile_x = wi->x + 0x20 + col * TILE_WIDTH;

                    /* Determine sprite offset based on grid position */
                    /* Top row */
                    if (row == 0) {
                        if (col == 0) {
                            /* Top-left corner */
                            sprite_id = sprite_base + sprite_offset + 0;
                        } else if (col == w - 1) {
                            /* Top-right corner */
                            sprite_id = sprite_base + sprite_offset + 2;
                        } else {
                            /* Top-center */
                            /* Alpha mode check from binary */
                            if (g_alpha_mode && sprite_base == s_sprite_base_alpha) {
                                sprite_id = sprite_base + sprite_offset + 1;
                                tile_y = wi->y + 0x19;  /* Different Y offset */
                            } else {
                                sprite_id = sprite_base + sprite_offset + 1;
                            }
                        }
                    }
                    /* Bottom row */
                    else if (row == h - 1) {
                        if (col == 0) {
                            /* Bottom-left corner */
                            sprite_id = sprite_base + sprite_offset + 6;
                        } else if (col == w - 1) {
                            /* Bottom-right corner */
                            sprite_id = sprite_base + sprite_offset + 8;
                        } else {
                            /* Bottom-center */
                            if (g_alpha_mode && sprite_base == s_sprite_base_alpha) {
                                sprite_id = sprite_base + sprite_offset + 7;
                            } else {
                                sprite_id = sprite_base + sprite_offset + 7;
                            }
                        }
                    }
                    /* Middle rows */
                    else {
                        if (col == 0) {
                            /* Left edge */
                            sprite_id = sprite_base + sprite_offset + 3;
                        } else if (col == w - 1) {
                            /* Right edge */
                            sprite_id = sprite_base + sprite_offset + 5;
                        } else {
                            /* Center */
                            sprite_id = sprite_base + sprite_offset + 4;
                        }
                    }

                    /* Add sprite to render queue - FUN_0047e210 */
                    render_queue_add_sprite(tile_x, tile_y, 0x68, sprite_id, layer);
                }
            }

            /* Draw close button if present */
            if (wi->close_btn_sprite != 0) {
                render_queue_add_sprite(wi->close_btn_x, wi->close_btn_y,
                                       0x69, wi->close_btn_sprite, layer);
            }

            /* Fall through to complete state */
            /* In binary, case 2 falls through to case 1 */

        case WINDOW_STATE_COMPLETE:
            /* Case 1: Window is fully rendered */
            wnd->render_complete = 1;
            break;

        case WINDOW_STATE_BUTTONS:
            /* Case 3: Handle button interactions */
            /* Check mouse hover state - DAT_045f1bc4 & 1 */
            if (g_mouse_state_flags & 1) {
                if (g_mouse_hover_target == wi->close_btn_index) {
                    wi->close_btn_hovered = 1;
                }
                if (g_mouse_hover_target == wi->minimize_btn_index) {
                    wi->close_btn_hovered = 0;
                }
            }

            /* Draw close button background */
            render_queue_add_sprite(wi->center_x, wi->center_y,
                                   CLOSE_BTN_BG, s_close_btn_bg, 1);

            /* Draw close button normal state */
            wi->close_btn_index = render_queue_add_sprite(
                wi->center_x, wi->center_y,
                0x6e, CLOSE_BTN_NORMAL, 2
            );

            /* Draw close button hover state */
            wi->minimize_btn_index = render_queue_add_sprite(
                wi->center_x, wi->center_y,
                0x6e, CLOSE_BTN_HOVER, 2
            );
            break;
    }
}

/*
 * Update window animation
 */
void ui_widget_window_update(WindowWidget* wnd) {
    if (!wnd) return;

    /* Animation is handled in render handler */
    /* This function can be used for additional per-frame updates */
}

/*
 * Handle window click
 */
int window_handle_click(WindowWidget* wnd, int click_x, int click_y) {
    WindowInternal* wi;

    if (!wnd || !wnd->render_complete) {
        return WINDOW_CLICK_NONE;
    }

    wi = &wnd->internal;

    /* Check if click is on close button */
    if (wi->has_close_btn) {
        int btn_x = wi->close_btn_x;
        int btn_y = wi->close_btn_y;

        if (click_x >= btn_x && click_x < btn_x + 16 &&
            click_y >= btn_y && click_y < btn_y + 16) {
            wnd->close_clicked = 1;
            return WINDOW_CLICK_CLOSE;
        }
    }

    /* Check if click is on minimize button */
    if (wi->has_minimize_btn) {
        int btn_x = wi->close_btn_x - 20;
        int btn_y = wi->close_btn_y;

        if (click_x >= btn_x && click_x < btn_x + 16 &&
            click_y >= btn_y && click_y < btn_y + 16) {
            return WINDOW_CLICK_MINIMIZE;
        }
    }

    /* Check if click is on title bar for dragging */
    if (click_y < wi->y + 24) {
        return WINDOW_CLICK_TITLEBAR;
    }

    /* Click is inside window content area */
    return WINDOW_CLICK_CONTENT;
}

/*
 * Set window position
 */
void window_set_position(WindowWidget* wnd, int x, int y) {
    WindowInternal* wi;
    int half_w, half_h;

    if (!wnd) return;

    wi = &wnd->internal;

    wnd->x = x;
    wnd->y = y;
    wi->x = x;
    wi->y = y + 0x1b;

    half_w = (wnd->width << 6) / 2;
    half_h = (wnd->height << 6) / 2;

    wi->center_x = x + half_w;
    wi->center_y = y + half_h;
}

/*
 * Set window title
 */
void window_set_title(WindowWidget* wnd, const char* title) {
    if (!wnd || !title) return;

    strncpy(wnd->title, title, sizeof(wnd->title) - 1);
    wnd->title[sizeof(wnd->title) - 1] = '\0';
}

/*
 * Check if point is inside window
 */
int window_contains_point(WindowWidget* wnd, int x, int y) {
    if (!wnd) return 0;

    return (x >= wnd->x && x < wnd->x + wnd->width * TILE_WIDTH &&
            y >= wnd->y && y < wnd->y + wnd->height * TILE_HEIGHT);
}

/*
 * Get window content area (inside borders)
 */
void window_get_content_rect(WindowWidget* wnd, int* x, int* y, int* w, int* h) {
    if (!wnd) {
        *x = *y = *w = *h = 0;
        return;
    }

    /* Content area is inside the window border */
    *x = wnd->x + 8;
    *y = wnd->y + 24;  /* Below title bar */
    *w = wnd->width * TILE_WIDTH - 16;
    *h = wnd->height * TILE_HEIGHT - 32;
}
