/*
 * Stone Age Client - Chat Command System Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "types.h"
#include "chatcommand.h"
#include "friend.h"
#include "network.h"
#include "logger.h"

/* Global command context */
ChatCommandContext g_chatcmd = {0};

/* Forward declarations for built-in commands */
static int cmd_help(int argc, char** argv, const char* full_input);
static int cmd_emote(int argc, char** argv, const char* full_input);
static int cmd_whisper(int argc, char** argv, const char* full_input);
static int cmd_reply(int argc, char** argv, const char* full_input);
static int cmd_party(int argc, char** argv, const char* full_input);
static int cmd_guild(int argc, char** argv, const char* full_input);
static int cmd_trade(int argc, char** argv, const char* full_input);
static int cmd_friend(int argc, char** argv, const char* full_input);
static int cmd_dance(int argc, char** argv, const char* full_input);
static int cmd_sit(int argc, char** argv, const char* full_input);
static int cmd_stand(int argc, char** argv, const char* full_input);
static int cmd_time(int argc, char** argv, const char* full_input);
static int cmd_loc(int argc, char** argv, const char* full_input);
static int cmd_ping(int argc, char** argv, const char* full_input);

/* Built-in commands table */
static const ChatCommand s_builtin_commands[] = {
    { "/help",     cmd_help,    0, "Show available commands" },
    { "/emote",    cmd_emote,   1, "Play emote animation" },
    { "/w",        cmd_whisper, 2, "Whisper to player: /w <name> <message>" },
    { "/whisper",  cmd_whisper, 2, "Whisper to player" },
    { "/r",        cmd_reply,   1, "Reply to last whisper" },
    { "/reply",    cmd_reply,   1, "Reply to last whisper" },
    { "/p",        cmd_party,   1, "Party chat: /p <message>" },
    { "/party",    cmd_party,   1, "Party chat" },
    { "/g",        cmd_guild,   1, "Guild chat: /g <message>" },
    { "/guild",    cmd_guild,   1, "Guild chat" },
    { "/trade",    cmd_trade,   1, "Broadcast trade message" },
    { "/friend",   cmd_friend,  1, "Add friend: /friend <name>" },
    { "/dance",    cmd_dance,   0, "Play dance animation" },
    { "/sit",      cmd_sit,     0, "Sit down" },
    { "/stand",    cmd_stand,   0, "Stand up" },
    { "/time",     cmd_time,    0, "Show current time" },
    { "/loc",      cmd_loc,     0, "Show current location" },
    { "/ping",     cmd_ping,    0, "Test connection" }
};

#define BUILTIN_COMMAND_COUNT (sizeof(s_builtin_commands) / sizeof(s_builtin_commands[0]))

/*
 * Initialize chat command system
 */
int chatcommand_init(void) {
    memset(&g_chatcmd, 0, sizeof(ChatCommandContext));

    /* Register built-in commands */
    memcpy(g_chatcmd.commands, s_builtin_commands, sizeof(s_builtin_commands));
    g_chatcmd.command_count = BUILTIN_COMMAND_COUNT;

    /* Initialize filter */
    chatfilter_init();

    LOG_INFO("Chat command system initialized");
    return 1;
}

/*
 * Shutdown chat command system
 */
void chatcommand_shutdown(void) {
    chatfilter_shutdown();
    memset(&g_chatcmd, 0, sizeof(ChatCommandContext));
    LOG_INFO("Chat command system shutdown");
}

/*
 * Process chat input
 */
int chatcommand_process(const char* input) {
    char buffer[512];
    char* argv[16];
    int argc;
    char* token;
    ChatCommand* cmd;
    int i;

    if (!input || input[0] == '\0') {
        return 0;
    }

    /* Copy input to buffer for parsing */
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    /* Check if it's a command */
    if (buffer[0] != '/') {
        /* Not a command, check filter and send as normal chat */
        if (chatfilter_check(buffer)) {
            g_chatcmd.last_message_time = timeGetTime();
            return CHAT_RESULT_NORMAL;
        } else {
            return CHAT_RESULT_FILTERED;
        }
    }

    /* Parse command and arguments */
    argc = 0;
    token = strtok(buffer, " ");
    while (token && argc < 16) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (argc == 0) {
        return CHAT_RESULT_INVALID;
    }

    /* Find command */
    cmd = NULL;
    for (i = 0; i < g_chatcmd.command_count; i++) {
        if (strcmp(argv[0], g_chatcmd.commands[i].command) == 0) {
            cmd = &g_chatcmd.commands[i];
            break;
        }
    }

    if (!cmd) {
        return CHAT_RESULT_NOTFOUND;
    }

    /* Check argument count */
    if (cmd->min_args > 0 && argc - 1 < cmd->min_args) {
        return CHAT_RESULT_BADARGS;
    }

    /* Execute command */
    g_chatcmd.last_command = cmd;
    g_chatcmd.last_command_time = timeGetTime();

    return cmd->handler(argc, argv, input);
}

/*
 * Register custom command
 */
int chatcommand_register(const char* command, CommandHandler handler, int min_args, const char* help) {
    ChatCommand* cmd;

    if (!command || !handler) {
        return 0;
    }

    if (g_chatcmd.command_count >= MAX_CHAT_COMMANDS) {
        return 0;
    }

    cmd = &g_chatcmd.commands[g_chatcmd.command_count];
    strncpy(cmd->command, command, 31);
    cmd->command[31] = '\0';
    cmd->handler = handler;
    cmd->min_args = min_args;
    strncpy(cmd->help, help ? help : "", 63);
    cmd->help[63] = '\0';

    g_chatcmd.command_count++;
    return 1;
}

/*
 * Unregister command
 */
int chatcommand_unregister(const char* command) {
    int i, j;

    for (i = 0; i < g_chatcmd.command_count; i++) {
        if (strcmp(g_chatcmd.commands[i].command, command) == 0) {
            for (j = i; j < g_chatcmd.command_count - 1; j++) {
                g_chatcmd.commands[j] = g_chatcmd.commands[j + 1];
            }
            g_chatcmd.command_count--;
            return 1;
        }
    }

    return 0;
}

/*
 * Get command by name
 */
ChatCommand* chatcommand_get(const char* command) {
    int i;

    for (i = 0; i < g_chatcmd.command_count; i++) {
        if (strcmp(g_chatcmd.commands[i].command, command) == 0) {
            return &g_chatcmd.commands[i];
        }
    }

    return NULL;
}

/*
 * Get command count
 */
int chatcommand_get_count(void) {
    return g_chatcmd.command_count;
}

/*
 * Get command by index
 */
ChatCommand* chatcommand_get_by_index(int index) {
    if (index < 0 || index >= g_chatcmd.command_count) {
        return NULL;
    }
    return &g_chatcmd.commands[index];
}

/*
 * Initialize chat filter
 */
void chatfilter_init(void) {
    memset(&g_chatcmd.filter, 0, sizeof(ChatFilter));

    /* Add default bad words (would be loaded from file in real implementation) */
    g_chatcmd.filter.enabled = 1;
    g_chatcmd.filter.filter_count = 0;
    g_chatcmd.filter.replacement_char = '*';
}

/*
 * Shutdown chat filter
 */
void chatfilter_shutdown(void) {
    g_chatcmd.filter.filter_count = 0;
}

/*
 * Load filter words from file
 */
int chatfilter_load(const char* filename) {
    FILE* fp;
    char line[64];

    fp = fopen(filename, "r");
    if (!fp) {
        return 0;
    }

    g_chatcmd.filter.filter_count = 0;
    while (fgets(line, sizeof(line), fp) && g_chatcmd.filter.filter_count < MAX_FILTER_WORDS) {
        /* Remove newline */
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] != '\0' && line[0] != '#') {
            strncpy(g_chatcmd.filter.words[g_chatcmd.filter.filter_count], line, 31);
            g_chatcmd.filter.words[g_chatcmd.filter.filter_count][31] = '\0';
            g_chatcmd.filter.filter_count++;
        }
    }

    fclose(fp);
    return 1;
}

/*
 * Check and filter message
 */
int chatfilter_check(char* message) {
    int i, j, k;
    int len;
    int word_len;
    int filtered = 0;

    if (!g_chatcmd.filter.enabled || !message) {
        return 1;
    }

    len = strlen(message);

    for (i = 0; i < g_chatcmd.filter.filter_count; i++) {
        word_len = strlen(g_chatcmd.filter.words[i]);

        for (j = 0; j <= len - word_len; j++) {
            int match = 1;
            for (k = 0; k < word_len; k++) {
                if (tolower(message[j + k]) != tolower(g_chatcmd.filter.words[i][k])) {
                    match = 0;
                    break;
                }
            }

            if (match) {
                /* Replace with asterisks */
                for (k = 0; k < word_len; k++) {
                    message[j + k] = g_chatcmd.filter.replacement_char;
                }
                filtered = 1;
                j += word_len - 1;
            }
        }
    }

    return !filtered;
}

/*
 * Add filter word
 */
int chatfilter_add_word(const char* word) {
    if (!word || g_chatcmd.filter.filter_count >= MAX_FILTER_WORDS) {
        return 0;
    }

    strncpy(g_chatcmd.filter.words[g_chatcmd.filter.filter_count], word, 31);
    g_chatcmd.filter.words[g_chatcmd.filter.filter_count][31] = '\0';
    g_chatcmd.filter.filter_count++;

    return 1;
}

/*
 * Remove filter word
 */
int chatfilter_remove_word(const char* word) {
    int i, j;

    for (i = 0; i < g_chatcmd.filter.filter_count; i++) {
        if (strcmp(g_chatcmd.filter.words[i], word) == 0) {
            for (j = i; j < g_chatcmd.filter.filter_count - 1; j++) {
                strcpy(g_chatcmd.filter.words[j], g_chatcmd.filter.words[j + 1]);
            }
            g_chatcmd.filter.filter_count--;
            return 1;
        }
    }

    return 0;
}

/*
 * Enable/disable filter
 */
void chatfilter_set_enabled(int enabled) {
    g_chatcmd.filter.enabled = enabled;
}

/*
 * Check if filter enabled
 */
int chatfilter_is_enabled(void) {
    return g_chatcmd.filter.enabled;
}

/*
 * Get filter word count
 */
int chatfilter_get_word_count(void) {
    return g_chatcmd.filter.filter_count;
}

/* Built-in command implementations */

static int cmd_help(int argc, char** argv, const char* full_input) {
    int i;
    LOG_INFO("=== Available Commands ===");
    for (i = 0; i < g_chatcmd.command_count; i++) {
        LOG_INFO("%s - %s", g_chatcmd.commands[i].command, g_chatcmd.commands[i].help);
    }
    return CHAT_RESULT_SUCCESS;
}

static int cmd_emote(int argc, char** argv, const char* full_input) {
    /* Would trigger emote animation */
    LOG_DEBUG("Emote: %s", argc > 1 ? argv[1] : "wave");
    return CHAT_RESULT_SUCCESS;
}

static int cmd_whisper(int argc, char** argv, const char* full_input) {
    char message[256];
    int i;

    if (argc < 3) {
        return CHAT_RESULT_BADARGS;
    }

    /* Reconstruct message from remaining args */
    message[0] = '\0';
    for (i = 2; i < argc; i++) {
        if (i > 2) strcat(message, " ");
        strncat(message, argv[i], sizeof(message) - strlen(message) - 1);
    }

    /* Send whisper via friend system */
    if (friend_send_whisper(argv[1], message)) {
        strncpy(g_chatcmd.last_whisper_target, argv[1], 23);
        g_chatcmd.last_whisper_target[23] = '\0';
        return CHAT_RESULT_SUCCESS;
    }
    return CHAT_RESULT_ERROR;
}

static int cmd_reply(int argc, char** argv, const char* full_input) {
    char message[256];
    int i;

    if (argc < 2) {
        return CHAT_RESULT_BADARGS;
    }

    /* Reconstruct message from remaining args */
    message[0] = '\0';
    for (i = 1; i < argc; i++) {
        if (i > 1) strcat(message, " ");
        strncat(message, argv[i], sizeof(message) - strlen(message) - 1);
    }

    /* Reply via friend system */
    if (friend_reply_whisper(message)) {
        return CHAT_RESULT_SUCCESS;
    }
    return CHAT_RESULT_ERROR;
}

static int cmd_party(int argc, char** argv, const char* full_input) {
    char packet[512];
    char message[256];
    int i;

    if (argc < 2) {
        return CHAT_RESULT_BADARGS;
    }

    /* Reconstruct message */
    message[0] = '\0';
    for (i = 1; i < argc; i++) {
        if (i > 1) strcat(message, " ");
        strncat(message, argv[i], sizeof(message) - strlen(message) - 1);
    }

    /* Send party chat packet: P|/P<message> */
    _snprintf(packet, sizeof(packet), "P|/P%s", message);
    network_send(packet, strlen(packet));
    network_send("\n", 1);

    return CHAT_RESULT_SUCCESS;
}

static int cmd_guild(int argc, char** argv, const char* full_input) {
    char packet[512];
    char message[256];
    int i;

    if (argc < 2) {
        return CHAT_RESULT_BADARGS;
    }

    /* Reconstruct message */
    message[0] = '\0';
    for (i = 1; i < argc; i++) {
        if (i > 1) strcat(message, " ");
        strncat(message, argv[i], sizeof(message) - strlen(message) - 1);
    }

    /* Send guild chat packet: P|/G<message> */
    _snprintf(packet, sizeof(packet), "P|/G%s", message);
    network_send(packet, strlen(packet));
    network_send("\n", 1);

    return CHAT_RESULT_SUCCESS;
}

static int cmd_trade(int argc, char** argv, const char* full_input) {
    char packet[512];
    char message[256];
    int i;

    if (argc < 2) {
        return CHAT_RESULT_BADARGS;
    }

    /* Reconstruct message */
    message[0] = '\0';
    for (i = 1; i < argc; i++) {
        if (i > 1) strcat(message, " ");
        strncat(message, argv[i], sizeof(message) - strlen(message) - 1);
    }

    /* Send trade broadcast packet: P|/T<message> */
    _snprintf(packet, sizeof(packet), "P|/T%s", message);
    network_send(packet, strlen(packet));
    network_send("\n", 1);

    return CHAT_RESULT_SUCCESS;
}

static int cmd_friend(int argc, char** argv, const char* full_input) {
    if (argc < 2) {
        return CHAT_RESULT_BADARGS;
    }

    /* Send friend add request via friend system */
    if (friend_add(argv[1])) {
        return CHAT_RESULT_SUCCESS;
    }
    return CHAT_RESULT_ERROR;
}

static int cmd_dance(int argc, char** argv, const char* full_input) {
    /* Would trigger dance animation */
    LOG_DEBUG("Dancing");
    return CHAT_RESULT_SUCCESS;
}

static int cmd_sit(int argc, char** argv, const char* full_input) {
    /* Would make character sit */
    LOG_DEBUG("Sitting");
    return CHAT_RESULT_SUCCESS;
}

static int cmd_stand(int argc, char** argv, const char* full_input) {
    /* Would make character stand */
    LOG_DEBUG("Standing");
    return CHAT_RESULT_SUCCESS;
}

static int cmd_time(int argc, char** argv, const char* full_input) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    LOG_INFO("Current time: %02d:%02d:%02d", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    return CHAT_RESULT_SUCCESS;
}

static int cmd_loc(int argc, char** argv, const char* full_input) {
    /* Would show current map location */
    LOG_DEBUG("Location query");
    return CHAT_RESULT_SUCCESS;
}

static int cmd_ping(int argc, char** argv, const char* full_input) {
    g_chatcmd.ping_sent = timeGetTime();
    /* Would send ping packet */
    LOG_DEBUG("Ping sent");
    return CHAT_RESULT_SUCCESS;
}
