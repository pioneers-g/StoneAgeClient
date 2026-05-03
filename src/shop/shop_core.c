/*
 * Stone Age Client - Shop Core Implementation
 * Reverse engineered from sa_9061.exe (FUN_00416be0)
 *
 * Core shop logic: transactions, pricing, database
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "shop.h"
#include "shop_protocol.h"
#include "item.h"
#include "network.h"
#include "logger.h"

/* Forward declarations for inventory functions */
extern int inventory_add_item(u32 item_id, u16 count);
extern int inventory_remove_item(u32 item_id, u16 count);
extern s32 inventory_find_empty_slot(void);
extern Item* item_get(u32 item_id);

/* Global shop context */
ShopContext g_shop = {0};

/*
 * Initialize shop system
 */
int shop_init(void) {
    memset(&g_shop, 0, sizeof(ShopContext));

    LOG_INFO("Shop system initialized");
    return 1;
}

/*
 * Shutdown shop system
 */
void shop_shutdown(void) {
    memset(&g_shop, 0, sizeof(ShopContext));
    LOG_INFO("Shop system shutdown");
}

/*
 * Load shop database
 */
int shop_load_database(const char* path) {
    HANDLE hFile;
    DWORD bytes_read;
    u32 count, i;
    Shop shop;

    hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        LOG_WARN("Failed to load shop database: %s", path);
        return 0;
    }

    /* Read count */
    ReadFile(hFile, &count, sizeof(u32), &bytes_read, NULL);
    if (count > 50) count = 50;

    /* Read shops */
    for (i = 0; i < count; i++) {
        if (!ReadFile(hFile, &shop, sizeof(Shop), &bytes_read, NULL)) break;
        if (g_shop.shops[shop.id].id == 0) {
            memcpy(&g_shop.shops[shop.id], &shop, sizeof(Shop));
            g_shop.shop_count++;
        }
    }

    CloseHandle(hFile);

    LOG_INFO("Loaded %u shops from database", count);
    return 1;
}

/*
 * Get shop by ID
 */
Shop* shop_get(u32 shop_id) {
    if (shop_id >= 50) return NULL;
    if (g_shop.shops[shop_id].id == 0) return NULL;
    return &g_shop.shops[shop_id];
}

/*
 * Get shop by NPC ID
 */
Shop* shop_get_by_npc(u32 npc_id) {
    u32 i;

    for (i = 0; i < 50; i++) {
        if (g_shop.shops[i].npc_id == npc_id) {
            return &g_shop.shops[i];
        }
    }

    return NULL;
}

/*
 * Open shop by ID
 */
int shop_open(u32 shop_id) {
    Shop* shop = shop_get(shop_id);

    if (!shop) {
        LOG_WARN("Shop %u not found", shop_id);
        return 0;
    }

    g_shop.current_shop = shop;
    g_shop.selected_item_index = 0;
    g_shop.selected_count = 1;
    shop->is_open = 1;

    LOG_INFO("Opened shop: %s", shop->name);
    return 1;
}

/*
 * Open shop by NPC
 */
int shop_open_by_npc(u32 npc_id) {
    Shop* shop = shop_get_by_npc(npc_id);

    if (!shop) {
        LOG_WARN("No shop for NPC %u", npc_id);
        return 0;
    }

    return shop_open(shop->id);
}

/*
 * Close shop
 */
void shop_close(void) {
    if (g_shop.current_shop) {
        g_shop.current_shop->is_open = 0;
        LOG_INFO("Closed shop: %s", g_shop.current_shop->name);
    }
    g_shop.current_shop = NULL;
}

/*
 * Check if shop is open
 */
int shop_is_open(void) {
    return g_shop.current_shop != NULL && g_shop.current_shop->is_open;
}

/*
 * Check if player can buy from shop
 */
int shop_can_buy(u32 item_index) {
    Shop* shop = g_shop.current_shop;
    ShopItemEntry* entry;
    Item* item;
    u32 total_price;

    if (!shop || !shop->is_open || !shop->can_buy) {
        return 0;
    }

    if (item_index >= shop->item_count) {
        return 0;
    }

    entry = &shop->items[item_index];

    /* Check stock */
    if (entry->stock > 0 && entry->stock < g_shop.selected_count) {
        return 0;
    }

    /* Check if player has enough gold */
    total_price = entry->price * g_shop.selected_count;
    if (g_inventory.gold < total_price) {
        return 0;
    }

    /* Check inventory space */
    item = item_get(entry->item_id);
    if (item && item->max_stack == 1) {
        /* Non-stackable - need empty slot */
        if (inventory_find_empty_slot() < 0) {
            return 0;
        }
    }

    return 1;
}

/*
 * Buy item from shop
 */
int shop_buy(u32 item_index, u16 count) {
    Shop* shop = g_shop.current_shop;
    ShopItemEntry* entry;
    u32 total_price;
    u32 item_id;

    if (!shop_can_buy(item_index)) {
        return 0;
    }

    entry = &shop->items[item_index];
    item_id = entry->item_id;
    total_price = entry->price * count;

    /* Deduct gold */
    g_inventory.gold -= total_price;

    /* Add item to inventory */
    if (!inventory_add_item(item_id, count)) {
        /* Refund gold on failure */
        g_inventory.gold += total_price;
        return 0;
    }

    /* Update stock */
    if (entry->stock > 0) {
        entry->stock -= count;
    }

    /* Update statistics */
    g_shop.total_bought += count;
    g_shop.gold_spent += total_price;

    /* Send network packet */
    shop_buy_network(item_id, total_price);

    LOG_INFO("Bought %d x item %u for %u gold", count, item_id, total_price);
    return 1;
}

/*
 * Buy specific item (for network commands)
 */
int shop_buy_item(u32 item_id, u32 price) {
    if (g_inventory.gold < price) {
        return 0;
    }

    g_inventory.gold -= price;

    if (!inventory_add_item(item_id, 1)) {
        g_inventory.gold += price;
        return 0;
    }

    g_shop.total_bought++;
    g_shop.gold_spent += price;

    return 1;
}

/*
 * Buy gold (special shop function)
 */
int shop_buy_gold(u32 amount) {
    /* This would typically convert real currency or points to gold */
    g_inventory.gold += amount;
    LOG_INFO("Purchased %u gold", amount);
    return 1;
}

/*
 * Check if player can sell to shop
 */
int shop_can_sell(u32 item_id) {
    Shop* shop = g_shop.current_shop;
    Item* item;

    if (!shop || !shop->is_open || !shop->can_sell) {
        return 0;
    }

    item = item_get(item_id);
    if (!item) {
        return 0;
    }

    /* Check if item is sellable */
    if (!(item->flags & ITEM_FLAG_TRADEABLE)) {
        return 0;
    }

    return 1;
}

/*
 * Sell item to shop
 */
int shop_sell(u32 slot_index, u16 count) {
    Shop* shop = g_shop.current_shop;
    Item* item;
    u32 item_id;
    u32 total_price;

    if (!shop || !shop->is_open || !shop->can_sell) {
        return 0;
    }

    if (slot_index >= g_inventory.capacity) {
        return 0;
    }

    if (g_inventory.slots[slot_index].item_id == 0 ||
        g_inventory.slots[slot_index].count < count) {
        return 0;
    }

    item_id = g_inventory.slots[slot_index].item_id;
    if (!shop_can_sell(item_id)) {
        return 0;
    }

    item = item_get(item_id);

    /* Calculate sell price (50% of buy price typically) */
    total_price = (item->price * shop->sell_rate / 100) * count;

    /* Remove item from inventory */
    if (!inventory_remove_item(item_id, count)) {
        return 0;
    }

    /* Add gold */
    g_inventory.gold += total_price;

    /* Update statistics */
    g_shop.total_sold += count;
    g_shop.gold_earned += total_price;

    /* Send network packet */
    shop_sell_network(slot_index, count);

    LOG_INFO("Sold %d x item %u for %u gold", count, item_id, total_price);
    return 1;
}

/*
 * Sell item by name (for network commands)
 */
int shop_sell_item(const char* item_name, u32 slot_index, u16 count) {
    return shop_sell(slot_index, count);
}

/*
 * Get shop item entry
 */
ShopItemEntry* shop_get_item(u32 item_index) {
    Shop* shop = g_shop.current_shop;

    if (!shop || item_index >= shop->item_count) {
        return NULL;
    }

    return &shop->items[item_index];
}

/*
 * Get buy price for item
 */
u32 shop_get_buy_price(u32 item_index) {
    ShopItemEntry* entry = shop_get_item(item_index);

    if (!entry) {
        return 0;
    }

    return entry->price * g_shop.selected_count;
}

/*
 * Get sell price for item
 */
u32 shop_get_sell_price(u32 item_id) {
    Shop* shop = g_shop.current_shop;
    Item* item;

    if (!shop) {
        return 0;
    }

    item = item_get(item_id);
    if (!item) {
        return 0;
    }

    return item->price * shop->sell_rate / 100;
}

/*
 * Get stock for item
 */
int shop_get_stock(u32 item_index) {
    ShopItemEntry* entry = shop_get_item(item_index);

    if (!entry) {
        return 0;
    }

    return entry->stock;
}

/*
 * On buy success callback
 */
void shop_on_buy_success(u32 item_id, u16 count, u32 gold_spent) {
    g_shop.total_bought += count;
    g_shop.gold_spent += gold_spent;

    LOG_INFO("Shop: Bought %d x item %u for %u gold", count, item_id, gold_spent);
}

/*
 * On sell success callback
 */
void shop_on_sell_success(u32 item_id, u16 count, u32 gold_earned) {
    g_shop.total_sold += count;
    g_shop.gold_earned += gold_earned;

    LOG_INFO("Shop: Sold %d x item %u for %u gold", count, item_id, gold_earned);
}

/*
 * On transaction failure callback
 */
void shop_on_transaction_fail(ShopResult result) {
    const char* messages[] = {
        "Success",
        "Not enough gold",
        "Not enough items",
        "Inventory full",
        "Shop is closed",
        "Cannot sell this item",
        "Cannot buy this item",
        "Item not found",
        "Out of stock"
    };

    if (result < sizeof(messages) / sizeof(messages[0])) {
        LOG_WARN("Shop transaction failed: %s", messages[result]);
    }
}
