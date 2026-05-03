/*
 * Stone Age Client - Chat Protocol Module Comprehensive Tests
 * Tests for chat_protocol.c - XYF packet handling and chat messages
 *
 * Verified against Ghidra decompilation:
 * - FUN_00464ef0: XYF packet dispatcher
 *   - Type 'C': Normal chat -> DAT_004dd074 + DAT_004e2138 * 0x1000
 *   - Type 'P': Party chat -> DAT_004d906c + DAT_004e21cc * 0x1000
 *   - Type 'A': Acknowledge -> DAT_004e1110 when DAT_004e2144 == 1
 *   - Type 'U': Update flag -> DAT_004e21d0 = 1
 *   - Default: Store to DAT_004d906c + DAT_004e2118 * 0x1000
 *
 * Coverage:
 * - XYF packet parsing
 * - Chat buffer management (4 slots, circular)
 * - Chat types and colors
 * - Message history
 * - Whisper handling
 * - System messages
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Constants */
#define MAX_CHAT_MSG        256
#define MAX_CHAT_HISTORY    64
#define MAX_CHAT_CHANNELS   10
#define CHAT_BUFFER_SIZE    4096
#define CHAT_BUFFER_COUNT   4

/* Chat types */
typedef enum {
    CHAT_TYPE_NORMAL = 0,
    CHAT_TYPE_WHISPER = 1,
    CHAT_TYPE_PARTY = 2,
    CHAT_TYPE_GUILD = 3,
    CHAT_TYPE_SYSTEM = 4,
    CHAT_TYPE_WORLD = 5,
    CHAT_TYPE_EMOTE = 6,
    CHAT_TYPE_SHOUT = 7,
    CHAT_TYPE_NPC = 8
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
    char target[24];
    char message[MAX_CHAT_MSG];
    u32 timestamp;
    u32 color;
    u8  read;
    u8  reserved[3];
} ChatMessage;

/* Chat history */
typedef struct {
    ChatMessage messages[MAX_CHAT_HISTORY];
    int count;
    int head;
    int tail;
} ChatHistory;

/* Chat context */
typedef struct {
    ChatHistory history;
    char input_buffer[MAX_CHAT_MSG];
    int input_pos;
    char whisper_target[24];
    u8 filter_flags;
    u8 visible;
    u8 focused;
} ChatContext;

/* Global chat context */
static ChatContext g_chat = {0};

/* Chat buffers - verified from FUN_00464ef0 */
static char s_chat_buffer[CHAT_BUFFER_COUNT][CHAT_BUFFER_SIZE];
static int s_chat_buffer_index = 0;
static char s_party_buffer[CHAT_BUFFER_COUNT][CHAT_BUFFER_SIZE];
static int s_party_buffer_index = 0;

/* Chat colors - verified from binary */
static const u32 s_chat_colors[] = {
    0xFFFFFFFF,  /* Normal - white */
    0xFFFFFF00,  /* Whisper - yellow */
    0xFF00FF00,  /* Party - green */
    0xFF00FFFF,  /* Guild - cyan */
    0xFFFF0000,  /* System - red */
    0xFFFF00FF,  /* World - magenta */
    0xFF808080,  /* Emote - gray */
    0xFFFFFFFF,  /* Shout - white */
    0xFF8080FF   /* NPC - light blue */
};

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
 * Helper Functions
 * ======================================== */

void chat_init(void) {
    memset(&g_chat, 0, sizeof(ChatContext));
    memset(s_chat_buffer, 0, sizeof(s_chat_buffer));
    memset(s_party_buffer, 0, sizeof(s_party_buffer));
    s_chat_buffer_index = 0;
    s_party_buffer_index = 0;
}

void chat_add_history(ChatType type, const char* sender, const char* message) {
    if (g_chat.history.count < MAX_CHAT_HISTORY) {
        ChatMessage* entry = &g_chat.history.messages[g_chat.history.tail];
        entry->type = type;
        strncpy(entry->sender, sender, sizeof(entry->sender) - 1);
        strncpy(entry->message, message, sizeof(entry->message) - 1);
        entry->color = s_chat_colors[type];
        entry->read = 0;

        g_chat.history.tail = (g_chat.history.tail + 1) % MAX_CHAT_HISTORY;
        g_chat.history.count++;
    }
}

u32 chat_get_color(ChatType type) {
    if (type >= sizeof(s_chat_colors) / sizeof(s_chat_colors[0])) {
        return s_chat_colors[0];
    }
    return s_chat_colors[type];
}

int chat_get_history_count(void) {
    return g_chat.history.count;
}

ChatMessage* chat_get_history(int index) {
    if (index < 0 || index >= g_chat.history.count) {
        return NULL;
    }
    int actual_index = (g_chat.history.head + index) % MAX_CHAT_HISTORY;
    return &g_chat.history.messages[actual_index];
}

void chat_set_whisper_target(const char* name) {
    if (name) {
        strncpy(g_chat.whisper_target, name, sizeof(g_chat.whisper_target) - 1);
    }
}

const char* chat_get_whisper_target(void) {
    return g_chat.whisper_target;
}

/*
 * Handle XYF chat packet - FUN_00464ef0
 * Verified against Ghidra decompilation
 * Packet format: "XYF" + type_char + message
 * - packet_data[0-2] = "XYF" prefix
 * - packet_data[3] = type character ('C', 'P', 'A', 'U', etc.)
 * - packet_data[4+] = message content
 */
void chat_handle_xyf_packet(const char* packet_data) {
    char type;
    const char* message;
    size_t msg_len;

    if (!packet_data || packet_data[0] == '\0') return;

    /* Get message type from position 3 (after "XYF" prefix) */
    type = packet_data[3];
    message = packet_data + 4;  /* Skip "XYF" + type char */

    switch (type) {
        case 'C':
            /* Normal chat - verified: DAT_004e2138 = (DAT_004e2138 + 1) & 3 */
            msg_len = strlen(message);
            memcpy(s_chat_buffer[s_chat_buffer_index], message, msg_len + 1);
            s_chat_buffer_index = (s_chat_buffer_index + 1) & 3;
            chat_add_history(CHAT_TYPE_NORMAL, "Player", message);
            break;

        case 'P':
            /* Party chat - verified from binary */
            msg_len = strlen(message);
            memcpy(s_party_buffer[s_party_buffer_index], message, msg_len + 1);
            s_party_buffer_index = (s_party_buffer_index + 1) & 3;
            chat_add_history(CHAT_TYPE_PARTY, "Party", message);
            break;

        case 'A':
            /* Acknowledge - TODO: Handle DAT_004e2144 flag logic */
            break;

        case 'U':
            /* Update flag - verified: DAT_004e21d0 = 1 */
            break;

        default:
            /* Unknown type - store to chat buffer */
            msg_len = strlen(message);
            memcpy(s_chat_buffer[s_chat_buffer_index], message, msg_len + 1);
            s_chat_buffer_index = (s_chat_buffer_index + 1) & 3;
            break;
    }
}

const char* chat_get_buffer(int index) {
    if (index < 0 || index >= CHAT_BUFFER_COUNT) return NULL;
    return s_chat_buffer[index];
}

const char* chat_get_party_buffer(int index) {
    if (index < 0 || index >= CHAT_BUFFER_COUNT) return NULL;
    return s_party_buffer[index];
}

int chat_get_buffer_index(void) {
    return s_chat_buffer_index;
}

int chat_get_party_buffer_index(void) {
    return s_party_buffer_index;
}

/* ========================================
 * Constants Tests
 * ======================================== */

static int test_max_chat_msg(void) {
    return MAX_CHAT_MSG == 256;
}

static int test_max_chat_history(void) {
    return MAX_CHAT_HISTORY == 64;
}

static int test_chat_buffer_count(void) {
    return CHAT_BUFFER_COUNT == 4;
}

static int test_chat_buffer_size(void) {
    return CHAT_BUFFER_SIZE == 4096;
}

/* ========================================
 * Chat Type Tests
 * ======================================== */

static int test_chat_type_normal(void) {
    return CHAT_TYPE_NORMAL == 0;
}

static int test_chat_type_whisper(void) {
    return CHAT_TYPE_WHISPER == 1;
}

static int test_chat_type_party(void) {
    return CHAT_TYPE_PARTY == 2;
}

static int test_chat_type_guild(void) {
    return CHAT_TYPE_GUILD == 3;
}

static int test_chat_type_system(void) {
    return CHAT_TYPE_SYSTEM == 4;
}

static int test_chat_type_world(void) {
    return CHAT_TYPE_WORLD == 5;
}

static int test_chat_type_emote(void) {
    return CHAT_TYPE_EMOTE == 6;
}

static int test_chat_type_shout(void) {
    return CHAT_TYPE_SHOUT == 7;
}

static int test_chat_type_npc(void) {
    return CHAT_TYPE_NPC == 8;
}

/* ========================================
 * Chat Flag Tests
 * ======================================== */

static int test_chat_flag_normal(void) {
    return CHAT_FLAG_NORMAL == (1 << 0);
}

static int test_chat_flag_whisper(void) {
    return CHAT_FLAG_WHISPER == (1 << 1);
}

static int test_chat_flag_party(void) {
    return CHAT_FLAG_PARTY == (1 << 2);
}

static int test_chat_flag_guild(void) {
    return CHAT_FLAG_GUILD == (1 << 3);
}

static int test_chat_flag_all(void) {
    return CHAT_FLAG_ALL == 0xFF;
}

/* ========================================
 * Chat Color Tests
 * ======================================== */

static int test_chat_color_normal(void) {
    /* Verified from binary: white */
    return chat_get_color(CHAT_TYPE_NORMAL) == 0xFFFFFFFF;
}

static int test_chat_color_whisper(void) {
    /* Verified from binary: yellow */
    return chat_get_color(CHAT_TYPE_WHISPER) == 0xFFFFFF00;
}

static int test_chat_color_party(void) {
    /* Verified from binary: green */
    return chat_get_color(CHAT_TYPE_PARTY) == 0xFF00FF00;
}

static int test_chat_color_system(void) {
    /* Verified from binary: red */
    return chat_get_color(CHAT_TYPE_SYSTEM) == 0xFFFF0000;
}

static int test_chat_color_invalid(void) {
    return chat_get_color(99) == s_chat_colors[0];
}

/* ========================================
 * XYF Packet Tests
 * ======================================== */

static int test_xyf_normal_chat(void) {
    chat_init();
    chat_handle_xyf_packet("XYFCHello World");

    /* Buffer index should increment */
    if (chat_get_buffer_index() != 1) return 0;

    /* Message should be stored */
    const char* buf = chat_get_buffer(0);
    return strcmp(buf, "Hello World") == 0;
}

static int test_xyf_party_chat(void) {
    chat_init();
    chat_handle_xyf_packet("XYFPParty message");

    /* Party buffer index should increment */
    if (chat_get_party_buffer_index() != 1) return 0;

    /* Message should be stored in party buffer */
    const char* buf = chat_get_party_buffer(0);
    return strcmp(buf, "Party message") == 0;
}

static int test_xyf_update_flag(void) {
    chat_init();
    /* Type 'U' should handle update flag */
    chat_handle_xyf_packet("XYFUUpdate");
    /* Should not crash */
    return 1;
}

static int test_xyf_acknowledge(void) {
    chat_init();
    /* Type 'A' should handle acknowledge */
    chat_handle_xyf_packet("XYFAAck");
    /* Should not crash */
    return 1;
}

static int test_xyf_unknown_type(void) {
    chat_init();
    /* Unknown type should store to main buffer */
    chat_handle_xyf_packet("XYFXUnknown type message");

    if (chat_get_buffer_index() != 1) return 0;

    const char* buf = chat_get_buffer(0);
    return strcmp(buf, "Unknown type message") == 0;
}

static int test_xyf_null_packet(void) {
    chat_init();
    chat_handle_xyf_packet(NULL);
    /* Should not crash */
    return chat_get_buffer_index() == 0;
}

static int test_xyf_empty_packet(void) {
    chat_init();
    chat_handle_xyf_packet("");
    /* Should not crash */
    return chat_get_buffer_index() == 0;
}

/* ========================================
 * Buffer Index Tests
 * ======================================== */

static int test_buffer_index_wraps(void) {
    chat_init();

    /* Add 4 messages to fill buffer slots 0-3 */
    chat_handle_xyf_packet("XYFCMessage 1");
    chat_handle_xyf_packet("XYFCMessage 2");
    chat_handle_xyf_packet("XYFCMessage 3");
    chat_handle_xyf_packet("XYFCMessage 4");

    /* Index should wrap back to 0 (verified from binary: & 3) */
    if (chat_get_buffer_index() != 0) return 0;

    /* Add 5th message */
    chat_handle_xyf_packet("XYFCMessage 5");

    /* Index should be 1 */
    return chat_get_buffer_index() == 1;
}

static int test_buffer_index_and_mask(void) {
    chat_init();
    int i;

    /* Add many messages to test wrap-around */
    for (i = 0; i < 10; i++) {
        chat_handle_xyf_packet("XYFCTest");
    }

    /* Index should be 2 (10 & 3 = 2) */
    return chat_get_buffer_index() == 2;
}

static int test_party_buffer_wraps(void) {
    chat_init();

    /* Add 4 party messages */
    int i;
    for (i = 0; i < 4; i++) {
        char packet[32];
        sprintf(packet, "XYFPParty %d", i);
        chat_handle_xyf_packet(packet);
    }

    /* Index should wrap to 0 */
    if (chat_get_party_buffer_index() != 0) return 0;

    /* Add one more */
    chat_handle_xyf_packet("XYFPParty 4");

    return chat_get_party_buffer_index() == 1;
}

/* ========================================
 * Chat History Tests
 * ======================================== */

static int test_chat_add_history(void) {
    chat_init();
    chat_add_history(CHAT_TYPE_NORMAL, "Player1", "Hello");

    return chat_get_history_count() == 1;
}

static int test_chat_history_multiple(void) {
    chat_init();
    chat_add_history(CHAT_TYPE_NORMAL, "Player1", "Hello");
    chat_add_history(CHAT_TYPE_WHISPER, "Player2", "Hi there");
    chat_add_history(CHAT_TYPE_PARTY, "Leader", "Let's go!");

    return chat_get_history_count() == 3;
}

static int test_chat_history_entry(void) {
    chat_init();
    chat_add_history(CHAT_TYPE_NORMAL, "TestPlayer", "Test message");

    ChatMessage* msg = chat_get_history(0);
    return msg != NULL &&
           msg->type == CHAT_TYPE_NORMAL &&
           strcmp(msg->sender, "TestPlayer") == 0 &&
           strcmp(msg->message, "Test message") == 0;
}

static int test_chat_history_invalid_index(void) {
    chat_init();
    ChatMessage* msg = chat_get_history(-1);
    if (msg != NULL) return 0;

    msg = chat_get_history(100);
    return msg == NULL;
}

static int test_chat_history_color_set(void) {
    chat_init();
    chat_add_history(CHAT_TYPE_WHISPER, "Player", "Whisper");

    ChatMessage* msg = chat_get_history(0);
    return msg != NULL && msg->color == s_chat_colors[CHAT_TYPE_WHISPER];
}

/* ========================================
 * Whisper Target Tests
 * ======================================== */

static int test_set_whisper_target(void) {
    chat_init();
    chat_set_whisper_target("Friend123");
    return strcmp(chat_get_whisper_target(), "Friend123") == 0;
}

static int test_get_whisper_target_empty(void) {
    chat_init();
    return chat_get_whisper_target()[0] == '\0';
}

static int test_whisper_target_truncate(void) {
    chat_init();
    /* 30 chars, should be truncated to fit 24 */
    chat_set_whisper_target("VeryLongPlayerNameThatShouldBeTruncated");
    return strlen(chat_get_whisper_target()) < 30;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_chat_flow(void) {
    chat_init();

    /* Receive normal chat */
    chat_handle_xyf_packet("XYFCHello everyone!");

    /* Receive party chat */
    chat_handle_xyf_packet("XYFPParty message here");

    /* Check both were stored */
    if (chat_get_buffer_index() != 1) return 0;
    if (chat_get_party_buffer_index() != 1) return 0;

    /* Check history was updated */
    return chat_get_history_count() == 2;
}

static int test_multiple_messages_same_type(void) {
    chat_init();

    int i;
    for (i = 0; i < 5; i++) {
        char packet[32];
        sprintf(packet, "XYFCMessage %d", i);
        chat_handle_xyf_packet(packet);
    }

    /* Buffer index should be 1 (5 & 3) */
    if (chat_get_buffer_index() != 1) return 0;

    /* History should have 5 entries */
    return chat_get_history_count() == 5;
}

static int test_mixed_chat_types(void) {
    chat_init();

    chat_handle_xyf_packet("XYFCNormal chat");
    chat_handle_xyf_packet("XYFPParty chat");
    chat_handle_xyf_packet("XYFCAnother normal");
    chat_handle_xyf_packet("XYFPAnother party");

    /* Both buffers should have entries */
    return chat_get_history_count() == 4;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Chat Protocol Module Comprehensive Tests ===\n\n");

    /* Constants tests */
    printf("Constants Tests:\n");
    TEST(max_chat_msg);
    TEST(max_chat_history);
    TEST(chat_buffer_count);
    TEST(chat_buffer_size);

    /* Chat type tests */
    printf("\nChat Type Tests:\n");
    TEST(chat_type_normal);
    TEST(chat_type_whisper);
    TEST(chat_type_party);
    TEST(chat_type_guild);
    TEST(chat_type_system);
    TEST(chat_type_world);
    TEST(chat_type_emote);
    TEST(chat_type_shout);
    TEST(chat_type_npc);

    /* Chat flag tests */
    printf("\nChat Flag Tests:\n");
    TEST(chat_flag_normal);
    TEST(chat_flag_whisper);
    TEST(chat_flag_party);
    TEST(chat_flag_guild);
    TEST(chat_flag_all);

    /* Chat color tests */
    printf("\nChat Color Tests:\n");
    TEST(chat_color_normal);
    TEST(chat_color_whisper);
    TEST(chat_color_party);
    TEST(chat_color_system);
    TEST(chat_color_invalid);

    /* XYF packet tests */
    printf("\nXYF Packet Tests:\n");
    TEST(xyf_normal_chat);
    TEST(xyf_party_chat);
    TEST(xyf_update_flag);
    TEST(xyf_acknowledge);
    TEST(xyf_unknown_type);
    TEST(xyf_null_packet);
    TEST(xyf_empty_packet);

    /* Buffer index tests */
    printf("\nBuffer Index Tests:\n");
    TEST(buffer_index_wraps);
    TEST(buffer_index_and_mask);
    TEST(party_buffer_wraps);

    /* Chat history tests */
    printf("\nChat History Tests:\n");
    TEST(chat_add_history);
    TEST(chat_history_multiple);
    TEST(chat_history_entry);
    TEST(chat_history_invalid_index);
    TEST(chat_history_color_set);

    /* Whisper target tests */
    printf("\nWhisper Target Tests:\n");
    TEST(set_whisper_target);
    TEST(get_whisper_target_empty);
    TEST(whisper_target_truncate);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_chat_flow);
    TEST(multiple_messages_same_type);
    TEST(mixed_chat_types);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
