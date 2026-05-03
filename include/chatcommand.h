/*
 * Stone Age Client - Chat Command System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef CHATCOMMAND_H
#define CHATCOMMAND_H

#include "types.h"

/* Constants */
#define MAX_CHAT_COMMANDS      64
#define MAX_FILTER_WORDS       100

/* Command results */
typedef enum {
    CHAT_RESULT_SUCCESS = 0,
    CHAT_RESULT_ERROR = 1,
    CHAT_RESULT_INVALID = 2,
    CHAT_RESULT_NOTFOUND = 3,
    CHAT_RESULT_BADARGS = 4,
    CHAT_RESULT_FILTERED = 5,
    CHAT_RESULT_NORMAL = 6,
    CHAT_RESULT_PERMISSION = 7
} ChatResult;

/* Command handler function type */
typedef int (*CommandHandler)(int argc, char** argv, const char* full_input);

/* Chat command */
typedef struct {
    char command[32];
    CommandHandler handler;
    int min_args;
    char help[64];

} ChatCommand;

/* Chat filter */
typedef struct {
    int enabled;
    char words[MAX_FILTER_WORDS][32];
    int filter_count;
    char replacement_char;

} ChatFilter;

/* Chat command context */
typedef struct {
    /* Commands */
    ChatCommand commands[MAX_CHAT_COMMANDS];
    int command_count;

    /* Filter */
    ChatFilter filter;

    /* State */
    ChatCommand* last_command;
    u32 last_command_time;
    u32 last_message_time;
    char last_whisper_target[24];
    u32 ping_sent;

} ChatCommandContext;

/* Global command context */
extern ChatCommandContext g_chatcmd;

/* Initialization */
int chatcommand_init(void);
void chatcommand_shutdown(void);

/* Process */
int chatcommand_process(const char* input);

/* Command management */
int chatcommand_register(const char* command, CommandHandler handler, int min_args, const char* help);
int chatcommand_unregister(const char* command);
ChatCommand* chatcommand_get(const char* command);
int chatcommand_get_count(void);
ChatCommand* chatcommand_get_by_index(int index);

/* Chat filter */
void chatfilter_init(void);
void chatfilter_shutdown(void);
int chatfilter_load(const char* filename);
int chatfilter_check(char* message);
int chatfilter_add_word(const char* word);
int chatfilter_remove_word(const char* word);
void chatfilter_set_enabled(int enabled);
int chatfilter_is_enabled(void);
int chatfilter_get_word_count(void);

#endif /* CHATCOMMAND_H */
