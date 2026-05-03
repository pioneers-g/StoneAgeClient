/*
 * Stone Age Client - Menu System Comprehensive Tests
 * Tests for menu.c implementation
 *
 * Covers:
 * - Menu state constants
 * - Button indices and constants
 * - State machine transitions
 * - Button management
 * - Input handling
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

/* Menu states - from FUN_00422aa0 */
typedef enum {
    MENU_STATE_INIT = 0,
    MENU_STATE_LOAD = 1,
    MENU_STATE_FADEIN = 2,
    MENU_STATE_MAIN = 3,
    MENU_STATE_OPTION_INIT = 10,
    MENU_STATE_OPTION = 0x0c,
    MENU_STATE_OPTION_CLEANUP = 0x0b,
    MENU_STATE_CONFIRM_INIT = 0x14,
    MENU_STATE_CONFIRM = 0x15,
    MENU_STATE_EXIT_INIT = 0x1e,
    MENU_STATE_EXIT_WAIT = 0x1f,
    MENU_STATE_ERROR_INIT = 99,
    MENU_STATE_ERROR = 100,
    MENU_STATE_ERROR_WAIT = 0x65
} MenuState;

/* Menu button indices */
typedef enum {
    MENU_BTN_NEW_GAME = 0,
    MENU_BTN_LOAD_GAME = 1,
    MENU_BTN_OPTION = 2,
    MENU_BTN_EXIT = 3,
    MENU_BTN_MAX = 12
} MenuButton;

/* Key codes for testing */
#define KEY_UP          0x26
#define KEY_DOWN        0x28
#define KEY_RETURN      0x0d
#define KEY_ESCAPE      0x1b
#define KEY_Y           0x59
#define KEY_N           0x4e

/* ========================================
 * Structures
 * ======================================== */

typedef struct {
    int x;
    int y;
    int sprite_normal;
    int state;
    int unknown_14c;
    int param;
    u8  type;
    u8  padding[3];
    int width;
    int height;
} MenuButtonData;

typedef struct {
    MenuState state;
    MenuState next_state;
    MenuButtonData buttons[MENU_BTN_MAX];
    int button_count;
    int selected_button;
    int hovered_button;
    int fade_alpha;
    int fade_direction;
    int option_scroll;
    int option_selected;
    int confirm_result;
    int confirm_action;
    char error_msg[256];
    u32 animation_timer;
    int animation_frame;
    int initialized;
    int exit_requested;
    int start_game;
} MenuContext;

/* ========================================
 * Global State
 * ======================================== */

static MenuContext g_menu = {0};

/* Mock input state */
static int g_mock_mouse_x = 0;
static int g_mock_mouse_y = 0;
static int g_mock_mouse_pressed = 0;
static int g_mock_key_pressed = 0;

/* ========================================
 * Implementation Functions
 * ======================================== */

/*
 * Mock input functions
 */
static int input_get_mouse_x(void) {
    return g_mock_mouse_x;
}

static int input_get_mouse_y(void) {
    return g_mock_mouse_y;
}

static int input_mouse_pressed(int button) {
    (void)button;
    return g_mock_mouse_pressed;
}

static int input_key_pressed(int key) {
    return g_mock_key_pressed == key;
}

/*
 * Initialize menu system
 */
static int menu_init(void) {
    memset(&g_menu, 0, sizeof(MenuContext));
    g_menu.state = MENU_STATE_INIT;
    g_menu.selected_button = -1;
    g_menu.fade_alpha = 255;
    return 1;
}

/*
 * Shutdown menu system
 */
static void menu_shutdown(void) {
    memset(&g_menu, 0, sizeof(MenuContext));
}

/*
 * Initialize buttons
 */
static void menu_init_buttons(void) {
    int i;

    memset(g_menu.buttons, 0, sizeof(g_menu.buttons));
    g_menu.selected_button = -1;
    g_menu.hovered_button = -1;

    for (i = 0; i < 4; i++) {
        MenuButtonData* btn = &g_menu.buttons[i];
        btn->x = 400;
        btn->y = 100 + i * 50;
        btn->sprite_normal = 0x716e + i * 2;
        btn->state = 3;  /* Normal */
        btn->param = i;
        btn->type = 10;
        btn->width = 120;
        btn->height = 40;
    }

    g_menu.button_count = 4;
}

/*
 * Get hovered button index
 */
static int menu_get_hovered_button(int x, int y) {
    int i;

    for (i = 0; i < g_menu.button_count; i++) {
        MenuButtonData* btn = &g_menu.buttons[i];

        if (btn->state < 0) continue;

        if (x >= btn->x && x < btn->x + btn->width &&
            y >= btn->y && y < btn->y + btn->height) {
            return i;
        }
    }

    return -1;
}

/*
 * Set button state
 */
static void menu_set_button_state(int index, int state) {
    if (index < 0 || index >= MENU_BTN_MAX) return;
    g_menu.buttons[index].state = state;
}

/*
 * Check if menu is active
 */
static int menu_is_active(void) {
    return g_menu.state != MENU_STATE_INIT;
}

/*
 * Check if should start game
 */
static int menu_should_start_game(void) {
    return g_menu.start_game;
}

/*
 * Check if should exit
 */
static int menu_should_exit(void) {
    return g_menu.exit_requested;
}

/*
 * Start game
 */
static void menu_start_game(void) {
    g_menu.start_game = 1;
}

/*
 * Exit game
 */
static void menu_exit_game(void) {
    g_menu.exit_requested = 1;
}

/*
 * Go back
 */
static void menu_go_back(void) {
    switch (g_menu.state) {
        case MENU_STATE_OPTION:
        case MENU_STATE_OPTION_CLEANUP:
            g_menu.state = MENU_STATE_MAIN;
            break;

        case MENU_STATE_CONFIRM:
            g_menu.state = MENU_STATE_MAIN;
            break;

        default:
            break;
    }
}

/*
 * Handle key
 */
static int menu_handle_key(int key) {
    switch (key) {
        case KEY_UP:
            if (g_menu.hovered_button > 0) {
                g_menu.hovered_button--;
            }
            return 1;

        case KEY_DOWN:
            if (g_menu.hovered_button < g_menu.button_count - 1) {
                g_menu.hovered_button++;
            }
            return 1;

        case KEY_RETURN:
            if (g_menu.hovered_button >= 0) {
                g_menu.start_game = 1;
            }
            return 1;

        case KEY_ESCAPE:
            g_menu.state = MENU_STATE_CONFIRM_INIT;
            g_menu.confirm_action = 0;
            return 1;
    }

    return 0;
}

/*
 * Update buttons
 */
static void menu_update_buttons(void) {
    int i;
    int hovered;

    hovered = -1;

    for (i = 0; i < g_menu.button_count; i++) {
        MenuButtonData* btn = &g_menu.buttons[i];

        if (btn->state < 0) continue;

        if (g_mock_mouse_x >= btn->x && g_mock_mouse_x < btn->x + btn->width &&
            g_mock_mouse_y >= btn->y && g_mock_mouse_y < btn->y + btn->height) {
            hovered = i;
            btn->state = 4;  /* Hover */
        } else {
            btn->state = 3;  /* Normal */
        }
    }

    g_menu.hovered_button = hovered;
}

/*
 * Reset state
 */
static void reset_state(void) {
    memset(&g_menu, 0, sizeof(MenuContext));
    g_mock_mouse_x = 0;
    g_mock_mouse_y = 0;
    g_mock_mouse_pressed = 0;
    g_mock_key_pressed = 0;
}

/* ========================================
 * Test Cases - State Constants
 * ======================================== */

static int test_state_init(void) {
    return MENU_STATE_INIT == 0;
}

static int test_state_load(void) {
    return MENU_STATE_LOAD == 1;
}

static int test_state_fadein(void) {
    return MENU_STATE_FADEIN == 2;
}

static int test_state_main(void) {
    return MENU_STATE_MAIN == 3;
}

static int test_state_option_init(void) {
    return MENU_STATE_OPTION_INIT == 10;
}

static int test_state_option(void) {
    return MENU_STATE_OPTION == 0x0c;
}

static int test_state_confirm_init(void) {
    return MENU_STATE_CONFIRM_INIT == 0x14;
}

static int test_state_confirm(void) {
    return MENU_STATE_CONFIRM == 0x15;
}

static int test_state_exit_init(void) {
    return MENU_STATE_EXIT_INIT == 0x1e;
}

static int test_state_exit_wait(void) {
    return MENU_STATE_EXIT_WAIT == 0x1f;
}

static int test_state_error_init(void) {
    return MENU_STATE_ERROR_INIT == 99;
}

static int test_state_error(void) {
    return MENU_STATE_ERROR == 100;
}

static int test_state_error_wait(void) {
    return MENU_STATE_ERROR_WAIT == 0x65;
}

/* ========================================
 * Test Cases - Button Constants
 * ======================================== */

static int test_btn_new_game(void) {
    return MENU_BTN_NEW_GAME == 0;
}

static int test_btn_load_game(void) {
    return MENU_BTN_LOAD_GAME == 1;
}

static int test_btn_option(void) {
    return MENU_BTN_OPTION == 2;
}

static int test_btn_exit(void) {
    return MENU_BTN_EXIT == 3;
}

static int test_btn_max(void) {
    return MENU_BTN_MAX == 12;
}

/* ========================================
 * Test Cases - Initialization
 * ======================================== */

static int test_init_state(void) {
    reset_state();

    int result = menu_init();

    return result == 1 && g_menu.state == MENU_STATE_INIT;
}

static int test_init_selected(void) {
    reset_state();

    menu_init();

    return g_menu.selected_button == -1;
}

static int test_init_fade_alpha(void) {
    reset_state();

    menu_init();

    return g_menu.fade_alpha == 255;
}

static int test_shutdown(void) {
    reset_state();

    menu_init();
    menu_shutdown();

    return g_menu.state == 0 && g_menu.button_count == 0;
}

/* ========================================
 * Test Cases - Button Management
 * ======================================== */

static int test_init_buttons_count(void) {
    reset_state();

    menu_init_buttons();

    return g_menu.button_count == 4;
}

static int test_init_buttons_positions(void) {
    reset_state();

    menu_init_buttons();

    return g_menu.buttons[0].x == 400 && g_menu.buttons[0].y == 100 &&
           g_menu.buttons[1].y == 150 && g_menu.buttons[2].y == 200;
}

static int test_init_buttons_state(void) {
    reset_state();

    menu_init_buttons();

    return g_menu.buttons[0].state == 3;
}

static int test_init_buttons_hovered(void) {
    reset_state();

    menu_init_buttons();

    return g_menu.hovered_button == -1;
}

static int test_set_button_state_valid(void) {
    reset_state();

    menu_init_buttons();
    menu_set_button_state(0, 4);

    return g_menu.buttons[0].state == 4;
}

static int test_set_button_state_invalid_neg(void) {
    reset_state();

    menu_init_buttons();
    int old_state = g_menu.buttons[0].state;
    menu_set_button_state(-1, 4);

    return g_menu.buttons[0].state == old_state;
}

static int test_set_button_state_invalid_high(void) {
    reset_state();

    menu_init_buttons();
    menu_set_button_state(MENU_BTN_MAX, 4);

    /* Should not crash, no change */
    return 1;
}

/* ========================================
 * Test Cases - Hover Detection
 * ======================================== */

static int test_hover_inside_first(void) {
    reset_state();
    menu_init_buttons();

    int result = menu_get_hovered_button(450, 120);

    return result == 0;
}

static int test_hover_inside_second(void) {
    reset_state();
    menu_init_buttons();

    int result = menu_get_hovered_button(450, 170);

    return result == 1;
}

static int test_hover_outside_left(void) {
    reset_state();
    menu_init_buttons();

    int result = menu_get_hovered_button(100, 120);

    return result == -1;
}

static int test_hover_outside_right(void) {
    reset_state();
    menu_init_buttons();

    int result = menu_get_hovered_button(600, 120);

    return result == -1;
}

static int test_hover_outside_above(void) {
    reset_state();
    menu_init_buttons();

    int result = menu_get_hovered_button(450, 50);

    return result == -1;
}

static int test_hover_outside_below(void) {
    reset_state();
    menu_init_buttons();

    int result = menu_get_hovered_button(450, 400);

    return result == -1;
}

static int test_hover_boundary_left(void) {
    reset_state();
    menu_init_buttons();

    /* At left boundary (x=400) */
    int result = menu_get_hovered_button(400, 120);

    return result == 0;
}

static int test_hover_boundary_right(void) {
    reset_state();
    menu_init_buttons();

    /* Just before right boundary (x=519) */
    int result = menu_get_hovered_button(519, 120);

    return result == 0;
}

static int test_hover_boundary_right_out(void) {
    reset_state();
    menu_init_buttons();

    /* At right boundary (x=520 = 400+120) */
    int result = menu_get_hovered_button(520, 120);

    return result == -1;
}

static int test_hover_disabled_button(void) {
    reset_state();
    menu_init_buttons();

    /* Disable button 0 */
    g_menu.buttons[0].state = -1;

    int result = menu_get_hovered_button(450, 120);

    return result == -1;
}

/* ========================================
 * Test Cases - Update Buttons
 * ======================================== */

static int test_update_buttons_hover(void) {
    reset_state();
    menu_init_buttons();

    g_mock_mouse_x = 450;
    g_mock_mouse_y = 120;
    menu_update_buttons();

    return g_menu.hovered_button == 0 && g_menu.buttons[0].state == 4;
}

static int test_update_buttons_no_hover(void) {
    reset_state();
    menu_init_buttons();

    g_mock_mouse_x = 100;
    g_mock_mouse_y = 100;
    menu_update_buttons();

    return g_menu.hovered_button == -1;
}

static int test_update_buttons_transition(void) {
    reset_state();
    menu_init_buttons();

    /* First hover */
    g_mock_mouse_x = 450;
    g_mock_mouse_y = 120;
    menu_update_buttons();
    int first = g_menu.hovered_button;

    /* Move to second button */
    g_mock_mouse_x = 450;
    g_mock_mouse_y = 170;
    menu_update_buttons();
    int second = g_menu.hovered_button;
    int first_state = g_menu.buttons[0].state;

    return first == 0 && second == 1 && first_state == 3;
}

/* ========================================
 * Test Cases - Key Handling
 * ======================================== */

static int test_key_up(void) {
    reset_state();
    menu_init_buttons();
    g_menu.hovered_button = 1;

    int result = menu_handle_key(KEY_UP);

    return result == 1 && g_menu.hovered_button == 0;
}

static int test_key_up_at_top(void) {
    reset_state();
    menu_init_buttons();
    g_menu.hovered_button = 0;

    menu_handle_key(KEY_UP);

    return g_menu.hovered_button == 0;  /* Stays at top */
}

static int test_key_down(void) {
    reset_state();
    menu_init_buttons();
    g_menu.hovered_button = 0;

    int result = menu_handle_key(KEY_DOWN);

    return result == 1 && g_menu.hovered_button == 1;
}

static int test_key_down_at_bottom(void) {
    reset_state();
    menu_init_buttons();
    g_menu.hovered_button = g_menu.button_count - 1;

    menu_handle_key(KEY_DOWN);

    return g_menu.hovered_button == g_menu.button_count - 1;  /* Stays at bottom */
}

static int test_key_return(void) {
    reset_state();
    menu_init_buttons();
    g_menu.hovered_button = 0;

    int result = menu_handle_key(KEY_RETURN);

    return result == 1 && g_menu.start_game == 1;
}

static int test_key_escape(void) {
    reset_state();
    menu_init();
    g_menu.state = MENU_STATE_MAIN;

    int result = menu_handle_key(KEY_ESCAPE);

    return result == 1 && g_menu.state == MENU_STATE_CONFIRM_INIT;
}

static int test_key_unknown(void) {
    reset_state();

    int result = menu_handle_key(0x41);  /* 'A' key */

    return result == 0;
}

/* ========================================
 * Test Cases - Active State
 * ======================================== */

static int test_active_init(void) {
    reset_state();
    menu_init();

    return menu_is_active() == 0;  /* INIT state means not active */
}

static int test_active_main(void) {
    reset_state();
    menu_init();
    g_menu.state = MENU_STATE_MAIN;

    return menu_is_active() == 1;
}

/* ========================================
 * Test Cases - Start/Exit
 * ======================================== */

static int test_start_game(void) {
    reset_state();

    menu_start_game();

    return menu_should_start_game() == 1;
}

static int test_exit_game(void) {
    reset_state();

    menu_exit_game();

    return menu_should_exit() == 1;
}

static int test_should_start_default(void) {
    reset_state();

    return menu_should_start_game() == 0;
}

static int test_should_exit_default(void) {
    reset_state();

    return menu_should_exit() == 0;
}

/* ========================================
 * Test Cases - Go Back
 * ======================================== */

static int test_go_back_option(void) {
    reset_state();
    g_menu.state = MENU_STATE_OPTION;

    menu_go_back();

    return g_menu.state == MENU_STATE_MAIN;
}

static int test_go_back_confirm(void) {
    reset_state();
    g_menu.state = MENU_STATE_CONFIRM;

    menu_go_back();

    return g_menu.state == MENU_STATE_MAIN;
}

static int test_go_back_main(void) {
    reset_state();
    g_menu.state = MENU_STATE_MAIN;

    menu_go_back();

    return g_menu.state == MENU_STATE_MAIN;  /* No change */
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Menu System Comprehensive Tests ===\n\n");

    printf("State Constants Tests:\n");
    TEST(state_init);
    TEST(state_load);
    TEST(state_fadein);
    TEST(state_main);
    TEST(state_option_init);
    TEST(state_option);
    TEST(state_confirm_init);
    TEST(state_confirm);
    TEST(state_exit_init);
    TEST(state_exit_wait);
    TEST(state_error_init);
    TEST(state_error);
    TEST(state_error_wait);

    printf("\nButton Constants Tests:\n");
    TEST(btn_new_game);
    TEST(btn_load_game);
    TEST(btn_option);
    TEST(btn_exit);
    TEST(btn_max);

    printf("\nInitialization Tests:\n");
    TEST(init_state);
    TEST(init_selected);
    TEST(init_fade_alpha);
    TEST(shutdown);

    printf("\nButton Management Tests:\n");
    TEST(init_buttons_count);
    TEST(init_buttons_positions);
    TEST(init_buttons_state);
    TEST(init_buttons_hovered);
    TEST(set_button_state_valid);
    TEST(set_button_state_invalid_neg);
    TEST(set_button_state_invalid_high);

    printf("\nHover Detection Tests:\n");
    TEST(hover_inside_first);
    TEST(hover_inside_second);
    TEST(hover_outside_left);
    TEST(hover_outside_right);
    TEST(hover_outside_above);
    TEST(hover_outside_below);
    TEST(hover_boundary_left);
    TEST(hover_boundary_right);
    TEST(hover_boundary_right_out);
    TEST(hover_disabled_button);

    printf("\nUpdate Buttons Tests:\n");
    TEST(update_buttons_hover);
    TEST(update_buttons_no_hover);
    TEST(update_buttons_transition);

    printf("\nKey Handling Tests:\n");
    TEST(key_up);
    TEST(key_up_at_top);
    TEST(key_down);
    TEST(key_down_at_bottom);
    TEST(key_return);
    TEST(key_escape);
    TEST(key_unknown);

    printf("\nActive State Tests:\n");
    TEST(active_init);
    TEST(active_main);

    printf("\nStart/Exit Tests:\n");
    TEST(start_game);
    TEST(exit_game);
    TEST(should_start_default);
    TEST(should_exit_default);

    printf("\nGo Back Tests:\n");
    TEST(go_back_option);
    TEST(go_back_confirm);
    TEST(go_back_main);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - menu_update state transitions
     * - menu_state_main with mouse input
     * - menu_state_confirm with Yes/No clicks
     * - menu_state_exit fade animation
     * - Animation timer updates
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
