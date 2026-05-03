/*
 * Stone Age Client - UI Input Handling
 * Mouse and keyboard input for UI elements
 */

#include <string.h>
#include "ui.h"
#include "ui_element.h"
#include "ui_input.h"
#include "input.h"

/* ========================================
 * Mouse Input Handling
 * ======================================== */

int ui_handle_mouse_move(int x, int y) {
    g_ui.mouse_x = x;
    g_ui.mouse_y = y;

    /* Check for hover */
    UIElement* elem = g_ui.root;
    g_ui.hovered = NULL;

    while (elem) {
        if ((elem->flags & UI_FLAG_VISIBLE) &&
            x >= elem->x && x < elem->x + elem->width &&
            y >= elem->y && y < elem->y + elem->height) {
            g_ui.hovered = elem;
            elem->state = UI_STATE_HOVER;
            if (elem->on_hover) elem->on_hover(elem);
        } else if (elem->state == UI_STATE_HOVER) {
            elem->state = UI_STATE_NORMAL;
        }
        elem = elem->next;
    }

    return g_ui.hovered != NULL;
}

int ui_handle_mouse_down(int x, int y, int button) {
    g_ui.mouse_down = 1;

    if (g_ui.hovered) {
        g_ui.hovered->state = UI_STATE_PRESSED;
        g_ui.captured = g_ui.hovered;

        if (button == 1 && g_ui.hovered->on_click) {
            g_ui.hovered->on_click(g_ui.hovered);
        }

        return 1;
    }

    return 0;
}

int ui_handle_mouse_up(int x, int y, int button) {
    g_ui.mouse_down = 0;

    if (g_ui.captured) {
        if (g_ui.captured->state == UI_STATE_PRESSED) {
            g_ui.captured->state = UI_STATE_HOVER;
        }
        g_ui.captured = NULL;
    }

    return 0;
}

void ui_handle_click(int x, int y) {
    ui_handle_mouse_move(x, y);
    ui_handle_mouse_down(x, y, 1);
    ui_handle_mouse_up(x, y, 1);
}

/* ========================================
 * Keyboard Input Handling
 * ======================================== */

int ui_handle_key_down(int key) {
    if (g_ui.focused) {
        if (g_ui.focused->on_key) {
            g_ui.focused->on_key(g_ui.focused, key);
        }

        /* Textbox handling */
        if (g_ui.focused->type == UI_TEXTBOX) {
            UITextbox* tb = (UITextbox*)g_ui.focused;

            switch (key) {
                case DIK_BACK:
                    if (tb->cursor_pos > 0) {
                        tb->cursor_pos--;
                        memmove(tb->buffer + tb->cursor_pos,
                                tb->buffer + tb->cursor_pos + 1,
                                strlen(tb->buffer) - tb->cursor_pos);
                    }
                    break;
                case DIK_DELETE:
                    if (tb->cursor_pos < (int)strlen(tb->buffer)) {
                        memmove(tb->buffer + tb->cursor_pos,
                                tb->buffer + tb->cursor_pos + 1,
                                strlen(tb->buffer) - tb->cursor_pos);
                    }
                    break;
                case DIK_LEFT:
                    if (tb->cursor_pos > 0) tb->cursor_pos--;
                    break;
                case DIK_RIGHT:
                    if (tb->cursor_pos < (int)strlen(tb->buffer)) tb->cursor_pos++;
                    break;
                case DIK_HOME:
                    tb->cursor_pos = 0;
                    break;
                case DIK_END:
                    tb->cursor_pos = strlen(tb->buffer);
                    break;
                case DIK_RETURN:
                    if (tb->base.on_key) {
                        tb->base.on_key(&tb->base, key);
                    }
                    break;
            }
        }

        return 1;
    }

    return 0;
}

int ui_handle_char(int ch) {
    if (g_ui.focused && g_ui.focused->type == UI_TEXTBOX) {
        UITextbox* tb = (UITextbox*)g_ui.focused;

        if (ch >= 32 && ch < 127 && tb->cursor_pos < tb->max_length - 1) {
            /* Insert character */
            memmove(tb->buffer + tb->cursor_pos + 1,
                    tb->buffer + tb->cursor_pos,
                    strlen(tb->buffer) - tb->cursor_pos + 1);
            tb->buffer[tb->cursor_pos++] = (char)ch;
            return 1;
        }
    }

    return 0;
}

/* ========================================
 * Main Input Processing
 * ======================================== */

void ui_handle_input(void) {
    s32 mouse_x, mouse_y;

    input_get_mouse_position(&mouse_x, &mouse_y);
    ui_handle_mouse_move(mouse_x, mouse_y);

    if (input_mouse_button_pressed(MOUSE_LBUTTON)) {
        ui_handle_mouse_down(mouse_x, mouse_y, 1);

        /* Focus element on click */
        UIElement* hit = ui_hit_test(mouse_x, mouse_y);
        if (hit && (hit->flags & UI_FLAG_FOCUSABLE)) {
            ui_set_focus(hit);

            if (hit->type == UI_TEXTBOX) {
                ((UITextbox*)hit)->is_focused = 1;
            }
        }
    }

    if (input_mouse_button_released(MOUSE_LBUTTON)) {
        ui_handle_mouse_up(mouse_x, mouse_y, 1);
    }
}
