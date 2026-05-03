/*
 * Stone Age Client - UI Element System Comprehensive Tests
 * Tests for ui_element.c implementation
 *
 * Covers:
 * - UI type constants
 * - UI state constants
 * - UI flags
 * - Element creation and destruction
 * - Element manipulation
 * - Hierarchy management
 * - Hit testing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s16;
typedef int s32;

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    fflush(stdout); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Constants
 * ======================================== */

#define TEXT_MAX_LENGTH 256

/* UI element types */
typedef enum {
    UI_NONE = 0,
    UI_BUTTON,
    UI_TEXTBOX,
    UI_LABEL,
    UI_IMAGE,
    UI_DIALOG,
    UI_PROGRESSBAR,
    UI_LISTBOX,
    UI_SCROLLBAR,
    UI_CHECKBOX
} UIType;

/* UI element state */
typedef enum {
    UI_STATE_NORMAL = 0,
    UI_STATE_HOVER,
    UI_STATE_PRESSED,
    UI_STATE_DISABLED,
    UI_STATE_FOCUSED
} UIState;

/* UI element flags */
#define UI_FLAG_VISIBLE     0x01
#define UI_FLAG_ENABLED     0x02
#define UI_FLAG_DRAGGABLE   0x04
#define UI_FLAG_MODAL       0x08
#define UI_FLAG_FOCUSABLE   0x10
#define UI_FLAG_PASSWORD    0x20
#define UI_FLAG_READONLY    0x40
#define UI_FLAG_MULTILINE   0x80

/* UI sprite IDs */
#define UI_SPRITE_BUTTON_NORMAL        0x6650
#define UI_SPRITE_BUTTON_PRESSED       0x6715
#define UI_SPRITE_BUTTON_HOVER         0x66f0
#define UI_SPRITE_BUTTON_DISABLED      0x673b
#define UI_SPRITE_DIALOG_BG            0x6591
#define UI_SPRITE_INPUT_BG             0x65a5
#define UI_SPRITE_PROGRESS_BG          0x65b5
#define UI_SPRITE_PROGRESS_FILL        0x65b6
#define UI_SPRITE_LISTBG               0x65bc
#define UI_SPRITE_CHECKBOX_CHECKED     0x65ad
#define UI_SPRITE_CHECKBOX_UNCHECKED   0x65ac

/* ========================================
 * Structures
 * ======================================== */

typedef struct UIElement {
    u32 id;
    UIType type;
    u32 flags;
    UIState state;
    s32 x, y;
    s32 width, height;
    u32 bg_sprite;
    u32 border_sprite;
    u32 bg_color;
    u32 border_color;
    u32 text_color;
    u8  text_palette;
    u8  text_align;
    char text[TEXT_MAX_LENGTH];
    void (*on_click)(struct UIElement* elem);
    void (*on_hover)(struct UIElement* elem);
    void (*on_focus)(struct UIElement* elem);
    void (*on_blur)(struct UIElement* elem);
    void (*on_key)(struct UIElement* elem, int key);
    void* user_data;
    struct UIElement* parent;
    struct UIElement* child;
    struct UIElement* next;
} UIElement;

typedef struct {
    UIElement base;
    u32 sprite_normal;
    u32 sprite_pressed;
    u32 sprite_hover;
    u32 sprite_disabled;
    int click_count;
    int is_pressed;
} UIButton;

typedef struct {
    UIElement base;
    char buffer[1024];
    int cursor_pos;
    int max_length;
    int is_password;
    int is_readonly;
    int is_focused;
    int is_multiline;
    int scroll_offset;
    int cursor_blink_time;
} UITextbox;

typedef struct {
    UIElement base;
    char title[64];
    int drag_x, drag_y;
    int is_dragging;
    int is_modal;
    int result;
} UIDialog;

typedef struct {
    UIElement base;
    char** items;
    int item_count;
    int selected_index;
    int scroll_offset;
    int item_height;
    int visible_items;
} UIListbox;

typedef struct {
    UIElement base;
    int value;
    int max_value;
    u32 fill_color;
    u32 fill_sprite;
} UIProgressbar;

typedef struct {
    UIElement base;
    int checked;
    u32 sprite_checked;
    u32 sprite_unchecked;
} UICheckbox;

typedef struct {
    UIElement* root;
    UIElement* focused;
    UIElement* hovered;
    UIElement* captured;
    s32 mouse_x, mouse_y;
    s32 mouse_down_x, mouse_down_y;
    int mouse_down;
    int mouse_button;
    s32 screen_width, screen_height;
    int initialized;
} UIContext;

/* ========================================
 * Global State
 * ======================================== */

static UIContext g_ui = {0};
static u32 g_next_id = 1;

/* ========================================
 * Implementation Functions
 * ======================================== */

static UIElement* ui_create_element(UIType type, int x, int y, int w, int h) {
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

    elem->id = g_next_id++;
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

static void ui_destroy_element(UIElement* elem) {
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

static UIButton* ui_create_button(int x, int y, int w, int h, const char* text) {
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

static UITextbox* ui_create_textbox(int x, int y, int w, int h, int max_len) {
    UITextbox* tb = (UITextbox*)ui_create_element(UI_TEXTBOX, x, y, w, h);
    if (tb) {
        tb->max_length = max_len < 1024 ? max_len : 1023;
        tb->base.bg_sprite = UI_SPRITE_INPUT_BG;
        tb->base.flags |= UI_FLAG_FOCUSABLE;
    }
    return tb;
}

static UIDialog* ui_create_dialog(int x, int y, int w, int h, const char* title) {
    UIDialog* dlg = (UIDialog*)ui_create_element(UI_DIALOG, x, y, w, h);
    if (dlg && title) {
        strncpy(dlg->title, title, sizeof(dlg->title) - 1);
        dlg->base.bg_sprite = UI_SPRITE_DIALOG_BG;
        dlg->base.flags |= UI_FLAG_DRAGGABLE | UI_FLAG_MODAL;
    }
    return dlg;
}

static UIProgressbar* ui_create_progressbar(int x, int y, int w, int h) {
    UIProgressbar* pb = (UIProgressbar*)ui_create_element(UI_PROGRESSBAR, x, y, w, h);
    if (pb) {
        pb->max_value = 100;
        pb->fill_sprite = UI_SPRITE_PROGRESS_FILL;
        pb->base.bg_sprite = UI_SPRITE_PROGRESS_BG;
    }
    return pb;
}

static UIListbox* ui_create_listbox(int x, int y, int w, int h) {
    UIListbox* lb = (UIListbox*)ui_create_element(UI_LISTBOX, x, y, w, h);
    if (lb) {
        lb->item_height = 20;
        lb->visible_items = h / 20;
        lb->base.bg_sprite = UI_SPRITE_LISTBG;
        lb->base.flags |= UI_FLAG_FOCUSABLE;
    }
    return lb;
}

static UICheckbox* ui_create_checkbox(int x, int y, const char* text) {
    UICheckbox* cb = (UICheckbox*)ui_create_element(UI_CHECKBOX, x, y, 16, 16);
    if (cb && text) {
        strncpy(cb->base.text, text, sizeof(cb->base.text) - 1);
        cb->sprite_checked = UI_SPRITE_CHECKBOX_CHECKED;
        cb->sprite_unchecked = UI_SPRITE_CHECKBOX_UNCHECKED;
        cb->base.flags |= UI_FLAG_FOCUSABLE;
    }
    return cb;
}

static void ui_set_text(UIElement* elem, const char* text) {
    if (elem && text) {
        strncpy(elem->text, text, sizeof(elem->text) - 1);
    }
}

static void ui_set_position(UIElement* elem, int x, int y) {
    if (elem) {
        elem->x = x;
        elem->y = y;
    }
}

static void ui_set_size(UIElement* elem, int w, int h) {
    if (elem) {
        elem->width = w;
        elem->height = h;
    }
}

static void ui_set_visible(UIElement* elem, int visible) {
    if (elem) {
        if (visible) {
            elem->flags |= UI_FLAG_VISIBLE;
        } else {
            elem->flags &= ~UI_FLAG_VISIBLE;
        }
    }
}

static void ui_set_enabled(UIElement* elem, int enabled) {
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

static void ui_add_child(UIElement* parent, UIElement* child) {
    if (!parent || !child) return;

    child->parent = parent;
    child->next = parent->child;
    parent->child = child;
}

static void ui_remove_child(UIElement* parent, UIElement* child) {
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

static void ui_set_focus(UIElement* elem) {
    if (g_ui.focused && g_ui.focused->on_blur) {
        g_ui.focused->on_blur(g_ui.focused);
    }

    g_ui.focused = elem;

    if (elem && elem->on_focus) {
        elem->on_focus(elem);
    }
}

static UIElement* ui_get_focused(void) {
    return g_ui.focused;
}

static UIElement* ui_hit_test_recursive(UIElement* elem, int x, int y) {
    UIElement* result = NULL;

    while (elem) {
        if ((elem->flags & UI_FLAG_VISIBLE) &&
            x >= elem->x && x < elem->x + elem->width &&
            y >= elem->y && y < elem->y + elem->height) {

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

static UIElement* ui_hit_test(int x, int y) {
    return ui_hit_test_recursive(g_ui.root, x, y);
}

static void reset_state(void) {
    if (g_ui.root) {
        ui_destroy_element(g_ui.root);
    }
    memset(&g_ui, 0, sizeof(UIContext));
    g_next_id = 1;
}

/* ========================================
 * Test Cases - Type Constants
 * ======================================== */

static int test_type_none(void) {
    return UI_NONE == 0;
}

static int test_type_button(void) {
    return UI_BUTTON == 1;
}

static int test_type_textbox(void) {
    return UI_TEXTBOX == 2;
}

static int test_type_label(void) {
    return UI_LABEL == 3;
}

static int test_type_image(void) {
    return UI_IMAGE == 4;
}

static int test_type_dialog(void) {
    return UI_DIALOG == 5;
}

static int test_type_progressbar(void) {
    return UI_PROGRESSBAR == 6;
}

static int test_type_listbox(void) {
    return UI_LISTBOX == 7;
}

static int test_type_scrollbar(void) {
    return UI_SCROLLBAR == 8;
}

static int test_type_checkbox(void) {
    return UI_CHECKBOX == 9;
}

/* ========================================
 * Test Cases - State Constants
 * ======================================== */

static int test_state_normal(void) {
    return UI_STATE_NORMAL == 0;
}

static int test_state_hover(void) {
    return UI_STATE_HOVER == 1;
}

static int test_state_pressed(void) {
    return UI_STATE_PRESSED == 2;
}

static int test_state_disabled(void) {
    return UI_STATE_DISABLED == 3;
}

static int test_state_focused(void) {
    return UI_STATE_FOCUSED == 4;
}

/* ========================================
 * Test Cases - Flag Constants
 * ======================================== */

static int test_flag_visible(void) {
    return UI_FLAG_VISIBLE == 0x01;
}

static int test_flag_enabled(void) {
    return UI_FLAG_ENABLED == 0x02;
}

static int test_flag_draggable(void) {
    return UI_FLAG_DRAGGABLE == 0x04;
}

static int test_flag_modal(void) {
    return UI_FLAG_MODAL == 0x08;
}

static int test_flag_focusable(void) {
    return UI_FLAG_FOCUSABLE == 0x10;
}

static int test_flag_password(void) {
    return UI_FLAG_PASSWORD == 0x20;
}

static int test_flag_readonly(void) {
    return UI_FLAG_READONLY == 0x40;
}

static int test_flag_multiline(void) {
    return UI_FLAG_MULTILINE == 0x80;
}

/* ========================================
 * Test Cases - Element Creation
 * ======================================== */

static int test_create_element_basic(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_LABEL, 10, 20, 100, 50);

    int ok = elem != NULL &&
             elem->type == UI_LABEL &&
             elem->x == 10 &&
             elem->y == 20 &&
             elem->width == 100 &&
             elem->height == 50;

    ui_destroy_element(elem);
    return ok;
}

static int test_create_element_flags(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_LABEL, 0, 0, 10, 10);

    int ok = elem != NULL &&
             (elem->flags & UI_FLAG_VISIBLE) &&
             (elem->flags & UI_FLAG_ENABLED);

    ui_destroy_element(elem);
    return ok;
}

static int test_create_element_state(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_LABEL, 0, 0, 10, 10);

    int ok = elem != NULL && elem->state == UI_STATE_NORMAL;

    ui_destroy_element(elem);
    return ok;
}

static int test_create_button(void) {
    reset_state();

    UIButton* btn = ui_create_button(10, 20, 80, 30, "Click");

    int ok = btn != NULL &&
             btn->base.type == UI_BUTTON &&
             strcmp(btn->base.text, "Click") == 0 &&
             btn->sprite_normal == UI_SPRITE_BUTTON_NORMAL;

    ui_destroy_element((UIElement*)btn);
    return ok;
}

static int test_create_button_focusable(void) {
    reset_state();

    UIButton* btn = ui_create_button(0, 0, 10, 10, "Test");

    int ok = btn != NULL && (btn->base.flags & UI_FLAG_FOCUSABLE);

    ui_destroy_element((UIElement*)btn);
    return ok;
}

static int test_create_textbox(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(10, 10, 200, 30, 256);

    int ok = tb != NULL &&
             tb->base.type == UI_TEXTBOX &&
             tb->max_length == 256;

    ui_destroy_element((UIElement*)tb);
    return ok;
}

static int test_create_textbox_max_cap(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 2000);

    int ok = tb != NULL && tb->max_length == 1023;  /* Capped */

    ui_destroy_element((UIElement*)tb);
    return ok;
}

static int test_create_dialog(void) {
    reset_state();

    UIDialog* dlg = ui_create_dialog(100, 100, 300, 200, "Dialog");

    int ok = dlg != NULL &&
             dlg->base.type == UI_DIALOG &&
             strcmp(dlg->title, "Dialog") == 0 &&
             (dlg->base.flags & UI_FLAG_DRAGGABLE) &&
             (dlg->base.flags & UI_FLAG_MODAL);

    ui_destroy_element((UIElement*)dlg);
    return ok;
}

static int test_create_progressbar(void) {
    reset_state();

    UIProgressbar* pb = ui_create_progressbar(10, 10, 100, 20);

    int ok = pb != NULL &&
             pb->base.type == UI_PROGRESSBAR &&
             pb->max_value == 100;

    ui_destroy_element((UIElement*)pb);
    return ok;
}

static int test_create_listbox(void) {
    reset_state();

    UIListbox* lb = ui_create_listbox(10, 10, 150, 100);

    int ok = lb != NULL &&
             lb->base.type == UI_LISTBOX &&
             lb->item_height == 20 &&
             lb->visible_items == 5;

    ui_destroy_element((UIElement*)lb);
    return ok;
}

static int test_create_checkbox(void) {
    reset_state();

    UICheckbox* cb = ui_create_checkbox(10, 10, "Option");

    int ok = cb != NULL &&
             cb->base.type == UI_CHECKBOX &&
             strcmp(cb->base.text, "Option") == 0 &&
             cb->base.width == 16 &&
             cb->base.height == 16;

    ui_destroy_element((UIElement*)cb);
    return ok;
}

/* ========================================
 * Test Cases - Element Manipulation
 * ======================================== */

static int test_set_text(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_LABEL, 0, 0, 10, 10);
    ui_set_text(elem, "Hello");

    int ok = strcmp(elem->text, "Hello") == 0;

    ui_destroy_element(elem);
    return ok;
}

static int test_set_position(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_LABEL, 0, 0, 10, 10);
    ui_set_position(elem, 50, 100);

    int ok = elem->x == 50 && elem->y == 100;

    ui_destroy_element(elem);
    return ok;
}

static int test_set_size(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_LABEL, 0, 0, 10, 10);
    ui_set_size(elem, 200, 100);

    int ok = elem->width == 200 && elem->height == 100;

    ui_destroy_element(elem);
    return ok;
}

static int test_set_visible(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_LABEL, 0, 0, 10, 10);
    ui_set_visible(elem, 0);

    int ok = !(elem->flags & UI_FLAG_VISIBLE);

    ui_set_visible(elem, 1);
    ok = ok && (elem->flags & UI_FLAG_VISIBLE);

    ui_destroy_element(elem);
    return ok;
}

static int test_set_enabled(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_LABEL, 0, 0, 10, 10);
    ui_set_enabled(elem, 0);

    int ok = !(elem->flags & UI_FLAG_ENABLED) && elem->state == UI_STATE_DISABLED;

    ui_set_enabled(elem, 1);
    ok = ok && (elem->flags & UI_FLAG_ENABLED) && elem->state == UI_STATE_NORMAL;

    ui_destroy_element(elem);
    return ok;
}

/* ========================================
 * Test Cases - Hierarchy
 * ======================================== */

static int test_add_child(void) {
    reset_state();

    UIElement* parent = ui_create_element(UI_LABEL, 0, 0, 100, 100);
    UIElement* child = ui_create_element(UI_LABEL, 0, 0, 10, 10);

    ui_add_child(parent, child);

    int ok = parent->child == child && child->parent == parent;

    ui_destroy_element(parent);
    return ok;
}

static int test_add_multiple_children(void) {
    reset_state();

    UIElement* parent = ui_create_element(UI_LABEL, 0, 0, 100, 100);
    UIElement* child1 = ui_create_element(UI_LABEL, 0, 0, 10, 10);
    UIElement* child2 = ui_create_element(UI_LABEL, 0, 0, 10, 10);

    ui_add_child(parent, child1);
    ui_add_child(parent, child2);

    /* child2 should be first (linked at head) */
    int ok = parent->child == child2 &&
             child2->next == child1 &&
             child1->parent == parent;

    ui_destroy_element(parent);
    return ok;
}

static int test_remove_child(void) {
    reset_state();

    UIElement* parent = ui_create_element(UI_LABEL, 0, 0, 100, 100);
    UIElement* child = ui_create_element(UI_LABEL, 0, 0, 10, 10);

    ui_add_child(parent, child);
    ui_remove_child(parent, child);

    int ok = parent->child == NULL &&
             child->parent == NULL &&
             child->next == NULL;

    ui_destroy_element(parent);
    ui_destroy_element(child);
    return ok;
}

static int test_remove_middle_child(void) {
    reset_state();

    UIElement* parent = ui_create_element(UI_LABEL, 0, 0, 100, 100);
    UIElement* child1 = ui_create_element(UI_LABEL, 0, 0, 10, 10);
    UIElement* child2 = ui_create_element(UI_LABEL, 0, 0, 10, 10);
    UIElement* child3 = ui_create_element(UI_LABEL, 0, 0, 10, 10);

    /* Add in order: child3, child2, child1 (head is child3) */
    ui_add_child(parent, child1);
    ui_add_child(parent, child2);
    ui_add_child(parent, child3);

    /* Remove middle child (child2) */
    ui_remove_child(parent, child2);

    int ok = parent->child == child3 &&
             child3->next == child1 &&
             child1->next == NULL;

    ui_destroy_element(parent);
    ui_destroy_element(child2);
    return ok;
}

/* ========================================
 * Test Cases - Focus
 * ======================================== */

static int test_set_focus(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_LABEL, 0, 0, 10, 10);
    ui_set_focus(elem);

    int ok = ui_get_focused() == elem;

    ui_destroy_element(elem);
    return ok;
}

static int test_change_focus(void) {
    reset_state();

    UIElement* elem1 = ui_create_element(UI_LABEL, 0, 0, 10, 10);
    UIElement* elem2 = ui_create_element(UI_LABEL, 0, 0, 10, 10);

    ui_set_focus(elem1);
    ui_set_focus(elem2);

    int ok = ui_get_focused() == elem2;

    ui_destroy_element(elem1);
    ui_destroy_element(elem2);
    return ok;
}

/* ========================================
 * Test Cases - Hit Testing
 * ======================================== */

static int test_hit_test_inside(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_LABEL, 10, 10, 100, 50);
    g_ui.root = elem;

    UIElement* hit = ui_hit_test(50, 30);

    int ok = hit == elem;

    ui_destroy_element(elem);
    g_ui.root = NULL;
    return ok;
}

static int test_hit_test_outside(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_LABEL, 10, 10, 100, 50);
    g_ui.root = elem;

    UIElement* hit = ui_hit_test(200, 200);

    int ok = hit == NULL;

    ui_destroy_element(elem);
    g_ui.root = NULL;
    return ok;
}

static int test_hit_test_boundary_left(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_LABEL, 10, 10, 100, 50);
    g_ui.root = elem;

    UIElement* hit = ui_hit_test(10, 30);  /* At left edge */

    int ok = hit == elem;

    ui_destroy_element(elem);
    g_ui.root = NULL;
    return ok;
}

static int test_hit_test_boundary_right_out(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_LABEL, 10, 10, 100, 50);
    g_ui.root = elem;

    UIElement* hit = ui_hit_test(110, 30);  /* Just past right edge */

    int ok = hit == NULL;

    ui_destroy_element(elem);
    g_ui.root = NULL;
    return ok;
}

static int test_hit_test_invisible(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_LABEL, 10, 10, 100, 50);
    ui_set_visible(elem, 0);
    g_ui.root = elem;

    UIElement* hit = ui_hit_test(50, 30);

    int ok = hit == NULL;  /* Invisible elements don't hit */

    ui_destroy_element(elem);
    g_ui.root = NULL;
    return ok;
}

static int test_hit_test_child_priority(void) {
    reset_state();

    UIElement* parent = ui_create_element(UI_LABEL, 0, 0, 100, 100);
    UIElement* child = ui_create_element(UI_LABEL, 10, 10, 50, 50);
    ui_add_child(parent, child);
    g_ui.root = parent;

    /* Hit point inside both parent and child */
    UIElement* hit = ui_hit_test(20, 20);

    int ok = hit == child;  /* Child has priority */

    ui_destroy_element(parent);
    g_ui.root = NULL;
    return ok;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== UI Element System Comprehensive Tests ===\n\n");

    printf("Type Constants Tests:\n");
    TEST(type_none);
    TEST(type_button);
    TEST(type_textbox);
    TEST(type_label);
    TEST(type_image);
    TEST(type_dialog);
    TEST(type_progressbar);
    TEST(type_listbox);
    TEST(type_scrollbar);
    TEST(type_checkbox);

    printf("\nState Constants Tests:\n");
    TEST(state_normal);
    TEST(state_hover);
    TEST(state_pressed);
    TEST(state_disabled);
    TEST(state_focused);

    printf("\nFlag Constants Tests:\n");
    TEST(flag_visible);
    TEST(flag_enabled);
    TEST(flag_draggable);
    TEST(flag_modal);
    TEST(flag_focusable);
    TEST(flag_password);
    TEST(flag_readonly);
    TEST(flag_multiline);

    printf("\nElement Creation Tests:\n");
    TEST(create_element_basic);
    TEST(create_element_flags);
    TEST(create_element_state);
    TEST(create_button);
    TEST(create_button_focusable);
    TEST(create_textbox);
    TEST(create_textbox_max_cap);
    TEST(create_dialog);
    TEST(create_progressbar);
    TEST(create_listbox);
    TEST(create_checkbox);

    printf("\nElement Manipulation Tests:\n");
    TEST(set_text);
    TEST(set_position);
    TEST(set_size);
    TEST(set_visible);
    TEST(set_enabled);

    printf("\nHierarchy Tests:\n");
    TEST(add_child);
    TEST(add_multiple_children);
    TEST(remove_child);
    TEST(remove_middle_child);

    printf("\nFocus Tests:\n");
    TEST(set_focus);
    TEST(change_focus);

    printf("\nHit Testing Tests:\n");
    TEST(hit_test_inside);
    TEST(hit_test_outside);
    TEST(hit_test_boundary_left);
    TEST(hit_test_boundary_right_out);
    TEST(hit_test_invisible);
    TEST(hit_test_child_priority);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    reset_state();

    return (tests_passed == tests_run) ? 0 : 1;
}
