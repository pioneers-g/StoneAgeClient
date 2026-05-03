/*
 * Stone Age Client - UI Rendering
 * Element rendering, text rendering, screen rendering
 */

#ifndef UI_RENDER_H
#define UI_RENDER_H

#include "types.h"
#include "ui.h"

/* ========================================
 * Element Rendering
 * ======================================== */

void ui_render_element(UIElement* elem);
void ui_render_button(UIButton* btn);
void ui_render_textbox(UITextbox* tb);
void ui_render_dialog(UIDialog* dlg);
void ui_render_progressbar(UIProgressbar* pb);
void ui_render_listbox(UIListbox* lb);
void ui_render_checkbox(UICheckbox* cb);

/* ========================================
 * Text Rendering - FUN_0041d7c0, FUN_0041d890
 * ======================================== */

int ui_render_text(int x, int y, const char* text, int palette, int flags);
int ui_render_text_multiline(int x, int y, int w, int h, const char* text,
                              int palette, int line_height);
void ui_render_text_cursor(int x, int y, int cursor_pos);

/* ========================================
 * Sprite-based Rendering
 * ======================================== */

void ui_render_sprite_button(u32 sprite_id, int x, int y, int state);
void ui_render_sprite_dialog(u32 sprite_id, int x, int y, int w, int h);

/* ========================================
 * Screen Rendering
 * ======================================== */

void ui_render_login_screen(void);
void ui_render_character_select_screen(void);
void ui_render_character_create_screen(void);

/* ========================================
 * Chat Rendering
 * ======================================== */

void ui_chat_render(void);

#endif /* UI_RENDER_H */
