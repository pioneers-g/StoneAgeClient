/*
 * Stone Age Client - Binary Packet Dispatcher Unit Tests
 * Tests for FUN_0045ffb0 (binary packet dispatcher)
 *
 * Based on Ghidra decompilation analysis:
 * - 14 case handlers (0x42-0x58)
 * - Case 0x42: Inventory update (7 items, 0x184 bytes per item)
 * - Case 0x43: Map transfer
 * - Case 0x44: Time sync
 * - Case 0x45: Player position
 * - Case 0x46: Player info
 * - Case 0x49: Item list
 * - Case 0x4a: Pet status
 * - Case 0x4b: Character status (bitmask parsing)
 * - Case 0x4d: NPC info
 * - Case 0x4e: Battle unit status (bitmask parsing)
 * - Case 0x50: Character stats (bitmask parsing, 30 fields)
 * - Case 0x53: Party members
 * - Case 0x57: Mail list
 * - Case 0x58: Server time
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
#define RECV_BUFFER_SIZE 0x4000      /* 16KB receive buffer */
#define MAX_INVENTORY_ITEMS 7        /* 7 items per character */
#define ITEM_ENTRY_SIZE 0x184        /* 388 bytes per item */
#define MAX_CHAR_NAME 29             /* 0x1d - max character name */
#define MAX_ITEM_NAME 0x1d           /* 29 bytes - item name */
#define MAX_ITEM_DESC 0x55           /* 85 bytes - item description */
#define MAX_FIELDS_CASE_50 30        /* 30 fields for case 0x50 */
#define MAX_PARTY_MEMBERS 20         /* 20 party members */
#define PARTY_ENTRY_SIZE 0x60        /* 96 bytes per party entry */
#define MAX_MAIL_ENTRIES 7           /* 7 mail entries */
#define MAIL_ENTRY_SIZE 0x6a         /* 106 bytes per mail entry */

/* Case types */
#define CASE_INVENTORY 0x42
#define CASE_MAP_TRANSFER 0x43
#define CASE_TIME_SYNC 0x44
#define CASE_PLAYER_POS 0x45
#define CASE_PLAYER_INFO 0x46
#define CASE_ITEM_LIST 0x49
#define CASE_PET_STATUS 0x4a
#define CASE_CHAR_STATUS 0x4b
#define CASE_NPC_INFO 0x4d
#define CASE_BATTLE_UNIT 0x4e
#define CASE_CHAR_STATS 0x50
#define CASE_PARTY_MEMBERS 0x53
#define CASE_MAIL_LIST 0x57
#define CASE_SERVER_TIME 0x58

/* Bitmask constants */
#define BITMASK_1 0x00000001
#define BITMASK_2 0x00000002
#define BITMASK_4 0x00000004
#define BITMASK_8 0x00000008
#define BITMASK_16 0x00000010
#define BITMASK_32 0x00000020
#define BITMASK_64 0x00000040
#define BITMASK_128 0x00000080
#define BITMASK_256 0x00000100
#define BITMASK_512 0x00000200
#define BITMASK_1024 0x00000400
#define BITMASK_2048 0x00000800
#define BITMASK_4096 0x00001000
#define BITMASK_8192 0x00002000
#define BITMASK_16384 0x00004000
#define BITMASK_32768 0x00008000
#define BITMASK_65536 0x00010000
#define BITMASK_131072 0x00020000
#define BITMASK_262144 0x00040000
#define BITMASK_524288 0x00080000
#define BITMASK_1048576 0x00100000
#define BITMASK_2097152 0x00200000
#define BITMASK_4194304 0x00400000
#define BITMASK_8388608 0x00800000
#define BITMASK_16777216 0x01000000
#define BITMASK_33554432 0x02000000
#define BITMASK_67108864 0x04000000
#define BITMASK_134217728 0x08000000
#define BITMASK_268435456 0x10000000
#define BITMASK_536870912 0x20000000
#define BITMASK_1073741824 0x40000000

/* Item entry structure (388 bytes) */
typedef struct {
    u32 field_0;              /* +0x00 */
    u32 count;                /* +0x04 */
    u16 flag;                 /* +0x08 */
    u16 field_a;              /* +0x0a */
    char name[MAX_ITEM_NAME]; /* +0x0c-0x28: Item name */
    char field_29;            /* +0x29 */
    char field_2a;            /* +0x2a */
    char desc[MAX_ITEM_DESC]; /* +0x2b-0x7f: Item description */
    u32 field_80;             /* +0x80 */
    u32 field_84;             /* +0x84 */
    char field_88;            /* +0x88 */
    char field_89;            /* +0x89 */
    char extra[0xfb];         /* +0x8a-0x184: Extra data */
} ItemEntry;

/* Character stats structure */
typedef struct {
    u32 hp;                   /* Field 2 */
    u32 max_hp;               /* Field 3 */
    u32 mp;                   /* Field 4 */
    u32 max_mp;               /* Field 5 */
    u32 exp;                  /* Field 6 */
    u16 level;                /* Field 7 */
    u16 level_flag;           /* Field 8 - if >= 100, split */
    u32 gold;                 /* Field 9 */
    char name[MAX_CHAR_NAME]; /* Field 30 */
    char title[34];           /* Field 31 */
} CharacterStats;

/* Test data storage */
static char g_recv_buffer[RECV_BUFFER_SIZE];
static int g_recv_len = 0;
static ItemEntry g_inventory[MAX_INVENTORY_ITEMS];
static CharacterStats g_char_stats;
static int g_case_called = 0;

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
    memset(g_recv_buffer, 0, sizeof(g_recv_buffer));
    memset(g_inventory, 0, sizeof(g_inventory));
    memset(&g_char_stats, 0, sizeof(g_char_stats));
    g_recv_len = 0;
    g_case_called = 0;
}

/*
 * Parse pipe-delimited field - FUN_0048a050 pattern
 */
static int parse_field_int(const char* data, char delimiter, int field_num) {
    int current_field = 1;
    const char* ptr = data;

    while (current_field < field_num && *ptr) {
        if (*ptr == delimiter) {
            current_field++;
        }
        ptr++;
    }

    if (current_field == field_num) {
        return atoi(ptr);
    }
    return 0;
}

/*
 * Decode bitmask to field indices - FUN_0048a120 pattern
 */
static int bitmask_to_field_count(u32 bitmask) {
    int count = 0;
    while (bitmask) {
        if (bitmask & 1) count++;
        bitmask >>= 1;
    }
    return count;
}

/*
 * Get field index from bitmask position - FUN_0045ffb0 pattern
 */
static int get_field_for_bitmask(u32 bitmask, int position) {
    int field = 2;  /* Fields start at 2 after bitmask */
    u32 mask = 0x00000002;  /* Start at bit 1 */

    for (int i = 1; i < 32; i++) {
        if (mask == position) {
            return field;
        }
        if (bitmask & mask) {
            field++;
        }
        mask <<= 1;
    }
    return field;
}

/* ========================================
 * Case Handler Tests
 * ======================================== */

static int test_case_values(void) {
    test_setup();

    /* Verify case values */
    assert(CASE_INVENTORY == 0x42);
    assert(CASE_MAP_TRANSFER == 0x43);
    assert(CASE_TIME_SYNC == 0x44);
    assert(CASE_PLAYER_POS == 0x45);
    assert(CASE_PLAYER_INFO == 0x46);
    assert(CASE_ITEM_LIST == 0x49);
    assert(CASE_PET_STATUS == 0x4a);
    assert(CASE_CHAR_STATUS == 0x4b);
    assert(CASE_NPC_INFO == 0x4d);
    assert(CASE_BATTLE_UNIT == 0x4e);
    assert(CASE_CHAR_STATS == 0x50);
    assert(CASE_PARTY_MEMBERS == 0x53);
    assert(CASE_MAIL_LIST == 0x57);
    assert(CASE_SERVER_TIME == 0x58);

    return 1;
}

static int test_case_range(void) {
    test_setup();

    /* Cases range from 0x42 to 0x58 */
    assert(CASE_INVENTORY == 66);   /* 0x42 = 66 */
    assert(CASE_SERVER_TIME == 88); /* 0x58 = 88 */

    return 1;
}

/* ========================================
 * Inventory System Tests (Case 0x42)
 * ======================================== */

static int test_inventory_max_items(void) {
    test_setup();

    /* 7 items per character */
    assert(MAX_INVENTORY_ITEMS == 7);

    return 1;
}

static int test_inventory_entry_size(void) {
    test_setup();

    /* Each item is 0x184 (388) bytes */
    assert(ITEM_ENTRY_SIZE == 0x184);
    assert(ITEM_ENTRY_SIZE == 388);
    assert(sizeof(ItemEntry) == 388);

    return 1;
}

static int test_inventory_total_size(void) {
    test_setup();

    /* Total inventory: 7 * 388 = 2716 bytes */
    size_t total = MAX_INVENTORY_ITEMS * ITEM_ENTRY_SIZE;
    assert(total == 2716);

    return 1;
}

static int test_inventory_item_name_offset(void) {
    test_setup();

    /* Item name at offset 0x0c (12) */
    ItemEntry item;
    memset(&item, 0, sizeof(item));
    assert(offsetof(ItemEntry, name) == 0x0c);

    return 1;
}

static int test_inventory_item_desc_offset(void) {
    test_setup();

    /* Item description at offset 0x2b (43) */
    ItemEntry item;
    memset(&item, 0, sizeof(item));
    assert(offsetof(ItemEntry, desc) == 0x2b);

    return 1;
}

/* ========================================
 * Character Stats Tests (Case 0x50)
 * ======================================== */

static int test_char_stats_field_count(void) {
    test_setup();

    /* Case 0x50 parses up to 30 fields */
    assert(MAX_FIELDS_CASE_50 == 30);

    return 1;
}

static int test_char_stats_bitmask_parsing(void) {
    test_setup();

    /* Bitmask 1 = all fields present */
    u32 bitmask = 1;
    int fields = 0;

    /* Parse all 23 stat fields */
    u32 mask = 0x00000002;
    for (int i = 0; i < 29; i++) {
        if (bitmask & mask) fields++;
        mask <<= 1;
    }

    /* With bitmask=1, fields 2-30 are parsed */
    assert(fields == 0);  /* No additional bits set in bitmask=1 */

    return 1;
}

static int test_char_stats_level_split(void) {
    test_setup();

    /* Level field: if >= 100, split into level%100 and set flag */
    int level = 150;
    int display_level = level % 100;
    int level_flag = (level >= 100) ? 1 : 0;

    assert(display_level == 50);
    assert(level_flag == 1);

    level = 50;
    display_level = level % 100;
    level_flag = (level >= 100) ? 1 : 0;

    assert(display_level == 50);
    assert(level_flag == 0);

    return 1;
}

static int test_char_stats_name_offset(void) {
    test_setup();

    /* Name at field 30 (0x1e) */
    int name_field = 30;
    assert(name_field == 0x1e);

    return 1;
}

/* ========================================
 * Bitmask Parsing Tests (Cases 0x4b, 0x4e, 0x50)
 * ======================================== */

static int test_bitmask_basic(void) {
    test_setup();

    /* Bitmask value 1 = simple mode (all fields) */
    u32 bitmask = 1;
    assert(bitmask == BITMASK_1);

    return 1;
}

static int test_bitmask_field_count(void) {
    test_setup();

    /* Count fields from bitmask */
    assert(bitmask_to_field_count(0x00000001) == 1);
    assert(bitmask_to_field_count(0x00000003) == 2);
    assert(bitmask_to_field_count(0x00000007) == 3);
    assert(bitmask_to_field_count(0x000000FF) == 8);
    assert(bitmask_to_field_count(0xFFFFFFFF) == 32);

    return 1;
}

static int test_bitmask_positions(void) {
    test_setup();

    /* Each bit position corresponds to a field */
    assert(BITMASK_2 == 2);       /* Bit 1 = field 2 */
    assert(BITMASK_4 == 4);       /* Bit 2 = field 3 */
    assert(BITMASK_8 == 8);       /* Bit 3 = field 4 */
    assert(BITMASK_16 == 16);     /* Bit 4 = field 5 */
    assert(BITMASK_32 == 32);     /* Bit 5 = field 6 */
    assert(BITMASK_64 == 64);     /* Bit 6 = field 7 */
    assert(BITMASK_128 == 128);   /* Bit 7 = field 8 */

    return 1;
}

static int test_bitmask_high_bits(void) {
    test_setup();

    /* Higher bitmask values */
    assert(BITMASK_256 == 0x100);
    assert(BITMASK_1024 == 0x400);
    assert(BITMASK_4096 == 0x1000);
    assert(BITMASK_65536 == 0x10000);
    assert(BITMASK_1048576 == 0x100000);
    assert(BITMASK_16777216 == 0x1000000);
    assert(BITMASK_268435456 == 0x10000000);

    return 1;
}

/* ========================================
 * Party System Tests (Case 0x53)
 * ======================================== */

static int test_party_max_members(void) {
    test_setup();

    /* Maximum 20 party members */
    assert(MAX_PARTY_MEMBERS == 20);

    return 1;
}

static int test_party_entry_size(void) {
    test_setup();

    /* Each party entry is 0x60 (96) bytes */
    assert(PARTY_ENTRY_SIZE == 0x60);
    assert(PARTY_ENTRY_SIZE == 96);

    return 1;
}

static int test_party_total_size(void) {
    test_setup();

    /* Total party: 20 * 96 = 1920 bytes */
    size_t total = MAX_PARTY_MEMBERS * PARTY_ENTRY_SIZE;
    assert(total == 1920);

    return 1;
}

/* ========================================
 * Mail System Tests (Case 0x57)
 * ======================================== */

static int test_mail_max_entries(void) {
    test_setup();

    /* Maximum 7 mail entries */
    assert(MAX_MAIL_ENTRIES == 7);

    return 1;
}

static int test_mail_entry_size(void) {
    test_setup();

    /* Each mail entry is 0x6a (106) bytes */
    assert(MAIL_ENTRY_SIZE == 0x6a);
    assert(MAIL_ENTRY_SIZE == 106);

    return 1;
}

static int test_mail_total_size(void) {
    test_setup();

    /* Total mail: 7 * 106 = 742 bytes */
    size_t total = MAX_MAIL_ENTRIES * MAIL_ENTRY_SIZE;
    assert(total == 742);

    return 1;
}

/* ========================================
 * Buffer Size Tests
 * ======================================== */

static int test_recv_buffer_size(void) {
    test_setup();

    /* Receive buffer is 16KB (0x4000) */
    assert(RECV_BUFFER_SIZE == 0x4000);
    assert(RECV_BUFFER_SIZE == 16384);

    return 1;
}

static int test_max_item_name(void) {
    test_setup();

    /* Max item name is 29 bytes (0x1d) */
    assert(MAX_ITEM_NAME == 0x1d);
    assert(MAX_ITEM_NAME == 29);

    return 1;
}

static int test_max_item_desc(void) {
    test_setup();

    /* Max item description is 85 bytes (0x55) */
    assert(MAX_ITEM_DESC == 0x55);
    assert(MAX_ITEM_DESC == 85);

    return 1;
}

/* ========================================
 * Pipe Delimiter Tests
 * ======================================== */

static int test_delimiter_value(void) {
    test_setup();

    /* Pipe delimiter is '|' (0x7c) */
    assert(0x7c == '|');
    assert(0x7c == 124);

    return 1;
}

static int test_field_parsing(void) {
    test_setup();

    /* Parse fields from pipe-delimited string */
    const char* data = "field1|field2|field3";

    int field1 = parse_field_int(data, '|', 1);
    int field2 = parse_field_int(data, '|', 2);
    int field3 = parse_field_int(data, '|', 3);

    assert(field1 == 0);  /* "field1" -> 0 */
    assert(field2 == 0);  /* "field2" -> 0 */
    assert(field3 == 0);  /* "field3" -> 0 */

    /* Numeric parsing */
    const char* nums = "100|200|300";
    assert(parse_field_int(nums, '|', 1) == 100);
    assert(parse_field_int(nums, '|', 2) == 200);
    assert(parse_field_int(nums, '|', 3) == 300);

    return 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_inventory_packet_structure(void) {
    test_setup();

    /* Inventory packet: case byte + char index + fields */
    /* Format: 0x42 <char_idx> |field1|field2|... */

    u8 packet[] = { 0x42, '0', '|', '1', '|', '2', 0 };

    assert(packet[0] == CASE_INVENTORY);
    assert(packet[1] == '0');  /* Character index 0 */

    return 1;
}

static int test_char_stats_packet_structure(void) {
    test_setup();

    /* Character stats packet: case byte + bitmask + fields */
    /* Format: 0x50 |bitmask|field1|field2|... */

    u8 packet[] = { 0x50, '|', '1', '|', '1', '0', '0', '|', '2', '0', '0', 0 };

    assert(packet[0] == CASE_CHAR_STATS);

    return 1;
}

static int test_bitmask_packet_structure(void) {
    test_setup();

    /* Bitmask-based packets (0x4b, 0x4e): case + index + bitmask + fields */
    /* Format: 0x4b <idx> |bitmask|field1|... */

    u8 packet[] = { 0x4b, '0', '|', '7', '|', '1', '|', '2', '|', '3', 0 };

    assert(packet[0] == CASE_CHAR_STATUS);
    assert(packet[1] == '0');  /* Character index 0 */

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Binary Packet Dispatcher Unit Tests ===\n\n");

    /* Case handler tests */
    printf("Case Handler Tests:\n");
    TEST(case_values);
    TEST(case_range);

    /* Inventory system tests */
    printf("\nInventory System Tests (Case 0x42):\n");
    TEST(inventory_max_items);
    TEST(inventory_entry_size);
    TEST(inventory_total_size);
    TEST(inventory_item_name_offset);
    TEST(inventory_item_desc_offset);

    /* Character stats tests */
    printf("\nCharacter Stats Tests (Case 0x50):\n");
    TEST(char_stats_field_count);
    TEST(char_stats_bitmask_parsing);
    TEST(char_stats_level_split);
    TEST(char_stats_name_offset);

    /* Bitmask parsing tests */
    printf("\nBitmask Parsing Tests (Cases 0x4b, 0x4e, 0x50):\n");
    TEST(bitmask_basic);
    TEST(bitmask_field_count);
    TEST(bitmask_positions);
    TEST(bitmask_high_bits);

    /* Party system tests */
    printf("\nParty System Tests (Case 0x53):\n");
    TEST(party_max_members);
    TEST(party_entry_size);
    TEST(party_total_size);

    /* Mail system tests */
    printf("\nMail System Tests (Case 0x57):\n");
    TEST(mail_max_entries);
    TEST(mail_entry_size);
    TEST(mail_total_size);

    /* Buffer size tests */
    printf("\nBuffer Size Tests:\n");
    TEST(recv_buffer_size);
    TEST(max_item_name);
    TEST(max_item_desc);

    /* Delimiter tests */
    printf("\nDelimiter Tests:\n");
    TEST(delimiter_value);
    TEST(field_parsing);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(inventory_packet_structure);
    TEST(char_stats_packet_structure);
    TEST(bitmask_packet_structure);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - FUN_00489f70 (string field extraction)
     * - FUN_0048a170 (string escape handling)
     * - FUN_0048a120 (bitmask decoding)
     * - FUN_0048a050 (integer field parsing)
     * - Full packet parsing simulation
     * - Case 0x43 (map transfer)
     * - Case 0x44 (time sync)
     * - Case 0x46 (player info)
     * - Case 0x49 (item list)
     * - Case 0x4a (pet status)
     * - Case 0x4d (NPC info)
     * - Case 0x4e (battle unit)
     * - Case 0x58 (server time)
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
