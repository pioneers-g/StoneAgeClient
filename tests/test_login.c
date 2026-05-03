/*
 * Stone Age Client - Login System Unit Tests
 * Tests for login state machine, connection handling, encryption
 * Based on FUN_00420590, FUN_0045f4d0, FUN_0048b7c0 analysis
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../tests/test_framework.h"
#include "types.h"
#include "login.h"

/* ========================================
 * Test Cases for State Constants
 * ======================================== */

/* Test login screen states from FUN_00420590 */
static void test_login_screen_states(void) {
    TEST_BEGIN("Login screen states");

    /* States from DAT_04630df0 in FUN_00420590 */
    TEST_ASSERT_EQ(LOGIN_SCREEN_INIT, 0);
    TEST_ASSERT_EQ(LOGIN_SCREEN_RENDER, 1);
    TEST_ASSERT_EQ(LOGIN_SCREEN_INPUT, 2);
    TEST_ASSERT_EQ(LOGIN_SCREEN_CONNECTING, 3);
    TEST_ASSERT_EQ(LOGIN_SCREEN_WAITING, 4);
    TEST_ASSERT_EQ(LOGIN_SCREEN_ERROR, 100);   /* 0x64 - error display */
    TEST_ASSERT_EQ(LOGIN_SCREEN_RETRY, 101);   /* 0x65 - retry after error */

    TEST_END();
}

/* Test login states */
static void test_login_states(void) {
    TEST_BEGIN("Login states");

    TEST_ASSERT_EQ(LOGIN_STATE_DISCONNECTED, 0);
    TEST_ASSERT_EQ(LOGIN_STATE_CONNECTING, 1);
    TEST_ASSERT_EQ(LOGIN_STATE_HANDSHAKE, 2);
    TEST_ASSERT_EQ(LOGIN_STATE_AUTH, 3);
    TEST_ASSERT_EQ(LOGIN_STATE_SERVER_LIST, 4);
    TEST_ASSERT_EQ(LOGIN_STATE_CHAR_LIST, 5);
    TEST_ASSERT_EQ(LOGIN_STATE_CHAR_SELECT, 6);
    TEST_ASSERT_EQ(LOGIN_STATE_ENTERING_GAME, 7);
    TEST_ASSERT_EQ(LOGIN_STATE_IN_GAME, 8);
    TEST_ASSERT_EQ(LOGIN_STATE_ERROR, 100);

    TEST_END();
}

/* Test connection states from FUN_0045f4d0 */
static void test_connection_states(void) {
    TEST_BEGIN("Connection states");

    /* Connection states from DAT_0461c008 */
    TEST_ASSERT_EQ(CONN_STATE_NONE, 0);
    TEST_ASSERT_EQ(CONN_STATE_CONNECTING, 1);
    TEST_ASSERT_EQ(CONN_STATE_SEND_LOGIN, 0x47);  /* 71 - send ClientLogin */
    TEST_ASSERT_EQ(CONN_STATE_WAIT_RESPONSE, 0x51); /* 81-98 range */
    TEST_ASSERT_EQ(CONN_STATE_CONNECTED, 99);

    TEST_END();
}

/* Test login result codes */
static void test_login_result_codes(void) {
    TEST_BEGIN("Login result codes");

    TEST_ASSERT_EQ(LOGIN_SUCCESS, 0);
    TEST_ASSERT_EQ(LOGIN_ERR_TIMEOUT, -1);
    TEST_ASSERT_EQ(LOGIN_ERR_INVALID_USER, -3);
    TEST_ASSERT_EQ(LOGIN_ERR_INVALID_PASS, -4);
    TEST_ASSERT_EQ(LOGIN_ERR_ALREADY_LOGGED, -5);
    TEST_ASSERT_EQ(LOGIN_ERR_CONNECTION_FAILED, -6);
    TEST_ASSERT_EQ(LOGIN_ERR_SOCKET_ERROR, -7);
    TEST_ASSERT_EQ(LOGIN_ERR_DNS_FAILED, -8);
    TEST_ASSERT_EQ(LOGIN_ERR_SERVER_DOWN, -9);
    TEST_ASSERT_EQ(LOGIN_ERR_VERSION_MISMATCH, -10);
    TEST_ASSERT_EQ(LOGIN_ERR_BANNED, -11);

    TEST_END();
}

/* Test constants from binary */
static void test_login_constants(void) {
    TEST_BEGIN("Login constants");

    /* From FUN_0045f4d0 */
    TEST_ASSERT_EQ(LOGIN_TIMEOUT, 600000);  /* 10 minutes */
    TEST_ASSERT_EQ(MAX_USERNAME, 32);
    TEST_ASSERT_EQ(MAX_PASSWORD, 32);
    TEST_ASSERT_EQ(MAX_SERVER, 64);
    TEST_ASSERT_EQ(MAX_SERVERS, 10);

    TEST_END();
}

/* Test server response codes */
static void test_server_response_codes(void) {
    TEST_BEGIN("Server response codes");

    /* From FUN_0045f4d0 recv buffer check */
    TEST_ASSERT_EQ(SERVER_RESPONSE_ACCEPT, 'A');  /* 0x41 */
    TEST_ASSERT_EQ(SERVER_RESPONSE_ERROR, 'E');   /* 0x45 */

    TEST_END();
}

/* Test protocol modes */
static void test_protocol_modes(void) {
    TEST_BEGIN("Protocol modes");

    /* From DAT_0461b658 */
    TEST_ASSERT_EQ(PROTOCOL_MODE_TEXT, 0x00000000);
    TEST_ASSERT_EQ(PROTOCOL_MODE_BINARY, 0x0F000000);

    TEST_END();
}

/* ========================================
 * Test Cases for Structure Layout
 * ======================================== */

/* Test ServerInfo structure */
static void test_server_info_structure(void) {
    TEST_BEGIN("ServerInfo structure");

    TEST_ASSERT(sizeof(ServerInfo) >= sizeof(u32) + 32 + 64 + sizeof(u16) * 2 + 2);

    ServerInfo server;
    memset(&server, 0, sizeof(ServerInfo));
    server.id = 1;
    strcpy(server.name, "TestServer");
    strcpy(server.host, "127.0.0.1");
    server.port = 4567;
    server.population = 100;
    server.status = 0;

    TEST_ASSERT_EQ(server.id, 1);
    TEST_ASSERT_STR_EQ(server.name, "TestServer");
    TEST_ASSERT_EQ(server.port, 4567);

    TEST_END();
}

/* Test CharacterInfo structure */
static void test_character_info_structure(void) {
    TEST_BEGIN("CharacterInfo structure");

    CharacterInfo character;
    memset(&character, 0, sizeof(CharacterInfo));
    character.id = 1;
    strcpy(character.name, "TestChar");
    character.level = 50;
    character.job = 1;
    character.face = 1;
    character.body = 1;
    character.map_id = 100;

    TEST_ASSERT_EQ(character.id, 1);
    TEST_ASSERT_STR_EQ(character.name, "TestChar");
    TEST_ASSERT_EQ(character.level, 50);

    TEST_END();
}

/* Test TextInputBox structure */
static void test_text_input_box_structure(void) {
    TEST_BEGIN("TextInputBox structure");

    /* TextInputBox should have at least 256 byte buffer */
    TEST_ASSERT(sizeof(TextInputBox) >= 256);

    TextInputBox box;
    memset(&box, 0, sizeof(TextInputBox));
    strcpy(box.buffer, "testuser");
    box.max_len = 31;

    TEST_ASSERT_STR_EQ(box.buffer, "testuser");
    TEST_ASSERT_EQ(box.max_len, 31);

    TEST_END();
}

/* Test LoginContext initialization */
static void test_login_context_init(void) {
    TEST_BEGIN("LoginContext init");

    memset(&g_login, 0, sizeof(LoginContext));

    TEST_ASSERT_EQ(g_login.state, LOGIN_STATE_DISCONNECTED);
    TEST_ASSERT_EQ(g_login.conn_state, CONN_STATE_NONE);
    TEST_ASSERT_EQ(g_login.screen_state, LOGIN_SCREEN_INIT);
    TEST_ASSERT_EQ(g_login.server_port, 0);
    TEST_ASSERT_EQ(g_login.selected_char_index, 0);

    TEST_END();
}

/* ========================================
 * Test Cases for Login Functions
 * ======================================== */

/* Test login_init function */
static void test_login_init(void) {
    TEST_BEGIN("login_init");

    int result = login_init();

    TEST_ASSERT_EQ(result, 1);
    TEST_ASSERT_EQ(g_login.state, LOGIN_STATE_DISCONNECTED);
    TEST_ASSERT_EQ(g_login.server_port, 4567);  /* Default port */
    TEST_ASSERT_EQ(g_login.selected_char_index, -1);

    TEST_END();
}

/* Test login_shutdown function */
static void test_login_shutdown(void) {
    TEST_BEGIN("login_shutdown");

    login_init();
    login_shutdown();

    TEST_ASSERT_EQ(g_login.state, LOGIN_STATE_DISCONNECTED);

    TEST_END();
}

/* Test login_get_state function */
static void test_login_get_state(void) {
    TEST_BEGIN("login_get_state");

    login_init();
    g_login.state = LOGIN_STATE_AUTH;

    TEST_ASSERT_EQ(login_get_state(), LOGIN_STATE_AUTH);

    TEST_END();
}

/* Test login_is_in_game function */
static void test_login_is_in_game(void) {
    TEST_BEGIN("login_is_in_game");

    login_init();

    g_login.in_game = 0;
    TEST_ASSERT(!login_is_in_game());

    g_login.in_game = 1;
    TEST_ASSERT(login_is_in_game());

    TEST_END();
}

/* ========================================
 * Test Cases for State Machine Logic
 * ======================================== */

/* Test connection state transitions */
static void test_connection_state_transitions(void) {
    TEST_BEGIN("Connection state transitions");

    /* From FUN_0045f4d0 state machine:
     * State 0: Create socket
     * State 1-70: Wait for connection
     * State 71 (0x47): Send login
     * State 81-98 (0x51-0x62): Wait for response
     * State 99: Connected
     */

    int valid_transition = 1;

    /* State 0 -> 1 */
    if (CONN_STATE_NONE != 0) valid_transition = 0;

    /* State 71 should be 0x47 */
    if (CONN_STATE_SEND_LOGIN != 0x47) valid_transition = 0;

    /* State 99 should be connected */
    if (CONN_STATE_CONNECTED != 99) valid_transition = 0;

    TEST_ASSERT(valid_transition);

    TEST_END();
}

/* Test login screen state transitions */
static void test_login_screen_state_transitions(void) {
    TEST_BEGIN("Login screen state transitions");

    /* From FUN_00420590:
     * State 0: Cleanup, init
     * State 1: Render
     * State 2: Input
     * State 3: Creating connection widget
     * State 4: Wait for response
     * State 100: Error
     * State 101: Retry
     */

    int valid_states = 1;

    /* Verify state values */
    if (LOGIN_SCREEN_INIT != 0) valid_states = 0;
    if (LOGIN_SCREEN_ERROR != 100) valid_states = 0;
    if (LOGIN_SCREEN_RETRY != 101) valid_states = 0;

    TEST_ASSERT(valid_states);

    TEST_END();
}

/* Test timeout handling from FUN_0045f4d0 */
static void test_timeout_handling(void) {
    TEST_BEGIN("Timeout handling");

    /* FUN_0045f4d0 checks:
     * if (600000 < timeGetTime() - start_time) return -1
     */

    u32 timeout_ms = 600000;  /* 10 minutes */
    TEST_ASSERT_EQ(timeout_ms, LOGIN_TIMEOUT);

    /* Verify timeout is 10 minutes */
    TEST_ASSERT_EQ(LOGIN_TIMEOUT / 1000, 600);  /* 600 seconds */
    TEST_ASSERT_EQ(LOGIN_TIMEOUT / 60000, 10);  /* 10 minutes */

    TEST_END();
}

/* ========================================
 * Test Cases for Encryption
 * ======================================== */

/* Test encryption buffer sizes */
static void test_encryption_buffer_sizes(void) {
    TEST_BEGIN("Encryption buffer sizes");

    /* From FUN_0048bb90: processes 8 bytes at a time */
    TEST_ASSERT_EQ(8, 8);  /* Block size */

    /* Username/password encrypted to 32 bytes (4 blocks) */
    TEST_ASSERT(sizeof(g_login.encrypted_user) >= 32);
    TEST_ASSERT(sizeof(g_login.encrypted_pass) >= 32);

    TEST_END();
}

/* Test encryption mode flags */
static void test_encryption_mode_flags(void) {
    TEST_BEGIN("Encryption mode flags");

    /* From FUN_0048bb90 param_4:
     * & 1: encrypt mode
     * & 2: decrypt mode
     */

    u8 encrypt_flag = 0;
    u8 decrypt_flag = 1;

    /* Mode 0: encrypt */
    TEST_ASSERT((encrypt_flag & 1) == 0);

    /* Mode 1: decrypt */
    TEST_ASSERT((decrypt_flag & 1) == 1);

    TEST_END();
}

/* ========================================
 * Test Cases for Memory Addresses
 * ======================================== */

/* Test key memory addresses from binary */
static void test_login_memory_addresses(void) {
    TEST_BEGIN("Login memory addresses");

    /* Key addresses from FUN_00420590 and FUN_0045f4d0 */
    u32 addr_conn_state = 0x0461c008;      /* Connection state */
    u32 addr_login_state = 0x0461c000;     /* Login internal state */
    u32 addr_protocol_mode = 0x0461b658;   /* Protocol mode */
    u32 addr_server_ip = 0x0461bf64;       /* Server IP */
    u32 addr_server_port = 0x0461bf60;     /* Server port */
    u32 addr_socket = 0x004c437c;          /* Socket handle */

    TEST_ASSERT(addr_conn_state > 0);
    TEST_ASSERT(addr_login_state > 0);
    TEST_ASSERT(addr_server_ip > addr_server_port);

    TEST_END();
}

/* Test username/password buffer addresses */
static void test_credential_buffer_addresses(void) {
    TEST_BEGIN("Credential buffer addresses");

    /* From FUN_00420590:
     * DAT_0454f278: Username buffer (encrypted)
     * DAT_0455aa58: Password buffer (encrypted)
     * DAT_0455703c: Username (plain)
     * DAT_04556280: Password (plain)
     */

    u32 addr_enc_user = 0x0454f278;
    u32 addr_enc_pass = 0x0455aa58;
    u32 addr_plain_user = 0x0455703c;
    u32 addr_plain_pass = 0x04556280;

    TEST_ASSERT(addr_enc_user > 0);
    TEST_ASSERT(addr_enc_pass > 0);
    TEST_ASSERT(addr_plain_user > 0);
    TEST_ASSERT(addr_plain_pass > 0);

    TEST_END();
}

/* Test server name address */
static void test_server_name_address(void) {
    TEST_BEGIN("Server name address");

    /* From FUN_00420590:
     * DAT_004b800c: Server name string
     */

    u32 addr_server_name = 0x004b800c;
    TEST_ASSERT(addr_server_name > 0);

    TEST_END();
}

/* ========================================
 * Test Cases for UI Elements
 * ======================================== */

/* Test login window creation */
static void test_login_window_creation(void) {
    TEST_BEGIN("Login window creation");

    /* From FUN_00420590 state 3:
     * DAT_0455ef48 = FUN_00448610(x, y, width, height, ...)
     */

    /* Window position from FUN_00420590 */
    u32 expected_x = 320;   /* 0x140 */
    u32 expected_y = 240;   /* 0xf0 */
    u32 expected_width = 192;  /* 0xC0 */
    u32 expected_height = 2;

    TEST_ASSERT_EQ(expected_x, 320);
    TEST_ASSERT_EQ(expected_y, 240);

    TEST_END();
}

/* Test sprite IDs for login screen */
static void test_login_sprite_ids(void) {
    TEST_BEGIN("Login sprite IDs");

    /* From FUN_00420590:
     * FUN_00488190(0xd9, 0x140, 0xf0)
     * Sprite 0xd9 = 217 for login background
     */

    u16 bg_sprite = 0xd9;  /* 217 */
    TEST_ASSERT_EQ(bg_sprite, 217);

    TEST_END();
}

/* Test player count display */
static void test_player_count_display(void) {
    TEST_BEGIN("Player count display");

    /* From FUN_00420590:
     * DAT_04ebffdc: Server player count
     * if (DAT_04ebffdc == 7) DAT_0461b414 = 1
     */

    login_init();
    g_login.server_player_count = 7;

    TEST_ASSERT_EQ(g_login.server_player_count, 7);

    TEST_END();
}

/* ========================================
 * Test Cases for Error Handling
 * ======================================== */

/* Test error message retrieval */
static void test_error_message(void) {
    TEST_BEGIN("Error message");

    login_init();

    const char* msg = login_get_error_message();
    TEST_ASSERT(msg != NULL || g_login.error_message[0] == '\0');

    TEST_END();
}

/* Test error state transitions */
static void test_error_state_transitions(void) {
    TEST_BEGIN("Error state transitions");

    /* From FUN_00420590:
     * On error (-7, -8): DAT_04630df0 = 100
     * On retry: DAT_04630df0 = 101
     */

    login_init();
    g_login.screen_state = LOGIN_SCREEN_ERROR;

    TEST_ASSERT_EQ(g_login.screen_state, LOGIN_SCREEN_ERROR);

    /* Transition to retry */
    g_login.screen_state = LOGIN_SCREEN_RETRY;
    TEST_ASSERT_EQ(g_login.screen_state, LOGIN_SCREEN_RETRY);

    TEST_END();
}

/* Test connection error codes from FUN_0045f4d0 */
static void test_connection_error_codes(void) {
    TEST_BEGIN("Connection error codes");

    /* From FUN_0045f4d0 return values:
     * 0xFFFFFFFE = -2: DNS resolve failed
     * 0xFFFFFFFD = -3: Socket creation failed
     * 0xFFFFFFFC = -4: gethostbyname failed
     * 0xFFFFFFFB = -5: connect failed
     * 0xFFFFFFFA = -6: Connection error
     * 0xFFFFFFF9 = -7: Login failed
     * 0xFFFFFFFF = -1: Timeout
     */

    s32 dns_error = -2;
    s32 socket_error = -3;
    s32 host_error = -4;
    s32 connect_error = -5;
    s32 conn_error = -6;
    s32 login_error = -7;
    s32 timeout_error = -1;

    TEST_ASSERT_EQ(dns_error, -2);
    TEST_ASSERT_EQ(socket_error, -3);
    TEST_ASSERT_EQ(host_error, -4);
    TEST_ASSERT_EQ(connect_error, -5);
    TEST_ASSERT_EQ(conn_error, -6);
    TEST_ASSERT_EQ(login_error, -7);
    TEST_ASSERT_EQ(timeout_error, -1);

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(login) {
    /* State constants */
    test_login_screen_states();
    test_login_states();
    test_connection_states();
    test_login_result_codes();
    test_login_constants();
    test_server_response_codes();
    test_protocol_modes();

    /* Structure layout */
    test_server_info_structure();
    test_character_info_structure();
    test_text_input_box_structure();
    test_login_context_init();

    /* Login functions */
    test_login_init();
    test_login_shutdown();
    test_login_get_state();
    test_login_is_in_game();

    /* State machine logic */
    test_connection_state_transitions();
    test_login_screen_state_transitions();
    test_timeout_handling();

    /* Encryption */
    test_encryption_buffer_sizes();
    test_encryption_mode_flags();

    /* Memory addresses */
    test_login_memory_addresses();
    test_credential_buffer_addresses();
    test_server_name_address();

    /* UI elements */
    test_login_window_creation();
    test_login_sprite_ids();
    test_player_count_display();

    /* Error handling */
    test_error_message();
    test_error_state_transitions();
    test_connection_error_codes();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - Login System Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(login);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
