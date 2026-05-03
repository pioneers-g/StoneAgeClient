/*
 * Stone Age Client - UI Element Management
 * Element creation, manipulation, and hierarchy
 */

#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H

#include "types.h"
#include "ui.h"

/* ========================================
 * Element Creation Functions
 * ======================================== */

UIElement* ui_create_element(UIType type, int x, int y, int w, int h);
void ui_destroy_element(UIElement* elem);

UIButton* ui_create_button(int x, int y, int w, int h, const char* text);
UITextbox* ui_create_textbox(int x, int y, int w, int h, int max_len);
UIDialog* ui_create_dialog(int x, int y, int w, int h, const char* title);
UIProgressbar* ui_create_progressbar(int x, int y, int w, int h);
UIListbox* ui_create_listbox(int x, int y, int w, int h);
UICheckbox* ui_create_checkbox(int x, int y, const char* text);

/* ========================================
 * Element Manipulation Functions
 * ======================================== */

void ui_set_text(UIElement* elem, const char* text);
void ui_set_position(UIElement* elem, int x, int y);
void ui_set_size(UIElement* elem, int w, int h);
void ui_set_visible(UIElement* elem, int visible);
void ui_set_enabled(UIElement* elem, int enabled);
void ui_set_callback(UIElement* elem, void (*callback)(UIElement*));
void ui_set_sprites(UIElement* elem, u32 normal, u32 pressed, u32 hover, u32 disabled);

/* ========================================
 * Hierarchy Management
 * ======================================== */

void ui_add_child(UIElement* parent, UIElement* child);
void ui_remove_child(UIElement* parent, UIElement* child);

/* ========================================
 * Focus Management
 * ======================================== */

void ui_set_focus(UIElement* elem);
UIElement* ui_get_focused(void);

/* ========================================
 * Hit Testing
 * ======================================== */

UIElement* ui_hit_test(int x, int y);
UIElement* ui_hit_test_recursive(UIElement* elem, int x, int y);

#endif /* UI_ELEMENT_H */
