/*
 * Stone Age Client - Login Core Module
 * Reverse engineered from sa_9061.exe (FUN_00420590, FUN_0045f4d0)
 *
 * Core login operations: initialization, connection, state management
 */

#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "login.h"
#include "login_crypto.h"
#include "network.h"
#include "logger.h"
#include "commandline.h"

/* Global login context */
LoginContext g_login = {0};

/*
 * Initialize login system
 */
int login_init(void) {
    memset(&g_login, 0, sizeof(LoginContext));

    g_login.state = LOGIN_STATE_DISCONNECTED;
    g_login.conn_state = LOGIN_CONN_STATE_NONE;
    g_login.screen_state = LOGIN_SCREEN_INIT;
    g_login.server_port = 4567;
    g_login.selected_char_index = -1;
    g_login.protocol_mode = PROTOCOL_MODE_TEXT;
    g_login.input_focus = 0;

    /* Initialize text input boxes */
    login_init_text_input(&g_login.username_box, 300, 170, 31, 0, 0xFFFFFFFF);
    login_init_text_input(&g_login.password_box, 300, 200, 31, 1, 0xFFFFFFFF);

    /* Auto-fill credentials from command line */
    if (g_cmdline.user_id[0]) {
        strncpy(g_login.username, g_cmdline.user_id, MAX_USERNAME - 1);
        strncpy(g_login.username_box.buffer, g_cmdline.user_id, 255);
        g_login.username_cursor = (u32)strlen(g_cmdline.user_id);
        g_login.username_box.cursor_index = (u8)g_login.username_cursor;
    }

    if (g_cmdline.password[0]) {
        strncpy(g_login.password, g_cmdline.password, MAX_PASSWORD - 1);
        strncpy(g_login.password_box.buffer, g_cmdline.password, 255);
        g_login.password_cursor = (u32)strlen(g_cmdline.password);
        g_login.password_box.cursor_index = (u8)g_login.password_cursor;
    }

    LOG_INFO("Login system initialized");
    return 1;
}

/*
 * Shutdown login system
 */
void login_shutdown(void) {
    login_disconnect();
    memset(&g_login, 0, sizeof(LoginContext));
    LOG_INFO("Login system shutdown");
}

/*
 * Connect to login server - FUN_0045f4d0 pattern
 */
int login_connect(const char* host, u16 port) {
    struct hostent* he;
    struct sockaddr_in addr;
    u_long mode = 1;
    int result;

    if (!host || port == 0) {
        LOG_ERROR("Invalid server address");
        return LOGIN_ERR_CONNECTION_FAILED;
    }

    /* Store server info */
    strncpy(g_login.server_host, host, MAX_SERVER - 1);
    g_login.server_port = port;

    /* Create socket */
    g_login.sock = socket(AF_INET, SOCK_STREAM, 0);
    if (g_login.sock == INVALID_SOCKET) {
        LOG_ERROR("Failed to create socket");
        return LOGIN_ERR_SOCKET_ERROR;
    }

    /* Set non-blocking mode */
    ioctlsocket(g_login.sock, FIONBIO, &mode);

    /* Resolve host */
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host);

    if (addr.sin_addr.s_addr == INADDR_NONE) {
        he = gethostbyname(host);
        if (!he) {
            LOG_ERROR("Failed to resolve host: %s", host);
            closesocket(g_login.sock);
            g_login.sock = INVALID_SOCKET;
            return LOGIN_ERR_DNS_FAILED;
        }
        addr.sin_addr.s_addr = *(u_long*)he->h_addr_list[0];
    }

    /* Store resolved IP */
    g_login.server_ip = addr.sin_addr.s_addr;
    g_login.connect_port = port;

    /* Start connection */
    result = connect(g_login.sock, (struct sockaddr*)&addr, sizeof(addr));
    if (result == SOCKET_ERROR) {
        result = WSAGetLastError();
        if (result != WSAEWOULDBLOCK) {
            LOG_ERROR("Connection failed: %d", result);
            closesocket(g_login.sock);
            g_login.sock = INVALID_SOCKET;
            return LOGIN_ERR_CONNECTION_FAILED;
        }
    }

    g_login.conn_state = LOGIN_CONN_STATE_CONNECTING;
    g_login.start_time = timeGetTime();
    g_login.state = LOGIN_STATE_CONNECTING;

    LOG_INFO("Connecting to %s:%d", host, port);
    return 0;
}

/*
 * Disconnect from server
 */
void login_disconnect(void) {
    if (g_login.sock != INVALID_SOCKET) {
        closesocket(g_login.sock);
        g_login.sock = INVALID_SOCKET;
    }

    g_login.conn_state = LOGIN_CONN_STATE_NONE;
    g_login.state = LOGIN_STATE_DISCONNECTED;
    g_login.logged_in = 0;
    g_login.in_game = 0;

    LOG_INFO("Disconnected from server");
}

/*
 * Check if connected
 */
int login_is_connected(void) {
    return g_login.sock != INVALID_SOCKET && g_login.conn_state == LOGIN_CONN_STATE_CONNECTED;
}

/*
 * Process connection state machine - FUN_0045f4d0 pattern
 */
int login_process_connect(void) {
    fd_set read_fds, write_fds, error_fds;
    struct timeval timeout = {0, 0};
    int result;
    u32 elapsed;

    if (g_login.conn_state == LOGIN_CONN_STATE_NONE) {
        return 0;
    }

    /* Check timeout */
    elapsed = timeGetTime() - g_login.start_time;
    if (elapsed > LOGIN_TIMEOUT) {
        LOG_ERROR("Connection timeout");
        login_disconnect();
        return LOGIN_ERR_TIMEOUT;
    }

    switch (g_login.conn_state) {
        case LOGIN_CONN_STATE_CONNECTING:
            /* Wait for connection using select */
            FD_ZERO(&read_fds);
            FD_ZERO(&write_fds);
            FD_ZERO(&error_fds);
            FD_SET(g_login.sock, &read_fds);
            FD_SET(g_login.sock, &write_fds);
            FD_SET(g_login.sock, &error_fds);

            result = select(0, &read_fds, &write_fds, &error_fds, &timeout);
            if (result > 0) {
                if (FD_ISSET(g_login.sock, &error_fds)) {
                    LOG_ERROR("Connection error");
                    login_disconnect();
                    return LOGIN_ERR_CONNECTION_FAILED;
                }

                if (FD_ISSET(g_login.sock, &read_fds) || FD_ISSET(g_login.sock, &write_fds)) {
                    /* Check for server response */
                    result = recv(g_login.sock, (char*)&g_login.first_byte, 1, MSG_PEEK);
                    if (result > 0) {
                        if (g_login.first_byte == SERVER_RESPONSE_ACCEPT) {
                            g_login.protocol_mode = PROTOCOL_MODE_BINARY;
                            g_login.conn_state = LOGIN_CONN_STATE_SEND_LOGIN;
                            LOG_DEBUG("Server accepted connection");
                        } else if (g_login.first_byte == SERVER_RESPONSE_ERROR) {
                            LOG_ERROR("Server rejected connection");
                            login_disconnect();
                            return LOGIN_ERR_CONNECTION_FAILED;
                        }
                    }
                }
            }
            break;

        case LOGIN_CONN_STATE_SEND_LOGIN:
            /* Send encrypted credentials */
            if (!g_login.login_sent) {
                login_auth(g_login.username, g_login.password);
                g_login.login_sent = 1;
                g_login.conn_state = LOGIN_CONN_STATE_WAIT_RESPONSE;
            }
            break;

        case LOGIN_CONN_STATE_WAIT_RESPONSE:
            /* Wait for login response */
            result = login_check_connection();
            if (result == 1) {
                g_login.conn_state = LOGIN_CONN_STATE_CONNECTED;
                g_login.state = LOGIN_STATE_SERVER_LIST;
                return 1;
            } else if (result < 0) {
                return result;
            }
            break;

        case LOGIN_CONN_STATE_CONNECTED:
            return 1;

        default:
            break;
    }

    return 0;
}

/*
 * Main update function
 */
void login_update(void) {
    if (g_login.state == LOGIN_STATE_CONNECTING) {
        login_process_connect();
    }
}

/*
 * Handle incoming packet
 */
void login_handle_packet(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 packet_type;

    if (!data || size == 0) return;

    packet_type = *ptr++;

    switch (packet_type) {
        case 0x01:  /* Server list */
            login_handle_servers(ptr, size - 1);
            break;
        case 0x02:  /* Character list */
            login_handle_characters(ptr, size - 1);
            break;
        case 0x03:  /* Login result */
            login_handle_result(ptr, size - 1);
            break;
        case 0x04:  /* Create result */
            login_handle_create_result(ptr, size - 1);
            break;
        case 0x05:  /* Delete result */
            login_handle_delete_result(ptr, size - 1);
            break;
        case 0x06:  /* Enter game */
            login_handle_enter_game(ptr, size - 1);
            break;
        default:
            LOG_DEBUG("Unknown packet type: 0x%02X", packet_type);
            break;
    }
}

/*
 * Get error message
 */
const char* login_get_error_message(void) {
    return g_login.error_message;
}

/*
 * Check if in game
 */
int login_is_in_game(void) {
    return g_login.in_game;
}

/*
 * Get current login state
 */
LoginState login_get_state(void) {
    return g_login.state;
}
