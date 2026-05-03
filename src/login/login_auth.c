/*
 * Stone Age Client - Login Authentication Module
 * Reverse engineered from sa_9061.exe (FUN_0043bb10)
 *
 * Authentication and server selection
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "login.h"
#include "login_crypto.h"
#include "network.h"
#include "logger.h"

/* External login context */
extern LoginContext g_login;

/*
 * Send authentication request - FUN_0043bb10 pattern
 */
int login_auth(const char* username, const char* password) {
    char encrypted_user[32];
    char encrypted_pass[32];
    char packet[256];

    if (!username || !password) {
        return 0;
    }

    if (g_login.state != LOGIN_STATE_CONNECTING &&
        g_login.state != LOGIN_STATE_AUTH) {
        LOG_WARN("Invalid state for authentication");
        return 0;
    }

    /* Store credentials */
    strncpy(g_login.username, username, MAX_USERNAME - 1);
    strncpy(g_login.password, password, MAX_PASSWORD - 1);

    /* Encrypt credentials using DES */
    memset(encrypted_user, 0, sizeof(encrypted_user));
    memset(encrypted_pass, 0, sizeof(encrypted_pass));

    strncpy(encrypted_user, username, 31);
    strncpy(encrypted_pass, password, 31);

    /* Apply DES encryption with key "f;encor1c" */
    des_encrypt("f;encor1c", encrypted_user, 32, 1);
    des_encrypt("f;encor1c", encrypted_pass, 32, 1);

    /* Store encrypted versions */
    memcpy(g_login.encrypted_user, encrypted_user, 32);
    memcpy(g_login.encrypted_pass, encrypted_pass, 32);

    /* Build and send ClientLogin packet */
    _snprintf(packet, sizeof(packet), "ClientLogin|%s|%s\n",
              encrypted_user, encrypted_pass);
    network_send(packet, strlen(packet));

    g_login.state = LOGIN_STATE_AUTH;
    LOG_DEBUG("Sent authentication for user: %s", username);

    return 1;
}

/*
 * Auto login with saved credentials
 */
int login_auto_auth(void) {
    if (g_login.remember_pass && g_login.username[0] && g_login.password[0]) {
        return login_auth(g_login.username, g_login.password);
    }
    return 0;
}

/*
 * Handle authentication result
 */
void login_handle_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 response;

    if (size < 1) return;

    response = *ptr++;

    switch (response) {
        case SERVER_RESPONSE_ACCEPT:
            /* Connection accepted, wait for login result */
            g_login.protocol_mode = PROTOCOL_MODE_BINARY;
            LOG_DEBUG("Server accepted connection");
            break;

        case SERVER_RESPONSE_LOGIN_OK:
            /* Full login success */
            g_login.result = LOGIN_SUCCESS;
            g_login.logged_in = 1;
            g_login.state = LOGIN_STATE_SERVER_LIST;

            /* Parse session key if present */
            if (size > 1) {
                strncpy(g_login.session_key, (char*)ptr,
                        sizeof(g_login.session_key) - 1);
            }

            LOG_INFO("Login successful for user: %s", g_login.username);

            /* Request server list */
            login_request_servers();
            break;

        case SERVER_RESPONSE_ERROR:
            /* Error response */
            if (size > 1) {
                u8 error_code = *ptr;
                switch (error_code) {
                    case 0x01:
                        g_login.result = LOGIN_ERR_INVALID_USER;
                        strcpy(g_login.error_message, "Invalid username");
                        break;
                    case 0x02:
                        g_login.result = LOGIN_ERR_INVALID_PASS;
                        strcpy(g_login.error_message, "Invalid password");
                        break;
                    case 0x03:
                        g_login.result = LOGIN_ERR_ALREADY_LOGGED;
                        strcpy(g_login.error_message, "Already logged in");
                        break;
                    case 0x04:
                        g_login.result = LOGIN_ERR_SERVER_DOWN;
                        strcpy(g_login.error_message, "Server maintenance");
                        break;
                    case 0x05:
                        g_login.result = LOGIN_ERR_VERSION_MISMATCH;
                        strcpy(g_login.error_message, "Version mismatch");
                        break;
                    case 0x06:
                        g_login.result = LOGIN_ERR_BANNED;
                        strcpy(g_login.error_message, "Account banned");
                        break;
                    default:
                        g_login.result = LOGIN_ERR_CONNECTION_FAILED;
                        strcpy(g_login.error_message, "Unknown error");
                        break;
                }
            }
            g_login.state = LOGIN_STATE_ERROR;
            LOG_WARN("Login failed: %s", g_login.error_message);
            break;

        default:
            LOG_WARN("Unknown login response: 0x%02X", response);
            break;
    }
}

/*
 * Request server list
 */
void login_request_servers(void) {
    char packet[] = "ProcGet\n";
    network_send(packet, strlen(packet));
    g_login.state = LOGIN_STATE_SERVER_LIST;
    LOG_DEBUG("Requesting server list");
}

/*
 * Handle server list response
 */
void login_handle_servers(void* data, u32 size) {
    u8* ptr = (u8*)data;
    int i;
    ServerInfo* server;

    g_login.server_count = 0;

    for (i = 0; i < MAX_SERVERS && ptr < (u8*)data + size; i++) {
        server = &g_login.servers[i];

        /* Parse server entry: id|name|host|port|population|status */
        server->id = *(u32*)ptr; ptr += 4;
        strncpy(server->name, (char*)ptr, 31); ptr += 32;
        strncpy(server->host, (char*)ptr, 63); ptr += 64;
        server->port = *(u16*)ptr; ptr += 2;
        server->population = *(u16*)ptr; ptr += 2;
        server->status = *ptr++;

        g_login.server_count++;
    }

    LOG_DEBUG("Received %d servers", g_login.server_count);

    /* Auto-select first server if only one */
    if (g_login.server_count == 1) {
        login_select_server(0);
    }
}

/*
 * Select server
 */
int login_select_server(int index) {
    ServerInfo* server;

    if (index < 0 || index >= g_login.server_count) {
        return 0;
    }

    server = &g_login.servers[index];
    g_login.selected_server_index = index;

    /* Request character list for this server */
    login_request_characters();

    g_login.state = LOGIN_STATE_CHAR_LIST;
    LOG_INFO("Selected server: %s", server->name);

    return 1;
}

/*
 * Get server by index
 */
ServerInfo* login_get_server(int index) {
    if (index < 0 || index >= g_login.server_count) {
        return NULL;
    }
    return &g_login.servers[index];
}

/*
 * Get server count
 */
int login_get_server_count(void) {
    return g_login.server_count;
}
