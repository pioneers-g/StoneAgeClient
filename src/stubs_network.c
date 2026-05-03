/*
 * Stone Age Client - Network Stub Functions
 * Split from stubs.c to reduce file size
 */

#include <windows.h>
#include <string.h>
#include "types.h"

/* External globals */
extern u32 DAT_0461b408;
extern void* DAT_0461b41c;
extern u32 DAT_0461b3fc;
extern void* gSocket;

/* Protocol parsing functions - defined in protocol_util.c but stubs for linking */
int parse_text_field(const char* str, char delimiter, int field_num, int max_size, char* output) {
    (void)str; (void)delimiter; (void)field_num; (void)max_size;
    if (output) output[0] = '\0';
    return 1;
}

int parse_field_int(const char* str, char delimiter, int field_num) {
    (void)str; (void)delimiter; (void)field_num;
    return -1;
}

/* Network action response stubs */
void network_send_action_complete(int action_type) {
    (void)action_type;
}

void network_send_action_response(int result) {
    (void)result;
}

/* Network send stubs */
void network_send_pvp_battle_end(void) {}
void network_send_battle_end(void) {}
void network_send_pvp_ride_request(void) {}
void network_send_ride_request(void) {}
void network_send_pvp_special_command(int cmd) { (void)cmd; }
void network_send_special_command(int cmd) { (void)cmd; }

/* Protocol encoding/decoding stubs */
int protocol_decode_int(const char* str) {
    (void)str;
    return 0;
}

char* protocol_unescape_string(const char* str) {
    (void)str;
    return NULL;
}

void protocol_unescape_string_to(const char* src, char* dst, int max_len) {
    (void)src; (void)max_len;
    if (dst) dst[0] = '\0';
}

int protocol_parse_field(const char* str, int field_num, char* output, int max_len) {
    (void)str; (void)field_num; (void)max_len;
    if (output) output[0] = '\0';
    return 0;
}

/* NPC protocol stubs */
void npc_send_ack_binary_impl(int param_1, int param_2) {
    (void)param_1; (void)param_2;
}

/*
 * FUN_0045ec80 - Receive Buffer Append
 *
 * Binary analysis:
 * - Appends data to receive buffer
 * - param_1: data pointer
 * - param_2: data length
 * - Returns 0 on success, negative on error
 * - Buffer max size is 0x4000 (16KB)
 */
int FUN_0045ec80(void* param_1, unsigned int param_2) {
    if (!DAT_0461b3fc) return -100;  /* Buffer not initialized */
    if (DAT_0461b408 + param_2 > 0x4000) return -1;  /* Overflow */

    memcpy((char*)DAT_0461b41c + DAT_0461b408, param_1, param_2);
    DAT_0461b408 += param_2;
    return 0;
}

/*
 * FUN_0045ee40 - Line Extraction from Buffer
 *
 * Binary analysis:
 * - Extracts a line from receive buffer
 * - Looks for '\n' delimiter
 * - Returns line length or 0 if no complete line
 */
int FUN_0045ee40(char* output, int max_size) {
    char* buf = (char*)DAT_0461b41c;
    int pos = DAT_0461b408;
    int i;

    if (!buf || !output || max_size <= 0) return -1;

    for (i = 0; i < pos && i < max_size - 1; i++) {
        if (buf[i] == '\n') {
            memcpy(output, buf, i);
            output[i] = '\0';
            /* Shift remaining data */
            memmove(buf, buf + i + 1, pos - i - 1);
            DAT_0461b408 = pos - i - 1;
            return i;
        }
    }
    return 0;  /* No complete line */
}

/* Forward declaration */
int FUN_0045ede0(int param_1);

/*
 * FUN_0045e880 - Main Network I/O Loop
 *
 * Binary analysis:
 * - Uses select() for non-blocking I/O
 * - Calls recv() and send() based on socket state
 * - Processes incoming packets
 * - Returns 0 on success, negative on error
 */
int FUN_0045e880(void) {
    /* Network I/O processing */
    /* TODO: Full implementation with select/recv/send */
    return 0;
}

/*
 * FUN_0045ede0 - Buffer Shift
 *
 * Binary analysis:
 * - Shifts buffer data by specified amount
 * - Used after processing data from buffer
 */
int FUN_0045ede0(int param_1) {
    char* buf = (char*)DAT_0461b41c;
    int pos = DAT_0461b408;

    if (!buf || param_1 <= 0 || param_1 > pos) return -1;

    memmove(buf, buf + param_1, pos - param_1);
    DAT_0461b408 = pos - param_1;
    return 0;
}

/* Network send stubs */
void network_send_login(const char* user, const char* pass) {
    (void)user; (void)pass;
}

void network_send_heartbeat(void) {}

int network_send_packet(int opcode, void* data, int len) {
    (void)opcode; (void)data; (void)len;
    return 0;
}

/* network_disconnect and network_is_connected are defined in network_core.c */
