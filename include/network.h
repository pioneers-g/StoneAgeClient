/*
 * Stone Age Client - Network Module Header
 * Reverse engineered from sa_9061.exe (FUN_0045e880, FUN_0045ec80, FUN_0045ee40)
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <winsock2.h>
#include "types.h"
#include "network_protocol.h"
#include "network_lsrpc.h"

/* Buffer sizes - matches original binary */
#define NET_RECV_BUFFER_SIZE    0x4000   /* 16KB receive buffer */
#define NET_SEND_BUFFER_SIZE    0x4000   /* 16KB send buffer */
#define NET_TEMP_BUFFER_SIZE    0x2000   /* 8KB temp buffer for recv() */

/* Key memory addresses from original binary */
#define NET_ADDR_RECV_BUFFER        0x0461b41c  /* Receive buffer pointer */
#define NET_ADDR_RECV_LEN           0x0461b408  /* Receive buffer length */
#define NET_ADDR_SEND_BUFFER        0x0461b420  /* Send buffer pointer */
#define NET_ADDR_SEND_LEN           0x0461b424  /* Send buffer length */
#define NET_ADDR_LAST_SEND_TIME     0x0461b410  /* Last send timestamp */
#define NET_ADDR_CONN_STATE         0x04ebffd8  /* Connection state (3=ingame) */
#define NET_ADDR_PROTOCOL_MODE      0x0461b658  /* Protocol mode (0/f000000) */
#define NET_ADDR_STATE_MACHINE      0x0461c008  /* State machine value */

/* Network states - from FUN_0045f4d0 state machine */
typedef enum {
    NET_STATE_DISCONNECTED = 0,
    NET_STATE_CONNECTING = 1,       /* States 1-69: Waiting for connection */
    NET_STATE_HANDSHAKE_1 = 70,     /* 0x46: Receive 'A' or 'E' packet */
    NET_STATE_HANDSHAKE_2 = 71,     /* 0x47-0x50: Send login credentials */
    NET_STATE_HANDSHAKE_3 = 81,     /* 0x51-0x62: Wait for login response */
    NET_STATE_CONNECTED = 99,       /* Connection fully established */
    NET_STATE_LOGGED_IN = 100       /* In-game state */
} NetworkState;

/* Connection status values - DAT_04ebffd8 */
#define CONN_STATUS_LOGIN          1
#define CONN_STATUS_CONNECTED      2
#define CONN_STATUS_IN_GAME        3

/* Protocol modes - from FUN_0045f4d0, DAT_0461b658 */
#define PROTOCOL_MODE_TEXT          0           /* Text-based protocol */
#define PROTOCOL_MODE_BINARY        0xf000000   /* Binary protocol flag */

typedef enum {
    PROTOCOL_TEXT = 0,          /* Text-based protocol (FUN_0043bf90) */
    PROTOCOL_BINARY = 1         /* Binary protocol (FUN_0048d3e0) - sets 0xf000000 */
} ProtocolMode;

/* Network context - matches DAT_0461b3f8 region */
typedef struct {
    /* Socket */
    SOCKET socket;
    NetworkState state;

    /* Protocol mode */
    ProtocolMode protocol_mode;     /* DAT_0461b658 */

    /* Timing */
    DWORD last_send_time;           /* DAT_0461b410 */
    DWORD last_recv_time;
    DWORD last_ping_time;           /* Last ping timestamp */
    u32 latency;                    /* Network latency in ms */

    /* Connection info */
    char server_ip[64];
    uint16_t server_port;

    /* Receive buffer - DAT_0461b41c, DAT_0461b408 */
    u8* recv_buffer;                /* Dynamically allocated */
    u32 recv_len;                   /* Current data length */
    u32 recv_capacity;

    /* Send buffer - gBuffer, len */
    u8* send_buffer;                /* Dynamically allocated */
    u32 send_len;                   /* Current data length */
    u32 send_capacity;

    /* Temp buffer for recv() - DAT_045f1bf8 */
    u8 temp_buffer[NET_TEMP_BUFFER_SIZE];

    /* Flags */
    int initialized;                /* DAT_0461b3fc */
    int connection_flag;            /* DAT_0461b420 */
    int heartbeat_flag;             /* DAT_0461b424 */
    int large_packet_flag;          /* DAT_0461b414 */
    u32 timer_flags;                /* DAT_0461b40c */

    /* Statistics */
    u32 heartbeat_count;            /* DAT_005ab714 */
    u32 reconnect_count;
} NetworkContext;

/* Global network context */
extern NetworkContext g_net;

/* Network functions - FUN_0045d890, FUN_0045d920 */
int network_init(void);
void network_shutdown(void);

/* Connection management */
int network_connect(const char* server, uint16_t port);
void network_disconnect(void);

/* Main network I/O - FUN_0045e880 */
void network_process(void);

/* Buffer management - FUN_0045ec80, FUN_0045ed80, FUN_0045ede0 */
int network_buffer_append(const void* data, u32 len);
void network_buffer_remove(u32 len);
int network_send_buffer_remove(u32 len);

/* Packet extraction - FUN_0045ee40 */
int network_extract_line(char* out, u32 max_len);

/* Send/receive */
int network_send(const void* data, size_t len);
int network_send_raw(const void* data, size_t len);

/* Convenience functions for field_update */
#define network_send_keepalive() packet_send_heartbeat()
int network_send_move(int x, int y, int is_running);
int network_send_pet_action(void);

/* Packet handling */
void packet_process(void);
void packet_send_heartbeat(void);

/* New functions for FUN_0045e880 */
int network_extract_message(void);
void network_process_login_packet(void);
void packet_send_heartbeat_text(SOCKET sock);
void packet_send_heartbeat_binary(SOCKET sock);
void packet_dispatch_binary_encrypted(SOCKET sock);

/* Text protocol dispatcher - FUN_0043bf90 */
void packet_dispatch_text(SOCKET sock, const char* line);

/* Binary protocol dispatcher - FUN_0048d3e0 */
void packet_dispatch_binary(SOCKET sock);

/* Protocol mode */
void network_set_protocol_mode(ProtocolMode mode);

/* State query */
u32 network_get_state(void);
u32 network_get_login_result(void);
const char* network_get_error_message(void);
int network_is_connected(void);
int network_check_socket(void);

/* Protocol encoding/decoding - FUN_0043efe0, FUN_0043ded0 */
int protocol_decode_int(const char* str);
char* protocol_unescape_string(const char* str);
void protocol_escape_string(const char* src, char* dst, int max_len);
void protocol_encode_int(int value, char* buf, int max_len);

/* LSRPC encryption/decryption - FUN_0043e320, FUN_0043e810, FUN_0043e540 */
int lsrpc_decode(const char* input, char* output);
void lsrpc_decrypt(char* data, int len, u32 key, char* output, int* out_len);
int lsrpc_decrypt_packet(const char* input, char* output);
void lsrpc_init_buffer(void* buffer);
char* lsrpc_get_output(void);

/* ========================================
 * Send Queue System - FUN_004115e0 pattern
 * ======================================== */

/* Send queue constants */
#define SEND_QUEUE_SIZE         20      /* 0x14 entries - DAT_0054a500 */
#define SEND_QUEUE_ENTRY_SIZE   0x10c   /* 268 bytes per entry */

/* Send queue entry - from DAT_00544d88 region */
typedef struct {
    char text[SEND_QUEUE_ENTRY_SIZE];   /* +0x00: Message text */
    u8 color;                            /* +0x108: Color index */
    u8 reserved[3];
    u32 type;                            /* +0x10C: Message type */
} SendQueueEntry;

/* Send queue context - DAT_00544d88, DAT_0054a4f8 */
typedef struct {
    SendQueueEntry entries[SEND_QUEUE_SIZE];
    u32 write_index;                     /* DAT_0054a4f8 */
    u32 count;                           /* DAT_0054a500 */
} SendQueue;

/* Global send queue */
extern SendQueue g_send_queue;

/* Send queue functions - FUN_004115e0, FUN_004115c0 */
void send_queue_init(void);
void send_queue_add(const char* text, u8 color, u32 type);
void send_queue_process(void);

/* ========================================
 * Field Parsing - FUN_00489f70, FUN_00489fe0
 * ======================================== */

/* Field parsing for pipe-delimited protocol */
#define FIELD_SEPARATOR     '|'     /* 0x7c */
#define MAX_FIELD_LEN       256

/* Parse fields from a pipe-delimited string - FUN_00489f70 */
int parse_fields(const char* input, char separator, int field_index,
                 int max_len, char* output);

/* Extract single field - FUN_00489fe0 */
int extract_field(const char* input, char separator, int max_len, char* output);

/* String to integer conversion - FUN_00492973, FUN_004929fe */
int string_to_int(const char* str);

/* ========================================
 * Protocol Commands - from FUN_0043bf90
 * ======================================== */

/* Character commands */
#define CMD_CLIENT_LOGIN     "ClientLogin"
#define CMD_CREATE_CHAR      "CreateNewChar"
#define CMD_CHAR_DELETE      "CharDelete"
#define CMD_CHAR_LOGIN       "CharLogin"
#define CMD_CHAR_LIST        "CharList"
#define CMD_CHAR_LOGOUT      "CharLogout"

/* Party commands */
#define CMD_PARTY_CREATE     "CP"
#define CMD_PARTY_ADD        "PA"
#define CMD_PARTY_DELETE     "PD"
#define CMD_PARTY_LIST       "PL"
#define CMD_PARTY_EXPEL      "PE"
#define CMD_PARTY_TRANSFER   "PT"
#define CMD_PARTY_STATUS     "PS"
#define CMD_PARTY_CHAT       "PC"

/* Friend/Social commands */
#define CMD_FRIEND_LIST      "PF"
#define CMD_FRIEND_ADD       "FA"    /* Actually PA in some contexts */
#define CMD_FRIEND_DELETE    "FD"
#define CMD_WHISPER          "FM"
#define CMD_BLOCK            "FB"
#define CMD_UNBLOCK          "FU"

/* Guild commands */
#define CMD_GUILD_CREATE     "GC"
#define CMD_GUILD_DISMISS    "GD"
#define CMD_GUILD_LIST       "GL"
#define CMD_GUILD_KICK       "GK"
#define CMD_GUILD_INVITE     "GI"
#define CMD_GUILD_APPLY      "GA"
#define CMD_GUILD_RESPONSE   "GR"
#define CMD_GUILD_NOTICE     "GN"
#define CMD_GUILD_TITLE      "GT"
#define CMD_GUILD_CHAT       "GF"

/* Battle commands */
#define CMD_BATTLE           "B"

/* Trade commands */
#define CMD_TRADE            "T"

/* Shop commands */
#define CMD_SHOP             "S"

/* Skill commands */
#define CMD_SKILL_LIST       "SL"
#define CMD_SKILL_USE        "SLU"
#define CMD_SKILL_RESPONSE   "SLR"

/* Other commands */
#define CMD_PROC_GET         "ProcGet"
#define CMD_PLAYER_NUM       "PlayerNumGet"

#endif /* NETWORK_H */
