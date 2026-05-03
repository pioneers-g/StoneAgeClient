/*
 * Stone Age Client - Unit Tests for Game Data Parsing
 * Test file: test_game_data.c
 *
 * Tests using real game data from:
 * D:\Games\石器时代8.0\石器时代8.0单机版\data\
 *
 * These tests verify reverse engineering analysis against actual data.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/protocol_util.h"

/* External functions from stringutil.c */
extern void string_truncate(const char* src, char* dest, size_t max_len);
extern void string_copy_dbcs(const char* src, char* dest, size_t max_len);
extern size_t string_len_dbcs(const char* str);

#define GAME_DATA_PATH "D:/Games/石器时代8.0/石器时代8.0单机版/data/"

/* ========================================
 * AISetting.dat Format Analysis
 * ======================================== */

/*
 * AISetting.dat structure (reverse engineered):
 * - Bytes 0x00-0x0F: 16-byte XOR encrypted header
 * - Bytes 0x10-0x5F: Preset 0 (80 bytes)
 * - Bytes 0x60-0xAF: Preset 1 (80 bytes)
 * - etc.
 *
 * Each preset structure (80 bytes):
 * - Offset 0x00: int32 encrypted timestamp/checksum (4 bytes)
 * - Offset 0x04: int32 ai_mode (9 = on, 0 = off)
 * - Offset 0x08: int32[4] skill_slots (16 bytes)
 * - Offset 0x18: int32 primary_skill
 * - Offset 0x1C: int32 skill_level (default 30 = 0x1E)
 * - ... additional fields
 */

typedef struct {
    u8 header[16];         /* XOR encrypted header */
    /* Preset data follows, each 80 bytes */
} AISettingHeader;

typedef struct {
    u32 checksum;          /* Encrypted timestamp */
    u32 ai_mode;           /* 9 = AI on, 0 = AI off */
    u32 skills[4];         /* Skill slots (0xFFFFFFFF = empty) */
    u32 primary_skill;     /* Primary skill ID */
    u32 skill_level;       /* Skill level (0-100) */
    u32 attack_mode;       /* Attack mode */
    u32 target_priority;   /* Target selection priority */
    u32 reserved[10];      /* Reserved fields */
} AISettingPreset;         /* Total: 80 bytes */

/*
 * Test 1: AISetting.dat file exists and size
 */
static void test_ai_settings_file_exists(void) {
    TEST_BEGIN("AISetting.dat exists");

    char path[512];
    snprintf(path, sizeof(path), "%sAISetting.dat", GAME_DATA_PATH);

    FILE* fp = fopen(path, "rb");
    TEST_ASSERT(fp != NULL, "File should exist");

    if (fp) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fclose(fp);

        /* File should be 1955 bytes based on ls output */
        TEST_ASSERT(size > 0, "File should not be empty");
        TEST_ASSERT(size == 1955, "File size should be 1955 bytes");

        /* Verify size matches expected preset count */
        /* 16 header + (N * 80) presets = 1955 */
        /* 1955 - 16 = 1939, not divisible by 80 */
        /* Actual: 1955 = 16 + 24*80 + 19 (extra data?) */
    }

    TEST_END();
}

/*
 * Test 2: AISetting.dat header format
 */
static void test_ai_settings_header(void) {
    TEST_BEGIN("AISetting.dat header");

    char path[512];
    snprintf(path, sizeof(path), "%sAISetting.dat", GAME_DATA_PATH);

    FILE* fp = fopen(path, "rb");
    if (!fp) {
        TEST_ASSERT(0, "Could not open file");
        TEST_END();
        return;
    }

    u8 header[16];
    size_t read = fread(header, 1, 16, fp);
    fclose(fp);

    TEST_ASSERT(read == 16, "Should read 16 bytes header");

    /* Header appears XOR encrypted */
    /* Known pattern: bytes 0-3 are 0x2b, 0x1c, 0x57, 0x57 */
    TEST_ASSERT(header[0] == 0x2b, "First byte should be 0x2b");
    TEST_ASSERT(header[1] == 0x1c, "Second byte should be 0x1c");

    /* TODO: Determine XOR key from binary analysis */
    /* Key appears to change per session (timestamp-based?) */

    TEST_END();
}

/*
 * Test 3: AISetting.dat preset parsing
 */
static void test_ai_settings_preset(void) {
    TEST_BEGIN("AISetting.dat preset");

    char path[512];
    snprintf(path, sizeof(path), "%sAISetting.dat", GAME_DATA_PATH);

    FILE* fp = fopen(path, "rb");
    if (!fp) {
        TEST_ASSERT(0, "Could not open file");
        TEST_END();
        return;
    }

    /* Skip header */
    fseek(fp, 16, SEEK_SET);

    AISettingPreset preset;
    size_t read = fread(&preset, 1, sizeof(preset), fp);
    fclose(fp);

    TEST_ASSERT(read == sizeof(preset), "Should read preset structure");

    /* Verify known values from hex dump:
     * Offset 0x10: 0x00000000 (checksum)
     * Offset 0x14: 0x09000000 (LE: 9, AI mode on)
     * Offset 0x18-0x24: 0xFFFFFFFF (empty skill slots)
     * Offset 0x28: 0x09000000 (LE: 9, primary skill)
     * Offset 0x2C: 0x1A000000 (LE: 26, but should be 30?)
     */

    /* Note: Values are little-endian */
    TEST_ASSERT(preset.ai_mode == 9, "AI mode should be 9 (on)");
    TEST_ASSERT(preset.checksum == 0, "First preset checksum should be 0");

    /* Skill slots 0-3 should be 0xFFFFFFFF (empty) */
    TEST_ASSERT(preset.skills[0] == 0xFFFFFFFF, "Skill slot 0 should be empty");
    TEST_ASSERT(preset.skills[1] == 0xFFFFFFFF, "Skill slot 1 should be empty");
    TEST_ASSERT(preset.skills[2] == 0xFFFFFFFF, "Skill slot 2 should be empty");
    TEST_ASSERT(preset.skills[3] == 0xFFFFFFFF, "Skill slot 3 should be empty");

    TEST_END();
}

/*
 * Test 4: AISetting.dat AI mode values
 */
static void test_ai_settings_mode(void) {
    TEST_BEGIN("AISetting.dat AI modes");

    char path[512];
    snprintf(path, sizeof(path), "%sAISetting.dat", GAME_DATA_PATH);

    FILE* fp = fopen(path, "rb");
    if (!fp) {
        TEST_ASSERT(0, "Could not open file");
        TEST_END();
        return;
    }

    /* Read multiple presets */
    fseek(fp, 16, SEEK_SET);  /* Skip header */

    int mode_count[16] = {0};  /* Count of each mode value */
    AISettingPreset preset;

    for (int i = 0; i < 24; i++) {  /* 24 presets expected */
        if (fread(&preset, sizeof(preset), 1, fp) != 1) break;

        /* AI mode should be 0 or 9 based on binary analysis */
        /* FUN_00401ab0 validates: mode must be 0, 3, or 9 */
        if (preset.ai_mode < 16) {
            mode_count[preset.ai_mode]++;
        }
    }
    fclose(fp);

    /* Most presets should have mode 9 (AI on) */
    TEST_ASSERT(mode_count[9] > 0, "Should have presets with mode 9");

    /* TODO: Verify mode values against FUN_00401ab0 validation */
    /* Binary shows mode can be 0 (off), 3 (semi-auto?), or 9 (on) */

    TEST_END();
}

/* ========================================
 * Album/Pet Data Format Analysis
 * ======================================== */

/*
 * Test 5: Album file exists
 */
static void test_album_file_exists(void) {
    TEST_BEGIN("Album file exists");

    char path[512];
    snprintf(path, sizeof(path), "%salbum_37.dat", GAME_DATA_PATH);

    FILE* fp = fopen(path, "rb");
    TEST_ASSERT(fp != NULL, "File should exist");

    if (fp) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fclose(fp);

        /* File should be 909328 bytes */
        TEST_ASSERT(size == 909328, "album_37.dat should be 909328 bytes");
    }

    TEST_END();
}

/* ========================================
 * Protocol Format Tests
 * ======================================== */

/*
 * Test 6: Protocol field parsing with pipe delimiter
 */
static void test_protocol_field_parsing(void) {
    TEST_BEGIN("Protocol field parsing");

    /* Real protocol format: "B|0|item_name|price|..." */
    const char* test_data = "B|0|回生药|1000|回复HP100";

    char field[256];

    /* Field 1: Command "B" */
    int result = parse_text_field(test_data, '|', 1, 255, field);
    TEST_ASSERT(result == 0, "Should find field 1");
    TEST_ASSERT_STR_EQ("B", field, "Field 1 should be B");

    /* Field 2: Index "0" */
    result = parse_text_field(test_data, '|', 2, 255, field);
    TEST_ASSERT(result == 0, "Should find field 2");
    TEST_ASSERT_STR_EQ("0", field, "Field 2 should be 0");

    /* Field 4: Price "1000" */
    result = parse_text_field(test_data, '|', 4, 255, field);
    TEST_ASSERT(result == 0, "Should find field 4");
    TEST_ASSERT_STR_EQ("1000", field, "Field 4 should be 1000");

    TEST_END();
}

/*
 * Test 7: Base-62 encoding verification
 * Used for bitmask values in protocol
 */
static void test_base62_encoding(void) {
    TEST_BEGIN("Base-62 encoding");

    /* Base-62 character set: 0-9, a-z, A-Z */
    /* Values: 0-9 = 0-9, a-z = 10-35, A-Z = 36-61 */

    /* Test single characters */
    TEST_ASSERT_EQ(0, parse_base62("0"), "0 should be 0");
    TEST_ASSERT_EQ(9, parse_base62("9"), "9 should be 9");
    TEST_ASSERT_EQ(10, parse_base62("a"), "a should be 10");
    TEST_ASSERT_EQ(35, parse_base62("z"), "z should be 35");
    TEST_ASSERT_EQ(36, parse_base62("A"), "A should be 36");
    TEST_ASSERT_EQ(61, parse_base62("Z"), "Z should be 61");

    /* Test multi-character values */
    /* "10" = 1*62 + 0 = 62 */
    TEST_ASSERT_EQ(62, parse_base62("10"), "10 in base62 should be 62");

    /* "1a" = 1*62 + 10 = 72 */
    TEST_ASSERT_EQ(72, parse_base62("1a"), "1a in base62 should be 72");

    /* "ZZ" = 61*62 + 61 = 3843 */
    TEST_ASSERT_EQ(3843, parse_base62("ZZ"), "ZZ in base62 should be 3843");

    TEST_END();
}

/* ========================================
 * String Utility Tests
 * ======================================== */

/*
 * Test 8: String truncation with ellipsis
 * FUN_0044a940 behavior
 */
static void test_string_truncation(void) {
    TEST_BEGIN("String truncation");

    char output[32];

    /* String that fits */
    string_truncate("Hello", output, 10);
    TEST_ASSERT_STR_EQ("Hello", output, "Should not truncate short string");

    /* String that needs truncation */
    string_truncate("HelloWorld1234567890", output, 10);
    TEST_ASSERT(strlen(output) <= 10, "Output should fit in max_len");

    /* Very long string */
    string_truncate("This is a very long string that needs truncation", output, 15);
    TEST_ASSERT(strlen(output) <= 15, "Should truncate to fit");

    TEST_END();
}

/*
 * Test 9: DBCS string handling
 */
static void test_dbcs_handling(void) {
    TEST_BEGIN("DBCS handling");

    /* Chinese characters in GBK encoding */
    /* "中文" = 4 bytes in GBK (2 chars * 2 bytes each) */

    char output[32];
    const char* chinese = "中文测试";

    /* Copy with DBCS support */
    string_copy_dbcs(chinese, output, 32);
    TEST_ASSERT(strlen(output) > 0, "Should copy DBCS string");

    /* Length should count characters, not bytes */
    size_t char_len = string_len_dbcs(chinese);
    TEST_ASSERT(char_len == 4, "Should count 4 characters");

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(game_data) {
    /* AISetting.dat tests */
    test_ai_settings_file_exists();
    test_ai_settings_header();
    test_ai_settings_preset();
    test_ai_settings_mode();

    /* Album tests */
    test_album_file_exists();

    /* Protocol tests */
    test_protocol_field_parsing();
    test_base62_encoding();

    /* String utility tests */
    test_string_truncation();
    test_dbcs_handling();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - Game Data Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(game_data);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
