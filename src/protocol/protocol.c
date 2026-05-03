/*
 * Stone Age Client - Network Protocol Implementation
 * Reverse engineered from sa_9061.exe (FUN_0048d3e0)
 */

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "types.h"
#include "protocol.h"
#include "protocol_binary.h"
#include "protocol_text.h"
#include "network.h"
#include "logger.h"

/* Encryption keys - derived from FUN_0041db40 analysis */
static u32 g_encrypt_key = 0;
static u32 g_decrypt_key = 0;
static u32 g_default_key = 0xBCDE;

/*
 * Initialize protocol
 */
void protocol_init(void) {
    g_encrypt_key = 0;
    g_decrypt_key = 0;
    LOG_INFO("Protocol initialized");
}

/*
 * Shutdown protocol
 */
void protocol_shutdown(void) {
    g_encrypt_key = 0;
    g_decrypt_key = 0;
}

/*
 * Calculate packet checksum - FUN_004929fe pattern
 */
u16 packet_checksum(void* data, int len) {
    u16 sum = 0;
    u8* ptr = (u8*)data;

    while (len-- > 0) {
        sum += *ptr++;
    }

    return sum ^ 0xFFFF;
}

/*
 * Initialize packet reader - FUN_00492d80 pattern
 */
void packet_reader_init(PacketReader* reader, void* data, u32 size) {
    reader->data = (u8*)data;
    reader->size = size;
    reader->offset = 0;
    reader->checksum_calc = 0;
    reader->checksum_recv = 0;
}

/*
 * Read 16-bit value - FUN_0048d190 pattern
 */
u16 packet_read_u16(PacketReader* reader) {
    u16 value;

    if (reader->offset + 2 > reader->size) {
        return 0;
    }

    value = *(u16*)(reader->data + reader->offset);
    reader->offset += 2;
    reader->checksum_calc += value;

    return value;
}

/*
 * Read 32-bit value - FUN_0048d2b0 pattern
 */
u32 packet_read_u32(PacketReader* reader) {
    u32 value;

    if (reader->offset + 4 > reader->size) {
        return 0;
    }

    value = *(u32*)(reader->data + reader->offset);
    reader->offset += 4;
    reader->checksum_calc += (value & 0xFFFF) + ((value >> 16) & 0xFFFF);

    return value;
}

/*
 * Validate packet checksum
 */
int packet_validate_checksum(PacketReader* reader) {
    return reader->checksum_calc == reader->checksum_recv;
}

/*
 * Build login packet
 */
int packet_build_login(char* buffer, const char* username, const char* password) {
    PacketLogin* pkt = (PacketLogin*)buffer;

    memset(pkt, 0, sizeof(PacketLogin));

    pkt->opcode = PKT_CLIENT_LOGIN;
    strncpy(pkt->username, username, sizeof(pkt->username) - 1);
    strncpy(pkt->password, password, sizeof(pkt->password) - 1);
    strncpy(pkt->version, "9.0.6.1", sizeof(pkt->version) - 1);
    pkt->flags = 0;

    pkt->size = sizeof(PacketLogin);
    pkt->checksum = packet_checksum(((u8*)pkt) + 4, pkt->size - 4);

    return pkt->size;
}

/*
 * Build character select packet
 */
int packet_build_char_select(char* buffer, u32 char_id) {
    struct {
        u16 size;
        u16 opcode;
        u16 checksum;
        u32 char_id;
    } *pkt = (void*)buffer;

    pkt->opcode = PKT_CHAR_LOGIN;
    pkt->char_id = char_id;
    pkt->size = 10;
    pkt->checksum = packet_checksum(&pkt->char_id, 4);

    return pkt->size;
}

/*
 * Build movement packet
 */
int packet_build_move(char* buffer, u16 x, u16 y, u8 direction) {
    struct {
        u16 size;
        u16 opcode;
        u16 checksum;
        u16 x;
        u16 y;
        u8  direction;
        u8  padding;
    } *pkt = (void*)buffer;

    pkt->opcode = PKT_CLIENT_MOVE;
    pkt->x = x;
    pkt->y = y;
    pkt->direction = direction;
    pkt->size = 12;
    pkt->checksum = 0;

    return pkt->size;
}

/*
 * Build action packet
 */
int packet_build_action(char* buffer, u8 action, u32 target_id) {
    struct {
        u16 size;
        u16 opcode;
        u16 checksum;
        u8  action;
        u8  padding;
        u32 target_id;
    } *pkt = (void*)buffer;

    pkt->opcode = PKT_CLIENT_ACTION;
    pkt->action = action;
    pkt->target_id = target_id;
    pkt->size = 12;
    pkt->checksum = 0;

    return pkt->size;
}

/*
 * Build talk packet
 */
int packet_build_talk(char* buffer, const char* message) {
    struct {
        u16 size;
        u16 opcode;
        u16 checksum;
        char message[256];
    } *pkt = (void*)buffer;

    pkt->opcode = PKT_CLIENT_TALK;
    strncpy(pkt->message, message, sizeof(pkt->message) - 1);
    pkt->size = 6 + (u16)strlen(pkt->message) + 1;
    pkt->checksum = 0;

    return pkt->size;
}

/*
 * Build heartbeat packet
 */
int packet_build_heartbeat(char* buffer) {
    struct {
        u16 size;
        u16 opcode;
        u16 checksum;
        u32 timestamp;
    } *pkt = (void*)buffer;

    pkt->opcode = PKT_HEARTBEAT;
    pkt->timestamp = timeGetTime();
    pkt->size = 10;
    pkt->checksum = 0;

    return pkt->size;
}

/*
 * Parse received packet
 */
int packet_parse(char* buffer, int len, void (*handler)(u16 opcode, void* data, int size)) {
    PacketHeader* hdr;
    int processed = 0;

    while (len >= 4) {
        hdr = (PacketHeader*)buffer;

        /* Check if we have complete packet */
        if (len < hdr->size) {
            break;
        }

        /* Validate checksum */
        u16 calc_checksum = packet_checksum(buffer + 6, hdr->size - 6);
        if (calc_checksum != hdr->checksum) {
            LOG_WARN("Packet checksum mismatch: expected 0x%04X, got 0x%04X",
                     calc_checksum, hdr->checksum);
            /* Continue processing anyway for compatibility */
        }

        /* Call handler */
        if (handler) {
            handler(hdr->opcode, hdr->data, hdr->size - 6);
        }

        processed += hdr->size;
        buffer += hdr->size;
        len -= hdr->size;
    }

    return processed;
}

/*
 * Base-62 string to integer conversion - FUN_0043efe0
 */
int protocol_decode_int(const char* str) {
    int result = 0;
    int sign = 1;

    if (*str == '-') {
        sign = -1;
        str++;
    }

    while (*str) {
        result *= 62;  /* 0x3e */
        if (*str >= '0' && *str <= '9') {
            result += *str - '0';
        } else if (*str >= 'a' && *str <= 'z') {
            result += *str - 'a' + 10;
        } else if (*str >= 'A' && *str <= 'Z') {
            result += *str - 'A' + 36;
        } else {
            return 0;  /* Invalid character */
        }
        str++;
    }

    return result * sign;
}

/*
 * Integer to Base-62 string conversion - inverse of FUN_0043efe0
 */
void protocol_encode_int(int value, char* buffer, int max_len) {
    static const char base62[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char temp[16];
    int i = 0;
    int is_negative = 0;
    unsigned int uval;

    if (value < 0) {
        is_negative = 1;
        uval = (unsigned int)(-value);
    } else {
        uval = (unsigned int)value;
    }

    /* Handle zero case */
    if (uval == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    /* Convert to base62 (reversed) */
    while (uval > 0 && i < 15) {
        temp[i++] = base62[uval % 62];
        uval /= 62;
    }

    /* Reverse into output buffer */
    int pos = 0;
    if (is_negative && pos < max_len - 1) {
        buffer[pos++] = '-';
    }

    while (i > 0 && pos < max_len - 1) {
        buffer[pos++] = temp[--i];
    }

    buffer[pos] = '\0';
}

/* Static buffer for unescape function - DAT_004b9ff8 pattern */
static char s_unescape_buffer[1024];

/*
 * Unescape string - FUN_0043ded0 pattern
 * Handles escape sequences: \S -> space, \n -> newline, \r -> CR, \\ -> backslash
 * Also handles DBCS characters correctly
 */
void protocol_unescape_string_to(const char* src, char* dst, int max_len) {
    int src_idx = 0;
    int dst_idx = 0;

    if (!src || !dst || max_len <= 0) {
        if (dst && max_len > 0) dst[0] = '\0';
        return;
    }

    while (src[src_idx] && dst_idx < max_len - 1) {
        /* Check for DBCS lead byte */
        if (IsDBCSLeadByte((BYTE)src[src_idx])) {
            /* Copy DBCS character pair */
            dst[dst_idx++] = src[src_idx++];
            if (src[src_idx] && dst_idx < max_len - 1) {
                dst[dst_idx++] = src[src_idx++];
            }
            continue;
        }

        /* Check for escape sequence */
        if (src[src_idx] == '\\') {
            char next = src[src_idx + 1];

            if (next == '\0') {
                /* Trailing backslash */
                dst[dst_idx++] = '\\';
                break;
            }

            switch (next) {
                case 'S':
                    dst[dst_idx++] = ' ';  /* Space */
                    src_idx += 2;
                    break;
                case 'n':
                    dst[dst_idx++] = '\n';  /* Newline */
                    src_idx += 2;
                    break;
                case 'r':
                    dst[dst_idx++] = '\r';  /* Carriage return */
                    src_idx += 2;
                    break;
                case '\\':
                    dst[dst_idx++] = '\\';  /* Backslash */
                    src_idx += 2;
                    break;
                default:
                    /* Unknown escape - keep backslash */
                    dst[dst_idx++] = '\\';
                    src_idx++;
                    break;
            }
        } else {
            dst[dst_idx++] = src[src_idx++];
        }
    }

    dst[dst_idx] = '\0';
}

/*
 * Unescape string to static buffer - FUN_0043dd70 pattern
 */
char* protocol_unescape_string(const char* str) {
    if (!str || !str[0]) {
        s_unescape_buffer[0] = '\0';
        return s_unescape_buffer;
    }

    protocol_unescape_string_to(str, s_unescape_buffer, sizeof(s_unescape_buffer));
    return s_unescape_buffer;
}

/*
 * Send text protocol command - matches FUN_0043bea0 pattern
 */
int protocol_send_text_command(SOCKET sock, const char* cmd, const char* params) {
    char buffer[2048];
    int len;

    if (params && params[0]) {
        len = snprintf(buffer, sizeof(buffer), "%s %s\n", cmd, params);
    } else {
        len = snprintf(buffer, sizeof(buffer), "%s\n", cmd);
    }

    if (len <= 0 || len >= (int)sizeof(buffer)) {
        return -1;
    }

    /* Queue for sending via network module */
    extern NetworkContext g_net;
    if (g_net.send_len + len > g_net.send_capacity) {
        return -1;
    }

    memcpy(g_net.send_buffer + g_net.send_len, buffer, len);
    g_net.send_len += len;

    LOG_DEBUG("Queued text command: %s", cmd);
    return 0;
}
