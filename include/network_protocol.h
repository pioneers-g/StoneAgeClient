/*
 * Stone Age Client - Network Protocol Header
 * Split from network.h for code organization
 */

#ifndef NETWORK_PROTOCOL_H
#define NETWORK_PROTOCOL_H

#include <winsock2.h>
#include "types.h"
#include "network.h"

/* ========================================
 * Binary Packet Types - from FUN_0045ffb0
 * ======================================== */

/* Packet type is first byte of packet data */
#define PACKET_TYPE_ITEM          0x42  /* 'B' - Item/pet inventory */
#define PACKET_TYPE_MAP_ENTRY     0x43  /* 'C' - Map entry/battle start */
#define PACKET_TYPE_PLAYER_POS    0x44  /* 'D' - Player position */
#define PACKET_TYPE_UNKNOWN_E     0x45  /* 'E' - Unknown */
#define PACKET_TYPE_PLAYER_DATA   0x46  /* 'F' - Player data */
#define PACKET_TYPE_PARTY         0x49  /* 'I' - Party data */
#define PACKET_TYPE_PET_STATUS    0x4a  /* 'J' - Pet status */
#define PACKET_TYPE_CHAR_STATUS   0x4b  /* 'K' - Character status bitmask */
#define PACKET_TYPE_BATTLE_START  0x4d  /* 'M' - Battle start */
#define PACKET_TYPE_BATTLE_UNIT   0x4e  /* 'N' - Battle unit status */
#define PACKET_TYPE_CHAR_STATS    0x50  /* 'P' - Character stats bitmask */
#define PACKET_TYPE_SKILL_LIST    0x53  /* 'S' - Skill list */
#define PACKET_TYPE_PET_DATA      0x57  /* 'W' - Pet data */
#define PACKET_TYPE_UNKNOWN_X     0x58  /* 'X' - Unknown */

/* Text packet types - from DAT_04ec0474 */
#define PACKET_TYPE_RESPONSE      'R'
#define PACKET_TYPE_SUCCESS       'T'
#define PACKET_TYPE_ERROR_MSG     'E'
#define PACKET_TYPE_GAME_DATA     'G'
#define PACKET_TYPE_WORLD_DATA    'W'
#define PACKET_TYPE_UPDATE        'U'

/* Connection state values - DAT_04ebffd8 */
#define CONN_STATE_LOGIN          1
#define CONN_STATE_CONNECTED      2
#define CONN_STATE_IN_GAME        3

/* ========================================
 * Protocol Dispatchers
 * ======================================== */

/* Text protocol dispatcher - FUN_0048a6f0 pattern */
void packet_dispatch_text(SOCKET sock, const char* line);

/* Binary protocol dispatcher - FUN_0043bf90 pattern */
void packet_dispatch_binary(SOCKET sock);

/* ========================================
 * Send Queue System
 * ======================================== */

/* Initialize send queue */
void send_queue_init(void);

/* Add message to send queue - FUN_004115e0 pattern */
void send_queue_add(const char* text, u8 color, u32 type);

/* Get send queue count */
int send_queue_count(void);

/* Process send queue */
void send_queue_process(void);

/* ========================================
 * Field Parsing
 * ======================================== */

/* Parse fields from pipe-delimited string - FUN_00489f70 pattern */
int parse_fields(const char* input, char separator, int field_index,
                 int max_len, char* output);

/* Extract single field until separator - FUN_00489fe0 pattern */
int extract_field(const char* input, char separator, int max_len, char* output);

/* String to integer conversion - FUN_00492973 pattern */
int string_to_int(const char* str);

/* ========================================
 * Protocol Encoding/Decoding
 * ======================================== */

/* Decode base-62 encoded integer - FUN_0043efe0 pattern */
int protocol_decode_int(const char* str);

/* Unescape a protocol string - FUN_0043ded0 pattern */
char* protocol_unescape_string(const char* str);

/* Escape a string for protocol transmission */
void protocol_escape_string(const char* src, char* dst, int max_len);

/* Encode integer to base-62 string */
void protocol_encode_int(int value, char* buf, int max_len);

#endif /* NETWORK_PROTOCOL_H */
