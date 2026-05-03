/*
 * Stone Age Client - Network Protocol Utilities
 * Split from network_protocol.c
 *
 * Reverse engineered from sa_9061.exe:
 * FUN_00489f70 - Field parsing from pipe-delimited strings
 * FUN_00489fe0 - Field extraction utility
 * FUN_0048a050 - Integer field parsing
 * FUN_0048a120 - Base-62 integer decoder
 * FUN_0048a170 - String unescape function
 */

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "network.h"
#include "logger.h"

/* ========================================
 * Protocol Command Constants
 * ======================================== */

/* Multi-character commands - from DAT_004b9xxx */
#define CMD_CLIENT_LOGIN    "ClientLogin"
#define CMD_CREATE_NEW_CHAR "CreateNewChar"
#define CMD_CHAR_DELETE     "CharDelete"
#define CMD_CHAR_LOGIN      "CharLogin"
#define CMD_CHAR_LIST       "CharList"
#define CMD_CHAR_LOGOUT     "CharLogout"

/* Packet type characters */
#define PACKET_TYPE_RESPONSE    'R'
#define PACKET_TYPE_SUCCESS     'T'
#define PACKET_TYPE_ERROR       'E'
#define PACKET_TYPE_GAME_DATA   'G'

/* ========================================
 * Global State Variables
 * ======================================== */

static u32 s_connection_state = 0;
static u32 s_login_result = 0;
static char s_error_message[256];

/* ========================================
 * Field Parsing - FUN_00489f70
 * ======================================== */

/*
 * Parse fields from pipe-delimited string
 * Parameters:
 *   input: Input string
 *   separator: Field separator character (usually '|')
 *   field_index: Which field to extract (1-based)
 *   max_len: Maximum output length
 *   output: Output buffer
 */
int pet_parse_field(const char* input, char separator, int field_index,
                    int max_len, char* output) {
    const char* ptr = input;
    int current_field = 1;
    int i;

    if (!input || !output || max_len < 1) {
        if (output) *output = '\0';
        return 1;
    }

    /* Skip to requested field */
    while (current_field < field_index && *ptr) {
        if ((unsigned char)*ptr < 0x80) {
            /* ASCII character */
            if (*ptr == separator) {
                current_field++;
            }
            ptr++;
        } else {
            /* DBCS character - skip 2 bytes */
            if (ptr[1]) {
                ptr += 2;
            } else {
                break;
            }
        }
    }

    /* Check if we reached end of string */
    if (!*ptr) {
        *output = '\0';
        return 1;
    }

    /* Extract field */
    for (i = 0; i < max_len - 1 && *ptr; i++) {
        unsigned char c = (unsigned char)*ptr;

        if (c < 0x80) {
            if (*ptr == separator) {
                break;
            }
            output[i] = *ptr++;
        } else {
            /* DBCS character */
            if (i + 1 >= max_len - 1) break;
            output[i] = *ptr++;
            if (*ptr) {
                output[++i] = *ptr++;
            }
        }
    }

    output[i] = '\0';
    return 0;
}

/*
 * Parse integer field - FUN_0048a050
 */
int pet_parse_field_int(const char* input, char separator, int field_index) {
    char buffer[64];
    const char* ptr;
    int result = 0;
    int sign = 1;

    if (pet_parse_field(input, separator, field_index, sizeof(buffer), buffer) != 0) {
        return 0;
    }

    ptr = buffer;

    /* Skip whitespace */
    while (*ptr == ' ' || *ptr == '\t') ptr++;

    /* Check sign */
    if (*ptr == '-') {
        sign = -1;
        ptr++;
    } else if (*ptr == '+') {
        ptr++;
    }

    /* Parse digits */
    while (*ptr >= '0' && *ptr <= '9') {
        result = result * 10 + (*ptr - '0');
        ptr++;
    }

    return result * sign;
}

/*
 * Base-62 integer decoder - FUN_0048a120, FUN_0048a0a0
 * Used for bitmask parsing in cases 0x4B, 0x4E, 0x50
 *
 * Returns -1 (0xFFFFFFFF) for empty string
 * Supports negative numbers with '-' prefix
 */
s32 pet_parse_field_base62(const char* input, char separator, int field_index) {
    char buffer[128];
    const char* ptr;
    s32 result = 0;
    s32 sign = 1;

    if (pet_parse_field(input, separator, field_index, sizeof(buffer), buffer) != 0) {
        return -1;  /* Return -1 for empty/missing field */
    }

    if (buffer[0] == '\0') {
        return -1;
    }

    ptr = buffer;

    while (*ptr) {
        unsigned char c = (unsigned char)*ptr;

        if (c >= '0' && c <= '9') {
            result = result * 62 + (c - '0');
        } else if (c >= 'a' && c <= 'z') {
            result = result * 62 + (c - 'a' + 10);
        } else if (c >= 'A' && c <= 'Z') {
            result = result * 62 + (c - 'A' + 36);
        } else if (c == '-') {
            sign = -1;
        } else {
            break;
        }
        ptr++;
    }

    return sign * result;
}

/*
 * Unescape a protocol string - FUN_0048a170
 * Handles: \\S -> space, \\n -> newline, \\r -> CR, \\\\ -> backslash
 */
char* pet_unescape_string(char* str) {
    int src_pos = 0;
    int dst_pos = 0;

    if (!str || !str[0]) {
        return str;
    }

    while (str[src_pos]) {
        /* Check for DBCS lead byte */
        if (IsDBCSLeadByte((BYTE)str[src_pos])) {
            str[dst_pos++] = str[src_pos++];
            if (str[src_pos]) {
                str[dst_pos++] = str[src_pos++];
            } else {
                str[dst_pos] = '\0';
                return str;
            }
            continue;
        }

        /* Handle escape sequences */
        if (str[src_pos] == '\\' && str[src_pos + 1]) {
            src_pos++;
            switch (str[src_pos]) {
                case 'S':
                    str[dst_pos++] = ' ';
                    src_pos++;
                    break;
                case 'n':
                    str[dst_pos++] = '\n';
                    src_pos++;
                    break;
                case 'r':
                    str[dst_pos++] = '\r';
                    src_pos++;
                    break;
                case '\\':
                    str[dst_pos++] = '\\';
                    src_pos++;
                    break;
                default:
                    str[dst_pos++] = '\\';
                    break;
            }
        } else {
            str[dst_pos++] = str[src_pos++];
        }
    }

    str[dst_pos] = '\0';
    return str;
}

/*
 * Escape a string for protocol transmission
 */
void protocol_escape_string(const char* src, char* dst, int max_len) {
    int src_pos = 0;
    int dst_pos = 0;

    if (!src || !dst || max_len < 1) {
        return;
    }

    while (src[src_pos] && dst_pos < max_len - 2) {
        switch (src[src_pos]) {
            case ' ':
                dst[dst_pos++] = '\\';
                dst[dst_pos++] = 'S';
                src_pos++;
                break;
            case '\n':
                dst[dst_pos++] = '\\';
                dst[dst_pos++] = 'n';
                src_pos++;
                break;
            case '\r':
                dst[dst_pos++] = '\\';
                dst[dst_pos++] = 'r';
                src_pos++;
                break;
            case '\\':
                dst[dst_pos++] = '\\';
                dst[dst_pos++] = '\\';
                src_pos++;
                break;
            default:
                dst[dst_pos++] = src[src_pos++];
                break;
        }
    }

    dst[dst_pos] = '\0';
}

/*
 * Encode integer to base-62 string
 */
void protocol_encode_int(int value, char* buf, int max_len) {
    static const char chars[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int pos = 0;
    int is_negative = 0;

    if (!buf || max_len < 1) {
        return;
    }

    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    /* Encode in reverse */
    char temp[32];
    int temp_pos = 0;

    if (value == 0) {
        temp[temp_pos++] = '0';
    } else {
        while (value > 0 && temp_pos < 31) {
            temp[temp_pos++] = chars[value % 62];
            value /= 62;
        }
    }

    /* Copy reversed */
    if (is_negative && pos < max_len - 1) {
        buf[pos++] = '-';
    }

    while (temp_pos > 0 && pos < max_len - 1) {
        buf[pos++] = temp[--temp_pos];
    }

    buf[pos] = '\0';
}

/* ========================================
 * Send Queue System - FUN_004115e0
 * ======================================== */

/* Use definitions from network.h */
SendQueue g_send_queue = {0};
static u32 s_msg_type_threshold = 0;

/*
 * Initialize send queue
 */
void send_queue_init(void) {
    memset(&g_send_queue, 0, sizeof(SendQueue));
    s_msg_type_threshold = 0;
}

/*
 * Add message to send queue
 */
void send_queue_add(const char* text, u8 color, u32 type) {
    SendQueueEntry* entry;
    u32 index;
    size_t len;

    if (!text || !text[0]) {
        return;
    }

    len = strlen(text);
    if (len >= SEND_QUEUE_ENTRY_SIZE) {
        len = SEND_QUEUE_ENTRY_SIZE - 1;
    }

    index = g_send_queue.write_index;
    entry = &g_send_queue.entries[index];

    strncpy(entry->text, text, len);
    entry->text[len] = '\0';
    entry->color = color;
    entry->type = type;

    g_send_queue.write_index = (index + 1) % SEND_QUEUE_SIZE;
    g_send_queue.count++;

    if (g_send_queue.count > SEND_QUEUE_SIZE) {
        g_send_queue.count = SEND_QUEUE_SIZE;
    }

    LOG_DEBUG("Queued message: color=%d type=%d", color, type);
}

/*
 * Process send queue
 */
void send_queue_process(void) {
    u32 i;

    for (i = 0; i < g_send_queue.count; i++) {
        SendQueueEntry* entry = &g_send_queue.entries[i];

        if (entry->text[0]) {
            char packet[SEND_QUEUE_ENTRY_SIZE + 16];
            int len = snprintf(packet, sizeof(packet), "%s\n", entry->text);
            network_send(packet, len);
            memset(entry, 0, sizeof(SendQueueEntry));
        }
    }

    g_send_queue.count = 0;
    g_send_queue.write_index = 0;
}

/*
 * Get send queue count
 */
int send_queue_count(void) {
    return g_send_queue.count;
}

/* ========================================
 * State Access Functions
 * ======================================== */

u32 network_get_state(void) {
    return s_connection_state;
}

u32 network_get_login_result(void) {
    return s_login_result;
}

const char* network_get_error_message(void) {
    return s_error_message;
}

/* ========================================
 * Field Parsing Utilities - FUN_00489f70, FUN_00489fe0
 * ======================================== */

/*
 * Parse fields from pipe-delimited string - FUN_00489f70
 */
int parse_fields(const char* input, char separator, int field_index,
                 int max_len, char* output) {
    return pet_parse_field(input, separator, field_index, max_len, output);
}

/*
 * Extract single field until separator - FUN_00489fe0
 */
int extract_field(const char* input, char separator, int max_len, char* output) {
    const char* src = input;
    char* dst = output;
    int i = 0;

    if (!input || !output || max_len < 1) {
        if (output) *output = '\0';
        return 1;
    }

    while (i < max_len && *src) {
        unsigned char c = (unsigned char)*src;

        if (c < 0x80) {
            if (c == separator) {
                break;
            }
            *dst++ = *src++;
            i++;
        } else {
            *dst++ = *src++;
            i++;

            if (i >= max_len || !*src) {
                break;
            }

            *dst++ = *src++;
            i++;
        }
    }

    *dst = '\0';
    return 0;
}

/*
 * String to integer conversion
 */
int string_to_int(const char* str) {
    const char* ptr = str;
    int result = 0;
    int sign = 1;

    if (!str) {
        return 0;
    }

    while (*ptr == ' ' || *ptr == '\t') {
        ptr++;
    }

    if (*ptr == '-') {
        sign = -1;
        ptr++;
    } else if (*ptr == '+') {
        ptr++;
    }

    while (*ptr >= '0' && *ptr <= '9') {
        result = result * 10 + (*ptr - '0');
        ptr++;
    }

    return result * sign;
}
