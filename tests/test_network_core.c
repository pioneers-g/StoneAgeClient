/*
 * Stone Age Client - Unit Tests for Network Core System
 * Test file: test_network_core.c
 *
 * Tests for network core loop, buffer management, protocol dispatcher
 * Based on reverse engineering of sa_9061.exe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/network.h"

/* ========================================
 * Test Cases for Network Constants
 * ======================================== */

/*
 * Test 1: Network buffer size
 */
static void test_network_buffer_size(void) {
    TEST_BEGIN("Network buffer size");

    /* Buffer sizes from FUN_0045e880 */
    TEST_ASSERT(NETWORK_BUFFER_SIZE == 0x4000, "Buffer size should be 0x4000 (16384)");
    TEST_ASSERT(NETWORK_RECV_SIZE == 0x1FFF, "Recv size should be 0x1FFF (8191)");

    TEST_END();
}

/*
 * Test 2: Connection state constants
 */
static void test_connection_state_constants(void) {
    TEST_BEGIN("Connection state constants");

    /* Connection states from DAT_04ebffd8 */
    TEST_ASSERT(CONN_STATE_DISCONNECTED == 0, "Disconnected state should be 0");
    TEST_ASSERT(CONN_STATE_CONNECTING == 1, "Connecting state should be 1");
    TEST_ASSERT(CONN_STATE_CONNECTED == 3, "Connected state should be 3");

    TEST_END();
}

/*
 * Test 3: Heartbeat timeout
 */
static void test_heartbeat_timeout(void) {
    TEST_BEGIN("Heartbeat timeout");

    /* Heartbeat interval from FUN_0045e880: 30000ms */
    TEST_ASSERT(HEARTBEAT_INTERVAL == 30000, "Heartbeat should be 30000ms");

    TEST_END();
}

/*
 * Test 4: WSA error codes
 */
static void test_wsa_error_codes(void) {
    TEST_BEGIN("WSA error codes");

    /* WSA error codes from FUN_0045e880 */
    TEST_ASSERT(WSAEWOULDBLOCK == 0x2733, "WSAEWOULDBLOCK should be 0x2733 (10035)");

    TEST_END();
}

/* ========================================
 * Test Cases for Network Buffer Management
 * ======================================== */

/*
 * Test 5: Buffer append (FUN_0045ec80)
 */
static void test_buffer_append(void) {
    TEST_BEGIN("Buffer append");

    NetworkContext ctx;
    memset(&ctx, 0, sizeof(NetworkContext));
    ctx.connected = 1;

    const char* data = "Hello";
    int result = network_buffer_append(&ctx, data, 5);

    TEST_ASSERT(result == 0, "Buffer append should succeed");
    TEST_ASSERT(ctx.recv_len == 5, "Recv length should be 5");

    TEST_END();
}

/*
 * Test 6: Buffer append overflow
 */
static void test_buffer_append_overflow(void) {
    TEST_BEGIN("Buffer append overflow");

    NetworkContext ctx;
    memset(&ctx, 0, sizeof(NetworkContext));
    ctx.connected = 1;
    ctx.recv_len = 0x3FFF;  /* Near max */

    char data[100];
    memset(data, 'A', 100);
    int result = network_buffer_append(&ctx, data, 100);

    TEST_ASSERT(result != 0, "Buffer append overflow should fail");

    TEST_END();
}

/*
 * Test 7: Buffer shift (FUN_0045ed80)
 */
static void test_buffer_shift(void) {
    TEST_BEGIN("Buffer shift");

    NetworkContext ctx;
    memset(&ctx, 0, sizeof(NetworkContext));
    ctx.connected = 1;
    strcpy(ctx.recv_buffer, "Hello World");
    ctx.recv_len = 11;

    int result = network_buffer_shift(&ctx, 6);  /* Remove "Hello " */

    TEST_ASSERT(result == 0, "Buffer shift should succeed");
    TEST_ASSERT(ctx.recv_len == 5, "Recv length should be 5");
    TEST_ASSERT(strncmp(ctx.recv_buffer, "World", 5) == 0, "Buffer should contain 'World'");

    TEST_END();
}

/*
 * Test 8: Send buffer shift (FUN_0045ede0)
 */
static void test_send_buffer_shift(void) {
    TEST_BEGIN("Send buffer shift");

    NetworkContext ctx;
    memset(&ctx, 0, sizeof(NetworkContext));
    ctx.connected = 1;
    strcpy(ctx.send_buffer, "Hello World");
    ctx.send_len = 11;

    int result = network_send_buffer_shift(&ctx, 6);

    TEST_ASSERT(result == 0, "Send buffer shift should succeed");
    TEST_ASSERT(ctx.send_len == 5, "Send length should be 5");

    TEST_END();
}

/*
 * Test 9: Extract packet (FUN_0045ee40)
 */
static void test_extract_packet(void) {
    TEST_BEGIN("Extract packet");

    NetworkContext ctx;
    memset(&ctx, 0, sizeof(NetworkContext));
    ctx.connected = 1;
    strcpy(ctx.recv_buffer, "XYF|test\r\nmore");
    ctx.recv_len = 13;

    char packet[256];
    int result = network_extract_packet(&ctx, packet, sizeof(packet));

    TEST_ASSERT(result == 0, "Extract packet should succeed");
    TEST_ASSERT(strcmp(packet, "XYF|test") == 0, "Packet should be 'XYF|test'");
    TEST_ASSERT(ctx.recv_len == 4, "Remaining length should be 4");

    TEST_END();
}

/*
 * Test 10: Extract packet no newline
 */
static void test_extract_packet_no_newline(void) {
    TEST_BEGIN("Extract packet no newline");

    NetworkContext ctx;
    memset(&ctx, 0, sizeof(NetworkContext));
    ctx.connected = 1;
    strcpy(ctx.recv_buffer, "incomplete");
    ctx.recv_len = 9;

    char packet[256];
    int result = network_extract_packet(&ctx, packet, sizeof(packet));

    TEST_ASSERT(result != 0, "Extract should fail without newline");
    TEST_ASSERT(ctx.recv_len == 9, "Buffer should be unchanged");

    TEST_END();
}

/* ========================================
 * Test Cases for Protocol Parsing
 * ======================================== */

/*
 * Test 11: Parse text field (FUN_00489f70)
 */
static void test_parse_text_field(void) {
    TEST_BEGIN("Parse text field");

    const char* data = "field1|field2|field3";
    char output[64];

    int result = parse_text_field(data, 0, '|', output, sizeof(output));
    TEST_ASSERT(result == 0, "Parse first field should succeed");
    TEST_ASSERT(strcmp(output, "field1") == 0, "First field should be 'field1'");

    result = parse_text_field(data, 1, '|', output, sizeof(output));
    TEST_ASSERT(result == 0, "Parse second field should succeed");
    TEST_ASSERT(strcmp(output, "field2") == 0, "Second field should be 'field2'");

    TEST_END();
}

/*
 * Test 12: Parse integer field
 */
static void test_parse_int_field(void) {
    TEST_BEGIN("Parse integer field");

    const char* data = "123|456|789";
    int value;

    value = parse_field_int(data, 0, '|');
    TEST_ASSERT(value == 123, "First int should be 123");

    value = parse_field_int(data, 1, '|');
    TEST_ASSERT(value == 456, "Second int should be 456");

    TEST_END();
}

/*
 * Test 13: Parse Base-62 field
 */
static void test_parse_base62_field(void) {
    TEST_BEGIN("Parse Base-62 field");

    /* Base-62 encoding from FUN_0048a0a0 */
    u32 value = parse_base62("1A");
    TEST_ASSERT(value > 0, "Base62 decode should return positive value");

    value = parse_base62("zz");
    TEST_ASSERT(value > 0, "Base62 decode 'zz' should succeed");

    TEST_END();
}

/*
 * Test 14: String unescape (FUN_0048a170)
 */
static void test_string_unescape(void) {
    TEST_BEGIN("String unescape");

    char input[] = "hello%20world";
    char output[64];

    int result = string_unescape(input, output, sizeof(output));
    TEST_ASSERT(result == 0, "Unescape should succeed");
    TEST_ASSERT(strcmp(output, "hello world") == 0, "Should unescape %20 to space");

    TEST_END();
}

/* ========================================
 * Test Cases for Protocol Commands
 * ======================================== */

/*
 * Test 15: Protocol command constants
 */
static void test_protocol_command_constants(void) {
    TEST_BEGIN("Protocol command constants");

    /* Protocol commands from FUN_0043bf90 */
    TEST_ASSERT(strcmp(CMD_CLIENTLOGIN, "ClientLogin") == 0, "ClientLogin command");
    TEST_ASSERT(strcmp(CMD_CREATENEWCHAR, "CreateNewChar") == 0, "CreateNewChar command");
    TEST_ASSERT(strcmp(CMD_CHARDELETE, "CharDelete") == 0, "CharDelete command");
    TEST_ASSERT(strcmp(CMD_CHARLOGIN, "CharLogin") == 0, "CharLogin command");
    TEST_ASSERT(strcmp(CMD_CHARLIST, "CharList") == 0, "CharList command");
    TEST_ASSERT(strcmp(CMD_CHARLOGOUT, "CharLogout") == 0, "CharLogout command");
    TEST_ASSERT(strcmp(CMD_PROCGET, "ProcGet") == 0, "ProcGet command");
    TEST_ASSERT(strcmp(CMD_PLAYERNUMGET, "PlayerNumGet") == 0, "PlayerNumGet command");

    TEST_END();
}

/*
 * Test 16: Protocol dispatcher lookup
 */
static void test_protocol_dispatcher_lookup(void) {
    TEST_BEGIN("Protocol dispatcher lookup");

    ProtocolHandler handler = protocol_find_handler("ClientLogin");
    TEST_ASSERT(handler != NULL, "ClientLogin handler should exist");

    handler = protocol_find_handler("CharList");
    TEST_ASSERT(handler != NULL, "CharList handler should exist");

    handler = protocol_find_handler("UnknownCommand");
    TEST_ASSERT(handler == NULL, "Unknown command should return NULL");

    TEST_END();
}

/*
 * Test 17: Protocol field count
 */
static void test_protocol_field_count(void) {
    TEST_BEGIN("Protocol field count");

    const char* data = "a|b|c|d|e";
    int count = count_fields(data, '|');

    TEST_ASSERT(count == 5, "Should have 5 fields");

    count = count_fields("single", '|');
    TEST_ASSERT(count == 1, "Single field should return 1");

    count = count_fields("", '|');
    TEST_ASSERT(count == 0, "Empty string should return 0");

    TEST_END();
}

/* ========================================
 * Test Cases for Connection State
 * ======================================== */

/*
 * Test 18: Connection initialization
 */
static void test_connection_init(void) {
    TEST_BEGIN("Connection initialization");

    NetworkContext ctx;
    memset(&ctx, 0xFF, sizeof(NetworkContext));
    network_init_context(&ctx);

    TEST_ASSERT(ctx.socket == INVALID_SOCKET, "Socket should be INVALID_SOCKET");
    TEST_ASSERT(ctx.connected == 0, "Should not be connected");
    TEST_ASSERT(ctx.recv_len == 0, "Recv length should be 0");
    TEST_ASSERT(ctx.send_len == 0, "Send length should be 0");

    TEST_END();
}

/*
 * Test 19: Connection state transitions
 */
static void test_connection_state_transitions(void) {
    TEST_BEGIN("Connection state transitions");

    NetworkContext ctx;
    network_init_context(&ctx);

    TEST_ASSERT(ctx.state == CONN_STATE_DISCONNECTED, "Initial state should be disconnected");

    ctx.state = CONN_STATE_CONNECTING;
    TEST_ASSERT(ctx.state == CONN_STATE_CONNECTING, "State should be connecting");

    ctx.state = CONN_STATE_CONNECTED;
    TEST_ASSERT(ctx.state == CONN_STATE_CONNECTED, "State should be connected");

    TEST_END();
}

/*
 * Test 20: Heartbeat tracking
 */
static void test_heartbeat_tracking(void) {
    TEST_BEGIN("Heartbeat tracking");

    NetworkContext ctx;
    network_init_context(&ctx);

    ctx.last_activity = 0;
    ctx.state = CONN_STATE_CONNECTED;

    u32 current_time = 30001;  /* 30001ms since epoch */
    int needs_heartbeat = network_check_heartbeat(&ctx, current_time);

    TEST_ASSERT(needs_heartbeat != 0, "Should need heartbeat after 30000ms");

    TEST_END();
}

/* ========================================
 * Test Cases for Error Handling
 * ======================================== */

/*
 * Test 21: WSA error handling
 */
static void test_wsa_error_handling(void) {
    TEST_BEGIN("WSA error handling");

    /* WSAEWOULDBLOCK should not cause disconnect */
    int should_disconnect = network_should_disconnect(WSAEWOULDBLOCK);
    TEST_ASSERT(should_disconnect == 0, "WSAEWOULDBLOCK should not disconnect");

    /* Other errors should cause disconnect */
    should_disconnect = network_should_disconnect(WSAECONNRESET);
    TEST_ASSERT(should_disconnect != 0, "Other errors should disconnect");

    TEST_END();
}

/*
 * Test 22: Null buffer handling
 */
static void test_null_buffer_handling(void) {
    TEST_BEGIN("Null buffer handling");

    NetworkContext ctx;
    memset(&ctx, 0, sizeof(NetworkContext));

    int result = network_buffer_append(NULL, "test", 4);
    TEST_ASSERT(result != 0, "Null context should fail");

    result = network_buffer_append(&ctx, NULL, 4);
    TEST_ASSERT(result != 0, "Null data should fail");

    TEST_END();
}

/*
 * Test 23: Disconnected operation
 */
static void test_disconnected_operation(void) {
    TEST_BEGIN("Disconnected operation");

    NetworkContext ctx;
    memset(&ctx, 0, sizeof(NetworkContext));
    /* ctx.connected = 0 by default */

    const char* data = "test";
    int result = network_buffer_append(&ctx, data, 4);
    TEST_ASSERT(result != 0, "Operation on disconnected context should fail");

    TEST_END();
}

/* ========================================
 * Test Cases for Send Queue
 * ======================================== */

/*
 * Test 24: Send queue add (FUN_0045d6f0)
 */
static void test_send_queue_add(void) {
    TEST_BEGIN("Send queue add");

    NetworkContext ctx;
    network_init_context(&ctx);
    ctx.connected = 1;

    const char* packet = "XYF|test\n";
    int result = network_send_packet(&ctx, packet, strlen(packet));

    TEST_ASSERT(result == 0, "Send queue add should succeed");
    TEST_ASSERT(ctx.send_len == strlen(packet), "Send length should match");

    TEST_END();
}

/*
 * Test 25: Send queue overflow
 */
static void test_send_queue_overflow(void) {
    TEST_BEGIN("Send queue overflow");

    NetworkContext ctx;
    network_init_context(&ctx);
    ctx.connected = 1;
    ctx.send_len = 0x3FFF;

    char data[100];
    memset(data, 'A', 100);
    int result = network_send_packet(&ctx, data, 100);

    TEST_ASSERT(result != 0, "Send queue overflow should fail");

    TEST_END();
}

/* ========================================
 * Test Cases for VirtualProtect
 * ======================================== */

/*
 * Test 26: Memory protection flags
 */
static void test_memory_protection_flags(void) {
    TEST_BEGIN("Memory protection flags");

    /* VirtualProtect flags from FUN_0045e880 */
    TEST_ASSERT(PAGE_READWRITE == 4, "PAGE_READWRITE should be 4");
    TEST_ASSERT(PAGE_EXECUTE_READWRITE == 0x40, "PAGE_EXECUTE_READWRITE should be 0x40");

    TEST_END();
}

/*
 * Test 27: Memory protection region size
 */
static void test_memory_protection_region_size(void) {
    TEST_BEGIN("Memory protection region size");

    /* Buffer protection size from FUN_0045e880 */
    TEST_ASSERT(NETWORK_PROTECT_SIZE == 0x4000, "Protection size should be 0x4000");

    TEST_END();
}

/* ========================================
 * Test Cases for Large Packet Handling
 * ======================================== */

/*
 * Test 28: Large packet split (200-4391 bytes)
 */
static void test_large_packet_split(void) {
    TEST_BEGIN("Large packet split");

    /* From FUN_0045e880: packets between 200 and 0x1127 (4391) bytes
       trigger a delay and additional recv */
    TEST_ASSERT(NETWORK_LARGE_PACKET_MIN == 200, "Large packet min should be 200");
    TEST_ASSERT(NETWORK_LARGE_PACKET_MAX == 0x1127, "Large packet max should be 0x1127");

    TEST_END();
}

/*
 * Test 29: Large packet delay
 */
static void test_large_packet_delay(void) {
    TEST_BEGIN("Large packet delay");

    /* From FUN_0045e880: Sleep(2000) for large packets */
    TEST_ASSERT(NETWORK_LARGE_PACKET_DELAY == 2000, "Large packet delay should be 2000ms");

    TEST_END();
}

/*
 * Test 30: Heartbeat counter
 */
static void test_heartbeat_counter(void) {
    TEST_BEGIN("Heartbeat counter");

    NetworkContext ctx;
    network_init_context(&ctx);
    ctx.state = CONN_STATE_CONNECTED;
    ctx.login_state = 1;

    /* Simulate heartbeat increment */
    ctx.heartbeat_count = 0;
    network_send_heartbeat(&ctx);
    TEST_ASSERT(ctx.heartbeat_count == 1, "Heartbeat count should increment");

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(network_core) {
    /* Constants tests */
    test_network_buffer_size();
    test_connection_state_constants();
    test_heartbeat_timeout();
    test_wsa_error_codes();

    /* Buffer management tests */
    test_buffer_append();
    test_buffer_append_overflow();
    test_buffer_shift();
    test_send_buffer_shift();
    test_extract_packet();
    test_extract_packet_no_newline();

    /* Protocol parsing tests */
    test_parse_text_field();
    test_parse_int_field();
    test_parse_base62_field();
    test_string_unescape();

    /* Protocol commands tests */
    test_protocol_command_constants();
    test_protocol_dispatcher_lookup();
    test_protocol_field_count();

    /* Connection state tests */
    test_connection_init();
    test_connection_state_transitions();
    test_heartbeat_tracking();

    /* Error handling tests */
    test_wsa_error_handling();
    test_null_buffer_handling();
    test_disconnected_operation();

    /* Send queue tests */
    test_send_queue_add();
    test_send_queue_overflow();

    /* Memory protection tests */
    test_memory_protection_flags();
    test_memory_protection_region_size();

    /* Large packet tests */
    test_large_packet_split();
    test_large_packet_delay();
    test_heartbeat_counter();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - Network Core Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(network_core);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
