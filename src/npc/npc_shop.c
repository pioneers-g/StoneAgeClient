/*
 * Stone Age Client - NPC Shop System
 * Split from npc.c for code organization
 *
 * Handles shop interactions, buying, and selling
 *
 * TODO: Verify shop packet format from FUN_00463380
 * TODO: Verify buy/sell protocol commands (B|T, B|G, S|D)
 * TODO: Verify shop mode values (1=Buy, 2=Sell, 3=Trade, 4=Storage, 5=Heal, 6=Teleport)
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "npc.h"
#include "npc_shop.h"
#include "network_protocol.h"
#include "logger.h"

/* External NPC context */
extern NPCContext g_npc;

/*
 * Open shop
 */
int npc_open_shop(u32 npc_id, const char* shop_name) {
    if (g_npc.shop_open) {
        return 0;
    }

    g_npc.shop_open = 1;
    g_npc.current_npc = npc_id;
    strncpy(g_npc.shop_name, shop_name, sizeof(g_npc.shop_name) - 1);

    /* Initialize shop items */
    if (!g_npc.shop_items) {
        g_npc.shop_items = (ShopItem*)malloc(MAX_SHOP_ITEMS * sizeof(ShopItem));
        if (!g_npc.shop_items) {
            return 0;
        }
        memset(g_npc.shop_items, 0, MAX_SHOP_ITEMS * sizeof(ShopItem));
    }

    g_npc.shop_count = 0;

    LOG_INFO("Shop opened: %s (NPC %u)", shop_name, npc_id);
    return 1;
}

/*
 * Close shop
 */
void npc_close_shop(void) {
    g_npc.shop_open = 0;
    g_npc.shop_count = 0;
    g_npc.current_npc = 0;

    LOG_INFO("Shop closed");
}

/*
 * Add item to shop
 */
int npc_add_shop_item(u32 item_id, u32 price, u32 stock) {
    if (g_npc.shop_count >= MAX_SHOP_ITEMS) {
        return 0;
    }

    g_npc.shop_items[g_npc.shop_count].id = item_id;
    g_npc.shop_items[g_npc.shop_count].price = price;
    g_npc.shop_items[g_npc.shop_count].stock = stock;
    g_npc.shop_items[g_npc.shop_count].flags = 1;

    g_npc.shop_count++;

    return 1;
}

/*
 * Find shop item by ID
 */
int npc_find_shop_item(u32 item_id) {
    int i;

    for (i = 0; i < g_npc.shop_count; i++) {
        if (g_npc.shop_items[i].id == item_id) {
            return i;
        }
    }

    return -1;
}

/*
 * Update shop display
 */
void npc_update_shop(void) {
    /* Would refresh shop UI */
}

/*
 * Buy item from shop - B|T|%d or B|G|%d protocol
 */
int npc_buy_item(u32 item_id, u32 count) {
    int index;
    ShopItem* item;
    char packet[64];
    u32 total_cost;

    index = npc_find_shop_item(item_id);
    if (index < 0) {
        return 0;
    }

    item = &g_npc.shop_items[index];

    /* Check stock */
    if (item->stock > 0 && item->stock < count) {
        return 0;
    }

    /* Calculate total cost */
    total_cost = item->price * count;

    /* Check player gold - from player data */
    if (g_npc.player_data) {
        u32* player_gold = (u32*)((char*)g_npc.player_data + 0x10);
        if (*player_gold < total_cost) {
            LOG_DEBUG("Not enough gold: have %u, need %u", *player_gold, total_cost);
            return 0;
        }
    }

    /* Send buy packet: B|T|item_id for trade, B|G|item_id for gold shop */
    if (g_npc.shop_mode == 3) {
        /* Trade shop */
        snprintf(packet, sizeof(packet), "B|T|%u", item_id);
    } else {
        /* Gold shop */
        snprintf(packet, sizeof(packet), "B|G|%u", item_id);
    }

    send_queue_add(packet, 0, 0);

    LOG_DEBUG("Buying %u x %u (cost %u)", item_id, count, total_cost);
    return 1;
}

/*
 * Sell item to shop - S|D|%s|%d|%d protocol
 */
int npc_sell_item(u32 item_id, u32 count) {
    char packet[128];
    char item_name[32];
    u32 sell_price;

    /* Get item name from inventory - placeholder */
    snprintf(item_name, sizeof(item_name), "item_%u", item_id);

    /* Calculate sell price (typically 50% of buy price) */
    sell_price = 0;

    /* Send sell packet: S|D|item_name|count|price */
    snprintf(packet, sizeof(packet), "S|D|%s|%u|%u", item_name, count, sell_price);

    send_queue_add(packet, 0, 0);

    LOG_DEBUG("Selling %u x %u", item_id, count);
    return 1;
}

/*
 * Handle shop packet from server
 */
void npc_handle_shop_packet(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 npc_id;
    u16 item_count;
    int i;

    if (size < 6) return;

    npc_id = *(u32*)ptr; ptr += 4;
    item_count = *(u16*)ptr; ptr += 2;

    /* Open shop */
    npc_open_shop(npc_id, "Shop");

    /* Parse items */
    for (i = 0; i < item_count && i < MAX_SHOP_ITEMS; i++) {
        u32 item_id = *(u32*)ptr; ptr += 4;
        u32 price = *(u32*)ptr; ptr += 4;
        u32 stock = *(u32*)ptr; ptr += 4;

        npc_add_shop_item(item_id, price, stock);
    }

    LOG_DEBUG("Shop packet: NPC %u, %d items", npc_id, item_count);
}
