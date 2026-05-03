/*
 * Stone Age Client - Text Protocol Message Log
 * Message log circular buffer management
 * Split from text_protocol.c - FUN_00464ef0
 */

#include <string.h>
#include "types.h"
#include "text_protocol_internal.h"

/*
 * Initialize message log system
 */
void message_log_init(void) {
    memset(s_chat_log, 0, sizeof(s_chat_log));
    memset(s_message_log, 0, sizeof(s_message_log));
    s_chat_log_index = 0;
    s_message_log_index = 0;
    s_update_flag = 0;
    s_ack_flag = 0;
    s_ack_value = 0;
}

/*
 * Add message to chat log
 */
void message_log_chat(const char* message) {
    size_t len;
    u32 idx;

    if (!message || !message[0]) return;

    len = strlen(message);
    if (len >= MSG_LOG_SIZE) len = MSG_LOG_SIZE - 1;

    idx = s_chat_log_index;
    memcpy(s_chat_log[idx], message, len);
    s_chat_log[idx][len] = '\0';
    s_chat_log_index = (s_chat_log_index + 1) & 3;
}

/*
 * Add message to system message log
 */
void message_log_system(const char* message) {
    size_t len;
    u32 idx;

    if (!message || !message[0]) return;

    len = strlen(message);
    if (len >= MSG_LOG_SIZE) len = MSG_LOG_SIZE - 1;

    idx = s_message_log_index;
    memcpy(s_message_log[idx], message, len);
    s_message_log[idx][len] = '\0';
    s_message_log_index = (s_message_log_index + 1) & 3;
}
