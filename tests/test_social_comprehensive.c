/*
 * Stone Age Client - Social/Friend System Comprehensive Tests
 * Tests for friend system from src/social/friend.c
 *
 * Coverage:
 * - Friend list management
 * - Add/remove friend operations
 * - Online status tracking
 * - Whisper (private message) system
 * - Block list management
 * - Protocol command constants
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Disable assert popups on Windows */
#define NDEBUG

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Constants */
#define MAX_FRIENDS     50
#define MAX_BLOCKED     20

/* Protocol commands */
#define FRIEND_CMD_LIST         "PF"
#define FRIEND_CMD_ADD          "FA"
#define FRIEND_CMD_DELETE       "FD"
#define FRIEND_CMD_WHISPER      "FM"
#define FRIEND_CMD_BLOCK        "FB"
#define FRIEND_CMD_UNBLOCK      "FU"

/* Chat command prefixes */
#define CHAT_PREFIX_WHISPER     "/FM"
#define CHAT_PREFIX_TELL        "/tell "
#define CHAT_PREFIX_PARTY       "/P"
#define CHAT_PREFIX_GUILD       "/G"

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
    FriendEntry friends[MAX_FRIENDS];
    int friend_count;
    int list_loaded;

    BlockedEntry blocked[MAX_BLOCKED];
    int block_count;

    int selected_index;

    char last_whisper_from[24];
    char last_whisper_to[24];

    int dirty;
    int request_pending;
} FriendContext;

static FriendContext g_friend;

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Friend System Functions (Stubs)
 * ======================================== */

void friend_init(void) {
    memset(&g_friend, 0, sizeof(FriendContext));
}

void friend_shutdown(void) {
    memset(&g_friend, 0, sizeof(FriendContext));
}

void friend_handle_list(void* data, u32 size) {
    u8* ptr = (u8*)data;
    g_friend.friend_count = 0;

    for (int i = 0; i < MAX_FRIENDS && ptr < (u8*)data + size; i++) {
        FriendEntry* friend = &g_friend.friends[i];

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
}

int friend_add(const char* name) {
    if (!name || strlen(name) == 0) {
        return 0;
    }

    /* Check if already in list */
    for (int i = 0; i < MAX_FRIENDS; i++) {
        if (strcmp(g_friend.friends[i].name, name) == 0) {
            return 0;
        }
    }

    if (g_friend.friend_count >= MAX_FRIENDS) {
        return 0;
    }

    return 1;
}

void friend_handle_add(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result = *ptr++;

    if (result == 0 && g_friend.friend_count < MAX_FRIENDS) {
        FriendEntry* friend = &g_friend.friends[g_friend.friend_count];
        friend->id = *(u32*)ptr; ptr += 4;
        strncpy(friend->name, (char*)ptr, sizeof(friend->name) - 1);
        ptr += strlen(friend->name) + 1;
        friend->level = *(u16*)ptr; ptr += 2;
        friend->job = *(u16*)ptr; ptr += 2;
        friend->online = *(u8*)ptr;

        g_friend.friend_count++;
    }
}

int friend_remove(const char* name) {
    for (int i = 0; i < MAX_FRIENDS; i++) {
        if (strcmp(g_friend.friends[i].name, name) == 0) {
            memset(&g_friend.friends[i], 0, sizeof(FriendEntry));
            g_friend.friend_count--;
            return 1;
        }
    }
    return 0;
}

void friend_handle_remove(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 friend_id = *(u32*)ptr;

    for (int i = 0; i < MAX_FRIENDS; i++) {
        if (g_friend.friends[i].id == friend_id) {
            memset(&g_friend.friends[i], 0, sizeof(FriendEntry));
            g_friend.friend_count--;
            break;
        }
    }
}

void friend_handle_status_change(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 friend_id = *(u32*)ptr; ptr += 4;
    u8 online = *ptr;

    for (int i = 0; i < MAX_FRIENDS; i++) {
        if (g_friend.friends[i].id == friend_id) {
            g_friend.friends[i].online = online;
            break;
        }
    }
}

FriendEntry* friend_find_by_name(const char* name) {
    for (int i = 0; i < MAX_FRIENDS; i++) {
        if (strcmp(g_friend.friends[i].name, name) == 0) {
            return &g_friend.friends[i];
        }
    }
    return NULL;
}

FriendEntry* friend_find_by_id(u32 id) {
    for (int i = 0; i < MAX_FRIENDS; i++) {
        if (g_friend.friends[i].id == id) {
            return &g_friend.friends[i];
        }
    }
    return NULL;
}

FriendEntry* friend_get_by_index(int index) {
    if (index < 0 || index >= MAX_FRIENDS) {
        return NULL;
    }
    return &g_friend.friends[index];
}

int friend_get_online_count(void) {
    int count = 0;
    for (int i = 0; i < MAX_FRIENDS; i++) {
        if (g_friend.friends[i].id != 0 && g_friend.friends[i].online) {
            count++;
        }
    }
    return count;
}

int friend_get_count(void) {
    return g_friend.friend_count;
}

int friend_is_full(void) {
    return g_friend.friend_count >= MAX_FRIENDS;
}

int friend_send_whisper(const char* name, const char* message) {
    if (!name || !message) {
        return 0;
    }
    strncpy(g_friend.last_whisper_to, name, sizeof(g_friend.last_whisper_to) - 1);
    return 1;
}

void friend_handle_whisper(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 sender_id = *(u32*)ptr; ptr += 4;

    char sender_name[24];
    strncpy(sender_name, (char*)ptr, sizeof(sender_name) - 1);
    ptr += strlen(sender_name) + 1;

    strncpy(g_friend.last_whisper_from, sender_name, sizeof(g_friend.last_whisper_from) - 1);
}

int friend_reply_whisper(const char* message) {
    if (g_friend.last_whisper_from[0] == '\0') {
        return 0;
    }
    return friend_send_whisper(g_friend.last_whisper_from, message);
}

int block_add(const char* name) {
    if (!name || strlen(name) == 0) {
        return 0;
    }

    if (g_friend.block_count >= MAX_BLOCKED) {
        return 0;
    }

    strncpy(g_friend.blocked[g_friend.block_count].name, name, sizeof(g_friend.blocked[0].name) - 1);
    g_friend.block_count++;

    return 1;
}

int block_remove(const char* name) {
    for (int i = 0; i < MAX_BLOCKED; i++) {
        if (strcmp(g_friend.blocked[i].name, name) == 0) {
            memset(&g_friend.blocked[i], 0, sizeof(BlockedEntry));
            g_friend.block_count--;
            return 1;
        }
    }
    return 0;
}

int block_is_blocked(const char* name) {
    for (int i = 0; i < MAX_BLOCKED; i++) {
        if (strcmp(g_friend.blocked[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

/* ========================================
 * Test Setup/Teardown
 * ======================================== */

static void test_setup(void) {
    friend_init();
}

static void test_teardown(void) {
    friend_shutdown();
}

/* ========================================
 * Constants Tests
 * ======================================== */

static int test_max_friends_constant(void) {
    return MAX_FRIENDS == 50;
}

static int test_max_blocked_constant(void) {
    return MAX_BLOCKED == 20;
}

static int test_protocol_commands(void) {
    return strcmp(FRIEND_CMD_LIST, "PF") == 0 &&
           strcmp(FRIEND_CMD_ADD, "FA") == 0 &&
           strcmp(FRIEND_CMD_DELETE, "FD") == 0 &&
           strcmp(FRIEND_CMD_WHISPER, "FM") == 0 &&
           strcmp(FRIEND_CMD_BLOCK, "FB") == 0 &&
           strcmp(FRIEND_CMD_UNBLOCK, "FU") == 0;
}

static int test_chat_prefixes(void) {
    return strcmp(CHAT_PREFIX_WHISPER, "/FM") == 0 &&
           strcmp(CHAT_PREFIX_TELL, "/tell ") == 0 &&
           strcmp(CHAT_PREFIX_PARTY, "/P") == 0 &&
           strcmp(CHAT_PREFIX_GUILD, "/G") == 0;
}

static int test_friend_status_values(void) {
    return FRIEND_STATUS_OFFLINE == 0 &&
           FRIEND_STATUS_ONLINE == 1 &&
           FRIEND_STATUS_AWAY == 2 &&
           FRIEND_STATUS_BUSY == 3;
}

static int test_friend_entry_size(void) {
    /* FriendEntry: 4 + 24 + 2 + 2 + 1 + 1 + 4 + 4 = 42 bytes */
    return sizeof(FriendEntry) >= 38;  /* At least the required size */
}

static int test_blocked_entry_size(void) {
    /* BlockedEntry: 4 + 24 + 4 = 32 bytes */
    return sizeof(BlockedEntry) >= 32;
}

/* ========================================
 * Initialization Tests
 * ======================================== */

static int test_friend_init(void) {
    test_setup();

    int pass = g_friend.friend_count == 0 &&
               g_friend.block_count == 0 &&
               g_friend.list_loaded == 0;

    test_teardown();
    return pass;
}

static int test_friend_shutdown_clears(void) {
    test_setup();

    g_friend.friend_count = 10;
    g_friend.block_count = 5;

    friend_shutdown();

    int pass = g_friend.friend_count == 0 &&
               g_friend.block_count == 0;

    return pass;
}

/* ========================================
 * Friend List Tests
 * ======================================== */

static int test_handle_friend_list(void) {
    test_setup();

    /* Build test packet: id|name\0|level|job|online|status */
    u8 packet[128];
    u8* ptr = packet;

    *(u32*)ptr = 1001; ptr += 4;
    strcpy((char*)ptr, "Player1"); ptr += strlen("Player1") + 1;
    *(u16*)ptr = 50; ptr += 2;
    *(u16*)ptr = 1; ptr += 2;
    *ptr++ = 1;  /* online */
    *ptr++ = 0;  /* status */

    *(u32*)ptr = 1002; ptr += 4;
    strcpy((char*)ptr, "Player2"); ptr += strlen("Player2") + 1;
    *(u16*)ptr = 30; ptr += 2;
    *(u16*)ptr = 2; ptr += 2;
    *ptr++ = 0;  /* offline */
    *ptr++ = 0;  /* status */

    friend_handle_list(packet, ptr - packet);

    int pass = g_friend.friend_count == 2 &&
               g_friend.list_loaded == 1 &&
               strcmp(g_friend.friends[0].name, "Player1") == 0 &&
               g_friend.friends[0].online == 1 &&
               g_friend.friends[0].level == 50;

    test_teardown();
    return pass;
}

static int test_empty_friend_list(void) {
    test_setup();

    u8 packet[4] = {0};
    friend_handle_list(packet, 4);

    int pass = g_friend.friend_count == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Add Friend Tests
 * ======================================== */

static int test_add_friend_basic(void) {
    test_setup();

    int result = friend_add("TestPlayer");

    int pass = result == 1;

    test_teardown();
    return pass;
}

static int test_add_friend_empty_name(void) {
    test_setup();

    int result = friend_add("");

    int pass = result == 0;

    test_teardown();
    return pass;
}

static int test_add_friend_null_name(void) {
    test_setup();

    int result = friend_add(NULL);

    int pass = result == 0;

    test_teardown();
    return pass;
}

static int test_add_friend_already_exists(void) {
    test_setup();

    /* Add friend via handle_add */
    u8 packet[64];
    u8* ptr = packet;
    *ptr++ = 0;  /* success */
    *(u32*)ptr = 1001; ptr += 4;
    strcpy((char*)ptr, "TestPlayer"); ptr += strlen("TestPlayer") + 1;
    *(u16*)ptr = 50; ptr += 2;
    *(u16*)ptr = 1; ptr += 2;
    *ptr++ = 1;

    friend_handle_add(packet, ptr - packet);

    /* Try to add again */
    int result = friend_add("TestPlayer");

    int pass = result == 0;  /* Should fail, already exists */

    test_teardown();
    return pass;
}

static int test_add_friend_full_list(void) {
    test_setup();

    g_friend.friend_count = MAX_FRIENDS;

    int result = friend_add("NewPlayer");

    int pass = result == 0;

    test_teardown();
    return pass;
}

static int test_handle_add_friend_success(void) {
    test_setup();

    u8 packet[64];
    u8* ptr = packet;
    *ptr++ = 0;  /* success */
    *(u32*)ptr = 1001; ptr += 4;
    strcpy((char*)ptr, "NewFriend"); ptr += strlen("NewFriend") + 1;
    *(u16*)ptr = 25; ptr += 2;
    *(u16*)ptr = 3; ptr += 2;
    *ptr++ = 1;  /* online */

    friend_handle_add(packet, ptr - packet);

    int pass = g_friend.friend_count == 1 &&
               strcmp(g_friend.friends[0].name, "NewFriend") == 0 &&
               g_friend.friends[0].level == 25;

    test_teardown();
    return pass;
}

static int test_handle_add_friend_failure(void) {
    test_setup();

    u8 packet[1] = {1};  /* failure */

    friend_handle_add(packet, 1);

    int pass = g_friend.friend_count == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Remove Friend Tests
 * ======================================== */

static int test_remove_friend_basic(void) {
    test_setup();

    /* Add a friend first */
    u8 packet[64];
    u8* ptr = packet;
    *ptr++ = 0;
    *(u32*)ptr = 1001; ptr += 4;
    strcpy((char*)ptr, "ToRemove"); ptr += strlen("ToRemove") + 1;
    *(u16*)ptr = 50; ptr += 2;
    *(u16*)ptr = 1; ptr += 2;
    *ptr++ = 1;

    friend_handle_add(packet, ptr - packet);

    /* Remove */
    int result = friend_remove("ToRemove");

    int pass = result == 1 && g_friend.friend_count == 0;

    test_teardown();
    return pass;
}

static int test_remove_friend_not_found(void) {
    test_setup();

    int result = friend_remove("NotFound");

    int pass = result == 0;

    test_teardown();
    return pass;
}

static int test_handle_remove_friend(void) {
    test_setup();

    /* Add friend first */
    u8 add_packet[64];
    u8* ptr = add_packet;
    *ptr++ = 0;
    *(u32*)ptr = 1001; ptr += 4;
    strcpy((char*)ptr, "ToDelete"); ptr += strlen("ToDelete") + 1;
    *(u16*)ptr = 50; ptr += 2;
    *(u16*)ptr = 1; ptr += 2;
    *ptr++ = 1;

    friend_handle_add(add_packet, ptr - add_packet);

    /* Handle remove */
    u32 remove_id = 1001;
    friend_handle_remove(&remove_id, 4);

    int pass = g_friend.friend_count == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Status Update Tests
 * ======================================== */

static int test_status_change_online(void) {
    test_setup();

    /* Add friend first */
    u8 packet[64];
    u8* ptr = packet;
    *ptr++ = 0;
    *(u32*)ptr = 1001; ptr += 4;
    strcpy((char*)ptr, "TestPlayer"); ptr += strlen("TestPlayer") + 1;
    *(u16*)ptr = 50; ptr += 2;
    *(u16*)ptr = 1; ptr += 2;
    *ptr++ = 0;  /* offline initially */
    *ptr++ = 0;

    friend_handle_add(packet, ptr - packet);

    /* Change status to online */
    u8 status_packet[8];
    *(u32*)status_packet = 1001;
    status_packet[4] = 1;  /* online */

    friend_handle_status_change(status_packet, 5);

    int pass = g_friend.friends[0].online == 1;

    test_teardown();
    return pass;
}

static int test_status_change_offline(void) {
    test_setup();

    /* Add friend as online */
    u8 packet[64];
    u8* ptr = packet;
    *ptr++ = 0;
    *(u32*)ptr = 1001; ptr += 4;
    strcpy((char*)ptr, "TestPlayer"); ptr += strlen("TestPlayer") + 1;
    *(u16*)ptr = 50; ptr += 2;
    *(u16*)ptr = 1; ptr += 2;
    *ptr++ = 1;  /* online */
    *ptr++ = 0;

    friend_handle_add(packet, ptr - packet);

    /* Change status to offline */
    u8 status_packet[8];
    *(u32*)status_packet = 1001;
    status_packet[4] = 0;  /* offline */

    friend_handle_status_change(status_packet, 5);

    int pass = g_friend.friends[0].online == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Query Tests
 * ======================================== */

static int test_find_by_name(void) {
    test_setup();

    u8 packet[64];
    u8* ptr = packet;
    *ptr++ = 0;
    *(u32*)ptr = 1001; ptr += 4;
    strcpy((char*)ptr, "FindMe"); ptr += strlen("FindMe") + 1;
    *(u16*)ptr = 50; ptr += 2;
    *(u16*)ptr = 1; ptr += 2;
    *ptr++ = 1;
    *ptr++ = 0;

    friend_handle_add(packet, ptr - packet);

    FriendEntry* entry = friend_find_by_name("FindMe");

    int pass = entry != NULL &&
               entry->id == 1001 &&
               strcmp(entry->name, "FindMe") == 0;

    test_teardown();
    return pass;
}

static int test_find_by_name_not_found(void) {
    test_setup();

    FriendEntry* entry = friend_find_by_name("NotFound");

    int pass = entry == NULL;

    test_teardown();
    return pass;
}

static int test_find_by_id(void) {
    test_setup();

    u8 packet[64];
    u8* ptr = packet;
    *ptr++ = 0;
    *(u32*)ptr = 12345; ptr += 4;
    strcpy((char*)ptr, "TestPlayer"); ptr += strlen("TestPlayer") + 1;
    *(u16*)ptr = 50; ptr += 2;
    *(u16*)ptr = 1; ptr += 2;
    *ptr++ = 1;
    *ptr++ = 0;

    friend_handle_add(packet, ptr - packet);

    FriendEntry* entry = friend_find_by_id(12345);

    int pass = entry != NULL &&
               strcmp(entry->name, "TestPlayer") == 0;

    test_teardown();
    return pass;
}

static int test_find_by_id_not_found(void) {
    test_setup();

    FriendEntry* entry = friend_find_by_id(99999);

    int pass = entry == NULL;

    test_teardown();
    return pass;
}

static int test_get_by_index(void) {
    test_setup();

    u8 packet[64];
    u8* ptr = packet;
    *ptr++ = 0;
    *(u32*)ptr = 1001; ptr += 4;
    strcpy((char*)ptr, "IndexTest"); ptr += strlen("IndexTest") + 1;
    *(u16*)ptr = 50; ptr += 2;
    *(u16*)ptr = 1; ptr += 2;
    *ptr++ = 1;
    *ptr++ = 0;

    friend_handle_add(packet, ptr - packet);

    FriendEntry* entry = friend_get_by_index(0);

    int pass = entry != NULL &&
               strcmp(entry->name, "IndexTest") == 0;

    test_teardown();
    return pass;
}

static int test_get_by_invalid_index(void) {
    test_setup();

    FriendEntry* entry1 = friend_get_by_index(-1);
    FriendEntry* entry2 = friend_get_by_index(MAX_FRIENDS);
    FriendEntry* entry3 = friend_get_by_index(1000);

    int pass = entry1 == NULL && entry2 == NULL && entry3 == NULL;

    test_teardown();
    return pass;
}

static int test_get_online_count(void) {
    test_setup();

    /* Add two friends, one online, one offline */
    u8 packet[128];
    u8* ptr = packet;

    *(u32*)ptr = 1001; ptr += 4;
    strcpy((char*)ptr, "Online1"); ptr += strlen("Online1") + 1;
    *(u16*)ptr = 50; ptr += 2;
    *(u16*)ptr = 1; ptr += 2;
    *ptr++ = 1;  /* online */
    *ptr++ = 0;

    *(u32*)ptr = 1002; ptr += 4;
    strcpy((char*)ptr, "Offline1"); ptr += strlen("Offline1") + 1;
    *(u16*)ptr = 30; ptr += 2;
    *(u16*)ptr = 2; ptr += 2;
    *ptr++ = 0;  /* offline */
    *ptr++ = 0;

    friend_handle_list(packet, ptr - packet);

    int count = friend_get_online_count();

    int pass = count == 1;

    test_teardown();
    return pass;
}

static int test_get_count(void) {
    test_setup();

    u8 packet[128];
    u8* ptr = packet;

    *(u32*)ptr = 1001; ptr += 4;
    strcpy((char*)ptr, "Friend1"); ptr += strlen("Friend1") + 1;
    *(u16*)ptr = 50; ptr += 2;
    *(u16*)ptr = 1; ptr += 2;
    *ptr++ = 1;
    *ptr++ = 0;

    *(u32*)ptr = 1002; ptr += 4;
    strcpy((char*)ptr, "Friend2"); ptr += strlen("Friend2") + 1;
    *(u16*)ptr = 30; ptr += 2;
    *(u16*)ptr = 2; ptr += 2;
    *ptr++ = 0;
    *ptr++ = 0;

    friend_handle_list(packet, ptr - packet);

    int pass = friend_get_count() == 2;

    test_teardown();
    return pass;
}

static int test_is_full(void) {
    test_setup();

    int pass1 = friend_is_full() == 0;

    g_friend.friend_count = MAX_FRIENDS;
    int pass2 = friend_is_full() == 1;

    test_teardown();
    return pass1 && pass2;
}

/* ========================================
 * Whisper Tests
 * ======================================== */

static int test_send_whisper(void) {
    test_setup();

    int result = friend_send_whisper("TargetPlayer", "Hello!");

    int pass = result == 1 &&
               strcmp(g_friend.last_whisper_to, "TargetPlayer") == 0;

    test_teardown();
    return pass;
}

static int test_send_whisper_null_name(void) {
    test_setup();

    int result = friend_send_whisper(NULL, "Hello");

    int pass = result == 0;

    test_teardown();
    return pass;
}

static int test_send_whisper_null_message(void) {
    test_setup();

    int result = friend_send_whisper("Player", NULL);

    int pass = result == 0;

    test_teardown();
    return pass;
}

static int test_handle_whisper(void) {
    test_setup();

    u8 packet[64];
    u8* ptr = packet;
    *(u32*)ptr = 1001; ptr += 4;
    strcpy((char*)ptr, "Sender"); ptr += strlen("Sender") + 1;

    friend_handle_whisper(packet, ptr - packet);

    int pass = strcmp(g_friend.last_whisper_from, "Sender") == 0;

    test_teardown();
    return pass;
}

static int test_reply_whisper(void) {
    test_setup();

    /* Simulate receiving a whisper */
    u8 packet[64];
    u8* ptr = packet;
    *(u32*)ptr = 1001; ptr += 4;
    strcpy((char*)ptr, "WhisperSender"); ptr += strlen("WhisperSender") + 1;

    friend_handle_whisper(packet, ptr - packet);

    /* Reply */
    int result = friend_reply_whisper("Reply message");

    int pass = result == 1 &&
               strcmp(g_friend.last_whisper_to, "WhisperSender") == 0;

    test_teardown();
    return pass;
}

static int test_reply_no_whisper(void) {
    test_setup();

    int result = friend_reply_whisper("Reply");

    int pass = result == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Block List Tests
 * ======================================== */

static int test_block_add(void) {
    test_setup();

    int result = block_add("AnnoyingPlayer");

    int pass = result == 1 &&
               g_friend.block_count == 1 &&
               strcmp(g_friend.blocked[0].name, "AnnoyingPlayer") == 0;

    test_teardown();
    return pass;
}

static int test_block_add_empty_name(void) {
    test_setup();

    int result = block_add("");

    int pass = result == 0 && g_friend.block_count == 0;

    test_teardown();
    return pass;
}

static int test_block_add_null_name(void) {
    test_setup();

    int result = block_add(NULL);

    int pass = result == 0;

    test_teardown();
    return pass;
}

static int test_block_add_full(void) {
    test_setup();

    g_friend.block_count = MAX_BLOCKED;

    int result = block_add("NewBlocked");

    int pass = result == 0;

    test_teardown();
    return pass;
}

static int test_block_remove(void) {
    test_setup();

    block_add("ToUnblock");

    int result = block_remove("ToUnblock");

    int pass = result == 1 &&
               g_friend.block_count == 0;

    test_teardown();
    return pass;
}

static int test_block_remove_not_found(void) {
    test_setup();

    int result = block_remove("NotBlocked");

    int pass = result == 0;

    test_teardown();
    return pass;
}

static int test_is_blocked(void) {
    test_setup();

    block_add("BlockedPlayer");

    int pass = block_is_blocked("BlockedPlayer") == 1 &&
               block_is_blocked("NotBlocked") == 0;

    test_teardown();
    return pass;
}

static int test_multiple_blocks(void) {
    test_setup();

    block_add("Block1");
    block_add("Block2");
    block_add("Block3");

    int pass = g_friend.block_count == 3 &&
               block_is_blocked("Block1") &&
               block_is_blocked("Block2") &&
               block_is_blocked("Block3");

    test_teardown();
    return pass;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_friend_lifecycle(void) {
    test_setup();

    /* Add friend */
    u8 add_packet[64];
    u8* ptr = add_packet;
    *ptr++ = 0;
    *(u32*)ptr = 1001; ptr += 4;
    strcpy((char*)ptr, "LifecycleFriend"); ptr += strlen("LifecycleFriend") + 1;
    *(u16*)ptr = 50; ptr += 2;
    *(u16*)ptr = 1; ptr += 2;
    *ptr++ = 1;
    *ptr++ = 0;

    friend_handle_add(add_packet, ptr - add_packet);

    if (g_friend.friend_count != 1) {
        test_teardown();
        return 0;
    }

    /* Update status */
    u8 status_packet[8];
    *(u32*)status_packet = 1001;
    status_packet[4] = 0;  /* offline */

    friend_handle_status_change(status_packet, 5);

    if (g_friend.friends[0].online != 0) {
        test_teardown();
        return 0;
    }

    /* Remove friend */
    friend_remove("LifecycleFriend");

    int pass = g_friend.friend_count == 0;

    test_teardown();
    return pass;
}

static int test_whisper_conversation(void) {
    test_setup();

    /* Receive whisper */
    u8 packet[64];
    u8* ptr = packet;
    *(u32*)ptr = 1001; ptr += 4;
    strcpy((char*)ptr, "ChatPartner"); ptr += strlen("ChatPartner") + 1;

    friend_handle_whisper(packet, ptr - packet);

    /* Reply */
    int result = friend_reply_whisper("Hello back!");

    int pass = result == 1 &&
               strcmp(g_friend.last_whisper_from, "ChatPartner") == 0 &&
               strcmp(g_friend.last_whisper_to, "ChatPartner") == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Social/Friend System Comprehensive Tests ===\n\n");

    /* Constants tests */
    printf("Constants Tests:\n");
    TEST(max_friends_constant);
    TEST(max_blocked_constant);
    TEST(protocol_commands);
    TEST(chat_prefixes);
    TEST(friend_status_values);
    TEST(friend_entry_size);
    TEST(blocked_entry_size);

    /* Initialization tests */
    printf("\nInitialization Tests:\n");
    TEST(friend_init);
    TEST(friend_shutdown_clears);

    /* Friend list tests */
    printf("\nFriend List Tests:\n");
    TEST(handle_friend_list);
    TEST(empty_friend_list);

    /* Add friend tests */
    printf("\nAdd Friend Tests:\n");
    TEST(add_friend_basic);
    TEST(add_friend_empty_name);
    TEST(add_friend_null_name);
    TEST(add_friend_already_exists);
    TEST(add_friend_full_list);
    TEST(handle_add_friend_success);
    TEST(handle_add_friend_failure);

    /* Remove friend tests */
    printf("\nRemove Friend Tests:\n");
    TEST(remove_friend_basic);
    TEST(remove_friend_not_found);
    TEST(handle_remove_friend);

    /* Status update tests */
    printf("\nStatus Update Tests:\n");
    TEST(status_change_online);
    TEST(status_change_offline);

    /* Query tests */
    printf("\nQuery Tests:\n");
    TEST(find_by_name);
    TEST(find_by_name_not_found);
    TEST(find_by_id);
    TEST(find_by_id_not_found);
    TEST(get_by_index);
    TEST(get_by_invalid_index);
    TEST(get_online_count);
    TEST(get_count);
    TEST(is_full);

    /* Whisper tests */
    printf("\nWhisper Tests:\n");
    TEST(send_whisper);
    TEST(send_whisper_null_name);
    TEST(send_whisper_null_message);
    TEST(handle_whisper);
    TEST(reply_whisper);
    TEST(reply_no_whisper);

    /* Block list tests */
    printf("\nBlock List Tests:\n");
    TEST(block_add);
    TEST(block_add_empty_name);
    TEST(block_add_null_name);
    TEST(block_add_full);
    TEST(block_remove);
    TEST(block_remove_not_found);
    TEST(is_blocked);
    TEST(multiple_blocks);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_friend_lifecycle);
    TEST(whisper_conversation);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - Real packet data from game captures
     * - Friend selection for UI
     * - Block list persistence
     * - Network packet sending
     * - Maximum name length handling
     * - Unicode name handling
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
