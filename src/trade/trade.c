/*
 * Stone Age Client - Trade System Core
 * Reverse engineered from sa_9061.exe (FUN_0045a9a0, FUN_0045bb00)
 *
 * Core trade functionality:
 * - Initialization and shutdown
 * - Trade state management
 * - Item/gold operations
 * - Lock/unlock operations
 *
 * Protocol handlers: trade_protocol.c
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "trade.h"
#include "network.h"
#include "item.h"
#include "logger.h"

/* Global trade context */
TradeContext g_trade = {0};

/* Trade bit flag constant - from DAT_045e8ce0 */
#define TRADE_BIT_WINDOW    0x400000

/* Forward declarations */
static void trade_send_packet(const char* cmd, const char* params);

/*
 * Initialize trade system
 */
int trade_init(void) {
    memset(&g_trade, 0, sizeof(TradeContext));
    g_trade.selected_index = -1;
    g_trade.my_window = NULL;
    g_trade.partner_window = NULL;
    g_trade.bit_flags = 0;
    g_trade.window_visible = 0;

    LOG_INFO("Trade system initialized");
    return 1;
}

/*
 * Shutdown trade system
 */
void trade_shutdown(void) {
    trade_cancel();
    g_trade.my_window = NULL;
    g_trade.partner_window = NULL;
    memset(&g_trade, 0, sizeof(TradeContext));
    LOG_INFO("Trade system shutdown");
}

/*
 * Send trade packet to server
 */
static void trade_send_packet(const char* cmd, const char* params) {
    char packet[512];

    if (params && params[0]) {
        _snprintf(packet, sizeof(packet), "%s|%s", cmd, params);
    } else {
        strncpy(packet, cmd, sizeof(packet) - 1);
    }

    network_send(packet, strlen(packet));
    network_send("\n", 1);
}

/*
 * Request trade with player - FUN_0045a900 pattern
 */
int trade_request(u32 player_id) {
    char params[64];

    if (g_trade.state != TRADE_STATE_NONE) {
        LOG_WARN("Already in a trade");
        return 0;
    }

    /* Build and send trade request packet */
    _snprintf(params, sizeof(params), "%u", player_id);
    trade_send_packet("T", params);

    g_trade.state = TRADE_STATE_REQUESTING;
    g_trade.request_time = timeGetTime();
    g_trade.partner_id = player_id;

    LOG_DEBUG("Sent trade request to player %u", player_id);
    return 1;
}

/*
 * Accept trade request
 */
int trade_accept(void) {
    if (g_trade.state != TRADE_STATE_INVITED) {
        return 0;
    }

    /* Check timeout (30 seconds) */
    if (timeGetTime() - g_trade.request_time > 30000) {
        trade_cancel();
        LOG_WARN("Trade request expired");
        return 0;
    }

    /* Send accept packet */
    trade_send_packet("T", "A");

    g_trade.state = TRADE_STATE_TRADING;
    g_trade.sub_state = TRADE_SUB_INIT;
    g_trade.my_locked = 0;
    g_trade.partner_locked = 0;
    g_trade.bit_flags |= TRADE_BIT_WINDOW;

    LOG_INFO("Accepted trade with %s", g_trade.partner.name);
    return 1;
}

/*
 * Decline trade request
 */
void trade_decline(void) {
    /* Send decline packet */
    trade_send_packet("T", "D");

    trade_reset();
    LOG_DEBUG("Declined trade request");
}

/*
 * Cancel trade
 */
void trade_cancel(void) {
    if (g_trade.state == TRADE_STATE_NONE) {
        return;
    }

    /* Send cancel packet */
    trade_send_packet("T", "C");

    LOG_INFO("Trade cancelled");
    trade_reset();
}

/*
 * Reset trade state
 */
void trade_reset(void) {
    int i;

    /* Return items to inventory */
    for (i = 0; i < MAX_TRADE_ITEMS; i++) {
        g_trade.my_items[i].id = 0;
        g_trade.partner_items[i].id = 0;
    }

    g_trade.my_gold = 0;
    g_trade.partner_gold = 0;
    memset(&g_trade.partner, 0, sizeof(TradePartnerData));
    g_trade.state = TRADE_STATE_NONE;
    g_trade.sub_state = TRADE_SUB_NONE;
    g_trade.my_locked = 0;
    g_trade.partner_locked = 0;
    g_trade.window_visible = 0;
}

/*
 * Add item to trade - FUN_0045a900 pattern
 */
int trade_add_item(u32 item_id, u32 count) {
    char params[64];
    int i;

    if (g_trade.state != TRADE_STATE_TRADING) {
        return 0;
    }

    if (g_trade.my_locked) {
        LOG_WARN("Cannot modify locked trade");
        return 0;
    }

    /* Find empty slot */
    for (i = 0; i < MAX_TRADE_ITEMS; i++) {
        if (g_trade.my_items[i].id == 0) {
            g_trade.my_items[i].id = item_id;
            g_trade.my_items[i].count = count;
            g_trade.my_item_count++;
            break;
        }
    }

    if (i >= MAX_TRADE_ITEMS) {
        LOG_WARN("Trade window full");
        return 0;
    }

    /* Send add item packet */
    _snprintf(params, sizeof(params), "%u|%u", item_id, count);
    trade_send_packet("TI", params);

    LOG_DEBUG("Added item %u x%u to trade", item_id, count);
    return 1;
}

/*
 * Remove item from trade
 */
int trade_remove_item(int slot) {
    char params[32];

    if (g_trade.state != TRADE_STATE_TRADING) {
        return 0;
    }

    if (g_trade.my_locked) {
        LOG_WARN("Cannot modify locked trade");
        return 0;
    }

    if (slot < 0 || slot >= MAX_TRADE_ITEMS) {
        return 0;
    }

    if (g_trade.my_items[slot].id == 0) {
        return 0;
    }

    /* Send remove item packet */
    _snprintf(params, sizeof(params), "%d", slot);
    trade_send_packet("TR", params);

    g_trade.my_items[slot].id = 0;
    g_trade.my_items[slot].count = 0;
    g_trade.my_item_count--;

    LOG_DEBUG("Removed item from trade slot %d", slot);
    return 1;
}

/*
 * Set gold amount for trade
 */
int trade_set_gold(u32 gold) {
    char params[32];

    if (g_trade.state != TRADE_STATE_TRADING) {
        return 0;
    }

    if (g_trade.my_locked) {
        LOG_WARN("Cannot modify locked trade");
        return 0;
    }

    /* Check if player has enough gold */
    if (gold > g_inventory.gold) {
        LOG_WARN("Not enough gold (have %u, need %u)", g_inventory.gold, gold);
        return 0;
    }

    g_trade.my_gold = gold;

    /* Send set gold packet */
    _snprintf(params, sizeof(params), "%u", gold);
    trade_send_packet("TG", params);

    LOG_DEBUG("Set trade gold to %u", gold);
    return 1;
}

/*
 * Lock trade (confirm)
 */
int trade_lock(void) {
    if (g_trade.state != TRADE_STATE_TRADING) {
        return 0;
    }

    if (g_trade.my_locked) {
        return 0;
    }

    /* Send lock packet */
    trade_send_packet("TL", "");

    g_trade.my_locked = 1;

    LOG_DEBUG("Trade locked");
    return 1;
}

/*
 * Unlock trade
 */
int trade_unlock(void) {
    if (g_trade.state != TRADE_STATE_TRADING) {
        return 0;
    }

    if (!g_trade.my_locked) {
        return 0;
    }

    /* Check if partner also locked */
    if (g_trade.partner_locked) {
        LOG_WARN("Cannot unlock after both parties locked");
        return 0;
    }

    /* Send unlock packet */
    trade_send_packet("TU", "");

    g_trade.my_locked = 0;

    LOG_DEBUG("Trade unlocked");
    return 1;
}

/*
 * Get trade state
 */
TradeState trade_get_state(void) {
    return g_trade.state;
}

/*
 * Check if in trade
 */
int trade_is_active(void) {
    return g_trade.state != TRADE_STATE_NONE;
}

/*
 * Get partner name
 */
const char* trade_get_partner_name(void) {
    return g_trade.partner.name;
}

/*
 * Get my trade item
 */
TradeItem* trade_get_my_item(int slot) {
    if (slot < 0 || slot >= MAX_TRADE_ITEMS) {
        return NULL;
    }
    return &g_trade.my_items[slot];
}

/*
 * Get partner trade item
 */
TradeItem* trade_get_partner_item(int slot) {
    if (slot < 0 || slot >= MAX_TRADE_ITEMS) {
        return NULL;
    }
    return &g_trade.partner_items[slot];
}

/*
 * Get my gold amount
 */
u32 trade_get_my_gold(void) {
    return g_trade.my_gold;
}

/*
 * Get partner gold amount
 */
u32 trade_get_partner_gold(void) {
    return g_trade.partner_gold;
}

/*
 * Check if trade is locked by me
 */
int trade_is_my_locked(void) {
    return g_trade.my_locked;
}

/*
 * Check if trade is locked by partner
 */
int trade_is_partner_locked(void) {
    return g_trade.partner_locked;
}

/*
 * Check if trade is ready to complete
 */
int trade_is_ready(void) {
    return g_trade.my_locked && g_trade.partner_locked;
}

/*
 * Get my item count
 */
int trade_get_my_item_count(void) {
    return g_trade.my_item_count;
}

/*
 * Get partner item count
 */
int trade_get_partner_item_count(void) {
    return g_trade.partner_item_count;
}

/*
 * Check if trade window is visible
 */
int trade_is_window_visible(void) {
    return g_trade.window_visible;
}

/*
 * Get trade bit flags
 */
u32 trade_get_bit_flags(void) {
    return g_trade.bit_flags;
}

/*
 * Get partner character ID
 */
u32 trade_get_partner_id(void) {
    return g_trade.partner.char_id;
}
