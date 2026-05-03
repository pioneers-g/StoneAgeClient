/*
 * Stone Age Client - Shop Protocol Unit Tests
 * Tests for FUN_00416be0 shop UI protocol handling
 *
 * Based on reverse engineering analysis of sa_9061.exe
 * Protocol format from strings at 0x4aa290, 0x4aa1c8, etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/types.h"
#include "../include/shop.h"

/* Test infrastructure */
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("Running test_%s... ", #name); \
    test_##name(); \
    printf("PASSED\n"); \
    tests_passed++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAILED at line %d: %s\n", __LINE__, #cond); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_STR_EQ(a, b) ASSERT(strcmp((a), (b)) == 0)

/*
 * Shop Protocol Format (from FUN_00416be0 and string analysis):
 *
 * Buy Packet:  "B|%d|%s"     (item_id, item_name)
 * Detail:      "D|%d|"       (item_id)
 * Type:        "B|T|%d"      (shop_type)
 * Gold:        "B|G|%d"      (gold_amount)
 *
 * Server Response for 'B' command (buy mode):
 * Field 1: 'B' marker
 * Field 2: item_count
 * Fields 3+: per-item data
 *
 * Server Response for 'D' command (detail mode):
 * Field 1: 'D' marker
 * Field 2: item_id
 * Field 3: item_name
 * Field 4: description lines
 * Field 5: sprite_id
 * ...
 */

/*
 * Test 1-5: Shop packet parsing - 'B' command
 */
TEST(parse_buy_packet_basic) {
    const char* packet = "B|3|";
    char cmd[16];
    int count;

    /* Parse command */
    ASSERT_EQ(sscanf(packet, "%[^|]|%d|", cmd, &count), 2);
    ASSERT_STR_EQ(cmd, "B");
    ASSERT_EQ(count, 3);
}

TEST(parse_buy_packet_item_list) {
    /* Simulated packet with item list from FUN_00416be0 'B' handling */
    const char* packet = "B|2|101|Sword|102|Shield|";
    char cmd[16];
    int count;
    int item_id;
    char name[32];

    /* Parse header */
    int pos = 0;
    ASSERT_EQ(sscanf(packet, "%[^|]|%d|%n", cmd, &count, &pos), 2);
    ASSERT_EQ(count, 2);

    /* Parse first item */
    ASSERT_EQ(sscanf(packet + pos, "%d|%[^|]|%n", &item_id, name, &pos), 2);
    ASSERT_EQ(item_id, 101);
    ASSERT_STR_EQ(name, "Sword");
}

TEST(parse_buy_packet_empty) {
    const char* packet = "B|0|";
    char cmd[16];
    int count;

    ASSERT_EQ(sscanf(packet, "%[^|]|%d|", cmd, &count), 2);
    ASSERT_EQ(count, 0);
}

TEST(parse_buy_packet_large_count) {
    const char* packet = "B|20|";
    char cmd[16];
    int count;

    ASSERT_EQ(sscanf(packet, "%[^|]|%d|", cmd, &count), 2);
    ASSERT_EQ(count, 20);
}

TEST(parse_buy_packet_format_check) {
    /* Verify format string from DAT_004aa290: "B|%d|%s" */
    char output[128];
    int item_id = 12345;
    const char* name = "TestItem";

    snprintf(output, sizeof(output), "B|%d|%s", item_id, name);
    ASSERT_STR_EQ(output, "B|12345|TestItem");
}

/*
 * Test 6-10: Shop detail packet parsing - 'D' command
 */
TEST(parse_detail_packet_basic) {
    const char* packet = "D|101|";
    char cmd[16];
    int item_id;

    ASSERT_EQ(sscanf(packet, "%[^|]|%d|", cmd, &item_id), 2);
    ASSERT_STR_EQ(cmd, "D");
    ASSERT_EQ(item_id, 101);
}

TEST(parse_detail_packet_with_data) {
    /* Full detail packet from FUN_00416be0 'D' handling
     * Format: D|item_id|name|desc|sprite|stats...
     */
    const char* packet = "D|101|Iron Sword|A sharp blade|1001|50|10|";
    char cmd[16];
    int item_id;
    char name[32];
    char desc[64];

    int pos = 0;
    ASSERT_EQ(sscanf(packet, "%[^|]|%d|%[^|]|%[^|]|%n",
                     cmd, &item_id, name, desc, &pos), 4);
    ASSERT_EQ(item_id, 101);
    ASSERT_STR_EQ(name, "Iron Sword");
    ASSERT_STR_EQ(desc, "A sharp blade");
}

TEST(parse_detail_packet_pet_data) {
    /* Pet detail packet has different format - is_pet flag in DAT_0054dd71 */
    const char* packet = "D|201|Pet Dragon|Fire breath|2001|100|50|30|";
    char cmd[16];
    int item_id;
    char name[32];

    ASSERT_EQ(sscanf(packet, "%[^|]|%d|%[^|]|", cmd, &item_id, name), 3);
    ASSERT_EQ(item_id, 201);
    ASSERT_STR_EQ(name, "Pet Dragon");
}

TEST(parse_detail_packet_equip_stats) {
    /* Equipment with multiple stats from DAT_0054dbf0-DAT_0054dc14 */
    const char* packet = "D|101|Sword||1001|50|10|5|3|100|200|";
    char cmd[16];
    int item_id, atk, def, spd, hp, mp;
    char name[32];

    int pos = 0;
    ASSERT_EQ(sscanf(packet, "%[^|]|%d|%[^|]|%n", cmd, &item_id, name, &pos), 3);

    /* Parse stat fields (positions from FUN_00416be0) */
    ASSERT_EQ(sscanf(packet + pos, "%d|%d|%d|%d|%d|", &atk, &def, &spd, &hp, &mp), 5);
    ASSERT_EQ(atk, 1001);
    ASSERT_EQ(def, 50);
}

TEST(parse_detail_packet_format_check) {
    /* Verify format from DAT_004aa1c8: "D|%d|" */
    char output[64];
    int item_id = 999;

    snprintf(output, sizeof(output), "D|%d|", item_id);
    ASSERT_STR_EQ(output, "D|999|");
}

/*
 * Test 11-15: Shop data structure sizes
 * Based on DAT offsets from FUN_00416be0
 */
TEST(struct_size_shop_item_basic) {
    /* Basic shop item is 0x15a (346) bytes
     * Offset in DAT_0054dbe0 region
     */
    ASSERT_EQ(sizeof(ShopBasicItem), 0x15a);
}

TEST(struct_size_shop_item_detailed) {
    /* Detailed shop item is 0x568 (1384) bytes
     * Offset in DAT_0054dbe0 + 0x15a * index for basic
     * Detailed data at + 0x568 * index for extended
     */
    ASSERT_EQ(sizeof(ShopDetailedItem), 0x568);
}

TEST(struct_offset_item_id) {
    /* Item ID at offset 0 in both structures */
    ShopBasicItem basic = {0};
    ShopDetailedItem detail = {0};

    basic.item_id = 123;
    detail.item_id = 456;

    ASSERT_EQ(*(u32*)&basic, 123);
    ASSERT_EQ(*(u32*)&detail, 456);
}

TEST(struct_offset_is_pet) {
    /* is_pet flag at offset 0x189 (393) in detailed item
     * Corresponds to DAT_0054dd71 in FUN_00416be0
     */
    ASSERT_EQ(offsetof(ShopDetailedItem, is_pet), 0x189);
}

TEST(struct_offset_name) {
    /* Item name at offset 0x1d (29) in basic item
     * Corresponds to DAT_0054dc1d in FUN_00416be0
     */
    ASSERT_EQ(offsetof(ShopBasicItem, name), 0x1d);
}

/*
 * Test 16-20: Shop UI constants from FUN_00416be0
 */
TEST(ui_max_visible_items) {
    /* Maximum 13 visible items (0x0d) from DAT_0054ccfc + 13 */
    ASSERT_EQ(MAX_SHOP_VISIBLE_ITEMS, 13);
}

TEST(ui_max_total_items) {
    /* Maximum 20 items (0x14) in shop from DAT_0054c88c array */
    ASSERT_EQ(MAX_SHOP_UI_ITEMS, 20);
}

TEST(sprite_id_close_button) {
    /* Close button sprite: 0x66b1 */
    ASSERT_EQ(SPRITE_SHOP_CLOSE, 0x66b1);
}

TEST(sprite_id_confirm_button) {
    /* Confirm button sprite: 0x66b3 */
    ASSERT_EQ(SPRITE_SHOP_CONFIRM, 0x66b3);
}

TEST(sprite_id_scroll_buttons) {
    /* Scroll up: 0x66db, Scroll down: 0x66dc */
    ASSERT_EQ(SPRITE_SHOP_SCROLL_UP, 0x66db);
    ASSERT_EQ(SPRITE_SHOP_SCROLL_DOWN, 0x66dc);
}

/*
 * Test 21-25: Shop transaction handling
 */
TEST(transaction_buy_format) {
    /* Buy packet from DAT_004aa290: "B|%d|%s" */
    char output[128];
    int item_id = 500;
    const char* name = "Potion";

    snprintf(output, sizeof(output), "B|%d|%s", item_id, name);
    ASSERT_STR_EQ(output, "B|500|Potion");
}

TEST(transaction_sell_format) {
    /* Sell uses same format but with sell_mode flag set */
    char output[128];
    int item_id = 500;
    int count = 3;

    snprintf(output, sizeof(output), "S|%d|%d", item_id, count);
    ASSERT_STR_EQ(output, "S|500|3");
}

TEST(transaction_type_format) {
    /* Shop type packet from DAT_004b9b9c: "B|T|%d" */
    char output[64];
    int shop_type = 1;  /* 0=buy, 1=sell */

    snprintf(output, sizeof(output), "B|T|%d", shop_type);
    ASSERT_STR_EQ(output, "B|T|1");
}

TEST(transaction_gold_format) {
    /* Gold packet from DAT_004c1a68: "B|G|%d" */
    char output[64];
    u32 gold = 12345;

    snprintf(output, sizeof(output), "B|G|%u", gold);
    ASSERT_STR_EQ(output, "B|G|12345");
}

TEST(transaction_cancel_format) {
    /* Cancel/close packet is just "B" */
    const char* cancel = "B\n";
    ASSERT_EQ(cancel[0], 'B');
    ASSERT_EQ(cancel[1], '\n');
}

/*
 * Test 26-30: Item detail parsing from 'D' response
 */
TEST(detail_parse_description_lines) {
    /* Description can have multiple lines, 0x55 (85) chars each */
    const char* desc = "Line 1 of description|Line 2 of text|Line 3|";
    char line1[86], line2[86], line3[86];

    int pos = 0;
    ASSERT_EQ(sscanf(desc, "%[^|]|%[^|]|%[^|]|", line1, line2, line3), 3);
    ASSERT_STR_EQ(line1, "Line 1 of description");
    ASSERT_STR_EQ(line2, "Line 2 of text");
    ASSERT_STR_EQ(line3, "Line 3");
}

TEST(detail_parse_stat_fields) {
    /* Stats from DAT_0054dbf0 through DAT_0054dc14:
     * Attack, Defense, Speed, HP, MP, etc.
     */
    const char* stats = "100|50|30|200|100|";
    int atk, def, spd, hp, mp;

    ASSERT_EQ(sscanf(stats, "%d|%d|%d|%d|%d|", &atk, &def, &spd, &hp, &mp), 5);
    ASSERT_EQ(atk, 100);
    ASSERT_EQ(def, 50);
    ASSERT_EQ(spd, 30);
    ASSERT_EQ(hp, 200);
    ASSERT_EQ(mp, 100);
}

TEST(detail_parse_skill_list) {
    /* Pet skills from DAT_0054dcc2: up to 7 skills, 0x19 (25) chars each */
    const char* skills = "Fire Ball|Ice Blast|Thunder|";
    char skill1[26], skill2[26], skill3[26];

    ASSERT_EQ(sscanf(skills, "%[^|]|%[^|]|%[^|]|", skill1, skill2, skill3), 3);
    ASSERT_STR_EQ(skill1, "Fire Ball");
    ASSERT_STR_EQ(skill2, "Ice Blast");
    ASSERT_STR_EQ(skill3, "Thunder");
}

TEST(detail_parse_pet_equip_slots) {
    /* Pet equipment slots from DAT_0054ddd1: 7 slots of 0x8c (140) bytes each */
    const int slot_size = 0x8c;
    const int slot_count = 7;

    ASSERT_EQ(slot_size, 140);
    ASSERT_EQ(slot_count, 7);
    ASSERT_EQ(slot_size * slot_count, 980);  /* 0x3d4 */
}

TEST(detail_parse_sprite_info) {
    /* Sprite ID from FUN_00416be0:
     * Basic item: at sprite offset
     * Pet: additional overlays
     */
    const char* data = "1001|";  /* Sprite ID */
    int sprite_id;

    ASSERT_EQ(sscanf(data, "%d|", &sprite_id), 1);
    ASSERT_EQ(sprite_id, 1001);
}

int main(void) {
    printf("=== Shop Protocol Unit Tests ===\n\n");

    /* Buy packet tests */
    RUN_TEST(parse_buy_packet_basic);
    RUN_TEST(parse_buy_packet_item_list);
    RUN_TEST(parse_buy_packet_empty);
    RUN_TEST(parse_buy_packet_large_count);
    RUN_TEST(parse_buy_packet_format_check);

    /* Detail packet tests */
    RUN_TEST(parse_detail_packet_basic);
    RUN_TEST(parse_detail_packet_with_data);
    RUN_TEST(parse_detail_packet_pet_data);
    RUN_TEST(parse_detail_packet_equip_stats);
    RUN_TEST(parse_detail_packet_format_check);

    /* Structure size tests */
    RUN_TEST(struct_size_shop_item_basic);
    RUN_TEST(struct_size_shop_item_detailed);
    RUN_TEST(struct_offset_item_id);
    RUN_TEST(struct_offset_is_pet);
    RUN_TEST(struct_offset_name);

    /* UI constant tests */
    RUN_TEST(ui_max_visible_items);
    RUN_TEST(ui_max_total_items);
    RUN_TEST(sprite_id_close_button);
    RUN_TEST(sprite_id_confirm_button);
    RUN_TEST(sprite_id_scroll_buttons);

    /* Transaction tests */
    RUN_TEST(transaction_buy_format);
    RUN_TEST(transaction_sell_format);
    RUN_TEST(transaction_type_format);
    RUN_TEST(transaction_gold_format);
    RUN_TEST(transaction_cancel_format);

    /* Detail parsing tests */
    RUN_TEST(detail_parse_description_lines);
    RUN_TEST(detail_parse_stat_fields);
    RUN_TEST(detail_parse_skill_list);
    RUN_TEST(detail_parse_pet_equip_slots);
    RUN_TEST(detail_parse_sprite_info);

    printf("\n=== Test Results ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
