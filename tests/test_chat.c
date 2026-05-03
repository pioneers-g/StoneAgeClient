/*
 * Stone Age Client - Chat System Unit Tests
 * Tests for chat.c, chatcommand.c
 * Reverse engineered from FUN_00464ef0, FUN_00465390, FUN_00465400
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/chat.h"

/* ========================================
 * Test Cases for Chat System
 * ======================================== */

/*
 * Test 1: Chat initialization
 */
static void test_chat_init(void) {
    TEST_BEGIN("Chat init");

    int result = chat_init();

    TEST_ASSERT(result == 1, "chat_init should return 1");
    TEST_ASSERT(g_chat.filter_flags == CHAT_FLAG_ALL, "Filter flags should be ALL");
    TEST_ASSERT(g_chat.visible == 1, "Chat should be visible");
    TEST_ASSERT(g_chat.max_lines > 0, "Max lines should be set");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 2: Chat send normal message
 */
static void test_chat_send_normal(void) {
    TEST_BEGIN("Chat send normal");

    chat_init();

    /* Normal message should be sent */
    int result = chat_send("Hello World");
    TEST_ASSERT(result == 1, "Send normal message should succeed");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 3: Chat send empty message
 */
static void test_chat_send_empty(void) {
    TEST_BEGIN("Chat send empty");

    chat_init();

    int result = chat_send("");
    TEST_ASSERT(result == 0, "Send empty message should fail");

    result = chat_send(NULL);
    TEST_ASSERT(result == 0, "Send NULL message should fail");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 4: Chat whisper
 */
static void test_chat_whisper(void) {
    TEST_BEGIN("Chat whisper");

    chat_init();

    int result = chat_whisper("Player1", "Hello");
    TEST_ASSERT(result == 1, "Whisper should succeed");

    /* Whisper without target should fail */
    result = chat_whisper(NULL, "Hello");
    TEST_ASSERT(result == 0, "Whisper without target should fail");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 5: Chat party message
 */
static void test_chat_party(void) {
    TEST_BEGIN("Chat party");

    chat_init();

    int result = chat_party("Party message");
    TEST_ASSERT(result == 1, "Party chat should succeed");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 6: Chat guild message
 */
static void test_chat_guild(void) {
    TEST_BEGIN("Chat guild");

    chat_init();

    int result = chat_guild("Guild message");
    TEST_ASSERT(result == 1, "Guild chat should succeed");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 7: Chat shout
 */
static void test_chat_shout(void) {
    TEST_BEGIN("Chat shout");

    chat_init();

    int result = chat_shout("Shout message");
    TEST_ASSERT(result == 1, "Shout should succeed");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 8: Chat emote
 */
static void test_chat_emote(void) {
    TEST_BEGIN("Chat emote");

    chat_init();

    int result = chat_emote("waves");
    TEST_ASSERT(result == 1, "Emote should succeed");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 9: Chat receive message
 */
static void test_chat_receive(void) {
    TEST_BEGIN("Chat receive");

    chat_init();
    chat_clear_history();

    chat_receive(CHAT_TYPE_NORMAL, "Player1", NULL, "Hello", 0xFFFF);

    TEST_ASSERT(g_chat.history.count == 1, "History should have 1 message");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 10: Chat system message
 */
static void test_chat_system_message(void) {
    TEST_BEGIN("Chat system message");

    chat_init();
    chat_clear_history();

    chat_system_message("Test %d", 123);

    TEST_ASSERT(g_chat.history.count == 1, "History should have system message");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 11: Chat filter
 */
static void test_chat_filter(void) {
    TEST_BEGIN("Chat filter");

    chat_init();

    /* Default should not filter anything */
    TEST_ASSERT(chat_is_filtered(CHAT_TYPE_NORMAL) == 0, "Normal should not be filtered");

    /* Set filter to block whispers */
    g_chat.filter_flags &= ~CHAT_FLAG_WHISPER;
    TEST_ASSERT(chat_is_filtered(CHAT_TYPE_WHISPER) == 1, "Whisper should be filtered");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 12: Chat history
 */
static void test_chat_history(void) {
    TEST_BEGIN("Chat history");

    chat_init();
    chat_clear_history();

    /* Add messages */
    chat_history_add("Message 1");
    chat_history_add("Message 2");
    chat_history_add("Message 3");

    TEST_ASSERT(g_chat.history_count == 3, "Should have 3 history entries");

    /* History navigation */
    const char* msg = chat_history_get_prev();
    TEST_ASSERT(msg != NULL, "Should get history message");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 13: Chat history wrap around
 */
static void test_chat_history_wrap(void) {
    TEST_BEGIN("Chat history wrap");

    chat_init();
    chat_clear_history();

    /* Add more than max */
    int i;
    for (i = 0; i < MAX_CHAT_HISTORY + 10; i++) {
        char msg[32];
        snprintf(msg, sizeof(msg), "Message %d", i);
        chat_history_add(msg);
    }

    /* Count should be capped */
    TEST_ASSERT(g_chat.history_count <= MAX_CHAT_HISTORY, "History should be capped");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 14: Chat command processing
 */
static void test_chat_command(void) {
    TEST_BEGIN("Chat command");

    chat_init();

    /* Command with / prefix */
    int result = chat_send("/help");
    TEST_ASSERT(result == 1, "Command should be processed");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 15: Chat whisper target storage
 */
static void test_chat_whisper_target(void) {
    TEST_BEGIN("Chat whisper target");

    chat_init();

    /* Receive whisper should store sender */
    chat_handle_whisper("Player1\0Hello\0", 13);

    TEST_ASSERT(strcmp(g_chat.whisper_target, "Player1") == 0, "Whisper target should be stored");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 16: Chat color mapping
 */
static void test_chat_colors(void) {
    TEST_BEGIN("Chat colors");

    u32 color;

    color = chat_get_color(CHAT_TYPE_NORMAL);
    TEST_ASSERT(color == 0xFFFF, "Normal color should be white");

    color = chat_get_color(CHAT_TYPE_WHISPER);
    TEST_ASSERT(color == 0xFFE0, "Whisper color should be yellow");

    color = chat_get_color(CHAT_TYPE_PARTY);
    TEST_ASSERT(color == 0x07FF, "Party color should be cyan");

    color = chat_get_color(CHAT_TYPE_SYSTEM);
    TEST_ASSERT(color == 0xF800, "System color should be red");

    TEST_END();
}

/*
 * Test 17: Chat channel unread count
 */
static void test_chat_unread(void) {
    TEST_BEGIN("Chat unread count");

    chat_init();
    chat_init_channels();

    chat_receive(CHAT_TYPE_NORMAL, "Player1", NULL, "Test", 0xFFFF);
    chat_receive(CHAT_TYPE_NORMAL, "Player2", NULL, "Test2", 0xFFFF);

    TEST_ASSERT(g_chat.channels[CHAT_TYPE_NORMAL].unread == 2, "Unread should be 2");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 18: Chat channel toggle
 */
static void test_chat_channel_toggle(void) {
    TEST_BEGIN("Chat channel toggle");

    chat_init();
    chat_init_channels();

    /* Disable channel */
    chat_set_channel_enabled(CHAT_TYPE_WHISPER, 0);
    TEST_ASSERT(g_chat.channels[CHAT_TYPE_WHISPER].enabled == 0, "Channel should be disabled");

    /* Enable channel */
    chat_set_channel_enabled(CHAT_TYPE_WHISPER, 1);
    TEST_ASSERT(g_chat.channels[CHAT_TYPE_WHISPER].enabled == 1, "Channel should be enabled");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 19: Chat max message length
 */
static void test_chat_max_length(void) {
    TEST_BEGIN("Chat max length");

    chat_init();

    /* Create message that's too long */
    char long_msg[MAX_CHAT_MSG + 100];
    memset(long_msg, 'A', sizeof(long_msg) - 1);
    long_msg[sizeof(long_msg) - 1] = '\0';

    int result = chat_send(long_msg);
    TEST_ASSERT(result == 0, "Long message should fail");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 20: Chat visible toggle
 */
static void test_chat_visible(void) {
    TEST_BEGIN("Chat visible");

    chat_init();

    chat_set_visible(0);
    TEST_ASSERT(g_chat.visible == 0, "Chat should be hidden");

    chat_set_visible(1);
    TEST_ASSERT(g_chat.visible == 1, "Chat should be visible");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 21: Chat type from command prefix
 */
static void test_chat_type_from_prefix(void) {
    TEST_BEGIN("Chat type from prefix");

    ChatType type;

    type = chat_type_from_command("/w Player hello");
    TEST_ASSERT(type == CHAT_TYPE_WHISPER, "Should detect whisper");

    type = chat_type_from_command("/p party message");
    TEST_ASSERT(type == CHAT_TYPE_PARTY, "Should detect party");

    type = chat_type_from_command("/g guild message");
    TEST_ASSERT(type == CHAT_TYPE_GUILD, "Should detect guild");

    type = chat_type_from_command("normal message");
    TEST_ASSERT(type == CHAT_TYPE_NORMAL, "Should detect normal");

    TEST_END();
}

/*
 * Test 22: Chat clear history
 */
static void test_chat_clear(void) {
    TEST_BEGIN("Chat clear history");

    chat_init();

    chat_add_history("Message 1");
    chat_add_history("Message 2");

    TEST_ASSERT(g_chat.history_count == 2, "Should have messages");

    chat_clear_history();

    TEST_ASSERT(g_chat.history_count == 0, "History should be cleared");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 23: Chat command parsing
 * Tests FUN_00464ef0 pattern - command prefix detection
 */
static void test_chat_command_parsing(void) {
    TEST_BEGIN("Chat command parsing");

    /* From FUN_00464ef0 analysis:
     * XYC - Chat message
     * XYP - Party chat
     * XYA - Acknowledge
     * XYU - Update
     */

    chat_init();

    /* Test command parsing based on second character */
    int result = chat_send("/help");
    TEST_ASSERT(result == 1, "/help should be valid command");

    result = chat_send("/w Player test");
    TEST_ASSERT(result == 1, "/w should parse whisper");

    result = chat_send("/p party chat");
    TEST_ASSERT(result == 1, "/p should parse party");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 24: Chat circular buffer
 * Tests DAT_004d906c and DAT_004e2118 circular buffer logic
 */
static void test_chat_circular_buffer(void) {
    TEST_BEGIN("Chat circular buffer");

    chat_init();
    chat_clear_history();

    /* Fill buffer */
    int i;
    for (i = 0; i < MAX_CHAT_HISTORY; i++) {
        chat_add_history("Test message");
    }

    /* Add more - should wrap around */
    chat_add_history("Wrap message");

    /* Buffer should still be at max capacity */
    TEST_ASSERT(g_chat.history_count == MAX_CHAT_HISTORY, "Buffer should be at max");

    chat_shutdown();
    TEST_END();
}

/*
 * Test 25: Chat message timestamp
 */
static void test_chat_timestamp(void) {
    TEST_BEGIN("Chat timestamp");

    chat_init();
    chat_clear_history();

    u32 before = timeGetTime();
    chat_add_history("Test");
    u32 after = timeGetTime();

    /* Message should have a valid timestamp */
    ChatMessage* msg = &g_chat.history.messages[0];
    TEST_ASSERT(msg->timestamp >= before && msg->timestamp <= after, "Timestamp should be valid");

    chat_shutdown();
    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(chat) {
    test_chat_init();
    test_chat_send_normal();
    test_chat_send_empty();
    test_chat_whisper();
    test_chat_party();
    test_chat_guild();
    test_chat_shout();
    test_chat_emote();
    test_chat_receive();
    test_chat_system_message();
    test_chat_filter();
    test_chat_history();
    test_chat_history_wrap();
    test_chat_command();
    test_chat_whisper_target();
    test_chat_colors();
    test_chat_unread();
    test_chat_channel_toggle();
    test_chat_max_length();
    test_chat_visible();
    test_chat_type_from_prefix();
    test_chat_clear();
    test_chat_command_parsing();
    test_chat_circular_buffer();
    test_chat_timestamp();
}

/* ========================================
 * Main
 * ======================================== */

int main(void) {
    printf("=== Chat System Unit Tests ===\n\n");
    RUN_SUITE(chat);

    printf("\n=== Test Summary ===\n");
    printf("Total:  %d\n", s_tests_run);
    printf("Passed: %d\n", s_tests_passed);
    printf("Failed: %d\n", s_tests_failed);

    if (s_tests_failed > 0) {
        printf("\nSOME TESTS FAILED!\n");
        return 1;
    }

    printf("\nAll tests passed!\n");
    return 0;
}
