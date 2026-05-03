/*
 * Stone Age Client - Storage/Warehouse System Core
 * Reverse engineered from sa_9061.exe
 *
 * Core storage functionality:
 * - Initialization and shutdown
 * - Item/gold operations
 * - Filtering and sorting
 * - Utility functions
 *
 * Protocol handlers: storage_protocol.c
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "storage.h"
#include "item.h"
#include "character.h"
#include "network.h"
#include "logger.h"

/* Global storage context */
StorageContext g_storage = {0};

/* Expansion costs */
static const u32 s_expansion_costs[] = {
    10000,   /* +10 slots */
    50000,   /* +10 slots */
    100000,  /* +10 slots */
    500000,  /* +10 slots */
    1000000  /* +10 slots */
};

/* Storage protocol commands */
#define STORAGE_CMD_DEPOSIT     "SA|D"
#define STORAGE_CMD_WITHDRAW    "SA|W"
#define STORAGE_CMD_GOLD_IN     "SA|G"
#define STORAGE_CMD_GOLD_OUT    "SA|GC"
#define STORAGE_CMD_EXPAND      "SA|E"
#define STORAGE_CMD_CLOSE       "SA|C"

/* Forward declarations */
static void storage_send_packet(const char* cmd, const char* params);

/*
 * Initialize storage system
 */
int storage_init(void) {
    memset(&g_storage, 0, sizeof(StorageContext));

    g_storage.max_slots = 50;  /* Base slots */
    g_storage.access_fee = STORAGE_FEE;

    LOG_INFO("Storage system initialized");
    return 1;
}

/*
 * Shutdown storage system
 */
void storage_shutdown(void) {
    memset(&g_storage, 0, sizeof(StorageContext));
    LOG_INFO("Storage system shutdown");
}

/*
 * Send storage protocol packet
 */
static void storage_send_packet(const char* cmd, const char* params) {
    char packet[256];

    if (params && params[0]) {
        _snprintf(packet, sizeof(packet), "%s|%s", cmd, params);
    } else {
        strncpy(packet, cmd, sizeof(packet) - 1);
    }

    send_queue_add(packet, 0, 0);
}

/*
 * Open storage
 */
int storage_open(void) {
    if (g_storage.is_open) {
        return 1;
    }

    /* Storage is opened via NPC dialog action 14 */
    g_storage.is_open = 1;
    g_storage.has_access = 1;

    LOG_DEBUG("Storage opened");
    return 1;
}

/*
 * Close storage
 */
void storage_close(void) {
    if (g_storage.dirty) {
        /* Send close packet to save data */
        storage_send_packet(STORAGE_CMD_CLOSE, "");
        g_storage.dirty = 0;
    }

    g_storage.is_open = 0;
    g_storage.selected_index = -1;
    g_storage.has_access = 0;

    LOG_DEBUG("Storage closed");
}

/*
 * Check if storage is open
 */
int storage_is_open(void) {
    return g_storage.is_open;
}

/*
 * Check if can access storage
 */
int storage_can_access(void) {
    return g_storage.has_access;
}

/*
 * Deposit item to storage
 */
int storage_deposit_item(u32 item_id, u16 count) {
    char params[64];
    int slot;

    if (!g_storage.is_open) {
        LOG_WARN("Storage not open");
        return 0;
    }

    if (count == 0) {
        return 0;
    }

    /* Check if storage has space */
    if (!storage_has_space_for(item_id, count)) {
        LOG_WARN("Storage full");
        return 0;
    }

    /* Find existing slot or free slot */
    slot = storage_find_item(item_id);
    if (slot < 0) {
        slot = storage_find_free_slot();
        if (slot < 0) {
            LOG_WARN("No free storage slot");
            return 0;
        }
    }

    /* Send deposit packet: SA|D|<item_id>|<count> */
    _snprintf(params, sizeof(params), "%u|%u", item_id, count);
    storage_send_packet(STORAGE_CMD_DEPOSIT, params);

    g_storage.dirty = 1;
    LOG_DEBUG("Depositing item %u x %d", item_id, count);
    return 1;
}

/*
 * Withdraw item from storage
 */
int storage_withdraw_item(int slot, u16 count) {
    StorageSlot* item;
    char params[64];

    if (!g_storage.is_open) {
        LOG_WARN("Storage not open");
        return 0;
    }

    if (slot < 0 || slot >= g_storage.max_slots) {
        return 0;
    }

    item = &g_storage.items[slot];
    if (item->item_id == 0 || item->count < count) {
        return 0;
    }

    /* Check inventory space via item system */
    if (inventory_find_empty_slot() < 0) {
        LOG_WARN("Inventory full");
        return 0;
    }

    /* Send withdraw packet: SA|W|<slot>|<count> */
    _snprintf(params, sizeof(params), "%d|%u", slot, count);
    storage_send_packet(STORAGE_CMD_WITHDRAW, params);

    g_storage.dirty = 1;
    LOG_DEBUG("Withdrawing item %u x %d from slot %d", item->item_id, count, slot);
    return 1;
}

/*
 * Get item count in slot
 */
int storage_get_item_count(int slot) {
    if (slot < 0 || slot >= g_storage.max_slots) {
        return 0;
    }
    return g_storage.items[slot].count;
}

/*
 * Get item ID in slot
 */
u32 storage_get_item_id(int slot) {
    if (slot < 0 || slot >= g_storage.max_slots) {
        return 0;
    }
    return g_storage.items[slot].item_id;
}

/*
 * Find item by ID
 */
int storage_find_item(u32 item_id) {
    int i;

    for (i = 0; i < g_storage.max_slots; i++) {
        if (g_storage.items[i].item_id == item_id) {
            return i;
        }
    }

    return -1;
}

/*
 * Find free slot
 */
int storage_find_free_slot(void) {
    int i;

    for (i = 0; i < g_storage.max_slots; i++) {
        if (g_storage.items[i].item_id == 0) {
            return i;
        }
    }

    return -1;
}

/*
 * Deposit gold to storage
 */
int storage_deposit_gold(u32 amount) {
    char params[64];

    if (!g_storage.is_open) {
        return 0;
    }

    if (amount == 0) {
        return 0;
    }

    if (g_storage.gold + amount > MAX_GOLD_STORAGE) {
        LOG_WARN("Gold storage limit reached");
        return 0;
    }

    /* Send gold deposit packet: SA|G|<amount> */
    _snprintf(params, sizeof(params), "%u", amount);
    storage_send_packet(STORAGE_CMD_GOLD_IN, params);

    g_storage.dirty = 1;
    LOG_DEBUG("Depositing %u gold", amount);
    return 1;
}

/*
 * Withdraw gold from storage
 */
int storage_withdraw_gold(u32 amount) {
    char params[64];

    if (!g_storage.is_open) {
        return 0;
    }

    if (amount == 0 || g_storage.gold < amount) {
        return 0;
    }

    /* Send gold withdraw packet: SA|GC|<amount> */
    _snprintf(params, sizeof(params), "%u", amount);
    storage_send_packet(STORAGE_CMD_GOLD_OUT, params);

    g_storage.gold -= amount;
    g_storage.dirty = 1;

    LOG_DEBUG("Withdrawing %u gold", amount);
    return 1;
}

/*
 * Get stored gold
 */
u32 storage_get_gold(void) {
    return g_storage.gold;
}

/*
 * Get max gold storage
 */
u32 storage_get_max_gold(void) {
    return MAX_GOLD_STORAGE;
}

/*
 * Set filter category
 */
void storage_set_filter(StorageCategory category) {
    g_storage.filter = category;
}

/*
 * Set search text
 */
void storage_set_search(const char* text) {
    if (text) {
        strncpy(g_storage.search_text, text, sizeof(g_storage.search_text) - 1);
    } else {
        g_storage.search_text[0] = '\0';
    }
}

/* Item category lookup (would integrate with item database) */
static int item_get_category(u32 item_id) {
    if (item_id >= 1000 && item_id < 2000) return STORAGE_CAT_WEAPON;
    if (item_id >= 2000 && item_id < 3000) return STORAGE_CAT_ARMOR;
    if (item_id >= 3000 && item_id < 4000) return STORAGE_CAT_CONSUMABLE;
    if (item_id >= 4000 && item_id < 5000) return STORAGE_CAT_MATERIAL;
    if (item_id >= 5000 && item_id < 6000) return STORAGE_CAT_QUEST;
    return STORAGE_CAT_OTHER;
}

/*
 * Check if item matches current filter
 */
static int item_matches_filter(StorageSlot* item) {
    /* Empty slots don't match */
    if (item->item_id == 0) return 0;

    /* Category filter */
    if (g_storage.filter != STORAGE_CAT_ALL) {
        int category = item_get_category(item->item_id);
        if (category != g_storage.filter) return 0;
    }

    /* Search text filter */
    if (g_storage.search_text[0] != '\0') {
        char id_str[16];
        _snprintf(id_str, sizeof(id_str), "%u", item->item_id);
        if (strstr(id_str, g_storage.search_text) == NULL) {
            return 0;
        }
    }

    return 1;
}

/*
 * Get filtered item count
 */
int storage_get_filtered_count(void) {
    int count = 0;
    int i;

    for (i = 0; i < g_storage.max_slots; i++) {
        if (item_matches_filter(&g_storage.items[i])) {
            count++;
        }
    }

    return count;
}

/*
 * Get filtered item
 */
StorageSlot* storage_get_filtered_item(int index) {
    int i, count = 0;

    for (i = 0; i < g_storage.max_slots; i++) {
        if (g_storage.items[i].item_id != 0) {
            if (count == index) {
                return &g_storage.items[i];
            }
            count++;
        }
    }

    return NULL;
}

/*
 * Expand storage
 */
int storage_expand(void) {
    u32 cost;

    if (g_storage.expansion_level >= 5) {
        LOG_WARN("Storage fully expanded");
        return 0;
    }

    cost = s_expansion_costs[g_storage.expansion_level];

    /* Send expansion request: SA|E| */
    storage_send_packet(STORAGE_CMD_EXPAND, "");

    g_storage.max_slots += 10;
    g_storage.expansion_level++;
    g_storage.expansion_cost = (g_storage.expansion_level < 5) ?
        s_expansion_costs[g_storage.expansion_level] : 0;

    LOG_INFO("Storage expanded to %d slots, cost: %u", g_storage.max_slots, cost);
    return 1;
}

/*
 * Get expansion cost
 */
int storage_get_expansion_cost(void) {
    if (g_storage.expansion_level >= 5) {
        return 0;
    }
    return s_expansion_costs[g_storage.expansion_level];
}

/*
 * Get max slots
 */
int storage_get_max_slots(void) {
    return g_storage.max_slots;
}

/*
 * Get total item count
 */
int storage_get_item_count_total(void) {
    int total = 0;
    int i;

    for (i = 0; i < g_storage.max_slots; i++) {
        if (g_storage.items[i].item_id != 0) {
            total += g_storage.items[i].count;
        }
    }

    return total;
}

/*
 * Check if has space for item
 */
int storage_has_space_for(u32 item_id, u16 count) {
    int slot = storage_find_item(item_id);

    if (slot >= 0) {
        return 1;
    }

    return storage_find_free_slot() >= 0;
}

/*
 * Get used slots
 */
int storage_get_used_slots(void) {
    return g_storage.item_count;
}

/*
 * Select slot
 */
void storage_select(int index) {
    g_storage.selected_index = index;
}

/*
 * Get selected index
 */
int storage_get_selected(void) {
    return g_storage.selected_index;
}

/*
 * Get selected item
 */
StorageSlot* storage_get_selected_item(void) {
    if (g_storage.selected_index < 0 ||
        g_storage.selected_index >= g_storage.max_slots) {
        return NULL;
    }
    return &g_storage.items[g_storage.selected_index];
}

/*
 * Deposit all of item from inventory
 */
int storage_deposit_all(u32 item_id) {
    int inv_count;
    char params[64];

    if (!g_storage.is_open) {
        return 0;
    }

    /* Get count from inventory */
    inv_count = inventory_get_count(item_id);
    if (inv_count <= 0) {
        LOG_DEBUG("No item %u in inventory to deposit", item_id);
        return 0;
    }

    /* Check storage space */
    if (!storage_has_space_for(item_id, inv_count)) {
        inv_count = g_storage.max_slots - g_storage.item_count;
        if (inv_count <= 0) {
            LOG_WARN("Storage full");
            return 0;
        }
    }

    /* Send deposit all packet */
    _snprintf(params, sizeof(params), "%u|%d", item_id, inv_count);
    storage_send_packet(STORAGE_CMD_DEPOSIT, params);

    g_storage.dirty = 1;
    LOG_DEBUG("Depositing all %u of item %u", inv_count, item_id);
    return 1;
}

/*
 * Withdraw all from slot
 */
int storage_withdraw_all(int slot) {
    if (slot < 0 || slot >= g_storage.max_slots) {
        return 0;
    }

    u16 count = g_storage.items[slot].count;
    if (count == 0) return 0;

    return storage_withdraw_item(slot, count);
}

/*
 * Swap with inventory slot
 */
int storage_swap_with_inventory(int inv_slot, int storage_slot) {
    char params[64];

    if (!g_storage.is_open) {
        return 0;
    }

    if (storage_slot < 0 || storage_slot >= g_storage.max_slots) {
        return 0;
    }

    if (inv_slot < 0 || inv_slot >= 100) {
        return 0;
    }

    /* Build swap packet: SA|X|<inv_slot>|<storage_slot> */
    _snprintf(params, sizeof(params), "%d|%d", inv_slot, storage_slot);
    storage_send_packet("SA|X", params);

    g_storage.dirty = 1;
    LOG_DEBUG("Swapping inv slot %d with storage slot %d", inv_slot, storage_slot);
    return 1;
}

/*
 * Refresh storage data
 */
void storage_refresh(void) {
    if (!g_storage.is_open) {
        return;
    }

    /* Request storage data: SA|R| */
    storage_send_packet("SA|R", "");

    LOG_DEBUG("Requested storage refresh");
}

/*
 * Clear storage (debug)
 */
void storage_clear(void) {
    memset(g_storage.items, 0, sizeof(g_storage.items));
    g_storage.item_count = 0;
    g_storage.gold = 0;
    g_storage.dirty = 0;
}

/*
 * Sort storage - in-place sorting of items
 */
void storage_sort(int mode, int ascending) {
    int i, j;
    StorageSlot temp;

    g_storage.sort_mode = mode;
    g_storage.sort_ascending = ascending;

    /* Bubble sort for simplicity */
    for (i = 0; i < g_storage.max_slots - 1; i++) {
        for (j = 0; j < g_storage.max_slots - i - 1; j++) {
            int should_swap = 0;
            StorageSlot* a = &g_storage.items[j];
            StorageSlot* b = &g_storage.items[j + 1];

            if (a->item_id == 0) continue;
            if (b->item_id == 0) {
                should_swap = 1;
            } else {
                switch (mode) {
                    case 0:  /* Sort by ID */
                        should_swap = ascending ?
                            (a->item_id > b->item_id) :
                            (a->item_id < b->item_id);
                        break;
                    case 1:  /* Sort by count */
                        should_swap = ascending ?
                            (a->count > b->count) :
                            (a->count < b->count);
                        break;
                    case 2:  /* Sort by name */
                        should_swap = ascending ?
                            (a->item_id > b->item_id) :
                            (a->item_id < b->item_id);
                        break;
                    default:
                        break;
                }
            }

            if (should_swap) {
                temp = *a;
                *a = *b;
                *b = temp;
            }
        }
    }

    g_storage.dirty = 1;
    LOG_DEBUG("Storage sorted by mode %d, ascending %d", mode, ascending);
}
