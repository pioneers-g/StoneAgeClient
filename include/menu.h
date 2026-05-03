/*
 * Stone Age Client - Menu System Header
 * Reverse engineered from sa_9061.exe (FUN_00422aa0, FUN_00422ce0, FUN_00422e70)
 *
 * Game menu state machine for main menu and in-game menus
 */

#ifndef MENU_H
#define MENU_H

#include "types.h"

/* Menu states - from FUN_00422aa0 DAT_04630df0 values */
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

/* Menu button indices - from DAT_0454e180 array */
typedef enum {
    MENU_BTN_NEW_GAME = 0,
    MENU_BTN_LOAD_GAME = 1,
    MENU_BTN_OPTION = 2,
    MENU_BTN_EXIT = 3,
    MENU_BTN_MAX = 12
} MenuButton;

/* Menu button structure - from FUN_00422ce0 analysis */
typedef struct {
    int x;                  /* +0x18: X position */
    int y;                  /* +0x1c: Y position */
    int sprite_normal;      /* +0x140: Normal sprite ID */
    int state;              /* +0x148: Button state (3=normal, 4=hover) */
    int unknown_14c;
    int param;              /* +0x150: Button parameter */
    u8  type;               /* +0x15: Button type */
    u8  padding[3];
    int width;
    int height;
} MenuButtonData;

/* Menu context - matches DAT_0455xxxx region */
typedef struct {
    /* Current state - DAT_04630df0 */
    MenuState state;
    MenuState next_state;

    /* Button array - DAT_0454e180 */
    MenuButtonData buttons[MENU_BTN_MAX];
    int button_count;

    /* Selection - DAT_0454f68c */
    int selected_button;
    int hovered_button;     /* DAT_04552b34 controls hover state */

    /* Fade animation */
    int fade_alpha;
    int fade_direction;

    /* Option menu state */
    int option_scroll;
    int option_selected;

    /* Confirm dialog state */
    int confirm_result;
    int confirm_action;

    /* Error message */
    char error_msg[256];    /* DAT_04553cf4 */

    /* Animation counters */
    u32 animation_timer;
    int animation_frame;

    /* Flags */
    int initialized;
    int exit_requested;
    int start_game;

} MenuContext;

/* Global menu context */
extern MenuContext g_menu;

/* Initialization */
int menu_init(void);
void menu_shutdown(void);

/* State machine - FUN_00422aa0 */
void menu_update(void);
void menu_render(void);

/* State handlers */
void menu_state_init(void);
void menu_state_load(void);
void menu_state_main(void);
void menu_state_option(void);
void menu_state_confirm(void);
void menu_state_exit(void);

/* Button management - FUN_00422ce0 pattern */
void menu_init_buttons(void);
void menu_update_buttons(void);
int menu_get_hovered_button(int x, int y);
void menu_set_button_state(int index, int state);

/* Input handling */
int menu_handle_click(int x, int y);
int menu_handle_key(int key);

/* Actions */
void menu_start_game(void);
void menu_show_options(void);
void menu_exit_game(void);
void menu_go_back(void);

/* Query */
int menu_is_active(void);
int menu_should_start_game(void);
int menu_should_exit(void);

#endif /* MENU_H */
