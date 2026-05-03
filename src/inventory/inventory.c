/*
 * Stone Age Client - Inventory Management
 * Backpack, equipment, and bank operations
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "item.h"
#include "character.h"
#include "logger.h"

/* Global inventory */
extern Inventory g_inventory;
extern Item g_items[MAX_ITEMS];
extern u32 g_item_count;

/* Forward declaration */
Item* item_get(u32 item_id);

/*
 * Add item to inventory
 */
int inventory_add_item(u32 item_id, u16 count) {
    Item* item = item_get(item_id);
    int slot_index;
    int empty_slot;

    if (!item || count == 0) return 0;

    /* Check if stackable */
    if (item->max_stack > 1) {
        /* Find existing stack */
        slot_index = inventory_find_slot(item_id);
        if (slot_index >= 0) {
            u16 new_count = g_inventory.slots[slot_index].count + count;
            if (new_count <= item->max_stack) {
                g_inventory.slots[slot_index].count = new_count;
                return 1;
            } else {
                /* Split into multiple stacks */
                g_inventory.slots[slot_index].count = item->max_stack;
                count = new_count - item->max_stack;
            }
        }
    }

    /* Find empty slot */
    empty_slot = inventory_find_empty_slot();
    if (empty_slot < 0) {
        LOG_WARN("Inventory full");
        return 0;
    }

    g_inventory.slots[empty_slot].item_id = item_id;
    g_inventory.slots[empty_slot].count = count;
    g_inventory.slots[empty_slot].equipped = 0;
    g_inventory.count++;

    LOG_DEBUG("Added %d x %s to slot %d", count, item->name, empty_slot);
    return 1;
}

/*
 * Remove item from inventory
 */
int inventory_remove_item(u32 item_id, u16 count) {
    int slot_index;

    if (!inventory_has_item(item_id, count)) return 0;

    /* Find and remove from slots */
    while (count > 0) {
        slot_index = inventory_find_slot(item_id);
        if (slot_index < 0) break;

        if (g_inventory.slots[slot_index].count <= count) {
            count -= g_inventory.slots[slot_index].count;
            g_inventory.slots[slot_index].item_id = 0;
            g_inventory.slots[slot_index].count = 0;
            g_inventory.count--;
        } else {
            g_inventory.slots[slot_index].count -= count;
            count = 0;
        }
    }

    return 1;
}

/*
 * Check if inventory has item
 */
int inventory_has_item(u32 item_id, u16 count) {
    return inventory_get_count(item_id) >= count;
}

/*
 * Get item count in inventory
 */
u16 inventory_get_count(u32 item_id) {
    u16 total = 0;
    u32 i;

    for (i = 0; i < g_inventory.capacity; i++) {
        if (g_inventory.slots[i].item_id == item_id) {
            total += g_inventory.slots[i].count;
        }
    }

    return total;
}

/*
 * Find slot with item
 */
int inventory_find_slot(u32 item_id) {
    u32 i;

    for (i = 0; i < g_inventory.capacity; i++) {
        if (g_inventory.slots[i].item_id == item_id) {
            return (int)i;
        }
    }

    return -1;
}

/*
 * Find empty slot
 */
int inventory_find_empty_slot(void) {
    u32 i;

    for (i = 0; i < g_inventory.capacity; i++) {
        if (g_inventory.slots[i].item_id == 0) {
            return (int)i;
        }
    }

    return -1;
}

/*
 * Equip item
 */
int inventory_equip(u32 slot_index, EquipSlot equip_slot) {
    Item* item;
    Item* equipped_item;

    if (slot_index >= g_inventory.capacity) return 0;

    item = item_get(g_inventory.slots[slot_index].item_id);
    if (!item) return 0;

    /* Check if item can be equipped in this slot */
    if (!inventory_can_equip(g_inventory.slots[slot_index].item_id, equip_slot)) {
        return 0;
    }

    /* Unequip current item if any */
    equipped_item = inventory_get_equipped(equip_slot);
    if (equipped_item) {
        inventory_unequip(equip_slot);
    }

    /* Move item to equipment slot */
    g_inventory.equipment[equip_slot] = g_inventory.slots[slot_index];
    g_inventory.equipment[equip_slot].equipped = 1;

    /* Clear inventory slot */
    g_inventory.slots[slot_index].item_id = 0;
    g_inventory.slots[slot_index].count = 0;

    /* Apply stat bonuses */
    item_apply_stat_bonus(item, character_get_player());

    LOG_INFO("Equipped %s", item->name);
    return 1;
}

/*
 * Unequip item
 */
int inventory_unequip(EquipSlot equip_slot) {
    Item* item;
    int empty_slot;

    if (equip_slot >= EQUIP_SLOT_MAX) return 0;
    if (g_inventory.equipment[equip_slot].item_id == 0) return 0;

    /* Find empty inventory slot */
    empty_slot = inventory_find_empty_slot();
    if (empty_slot < 0) {
        LOG_WARN("Inventory full, cannot unequip");
        return 0;
    }

    item = item_get(g_inventory.equipment[equip_slot].item_id);

    /* Remove stat bonuses */
    if (item) {
        item_remove_stat_bonus(item, character_get_player());
    }

    /* Move to inventory */
    g_inventory.slots[empty_slot] = g_inventory.equipment[equip_slot];
    g_inventory.slots[empty_slot].equipped = 0;

    /* Clear equipment slot */
    g_inventory.equipment[equip_slot].item_id = 0;
    g_inventory.equipment[equip_slot].count = 0;

    if (item) {
        LOG_INFO("Unequipped %s", item->name);
    }

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
 * Check if item can be equipped
 */
int inventory_can_equip(u32 item_id, EquipSlot slot) {
    Item* item = item_get(item_id);
    Character* player;

    if (!item) return 0;

    /* Check item type matches slot */
    switch (slot) {
        case EQUIP_SLOT_WEAPON:
            if (item->type != ITEM_TYPE_WEAPON) return 0;
            break;
        case EQUIP_SLOT_ARMOR:
            if (item->type != ITEM_TYPE_ARMOR) return 0;
            break;
        case EQUIP_SLOT_HELMET:
            if (item->type != ITEM_TYPE_HELMET) return 0;
            break;
        case EQUIP_SLOT_ACCESSORY1:
        case EQUIP_SLOT_ACCESSORY2:
            if (item->type != ITEM_TYPE_ACCESSORY) return 0;
            break;
        default:
            return 0;
    }

    /* Check level requirement */
    player = character_get_player();
    if (player && player->stats.level < item->req_level) {
        return 0;
    }

    /* Check stat requirements */
    if (player) {
        if (item->req_strength > 0 && player->stats.strength < item->req_strength) return 0;
        if (item->req_dexterity > 0 && player->stats.dexterity < item->req_dexterity) return 0;
    }

    return 1;
}

/*
 * Sort inventory by type
 */
void inventory_sort_by_type(void) {
    /* Simple bubble sort */
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
    if (!inventory_remove_item(item_id, count)) return 0;

    /* Add to bank */
    u32 i;
    for (i = 0; i < 200; i++) {
        if (g_inventory.bank[i].item_id == item_id) {
            g_inventory.bank[i].count += count;
            return 1;
        }
    }

    for (i = 0; i < 200; i++) {
        if (g_inventory.bank[i].item_id == 0) {
            g_inventory.bank[i].item_id = item_id;
            g_inventory.bank[i].count = count;
            return 1;
        }
    }

    return 0;
}

/*
 * Bank withdraw
 */
int inventory_bank_withdraw(u32 item_id, u16 count) {
    u32 i;

    for (i = 0; i < 200; i++) {
        if (g_inventory.bank[i].item_id == item_id) {
            if (g_inventory.bank[i].count >= count) {
                g_inventory.bank[i].count -= count;
                if (g_inventory.bank[i].count == 0) {
                    g_inventory.bank[i].item_id = 0;
                }
                return inventory_add_item(item_id, count);
            }
        }
    }

    return 0;
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
