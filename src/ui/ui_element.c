/*
 * Stone Age Client - UI Element Management
 * Element creation, manipulation, and hierarchy
 */

#include <stdlib.h>
#include <string.h>
#include "ui.h"
#include "ui_element.h"
#include "logger.h"

/* ========================================
 * Element Creation Functions
 * ======================================== */

UIElement* ui_create_element(UIType type, int x, int y, int w, int h) {
    UIElement* elem = NULL;
    size_t size;

    switch (type) {
        case UI_BUTTON:
            size = sizeof(UIButton);
            break;
        case UI_TEXTBOX:
            size = sizeof(UITextbox);
            break;
        case UI_DIALOG:
            size = sizeof(UIDialog);
            break;
        case UI_PROGRESSBAR:
            size = sizeof(UIProgressbar);
            break;
        case UI_LISTBOX:
            size = sizeof(UIListbox);
            break;
        case UI_CHECKBOX:
            size = sizeof(UICheckbox);
            break;
        default:
            size = sizeof(UIElement);
            break;
    }

    elem = (UIElement*)calloc(1, size);
    if (!elem) return NULL;

    elem->type = type;
    elem->x = x;
    elem->y = y;
    elem->width = w;
    elem->height = h;
    elem->flags = UI_FLAG_VISIBLE | UI_FLAG_ENABLED;
    elem->state = UI_STATE_NORMAL;
    elem->text_palette = 0;

    return elem;
}

void ui_destroy_element(UIElement* elem) {
    if (!elem) return;

    /* Destroy children first */
    UIElement* child = elem->child;
    while (child) {
        UIElement* next = child->next;
        ui_destroy_element(child);
        child = next;
    }

    /* Free listbox items */
    if (elem->type == UI_LISTBOX) {
        UIListbox* lb = (UIListbox*)elem;
        if (lb->items) {
            int i;
            for (i = 0; i < lb->item_count; i++) {
                free(lb->items[i]);
            }
            free(lb->items);
        }
    }

    free(elem);
}

UIButton* ui_create_button(int x, int y, int w, int h, const char* text) {
    UIButton* btn = (UIButton*)ui_create_element(UI_BUTTON, x, y, w, h);
    if (btn && text) {
        strncpy(btn->base.text, text, sizeof(btn->base.text) - 1);
        btn->sprite_normal = UI_SPRITE_BUTTON_NORMAL;
        btn->sprite_pressed = UI_SPRITE_BUTTON_PRESSED;
        btn->sprite_hover = UI_SPRITE_BUTTON_HOVER;
        btn->sprite_disabled = UI_SPRITE_BUTTON_DISABLED;
        btn->base.flags |= UI_FLAG_FOCUSABLE;
    }
    return btn;
}

UITextbox* ui_create_textbox(int x, int y, int w, int h, int max_len) {
    UITextbox* tb = (UITextbox*)ui_create_element(UI_TEXTBOX, x, y, w, h);
    if (tb) {
        tb->max_length = max_len < 1024 ? max_len : 1023;
        tb->base.bg_sprite = UI_SPRITE_INPUT_BG;
        tb->base.flags |= UI_FLAG_FOCUSABLE;
    }
    return tb;
}

UIDialog* ui_create_dialog(int x, int y, int w, int h, const char* title) {
    UIDialog* dlg = (UIDialog*)ui_create_element(UI_DIALOG, x, y, w, h);
    if (dlg && title) {
        strncpy(dlg->title, title, sizeof(dlg->title) - 1);
        dlg->base.bg_sprite = UI_SPRITE_DIALOG_BG;
        dlg->base.flags |= UI_FLAG_DRAGGABLE | UI_FLAG_MODAL;
    }
    return dlg;
}

UIProgressbar* ui_create_progressbar(int x, int y, int w, int h) {
    UIProgressbar* pb = (UIProgressbar*)ui_create_element(UI_PROGRESSBAR, x, y, w, h);
    if (pb) {
        pb->max_value = 100;
        pb->fill_sprite = UI_SPRITE_PROGRESS_FILL;
        pb->base.bg_sprite = UI_SPRITE_PROGRESS_BG;
    }
    return pb;
}

UIListbox* ui_create_listbox(int x, int y, int w, int h) {
    UIListbox* lb = (UIListbox*)ui_create_element(UI_LISTBOX, x, y, w, h);
    if (lb) {
        lb->item_height = 20;
        lb->visible_items = h / 20;
        lb->base.bg_sprite = UI_SPRITE_LISTBG;
        lb->base.flags |= UI_FLAG_FOCUSABLE;
    }
    return lb;
}

UICheckbox* ui_create_checkbox(int x, int y, const char* text) {
    UICheckbox* cb = (UICheckbox*)ui_create_element(UI_CHECKBOX, x, y, 16, 16);
    if (cb && text) {
        strncpy(cb->base.text, text, sizeof(cb->base.text) - 1);
        cb->sprite_checked = UI_SPRITE_CHECKBOX_CHECKED;
        cb->sprite_unchecked = UI_SPRITE_CHECKBOX_UNCHECKED;
        cb->base.flags |= UI_FLAG_FOCUSABLE;
    }
    return cb;
}

/* ========================================
 * Element Manipulation Functions
 * ======================================== */

void ui_set_text(UIElement* elem, const char* text) {
    if (elem && text) {
        strncpy(elem->text, text, sizeof(elem->text) - 1);
    }
}

void ui_set_position(UIElement* elem, int x, int y) {
    if (elem) {
        elem->x = x;
        elem->y = y;
    }
}

void ui_set_size(UIElement* elem, int w, int h) {
    if (elem) {
        elem->width = w;
        elem->height = h;
    }
}

void ui_set_visible(UIElement* elem, int visible) {
    if (elem) {
        if (visible) {
            elem->flags |= UI_FLAG_VISIBLE;
        } else {
            elem->flags &= ~UI_FLAG_VISIBLE;
        }
    }
}

void ui_set_enabled(UIElement* elem, int enabled) {
    if (elem) {
        if (enabled) {
            elem->flags |= UI_FLAG_ENABLED;
            elem->state = UI_STATE_NORMAL;
        } else {
            elem->flags &= ~UI_FLAG_ENABLED;
            elem->state = UI_STATE_DISABLED;
        }
    }
}

void ui_set_callback(UIElement* elem, void (*callback)(UIElement*)) {
    if (elem) {
        elem->on_click = callback;
    }
}

void ui_set_sprites(UIElement* elem, u32 normal, u32 pressed, u32 hover, u32 disabled) {
    if (elem && elem->type == UI_BUTTON) {
        UIButton* btn = (UIButton*)elem;
        btn->sprite_normal = normal;
        btn->sprite_pressed = pressed;
        btn->sprite_hover = hover;
        btn->sprite_disabled = disabled;
    }
}

/* ========================================
 * Hierarchy Management
 * ======================================== */

void ui_add_child(UIElement* parent, UIElement* child) {
    if (!parent || !child) return;

    child->parent = parent;
    child->next = parent->child;
    parent->child = child;
}

void ui_remove_child(UIElement* parent, UIElement* child) {
    if (!parent || !child) return;

    UIElement** pp = &parent->child;
    while (*pp) {
        if (*pp == child) {
            *pp = child->next;
            child->parent = NULL;
            child->next = NULL;
            return;
        }
        pp = &(*pp)->next;
    }
}

/* ========================================
 * Focus Management
 * ======================================== */

void ui_set_focus(UIElement* elem) {
    if (g_ui.focused && g_ui.focused->on_blur) {
        g_ui.focused->on_blur(g_ui.focused);
    }

    g_ui.focused = elem;

    if (elem && elem->on_focus) {
        elem->on_focus(elem);
    }
}

UIElement* ui_get_focused(void) {
    return g_ui.focused;
}

/* ========================================
 * Hit Testing
 * ======================================== */

UIElement* ui_hit_test(int x, int y) {
    return ui_hit_test_recursive(g_ui.root, x, y);
}

UIElement* ui_hit_test_recursive(UIElement* elem, int x, int y) {
    UIElement* result = NULL;

    while (elem) {
        if ((elem->flags & UI_FLAG_VISIBLE) &&
            x >= elem->x && x < elem->x + elem->width &&
            y >= elem->y && y < elem->y + elem->height) {

            /* Check children first (they're on top) */
            if (elem->child) {
                result = ui_hit_test_recursive(elem->child, x, y);
                if (result) return result;
            }

            return elem;
        }
        elem = elem->next;
    }

    return NULL;
}
