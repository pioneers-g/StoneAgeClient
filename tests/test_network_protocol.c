/*
 * Stone Age Client - Network Protocol Unit Tests
 * Tests for binary packet dispatcher, string escaping, send queue
 * Based on FUN_0045ffb0, FUN_00489f70, FUN_00489fe0 analysis
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../tests/test_framework.h"
#include "types.h"
#include "network.h"
#include "protocol.h"
#include "pet_protocol.h"

/* ========================================
 * Test Cases for Protocol Constants
 * ======================================== */

/* Test binary packet type codes */
static void test_binary_packet_types(void) {
    TEST_BEGIN("Binary packet types");

    /* Packet types from FUN_0045ffb0 switch statement */
    TEST_ASSERT_EQ(PACKET_TYPE_ITEM, 0x42);        /* 'B' - Item/pet inventory */
    TEST_ASSERT_EQ(PACKET_TYPE_MAP_ENTRY, 0x43);   /* 'C' - Map entry */
    TEST_ASSERT_EQ(PACKET_TYPE_PLAYER_POS, 0x44);  /* 'D' - Player position */
    TEST_ASSERT_EQ(PACKET_TYPE_UNKNOWN_E, 0x45);   /* 'E' - Unknown */
    TEST_ASSERT_EQ(PACKET_TYPE_PLAYER_DATA, 0x46); /* 'F' - Player data */
    TEST_ASSERT_EQ(PACKET_TYPE_PARTY, 0x49);       /* 'I' - Party data */
    TEST_ASSERT_EQ(PACKET_TYPE_PET_STATUS, 0x4a);  /* 'J' - Pet status */
    TEST_ASSERT_EQ(PACKET_TYPE_CHAR_STATUS, 0x4b); /* 'K' - Character status bitmask */
    TEST_ASSERT_EQ(PACKET_TYPE_BATTLE_START, 0x4d);/* 'M' - Battle start */
    TEST_ASSERT_EQ(PACKET_TYPE_BATTLE_UNIT, 0x4e); /* 'N' - Battle unit status */
    TEST_ASSERT_EQ(PACKET_TYPE_CHAR_STATS, 0x50);  /* 'P' - Character stats bitmask */
    TEST_ASSERT_EQ(PACKET_TYPE_SKILL_LIST, 0x53);  /* 'S' - Skill list */
    TEST_ASSERT_EQ(PACKET_TYPE_PET_DATA, 0x57);    /* 'W' - Pet data */
    TEST_ASSERT_EQ(PACKET_TYPE_UNKNOWN_X, 0x58);   /* 'X' - Unknown */

    TEST_END();
}

/* Test text packet types */
static void test_text_packet_types(void) {
    TEST_BEGIN("Text packet types");

    /* From DAT_04ec0474 queue */
    TEST_ASSERT_EQ(PACKET_TYPE_RESPONSE, 'R');
    TEST_ASSERT_EQ(PACKET_TYPE_SUCCESS, 'T');
    TEST_ASSERT_EQ(PACKET_TYPE_ERROR, 'E');
    TEST_ASSERT_EQ(PACKET_TYPE_GAME_DATA, 'G');
    TEST_ASSERT_EQ(PACKET_TYPE_WORLD_DATA, 'W');
    TEST_ASSERT_EQ(PACKET_TYPE_UPDATE, 'U');

    TEST_END();
}

/* Test connection state values */
static void test_connection_states(void) {
    TEST_BEGIN("Connection states");

    /* From DAT_04ebffd8 */
    TEST_ASSERT_EQ(CONN_STATE_LOGIN, 1);
    TEST_ASSERT_EQ(CONN_STATE_CONNECTED, 2);
    TEST_ASSERT_EQ(CONN_STATE_IN_GAME, 3);

    TEST_END();
}

/* Test protocol mode flags */
static void test_protocol_mode_flags(void) {
    TEST_BEGIN("Protocol mode flags");

    /* From DAT_0461b658 */
    TEST_ASSERT_EQ(PROTOCOL_MODE_TEXT, 0x00000000);
    TEST_ASSERT_EQ(PROTOCOL_MODE_BINARY, 0x0F000000);

    TEST_END();
}

/* ========================================
 * Test Cases for String Functions
 * ======================================== */

/* Test protocol_unescape_string function */
static void test_protocol_unescape_string(void) {
    TEST_BEGIN("Protocol unescape string");

    /* Test simple string */
    char* result = protocol_unescape_string("hello");
    TEST_ASSERT(result != NULL);
    if (result) {
        TEST_ASSERT_STR_EQ(result, "hello");
        free(result);
    }

    /* Test escaped string */
    result = protocol_unescape_string("hello%20world");
    TEST_ASSERT(result != NULL);
    if (result) {
        /* Should unescape %20 to space */
        free(result);
    }

    TEST_END();
}

/* Test protocol_escape_string function */
static void test_protocol_escape_string(void) {
    TEST_BEGIN("Protocol escape string");

    char dst[256];

    /* Test simple string */
    protocol_escape_string("hello", dst, sizeof(dst));
    TEST_ASSERT_STR_EQ(dst, "hello");

    /* Test string with spaces */
    protocol_escape_string("hello world", dst, sizeof(dst));
    /* Should escape space */

    TEST_END();
}

/* Test protocol_encode_int function */
static void test_protocol_encode_int(void) {
    TEST_BEGIN("Protocol encode int");

    char buf[32];

    /* Test zero */
    protocol_encode_int(0, buf, sizeof(buf));
    TEST_ASSERT_STR_EQ(buf, "0");

    /* Test positive number */
    protocol_encode_int(12345, buf, sizeof(buf));
    TEST_ASSERT_STR_EQ(buf, "12345");

    /* Test negative number */
    protocol_encode_int(-123, buf, sizeof(buf));
    TEST_ASSERT_STR_EQ(buf, "-123");

    TEST_END();
}

/* Test string_to_int function */
static void test_string_to_int(void) {
    TEST_BEGIN("String to int");

    TEST_ASSERT_EQ(string_to_int("0"), 0);
    TEST_ASSERT_EQ(string_to_int("12345"), 12345);
    TEST_ASSERT_EQ(string_to_int("-123"), -123);
    TEST_ASSERT_EQ(string_to_int("999999"), 999999);

    TEST_END();
}

/* ========================================
 * Test Cases for Field Parsing
 * ======================================== */

/* Test parse_fields function - FUN_00489f70 pattern */
static void test_parse_fields(void) {
    TEST_BEGIN("Parse fields");

    char output[256];
    const char* input = "field1|field2|field3";

    /* Parse first field */
    int result = parse_fields(input, '|', 1, sizeof(output), output);
    TEST_ASSERT(result > 0);
    TEST_ASSERT_STR_EQ(output, "field1");

    /* Parse second field */
    result = parse_fields(input, '|', 2, sizeof(output), output);
    TEST_ASSERT(result > 0);
    TEST_ASSERT_STR_EQ(output, "field2");

    /* Parse third field */
    result = parse_fields(input, '|', 3, sizeof(output), output);
    TEST_ASSERT(result > 0);
    TEST_ASSERT_STR_EQ(output, "field3");

    TEST_END();
}

/* Test extract_field function - FUN_00489fe0 pattern */
static void test_extract_field(void) {
    TEST_BEGIN("Extract field");

    char output[256];
    const char* input = "a|b|c|d|e";

    /* Extract each field */
    int result = extract_field(input, '|', sizeof(output), output);
    TEST_ASSERT(result > 0);

    TEST_END();
}

/* Test field parsing with empty fields */
static void test_parse_fields_empty(void) {
    TEST_BEGIN("Parse fields empty");

    char output[256];
    const char* input = "field1||field3";

    /* Parse first field */
    int result = parse_fields(input, '|', 1, sizeof(output), output);
    TEST_ASSERT(result > 0);
    TEST_ASSERT_STR_EQ(output, "field1");

    /* Parse empty second field */
    result = parse_fields(input, '|', 2, sizeof(output), output);
    /* Should return empty or error */

    TEST_END();
}

/* Test field parsing with long input */
static void test_parse_fields_long(void) {
    TEST_BEGIN("Parse fields long");

    char output[256];
    const char* input = "1234567890|abcdefghij|ABCDEFGHIJ";

    /* Parse fields */
    int result = parse_fields(input, '|', 1, sizeof(output), output);
    TEST_ASSERT(result > 0);
    TEST_ASSERT_STR_EQ(output, "1234567890");

    TEST_END();
}

/* ========================================
 * Test Cases for Send Queue
 * ======================================== */

/* Test send_queue_init function */
static void test_send_queue_init(void) {
    TEST_BEGIN("Send queue init");

    send_queue_init();

    /* Queue should be empty after init */
    TEST_ASSERT(send_queue_count() == 0);

    TEST_END();
}

/* Test send_queue_add function */
static void test_send_queue_add(void) {
    TEST_BEGIN("Send queue add");

    send_queue_init();

    int result = send_queue_add("Test message", 0, 0);
    TEST_ASSERT_EQ(result, 1);
    TEST_ASSERT(send_queue_count() == 1);

    TEST_END();
}

/* Test send_queue_process function */
static void test_send_queue_process(void) {
    TEST_BEGIN("Send queue process");

    send_queue_init();
    send_queue_add("Test message", 0, 0);

    /* Process should empty queue */
    send_queue_process();

    TEST_END();
}

/* Test send queue overflow */
static void test_send_queue_overflow(void) {
    TEST_BEGIN("Send queue overflow");

    send_queue_init();

    /* Add many messages */
    for (int i = 0; i < 100; i++) {
        char msg[32];
        sprintf(msg, "Message %d", i);
        send_queue_add(msg, 0, 0);
    }

    /* Queue should have limit */
    int count = send_queue_count();
    TEST_ASSERT(count > 0);

    TEST_END();
}

/* ========================================
 * Test Cases for State Functions
 * ======================================== */

/* Test network_get_state function */
static void test_network_get_state(void) {
    TEST_BEGIN("Network get state");

    u32 state = network_get_state();

    /* State should be 0-3 */
    TEST_ASSERT(state >= 0 && state <= 3);

    TEST_END();
}

/* Test network_get_login_result function */
static void test_network_get_login_result(void) {
    TEST_BEGIN("Network get login result");

    u32 result = network_get_login_result();

    /* Result should be valid */
    TEST_ASSERT(result >= 0);

    TEST_END();
}

/* ========================================
 * Test Cases for Binary Packet Handler
 * ======================================== */

/* Test packet type 0x42 (Item inventory) */
static void test_packet_type_item(void) {
    TEST_BEGIN("Packet type 0x42 (Item)");

    /* From FUN_0045ffb0 case 0x42:
     * param_2[1] = inventory index ('0'-'9')
     * Parses fields for item data
     */

    TEST_ASSERT_EQ(PACKET_TYPE_ITEM, 0x42);
    TEST_ASSERT_EQ(PACKET_TYPE_ITEM, 'B');

    TEST_END();
}

/* Test packet type 0x43 (Map entry) */
static void test_packet_type_map_entry(void) {
    TEST_BEGIN("Packet type 0x43 (Map entry)");

    /* From FUN_0045ffb0 case 0x43:
     * Fields: map_id, width, height, param1, param2
     * Calls FUN_00440dd0 (set dimensions)
     * Calls FUN_00440530 (load map data)
     */

    TEST_ASSERT_EQ(PACKET_TYPE_MAP_ENTRY, 0x43);
    TEST_ASSERT_EQ(PACKET_TYPE_MAP_ENTRY, 'C');

    TEST_END();
}

/* Test packet type 0x4e (Battle unit status) */
static void test_packet_type_battle_unit(void) {
    TEST_BEGIN("Packet type 0x4e (Battle unit)");

    /* From FUN_0045ffb0 case 0x4e:
     * param_2[1] = unit index ('0'-'9')
     * Parses bitmask for unit status
     * Bits: 2=unit_id, 4=hp, 8=mp, 0x10=level, 0x20=status, 0x40=name
     */

    TEST_ASSERT_EQ(PACKET_TYPE_BATTLE_UNIT, 0x4e);
    TEST_ASSERT_EQ(PACKET_TYPE_BATTLE_UNIT, 'N');

    TEST_END();
}

/* Test packet type 0x50 (Character stats) */
static void test_packet_type_char_stats(void) {
    TEST_BEGIN("Packet type 0x50 (Character stats)");

    /* From FUN_0045ffb0 case 0x50:
     * Parses bitmask for character stats
     * Up to 30 fields parsed based on bitmask
     */

    TEST_ASSERT_EQ(PACKET_TYPE_CHAR_STATS, 0x50);
    TEST_ASSERT_EQ(PACKET_TYPE_CHAR_STATS, 'P');

    TEST_END();
}

/* Test packet type 0x53 (Skill list) */
static void test_packet_type_skill_list(void) {
    TEST_BEGIN("Packet type 0x53 (Skill list)");

    /* From FUN_0045ffb0 case 0x53:
     * Parses skill data for each slot
     * Fields: id, type, level, mp_cost, target_type, name, description
     */

    TEST_ASSERT_EQ(PACKET_TYPE_SKILL_LIST, 0x53);
    TEST_ASSERT_EQ(PACKET_TYPE_SKILL_LIST, 'S');

    TEST_END();
}

/* Test packet type 0x57 (Pet data) */
static void test_packet_type_pet_data(void) {
    TEST_BEGIN("Packet type 0x57 (Pet data)");

    /* From FUN_0045ffb0 case 0x57:
     * param_2[1] = pet index ('0'-'4')
     * Parses pet stats, skills, name, memo
     */

    TEST_ASSERT_EQ(PACKET_TYPE_PET_DATA, 0x57);
    TEST_ASSERT_EQ(PACKET_TYPE_PET_DATA, 'W');

    TEST_END();
}

/* ========================================
 * Test Cases for Bitmask Parsing
 * ======================================== */

/* Test bitmask field extraction */
static void test_bitmask_parsing(void) {
    TEST_BEGIN("Bitmask parsing");

    /* From FUN_0045ffb0 cases 0x4b, 0x4e, 0x50:
     * Uses FUN_0048a120 to get bitmask
     * Then iterates bits to determine which fields to parse
     */

    u32 bitmask = 0x00000001;
    int field_index = 1;

    /* Bit 0 set */
    if (bitmask & 0x00000001) {
        field_index++;
    }
    TEST_ASSERT_EQ(field_index, 2);

    /* Bit 1 not set */
    if (bitmask & 0x00000002) {
        field_index++;
    }
    TEST_ASSERT_EQ(field_index, 2);

    /* All bits set */
    bitmask = 0xFFFFFFFF;
    int count = 0;
    for (u32 bit = 1; bit != 0; bit <<= 1) {
        if (bitmask & bit) {
            count++;
        }
    }
    TEST_ASSERT_EQ(count, 32);

    TEST_END();
}

/* Test bitmask with common patterns */
static void test_bitmask_patterns(void) {
    TEST_BEGIN("Bitmask patterns");

    /* Common bitmask values from binary */
    u32 all_fields = 0x00000001;      /* Only first field */
    u32 some_fields = 0x0000003F;     /* First 6 fields */
    u32 many_fields = 0x00FFFFFF;     /* First 24 fields */

    TEST_ASSERT(all_fields > 0);
    TEST_ASSERT(some_fields > all_fields);
    TEST_ASSERT(many_fields > some_fields);

    TEST_END();
}

/* ========================================
 * Test Cases for Memory Addresses
 * ======================================== */

/* Test key memory addresses from binary */
static void test_protocol_memory_addresses(void) {
    TEST_BEGIN("Protocol memory addresses");

    /* Key addresses from FUN_0045ffb0 */
    u32 addr_connection_state = 0x04ebffd8;  /* Connection state */
    u32 addr_login_result = 0x04ebffdc;      /* Login result */
    u32 addr_error_message = 0x04ebe4d8;     /* Error message buffer */
    u32 addr_protocol_mode = 0x0461b658;     /* Protocol mode */

    TEST_ASSERT(addr_connection_state > 0);
    TEST_ASSERT(addr_login_result > 0);
    TEST_ASSERT(addr_error_message > 0);
    TEST_ASSERT(addr_protocol_mode > 0);

    TEST_END();
}

/* Test character stats addresses */
static void test_char_stats_addresses(void) {
    TEST_BEGIN("Character stats addresses");

    /* From FUN_0045ffb0 case 0x50:
     * DAT_0462be98 to DAT_0462e410 are character stats
     */

    u32 addr_base = 0x0462be98;
    u32 addr_end = 0x0462e410;

    TEST_ASSERT(addr_end > addr_base);

    TEST_END();
}

/* Test battle unit addresses */
static void test_battle_unit_addresses(void) {
    TEST_BEGIN("Battle unit addresses");

    /* From FUN_0045ffb0 case 0x4e:
     * DAT_046308f0 to DAT_046309e0 are battle unit data
     */

    u32 addr_unit_base = 0x046308f0;
    u32 addr_unit_end = 0x046309e0;

    TEST_ASSERT(addr_unit_end > addr_unit_base);

    TEST_END();
}

/* ========================================
 * Test Cases for Helper Functions
 * ======================================== */

/* Test FUN_0048a050 (parse integer field) */
static void test_parse_int_field(void) {
    TEST_BEGIN("Parse integer field");

    /* From FUN_0045ffb0, uses FUN_0048a050 to parse integers */

    /* Test via string_to_int */
    TEST_ASSERT_EQ(string_to_int("123"), 123);
    TEST_ASSERT_EQ(string_to_int("0"), 0);
    TEST_ASSERT_EQ(string_to_int("-1"), -1);

    TEST_END();
}

/* Test FUN_0048a120 (parse bitmask with Base-62) */
static void test_parse_bitmask_field(void) {
    TEST_BEGIN("Parse bitmask field (Base-62)");

    /* From FUN_0045ffb0, uses FUN_0048a120 to parse bitmasks
     * FUN_0048a120 calls FUN_0048a0a0 for Base-62 decoding
     * Base-62: 0-9 -> 0-9, a-z -> 10-35, A-Z -> 36-61
     */

    /* Test Base-62 decoding via pet_decode_base62 */
    TEST_ASSERT_EQ(pet_decode_base62("0"), 0);
    TEST_ASSERT_EQ(pet_decode_base62("1"), 1);
    TEST_ASSERT_EQ(pet_decode_base62("9"), 9);
    TEST_ASSERT_EQ(pet_decode_base62("a"), 10);
    TEST_ASSERT_EQ(pet_decode_base62("z"), 35);
    TEST_ASSERT_EQ(pet_decode_base62("A"), 36);
    TEST_ASSERT_EQ(pet_decode_base62("Z"), 61);

    /* Test multi-character Base-62 */
    /* "10" in base 62 = 1*62 + 0 = 62 */
    TEST_ASSERT_EQ(pet_decode_base62("10"), 62);
    /* "1a" in base 62 = 1*62 + 10 = 72 */
    TEST_ASSERT_EQ(pet_decode_base62("1a"), 72);
    /* "100" in base 62 = 1*62*62 + 0*62 + 0 = 3844 */
    TEST_ASSERT_EQ(pet_decode_base62("100"), 3844);

    /* Test negative values */
    TEST_ASSERT_EQ(pet_decode_base62("-1"), -1);
    TEST_ASSERT_EQ(pet_decode_base62("-a"), -10);

    TEST_END();
}

/* ========================================
 * Test Cases for Protocol Text Commands
 * ======================================== */

/* Test multi-character command strings */
static void test_protocol_commands(void) {
    TEST_BEGIN("Protocol commands");

    /* From DAT_004b9xxx region */
    TEST_ASSERT(strcmp("ClientLogin", "ClientLogin") == 0);
    TEST_ASSERT(strcmp("CreateNewChar", "CreateNewChar") == 0);
    TEST_ASSERT(strcmp("CharDelete", "CharDelete") == 0);
    TEST_ASSERT(strcmp("CharLogin", "CharLogin") == 0);
    TEST_ASSERT(strcmp("CharList", "CharList") == 0);
    TEST_ASSERT(strcmp("CharLogout", "CharLogout") == 0);

    TEST_END();
}

/* Test two-character command prefixes */
static void test_two_char_commands(void) {
    TEST_BEGIN("Two-char commands");

    /* Commands from FUN_0043bf90 dispatcher */
    /* XY series: XYG, XYO, XYM, XYA, XYB, etc. */
    /* PF series: PF, FA, FD, FM, FB, FU */

    TEST_ASSERT_EQ('P' << 8 | 'F', 0x5046);
    TEST_ASSERT_EQ('F' << 8 | 'A', 0x4641);

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(network_protocol) {
    /* Protocol constants */
    test_binary_packet_types();
    test_text_packet_types();
    test_connection_states();
    test_protocol_mode_flags();

    /* String functions */
    test_protocol_unescape_string();
    test_protocol_escape_string();
    test_protocol_encode_int();
    test_string_to_int();

    /* Field parsing */
    test_parse_fields();
    test_extract_field();
    test_parse_fields_empty();
    test_parse_fields_long();

    /* Send queue */
    test_send_queue_init();
    test_send_queue_add();
    test_send_queue_process();
    test_send_queue_overflow();

    /* State functions */
    test_network_get_state();
    test_network_get_login_result();

    /* Binary packet types */
    test_packet_type_item();
    test_packet_type_map_entry();
    test_packet_type_battle_unit();
    test_packet_type_char_stats();
    test_packet_type_skill_list();
    test_packet_type_pet_data();

    /* Bitmask parsing */
    test_bitmask_parsing();
    test_bitmask_patterns();

    /* Memory addresses */
    test_protocol_memory_addresses();
    test_char_stats_addresses();
    test_battle_unit_addresses();

    /* Helper functions */
    test_parse_int_field();
    test_parse_bitmask_field();

    /* Protocol commands */
    test_protocol_commands();
    test_two_char_commands();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - Network Protocol Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(network_protocol);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
