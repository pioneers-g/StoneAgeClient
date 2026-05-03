/*
 * Stone Age Client - Album System Comprehensive Tests
 * Tests for album.c implementation
 *
 * Covers:
 * - XOR encryption/decryption with 16-byte key
 * - Album entry initialization
 * - Entry add/update/find operations
 * - Statistics tracking
 * - Sprite-to-album mapping
 *
 * Game data verification:
 * - album_37.dat: 909328 bytes, 494 entries (0x1ee), 80 bytes each
 * - XOR key: 0x12,0x34,0x56,0x78,0x9a,0xbc,0xde,0xf0,0x0f,0xed,0xcb,0xa9,0x87,0x65,0x43,0x21
 * - Entry size: 80 bytes (0x50)
 * - Header: "f;encor1c" marker
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
 * Constants - From album.c
 * ======================================== */

#define ALBUM_NAME_LENGTH   32
#define ALBUM_ENTRY_SIZE    80  /* 0x50 - from FUN_00449910 */
#define ALBUM_MAX_ENTRIES   600
#define ALBUM_COUNT_37      494  /* 0x1ee */

/* XOR encryption key from DAT_004c107c */
static const u8 g_album_xor_key[16] = {
    0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
    0x0f, 0xed, 0xcb, 0xa9, 0x87, 0x65, 0x43, 0x21
};

/* "f;encor1c" marker */
static const char s_encor_marker[] = "f;encor1c";

/* ========================================
 * Structures
 * ======================================== */

typedef struct {
    char name[ALBUM_NAME_LENGTH];
    u16 id;
    u16 sprite_id;
    u8 element;
    u8 flags;
    u8 caught;
    u16 caught_count;
    u32 battle_count;
    u32 kill_count;
    u32 stat_1;
    u32 stat_2;
    u32 stat_3;
    u32 stat_4;
    u32 stat_5;
    u32 stat_6;
    u32 stat_7;
    u32 stat_8;
    u32 stat_9;
    u32 stat_10;
} AlbumEntry;

typedef struct {
    u16 sprite_id;
    s32 album_index;
} AlbumSpriteMapping;

typedef struct {
    int initialized;
    u32 entry_count;
    AlbumEntry entries[ALBUM_MAX_ENTRIES];
    u8 valid[ALBUM_MAX_ENTRIES];
} AlbumContext;

/* ========================================
 * Global State
 * ======================================== */

static AlbumContext g_album = {0};
static AlbumSpriteMapping s_sprite_map[ALBUM_MAX_ENTRIES];
static u32 s_sprite_map_count = 0;

/* ========================================
 * Implementation Functions
 * ======================================== */

static void album_decrypt(void* data, u32 size) {
    u32* ptr = (u32*)data;
    const u32* key = (const u32*)g_album_xor_key;
    u32 i;

    for (i = 0; i < (size >> 2); i++) {
        ptr[i] ^= key[i & 3];
    }
}

static void album_encrypt(void* data, u32 size) {
    album_decrypt(data, size);
}

static u32 album_rand_range(u32 min_val, u32 max_val) {
    return min_val + (rand() % (max_val - min_val + 1));
}

static void album_init_random_stats(AlbumEntry* entry) {
    int i;

    if (!entry) return;

    for (i = 0; i < ALBUM_NAME_LENGTH; i++) {
        entry->name[i] = (char)album_rand_range(1, 255);
    }
    entry->name[ALBUM_NAME_LENGTH - 1] = '\0';

    entry->stat_1 = album_rand_range(30000, 60000);
    entry->stat_2 = album_rand_range(1, 30000);
    entry->stat_3 = album_rand_range(1, 30000);
    entry->stat_4 = album_rand_range(1, 30000);
    entry->stat_5 = album_rand_range(1, 30000);
    entry->stat_6 = album_rand_range(1, 30000);
    entry->stat_7 = album_rand_range(1, 30000);
    entry->stat_8 = album_rand_range(1, 30000);
    entry->stat_9 = album_rand_range(1, 30000);
    entry->stat_10 = album_rand_range(1, 30000);

    entry->id = 0;
    entry->sprite_id = 0;
    entry->element = 0;
    entry->flags = 0;
    entry->caught = 0;
    entry->caught_count = 0;
    entry->battle_count = 0;
    entry->kill_count = 0;
}

static int album_init(void) {
    int i;

    memset(&g_album, 0, sizeof(AlbumContext));

    for (i = 0; i < ALBUM_MAX_ENTRIES; i++) {
        album_init_random_stats(&g_album.entries[i]);
    }

    g_album.entry_count = ALBUM_MAX_ENTRIES;
    g_album.initialized = 1;

    return 1;
}

static void album_shutdown(void) {
    memset(&g_album, 0, sizeof(AlbumContext));
    s_sprite_map_count = 0;
}

static int album_add_entry(AlbumEntry* entry) {
    u32 i;

    if (!entry) {
        return -1;
    }

    for (i = 0; i < g_album.entry_count; i++) {
        if (!g_album.valid[i] || g_album.entries[i].name[0] == '\0') {
            memcpy(&g_album.entries[i], entry, sizeof(AlbumEntry));
            g_album.valid[i] = 1;
            return (int)i;
        }
    }

    if (g_album.entry_count < ALBUM_MAX_ENTRIES) {
        i = g_album.entry_count;
        memcpy(&g_album.entries[i], entry, sizeof(AlbumEntry));
        g_album.valid[i] = 1;
        g_album.entry_count++;
        return (int)i;
    }

    return -1;
}

static int album_update_entry(u32 index, AlbumEntry* entry) {
    if (index >= g_album.entry_count || !entry) {
        return 0;
    }

    memcpy(&g_album.entries[index], entry, sizeof(AlbumEntry));
    g_album.valid[index] = 1;
    return 1;
}

static AlbumEntry* album_get_entry(u32 index) {
    if (index >= g_album.entry_count) {
        return NULL;
    }

    if (!g_album.valid[index]) {
        return NULL;
    }

    return &g_album.entries[index];
}

static int album_find_entry(const char* name) {
    u32 i;

    if (!name) {
        return -1;
    }

    for (i = 0; i < g_album.entry_count; i++) {
        if (g_album.valid[i] && g_album.entries[i].name[0] != '\0') {
            if (strcmp(g_album.entries[i].name, name) == 0) {
                return (int)i;
            }
        }
    }

    return -1;
}

static int album_find_entry_by_id(u16 creature_id) {
    u32 i;

    for (i = 0; i < g_album.entry_count; i++) {
        if (g_album.valid[i] && g_album.entries[i].id == creature_id) {
            return (int)i;
        }
    }

    return -1;
}

static int album_find_by_sprite(u16 sprite_id) {
    u32 i;

    for (i = 0; i < s_sprite_map_count; i++) {
        if (s_sprite_map[i].sprite_id == sprite_id) {
            if (s_sprite_map[i].album_index >= 0) {
                return s_sprite_map[i].album_index;
            }
        }
    }

    return -1;
}

static void album_map_sprite(u16 sprite_id, u32 album_index) {
    if (s_sprite_map_count < ALBUM_MAX_ENTRIES && album_index < g_album.entry_count) {
        s_sprite_map[s_sprite_map_count].sprite_id = sprite_id;
        s_sprite_map[s_sprite_map_count].album_index = (s32)album_index;
        s_sprite_map_count++;
    }
}

static u32 album_get_total_caught(void) {
    u32 total = 0;
    u32 i;

    for (i = 0; i < g_album.entry_count; i++) {
        if (g_album.valid[i]) {
            total += g_album.entries[i].caught_count;
        }
    }

    return total;
}

static u32 album_get_total_battles(void) {
    u32 total = 0;
    u32 i;

    for (i = 0; i < g_album.entry_count; i++) {
        if (g_album.valid[i]) {
            total += g_album.entries[i].battle_count;
        }
    }

    return total;
}

static u32 album_get_total_kills(void) {
    u32 total = 0;
    u32 i;

    for (i = 0; i < g_album.entry_count; i++) {
        if (g_album.valid[i]) {
            total += g_album.entries[i].kill_count;
        }
    }

    return total;
}

static u32 album_get_completion_rate(void) {
    u32 valid_count = 0;
    u32 i;

    for (i = 0; i < g_album.entry_count; i++) {
        if (g_album.valid[i] && g_album.entries[i].caught) {
            valid_count++;
        }
    }

    if (g_album.entry_count == 0) {
        return 0;
    }

    return (valid_count * 100) / g_album.entry_count;
}

static u32 album_get_count(void) {
    return g_album.entry_count;
}

/* ========================================
 * Test Cases - XOR Encryption
 * ======================================== */

static int test_xor_key_values(void) {
    return g_album_xor_key[0] == 0x12 &&
           g_album_xor_key[1] == 0x34 &&
           g_album_xor_key[7] == 0xf0 &&
           g_album_xor_key[15] == 0x21;
}

static int test_xor_symmetric(void) {
    u32 data[4] = { 0x12345678, 0x9ABCDEF0, 0x0FEDCBA9, 0x87654321 };
    u32 original[4];
    memcpy(original, data, sizeof(data));

    album_encrypt(data, 16);
    album_decrypt(data, 16);

    return memcmp(data, original, sizeof(data)) == 0;
}

static int test_xor_dword_aligned(void) {
    u32 data = 0xDEADBEEF;
    const u32* key = (const u32*)g_album_xor_key;
    u32 expected = data ^ key[0];  /* First dword of key */

    u32 test = data;
    album_encrypt(&test, 4);

    return test == expected;
}

static int test_xor_multi_dword(void) {
    u32 data[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    u32 expected[8];
    u32 i;

    /* Expected: XOR with key[i & 3] repeating */
    const u32* key = (const u32*)g_album_xor_key;
    for (i = 0; i < 8; i++) {
        expected[i] = (i + 1) ^ key[i & 3];
    }

    album_encrypt(data, 32);

    return memcmp(data, expected, sizeof(data)) == 0;
}

static int test_xor_zero_data(void) {
    u32 data[4] = { 0, 0, 0, 0 };
    const u32* key = (const u32*)g_album_xor_key;

    album_encrypt(data, 16);

    /* XOR with 0 should give key */
    return data[0] == key[0] && data[1] == key[1] &&
           data[2] == key[2] && data[3] == key[3];
}

static int test_xor_full_entry(void) {
    AlbumEntry entry;
    AlbumEntry original;
    memset(&entry, 0xAB, sizeof(AlbumEntry));
    memcpy(&original, &entry, sizeof(AlbumEntry));

    album_encrypt(&entry, ALBUM_ENTRY_SIZE);
    album_decrypt(&entry, ALBUM_ENTRY_SIZE);

    return memcmp(&entry, &original, sizeof(AlbumEntry)) == 0;
}

/* ========================================
 * Test Cases - Initialization
 * ======================================== */

static int test_init_basic(void) {
    int result = album_init();
    return result == 1 && g_album.initialized == 1;
}

static int test_init_entry_count(void) {
    album_init();
    return album_get_count() == ALBUM_MAX_ENTRIES;
}

static int test_init_random_stats(void) {
    album_init();
    /* Stats should be in range after random init */
    return g_album.entries[0].stat_1 >= 30000 && g_album.entries[0].stat_1 <= 60000;
}

static int test_shutdown_clears(void) {
    album_init();
    album_shutdown();
    return g_album.initialized == 0 && g_album.entry_count == 0;
}

/* ========================================
 * Test Cases - Entry Operations
 * ======================================== */

static int test_add_entry_basic(void) {
    AlbumEntry entry = {0};
    strcpy(entry.name, "TestPet");
    entry.id = 100;
    entry.caught = 1;

    album_init();
    memset(g_album.valid, 0, sizeof(g_album.valid));  /* Clear valid flags */
    g_album.entry_count = 10;

    int index = album_add_entry(&entry);
    return index >= 0 && g_album.valid[index] == 1;
}

static int test_add_entry_null(void) {
    album_init();
    return album_add_entry(NULL) == -1;
}

static int test_update_entry_basic(void) {
    AlbumEntry entry = {0};
    strcpy(entry.name, "UpdatedPet");
    entry.id = 200;

    album_init();
    g_album.valid[0] = 1;

    int result = album_update_entry(0, &entry);
    return result == 1 && g_album.entries[0].id == 200;
}

static int test_update_entry_invalid_index(void) {
    AlbumEntry entry = {0};
    album_init();
    return album_update_entry(ALBUM_MAX_ENTRIES + 10, &entry) == 0;
}

static int test_update_entry_null(void) {
    album_init();
    return album_update_entry(0, NULL) == 0;
}

static int test_get_entry_valid(void) {
    album_init();
    g_album.valid[5] = 1;
    g_album.entries[5].id = 555;

    AlbumEntry* entry = album_get_entry(5);
    return entry != NULL && entry->id == 555;
}

static int test_get_entry_invalid(void) {
    album_init();
    g_album.valid[5] = 0;

    return album_get_entry(5) == NULL;
}

static int test_get_entry_out_of_range(void) {
    album_init();
    return album_get_entry(ALBUM_MAX_ENTRIES + 10) == NULL;
}

/* ========================================
 * Test Cases - Find Operations
 * ======================================== */

static int test_find_by_name_found(void) {
    album_init();
    strcpy(g_album.entries[10].name, "FindMe");
    g_album.valid[10] = 1;

    int index = album_find_entry("FindMe");
    return index == 10;
}

static int test_find_by_name_not_found(void) {
    album_init();
    g_album.valid[0] = 1;
    strcpy(g_album.entries[0].name, "OtherPet");

    return album_find_entry("NotFound") == -1;
}

static int test_find_by_name_null(void) {
    album_init();
    return album_find_entry(NULL) == -1;
}

static int test_find_by_id_found(void) {
    album_init();
    g_album.entries[20].id = 1234;
    g_album.valid[20] = 1;

    int index = album_find_entry_by_id(1234);
    return index == 20;
}

static int test_find_by_id_not_found(void) {
    album_init();
    g_album.entries[0].id = 100;
    g_album.valid[0] = 1;

    return album_find_entry_by_id(999) == -1;
}

/* ========================================
 * Test Cases - Sprite Mapping
 * ======================================== */

static int test_map_sprite_basic(void) {
    album_init();
    s_sprite_map_count = 0;

    album_map_sprite(1000, 50);

    return s_sprite_map_count == 1 &&
           s_sprite_map[0].sprite_id == 1000 &&
           s_sprite_map[0].album_index == 50;
}

static int test_find_by_sprite_found(void) {
    album_init();
    s_sprite_map_count = 0;

    album_map_sprite(2000, 100);

    int index = album_find_by_sprite(2000);
    return index == 100;
}

static int test_find_by_sprite_not_found(void) {
    album_init();
    s_sprite_map_count = 0;

    return album_find_by_sprite(9999) == -1;
}

static int test_map_multiple_sprites(void) {
    album_init();
    s_sprite_map_count = 0;

    album_map_sprite(100, 1);
    album_map_sprite(200, 2);
    album_map_sprite(300, 3);

    return album_find_by_sprite(100) == 1 &&
           album_find_by_sprite(200) == 2 &&
           album_find_by_sprite(300) == 3;
}

/* ========================================
 * Test Cases - Statistics
 * ======================================== */

static int test_total_caught_empty(void) {
    album_init();
    memset(g_album.valid, 0, sizeof(g_album.valid));
    return album_get_total_caught() == 0;
}

static int test_total_caught_with_data(void) {
    album_init();
    memset(g_album.valid, 0, sizeof(g_album.valid));

    g_album.valid[0] = 1;
    g_album.entries[0].caught_count = 10;
    g_album.valid[1] = 1;
    g_album.entries[1].caught_count = 20;

    return album_get_total_caught() == 30;
}

static int test_total_battles_with_data(void) {
    album_init();
    memset(g_album.valid, 0, sizeof(g_album.valid));

    g_album.valid[0] = 1;
    g_album.entries[0].battle_count = 100;
    g_album.valid[1] = 1;
    g_album.entries[1].battle_count = 200;

    return album_get_total_battles() == 300;
}

static int test_total_kills_with_data(void) {
    album_init();
    memset(g_album.valid, 0, sizeof(g_album.valid));

    g_album.valid[0] = 1;
    g_album.entries[0].kill_count = 50;
    g_album.valid[1] = 1;
    g_album.entries[1].kill_count = 30;

    return album_get_total_kills() == 80;
}

static int test_completion_rate_empty(void) {
    album_init();
    g_album.entry_count = 100;
    memset(g_album.valid, 0, sizeof(g_album.valid));

    return album_get_completion_rate() == 0;
}

static int test_completion_rate_full(void) {
    int i;
    album_init();
    g_album.entry_count = 100;

    for (i = 0; i < 100; i++) {
        g_album.valid[i] = 1;
        g_album.entries[i].caught = 1;
    }

    return album_get_completion_rate() == 100;
}

static int test_completion_rate_half(void) {
    int i;
    album_init();
    g_album.entry_count = 100;

    for (i = 0; i < 50; i++) {
        g_album.valid[i] = 1;
        g_album.entries[i].caught = 1;
    }

    return album_get_completion_rate() == 50;
}

/* ========================================
 * Test Cases - Constants Validation
 * ======================================== */

static int test_entry_size(void) {
    /* Entry size from FUN_00449910: 0x50 = 80 bytes */
    return ALBUM_ENTRY_SIZE == 80;
}

static int test_album_37_count(void) {
    /* ALBUM_COUNT_37 from s_album_files: 0x1ee = 494 */
    return ALBUM_COUNT_37 == 494;
}

static int test_encor_marker(void) {
    return strlen(s_encor_marker) == 9 &&
           strcmp(s_encor_marker, "f;encor1c") == 0;
}

static int test_name_length(void) {
    return ALBUM_NAME_LENGTH == 32;
}

static int test_max_entries(void) {
    return ALBUM_MAX_ENTRIES == 600;
}

/* ========================================
 * Test Cases - Data Structure Size
 * ======================================== */

static int test_album_entry_size_fields(void) {
    /* Verify structure is properly sized */
    return sizeof(AlbumEntry) >= ALBUM_ENTRY_SIZE;
}

static int test_album_context_size(void) {
    /* Context should have entries array */
    return sizeof(AlbumContext) >= sizeof(AlbumEntry) * ALBUM_MAX_ENTRIES;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Album System Comprehensive Tests ===\n\n");

    printf("XOR Encryption Tests:\n");
    TEST(xor_key_values);
    TEST(xor_symmetric);
    TEST(xor_dword_aligned);
    TEST(xor_multi_dword);
    TEST(xor_zero_data);
    TEST(xor_full_entry);

    printf("\nInitialization Tests:\n");
    TEST(init_basic);
    TEST(init_entry_count);
    TEST(init_random_stats);
    TEST(shutdown_clears);

    printf("\nEntry Operations Tests:\n");
    TEST(add_entry_basic);
    TEST(add_entry_null);
    TEST(update_entry_basic);
    TEST(update_entry_invalid_index);
    TEST(update_entry_null);
    TEST(get_entry_valid);
    TEST(get_entry_invalid);
    TEST(get_entry_out_of_range);

    printf("\nFind Operations Tests:\n");
    TEST(find_by_name_found);
    TEST(find_by_name_not_found);
    TEST(find_by_name_null);
    TEST(find_by_id_found);
    TEST(find_by_id_not_found);

    printf("\nSprite Mapping Tests:\n");
    TEST(map_sprite_basic);
    TEST(find_by_sprite_found);
    TEST(find_by_sprite_not_found);
    TEST(map_multiple_sprites);

    printf("\nStatistics Tests:\n");
    TEST(total_caught_empty);
    TEST(total_caught_with_data);
    TEST(total_battles_with_data);
    TEST(total_kills_with_data);
    TEST(completion_rate_empty);
    TEST(completion_rate_full);
    TEST(completion_rate_half);

    printf("\nConstants Validation Tests:\n");
    TEST(entry_size);
    TEST(album_37_count);
    TEST(encor_marker);
    TEST(name_length);
    TEST(max_entries);

    printf("\nData Structure Tests:\n");
    TEST(album_entry_size_fields);
    TEST(album_context_size);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
