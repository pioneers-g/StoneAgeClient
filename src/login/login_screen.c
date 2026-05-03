/*
 * Stone Age Client - Login Screen Module
 * Reverse engineered from sa_9061.exe (FUN_00420590)
 *
 * Login screen state machine and UI handling
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "login.h"
#include "login_crypto.h"
#include "network.h"
#include "render.h"
#include "input.h"
#include "logger.h"

/* External login context */
extern LoginContext g_login;

/* Forward declarations */
extern int login_process_connect(void);

/*
 * Initialize login screen
 */
void login_screen_init(void) {
    g_login.screen_state = LOGIN_SCREEN_INIT;
    g_login.first_render = 0;
    g_login.input_focus = 0;

    LOG_DEBUG("Login screen initialized");
}

/*
 * Login screen state machine - FUN_00420590 pattern
 */
void login_screen_update(void) {
    int result;
    u32 elapsed;

    switch (g_login.screen_state) {
        case LOGIN_SCREEN_INIT:
            /* State 0: Cleanup and init UI */
            if (g_login.sock != INVALID_SOCKET) {
                closesocket(g_login.sock);
                g_login.sock = INVALID_SOCKET;
            }

            g_login.first_render = 0;
            g_login.screen_state = LOGIN_SCREEN_RENDER;
            break;

        case LOGIN_SCREEN_RENDER:
            /* State 1: Render login screen */
            if (!g_login.first_render) {
                g_login.first_render = 1;

                /* Encrypt credentials for display/check */
                if (g_login.username[0] && g_login.password[0]) {
                    /* Pre-encrypt for quick send */
                    memcpy(g_login.encrypted_user, g_login.username, 32);
                    memcpy(g_login.encrypted_pass, g_login.password, 32);
                    des_encrypt("f;encor1c", g_login.encrypted_user, 32, 1);
                    des_encrypt("f;encor1c", g_login.encrypted_pass, 32, 1);

                    g_login.username_cursor = (u32)strlen(g_login.username);
                    g_login.password_cursor = (u32)strlen(g_login.password);
                }
            }

            /* Check for input or button click */
            result = login_screen_handle_input();
            if (result == 1) {
                /* Login clicked */
                g_login.screen_state = LOGIN_SCREEN_CONNECTING;
            } else if (result == 2) {
                /* Cancel clicked */
                PostQuitMessage(0);
            }
            break;

        case LOGIN_SCREEN_CONNECTING:
            /* State 3: Create connection widget */
            if (g_login.login_window == 0) {
                /* Create connecting dialog */
                g_login.login_window = 1; /* TODO: Create actual window widget */
            }

            /* Start connection if not already */
            if (g_login.state == LOGIN_STATE_DISCONNECTED) {
                result = login_connect(g_login.server_host, g_login.server_port);
                if (result < 0) {
                    g_login.screen_state = LOGIN_SCREEN_ERROR;
                    break;
                }
            }

            g_login.screen_state = LOGIN_SCREEN_WAITING;
            break;

        case LOGIN_SCREEN_WAITING:
            /* State 4: Wait for server response */
            result = login_process_connect();

            if (result == 1) {
                /* Connection successful */
                if (g_login.login_window) {
                    /* Close connecting dialog */
                    g_login.login_window = 0;
                }

                /* Decrypt and clear credentials */
                des_encrypt("f;encor1c", g_login.encrypted_user, 32, 0);
                des_encrypt("f;encor1c", g_login.encrypted_pass, 32, 0);
                memset(g_login.encrypted_user, 0, sizeof(g_login.encrypted_user));
                memset(g_login.encrypted_pass, 0, sizeof(g_login.encrypted_pass));

                /* Move to game state */
                g_login.screen_state = LOGIN_SCREEN_RENDER;
                /* TODO: Call game state change function */
            } else if (result == -7) {
                /* Connection failed - error */
                strcpy(g_login.error_message, "Connection failed");
                g_login.screen_state = LOGIN_SCREEN_ERROR;
            } else if (result == -8) {
                /* Timeout */
                strcpy(g_login.error_message, "Connection timeout");
                g_login.screen_state = LOGIN_SCREEN_ERROR;
            }

            /* Check for overall timeout */
            elapsed = timeGetTime() - g_login.start_time;
            if (elapsed > LOGIN_TIMEOUT) {
                strcpy(g_login.error_message, "Login timeout");
                g_login.screen_state = LOGIN_SCREEN_ERROR;
            }
            break;

        case LOGIN_SCREEN_ERROR:
            /* State 100: Error display */
            result = login_screen_handle_input();
            if (result == 1) {
                /* Retry clicked */
                g_login.screen_state = LOGIN_SCREEN_INIT;
                Sleep(1000);
            } else if (result == 2) {
                /* Cancel clicked */
                login_disconnect();
                g_login.screen_state = LOGIN_SCREEN_RENDER;
            }
            break;

        default:
            break;
    }
}

/*
 * Initialize text input box - FUN_00420fb0 pattern
 */
void login_init_text_input(TextInputBox* box, int x, int y, int max_len, int is_password, int color) {
    if (!box) return;

    memset(box, 0, sizeof(TextInputBox));
    box->field_10c = x;
    box->field_110 = y;
    box->max_len = (u8)max_len;
    box->field_120 = is_password;
    box->field_11c = color;
    box->char_width = 8;
    box->cursor_index = 0;
    box->cursor_pos = x;
    box->cursor_y = y;
}

/*
 * Update text input box
 */
void login_update_text_input(TextInputBox* box) {
    int key;
    char ch;
    int len;

    if (!box) return;

    len = (int)strlen(box->buffer);

    /* Process keyboard input */
    while ((key = input_get_key()) != 0) {
        if (key == VK_BACK) {
            /* Backspace */
            if (box->cursor_index > 0) {
                memmove(&box->buffer[box->cursor_index - 1],
                        &box->buffer[box->cursor_index],
                        len - box->cursor_index + 1);
                box->cursor_index--;
                len--;
            }
        } else if (key == VK_DELETE) {
            /* Delete */
            if (box->cursor_index < len) {
                memmove(&box->buffer[box->cursor_index],
                        &box->buffer[box->cursor_index + 1],
                        len - box->cursor_index);
                len--;
            }
        } else if (key == VK_LEFT) {
            if (box->cursor_index > 0) {
                box->cursor_index--;
            }
        } else if (key == VK_RIGHT) {
            if (box->cursor_index < len) {
                box->cursor_index++;
            }
        } else if (key == VK_HOME) {
            box->cursor_index = 0;
        } else if (key == VK_END) {
            box->cursor_index = (u8)len;
        } else if (key == VK_RETURN) {
            /* Enter key - move to next field or submit */
            /* Handled by caller */
        } else if ((ch = input_get_char()) != 0) {
            /* Regular character input */
            if (len < box->max_len && box->cursor_index < box->max_len) {
                if (box->cursor_index < len) {
                    memmove(&box->buffer[box->cursor_index + 1],
                            &box->buffer[box->cursor_index],
                            len - box->cursor_index + 1);
                }
                box->buffer[box->cursor_index] = ch;
                box->cursor_index++;
                box->buffer[len + 1] = '\0';
            }
        }

        /* Update cursor position */
        box->cursor_pos = box->field_10c + (u32)(box->cursor_index * box->char_width);
    }
}

/*
 * Render text input box
 */
void login_render_text_input(TextInputBox* box) {
    if (!box) return;

    /* Render background box */
    /* TODO: Implement actual rendering */

    /* Render text or password mask */
    if (box->field_120) {
        /* Password - render as *** */
        char masked[256];
        int len = (int)strlen(box->buffer);
        int i;

        for (i = 0; i < len && i < box->max_len; i++) {
            masked[i] = '*';
        }
        masked[len] = '\0';

        /* Render masked text */
        render_text(box->field_10c, box->field_110, masked, box->field_11c);
    } else {
        /* Regular text */
        render_text(box->field_10c, box->field_110, box->buffer, box->field_11c);
    }

    /* Render cursor */
    if (box->cursor_index <= strlen(box->buffer)) {
        u32 cursor_x = box->field_10c + box->cursor_index * box->char_width;
        /* TODO: Render cursor line */
        (void)cursor_x;
    }
}

/*
 * Handle login screen input
 */
int login_screen_handle_input(void) {
    int mouse_x, mouse_y;
    int click;

    /* Get mouse state */
    input_get_mouse_pos(&mouse_x, &mouse_y);
    click = input_mouse_button_pressed(0);

    /* Update focused text input */
    if (g_login.input_focus == 0) {
        login_update_text_input(&g_login.username_box);
        strncpy(g_login.username, g_login.username_box.buffer, MAX_USERNAME - 1);
        g_login.username_cursor = g_login.username_box.cursor_index;
    } else {
        login_update_text_input(&g_login.password_box);
        strncpy(g_login.password, g_login.password_box.buffer, MAX_PASSWORD - 1);
        g_login.password_cursor = g_login.password_box.cursor_index;
    }

    /* Check for tab to switch focus */
    if (input_key_pressed(VK_TAB)) {
        g_login.input_focus = 1 - g_login.input_focus;
    }

    /* Check for enter key */
    if (input_key_pressed(VK_RETURN)) {
        if (g_login.username[0] && g_login.password[0]) {
            return 1; /* Login */
        }
    }

    /* Check button clicks */
    if (click) {
        /* Login button area (approximate) */
        if (mouse_x >= 300 && mouse_x <= 400 &&
            mouse_y >= 240 && mouse_y <= 270) {
            if (g_login.username[0] && g_login.password[0]) {
                return 1; /* Login clicked */
            }
        }

        /* Cancel button area */
        if (mouse_x >= 420 && mouse_x <= 520 &&
            mouse_y >= 240 && mouse_y <= 270) {
            return 2; /* Cancel clicked */
        }

        /* Username field click */
        if (mouse_x >= 300 && mouse_x <= 550 &&
            mouse_y >= 170 && mouse_y <= 195) {
            g_login.input_focus = 0;
        }

        /* Password field click */
        if (mouse_x >= 300 && mouse_x <= 550 &&
            mouse_y >= 200 && mouse_y <= 225) {
            g_login.input_focus = 1;
        }
    }

    return 0;
}

/*
 * Check connection status - FUN_0045ef60 pattern
 */
int login_check_connection(void) {
    fd_set read_fds;
    struct timeval timeout = {0, 0};
    int result;
    char buffer[64];

    if (g_login.sock == INVALID_SOCKET) {
        return -1;
    }

    FD_ZERO(&read_fds);
    FD_SET(g_login.sock, &read_fds);

    result = select(0, &read_fds, NULL, NULL, &timeout);
    if (result > 0) {
        /* Data available */
        result = recv(g_login.sock, buffer, sizeof(buffer), 0);
        if (result > 0) {
            /* Check response */
            if (buffer[0] == SERVER_RESPONSE_ACCEPT ||
                buffer[0] == SERVER_RESPONSE_LOGIN_OK) {
                return 1; /* Success */
            } else if (buffer[0] == SERVER_RESPONSE_ERROR) {
                return -7; /* Error */
            }
        } else if (result == 0) {
            return -8; /* Connection closed */
        } else {
            return -1; /* Error */
        }
    }

    return 0; /* Still waiting */
}

/*
 * Render login screen
 */
void login_screen_render(void) {
    /* Render background */
    /* TODO: Implement actual rendering */

    /* Render server name */
    if (g_login.server_name[0]) {
        char display[64];
        _snprintf(display, sizeof(display), "%s - %d",
                  g_login.server_name, g_login.server_player_count);
        render_text(100, 100, display, 0xFFFFFFFF);
    }

    /* Render username label and input */
    render_text(200, 175, "Username:", 0xFFFFFFFF);
    login_render_text_input(&g_login.username_box);

    /* Render password label and input */
    render_text(200, 205, "Password:", 0xFFFFFFFF);
    login_render_text_input(&g_login.password_box);

    /* Render buttons */
    render_text(330, 250, "Login", 0xFFFFFFFF);
    render_text(450, 250, "Cancel", 0xFFFFFFFF);

    /* Render error message if present */
    if (g_login.screen_state == LOGIN_SCREEN_ERROR && g_login.error_message[0]) {
        render_text(200, 300, g_login.error_message, 0xFFFF0000);
    }

    /* Render connecting status */
    if (g_login.screen_state == LOGIN_SCREEN_CONNECTING ||
        g_login.screen_state == LOGIN_SCREEN_WAITING) {
        render_text(300, 300, "Connecting...", 0xFFFFFFFF);
    }
}
