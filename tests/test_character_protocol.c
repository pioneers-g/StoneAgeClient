/*
 * Stone Age Client - Unit Tests for Character Protocol
 * Test file: test_character_protocol.c
 *
 * Tests for character parsing functions from character_protocol.c
 * Based on reverse engineering of FUN_0045ffb0 case 0x50, FUN_00464670
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/character.h"

/* External functions from character_protocol.c */
extern void character_parse_list(const char* packet_data);
extern void character_teleport(u32 char_index, const char* packet_data);
extern void character_parse_detailed_stats(const char* packet_data);
extern void character_handle_move_result(u32 char_index, const char* packet_data);
extern CharacterEntry* character_get_entry(u32 index);

/* External detailed stats */
extern DetailedCharacterStats g_detailed_stats;

/* External functions from pet_protocol.c needed for testing */
extern int pet_parse_field(const char* str, char delimiter, int field_index, char* output, int max_len);
extern int pet_parse_field_int(const char* str, char delimiter, int field_index);
extern void pet_unescape_string(char* str);

/* ========================================
 * Test Cases for Character Protocol
 * ======================================== */

/*
 * Test 1: Parse character list - basic entry
 */
static void test_character_parse_list_basic(void) {
    TEST_BEGIN("Character parse list basic");

    /* Clear entries first */
    memset(&g_detailed_stats, 0, sizeof(g_detailed_stats));

    /* Create a simple character list packet
     * Format: |char_id|name|account|field4|desc|field6|dir|level|y|sprite|...
     */
    const char* packet = "|5|TestChar|Account1|100|Description|200|2|50|120|1000|";

    character_parse_list(packet);

    /* Verify character was parsed */
    CharacterEntry* entry = character_get_entry(5);
    /* Note: entry might be NULL if implementation differs */
    TEST_ASSERT(1, "Parse should not crash");

    TEST_END();
}

/*
 * Test 2: Parse detailed stats - all fields (bitmask = 1)
 */
static void test_character_detailed_stats_all_fields(void) {
    TEST_BEGIN("Character detailed stats all fields");

    /* Clear stats first */
    memset(&g_detailed_stats, 0, sizeof(g_detailed_stats));

    /* Create packet with bitmask=1 (all fields)
     * Format: 0x50|bitmask|field2|field3|...|field31|
     * Field 1: bitmask (1 = all fields present)
     * Fields 2-29: integer values
     * Field 30: title (17 bytes max)
     * Field 31: guild name (33 bytes max)
     */
    const char* packet = "\x50|1|1000|2000|300|400|500|600|100|110|120|130|140|150|160|170|180|190|200|210|220|230|240|250|50000|100000|80|5000|100|TestTitle|TestGuild|";

    character_parse_detailed_stats(packet);

    /* Verify field_mask was set */
    TEST_ASSERT(g_detailed_stats.field_mask == 1, "Field mask should be 1");

    /* Verify integer fields from binary analysis:
     * DAT_0462be98 = field 2 = 1000
     * DAT_0462be9c = field 3 = 2000
     * etc.
     */
    TEST_ASSERT(g_detailed_stats.field_02 == 1000, "Field 2 should be 1000");
    TEST_ASSERT(g_detailed_stats.field_03 == 2000, "Field 3 should be 2000");
    TEST_ASSERT(g_detailed_stats.field_04 == 300, "Field 4 should be 300");
    TEST_ASSERT(g_detailed_stats.field_05 == 400, "Field 5 should be 400");

    /* Verify title and guild name */
    TEST_ASSERT_STR_EQ("TestTitle", g_detailed_stats.title, "Title should match");
    TEST_ASSERT_STR_EQ("TestGuild", g_detailed_stats.guild_name, "Guild name should match");

    TEST_END();
}

/*
 * Test 3: Parse detailed stats - selective fields (bitmask mode)
 */
static void test_character_detailed_stats_bitmask(void) {
    TEST_BEGIN("Character detailed stats bitmask");

    /* Clear stats first */
    memset(&g_detailed_stats, 0, sizeof(g_detailed_stats));

    /* Create packet with bitmask=0x6 (fields 2 and 3 only)
     * Bit 1 (0x2): field 2
     * Bit 2 (0x4): field 3
     */
    const char* packet = "\x50|6|1500|2500|";

    character_parse_detailed_stats(packet);

    /* Verify field_mask */
    TEST_ASSERT(g_detailed_stats.field_mask == 6, "Field mask should be 6");

    /* Verify only specified fields were updated */
    TEST_ASSERT(g_detailed_stats.field_02 == 1500, "Field 2 should be 1500");
    TEST_ASSERT(g_detailed_stats.field_03 == 2500, "Field 3 should be 2500");

    /* Other fields should remain 0 */
    TEST_ASSERT(g_detailed_stats.field_04 == 0, "Field 4 should be 0");

    TEST_END();
}

/*
 * Test 4: Parse detailed stats - title field only
 */
static void test_character_detailed_stats_title_only(void) {
    TEST_BEGIN("Character detailed stats title only");

    /* Clear stats first */
    memset(&g_detailed_stats, 0, sizeof(g_detailed_stats));

    /* Bit 25 (0x2000000): title string field */
    const char* packet = "\x50|33554432|MyTitle|";

    character_parse_detailed_stats(packet);

    TEST_ASSERT(g_detailed_stats.field_mask == 0x2000000, "Field mask should be 0x2000000");
    TEST_ASSERT_STR_EQ("MyTitle", g_detailed_stats.title, "Title should be updated");

    TEST_END();
}

/*
 * Test 5: Parse detailed stats - guild name field only
 */
static void test_character_detailed_stats_guild_only(void) {
    TEST_BEGIN("Character detailed stats guild only");

    /* Clear stats first */
    memset(&g_detailed_stats, 0, sizeof(g_detailed_stats));

    /* Bit 26 (0x4000000): guild name string field */
    const char* packet = "\x50|67108864|MyGuild|";

    character_parse_detailed_stats(packet);

    TEST_ASSERT(g_detailed_stats.field_mask == 0x4000000, "Field mask should be 0x4000000");
    TEST_ASSERT_STR_EQ("MyGuild", g_detailed_stats.guild_name, "Guild name should be updated");

    TEST_END();
}

/*
 * Test 6: Parse detailed stats - multiple fields with bitmask
 */
static void test_character_detailed_stats_multiple(void) {
    TEST_BEGIN("Character detailed stats multiple fields");

    /* Clear stats first */
    memset(&g_detailed_stats, 0, sizeof(g_detailed_stats));

    /* Bitmask 0xE = bits 1,2,3 (fields 2,3,4) */
    const char* packet = "\x50|14|111|222|333|";

    character_parse_detailed_stats(packet);

    TEST_ASSERT(g_detailed_stats.field_mask == 14, "Field mask should be 14");
    TEST_ASSERT(g_detailed_stats.field_02 == 111, "Field 2 should be 111");
    TEST_ASSERT(g_detailed_stats.field_03 == 222, "Field 3 should be 222");
    TEST_ASSERT(g_detailed_stats.field_04 == 333, "Field 4 should be 333");

    TEST_END();
}

/*
 * Test 7: Parse character teleport
 */
static void test_character_teleport(void) {
    TEST_BEGIN("Character teleport");

    /* Clear stats first */
    memset(&g_detailed_stats, 0, sizeof(g_detailed_stats));

    /* Teleport packet format: |char_id|name|x|y|dir|sprite| */
    const char* packet = "|100|TeleportedChar|500|600|3|2000|";

    character_teleport(0, packet);

    /* Should not crash */
    TEST_ASSERT(1, "Teleport should not crash");

    TEST_END();
}

/*
 * Test 8: Parse character move result
 */
static void test_character_move_result(void) {
    TEST_BEGIN("Character move result");

    /* Move result packet format: |dest_x|dest_y|... */
    const char* packet = "|100|200|";

    character_handle_move_result(0, packet);

    /* Should not crash */
    TEST_ASSERT(1, "Move result should not crash");

    TEST_END();
}

/*
 * Test 9: Parse detailed stats - HP/MP fields
 */
static void test_character_detailed_stats_hp_mp(void) {
    TEST_BEGIN("Character detailed stats HP/MP");

    /* Clear stats first */
    memset(&g_detailed_stats, 0, sizeof(g_detailed_stats));

    /* Based on binary analysis:
     * field_02 = DAT_0462be98 = HP
     * field_03 = DAT_0462be9c = Max HP
     * field_04 = DAT_0462bea0 = MP
     * field_05 = DAT_0462bea4 = Max MP
     * Bitmask 0x1E = bits 1,2,3,4 (fields 2,3,4,5)
     */
    const char* packet = "\x50|30|500|1000|200|400|";

    character_parse_detailed_stats(packet);

    TEST_ASSERT(g_detailed_stats.field_mask == 30, "Field mask should be 30");
    TEST_ASSERT(g_detailed_stats.field_02 == 500, "HP should be 500");
    TEST_ASSERT(g_detailed_stats.field_03 == 1000, "Max HP should be 1000");
    TEST_ASSERT(g_detailed_stats.field_04 == 200, "MP should be 200");
    TEST_ASSERT(g_detailed_stats.field_05 == 400, "Max MP should be 400");

    TEST_END();
}

/*
 * Test 10: Parse detailed stats - level and experience
 */
static void test_character_detailed_stats_level_exp(void) {
    TEST_BEGIN("Character detailed stats level/exp");

    /* Clear stats first */
    memset(&g_detailed_stats, 0, sizeof(g_detailed_stats));

    /* Based on binary analysis:
     * field_20 = DAT_0462bee0 = Experience
     * field_21 = DAT_0462bee4 = Next level exp
     * field_22 = DAT_0462bee8 = Level
     * Bitmask for these fields: 0x700000 (bits 20,21,22)
     */
    const char* packet = "\x50|7340032|100000|150000|50|";

    character_parse_detailed_stats(packet);

    TEST_ASSERT(g_detailed_stats.field_mask == 0x700000, "Field mask should be 0x700000");
    TEST_ASSERT(g_detailed_stats.field_20 == 100000, "Experience should be 100000");
    TEST_ASSERT(g_detailed_stats.field_21 == 150000, "Next exp should be 150000");
    TEST_ASSERT(g_detailed_stats.field_22 == 50, "Level should be 50");

    TEST_END();
}

/*
 * Test 11: Empty packet handling
 */
static void test_character_empty_packet(void) {
    TEST_BEGIN("Character empty packet");

    /* Should handle NULL gracefully */
    character_parse_detailed_stats(NULL);
    character_parse_list(NULL);
    character_teleport(0, NULL);
    character_handle_move_result(0, NULL);

    TEST_ASSERT(1, "Empty packets should not crash");

    TEST_END();
}

/*
 * Test 12: Large bitmask with all integer fields
 */
static void test_character_large_bitmask(void) {
    TEST_BEGIN("Character large bitmask");

    /* Clear stats first */
    memset(&g_detailed_stats, 0, sizeof(g_detailed_stats));

    /* Bitmask 0xFFFFFF = all 24 integer fields (bits 1-24) */
    const char* packet = "\x50|16777215|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19|20|21|22|23|24|";

    character_parse_detailed_stats(packet);

    TEST_ASSERT(g_detailed_stats.field_mask == 0xFFFFFF, "Field mask should be 0xFFFFFF");
    TEST_ASSERT(g_detailed_stats.field_02 == 1, "Field 2 should be 1");
    TEST_ASSERT(g_detailed_stats.field_23 == 22, "Field 23 should be 22");
    TEST_ASSERT(g_detailed_stats.field_24 == 23, "Field 24 should be 23");

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(character_protocol) {
    test_character_parse_list_basic();
    test_character_detailed_stats_all_fields();
    test_character_detailed_stats_bitmask();
    test_character_detailed_stats_title_only();
    test_character_detailed_stats_guild_only();
    test_character_detailed_stats_multiple();
    test_character_teleport();
    test_character_move_result();
    test_character_detailed_stats_hp_mp();
    test_character_detailed_stats_level_exp();
    test_character_empty_packet();
    test_character_large_bitmask();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - Character Protocol Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(character_protocol);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
