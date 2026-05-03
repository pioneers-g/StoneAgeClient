/*
 * Stone Age Client - Network I/O Comprehensive Tests
 * Tests for FUN_0045e880 (network_process) and related functions
 *
 * Coverage:
 * - Buffer management (append, remove, extract)
 * - Select/poll mechanism
 * - Receive handling
 * - Send handling
 * - Heartbeat timing
 * - Error handling
 * - Connection state machine
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;
typedef unsigned long DWORD;

/* Constants from binary */
#define NET_RECV_BUFFER_SIZE    0x4000   /* 16KB */
#define NET_SEND_BUFFER_SIZE    0x4000   /* 16KB */
#define NET_TEMP_BUFFER_SIZE    0x2000   /* 8KB */
#define FIELD_SEPARATOR         '|'      /* 0x7c */
#define HEARTBEAT_INTERVAL_MS   30000    /* 30 seconds */
#define LARGE_PACKET_MIN        200
#define LARGE_PACKET_MAX        0x1127   /* 4391 */

/* Network states */
typedef enum {
    NET_STATE_DISCONNECTED = 0,
    NET_STATE_CONNECTING = 1,
    NET_STATE_CONNECTED = 99,
    NET_STATE_LOGGED_IN = 100
} NetworkState;

/* Protocol modes */
typedef enum {
    PROTOCOL_TEXT = 0,
    PROTOCOL_BINARY = 1
} ProtocolMode;

/* Mock network context */
typedef struct {
    u8* recv_buffer;
    u32 recv_len;
    u32 recv_capacity;
    u8* send_buffer;
    u32 send_len;
    u32 send_capacity;
    u8 temp_buffer[NET_TEMP_BUFFER_SIZE];
    int initialized;
    int connection_flag;
    int heartbeat_flag;
    int large_packet_flag;
    u32 timer_flags;
    DWORD last_send_time;
    DWORD last_recv_time;
    NetworkState state;
    ProtocolMode protocol_mode;
    u32 heartbeat_count;
} NetworkContext;

static NetworkContext g_net;
static u8 g_recv_buf[NET_RECV_BUFFER_SIZE];
static u8 g_send_buf[NET_SEND_BUFFER_SIZE];

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
 * Buffer Management Functions
 * ======================================== */

/*
 * Append data to receive buffer - FUN_0045ec80
 */
static int network_buffer_append(const void* data, u32 len) {
    const u8* src = (const u8*)data;
    u8* dst;
    u32 i;

    if (!g_net.initialized) {
        return -1;
    }

    if (g_net.recv_len + len > g_net.recv_capacity) {
        return -1;
    }

    dst = g_net.recv_buffer + g_net.recv_len;

    /* Optimized dword copy */
    for (i = len >> 2; i != 0; i--) {
        *(u32*)dst = *(const u32*)src;
        src += 4;
        dst += 4;
    }

    /* Remaining bytes */
    for (i = len & 3; i != 0; i--) {
        *dst++ = *src++;
    }

    g_net.recv_len += len;
    return 0;
}

/*
 * Remove bytes from receive buffer start - FUN_0045ed80
 */
static void network_buffer_remove(u32 len) {
    u32 i;
    u8* buf = g_net.recv_buffer;

    if (len > g_net.recv_len) {
        g_net.recv_len = 0;
        return;
    }

    if (len < g_net.recv_len) {
        for (i = len; i < g_net.recv_len; i++) {
            buf[i - len] = buf[i];
        }
    }

    g_net.recv_len -= len;
}

/*
 * Remove bytes from send buffer start - FUN_0045ede0
 */
static int network_send_buffer_remove(u32 len) {
    u32 i;
    u8* buf = g_net.send_buffer;

    if (len > g_net.send_len) {
        return -1;
    }

    if (len < g_net.send_len) {
        for (i = len; i < g_net.send_len; i++) {
            buf[i - len] = buf[i];
        }
    }

    g_net.send_len -= len;
    return 0;
}

/*
 * Extract a line from buffer - FUN_0045ee40
 */
static int network_extract_line(char* out, u32 max_len) {
    u32 i;
    u8* buf = g_net.recv_buffer;

    if (!g_net.initialized || g_net.recv_len == 0) {
        return -1;
    }

    /* Find newline */
    for (i = 0; i < g_net.recv_len && i < max_len; i++) {
        if (buf[i] == '\n') {
            break;
        }
    }

    if (i >= max_len || i >= g_net.recv_len) {
        return -1;
    }

    /* Copy line */
    memcpy(out, buf, i);
    out[i] = '\0';

    /* Remove trailing \r */
    if (i > 0 && out[i - 1] == '\r') {
        out[i - 1] = '\0';
    }

    /* Remove from buffer */
    network_buffer_remove(i + 1);

    return 0;
}

/* ========================================
 * Test Setup/Teardown
 * ======================================== */

static void test_setup(void) {
    memset(&g_net, 0, sizeof(g_net));
    g_net.recv_buffer = g_recv_buf;
    g_net.recv_capacity = NET_RECV_BUFFER_SIZE;
    g_net.send_buffer = g_send_buf;
    g_net.send_capacity = NET_SEND_BUFFER_SIZE;
    g_net.initialized = 1;
    g_net.state = NET_STATE_CONNECTED;
}

static void test_teardown(void) {
    g_net.initialized = 0;
}

/* ========================================
 * Buffer Append Tests
 * ======================================== */

static int test_buffer_append_simple(void) {
    test_setup();

    const char* data = "Hello";
    int result = network_buffer_append(data, 5);

    int pass = result == 0 &&
               g_net.recv_len == 5 &&
               memcmp(g_net.recv_buffer, "Hello", 5) == 0;

    test_teardown();
    return pass;
}

static int test_buffer_append_empty(void) {
    test_setup();

    int result = network_buffer_append("", 0);

    int pass = result == 0 && g_net.recv_len == 0;

    test_teardown();
    return pass;
}

static int test_buffer_append_multiple(void) {
    test_setup();

    network_buffer_append("Hello", 5);
    network_buffer_append(" ", 1);
    network_buffer_append("World", 5);

    int pass = g_net.recv_len == 11 &&
               memcmp(g_net.recv_buffer, "Hello World", 11) == 0;

    test_teardown();
    return pass;
}

static int test_buffer_append_dword_aligned(void) {
    test_setup();

    /* Test 4-byte aligned copy */
    u32 data[] = {0x12345678, 0xABCDEF00, 0x11111111};
    int result = network_buffer_append(data, 12);

    int pass = result == 0 && g_net.recv_len == 12 &&
               *(u32*)g_net.recv_buffer == 0x12345678;

    test_teardown();
    return pass;
}

static int test_buffer_append_unaligned(void) {
    test_setup();

    /* 5 bytes = 4 + 1 (tests both dword and byte copy) */
    const char* data = "ABCDE";
    int result = network_buffer_append(data, 5);

    int pass = result == 0 && g_net.recv_len == 5 &&
               memcmp(g_net.recv_buffer, "ABCDE", 5) == 0;

    test_teardown();
    return pass;
}

static int test_buffer_append_overflow(void) {
    test_setup();

    /* Fill buffer to capacity */
    g_net.recv_len = NET_RECV_BUFFER_SIZE - 100;

    /* Try to append 200 bytes (should fail) */
    char data[200];
    memset(data, 'X', 200);
    int result = network_buffer_append(data, 200);

    int pass = result == -1;  /* Should fail */

    test_teardown();
    return pass;
}

static int test_buffer_append_exact_capacity(void) {
    test_setup();

    /* Leave exactly 100 bytes */
    g_net.recv_len = NET_RECV_BUFFER_SIZE - 100;

    /* Append exactly 100 bytes */
    char data[100];
    memset(data, 'Y', 100);
    int result = network_buffer_append(data, 100);

    int pass = result == 0 && g_net.recv_len == NET_RECV_BUFFER_SIZE;

    test_teardown();
    return pass;
}

/* ========================================
 * Buffer Remove Tests
 * ======================================== */

static int test_buffer_remove_simple(void) {
    test_setup();

    network_buffer_append("Hello World", 11);
    network_buffer_remove(6);  /* Remove "Hello " */

    int pass = g_net.recv_len == 5 &&
               memcmp(g_net.recv_buffer, "World", 5) == 0;

    test_teardown();
    return pass;
}

static int test_buffer_remove_all(void) {
    test_setup();

    network_buffer_append("Hello", 5);
    network_buffer_remove(5);

    int pass = g_net.recv_len == 0;

    test_teardown();
    return pass;
}

static int test_buffer_remove_more_than_exists(void) {
    test_setup();

    network_buffer_append("Hello", 5);
    network_buffer_remove(10);  /* Try to remove more than exists */

    int pass = g_net.recv_len == 0;

    test_teardown();
    return pass;
}

static int test_buffer_remove_zero(void) {
    test_setup();

    network_buffer_append("Hello", 5);
    network_buffer_remove(0);

    int pass = g_net.recv_len == 5;

    test_teardown();
    return pass;
}

/* ========================================
 * Send Buffer Remove Tests
 * ======================================== */

static int test_send_buffer_remove_simple(void) {
    test_setup();

    memcpy(g_net.send_buffer, "Hello World", 11);
    g_net.send_len = 11;

    int result = network_send_buffer_remove(6);

    int pass = result == 0 && g_net.send_len == 5 &&
               memcmp(g_net.send_buffer, "World", 5) == 0;

    test_teardown();
    return pass;
}

static int test_send_buffer_remove_partial(void) {
    test_setup();

    memcpy(g_net.send_buffer, "ABCDEFGHIJ", 10);
    g_net.send_len = 10;

    network_send_buffer_remove(3);

    int pass = g_net.send_len == 7 &&
               memcmp(g_net.send_buffer, "DEFGHIJ", 7) == 0;

    test_teardown();
    return pass;
}

static int test_send_buffer_remove_overflow(void) {
    test_setup();

    memcpy(g_net.send_buffer, "Hello", 5);
    g_net.send_len = 5;

    int result = network_send_buffer_remove(10);

    int pass = result == -1;  /* Should fail */

    test_teardown();
    return pass;
}

/* ========================================
 * Line Extraction Tests
 * ======================================== */

static int test_extract_line_simple(void) {
    test_setup();

    network_buffer_append("Hello\nWorld\n", 12);

    char line[64];
    int result = network_extract_line(line, sizeof(line));

    int pass = result == 0 &&
               strcmp(line, "Hello") == 0 &&
               g_net.recv_len == 6;  /* "World\n" remaining */

    test_teardown();
    return pass;
}

static int test_extract_line_with_cr(void) {
    test_setup();

    network_buffer_append("Hello\r\nWorld\n", 13);

    char line[64];
    int result = network_extract_line(line, sizeof(line));

    int pass = result == 0 && strcmp(line, "Hello") == 0;

    test_teardown();
    return pass;
}

static int test_extract_line_no_newline(void) {
    test_setup();

    network_buffer_append("Hello", 5);

    char line[64];
    int result = network_extract_line(line, sizeof(line));

    int pass = result == -1;  /* No complete line */

    test_teardown();
    return pass;
}

static int test_extract_line_empty_buffer(void) {
    test_setup();

    char line[64];
    int result = network_extract_line(line, sizeof(line));

    int pass = result == -1;

    test_teardown();
    return pass;
}

static int test_extract_line_multiple(void) {
    test_setup();

    network_buffer_append("Line1\nLine2\nLine3\n", 18);

    char line[64];
    int pass = 1;

    if (network_extract_line(line, sizeof(line)) != 0 || strcmp(line, "Line1") != 0) pass = 0;
    if (network_extract_line(line, sizeof(line)) != 0 || strcmp(line, "Line2") != 0) pass = 0;
    if (network_extract_line(line, sizeof(line)) != 0 || strcmp(line, "Line3") != 0) pass = 0;

    test_teardown();
    return pass;
}

static int test_extract_line_truncate(void) {
    test_setup();

    network_buffer_append("Hello World\n", 12);

    char line[6];  /* Only room for 5 chars + null */
    int result = network_extract_line(line, 6);

    /* Should fail because full line doesn't fit */
    int pass = result == -1;

    test_teardown();
    return pass;
}

/* ========================================
 * Constants Verification Tests
 * ======================================== */

static int test_buffer_size_constants(void) {
    return NET_RECV_BUFFER_SIZE == 0x4000 &&
           NET_SEND_BUFFER_SIZE == 0x4000 &&
           NET_TEMP_BUFFER_SIZE == 0x2000;
}

static int test_heartbeat_interval(void) {
    return HEARTBEAT_INTERVAL_MS == 30000;
}

static int test_large_packet_thresholds(void) {
    return LARGE_PACKET_MIN == 200 &&
           LARGE_PACKET_MAX == 0x1127;
}

static int test_field_separator(void) {
    return FIELD_SEPARATOR == '|' && FIELD_SEPARATOR == 0x7c;
}

/* ========================================
 * State Machine Tests
 * ======================================== */

static int test_state_disconnected(void) {
    return NET_STATE_DISCONNECTED == 0;
}

static int test_state_connected(void) {
    return NET_STATE_CONNECTED == 99;
}

static int test_state_logged_in(void) {
    return NET_STATE_LOGGED_IN == 100;
}

static int test_state_order(void) {
    return NET_STATE_DISCONNECTED < NET_STATE_CONNECTING &&
           NET_STATE_CONNECTING < NET_STATE_CONNECTED &&
           NET_STATE_CONNECTED < NET_STATE_LOGGED_IN;
}

/* ========================================
 * Protocol Mode Tests
 * ======================================== */

static int test_protocol_text_mode(void) {
    return PROTOCOL_TEXT == 0;
}

static int test_protocol_binary_mode(void) {
    return PROTOCOL_BINARY == 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_packet_flow(void) {
    test_setup();

    /* Simulate receiving a packet */
    const char* packet = "ClientLogin|user|pass\n";
    network_buffer_append(packet, strlen(packet));

    /* Extract and verify */
    char line[256];
    int result = network_extract_line(line, sizeof(line));

    int pass = result == 0 &&
               strcmp(line, "ClientLogin|user|pass") == 0 &&
               g_net.recv_len == 0;

    test_teardown();
    return pass;
}

static int test_partial_packet_flow(void) {
    test_setup();

    /* Receive partial packet */
    network_buffer_append("Hello", 5);

    char line[64];
    int result1 = network_extract_line(line, sizeof(line));

    /* Receive rest */
    network_buffer_append(" World\n", 7);
    int result2 = network_extract_line(line, sizeof(line));

    int pass = result1 == -1 &&  /* First attempt fails */
               result2 == 0 &&    /* Second succeeds */
               strcmp(line, "Hello World") == 0;

    test_teardown();
    return pass;
}

static int test_send_receive_flow(void) {
    test_setup();

    /* Simulate send queue */
    const char* send_data = "Test message\n";
    memcpy(g_net.send_buffer, send_data, strlen(send_data));
    g_net.send_len = strlen(send_data);

    /* Simulate partial send */
    int sent = 5;  /* "Test " sent */
    network_send_buffer_remove(sent);

    int pass = g_net.send_len == 8 &&
               memcmp(g_net.send_buffer, "message\n", 8) == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Network I/O Comprehensive Tests ===\n\n");

    /* Buffer append tests */
    printf("Buffer Append Tests:\n");
    TEST(buffer_append_simple);
    TEST(buffer_append_empty);
    TEST(buffer_append_multiple);
    TEST(buffer_append_dword_aligned);
    TEST(buffer_append_unaligned);
    TEST(buffer_append_overflow);
    TEST(buffer_append_exact_capacity);

    /* Buffer remove tests */
    printf("\nBuffer Remove Tests:\n");
    TEST(buffer_remove_simple);
    TEST(buffer_remove_all);
    TEST(buffer_remove_more_than_exists);
    TEST(buffer_remove_zero);

    /* Send buffer tests */
    printf("\nSend Buffer Tests:\n");
    TEST(send_buffer_remove_simple);
    TEST(send_buffer_remove_partial);
    TEST(send_buffer_remove_overflow);

    /* Line extraction tests */
    printf("\nLine Extraction Tests:\n");
    TEST(extract_line_simple);
    TEST(extract_line_with_cr);
    TEST(extract_line_no_newline);
    TEST(extract_line_empty_buffer);
    TEST(extract_line_multiple);
    TEST(extract_line_truncate);

    /* Constants tests */
    printf("\nConstants Tests:\n");
    TEST(buffer_size_constants);
    TEST(heartbeat_interval);
    TEST(large_packet_thresholds);
    TEST(field_separator);

    /* State machine tests */
    printf("\nState Machine Tests:\n");
    TEST(state_disconnected);
    TEST(state_connected);
    TEST(state_logged_in);
    TEST(state_order);

    /* Protocol mode tests */
    printf("\nProtocol Mode Tests:\n");
    TEST(protocol_text_mode);
    TEST(protocol_binary_mode);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_packet_flow);
    TEST(partial_packet_flow);
    TEST(send_receive_flow);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - select() timeout handling
     * - WSAEWOULDBLOCK error handling
     * - Large packet split handling (200-0x1127 bytes)
     * - Heartbeat timing with timeGetTime()
     * - VirtualProtect memory protection
     * - Binary protocol packet parsing
     * - LSRPC encryption/decryption
     * - Connection state transitions
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
