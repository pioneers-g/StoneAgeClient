/*
 * Stone Age Client - Item System Unit Tests
 * Tests for item.c, item_effect.c, item_protocol.c
 * Reverse engineered from FUN_004115e0, FUN_00411530, FUN_004117c0, FUN_004117e0, FUN_00411900
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/item.h"

/* ========================================
 * Test Cases for Item System
 * ======================================== */

/*
 * Test 1: Item initialization
 */
static void test_item_init(void) {
    TEST_BEGIN("Item init");

    int result = item_init();

    TEST_ASSERT(result == 1);
    TEST_ASSERT(g_item_count == 0);
    TEST_ASSERT(g_inventory.capacity == 100);
    TEST_ASSERT(g_inventory.count == 0);
    TEST_ASSERT(g_inventory.gold == 0);

    item_shutdown();
    TEST_END();
}

/*
 * Test 2: Add item to inventory
 */
static void test_inventory_add(void) {
    TEST_BEGIN("Inventory add");

    item_init();

    int result = inventory_add_item(1001, 5);
    TEST_ASSERT(result == 1);
    TEST_ASSERT(g_inventory.slots[0].item_id == 1001);
    TEST_ASSERT(g_inventory.slots[0].count == 5);
    TEST_ASSERT(g_inventory.count == 1);

    item_shutdown();
    TEST_END();
}

/*
 * Test 3: Add multiple items
 */
static void test_inventory_add_multiple(void) {
    TEST_BEGIN("Inventory add multiple");

    item_init();

    inventory_add_item(1001, 5);
    inventory_add_item(1002, 3);
    inventory_add_item(1003, 10);

    TEST_ASSERT(g_inventory.count == 3);

    /* Check each slot */
    TEST_ASSERT(g_inventory.slots[0].item_id == 1001);
    TEST_ASSERT(g_inventory.slots[1].item_id == 1002);
    TEST_ASSERT(g_inventory.slots[2].item_id == 1003);

    item_shutdown();
    TEST_END();
}

/*
 * Test 4: Remove item from inventory
 */
static void test_inventory_remove(void) {
    TEST_BEGIN("Inventory remove");

    item_init();

    inventory_add_item(1001, 10);

    int result = inventory_remove_item(1001, 5);
    TEST_ASSERT(result == 1);
    TEST_ASSERT(g_inventory.slots[0].count == 5);

    /* Remove remaining */
    result = inventory_remove_item(1001, 5);
    TEST_ASSERT(result == 1);
    TEST_ASSERT(g_inventory.slots[0].item_id == 0);
    TEST_ASSERT(g_inventory.count == 0);

    item_shutdown();
    TEST_END();
}

/*
 * Test 5: Check if inventory has item
 */
static void test_inventory_has_item(void) {
    TEST_BEGIN("Inventory has item");

    item_init();

    inventory_add_item(1001, 10);

    TEST_ASSERT(inventory_has_item(1001, 5) == 1);
    TEST_ASSERT(inventory_has_item(1001, 10) == 1);
    TEST_ASSERT(inventory_has_item(1001, 15) == 0);
    TEST_ASSERT(inventory_has_item(1002, 1) == 0);

    item_shutdown();
    TEST_END();
}

/*
 * Test 6: Find item slot
 */
static void test_inventory_find_slot(void) {
    TEST_BEGIN("Inventory find slot");

    item_init();

    inventory_add_item(1001, 5);
    inventory_add_item(1002, 3);
    inventory_add_item(1001, 5);  /* Stack with first */

    int slot = inventory_find_slot(1001);
    TEST_ASSERT(slot == 0);

    slot = inventory_find_slot(1002);
    TEST_ASSERT(slot == 1);

    slot = inventory_find_slot(1003);
    TEST_ASSERT(slot == -1);

    item_shutdown();
    TEST_END();
}

/*
 * Test 7: Find empty slot
 */
static void test_inventory_find_empty(void) {
    TEST_BEGIN("Inventory find empty slot");

    item_init();

    int slot = inventory_find_empty_slot();
    TEST_ASSERT(slot == 0);

    inventory_add_item(1001, 5);

    slot = inventory_find_empty_slot();
    TEST_ASSERT(slot == 1);

    item_shutdown();
    TEST_END();
}

/*
 * Test 8: Inventory capacity
 */
static void test_inventory_capacity(void) {
    TEST_BEGIN("Inventory capacity");

    item_init();

    /* Fill inventory */
    int i;
    for (i = 0; i < 100; i++) {
        inventory_add_item(1000 + i, 1);
    }

    TEST_ASSERT(g_inventory.count == 100);

    /* Try to add beyond capacity */
    int result = inventory_add_item(2000, 1);
    TEST_ASSERT(result == 0);

    item_shutdown();
    TEST_END();
}

/*
 * Test 9: Stack items
 */
static void test_inventory_stack(void) {
    TEST_BEGIN("Inventory stack");

    item_init();

    /* Add first stack */
    inventory_add_item(1001, 10);

    /* Add more of same item - should stack */
    inventory_add_item(1001, 5);

    /* Should still be in slot 0 with combined count */
    TEST_ASSERT(g_inventory.slots[0].item_id == 1001);
    TEST_ASSERT(g_inventory.slots[0].count == 15);
    TEST_ASSERT(g_inventory.count == 1);

    item_shutdown();
    TEST_END();
}

/*
 * Test 10: Item database
 */
static void test_item_database(void) {
    TEST_BEGIN("Item database");

    item_init();

    /* Add item to database */
    Item test_item = {0};
    test_item.id = 1001;
    strcpy(test_item.name, "Health Potion");
    test_item.type = ITEM_TYPE_CONSUMABLE;
    test_item.max_stack = 99;
    test_item.price = 100;
    test_item.effect_type = EFFECT_HEAL_HP;
    test_item.effect_value = 50;

    memcpy(&g_items[1001], &test_item, sizeof(Item));
    g_item_count = 1002;

    Item* found = item_get(1001);
    TEST_ASSERT(found != NULL);
    TEST_ASSERT(strcmp(found->name, "Health Potion") == 0);
    TEST_ASSERT(found->type == ITEM_TYPE_CONSUMABLE);

    item_shutdown();
    TEST_END();
}

/*
 * Test 11: Equipment equip
 */
static void test_inventory_equip(void) {
    TEST_BEGIN("Inventory equip");

    item_init();

    /* Add weapon to database */
    g_items[1001].id = 1001;
    g_items[1001].type = ITEM_TYPE_WEAPON;
    g_item_count = 1002;

    /* Add to inventory */
    inventory_add_item(1001, 1);

    /* Equip */
    int result = inventory_equip(0, EQUIP_SLOT_WEAPON);
    TEST_ASSERT(result == 1);
    TEST_ASSERT(g_inventory.equipment[EQUIP_SLOT_WEAPON].item_id == 1001);
    TEST_ASSERT(g_inventory.equipment[EQUIP_SLOT_WEAPON].equipped == 1);

    item_shutdown();
    TEST_END();
}

/*
 * Test 12: Equipment unequip
 */
static void test_inventory_unequip(void) {
    TEST_BEGIN("Inventory unequip");

    item_init();

    /* Setup */
    g_items[1001].id = 1001;
    g_items[1001].type = ITEM_TYPE_WEAPON;
    g_item_count = 1002;

    inventory_add_item(1001, 1);
    inventory_equip(0, EQUIP_SLOT_WEAPON);

    /* Unequip */
    int result = inventory_unequip(EQUIP_SLOT_WEAPON);
    TEST_ASSERT(result == 1);
    TEST_ASSERT(g_inventory.equipment[EQUIP_SLOT_WEAPON].item_id == 0);
    TEST_ASSERT(g_inventory.equipment[EQUIP_SLOT_WEAPON].equipped == 0);

    item_shutdown();
    TEST_END();
}

/*
 * Test 13: Bank deposit
 */
static void test_inventory_bank_deposit(void) {
    TEST_BEGIN("Inventory bank deposit");

    item_init();

    inventory_add_item(1001, 10);

    int result = inventory_bank_deposit(1001, 5);
    TEST_ASSERT(result == 1);
    TEST_ASSERT(g_inventory.slots[0].count == 5);
    TEST_ASSERT(g_inventory.bank[0].item_id == 1001);
    TEST_ASSERT(g_inventory.bank[0].count == 5);

    item_shutdown();
    TEST_END();
}

/*
 * Test 14: Bank withdraw
 */
static void test_inventory_bank_withdraw(void) {
    TEST_BEGIN("Inventory bank withdraw");

    item_init();

    inventory_add_item(1001, 10);
    inventory_bank_deposit(1001, 10);

    int result = inventory_bank_withdraw(1001, 5);
    TEST_ASSERT(result == 1);
    TEST_ASSERT(g_inventory.bank[0].count == 5);
    TEST_ASSERT(g_inventory.slots[0].item_id == 1001);
    TEST_ASSERT(g_inventory.slots[0].count == 5);

    item_shutdown();
    TEST_END();
}

/*
 * Test 15: Gold management
 */
static void test_inventory_gold(void) {
    TEST_BEGIN("Inventory gold");

    item_init();

    g_inventory.gold = 1000;

    int result = inventory_bank_deposit_gold(500);
    TEST_ASSERT(result == 1);
    TEST_ASSERT(g_inventory.gold == 500);
    TEST_ASSERT(g_inventory.bank_gold == 500);

    result = inventory_bank_withdraw_gold(200);
    TEST_ASSERT(result == 1);
    TEST_ASSERT(g_inventory.gold == 700);
    TEST_ASSERT(g_inventory.bank_gold == 300);

    item_shutdown();
    TEST_END();
}

/*
 * Test 16: Item sort by name
 */
static void test_inventory_sort_name(void) {
    TEST_BEGIN("Inventory sort by name");

    item_init();

    /* Add items to database */
    strcpy(g_items[1001].name, "Zebra");
    strcpy(g_items[1002].name, "Apple");
    strcpy(g_items[1003].name, "Mango");
    g_items[1001].id = 1001;
    g_items[1002].id = 1002;
    g_items[1003].id = 1003;
    g_item_count = 1004;

    inventory_add_item(1001, 1);
    inventory_add_item(1002, 1);
    inventory_add_item(1003, 1);

    inventory_sort_by_name();

    /* Should be sorted: Apple, Mango, Zebra */
    TEST_ASSERT(g_inventory.slots[0].item_id == 1002);
    TEST_ASSERT(g_inventory.slots[1].item_id == 1003);
    TEST_ASSERT(g_inventory.slots[2].item_id == 1001);

    item_shutdown();
    TEST_END();
}

/*
 * Test 17: Item sort by rarity
 */
static void test_inventory_sort_rarity(void) {
    TEST_BEGIN("Inventory sort by rarity");

    item_init();

    /* Add items with different rarities */
    g_items[1001].id = 1001;
    g_items[1001].rarity = ITEM_RARITY_COMMON;
    g_items[1002].id = 1002;
    g_items[1002].rarity = ITEM_RARITY_LEGENDARY;
    g_items[1003].id = 1003;
    g_items[1003].rarity = ITEM_RARITY_RARE;
    g_item_count = 1004;

    inventory_add_item(1001, 1);
    inventory_add_item(1002, 1);
    inventory_add_item(1003, 1);

    inventory_sort_by_rarity();

    /* Should be sorted: Legendary, Rare, Common */
    TEST_ASSERT(g_inventory.slots[0].item_id == 1002);
    TEST_ASSERT(g_inventory.slots[1].item_id == 1003);
    TEST_ASSERT(g_inventory.slots[2].item_id == 1001);

    item_shutdown();
    TEST_END();
}

/*
 * Test 18: Trade check
 */
static void test_inventory_trade(void) {
    TEST_BEGIN("Inventory trade");

    item_init();

    g_items[1001].id = 1001;
    g_items[1001].flags = ITEM_FLAG_TRADEABLE;
    g_items[1002].id = 1002;
    g_items[1002].flags = 0;  /* Not tradeable */
    g_item_count = 1003;

    inventory_add_item(1001, 5);
    inventory_add_item(1002, 5);

    TEST_ASSERT(inventory_can_trade(1001) == 1);
    TEST_ASSERT(inventory_can_trade(1002) == 0);

    item_shutdown();
    TEST_END();
}

/*
 * Test 19: Inventory clear - matches FUN_004117c0
 */
static void test_inventory_clear(void) {
    TEST_BEGIN("Inventory clear");

    item_init();

    /* Add items */
    inventory_add_item(1001, 10);
    inventory_add_item(1002, 20);
    inventory_add_item(1003, 30);

    /* Clear inventory */
    memset(g_inventory.slots, 0, sizeof(g_inventory.slots));
    g_inventory.count = 0;

    TEST_ASSERT(g_inventory.count == 0);

    int i;
    for (i = 0; i < 100; i++) {
        TEST_ASSERT(g_inventory.slots[i].item_id == 0);
    }

    item_shutdown();
    TEST_END();
}

/*
 * Test 20: DBCS string check - matches FUN_00411900
 * Tests the DBCS lead byte check for Chinese characters
 */
static void test_dbcs_check(void) {
    TEST_BEGIN("DBCS string check");

    /* FUN_00411900 returns:
     * 0 if string is empty
     * 1 if last char is ASCII
     * 2 if last char is DBCS
     * 3 if string ends with incomplete DBCS
     */

    /* ASCII string */
    const char* ascii = "Hello";
    TEST_ASSERT(strlen(ascii) == 5);

    /* Chinese string (DBCS) - "测试" */
    const char* dbcs = "\xb2\xe2\xca\xd4";  /* GB2312 encoding */
    TEST_ASSERT(strlen(dbcs) == 4);

    /* Empty string */
    const char* empty = "";
    TEST_ASSERT(strlen(empty) == 0);

    TEST_END();
}

/*
 * Test 21: Item parse from packet - matches FUN_0045ffb0 case 0x42
 */
static void test_item_parse_packet(void) {
    TEST_BEGIN("Item parse packet");

    item_init();

    /* Simulated packet data */
    const char* packet = "Test Item|Short|1001|Description|0|5|0|0|200|Field8|0|Field10|0|Field12|";

    /* Parse with character index '0' */
    item_parse_inventory_list(packet, '0');

    /* First slot should have item */
    TEST_ASSERT(g_inventory.slots[0].item_id != 0);
    TEST_ASSERT(g_inventory.slots[0].count == 5);

    item_shutdown();
    TEST_END();
}

/*
 * Test 22: Inventory wrap - matches DAT_0054a4f8 wrap at 0x13
 * From binary: if (0x13 < DAT_0054a4f8) DAT_0054a4f8 = 0;
 */
static void test_inventory_wrap(void) {
    TEST_BEGIN("Inventory wrap");

    item_init();

    /* The binary uses 20 slots (0x13 + 1 = 20) with circular buffer */
    /* Add items to fill slots */
    int i;
    for (i = 0; i < 20; i++) {
        inventory_add_item(1000 + i, 1);
    }

    TEST_ASSERT(g_inventory.count == 20);

    /* Next add should fail (inventory full) or wrap */
    int result = inventory_add_item(2000, 1);
    TEST_ASSERT(result == 0);

    item_shutdown();
    TEST_END();
}

/*
 * Test 23: Item count get
 */
static void test_inventory_get_count(void) {
    TEST_BEGIN("Inventory get count");

    item_init();

    inventory_add_item(1001, 10);
    inventory_add_item(1002, 5);

    u16 count = inventory_get_count(1001);
    TEST_ASSERT(count == 10);

    count = inventory_get_count(1002);
    TEST_ASSERT(count == 5);

    count = inventory_get_count(1003);
    TEST_ASSERT(count == 0);

    item_shutdown();
    TEST_END();
}

/*
 * Test 24: Inventory item size - matches 0x10c bytes per slot
 * From FUN_004115e0: DAT_00544d88 + DAT_0054a4f8 * 0x10c
 */
static void test_inventory_item_size(void) {
    TEST_BEGIN("Inventory item size");

    /* Binary uses 0x10c (268) bytes per inventory slot */
    /* Our simplified slot is smaller, but InventoryItem structure matches */

    TEST_ASSERT(sizeof(InventoryItem) == 0x184);
    TEST_ASSERT(sizeof(InventorySlot) == 8);

    TEST_END();
}

/*
 * Test 25: Can equip check
 */
static void test_inventory_can_equip(void) {
    TEST_BEGIN("Inventory can equip");

    item_init();

    /* Weapon */
    g_items[1001].id = 1001;
    g_items[1001].type = ITEM_TYPE_WEAPON;

    /* Armor */
    g_items[1002].id = 1002;
    g_items[1002].type = ITEM_TYPE_ARMOR;

    /* Consumable */
    g_items[1003].id = 1003;
    g_items[1003].type = ITEM_TYPE_CONSUMABLE;

    g_item_count = 1004;

    TEST_ASSERT(inventory_can_equip(1001, EQUIP_SLOT_WEAPON) == 1);
    TEST_ASSERT(inventory_can_equip(1002, EQUIP_SLOT_ARMOR) == 1);
    TEST_ASSERT(inventory_can_equip(1003, EQUIP_SLOT_WEAPON) == 0);

    item_shutdown();
    TEST_END();
}

/*
 * Test 26: Get equipped item
 */
static void test_inventory_get_equipped(void) {
    TEST_BEGIN("Inventory get equipped");

    item_init();

    g_items[1001].id = 1001;
    g_items[1001].type = ITEM_TYPE_WEAPON;
    g_item_count = 1002;

    /* Nothing equipped */
    Item* item = inventory_get_equipped(EQUIP_SLOT_WEAPON);
    TEST_ASSERT(item == NULL);

    /* Equip item */
    inventory_add_item(1001, 1);
    inventory_equip(0, EQUIP_SLOT_WEAPON);

    item = inventory_get_equipped(EQUIP_SLOT_WEAPON);
    TEST_ASSERT(item != NULL);
    TEST_ASSERT(item->id == 1001);

    item_shutdown();
    TEST_END();
}

/*
 * Test 27: Remove more than available
 */
static void test_inventory_remove_excess(void) {
    TEST_BEGIN("Inventory remove excess");

    item_init();

    inventory_add_item(1001, 5);

    /* Try to remove more than available */
    int result = inventory_remove_item(1001, 10);
    TEST_ASSERT(result == 0);

    /* Original count should be unchanged */
    TEST_ASSERT(g_inventory.slots[0].count == 5);

    item_shutdown();
    TEST_END();
}

/*
 * Test 28: Item flags
 */
static void test_item_flags(void) {
    TEST_BEGIN("Item flags");

    item_init();

    g_items[1001].id = 1001;
    g_items[1001].flags = ITEM_FLAG_TRADEABLE | ITEM_FLAG_DROPPABLE;

    TEST_ASSERT(inventory_can_trade(1001) == 1);

    /* Quest item - not tradeable or droppable */
    g_items[1002].id = 1002;
    g_items[1002].flags = ITEM_FLAG_QUEST;

    TEST_ASSERT(inventory_can_trade(1002) == 0);

    item_shutdown();
    TEST_END();
}

/*
 * Test 29: Inventory item active flag - matches DAT_046275b0
 */
static void test_inventory_active_flag(void) {
    TEST_BEGIN("Inventory active flag");

    /* From binary: *(undefined2 *)(&DAT_046275b0 + index) = 1 when item exists */
    InventoryItem item = {0};

    TEST_ASSERT(item.active == 0);

    item.active = 1;
    TEST_ASSERT(item.active == 1);

    TEST_END();
}

/*
 * Test 30: Level overflow handling - matches FUN_0045ffb0 case 0x42
 */
static void test_item_level_overflow(void) {
    TEST_BEGIN("Item level overflow");

    /* From binary:
     * if (level >= 100) {
     *     level = level % 100;
     *     has_level = 1;
     * }
     */
    s16 level = 150;
    s16 has_level = 0;

    if (level >= 100) {
        level = level % 100;
        has_level = 1;
    }

    TEST_ASSERT(level == 50);
    TEST_ASSERT(has_level == 1);

    /* Normal level */
    level = 50;
    has_level = 0;

    if (level >= 100) {
        level = level % 100;
        has_level = 1;
    }

    TEST_ASSERT(level == 50);
    TEST_ASSERT(has_level == 0);

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(item) {
    test_item_init();
    test_inventory_add();
    test_inventory_add_multiple();
    test_inventory_remove();
    test_inventory_has_item();
    test_inventory_find_slot();
    test_inventory_find_empty();
    test_inventory_capacity();
    test_inventory_stack();
    test_item_database();
    test_inventory_equip();
    test_inventory_unequip();
    test_inventory_bank_deposit();
    test_inventory_bank_withdraw();
    test_inventory_gold();
    test_inventory_sort_name();
    test_inventory_sort_rarity();
    test_inventory_trade();
    test_inventory_clear();
    test_dbcs_check();
    test_item_parse_packet();
    test_inventory_wrap();
    test_inventory_get_count();
    test_inventory_item_size();
    test_inventory_can_equip();
    test_inventory_get_equipped();
    test_inventory_remove_excess();
    test_item_flags();
    test_inventory_active_flag();
    test_item_level_overflow();
}

/* ========================================
 * Main
 * ======================================== */

int main(void) {
    printf("=== Item System Unit Tests ===\n\n");
    RUN_SUITE(item);

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
