/*
 * Stone Age Client - Chat System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef CHAT_H
#define CHAT_H

#include "types.h"

/* Constants from sa_9061.exe FUN_004106a0 */
#define MAX_CHAT_MSG        256
#define MAX_CHAT_HISTORY    64      /* 0x4144 / 0x105 = ~64 entries */
#define MAX_CHAT_CHANNELS   10
#define CHAT_HISTORY_ENTRY_SIZE 0x105  /* 261 bytes per entry */
#define CHAT_HISTORY_FILE_SIZE  0x4144 /* 16708 bytes total */
#define CHAT_LOG_FILE       "data\\chathis.dat"
#define CHAT_REG_FILE       "data\\chatreg.dat"

/* Chat types */
typedef enum {
    CHAT_TYPE_NORMAL = 0,       /* Normal chat */
    CHAT_TYPE_WHISPER = 1,      /* Private message */
    CHAT_TYPE_PARTY = 2,        /* Party chat */
    CHAT_TYPE_GUILD = 3,        /* Guild chat */
    CHAT_TYPE_SYSTEM = 4,       /* System message */
    CHAT_TYPE_WORLD = 5,        /* World broadcast */
    CHAT_TYPE_EMOTE = 6,        /* Emote/action */
    CHAT_TYPE_SHOUT = 7,        /* Shout (nearby) */
    CHAT_TYPE_NPC = 8           /* NPC dialog */
} ChatType;

/* Chat channel flags */
typedef enum {
    CHAT_FLAG_NORMAL    = (1 << 0),
    CHAT_FLAG_WHISPER   = (1 << 1),
    CHAT_FLAG_PARTY     = (1 << 2),
    CHAT_FLAG_GUILD     = (1 << 3),
    CHAT_FLAG_SYSTEM    = (1 << 4),
    CHAT_FLAG_WORLD     = (1 << 5),
    CHAT_FLAG_ALL       = 0xFF
} ChatChannelFlags;

/* Chat message entry */
typedef struct {
    ChatType type;
    char sender[24];
    char target[24];    /* For whispers */
    char message[MAX_CHAT_MSG];
    u32 timestamp;
    u32 color;
    u8  read;
    u8  reserved[3];
} ChatMessage;

/* Chat channel */
typedef struct {
    u8 enabled;
    u8 filter_flags;
    char name[16];
    u32 color;
    int msg_count;
    int unread;
} ChatChannel;

/* Chat history */
typedef struct {
    ChatMessage messages[MAX_CHAT_HISTORY];
    int count;
    int head;
    int tail;
} ChatHistory;

/* Chat context */
typedef struct {
    /* History */
    ChatHistory history;

    /* Channels */
    ChatChannel channels[MAX_CHAT_CHANNELS];

    /* Input buffer */
    char input_buffer[MAX_CHAT_MSG];
    int input_pos;
    int input_length;

    /* Chat history for recall */
    char sent_history[20][MAX_CHAT_MSG];
    int sent_history_count;
    int sent_history_pos;

    /* Current target for whisper */
    char whisper_target[24];

    /* Filter settings */
    u8 filter_flags;
    u8 show_timestamp;
    u8 profanity_filter;

    /* UI state */
    u8 visible;
    u8 focused;
    u8 scroll_pos;
    u8 max_lines;

    /* Chat logging */
    u8 logging_enabled;
    char log_path[64];

    /* Quick chat/phrase system */
    char quick_phrases[10][MAX_CHAT_MSG];
    int quick_phrase_count;

} ChatContext;

/* Global chat context */
extern ChatContext g_chat;

/* Initialization */
int chat_init(void);
void chat_shutdown(void);

/* Send messages */
int chat_send(const char* message);
int chat_send_to(ChatType type, const char* target, const char* message);
int chat_whisper(const char* target, const char* message);
int chat_party(const char* message);
int chat_guild(const char* message);
int chat_shout(const char* message);
int chat_emote(const char* action);
int chat_world(const char* message);  /* Requires item */

/* Receive messages */
void chat_receive(ChatType type, const char* sender, const char* target,
                  const char* message, u32 color);
void chat_system_message(const char* format, ...);
void chat_system_color(const char* message, u32 color);

/* Packet handlers */
void chat_handle_message(void* data, u32 size);
void chat_handle_whisper(void* data, u32 size);
void chat_handle_system(void* data, u32 size);

/* History */
void chat_add_history(ChatType type, const char* sender, const char* message);
void chat_clear_history(void);
ChatMessage* chat_get_history(int index);
int chat_get_history_count(void);

/* Filter */
void chat_set_filter(u8 flags);
u8 chat_get_filter(void);
int chat_is_filtered(ChatType type);

/* Channels */
void chat_init_channels(void);
void chat_toggle_channel(int channel);
int chat_is_channel_enabled(int channel);

/* Input */
void chat_input_char(char c);
void chat_input_backspace(void);
void chat_input_clear(void);
void chat_set_input(const char* text);
const char* chat_get_input(void);

/* History recall */
void chat_history_up(void);
void chat_history_down(void);
void chat_history_add(const char* message);

/* Whisper */
void chat_set_whisper_target(const char* name);
const char* chat_get_whisper_target(void);
void chat_reply(void);  /* Reply to last whisper */

/* Quick phrases */
int chat_add_quick_phrase(const char* phrase);
const char* chat_get_quick_phrase(int index);
void chat_use_quick_phrase(int index);

/* Logging */
void chat_enable_logging(int enable);
void chat_load_history(void);
void chat_save_history(void);

/* UI */
void chat_toggle_visibility(void);
void chat_set_visible(int visible);
int chat_is_visible(void);
void chat_set_focused(int focused);
int chat_is_focused(void);

/* Commands */
int chat_process_command(const char* command);
int chat_is_command(const char* message);

/* Utility */
const char* chat_type_to_string(ChatType type);
u32 chat_get_color(ChatType type);
void chat_format_time(u32 timestamp, char* buffer, int size);

#endif /* CHAT_H */
