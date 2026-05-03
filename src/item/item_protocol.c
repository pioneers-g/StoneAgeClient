/*
 * Stone Age Client - Item Protocol Implementation
 * Network protocol functions for item operations
 *
 * Item protocol commands from binary analysis:
 * - I|<item_id>|<target>: Use item (0x0049e680: "I|%X|%X")
 * - D|<item_index>|: Drop item (0x004aa1c8: "D|%d|")
 * - B|T|<item_id>: Buy item from shop (0x004b9b9c: "B|T|%d")
 * - B|G|<amount>: Buy gold (0x004c1a68: "B|G|%d")
 * - S|D|<name>|<param1>|<param2>: Sell/deposit (0x004b94cc: "S|D|%s|%d|%d")
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "types.h"
#include "item.h"
#include "pet_protocol.h"
#include "network.h"
#include "logger.h"

/* Forward declaration */
Item* item_get(u32 item_id);

/*
 * Send item protocol packet - helper function
 */
static void item_send_packet(const char* cmd, const char* params) {
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
 * Send formatted item packet - helper function
 */
static void item_send_formatted(const char* fmt, ...) {
    char packet[1024];
    va_list args;

    va_start(args, fmt);
    _vsnprintf(packet, sizeof(packet), fmt, args);
    va_end(args);

    network_send(packet, strlen(packet));
    network_send("\n", 1);
}

/*
 * Use item with network protocol - sends I|<item_id>|<target>
 * Format from 0x0049e680: "I|%X|%X"
 */
int item_use_network(u32 item_id, u32 target_id) {
    char params[64];

    if (item_id == 0) {
        return 0;
    }

    /* Send item use packet: I|<item_id>|<target> */
    _snprintf(params, sizeof(params), "%X|%X", item_id, target_id);
    item_send_packet("I", params);

    LOG_DEBUG("Using item %X on target %X", item_id, target_id);
    return 1;
}

/*
 * Drop item from inventory - sends D|<slot_index>|
 * Format from 0x004aa1c8: "D|%d|"
 */
int item_drop_network(u32 slot_index, u16 count) {
    char params[32];

    if (slot_index >= g_inventory.capacity) {
        return 0;
    }

    if (g_inventory.slots[slot_index].item_id == 0) {
        return 0;
    }

    /* Send drop packet: D|<slot_index>| */
    _snprintf(params, sizeof(params), "%d|", slot_index);
    item_send_packet("D", params);

    /* Clear slot locally */
    g_inventory.slots[slot_index].item_id = 0;
    g_inventory.slots[slot_index].count = 0;
    g_inventory.count--;

    LOG_DEBUG("Dropped item from slot %u", slot_index);
    return 1;
}

/*
 * Buy item from shop - sends B|T|<item_id>
 * Format from 0x004b9b9c: "B|T|%d"
 */
int item_buy_network(u32 item_id, u32 price) {
    char params[32];

    if (item_id == 0) {
        return 0;
    }

    /* Check if player has enough gold */
    if (g_inventory.gold < price) {
        LOG_WARN("Not enough gold to buy item");
        return 0;
    }

    /* Send buy packet: B|T|<item_id> */
    _snprintf(params, sizeof(params), "T|%d", item_id);
    item_send_packet("B", params);

    LOG_DEBUG("Buying item %u for %u gold", item_id, price);
    return 1;
}

/*
 * Sell item to shop - sends S|D|<name>|<param1>|<param2>
 * Format from 0x004b94cc: "S|D|%s|%d|%d"
 */
int item_sell_network(u32 slot_index, u32 price) {
    char params[128];
    Item* item;

    if (slot_index >= g_inventory.capacity) {
        return 0;
    }

    item = item_get(g_inventory.slots[slot_index].item_id);
    if (!item) {
        return 0;
    }

    /* Send sell packet: S|D|<name>|<slot>|<count> */
    _snprintf(params, sizeof(params), "D|%s|%d|%d",
             item->name, slot_index, g_inventory.slots[slot_index].count);
    item_send_packet("S", params);

    LOG_DEBUG("Selling item %s from slot %u", item->name, slot_index);
    return 1;
}

/*
 * Request inventory from server
 */
void item_request_inventory(void) {
    item_send_packet("IR", "");
    LOG_DEBUG("Requested inventory");
}

/*
 * Move item between slots
 */
int item_move_slot(u32 from_slot, u32 to_slot) {
    char params[32];

    if (from_slot >= g_inventory.capacity || to_slot >= g_inventory.capacity) {
        return 0;
    }

    if (g_inventory.slots[from_slot].item_id == 0) {
        return 0;
    }

    /* Send move packet: IM|<from>|<to> */
    _snprintf(params, sizeof(params), "M|%d|%d", from_slot, to_slot);
    item_send_packet("I", params);

    /* Move locally */
    InventorySlot temp = g_inventory.slots[to_slot];
    g_inventory.slots[to_slot] = g_inventory.slots[from_slot];
    g_inventory.slots[from_slot] = temp;

    LOG_DEBUG("Moved item from slot %u to %u", from_slot, to_slot);
    return 1;
}

/*
 * Split item stack
 */
int item_split_stack(u32 slot_index, u16 count) {
    char params[32];
    int empty_slot;

    if (slot_index >= g_inventory.capacity) {
        return 0;
    }

    if (g_inventory.slots[slot_index].count <= count) {
        return 0;  /* Can't split if count is equal or less */
    }

    empty_slot = inventory_find_empty_slot();
    if (empty_slot < 0) {
        return 0;
    }

    /* Send split packet: IS|<slot>|<count> */
    _snprintf(params, sizeof(params), "S|%d|%d", slot_index, count);
    item_send_packet("I", params);

    /* Split locally */
    g_inventory.slots[empty_slot].item_id = g_inventory.slots[slot_index].item_id;
    g_inventory.slots[empty_slot].count = count;
    g_inventory.slots[slot_index].count -= count;
    g_inventory.count++;

    LOG_DEBUG("Split %d items from slot %u to %u", count, slot_index, empty_slot);
    return 1;
}

/*
 * Equip item network command
 */
int item_equip_network(u32 slot_index, EquipSlot equip_slot) {
    char params[32];

    /* First equip locally */
    if (!inventory_equip(slot_index, equip_slot)) {
        return 0;
    }

    /* Send equip packet: IE|<slot>|<equip_slot> */
    _snprintf(params, sizeof(params), "E|%d|%d", slot_index, equip_slot);
    item_send_packet("I", params);

    LOG_DEBUG("Equipped item from slot %u to equip slot %d", slot_index, equip_slot);
    return 1;
}

/*
 * Unequip item network command
 */
int item_unequip_network(EquipSlot equip_slot) {
    char params[32];

    /* First unequip locally */
    if (!inventory_unequip(equip_slot)) {
        return 0;
    }

    /* Send unequip packet: IU|<equip_slot> */
    _snprintf(params, sizeof(params), "U|%d", equip_slot);
    item_send_packet("I", params);

    LOG_DEBUG("Unequipped item from equip slot %d", equip_slot);
    return 1;
}

/*
 * Parse inventory list from packet - FUN_0045ffb0 case 0x42
 * Packet format: 0x42<char_index>|field1|field2|...| per item (14 fields per item)
 * char_index: ASCII '0'-'9' for character selection
 *
 * Memory layout from Ghidra:
 * - Base: DAT_046274d4 + (char_index - '0') * 0xb18 + item_index * 0x184
 * - active (offset 0xDC): 1 if item exists
 * - name (offset 0xE6): max 0x1d bytes
 * - short_field (offset 0x103): max 0x11 bytes
 * - description (offset 0x114): max 0x55 bytes
 * - count (offset 0x122): u16
 * - level (offset 0x124): u16 (if >= 100, store % 100 and set has_level)
 * - has_level (offset 0x126): u16 flag
 * - sprite_id (offset 0x128): u16
 */
void item_parse_inventory_list(const char* packet_data, int char_index) {
    const char* ptr = packet_data;
    char field_buffer[256];
    int field_index;
    int item_index;
    s32 int_val;
    s16 level_val;
    size_t name_len;

    if (!packet_data) return;
    if (char_index < '0' || char_index > '9') return;

    /* Skip header bytes (0x42, char_index) - ptr already points to data */
    /* field_index starts at 2 in the binary (local_37c = 0x2) */
    /* But field parsing uses field_index - 1 for name, so first field is 1 */
    field_index = 2;

    /* Parse up to 7 items per character */
    for (item_index = 0; item_index < MAX_INVENTORY_SLOTS; item_index++) {
        /* Field 1 (field_index - 1 = 1): Item name (max 0x1d = 29 bytes) */
        if (pet_parse_field(ptr, '|', field_index - 1, field_buffer, sizeof(field_buffer)) != 0) {
            /* Failed to parse - treat as empty slot */
            g_inventory.slots[item_index].item_id = 0;
            g_inventory.slots[item_index].count = 0;
            field_index += 14;
            continue;
        }

        pet_unescape_string(field_buffer);

        /* Check if name is empty (empty slot) */
        name_len = strlen(field_buffer);
        if (name_len == 0 || name_len >= MAX_ITEM_NAME_LEN) {
            /* Empty slot - clear it */
            g_inventory.slots[item_index].item_id = 0;
            g_inventory.slots[item_index].count = 0;
            field_index += 14;
            continue;
        }

        /* Item exists - set active flag */
        g_inventory.slots[item_index].item_id = 1;  /* Placeholder - real ID from field 3 */

        /* Field 2: Short field (max 0x11 = 17 bytes) */
        pet_parse_field(ptr, '|', field_index, field_buffer, sizeof(field_buffer));
        pet_unescape_string(field_buffer);
        /* Store short_field - would be stored at DAT_046275d7 offset */

        /* Field 3: Integer value - item database ID */
        int_val = pet_parse_field_int(ptr, '|', field_index + 1);
        if (int_val < 0) int_val = 0;
        /* Store as item_id if this is the database ID */
        g_inventory.slots[item_index].item_id = (int_val > 0) ? (u32)int_val : 1;

        /* Field 4: Description string (max 0x55 = 85 bytes) */
        pet_parse_field(ptr, '|', field_index + 2, field_buffer, sizeof(field_buffer));
        pet_unescape_string(field_buffer);
        /* Store description - would be stored at DAT_046275e8 offset */

        /* Field 5: Integer value */
        int_val = pet_parse_field_int(ptr, '|', field_index + 3);

        /* Field 6: Count (u16) - stored at DAT_046275b2 */
        int_val = pet_parse_field_int(ptr, '|', field_index + 4);
        if (int_val < 0) int_val = 0;
        g_inventory.slots[item_index].count = (u16)int_val;

        /* Field 7: Level (s16) - stored at DAT_046275b4 */
        level_val = (s16)pet_parse_field_int(ptr, '|', field_index + 5);
        if (level_val >= 100) {
            /* Level >= 100: store level % 100 and set has_level flag */
            /* has_level stored at DAT_046275b6 */
            level_val = level_val % 100;
        }

        /* Field 8: Integer value */
        int_val = pet_parse_field_int(ptr, '|', field_index + 6);

        /* Field 9: Sprite ID (u16) - stored at DAT_046275b8 */
        int_val = pet_parse_field_int(ptr, '|', field_index + 7);
        g_inventory.slots[item_index].flags = (u8)(int_val & 0xFF);

        /* Field 10: String field (max 0x11 = 17 bytes) */
        pet_parse_field(ptr, '|', field_index + 8, field_buffer, sizeof(field_buffer));
        pet_unescape_string(field_buffer);
        /* Store field_8 - would be stored at DAT_0462763d offset */

        /* Field 11: Integer value */
        int_val = pet_parse_field_int(ptr, '|', field_index + 9);

        /* Field 12: String field */
        pet_parse_field(ptr, '|', field_index + 10, field_buffer, sizeof(field_buffer));
        pet_unescape_string(field_buffer);
        /* Store field_10 - would be stored at DAT_046274e4 offset */

        /* Field 13: Integer value */
        int_val = pet_parse_field_int(ptr, '|', field_index + 11);

        /* Field 14: String field */
        pet_parse_field(ptr, '|', field_index + 12, field_buffer, sizeof(field_buffer));
        pet_unescape_string(field_buffer);
        /* Store at DAT_0462764e offset */

        g_inventory.slots[item_index].equipped = 0;

        LOG_DEBUG("Parsed item %d: count=%u, level=%d",
                  item_index, g_inventory.slots[item_index].count, level_val);

        /* Move to next item (14 fields per item) */
        field_index += 14;
    }

    /* Count total items */
    g_inventory.count = 0;
    for (item_index = 0; item_index < MAX_INVENTORY_SLOTS; item_index++) {
        if (g_inventory.slots[item_index].item_id != 0) {
            g_inventory.count++;
        }
    }
}

/*
 * Handle item pickup from server
 * Called when server sends PKT_SV_ITEM_PICKUP (0x2E)
 */
void item_handle_pickup(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 item_id;
    u16 count;
    u16 sprite_id;
    char name[32];

    if (size < 8) return;

    item_id = *(u32*)ptr; ptr += 4;
    count = *(u16*)ptr; ptr += 2;
    sprite_id = *(u16*)ptr; ptr += 2;

    /* Optional name string */
    if (ptr < (u8*)data + size) {
        strncpy(name, (char*)ptr, sizeof(name) - 1);
    } else {
        name[0] = '\0';
    }

    /* Add to inventory */
    inventory_add_item(item_id, count);

    LOG_INFO("Picked up %s (ID: %u, Count: %u)", name[0] ? name : "item", item_id, count);
}

/*
 * Handle item drop from server
 * Called when server sends PKT_SV_ITEM_DROP (0x2F)
 */
void item_handle_drop(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u16 x, y;
    u32 item_id;
    u16 count;
    u16 sprite_id;

    if (size < 12) return;

    x = *(u16*)ptr; ptr += 2;
    y = *(u16*)ptr; ptr += 2;
    item_id = *(u32*)ptr; ptr += 4;
    count = *(u16*)ptr; ptr += 2;
    sprite_id = *(u16*)ptr; ptr += 2;

    /* Create ground item - would be handled by map system */
    LOG_DEBUG("Item dropped at (%u, %u): ID=%u, Count=%u", x, y, item_id, count);
}

/*
 * Handle inventory update from server
 * Called when server sends PKT_SV_INVENTORY_UPDATE (0x31)
 */
void item_handle_inventory_update(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u16 slot_index;
    u32 item_id;
    u16 count;
    u8 flags;

    if (size < 9) return;

    slot_index = *(u16*)ptr; ptr += 2;
    item_id = *(u32*)ptr; ptr += 4;
    count = *(u16*)ptr; ptr += 2;
    flags = *ptr++;

    if (slot_index < g_inventory.capacity) {
        if (item_id == 0) {
            /* Clear slot */
            g_inventory.slots[slot_index].item_id = 0;
            g_inventory.slots[slot_index].count = 0;
            g_inventory.slots[slot_index].flags = 0;
        } else {
            /* Update slot */
            g_inventory.slots[slot_index].item_id = item_id;
            g_inventory.slots[slot_index].count = count;
            g_inventory.slots[slot_index].flags = flags;
        }
    }

    LOG_DEBUG("Inventory slot %u updated: item=%u, count=%u", slot_index, item_id, count);
}

/*
 * Handle item use result from server
 * Called when server sends PKT_SV_ITEM_USE_RESULT (0x33)
 */
void item_handle_use_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u16 result;
    u16 item_id;
    u32 target_id;
    s16 effect_value;

    if (size < 10) return;

    result = *(u16*)ptr; ptr += 2;
    item_id = *(u16*)ptr; ptr += 2;
    target_id = *(u32*)ptr; ptr += 4;
    effect_value = *(s16*)ptr;

    if (result == 0) {
        /* Success */
        LOG_INFO("Used item %u on target %u, effect: %d", item_id, target_id, effect_value);
    } else {
        /* Failed */
        LOG_WARN("Failed to use item %u, result: %d", item_id, result);
    }
}

/*
 * Handle equip result from server
 * Called when server sends PKT_SV_EQUIP_RESULT (0x35)
 */
void item_handle_equip_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u16 result;
    u16 slot_index;
    u16 equip_slot;
    u32 item_id;

    if (size < 10) return;

    result = *(u16*)ptr; ptr += 2;
    slot_index = *(u16*)ptr; ptr += 2;
    equip_slot = *(u16*)ptr; ptr += 2;
    item_id = *(u32*)ptr;

    if (result == 0) {
        /* Success - update local state */
        if (equip_slot < EQUIP_SLOT_MAX) {
            g_inventory.equipment[equip_slot].item_id = item_id;
            g_inventory.equipment[equip_slot].equipped = 1;

            /* Clear source slot */
            if (slot_index < g_inventory.capacity) {
                g_inventory.slots[slot_index].item_id = 0;
                g_inventory.slots[slot_index].count = 0;
            }
        }

        LOG_INFO("Equipped item %u to slot %d", item_id, equip_slot);
    } else {
        LOG_WARN("Failed to equip item, result: %d", result);
    }
}

/*
 * Handle unequip result from server
 * Called when server sends PKT_SV_UNEQUIP_RESULT (0x37)
 */
void item_handle_unequip_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u16 result;
    u16 equip_slot;
    u16 slot_index;
    u32 item_id;

    if (size < 10) return;

    result = *(u16*)ptr; ptr += 2;
    equip_slot = *(u16*)ptr; ptr += 2;
    slot_index = *(u16*)ptr; ptr += 2;
    item_id = *(u32*)ptr;

    if (result == 0) {
        /* Success - update local state */
        if (equip_slot < EQUIP_SLOT_MAX) {
            g_inventory.equipment[equip_slot].item_id = 0;
            g_inventory.equipment[equip_slot].equipped = 0;
        }

        /* Item moved to inventory slot */
        if (slot_index < g_inventory.capacity) {
            g_inventory.slots[slot_index].item_id = item_id;
            g_inventory.slots[slot_index].equipped = 0;
        }

        LOG_INFO("Unequipped item from slot %d to inventory slot %d", equip_slot, slot_index);
    } else {
        LOG_WARN("Failed to unequip item, result: %d", result);
    }
}

/*
 * Handle text protocol item command
 */
void item_handle_text(u32 param) {
    LOG_DEBUG("Item text command: %u", param);
}

/*
 * Handle text protocol item use command
 */
void item_handle_use_text(u32 item_id) {
    LOG_DEBUG("Item use text: %u", item_id);
}

/*
 * Handle text protocol item equip command
 */
void item_handle_equip_text(u32 item_id, u32 slot) {
    LOG_DEBUG("Item equip text: %u -> %u", item_id, slot);
}

/*
 * Handle text protocol item quantity update
 */
void item_handle_quantity_text(u32 item_id, u32 count) {
    int slot = inventory_find_slot(item_id);
    if (slot >= 0) {
        g_inventory.slots[slot].count = (u16)count;
        LOG_DEBUG("Item %u quantity updated to %u", item_id, count);
    }
}
