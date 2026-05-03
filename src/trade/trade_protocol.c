/*
 * Stone Age Client - Trade Protocol Handlers
 * Split from trade.c
 *
 * Contains protocol handlers for trade messages:
 * - Text protocol handlers (FUN_0045a9a0)
 * - Packet handlers
 * - Partner action handlers
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "trade.h"
#include "network.h"
#include "logger.h"

/* External trade context */
extern TradeContext g_trade;

/* Trade bit flag constant - from DAT_045e8ce0 */
#define TRADE_BIT_WINDOW    0x400000

/* Forward declarations */
static int trade_parse_field(const char* src, int field_index, char* dest, int max_len);

/*
 * Parse pipe-delimited field - FUN_00489f70 pattern
 */
static int trade_parse_field(const char* src, int field_index, char* dest, int max_len) {
    const char* ptr = src;
    const char* end;
    int current_field = 0;
    int len;

    if (!src || !dest || max_len <= 0) {
        return -1;
    }

    dest[0] = '\0';

    /* Find the requested field */
    while (current_field < field_index && *ptr) {
        if (*ptr == '|') {
            current_field++;
        }
        ptr++;
    }

    if (current_field != field_index) {
        return -1;
    }

    /* Find end of field */
    end = ptr;
    while (*end && *end != '|') {
        end++;
    }

    /* Copy field content */
    len = end - ptr;
    if (len >= max_len) {
        len = max_len - 1;
    }
    strncpy(dest, ptr, len);
    dest[len] = '\0';

    return 0;
}

/*
 * Handle partner lock notification
 */
void trade_handle_partner_lock(int locked) {
    g_trade.partner_locked = locked;

    if (locked) {
        LOG_DEBUG("Partner locked trade");

        /* Check if both locked */
        if (g_trade.my_locked) {
            /* Trade ready to complete */
            g_trade.state = TRADE_STATE_CONFIRMING;
        }
    } else {
        LOG_DEBUG("Partner unlocked trade");
        g_trade.state = TRADE_STATE_TRADING;
    }
}

/*
 * Handle partner add item - FUN_0045a980 pattern
 */
void trade_handle_partner_add_item(void* data, u32 size) {
    u8* ptr = (u8*)data;
    int slot;
    u32 item_id;
    u32 count;

    item_id = *(u32*)ptr; ptr += 4;
    count = *(u32*)ptr;

    /* Find empty slot */
    for (slot = 0; slot < MAX_TRADE_ITEMS; slot++) {
        if (g_trade.partner_items[slot].id == 0) {
            g_trade.partner_items[slot].id = item_id;
            g_trade.partner_items[slot].count = count;
            g_trade.partner_item_count++;
            break;
        }
    }

    LOG_DEBUG("Partner added item %u x%u", item_id, count);
}

/*
 * Handle partner remove item
 */
void trade_handle_partner_remove_item(void* data, u32 size) {
    u8* ptr = (u8*)data;
    int slot;

    slot = *(int*)ptr;

    if (slot >= 0 && slot < MAX_TRADE_ITEMS) {
        g_trade.partner_items[slot].id = 0;
        g_trade.partner_items[slot].count = 0;
        g_trade.partner_item_count--;
    }

    LOG_DEBUG("Partner removed item from slot %d", slot);
}

/*
 * Handle partner set gold
 */
void trade_handle_partner_gold(void* data, u32 size) {
    u8* ptr = (u8*)data;
    g_trade.partner_gold = *(u32*)ptr;
    LOG_DEBUG("Partner set gold to %u", g_trade.partner_gold);
}

/*
 * Handle trade complete - FUN_0045bb00 pattern
 */
void trade_handle_complete(void* data, u32 size) {
    LOG_INFO("Trade completed successfully");

    /* Toggle bit flag from original */
    g_trade.bit_flags ^= TRADE_BIT_WINDOW;

    /* Cleanup windows */
    g_trade.my_window = NULL;
    g_trade.partner_window = NULL;
    g_trade.selected_index = -1;
    g_trade.window_visible = 0;

    /* Reset state flags */
    g_trade.state = TRADE_STATE_NONE;
    g_trade.sub_state = TRADE_SUB_CONFIRMED;

    /* Items are now transferred */
    g_trade.state = TRADE_STATE_COMPLETE;
}

/*
 * Handle trade cancel from partner
 */
void trade_handle_cancel(void) {
    LOG_INFO("Trade cancelled by partner");

    /* Toggle bit flag */
    g_trade.bit_flags ^= TRADE_BIT_WINDOW;

    /* Cleanup */
    g_trade.my_window = NULL;
    g_trade.partner_window = NULL;
    g_trade.selected_index = -1;
    g_trade.window_visible = 0;

    /* Reset state */
    memset(&g_trade.partner, 0, sizeof(TradePartnerData));
    g_trade.state = TRADE_STATE_NONE;
    g_trade.sub_state = TRADE_SUB_NONE;
    g_trade.my_locked = 0;
    g_trade.partner_locked = 0;
}

/*
 * Handle incoming trade request
 */
void trade_handle_request(void* data, u32 size) {
    u8* ptr = (u8*)data;

    if (g_trade.state != TRADE_STATE_NONE) {
        /* Auto-decline if busy - send decline packet */
        char packet[] = "T|D\n";
        network_send(packet, strlen(packet));
        return;
    }

    /* Parse request */
    g_trade.partner.char_id = *(u32*)ptr; ptr += 4;
    strncpy(g_trade.partner.name, (char*)ptr, sizeof(g_trade.partner.name) - 1);

    g_trade.state = TRADE_STATE_INVITED;
    g_trade.request_time = timeGetTime();

    LOG_INFO("Received trade request from %s", g_trade.partner.name);
}

/*
 * Handle text protocol trade command - FUN_0045a9a0 pattern
 * Parses pipe-delimited trade data from server
 */
void trade_handle_text(const char* param) {
    char field[256];
    char name1[64], name2[64], name3[64];
    int int_val;

    if (!param) return;

    LOG_DEBUG("Trade text: %s", param);

    /* Field 0: Command type */
    if (trade_parse_field(param, 0, field, sizeof(field)) != 0) {
        return;
    }

    /* Empty field - initial trade request */
    if (field[0] == '\0') {
        if (g_trade.state == TRADE_STATE_NONE) {
            /* Parse names from fields 2-4 */
            trade_parse_field(param, 2, name1, sizeof(name1));
            trade_parse_field(param, 3, name2, sizeof(name2));
            trade_parse_field(param, 4, name3, sizeof(name3));

            /* Check if invitation accepted */
            if (strcmp(name3, "accept") == 0) {
                g_trade.state = TRADE_STATE_TRADING;
                g_trade.sub_state = TRADE_SUB_INIT;
                g_trade.bit_flags |= TRADE_BIT_WINDOW;
                LOG_INFO("Trade started with %s", name1);
            }
        }
        return;
    }

    /* "accept" - Trade accepted */
    if (strcmp(field, "accept") == 0) {
        if (g_trade.state != TRADE_STATE_NONE) {
            /* Parse partner data from fields 2-4 */
            trade_parse_field(param, 2, g_trade.partner.name, sizeof(g_trade.partner.name));
            trade_parse_field(param, 3, name2, sizeof(name2));
            trade_parse_field(param, 4, name3, sizeof(name3));

            /* Start trading */
            g_trade.state = TRADE_STATE_TRADING;
            g_trade.sub_state = TRADE_SUB_INIT;
            g_trade.bit_flags |= TRADE_BIT_WINDOW;

            LOG_INFO("Trade accepted with %s", g_trade.partner.name);
        }
        return;
    }

    /* "cancel" - Trade cancelled */
    if (strcmp(field, "cancel") == 0) {
        trade_handle_cancel();
        return;
    }

    /* "complete" - Trade completed */
    if (strcmp(field, "complete") == 0) {
        trade_handle_complete(NULL, 0);
        return;
    }

    /* "lock" - Partner locked trade */
    if (strcmp(field, "lock") == 0) {
        trade_handle_partner_lock(1);
        return;
    }

    /* "unlock" - Partner unlocked trade */
    if (strcmp(field, "unlock") == 0) {
        trade_handle_partner_lock(0);
        return;
    }

    /* "item" - Item data */
    if (strcmp(field, "item") == 0) {
        /* Parse item fields */
        trade_parse_field(param, 6, field, sizeof(field));
        int_val = atoi(field);
        LOG_DEBUG("Trade item slot %d", int_val);
        return;
    }

    /* "gold" - Gold amount */
    if (strcmp(field, "gold") == 0) {
        trade_parse_field(param, 6, field, sizeof(field));
        g_trade.partner_gold = atoi(field);
        LOG_DEBUG("Partner gold: %u", g_trade.partner_gold);
        return;
    }
}

/*
 * Handle text protocol trade invite
 */
void trade_handle_invite_text(const char* player_name) {
    if (player_name && player_name[0]) {
        strncpy(g_trade.partner.name, player_name, sizeof(g_trade.partner.name) - 1);
        g_trade.state = TRADE_STATE_INVITING;
        LOG_INFO("Trade invite from: %s", player_name);
    }
}

/*
 * Handle text protocol trade start
 */
void trade_handle_start_text(const char* param) {
    g_trade.state = TRADE_STATE_TRADING;
    g_trade.sub_state = TRADE_SUB_ACTIVE;
    g_trade.my_locked = 0;
    g_trade.partner_locked = 0;
    LOG_INFO("Trade started");
}

/*
 * Handle text protocol trade complete
 */
void trade_handle_complete_text(const char* result) {
    if (result && result[0] == '1') {
        LOG_INFO("Trade completed successfully");
        g_trade.state = TRADE_STATE_COMPLETE;
    } else {
        LOG_INFO("Trade cancelled or failed");
        g_trade.state = TRADE_STATE_CANCELLED;
    }

    /* Cleanup */
    g_trade.my_window = NULL;
    g_trade.partner_window = NULL;
    g_trade.selected_index = -1;
    g_trade.window_visible = 0;
}

/*
 * Handle full trade packet from FUN_0045a9a0
 * This is the main dispatcher for trade protocol messages
 */
void trade_handle_packet(const char* packet_data) {
    char field0[64], field4[64], field5[256];

    if (!packet_data) return;

    /* Parse field 0 to determine message type */
    if (trade_parse_field(packet_data, 0, field0, sizeof(field0)) != 0) {
        return;
    }

    /* Parse field 4 for sub-type */
    trade_parse_field(packet_data, 4, field4, sizeof(field4));

    /* Empty field 0 - Initial trade request */
    if (field0[0] == '\0') {
        if (g_trade.state == TRADE_STATE_NONE) {
            /* Set up trade invitation */
            trade_parse_field(packet_data, 2, g_trade.partner.name,
                              sizeof(g_trade.partner.name));

            g_trade.state = TRADE_STATE_INVITED;
            g_trade.sub_state = TRADE_SUB_PENDING;
            g_trade.request_time = timeGetTime();

            LOG_INFO("Trade invitation from: %s", g_trade.partner.name);
        }
        return;
    }

    /* "accept" in field 0 - Trade accepted */
    if (strcmp(field0, "accept") == 0) {
        if (g_trade.state != TRADE_STATE_NONE) {
            /* Initialize trade windows */
            if (g_trade.my_window == NULL) {
                /* Create trade windows - FUN_00448610 pattern */
                g_trade.my_window = (void*)1;  /* Placeholder for window handle */
                g_trade.partner_window = (void*)1;
            }

            /* Set state */
            g_trade.state = TRADE_STATE_TRADING;
            g_trade.sub_state = TRADE_SUB_ACTIVE;
            g_trade.bit_flags |= TRADE_BIT_WINDOW;
            g_trade.window_visible = 1;

            LOG_INFO("Trade accepted with %s", g_trade.partner.name);
        }
        return;
    }

    /* "cancel" in field 0 - Trade cancelled */
    if (strcmp(field0, "cancel") == 0) {
        trade_handle_cancel();
        return;
    }

    /* Check field 4 for additional types */
    if (strcmp(field4, "lock") == 0) {
        g_trade.partner_locked = 1;

        /* Check if both locked */
        if (g_trade.my_locked) {
            g_trade.state = TRADE_STATE_CONFIRMING;
        }
        LOG_DEBUG("Partner locked trade");
        return;
    }

    if (strcmp(field4, "unlock") == 0) {
        g_trade.partner_locked = 0;
        g_trade.state = TRADE_STATE_TRADING;
        LOG_DEBUG("Partner unlocked trade");
        return;
    }

    /* Item data - field 5 has item slot */
    trade_parse_field(packet_data, 5, field5, sizeof(field5));
    if (field5[0] != '\0') {
        int slot = atoi(field5);
        if (slot >= 0 && slot < MAX_TRADE_ITEMS) {
            /* Parse item data from subsequent fields */
            trade_parse_field(packet_data, 6, field5, sizeof(field5));

            /* Add to partner items */
            g_trade.partner_items[slot].id = atoi(field5);
            g_trade.partner_item_count++;

            LOG_DEBUG("Partner added item to slot %d", slot);
        }
    }
}
