/*
 * Stone Age Client - UI Widget Input Interface
 */

#ifndef UIWIDGET_INPUT_H
#define UIWIDGET_INPUT_H

#include "types.h"
#include "uiwidget.h"

/* Input handling */
int widgetmgr_handle_input(u32 msg, u32 wparam, u32 lparam);
int widget_handle_mouse_move(Widget* widget, int x, int y);
int widget_handle_mouse_click(Widget* widget, int x, int y, int button);
int widget_handle_key(Widget* widget, u32 key, u32 flags);

/* Focus management */
void widget_set_focus(Widget* widget);
Widget* widget_get_focus(void);
void widget_clear_focus(void);

/* Z-order */
void widget_bring_to_front(Widget* widget);
void widget_send_to_back(Widget* widget);

/* Widget lookup */
Widget* widget_find_by_id(u32 id);
Widget* widget_find_by_name(const char* name);
Widget* widget_find_at_point(int x, int y);

/* Utilities */
void widget_screen_to_local(Widget* widget, int* x, int* y);
void widget_local_to_screen(Widget* widget, int* x, int* y);
WidgetRect widget_get_screen_rect(Widget* widget);

#endif /* UIWIDGET_INPUT_H */
