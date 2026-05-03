/*
 * Stone Age Client - Chat File Operations
 * Split from chat.c
 *
 * Contains chat history file operations:
 * - Load history (FUN_00410750 pattern)
 * - Save history (FUN_004106a0 pattern)
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "chat.h"
#include "logger.h"

/* External chat context */
extern ChatContext g_chat;

/*
 * Load chat history from file - matches FUN_00410750 pattern
 * Reads 0x4144 bytes containing 64 entries of 0x105 bytes each
 */
void chat_load_history(void) {
    FILE* fp;
    u8 buffer[CHAT_HISTORY_ENTRY_SIZE];
    int i;

    fp = fopen(g_chat.log_path, "rb");
    if (!fp) {
        LOG_DEBUG("Chat history file not found, starting fresh");
        return;
    }

    /* Read entire history file - 0x4144 bytes (64 * 0x105) */
    for (i = 0; i < MAX_CHAT_HISTORY; i++) {
        if (fread(buffer, 1, CHAT_HISTORY_ENTRY_SIZE, fp) != CHAT_HISTORY_ENTRY_SIZE) {
            break;
        }

        /* Parse entry - format from FUN_004106a0 */
        ChatMessage* msg = &g_chat.history.messages[i];

        /* Entry format: type(1) + timestamp(4) + sender(24) + target(24) + message(256) + padding */
        msg->type = buffer[0];
        msg->timestamp = *(u32*)(buffer + 1);
        strncpy(msg->sender, (char*)(buffer + 5), 23);
        msg->sender[23] = '\0';
        strncpy(msg->target, (char*)(buffer + 29), 23);
        msg->target[23] = '\0';
        strncpy(msg->message, (char*)(buffer + 53), MAX_CHAT_MSG - 1);
        msg->message[MAX_CHAT_MSG - 1] = '\0';
        msg->color = chat_get_color(msg->type);
        msg->read = 1;

        if (msg->message[0] != '\0') {
            g_chat.history.count++;
        }
    }

    fclose(fp);
    g_chat.history.head = g_chat.history.count;

    LOG_DEBUG("Loaded %d chat history entries", g_chat.history.count);
}

/*
 * Save chat history to file - matches FUN_004106a0 pattern
 * Writes to specific entry position: index * 0x105
 */
void chat_save_history(void) {
    FILE* fp;
    u8 buffer[CHAT_HISTORY_ENTRY_SIZE];
    int i;

    if (!g_chat.logging_enabled) return;

    fp = fopen(g_chat.log_path, "wb");
    if (!fp) {
        LOG_WARN("Cannot create chat history file");
        return;
    }

    /* Write all entries */
    for (i = 0; i < MAX_CHAT_HISTORY; i++) {
        ChatMessage* msg = &g_chat.history.messages[i];

        memset(buffer, 0, CHAT_HISTORY_ENTRY_SIZE);

        /* Pack entry */
        buffer[0] = (u8)msg->type;
        *(u32*)(buffer + 1) = msg->timestamp;
        strncpy((char*)(buffer + 5), msg->sender, 23);
        strncpy((char*)(buffer + 29), msg->target, 23);
        strncpy((char*)(buffer + 53), msg->message, MAX_CHAT_MSG - 1);

        fwrite(buffer, 1, CHAT_HISTORY_ENTRY_SIZE, fp);
    }

    /* Write current index at end - from FUN_004106a0 */
    i = g_chat.history.head;
    fwrite(&i, 4, 1, fp);

    fclose(fp);

    LOG_DEBUG("Saved %d chat history entries", g_chat.history.count);
}

/*
 * Clear chat history
 */
void chat_clear_history(void) {
    memset(&g_chat.history, 0, sizeof(ChatHistory));
}

/*
 * Get history entry
 */
ChatMessage* chat_get_history(int index) {
    if (index < 0 || index >= g_chat.history.count) {
        return NULL;
    }

    /* Calculate actual index in circular buffer */
    int actual = (g_chat.history.head - g_chat.history.count + index + MAX_CHAT_HISTORY) % MAX_CHAT_HISTORY;
    return &g_chat.history.messages[actual];
}

/*
 * Get history count
 */
int chat_get_history_count(void) {
    return g_chat.history.count;
}
