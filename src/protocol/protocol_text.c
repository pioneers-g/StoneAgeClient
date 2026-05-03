/*
 * Stone Age Client - Text Protocol Dispatcher
 * Reverse engineered from sa_9061.exe (FUN_0043bf90)
 *
 * Protocol format: CMD|param1|param2|...\n
 * - Commands are 1-2 character codes (EV, EN, DU, EO, EB, etc.)
 * - Parameters are pipe-delimited
 * - Integers use Base-62 encoding (FUN_0043efe0)
 * - Strings use backslash escapes (FUN_0043ded0)
 *
 * Handler implementations: protocol_text_handlers.c
 */

#include <winsock2.h>
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "protocol.h"
#include "protocol_text.h"
#include "network.h"
#include "logger.h"
#include "gamestate.h"

/* Command strings are defined as macros in protocol_text.h */

/* ========================================
 * Protocol Parsing Context
 * ======================================== */

#define MAX_PACKET_ARGS    40
#define PARSE_BUFFER_SIZE  1024

static const char* s_packet_args[MAX_PACKET_ARGS];
static u32 s_packet_arg_count = 0;
static char s_decode_buffer[PARSE_BUFFER_SIZE];
static char s_decoded_strings[MAX_PACKET_ARGS][256];

/* ========================================
 * Parsing Functions
 * ======================================== */

/*
 * Parse packet arguments - FUN_0043e000
 */
static void parse_packet_args(char* line) {
    char* ptr = line;
    int in_arg = 0;
    u32 arg_index = 0;

    s_packet_arg_count = 0;
    while (*ptr == ' ') ptr++;

    while (*ptr && arg_index < MAX_PACKET_ARGS - 1) {
        if (*ptr == ' ') {
            *ptr = '\0';
            in_arg = 0;
        }
        else if (*ptr == '\n' || *ptr == '\r') {
            *ptr = '\0';
            break;
        }
        else if (!in_arg) {
            s_packet_args[arg_index++] = ptr;
            in_arg = 1;
        }
        ptr++;
    }

    s_packet_args[arg_index] = NULL;
    s_packet_arg_count = arg_index;
}

/*
 * Get integer argument - FUN_0043dd50
 */
static int get_arg_int(int index) {
    if (index < 0 || index >= (int)s_packet_arg_count) {
        return 0;
    }
    return protocol_decode_int(s_packet_args[index]);
}

/*
 * Get string argument (unescaped) - FUN_0043dd70
 */
static const char* get_arg_string(int index) {
    if (index < 0 || index >= (int)s_packet_arg_count) {
        return "";
    }

    const char* raw = s_packet_args[index];
    char* decoded = s_decoded_strings[index];
    protocol_unescape_string_to(raw, decoded, sizeof(s_decoded_strings[0]));
    return decoded;
}

/* ========================================
 * External Handler Declarations
 * Implemented in protocol_text_handlers.c
 * ======================================== */

extern void handler_xyd(int p1, int p2, int p3);
extern void handler_ev(int p1, int p2);
extern void handler_en(int p1, int p2);
extern void handler_du(const char* str);
extern void handler_eo(const char* str);
extern void handler_eb(const char* str);
extern void handler_ec(const char* str);
extern void handler_ed(int p1, int p2);
extern void handler_ee(int p1, const char* str, int p3);
extern void handler_ef(int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8, const char* str);
extern void handler_eg(const char* str);
extern void handler_eh(int p1);
extern void handler_ei(int p1, int p2);
extern void handler_ej(int p1);
extern void handler_ek(int p1);
extern void handler_el(const char* str);
extern void handler_em(const char* str);
extern void handler_en2(const char* str);
extern void handler_eo2(const char* str);
extern void handler_ep(const char* str);
extern void handler_eq(int p1, int p2, int p3, const char* str);
extern void handler_er(int p1);
extern void handler_es(int p1);
extern void handler_et(int p1, int p2);
extern void handler_eu(int p1, int p2);
extern void handler_ev2(int p1, int p2, int p3, int p4);
extern void handler_ew(int p1);
extern void handler_ex(int p1, int p2, int p3, int p4, const char* str);
extern void handler_ey(int p1, int p2, const char* str);
extern void handler_ez(int p1, int p2, int p3, int p4);
extern void handler_echo(const char* str);
extern void handler_td(const char* str);
extern void handler_fm(const char* str);
extern void handler_nu(int p1);
extern void handler_se(int p1, int p2, int p3, int p4);
extern void handler_ef2(int p1, int p2, const char* str);
extern void handler_r(const char* str);
extern void handler_client_login(const char* password);
extern void handler_create_new_char(const char* name, const char* data);
extern void handler_char_delete(const char* name, const char* data);
extern void handler_char_login(const char* name, const char* data);
extern void handler_char_list(const char* data1, const char* data2);
extern void handler_char_logout(const char* data1, const char* data2);
extern void handler_proc_get(const char* data);
extern void handler_player_num_get(int p1, int p2);

/* ========================================
 * Main Text Protocol Dispatcher - FUN_0043bf90
 * ======================================== */

void packet_dispatch_text(SOCKET sock, const char* line) {
    char* buffer;
    const char* cmd;
    size_t len;
    char first_char;

    (void)sock;

    if (!line || !line[0]) return;

    len = strlen(line);
    if (len < 1) return;

    first_char = line[0];

    /* Single-character responses */
    if (len == 1 || (line[1] != ' ' && line[1] != '|')) {
        switch (first_char) {
            case 'R':
                LOG_DEBUG("Login response received");
                return;
            case 'T':
                LOG_INFO("Login successful");
                gamestate_set_state(GAME_STATE_CHAR_SELECT, 0);
                return;
            case 'E':
                LOG_WARN("Server error: %s", line + 1);
                return;
            case 'G':
                LOG_DEBUG("Game data received");
                return;
            case 'W':
                LOG_DEBUG("World data received");
                return;
            case 'U':
                LOG_DEBUG("Update received");
                return;
            case 't':
                return;
        }
    }

    if (len < 2) return;

    if (len >= PARSE_BUFFER_SIZE) {
        len = PARSE_BUFFER_SIZE - 1;
    }
    memcpy(s_decode_buffer, line, len);
    s_decode_buffer[len] = '\0';

    parse_packet_args(s_decode_buffer);
    if (s_packet_arg_count == 0) return;

    cmd = s_packet_args[0];
    memset(s_decoded_strings, 0, sizeof(s_decoded_strings));

    /* Short command dispatch */
    if (strcmp(cmd, CMD_XYD) == 0) { handler_xyd(get_arg_int(1), get_arg_int(2), get_arg_int(3)); return; }
    if (strcmp(cmd, CMD_EV) == 0) { handler_ev(get_arg_int(1), get_arg_int(2)); return; }
    if (strcmp(cmd, CMD_EN) == 0) { handler_en(get_arg_int(1), get_arg_int(2)); return; }
    if (strcmp(cmd, CMD_DU) == 0) { handler_du(get_arg_string(1)); return; }
    if (strcmp(cmd, CMD_EO) == 0) { handler_eo(get_arg_string(1)); return; }
    if (strcmp(cmd, CMD_EB) == 0) { handler_eb(get_arg_string(1)); return; }
    if (strcmp(cmd, CMD_EC) == 0) { handler_ec(get_arg_string(1)); return; }
    if (strcmp(cmd, CMD_ED) == 0) { handler_ed(get_arg_int(1), get_arg_int(2)); return; }
    if (strcmp(cmd, CMD_EE) == 0) { handler_ee(get_arg_int(1), get_arg_string(2), get_arg_int(3)); return; }
    if (strcmp(cmd, CMD_EF) == 0) { handler_ef(get_arg_int(1), get_arg_int(2), get_arg_int(3), get_arg_int(4), get_arg_int(5), get_arg_int(6), get_arg_int(7), get_arg_int(8), get_arg_string(9)); return; }
    if (strcmp(cmd, CMD_EG) == 0) { handler_eg(get_arg_string(1)); return; }
    if (strcmp(cmd, CMD_EH) == 0) { handler_eh(get_arg_int(1)); return; }
    if (strcmp(cmd, CMD_EI) == 0) { handler_ei(get_arg_int(1), get_arg_int(2)); return; }
    if (strcmp(cmd, CMD_EJ) == 0) { handler_ej(get_arg_int(1)); return; }
    if (strcmp(cmd, CMD_EK) == 0) { handler_ek(get_arg_int(1)); return; }
    if (strcmp(cmd, CMD_EL) == 0) { handler_el(get_arg_string(1)); return; }
    if (strcmp(cmd, CMD_EM) == 0) { handler_em(get_arg_string(1)); return; }
    if (strcmp(cmd, CMD_EN2) == 0) { handler_en2(get_arg_string(1)); return; }
    if (strcmp(cmd, CMD_EO2) == 0) { handler_eo2(get_arg_string(1)); return; }
    if (strcmp(cmd, CMD_EP) == 0) { handler_ep(get_arg_string(1)); return; }
    if (strcmp(cmd, CMD_EQ) == 0) { handler_eq(get_arg_int(1), get_arg_int(2), get_arg_int(3), get_arg_string(4)); return; }
    if (strcmp(cmd, CMD_ER) == 0) { handler_er(get_arg_int(1)); return; }
    if (strcmp(cmd, CMD_ES) == 0) { handler_es(get_arg_int(1)); return; }
    if (strcmp(cmd, CMD_ET) == 0) { handler_et(get_arg_int(1), get_arg_int(2)); return; }
    if (strcmp(cmd, CMD_EU) == 0) { handler_eu(get_arg_int(1), get_arg_int(2)); return; }
    if (strcmp(cmd, CMD_EV2) == 0) { handler_ev2(get_arg_int(1), get_arg_int(2), get_arg_int(3), get_arg_int(4)); return; }
    if (strcmp(cmd, CMD_EW) == 0) { handler_ew(get_arg_int(1)); return; }
    if (strcmp(cmd, CMD_EX) == 0) { handler_ex(get_arg_int(1), get_arg_int(2), get_arg_int(3), get_arg_int(4), get_arg_string(5)); return; }
    if (strcmp(cmd, CMD_EY) == 0) { handler_ey(get_arg_int(1), get_arg_int(2), get_arg_string(3)); return; }
    if (strcmp(cmd, CMD_EZ) == 0) { handler_ez(get_arg_int(1), get_arg_int(2), get_arg_int(3), get_arg_int(4)); return; }
    if (strcmp(cmd, CMD_Echo) == 0) { handler_echo(get_arg_string(1)); return; }
    if (strcmp(cmd, CMD_TD) == 0) { handler_td(get_arg_string(1)); return; }
    if (strcmp(cmd, CMD_FM) == 0) { handler_fm(get_arg_string(1)); return; }
    if (strcmp(cmd, CMD_NU) == 0) { handler_nu(get_arg_int(1)); return; }
    if (strcmp(cmd, CMD_SE) == 0) { handler_se(get_arg_int(1), get_arg_int(2), get_arg_int(3), get_arg_int(4)); return; }
    if (strcmp(cmd, CMD_EF2) == 0) { handler_ef2(get_arg_int(1), get_arg_int(2), get_arg_string(3)); return; }
    if (strcmp(cmd, CMD_R) == 0) { handler_r(get_arg_string(1)); return; }

    /* Long command dispatch */
    if (strcmp(cmd, CMD_ClientLogin) == 0) { handler_client_login(get_arg_string(1)); return; }
    if (strcmp(cmd, CMD_CreateNewChar) == 0) { handler_create_new_char(get_arg_string(1), get_arg_string(2)); return; }
    if (strcmp(cmd, CMD_CharDelete) == 0) { handler_char_delete(get_arg_string(1), get_arg_string(2)); return; }
    if (strcmp(cmd, CMD_CharLogin) == 0) { handler_char_login(get_arg_string(1), get_arg_string(2)); return; }
    if (strcmp(cmd, CMD_CharList) == 0) { handler_char_list(get_arg_string(1), get_arg_string(2)); return; }
    if (strcmp(cmd, CMD_CharLogout) == 0) { handler_char_logout(get_arg_string(1), get_arg_string(2)); return; }
    if (strcmp(cmd, CMD_ProcGet) == 0) { handler_proc_get(get_arg_string(1)); return; }
    if (strcmp(cmd, CMD_PlayerNumGet) == 0) { handler_player_num_get(get_arg_int(1), get_arg_int(2)); return; }

    LOG_DEBUG("Unknown command: %s", cmd);
}
