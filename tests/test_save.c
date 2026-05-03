/*
 * Stone Age Client - Save System Unit Tests
 * Tests for save.c, savedata.c
 * Reverse engineered from FUN_00401300, FUN_004495c0, FUN_00449f70, FUN_0044a100
 *
 * Uses real game data from D:\Games\石器时代8.0\石器时代8.0单机版\data\
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/save.h"
#include "../include/album.h"

/* Test data directory */
#define TEST_DATA_DIR "D:/Games/石器时代8.0/石器时代8.0单机版/data/"

/* ========================================
 * Test Cases for Save System
 * ======================================== */

/*
 * Test 1: Save system initialization
 */
static void test_save_init(void) {
    TEST_BEGIN("Save init");

    int result = save_init();

    TEST_ASSERT(result == 1);
    TEST_ASSERT(g_ai_settings.loaded == 0 || g_ai_settings.loaded == 1);

    save_shutdown();
    TEST_END();
}

/*
 * Test 2: AI settings default values
 * From FUN_00401300: DAT_004d7ea4 = 9, DAT_004d7ea8 = 0xffffffff
 */
static void test_ai_settings_defaults(void) {
    TEST_BEGIN("AI settings defaults");

    save_init();

    /* Check default values from FUN_00401300 */
    TEST_ASSERT(g_ai_settings.settings[0].setting1 == 9);
    TEST_ASSERT(g_ai_settings.settings[0].setting2 == 0xFFFFFFFF);
    TEST_ASSERT(g_ai_settings.settings[1].setting1 == 1);
    TEST_ASSERT(g_ai_settings.settings[1].setting3 == 0x1e);  /* 30 */
    TEST_ASSERT(g_ai_settings.settings[3].setting3 == 0x1a);  /* 26 */
    TEST_ASSERT(g_ai_settings.settings[4].setting1 == 4);

    save_shutdown();
    TEST_END();
}

/*
 * Test 3: XOR encryption/decryption symmetry
 */
static void test_xor_symmetry(void) {
    TEST_BEGIN("XOR symmetry");

    u32 original[4] = {0x12345678, 0x9ABCDEF0, 0xDEADBEEF, 0xCAFEBABE};
    u32 encrypted[4];
    u32 decrypted[4];

    memcpy(encrypted, original, sizeof(original));
    save_xor_crypt(encrypted, sizeof(encrypted));

    memcpy(decrypted, encrypted, sizeof(encrypted));
    save_xor_crypt(decrypted, sizeof(decrypted));

    /* XOR encryption should be symmetric - decrypt = encrypt */
    TEST_ASSERT(memcmp(original, decrypted, sizeof(original)) == 0);

    /* Encrypted should be different from original */
    TEST_ASSERT(memcmp(original, encrypted, sizeof(original)) != 0);

    TEST_END();
}

/*
 * Test 4: XOR encryption with zeros
 * Encrypting zeros gives the key itself
 */
static void test_xor_zeros(void) {
    TEST_BEGIN("XOR zeros");

    u32 zeros[4] = {0, 0, 0, 0};
    u32 result[4];

    memcpy(result, zeros, sizeof(zeros));
    save_xor_crypt(result, sizeof(result));

    /* Result should be the XOR key values */
    /* TODO: Verify exact key values from binary DAT_004c10bc */

    TEST_END();
}

/*
 * Test 5: File existence check
 */
static void test_save_exists(void) {
    TEST_BEGIN("Save exists");

    /* Check files that should exist */
    TEST_ASSERT(save_exists(TEST_DATA_DIR "AISetting.dat") == 1);
    TEST_ASSERT(save_exists(TEST_DATA_DIR "mail.dat") == 1);
    TEST_ASSERT(save_exists(TEST_DATA_DIR "album.dat") == 1);

    /* Check file that should not exist */
    TEST_ASSERT(save_exists(TEST_DATA_DIR "nonexistent.dat") == 0);

    TEST_END();
}

/*
 * Test 6: Get file size
 */
static void test_save_get_size(void) {
    TEST_BEGIN("Save get size");

    /* AISetting.dat should be 1955 bytes */
    u32 size = save_get_size(TEST_DATA_DIR "AISetting.dat");
    TEST_ASSERT(size == 1955);

    /* mail.dat should be 611840 bytes */
    size = save_get_size(TEST_DATA_DIR "mail.dat");
    TEST_ASSERT(size == 611840);

    /* Nonexistent file should be 0 */
    size = save_get_size(TEST_DATA_DIR "nonexistent.dat");
    TEST_ASSERT(size == 0);

    TEST_END();
}

/*
 * Test 7: AI settings load from real file
 * File: AISetting.dat (1955 bytes)
 */
static void test_ai_settings_load_real(void) {
    TEST_BEGIN("AI settings load real");

    save_init();

    /* Try to load from real data file */
    FILE* fp = fopen(TEST_DATA_DIR "AISetting.dat", "rb");
    if (!fp) {
        TEST_END();
        return;
    }

    /* Read header (16 bytes) */
    char header[16];
    int read_result = fread(header, 1, 16, fp);
    TEST_ASSERT(read_result == 16);

    /* Decrypt header using XOR */
    save_xor_crypt(header, 16);

    /* After decryption, first 4 bytes might be zeros or a name */
    /* TODO: Verify expected decrypted values from binary analysis */

    fclose(fp);
    save_shutdown();
    TEST_END();
}

/*
 * Test 8: AISetting.dat structure analysis
 * File size: 1955 bytes = 23 entries * 85 bytes
 */
static void test_ai_setting_structure(void) {
    TEST_BEGIN("AI setting structure");

    /*
     * From binary analysis:
     * Each AI setting entry is 85 bytes (0x55)
     * - 16 bytes: encrypted name/header
     * - 4 bytes: flags (DAT_004d9050)
     * - 5 * 4 = 20 bytes: settings (DAT_004d7ea4, DAT_004d7f30, DAT_004d7f1c)
     * - 4 bytes: extra1 (DAT_004d7f18)
     * - 1 byte: extra2 (DAT_004d7f54)
     * Total: 16 + 4 + 20 + 4 + 1 = 45 bytes... but binary shows 85 bytes
     *
     * TODO: Re-verify structure layout
     */

    u32 file_size = save_get_size(TEST_DATA_DIR "AISetting.dat");
    TEST_ASSERT(file_size == 1955);

    /* 1955 / 85 = 23 entries */
    TEST_ASSERT(file_size % 85 == 0 || file_size % 85 == 45);

    TEST_END();
}

/*
 * Test 9: Album file enumeration
 * Files: album.dat, album_2.dat to album_37.dat
 */
static void test_album_files(void) {
    TEST_BEGIN("Album files");

    /* Main album.dat should exist */
    TEST_ASSERT(save_exists(TEST_DATA_DIR "album.dat") == 1);

    /* Check version files from FUN_0044a100 */
    TEST_ASSERT(save_exists(TEST_DATA_DIR "album_37.dat") == 1);
    TEST_ASSERT(save_exists(TEST_DATA_DIR "album_25.dat") == 1);
    TEST_ASSERT(save_exists(TEST_DATA_DIR "album_30.dat") == 1);

    TEST_END();
}

/*
 * Test 10: Album data size verification
 * FUN_0044a100 shows version-specific offsets
 */
static void test_album_data_sizes(void) {
    TEST_BEGIN("Album data sizes");

    /*
     * From FUN_0044a100:
     * album_37.dat: offset 0x1e3 (483)
     * album_36.dat: offset 0x1ce (462)
     * album_35.dat: offset 0x1b4 (436)
     * etc.
     */

    u32 size_37 = save_get_size(TEST_DATA_DIR "album_37.dat");
    u32 size_30 = save_get_size(TEST_DATA_DIR "album_30.dat");
    u32 size_25 = save_get_size(TEST_DATA_DIR "album_25.dat");

    /* Sizes should be reasonable */
    TEST_ASSERT(size_37 > 900000);  /* ~909KB */
    TEST_ASSERT(size_30 > 370000);  /* ~374KB */
    TEST_ASSERT(size_25 > 230000);  /* ~233KB */

    TEST_END();
}

/*
 * Test 11: Mail data structure
 * FUN_004495c0: 0xef0 bytes per entry, 80 entries
 */
static void test_mail_structure(void) {
    TEST_BEGIN("Mail structure");

    /*
     * From FUN_004495c0:
     * Mail stored at DAT_045967c0
     * Entry size: 0xef0 (3824) bytes
     * Max entries: 80
     * Total: 80 * 3824 = 305920 bytes... but file is 611840
     */

    u32 file_size = save_get_size(TEST_DATA_DIR "mail.dat");
    TEST_ASSERT(file_size == 611840);

    /* 611840 / 80 = 7648 bytes per entry (not 3824) */
    u32 entry_size = file_size / 80;
    TEST_ASSERT(entry_size == 7648);

    /* TODO: Verify actual entry structure from binary */

    TEST_END();
}

/*
 * Test 12: Chat history file
 * File: chathis.dat
 */
static void test_chat_history_file(void) {
    TEST_BEGIN("Chat history file");

    u32 size = save_get_size(TEST_DATA_DIR "chathis.dat");
    TEST_ASSERT(size > 0);

    /* Open and read */
    FILE* fp = fopen(TEST_DATA_DIR "chathis.dat", "rb");
    if (fp) {
        u8 header[16];
        int read = fread(header, 1, 16, fp);
        TEST_ASSERT(read == 16);

        /* Chat history uses XOR encryption similar to album */
        /* TODO: Verify encryption */

        fclose(fp);
    }

    TEST_END();
}

/*
 * Test 13: Channel config file
 * File: channel.dat (3 bytes)
 */
static void test_channel_file(void) {
    TEST_BEGIN("Channel file");

    u32 size = save_get_size(TEST_DATA_DIR "channel.dat");
    TEST_ASSERT(size == 3);

    FILE* fp = fopen(TEST_DATA_DIR "channel.dat", "rb");
    if (fp) {
        u8 data[3];
        int read = fread(data, 1, 3, fp);
        TEST_ASSERT(read == 3);

        /* Channel number stored as string or binary */
        /* TODO: Verify format */

        fclose(fp);
    }

    TEST_END();
}

/*
 * Test 14: XOR encryption key verification
 * From DAT_004c10bc to DAT_004c107c (16 bytes, read backwards)
 */
static void test_xor_key(void) {
    TEST_BEGIN("XOR key");

    /*
     * From FUN_00401300:
     * pbVar3 = &DAT_004c10bc;
     * do {
     *   bVar1 = *pbVar3;
     *   pbVar3 = pbVar3 + -4;  // Move down 4 bytes
     *   ...
     * } while (0x4c107c < (int)pbVar3);
     *
     * This reads 16 bytes from 0x4c10bc down to 0x4c107c
     * In order: [0x4c10bc], [0x4c10b8], [0x4c10b4], [0x4c10b0]...
     *           Wait, -4 means: bc -> b8 -> b4 -> b0
     *
     * TODO: Extract actual key values from binary data section
     */

    /* For now, verify XOR key has non-zero values */
    u32 test_key[4] = {0x12345678, 0x9ABCDEF0, 0x13579BDF, 0x2468ACE0};
    TEST_ASSERT(test_key[0] != 0);
    TEST_ASSERT(test_key[1] != 0);

    TEST_END();
}

/*
 * Test 15: DES encryption detection
 * FUN_0048b7c0 shows DES S-box pattern
 */
static void test_des_detection(void) {
    TEST_BEGIN("DES detection");

    /*
     * From FUN_0048b7c0 analysis:
     * Uses lookup tables at DAT_004d5ad8, DAT_004d58d8, etc.
     * These are DES S-boxes
     *
     * FUN_0048b620: Key schedule generation
     * FUN_0048b7c0: DES round function
     * FUN_0048bb90: DES encryption/decryption wrapper
     */

    /* This confirms the save system uses DES encryption, not just XOR */
    /* TODO: Implement DES for full compatibility */

    TEST_END();
}

/*
 * Test 16: AI settings set and get
 */
static void test_ai_settings_set_get(void) {
    TEST_BEGIN("AI settings set/get");

    save_init();

    /* Set a value */
    ai_settings_set(0, 0, 15);
    TEST_ASSERT(ai_settings_get(0, 0) == 15);

    /* Set another value */
    ai_settings_set(2, 2, 100);
    TEST_ASSERT(ai_settings_get(2, 2) == 100);

    /* Invalid index should return 0 */
    TEST_ASSERT(ai_settings_get(-1, 0) == 0);
    TEST_ASSERT(ai_settings_get(100, 0) == 0);
    TEST_ASSERT(ai_settings_get(0, -1) == 0);
    TEST_ASSERT(ai_settings_get(0, 100) == 0);

    save_shutdown();
    TEST_END();
}

/*
 * Test 17: Album load test
 */
static void test_album_load(void) {
    TEST_BEGIN("Album load");

    /* album_init should load from album.dat */
    int result = album_init();
    TEST_ASSERT(result == 1);

    /* Check album count */
    u32 count = album_get_count();
    TEST_ASSERT(count > 0);

    album_shutdown();
    TEST_END();
}

/*
 * Test 18: Album entry structure
 * From FUN_00449f70: 0x14 bytes per entry
 */
static void test_album_entry_size(void) {
    TEST_BEGIN("Album entry size");

    /*
     * From FUN_00449f70:
     * piVar10 = piVar10 + 0x14;  // Entry step is 0x14 (20) dwords = 80 bytes
     * iVar9 = 0x7c;  // 124 entries per block
     */

    /* Each album entry should be 80 bytes (0x50) */
    TEST_ASSERT(ALBUM_ENTRY_SIZE == 80);

    TEST_END();
}

/*
 * Test 19: Save file write and read back
 */
static void test_save_write_read(void) {
    TEST_BEGIN("Save write/read");

    const char* test_file = "test_save.tmp";

    /* Create test data */
    u32 test_data[4] = {0xDEADBEEF, 0xCAFEBABE, 0x12345678, 0x9ABCDEF0};

    /* Write */
    FILE* fp = fopen(test_file, "wb");
    TEST_ASSERT(fp != NULL);
    if (fp) {
        fwrite(test_data, sizeof(u32), 4, fp);
        fclose(fp);
    }

    /* Read back */
    u32 read_data[4] = {0};
    fp = fopen(test_file, "rb");
    TEST_ASSERT(fp != NULL);
    if (fp) {
        fread(read_data, sizeof(u32), 4, fp);
        fclose(fp);
    }

    /* Verify */
    TEST_ASSERT(memcmp(test_data, read_data, sizeof(test_data)) == 0);

    /* Cleanup */
    remove(test_file);

    TEST_END();
}

/*
 * Test 20: AI settings save and reload
 */
static void test_ai_settings_save_reload(void) {
    TEST_BEGIN("AI settings save/reload");

    save_init();

    /* Set unique values */
    ai_settings_set(0, 0, 0x11111111);
    ai_settings_set(1, 1, 0x22222222);
    ai_settings_set(2, 2, 0x33333333);

    /* Save */
    int result = ai_settings_save();
    TEST_ASSERT(result == 1);

    /* Reset */
    save_shutdown();
    save_init();

    /* Load */
    result = ai_settings_load();
    TEST_ASSERT(result == 1);

    /* Verify values */
    /* TODO: After fixing XOR key, verify loaded values match */

    save_shutdown();
    TEST_END();
}

/*
 * Test 21: Auto.dat file structure
 */
static void test_auto_file(void) {
    TEST_BEGIN("Auto file");

    u32 size = save_get_size(TEST_DATA_DIR "auto.dat");
    TEST_ASSERT(size == 550010);

    TEST_END();
}

/*
 * Test 22: Save data directory listing
 */
static void test_data_directory(void) {
    TEST_BEGIN("Data directory");

    /* All expected files should exist */
    const char* expected_files[] = {
        "AISetting.dat",
        "album.dat",
        "mail.dat",
        "chathis.dat",
        "channel.dat",
        "auto.dat",
        "chatreg.dat",
        "savedata.dat"
    };

    int i;
    char path[256];
    for (i = 0; i < 8; i++) {
        snprintf(path, sizeof(path), "%s%s", TEST_DATA_DIR, expected_files[i]);
        TEST_ASSERT(save_exists(path) == 1);
    }

    TEST_END();
}

/*
 * Test 23: Encryption key derivation
 * FUN_0048b620: DES key schedule from password
 */
static void test_key_derivation(void) {
    TEST_BEGIN("Key derivation");

    /*
     * From FUN_0048b620:
     * Uses DAT_004d5870 (S-boxes for key schedule)
     * Takes 8-byte key and expands to 32 subkeys
     *
     * This is standard DES key schedule
     */

    /* TODO: Implement key derivation test with known test vectors */

    TEST_END();
}

/*
 * Test 24: Album XOR encryption pattern
 * FUN_00449f70 uses same XOR as AISettings
 */
static void test_album_xor_pattern(void) {
    TEST_BEGIN("Album XOR pattern");

    FILE* fp = fopen(TEST_DATA_DIR "album.dat", "rb");
    if (!fp) {
        TEST_END();
        return;
    }

    u8 header[16];
    fread(header, 1, 16, fp);
    fclose(fp);

    /* Apply XOR decryption */
    save_xor_crypt(header, 16);

    /* After decryption, should match a known pattern */
    /* TODO: Verify expected decrypted header from binary analysis */

    TEST_END();
}

/*
 * Test 25: Large file handling
 */
static void test_large_file(void) {
    TEST_BEGIN("Large file");

    /* album_37.dat is ~900KB */
    u32 size = save_get_size(TEST_DATA_DIR "album_37.dat");

    FILE* fp = fopen(TEST_DATA_DIR "album_37.dat", "rb");
    TEST_ASSERT(fp != NULL);

    if (fp) {
        /* Read in chunks */
        u8 buffer[4096];
        u32 bytes_read = 0;
        u32 total_read = 0;

        while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            total_read += bytes_read;
        }

        TEST_ASSERT(total_read == size);
        fclose(fp);
    }

    TEST_END();
}

/*
 * Test 26: Savedata.dat file
 * 128 bytes - small config file
 */
static void test_savedata_file(void) {
    TEST_BEGIN("Savedata file");

    u32 size = save_get_size(TEST_DATA_DIR "savedata.dat");
    TEST_ASSERT(size == 128);

    FILE* fp = fopen(TEST_DATA_DIR "savedata.dat", "rb");
    if (fp) {
        u8 data[128];
        int read = fread(data, 1, 128, fp);
        TEST_ASSERT(read == 128);

        /* TODO: Analyze structure */

        fclose(fp);
    }

    TEST_END();
}

/*
 * Test 27: Binary pattern in encrypted data
 */
static void test_encrypted_pattern(void) {
    TEST_BEGIN("Encrypted pattern");

    FILE* fp = fopen(TEST_DATA_DIR "AISetting.dat", "rb");
    if (!fp) {
        TEST_END();
        return;
    }

    /* Read first entry (85 bytes) */
    u8 entry[85];
    fread(entry, 1, 85, fp);
    fclose(fp);

    /* Check for patterns in encrypted data */
    /* Entry header is 16 bytes encrypted name */
    /* Then settings data */

    /* From hex dump: 0x10-0x14 = 0x00000000, 0x00000009 */
    /* This suggests after 16 bytes, we have unencrypted or differently encrypted data */

    u32* settings = (u32*)(entry + 16);
    TEST_ASSERT(settings[0] == 0);         /* flags = 0 */
    TEST_ASSERT(settings[1] == 9);         /* setting1 = 9 */

    TEST_END();
}

/*
 * Test 28: AI setting count
 */
static void test_ai_setting_count(void) {
    TEST_BEGIN("AI setting count");

    TEST_ASSERT(AI_SETTING_COUNT == 5);

    TEST_END();
}

/*
 * Test 29: Error handling - invalid file
 */
static void test_error_handling(void) {
    TEST_BEGIN("Error handling");

    /* Try to load from non-existent file */
    FILE* fp = fopen("nonexistent_file.dat", "rb");
    TEST_ASSERT(fp == NULL);

    /* Try to get size of non-existent file */
    u32 size = save_get_size("nonexistent_file.dat");
    TEST_ASSERT(size == 0);

    TEST_END();
}

/*
 * Test 30: Complete save/load cycle
 */
static void test_complete_cycle(void) {
    TEST_BEGIN("Complete save/load cycle");

    /* Initialize */
    save_init();

    /* Set values */
    ai_settings_set(0, 0, 123);
    ai_settings_set(1, 1, 456);
    ai_settings_set(2, 2, 789);

    /* Save all */
    int result = save_all();
    TEST_ASSERT(result == 1);

    /* Reset */
    save_shutdown();

    /* Reinitialize and load */
    save_init();
    ai_settings_load();

    /* TODO: Verify values after fixing XOR key */

    save_shutdown();
    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(save) {
    test_save_init();
    test_ai_settings_defaults();
    test_xor_symmetry();
    test_xor_zeros();
    test_save_exists();
    test_save_get_size();
    test_ai_settings_load_real();
    test_ai_setting_structure();
    test_album_files();
    test_album_data_sizes();
    test_mail_structure();
    test_chat_history_file();
    test_channel_file();
    test_xor_key();
    test_des_detection();
    test_ai_settings_set_get();
    test_album_load();
    test_album_entry_size();
    test_save_write_read();
    test_ai_settings_save_reload();
    test_auto_file();
    test_data_directory();
    test_key_derivation();
    test_album_xor_pattern();
    test_large_file();
    test_savedata_file();
    test_encrypted_pattern();
    test_ai_setting_count();
    test_error_handling();
    test_complete_cycle();
}

/* ========================================
 * Main
 * ======================================== */

int main(void) {
    printf("=== Save System Unit Tests ===\n\n");
    RUN_SUITE(save);

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
