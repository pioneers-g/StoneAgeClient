/*
 * Stone Age Client - Chat System Core
 * Reverse engineered from sa_9061.exe
 *
 * Core chat functionality:
 * - Initialization and shutdown
 * - Message sending functions
 * - Input handling
 * - Utility functions
 *
 * Protocol handlers: chat_protocol.c
 * File operations: chat_file.c
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "types.h"
#include "chat.h"
#include "network_protocol.h"
#include "logger.h"

/* Global chat context */
ChatContext g_chat = {0};

/* Chat type colors (16-bit 565 format) */
const u32 s_chat_colors[] = {
    0xFFFF,     /* CHAT_TYPE_NORMAL - White */
    0xFFE0,     /* CHAT_TYPE_WHISPER - Yellow */
    0x07FF,     /* CHAT_TYPE_PARTY - Cyan */
    0x001F,     /* CHAT_TYPE_GUILD - Blue */
    0xF800,     /* CHAT_TYPE_SYSTEM - Red */
    0xF81F,     /* CHAT_TYPE_WORLD - Magenta */
    0x07E0,     /* CHAT_TYPE_EMOTE - Green */
    0xFF80,     /* CHAT_TYPE_SHOUT - Orange */
    0xFFFF      /* CHAT_TYPE_NPC - White */
};

/* Chat type command prefixes */
static const char* s_chat_prefixes[] = {
    "",         /* CHAT_TYPE_NORMAL */
    "/w ",      /* CHAT_TYPE_WHISPER */
    "/p ",      /* CHAT_TYPE_PARTY */
    "/g ",      /* CHAT_TYPE_GUILD */
    "",         /* CHAT_TYPE_SYSTEM */
    "/world ",  /* CHAT_TYPE_WORLD */
    "/me ",     /* CHAT_TYPE_EMOTE */
    "/s ",      /* CHAT_TYPE_SHOUT */
    ""          /* CHAT_TYPE_NPC */
};

/*
 * Initialize chat system
 */
int chat_init(void) {
    memset(&g_chat, 0, sizeof(ChatContext));

    g_chat.filter_flags = CHAT_FLAG_ALL;
    g_chat.visible = 1;
    g_chat.max_lines = 8;
    g_chat.logging_enabled = 1;
    strcpy(g_chat.log_path, CHAT_LOG_FILE);

    chat_init_channels();
    chat_load_history();

    LOG_INFO("Chat system initialized");
    return 1;
}

/*
 * Shutdown chat system
 */
void chat_shutdown(void) {
    chat_save_history();
    memset(&g_chat, 0, sizeof(ChatContext));
    LOG_INFO("Chat system shutdown");
}

/*
 * Initialize chat channels
 */
void chat_init_channels(void) {
    int i;

    for (i = 0; i < MAX_CHAT_CHANNELS; i++) {
        g_chat.channels[i].enabled = 1;
        g_chat.channels[i].filter_flags = (1 << i);
        g_chat.channels[i].color = s_chat_colors[i];
        g_chat.channels[i].msg_count = 0;
        g_chat.channels[i].unread = 0;
    }

    strcpy(g_chat.channels[0].name, "Normal");
    strcpy(g_chat.channels[1].name, "Whisper");
    strcpy(g_chat.channels[2].name, "Party");
    strcpy(g_chat.channels[3].name, "Guild");
    strcpy(g_chat.channels[4].name, "System");
    strcpy(g_chat.channels[5].name, "World");
    strcpy(g_chat.channels[6].name, "Emote");
    strcpy(g_chat.channels[7].name, "Shout");
}

/*
 * Send normal chat message
 */
int chat_send(const char* message) {
    if (!message || strlen(message) == 0) {
        return 0;
    }

    if (message[0] == '/' || message[0] == '@') {
        return chat_process_command(message);
    }

    return chat_send_to(CHAT_TYPE_NORMAL, NULL, message);
}

/*
 * Send chat message with specific type
 */
int chat_send_to(ChatType type, const char* target, const char* message) {
    char packet[512];
    int len;

    if (!message || strlen(message) == 0) {
        return 0;
    }

    if (strlen(message) > MAX_CHAT_MSG - 1) {
        LOG_WARN("Message too long");
        return 0;
    }

    switch (type) {
        case CHAT_TYPE_NORMAL:
            len = snprintf(packet, sizeof(packet), "C%s\n", message);
            break;
        case CHAT_TYPE_WHISPER:
            if (!target || strlen(target) == 0) {
                target = g_chat.whisper_target;
            }
            if (!target || strlen(target) == 0) {
                LOG_WARN("No whisper target");
                return 0;
            }
            len = snprintf(packet, sizeof(packet), "W%s %s\n", target, message);
            break;
        case CHAT_TYPE_PARTY:
            len = snprintf(packet, sizeof(packet), "P%s\n", message);
            break;
        case CHAT_TYPE_GUILD:
            len = snprintf(packet, sizeof(packet), "G%s\n", message);
            break;
        case CHAT_TYPE_SHOUT:
            len = snprintf(packet, sizeof(packet), "S%s\n", message);
            break;
        case CHAT_TYPE_EMOTE:
            len = snprintf(packet, sizeof(packet), "E%s\n", message);
            break;
        case CHAT_TYPE_WORLD:
            len = snprintf(packet, sizeof(packet), "T%s\n", message);
            break;
        default:
            return 0;
    }

    send_queue_add(packet, 0, 0);
    chat_history_add(message);

    LOG_DEBUG("Sent chat [%d]: %s", type, message);
    return 1;
}

/*
 * Send whisper
 */
int chat_whisper(const char* target, const char* message) {
    return chat_send_to(CHAT_TYPE_WHISPER, target, message);
}

/*
 * Send party chat
 */
int chat_party(const char* message) {
    return chat_send_to(CHAT_TYPE_PARTY, NULL, message);
}

/*
 * Send guild chat
 */
int chat_guild(const char* message) {
    return chat_send_to(CHAT_TYPE_GUILD, NULL, message);
}

/*
 * Send shout
 */
int chat_shout(const char* message) {
    return chat_send_to(CHAT_TYPE_SHOUT, NULL, message);
}

/*
 * Send emote
 */
int chat_emote(const char* action) {
    return chat_send_to(CHAT_TYPE_EMOTE, NULL, action);
}

/*
 * Send world chat
 */
int chat_world(const char* message) {
    return chat_send_to(CHAT_TYPE_WORLD, NULL, message);
}

/*
 * Receive chat message
 */
void chat_receive(ChatType type, const char* sender, const char* target,
                  const char* message, u32 color) {
    if (chat_is_filtered(type)) {
        return;
    }

    chat_add_history(type, sender, message);

    if (type < MAX_CHAT_CHANNELS) {
        g_chat.channels[type].unread++;
    }

    LOG_DEBUG("Chat [%d] %s: %s", type, sender, message);
}

/*
 * System message
 */
void chat_system_message(const char* format, ...) {
    char buffer[MAX_CHAT_MSG];
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    chat_receive(CHAT_TYPE_SYSTEM, "System", NULL, buffer, s_chat_colors[CHAT_TYPE_SYSTEM]);
}

/*
 * System message with color
 */
void chat_system_color(const char* message, u32 color) {
    chat_receive(CHAT_TYPE_SYSTEM, "System", NULL, message, color);
}

/*
 * Add message to history
 */
void chat_add_history(ChatType type, const char* sender, const char* message) {
    ChatMessage* entry;

    entry = &g_chat.history.messages[g_chat.history.head];
    g_chat.history.head = (g_chat.history.head + 1) % MAX_CHAT_HISTORY;

    if (g_chat.history.count < MAX_CHAT_HISTORY) {
        g_chat.history.count++;
    }

    entry->type = type;
    entry->timestamp = timeGetTime();
    entry->color = chat_get_color(type);
    entry->read = 0;
    strncpy(entry->sender, sender, sizeof(entry->sender) - 1);
    strncpy(entry->message, message, sizeof(entry->message) - 1);
}

/*
 * Set filter flags
 */
void chat_set_filter(u8 flags) {
    g_chat.filter_flags = flags;
}

/*
 * Get filter flags
 */
u8 chat_get_filter(void) {
    return g_chat.filter_flags;
}

/*
 * Check if chat type is filtered
 */
int chat_is_filtered(ChatType type) {
    if (type >= MAX_CHAT_CHANNELS) {
        return 0;
    }
    return !(g_chat.filter_flags & (1 << type));
}

/*
 * Toggle channel
 */
void chat_toggle_channel(int channel) {
    if (channel < 0 || channel >= MAX_CHAT_CHANNELS) {
        return;
    }
    g_chat.channels[channel].enabled = !g_chat.channels[channel].enabled;
}

/*
 * Check if channel is enabled
 */
int chat_is_channel_enabled(int channel) {
    if (channel < 0 || channel >= MAX_CHAT_CHANNELS) {
        return 0;
    }
    return g_chat.channels[channel].enabled;
}

/*
 * Input character
 */
void chat_input_char(char c) {
    if (g_chat.input_length >= MAX_CHAT_MSG - 1) {
        return;
    }
    g_chat.input_buffer[g_chat.input_pos] = c;
    g_chat.input_pos++;
    g_chat.input_length++;
    g_chat.input_buffer[g_chat.input_length] = '\0';
}

/*
 * Backspace
 */
void chat_input_backspace(void) {
    if (g_chat.input_pos > 0) {
        g_chat.input_pos--;
        g_chat.input_length--;
        g_chat.input_buffer[g_chat.input_length] = '\0';
    }
}

/*
 * Clear input
 */
void chat_input_clear(void) {
    memset(g_chat.input_buffer, 0, sizeof(g_chat.input_buffer));
    g_chat.input_pos = 0;
    g_chat.input_length = 0;
}

/*
 * Set input text
 */
void chat_set_input(const char* text) {
    if (!text) return;
    strncpy(g_chat.input_buffer, text, MAX_CHAT_MSG - 1);
    g_chat.input_length = strlen(g_chat.input_buffer);
    g_chat.input_pos = g_chat.input_length;
}

/*
 * Get input text
 */
const char* chat_get_input(void) {
    return g_chat.input_buffer;
}

/*
 * History up
 */
void chat_history_up(void) {
    if (g_chat.sent_history_count == 0) return;
    g_chat.sent_history_pos--;
    if (g_chat.sent_history_pos < 0) {
        g_chat.sent_history_pos = g_chat.sent_history_count - 1;
    }
    chat_set_input(g_chat.sent_history[g_chat.sent_history_pos]);
}

/*
 * History down
 */
void chat_history_down(void) {
    if (g_chat.sent_history_count == 0) return;
    g_chat.sent_history_pos++;
    if (g_chat.sent_history_pos >= g_chat.sent_history_count) {
        g_chat.sent_history_pos = 0;
    }
    chat_set_input(g_chat.sent_history[g_chat.sent_history_pos]);
}

/*
 * Add to sent history
 */
void chat_history_add(const char* message) {
    if (g_chat.sent_history_count < 20) {
        strncpy(g_chat.sent_history[g_chat.sent_history_count], message, MAX_CHAT_MSG - 1);
        g_chat.sent_history_count++;
    } else {
        memmove(g_chat.sent_history[0], g_chat.sent_history[1], 19 * MAX_CHAT_MSG);
        strncpy(g_chat.sent_history[19], message, MAX_CHAT_MSG - 1);
    }
    g_chat.sent_history_pos = g_chat.sent_history_count;
}

/*
 * Set whisper target
 */
void chat_set_whisper_target(const char* name) {
    strncpy(g_chat.whisper_target, name, sizeof(g_chat.whisper_target) - 1);
}

/*
 * Get whisper target
 */
const char* chat_get_whisper_target(void) {
    return g_chat.whisper_target;
}

/*
 * Reply to last whisper
 */
void chat_reply(void) {
    char buffer[32];
    if (g_chat.whisper_target[0] == '\0') {
        chat_system_message("No one to reply to");
        return;
    }
    snprintf(buffer, sizeof(buffer), "/w %s ", g_chat.whisper_target);
    chat_set_input(buffer);
}

/*
 * Add quick phrase
 */
int chat_add_quick_phrase(const char* phrase) {
    if (g_chat.quick_phrase_count >= 10) {
        return 0;
    }
    strncpy(g_chat.quick_phrases[g_chat.quick_phrase_count], phrase, MAX_CHAT_MSG - 1);
    g_chat.quick_phrase_count++;
    return 1;
}

/*
 * Get quick phrase
 */
const char* chat_get_quick_phrase(int index) {
    if (index < 0 || index >= g_chat.quick_phrase_count) {
        return NULL;
    }
    return g_chat.quick_phrases[index];
}

/*
 * Use quick phrase
 */
void chat_use_quick_phrase(int index) {
    const char* phrase = chat_get_quick_phrase(index);
    if (phrase) {
        chat_send(phrase);
    }
}

/*
 * Enable logging
 */
void chat_enable_logging(int enable) {
    g_chat.logging_enabled = enable;
}

/*
 * Toggle visibility
 */
void chat_toggle_visibility(void) {
    g_chat.visible = !g_chat.visible;
}

/*
 * Set visible
 */
void chat_set_visible(int visible) {
    g_chat.visible = visible;
}

/*
 * Is visible
 */
int chat_is_visible(void) {
    return g_chat.visible;
}

/*
 * Set focused
 */
void chat_set_focused(int focused) {
    g_chat.focused = focused;
}

/*
 * Is focused
 */
int chat_is_focused(void) {
    return g_chat.focused;
}

/*
 * Process command
 */
int chat_process_command(const char* command) {
    char target[24];
    const char* msg;

    if (!command || command[0] != '/') {
        return 0;
    }

    if (strncmp(command, "/w ", 3) == 0 || strncmp(command, "/whisper ", 9) == 0) {
        msg = strchr(command + 3, ' ');
        if (msg) {
            int target_len = msg - (command + 3);
            strncpy(target, command + 3, target_len);
            target[target_len] = '\0';
            return chat_whisper(target, msg + 1);
        }
    }
    else if (strncmp(command, "/r", 2) == 0 || strncmp(command, "/reply", 6) == 0) {
        chat_reply();
        return 1;
    }
    else if (strncmp(command, "/p ", 3) == 0 || strncmp(command, "/party ", 7) == 0) {
        return chat_party(command + 3);
    }
    else if (strncmp(command, "/g ", 3) == 0 || strncmp(command, "/guild ", 7) == 0) {
        return chat_guild(command + 3);
    }
    else if (strncmp(command, "/s ", 3) == 0 || strncmp(command, "/shout ", 7) == 0) {
        return chat_shout(command + 3);
    }
    else if (strncmp(command, "/me ", 4) == 0) {
        return chat_emote(command + 4);
    }
    else if (strncmp(command, "/world ", 7) == 0) {
        return chat_world(command + 7);
    }

    return chat_send_to(CHAT_TYPE_NORMAL, NULL, command);
}

/*
 * Check if message is a command
 */
int chat_is_command(const char* message) {
    return message && message[0] == '/';
}

/*
 * Get color for chat type
 */
u32 chat_get_color(ChatType type) {
    if (type < sizeof(s_chat_colors) / sizeof(s_chat_colors[0])) {
        return s_chat_colors[type];
    }
    return s_chat_colors[0];
}

/*
 * Chat type to string
 */
const char* chat_type_to_string(ChatType type) {
    static const char* type_names[] = {
        "Normal", "Whisper", "Party", "Guild",
        "System", "World", "Emote", "Shout", "NPC"
    };

    if (type < sizeof(type_names) / sizeof(type_names[0])) {
        return type_names[type];
    }
    return "Unknown";
}

/*
 * Format timestamp
 */
void chat_format_time(u32 timestamp, char* buffer, int size) {
    time_t t = timestamp / 1000;
    struct tm* tm_info = localtime(&t);
    strftime(buffer, size, "%H:%M:%S", tm_info);
}

/*
 * Add message to chat display (compatibility function)
 */
void chat_add_message(const char* sender, const char* message, ChatType type, u32 param) {
    chat_add_history(type, sender, message);
}
