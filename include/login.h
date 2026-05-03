/*
 * Stone Age Client - Login System Header
 * Reverse engineered from sa_9061.exe (FUN_00420590, FUN_0045f4d0, FUN_0043bb10)
 */

#ifndef LOGIN_H
#define LOGIN_H

#include <winsock2.h>
#include "types.h"

/* Constants - from original binary */
#define MAX_USERNAME        32
#define MAX_PASSWORD        32
#define MAX_SERVER          64
#define MAX_SERVERS         10
#define MAX_CHARACTERS      3       /* Max characters per account */
#define LOGIN_TIMEOUT       600000  /* 10 minutes in ms - from FUN_0045f4d0 */
#define HANDSHAKE_TIMEOUT   30000

/* Login screen states - from FUN_00420590 DAT_04630df0 */
typedef enum {
    LOGIN_SCREEN_INIT = 0,          /* State 0: Cleanup, init UI */
    LOGIN_SCREEN_RENDER = 1,        /* State 1: Render login screen */
    LOGIN_SCREEN_INPUT = 2,         /* State 2: Handle input */
    LOGIN_SCREEN_CONNECTING = 3,    /* State 3: Creating connection widget */
    LOGIN_SCREEN_WAITING = 4,       /* State 4: Wait for server response */
    LOGIN_SCREEN_ERROR = 100,       /* State 100: Error display */
    LOGIN_SCREEN_RETRY = 101        /* State 101 (0x65): Retry after error */
} LoginScreenState;

/* Login states - from FUN_0045f4d0 state machine */
typedef enum {
    LOGIN_STATE_DISCONNECTED = 0,
    LOGIN_STATE_CONNECTING = 1,
    LOGIN_STATE_HANDSHAKE = 2,
    LOGIN_STATE_AUTH = 3,
    LOGIN_STATE_SERVER_LIST = 4,
    LOGIN_STATE_CHAR_LIST = 5,
    LOGIN_STATE_CHAR_SELECT = 6,
    LOGIN_STATE_ENTERING_GAME = 7,
    LOGIN_STATE_IN_GAME = 8,
    LOGIN_STATE_ERROR = 100
} LoginState;

/* Connection states - DAT_0461c008 values */
typedef enum {
    LOGIN_CONN_STATE_NONE = 0,
    LOGIN_CONN_STATE_CONNECTING = 1,          /* States 1-70 (0x46) */
    LOGIN_CONN_STATE_SEND_LOGIN = 0x47,       /* State 71 - send ClientLogin */
    LOGIN_CONN_STATE_WAIT_RESPONSE = 0x51,    /* States 81-98 (0x51-0x62) */
    LOGIN_CONN_STATE_CONNECTED = 99
} ConnectionState;

/* Login result codes */
typedef enum {
    LOGIN_SUCCESS = 0,
    LOGIN_ERR_TIMEOUT = -1,
    LOGIN_ERR_CONNECTION_FAILED = -6,
    LOGIN_ERR_SOCKET_ERROR = -7,
    LOGIN_ERR_DNS_FAILED = -8,
    LOGIN_ERR_INVALID_USER = -3,
    LOGIN_ERR_INVALID_PASS = -4,
    LOGIN_ERR_ALREADY_LOGGED = -5,
    LOGIN_ERR_SERVER_DOWN = -9,
    LOGIN_ERR_VERSION_MISMATCH = -10,
    LOGIN_ERR_BANNED = -11
} LoginResult;

/* Protocol modes - defined in network.h */

/* Server response codes - from FUN_0045f4d0 */
#define SERVER_RESPONSE_ACCEPT  'A'
#define SERVER_RESPONSE_ERROR   'E'
#define SERVER_RESPONSE_LOGIN_OK 'L'

/* Encryption key defined in login_crypto.h */

/* Server information */
typedef struct {
    u32 id;
    char name[32];
    char host[64];
    u16 port;
    u16 population;
    u8 status;          /* 0 = online, 1 = busy, 2 = maintenance */
    u8 padding;
} ServerInfo;

/* Character information for selection screen */
typedef struct {
    u32 id;
    char name[24];
    u16 level;
    u16 job;
    u8 face;
    u8 body;
    u32 map_id;
    u16 x;
    u16 y;
    u8 exists;
    u8 padding;
} CharacterInfo;

/* Text input box structure - from FUN_00420fb0 */
typedef struct {
    char buffer[256];           /* +0x00: Text buffer */
    u32 field_100;              /* +0x100 */
    u32 field_104;              /* +0x104 */
    u32 field_108;              /* +0x108 */
    u32 field_10c;              /* +0x10c: X position */
    u32 field_110;              /* +0x110: Y position */
    u32 cursor_pos;             /* +0x114: Cursor X */
    u32 cursor_y;               /* +0x118: Cursor Y */
    u32 field_11c;              /* +0x11c: Color */
    u32 field_120;              /* +0x120: Password flag */
    u8 max_len;                 /* +0x105: Max length */
    u8 char_width;              /* +0x106: Character width */
    u8 cursor_index;            /* +0x109: Cursor position in text */
    u8 field_107;               /* +0x107 */
    s32 render_index;           /* +0x124: Render queue index */
} TextInputBox;

/* Login context - matches DAT_0461bfXX region */
typedef struct {
    /* Connection state - from FUN_0045f4d0 */
    ConnectionState conn_state;     /* DAT_0461c008 */
    u32 login_internal_state;       /* DAT_0461c000 */
    u32 protocol_mode;              /* DAT_0461b658 */

    /* Login screen state - DAT_04630df0 */
    LoginScreenState screen_state;

    /* Socket - global in original, but kept in context for our implementation */
    SOCKET sock;                    /* Network socket */

    /* Server connection */
    char server_host[MAX_SERVER];
    u16 server_port;
    u32 server_ip;                  /* DAT_0461bf64 */
    u16 connect_port;               /* DAT_0461bf60 */

    /* Credentials */
    char username[MAX_USERNAME];    /* DAT_0454f278 region */
    char password[MAX_PASSWORD];    /* DAT_0455aa58 region */
    char encrypted_user[32];        /* DAT_0455703c */
    char encrypted_pass[32];        /* DAT_04556280 */
    char session_key[64];

    /* Text input boxes */
    TextInputBox username_box;      /* Username input */
    TextInputBox password_box;      /* Password input */

    /* Cursor positions */
    u32 username_cursor;            /* DAT_0454f37f */
    u32 password_cursor;            /* DAT_0455ab5f */
    u32 username_cursor_prev;       /* DAT_0454f381 */
    u32 password_cursor_prev;       /* DAT_0455ab61 */

    /* Server list */
    ServerInfo servers[MAX_SERVERS];
    int server_count;
    int selected_server_index;

    /* Character list */
    CharacterInfo characters[MAX_CHARACTERS];
    int char_count;
    int selected_char_index;

    /* State tracking */
    LoginState state;
    LoginResult result;
    char error_message[128];

    /* Timing - from FUN_0045f4d0 */
    u32 start_time;                 /* DAT_0461c004 */
    u32 last_activity;

    /* Flags */
    u8 logged_in;
    u8 in_game;
    u8 remember_pass;
    u8 login_sent;                  /* DAT_0461bff4 */

    /* UI state */
    u32 login_window;               /* DAT_0455ef48: Login window widget */
    u32 button_login;               /* DAT_004b8004: Login button sprite */
    u32 button_cancel;              /* DAT_004b8008: Cancel button sprite */
    u32 input_focus;                /* 0 = username, 1 = password */
    u32 first_render;               /* DAT_0455ef44: First render flag */
    u32 server_player_count;        /* DAT_04ebffdc: Players online */

    /* Server name for display */
    char server_name[32];           /* DAT_004b800c: Server name string */

    /* Receive buffer - DAT_0461bb60 region */
    u8 recv_buffer[1024];
    u32 recv_len;
    u8 first_byte;                  /* DAT_0461bb60 - 'A', 'E', or 'L' */

} LoginContext;

/* Global login context */
extern LoginContext g_login;

/* Initialization */
int login_init(void);
void login_shutdown(void);

/* Connection - FUN_0045f4d0 pattern */
int login_connect(const char* host, u16 port);
void login_disconnect(void);
int login_is_connected(void);

/* Authentication - FUN_0043bb10 pattern */
int login_auth(const char* username, const char* password);
int login_auto_auth(void);

/* State machine */
void login_update(void);
int login_process_connect(void);
int login_process_handshake(void);
int login_process_login(void);

/* Encryption functions moved to login_crypto.h */

/* Server selection */
void login_request_servers(void);
void login_handle_servers(void* data, u32 size);
int login_select_server(int index);
ServerInfo* login_get_server(int index);
int login_get_server_count(void);

/* Character management */
void login_request_characters(void);
void login_handle_characters(void* data, u32 size);
int login_select_character(int index);
int login_create_character(const char* name, u8 face, u8 body, u16 job);
int login_delete_character(int index);
CharacterInfo* login_get_character(int index);
CharacterInfo* login_get_selected_character(void);
int login_get_character_count(void);

/* Game entry */
int login_enter_game(void);
int login_logout(void);
int login_return_to_char_select(void);

/* State queries */
LoginState login_get_state(void);
const char* login_get_error_message(void);
int login_is_in_game(void);

/* Packet handling */
void login_handle_packet(void* data, u32 size);
void login_handle_result(void* data, u32 size);
void login_handle_create_result(void* data, u32 size);
void login_handle_delete_result(void* data, u32 size);
void login_handle_enter_game(void* data, u32 size);

/* Text protocol handlers - FUN_0045fa40 pattern */
void login_handle_text_result(const char* result);
void login_handle_create_text_result(const char* result, const char* name);
void login_handle_delete_text_result(const char* result, const char* name);
void login_handle_char_login_result(const char* result, const char* name);
void login_handle_char_list_text(const char* param1, const char* param2);

/* Login screen handler - FUN_00420590 */
void login_screen_init(void);
void login_screen_update(void);
void login_screen_render(void);
int login_screen_handle_input(void);

/* Text input handling - FUN_00420fb0, FUN_0041d890 */
void login_init_text_input(TextInputBox* box, int x, int y, int max_len, int is_password, int color);
void login_update_text_input(TextInputBox* box);
void login_render_text_input(TextInputBox* box);

/* Login check - FUN_0045ef60 */
int login_check_connection(void);

#endif /* LOGIN_H */
