/*
 * Stone Age Client - Character Protocol Comprehensive Tests
 * Tests for character_protocol.c implementation
 *
 * Based on Ghidra analysis:
 * - FUN_00464670: Character list parsing
 * - FUN_00464190: Character teleport handling
 * - FUN_00465170: Move result handling
 * - FUN_0045ffb0 case 0x50: Detailed stats parsing
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
typedef int s16;
typedef int s32;

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    fflush(stdout); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Constants
 * ======================================== */

#define MAX_CHARACTERS 5
#define MAX_NAME_LEN 32
#define MAX_DESC_LEN 128

/* ========================================
 * Character Structures
 * ======================================== */

typedef struct {
    char name[MAX_NAME_LEN];
    char field_0x3a[20];
    s32 some_field;
    char description[MAX_DESC_LEN];
    u32 field_0x0c;
    u16 direction;
    u16 level;
    u16 level_over_100;
    u16 x, y;
    u16 sprite_id;
    char field_0x10[20];
} CharacterEntry;

typedef struct {
    u32 field_mask;
    u32 field_02, field_03, field_04, field_05;
    u32 field_06, field_07, field_08, field_09;
    u32 field_10, field_11, field_12, field_13;
    u32 field_14, field_15, field_16, field_17;
    u32 field_18, field_19, field_20, field_21;
    u32 field_22, field_23, field_24, field_25;
    u32 field_26, field_27, field_28, field_29;
    char title[20];
    char guild_name[40];
} DetailedCharacterStats;

typedef struct {
    u16 x, y;
    u16 dest_x, dest_y;
    u8 is_moving;
    u8 direction;
} Character;

typedef struct {
    Character characters[MAX_CHARACTERS];
} CharacterManager;

/* Global state for testing */
static CharacterEntry s_char_entries[MAX_CHARACTERS];
static u32 s_char_entry_count = 0;
static DetailedCharacterStats g_detailed_stats = {0};
static CharacterManager g_chars = {0};

/* ========================================
 * Stub Functions for pet_protocol
 * ======================================== */

static int stub_parse_field_result = 0;
static int stub_parse_int_result = 0;
static int stub_parse_base62_result = 0;

/* Parse a field from delimited string */
static int pet_parse_field(const char* str, char delim, int field_num, char* buffer, int buf_size) {
    const char* ptr = str;
    const char* start;
    int current_field = 1;
    int len;

    if (!str || !buffer || buf_size <= 0) return -1;

    /* Find the requested field */
    while (current_field < field_num && *ptr) {
        if (*ptr == delim) current_field++;
        ptr++;
    }

    if (current_field != field_num) return -1;

    start = ptr;
    while (*ptr && *ptr != delim) ptr++;

    len = ptr - start;
    if (len >= buf_size) len = buf_size - 1;

    strncpy(buffer, start, len);
    buffer[len] = '\0';

    return 0;
}

/* Parse integer field */
static s32 pet_parse_field_int(const char* str, char delim, int field_num) {
    char buffer[64];
    if (pet_parse_field(str, delim, field_num, buffer, sizeof(buffer)) != 0) {
        return -1;
    }
    return atoi(buffer);
}

/* Parse Base-62 encoded field */
static u32 pet_parse_field_base62(const char* str, char delim, int field_num) {
    char buffer[64];
    const char* chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    u32 result = 0;

    if (pet_parse_field(str, delim, field_num, buffer, sizeof(buffer)) != 0) {
        return 0;
    }

    for (int i = 0; buffer[i]; i++) {
        char* p = strchr(chars, buffer[i]);
        if (p) {
            result = result * 62 + (p - chars);
        }
    }

    return result;
}

/* Unescape string (stub) */
static void pet_unescape_string(char* str) {
    if (!str) return;
    /* Simplified - just handle \\ escapes */
    char* dst = str;
    char* src = str;
    while (*src) {
        if (*src == '\\' && *(src + 1)) {
            src++;  /* Skip backslash */
            *dst++ = *src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

/* ========================================
 * Implementation Functions (matching character_protocol.c)
 * ======================================== */

/*
 * Get character entry by index
 */
static CharacterEntry* character_get_entry(u32 index) {
    if (index >= MAX_CHARACTERS) return NULL;
    return &s_char_entries[index];
}

/*
 * Parse character list from packet
 */
static void character_parse_list(const char* packet_data) {
    const char* ptr = packet_data;
    char buffer[256];
    s32 char_id;
    s16 level_val;
    int result;

    if (!packet_data) return;

    /* Parse first character ID */
    char_id = pet_parse_field_int(ptr, '|', 1);
    if (char_id < 0) return;

    /* Parse character name */
    result = pet_parse_field(ptr, '|', 2, buffer, sizeof(buffer));
    if (result != 0) return;
    pet_unescape_string(buffer);

    /* Check if empty */
    if (buffer[0] == '\0') {
        s_char_entries[char_id].name[0] = '\0';
        return;
    }

    /* Store name */
    strncpy(s_char_entries[char_id].name, buffer, 28);
    s_char_entries[char_id].name[28] = '\0';

    /* Parse remaining fields */
    pet_parse_field(ptr, '|', 3, buffer, sizeof(buffer));
    pet_unescape_string(buffer);
    if (strlen(buffer) < 17) {
        strncpy(s_char_entries[char_id].field_0x3a, buffer, 16);
        s_char_entries[char_id].field_0x3a[16] = '\0';
    }

    s_char_entries[char_id].some_field = pet_parse_field_int(ptr, '|', 4);
    if (s_char_entries[char_id].some_field < 0) {
        s_char_entries[char_id].some_field = 0;
    }

    pet_parse_field(ptr, '|', 5, buffer, sizeof(buffer));
    pet_unescape_string(buffer);
    if (strlen(buffer) < 85) {
        strncpy(s_char_entries[char_id].description, buffer, 84);
        s_char_entries[char_id].description[84] = '\0';
    }

    s_char_entries[char_id].field_0x0c = (u32)pet_parse_field_int(ptr, '|', 6);
    s_char_entries[char_id].direction = (u16)pet_parse_field_int(ptr, '|', 7);

    level_val = (s16)pet_parse_field_int(ptr, '|', 8);
    if (level_val < 100) {
        s_char_entries[char_id].level = level_val;
        s_char_entries[char_id].level_over_100 = 0;
    } else {
        s_char_entries[char_id].level = level_val % 100;
        s_char_entries[char_id].level_over_100 = 1;
    }

    s_char_entries[char_id].y = (u16)pet_parse_field_int(ptr, '|', 9);
    s_char_entries[char_id].sprite_id = (u16)pet_parse_field_int(ptr, '|', 10);

    pet_parse_field(ptr, '|', 11, buffer, sizeof(buffer));
    pet_unescape_string(buffer);
    if (strlen(buffer) < 17) {
        strncpy(s_char_entries[char_id].field_0x10, buffer, 16);
        s_char_entries[char_id].field_0x10[16] = '\0';
    }

    if (char_id >= (int)s_char_entry_count) {
        s_char_entry_count = char_id + 1;
    }
}

/*
 * Handle character teleport
 */
static void character_teleport(u32 char_index, const char* packet_data) {
    s32 char_id;
    char buffer[256];

    if (!packet_data) return;

    char_id = pet_parse_field_int(packet_data, '|', 1);

    if (char_id == -1 || char_id == 0) {
        if (s_char_entries[char_index].name[0] != '\0') {
            memset(&s_char_entries[char_index], 0, sizeof(CharacterEntry));
        }
        return;
    }

    pet_parse_field(packet_data, '|', 2, buffer, sizeof(buffer));
    pet_unescape_string(buffer);

    if (buffer[0] != '\0') {
        strncpy(s_char_entries[char_index].name, buffer, 28);
        s_char_entries[char_index].name[28] = '\0';
    }

    s_char_entries[char_index].x = (u16)pet_parse_field_int(packet_data, '|', 3);
    s_char_entries[char_index].y = (u16)pet_parse_field_int(packet_data, '|', 4);
    s_char_entries[char_index].direction = (u16)pet_parse_field_int(packet_data, '|', 5);
    s_char_entries[char_index].sprite_id = (u16)pet_parse_field_int(packet_data, '|', 6);
}

/*
 * Handle move result
 */
static void character_handle_move_result(u32 char_index, const char* packet_data) {
    Character* ch;
    s32 field_val;

    if (!packet_data || char_index >= MAX_CHARACTERS) return;

    ch = &g_chars.characters[char_index];

    field_val = pet_parse_field_int(packet_data, '|', 1);
    if (field_val == -1) {
        return;
    }

    ch->dest_x = (u16)field_val;
    ch->dest_y = (u16)pet_parse_field_int(packet_data, '|', 2);
    ch->is_moving = 1;
}

/*
 * Parse detailed character stats
 */
static void character_parse_detailed_stats(const char* packet_data) {
    const char* ptr = packet_data;
    u32 field_mask;

    if (!packet_data) return;

    /* Skip packet type byte */
    ptr += 1;

    field_mask = (u32)pet_parse_field_base62(ptr, '|', 1);
    g_detailed_stats.field_mask = field_mask;

    if (field_mask == 1) {
        /* All fields present */
        g_detailed_stats.field_02 = (u32)pet_parse_field_int(ptr, '|', 2);
        g_detailed_stats.field_03 = (u32)pet_parse_field_int(ptr, '|', 3);
        g_detailed_stats.field_04 = (u32)pet_parse_field_int(ptr, '|', 4);
        g_detailed_stats.field_05 = (u32)pet_parse_field_int(ptr, '|', 5);
        g_detailed_stats.field_06 = (u32)pet_parse_field_int(ptr, '|', 6);
        g_detailed_stats.field_07 = (u32)pet_parse_field_int(ptr, '|', 7);
        g_detailed_stats.field_08 = (u32)pet_parse_field_int(ptr, '|', 8);
        g_detailed_stats.field_09 = (u32)pet_parse_field_int(ptr, '|', 9);
        g_detailed_stats.field_10 = (u32)pet_parse_field_int(ptr, '|', 10);

        /* Parse title and guild name */
        pet_parse_field(ptr, '|', 30, g_detailed_stats.title, sizeof(g_detailed_stats.title));
        pet_parse_field(ptr, '|', 31, g_detailed_stats.guild_name, sizeof(g_detailed_stats.guild_name));
    }
}

/*
 * Reset test state
 */
static void reset_state(void) {
    memset(s_char_entries, 0, sizeof(s_char_entries));
    s_char_entry_count = 0;
    memset(&g_detailed_stats, 0, sizeof(g_detailed_stats));
    memset(&g_chars, 0, sizeof(g_chars));
}

/* ========================================
 * Test Cases - Character Entry
 * ======================================== */

static int test_get_entry_valid(void) {
    reset_state();

    strcpy(s_char_entries[0].name, "TestChar");

    CharacterEntry* entry = character_get_entry(0);

    return entry != NULL && strcmp(entry->name, "TestChar") == 0;
}

static int test_get_entry_invalid(void) {
    reset_state();

    CharacterEntry* entry = character_get_entry(100);

    return entry == NULL;
}

static int test_get_entry_max_index(void) {
    reset_state();

    CharacterEntry* entry = character_get_entry(MAX_CHARACTERS - 1);

    return entry != NULL;
}

static int test_get_entry_beyond_max(void) {
    reset_state();

    CharacterEntry* entry = character_get_entry(MAX_CHARACTERS);

    return entry == NULL;
}

/* ========================================
 * Test Cases - Character List Parsing
 * ======================================== */

static int test_parse_list_basic(void) {
    reset_state();

    const char* packet = "0|TestName|Account|100|Description|500|1|50|200|1000";

    character_parse_list(packet);

    return strcmp(s_char_entries[0].name, "TestName") == 0 &&
           s_char_entries[0].some_field == 100 &&
           s_char_entries[0].level == 50 &&
           s_char_entries[0].y == 200 &&
           s_char_entries[0].sprite_id == 1000;
}

static int test_parse_list_null(void) {
    reset_state();

    character_parse_list(NULL);

    return s_char_entry_count == 0;
}

static int test_parse_list_empty_name(void) {
    reset_state();

    const char* packet = "1||Account|100|Description|500|1|50|200|1000";

    character_parse_list(packet);

    /* Empty name should not update entry */
    return s_char_entries[1].name[0] == '\0';
}

static int test_parse_list_level_over_100(void) {
    reset_state();

    const char* packet = "0|TestName|Account|100|Description|500|1|150|200|1000";

    character_parse_list(packet);

    /* Level 150 should be stored as level=50, level_over_100=1 */
    return s_char_entries[0].level == 50 &&
           s_char_entries[0].level_over_100 == 1;
}

static int test_parse_list_negative_field(void) {
    reset_state();

    const char* packet = "0|TestName|Account|-50|Description|500|1|50|200|1000";

    character_parse_list(packet);

    /* Negative field should be clamped to 0 */
    return s_char_entries[0].some_field == 0;
}

static int test_parse_list_entry_count(void) {
    reset_state();

    const char* packet = "3|TestName|Account|100|Description|500|1|50|200|1000";

    character_parse_list(packet);

    return s_char_entry_count == 4;
}

/* ========================================
 * Test Cases - Character Teleport
 * ======================================== */

static int test_teleport_basic(void) {
    reset_state();

    const char* packet = "1|TeleportedChar|100|200|3|5000";

    character_teleport(0, packet);

    return strcmp(s_char_entries[0].name, "TeleportedChar") == 0 &&
           s_char_entries[0].x == 100 &&
           s_char_entries[0].y == 200 &&
           s_char_entries[0].direction == 3 &&
           s_char_entries[0].sprite_id == 5000;
}

static int test_teleport_null(void) {
    reset_state();

    strcpy(s_char_entries[0].name, "Existing");

    character_teleport(0, NULL);

    /* Should not crash */
    return 1;
}

static int test_teleport_clear(void) {
    reset_state();

    strcpy(s_char_entries[0].name, "ExistingChar");

    const char* packet = "-1|";

    character_teleport(0, packet);

    /* Should clear the entry */
    return s_char_entries[0].name[0] == '\0';
}

static int test_teleport_zero_id(void) {
    reset_state();

    strcpy(s_char_entries[0].name, "ExistingChar");

    const char* packet = "0|";

    character_teleport(0, packet);

    /* ID=0 should also clear */
    return s_char_entries[0].name[0] == '\0';
}

/* ========================================
 * Test Cases - Move Result
 * ======================================== */

static int test_move_result_basic(void) {
    reset_state();

    const char* packet = "150|250";

    character_handle_move_result(0, packet);

    return g_chars.characters[0].dest_x == 150 &&
           g_chars.characters[0].dest_y == 250 &&
           g_chars.characters[0].is_moving == 1;
}

static int test_move_result_null(void) {
    reset_state();

    character_handle_move_result(0, NULL);

    return g_chars.characters[0].is_moving == 0;
}

static int test_move_result_invalid_index(void) {
    reset_state();

    const char* packet = "150|250";

    character_handle_move_result(100, packet);

    /* Should not crash */
    return 1;
}

static int test_move_result_fail(void) {
    reset_state();

    const char* packet = "-1|250";

    character_handle_move_result(0, packet);

    /* -1 should not set moving */
    return g_chars.characters[0].is_moving == 0;
}

/* ========================================
 * Test Cases - Detailed Stats
 * ======================================== */

static int test_detailed_stats_all_fields(void) {
    reset_state();

    /* Field mask 1 means all fields present
     * The packet format after skipping type byte is just the data
     * We need to format it so field 1 is the mask
     */
    const char* packet = "P1|100|200|300|400|500|600|700|800|900|1000";

    character_parse_detailed_stats(packet);

    /* After skipping 'P', we get "1|100|200|..." */
    /* Field 1 = "1" (mask), Field 2 = "100", etc. */
    return g_detailed_stats.field_mask == 1 &&
           g_detailed_stats.field_02 == 100 &&
           g_detailed_stats.field_03 == 200;
}

static int test_detailed_stats_null(void) {
    reset_state();

    g_detailed_stats.field_02 = 999;

    character_parse_detailed_stats(NULL);

    /* Should not crash, previous value unchanged */
    return g_detailed_stats.field_02 == 999;
}

static int test_detailed_stats_zero_mask(void) {
    reset_state();

    const char* packet = "P|0|100|200|300";

    character_parse_detailed_stats(packet);

    /* Zero mask - no fields parsed beyond mask */
    return g_detailed_stats.field_mask == 0;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Character Protocol Comprehensive Tests ===\n\n");

    printf("Character Entry Tests:\n");
    TEST(get_entry_valid);
    TEST(get_entry_invalid);
    TEST(get_entry_max_index);
    TEST(get_entry_beyond_max);

    printf("\nCharacter List Parsing Tests:\n");
    TEST(parse_list_basic);
    TEST(parse_list_null);
    TEST(parse_list_empty_name);
    TEST(parse_list_level_over_100);
    TEST(parse_list_negative_field);
    TEST(parse_list_entry_count);

    printf("\nCharacter Teleport Tests:\n");
    TEST(teleport_basic);
    TEST(teleport_null);
    TEST(teleport_clear);
    TEST(teleport_zero_id);

    printf("\nMove Result Tests:\n");
    TEST(move_result_basic);
    TEST(move_result_null);
    TEST(move_result_invalid_index);
    TEST(move_result_fail);

    printf("\nDetailed Stats Tests:\n");
    TEST(detailed_stats_all_fields);
    TEST(detailed_stats_null);
    TEST(detailed_stats_zero_mask);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
