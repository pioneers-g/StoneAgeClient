/*
 * Stone Age Client - UI System
 * Reverse engineered from sa_9061.exe
 * FUN_004155e0 - UI sprite ID initialization
 * FUN_0041d7c0 - Text render queue
 * FUN_0041d890 - Multi-line text rendering
 * FUN_00448610 - Button/control creation
 */

#ifndef UI_H
#define UI_H

#include "types.h"

/* ========================================
 * UI Sprite IDs - from FUN_004155e0
 * Standard mode sprite IDs (0x6xxx series)
 * Extended mode sprite IDs (0x7axxx series)
 * ======================================== */

/* Standard UI Sprites */
#define UI_SPRITE_BUTTON_NORMAL        0x6650
#define UI_SPRITE_BUTTON_PRESSED       0x6715
#define UI_SPRITE_BUTTON_HOVER         0x66f0
#define UI_SPRITE_BUTTON_DISABLED      0x673b
#define UI_SPRITE_SCROLL_UP            0x8995
#define UI_SPRITE_SCROLL_DOWN          0x8997
#define UI_SPRITE_SCROLL_TRACK         0x6712
#define UI_SPRITE_SCROLL_THUMB         0x6757
#define UI_SPRITE_SCROLL_LEFT          0x66d8
#define UI_SPRITE_SCROLL_RIGHT         0x66d9

/* Dialog/Input Box Sprites */
#define UI_SPRITE_DIALOG_BG            0x6591
#define UI_SPRITE_DIALOG_CORNER_TL     0x6592
#define UI_SPRITE_DIALOG_CORNER_TR     0x6593
#define UI_SPRITE_DIALOG_CORNER_BL     0x6594
#define UI_SPRITE_DIALOG_CORNER_BR     0x6595
#define UI_SPRITE_DIALOG_EDGE_TOP      0x6596
#define UI_SPRITE_DIALOG_EDGE_BOTTOM   0x6597
#define UI_SPRITE_DIALOG_EDGE_LEFT     0x6598
#define UI_SPRITE_DIALOG_EDGE_RIGHT    0x6599

/* Text Input Sprites */
#define UI_SPRITE_INPUT_BG             0x65a5
#define UI_SPRITE_INPUT_CURSOR         0x65a6
#define UI_SPRITE_INPUT_FOCUS          0x65a7
#define UI_SPRITE_INPUT_NORMAL         0x65a8
#define UI_SPRITE_INPUT_ACTIVE         0x65a9
#define UI_SPRITE_INPUT_DISABLED       0x65aa
#define UI_SPRITE_INPUT_READONLY       0x65ab

/* Checkbox Sprites */
#define UI_SPRITE_CHECKBOX_UNCHECKED   0x65ac
#define UI_SPRITE_CHECKBOX_CHECKED     0x65ad

/* Progress Bar Sprites */
#define UI_SPRITE_PROGRESS_BG          0x65b5
#define UI_SPRITE_PROGRESS_FILL        0x65b6
#define UI_SPRITE_PROGRESS_BORDER      0x65b7
#define UI_SPRITE_PROGRESS_EMPTY       0x65b8

/* List Box Sprites */
#define UI_SPRITE_LISTBG               0x65bc
#define UI_SPRITE_LISTITEM_NORMAL      0x65bd
#define UI_SPRITE_LISTITEM_SELECTED    0x65c1

/* Extended UI Sprites (for high-res mode) */
#define UI_SPRITE_EX_BUTTON_NORMAL     0x7a145
#define UI_SPRITE_EX_BUTTON_PRESSED    0x7a155
#define UI_SPRITE_EX_BUTTON_HOVER      0x7a152
#define UI_SPRITE_EX_BUTTON_DISABLED   0x7a156

/* Text render queue limits - from FUN_0041d7c0 */
#define TEXT_QUEUE_MAX_ENTRIES         1024
#define TEXT_MAX_LENGTH                256

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

/* Text alignment */
typedef enum {
    UI_ALIGN_LEFT = 0,
    UI_ALIGN_CENTER,
    UI_ALIGN_RIGHT
} UITextAlign;

/* Text queue entry - matches DAT_005676f8 region layout */
typedef struct {
    s16 x;                  /* +0x00: X position */
    s16 y;                  /* +0x02: Y position */
    char text[TEXT_MAX_LENGTH]; /* +0x04: Text string */
    u8  palette;            /* +0x104: Palette index */
    u8  flags;              /* +0x105: Render flags */
    u8  line_height;        /* +0x106: Line height for multiline */
    u8  cursor_pos;         /* +0x107: Cursor position */
    s32 sprite_id;          /* +0x108: Background sprite ID */
    u8  cursor_visible;     /* +0x10c: Cursor visibility */
    u8  reserved1[3];       /* +0x10d: Padding */
    s32 width;              /* +0x110: Text area width */
    s32 height;             /* +0x114: Text area height */
    s32 cursor_x;           /* +0x118: Cursor X position */
    s32 cursor_y;           /* +0x11c: Cursor Y position */
    s32 max_chars;          /* +0x120: Max characters (1=single line) */
    s32 queue_index;        /* +0x124: Queue index */
} UITextAreaEntry;

/* UI element base */
typedef struct UIElement {
    u32 id;
    UIType type;
    u32 flags;
    UIState state;

    /* Position and size */
    s32 x, y;
    s32 width, height;

    /* Rendering */
    u32 bg_sprite;
    u32 border_sprite;
    u32 bg_color;
    u32 border_color;
    u32 text_color;
    u8  text_palette;
    u8  text_align;

    /* Text */
    char text[TEXT_MAX_LENGTH];

    /* Callback */
    void (*on_click)(struct UIElement* elem);
    void (*on_hover)(struct UIElement* elem);
    void (*on_focus)(struct UIElement* elem);
    void (*on_blur)(struct UIElement* elem);
    void (*on_key)(struct UIElement* elem, int key);

    /* User data */
    void* user_data;

    /* Linked list */
    struct UIElement* parent;
    struct UIElement* child;
    struct UIElement* next;
} UIElement;

/* Button structure */
typedef struct {
    UIElement base;
    u32 sprite_normal;
    u32 sprite_pressed;
    u32 sprite_hover;
    u32 sprite_disabled;
    int click_count;
    int is_pressed;
} UIButton;

/* Textbox structure */
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

/* Dialog structure */
typedef struct {
    UIElement base;
    char title[64];
    int drag_x, drag_y;
    int is_dragging;
    int is_modal;
    int result;
} UIDialog;

/* Listbox structure */
typedef struct {
    UIElement base;
    char** items;
    int item_count;
    int selected_index;
    int scroll_offset;
    int item_height;
    int visible_items;
} UIListbox;

/* Progress bar structure */
typedef struct {
    UIElement base;
    int value;
    int max_value;
    u32 fill_color;
    u32 fill_sprite;
} UIProgressbar;

/* Checkbox structure */
typedef struct {
    UIElement base;
    int checked;
    u32 sprite_checked;
    u32 sprite_unchecked;
} UICheckbox;

/* UI Sprite IDs structure - populated by FUN_004155e0 */
typedef struct {
    /* Button sprites */
    u32 btn_normal;         /* DAT_0054bdcc */
    u32 btn_pressed;        /* DAT_0054b5c4 */
    u32 btn_hover;          /* DAT_0054bddc */
    u32 btn_disabled;       /* DAT_0054b5c0 */

    /* Scrollbar sprites */
    u32 scroll_up;          /* DAT_0054ad1c */
    u32 scroll_down;        /* DAT_0054ad2c */
    u32 scroll_track;       /* DAT_0054ad10 */
    u32 scroll_thumb;       /* DAT_0054b19c */
    u32 scroll_left;        /* DAT_0054ad24 */
    u32 scroll_right;       /* DAT_0054b5b8 */

    /* Dialog sprites (9-sprite grid) */
    u32 dialog_bg;          /* DAT_0054b194 / extended threshold */
    u32 dialog_corner_tl;   /* DAT_0054b198 */
    u32 dialog_corner_tr;   /* DAT_0054b18c */
    u32 dialog_corner_bl;   /* DAT_0054b190 */
    u32 dialog_corner_br;   /* DAT_0054b1ac */
    u32 dialog_edge_top;    /* DAT_0054b1b4 */
    u32 dialog_edge_bottom; /* DAT_0054b1a4 */
    u32 dialog_edge_left;   /* DAT_0054b1a8 */
    u32 dialog_edge_right;  /* DAT_0054b1a0 */

    /* Input/Text sprites */
    u32 input_bg;           /* DAT_0054c208 */
    u32 input_cursor;       /* DAT_0054c204 */
    u32 input_focus;        /* DAT_0054c200 */
    u32 input_normal;       /* DAT_0054c218 */
    u32 input_active;       /* DAT_0054c214 */
    u32 input_disabled;     /* DAT_0054c210 */
    u32 input_readonly;     /* DAT_0054c20c */

    /* Checkbox sprites */
    u32 checkbox_unchecked; /* DAT_0054c220 */
    u32 checkbox_checked;   /* DAT_0054c21c */

    /* Progress bar sprites */
    u32 progress_bg;        /* DAT_0054b138 */
    u32 progress_fill;      /* DAT_0054b134 */
    u32 progress_border;    /* DAT_0054b130 */
    u32 progress_empty;     /* DAT_0054b1b0 */

    /* List sprites */
    u32 list_bg;            /* DAT_0054ad28 */
    u32 list_item_normal;   /* DAT_0054b140 */
    u32 list_item_selected; /* DAT_0054b13c */

    /* Extended UI sprites */
    u32 ext_slot1;          /* DAT_0054c82c */
    u32 ext_slot2;          /* DAT_0054b184 */
    u32 ext_slot3;          /* DAT_0054c828 */
    u32 ext_slot4;          /* DAT_0054b178 */
    u32 ext_slot5;          /* DAT_0054b168 */
    u32 ext_slot6;          /* DAT_0054b148 */
    u32 ext_slot7;          /* DAT_0054b14c */
    u32 ext_slot8;          /* DAT_0054b170 */
    u32 ext_slot9;          /* DAT_0054ad14 */
    u32 ext_slot10;         /* DAT_0054c830 */
    u32 ext_slot11;         /* DAT_0054c224 */
    u32 ext_slot12;         /* DAT_0054bdfc */
    u32 ext_slot13;         /* DAT_0054b174 */
    u32 ext_slot14;         /* DAT_0054b164 */
    u32 ext_slot15;         /* DAT_0054b160 */
    u32 ext_slot16;         /* DAT_0054c834 */
    u32 ext_slot17;         /* DAT_0054bdd0 */
    u32 ext_slot18;         /* DAT_0054bde0 */
    u32 ext_slot19;         /* DAT_0054bde8 */
    u32 ext_slot20;         /* DAT_0054a908 */
    u32 ext_slot21;         /* DAT_0054bdd4 */
    u32 ext_slot22;         /* DAT_0054bdf8 */
} UISpriteIDs;

/* UI context */
typedef struct {
    UIElement* root;
    UIElement* focused;
    UIElement* hovered;
    UIElement* captured;

    /* Mouse state */
    s32 mouse_x, mouse_y;
    s32 mouse_down_x, mouse_down_y;
    int mouse_down;
    int mouse_button;

    /* Screen dimensions */
    s32 screen_width, screen_height;

    /* Text queue */
    UITextAreaEntry text_queue[TEXT_QUEUE_MAX_ENTRIES];
    int text_queue_count;

    /* Sprite IDs - populated by FUN_004155e0 */
    UISpriteIDs sprites;

    /* High-res mode flag */
    int high_res_mode;

    /* Initialized flag */
    int initialized;
} UIContext;

/* Global UI context */
extern UIContext g_ui;

/* ========================================
 * UI System Functions
 * ======================================== */

/* Initialization */
int ui_init(void);
void ui_shutdown(void);

/* Update and render */
void ui_update(void);
void ui_render(void);
void ui_handle_input(void);

/* ========================================
 * Dialog Functions
 * ======================================== */

void ui_show_dialog(UIDialog* dlg);
void ui_hide_dialog(UIDialog* dlg);
void ui_close_dialog(UIDialog* dlg);
int ui_dialog_get_result(UIDialog* dlg);

/* Helper dialogs */
void ui_show_message(const char* title, const char* message);
void ui_show_confirm(const char* title, const char* message,
                     void (*on_yes)(void), void (*on_no)(void));

/* ========================================
 * Chat System
 * ======================================== */

typedef struct {
    char messages[100][512];
    int count;
    int scroll;
    char input[256];
    int input_cursor;
    int input_max;
    int visible;
} ChatContext;

void ui_chat_init(void);
void ui_chat_add_message(const char* sender, const char* message);
void ui_chat_render(void);
void ui_chat_input(const char* text);
void ui_chat_clear(void);

/* ========================================
 * Screen Rendering
 * ======================================== */

void ui_render_login_screen(void);
void ui_render_character_select_screen(void);
void ui_render_character_create_screen(void);

/* ========================================
 * UI Sprite Initialization - FUN_004155e0
 * ======================================== */

void ui_init_sprites(int high_res_mode);

/* Include sub-module headers */
#include "ui_element.h"
#include "ui_input.h"
#include "ui_render.h"

#endif /* UI_H */
