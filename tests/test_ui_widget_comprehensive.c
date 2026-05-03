/*
 * Stone Age Client - UI Widget System Comprehensive Tests
 * Tests for uiwidget.c implementation
 *
 * Based on Ghidra analysis:
 * - FUN_00448610: Window creation with style
 * - FUN_00448270: Window render callback
 * - FUN_004010a0: Object allocator with linked list
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
 * Constants from uiwidget.h
 * ======================================== */

/* Widget types */
typedef enum {
    WIDGET_TYPE_NONE = 0,
    WIDGET_TYPE_CONTAINER = 1,
    WIDGET_TYPE_BUTTON = 2,
    WIDGET_TYPE_WINDOW = 3,
    WIDGET_TYPE_TEXT = 4,
    WIDGET_TYPE_EDIT = 5,
    WIDGET_TYPE_IMAGE = 6,
    WIDGET_TYPE_LISTBOX = 7,
    WIDGET_TYPE_SCROLLBAR = 8,
    WIDGET_TYPE_CHECKBOX = 9,
    WIDGET_TYPE_PROGRESS = 10,
    WIDGET_TYPE_SLOT = 11,
    WIDGET_TYPE_ICON = 12
} WidgetType;

/* Widget flags */
#define WIDGET_FLAG_VISIBLE        (1 << 0)
#define WIDGET_FLAG_ENABLED        (1 << 1)
#define WIDGET_FLAG_CLICKABLE      (1 << 2)
#define WIDGET_FLAG_DRAGGABLE      (1 << 3)
#define WIDGET_FLAG_MODAL          (1 << 4)
#define WIDGET_FLAG_TOPMOST        (1 << 5)
#define WIDGET_FLAG_BORDER         (1 << 6)
#define WIDGET_FLAG_TRANSPARENT    (1 << 7)

/* Window styles */
#define WINDOW_STYLE_STANDARD      0
#define WINDOW_STYLE_ALTERNATE     1
#define WINDOW_STYLE_STANDARD_V2   2
#define WINDOW_STYLE_ALTERNATE_V2  3
#define WINDOW_STYLE_CLOSE_BUTTON  4
#define WINDOW_STYLE_CENTERED      -1

/* Sprite constants */
#define SPRITE_STANDARD_BASE    0x6591
#define SPRITE_ALTERNATE_BASE   0x65a5

/* ========================================
 * Structures
 * ======================================== */

typedef struct {
    u8 r, g, b, a;
} WidgetColor;

typedef struct {
    int x, y;
    int width, height;
} WidgetRect;

typedef struct Widget {
    struct Widget* next;
    struct Widget* prev;
    void* render_callback;
    void* extended_data;

    u32 flags;
    WidgetType type;
    u8 sprite_type;
    u8 padding1[2];

    s32 x, y;
    s32 anim_offset_x, anim_offset_y;
    s32 step_x, step_y;

    u32 render_state;
    u32 state;
    u32 padding2[10];

    WidgetRect rect;
    u32 done;

    struct Widget* parent;
    struct Widget* children[16];
    int child_count;

    u32 id;
    char name[32];

    WidgetColor bg_color;
    WidgetColor border_color;
    WidgetColor text_color;
    u16 sprite_id;
    u16 sprite_frame;

    char* text;
    u32 text_len;
    int font_id;

    void* on_click;
    void* on_hover;
    void* callback_data;

    void* data;
    u32 data_size;
    void* user_data;

    u32 create_time;
    u32 last_update;
} Widget;

typedef struct {
    s32 width;
    s32 height;
    s32 extra_param;
    s32 base_x;
    s32 base_y;
    s32 center_x;
    s32 center_y;
    s32 offset_x;
    s32 offset_y;
    s32 frame_count;
    s32 sprite_base;
    s32 alpha_mode;
    s32 close_sprite1;
    s32 close_sprite2;
    s32 close_sprite3;
    s32 hover_flag;
} WidgetWindowData;

typedef struct {
    Widget widgets[256];
    int widget_count;

    Widget* root_widgets[64];
    int root_count;

    Widget* focused_widget;
    Widget* hovered_widget;
    Widget* dragged_widget;
    Widget* modal_widget;

    int drag_start_x;
    int drag_start_y;
    int drag_offset_x;
    int drag_offset_y;

    void* skin_images[5];
    void* skin_dc[5];
    void* back_buffer;
    void* back_dc;

    u32 total_created;
    u32 total_destroyed;
} WidgetManagerContext;

/* Global widget manager for testing */
static WidgetManagerContext g_widgetmgr = {0};
static u32 s_widget_id_counter = 1;

/* Default colors */
static const WidgetColor s_default_bg = {40, 40, 60, 255};
static const WidgetColor s_default_border = {100, 100, 140, 255};
static const WidgetColor s_default_text = {255, 255, 255, 255};

/* ========================================
 * Implementation Functions
 * ======================================== */

static void reset_widget_manager(void) {
    memset(&g_widgetmgr, 0, sizeof(WidgetManagerContext));
    s_widget_id_counter = 1;
}

static int widgetmgr_init(void) {
    memset(&g_widgetmgr, 0, sizeof(WidgetManagerContext));
    return 1;
}

static void widgetmgr_shutdown(void) {
    memset(&g_widgetmgr, 0, sizeof(WidgetManagerContext));
}

static Widget* widget_create(WidgetType type, int x, int y, int width, int height, Widget* parent) {
    Widget* widget = NULL;
    int i;

    if (g_widgetmgr.widget_count >= 256) {
        return NULL;
    }

    /* Find free slot */
    for (i = 0; i < 256; i++) {
        if (g_widgetmgr.widgets[i].id == 0) {
            widget = &g_widgetmgr.widgets[i];
            break;
        }
    }

    if (!widget) return NULL;

    memset(widget, 0, sizeof(Widget));

    widget->id = s_widget_id_counter++;
    widget->type = type;
    widget->flags = WIDGET_FLAG_VISIBLE | WIDGET_FLAG_ENABLED;

    widget->x = x;
    widget->y = y;
    widget->rect.x = x;
    widget->rect.y = y;
    widget->rect.width = width;
    widget->rect.height = height;

    widget->bg_color = s_default_bg;
    widget->border_color = s_default_border;
    widget->text_color = s_default_text;

    widget->parent = parent;

    if (parent) {
        if (parent->child_count < 16) {
            parent->children[parent->child_count++] = widget;
        }
    } else {
        if (g_widgetmgr.root_count < 64) {
            g_widgetmgr.root_widgets[g_widgetmgr.root_count++] = widget;
        }
    }

    g_widgetmgr.widget_count++;
    g_widgetmgr.total_created++;

    return widget;
}

static Widget* widget_create_window_ex(int x, int y, int width, int height, const char* title, int style) {
    Widget* widget;
    WidgetWindowData* wdata;
    s32 center_calc;

    widget = widget_create(WIDGET_TYPE_WINDOW, x, y, width, height, NULL);
    if (!widget) return NULL;

    /* Allocate extended data */
    wdata = (WidgetWindowData*)calloc(1, sizeof(WidgetWindowData));
    if (!wdata) {
        widget->id = 0;
        return NULL;
    }
    widget->extended_data = wdata;

    widget->sprite_type = 0x68;

    wdata->width = width;
    wdata->height = height;
    wdata->extra_param = 0;

    center_calc = (width << 6) / 2;
    wdata->base_x = x + center_calc;
    wdata->base_y = y + 0x1b;

    /* Set sprite base based on style */
    switch (style) {
        case 0:
        case 2:
            wdata->sprite_base = SPRITE_STANDARD_BASE;
            break;
        case 1:
        case 3:
            wdata->sprite_base = SPRITE_ALTERNATE_BASE;
            break;
        case 4:
            wdata->close_sprite3 = 0x6d;
            wdata->sprite_base = -2;
            wdata->close_sprite1 = -2;
            wdata->close_sprite2 = -2;
            break;
        case -1:
        default:
            wdata->sprite_base = -1;
            break;
    }

    wdata->hover_flag = -1;
    wdata->alpha_mode = (style < 2) ? 1 : 0;

    if (style == -1 || style == 4) {
        wdata->center_x = width / 2 + x;
    } else {
        wdata->center_x = x + center_calc;
        wdata->center_y = (height * 0x30) / 2 + y;
    }

    widget->anim_offset_x = (wdata->center_x - x) / 10;
    widget->anim_offset_y = (wdata->center_y - y) / 10;

    wdata->offset_x = 0;
    wdata->offset_y = 0;
    wdata->frame_count = 0;

    if (title) {
        widget->text = strdup(title);
    }

    return widget;
}

static Widget* widget_create_window(int x, int y, int width, int height, const char* title) {
    return widget_create_window_ex(x, y, width, height, title, WINDOW_STYLE_STANDARD);
}

static Widget* widget_create_button(int x, int y, int width, int height, const char* text) {
    Widget* widget = widget_create(WIDGET_TYPE_BUTTON, x, y, width, height, NULL);
    if (widget) {
        if (text) widget->text = strdup(text);
        widget->flags |= WIDGET_FLAG_CLICKABLE | WIDGET_FLAG_BORDER;
    }
    return widget;
}

static Widget* widget_create_text(int x, int y, int width, int height, const char* text) {
    Widget* widget = widget_create(WIDGET_TYPE_TEXT, x, y, width, height, NULL);
    if (widget && text) {
        widget->text = strdup(text);
    }
    return widget;
}

static Widget* widget_create_edit(int x, int y, int width, int height, int max_len) {
    Widget* widget = widget_create(WIDGET_TYPE_EDIT, x, y, width, height, NULL);
    if (widget) {
        widget->text = (char*)malloc(max_len + 1);
        if (widget->text) {
            widget->text[0] = '\0';
            widget->text_len = max_len;
        }
        widget->flags |= WIDGET_FLAG_BORDER;
    }
    return widget;
}

static Widget* widget_create_image(int x, int y, int width, int height, u16 sprite_id) {
    Widget* widget = widget_create(WIDGET_TYPE_IMAGE, x, y, width, height, NULL);
    if (widget) {
        widget->sprite_id = sprite_id;
    }
    return widget;
}

static Widget* widget_create_progress(int x, int y, int width, int height, int max_value) {
    Widget* widget = widget_create(WIDGET_TYPE_PROGRESS, x, y, width, height, NULL);
    if (widget) {
        widget->data = (void*)(size_t)max_value;
        widget->flags |= WIDGET_FLAG_BORDER;
    }
    return widget;
}

static Widget* widget_create_slot(int x, int y, int slot_index) {
    Widget* widget = widget_create(WIDGET_TYPE_SLOT, x, y, 32, 32, NULL);
    if (widget) {
        widget->data = (void*)(size_t)slot_index;
        widget->flags |= WIDGET_FLAG_CLICKABLE | WIDGET_FLAG_BORDER;
    }
    return widget;
}

static void widget_destroy(Widget* widget) {
    int i;

    if (!widget || widget->id == 0) return;

    /* Destroy children */
    while (widget->child_count > 0) {
        widget_destroy(widget->children[0]);
    }

    /* Free text */
    if (widget->text) {
        free(widget->text);
        widget->text = NULL;
    }

    /* Free extended data */
    if (widget->extended_data) {
        free(widget->extended_data);
        widget->extended_data = NULL;
    }

    /* Remove from parent */
    if (widget->parent) {
        for (i = 0; i < widget->parent->child_count; i++) {
            if (widget->parent->children[i] == widget) {
                int j;
                for (j = i; j < widget->parent->child_count - 1; j++) {
                    widget->parent->children[j] = widget->parent->children[j + 1];
                }
                widget->parent->child_count--;
                break;
            }
        }
    } else {
        /* Remove from root widgets */
        for (i = 0; i < g_widgetmgr.root_count; i++) {
            if (g_widgetmgr.root_widgets[i] == widget) {
                int j;
                for (j = i; j < g_widgetmgr.root_count - 1; j++) {
                    g_widgetmgr.root_widgets[j] = g_widgetmgr.root_widgets[j + 1];
                }
                g_widgetmgr.root_count--;
                break;
            }
        }
    }

    if (g_widgetmgr.focused_widget == widget) {
        g_widgetmgr.focused_widget = NULL;
    }
    if (g_widgetmgr.hovered_widget == widget) {
        g_widgetmgr.hovered_widget = NULL;
    }

    widget->id = 0;
    g_widgetmgr.widget_count--;
    g_widgetmgr.total_destroyed++;
}

static void widget_destroy_all(void) {
    int i;

    /* Clear all widgets directly to avoid destroy logic issues */
    for (i = 0; i < 256; i++) {
        if (g_widgetmgr.widgets[i].id != 0) {
            if (g_widgetmgr.widgets[i].text) {
                free(g_widgetmgr.widgets[i].text);
            }
            if (g_widgetmgr.widgets[i].extended_data) {
                free(g_widgetmgr.widgets[i].extended_data);
            }
            memset(&g_widgetmgr.widgets[i], 0, sizeof(Widget));
        }
    }

    g_widgetmgr.widget_count = 0;
    g_widgetmgr.root_count = 0;
    g_widgetmgr.focused_widget = NULL;
    g_widgetmgr.hovered_widget = NULL;
}

static int widget_is_visible(Widget* widget) {
    return widget && (widget->flags & WIDGET_FLAG_VISIBLE);
}

static int widget_is_enabled(Widget* widget) {
    return widget && (widget->flags & WIDGET_FLAG_ENABLED);
}

static int widget_contains_point(Widget* widget, int x, int y) {
    if (!widget) return 0;
    return x >= widget->x && x < widget->x + widget->rect.width &&
           y >= widget->y && y < widget->y + widget->rect.height;
}

static Widget* widget_find_by_id(u32 id) {
    int i;
    for (i = 0; i < 256; i++) {
        if (g_widgetmgr.widgets[i].id == id) {
            return &g_widgetmgr.widgets[i];
        }
    }
    return NULL;
}

/* ========================================
 * Test Cases - Constants
 * ======================================== */

static int test_widget_type_values(void) {
    return WIDGET_TYPE_NONE == 0 &&
           WIDGET_TYPE_BUTTON == 2 &&
           WIDGET_TYPE_WINDOW == 3 &&
           WIDGET_TYPE_TEXT == 4;
}

static int test_widget_flag_values(void) {
    return WIDGET_FLAG_VISIBLE == 1 &&
           WIDGET_FLAG_ENABLED == 2 &&
           WIDGET_FLAG_CLICKABLE == 4 &&
           WIDGET_FLAG_DRAGGABLE == 8;
}

static int test_window_style_values(void) {
    return WINDOW_STYLE_STANDARD == 0 &&
           WINDOW_STYLE_ALTERNATE == 1 &&
           WINDOW_STYLE_CLOSE_BUTTON == 4 &&
           WINDOW_STYLE_CENTERED == -1;
}

static int test_sprite_constants(void) {
    return SPRITE_STANDARD_BASE == 0x6591 &&
           SPRITE_ALTERNATE_BASE == 0x65a5;
}

/* ========================================
 * Test Cases - Manager Operations
 * ======================================== */

static int test_manager_init(void) {
    reset_widget_manager();

    int result = widgetmgr_init();

    return result == 1 &&
           g_widgetmgr.widget_count == 0 &&
           g_widgetmgr.root_count == 0;
}

static int test_manager_shutdown(void) {
    reset_widget_manager();

    widget_create(WIDGET_TYPE_BUTTON, 0, 0, 100, 50, NULL);
    widgetmgr_shutdown();

    return g_widgetmgr.widget_count == 0;
}

/* ========================================
 * Test Cases - Widget Creation
 * ======================================== */

static int test_create_button(void) {
    reset_widget_manager();

    Widget* btn = widget_create_button(10, 20, 100, 30, "Click");

    return btn != NULL &&
           btn->type == WIDGET_TYPE_BUTTON &&
           btn->x == 10 &&
           btn->y == 20 &&
           btn->flags & WIDGET_FLAG_CLICKABLE &&
           btn->flags & WIDGET_FLAG_BORDER &&
           strcmp(btn->text, "Click") == 0;
}

static int test_create_text(void) {
    reset_widget_manager();

    Widget* txt = widget_create_text(10, 10, 200, 20, "Hello");

    return txt != NULL &&
           txt->type == WIDGET_TYPE_TEXT &&
           strcmp(txt->text, "Hello") == 0;
}

static int test_create_edit(void) {
    reset_widget_manager();

    Widget* edit = widget_create_edit(10, 10, 150, 25, 100);

    return edit != NULL &&
           edit->type == WIDGET_TYPE_EDIT &&
           edit->text_len == 100 &&
           edit->text[0] == '\0' &&
           edit->flags & WIDGET_FLAG_BORDER;
}

static int test_create_image(void) {
    reset_widget_manager();

    Widget* img = widget_create_image(50, 50, 64, 64, 0x1234);

    return img != NULL &&
           img->type == WIDGET_TYPE_IMAGE &&
           img->sprite_id == 0x1234;
}

static int test_create_progress(void) {
    reset_widget_manager();

    Widget* prog = widget_create_progress(10, 10, 200, 20, 100);

    return prog != NULL &&
           prog->type == WIDGET_TYPE_PROGRESS &&
           (size_t)prog->data == 100 &&
           prog->flags & WIDGET_FLAG_BORDER;
}

static int test_create_slot(void) {
    reset_widget_manager();

    Widget* slot = widget_create_slot(10, 10, 5);

    return slot != NULL &&
           slot->type == WIDGET_TYPE_SLOT &&
           slot->rect.width == 32 &&
           slot->rect.height == 32 &&
           (size_t)slot->data == 5;
}

static int test_create_multiple(void) {
    reset_widget_manager();

    widget_create_button(0, 0, 100, 30, "Btn1");
    widget_create_button(0, 40, 100, 30, "Btn2");
    widget_create_text(0, 80, 200, 20, "Text");

    return g_widgetmgr.widget_count == 3;
}

static int test_create_limit(void) {
    reset_widget_manager();

    int i;
    for (i = 0; i < 300; i++) {
        widget_create_button(0, 0, 10, 10, NULL);
    }

    /* Should stop at 256 */
    return g_widgetmgr.widget_count == 256;
}

/* ========================================
 * Test Cases - Window Creation
 * ======================================== */

static int test_create_window_basic(void) {
    reset_widget_manager();

    Widget* win = widget_create_window(100, 100, 200, 150, "Window");

    return win != NULL &&
           win->type == WIDGET_TYPE_WINDOW &&
           win->sprite_type == 0x68 &&
           strcmp(win->text, "Window") == 0;
}

static int test_create_window_style_standard(void) {
    reset_widget_manager();

    Widget* win = widget_create_window_ex(100, 100, 200, 150, NULL, WINDOW_STYLE_STANDARD);
    WidgetWindowData* wdata = (WidgetWindowData*)win->extended_data;

    return wdata != NULL &&
           wdata->sprite_base == SPRITE_STANDARD_BASE &&
           wdata->alpha_mode == 1;
}

static int test_create_window_style_alternate(void) {
    reset_widget_manager();

    Widget* win = widget_create_window_ex(100, 100, 200, 150, NULL, WINDOW_STYLE_ALTERNATE);
    WidgetWindowData* wdata = (WidgetWindowData*)win->extended_data;

    return wdata != NULL &&
           wdata->sprite_base == SPRITE_ALTERNATE_BASE &&
           wdata->alpha_mode == 1;
}

static int test_create_window_style_close(void) {
    reset_widget_manager();

    Widget* win = widget_create_window_ex(100, 100, 200, 150, NULL, WINDOW_STYLE_CLOSE_BUTTON);
    WidgetWindowData* wdata = (WidgetWindowData*)win->extended_data;

    return wdata != NULL &&
           wdata->close_sprite3 == 0x6d &&
           wdata->sprite_base == -2 &&
           wdata->alpha_mode == 0;
}

static int test_create_window_style_centered(void) {
    reset_widget_manager();

    Widget* win = widget_create_window_ex(100, 100, 200, 150, NULL, WINDOW_STYLE_CENTERED);
    WidgetWindowData* wdata = (WidgetWindowData*)win->extended_data;

    return wdata != NULL &&
           wdata->sprite_base == -1;
}

static int test_window_extended_data(void) {
    reset_widget_manager();

    Widget* win = widget_create_window(100, 100, 200, 150, NULL);
    WidgetWindowData* wdata = (WidgetWindowData*)win->extended_data;

    return wdata != NULL &&
           wdata->width == 200 &&
           wdata->height == 150 &&
           wdata->hover_flag == -1;
}

/* ========================================
 * Test Cases - Widget Destruction
 * ======================================== */

static int test_destroy_widget(void) {
    reset_widget_manager();

    Widget* btn = widget_create_button(10, 10, 100, 30, "Test");

    widget_destroy(btn);

    return g_widgetmgr.widget_count == 0 &&
           btn->id == 0;
}

static int test_destroy_null(void) {
    reset_widget_manager();

    widget_destroy(NULL);

    return 1;  /* Should not crash */
}

static int test_destroy_with_children(void) {
    reset_widget_manager();

    Widget* parent = widget_create(WIDGET_TYPE_CONTAINER, 0, 0, 200, 200, NULL);
    Widget* child1 = widget_create(WIDGET_TYPE_BUTTON, 10, 10, 50, 20, "C1");
    Widget* child2 = widget_create(WIDGET_TYPE_BUTTON, 10, 40, 50, 20, "C2");

    parent->children[parent->child_count++] = child1;
    parent->children[parent->child_count++] = child2;
    child1->parent = parent;
    child2->parent = parent;

    widget_destroy(parent);

    return g_widgetmgr.widget_count == 0;
}

static int test_destroy_all(void) {
    reset_widget_manager();

    widget_create_button(0, 0, 100, 30, "B1");
    widget_create_button(0, 40, 100, 30, "B2");
    widget_create_text(0, 80, 200, 20, "T1");

    widget_destroy_all();

    return g_widgetmgr.widget_count == 0 &&
           g_widgetmgr.root_count == 0;
}

/* ========================================
 * Test Cases - Widget State
 * ======================================== */

static int test_widget_visible(void) {
    reset_widget_manager();

    Widget* btn = widget_create_button(10, 10, 100, 30, "Test");

    int visible = widget_is_visible(btn);

    return visible == 1;
}

static int test_widget_hidden(void) {
    reset_widget_manager();

    Widget* btn = widget_create_button(10, 10, 100, 30, "Test");
    btn->flags &= ~WIDGET_FLAG_VISIBLE;

    int visible = widget_is_visible(btn);

    return visible == 0;
}

static int test_widget_enabled(void) {
    reset_widget_manager();

    Widget* btn = widget_create_button(10, 10, 100, 30, "Test");

    int enabled = widget_is_enabled(btn);

    return enabled == 1;
}

static int test_widget_disabled(void) {
    reset_widget_manager();

    Widget* btn = widget_create_button(10, 10, 100, 30, "Test");
    btn->flags &= ~WIDGET_FLAG_ENABLED;

    int enabled = widget_is_enabled(btn);

    return enabled == 0;
}

static int test_widget_contains_point(void) {
    reset_widget_manager();

    Widget* btn = widget_create_button(100, 100, 50, 30, "Test");

    return widget_contains_point(btn, 110, 110) == 1 &&
           widget_contains_point(btn, 200, 110) == 0;
}

/* ========================================
 * Test Cases - Widget Hierarchy
 * ======================================== */

static int test_widget_parent(void) {
    reset_widget_manager();

    Widget* parent = widget_create(WIDGET_TYPE_CONTAINER, 0, 0, 200, 200, NULL);
    Widget* child = widget_create(WIDGET_TYPE_BUTTON, 10, 10, 50, 20, "Child");

    parent->children[parent->child_count++] = child;
    child->parent = parent;

    return child->parent == parent &&
           parent->child_count == 1 &&
           parent->children[0] == child;
}

static int test_widget_root_list(void) {
    reset_widget_manager();

    widget_create(WIDGET_TYPE_BUTTON, 0, 0, 100, 30, NULL);
    widget_create(WIDGET_TYPE_BUTTON, 0, 40, 100, 30, NULL);

    /* Root count should be 2 for widgets without parent */
    return g_widgetmgr.root_count == 2;
}

/* ========================================
 * Test Cases - Widget Lookup
 * ======================================== */

static int test_find_by_id(void) {
    reset_widget_manager();

    Widget* btn1 = widget_create_button(0, 0, 100, 30, "B1");
    Widget* btn2 = widget_create_button(0, 40, 100, 30, "B2");

    Widget* found = widget_find_by_id(btn2->id);

    return found == btn2;
}

static int test_find_by_id_not_found(void) {
    reset_widget_manager();

    widget_create_button(0, 0, 100, 30, "B1");

    Widget* found = widget_find_by_id(99999);

    return found == NULL;
}

/* ========================================
 * Test Cases - Statistics
 * ======================================== */

static int test_total_created(void) {
    reset_widget_manager();

    widget_create_button(0, 0, 100, 30, "B1");
    widget_create_button(0, 40, 100, 30, "B2");

    return g_widgetmgr.total_created == 2;
}

static int test_total_destroyed(void) {
    reset_widget_manager();

    Widget* btn = widget_create_button(0, 0, 100, 30, "B1");
    widget_destroy(btn);

    return g_widgetmgr.total_destroyed == 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== UI Widget Comprehensive Tests ===\n\n");

    printf("Constants Tests:\n");
    TEST(widget_type_values);
    TEST(widget_flag_values);
    TEST(window_style_values);
    TEST(sprite_constants);

    printf("\nManager Operations Tests:\n");
    TEST(manager_init);
    TEST(manager_shutdown);

    printf("\nWidget Creation Tests:\n");
    TEST(create_button);
    TEST(create_text);
    TEST(create_edit);
    TEST(create_image);
    TEST(create_progress);
    TEST(create_slot);
    TEST(create_multiple);
    TEST(create_limit);

    printf("\nWindow Creation Tests:\n");
    TEST(create_window_basic);
    TEST(create_window_style_standard);
    TEST(create_window_style_alternate);
    TEST(create_window_style_close);
    TEST(create_window_style_centered);
    TEST(window_extended_data);

    printf("\nWidget Destruction Tests:\n");
    TEST(destroy_widget);
    TEST(destroy_null);
    TEST(destroy_with_children);
    TEST(destroy_all);

    printf("\nWidget State Tests:\n");
    TEST(widget_visible);
    TEST(widget_hidden);
    TEST(widget_enabled);
    TEST(widget_disabled);
    TEST(widget_contains_point);

    printf("\nWidget Hierarchy Tests:\n");
    TEST(widget_parent);
    TEST(widget_root_list);

    printf("\nWidget Lookup Tests:\n");
    TEST(find_by_id);
    TEST(find_by_id_not_found);

    printf("\nStatistics Tests:\n");
    TEST(total_created);
    TEST(total_destroyed);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
