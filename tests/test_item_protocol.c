/*
 * Stone Age Client - Item Protocol Unit Tests
 * Tests for FUN_0045ffb0 case 0x42 (inventory update)
 *
 * Based on Ghidra decompilation analysis:
 * - 7 items per character
 * - Each item: 0x184 (388) bytes
 * - Character inventory block: 0xb18 (2840) bytes
 * - Fields parsed with '|' delimiter (0x7c)
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

/* Constants from binary */
#define MAX_INVENTORY_SLOTS     7
#define ITEM_ENTRY_SIZE         0x184   /* 388 bytes */
#define CHAR_INVENTORY_SIZE     0xb18   /* 2840 bytes */
#define MAX_ITEM_NAME_LEN       0x1d    /* 29 bytes */
#define MAX_ITEM_DESC_LEN       0x55    /* 85 bytes */
#define MAX_ITEM_FIELD_LEN      0x11    /* 17 bytes */

/* Inventory item structure - matching binary layout */
#pragma pack(push, 1)
typedef struct {
    u32 field_0x00;
    u32 field_0x04;
    u32 field_0x08;
    u32 reserved[54];           /* 0x0c - 0xd8 */
    u16 active;                 /* 0xdc */
    u16 padding1;
    u32 field_0xe0;
    char name[MAX_ITEM_NAME_LEN];      /* 0xe6 */
    char short_field[MAX_ITEM_FIELD_LEN]; /* 0x103 */
    char description[MAX_ITEM_DESC_LEN];   /* 0x114 */
    u16 count;                  /* 0x122 */
    u16 level;                  /* 0x124 */
    u16 has_level;              /* 0x126 */
    u16 sprite_id;              /* 0x128 */
    char field_8[MAX_ITEM_FIELD_LEN];   /* 0x13d */
    u32 field_0x150;
    u32 field_0x154;
    char field_10[32];          /* 0x164 */
} InventoryItem;
#pragma pack(pop)

/* Character inventory array - matches DAT_046274d4 region */
static InventoryItem g_char_inventory[10][MAX_INVENTORY_SLOTS];
static int g_inventory_initialized = 0;

/* Field parsing state */
static char* g_field_ptr = NULL;
static int g_field_index = 0;

/* Stub field parser - FUN_00489f70 */
static void parse_field(const char* input, char delimiter, int field_num, int max_len, char* output) {
    const char* ptr = input;
    int current_field = 0;
    int i = 0;

    /* Skip to desired field */
    while (current_field < field_num && *ptr) {
        if (*ptr == delimiter) {
            current_field++;
        }
        ptr++;
    }

    /* Copy field content */
    while (*ptr && *ptr != delimiter && i < max_len - 1) {
        output[i++] = *ptr++;
    }
    output[i] = '\0';
}

/* Stub integer parser - FUN_0048a050 */
static s32 parse_int_field(const char* input, char delimiter, int field_num) {
    char buffer[32];
    parse_field(input, delimiter, field_num, sizeof(buffer), buffer);
    return (s32)strtol(buffer, NULL, 10);
}

/* Unescape string - FUN_0048a170 stub */
static void unescape_string(char* str) {
    char* read = str;
    char* write = str;

    while (*read) {
        if (*read == '\\' && *(read + 1)) {
            read++;
            switch (*read) {
                case 'n': *write++ = '\n'; break;
                case 'r': *write++ = '\r'; break;
                case 't': *write++ = '\t'; break;
                case '\\': *write++ = '\\'; break;
                case '|': *write++ = '|'; break;
                default: *write++ = *read; break;
            }
        } else {
            *write++ = *read;
        }
        read++;
    }
    *write = '\0';
}

/*
 * Parse inventory update packet - FUN_0045ffb0 case 0x42
 *
 * From Ghidra analysis:
 * - param_2[0] = 0x42 (command)
 * - param_2[1] = character index ('0'-'9')
 * - Following: 7 items, each with 14 fields separated by '|'
 */
static int parse_inventory_update(const char* packet) {
    char char_index;
    int item_idx;
    int field_base;
    char buffer[256];
    s32 item_id;
    s32 value;
    s32 count;
    s32 level;
    s32 sprite_id;
    int inventory_offset;

    /* Parse character index */
    char_index = packet[1];
    if (char_index < '0' || char_index > '9') {
        return -1;
    }
    char_index -= '0';

    /* Calculate base offset: (char_index - 0x30) * 0xb18 */
    inventory_offset = char_index * CHAR_INVENTORY_SIZE;

    /* Parse 7 items */
    for (item_idx = 0; item_idx < MAX_INVENTORY_SLOTS; item_idx++) {
        InventoryItem* item = &g_char_inventory[char_index][item_idx];
        field_base = item_idx * 14;  /* 14 fields per item */

        /* Field 1: Name (max 0x1d = 29) */
        parse_field(packet + 2, '|', field_base + 0, MAX_ITEM_NAME_LEN, buffer);
        unescape_string(buffer);

        if (buffer[0] == '\0') {
            /* Empty slot - clear item */
            memset(item, 0, sizeof(InventoryItem));
            continue;
        }

        /* Mark as active */
        item->active = 1;

        /* Copy name (max 0x1d = 29 bytes) */
        strncpy(item->name, buffer, MAX_ITEM_NAME_LEN - 1);
        item->name[MAX_ITEM_NAME_LEN - 1] = '\0';

        /* Field 2: Short field (max 0x11 = 17) */
        parse_field(packet + 2, '|', field_base + 1, MAX_ITEM_FIELD_LEN, buffer);
        unescape_string(buffer);
        strncpy(item->short_field, buffer, MAX_ITEM_FIELD_LEN - 1);

        /* Field 3: Item ID (integer) */
        item_id = parse_int_field(packet + 2, '|', field_base + 2);
        item->field_0x00 = item_id > 0 ? item_id : 0;

        /* Field 4: Description (max 0x55 = 85) */
        parse_field(packet + 2, '|', field_base + 3, MAX_ITEM_DESC_LEN, buffer);
        unescape_string(buffer);
        strncpy(item->description, buffer, MAX_ITEM_DESC_LEN - 1);

        /* Field 5: Unknown integer */
        item->field_0x04 = parse_int_field(packet + 2, '|', field_base + 4);

        /* Field 6: Count */
        count = parse_int_field(packet + 2, '|', field_base + 5);
        item->count = (u16)count;

        /* Field 7: Level */
        level = parse_int_field(packet + 2, '|', field_base + 6);
        if (level >= 100) {
            item->level = level % 100;
            item->has_level = 1;
        } else {
            item->level = (u16)level;
            item->has_level = 0;
        }

        /* Field 8: Unknown integer */
        item->field_0x08 = parse_int_field(packet + 2, '|', field_base + 7);

        /* Field 9: Sprite ID */
        sprite_id = parse_int_field(packet + 2, '|', field_base + 8);
        item->sprite_id = (u16)sprite_id;

        /* Field 10: String (max 0x11) */
        parse_field(packet + 2, '|', field_base + 9, MAX_ITEM_FIELD_LEN, buffer);
        unescape_string(buffer);
        strncpy(item->field_8, buffer, MAX_ITEM_FIELD_LEN - 1);

        /* Field 11: Integer */
        item->field_0x150 = parse_int_field(packet + 2, '|', field_base + 10);

        /* Field 12: String */
        parse_field(packet + 2, '|', field_base + 11, 32, buffer);
        unescape_string(buffer);
        strncpy(item->field_10, buffer, 31);

        /* Field 13: Unknown byte */
        value = parse_int_field(packet + 2, '|', field_base + 12);
        item->field_0x154 = value;

        /* Field 14: String */
        parse_field(packet + 2, '|', field_base + 13, 16, buffer);
        /* Stored at offset that would need additional fields */
    }

    return 0;
}

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
    memset(g_char_inventory, 0, sizeof(g_char_inventory));
    g_inventory_initialized = 1;
}

/* ========================================
 * Structure Size Tests
 * ======================================== */

static int test_item_structure_size(void) {
    /* From Ghidra: each item is 0x184 (388) bytes */
    assert(sizeof(InventoryItem) == ITEM_ENTRY_SIZE);
    return 1;
}

static int test_char_inventory_size(void) {
    /* 7 items * 388 bytes = 2716 bytes, close to 0xb18 (2840) */
    assert(MAX_INVENTORY_SLOTS * ITEM_ENTRY_SIZE <= CHAR_INVENTORY_SIZE);
    return 1;
}

/* ========================================
 * Field Offset Tests
 * ======================================== */

static int test_active_flag_offset(void) {
    /* From Ghidra: active at offset 0xdc */
    InventoryItem item;
    memset(&item, 0, sizeof(item));
    item.active = 1;

    /* Verify offset via pointer arithmetic */
    u8* base = (u8*)&item;
    u16* active_ptr = (u16*)(base + 0xdc);

    assert(*active_ptr == 1);
    assert(&item.active == (u16*)(base + 0xdc));
    return 1;
}

static int test_name_offset(void) {
    /* From Ghidra: name at offset 0xe6 */
    InventoryItem item;
    memset(&item, 0, sizeof(item));
    strcpy(item.name, "TestItem");

    u8* base = (u8*)&item;
    char* name_ptr = (char*)(base + 0xe6);

    assert(strcmp(name_ptr, "TestItem") == 0);
    return 1;
}

static int test_count_offset(void) {
    /* From Ghidra: count at offset 0x122 */
    InventoryItem item;
    memset(&item, 0, sizeof(item));
    item.count = 99;

    u8* base = (u8*)&item;
    u16* count_ptr = (u16*)(base + 0x122);

    assert(*count_ptr == 99);
    return 1;
}

static int test_level_offset(void) {
    /* From Ghidra: level at offset 0x124 */
    InventoryItem item;
    memset(&item, 0, sizeof(item));
    item.level = 50;
    item.has_level = 1;

    u8* base = (u8*)&item;
    u16* level_ptr = (u16*)(base + 0x124);
    u16* has_level_ptr = (u16*)(base + 0x126);

    assert(*level_ptr == 50);
    assert(*has_level_ptr == 1);
    return 1;
}

static int test_sprite_id_offset(void) {
    /* From Ghidra: sprite_id at offset 0x128 */
    InventoryItem item;
    memset(&item, 0, sizeof(item));
    item.sprite_id = 1234;

    u8* base = (u8*)&item;
    u16* sprite_ptr = (u16*)(base + 0x128);

    assert(*sprite_ptr == 1234);
    return 1;
}

/* ========================================
 * Packet Parsing Tests
 * ======================================== */

static int test_parse_single_item(void) {
    test_setup();

    /* Simulate packet: char '0', one item */
    const char* packet = "0|Sword|Type1|1000|A sharp blade|1|5|10|0|100|Field1|200|Data1|Memo";

    parse_inventory_update(packet);

    /* Verify item parsed */
    InventoryItem* item = &g_char_inventory[0][0];

    assert(item->active == 1);
    assert(strcmp(item->name, "Sword") == 0);
    assert(item->field_0x00 == 1000);  /* Item ID */
    assert(item->count == 5);
    assert(item->level == 10);
    assert(item->sprite_id == 100);

    return 1;
}

static int test_parse_empty_slot(void) {
    test_setup();

    /* Empty slot has empty name field */
    const char* packet = "0|||0||0|0|0|0|0||0|||";

    parse_inventory_update(packet);

    /* Slot should be cleared */
    InventoryItem* item = &g_char_inventory[0][0];
    assert(item->active == 0);

    return 1;
}

static int test_parse_level_over_100(void) {
    test_setup();

    /* Level 150 -> stored as 50, has_level = 1 */
    const char* packet = "0|Item|F|1|Desc|1|10|150|0|0||0|||";

    parse_inventory_update(packet);

    InventoryItem* item = &g_char_inventory[0][0];
    assert(item->level == 50);
    assert(item->has_level == 1);

    return 1;
}

static int test_parse_level_under_100(void) {
    test_setup();

    /* Level 50 -> stored as 50, has_level = 0 */
    const char* packet = "0|Item|F|1|Desc|1|10|50|0|0||0|||";

    parse_inventory_update(packet);

    InventoryItem* item = &g_char_inventory[0][0];
    assert(item->level == 50);
    assert(item->has_level == 0);

    return 1;
}

static int test_parse_escaped_name(void) {
    test_setup();

    /* Name with escaped characters */
    const char* packet = "0|Sword\\|Special|F|1|Desc|1|1|0|0|0||0|||";

    parse_inventory_update(packet);

    InventoryItem* item = &g_char_inventory[0][0];
    /* After unescape, | should be restored */
    assert(strstr(item->name, "Special") != NULL);

    return 1;
}

static int test_parse_max_name_length(void) {
    test_setup();

    /* Name exactly 29 characters */
    const char* packet = "0|12345678901234567890123456789|F|1|D|1|1|0|0|0||0|||";

    parse_inventory_update(packet);

    InventoryItem* item = &g_char_inventory[0][0];
    assert(strlen(item->name) == 29);

    return 1;
}

static int test_parse_truncate_long_name(void) {
    test_setup();

    /* Name longer than 29 characters - should be truncated */
    const char* packet = "0|12345678901234567890123456789012345|F|1|D|1|1|0|0|0||0|||";

    parse_inventory_update(packet);

    InventoryItem* item = &g_char_inventory[0][0];
    assert(strlen(item->name) < 30);
    assert(item->name[MAX_ITEM_NAME_LEN - 1] == '\0');

    return 1;
}

/* ========================================
 * Character Index Tests
 * ======================================== */

static int test_char_index_0(void) {
    test_setup();

    const char* packet = "0|Item|F|100|D|1|1|0|0|0||0|||";
    parse_inventory_update(packet);

    /* Should be in first character's inventory */
    assert(g_char_inventory[0][0].field_0x00 == 100);
    /* Other characters should be empty */
    assert(g_char_inventory[1][0].active == 0);

    return 1;
}

static int test_char_index_5(void) {
    test_setup();

    const char* packet = "5|Item|F|200|D|1|1|0|0|0||0|||";
    parse_inventory_update(packet);

    /* Should be in character 5's inventory */
    assert(g_char_inventory[5][0].field_0x00 == 200);
    /* Character 0 should be empty */
    assert(g_char_inventory[0][0].active == 0);

    return 1;
}

/* ========================================
 * Field Parser Tests
 * ======================================== */

static int test_field_parser_basic(void) {
    const char* input = "field1|field2|field3";
    char output[32];

    parse_field(input, '|', 0, 32, output);
    assert(strcmp(output, "field1") == 0);

    parse_field(input, '|', 1, 32, output);
    assert(strcmp(output, "field2") == 0);

    parse_field(input, '|', 2, 32, output);
    assert(strcmp(output, "field3") == 0);

    return 1;
}

static int test_field_parser_max_length(void) {
    const char* input = "12345678901234567890";
    char output[10];

    parse_field(input, '|', 0, 10, output);
    assert(strlen(output) == 9);  /* max_len - 1 */

    return 1;
}

static int test_int_parser(void) {
    const char* input = "abc|123|456|789";

    assert(parse_int_field(input, '|', 1) == 123);
    assert(parse_int_field(input, '|', 2) == 456);
    assert(parse_int_field(input, '|', 3) == 789);

    return 1;
}

static int test_unescape_string(void) {
    char str[32];

    strcpy(str, "hello\\nworld");
    unescape_string(str);
    assert(strcmp(str, "hello\nworld") == 0);

    strcpy(str, "a\\|b");
    unescape_string(str);
    assert(strcmp(str, "a|b") == 0);

    strcpy(str, "test\\\\slash");
    unescape_string(str);
    assert(strcmp(str, "test\\slash") == 0);

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Item Protocol Unit Tests ===\n\n");

    /* Structure tests */
    printf("Structure Size Tests:\n");
    TEST(item_structure_size);
    TEST(char_inventory_size);

    /* Offset tests */
    printf("\nField Offset Tests:\n");
    TEST(active_flag_offset);
    TEST(name_offset);
    TEST(count_offset);
    TEST(level_offset);
    TEST(sprite_id_offset);

    /* Parsing tests */
    printf("\nPacket Parsing Tests:\n");
    TEST(parse_single_item);
    TEST(parse_empty_slot);
    TEST(parse_level_over_100);
    TEST(parse_level_under_100);
    TEST(parse_escaped_name);
    TEST(parse_max_name_length);
    TEST(parse_truncate_long_name);

    /* Character index tests */
    printf("\nCharacter Index Tests:\n");
    TEST(char_index_0);
    TEST(char_index_5);

    /* Field parser tests */
    printf("\nField Parser Tests:\n");
    TEST(field_parser_basic);
    TEST(field_parser_max_length);
    TEST(int_parser);
    TEST(unescape_string);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
