/*
 * Stone Age Client - Item Protocol Comprehensive Tests
 * Tests for FUN_0045ffb0 case 0x42 (inventory update packet parsing)
 *
 * Coverage:
 * - Packet structure validation
 * - Field parsing (14 fields per item)
 * - String field length limits
 * - Integer field parsing
 * - Level handling (>= 100 special case)
 * - Multi-character inventory
 * - Edge cases (empty slots, truncation)
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
typedef short s16;

/* Constants from binary analysis */
#define ITEM_SIZE           0x184   /* 388 bytes per item */
#define CHAR_INVENTORY_SIZE 0xb18   /* 2840 bytes per character inventory */
#define MAX_NAME_LEN        0x1d    /* 29 bytes max for item name */
#define MAX_SHORT_LEN       0x11    /* 17 bytes max for short strings */
#define MAX_DESC_LEN        0x55    /* 85 bytes max for description */
#define MAX_ITEMS_PER_CHAR  7       /* 7 items per character slot */
#define FIELDS_PER_ITEM     14      /* 14 fields per item */

/* Item structure offsets from binary */
#define ITEM_OFFSET_ACTIVE      0x00    /* Active flag */
#define ITEM_OFFSET_ID          0x00    /* Item ID (field 3) */
#define ITEM_OFFSET_FIELD5      0x04    /* Field 5 integer */
#define ITEM_OFFSET_COUNT       0xDE    /* Count (field 6) */
#define ITEM_OFFSET_LEVEL       0xE0    /* Level (field 7) */
#define ITEM_OFFSET_LEVEL_FLAG  0xE2    /* Level >= 100 flag */
#define ITEM_OFFSET_FIELD8      0x08    /* Field 8 integer */
#define ITEM_OFFSET_SPRITE      0xE4    /* Sprite ID (field 9) */
#define ITEM_OFFSET_NAME        0xE6    /* Name (field 1) - max 29 bytes */
#define ITEM_OFFSET_SHORT       0x103   /* Short string (field 2) - max 17 bytes */
#define ITEM_OFFSET_DESC        0x114   /* Description (field 4) - max 85 bytes */
#define ITEM_OFFSET_FIELD10     0x169   /* Field 10 string - max 17 bytes */
#define ITEM_OFFSET_FIELD11     0x0C    /* Field 11 integer */
#define ITEM_OFFSET_FIELD12     0x10    /* Field 12 string */
#define ITEM_OFFSET_FIELD13     0x79    /* Field 13 integer */
#define ITEM_OFFSET_FIELD14     0x7A    /* Field 14 string */

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

/* ========================================
 * Mock Item Structure
 * ======================================== */

typedef struct {
    u32 id;             /* Field 3: Item database ID */
    u32 field5;         /* Field 5: Unknown integer */
    u16 count;          /* Field 6: Item count */
    s16 level;          /* Field 7: Item level */
    u16 has_level;      /* Level flag (1 if level >= 100) */
    u32 field8;         /* Field 8: Unknown integer */
    u16 sprite_id;      /* Field 9: Sprite ID */
    char name[32];      /* Field 1: Item name (max 29) */
    char short_field[20]; /* Field 2: Short string (max 17) */
    char desc[88];      /* Field 4: Description (max 85) */
    char field10[20];   /* Field 10: String (max 17) */
    u32 field11;        /* Field 11: Integer */
    char field12[32];   /* Field 12: String */
    u32 field13;        /* Field 13: Integer */
    char field14[32];   /* Field 14: String */
    u8 active;          /* Active flag */
} ItemEntry;

/* Mock inventory for testing */
typedef struct {
    ItemEntry items[MAX_ITEMS_PER_CHAR];
    int count;
} Inventory;

static Inventory g_test_inventory;

/* ========================================
 * Field Parsing Functions (from binary)
 * ======================================== */

/*
 * Parse a pipe-delimited field - simplified FUN_00489f70
 * Returns field length, -1 on error
 */
static int parse_field(const char* data, char delimiter, int field_index, char* buffer, int max_len) {
    if (!data || !buffer || max_len <= 0) return -1;

    int current_field = 0;
    const char* start = data;
    const char* p = data;

    /* Find the requested field */
    while (*p && current_field < field_index) {
        if (*p == delimiter) {
            current_field++;
            if (current_field == field_index) {
                start = p + 1;
            }
        }
        p++;
    }

    if (current_field < field_index - 1) {
        buffer[0] = '\0';
        return 0;
    }

    /* Copy field content */
    int len = 0;
    while (*p && *p != delimiter && len < max_len - 1) {
        buffer[len++] = *p++;
    }
    buffer[len] = '\0';

    return len;
}

/*
 * Parse integer field - simplified FUN_0048a050
 */
static s32 parse_int_field(const char* data, char delimiter, int field_index) {
    char buffer[32];
    if (parse_field(data, delimiter, field_index, buffer, sizeof(buffer)) <= 0) {
        return 0;
    }
    return atoi(buffer);
}

/*
 * Unescape string - simplified FUN_0048a170
 * In the binary, this handles DBCS and escape sequences
 */
static void unescape_string(char* str) {
    if (!str) return;

    /* Simple pass-through for testing */
    /* TODO: Implement actual escape sequence handling */
    /* The binary handles sequences like \| \, etc. */
}

/*
 * Parse inventory update packet - FUN_0045ffb0 case 0x42
 * Packet format: 0x42<char_index>|field1|field2|...| (14 fields per item, 7 items)
 */
static int parse_inventory_packet(const char* packet, int char_index, Inventory* inv) {
    if (!packet || !inv) return -1;
    if (char_index < '0' || char_index > '9') return -1;

    /* Reset inventory */
    memset(inv, 0, sizeof(Inventory));

    /* Skip header bytes (0x42, char_index) - packet should start at field data */
    int field_index = 1;  /* Fields are 1-indexed in the binary */

    for (int item_idx = 0; item_idx < MAX_ITEMS_PER_CHAR; item_idx++) {
        ItemEntry* item = &inv->items[item_idx];

        /* Field 1: Item name (max 29 bytes) */
        char name_buffer[256];
        if (parse_field(packet, '|', field_index, name_buffer, sizeof(name_buffer)) < 0) {
            field_index += FIELDS_PER_ITEM;
            continue;
        }
        unescape_string(name_buffer);

        /* Check for empty slot (empty name) */
        if (strlen(name_buffer) == 0 || strlen(name_buffer) >= MAX_NAME_LEN) {
            item->active = 0;
            field_index += FIELDS_PER_ITEM;
            continue;
        }

        /* Store name with length check */
        size_t name_len = strlen(name_buffer);
        if (name_len < MAX_NAME_LEN) {
            strncpy(item->name, name_buffer, MAX_NAME_LEN - 1);
            item->name[MAX_NAME_LEN - 1] = '\0';
        }

        /* Field 2: Short string (max 17 bytes) */
        parse_field(packet, '|', field_index + 1, item->short_field, MAX_SHORT_LEN);
        unescape_string(item->short_field);

        /* Field 3: Item ID (integer) */
        item->id = parse_int_field(packet, '|', field_index + 2);
        if ((s32)item->id < 0) item->id = 0;

        /* Field 4: Description (max 85 bytes) */
        parse_field(packet, '|', field_index + 3, item->desc, MAX_DESC_LEN);
        unescape_string(item->desc);

        /* Field 5: Integer */
        item->field5 = parse_int_field(packet, '|', field_index + 4);

        /* Field 6: Count (u16) */
        s32 count = parse_int_field(packet, '|', field_index + 5);
        item->count = (count < 0) ? 0 : (u16)count;

        /* Field 7: Level (s16) - special handling for >= 100 */
        s32 level = parse_int_field(packet, '|', field_index + 6);
        if (level >= 100) {
            item->level = level % 100;
            item->has_level = 1;
        } else {
            item->level = (s16)level;
            item->has_level = 0;
        }

        /* Field 8: Integer */
        item->field8 = parse_int_field(packet, '|', field_index + 7);

        /* Field 9: Sprite ID (u16) */
        item->sprite_id = (u16)parse_int_field(packet, '|', field_index + 8);

        /* Field 10: String (max 17 bytes) */
        parse_field(packet, '|', field_index + 9, item->field10, MAX_SHORT_LEN);
        unescape_string(item->field10);

        /* Field 11: Integer */
        item->field11 = parse_int_field(packet, '|', field_index + 10);

        /* Field 12: String */
        parse_field(packet, '|', field_index + 11, item->field12, 32);
        unescape_string(item->field12);

        /* Field 13: Integer */
        item->field13 = parse_int_field(packet, '|', field_index + 12);

        /* Field 14: String */
        parse_field(packet, '|', field_index + 13, item->field14, 32);
        unescape_string(item->field14);

        item->active = 1;
        inv->count++;

        field_index += FIELDS_PER_ITEM;
    }

    return inv->count;
}

/* ========================================
 * Field Parsing Tests
 * ======================================== */

static int test_parse_field_simple(void) {
    const char* data = "field1|field2|field3";
    char buffer[64];

    int len = parse_field(data, '|', 1, buffer, sizeof(buffer));
    return len == 6 && strcmp(buffer, "field1") == 0;
}

static int test_parse_field_middle(void) {
    const char* data = "field1|field2|field3";
    char buffer[64];

    int len = parse_field(data, '|', 2, buffer, sizeof(buffer));
    return len == 6 && strcmp(buffer, "field2") == 0;
}

static int test_parse_field_last(void) {
    const char* data = "field1|field2|field3";
    char buffer[64];

    int len = parse_field(data, '|', 3, buffer, sizeof(buffer));
    return len == 6 && strcmp(buffer, "field3") == 0;
}

static int test_parse_field_empty(void) {
    const char* data = "field1||field3";
    char buffer[64];

    int len = parse_field(data, '|', 2, buffer, sizeof(buffer));
    return len == 0 && buffer[0] == '\0';
}

static int test_parse_field_truncate(void) {
    const char* data = "this_is_a_very_long_field_name";
    char buffer[16];

    int len = parse_field(data, '|', 1, buffer, sizeof(buffer));
    return len == 15 && strlen(buffer) == 15;
}

static int test_parse_int_positive(void) {
    const char* data = "item_name|123|456|789";
    return parse_int_field(data, '|', 2) == 123 &&
           parse_int_field(data, '|', 3) == 456 &&
           parse_int_field(data, '|', 4) == 789;
}

static int test_parse_int_negative(void) {
    const char* data = "item|-100|50";
    return parse_int_field(data, '|', 2) == -100;
}

static int test_parse_int_zero(void) {
    const char* data = "item|0|50";
    return parse_int_field(data, '|', 2) == 0;
}

/* ========================================
 * Item Structure Tests
 * ======================================== */

static int test_item_size_constant(void) {
    /* From binary: each item is 0x184 bytes */
    return ITEM_SIZE == 0x184 && ITEM_SIZE == 388;
}

static int test_inventory_size_constant(void) {
    /* From binary: each character inventory is 0xb18 bytes */
    return CHAR_INVENTORY_SIZE == 0xb18 && CHAR_INVENTORY_SIZE == 2840;
}

static int test_max_items_constant(void) {
    /* From binary: loop condition local_37c < 100 (field counter)
     * 7 items * 14 fields = 98 fields + 2 header fields = 100 */
    return MAX_ITEMS_PER_CHAR == 7 && FIELDS_PER_ITEM == 14;
}

static int test_name_offset(void) {
    /* From binary: DAT_046275ba = base + iVar19 + 0xE6
     * Item name stored at offset 0xE6 within item structure */
    return ITEM_OFFSET_NAME == 0xE6;
}

static int test_count_offset(void) {
    /* From binary: DAT_046275b2 = base + iVar19 + 0xDE
     * Item count stored at offset 0xDE within item structure */
    return ITEM_OFFSET_COUNT == 0xDE;
}

static int test_level_offset(void) {
    /* From binary: DAT_046275b4 = base + iVar19 + 0xE0
     * Item level stored at offset 0xE0 within item structure */
    return ITEM_OFFSET_LEVEL == 0xE0;
}

/* ========================================
 * Packet Parsing Tests
 * ======================================== */

static int test_parse_empty_packet(void) {
    Inventory inv;
    int result = parse_inventory_packet("", '0', &inv);
    return result == 0;
}

static int test_parse_single_item(void) {
    /* Simulated packet: 14 fields for one item */
    const char* packet = "Sword|Short|100|Sharp blade|5|1|10|0|50|Extra|0|Notes|0|End";
    Inventory inv;

    int result = parse_inventory_packet(packet, '0', &inv);

    return result == 1 &&
           inv.items[0].active == 1 &&
           strcmp(inv.items[0].name, "Sword") == 0 &&
           inv.items[0].id == 100 &&
           inv.items[0].count == 1 &&
           inv.items[0].level == 10 &&
           inv.items[0].sprite_id == 50;
}

static int test_parse_multiple_items(void) {
    /* 2 items = 28 fields */
    const char* packet = "Item1|S1|1|D1|1|5|10|0|1|E1|0|N1|0|X1|"
                        "Item2|S2|2|D2|2|10|20|0|2|E2|0|N2|0|X2";
    Inventory inv;

    int result = parse_inventory_packet(packet, '0', &inv);

    return result == 2 &&
           inv.items[0].active == 1 && strcmp(inv.items[0].name, "Item1") == 0 &&
           inv.items[1].active == 1 && strcmp(inv.items[1].name, "Item2") == 0;
}

static int test_parse_empty_slot(void) {
    /* Empty slot has empty name field */
    const char* packet = "|S1|1|D1|5|0|0|0|0|E1|0|N1|0|X1";
    Inventory inv;

    int result = parse_inventory_packet(packet, '0', &inv);

    return result == 0 && inv.items[0].active == 0;
}

static int test_parse_level_under_100(void) {
    const char* packet = "Item|S|1|D|0|1|50|0|1|E|0|N|0|X";
    Inventory inv;

    parse_inventory_packet(packet, '0', &inv);

    return inv.items[0].level == 50 && inv.items[0].has_level == 0;
}

static int test_parse_level_over_100(void) {
    /* Level >= 100: stored as level % 100, has_level = 1 */
    const char* packet = "Item|S|1|D|0|1|125|0|1|E|0|N|0|X";
    Inventory inv;

    parse_inventory_packet(packet, '0', &inv);

    return inv.items[0].level == 25 && inv.items[0].has_level == 1;
}

static int test_parse_level_exactly_100(void) {
    const char* packet = "Item|S|1|D|0|1|100|0|1|E|0|N|0|X";
    Inventory inv;

    parse_inventory_packet(packet, '0', &inv);

    return inv.items[0].level == 0 && inv.items[0].has_level == 1;
}

static int test_parse_count_zero(void) {
    const char* packet = "Item|S|1|D|0|0|0|0|1|E|0|N|0|X";
    Inventory inv;

    parse_inventory_packet(packet, '0', &inv);

    return inv.items[0].count == 0;
}

static int test_parse_count_max(void) {
    /* Max u16 count = 65535 */
    const char* packet = "Item|S|1|D|0|65535|0|0|1|E|0|N|0|X";
    Inventory inv;

    parse_inventory_packet(packet, '0', &inv);

    return inv.items[0].count == 65535;
}

/* ========================================
 * String Length Limit Tests
 * ======================================== */

static int test_name_max_length(void) {
    /* Max name length: 0x1d = 29 bytes */
    char long_name[64];
    memset(long_name, 'A', 40);
    long_name[40] = '\0';

    char packet[256];
    snprintf(packet, sizeof(packet), "%s|S|1|D|0|1|0|0|1|E|0|N|0|X", long_name);

    Inventory inv;
    parse_inventory_packet(packet, '0', &inv);

    /* Name >= MAX_NAME_LEN should be treated as empty slot */
    return inv.items[0].active == 0;
}

static int test_name_within_limit(void) {
    /* Name length 28 should work (within 29 limit) */
    char name[32];
    memset(name, 'B', 28);
    name[28] = '\0';

    char packet[128];
    snprintf(packet, sizeof(packet), "%s|S|1|D|0|1|0|0|1|E|0|N|0|X", name);

    Inventory inv;
    parse_inventory_packet(packet, '0', &inv);

    return inv.items[0].active == 1 && strlen(inv.items[0].name) == 28;
}

static int test_description_max_length(void) {
    /* Max desc length: 0x55 = 85 bytes */
    char long_desc[128];
    memset(long_desc, 'D', 100);
    long_desc[100] = '\0';

    char packet[256];
    snprintf(packet, sizeof(packet), "Item|S|1|%s|0|1|0|0|1|E|0|N|0|X", long_desc);

    Inventory inv;
    parse_inventory_packet(packet, '0', &inv);

    return strlen(inv.items[0].desc) < MAX_DESC_LEN;
}

/* ========================================
 * Character Index Tests
 * ======================================== */

static int test_char_index_0(void) {
    const char* packet = "Item|S|1|D|0|1|0|0|1|E|0|N|0|X";
    Inventory inv;

    int result = parse_inventory_packet(packet, '0', &inv);
    return result == 1;
}

static int test_char_index_9(void) {
    const char* packet = "Item|S|1|D|0|1|0|0|1|E|0|N|0|X";
    Inventory inv;

    int result = parse_inventory_packet(packet, '9', &inv);
    return result == 1;
}

static int test_char_index_invalid(void) {
    const char* packet = "Item|S|1|D|0|1|0|0|1|E|0|N|0|X";
    Inventory inv;

    int result = parse_inventory_packet(packet, 'A', &inv);
    return result == -1;
}

static int test_char_index_out_of_range(void) {
    const char* packet = "Item|S|1|D|0|1|0|0|1|E|0|N|0|X";
    Inventory inv;

    int result = parse_inventory_packet(packet, ':', &inv);  /* ':' is after '9' */
    return result == -1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_inventory_parse(void) {
    /* 7 items with all 14 fields each */
    const char* packet =
        "I1|S1|101|D1|1|1|10|0|51|E1|0|N1|0|X1|"
        "I2|S2|102|D2|2|2|20|0|52|E2|0|N2|0|X2|"
        "I3|S3|103|D3|3|3|30|0|53|E3|0|N3|0|X3|"
        "I4|S4|104|D4|4|4|40|0|54|E4|0|N4|0|X4|"
        "I5|S5|105|D5|5|5|50|0|55|E5|0|N5|0|X5|"
        "I6|S6|106|D6|6|6|60|0|56|E6|0|N6|0|X6|"
        "I7|S7|107|D7|7|7|70|0|57|E7|0|N7|0|X7";

    Inventory inv;
    int result = parse_inventory_packet(packet, '0', &inv);

    if (result != 7) return 0;

    /* Verify all items parsed correctly */
    for (int i = 0; i < 7; i++) {
        if (!inv.items[i].active) return 0;
        if (inv.items[i].id != 101 + i) return 0;
        if (inv.items[i].count != 1 + i) return 0;
        if (inv.items[i].level != (1 + i) * 10) return 0;
        if (inv.items[i].sprite_id != 51 + i) return 0;
    }

    return 1;
}

static int test_mixed_empty_slots(void) {
    /* Items at positions 0, 2, 4; empty at 1, 3 */
    const char* packet =
        "Item0|S|1|D|0|1|0|0|1|E|0|N|0|X|"
        "|S|0|D|0|0|0|0|0|E|0|N|0|X|"  /* Empty */
        "Item2|S|2|D|0|1|0|0|2|E|0|N|0|X|"
        "|S|0|D|0|0|0|0|0|E|0|N|0|X|"  /* Empty */
        "Item4|S|4|D|0|1|0|0|4|E|0|N|0|X";

    Inventory inv;
    int result = parse_inventory_packet(packet, '0', &inv);

    return result == 3 &&
           inv.items[0].active == 1 &&
           inv.items[1].active == 0 &&
           inv.items[2].active == 1 &&
           inv.items[3].active == 0 &&
           inv.items[4].active == 1;
}

static int test_negative_item_id(void) {
    /* Negative ID should be set to 0 */
    const char* packet = "Item|S|-1|D|0|1|0|0|1|E|0|N|0|X";
    Inventory inv;

    parse_inventory_packet(packet, '0', &inv);

    return inv.items[0].id == 0;
}

static int test_negative_count(void) {
    /* Negative count should be set to 0 */
    const char* packet = "Item|S|1|D|0|-5|0|0|1|E|0|N|0|X";
    Inventory inv;

    parse_inventory_packet(packet, '0', &inv);

    return inv.items[0].count == 0;
}

/* ========================================
 * Memory Layout Validation Tests
 * ======================================== */

static int test_inventory_base_address(void) {
    /* From binary: DAT_046274d4 is the base address for character 0 */
    /* Each character has 0xb18 bytes */
    u32 base_char0 = 0x046274d4;
    u32 base_char1 = base_char0 + CHAR_INVENTORY_SIZE;

    return base_char1 == 0x04627fec;
}

static int test_item_address_calculation(void) {
    /* From binary: DAT_046274d4 + (char_index - '0') * 0xb18 + item_index * 0x184 */
    u32 base = 0x046274d4;
    int char_index = '2' - '0';  /* Character 2 */
    int item_index = 3;          /* Item 3 */

    u32 addr = base + char_index * CHAR_INVENTORY_SIZE + item_index * ITEM_SIZE;

    /* Expected: 0x046274d4 + 2 * 0xb18 + 3 * 0x184 */
    u32 expected = 0x046274d4 + 0x1630 + 0x42c;

    return addr == expected;
}

static int test_fields_per_item_calculation(void) {
    /* From binary loop: local_37c starts at 2, increments by 14 per item
     * Loop continues while local_37c < 100
     * (100 - 2) / 14 = 7 items */
    int start_field = 2;
    int fields_per_item = 14;
    int max_fields = 100;

    int items = (max_fields - start_field) / fields_per_item;

    return items == MAX_ITEMS_PER_CHAR;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Item Protocol Comprehensive Tests ===\n\n");

    /* Field parsing tests */
    printf("Field Parsing Tests:\n");
    TEST(parse_field_simple);
    TEST(parse_field_middle);
    TEST(parse_field_last);
    TEST(parse_field_empty);
    TEST(parse_field_truncate);
    TEST(parse_int_positive);
    TEST(parse_int_negative);
    TEST(parse_int_zero);

    /* Item structure tests */
    printf("\nItem Structure Tests:\n");
    TEST(item_size_constant);
    TEST(inventory_size_constant);
    TEST(max_items_constant);
    TEST(name_offset);
    TEST(count_offset);
    TEST(level_offset);

    /* Packet parsing tests */
    printf("\nPacket Parsing Tests:\n");
    TEST(parse_empty_packet);
    TEST(parse_single_item);
    TEST(parse_multiple_items);
    TEST(parse_empty_slot);
    TEST(parse_level_under_100);
    TEST(parse_level_over_100);
    TEST(parse_level_exactly_100);
    TEST(parse_count_zero);
    TEST(parse_count_max);

    /* String length limit tests */
    printf("\nString Length Limit Tests:\n");
    TEST(name_max_length);
    TEST(name_within_limit);
    TEST(description_max_length);

    /* Character index tests */
    printf("\nCharacter Index Tests:\n");
    TEST(char_index_0);
    TEST(char_index_9);
    TEST(char_index_invalid);
    TEST(char_index_out_of_range);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_inventory_parse);
    TEST(mixed_empty_slots);
    TEST(negative_item_id);
    TEST(negative_count);

    /* Memory layout tests */
    printf("\nMemory Layout Tests:\n");
    TEST(inventory_base_address);
    TEST(item_address_calculation);
    TEST(fields_per_item_calculation);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - DBCS string handling in names
     * - Escape sequence parsing (|, commas)
     * - Real packet data from game capture
     * - Packet with pipe characters in strings
     * - Unicode/Shift-JIS character names
     * - Maximum packet length handling
     * - Malformed packet handling
     * - Field count mismatch
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
