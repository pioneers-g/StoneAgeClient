/*
 * Stone Age Client - Menu System Implementation
 * Reverse engineered from sa_9061.exe (FUN_00422aa0, FUN_00422ce0, FUN_00422e70)
 *
 * Game menu state machine for main menu and in-game menus
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "menu.h"
#include "render.h"
#include "input.h"
#include "gamestate.h"
#include "game.h"
#include "sound.h"
#include "save.h"
#include "logger.h"

/* Global menu context */
MenuContext g_menu = {0};

/* Button sprite IDs - from DAT_004b82xx region */
static const struct {
    int sprite_normal;
    int sprite_hover;
    int x;
    int y;
} s_button_data[MENU_BTN_MAX] = {
    { 0x716e, 0x716f, 400, 100 },   /* New Game */
    { 0x7170, 0x7171, 400, 150 },   /* Load Game */
    { 0x7172, 0x7173, 400, 200 },   /* Options */
    { 0x7174, 0x7175, 400, 250 },   /* Exit */
    /* Additional buttons... */
};

/* Error messages - from DAT_004b9xxx region */
static const char* s_error_server = "Server connection failed";
static const char* s_error_timeout = "Connection timeout";
static const char* s_error_version = "Version mismatch";

/*
 * Initialize menu system
 */
int menu_init(void) {
    memset(&g_menu, 0, sizeof(MenuContext));
    g_menu.state = MENU_STATE_INIT;
    g_menu.selected_button = -1;
    g_menu.fade_alpha = 255;

    LOG_INFO("Menu system initialized");
    return 1;
}

/*
 * Shutdown menu system
 */
void menu_shutdown(void) {
    memset(&g_menu, 0, sizeof(MenuContext));
    LOG_INFO("Menu system shutdown");
}

/*
 * Main update function - FUN_00422aa0 pattern
 */
void menu_update(void) {
    switch (g_menu.state) {
        case MENU_STATE_INIT:
            menu_state_init();
            break;

        case MENU_STATE_LOAD:
            menu_state_load();
            break;

        case MENU_STATE_FADEIN:
            /* Fade in animation */
            g_menu.fade_alpha -= 5;
            if (g_menu.fade_alpha <= 0) {
                g_menu.fade_alpha = 0;
                g_menu.state = MENU_STATE_MAIN;
            }
            break;

        case MENU_STATE_MAIN:
            menu_state_main();
            break;

        case MENU_STATE_OPTION_INIT:
            /* Initialize options menu */
            g_menu.option_scroll = 0;
            g_menu.option_selected = 0;
            g_menu.state = MENU_STATE_OPTION;
            break;

        case MENU_STATE_OPTION:
            menu_state_option();
            break;

        case MENU_STATE_OPTION_CLEANUP:
            /* Cleanup options menu */
            g_menu.state = MENU_STATE_MAIN;
            break;

        case MENU_STATE_CONFIRM_INIT:
            /* Initialize confirm dialog */
            g_menu.confirm_result = 0;
            g_menu.state = MENU_STATE_CONFIRM;
            break;

        case MENU_STATE_CONFIRM:
            menu_state_confirm();
            break;

        case MENU_STATE_EXIT_INIT:
            /* Start exit sequence */
            g_menu.state = MENU_STATE_EXIT_WAIT;
            break;

        case MENU_STATE_EXIT_WAIT:
            menu_state_exit();
            break;

        case MENU_STATE_ERROR:
            /* Show error message */
            g_menu.state = MENU_STATE_ERROR_WAIT;
            break;

        case MENU_STATE_ERROR_WAIT:
            /* Wait for user acknowledgment */
            if (input_key_pressed(KEY_RETURN) || input_mouse_pressed(0)) {
                g_menu.state = MENU_STATE_MAIN;
            }
            break;

        default:
            break;
    }

    /* Update animation */
    g_menu.animation_timer++;
    if (g_menu.animation_timer >= 30) {
        g_menu.animation_timer = 0;
        g_menu.animation_frame = (g_menu.animation_frame + 1) % 8;
    }
}

/*
 * Initialize state - FUN_00422aa0 case 0
 */
void menu_state_init(void) {
    menu_init_buttons();
    g_menu.state = MENU_STATE_LOAD;
}

/*
 * Load state - FUN_00422ce0
 */
void menu_state_load(void) {
    /* Clear button states */
    memset(g_menu.buttons, 0, sizeof(g_menu.buttons));

    /* Initialize buttons from data */
    for (int i = 0; i < 4; i++) {
        MenuButtonData* btn = &g_menu.buttons[i];
        btn->x = s_button_data[i].x;
        btn->y = s_button_data[i].y;
        btn->sprite_normal = s_button_data[i].sprite_normal;
        btn->state = 3;  /* Normal state */
        btn->param = i;
        btn->type = 10;
        btn->width = 120;
        btn->height = 40;
    }

    g_menu.button_count = 4;
    g_menu.selected_button = -1;
    g_menu.state = MENU_STATE_FADEIN;
}

/*
 * Main menu state - FUN_00422e70 pattern
 */
void menu_state_main(void) {
    int mouse_x, mouse_y;
    int hovered;

    mouse_x = input_get_mouse_x();
    mouse_y = input_get_mouse_y();

    /* Check button hover */
    hovered = menu_get_hovered_button(mouse_x, mouse_y);

    if (hovered != g_menu.hovered_button) {
        /* Button hover changed */
        if (g_menu.hovered_button >= 0) {
            menu_set_button_state(g_menu.hovered_button, 3);  /* Normal */
        }
        g_menu.hovered_button = hovered;
        if (hovered >= 0) {
            menu_set_button_state(hovered, 4);  /* Hover */
            /* Play hover sound */
            /* sound_play_effect(0xd9, 0x140, 0xf0); */
        }
    }

    /* Handle click */
    if (input_mouse_pressed(0)) {
        if (hovered >= 0) {
            menu_handle_click(mouse_x, mouse_y);
        } else {
            /* Click outside buttons - check for exit */
            g_menu.state = MENU_STATE_ERROR;
        }
    }

    /* Handle keyboard */
    if (input_key_pressed(KEY_RETURN)) {
        if (g_menu.hovered_button >= 0) {
            menu_handle_click(0, 0);
        }
    }

    /* Escape key */
    if (input_key_pressed(KEY_ESCAPE)) {
        g_menu.state = MENU_STATE_CONFIRM_INIT;
        g_menu.confirm_action = 0;  /* Exit confirm */
    }
}

/*
 * Options menu state
 */
void menu_state_option(void) {
    /* Handle keyboard navigation */
    if (input_key_pressed(KEY_UP)) {
        g_menu.option_selected--;
        if (g_menu.option_selected < 0) {
            g_menu.option_selected = 0;
        }
    }

    if (input_key_pressed(KEY_DOWN)) {
        g_menu.option_selected++;
        if (g_menu.option_selected >= 5) {
            g_menu.option_selected = 4;
        }
    }

    if (input_key_pressed(KEY_ESCAPE)) {
        g_menu.state = MENU_STATE_OPTION_CLEANUP;
    }

    if (input_key_pressed(KEY_RETURN)) {
        /* Apply selected option */
    }
}

/*
 * Confirm dialog state
 */
void menu_state_confirm(void) {
    /* Handle keyboard */
    if (input_key_pressed(KEY_RETURN) || input_key_pressed(KEY_Y)) {
        g_menu.confirm_result = 1;
        if (g_menu.confirm_action == 0) {
            /* Exit game */
            menu_exit_game();
        }
    }

    if (input_key_pressed(KEY_ESCAPE) || input_key_pressed(KEY_N)) {
        g_menu.confirm_result = 0;
        g_menu.state = MENU_STATE_MAIN;
    }

    /* Handle mouse */
    if (input_mouse_pressed(0)) {
        int x = input_get_mouse_x();
        int y = input_get_mouse_y();

        /* Check Yes button (left) */
        if (x >= 250 && x < 320 && y >= 280 && y < 320) {
            g_menu.confirm_result = 1;
            if (g_menu.confirm_action == 0) {
                menu_exit_game();
            }
        }

        /* Check No button (right) */
        if (x >= 340 && x < 410 && y >= 280 && y < 320) {
            g_menu.confirm_result = 0;
            g_menu.state = MENU_STATE_MAIN;
        }
    }
}

/*
 * Exit state
 */
void menu_state_exit(void) {
    /* Fade out */
    g_menu.fade_alpha += 5;
    if (g_menu.fade_alpha >= 255) {
        g_menu.fade_alpha = 255;
        g_menu.exit_requested = 1;
    }
}

/*
 * Initialize buttons - FUN_00422ce0
 */
void menu_init_buttons(void) {
    int i;

    /* Clear button array */
    memset(g_menu.buttons, 0, sizeof(g_menu.buttons));
    g_menu.selected_button = -1;
    g_menu.hovered_button = -1;

    /* Initialize buttons from data table */
    for (i = 0; i < MENU_BTN_MAX; i++) {
        MenuButtonData* btn = &g_menu.buttons[i];

        if (s_button_data[i].sprite_normal == 0) {
            btn->state = -1;  /* Disabled */
            continue;
        }

        btn->x = s_button_data[i].x;
        btn->y = s_button_data[i].y;
        btn->sprite_normal = s_button_data[i].sprite_normal;
        btn->state = 3;  /* Normal */
        btn->param = i;
        btn->type = 10;
        btn->width = 120;
        btn->height = 40;
    }

    g_menu.button_count = MENU_BTN_MAX;
}

/*
 * Update buttons
 */
void menu_update_buttons(void) {
    int i;
    int mouse_x, mouse_y;
    int hovered;

    mouse_x = input_get_mouse_x();
    mouse_y = input_get_mouse_y();

    hovered = -1;

    for (i = 0; i < g_menu.button_count; i++) {
        MenuButtonData* btn = &g_menu.buttons[i];

        if (btn->state < 0) continue;

        /* Check if mouse is over button */
        if (mouse_x >= btn->x && mouse_x < btn->x + btn->width &&
            mouse_y >= btn->y && mouse_y < btn->y + btn->height) {
            hovered = i;
            btn->state = 4;  /* Hover */
        } else {
            btn->state = 3;  /* Normal */
        }
    }

    g_menu.hovered_button = hovered;
}

/*
 * Get hovered button index
 */
int menu_get_hovered_button(int x, int y) {
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
void menu_set_button_state(int index, int state) {
    if (index < 0 || index >= MENU_BTN_MAX) return;
    g_menu.buttons[index].state = state;
}

/*
 * Handle click
 */
int menu_handle_click(int x, int y) {
    int button = g_menu.hovered_button;

    if (button < 0) {
        button = menu_get_hovered_button(x, y);
    }

    if (button < 0) return 0;

    /* Play click sound */
    /* sound_play_effect(0xd9, 0x140, 0xf0); */

    switch (button) {
        case MENU_BTN_NEW_GAME:
            menu_start_game();
            break;

        case MENU_BTN_LOAD_GAME:
            /* Load saved game - try auto save first */
            {
                int load_result = auto_save_load(NULL);
                if (load_result) {
                    LOG_INFO("Loaded saved game successfully");
                    /* Transition to game state */
                    game_set_state(GAME_STATE_FIELD_INIT);
                } else {
                    /* No save found, show message */
                    LOG_DEBUG("No saved game found");
                    g_menu.state = MENU_STATE_ERROR_INIT;
                    strncpy(g_menu.error_msg, "No saved game found", sizeof(g_menu.error_msg) - 1);
                }
            }
            break;

        case MENU_BTN_OPTION:
            menu_show_options();
            break;

        case MENU_BTN_EXIT:
            g_menu.confirm_action = 0;  /* Exit confirm */
            g_menu.state = MENU_STATE_CONFIRM_INIT;
            break;

        default:
            break;
    }

    return 1;
}

/*
 * Handle key
 */
int menu_handle_key(int key) {
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
                menu_handle_click(0, 0);
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
 * Render menu
 */
void menu_render(void) {
    int i;

    /* Render background */
    render_sprite(0x7166, 320, 240);  /* Menu background */

    /* Render buttons */
    for (i = 0; i < g_menu.button_count; i++) {
        MenuButtonData* btn = &g_menu.buttons[i];

        if (btn->state < 0) continue;

        /* Render button sprite */
        if (btn->state == 4) {
            /* Hover state */
            render_sprite(btn->sprite_normal + 1, btn->x, btn->y);
        } else {
            /* Normal state */
            render_sprite(btn->sprite_normal, btn->x, btn->y);
        }
    }

    /* Render confirm dialog if active */
    if (g_menu.state == MENU_STATE_CONFIRM) {
        /* Dialog background */
        render_fill_rect(NULL, 200, 200, 240, 160, 0x0421);
        render_fill_rect(NULL, 202, 202, 236, 156, 0x0842);

        /* Title */
        render_text(320, 220, "Confirm Exit", COLOR_WHITE);

        /* Buttons */
        render_sprite(0x716e, 250, 280);  /* Yes */
        render_sprite(0x7170, 340, 280);  /* No */
    }

    /* Render error message if active */
    if (g_menu.state == MENU_STATE_ERROR_WAIT && g_menu.error_msg[0]) {
        render_fill_rect(NULL, 150, 200, 340, 80, 0x0421);
        render_text(320, 230, g_menu.error_msg, COLOR_RED);
        render_text(320, 260, "Click to continue", COLOR_WHITE);
    }

    /* Render fade overlay */
    if (g_menu.fade_alpha > 0) {
        render_fill_rect(NULL, 0, 0, 640, 480,
            (g_menu.fade_alpha << 24) | 0x000000);
    }
}

/*
 * Start game
 */
void menu_start_game(void) {
    LOG_INFO("Starting new game...");
    g_menu.start_game = 1;

    /* Transition to character select */
    gamestate_set_state(GAME_STATE_CHAR_SELECT, 0);
}

/*
 * Show options
 */
void menu_show_options(void) {
    g_menu.state = MENU_STATE_OPTION_INIT;
}

/*
 * Exit game
 */
void menu_exit_game(void) {
    LOG_INFO("Exiting game...");
    g_menu.exit_requested = 1;
}

/*
 * Go back
 */
void menu_go_back(void) {
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
 * Check if menu is active
 */
int menu_is_active(void) {
    return g_menu.state != MENU_STATE_INIT;
}

/*
 * Check if should start game
 */
int menu_should_start_game(void) {
    return g_menu.start_game;
}

/*
 * Check if should exit
 */
int menu_should_exit(void) {
    return g_menu.exit_requested;
}
