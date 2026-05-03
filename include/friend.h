/*
 * Stone Age Client - Friend/Contact System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef FRIEND_H
#define FRIEND_H

#include "types.h"

/* Constants */
#define MAX_FRIENDS     50
#define MAX_BLOCKED     20

/* Protocol commands - from FUN_00465470 analysis */
#define FRIEND_CMD_LIST         "PF"    /* Request friend list */
#define FRIEND_CMD_ADD          "FA"    /* Add friend */
#define FRIEND_CMD_DELETE       "FD"    /* Delete friend */
#define FRIEND_CMD_WHISPER      "FM"    /* Private message prefix */
#define FRIEND_CMD_BLOCK        "FB"    /* Block player */
#define FRIEND_CMD_UNBLOCK      "FU"    /* Unblock player */

/* Chat command prefixes - from 0x004c44fc, 0x004c451c */
#define CHAT_PREFIX_WHISPER     "/FM"   /* Whisper prefix in chat */
#define CHAT_PREFIX_TELL        "/tell " /* Alternative whisper prefix */
#define CHAT_PREFIX_PARTY       "/P"    /* Party chat prefix */
#define CHAT_PREFIX_GUILD       "/G"    /* Guild chat prefix */

/* Whisper packet format: P|/FM<name> <message> */
#define WHISPER_FORMAT          "P|/FM%s"           /* 0x004c44fc */
#define WHISPER_FORMAT_ALT      "P|/tell %s"        /* 0x004c451c */

/* Friend status flags */
typedef enum {
    FRIEND_STATUS_OFFLINE = 0,
    FRIEND_STATUS_ONLINE = 1,
    FRIEND_STATUS_AWAY = 2,
    FRIEND_STATUS_BUSY = 3
} FriendStatus;

/* Friend entry */
typedef struct {
    u32 id;
    char name[24];
    u16 level;
    u16 job;
    u8  online;
    u8  status;
    u32 last_seen;
    u32 map_id;
} FriendEntry;

/* Blocked entry */
typedef struct {
    u32 id;
    char name[24];
    u32 timestamp;
} BlockedEntry;

/* Friend context */
typedef struct {
    /* Friends */
    FriendEntry friends[MAX_FRIENDS];
    int friend_count;
    int list_loaded;

    /* Block list */
    BlockedEntry blocked[MAX_BLOCKED];
    int block_count;

    /* Current selection */
    int selected_index;

    /* Whisper tracking */
    char last_whisper_from[24];
    char last_whisper_to[24];

    /* Flags */
    int dirty;
    int request_pending;

} FriendContext;

/* Global friend context */
extern FriendContext g_friend;

/* Initialization */
int friend_init(void);
void friend_shutdown(void);

/* Friend list */
void friend_request_list(void);
void friend_handle_list(void* data, u32 size);

/* Add/remove friends */
int friend_add(const char* name);
void friend_handle_add(void* data, u32 size);
int friend_remove(const char* name);
void friend_handle_remove(void* data, u32 size);

/* Status updates */
void friend_handle_status_change(void* data, u32 size);

/* Private messages (whisper) - FM protocol */
int friend_send_whisper(const char* name, const char* message);
void friend_handle_whisper(void* data, u32 size);
int friend_reply_whisper(const char* message);

/* Query */
FriendEntry* friend_find_by_name(const char* name);
FriendEntry* friend_find_by_id(u32 id);
FriendEntry* friend_get_by_index(int index);
int friend_get_online_count(void);
int friend_get_count(void);
int friend_is_full(void);

/* Selection */
void friend_select(int index);
FriendEntry* friend_get_selected(void);

/* Block list */
int block_init(void);
int block_add(const char* name);
int block_remove(const char* name);
int block_is_blocked(const char* name);
BlockedEntry* block_find(const char* name);

#endif /* FRIEND_H */
