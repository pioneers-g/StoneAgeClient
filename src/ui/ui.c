/*
 * Stone Age Client - UI System Implementation
 * Core module: initialization, update, main render, dialog management, chat
 * Reverse engineered from sa_9061.exe
 * FUN_004155e0 - UI sprite ID initialization
 * FUN_0041d7c0 - Text render queue
 * FUN_0041d890 - Multi-line text rendering
 */

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "ui.h"
#include "ui_element.h"
#include "ui_input.h"
#include "ui_render.h"
#include "logger.h"

/* Global UI context */
UIContext g_ui = {0};

/* Chat context */
ChatContext g_chat = {0};

/* ========================================
 * UI Sprite Initialization - FUN_004155e0
 * ======================================== */

void ui_init_sprites(int high_res_mode) {
    /* FUN_004155e0 - UI sprite ID initialization
     * Standard mode (DAT_0054c83c == 0): 0x6xxx sprite IDs
     * High-res mode (DAT_0054c83c != 0): 0x7axxx sprite IDs
     */
    g_ui.high_res_mode = high_res_mode;

    if (high_res_mode) {
        /* High-res mode sprites (0x7axxx series) */
        g_ui.sprites.btn_normal       = 0x7a145;
        g_ui.sprites.btn_pressed      = 0x7a155;
        g_ui.sprites.btn_hover        = 0x7a152;
        g_ui.sprites.btn_disabled     = 0x7a156;
        g_ui.sprites.scroll_up        = 0x7a157;
        g_ui.sprites.scroll_down      = 0x7a158;
        g_ui.sprites.scroll_track     = 0x7a153;
        g_ui.sprites.scroll_thumb     = 0x7a154;
        g_ui.sprites.scroll_left      = 0x7a150;
        g_ui.sprites.scroll_right     = 0x7a151;
        g_ui.sprites.dialog_bg        = 0x7a121;
        g_ui.sprites.dialog_corner_tl = 0x7a122;
        g_ui.sprites.dialog_corner_tr = 0x7a123;
        g_ui.sprites.dialog_corner_bl = 0x7a124;
        g_ui.sprites.dialog_corner_br = 0x7a125;
        g_ui.sprites.dialog_edge_top  = 0x7a126;
        g_ui.sprites.dialog_edge_bottom = 0x7a127;
        g_ui.sprites.dialog_edge_left = 0x7a128;
        g_ui.sprites.dialog_edge_right = 0x7a129;
        g_ui.sprites.input_bg         = 0x7a12a;
        g_ui.sprites.input_cursor     = 0x7a12b;
        g_ui.sprites.input_focus      = 0x7a12c;
        g_ui.sprites.input_normal     = 0x7a12d;
        g_ui.sprites.input_active     = 0x7a12e;
        g_ui.sprites.input_disabled   = 0x7a12f;
        g_ui.sprites.input_readonly   = 0x7a130;
        g_ui.sprites.checkbox_unchecked = 0x7a131;
        g_ui.sprites.checkbox_checked = 0x7a132;
        g_ui.sprites.progress_bg      = 0x7a133;
        g_ui.sprites.progress_fill    = 0x7a134;
        g_ui.sprites.progress_border  = 0x7a135;
        g_ui.sprites.progress_empty   = 0x7a136;
        g_ui.sprites.list_bg          = 0x7a137;
        g_ui.sprites.list_item_normal = 0x7a138;
        g_ui.sprites.list_item_selected = 0x7a139;
        g_ui.sprites.ext_slot1        = 0x7a13a;
        g_ui.sprites.ext_slot2        = 0x7a13b;
        g_ui.sprites.ext_slot3        = 0x7a13c;
        g_ui.sprites.ext_slot4        = 0x7a13d;
        g_ui.sprites.ext_slot5        = 0x7a2e4;
        g_ui.sprites.ext_slot6        = 0x7a13e;
        g_ui.sprites.ext_slot7        = 0x7a146;
        g_ui.sprites.ext_slot8        = 0x7a147;
        g_ui.sprites.ext_slot9        = 0x7a148;
        g_ui.sprites.ext_slot10       = 0x7a149;
        g_ui.sprites.ext_slot11       = 0x7a14a;
        g_ui.sprites.ext_slot12       = 0x7a13f;
        g_ui.sprites.ext_slot13       = 0x7a14d;
        g_ui.sprites.ext_slot14       = 0x7a14f;
        g_ui.sprites.ext_slot15       = 0x7a140;
        g_ui.sprites.ext_slot16       = 0x7a141;
        g_ui.sprites.ext_slot17       = 0x7a142;
        g_ui.sprites.ext_slot18       = 0x7a143;
        g_ui.sprites.ext_slot19       = 0x7a14b;
        g_ui.sprites.ext_slot20       = 0x7a14c;
        g_ui.sprites.ext_slot21       = 0x7a144;
        g_ui.sprites.ext_slot22       = 500000; /* Extended threshold */
    } else {
        /* Standard mode sprites (0x6xxx series) */
        g_ui.sprites.btn_normal       = 0x6650;
        g_ui.sprites.btn_pressed      = 0x6715;
        g_ui.sprites.btn_hover        = 0x66f0;
        g_ui.sprites.btn_disabled     = 0x673b;
        g_ui.sprites.scroll_up        = 0x8995;
        g_ui.sprites.scroll_down      = 0x8997;
        g_ui.sprites.scroll_track     = 0x6712;
        g_ui.sprites.scroll_thumb     = 0x6757;
        g_ui.sprites.scroll_left      = 0x66d8;
        g_ui.sprites.scroll_right     = 0x66d9;
        g_ui.sprites.dialog_bg        = 0x6591;
        g_ui.sprites.dialog_corner_tl = 0x6592;
        g_ui.sprites.dialog_corner_tr = 0x6593;
        g_ui.sprites.dialog_corner_bl = 0x6594;
        g_ui.sprites.dialog_corner_br = 0x6595;
        g_ui.sprites.dialog_edge_top  = 0x6596;
        g_ui.sprites.dialog_edge_bottom = 0x6597;
        g_ui.sprites.dialog_edge_left = 0x6598;
        g_ui.sprites.dialog_edge_right = 0x6599;
        g_ui.sprites.input_bg         = 0x65a5;
        g_ui.sprites.input_cursor     = 0x65a6;
        g_ui.sprites.input_focus      = 0x65a7;
        g_ui.sprites.input_normal     = 0x65a8;
        g_ui.sprites.input_active     = 0x65a9;
        g_ui.sprites.input_disabled   = 0x65aa;
        g_ui.sprites.input_readonly   = 0x65ab;
        g_ui.sprites.checkbox_unchecked = 0x65ac;
        g_ui.sprites.checkbox_checked = 0x65ad;
        g_ui.sprites.progress_bg      = 0x65b5;
        g_ui.sprites.progress_fill    = 0x65b6;
        g_ui.sprites.progress_border  = 0x65b7;
        g_ui.sprites.progress_empty   = 0x65b8;
        g_ui.sprites.list_bg          = 0x65bc;
        g_ui.sprites.list_item_normal = 0x65bd;
        g_ui.sprites.list_item_selected = 0x65c1;
        g_ui.sprites.ext_slot1        = 0x65cd;
        g_ui.sprites.ext_slot2        = 0x65d4;
        g_ui.sprites.ext_slot3        = 0x65d6;
        g_ui.sprites.ext_slot4        = 0x65d7;
        g_ui.sprites.ext_slot5        = 0x65e1;
        g_ui.sprites.ext_slot6        = 0x65d9;
        g_ui.sprites.ext_slot7        = 0x65e2;
        g_ui.sprites.ext_slot8        = 0x6658;
        g_ui.sprites.ext_slot9        = 0x6659;
        g_ui.sprites.ext_slot10       = 0x665b;
        g_ui.sprites.ext_slot11       = 0x6676;
        g_ui.sprites.ext_slot12       = 0x6678;
        g_ui.sprites.ext_slot13       = 0x65ea;
        g_ui.sprites.ext_slot14       = 0x6692;
        g_ui.sprites.ext_slot15       = 0x66b8;
        g_ui.sprites.ext_slot16       = 0x6612;
        g_ui.sprites.ext_slot17       = 0x661a;
        g_ui.sprites.ext_slot18       = 0x661b;
        g_ui.sprites.ext_slot19       = 0x661c;
        g_ui.sprites.ext_slot20       = 0x667f;
        g_ui.sprites.ext_slot21       = 0x6680;
        g_ui.sprites.ext_slot22       = 0x661d;
    }

    LOG_INFO("UI sprites initialized (high_res=%d)", high_res_mode);
}

/*
 * Initialize UI system
 */
int ui_init(void) {
    memset(&g_ui, 0, sizeof(UIContext));
    g_ui.screen_width = 640;
    g_ui.screen_height = 480;
    g_ui.initialized = 1;

    ui_chat_init();

    LOG_INFO("UI system initialized");
    return 1;
}

/*
 * Shutdown UI system
 */
void ui_shutdown(void) {
    /* Free all elements */
    while (g_ui.root) {
        ui_destroy_element(g_ui.root);
    }

    g_ui.initialized = 0;
    LOG_INFO("UI system shutdown");
}

/*
 * Update UI
 */
void ui_update(void) {
    if (!g_ui.initialized) return;

    /* Update element states */
    UIElement* elem = g_ui.root;
    while (elem) {
        if (!(elem->flags & UI_FLAG_ENABLED)) {
            elem->state = UI_STATE_DISABLED;
        }
        elem = elem->next;
    }
}

/*
 * Render UI
 */
void ui_render(void) {
    if (!g_ui.initialized) return;

    /* Render all root elements */
    UIElement* elem = g_ui.root;
    while (elem) {
        if (elem->flags & UI_FLAG_VISIBLE) {
            ui_render_element(elem);
        }
        elem = elem->next;
    }

    /* Render chat */
    ui_chat_render();
}

/* ========================================
 * Dialog Management
 * ======================================== */

void ui_show_dialog(UIDialog* dlg) {
    if (!dlg) return;

    dlg->base.flags |= UI_FLAG_VISIBLE;
    ui_add_child(g_ui.root, &dlg->base);

    if (dlg->is_modal) {
        ui_set_focus(&dlg->base);
    }
}

void ui_hide_dialog(UIDialog* dlg) {
    if (dlg) {
        dlg->base.flags &= ~UI_FLAG_VISIBLE;
    }
}

void ui_close_dialog(UIDialog* dlg) {
    if (dlg) {
        ui_remove_child(g_ui.root, &dlg->base);
        dlg->result = 0;
    }
}

int ui_dialog_get_result(UIDialog* dlg) {
    return dlg ? dlg->result : 0;
}

void ui_show_message(const char* title, const char* message) {
    /* Create simple message dialog */
    UIDialog* dlg = ui_create_dialog(170, 140, 300, 120, title);
    if (dlg) {
        strncpy(dlg->base.text, message, sizeof(dlg->base.text) - 1);
        ui_show_dialog(dlg);
    }
}

static void on_confirm_yes(UIElement* elem) {
    UIDialog* dlg = (UIDialog*)elem->parent;
    dlg->result = 1;
}

static void on_confirm_no(UIElement* elem) {
    UIDialog* dlg = (UIDialog*)elem->parent;
    dlg->result = 0;
}

void ui_show_confirm(const char* title, const char* message,
                     void (*on_yes)(void), void (*on_no)(void)) {
    UIDialog* dlg = ui_create_dialog(170, 140, 300, 150, title);
    if (dlg) {
        strncpy(dlg->base.text, message, sizeof(dlg->base.text) - 1);

        /* Create buttons */
        UIButton* btn_yes = ui_create_button(80, 100, 60, 28, "Yes");
        UIButton* btn_no = ui_create_button(160, 100, 60, 28, "No");

        if (btn_yes) {
            btn_yes->base.on_click = on_confirm_yes;
            ui_add_child(&dlg->base, &btn_yes->base);
        }
        if (btn_no) {
            btn_no->base.on_click = on_confirm_no;
            ui_add_child(&dlg->base, &btn_no->base);
        }

        ui_show_dialog(dlg);
    }
}

/* ========================================
 * Chat System
 * ======================================== */

void ui_chat_init(void) {
    memset(&g_chat, 0, sizeof(ChatContext));
}

void ui_chat_add_message(const char* sender, const char* message) {
    if (g_chat.count >= 100) {
        /* Scroll up */
        memmove(g_chat.messages[0], g_chat.messages[1], 99 * 512);
        g_chat.count = 99;
    }

    snprintf(g_chat.messages[g_chat.count], 512, "%s: %s", sender, message);
    g_chat.count++;
}

void ui_chat_input(const char* text) {
    if (text && text[0]) {
        ui_chat_add_message("You", text);
        /* Send to server */
    }
}

void ui_chat_clear(void) {
    g_chat.count = 0;
    g_chat.scroll = 0;
    g_chat.input[0] = '\0';
    g_chat.input_cursor = 0;
}
