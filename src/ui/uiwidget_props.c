/*
 * Stone Age Client - UI Widget Properties
 * Split from uiwidget.c for code organization
 *
 * Contains widget property setter/getter functions:
 * - Text, sprite, color properties
 * - Position and size
 * - Visibility and enabled state
 * - Hierarchy management
 */

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "uiwidget.h"
#include "logger.h"

/* External widget manager */
extern WidgetManagerContext g_widgetmgr;

/*
 * Set widget text
 */
void widget_set_text(Widget* widget, const char* text) {
    if (!widget) return;

    if (widget->text) {
        free(widget->text);
        widget->text = NULL;
    }

    if (text) {
        size_t len = strlen(text);
        widget->text = (char*)malloc(len + 1);
        if (widget->text) {
            strcpy(widget->text, text);
            widget->text_len = len;
        }
    }
}

/*
 * Get widget text
 */
const char* widget_get_text(Widget* widget) {
    return widget ? widget->text : NULL;
}

/*
 * Set widget sprite
 */
void widget_set_sprite(Widget* widget, u16 sprite_id, u16 frame) {
    if (widget) {
        widget->sprite_id = sprite_id;
        widget->sprite_frame = frame;
    }
}

/*
 * Set widget color
 */
void widget_set_color(Widget* widget, WidgetColor* bg, WidgetColor* border) {
    if (!widget) return;

    if (bg) widget->bg_color = *bg;
    if (border) widget->border_color = *border;
}

/*
 * Set widget position
 */
void widget_set_position(Widget* widget, int x, int y) {
    if (widget) {
        widget->x = x;
        widget->y = y;
    }
}

/*
 * Set widget size
 */
void widget_set_size(Widget* widget, int width, int height) {
    if (widget) {
        if (widget->extended_data) {
            WidgetWindowData* wdata = (WidgetWindowData*)widget->extended_data;
            wdata->width = width;
            wdata->height = height;
        }
    }
}

/*
 * Set widget rect
 */
void widget_set_rect(Widget* widget, int x, int y, int width, int height) {
    widget_set_position(widget, x, y);
    widget_set_size(widget, width, height);
}

/*
 * Set widget visible
 */
void widget_set_visible(Widget* widget, int visible) {
    if (widget) {
        if (visible) {
            widget->flags |= WIDGET_FLAG_VISIBLE;
        } else {
            widget->flags &= ~WIDGET_FLAG_VISIBLE;
        }
    }
}

/*
 * Set widget enabled
 */
void widget_set_enabled(Widget* widget, int enabled) {
    if (widget) {
        if (enabled) {
            widget->flags |= WIDGET_FLAG_ENABLED;
        } else {
            widget->flags &= ~WIDGET_FLAG_ENABLED;
        }
    }
}

/*
 * Set widget flags
 */
void widget_set_flags(Widget* widget, u32 flags) {
    if (widget) {
        widget->flags = flags;
    }
}

/*
 * Check if widget is visible
 */
int widget_is_visible(Widget* widget) {
    return widget && (widget->flags & WIDGET_FLAG_VISIBLE);
}

/*
 * Check if widget is enabled
 */
int widget_is_enabled(Widget* widget) {
    return widget && (widget->flags & WIDGET_FLAG_ENABLED);
}

/*
 * Check if widget is hovered
 */
int widget_is_hovered(Widget* widget) {
    return widget && g_widgetmgr.hovered_widget == widget;
}

/*
 * Check if widget is focused
 */
int widget_is_focused(Widget* widget) {
    return widget && g_widgetmgr.focused_widget == widget;
}

/*
 * Check if point is inside widget
 */
int widget_contains_point(Widget* widget, int x, int y) {
    int width, height;

    if (!widget) return 0;

    /* Get dimensions from extended data if window */
    if (widget->extended_data) {
        WidgetWindowData* wdata = (WidgetWindowData*)widget->extended_data;
        width = wdata->width * 64;   /* Cell size */
        height = wdata->height * 48;
    } else {
        width = 100;  /* Default size */
        height = 50;
    }

    return (x >= widget->x && x < widget->x + width &&
            y >= widget->y && y < widget->y + height);
}

/*
 * Add child widget
 */
void widget_add_child(Widget* parent, Widget* child) {
    if (!parent || !child || parent->child_count >= 16) return;

    parent->children[parent->child_count++] = child;
    child->parent = parent;
}

/*
 * Remove child widget
 */
void widget_remove_child(Widget* parent, Widget* child) {
    int i;

    if (!parent || !child) return;

    for (i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == child) {
            for (int j = i; j < parent->child_count - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->child_count--;
            child->parent = NULL;
            break;
        }
    }
}

/*
 * Get parent widget
 */
Widget* widget_get_parent(Widget* widget) {
    return widget ? widget->parent : NULL;
}

/*
 * Get child widget by index
 */
Widget* widget_get_child(Widget* widget, int index) {
    if (!widget || index < 0 || index >= widget->child_count) return NULL;
    return widget->children[index];
}

/*
 * Get child count
 */
int widget_get_child_count(Widget* widget) {
    return widget ? widget->child_count : 0;
}

/*
 * Set click callback
 */
void widget_set_on_click(Widget* widget, WidgetCallback callback, void* data) {
    if (widget) {
        widget->on_click = callback;
        widget->callback_data = data;
    }
}

/*
 * Set hover callback
 */
void widget_set_on_hover(Widget* widget, WidgetCallback callback, void* data) {
    if (widget) {
        widget->on_hover = callback;
        widget->callback_data = data;
    }
}

/*
 * Set render callback
 */
void widget_set_on_render(Widget* widget, WidgetRenderFunc func) {
    if (widget) {
        widget->render_callback = func;
    }
}
