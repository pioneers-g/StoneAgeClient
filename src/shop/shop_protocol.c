/*
 * Stone Age Client - Shop Protocol Handling
 * Split from shop.c for code organization
 *
 * Protocol formats:
 *   B|T|%d - Buy item
 *   B|G|%d - Buy gold
 *   S|D|%s|%d|%d - Sell item
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "shop.h"
#include "shop_ui.h"
#include "item.h"
#include "network.h"
#include "logger.h"

/* External shop context */
extern ShopContext g_shop;
extern u32 g_shop_item_count;

/* Protocol helper - parse delimited field */
static int protocol_parse_field(const char* data, char delim, int index, char* out, int max_len) {
    int i;
    const char* start = data;
    const char* end;
    int len;

    for (i = 0; i < index; i++) {
        start = strchr(start, delim);
        if (!start) return 0;
        start++;
    }

    end = strchr(start, delim);
    if (end) {
        len = (int)(end - start);
    } else {
        len = (int)strlen(start);
    }

    if (len >= max_len) len = max_len - 1;
    strncpy(out, start, len);
    out[len] = '\0';

    return 1;
}

/*
 * Send buy request to server
 */
int shop_buy_network(u32 item_id, u32 price) {
    char packet[128];

    snprintf(packet, sizeof(packet), "B|T|%u", item_id);
    return network_send(packet, strlen(packet));
}

/*
 * Send sell request to server
 */
int shop_sell_network(u32 slot_index, u16 count) {
    char packet[128];
    InventorySlot* slot;
    Item* item;

    /* Get item from inventory slot */
    slot = &g_inventory.slots[slot_index];
    if (slot->item_id == 0) return 0;

    item = item_get(slot->item_id);
    if (!item) return 0;

    /* Format: S|D|<name>|<slot>|<count> */
    snprintf(packet, sizeof(packet), "S|D|%s|%u|%u",
             item->name, slot_index, count);

    return network_send(packet, strlen(packet));
}

/*
 * Send gold purchase request
 */
int shop_buy_gold_network(u32 amount) {
    char packet[64];

    snprintf(packet, sizeof(packet), "B|G|%u", amount);
    return network_send(packet, strlen(packet));
}

/*
 * Handle shop open packet
 */
void shop_handle_open(void* data, u32 size) {
    char* str = (char*)data;

    if (size < 2) return;

    /* Parse shop info */
    if (str[0] == 'S' && str[1] == '|') {
        /* Format: S|<shop_id>|<name>|<item_count>|... */
        char field[64];

        protocol_parse_field(str, '|', 1, field, sizeof(field));
        g_shop.shop_id = atoi(field);

        protocol_parse_field(str, '|', 2, g_shop.shop_name, sizeof(g_shop.shop_name));

        protocol_parse_field(str, '|', 3, field, sizeof(field));
        g_shop.item_count = atoi(field);

        g_shop.is_open = 1;

        LOG_INFO("Shop opened: %s (%u items)", g_shop.shop_name, g_shop.item_count);
    }
}

/*
 * Handle item list packet
 */
void shop_handle_item_list(void* data, u32 size) {
    char* str = (char*)data;
    char field[64];
    u32 i, count;

    if (size < 2) return;

    /* Parse item count */
    protocol_parse_field(str, '|', 1, field, sizeof(field));
    count = atoi(field);

    /* Clear existing items */
    shop_ui_clear_items();

    /* Parse items */
    for (i = 0; i < count && i < MAX_SHOP_UI_ITEMS; i++) {
        ShopDetailedItem item = {0};
        int base_idx = 2 + i * 5;

        /* Parse item fields */
        protocol_parse_field(str, '|', base_idx, field, sizeof(field));
        item.item_id = atoi(field);

        protocol_parse_field(str, '|', base_idx + 1, item.name, sizeof(item.name));

        protocol_parse_field(str, '|', base_idx + 2, field, sizeof(field));
        item.price = atoi(field);

        protocol_parse_field(str, '|', base_idx + 3, field, sizeof(field));
        item.sprite_id = atoi(field);

        protocol_parse_field(str, '|', base_idx + 4, item.description, sizeof(item.description));

        item.has_details = 1;

        shop_ui_add_item(&item);
    }

    LOG_DEBUG("Received %u shop items", count);
}

/*
 * Handle buy result packet
 */
void shop_handle_buy_result(void* data, u32 size) {
    char* str = (char*)data;
    char field[64];
    int result;
    u32 item_id;
    u16 count;
    u32 gold_spent;

    if (size < 2) return;

    /* Parse result */
    protocol_parse_field(str, '|', 1, field, sizeof(field));
    result = atoi(field);

    if (result == 0) {
        /* Success */
        protocol_parse_field(str, '|', 2, field, sizeof(field));
        item_id = atoi(field);

        protocol_parse_field(str, '|', 3, field, sizeof(field));
        count = (u16)atoi(field);

        protocol_parse_field(str, '|', 4, field, sizeof(field));
        gold_spent = atoi(field);

        shop_on_buy_success(item_id, count, gold_spent);
    } else {
        shop_on_transaction_fail((ShopResult)result);
    }
}

/*
 * Handle sell result packet
 */
void shop_handle_sell_result(void* data, u32 size) {
    char* str = (char*)data;
    char field[64];
    int result;
    u32 item_id;
    u16 count;
    u32 gold_earned;

    if (size < 2) return;

    /* Parse result */
    protocol_parse_field(str, '|', 1, field, sizeof(field));
    result = atoi(field);

    if (result == 0) {
        /* Success */
        protocol_parse_field(str, '|', 2, field, sizeof(field));
        item_id = atoi(field);

        protocol_parse_field(str, '|', 3, field, sizeof(field));
        count = (u16)atoi(field);

        protocol_parse_field(str, '|', 4, field, sizeof(field));
        gold_earned = atoi(field);

        shop_on_sell_success(item_id, count, gold_earned);
    } else {
        shop_on_transaction_fail((ShopResult)result);
    }
}

/*
 * Process shop packet - main entry point
 */
void shop_process_packet(const char* data, u32 size) {
    if (!data || size == 0) return;

    switch (data[0]) {
        case 'S':
            /* Shop open or item list */
            if (size > 1 && data[1] == '|') {
                if (g_shop.is_open) {
                    shop_handle_item_list(data, size);
                } else {
                    shop_handle_open(data, size);
                }
            }
            break;

        case 'B':
            /* Buy result */
            shop_handle_buy_result(data, size);
            break;

        case 'D':
            /* Item detail - forward to UI */
            shop_ui_process_packet(data, size);
            break;

        default:
            break;
    }
}
