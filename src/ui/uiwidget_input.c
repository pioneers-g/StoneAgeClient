/*
 * Stone Age Client - UI Widget Input Handling
 * Mouse and keyboard input, focus management, z-order
 */

#include <windows.h>
#include <string.h>
#include "types.h"
#include "uiwidget.h"
#include "logger.h"

/*
 * Handle input for all widgets
 */
int widgetmgr_handle_input(u32 msg, u32 wparam, u32 lparam) {
    int x, y;
    Widget* widget;

    switch (msg) {
        case WM_MOUSEMOVE:
            x = (short)LOWORD(lparam);
            y = (short)HIWORD(lparam);

            /* Find widget under cursor */
            widget = widget_find_at_point(x, y);

            if (widget != g_widgetmgr.hovered_widget) {
                /* Leave old widget */
                if (g_widgetmgr.hovered_widget) {
                    g_widgetmgr.hovered_widget->state = WIDGET_STATE_NORMAL;
                    if (g_widgetmgr.hovered_widget->on_blur) {
                        g_widgetmgr.hovered_widget->on_blur(g_widgetmgr.hovered_widget,
                            g_widgetmgr.hovered_widget->callback_data);
                    }
                }

                /* Enter new widget */
                g_widgetmgr.hovered_widget = widget;
                if (widget && widget_is_enabled(widget)) {
                    widget->state = WIDGET_STATE_HOVER;
                    if (widget->on_hover) {
                        widget->on_hover(widget, widget->callback_data);
                    }
                }
            }

            /* Handle dragging */
            if (g_widgetmgr.dragged_widget) {
                g_widgetmgr.dragged_widget->rect.x = x - g_widgetmgr.drag_offset_x;
                g_widgetmgr.dragged_widget->rect.y = y - g_widgetmgr.drag_offset_y;
            }
            break;

        case WM_LBUTTONDOWN:
            x = (short)LOWORD(lparam);
            y = (short)HIWORD(lparam);

            widget = widget_find_at_point(x, y);
            if (widget && widget_is_enabled(widget)) {
                /* Set focus */
                widget_set_focus(widget);

                /* Check if draggable */
                if (widget->flags & WIDGET_FLAG_DRAGGABLE) {
                    g_widgetmgr.dragged_widget = widget;
                    g_widgetmgr.drag_offset_x = x - widget->rect.x;
                    g_widgetmgr.drag_offset_y = y - widget->rect.y;
                }

                /* Set pressed state */
                widget->state = WIDGET_STATE_PRESSED;
            }
            break;

        case WM_LBUTTONUP:
            x = (short)LOWORD(lparam);
            y = (short)HIWORD(lparam);

            if (g_widgetmgr.dragged_widget) {
                g_widgetmgr.dragged_widget = NULL;
            }

            widget = widget_find_at_point(x, y);
            if (widget && widget_is_enabled(widget)) {
                /* Reset state */
                widget->state = WIDGET_STATE_HOVER;

                /* Fire click callback */
                if (widget->on_click) {
                    widget->on_click(widget, widget->callback_data);
                }
            }
            break;

        case WM_KEYDOWN:
            if (g_widgetmgr.focused_widget) {
                return widget_handle_key(g_widgetmgr.focused_widget, wparam, lparam);
            }
            break;
    }

    return 0;
}

/*
 * Handle mouse move for widget
 */
int widget_handle_mouse_move(Widget* widget, int x, int y) {
    if (!widget || !widget_is_enabled(widget)) return 0;

    if (widget_contains_point(widget, x, y)) {
        if (widget->state != WIDGET_STATE_HOVER) {
            widget->state = WIDGET_STATE_HOVER;
            if (widget->on_hover) {
                widget->on_hover(widget, widget->callback_data);
            }
        }
        return 1;
    }

    return 0;
}

/*
 * Handle mouse click for widget
 */
int widget_handle_mouse_click(Widget* widget, int x, int y, int button) {
    if (!widget || !widget_is_enabled(widget)) return 0;

    if (widget_contains_point(widget, x, y)) {
        if (button == 0) {  /* Left click */
            widget->state = WIDGET_STATE_PRESSED;
            if (widget->on_click) {
                widget->on_click(widget, widget->callback_data);
            }
        }
        return 1;
    }

    return 0;
}

/*
 * Handle key input for widget
 */
int widget_handle_key(Widget* widget, u32 key, u32 flags) {
    if (!widget || !widget_is_enabled(widget)) return 0;

    /* Handle text input for edit widgets */
    if (widget->type == WIDGET_TYPE_EDIT && widget->text) {
        if (key == VK_BACK) {
            size_t len = strlen(widget->text);
            if (len > 0) {
                widget->text[len - 1] = '\0';
            }
            return 1;
        } else if (key == VK_RETURN) {
            /* Enter pressed */
            if (widget->on_click) {
                widget->on_click(widget, widget->callback_data);
            }
            return 1;
        }
    }

    return 0;
}

/*
 * Set focus to widget
 */
void widget_set_focus(Widget* widget) {
    if (g_widgetmgr.focused_widget == widget) return;

    /* Clear old focus */
    if (g_widgetmgr.focused_widget && g_widgetmgr.focused_widget->on_blur) {
        g_widgetmgr.focused_widget->on_blur(g_widgetmgr.focused_widget,
            g_widgetmgr.focused_widget->callback_data);
    }

    /* Set new focus */
    g_widgetmgr.focused_widget = widget;

    if (widget && widget->on_focus) {
        widget->on_focus(widget, widget->callback_data);
    }
}

/*
 * Get focused widget
 */
Widget* widget_get_focus(void) {
    return g_widgetmgr.focused_widget;
}

/*
 * Clear focus
 */
void widget_clear_focus(void) {
    if (g_widgetmgr.focused_widget && g_widgetmgr.focused_widget->on_blur) {
        g_widgetmgr.focused_widget->on_blur(g_widgetmgr.focused_widget,
            g_widgetmgr.focused_widget->callback_data);
    }
    g_widgetmgr.focused_widget = NULL;
}

/*
 * Bring widget to front
 */
void widget_bring_to_front(Widget* widget) {
    int i;

    if (!widget || widget->parent) return;

    /* Find and move to end */
    for (i = 0; i < g_widgetmgr.root_count; i++) {
        if (g_widgetmgr.root_widgets[i] == widget) {
            for (int j = i; j < g_widgetmgr.root_count - 1; j++) {
                g_widgetmgr.root_widgets[j] = g_widgetmgr.root_widgets[j + 1];
            }
            g_widgetmgr.root_widgets[g_widgetmgr.root_count - 1] = widget;
            break;
        }
    }
}

/*
 * Send widget to back
 */
void widget_send_to_back(Widget* widget) {
    int i;

    if (!widget || widget->parent) return;

    /* Find and move to front */
    for (i = 0; i < g_widgetmgr.root_count; i++) {
        if (g_widgetmgr.root_widgets[i] == widget) {
            for (int j = i; j > 0; j--) {
                g_widgetmgr.root_widgets[j] = g_widgetmgr.root_widgets[j - 1];
            }
            g_widgetmgr.root_widgets[0] = widget;
            break;
        }
    }
}

/*
 * Find widget by ID
 */
Widget* widget_find_by_id(u32 id) {
    int i;

    for (i = 0; i < 256; i++) {
        if (g_widgetmgr.widgets[i].id == id) {
            return &g_widgetmgr.widgets[i];
        }
    }

    return NULL;
}

/*
 * Find widget by name
 */
Widget* widget_find_by_name(const char* name) {
    int i;

    if (!name) return NULL;

    for (i = 0; i < 256; i++) {
        if (g_widgetmgr.widgets[i].id != 0 &&
            strcmp(g_widgetmgr.widgets[i].name, name) == 0) {
            return &g_widgetmgr.widgets[i];
        }
    }

    return NULL;
}

/*
 * Find widget at screen point
 */
Widget* widget_find_at_point(int x, int y) {
    int i;
    Widget* result = NULL;

    /* Search from top to bottom (reverse order) */
    for (i = g_widgetmgr.root_count - 1; i >= 0; i--) {
        Widget* widget = g_widgetmgr.root_widgets[i];

        if (widget_is_visible(widget) && widget_contains_point(widget, x, y)) {
            /* Check children first */
            for (int j = widget->child_count - 1; j >= 0; j--) {
                Widget* child = widget->children[j];
                if (widget_is_visible(child) && widget_contains_point(child, x, y)) {
                    result = child;
                    break;
                }
            }

            if (!result) {
                result = widget;
            }
            break;
        }
    }

    return result;
}

/*
 * Convert screen coordinates to widget-local coordinates
 */
void widget_screen_to_local(Widget* widget, int* x, int* y) {
    if (!widget || !x || !y) return;

    *x -= widget->rect.x;
    *y -= widget->rect.y;

    /* Recursively subtract parent offsets */
    if (widget->parent) {
        widget_screen_to_local(widget->parent, x, y);
    }
}

/*
 * Convert widget-local coordinates to screen coordinates
 */
void widget_local_to_screen(Widget* widget, int* x, int* y) {
    if (!widget || !x || !y) return;

    *x += widget->rect.x;
    *y += widget->rect.y;

    /* Recursively add parent offsets */
    if (widget->parent) {
        widget_local_to_screen(widget->parent, x, y);
    }
}

/*
 * Get widget's screen rectangle
 */
WidgetRect widget_get_screen_rect(Widget* widget) {
    WidgetRect rect = {0, 0, 0, 0};

    if (!widget) return rect;

    rect = widget->rect;

    /* Add parent offsets */
    if (widget->parent) {
        WidgetRect parent_rect = widget_get_screen_rect(widget->parent);
        rect.x += parent_rect.x;
        rect.y += parent_rect.y;
    }

    return rect;
}
