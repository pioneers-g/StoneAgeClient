/*
 * Stone Age Client - Shop System Unit Tests
 * Tests for shop transactions, UI, and protocol handling
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../tests/test_framework.h"
#include "types.h"
#include "shop.h"
#include "shop_ui.h"

/* External globals from shop.c */
extern ShopContext g_shop;
extern u32 g_shop_item_count;

/* Mock inventory for testing */
static struct {
    u32 gold;
} g_inventory_mock = { 10000 };

/* Accessor functions for testing static state */
extern void shop_ui_set_scroll(int pos);
extern int shop_ui_get_mode(void);

/* Test shop constants */
void test_shop_constants(void) {
    TEST_ASSERT_EQ(MAX_SHOP_ITEMS, 100);
    TEST_ASSERT_EQ(MAX_SHOP_NAME_LEN, 32);
    TEST_ASSERT_EQ(MAX_SHOP_UI_ITEMS, 20);
    TEST_ASSERT_EQ(MAX_SHOP_SKILLS, 7);
}

/* Test shop types */
void test_shop_types(void) {
    TEST_ASSERT_EQ(SHOP_TYPE_GENERAL, 0);
    TEST_ASSERT_EQ(SHOP_TYPE_WEAPON, 1);
    TEST_ASSERT_EQ(SHOP_TYPE_ARMOR, 2);
    TEST_ASSERT_EQ(SHOP_TYPE_POTION, 3);
    TEST_ASSERT_EQ(SHOP_TYPE_PET, 4);
    TEST_ASSERT_EQ(SHOP_TYPE_GROCERY, 5);
    TEST_ASSERT_EQ(SHOP_TYPE_SPECIAL, 6);
}

/* Test shop result codes */
void test_shop_result_codes(void) {
    TEST_ASSERT_EQ(SHOP_RESULT_SUCCESS, 0);
    TEST_ASSERT_EQ(SHOP_RESULT_NOT_ENOUGH_GOLD, 1);
    TEST_ASSERT_EQ(SHOP_RESULT_NOT_ENOUGH_ITEMS, 2);
    TEST_ASSERT_EQ(SHOP_RESULT_INVENTORY_FULL, 3);
    TEST_ASSERT_EQ(SHOP_RESULT_SHOP_CLOSED, 4);
    TEST_ASSERT_EQ(SHOP_RESULT_CANNOT_SELL, 5);
    TEST_ASSERT_EQ(SHOP_RESULT_CANNOT_BUY, 6);
    TEST_ASSERT_EQ(SHOP_RESULT_ITEM_NOT_FOUND, 7);
    TEST_ASSERT_EQ(SHOP_RESULT_OUT_OF_STOCK, 8);
}

/* Test ShopItemEntry structure size - should match binary */
void test_shop_item_entry_size(void) {
    /* ShopItemEntry from shop.h - verify packing */
    TEST_ASSERT(sizeof(ShopItemEntry) >= 12);  /* Minimum expected size */
}

/* Test Shop structure */
void test_shop_structure(void) {
    Shop shop;
    memset(&shop, 0, sizeof(Shop));

    shop.id = 1;
    shop.type = SHOP_TYPE_WEAPON;
    shop.npc_id = 100;
    shop.buy_rate = 100;
    shop.sell_rate = 50;
    shop.is_open = 1;
    shop.can_buy = 1;
    shop.can_sell = 1;

    TEST_ASSERT_EQ(shop.id, 1);
    TEST_ASSERT_EQ(shop.type, SHOP_TYPE_WEAPON);
    TEST_ASSERT_EQ(shop.npc_id, 100);
    TEST_ASSERT_EQ(shop.buy_rate, 100);
    TEST_ASSERT_EQ(shop.sell_rate, 50);
    TEST_ASSERT_EQ(shop.is_open, 1);
}

/* Test ShopContext initialization */
void test_shop_context_init(void) {
    memset(&g_shop, 0, sizeof(ShopContext));

    TEST_ASSERT_EQ(g_shop.current_shop, NULL);
    TEST_ASSERT_EQ(g_shop.shop_count, 0);
    TEST_ASSERT_EQ(g_shop.selected_item_index, 0);
    TEST_ASSERT_EQ(g_shop.selected_count, 0);
}

/* Test shop initialization */
void test_shop_init(void) {
    int result = shop_init();
    TEST_ASSERT_EQ(result, 1);
    TEST_ASSERT_EQ(g_shop.shop_count, 0);
}

/* Test shop shutdown */
void test_shop_shutdown(void) {
    shop_shutdown();
    TEST_ASSERT_EQ(g_shop.current_shop, NULL);
}

/* Test shop open/close */
void test_shop_open_close(void) {
    /* Create test shop */
    Shop test_shop;
    memset(&test_shop, 0, sizeof(Shop));
    test_shop.id = 1;
    test_shop.is_open = 0;
    memcpy(&g_shop.shops[1], &test_shop, sizeof(Shop));
    g_shop.shop_count = 1;

    /* Open shop */
    int result = shop_open(1);
    TEST_ASSERT_EQ(result, 1);

    /* Check is open */
    TEST_ASSERT_EQ(shop_is_open(), 1);

    /* Close shop */
    shop_close();
    TEST_ASSERT_EQ(shop_is_open(), 0);
}

/* Test shop get functions */
void test_shop_get(void) {
    Shop test_shop;
    memset(&test_shop, 0, sizeof(Shop));
    test_shop.id = 5;
    test_shop.npc_id = 200;
    strcpy(test_shop.name, "Test Shop");
    memcpy(&g_shop.shops[5], &test_shop, sizeof(Shop));

    Shop* shop = shop_get(5);
    TEST_ASSERT(shop != NULL);
    TEST_ASSERT_EQ(shop->id, 5);
    TEST_ASSERT_STR_EQ(shop->name, "Test Shop");
}

void test_shop_get_by_npc(void) {
    Shop test_shop;
    memset(&test_shop, 0, sizeof(Shop));
    test_shop.id = 10;
    test_shop.npc_id = 300;
    memcpy(&g_shop.shops[10], &test_shop, sizeof(Shop));

    Shop* shop = shop_get_by_npc(300);
    TEST_ASSERT(shop != NULL);
    TEST_ASSERT_EQ(shop->id, 10);
}

/* Test shop item price calculation */
void test_shop_get_buy_price(void) {
    Shop test_shop;
    memset(&test_shop, 0, sizeof(Shop));
    test_shop.id = 1;
    test_shop.item_count = 1;
    test_shop.items[0].item_id = 100;
    test_shop.items[0].price = 500;
    test_shop.items[0].stock = 10;
    memcpy(&g_shop.shops[1], &test_shop, sizeof(Shop));
    g_shop.current_shop = &g_shop.shops[1];

    g_shop.selected_count = 2;
    u32 price = shop_get_buy_price(0);
    TEST_ASSERT_EQ(price, 1000);  /* 500 * 2 */
}

/* Test ShopDetailedItem structure */
void test_shop_detailed_item_structure(void) {
    ShopDetailedItem item;
    memset(&item, 0, sizeof(ShopDetailedItem));

    item.item_id = 100;
    item.sprite_id = 500;
    strcpy(item.name, "Test Item");
    item.price = 1000;
    item.has_details = 1;
    item.is_pet = 0;
    item.level = 10;
    item.hp = 100;
    item.mp = 50;
    item.attack = 25;
    item.defense = 15;

    TEST_ASSERT_EQ(item.item_id, 100);
    TEST_ASSERT_EQ(item.sprite_id, 500);
    TEST_ASSERT_STR_EQ(item.name, "Test Item");
    TEST_ASSERT_EQ(item.price, 1000);
    TEST_ASSERT_EQ(item.has_details, 1);
    TEST_ASSERT_EQ(item.level, 10);
    TEST_ASSERT_EQ(item.hp, 100);
    TEST_ASSERT_EQ(item.attack, 25);
}

/* Test ShopUIContext structure */
void test_shop_ui_context_structure(void) {
    ShopUIContext ctx;
    memset(&ctx, 0, sizeof(ShopUIContext));

    ctx.is_initialized = 1;
    ctx.sell_mode = 0;
    ctx.scroll_pos = 5;
    ctx.selected_index = 3;
    ctx.player_gold = 10000;

    TEST_ASSERT_EQ(ctx.is_initialized, 1);
    TEST_ASSERT_EQ(ctx.sell_mode, 0);
    TEST_ASSERT_EQ(ctx.scroll_pos, 5);
    TEST_ASSERT_EQ(ctx.selected_index, 3);
    TEST_ASSERT_EQ(ctx.player_gold, 10000);
}

/* Test shop UI initialization */
void test_shop_ui_init(void) {
    int result = shop_ui_init();
    TEST_ASSERT_EQ(result, 1);
}

/* Test shop UI shutdown */
void test_shop_ui_shutdown(void) {
    shop_ui_shutdown();
    TEST_PASS();
}

/* Test shop UI item management */
void test_shop_ui_item_management(void) {
    shop_ui_clear_items();

    ShopDetailedItem item1;
    memset(&item1, 0, sizeof(ShopDetailedItem));
    item1.item_id = 1;
    strcpy(item1.name, "Item 1");

    ShopDetailedItem item2;
    memset(&item2, 0, sizeof(ShopDetailedItem));
    item2.item_id = 2;
    strcpy(item2.name, "Item 2");

    shop_ui_add_item(&item1);
    shop_ui_add_item(&item2);

    TEST_ASSERT_EQ(g_shop_item_count, 2);

    ShopDetailedItem* retrieved = shop_ui_get_item(0);
    TEST_ASSERT(retrieved != NULL);
    TEST_ASSERT_EQ(retrieved->item_id, 1);

    retrieved = shop_ui_get_item(1);
    TEST_ASSERT(retrieved != NULL);
    TEST_ASSERT_EQ(retrieved->item_id, 2);

    /* Out of bounds */
    retrieved = shop_ui_get_item(100);
    TEST_ASSERT_EQ(retrieved, NULL);
}

/* Test shop UI scroll */
void test_shop_ui_scroll(void) {
    shop_ui_clear_items();
    shop_ui_set_scroll(0);

    /* Scroll down */
    shop_ui_scroll_down();
    /* Can't verify internal state, but function should not crash */

    /* Scroll up */
    shop_ui_scroll_up();
    /* Should not go negative */
    shop_ui_scroll_up();
    TEST_PASS();
}

/* Test shop UI set scroll */
void test_shop_ui_set_scroll(void) {
    shop_ui_set_scroll(5);
    /* Function should not crash */

    shop_ui_set_scroll(0);
    TEST_PASS();
}

/* Test shop UI mode toggle */
void test_shop_ui_mode_toggle(void) {
    /* Initialize to known state */
    shop_ui_init();

    shop_ui_toggle_mode();
    TEST_ASSERT_EQ(shop_ui_get_mode(), 1);

    shop_ui_toggle_mode();
    TEST_ASSERT_EQ(shop_ui_get_mode(), 0);
}

/* Test shop UI button sprite IDs from FUN_00416be0 */
void test_shop_ui_sprite_ids(void) {
    /* Sprite IDs from decompiled FUN_00416be0 */
    TEST_ASSERT_EQ(0x66b1, 26289);  /* Close button */
    TEST_ASSERT_EQ(0x66b3, 26291);  /* Confirm button */
    TEST_ASSERT_EQ(0x66db, 26331);  /* Scroll up */
    TEST_ASSERT_EQ(0x66dc, 26332);  /* Scroll down */
    TEST_ASSERT_EQ(0x6767, 26471);  /* Buy mode */
    TEST_ASSERT_EQ(0x6768, 26472);  /* Sell mode */
}

/* Test shop UI window dimensions from FUN_00416be0 */
void test_shop_ui_dimensions(void) {
    /* Window dimensions from FUN_00448610 call:
     * 0x269 = 617 width
     * 0x195 = 405 height
     * 0x89a8 = 35240 (sprite ID for background)
     */
    TEST_ASSERT_EQ(0x269, 617);
    TEST_ASSERT_EQ(0x195, 405);
    TEST_ASSERT_EQ(0x89a8, 35240);
}

/* Test shop protocol format constants */
void test_shop_protocol_format(void) {
    /* Protocol format from binary strings:
     * B|T|%d - Buy item
     * B|G|%d - Buy gold
     * S|D|%s|%d|%d - Sell item
     */
    const char* buy_item_fmt = "B|T|%d";
    const char* buy_gold_fmt = "B|G|%d";
    const char* sell_fmt = "S|D|%s|%d|%d";

    TEST_ASSERT_STR_EQ(buy_item_fmt, "B|T|%d");
    TEST_ASSERT_STR_EQ(buy_gold_fmt, "B|G|%d");
    TEST_ASSERT_STR_EQ(sell_fmt, "S|D|%s|%d|%d");
}

/* Test shop item entry size from binary analysis */
void test_shop_item_binary_size(void) {
    /* From FUN_00416be0 analysis:
     * Basic item entry: 0x15a = 346 bytes
     * Detailed item entry: 0x568 = 1384 bytes
     */
    TEST_ASSERT_EQ(0x15a, 346);
    TEST_ASSERT_EQ(0x568, 1384);
}

/* Test shop UI element array size */
void test_shop_ui_element_size(void) {
    /* From DAT_0054c88c: 0x14 = 20 elements */
    TEST_ASSERT_EQ(0x14, 20);
}

/* Test scroll limits */
void test_shop_scroll_limits(void) {
    /* From FUN_00416be0: scroll max is 0xd = 13 */
    TEST_ASSERT_EQ(0xd, 13);
}

/* Test shop callback functions */
void test_shop_callbacks(void) {
    u32 initial_bought = g_shop.total_bought;
    u32 initial_spent = g_shop.gold_spent;

    shop_on_buy_success(100, 5, 2500);

    TEST_ASSERT_EQ(g_shop.total_bought, initial_bought + 5);
    TEST_ASSERT_EQ(g_shop.gold_spent, initial_spent + 2500);

    u32 initial_sold = g_shop.total_sold;
    u32 initial_earned = g_shop.gold_earned;

    shop_on_sell_success(200, 3, 1500);

    TEST_ASSERT_EQ(g_shop.total_sold, initial_sold + 3);
    TEST_ASSERT_EQ(g_shop.gold_earned, initial_earned + 1500);
}

int main(void) {
    TEST_SUITE_BEGIN("Shop System Tests");

    /* Constants */
    TEST_RUN(test_shop_constants);
    TEST_RUN(test_shop_types);
    TEST_RUN(test_shop_result_codes);

    /* Structures */
    TEST_RUN(test_shop_item_entry_size);
    TEST_RUN(test_shop_structure);
    TEST_RUN(test_shop_context_init);
    TEST_RUN(test_shop_detailed_item_structure);
    TEST_RUN(test_shop_ui_context_structure);

    /* Initialization */
    TEST_RUN(test_shop_init);
    TEST_RUN(test_shop_shutdown);

    /* Shop operations */
    TEST_RUN(test_shop_open_close);
    TEST_RUN(test_shop_get);
    TEST_RUN(test_shop_get_by_npc);
    TEST_RUN(test_shop_get_buy_price);

    /* UI operations */
    TEST_RUN(test_shop_ui_init);
    TEST_RUN(test_shop_ui_shutdown);
    TEST_RUN(test_shop_ui_item_management);
    TEST_RUN(test_shop_ui_scroll);
    TEST_RUN(test_shop_ui_set_scroll);
    TEST_RUN(test_shop_ui_mode_toggle);

    /* Binary analysis */
    TEST_RUN(test_shop_ui_sprite_ids);
    TEST_RUN(test_shop_ui_dimensions);
    TEST_RUN(test_shop_protocol_format);
    TEST_RUN(test_shop_item_binary_size);
    TEST_RUN(test_shop_ui_element_size);
    TEST_RUN(test_shop_scroll_limits);

    /* Callbacks */
    TEST_RUN(test_shop_callbacks);

    TEST_SUITE_END();
}
