/*
 * Stone Age Client - Item System Core Implementation
 * Item database, rendering, and core functionality
 *
 * Reverse engineered from sa_9061.exe (FUN_0045ffb0, FUN_00463ee0, FUN_00464ee0)
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "item.h"
#include "item_effect.h"
#include "item_protocol.h"
#include "character.h"
#include "render.h"
#include "logger.h"

/* Item database */
Item g_items[MAX_ITEMS] = {0};
u32 g_item_count = 0;

/* Global inventory */
Inventory g_inventory = {0};

/*
 * Initialize item system
 */
int item_init(void) {
    memset(g_items, 0, sizeof(g_items));
    memset(&g_inventory, 0, sizeof(Inventory));
    g_inventory.capacity = 100;

    LOG_INFO("Item system initialized");
    return 1;
}

/*
 * Shutdown item system
 */
void item_shutdown(void) {
    memset(g_items, 0, sizeof(g_items));
    memset(&g_inventory, 0, sizeof(Inventory));
    LOG_INFO("Item system shutdown");
}

/*
 * Get item by ID
 */
Item* item_get(u32 item_id) {
    if (item_id == 0 || item_id >= g_item_count) return NULL;

    /* Linear search - could optimize with hash table */
    u32 i;
    for (i = 0; i < g_item_count; i++) {
        if (g_items[i].id == item_id) {
            return &g_items[i];
        }
    }

    return NULL;
}

/*
 * Load item database
 */
int item_load_database(const char* path) {
    HANDLE hFile;
    DWORD bytes_read;
    u32 count, i;
    Item item;

    hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        LOG_WARN("Failed to load item database: %s", path);
        return 0;
    }

    /* Read count */
    ReadFile(hFile, &count, sizeof(u32), &bytes_read, NULL);
    if (count > MAX_ITEMS) count = MAX_ITEMS;

    /* Read items */
    for (i = 0; i < count; i++) {
        if (!ReadFile(hFile, &item, sizeof(Item), &bytes_read, NULL)) break;
        if (g_items[item.id].id == 0) {
            memcpy(&g_items[item.id], &item, sizeof(Item));
            if (item.id >= g_item_count) {
                g_item_count = item.id + 1;
            }
        }
    }

    CloseHandle(hFile);

    LOG_INFO("Loaded %u items from database", count);
    return 1;
}

/*
 * Render inventory
 */
void inventory_render(void) {
    int x, y, i;

    /* Background */
    render_fill_rect(NULL, 50, 50, 400, 380, 0x0842);
    render_draw_rect(NULL, 50, 50, 400, 380, COLOR_WHITE);

    /* Title */
    render_text(60, 60, "Inventory", COLOR_WHITE);

    /* Gold */
    char text[64];
    snprintf(text, sizeof(text), "Gold: %u", g_inventory.gold);
    render_text(300, 60, text, COLOR_YELLOW);

    /* Items */
    i = 0;
    for (y = 0; y < 10; y++) {
        for (x = 0; x < 10; x++) {
            int slot_x = 60 + x * 38;
            int slot_y = 90 + y * 32;

            if (i < (int)g_inventory.capacity) {
                inventory_render_slot(slot_x, slot_y, &g_inventory.slots[i]);
            }
            i++;
        }
    }
}

/*
 * Render inventory slot
 */
void inventory_render_slot(int x, int y, InventorySlot* slot) {
    Item* item;

    /* Slot background */
    render_fill_rect(NULL, x, y, 34, 28, 0x0421);
    render_draw_rect(NULL, x, y, 34, 28, 0x8410);

    if (!slot || slot->item_id == 0) return;

    item = item_get(slot->item_id);
    if (!item) return;

    /* Render item icon (sprite) */
    render_sprite(item->sprite_id, x + 1, y + 1);

    /* Render count if > 1 */
    if (slot->count > 1) {
        char text[16];
        snprintf(text, sizeof(text), "%d", slot->count);
        render_text(x + 18, y + 16, text, COLOR_WHITE);
    }

    /* Equipped indicator */
    if (slot->equipped) {
        render_draw_rect(NULL, x, y, 34, 28, COLOR_YELLOW);
    }
}

/*
 * Render item tooltip
 */
void inventory_render_tooltip(int x, int y, u32 item_id) {
    Item* item = item_get(item_id);
    char text[128];

    if (!item) return;

    /* Tooltip background */
    render_fill_rect(NULL, x, y, 200, 100, 0x0000);
    render_draw_rect(NULL, x, y, 200, 100, COLOR_WHITE);

    /* Item name */
    render_text(x + 5, y + 5, item->name, COLOR_WHITE);

    /* Type */
    snprintf(text, sizeof(text), "Type: %d", item->type);
    render_text(x + 5, y + 25, text, 0x8410);

    /* Stats */
    if (item->attack > 0) {
        snprintf(text, sizeof(text), "ATK: +%d", item->attack);
        render_text(x + 5, y + 45, text, COLOR_GREEN);
    }
    if (item->defense > 0) {
        snprintf(text, sizeof(text), "DEF: +%d", item->defense);
        render_text(x + 100, y + 45, text, COLOR_GREEN);
    }

    /* Description */
    render_text(x + 5, y + 65, item->description, 0x8410);
}

/*
 * Sort inventory by name
 */
void inventory_sort_by_name(void) {
    u32 i, j;
    InventorySlot temp;

    for (i = 0; i < g_inventory.capacity - 1; i++) {
        for (j = i + 1; j < g_inventory.capacity; j++) {
            Item* item1 = item_get(g_inventory.slots[i].item_id);
            Item* item2 = item_get(g_inventory.slots[j].item_id);

            if (item1 && item2 && strcmp(item1->name, item2->name) > 0) {
                temp = g_inventory.slots[i];
                g_inventory.slots[i] = g_inventory.slots[j];
                g_inventory.slots[j] = temp;
            }
        }
    }
}

/*
 * Sort inventory by rarity
 */
void inventory_sort_by_rarity(void) {
    u32 i, j;
    InventorySlot temp;

    for (i = 0; i < g_inventory.capacity - 1; i++) {
        for (j = i + 1; j < g_inventory.capacity; j++) {
            Item* item1 = item_get(g_inventory.slots[i].item_id);
            Item* item2 = item_get(g_inventory.slots[j].item_id);

            if (item1 && item2 && item1->rarity < item2->rarity) {
                temp = g_inventory.slots[i];
                g_inventory.slots[i] = g_inventory.slots[j];
                g_inventory.slots[j] = temp;
            }
        }
    }
}

/*
 * Check if item can be traded
 */
int inventory_can_trade(u32 item_id) {
    Item* item = item_get(item_id);
    if (!item) return 0;
    return (item->flags & ITEM_FLAG_TRADEABLE) != 0;
}

/*
 * Add item to trade window
 */
int inventory_trade_add(u32 item_id, u16 count) {
    if (!inventory_has_item(item_id, count)) return 0;
    if (!inventory_can_trade(item_id)) return 0;
    /* Trade window management would go here */
    return 1;
}

/*
 * Clear trade window
 */
void inventory_trade_clear(void) {
    /* Clear trade window state */
}

/* ========================================
 * Inventory Management Functions
 * ======================================== */

/*
 * Add item to inventory
 */
int inventory_add_item(u32 item_id, u16 count) {
    int slot;
    Item* item;

    if (item_id == 0 || count == 0) return 0;

    item = item_get(item_id);

    /* Find existing slot for stacking */
    if (item && item->max_stack > 1) {
        slot = inventory_find_slot(item_id);
        if (slot >= 0) {
            u16 new_count = g_inventory.slots[slot].count + count;
            if (new_count > item->max_stack) {
                new_count = item->max_stack;
            }
            g_inventory.slots[slot].count = new_count;
            return 1;
        }
    }

    /* Find empty slot */
    slot = inventory_find_empty_slot();
    if (slot < 0) return 0;  /* Inventory full */

    g_inventory.slots[slot].item_id = item_id;
    g_inventory.slots[slot].count = count;
    g_inventory.slots[slot].equipped = 0;
    g_inventory.slots[slot].flags = 0;
    g_inventory.count++;

    return 1;
}

/*
 * Remove item from inventory
 */
int inventory_remove_item(u32 item_id, u16 count) {
    int slot = inventory_find_slot(item_id);

    if (slot < 0) return 0;

    if (g_inventory.slots[slot].count < count) {
        return 0;  /* Not enough items */
    }

    g_inventory.slots[slot].count -= count;

    if (g_inventory.slots[slot].count == 0) {
        g_inventory.slots[slot].item_id = 0;
        g_inventory.slots[slot].equipped = 0;
        g_inventory.count--;
    }

    return 1;
}

/*
 * Check if inventory has item
 */
int inventory_has_item(u32 item_id, u16 count) {
    u16 total = inventory_get_count(item_id);
    return total >= count;
}

/*
 * Get item count in inventory
 */
u16 inventory_get_count(u32 item_id) {
    u16 total = 0;
    int i;

    for (i = 0; i < (int)g_inventory.capacity; i++) {
        if (g_inventory.slots[i].item_id == item_id) {
            total += g_inventory.slots[i].count;
        }
    }

    return total;
}

/*
 * Find slot containing item
 */
int inventory_find_slot(u32 item_id) {
    int i;

    for (i = 0; i < (int)g_inventory.capacity; i++) {
        if (g_inventory.slots[i].item_id == item_id) {
            return i;
        }
    }

    return -1;
}

/*
 * Find first empty slot
 */
int inventory_find_empty_slot(void) {
    int i;

    for (i = 0; i < (int)g_inventory.capacity; i++) {
        if (g_inventory.slots[i].item_id == 0) {
            return i;
        }
    }

    return -1;
}

/*
 * Equip item from inventory slot
 */
int inventory_equip(u32 slot_index, EquipSlot equip_slot) {
    Item* item;

    if (slot_index >= g_inventory.capacity) return 0;
    if (equip_slot >= EQUIP_SLOT_MAX) return 0;

    item = item_get(g_inventory.slots[slot_index].item_id);
    if (!item) return 0;

    /* Check if item can go in this slot */
    if (!inventory_can_equip(item->id, equip_slot)) return 0;

    /* Unequip current item if any */
    if (g_inventory.equipment[equip_slot].item_id != 0) {
        inventory_unequip(equip_slot);
    }

    /* Move item to equipment slot */
    g_inventory.equipment[equip_slot].item_id = g_inventory.slots[slot_index].item_id;
    g_inventory.equipment[equip_slot].count = 1;
    g_inventory.equipment[equip_slot].equipped = 1;

    /* Clear inventory slot */
    g_inventory.slots[slot_index].item_id = 0;
    g_inventory.slots[slot_index].count = 0;
    g_inventory.count--;

    return 1;
}

/*
 * Unequip item from equipment slot
 */
int inventory_unequip(EquipSlot equip_slot) {
    int empty_slot;

    if (equip_slot >= EQUIP_SLOT_MAX) return 0;
    if (g_inventory.equipment[equip_slot].item_id == 0) return 0;

    /* Find empty inventory slot */
    empty_slot = inventory_find_empty_slot();
    if (empty_slot < 0) return 0;  /* No space */

    /* Move item back to inventory */
    g_inventory.slots[empty_slot].item_id = g_inventory.equipment[equip_slot].item_id;
    g_inventory.slots[empty_slot].count = 1;
    g_inventory.slots[empty_slot].equipped = 0;
    g_inventory.count++;

    /* Clear equipment slot */
    g_inventory.equipment[equip_slot].item_id = 0;
    g_inventory.equipment[equip_slot].equipped = 0;

    return 1;
}

/*
 * Get equipped item
 */
Item* inventory_get_equipped(EquipSlot slot) {
    if (slot >= EQUIP_SLOT_MAX) return NULL;
    return item_get(g_inventory.equipment[slot].item_id);
}

/*
 * Check if item can be equipped in slot
 */
int inventory_can_equip(u32 item_id, EquipSlot slot) {
    Item* item = item_get(item_id);

    if (!item) return 0;

    switch (slot) {
        case EQUIP_SLOT_WEAPON:
            return item->type == ITEM_TYPE_WEAPON;
        case EQUIP_SLOT_ARMOR:
            return item->type == ITEM_TYPE_ARMOR;
        case EQUIP_SLOT_HELMET:
            return item->type == ITEM_TYPE_HELMET;
        case EQUIP_SLOT_ACCESSORY1:
        case EQUIP_SLOT_ACCESSORY2:
            return item->type == ITEM_TYPE_ACCESSORY;
        default:
            return 0;
    }
}

/*
 * Sort inventory by type
 */
void inventory_sort_by_type(void) {
    u32 i, j;
    InventorySlot temp;

    for (i = 0; i < g_inventory.capacity - 1; i++) {
        for (j = i + 1; j < g_inventory.capacity; j++) {
            Item* item1 = item_get(g_inventory.slots[i].item_id);
            Item* item2 = item_get(g_inventory.slots[j].item_id);

            if (item1 && item2 && item1->type > item2->type) {
                temp = g_inventory.slots[i];
                g_inventory.slots[i] = g_inventory.slots[j];
                g_inventory.slots[j] = temp;
            }
        }
    }
}

/*
 * Bank deposit
 */
int inventory_bank_deposit(u32 item_id, u16 count) {
    int inv_slot, bank_slot;

    if (!inventory_has_item(item_id, count)) return 0;

    inv_slot = inventory_find_slot(item_id);
    if (inv_slot < 0) return 0;

    /* Find empty bank slot */
    for (bank_slot = 0; bank_slot < 200; bank_slot++) {
        if (g_inventory.bank[bank_slot].item_id == 0) break;
    }
    if (bank_slot >= 200) return 0;  /* Bank full */

    /* Move to bank */
    g_inventory.bank[bank_slot].item_id = item_id;
    g_inventory.bank[bank_slot].count = count;

    /* Remove from inventory */
    g_inventory.slots[inv_slot].count -= count;
    if (g_inventory.slots[inv_slot].count == 0) {
        g_inventory.slots[inv_slot].item_id = 0;
        g_inventory.count--;
    }

    return 1;
}

/*
 * Bank withdraw
 */
int inventory_bank_withdraw(u32 item_id, u16 count) {
    int bank_slot, inv_slot;

    /* Find in bank */
    for (bank_slot = 0; bank_slot < 200; bank_slot++) {
        if (g_inventory.bank[bank_slot].item_id == item_id &&
            g_inventory.bank[bank_slot].count >= count) break;
    }
    if (bank_slot >= 200) return 0;  /* Not found in bank */

    /* Find inventory slot */
    inv_slot = inventory_find_empty_slot();
    if (inv_slot < 0) return 0;  /* Inventory full */

    /* Move to inventory */
    g_inventory.slots[inv_slot].item_id = item_id;
    g_inventory.slots[inv_slot].count = count;
    g_inventory.count++;

    /* Remove from bank */
    g_inventory.bank[bank_slot].count -= count;
    if (g_inventory.bank[bank_slot].count == 0) {
        g_inventory.bank[bank_slot].item_id = 0;
    }

    return 1;
}

/*
 * Bank gold deposit
 */
int inventory_bank_deposit_gold(u32 amount) {
    if (g_inventory.gold < amount) return 0;

    g_inventory.gold -= amount;
    g_inventory.bank_gold += amount;
    return 1;
}

/*
 * Bank gold withdraw
 */
int inventory_bank_withdraw_gold(u32 amount) {
    if (g_inventory.bank_gold < amount) return 0;

    g_inventory.bank_gold -= amount;
    g_inventory.gold += amount;
    return 1;
}
