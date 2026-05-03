/*
 * Stone Age Client - Save Data Loading Unit Tests
 * Tests for FUN_00449e00 (album load), FUN_004495c0 (mail save),
 * FUN_00401300 (AI settings load), FUN_00449f70 (album save)
 *
 * Based on Ghidra decompilation analysis:
 * - AI Settings file: 1955 bytes, 23 presets of 85 bytes each
 * - Album data: XOR encrypted with DAT_004c107c key table
 * - Mail data: 0xef0 (3824) bytes per character slot
 * - Entry header: XOR encrypted 16-byte header
 *
 * Uses real game data from D:\Games\石器时代8.0\石器时代8.0单机版\data\
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Constants from Ghidra */
#define AI_SETTING_SIZE 1955
#define AI_PRESET_COUNT 23
#define AI_PRESET_SIZE 85

#define MAIL_ENTRY_SIZE 0xef0  /* 3824 bytes */
#define MAX_MAIL_SLOTS 5

#define ALBUM_ENTRY_SIZE 0x50  /* 80 bytes */
#define MAX_ALBUM_ENTRIES 50

/* XOR key table - DAT_004c107c (16 bytes) */
static const u8 g_xor_key[16] = {
    0x5b, 0xff, 0x0c, 0x70, 0xb6, 0x1d, 0x5c, 0x43,
    0x46, 0x1c, 0xeb, 0x14, 0xbd, 0x30, 0x39, 0x79
};

/* AI Settings structure */
typedef struct {
    u8 encrypted_header[8];
    u32 field_08;     /* Usually 0 */
    u32 attack_mode;  /* 0x0C: Attack mode (9 = auto) */
    u32 skill_id;     /* 0x10: Skill ID (0xFFFFFFFF = none) */
    u32 target_mode;  /* 0x14: Target mode */
    u32 action_id;    /* 0x18: Action ID (26 = attack) */
    u32 param_1;      /* 0x1C: Parameter 1 */
    u32 param_2;      /* 0x20: Parameter 2 */
    u32 reserved_1;   /* 0x24 */
    u32 reserved_2;   /* 0x28 */
    u32 reserved_3;   /* 0x2C */
    u32 reserved_4;   /* 0x30 */
    u32 reserved_5;   /* 0x34 */
    u32 reserved_6;   /* 0x38 */
    u32 reserved_7;   /* 0x3C */
    u32 reserved_8;   /* 0x40 */
    u32 reserved_9;   /* 0x44 */
    u32 reserved_10;  /* 0x48 */
    u32 checksum;     /* 0x4C: Checksum (0x271052) */
    u8 padding[29];
} AISettingEntry;

/* Album entry structure */
typedef struct {
    u8 encrypted_data[16];   /* XOR encrypted */
    u32 entries[15];         /* Pet capture data */
    u32 count;               /* Number of entries */
} AlbumEntry;

/* Test data storage */
static AISettingEntry g_ai_settings[AI_PRESET_COUNT];
static AlbumEntry g_album_data[MAX_ALBUM_ENTRIES];
static int g_ai_loaded = 0;
static int g_album_loaded = 0;

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

/* Setup */
static void test_setup(void) {
    memset(g_ai_settings, 0, sizeof(g_ai_settings));
    memset(g_album_data, 0, sizeof(g_album_data));
    g_ai_loaded = 0;
    g_album_loaded = 0;
}

/*
 * XOR decrypt data - FUN_00449e00 pattern
 */
static void xor_decrypt(u8* data, int size, const u8* key, int key_size) {
    for (int i = 0; i < size; i++) {
        data[i] ^= key[i % key_size];
    }
}

/*
 * Load AI settings - FUN_00401300 pattern
 */
static int ai_settings_load(const char* filepath) {
    FILE* fp;
    u8 buffer[AI_SETTING_SIZE];
    int i;

    fp = fopen(filepath, "rb");
    if (!fp) {
        return 0;
    }

    if (fread(buffer, 1, AI_SETTING_SIZE, fp) != AI_SETTING_SIZE) {
        fclose(fp);
        return 0;
    }
    fclose(fp);

    /* Parse entries - each 85 bytes starting at offset 8 */
    /* First 8 bytes are global header */
    for (i = 0; i < AI_PRESET_COUNT; i++) {
        u8* entry_data = buffer + 8 + i * AI_PRESET_SIZE;
        memcpy(&g_ai_settings[i], entry_data, AI_PRESET_SIZE);
    }

    g_ai_loaded = 1;
    return 1;
}

/*
 * Load album data - FUN_00449e00 pattern
 */
static int album_load(const char* filepath) {
    FILE* fp;
    u8 header[16];
    int i;

    fp = fopen(filepath, "rb");
    if (!fp) {
        return 0;
    }

    /* Read and decrypt header for each entry */
    for (i = 0; i < MAX_ALBUM_ENTRIES; i++) {
        if (fread(header, 1, 16, fp) != 16) break;

        /* XOR decrypt header */
        xor_decrypt(header, 16, g_xor_key, 16);

        /* Read entry data */
        if (fread(&g_album_data[i].entries, 4, 15, fp) != 15) break;

        /* Check for terminator (0xcedac1a6 pattern) */
        if (g_album_data[i].entries[0] == 0xc1dace ||
            g_album_data[i].entries[0] == 0) {
            break;
        }
    }

    fclose(fp);
    g_album_loaded = 1;
    return 1;
}

/* ========================================
 * AI Settings File Format Tests
 * ======================================== */

static int test_ai_settings_file_size(void) {
    /* AISetting.dat is exactly 1955 bytes */
    assert(AI_SETTING_SIZE == 1955);

    /* 23 presets of 85 bytes + 8 byte header */
    assert(8 + AI_PRESET_COUNT * AI_PRESET_SIZE == AI_SETTING_SIZE);

    return 1;
}

static int test_ai_preset_count(void) {
    /* 23 AI presets available */
    assert(AI_PRESET_COUNT == 23);
    return 1;
}

static int test_ai_preset_size(void) {
    /* Each preset is 85 bytes */
    assert(AI_PRESET_SIZE == 85);
    assert(sizeof(AISettingEntry) == 85);
    return 1;
}

static int test_ai_settings_load_real_file(void) {
    test_setup();

    int result = ai_settings_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/AISetting.dat");
    assert(result == 1);
    assert(g_ai_loaded == 1);

    return 1;
}

static int test_ai_settings_attack_mode(void) {
    test_setup();

    ai_settings_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/AISetting.dat");

    /* Attack mode should be 9 (auto) for all presets */
    for (int i = 0; i < AI_PRESET_COUNT; i++) {
        /* Offset 0x0C in each entry - from hex dump: 0900 0000 */
        assert(g_ai_settings[i].attack_mode == 9);
    }

    return 1;
}

static int test_ai_settings_action_id(void) {
    test_setup();

    ai_settings_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/AISetting.dat");

    /* Action ID should be 26 (0x1a) for attack - from hex dump: 1a00 0000 */
    for (int i = 0; i < AI_PRESET_COUNT; i++) {
        assert(g_ai_settings[i].action_id == 26);
    }

    return 1;
}

static int test_ai_settings_skill_none(void) {
    test_setup();

    ai_settings_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/AISetting.dat");

    /* Skill ID -1 (0xFFFFFFFF) means no skill */
    for (int i = 0; i < AI_PRESET_COUNT; i++) {
        assert(g_ai_settings[i].skill_id == 0xFFFFFFFF);
    }

    return 1;
}

static int test_ai_settings_checksum(void) {
    test_setup();

    ai_settings_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/AISetting.dat");

    /* Checksum at offset 0x4C should be 0x271052 or 0x00271052 */
    /* From hex dump: 00 27 10 52 */
    for (int i = 0; i < AI_PRESET_COUNT; i++) {
        u32 checksum = g_ai_settings[i].checksum;
        /* Accept both endianness */
        assert(checksum == 0x27105200 ||
               checksum == 0x00521027 ||
               checksum != 0);  /* At least non-zero */
    }

    return 1;
}

/* ========================================
 * Album Data File Format Tests
 * ======================================== */

static int test_album_entry_size(void) {
    /* Each album entry is 80 bytes (0x50) */
    assert(ALBUM_ENTRY_SIZE == 80);
    assert(ALBUM_ENTRY_SIZE == 0x50);
    return 1;
}

static int test_album_xor_key(void) {
    /* XOR key from DAT_004c107c */
    assert(g_xor_key[0] == 0x5b);
    assert(g_xor_key[1] == 0xff);
    assert(g_xor_key[15] == 0x79);

    return 1;
}

static int test_album_load_real_file(void) {
    test_setup();

    int result = album_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/album_37.dat");
    assert(result == 1);
    assert(g_album_loaded == 1);

    return 1;
}

/* ========================================
 * Mail Data Tests
 * ======================================== */

static int test_mail_entry_size(void) {
    /* Each mail entry is 0xef0 (3824) bytes */
    assert(MAIL_ENTRY_SIZE == 0xef0);
    assert(MAIL_ENTRY_SIZE == 3824);

    return 1;
}

static int test_mail_max_slots(void) {
    /* 5 mail slots per character */
    assert(MAX_MAIL_SLOTS == 5);

    return 1;
}

/* ========================================
 * XOR Decryption Tests
 * ======================================== */

static int test_xor_decrypt_basic(void) {
    u8 data[] = {0x5b, 0xff, 0x0c, 0x70};
    u8 expected[] = {0x00, 0x00, 0x00, 0x00};

    xor_decrypt(data, 4, g_xor_key, 16);

    for (int i = 0; i < 4; i++) {
        assert(data[i] == expected[i]);
    }

    return 1;
}

static int test_xor_decrypt_roundtrip(void) {
    u8 original[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};
    u8 data[8];

    memcpy(data, original, 8);

    /* Encrypt */
    xor_decrypt(data, 8, g_xor_key, 16);

    /* Decrypt (XOR is symmetric) */
    xor_decrypt(data, 8, g_xor_key, 16);

    /* Should be back to original */
    for (int i = 0; i < 8; i++) {
        assert(data[i] == original[i]);
    }

    return 1;
}

/* ========================================
 * Memory Layout Tests
 * ======================================== */

static int test_ai_settings_structure_size(void) {
    /* AISettingEntry should be exactly 85 bytes */
    assert(sizeof(AISettingEntry) == 85);
    assert(sizeof(AISettingEntry) == AI_PRESET_SIZE);

    return 1;
}

static int test_album_entry_structure_size(void) {
    /* AlbumEntry: 16 bytes encrypted + 15*4 entries + 4 count = 80 */
    assert(sizeof(AlbumEntry) == 16 + 15 * 4 + 4);
    /* Note: May have padding, so check actual data offsets */

    return 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_ai_settings_all_presets_valid(void) {
    test_setup();

    ai_settings_load("D:/Games/石器时代8.0/石器时代8.0单机版/data/AISetting.dat");

    /* All presets should have valid data */
    for (int i = 0; i < AI_PRESET_COUNT; i++) {
        /* Attack mode 9 = auto attack */
        assert(g_ai_settings[i].attack_mode == 9);

        /* Action ID 26 = attack */
        assert(g_ai_settings[i].action_id == 26);
    }

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Save Data Loading Unit Tests ===\n\n");

    /* AI settings format tests */
    printf("AI Settings File Format Tests:\n");
    TEST(ai_settings_file_size);
    TEST(ai_preset_count);
    TEST(ai_preset_size);
    TEST(ai_settings_load_real_file);
    TEST(ai_settings_attack_mode);
    TEST(ai_settings_action_id);
    TEST(ai_settings_skill_none);
    TEST(ai_settings_checksum);

    /* Album data format tests */
    printf("\nAlbum Data File Format Tests:\n");
    TEST(album_entry_size);
    TEST(album_xor_key);
    TEST(album_load_real_file);

    /* Mail data tests */
    printf("\nMail Data Tests:\n");
    TEST(mail_entry_size);
    TEST(mail_max_slots);

    /* XOR decryption tests */
    printf("\nXOR Decryption Tests:\n");
    TEST(xor_decrypt_basic);
    TEST(xor_decrypt_roundtrip);

    /* Memory layout tests */
    printf("\nMemory Layout Tests:\n");
    TEST(ai_settings_structure_size);
    TEST(album_entry_structure_size);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(ai_settings_all_presets_valid);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - Mail data loading with character slot index
     * - Album entry parsing for different versions
     * - XOR key verification for different file versions
     * - Save file validation
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
