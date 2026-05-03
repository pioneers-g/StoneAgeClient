/*
 * Stone Age Client - Friend/Contact System Implementation
 * Reverse engineered from sa_9061.exe
 *
 * Protocol Analysis:
 * FUN_00465470 - Chat/message handler for whispers
 * FUN_00465440 - Message counter increment
 * FUN_0043bf90 - Text protocol dispatcher
 *
 * Protocol Commands:
 * - PF: Friend list request
 * - FA: Add friend
 * - FD: Delete friend
 * - P|/FM<name> <msg>: Private message (whisper) - format at 0x004c44fc
 * - FB: Block player
 * - FU: Unblock player
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "friend.h"
#include "network.h"
#include "logger.h"

/* Global friend context */
FriendContext g_friend = {0};

/* Forward declarations */
static void friend_send_packet(const char* cmd, const char* params);

/*
 * Initialize friend system
 */
int friend_init(void) {
    memset(&g_friend, 0, sizeof(FriendContext));
    LOG_INFO("Friend system initialized");
    return 1;
}

/*
 * Shutdown friend system
 */
void friend_shutdown(void) {
    memset(&g_friend, 0, sizeof(FriendContext));
    LOG_INFO("Friend system shutdown");
}

/*
 * Send friend protocol packet
 */
static void friend_send_packet(const char* cmd, const char* params) {
    char packet[512];

    if (params && params[0]) {
        _snprintf(packet, sizeof(packet), "%s|%s", cmd, params);
    } else {
        strncpy(packet, cmd, sizeof(packet) - 1);
    }

    network_send(packet, strlen(packet));
    network_send("\n", 1);
}

/*
 * Request friend list - PF command
 */
void friend_request_list(void) {
    friend_send_packet(FRIEND_CMD_LIST, "");
    LOG_DEBUG("Requesting friend list");
}

/*
 * Handle friend list response
 */
void friend_handle_list(void* data, u32 size) {
    u8* ptr = (u8*)data;
    int i;
    FriendEntry* friend;

    g_friend.friend_count = 0;

    for (i = 0; i < MAX_FRIENDS && ptr < (u8*)data + size; i++) {
        friend = &g_friend.friends[i];

        friend->id = *(u32*)ptr; ptr += 4;
        if (friend->id == 0) continue;

        strncpy(friend->name, (char*)ptr, sizeof(friend->name) - 1);
        ptr += strlen(friend->name) + 1;

        friend->level = *(u16*)ptr; ptr += 2;
        friend->job = *(u16*)ptr; ptr += 2;
        friend->online = *(u8*)ptr; ptr += 1;
        friend->status = *(u8*)ptr; ptr += 1;

        g_friend.friend_count++;
    }

    g_friend.list_loaded = 1;
    LOG_INFO("Loaded %d friends", g_friend.friend_count);
}

/*
 * Add friend - FA command
 */
int friend_add(const char* name) {
    FriendEntry* friend;
    int i;

    if (!name || strlen(name) == 0) {
        return 0;
    }

    /* Check if already in list */
    if (friend_find_by_name(name)) {
        LOG_WARN("Already friends with %s", name);
        return 0;
    }

    /* Check if list is full */
    if (g_friend.friend_count >= MAX_FRIENDS) {
        LOG_WARN("Friend list is full");
        return 0;
    }

    /* Send add friend packet */
    friend_send_packet(FRIEND_CMD_ADD, name);

    LOG_DEBUG("Adding friend: %s", name);
    return 1;
}

/*
 * Handle add friend response
 */
void friend_handle_add(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;
    FriendEntry* friend;

    result = *ptr++;

    if (result == 0) {
        /* Success - parse friend data */
        if (g_friend.friend_count >= MAX_FRIENDS) {
            LOG_WARN("Friend list full, cannot add");
            return;
        }

        friend = &g_friend.friends[g_friend.friend_count];
        friend->id = *(u32*)ptr; ptr += 4;
        strncpy(friend->name, (char*)ptr, sizeof(friend->name) - 1);
        ptr += strlen(friend->name) + 1;
        friend->level = *(u16*)ptr; ptr += 2;
        friend->job = *(u16*)ptr; ptr += 2;
        friend->online = *(u8*)ptr;

        g_friend.friend_count++;

        LOG_INFO("Added friend: %s", friend->name);
    } else {
        LOG_WARN("Failed to add friend");
    }
}

/*
 * Remove friend - FD command
 */
int friend_remove(const char* name) {
    FriendEntry* friend;

    friend = friend_find_by_name(name);
    if (!friend) {
        return 0;
    }

    /* Send remove friend packet */
    friend_send_packet(FRIEND_CMD_DELETE, name);

    LOG_DEBUG("Removing friend: %s", name);
    return 1;
}

/*
 * Handle remove friend response
 */
void friend_handle_remove(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 friend_id;
    int i;

    friend_id = *(u32*)ptr;

    for (i = 0; i < MAX_FRIENDS; i++) {
        if (g_friend.friends[i].id == friend_id) {
            LOG_INFO("Removed friend: %s", g_friend.friends[i].name);
            memset(&g_friend.friends[i], 0, sizeof(FriendEntry));
            g_friend.friend_count--;
            break;
        }
    }
}

/*
 * Handle friend online status change
 */
void friend_handle_status_change(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 friend_id;
    u8 online;
    int i;

    friend_id = *(u32*)ptr; ptr += 4;
    online = *ptr;

    for (i = 0; i < MAX_FRIENDS; i++) {
        if (g_friend.friends[i].id == friend_id) {
            g_friend.friends[i].online = online;
            LOG_DEBUG("Friend %s is now %s",
                     g_friend.friends[i].name,
                     online ? "online" : "offline");
            break;
        }
    }
}

/*
 * Find friend by name
 */
FriendEntry* friend_find_by_name(const char* name) {
    int i;

    for (i = 0; i < MAX_FRIENDS; i++) {
        if (strcmp(g_friend.friends[i].name, name) == 0) {
            return &g_friend.friends[i];
        }
    }

    return NULL;
}

/*
 * Find friend by ID
 */
FriendEntry* friend_find_by_id(u32 id) {
    int i;

    for (i = 0; i < MAX_FRIENDS; i++) {
        if (g_friend.friends[i].id == id) {
            return &g_friend.friends[i];
        }
    }

    return NULL;
}

/*
 * Get friend by index
 */
FriendEntry* friend_get_by_index(int index) {
    if (index < 0 || index >= MAX_FRIENDS) {
        return NULL;
    }
    return &g_friend.friends[index];
}

/*
 * Get online friend count
 */
int friend_get_online_count(void) {
    int i;
    int count = 0;

    for (i = 0; i < MAX_FRIENDS; i++) {
        if (g_friend.friends[i].id != 0 && g_friend.friends[i].online) {
            count++;
        }
    }

    return count;
}

/*
 * Get total friend count
 */
int friend_get_count(void) {
    return g_friend.friend_count;
}

/*
 * Check if friend list is full
 */
int friend_is_full(void) {
    return g_friend.friend_count >= MAX_FRIENDS;
}

/*
 * Send private message (whisper) - FM command
 * Format: P|/FM<name> <message>
 */
int friend_send_whisper(const char* name, const char* message) {
    char params[256];

    if (!name || !message) {
        return 0;
    }

    /* Format: /FM<name> <message> */
    _snprintf(params, sizeof(params), "%s%s %s", CHAT_PREFIX_WHISPER, name, message);
    friend_send_packet("P", params);

    /* Store last whisper target for reply */
    strncpy(g_friend.last_whisper_from, name, sizeof(g_friend.last_whisper_from) - 1);

    LOG_DEBUG("Whisper to %s: %s", name, message);
    return 1;
}

/*
 * Handle incoming whisper - FM response
 */
void friend_handle_whisper(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 sender_id;
    char sender_name[24];
    char message[256];

    sender_id = *(u32*)ptr; ptr += 4;
    strncpy(sender_name, (char*)ptr, sizeof(sender_name) - 1);
    ptr += strlen(sender_name) + 1;
    strncpy(message, (char*)ptr, sizeof(message) - 1);

    /* Store sender for reply */
    strncpy(g_friend.last_whisper_from, sender_name, sizeof(g_friend.last_whisper_from) - 1);

    LOG_INFO("[Whisper] %s: %s", sender_name, message);
}

/*
 * Reply to last whisper
 */
int friend_reply_whisper(const char* message) {
    if (g_friend.last_whisper_from[0] == '\0') {
        LOG_WARN("No one to reply to");
        return 0;
    }

    return friend_send_whisper(g_friend.last_whisper_from, message);
}

/*
 * Initialize block list
 */
int block_init(void) {
    memset(&g_friend.block_count, 0, sizeof(g_friend.block_count));
    return 1;
}

/*
 * Add to block list - FB command
 */
int block_add(const char* name) {
    char params[64];
    int i;

    if (!name || strlen(name) == 0) {
        return 0;
    }

    if (g_friend.block_count >= MAX_BLOCKED) {
        LOG_WARN("Block list is full");
        return 0;
    }

    /* Send block add packet */
    friend_send_packet(FRIEND_CMD_BLOCK, name);

    strncpy(g_friend.blocked[g_friend.block_count].name, name,
            sizeof(g_friend.blocked[0].name) - 1);
    g_friend.block_count++;

    LOG_DEBUG("Blocked: %s", name);
    return 1;
}

/*
 * Remove from block list - FU command
 */
int block_remove(const char* name) {
    int i;

    for (i = 0; i < MAX_BLOCKED; i++) {
        if (strcmp(g_friend.blocked[i].name, name) == 0) {
            memset(&g_friend.blocked[i], 0, sizeof(BlockedEntry));
            g_friend.block_count--;

            /* Send block remove packet */
            friend_send_packet(FRIEND_CMD_UNBLOCK, name);

            LOG_DEBUG("Unblocked: %s", name);
            return 1;
        }
    }

    return 0;
}

/*
 * Check if player is blocked
 */
int block_is_blocked(const char* name) {
    int i;

    for (i = 0; i < MAX_BLOCKED; i++) {
        if (strcmp(g_friend.blocked[i].name, name) == 0) {
            return 1;
        }
    }

    return 0;
}
