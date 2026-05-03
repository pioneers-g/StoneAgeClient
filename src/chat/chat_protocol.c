/*
 * Stone Age Client - Chat Protocol Handlers
 * Split from chat.c
 *
 * Contains packet handlers for chat messages:
 * - XYF dispatcher (FUN_00464ef0)
 * - Chat message receive
 * - Whisper handling
 * - System messages
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

/* External color array */
extern const u32 s_chat_colors[];

/* Chat circular buffers from FUN_00464ef0:
 * DAT_004d906c + index * 0x1000 - Main chat buffer (4 slots)
 * DAT_004dd074 + index * 0x1000 - Party chat buffer (4 slots)
 * DAT_004e2118 - Main buffer index (0-3)
 * DAT_004e2138 - Party buffer index (0-3)
 */
static char s_chat_buffer[4][4096];
static int s_chat_buffer_index = 0;

static char s_party_buffer[4][4096];
static int s_party_buffer_index = 0;

/* External function declarations */
extern void FUN_00492cd2(const char* msg, const char* fmt, void* arg1, void* arg2, void* arg3);

/*
 * Handle XYF chat packet - FUN_00464ef0
 *
 * Format: XYF<type><message>
 * Types:
 *   C - Normal chat (stores to main buffer at DAT_004dd074)
 *   P - Party chat (stores to party buffer at DAT_004d906c)
 *   A - Acknowledge (sends format response)
 *   U - Update flag
 *   Other - Direct storage to main buffer
 */
void chat_handle_xyf_packet(const char* packet_data) {
    char type;
    const char* message;
    size_t msg_len;
    char* dest_buffer;
    int* buffer_index;

    if (!packet_data || packet_data[0] == '\0') {
        return;
    }

    /* Get message type from position 1 */
    type = packet_data[1];
    message = packet_data + 3;  /* Skip "XYF" prefix */

    switch (type) {
        case 'C':
            /* Normal chat - store to main buffer */
            msg_len = strlen(message);
            dest_buffer = s_chat_buffer[s_chat_buffer_index];
            memcpy(dest_buffer, message, msg_len + 1);
            s_chat_buffer_index = (s_chat_buffer_index + 1) & 3;

            /* Also add to chat history */
            chat_add_history(CHAT_TYPE_NORMAL, "Player", message);
            LOG_DEBUG("Chat: %s", message);
            break;

        case 'P':
            /* Party chat */
            msg_len = strlen(message);
            dest_buffer = s_party_buffer[s_party_buffer_index];
            memcpy(dest_buffer, message, msg_len + 1);
            s_party_buffer_index = (s_party_buffer_index + 1) & 3;

            chat_add_history(CHAT_TYPE_PARTY, "Party", message);
            LOG_DEBUG("Party: %s", message);
            break;

        case 'A':
            /* Acknowledge - send format response */
            FUN_00492cd2(message, "%s %s %s", NULL, NULL, NULL);
            /* TODO: Handle DAT_004e2144 flag logic from binary */
            break;

        case 'U':
            /* Update flag */
            /* DAT_004e21d0 = 1; */
            LOG_DEBUG("Chat update flag set");
            break;

        default:
            /* Unknown type - store directly */
            msg_len = strlen(message);
            dest_buffer = s_chat_buffer[s_chat_buffer_index];
            memcpy(dest_buffer, message, msg_len + 1);
            s_chat_buffer_index = (s_chat_buffer_index + 1) & 3;
            break;
    }
}

/*
 * Handle chat message
 */
void chat_handle_message(void* data, u32 size) {
    u8* ptr = (u8*)data;
    char sender[24];
    char message[MAX_CHAT_MSG];
    ChatType type;
    u32 color;

    type = (ChatType)*ptr++;
    strncpy(sender, (char*)ptr, sizeof(sender) - 1);
    sender[sizeof(sender) - 1] = '\0';
    ptr += strlen(sender) + 1;
    strncpy(message, (char*)ptr, sizeof(message) - 1);
    message[sizeof(message) - 1] = '\0';

    color = chat_get_color(type);
    chat_receive(type, sender, NULL, message, color);
}

/*
 * Handle whisper packet
 */
void chat_handle_whisper(void* data, u32 size) {
    u8* ptr = (u8*)data;
    char sender[24];
    char message[MAX_CHAT_MSG];

    strncpy(sender, (char*)ptr, sizeof(sender) - 1);
    sender[sizeof(sender) - 1] = '\0';
    ptr += strlen(sender) + 1;
    strncpy(message, (char*)ptr, sizeof(message) - 1);
    message[sizeof(message) - 1] = '\0';

    /* Save sender for reply */
    strncpy(g_chat.whisper_target, sender, sizeof(g_chat.whisper_target) - 1);

    chat_receive(CHAT_TYPE_WHISPER, sender, NULL, message, s_chat_colors[CHAT_TYPE_WHISPER]);
}

/*
 * Handle system message packet
 */
void chat_handle_system(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 color;
    char message[MAX_CHAT_MSG];

    if (size < 4) return;

    color = *(u32*)ptr; ptr += 4;
    strncpy(message, (char*)ptr, sizeof(message) - 1);
    message[sizeof(message) - 1] = '\0';

    chat_receive(CHAT_TYPE_SYSTEM, "System", NULL, message, color);
}

/*
 * Handle system message from server
 * Called from PKT_SV_SYSTEM_MSG (0x73)
 */
void chat_handle_system_message(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u16 msg_type;
    u16 msg_id;
    char message[256];

    if (size < 4) return;

    msg_type = *(u16*)ptr; ptr += 2;
    msg_id = *(u16*)ptr; ptr += 2;

    if (ptr < (u8*)data + size) {
        strncpy(message, (char*)ptr, sizeof(message) - 1);
        message[sizeof(message) - 1] = '\0';
    } else {
        message[0] = '\0';
    }

    chat_add_history(CHAT_TYPE_SYSTEM, "[System]", message);

    LOG_DEBUG("System message type %u: %s", msg_type, message);
}

/*
 * Handle notice from server
 * Called from PKT_SV_NOTICE (0x85)
 */
void chat_handle_notice(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 notice_id;
    char notice[512];

    if (size < 4) return;

    notice_id = *(u32*)ptr; ptr += 4;

    if (ptr < (u8*)data + size) {
        strncpy(notice, (char*)ptr, sizeof(notice) - 1);
        notice[sizeof(notice) - 1] = '\0';
    } else {
        notice[0] = '\0';
    }

    chat_add_history(CHAT_TYPE_SYSTEM, "[Notice]", notice);

    /* Show popup for important notices */
    if (notice[0]) {
        MessageBoxA(NULL, notice, "Notice", MB_OK | MB_ICONINFORMATION);
    }

    LOG_INFO("Notice: %s", notice);
}

/*
 * Handle private message from server
 * Called from text protocol PM command
 */
void chat_handle_private_message(u32 sender_id, const char* sender_name, u32 param) {
    char message[256];

    if (sender_name && sender_name[0]) {
        _snprintf(message, sizeof(message), "[Whisper] %s", sender_name);
        chat_add_history(CHAT_TYPE_WHISPER, message, "");
    }

    LOG_DEBUG("Private message from %s (ID: %u)", sender_name ? sender_name : "unknown", sender_id);
}

/*
 * Get chat buffer by index
 */
const char* chat_get_buffer(int index) {
    if (index < 0 || index >= 4) return NULL;
    return s_chat_buffer[index];
}

/*
 * Get party buffer by index
 */
const char* chat_get_party_buffer(int index) {
    if (index < 0 || index >= 4) return NULL;
    return s_party_buffer[index];
}
