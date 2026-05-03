/*
 * Stone Age Client - UI Widget System Header
 * Reverse engineered from sa_9061.exe:
 *   FUN_00448610 - Window creation with style
 *   FUN_00448270 - Window render callback
 *   FUN_004010a0 - Object allocator
 *   FUN_0047e210 - Render queue add sprite
 *   FUN_0047e640 - Render with fade effect
 */

#ifndef UIWIDGET_H
#define UIWIDGET_H

#include "types.h"

/* Widget types - matches FUN_004010a0 type parameter */
typedef enum {
    WIDGET_TYPE_NONE = 0,
    WIDGET_TYPE_CONTAINER = 1,
    WIDGET_TYPE_BUTTON = 2,
    WIDGET_TYPE_WINDOW = 3,     /* Type 3 in FUN_004010a0 */
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

/* Widget render states - from FUN_00448270 switch */
typedef enum {
    WIDGET_RENDER_INIT = 0,      /* Case 0: Opening animation */
    WIDGET_RENDER_COMPLETE = 1,  /* Case 1: Set done flag */
    WIDGET_RENDER_ACTIVE = 2,    /* Case 2: Normal rendering */
    WIDGET_RENDER_BUTTONS = 3    /* Case 3: Button hover state */
} WidgetRenderState;

/* Widget interaction states */
typedef enum {
    WIDGET_STATE_NORMAL = 0,
    WIDGET_STATE_HOVER = 1,
    WIDGET_STATE_PRESSED = 2,
    WIDGET_STATE_DISABLED = 3,
    WIDGET_STATE_FOCUSED = 4
} WidgetState;

/* Widget flags */
#define WIDGET_FLAG_VISIBLE        (1 << 0)
#define WIDGET_FLAG_ENABLED        (1 << 1)
#define WIDGET_FLAG_CLICKABLE      (1 << 2)
#define WIDGET_FLAG_DRAGGABLE      (1 << 3)
#define WIDGET_FLAG_MODAL          (1 << 4)
#define WIDGET_FLAG_TOPMOST        (1 << 5)
#define WIDGET_FLAG_BORDER         (1 << 6)
#define WIDGET_FLAG_TRANSPARENT    (1 << 7)

/* Window styles - from FUN_00448610 param_6 */
#define WINDOW_STYLE_STANDARD      0    /* DAT_0054b194 sprites */
#define WINDOW_STYLE_ALTERNATE     1    /* DAT_0054c208 sprites */
#define WINDOW_STYLE_STANDARD_V2   2    /* DAT_0054b194 sprites */
#define WINDOW_STYLE_ALTERNATE_V2  3    /* DAT_0054c208 sprites */
#define WINDOW_STYLE_CLOSE_BUTTON  4    /* Sprite 0x6d, special */
#define WINDOW_STYLE_CENTERED      -1   /* Center on parent */

/* Sprite IDs for window decoration - from DAT_0054b194/DAT_0054c208 */
#define WINDOW_SPRITE_STANDARD     0x6591   /* DAT_0054b194 */
#define WINDOW_SPRITE_ALTERNATE    0x65a5   /* DAT_0054c208 */
#define WINDOW_SPRITE_CLOSE        0x006d   /* Close button sprite */
#define WINDOW_SPRITE_CLOSE_HOVER  0x006e   /* Close button hover */

/* Widget alignment */
typedef enum {
    ALIGN_TOP_LEFT = 0,
    ALIGN_TOP_CENTER,
    ALIGN_TOP_RIGHT,
    ALIGN_CENTER_LEFT,
    ALIGN_CENTER,
    ALIGN_CENTER_RIGHT,
    ALIGN_BOTTOM_LEFT,
    ALIGN_BOTTOM_CENTER,
    ALIGN_BOTTOM_RIGHT
} WidgetAlignment;

/* Color structure */
typedef struct {
    u8 r, g, b, a;
} WidgetColor;

/* Rectangle */
typedef struct {
    int x, y;
    int width, height;
} WidgetRect;

/* Widget callback types */
typedef void (*WidgetCallback)(struct Widget* widget, void* userdata);
typedef int (*WidgetRenderFunc)(struct Widget* widget, void* surface);
typedef int (*WidgetInputFunc)(struct Widget* widget, u32 msg, u32 wparam, u32 lparam);

/*
 * Window extended data - 0x40 bytes at offset +0x0c
 * From FUN_00448610 and FUN_00448270 analysis
 * piVar1 = *(int **)(widget + 0x0c)
 */
typedef struct {
    s32 width;              /* [0] piVar1[0] = param_3 */
    s32 height;             /* [1] piVar1[1] = param_4 */
    s32 extra_param;        /* [2] piVar1[2] = param_5 */
    s32 base_x;             /* [3] piVar1[3] = x + (width << 6) / 2 */
    s32 base_y;             /* [4] piVar1[4] = y + 0x1b */
    s32 center_x;           /* [5] piVar1[5] = center X position */
    s32 center_y;           /* [6] piVar1[6] = center Y position */
    s32 offset_x;           /* [7] piVar1[7] = animated X offset */
    s32 offset_y;           /* [8] piVar1[8] = animated Y offset */
    s32 frame_count;        /* [9] piVar1[9] = frame counter */
    s32 sprite_base;        /* [10] piVar1[10] = sprite ID base */
    s32 alpha_mode;         /* [11] piVar1[11] = alpha/blend mode */
    s32 close_sprite1;      /* [12] piVar1[0xc] = close button sprite 1 */
    s32 close_sprite2;      /* [13] piVar1[0xd] = close button sprite 2 */
    s32 close_sprite3;      /* [14] piVar1[0xe] = close button sprite 3 */
    s32 hover_flag;         /* [15] piVar1[0xf] = hover/active flag */
} WidgetWindowData;

/*
 * Widget structure - matches FUN_004010a0 allocation
 * Base size: 500 bytes (from FUN_00491f70)
 * Extra data: param_2 bytes at offset +0x0c
 */
typedef struct Widget {
    /* Linked list - inserted at DAT_004d7e3c */
    struct Widget* next;        /* +0x00 */
    struct Widget* prev;        /* +0x04 */

    /* Callback */
    WidgetRenderFunc render_callback;  /* +0x08 - FUN_00448270 for windows */

    /* Extended data pointer - allocated separately */
    void* extended_data;        /* +0x0c - WidgetWindowData for windows */

    /* Type and state */
    u32 flags;                  /* +0x10 */
    WidgetType type;            /* +0x14 as byte */
    u8 sprite_type;             /* +0x15 - 0x68 for window decoration */
    u8 padding1[2];

    /* Position - from FUN_00448610 */
    s32 x;                      /* +0x18 - param_1 */
    s32 y;                      /* +0x1c - param_2 */

    /* Animation offsets - from FUN_00448610 */
    s32 anim_offset_x;          /* +0x20 */
    s32 anim_offset_y;          /* +0x24 */

    /* Calculated from binary */
    s32 step_x;                 /* +0x28 - (center_x - x) / 10 */
    s32 step_y;                 /* +0x2c - (center_y - y) / 10 */

    /* Render state - from FUN_00448270 switch */
    u32 render_state;           /* +0xa0 - low bits |= 6 in FUN_00448610 */
    u32 state;                  /* Alias for render_state */
    u32 padding2[10];

    /* Bounding rectangle */
    WidgetRect rect;

    /* Done flag - set in case 1 */
    u32 done;                   /* +0x78 */

    /* Parent/children */
    struct Widget* parent;
    struct Widget* children[16];
    int child_count;

    /* Identity */
    u32 id;
    char name[32];

    /* Appearance */
    WidgetColor bg_color;
    WidgetColor border_color;
    WidgetColor text_color;
    u16 sprite_id;
    u16 sprite_frame;

    /* Text */
    char* text;
    u32 text_len;
    int font_id;

    /* Callbacks */
    WidgetCallback on_click;
    WidgetCallback on_hover;
    WidgetCallback on_focus;
    WidgetCallback on_blur;
    WidgetInputFunc on_input;
    void* callback_data;

    /* Custom data */
    void* data;
    u32 data_size;
    void* user_data;

    /* Timing */
    u32 create_time;
    u32 last_update;

} Widget;

/* Widget manager context */
typedef struct {
    /* Widget storage - linked list heads at DAT_004d7e3c/DAT_004d7e38 */
    Widget widgets[256];
    int widget_count;

    /* Root widgets (no parent) */
    Widget* root_widgets[64];
    int root_count;

    /* Focus */
    Widget* focused_widget;
    Widget* hovered_widget;
    Widget* dragged_widget;
    Widget* modal_widget;

    /* Drag state */
    int drag_start_x;
    int drag_start_y;
    int drag_offset_x;
    int drag_offset_y;

    /* Skin data */
    void* skin_images[5];
    void* skin_dc[5];
    void* back_buffer;
    void* back_dc;

    /* Statistics */
    u32 total_created;
    u32 total_destroyed;

} WidgetManagerContext;

/* Global widget manager */
extern WidgetManagerContext g_widgetmgr;

/* Initialization */
int widgetmgr_init(void);
void widgetmgr_shutdown(void);

/* Widget creation - FUN_004010a0, FUN_00448610 */
Widget* widget_create(WidgetType type, int x, int y, int width, int height, Widget* parent);
Widget* widget_create_window(int x, int y, int width, int height, const char* title);
Widget* widget_create_window_ex(int x, int y, int width, int height, const char* title, int style);
Widget* widget_create_button(int x, int y, int width, int height, const char* text);
Widget* widget_create_text(int x, int y, int width, int height, const char* text);
Widget* widget_create_edit(int x, int y, int width, int height, int max_len);
Widget* widget_create_image(int x, int y, int width, int height, u16 sprite_id);
Widget* widget_create_listbox(int x, int y, int width, int height);
Widget* widget_create_progress(int x, int y, int width, int height, int max_value);
Widget* widget_create_slot(int x, int y, int slot_index);

/* Widget destruction */
void widget_destroy(Widget* widget);
void widget_destroy_children(Widget* widget);
void widget_destroy_all(void);

/* Widget properties */
void widget_set_text(Widget* widget, const char* text);
const char* widget_get_text(Widget* widget);
void widget_set_sprite(Widget* widget, u16 sprite_id, u16 frame);
void widget_set_color(Widget* widget, WidgetColor* bg, WidgetColor* border);
void widget_set_position(Widget* widget, int x, int y);
void widget_set_size(Widget* widget, int width, int height);
void widget_set_rect(Widget* widget, int x, int y, int width, int height);
void widget_set_visible(Widget* widget, int visible);
void widget_set_enabled(Widget* widget, int enabled);
void widget_set_flags(Widget* widget, u32 flags);

/* Widget state */
int widget_is_visible(Widget* widget);
int widget_is_enabled(Widget* widget);
int widget_is_hovered(Widget* widget);
int widget_is_focused(Widget* widget);
int widget_contains_point(Widget* widget, int x, int y);

/* Widget hierarchy */
void widget_add_child(Widget* parent, Widget* child);
void widget_remove_child(Widget* parent, Widget* child);
Widget* widget_get_parent(Widget* widget);
Widget* widget_get_child(Widget* widget, int index);
int widget_get_child_count(Widget* widget);

/* Widget callbacks */
void widget_set_on_click(Widget* widget, WidgetCallback callback, void* data);
void widget_set_on_hover(Widget* widget, WidgetCallback callback, void* data);
void widget_set_on_render(Widget* widget, WidgetRenderFunc func);

/* Window render callback - FUN_00448270 */
void widget_window_render_callback(Widget* widget);

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

/* Widget input */
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

/* Skin loading */
int widget_load_skin(const char* path);
void widget_unload_skin(void);

#endif /* UIWIDGET_H */
