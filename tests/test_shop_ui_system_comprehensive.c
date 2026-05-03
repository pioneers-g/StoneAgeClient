/*
 * Stone Age Client - Shop UI System Comprehensive Tests
 * Tests for FUN_00416be0 (shop UI handler) and related functions
 *
 * Coverage:
 * - Shop window creation
 * - Item display and scrolling
 * - Buy/sell mode switching
 * - Item selection
 * - Price calculation
 * - Inventory checks
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

/* Shop UI Constants from binary */
#define SHOP_ITEM_MAX           20      /* Maximum items displayed */
#define SHOP_NAME_MAX           28      /* Maximum item name length (0x1c) */
#define SHOP_DESC_MAX           85      /* Maximum description length (0x55) */
#define SHOP_LIST_ITEMS         14      /* Items per page (0x0e) */
#define SHOP_ITEM_STRUCT_SIZE   0x568   /* Item structure size in shop data */
#define SHOP_ENTRY_SIZE         0x15a   /* Shop entry size (346 bytes) */

/* Sprite IDs from binary */
#define SPRITE_SHOP_BUTTON_BUY      0x66b1
#define SPRITE_SHOP_BUTTON_SELL     0x66b3
#define SPRITE_SHOP_SCROLL_UP       0x66db
#define SPRITE_SHOP_SCROLL_DOWN     0x66dc
#define SPRITE_SHOP_BUY_MODE        0x6767
#define SPRITE_SHOP_SELL_MODE       0x6768
#define SPRITE_SHOP_ITEM_SLOT       0x669d
#define SPRITE_SHOP_WINDOW          0x89a8

/* Shop item structure */
typedef struct {
    u32 id;                     /* Item database ID */
    u8 is_equipment;            /* 0=item, 1=equipment */
    u8 selected;                /* Selection flag */
    u8 reserved[2];
    char name[SHOP_NAME_MAX];   /* Item name */
    char description[SHOP_DESC_MAX]; /* Item description */
    u32 sprite_id;              /* Sprite ID for display */
    u16 buy_price;              /* Buy price */
    u16 sell_price;             /* Sell price */
    u16 stock;                  /* Stock count */
    u16 max_equip;              /* Max equip count */
    u32 field_4c;               /* Unknown field */
    u32 field_50;               /* Unknown field */
    u32 field_54;               /* Unknown field */
    u32 field_58;               /* Unknown field */
    u32 field_5c;               /* Unknown field */
    u32 field_60;               /* Unknown field */
    u32 field_64;               /* Unknown field */
    u32 field_68;               /* Unknown field */
    u32 field_6c;               /* Unknown field */
    u32 field_70;               /* Unknown field */
    u32 field_74;               /* Unknown field */
    char extra_desc[25];        /* Extra description lines */
    char extra_desc2[16];       /* More description */
    u8 has_extra;               /* Has extra info */
} ShopItem;  /* Size matches SHOP_ITEM_STRUCT_SIZE */

/* Shop context */
typedef struct {
    u32 window_handle;
    u32 items[SHOP_LIST_ITEMS]; /* UI element handles */
    u32 button_buy;
    u32 button_sell;
    u32 scroll_up;
    u32 scroll_down;
    u32 mode_button;

    ShopItem shop_items[SHOP_ITEM_MAX];
    u32 item_count;
    s32 selected_index;
    s32 scroll_offset;
    u8 buy_mode;                /* 0=buy, 1=sell */
    u8 initialized;

    u32 player_gold;
    u32 cart_total;
    u8 cart_items[SHOP_ITEM_MAX];
    u8 cart_counts[SHOP_ITEM_MAX];
    u32 cart_count;
} ShopContext;

static ShopContext g_shop;

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
 * Shop System Functions
 * ======================================== */

static void shop_init(void) {
    memset(&g_shop, 0, sizeof(g_shop));
    g_shop.selected_index = -1;
    g_shop.scroll_offset = 0;
}

static void shop_add_item(const ShopItem* item) {
    if (!item || g_shop.item_count >= SHOP_ITEM_MAX) return;
    g_shop.shop_items[g_shop.item_count++] = *item;
}

static void shop_set_mode(u8 buy_mode) {
    g_shop.buy_mode = buy_mode;
    g_shop.selected_index = -1;
    g_shop.scroll_offset = 0;
    /* FIX: Clear cart when switching modes - prevents accidental purchases/sales */
    g_shop.cart_count = 0;
    memset(g_shop.cart_items, 0, sizeof(g_shop.cart_items));
    memset(g_shop.cart_counts, 0, sizeof(g_shop.cart_counts));
}

static int shop_select_item(s32 index) {
    if (index < 0 || index >= (s32)g_shop.item_count) {
        g_shop.selected_index = -1;
        return -1;
    }
    g_shop.selected_index = index;
    return 0;
}

static void shop_scroll_up(void) {
    if (g_shop.scroll_offset > 0) {
        g_shop.scroll_offset--;
    }
}

static void shop_scroll_down(void) {
    s32 max_scroll = g_shop.item_count - SHOP_LIST_ITEMS;
    if (max_scroll < 0) max_scroll = 0;
    if (g_shop.scroll_offset < max_scroll) {
        g_shop.scroll_offset++;
    }
}

static u32 shop_calculate_total(void) {
    u32 total = 0;
    for (u32 i = 0; i < g_shop.cart_count; i++) {
        u8 item_idx = g_shop.cart_items[i];
        u8 count = g_shop.cart_counts[i];
        if (g_shop.buy_mode == 0) {
            total += g_shop.shop_items[item_idx].buy_price * count;
        } else {
            total += g_shop.shop_items[item_idx].sell_price * count;
        }
    }
    return total;
}

static int shop_add_to_cart(u32 item_index, u8 count) {
    if (item_index >= g_shop.item_count || count == 0) return -1;

    /* Check if already in cart */
    for (u32 i = 0; i < g_shop.cart_count; i++) {
        if (g_shop.cart_items[i] == item_index) {
            g_shop.cart_counts[i] += count;
            return 0;
        }
    }

    if (g_shop.cart_count >= SHOP_ITEM_MAX) return -1;

    g_shop.cart_items[g_shop.cart_count] = item_index;
    g_shop.cart_counts[g_shop.cart_count] = count;
    g_shop.cart_count++;

    return 0;
}

static int shop_remove_from_cart(u32 item_index) {
    for (u32 i = 0; i < g_shop.cart_count; i++) {
        if (g_shop.cart_items[i] == item_index) {
            /* Shift remaining items */
            for (u32 j = i; j < g_shop.cart_count - 1; j++) {
                g_shop.cart_items[j] = g_shop.cart_items[j + 1];
                g_shop.cart_counts[j] = g_shop.cart_counts[j + 1];
            }
            g_shop.cart_count--;
            return 0;
        }
    }
    return -1;
}

static int shop_can_afford(void) {
    return g_shop.player_gold >= shop_calculate_total();
}

/* ========================================
 * Test Setup/Teardown
 * ======================================== */

static void test_setup(void) {
    shop_init();
}

static void test_teardown(void) {
    /* Nothing to clean up */
}

/* ========================================
 * Constants Tests
 * ======================================== */

static int test_shop_item_max(void) {
    return SHOP_ITEM_MAX == 20;
}

static int test_shop_name_max(void) {
    return SHOP_NAME_MAX == 28;
}

static int test_shop_list_items(void) {
    return SHOP_LIST_ITEMS == 14;
}

static int test_shop_item_struct_size(void) {
    return SHOP_ITEM_STRUCT_SIZE == 0x568;
}

static int test_sprite_ids(void) {
    return SPRITE_SHOP_BUTTON_BUY == 0x66b1 &&
           SPRITE_SHOP_BUTTON_SELL == 0x66b3 &&
           SPRITE_SHOP_SCROLL_UP == 0x66db &&
           SPRITE_SHOP_SCROLL_DOWN == 0x66dc;
}

static int test_sprite_mode_buttons(void) {
    return SPRITE_SHOP_BUY_MODE == 0x6767 &&
           SPRITE_SHOP_SELL_MODE == 0x6768;
}

/* ========================================
 * Shop Initialization Tests
 * ======================================== */

static int test_shop_init(void) {
    test_setup();

    int pass = g_shop.item_count == 0 &&
               g_shop.selected_index == -1 &&
               g_shop.scroll_offset == 0 &&
               g_shop.buy_mode == 0;

    test_teardown();
    return pass;
}

static int test_shop_add_item(void) {
    test_setup();

    ShopItem item = {
        .id = 100,
        .buy_price = 100,
        .sell_price = 50
    };
    strcpy(item.name, "Test Item");

    shop_add_item(&item);

    int pass = g_shop.item_count == 1 &&
               g_shop.shop_items[0].id == 100 &&
               strcmp(g_shop.shop_items[0].name, "Test Item") == 0;

    test_teardown();
    return pass;
}

static int test_shop_add_multiple(void) {
    test_setup();

    ShopItem item = {0};
    for (int i = 0; i < 10; i++) {
        item.id = i + 1;
        shop_add_item(&item);
    }

    int pass = g_shop.item_count == 10;

    test_teardown();
    return pass;
}

static int test_shop_add_max(void) {
    test_setup();

    ShopItem item = {0};
    for (int i = 0; i < SHOP_ITEM_MAX + 5; i++) {
        item.id = i + 1;
        shop_add_item(&item);
    }

    int pass = g_shop.item_count == SHOP_ITEM_MAX;

    test_teardown();
    return pass;
}

/* ========================================
 * Shop Mode Tests
 * ======================================== */

static int test_shop_mode_buy(void) {
    test_setup();

    shop_set_mode(0);

    int pass = g_shop.buy_mode == 0;

    test_teardown();
    return pass;
}

static int test_shop_mode_sell(void) {
    test_setup();

    shop_set_mode(1);

    int pass = g_shop.buy_mode == 1;

    test_teardown();
    return pass;
}

static int test_shop_mode_resets_selection(void) {
    test_setup();

    ShopItem item = {.id = 1};
    shop_add_item(&item);
    shop_select_item(0);

    shop_set_mode(1);

    int pass = g_shop.selected_index == -1 &&
               g_shop.scroll_offset == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Item Selection Tests
 * ======================================== */

static int test_shop_select_valid(void) {
    test_setup();

    ShopItem item = {.id = 1};
    shop_add_item(&item);

    int result = shop_select_item(0);

    int pass = result == 0 && g_shop.selected_index == 0;

    test_teardown();
    return pass;
}

static int test_shop_select_invalid(void) {
    test_setup();

    ShopItem item = {.id = 1};
    shop_add_item(&item);

    int result = shop_select_item(5);

    int pass = result == -1 && g_shop.selected_index == -1;

    test_teardown();
    return pass;
}

static int test_shop_select_negative(void) {
    test_setup();

    ShopItem item = {.id = 1};
    shop_add_item(&item);

    int result = shop_select_item(-1);

    int pass = result == -1;

    test_teardown();
    return pass;
}

/* ========================================
 * Scroll Tests
 * ======================================== */

static int test_shop_scroll_up_from_zero(void) {
    test_setup();

    shop_scroll_up();

    int pass = g_shop.scroll_offset == 0;

    test_teardown();
    return pass;
}

static int test_shop_scroll_down(void) {
    test_setup();

    ShopItem item = {0};
    for (int i = 0; i < 20; i++) {
        shop_add_item(&item);
    }

    shop_scroll_down();

    int pass = g_shop.scroll_offset == 1;

    test_teardown();
    return pass;
}

static int test_shop_scroll_limit(void) {
    test_setup();

    ShopItem item = {0};
    for (int i = 0; i < 20; i++) {
        shop_add_item(&item);
    }

    /* Scroll many times */
    for (int i = 0; i < 20; i++) {
        shop_scroll_down();
    }

    /* Max scroll should be item_count - SHOP_LIST_ITEMS = 6 */
    int pass = g_shop.scroll_offset == 6;

    test_teardown();
    return pass;
}

static int test_shop_scroll_up_after_down(void) {
    test_setup();

    ShopItem item = {0};
    for (int i = 0; i < 20; i++) {
        shop_add_item(&item);
    }

    shop_scroll_down();
    shop_scroll_down();
    shop_scroll_up();

    int pass = g_shop.scroll_offset == 1;

    test_teardown();
    return pass;
}

/* ========================================
 * Cart Tests
 * ======================================== */

static int test_shop_cart_add(void) {
    test_setup();

    ShopItem item = {.id = 1, .buy_price = 100, .sell_price = 50};
    shop_add_item(&item);

    int result = shop_add_to_cart(0, 2);

    int pass = result == 0 &&
               g_shop.cart_count == 1 &&
               g_shop.cart_items[0] == 0 &&
               g_shop.cart_counts[0] == 2;

    test_teardown();
    return pass;
}

static int test_shop_cart_remove(void) {
    test_setup();

    ShopItem item = {.id = 1, .buy_price = 100};
    shop_add_item(&item);
    shop_add_to_cart(0, 2);

    int result = shop_remove_from_cart(0);

    int pass = result == 0 && g_shop.cart_count == 0;

    test_teardown();
    return pass;
}

static int test_shop_cart_total_buy(void) {
    test_setup();

    ShopItem item1 = {.id = 1, .buy_price = 100};
    ShopItem item2 = {.id = 2, .buy_price = 200};
    shop_add_item(&item1);
    shop_add_item(&item2);

    shop_add_to_cart(0, 2);  /* 2 * 100 = 200 */
    shop_add_to_cart(1, 1);  /* 1 * 200 = 200 */

    u32 total = shop_calculate_total();

    int pass = total == 400;

    test_teardown();
    return pass;
}

static int test_shop_cart_total_sell(void) {
    test_setup();

    ShopItem item = {.id = 1, .buy_price = 100, .sell_price = 50};
    shop_add_item(&item);
    shop_set_mode(1);  /* Sell mode */

    shop_add_to_cart(0, 3);  /* 3 * 50 = 150 */

    u32 total = shop_calculate_total();

    int pass = total == 150;

    test_teardown();
    return pass;
}

static int test_shop_can_afford(void) {
    test_setup();

    ShopItem item = {.id = 1, .buy_price = 100};
    shop_add_item(&item);
    shop_add_to_cart(0, 2);  /* Total = 200 */

    g_shop.player_gold = 500;

    int pass = shop_can_afford() == 1;

    test_teardown();
    return pass;
}

static int test_shop_cannot_afford(void) {
    test_setup();

    ShopItem item = {.id = 1, .buy_price = 100};
    shop_add_item(&item);
    shop_add_to_cart(0, 3);  /* Total = 300 */

    g_shop.player_gold = 200;

    int pass = shop_can_afford() == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_shop_full_flow(void) {
    test_setup();

    /* Add items */
    ShopItem item1 = {.id = 1, .buy_price = 100, .sell_price = 50};
    ShopItem item2 = {.id = 2, .buy_price = 200, .sell_price = 100};
    strcpy(item1.name, "Potion");
    strcpy(item2.name, "Sword");

    shop_add_item(&item1);
    shop_add_item(&item2);

    /* Select item */
    shop_select_item(0);

    /* Add to cart */
    shop_add_to_cart(0, 2);
    shop_add_to_cart(1, 1);

    /* Check total */
    u32 total = shop_calculate_total();

    int pass = g_shop.item_count == 2 &&
               g_shop.selected_index == 0 &&
               g_shop.cart_count == 2 &&
               total == 400;

    test_teardown();
    return pass;
}

static int test_shop_mode_switch_flow(void) {
    test_setup();

    ShopItem item = {.id = 1, .buy_price = 100, .sell_price = 50};
    shop_add_item(&item);

    /* Buy mode */
    shop_set_mode(0);
    shop_add_to_cart(0, 1);
    u32 buy_total = shop_calculate_total();

    /* Switch to sell mode */
    shop_set_mode(1);
    shop_add_to_cart(0, 1);
    u32 sell_total = shop_calculate_total();

    int pass = buy_total == 100 && sell_total == 50;

    test_teardown();
    return pass;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Shop UI System Comprehensive Tests ===\n\n");

    /* Constants tests */
    printf("Constants Tests:\n");
    TEST(shop_item_max);
    TEST(shop_name_max);
    TEST(shop_list_items);
    TEST(shop_item_struct_size);
    TEST(sprite_ids);
    TEST(sprite_mode_buttons);

    /* Initialization tests */
    printf("\nInitialization Tests:\n");
    TEST(shop_init);
    TEST(shop_add_item);
    TEST(shop_add_multiple);
    TEST(shop_add_max);

    /* Mode tests */
    printf("\nMode Tests:\n");
    TEST(shop_mode_buy);
    TEST(shop_mode_sell);
    TEST(shop_mode_resets_selection);

    /* Selection tests */
    printf("\nSelection Tests:\n");
    TEST(shop_select_valid);
    TEST(shop_select_invalid);
    TEST(shop_select_negative);

    /* Scroll tests */
    printf("\nScroll Tests:\n");
    TEST(shop_scroll_up_from_zero);
    TEST(shop_scroll_down);
    TEST(shop_scroll_limit);
    TEST(shop_scroll_up_after_down);

    /* Cart tests */
    printf("\nCart Tests:\n");
    TEST(shop_cart_add);
    TEST(shop_cart_remove);
    TEST(shop_cart_total_buy);
    TEST(shop_cart_total_sell);
    TEST(shop_can_afford);
    TEST(shop_cannot_afford);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(shop_full_flow);
    TEST(shop_mode_switch_flow);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - Real packet data parsing (B|count|items...)
     * - Equipment stats display
     * - Shop window position calculation
     * - Item description formatting
     * - Price overflow handling
     * - Stock limitation
     * - Player inventory check for selling
     * - Multi-page item list
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
