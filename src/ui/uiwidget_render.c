/*
 * Stone Age Client - UI Widget Rendering
 * Widget drawing functions for each widget type
 *
 * Reverse engineered from sa_9061.exe:
 *   FUN_00448270 - Window render callback (9-sprite grid)
 *   FUN_0047e210 - Render queue add sprite
 *   FUN_0047e640 - Render with fade effect
 */

#include <windows.h>
#include <string.h>
#include <timeapi.h>
#include "types.h"
#include "uiwidget.h"
#include "logger.h"

/* External render queue functions - from render_queue.c */
extern int render_queue_add_sprite(int x, int y, u8 sprite_type, s32 sprite_id, int priority);
extern void render_queue_add_fade(int x1, int y1, int x2, int y2, u8 sprite_type, u32 sprite_id, int fade_mode);

/* Alpha mode flag - DAT_0054c83c */
extern int g_alpha_enabled;

/* Sprite ID constants - from DAT_0054b194 and DAT_0054c208 */
#define SPRITE_STANDARD_BASE    0x6591   /* DAT_0054b194 */
#define SPRITE_ALTERNATE_BASE   0x65a5   /* DAT_0054c208 */
#define SPRITE_CLOSE_BUTTON     0x65eb   /* Close button normal */
#define SPRITE_CLOSE_HOVER      0x65ec   /* Close button hover */
#define SPRITE_CLOSE_SPECIAL    0x65b6   /* Special close sprite - DAT_0054b174 */

/* Tile size constants from FUN_00448270 */
#define TILE_WIDTH          0x40    /* 64 pixels - from iVar5 += 0x40 */
#define TILE_HEIGHT         0x30    /* 48 pixels - from iVar3 += 0x30 */
#define TILE_HEIGHT_ALT     0x2f    /* 47 pixels - for some rows */
#define TILE_OFFSET_Y       0x20    /* 32 pixels - first row offset */

/*
 * Window render callback - FUN_00448270
 * Renders window with 9-sprite grid decoration
 *
 * Render states:
 *   0: Opening animation (fade in)
 *   1: Complete (set done flag)
 *   2: Active rendering (draw 9-sprite grid)
 *   3: Button hover state
 */
void widget_window_render_callback(Widget* widget) {
    WidgetWindowData* data;
    int x, y;
    int grid_x, grid_y;
    int col, row;
    int sprite_id;
    int priority;
    int width, height;
    int offset_x, offset_y;
    int frame_count;

    if (!widget || !widget->extended_data) {
        return;
    }

    data = (WidgetWindowData*)widget->extended_data;
    width = data->width;
    height = data->height;
    sprite_id = data->sprite_base;
    priority = data->alpha_mode;
    offset_x = data->offset_x;
    offset_y = data->offset_y;
    frame_count = data->frame_count;

    /* Get base position */
    x = widget->x;
    y = widget->y;

    /* Handle render states - switch(*(param_1 + 0xa8)) */
    switch (widget->render_state) {
    case WIDGET_RENDER_INIT:
        /*
         * Case 0: Opening animation
         * FUN_0047e640 renders with fade effect
         */
        render_queue_add_fade(
            data->center_x - offset_x,
            data->center_y - offset_y,
            data->center_x + offset_x,
            data->center_y + offset_y,
            (u8)data->close_sprite3,  /* sprite_type from piVar1[0xe] */
            0,
            0
        );

        /* Update animation offsets - piVar1[7] += *(param_1 + 0x28) */
        data->offset_x += widget->anim_offset_x;
        data->offset_y += widget->anim_offset_y;
        data->frame_count++;

        /* Check if animation complete - if (9 < iVar3 + 1) */
        if (data->frame_count > 9) {
            if (data->sprite_base != -1) {
                /* Transition to active or special state */
                if (data->sprite_base == -2) {
                    widget->render_state = WIDGET_RENDER_BUTTONS;
                } else {
                    widget->render_state = WIDGET_RENDER_ACTIVE;
                }
            } else {
                widget->render_state = WIDGET_RENDER_COMPLETE;
            }
        }
        break;

    case WIDGET_RENDER_COMPLETE:
        /* Case 1: Set done flag */
        widget->done = 1;
        break;

    case WIDGET_RENDER_ACTIVE:
        /*
         * Case 2: Main 9-sprite grid rendering
         * Iterates through rows and columns to draw window decoration
         */
        {
            int base_sprite = data->sprite_base;
            int tile_y_offset = y + 0x18;  /* Start 24 pixels down */
            int tile_x_offset;

            /* Draw grid - nested loops from FUN_00448270 */
            for (row = 0; row < height; row++) {
                tile_x_offset = x + TILE_OFFSET_Y;  /* 32 pixel offset for first row */

                for (col = 0; col < width; col++) {
                    int draw_x = tile_x_offset + col * TILE_WIDTH;
                    int draw_y = tile_y_offset;
                    int use_sprite = base_sprite;
                    int use_priority = priority;

                    /* Determine sprite for each position */
                    if (row == 0) {
                        /* Top row */
                        if (col == 0) {
                            /* Top-left corner */
                            use_sprite = base_sprite;
                        } else if (col == width - 1) {
                            /* Top-right corner */
                            use_sprite = base_sprite + 2;
                        } else {
                            /* Top edge */
                            if (g_alpha_enabled && base_sprite == SPRITE_ALTERNATE_BASE) {
                                use_sprite = base_sprite + 1;
                                draw_y += 2;
                            } else {
                                use_sprite = base_sprite + 1;
                            }
                        }
                    } else if (row == height - 1) {
                        /* Bottom row */
                        if (col == 0) {
                            /* Bottom-left corner */
                            use_sprite = base_sprite + 6;
                        } else if (col == width - 1) {
                            /* Bottom-right corner */
                            use_sprite = base_sprite + 8;
                        } else {
                            /* Bottom edge */
                            if (g_alpha_enabled && base_sprite == SPRITE_ALTERNATE_BASE) {
                                use_sprite = base_sprite + 7;
                            } else {
                                use_sprite = base_sprite + 7;
                            }
                        }
                    } else {
                        /* Middle rows */
                        if (col == 0) {
                            /* Left edge */
                            use_sprite = base_sprite + 3;
                        } else if (col == width - 1) {
                            /* Right edge */
                            if (g_alpha_enabled && base_sprite == SPRITE_STANDARD_BASE) {
                                use_sprite = base_sprite + 5;
                            } else if (g_alpha_enabled && base_sprite == SPRITE_ALTERNATE_BASE) {
                                use_sprite = base_sprite + 5;
                            } else {
                                use_sprite = base_sprite + 5;
                            }
                        } else {
                            /* Center fill */
                            if (g_alpha_enabled) {
                                if (base_sprite == SPRITE_ALTERNATE_BASE) {
                                    use_sprite = base_sprite + 4;
                                } else {
                                    use_sprite = base_sprite + 4;
                                }
                            } else {
                                use_sprite = base_sprite + 4;
                            }
                        }
                    }

                    /* Add sprite to render queue */
                    render_queue_add_sprite(draw_x, draw_y, (u8)data->close_sprite3, use_sprite, use_priority);
                }

                tile_y_offset += TILE_HEIGHT;
            }

            /* Draw extra sprite if set - piVar1[2] */
            if (data->extra_param != 0) {
                render_queue_add_sprite(
                    data->base_x,
                    data->base_y,
                    0x69,  /* Sprite type for extra element */
                    data->extra_param,
                    priority
                );
            }
        }

        /* Set done flag after rendering */
        widget->done = 1;
        break;

    case WIDGET_RENDER_BUTTONS:
        /*
         * Case 3: Button hover state handling
         * Renders close button with hover detection
         */
        {
            /* Check for mouse interaction - DAT_045f1bc4 & 1 */
            extern u32 g_mouse_state;  /* DAT_045f1bc4 */
            extern s32 g_hover_sprite; /* DAT_045f1bf0 */

            if (g_mouse_state & 1) {
                /* Update hover state */
                if (g_hover_sprite == data->close_sprite1) {
                    data->hover_flag = 1;
                }
                if (g_hover_sprite == data->close_sprite2) {
                    data->hover_flag = 0;
                }
            }

            /* Draw close button background */
            render_queue_add_sprite(
                data->center_x,
                data->center_y,
                0x6d,  /* Window decoration layer */
                SPRITE_CLOSE_SPECIAL,  /* 0x65b6 - DAT_0054b174 */
                1
            );

            /* Draw close button normal state */
            data->close_sprite1 = render_queue_add_sprite(
                data->center_x,
                data->center_y,
                0x6e,  /* Button layer */
                SPRITE_CLOSE_BUTTON,  /* 0x65eb */
                2
            );

            /* Draw close button hover state */
            data->close_sprite2 = render_queue_add_sprite(
                data->center_x,
                data->center_y,
                0x6e,
                SPRITE_CLOSE_HOVER,  /* 0x65ec */
                2
            );
        }
        break;
    }
}

/*
 * Render all widgets
 */
void widgetmgr_render(void) {
    int i;

    for (i = 0; i < g_widgetmgr.root_count; i++) {
        Widget* widget = g_widgetmgr.root_widgets[i];
        if (widget_is_visible(widget)) {
            widget_render(widget, NULL);
        }
    }
}

/*
 * Render single widget
 */
void widget_render(Widget* widget, void* surface) {
    int i;

    if (!widget || !widget_is_visible(widget)) return;

    /* Call type-specific render */
    switch (widget->type) {
    case WIDGET_TYPE_WINDOW:
        /* Windows use the callback from FUN_00448270 */
        if (widget->render_callback) {
            widget->render_callback(widget, surface);
        } else {
            widget_render_window(widget, surface);
        }
        break;
    case WIDGET_TYPE_BUTTON:
        widget_render_button(widget, surface);
        break;
    case WIDGET_TYPE_TEXT:
        widget_render_text(widget, surface);
        break;
    case WIDGET_TYPE_IMAGE:
        widget_render_image(widget, surface);
        break;
    case WIDGET_TYPE_LISTBOX:
        widget_render_listbox(widget, surface);
        break;
    case WIDGET_TYPE_PROGRESS:
        widget_render_progress(widget, surface);
        break;
    case WIDGET_TYPE_SLOT:
        widget_render_slot(widget, surface);
        break;
    default:
        break;
    }

    /* Render children */
    for (i = 0; i < widget->child_count; i++) {
        widget_render(widget->children[i], surface);
    }
}

/*
 * Render window widget (fallback when no callback)
 */
void widget_render_window(Widget* widget, void* surface) {
    WidgetWindowData* data;
    int x, y, w, h;

    if (!widget) return;

    x = widget->x;
    y = widget->y;
    w = widget->rect.width;
    h = widget->rect.height;

    /* Simple window rendering without 9-sprite grid */
    /* This is a fallback - normally the callback is used */

    (void)surface;
    (void)data;
}

/*
 * Render button widget
 */
void widget_render_button(Widget* widget, void* surface) {
    WidgetColor color;
    int x, y, w, h;

    if (!widget) return;

    x = widget->x;
    y = widget->y;
    w = widget->rect.width;
    h = widget->rect.height;

    /* Determine color based on state */
    color = widget->bg_color;
    if (widget->state == 1) {  /* Hover */
        color.r = (color.r + 30 > 255) ? 255 : color.r + 30;
        color.g = (color.g + 30 > 255) ? 255 : color.g + 30;
        color.b = (color.b + 30 > 255) ? 255 : color.b + 30;
    } else if (widget->state == 2) {  /* Pressed */
        color.r = (color.r < 30) ? 0 : color.r - 30;
        color.g = (color.g < 30) ? 0 : color.g - 30;
        color.b = (color.b < 30) ? 0 : color.b - 30;
    }

    /* Draw button sprite if available */
    if (widget->sprite_id > 0) {
        render_queue_add_sprite(x, y, 0x68, widget->sprite_id, 1);
    }

    (void)surface;
}

/*
 * Render text widget
 */
void widget_render_text(Widget* widget, void* surface) {
    if (!widget || !widget->text) return;

    /* Text rendering would use render_text_queue or similar */
    (void)surface;
}

/*
 * Render image widget
 */
void widget_render_image(Widget* widget, void* surface) {
    if (!widget) return;

    if (widget->sprite_id > 0) {
        render_queue_add_sprite(
            widget->x,
            widget->y,
            0x68,
            widget->sprite_id,
            widget->sprite_frame
        );
    }

    (void)surface;
}

/*
 * Render listbox widget
 */
void widget_render_listbox(Widget* widget, void* surface) {
    if (!widget) return;

    /* Listbox background */
    if (widget->sprite_id > 0) {
        render_queue_add_sprite(widget->x, widget->y, 0x68, widget->sprite_id, 0);
    }

    (void)surface;
}

/*
 * Render progress bar widget
 */
void widget_render_progress(Widget* widget, void* surface) {
    int max_value;
    int current_value;

    if (!widget) return;

    max_value = (int)(size_t)widget->data;
    current_value = (int)(size_t)widget->callback_data;

    if (max_value > 0 && widget->sprite_id > 0) {
        /* Draw background */
        render_queue_add_sprite(widget->x, widget->y, 0x68, widget->sprite_id, 0);

        /* Draw progress fill */
        /* Would need sprite clipping for partial fill */
    }

    (void)surface;
    (void)current_value;
}

/*
 * Render slot widget (inventory/equipment slots)
 */
void widget_render_slot(Widget* widget, void* surface) {
    if (!widget) return;

    /* Draw slot background */
    if (widget->sprite_id > 0) {
        render_queue_add_sprite(
            widget->x,
            widget->y,
            0x68,
            widget->sprite_id,
            widget->sprite_frame
        );
    }

    (void)surface;
}
