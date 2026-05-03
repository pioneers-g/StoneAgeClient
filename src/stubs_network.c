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
 * - Uses select() for non-blocking I/O on gSocket
 * - Calls recv() when socket is readable
 * - Calls send() when socket is writable (via gBuffer/len)
 * - Processes incoming packets via FUN_0045ec80/FUN_0045ee40
 * - Dispatches text protocol via FUN_0043bf90 or binary via FUN_0048d3e0
 * - Sends heartbeat every 30 seconds (30000ms) via FUN_0043bea0
 * - Handles connection errors by setting DAT_0461b420=1
 * - Uses VirtualProtect for buffer memory protection
 * - Returns void (status via global state)
 *
 * Key globals:
 * - DAT_0461b3f8: Connection active flag
 * - DAT_0461b420: Disconnection flag
 * - DAT_0461b424: Previous disconnection state
 * - DAT_04ebffd8: Protocol mode (3 = connected)
 * - DAT_0461b410: Last activity timestamp
 * - DAT_0461b658: Binary protocol mode flag
 */
void FUN_0045e880(void) {
    /* Network I/O processing */
    /* TODO: Full implementation with select/recv/send */
}

/*
 * FUN_00489f70 - String Token Extraction
 *
 * Binary analysis:
 * - Extracts the Nth field from a delimited string
 * - param_1: input string
 * - param_2: delimiter character
 * - param_3: field index (0-based)
 * - param_4: maximum field size
 * - param_5: output buffer
 * - Returns 0 on success, 1 if field not found
 * - Handles DBCS characters (bytes >= 0x80)
 * - Calls FUN_00489fe0 to extract the token
 */
int FUN_00489f70(const char* str, char delimiter, int field_index, int max_size, char* output) {
    const unsigned char* ptr = (const unsigned char*)str;
    int count = 0;

    if (!str || !output) {
        if (output) output[0] = '\0';
        return 1;
    }

    /* Skip to the desired field */
    while (count < field_index) {
        while (*ptr && *ptr != (unsigned char)delimiter) {
            /* Handle DBCS characters */
            if (*ptr >= 0x80 && ptr[1]) {
                ptr += 2;
            } else {
                ptr++;
            }
        }
        if (*ptr == delimiter) {
            ptr++;
            count++;
        } else {
            /* End of string, field not found */
            output[0] = '\0';
            return 1;
        }
    }

    /* Extract the field */
    /* TODO: Call FUN_00489fe0 for actual extraction */
    int i = 0;
    while (*ptr && *ptr != (unsigned char)delimiter && i < max_size - 1) {
        output[i++] = *ptr++;
    }
    output[i] = '\0';
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

/*
 * FUN_0045fa40 - Login Server List Response Handler
 *
 * Binary analysis:
 * - Handles response from login server request
 * - param_1: connection handle
 * - param_2: response string (DBCS)
 * - Checks if response matches DAT_004c4474 (server list data)
 * - If match: sets DAT_0461bff4 = 1, calls FUN_00492750/FUN_004925f0
 * - Only processes when DAT_0461c000 == 1 (waiting for response)
 * - Sets DAT_0461c000 = 2 (processed)
 */
void FUN_0045fa40(int conn_handle, const char* response) {
    (void)conn_handle; (void)response;
}

/*
 * FUN_0045fb80 - Login Character List Response Handler
 *
 * Binary analysis:
 * - Handles character list response after successful login
 * - param_1: connection handle
 * - param_2: status string ("successful" or other)
 * - param_3: character data string
 * - On "successful": sets DAT_0461bff6 = 1
 *   - Parses 3 character entries (indices 1,3,5) from param_3
 *   - Uses FUN_00489f70 to extract fields delimited by '|'
 *   - Calls FUN_00478a30 to process character data
 * - On "OUTOFSERVICE": sets DAT_0461bff6 = 2 (server maintenance)
 * - State machine: DAT_0461c000 (1->2)
 */
void FUN_0045fb80(int conn_handle, const char* status, const char* char_data) {
    (void)conn_handle; (void)status; (void)char_data;
}

/*
 * FUN_0045fdc0 - Login Server Select Response Handler
 *
 * Binary analysis:
 * - Handles server selection response
 * - param_1: connection handle
 * - param_2: status string ("successful" or "close")
 * - param_3: server parameter (port number)
 * - On "successful":
 *   - Creates thread at LAB_00465900 if DAT_0461c00c == 0
 *   - Sets DAT_0461bffc = 1
 * - On "close":
 *   - Parses param_3 as port number via FUN_004929fe
 *   - If port == -502 (0xfffffe0a): sets DAT_0461bffc = 0xfe0a
 * - Clears DAT_00564e70/74 (connection state)
 * - State machine: DAT_0461c000 (1->2)
 */
void FUN_0045fdc0(int conn_handle, const char* status, int param) {
    (void)conn_handle; (void)status; (void)param;
}

/*
 * FUN_0045ff50 - Login Create Character Response Handler
 *
 * Binary analysis:
 * - Handles character creation response
 * - param_1: connection handle
 * - param_2: status string ("successful" or error)
 * - On "successful": sets DAT_0461bffe = 1
 * - State machine: DAT_0461c000 (1->2)
 */
void FUN_0045ff50(int conn_handle, const char* status) {
    (void)conn_handle; (void)status;
}

/*
 * FUN_00476860 - Parse Hexadecimal Number from Protocol String
 *
 * Binary analysis:
 * - Parses a hex number from DAT_004e1118 starting at DAT_0461c678
 * - Skips non-hex characters at start
 * - Accepts: '0'-'9' (0x30-0x39) and 'A'-'F' (0x41-0x46)
 * - Returns -1 if no hex digit found
 * - Converts: '0'-'9' -> 0-9, 'A'-'F' -> 10-15
 * - Multiplies by 16 for each subsequent hex digit
 * - Updates DAT_0461c678 to position after last hex digit
 */
int FUN_00476860(void) {
    return -1;
}

/*
 * FUN_004768e0 - Extract String Field from Protocol Message
 *
 * Binary analysis:
 * - Extracts a string field from DAT_004e1118 at position DAT_0461c678
 * - param_1: destination structure
 * - param_2: field selector (0 = offset 0x38, 1 = offset 0x55)
 * - Reads until null terminator or '|' delimiter
 * - Handles DBCS characters via IsDBCSLeadByte()
 * - For DBCS: copies 2 bytes (lead + trail)
 * - For SBCS: copies 1 byte
 * - Null-terminates the result
 * - Calls FUN_0048a170 to process the extracted string
 * - Updates DAT_0461c678 to position after delimiter
 */
void FUN_004768e0(void* dest_struct, int field_selector) {
    (void)dest_struct; (void)field_selector;
}
