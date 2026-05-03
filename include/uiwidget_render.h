/*
 * Stone Age Client - UI Widget Rendering Interface
 */

#ifndef UIWIDGET_RENDER_H
#define UIWIDGET_RENDER_H

#include "types.h"
#include "uiwidget.h"

/* Widget rendering */
void widgetmgr_render(void);
void widget_render(Widget* widget, void* surface);
void widget_render_window(Widget* widget, void* surface);
void widget_render_button(Widget* widget, void* surface);
void widget_render_text(Widget* widget, void* surface);
void widget_render_image(Widget* widget, void* surface);
void widget_render_listbox(Widget* widget, void* surface);
void widget_render_progress(Widget* widget, void* surface);
void widget_render_slot(Widget* widget, void* surface);

#endif /* UIWIDGET_RENDER_H */
