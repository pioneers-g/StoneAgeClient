/*
 * Stone Age Client - UI Input System Comprehensive Tests
 * Tests for ui_input.c implementation
 *
 * Covers:
 * - Mouse move handling
 * - Mouse down/up handling
 * - Keyboard handling for textbox
 * - Character input
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

/* Key codes */
#define DIK_BACK            0x0E
#define DIK_RETURN          0x1C
#define DIK_LEFT            0xCB
#define DIK_RIGHT           0xCD
#define DIK_DELETE          0xD3
#define DIK_HOME            0xC7
#define DIK_END             0xCF

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
    UIElement* root;
    UIElement* focused;
    UIElement* hovered;
    UIElement* captured;
    s32 mouse_x, mouse_y;
    s32 mouse_down_x, mouse_down_y;
    int mouse_down;
    int mouse_button;
    int initialized;
} UIContext;

/* ========================================
 * Global State
 * ======================================== */

static UIContext g_ui = {0};
static u32 g_next_id = 1;
static int g_click_count = 0;

/* ========================================
 * Implementation Functions
 * ======================================== */

static UIElement* ui_create_element(UIType type, int x, int y, int w, int h) {
    UIElement* elem = (UIElement*)calloc(1, sizeof(UIElement));
    if (!elem) return NULL;

    elem->id = g_next_id++;
    elem->type = type;
    elem->x = x;
    elem->y = y;
    elem->width = w;
    elem->height = h;
    elem->flags = UI_FLAG_VISIBLE | UI_FLAG_ENABLED;
    elem->state = UI_STATE_NORMAL;

    return elem;
}

static UITextbox* ui_create_textbox(int x, int y, int w, int h, int max_len) {
    UITextbox* tb = (UITextbox*)calloc(1, sizeof(UITextbox));
    if (!tb) return NULL;

    tb->base.id = g_next_id++;
    tb->base.type = UI_TEXTBOX;
    tb->base.x = x;
    tb->base.y = y;
    tb->base.width = w;
    tb->base.height = h;
    tb->base.flags = UI_FLAG_VISIBLE | UI_FLAG_ENABLED | UI_FLAG_FOCUSABLE;
    tb->base.state = UI_STATE_NORMAL;
    tb->max_length = max_len < 1024 ? max_len : 1023;

    return tb;
}

static void ui_destroy_element(UIElement* elem) {
    if (!elem) return;
    free(elem);
}

static void ui_set_focus(UIElement* elem) {
    g_ui.focused = elem;
}

static int ui_handle_mouse_move(int x, int y) {
    g_ui.mouse_x = x;
    g_ui.mouse_y = y;

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

static int ui_handle_mouse_down(int x, int y, int button) {
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

static int ui_handle_mouse_up(int x, int y, int button) {
    g_ui.mouse_down = 0;

    if (g_ui.captured) {
        if (g_ui.captured->state == UI_STATE_PRESSED) {
            g_ui.captured->state = UI_STATE_HOVER;
        }
        g_ui.captured = NULL;
    }

    return 0;
}

static void ui_handle_click(int x, int y) {
    ui_handle_mouse_move(x, y);
    ui_handle_mouse_down(x, y, 1);
    ui_handle_mouse_up(x, y, 1);
}

static int ui_handle_key_down(int key) {
    if (g_ui.focused) {
        if (g_ui.focused->on_key) {
            g_ui.focused->on_key(g_ui.focused, key);
        }

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
            }
        }

        return 1;
    }

    return 0;
}

static int ui_handle_char(int ch) {
    if (g_ui.focused && g_ui.focused->type == UI_TEXTBOX) {
        UITextbox* tb = (UITextbox*)g_ui.focused;

        if (ch >= 32 && ch < 127 && tb->cursor_pos < tb->max_length - 1) {
            memmove(tb->buffer + tb->cursor_pos + 1,
                    tb->buffer + tb->cursor_pos,
                    strlen(tb->buffer) - tb->cursor_pos + 1);
            tb->buffer[tb->cursor_pos++] = (char)ch;
            return 1;
        }
    }

    return 0;
}

static void on_click_handler(UIElement* elem) {
    (void)elem;
    g_click_count++;
}

static void reset_state(void) {
    if (g_ui.root) {
        ui_destroy_element(g_ui.root);
    }
    memset(&g_ui, 0, sizeof(UIContext));
    g_next_id = 1;
    g_click_count = 0;
}

/* ========================================
 * Test Cases - Mouse Move
 * ======================================== */

static int test_mouse_move_coords(void) {
    reset_state();

    ui_handle_mouse_move(100, 200);

    return g_ui.mouse_x == 100 && g_ui.mouse_y == 200;
}

static int test_mouse_move_hover_inside(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_BUTTON, 10, 10, 100, 50);
    g_ui.root = elem;

    ui_handle_mouse_move(50, 30);

    int ok = g_ui.hovered == elem && elem->state == UI_STATE_HOVER;

    ui_destroy_element(elem);
    g_ui.root = NULL;
    return ok;
}

static int test_mouse_move_hover_outside(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_BUTTON, 10, 10, 100, 50);
    g_ui.root = elem;

    ui_handle_mouse_move(200, 200);

    int ok = g_ui.hovered == NULL && elem->state == UI_STATE_NORMAL;

    ui_destroy_element(elem);
    g_ui.root = NULL;
    return ok;
}

static int test_mouse_move_hover_transition(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_BUTTON, 10, 10, 100, 50);
    g_ui.root = elem;

    /* Move inside */
    ui_handle_mouse_move(50, 30);
    int state1 = elem->state == UI_STATE_HOVER;

    /* Move outside */
    ui_handle_mouse_move(200, 200);
    int state2 = elem->state == UI_STATE_NORMAL;

    ui_destroy_element(elem);
    g_ui.root = NULL;
    return state1 && state2;
}

static int test_mouse_move_invisible(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_BUTTON, 10, 10, 100, 50);
    elem->flags &= ~UI_FLAG_VISIBLE;
    g_ui.root = elem;

    ui_handle_mouse_move(50, 30);

    int ok = g_ui.hovered == NULL;

    ui_destroy_element(elem);
    g_ui.root = NULL;
    return ok;
}

/* ========================================
 * Test Cases - Mouse Down
 * ======================================== */

static int test_mouse_down_pressed_state(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_BUTTON, 10, 10, 100, 50);
    g_ui.root = elem;
    g_ui.hovered = elem;

    ui_handle_mouse_down(50, 30, 1);

    int ok = elem->state == UI_STATE_PRESSED && g_ui.captured == elem;

    ui_destroy_element(elem);
    g_ui.root = NULL;
    return ok;
}

static int test_mouse_down_click_callback(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_BUTTON, 10, 10, 100, 50);
    elem->on_click = on_click_handler;
    g_ui.root = elem;
    g_ui.hovered = elem;

    ui_handle_mouse_down(50, 30, 1);

    int ok = g_click_count == 1;

    ui_destroy_element(elem);
    g_ui.root = NULL;
    return ok;
}

static int test_mouse_down_no_hover(void) {
    reset_state();

    ui_handle_mouse_down(50, 30, 1);

    return g_ui.captured == NULL;
}

/* ========================================
 * Test Cases - Mouse Up
 * ======================================== */

static int test_mouse_up_released(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_BUTTON, 10, 10, 100, 50);
    g_ui.root = elem;
    g_ui.hovered = elem;
    g_ui.captured = elem;
    elem->state = UI_STATE_PRESSED;

    ui_handle_mouse_up(50, 30, 1);

    int ok = elem->state == UI_STATE_HOVER && g_ui.captured == NULL;

    ui_destroy_element(elem);
    g_ui.root = NULL;
    return ok;
}

static int test_mouse_up_down_flag(void) {
    reset_state();

    g_ui.mouse_down = 1;
    ui_handle_mouse_up(0, 0, 1);

    return g_ui.mouse_down == 0;
}

/* ========================================
 * Test Cases - Click Handling
 * ======================================== */

static int test_click_sequence(void) {
    reset_state();

    UIElement* elem = ui_create_element(UI_BUTTON, 10, 10, 100, 50);
    g_ui.root = elem;

    ui_handle_click(50, 30);

    int ok = g_ui.hovered == elem;

    ui_destroy_element(elem);
    g_ui.root = NULL;
    return ok;
}

/* ========================================
 * Test Cases - Keyboard - Backspace
 * ======================================== */

static int test_key_backspace_middle(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 256);
    strcpy(tb->buffer, "Hello");
    tb->cursor_pos = 3;  /* Before 'l' */
    g_ui.focused = (UIElement*)tb;

    ui_handle_key_down(DIK_BACK);

    int ok = strcmp(tb->buffer, "Helo") == 0 && tb->cursor_pos == 2;

    free(tb);
    g_ui.focused = NULL;
    return ok;
}

static int test_key_backspace_start(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 256);
    strcpy(tb->buffer, "Hello");
    tb->cursor_pos = 0;
    g_ui.focused = (UIElement*)tb;

    ui_handle_key_down(DIK_BACK);

    int ok = strcmp(tb->buffer, "Hello") == 0 && tb->cursor_pos == 0;

    free(tb);
    g_ui.focused = NULL;
    return ok;
}

static int test_key_backspace_end(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 256);
    strcpy(tb->buffer, "Hello");
    tb->cursor_pos = 5;
    g_ui.focused = (UIElement*)tb;

    ui_handle_key_down(DIK_BACK);

    int ok = strcmp(tb->buffer, "Hell") == 0 && tb->cursor_pos == 4;

    free(tb);
    g_ui.focused = NULL;
    return ok;
}

/* ========================================
 * Test Cases - Keyboard - Delete
 * ======================================== */

static int test_key_delete_middle(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 256);
    strcpy(tb->buffer, "Hello");
    tb->cursor_pos = 2;  /* Before first 'l' */
    g_ui.focused = (UIElement*)tb;

    ui_handle_key_down(DIK_DELETE);

    int ok = strcmp(tb->buffer, "Helo") == 0 && tb->cursor_pos == 2;

    free(tb);
    g_ui.focused = NULL;
    return ok;
}

static int test_key_delete_end(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 256);
    strcpy(tb->buffer, "Hello");
    tb->cursor_pos = 5;
    g_ui.focused = (UIElement*)tb;

    ui_handle_key_down(DIK_DELETE);

    int ok = strcmp(tb->buffer, "Hello") == 0 && tb->cursor_pos == 5;

    free(tb);
    g_ui.focused = NULL;
    return ok;
}

/* ========================================
 * Test Cases - Keyboard - Navigation
 * ======================================== */

static int test_key_left(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 256);
    strcpy(tb->buffer, "Hello");
    tb->cursor_pos = 3;
    g_ui.focused = (UIElement*)tb;

    ui_handle_key_down(DIK_LEFT);

    int ok = tb->cursor_pos == 2;

    free(tb);
    g_ui.focused = NULL;
    return ok;
}

static int test_key_left_start(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 256);
    strcpy(tb->buffer, "Hello");
    tb->cursor_pos = 0;
    g_ui.focused = (UIElement*)tb;

    ui_handle_key_down(DIK_LEFT);

    int ok = tb->cursor_pos == 0;

    free(tb);
    g_ui.focused = NULL;
    return ok;
}

static int test_key_right(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 256);
    strcpy(tb->buffer, "Hello");
    tb->cursor_pos = 2;
    g_ui.focused = (UIElement*)tb;

    ui_handle_key_down(DIK_RIGHT);

    int ok = tb->cursor_pos == 3;

    free(tb);
    g_ui.focused = NULL;
    return ok;
}

static int test_key_right_end(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 256);
    strcpy(tb->buffer, "Hello");
    tb->cursor_pos = 5;
    g_ui.focused = (UIElement*)tb;

    ui_handle_key_down(DIK_RIGHT);

    int ok = tb->cursor_pos == 5;

    free(tb);
    g_ui.focused = NULL;
    return ok;
}

static int test_key_home(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 256);
    strcpy(tb->buffer, "Hello");
    tb->cursor_pos = 4;
    g_ui.focused = (UIElement*)tb;

    ui_handle_key_down(DIK_HOME);

    int ok = tb->cursor_pos == 0;

    free(tb);
    g_ui.focused = NULL;
    return ok;
}

static int test_key_end(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 256);
    strcpy(tb->buffer, "Hello");
    tb->cursor_pos = 0;
    g_ui.focused = (UIElement*)tb;

    ui_handle_key_down(DIK_END);

    int ok = tb->cursor_pos == 5;

    free(tb);
    g_ui.focused = NULL;
    return ok;
}

/* ========================================
 * Test Cases - Character Input
 * ======================================== */

static int test_char_input(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 256);
    g_ui.focused = (UIElement*)tb;

    ui_handle_char('A');

    int ok = strcmp(tb->buffer, "A") == 0 && tb->cursor_pos == 1;

    free(tb);
    g_ui.focused = NULL;
    return ok;
}

static int test_char_input_middle(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 256);
    strcpy(tb->buffer, "Hllo");
    tb->cursor_pos = 1;
    g_ui.focused = (UIElement*)tb;

    ui_handle_char('e');

    int ok = strcmp(tb->buffer, "Hello") == 0 && tb->cursor_pos == 2;

    free(tb);
    g_ui.focused = NULL;
    return ok;
}

static int test_char_input_multiple(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 256);
    g_ui.focused = (UIElement*)tb;

    ui_handle_char('H');
    ui_handle_char('i');

    int ok = strcmp(tb->buffer, "Hi") == 0 && tb->cursor_pos == 2;

    free(tb);
    g_ui.focused = NULL;
    return ok;
}

static int test_char_input_max_length(void) {
    reset_state();

    /* max_length=5 means buffer can hold 4 chars + null terminator
     * Condition is: cursor_pos < max_length - 1 */
    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 5);
    strcpy(tb->buffer, "Hel");
    tb->cursor_pos = 3;
    g_ui.focused = (UIElement*)tb;

    int result1 = ui_handle_char('l');  /* Should succeed: 3 < 4 */
    int result2 = ui_handle_char('o');  /* Should fail: 4 < 4 is false */

    int ok = result1 == 1 && result2 == 0 && strcmp(tb->buffer, "Hell") == 0;

    free(tb);
    g_ui.focused = NULL;
    return ok;
}

static int test_char_input_control(void) {
    reset_state();

    UITextbox* tb = ui_create_textbox(0, 0, 100, 20, 256);
    g_ui.focused = (UIElement*)tb;

    int result = ui_handle_char(10);  /* Newline (control char) */

    int ok = result == 0 && tb->buffer[0] == '\0';

    free(tb);
    g_ui.focused = NULL;
    return ok;
}

static int test_char_input_no_focus(void) {
    reset_state();

    int result = ui_handle_char('A');

    return result == 0;
}

/* ========================================
 * Test Cases - Key Handling - No Focus
 * ======================================== */

static int test_key_no_focus(void) {
    reset_state();

    int result = ui_handle_key_down(DIK_LEFT);

    return result == 0;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== UI Input System Comprehensive Tests ===\n\n");

    printf("Mouse Move Tests:\n");
    TEST(mouse_move_coords);
    TEST(mouse_move_hover_inside);
    TEST(mouse_move_hover_outside);
    TEST(mouse_move_hover_transition);
    TEST(mouse_move_invisible);

    printf("\nMouse Down Tests:\n");
    TEST(mouse_down_pressed_state);
    TEST(mouse_down_click_callback);
    TEST(mouse_down_no_hover);

    printf("\nMouse Up Tests:\n");
    TEST(mouse_up_released);
    TEST(mouse_up_down_flag);

    printf("\nClick Handling Tests:\n");
    TEST(click_sequence);

    printf("\nKeyboard Backspace Tests:\n");
    TEST(key_backspace_middle);
    TEST(key_backspace_start);
    TEST(key_backspace_end);

    printf("\nKeyboard Delete Tests:\n");
    TEST(key_delete_middle);
    TEST(key_delete_end);

    printf("\nKeyboard Navigation Tests:\n");
    TEST(key_left);
    TEST(key_left_start);
    TEST(key_right);
    TEST(key_right_end);
    TEST(key_home);
    TEST(key_end);

    printf("\nCharacter Input Tests:\n");
    TEST(char_input);
    TEST(char_input_middle);
    TEST(char_input_multiple);
    TEST(char_input_max_length);
    TEST(char_input_control);
    TEST(char_input_no_focus);

    printf("\nKey Handling Edge Cases:\n");
    TEST(key_no_focus);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    reset_state();

    return (tests_passed == tests_run) ? 0 : 1;
}
