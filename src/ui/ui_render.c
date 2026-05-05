/*
 * Stone Age Client - UI Rendering
 * Element rendering, text rendering, screen rendering
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "ui.h"
#include "ui_render.h"
#include "render.h"
#include "login.h"
#include "input.h"

/* ========================================
 * Element Rendering
 * ======================================== */

void ui_render_element(UIElement* elem) {
    if (!elem) return;

    switch (elem->type) {
        case UI_BUTTON:
            ui_render_button((UIButton*)elem);
            break;
        case UI_TEXTBOX:
            ui_render_textbox((UITextbox*)elem);
            break;
        case UI_DIALOG:
            ui_render_dialog((UIDialog*)elem);
            break;
        case UI_PROGRESSBAR:
            ui_render_progressbar((UIProgressbar*)elem);
            break;
        case UI_LISTBOX:
            ui_render_listbox((UIListbox*)elem);
            break;
        case UI_CHECKBOX:
            ui_render_checkbox((UICheckbox*)elem);
            break;
        default:
            /* Default rendering */
            render_fill_rect(NULL, elem->x, elem->y, elem->width, elem->height, elem->bg_color);
            render_draw_rect(NULL, elem->x, elem->y, elem->width, elem->height, elem->border_color);
            if (elem->text[0]) {
                render_text(elem->x + 4, elem->y + 4, elem->text, elem->text_color);
            }
            break;
    }

    /* Render children */
    UIElement* child = elem->child;
    while (child) {
        if (child->flags & UI_FLAG_VISIBLE) {
            ui_render_element(child);
        }
        child = child->next;
    }
}

void ui_render_button(UIButton* btn) {
    UIElement* e = &btn->base;
    u32 bg = 0x0842;
    u32 border = COLOR_WHITE;
    u32 text_color = COLOR_WHITE;

    /* Determine colors based on state */
    switch (e->state) {
        case UI_STATE_HOVER:
            bg = 0x1084;
            border = 0x18C6;
            break;
        case UI_STATE_PRESSED:
            bg = 0x184A;
            border = COLOR_YELLOW;
            break;
        case UI_STATE_DISABLED:
            bg = 0x0421;
            border = 0x8410;
            text_color = 0x8410;
            break;
        default:
            break;
    }

    /* Draw button background */
    render_fill_rect(NULL, e->x, e->y, e->width, e->height, bg);

    /* Draw border (2 pixels for 3D effect) */
    render_draw_rect(NULL, e->x, e->y, e->width, e->height, border);
    render_draw_rect(NULL, e->x + 1, e->y + 1, e->width - 2, e->height - 2,
                     e->state == UI_STATE_PRESSED ? 0x8410 : 0x1084);

    /* Draw button text */
    if (e->text[0]) {
        int text_x = e->x + e->width / 2;
        int text_y = e->y + (e->height - 14) / 2;
        render_text_centered(text_x, text_y, e->text, text_color);
    }
}

void ui_render_textbox(UITextbox* tb) {
    UIElement* e = &tb->base;
    u32 bg = 0x0000;
    u32 border = tb->is_focused ? COLOR_YELLOW : COLOR_WHITE;

    render_fill_rect(NULL, e->x, e->y, e->width, e->height, bg);
    render_draw_rect(NULL, e->x, e->y, e->width, e->height, border);

    if (tb->buffer[0]) {
        render_text(e->x + 4, e->y + 4, tb->buffer, COLOR_WHITE);
    }

    /* Draw cursor if focused */
    if (tb->is_focused) {
        int cursor_x = e->x + 4 + render_text_width(tb->buffer);
        render_draw_line(NULL, cursor_x, e->y + 4, cursor_x, e->y + e->height - 4, COLOR_WHITE);
    }
}

void ui_render_dialog(UIDialog* dlg) {
    UIElement* e = &dlg->base;
    int title_height = 24;
    int border_width = 2;
    u32 bg_color = 0x0842;     /* Dark background */
    u32 title_color = 0x1084;  /* Lighter title bar */
    u32 border_color = COLOR_WHITE;

    /* Outer border */
    render_draw_rect(NULL, e->x, e->y, e->width, e->height, border_color);
    render_draw_rect(NULL, e->x + 1, e->y + 1, e->width - 2, e->height - 2, 0x8410);

    /* Title bar background */
    render_fill_rect(NULL, e->x + border_width, e->y + border_width,
                     e->width - border_width * 2, title_height, title_color);

    /* Body background */
    render_fill_rect(NULL, e->x + border_width, e->y + title_height + border_width,
                     e->width - border_width * 2, e->height - title_height - border_width * 2,
                     bg_color);

    /* Title bar text */
    if (dlg->title[0]) {
        render_text(e->x + 8, e->y + 6, dlg->title, COLOR_WHITE);
    }

    /* Close button placeholder (top-right corner) */
    render_fill_rect(NULL, e->x + e->width - 20, e->y + 4, 14, 14, 0x0421);
    render_draw_rect(NULL, e->x + e->width - 20, e->y + 4, 14, 14, COLOR_WHITE);
}

void ui_render_progressbar(UIProgressbar* pb) {
    UIElement* e = &pb->base;
    int fill_width = 0;

    render_fill_rect(NULL, e->x, e->y, e->width, e->height, 0x0421);
    render_draw_rect(NULL, e->x, e->y, e->width, e->height, COLOR_WHITE);

    if (pb->max_value > 0) {
        fill_width = (e->width - 2) * pb->value / pb->max_value;
        render_fill_rect(NULL, e->x + 1, e->y + 1, fill_width, e->height - 2, pb->fill_color);
    }
}

void ui_render_listbox(UIListbox* lb) {
    UIElement* e = &lb->base;
    int i, y;

    render_fill_rect(NULL, e->x, e->y, e->width, e->height, 0x0000);
    render_draw_rect(NULL, e->x, e->y, e->width, e->height, COLOR_WHITE);

    for (i = lb->scroll_offset; i < lb->item_count; i++) {
        y = e->y + (i - lb->scroll_offset) * lb->item_height;
        if (y >= e->y + e->height) break;

        if (i == lb->selected_index) {
            render_fill_rect(NULL, e->x, y, e->width, lb->item_height, 0x1084);
        }

        if (lb->items && lb->items[i]) {
            render_text(e->x + 4, y + 2, lb->items[i], COLOR_WHITE);
        }
    }
}

void ui_render_checkbox(UICheckbox* cb) {
    UIElement* e = &cb->base;

    /* Draw checkbox sprite */
    render_fill_rect(NULL, e->x, e->y, e->width, e->height,
                     cb->checked ? 0x1084 : 0x0421);
    render_draw_rect(NULL, e->x, e->y, e->width, e->height, COLOR_WHITE);

    /* Draw checkmark if checked */
    if (cb->checked) {
        render_draw_line(NULL, e->x + 3, e->y + 8, e->x + 6, e->y + 12, COLOR_WHITE);
        render_draw_line(NULL, e->x + 6, e->y + 12, e->x + 13, e->y + 4, COLOR_WHITE);
    }

    /* Draw label */
    if (e->text[0]) {
        render_text(e->x + 20, e->y + 2, e->text, COLOR_WHITE);
    }
}

/* ========================================
 * Text Rendering - FUN_0041d7c0, FUN_0041d890
 * ======================================== */

int ui_render_text(int x, int y, const char* text, int palette, int flags) {
    if (!text || !text[0]) return 0;

    /* Add to text queue */
    if (g_ui.text_queue_count < TEXT_QUEUE_MAX_ENTRIES) {
        UITextAreaEntry* entry = &g_ui.text_queue[g_ui.text_queue_count++];
        entry->x = x;
        entry->y = y;
        strncpy(entry->text, text, TEXT_MAX_LENGTH - 1);
        entry->text[TEXT_MAX_LENGTH - 1] = '\0';
        entry->palette = palette;
        entry->flags = flags;
        entry->line_height = 14;

        return 1;
    }

    return 0;
}

int ui_render_text_multiline(int x, int y, int w, int h, const char* text,
                              int palette, int line_height) {
    if (!text || !text[0]) return 0;

    const char* ptr = text;
    int current_y = y;
    int lines = 0;
    char line[TEXT_MAX_LENGTH];
    int line_len = 0;

    while (*ptr && current_y < y + h) {
        /* Build line until width limit or newline */
        line_len = 0;
        while (*ptr && *ptr != '\n' && line_len < TEXT_MAX_LENGTH - 1) {
            line[line_len++] = *ptr++;

            /* Check width */
            int text_w = render_text_width_n(line, line_len);
            if (text_w > w - 8) {
                /* Back up to last space */
                while (line_len > 0 && line[line_len - 1] != ' ') {
                    line_len--;
                    ptr--;
                }
                break;
            }
        }

        if (*ptr == '\n') ptr++;

        line[line_len] = '\0';

        /* Render line */
        if (line_len > 0) {
            ui_render_text(x, current_y, line, palette, 0);
            current_y += line_height;
            lines++;
        }
    }

    return lines;
}

void ui_render_text_cursor(int x, int y, int cursor_pos) {
    int cursor_x = x + render_text_width_n(NULL, cursor_pos);

    /* Blink effect - 500ms on/off */
    u32 time = GetTickCount();
    if ((time / 500) % 2 == 0) {
        render_draw_line(NULL, cursor_x, y, cursor_x, y + 14, COLOR_WHITE);
    }
}

/* ========================================
 * Sprite-based Rendering
 * ======================================== */

void ui_render_sprite_button(u32 sprite_id, int x, int y, int state) {
    /* State-based palette adjustment */
    u32 color_mod = COLOR_WHITE;

    switch (state) {
        case UI_STATE_PRESSED:
            color_mod = 0x8410;
            break;
        case UI_STATE_HOVER:
            color_mod = 0xDEF7;
            break;
        case UI_STATE_DISABLED:
            color_mod = 0x4210;
            break;
    }

    /* Render sprite - color_mod not supported in basic render_sprite */
    (void)color_mod;  /* Suppress unused warning */
    render_sprite(sprite_id, x, y);
}

void ui_render_sprite_dialog(u32 sprite_id, int x, int y, int w, int h) {
    /* 9-slice rendering for dialog */
    int corner_size = 8;
    int edge_w = w - corner_size * 2;
    int edge_h = h - corner_size * 2;

    /* Corners */
    render_sprite(sprite_id + 0, x, y);
    render_sprite(sprite_id + 1, x + w - corner_size, y);
    render_sprite(sprite_id + 2, x, y + h - corner_size);
    render_sprite(sprite_id + 3, x + w - corner_size, y + h - corner_size);

    /* Edges */
    render_sprite_scaled(sprite_id + 4, x + corner_size, y, edge_w, corner_size);
    render_sprite_scaled(sprite_id + 5, x + corner_size, y + h - corner_size, edge_w, corner_size);
    render_sprite_scaled(sprite_id + 6, x, y + corner_size, corner_size, edge_h);
    render_sprite_scaled(sprite_id + 7, x + w - corner_size, y + corner_size, corner_size, edge_h);

    /* Center */
    render_sprite_scaled(sprite_id + 8, x + corner_size, y + corner_size, edge_w, edge_h);
}

/* ========================================
 * Screen Rendering
 * ======================================== */

void ui_render_login_screen(void) {
    extern LoginContext g_login;
    int center_x = 320;
    int center_y = 240;
    int dialog_width = 300;
    int dialog_height = 280;
    int dialog_x = center_x - dialog_width / 2;
    int dialog_y = center_y - dialog_height / 2;
    int mouse_x = 0, mouse_y = 0;
    DWORD tick;
    int cursor_visible;

    /* Background sprite 0x66bd at center - FUN_00420b70 pattern */
    render_queue_add_sprite(0x140, 0xf0, 0, 0x66bd, 0);

    /* Solid background behind sprites (fallback when no sprite data) */
    render_fill_rect(NULL, 0, 0, 640, 480, 0x0000);

    /* Title logo area */
    render_text_centered(center_x, 60, "Stone Age", COLOR_YELLOW);

    /* Login dialog box */
    render_fill_rect(NULL, dialog_x, dialog_y, dialog_width, dialog_height, 0x0421);
    render_draw_rect(NULL, dialog_x, dialog_y, dialog_width, dialog_height, COLOR_WHITE);
    render_draw_rect(NULL, dialog_x + 1, dialog_y + 1, dialog_width - 2, dialog_height - 2, 0x8410);

    /* Title bar */
    render_fill_rect(NULL, dialog_x + 2, dialog_y + 2, dialog_width - 4, 22, 0x1084);
    render_text(dialog_x + 10, dialog_y + 5, "Login", COLOR_WHITE);

    /* Server name and player count - FUN_00420590 state 3+ */
    if (g_login.server_name[0]) {
        char server_info[64];
        _snprintf(server_info, sizeof(server_info), "%s (%d)",
                  g_login.server_name, g_login.server_player_count);
        render_text(dialog_x + dialog_width - 100, dialog_y + 5, server_info, COLOR_CYAN);
    }

    /* Username field */
    render_text(dialog_x + 20, dialog_y + 40, "Username:", COLOR_WHITE);
    render_fill_rect(NULL, dialog_x + 20, dialog_y + 58, dialog_width - 40, 22, 0x0000);
    render_draw_rect(NULL, dialog_x + 20, dialog_y + 58, dialog_width - 40, 22,
                     g_login.input_focus == 0 ? COLOR_YELLOW : COLOR_WHITE);
    if (g_login.username[0]) {
        render_text(dialog_x + 25, dialog_y + 61, g_login.username, COLOR_WHITE);
    }

    /* Password field - shows asterisks */
    render_text(dialog_x + 20, dialog_y + 90, "Password:", COLOR_WHITE);
    render_fill_rect(NULL, dialog_x + 20, dialog_y + 108, dialog_width - 40, 22, 0x0000);
    render_draw_rect(NULL, dialog_x + 20, dialog_y + 108, dialog_width - 40, 22,
                     g_login.input_focus == 1 ? COLOR_YELLOW : COLOR_WHITE);
    if (g_login.password[0]) {
        char masked[32];
        int i;
        int len = (int)strlen(g_login.password);
        for (i = 0; i < len && i < 31; i++) masked[i] = '*';
        masked[i] = '\0';
        render_text(dialog_x + 25, dialog_y + 111, masked, COLOR_WHITE);
    }

    /* Blinking cursor on active field */
    tick = GetTickCount();
    cursor_visible = (tick / 500) % 2 == 0;
    if (cursor_visible) {
        int cursor_x_pos;
        int cursor_y_pos;
        if (g_login.input_focus == 0) {
            cursor_x_pos = dialog_x + 25 + (int)strlen(g_login.username) * 8;
            cursor_y_pos = dialog_y + 58;
        } else {
            cursor_x_pos = dialog_x + 25 + (int)strlen(g_login.password) * 8;
            cursor_y_pos = dialog_y + 108;
        }
        render_fill_rect(NULL, cursor_x_pos, cursor_y_pos + 2, 2, 18, COLOR_WHITE);
    }

    /* Login button - sprite 0x66be, hit area (320,225)-(410,350) from FUN_00420b70 */
    input_get_mouse_pos(&mouse_x, &mouse_y);
    if (mouse_x >= 320 && mouse_x <= 410 && mouse_y >= 225 && mouse_y <= 350) {
        render_queue_add_sprite(0x140, 0xf0, 0, 0x66be, 1);
    }
    render_fill_rect(NULL, dialog_x + 40, dialog_y + 150, 100, 26, 0x1084);
    render_draw_rect(NULL, dialog_x + 40, dialog_y + 150, 100, 26, COLOR_WHITE);
    render_text_centered(dialog_x + 90, dialog_y + 156, "Login", COLOR_WHITE);

    /* Exit button - sprite 0x66bf, hit area (412,232)-(494,372) from FUN_00420b70 */
    if (mouse_x >= 412 && mouse_x <= 494 && mouse_y >= 232 && mouse_y <= 372) {
        render_queue_add_sprite(0x140, 0xf0, 0, 0x66bf, 1);
    }
    render_fill_rect(NULL, dialog_x + 160, dialog_y + 150, 100, 26, 0x0842);
    render_draw_rect(NULL, dialog_x + 160, dialog_y + 150, 100, 26, COLOR_WHITE);
    render_text_centered(dialog_x + 210, dialog_y + 156, "Exit", COLOR_WHITE);

    /* Status text */
    if (g_login.screen_state == LOGIN_SCREEN_ERROR && g_login.error_message[0]) {
        render_text_centered(center_x, dialog_y + 200, g_login.error_message, COLOR_RED);
    } else if (g_login.screen_state == LOGIN_SCREEN_CONNECTING ||
               g_login.screen_state == LOGIN_SCREEN_WAITING) {
        render_text_centered(center_x, dialog_y + 200, "Connecting...", COLOR_CYAN);
    } else {
        render_text_centered(center_x, dialog_y + 200, "Tab: switch field  |  Enter: login",
                             COLOR_GRAY);
    }

    /* Input hint */
    render_text_centered(center_x, dialog_y + 230, "Press Tab to switch fields",
                         0x8410);
}

void ui_render_character_select_screen(void) {
    int i;

    /* Background */
    render_fill_rect(NULL, 0, 0, 640, 480, 0x0842);

    /* Title */
    render_text_centered(320, 40, "Select Character", COLOR_YELLOW);

    /* Character slots */
    for (i = 0; i < 3; i++) {
        int x = 120 + i * 180;
        int y = 120;

        /* Slot background */
        render_fill_rect(NULL, x, y, 160, 280, 0x0421);
        render_draw_rect(NULL, x, y, 160, 280, COLOR_WHITE);

        /* Character preview placeholder */
        render_fill_rect(NULL, x + 30, y + 20, 100, 120, 0x0842);

        /* Character info placeholder */
        render_text(x + 10, y + 160, "Character Name", COLOR_WHITE);
        render_text(x + 10, y + 180, "Level: 1", COLOR_WHITE);
        render_text(x + 10, y + 200, "Job: Warrior", COLOR_WHITE);
    }

    /* Create character button */
    render_fill_rect(NULL, 220, 420, 200, 30, 0x1084);
    render_draw_rect(NULL, 220, 420, 200, 30, COLOR_WHITE);
    render_text_centered(320, 430, "Create New Character", COLOR_WHITE);
}

void ui_render_character_create_screen(void) {
    int center_x = 320;
    int dialog_x = 160;
    int dialog_y = 60;

    /* Background */
    render_fill_rect(NULL, 0, 0, 640, 480, 0x0842);

    /* Dialog background */
    render_fill_rect(NULL, dialog_x, dialog_y, 320, 360, 0x1084);
    render_draw_rect(NULL, dialog_x, dialog_y, 320, 360, COLOR_WHITE);

    /* Title */
    render_text_centered(center_x, dialog_y + 20, "Create Character", COLOR_YELLOW);

    /* Name input */
    render_text(dialog_x + 20, dialog_y + 60, "Name:", COLOR_WHITE);
    render_fill_rect(NULL, dialog_x + 100, dialog_y + 55, 200, 24, 0x0000);
    render_draw_rect(NULL, dialog_x + 100, dialog_y + 55, 200, 24, COLOR_WHITE);

    /* Gender selection */
    render_text(dialog_x + 20, dialog_y + 100, "Gender:", COLOR_WHITE);
    render_fill_rect(NULL, dialog_x + 100, dialog_y + 95, 80, 24, 0x2108);
    render_text(dialog_x + 120, dialog_y + 102, "Male", COLOR_WHITE);
    render_fill_rect(NULL, dialog_x + 200, dialog_y + 95, 80, 24, 0x0842);
    render_text(dialog_x + 220, dialog_y + 102, "Female", COLOR_WHITE);

    /* Job selection */
    render_text(dialog_x + 20, dialog_y + 140, "Job:", COLOR_WHITE);
    render_fill_rect(NULL, dialog_x + 100, dialog_y + 135, 200, 24, 0x2108);
    render_text(dialog_x + 120, dialog_y + 142, "Warrior", COLOR_WHITE);

    /* Stats preview */
    render_text(dialog_x + 20, dialog_y + 180, "Stats:", COLOR_YELLOW);
    render_text(dialog_x + 30, dialog_y + 205, "STR: 10", COLOR_WHITE);
    render_text(dialog_x + 30, dialog_y + 225, "DEX: 10", COLOR_WHITE);
    render_text(dialog_x + 30, dialog_y + 245, "VIT: 10", COLOR_WHITE);
    render_text(dialog_x + 150, dialog_y + 205, "INT: 10", COLOR_WHITE);
    render_text(dialog_x + 150, dialog_y + 225, "AGI: 10", COLOR_WHITE);
    render_text(dialog_x + 150, dialog_y + 245, "LUK: 10", COLOR_WHITE);

    /* Character preview area */
    render_fill_rect(NULL, dialog_x + 20, dialog_y + 280, 120, 100, 0x0421);
    render_draw_rect(NULL, dialog_x + 20, dialog_y + 280, 120, 100, COLOR_GRAY);
    render_text_centered(dialog_x + 80, dialog_y + 325, "Preview", COLOR_GRAY);

    /* Buttons */
    render_fill_rect(NULL, dialog_x + 60, dialog_y + 395, 80, 28, 0x1084);
    render_draw_rect(NULL, dialog_x + 60, dialog_y + 395, 80, 28, COLOR_WHITE);
    render_text_centered(dialog_x + 100, dialog_y + 403, "Create", COLOR_WHITE);

    render_fill_rect(NULL, dialog_x + 180, dialog_y + 395, 80, 28, 0x1084);
    render_draw_rect(NULL, dialog_x + 180, dialog_y + 395, 80, 28, COLOR_WHITE);
    render_text_centered(dialog_x + 220, dialog_y + 403, "Cancel", COLOR_WHITE);
}

/* ========================================
 * Chat Rendering
 * ======================================== */

void ui_chat_render(void) {
    extern ChatContext g_chat;
    int i, y;
    int chat_x = 10;
    int chat_y = 320;
    int chat_w = 300;
    int chat_h = 150;

    /* Chat background */
    render_fill_rect(NULL, chat_x, chat_y, chat_w, chat_h, 0x0421);
    render_draw_rect(NULL, chat_x, chat_y, chat_w, chat_h, 0x8410);

    /* Messages */
    y = chat_y + chat_h - 20;
    for (i = g_chat.count - 1; i >= 0 && y > chat_y; i--) {
        render_text(chat_x + 4, y, g_chat.messages[i], COLOR_WHITE);
        y -= 14;
    }

    /* Input box */
    render_fill_rect(NULL, chat_x, chat_y + chat_h, chat_w, 20, 0x0000);
    render_draw_rect(NULL, chat_x, chat_y + chat_h, chat_w, 20, COLOR_WHITE);
    if (g_chat.input[0]) {
        render_text(chat_x + 4, chat_y + chat_h + 3, g_chat.input, COLOR_WHITE);
    }
}
