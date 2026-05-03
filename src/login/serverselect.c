/*
 * Stone Age Client - Server Selection System Implementation
 * Reverse engineered from sa_9061.exe:
 *   FUN_004794d0 - Main server selection screen handler
 *   FUN_004792f0 - Initialize text input fields
 *   FUN_004793b0 - Detect server list item click
 *   FUN_00420ff0 - Find clicked button index
 *   FUN_0044aba0 - Rectangle click detection
 *   FUN_004011c0 - Close window
 *   FUN_00488190 - Play sound effect
 *   FUN_0041f1a0 - Render text
 *
 * Memory layout from binary:
 *   DAT_04630b50: Username buffer
 *   DAT_04630c90: Password buffer
 *   DAT_04630c78: OK button state
 *   DAT_04630c7c: Cancel button state
 *   DAT_04630c80: New Account button state
 *   DAT_04630dbc: Click sprite 1
 *   DAT_04630dc0: Click sprite 2
 *   DAT_04630dc4: Click sprite 3
 *   DAT_04630dd4: Window widget pointer
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "serverselect.h"
#include "uiwidget.h"
#include "input.h"
#include "sound.h"
#include "render.h"
#include "logger.h"

/* Global server selection context */
ServerSelectContext g_server_select = {0};

/* UI Layout constants - from DAT_004cea58 region */
#define SERVER_WINDOW_X        0x113   /* 275 */
#define SERVER_WINDOW_Y        0xe1    /* 225 */
#define SERVER_WINDOW_W        0x113
#define SERVER_WINDOW_H        0xe1
#define USERNAME_INPUT_X       0x7d    /* Offset from window */
#define USERNAME_INPUT_Y       0x2f
#define PASSWORD_INPUT_X       0x11    /* Offset from window */
#define PASSWORD_INPUT_Y       0x57
#define MAX_INPUT_LENGTH       31

/* Sprite IDs from FUN_004794d0 */
#define SPRITE_BUTTON_OK       0x66bb
#define SPRITE_BUTTON_OK_OFF   0x66ba
#define SPRITE_BUTTON_CANCEL   0x66b0
#define SPRITE_BUTTON_CANCEL_HOVER  0x66b1
#define SPRITE_BUTTON_NEWACCT  0x66ac

/* Sound IDs from FUN_00488190 */
#define SOUND_BUTTON_CLICK     0xd9
#define SOUND_ERROR            0xcb

/* External functions matching binary */
extern int render_queue_add_sprite(int x, int y, int layer, u32 sprite_id, int priority);
extern void sound_play_effect(int sound_id, int volume, int pan);
extern int text_render(int x, int y, const char* text, u32 color);

/* External mouse position from input system */
extern s32 g_mouse_x;
extern s32 g_mouse_y;
extern u32 g_mouse_pressed;

/* External render queue sprites for click detection - DAT_045f1bf0 */
extern u32 g_last_click_sprite;

/*
 * Initialize server selection system - FUN_004794d0 first part
 */
int server_select_init(void) {
    memset(&g_server_select, 0, sizeof(ServerSelectContext));

    g_server_select.state = SERVER_SELECT_INIT;
    g_server_select.selected_server = 0;
    g_server_select.btn_ok_state = BUTTON_STATE_NORMAL;
    g_server_select.btn_cancel_state = BUTTON_STATE_NORMAL;
    g_server_select.btn_newacct_state = BUTTON_STATE_NORMAL;

    /* Initialize click sprite IDs to -2 (invalid) */
    g_server_select.click_sprites[0] = 0xFFFFFFFE;
    g_server_select.click_sprites[1] = 0xFFFFFFFE;
    g_server_select.click_sprites[2] = 0xFFFFFFFE;

    /* Window position from DAT_004cea58/004cea5c */
    g_server_select.window_x = 0x113;
    g_server_select.window_y = 0xe1;

    LOG_INFO("Server selection system initialized");
    return 1;
}

/*
 * Shutdown server selection system
 */
void server_select_shutdown(void) {
    if (g_server_select.window) {
        server_select_close_window(g_server_select.window);
        g_server_select.window = NULL;
    }
    memset(&g_server_select, 0, sizeof(ServerSelectContext));
    LOG_INFO("Server selection system shutdown");
}

/*
 * Initialize text input fields - FUN_004792f0
 * Sets up username and password input field positions and lengths
 */
void server_select_init_fields(void) {
    int len;

    /* Username field - DAT_04630b50 region */
    g_server_select.username.x = g_server_select.window_x + USERNAME_INPUT_X;
    g_server_select.username.y = g_server_select.window_y + USERNAME_INPUT_Y;
    g_server_select.username.max_length = MAX_INPUT_LENGTH;
    g_server_select.username.is_password = 0;
    g_server_select.username.active = 1;
    g_server_select.username.color = 0xFFFFFFFF;

    /* Password field - DAT_04630c90 region */
    g_server_select.password.x = g_server_select.window_x + PASSWORD_INPUT_X;
    g_server_select.password.y = g_server_select.window_y + PASSWORD_INPUT_Y;
    g_server_select.password.max_length = MAX_INPUT_LENGTH;
    g_server_select.password.is_password = 1;
    g_server_select.password.active = 1;
    g_server_select.password.color = 0xFFFFFFFF;

    /* Initialize cursor positions based on existing content */
    len = strlen(g_server_select.username.buffer);
    if (len == 0) {
        g_server_select.username.cursor_pos = 0;
        g_server_select.username.length = 0;
    } else {
        g_server_select.username.length = (u8)len;
        g_server_select.username.cursor_pos = (u8)len;
    }

    len = strlen(g_server_select.password.buffer);
    if (len == 0) {
        g_server_select.password.cursor_pos = 0;
        g_server_select.password.length = 0;
    } else {
        g_server_select.password.length = (u8)len;
        g_server_select.password.cursor_pos = (u8)len;
    }

    /* Additional initialization from FUN_004792f0 */
    g_server_select.selected_server = 0;
}

/*
 * Rectangle click test - FUN_0044aba0
 * Returns 1 if mouse is within the rectangle bounds
 */
int server_select_rect_test(int x1, int y1, int x2, int y2, int sprite_type) {
    /* Check if mouse is within rectangle */
    if (g_mouse_x >= x1 && g_mouse_x <= x2) {
        if (g_mouse_y >= y1 && g_mouse_y <= y2) {
            /* Render highlight sprite if valid */
            if (sprite_type >= 0) {
                /* FUN_0047e640 pattern for fade effect */
                render_queue_add_sprite(x1, y1, sprite_type, 0, 0);
            }
            return 1;
        }
    }
    return 0;
}

/*
 * Check button click by sprite ID - FUN_00420ff0
 * Searches through sprite ID array to find which button was clicked
 */
int server_select_check_button_click(s32* button_ids, int count) {
    int i;

    /* Check if mouse is pressed - DAT_045f1bc4 bit 0 */
    if (!(g_mouse_pressed & 1)) {
        return -1;
    }

    /* Find which sprite was clicked - compare with DAT_045f1bf0 */
    for (i = 0; i < count; i++) {
        if (button_ids[i] == (s32)g_last_click_sprite) {
            return i;
        }
    }

    return -1;
}

/*
 * Detect which server list item or button was clicked - FUN_004793b0
 * Returns index of clicked item:
 *   0 = OK button
 *   1 = Cancel button
 *   2 = New Account button
 *   3 = Change Password button
 *   4 = Back button
 *   -1 = No click
 */
int server_select_detect_click(int x, int y) {
    int result;
    int clicked;

    result = -1;

    /* Render text fields - FUN_0041d890 pattern */
    /* text_render_field(&g_server_select.username); */
    /* text_render_field(&g_server_select.password); */

    /* Check OK button area - coordinates from FUN_004793b0 */
    clicked = server_select_rect_test(
        g_server_select.window_x + 0x73,   /* x1 */
        g_server_select.window_y + 0x2d,   /* y1 */
        g_server_select.window_x + 0xdf,   /* x2 */
        g_server_select.window_y + 0x40,   /* y2 */
        0x65                               /* sprite type */
    );
    if (clicked) {
        result = 0;
        g_server_select.selected_server = 0;
    }

    /* Check Cancel button area */
    clicked = server_select_rect_test(
        g_server_select.window_x + 0x10,
        g_server_select.window_y + 0x50,
        g_server_select.window_x + 0x100,
        g_server_select.window_y + 0x96,
        0x65
    );
    if (clicked) {
        result = 1;
        g_server_select.selected_server = 1;
    }

    /* Check New Account button area */
    clicked = server_select_rect_test(
        g_server_select.window_x + 0x27,
        g_server_select.window_y + 0xbe,
        g_server_select.window_x + 0x6a,
        g_server_select.window_y + 0xd6,
        0x65
    );
    if (clicked) {
        result = 2;
    }

    /* Check Change Password button area */
    clicked = server_select_rect_test(
        g_server_select.window_x + 0x9b,
        g_server_select.window_y + 0xbe,
        g_server_select.window_x + 0xde,
        g_server_select.window_y + 0xd6,
        0x65
    );
    if (clicked) {
        result = 3;
    }

    /* Check Back button area */
    clicked = server_select_rect_test(
        g_server_select.window_x + 0xa7,
        g_server_select.window_y + 0xa0,
        g_server_select.window_x + 0xfb,
        g_server_select.window_y + 0xb3,
        0x65
    );
    if (clicked) {
        result = 4;
    }

    return result;
}

/*
 * Close window - FUN_004011c0
 * Sets close flag at offset +0x24
 */
void server_select_close_window(void* window) {
    if (window) {
        /* Set close flag - *(undefined4 *)(param_1 + 0x24) = 1 */
        *(u32*)((char*)window + 0x24) = 1;
    }
}

/*
 * Play sound effect - FUN_00488190 wrapper
 */
static void server_select_play_sound(int sound_id) {
    sound_play_effect(sound_id, 0x140, 0xf0);
}

/*
 * Main update function - FUN_004794d0
 * Handles initialization, input, rendering, and state management
 * Returns 0 if should continue, 1 if OK clicked, 2 if Cancel clicked
 */
int server_select_update(void) {
    s32 click_result;
    u32 current_time;
    int username_len, password_len;
    int result = 0;

    /* Initialize on first run - DAT_04630dc8 check */
    if (g_server_select.init_count == 0) {
        /* Set click sprites to invalid */
        g_server_select.click_sprites[0] = 0xFFFFFFFE;
        g_server_select.click_sprites[1] = 0xFFFFFFFE;
        g_server_select.click_sprites[2] = 0xFFFFFFFE;

        /* Set button states to normal */
        g_server_select.btn_ok_state = BUTTON_STATE_NORMAL;
        g_server_select.btn_cancel_state = BUTTON_STATE_NORMAL;
        g_server_select.btn_newacct_state = BUTTON_STATE_NORMAL;

        /* Create window widget - FUN_00448610 */
        g_server_select.window = widget_create_window_ex(
            g_server_select.window_x,
            g_server_select.window_y,
            SERVER_WINDOW_W,
            SERVER_WINDOW_H,
            NULL,
            WINDOW_STYLE_CENTERED
        );

        /* Initialize text fields - FUN_004792f0 */
        server_select_init_fields();

        g_server_select.selected_server = 0;
        g_server_select.init_count++;
        g_server_select.click_time = 0;
    }

    /* Check if window was created */
    if (!g_server_select.window) {
        return 0;
    }

    /* Check button clicks - FUN_00420ff0 */
    click_result = server_select_check_button_click(g_server_select.click_sprites, 3);

    /* Handle click timing - DAT_04630db8 logic */
    if (g_server_select.click_time == 0) {
        if (click_result >= 0 && click_result < 3) {
            /* Set button state to clicked */
            if (click_result == 0) {
                g_server_select.btn_ok_state = BUTTON_STATE_CLICKED;
            } else if (click_result == 1) {
                g_server_select.btn_cancel_state = BUTTON_STATE_CLICKED;
            } else {
                g_server_select.btn_newacct_state = BUTTON_STATE_CLICKED;
            }

            click_result = -1;
            g_server_select.click_time = timeGetTime();
            server_select_play_sound(SOUND_BUTTON_CLICK);
        }
    } else {
        /* Check if 100ms has passed */
        current_time = timeGetTime();
        if (g_server_select.click_time + 100 < current_time) {
            g_server_select.click_time = 0;

            /* Handle New Account button state */
            if (g_server_select.btn_newacct_state == BUTTON_STATE_CLICKED) {
                /* Clear password field if shown */
                if (g_server_select.password.length > 0) {
                    memset(g_server_select.password.buffer, 0, sizeof(g_server_select.password.buffer));
                    g_server_select.password.length = 0;
                    g_server_select.password.cursor_pos = 0;
                }

                if (click_result == -1) {
                    g_server_select.btn_newacct_state = BUTTON_STATE_NORMAL;
                }
            }
            /* Handle OK button state */
            else if (g_server_select.btn_ok_state == BUTTON_STATE_CLICKED) {
                username_len = strlen(g_server_select.username.buffer);
                password_len = strlen(g_server_select.password.buffer);

                if (username_len < 1 || password_len < 1) {
                    if (click_result == -1) {
                        g_server_select.btn_ok_state = BUTTON_STATE_NORMAL;
                    }
                } else {
                    /* Valid credentials - proceed */
                    g_server_select.username.buffer[g_server_select.username.cursor_pos] = '\0';
                    g_server_select.password.buffer[g_server_select.password.cursor_pos] = '\0';
                    result = 1;  /* OK clicked */
                }
            }
            /* Handle Cancel button state */
            else if (g_server_select.btn_cancel_state == BUTTON_STATE_CLICKED) {
                if (click_result == -1) {
                    g_server_select.btn_cancel_state = BUTTON_STATE_NORMAL;
                }
            }
        }
    }

    /* Render window decoration if done flag is set - check *(widget + 0x78) */
    if (g_server_select.window) {
        Widget* widget = (Widget*)g_server_select.window;
        if (widget->done) {
            /* Render background sprite */
            render_queue_add_sprite(
                widget->x,
                widget->y,
                0x68,           /* Window decoration sprite type */
                0x6591,         /* Standard sprite base from DAT_0054b160 */
                1
            );

            /* Render OK button */
            if (g_server_select.btn_newacct_state == BUTTON_STATE_CLICKED) {
                render_queue_add_sprite(
                    g_server_select.window_x + 0xd1,
                    g_server_select.window_y + 0xaa,
                    0x69,
                    SPRITE_BUTTON_OK,
                    1
                );
            } else {
                g_server_select.click_sprites[2] = render_queue_add_sprite(
                    g_server_select.window_x + 0xd1,
                    g_server_select.window_y + 0xaa,
                    0x69,
                    SPRITE_BUTTON_OK_OFF,
                    1
                );
            }

            /* Render Cancel button */
            if (g_server_select.btn_ok_state == BUTTON_STATE_CLICKED) {
                render_queue_add_sprite(
                    g_server_select.window_x + 0x49,
                    g_server_select.window_y + 0xca,
                    0x69,
                    SPRITE_BUTTON_CANCEL,
                    1
                );
            } else {
                g_server_select.click_sprites[0] = render_queue_add_sprite(
                    g_server_select.window_x + 0x49,
                    g_server_select.window_y + 0xca,
                    0x69,
                    SPRITE_BUTTON_CANCEL_HOVER,
                    1
                );
            }

            /* Render New Account button */
            if (g_server_select.btn_cancel_state != BUTTON_STATE_CLICKED) {
                g_server_select.click_sprites[1] = render_queue_add_sprite(
                    g_server_select.window_x + 0xbd,
                    g_server_select.window_y + 0xca,
                    0x69,
                    SPRITE_BUTTON_NEWACCT,
                    1
                );
            }
        }
    }

    /* Check for network activity or errors - FUN_0044b030 pattern */
    /* If network error, play error sound */
    /* TODO: Integrate with network module */

    /* Close window on completion */
    if (result != 0) {
        server_select_close_window(g_server_select.window);
        g_server_select.window = NULL;
    }

    return result;
}

/*
 * Handle mouse click
 */
void server_select_handle_click(int x, int y) {
    int clicked;

    clicked = server_select_detect_click(x, y);

    if (clicked >= 0) {
        switch (clicked) {
            case SERVER_BTN_OK:
                /* Validate and connect */
                if (strlen(g_server_select.username.buffer) > 0 &&
                    strlen(g_server_select.password.buffer) > 0) {
                    g_server_select.btn_ok_state = BUTTON_STATE_CLICKED;
                    g_server_select.click_time = timeGetTime();
                    server_select_play_sound(SOUND_BUTTON_CLICK);
                } else {
                    server_select_play_sound(SOUND_ERROR);
                }
                break;

            case SERVER_BTN_CANCEL:
                g_server_select.btn_cancel_state = BUTTON_STATE_CLICKED;
                g_server_select.click_time = timeGetTime();
                server_select_play_sound(SOUND_BUTTON_CLICK);
                break;

            case SERVER_BTN_NEW_ACCOUNT:
                g_server_select.btn_newacct_state = BUTTON_STATE_CLICKED;
                g_server_select.click_time = timeGetTime();
                server_select_play_sound(SOUND_BUTTON_CLICK);
                break;

            default:
                break;
        }
    }
}

/*
 * Handle keyboard input
 */
void server_select_handle_key(u32 key, u32 flags) {
    TextInputField* field;

    /* Determine which field is active */
    if (g_server_select.btn_ok_state == BUTTON_STATE_CLICKED) {
        field = &g_server_select.username;
    } else {
        field = &g_server_select.password;
    }

    /* Handle key input */
    if (key == VK_BACK) {
        if (field->cursor_pos > 0) {
            field->buffer[--field->cursor_pos] = '\0';
            field->length--;
        }
    } else if (key == VK_TAB) {
        /* Toggle between username and password fields */
        if (field == &g_server_select.username) {
            g_server_select.btn_ok_state = BUTTON_STATE_NORMAL;
        } else {
            g_server_select.btn_ok_state = BUTTON_STATE_CLICKED;
        }
    } else if (key == VK_RETURN) {
        /* Submit form */
        server_select_handle_click(0, 0);
    } else if (key >= 0x20 && key <= 0x7E) {
        /* Printable character */
        if (field->cursor_pos < field->max_length) {
            field->buffer[field->cursor_pos++] = (char)key;
            field->buffer[field->cursor_pos] = '\0';
            field->length++;
        }
    }
}

/*
 * Add server to list
 */
int server_select_add_server(const char* name, const char* host, u16 port) {
    if (g_server_select.server_count >= MAX_SERVER_LIST) {
        return -1;
    }

    ServerEntry* entry = &g_server_select.servers[g_server_select.server_count];
    strncpy(entry->name, name, MAX_SERVER_NAME_LEN - 1);
    strncpy(entry->host, host, 63);
    entry->port = port;
    entry->status = 1;
    entry->selected = 0;

    return g_server_select.server_count++;
}

/*
 * Clear server list
 */
void server_select_clear_servers(void) {
    g_server_select.server_count = 0;
    memset(g_server_select.servers, 0, sizeof(g_server_select.servers));
}

/*
 * Set selected server
 */
void server_select_set_server(int index) {
    if (index >= 0 && index < g_server_select.server_count) {
        g_server_select.selected_server = index;
    }
}

/*
 * Connect to selected server
 */
void server_select_connect(void) {
    ServerEntry* server;

    if (g_server_select.selected_server >= g_server_select.server_count) {
        return;
    }

    server = &g_server_select.servers[g_server_select.selected_server];

    LOG_INFO("Connecting to server: %s (%s:%u)",
             server->name, server->host, server->port);

    /* TODO: Integrate with network module */
    g_server_select.state = SERVER_SELECT_CONNECTING;
}

/*
 * Cancel server selection
 */
void server_select_cancel(void) {
    g_server_select.state = SERVER_SELECT_DONE;
    server_select_close_window(g_server_select.window);
    g_server_select.window = NULL;
}

/*
 * New account button handler
 */
void server_select_new_account(void) {
    /* TODO: Open new account dialog */
    LOG_INFO("New account button clicked");
}
