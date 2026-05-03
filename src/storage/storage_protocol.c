/*
 * Stone Age Client - Storage Protocol Handlers
 * Split from storage.c
 *
 * Contains protocol handlers for storage messages:
 * - Open/close responses
 * - Item list parsing
 * - Deposit/withdraw results
 * - Gold transaction results
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "storage.h"
#include "logger.h"

/* External storage context */
extern StorageContext g_storage;

/*
 * Handle storage open response
 */
void storage_handle_open(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;

    result = *ptr++;

    if (result == 0) {
        g_storage.has_access = 1;
        g_storage.is_open = 1;
        g_storage.max_slots = *(u16*)ptr; ptr += 2;
        g_storage.gold = *(u32*)ptr;

        LOG_INFO("Storage opened, max slots: %d", g_storage.max_slots);
    } else {
        LOG_WARN("Failed to open storage: result %d", result);
    }
}

/*
 * Handle storage item list
 */
void storage_handle_item_list(void* data, u32 size) {
    u8* ptr = (u8*)data;
    StorageSlot* slot;

    memset(g_storage.items, 0, sizeof(g_storage.items));
    g_storage.item_count = 0;

    while (ptr < (u8*)data + size) {
        u16 slot_index = *(u16*)ptr; ptr += 2;
        if (slot_index >= g_storage.max_slots) break;

        slot = &g_storage.items[slot_index];
        slot->item_id = *(u32*)ptr; ptr += 4;
        if (slot->item_id == 0) continue;

        slot->count = *(u16*)ptr; ptr += 2;
        slot->flags = *(u16*)ptr; ptr += 2;
        memcpy(slot->data, ptr, 8); ptr += 8;

        g_storage.item_count++;
    }

    LOG_DEBUG("Storage items loaded: %d", g_storage.item_count);
}

/*
 * Handle deposit result
 */
void storage_handle_deposit_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;
    u32 item_id;
    u16 count;
    u16 slot_index;

    result = *ptr++;
    item_id = *(u32*)ptr; ptr += 4;
    count = *(u16*)ptr; ptr += 2;

    if (result == 0) {
        slot_index = *(u16*)ptr;

        if (slot_index < g_storage.max_slots) {
            StorageSlot* slot = &g_storage.items[slot_index];
            slot->item_id = item_id;
            slot->count += count;

            if (slot->count == count) {
                g_storage.item_count++;
            }
        }

        LOG_DEBUG("Deposited item %u x %d to slot %d", item_id, count, slot_index);
    } else {
        LOG_WARN("Failed to deposit item: result %d", result);
    }
}

/*
 * Handle withdraw result
 */
void storage_handle_withdraw_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;
    u16 slot_index;
    u16 count;

    result = *ptr++;
    slot_index = *(u16*)ptr; ptr += 2;
    count = *(u16*)ptr;

    if (result == 0) {
        if (slot_index < g_storage.max_slots) {
            StorageSlot* slot = &g_storage.items[slot_index];
            slot->count -= count;

            if (slot->count == 0) {
                memset(slot, 0, sizeof(StorageSlot));
                g_storage.item_count--;
            }
        }

        LOG_DEBUG("Withdrew %d items from slot %d", count, slot_index);
    } else {
        LOG_WARN("Failed to withdraw item: result %d", result);
    }
}

/*
 * Handle gold transaction result
 */
void storage_handle_gold_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;
    u8 is_deposit;
    u32 amount;

    result = *ptr++;
    is_deposit = *ptr++;
    amount = *(u32*)ptr;

    if (result == 0) {
        if (is_deposit) {
            g_storage.gold += amount;
            LOG_DEBUG("Deposited %u gold, total: %u", amount, g_storage.gold);
        } else {
            g_storage.gold -= amount;
            LOG_DEBUG("Withdrew %u gold, remaining: %u", amount, g_storage.gold);
        }
    } else {
        LOG_WARN("Gold transaction failed: result %d", result);
    }
}

/*
 * Handle storage close response
 */
void storage_handle_close(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result = *ptr;

    if (result == 0) {
        g_storage.is_open = 0;
        g_storage.has_access = 0;
        g_storage.selected_index = -1;
        g_storage.dirty = 0;

        LOG_DEBUG("Storage closed successfully");
    } else {
        LOG_WARN("Failed to close storage: result %d", result);
    }
}

/*
 * Handle storage expansion result
 */
void storage_handle_expansion_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;
    u16 new_max_slots;

    result = *ptr++;

    if (result == 0) {
        new_max_slots = *(u16*)ptr;
        g_storage.max_slots = new_max_slots;
        g_storage.expansion_level++;

        LOG_INFO("Storage expanded to %d slots", new_max_slots);
    } else {
        LOG_WARN("Storage expansion failed: result %d", result);
    }
}

/*
 * Handle storage error message
 */
void storage_handle_error(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 error_code;
    char message[256];

    error_code = *ptr++;

    if (ptr < (u8*)data + size) {
        strncpy(message, (char*)ptr, sizeof(message) - 1);
        message[sizeof(message) - 1] = '\0';
    } else {
        message[0] = '\0';
    }

    LOG_WARN("Storage error %d: %s", error_code, message);
}
