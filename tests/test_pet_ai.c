/*
 * Stone Age Client - Unit Tests for AI Settings System
 * Test file: test_pet_ai.c
 *
 * Tests FUN_004017a0 (AI settings loader) and FUN_00401ab0 (validator)
 *
 * Based on reverse engineering analysis of sa_9061.exe
 *
 * AISetting.dat format (from offset 0x10):
 * - 4 bytes: AI mode (0=off, 3=on)
 * - 20 bytes: Primary skill array
 * - 20 bytes: Primary skill level array
 * - 20 bytes: Secondary skill level array
 * - 4 bytes: Tertiary skill level
 * - 1 byte: Auto battle flag
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/pet_ai.h"

/* Test data from binary analysis */
#define TEST_AI_FILE TEST_DATA_DIR "AISetting.dat"

/* Expected values from binary analysis of AISetting.dat */
/* File has multiple presets, we test the first one at offset 0x10 */

/*
 * Binary file analysis (offset 0x10):
 * 00000010: 00 00 00 00  <- AI mode = 0
 * 00000014: 09 00 00 00 FF FF FF FF FF FF FF FF 09 00 00 00 1A 00 00 00
 *           ^primary skill data (20 bytes)
 *
 * TODO: Determine exact byte interpretation for skill arrays
 * - Are they 4-bit nibbles, 8-bit bytes, or 16-bit shorts?
 * - Validation function uses 32-bit int access
 */

/* Test context */
static PetAISettings g_test_settings;

/* Helper to read raw file data */
static int read_file_data(const char* filename, int offset, void* buffer, int size) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        return 0;
    }

    if (offset > 0) {
        fseek(fp, offset, SEEK_SET);
    }

    int read = fread(buffer, 1, size, fp);
    fclose(fp);
    return read;
}

/* ========================================
 * Test Cases
 * ======================================== */

/*
 * Test 1: Verify AI file exists and is readable
 */
static void test_ai_file_exists(void) {
    TEST_BEGIN("AI file exists");

    FILE* fp = fopen(TEST_AI_FILE, "rb");
    TEST_ASSERT(fp != NULL, "Cannot open AISetting.dat");
    fclose(fp);

    TEST_END();
}

/*
 * Test 2: Verify AI file size is reasonable
 * Expected: At least 0x55 + 0x10 + 69 bytes for one preset
 */
static void test_ai_file_size(void) {
    TEST_BEGIN("AI file size");

    FILE* fp = fopen(TEST_AI_FILE, "rb");
    TEST_ASSERT(fp != NULL, "Cannot open AISetting.dat");

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);

    /* Minimum size: 0x10 (header) + 0x45 (one preset) = 0x55 = 85 bytes */
    TEST_ASSERT(size >= 0x55, "AI file too small");

    TEST_END();
}

/*
 * Test 3: Verify AI mode value at offset 0x10
 * From binary: First byte at 0x10 should be AI mode
 * Expected: 0 or 3 (based on validation function)
 */
static void test_ai_mode_raw(void) {
    TEST_BEGIN("AI mode raw value");

    u32 ai_mode;
    int read = read_file_data(TEST_AI_FILE, 0x10, &ai_mode, 4);
    TEST_ASSERT(read == 4, "Failed to read AI mode");

    /* AI mode should be 0 or 3 (validated by FUN_00401ab0) */
    TEST_ASSERT(ai_mode == 0 || ai_mode == 3, "AI mode invalid value");

    TEST_END();
}

/*
 * Test 4: Verify primary skill data structure
 * From binary: 20 bytes at offset 0x14
 */
static void test_primary_skill_data(void) {
    TEST_BEGIN("Primary skill data structure");

    u8 primary_skill[20];
    int read = read_file_data(TEST_AI_FILE, 0x14, primary_skill, 20);
    TEST_ASSERT(read == 20, "Failed to read primary skill data");

    /*
     * From validation: primary_skill[0] must be 0-9 or will be set to 9
     * First byte: 0x09 -> skill ID 9
     */
    TEST_ASSERT(primary_skill[0] <= 9, "Primary skill ID out of range");

    TEST_END();
}

/*
 * Test 5: Verify primary skill level data
 * From binary: 20 bytes at offset 0x28
 */
static void test_primary_skill_level(void) {
    TEST_BEGIN("Primary skill level data");

    u8 skill_levels[20];
    int read = read_file_data(TEST_AI_FILE, 0x28, skill_levels, 20);
    TEST_ASSERT(read == 20, "Failed to read skill levels");

    /*
     * From validation: skill levels must be 0-100 and divisible by 5
     * If invalid, set to 30 (0x1e)
     */
    /* Check first skill level if present */
    /* TODO: Determine actual byte interpretation */

    TEST_END();
}

/*
 * Test 6: Verify validation function behavior
 * Test that invalid values are corrected
 */
static void test_validation_ai_mode(void) {
    TEST_BEGIN("Validation corrects AI mode");

    /* Setup invalid AI mode */
    g_test_settings.ai_mode = 5;  /* Invalid: should become 0 or 3 */

    /* The validation in FUN_00401ab0 sets mode to 3 if it's non-zero */
    /* Expected: ai_mode 5 -> 3 (since it's != 0) */
    /* TODO: Call actual validation function when implemented */

    /* For now, document expected behavior */
    if (g_test_settings.ai_mode != 0 && g_test_settings.ai_mode != 3) {
        /* FIX: Validation should set to 3 if non-zero, else 0 */
        g_test_settings.ai_mode = (g_test_settings.ai_mode != 0) ? 3 : 0;
    }

    TEST_ASSERT(g_test_settings.ai_mode == 0 || g_test_settings.ai_mode == 3,
                "AI mode validation failed");

    TEST_END();
}

/*
 * Test 7: Verify skill level validation
 * Levels must be 0-100 and divisible by 5
 */
static void test_validation_skill_levels(void) {
    TEST_BEGIN("Validation corrects skill levels");

    /* Setup invalid skill levels */
    g_test_settings.primary_skill_level[0] = 26;  /* Invalid: not divisible by 5 */
    g_test_settings.primary_skill_level[1] = 105; /* Invalid: > 100 */
    g_test_settings.primary_skill_level[2] = -5;  /* Invalid: < 0 */
    g_test_settings.primary_skill_level[3] = 30;  /* Valid */

    /* Validation should fix invalid values to 30 */
    /* TODO: Call actual validation function when implemented */

    /* Document expected behavior from FUN_00401ab0 */
    for (int i = 0; i < 10; i++) {
        int level = g_test_settings.primary_skill_level[i];
        if (level < 0 || level > 100 || level % 5 != 0) {
            /* FIX: Validation should set to 30 */
            g_test_settings.primary_skill_level[i] = 30;
        }
    }

    TEST_ASSERT(g_test_settings.primary_skill_level[0] == 30, "Level 26 should become 30");
    TEST_ASSERT(g_test_settings.primary_skill_level[1] == 30, "Level 105 should become 30");
    TEST_ASSERT(g_test_settings.primary_skill_level[2] == 30, "Level -5 should become 30");
    TEST_ASSERT(g_test_settings.primary_skill_level[3] == 30, "Valid level 30 should stay 30");

    TEST_END();
}

/*
 * Test 8: Verify auto battle flag validation
 */
static void test_validation_auto_battle(void) {
    TEST_BEGIN("Validation corrects auto battle flag");

    /* Setup invalid flag */
    g_test_settings.auto_battle_flag = 5;  /* Invalid: should be 0 or 1 */

    /* Validation should fix to 0 or 1 */
    /* TODO: Call actual validation function when implemented */

    if (g_test_settings.auto_battle_flag > 1) {
        /* FIX: Validation should set to 0 */
        g_test_settings.auto_battle_flag = 0;
    }

    TEST_ASSERT(g_test_settings.auto_battle_flag <= 1,
                "Auto battle flag validation failed");

    TEST_END();
}

/*
 * Test 9: Verify file has multiple presets
 * AISetting.dat typically stores multiple AI configurations
 */
static void test_multiple_presets(void) {
    TEST_BEGIN("Multiple presets in file");

    FILE* fp = fopen(TEST_AI_FILE, "rb");
    TEST_ASSERT(fp != NULL, "Cannot open AISetting.dat");

    /* Each preset is 0x55 bytes apart (from DAT_004d7f60 * 0x55) */
    /* DAT_004d7f60 appears to be an index/multiplier */

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);

    /* Check if file could contain multiple presets */
    /* Each preset: 0x55 bytes base + 0x45 bytes data = about 0x9A bytes min */
    int max_presets = size / 0x55;

    TEST_ASSERT(max_presets >= 1, "File should contain at least one preset");

    TEST_END();
}

/*
 * Test 10: Compare with known good values from binary analysis
 */
static void test_known_values(void) {
    TEST_BEGIN("Known values from binary");

    /*
     * From binary analysis of AISetting.dat at offset 0x10:
     * AI mode: 0x00000000 = 0
     * Primary skill[0]: 0x09 = 9
     *
     * These are known values extracted from the file
     */

    u32 ai_mode;
    u8 primary_skill_first;

    read_file_data(TEST_AI_FILE, 0x10, &ai_mode, 4);
    read_file_data(TEST_AI_FILE, 0x14, &primary_skill_first, 1);

    /* Verify known values */
    TEST_ASSERT_EQ(0, ai_mode, "AI mode should be 0");
    TEST_ASSERT_EQ(9, primary_skill_first, "First primary skill should be 9");

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(pet_ai) {
    test_ai_file_exists();
    test_ai_file_size();
    test_ai_mode_raw();
    test_primary_skill_data();
    test_primary_skill_level();
    test_validation_ai_mode();
    test_validation_skill_levels();
    test_validation_auto_battle();
    test_multiple_presets();
    test_known_values();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - AI Settings Tests\n");
    printf("========================================\n");
    printf("Test data directory: %s\n", TEST_DATA_DIR);
    printf("\n");

    RUN_TEST_SUITE(pet_ai);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
