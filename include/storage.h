/*
 * Stone Age Client - Storage/Warehouse System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef STORAGE_H
#define STORAGE_H

#include "types.h"

/* Constants */
#define MAX_STORAGE_ITEMS   100
#define MAX_GOLD_STORAGE    100000000
#define STORAGE_FEE         100

/* Storage slot */
typedef struct {
    u32 item_id;
    u16 count;
    u16 flags;
    u8  data[8];    /* Item-specific data */
} StorageSlot;

/* Storage category */
typedef enum {
    STORAGE_CAT_ALL = 0,
    STORAGE_CAT_WEAPON = 1,
    STORAGE_CAT_ARMOR = 2,
    STORAGE_CAT_CONSUMABLE = 3,
    STORAGE_CAT_MATERIAL = 4,
    STORAGE_CAT_QUEST = 5,
    STORAGE_CAT_OTHER = 6
} StorageCategory;

/* Storage context */
typedef struct {
    /* Items */
    StorageSlot items[MAX_STORAGE_ITEMS];
    int item_count;
    int max_slots;          /* Can be expanded */

    /* Gold */
    u32 gold;

    /* UI state */
    int selected_index;
    int scroll_offset;
    StorageCategory filter;
    char search_text[32];

    /* Access */
    u8 is_open;
    u8 has_access;
    u32 access_fee;

    /* Sorting */
    u8 sort_mode;
    u8 sort_ascending;

    /* Expansion */
    u8 expansion_level;
    u32 expansion_cost;

    /* Dirty flag */
    u8 dirty;

} StorageContext;

/* Global storage context */
extern StorageContext g_storage;

/* Initialization */
int storage_init(void);
void storage_shutdown(void);

/* Access */
int storage_open(void);
void storage_close(void);
int storage_is_open(void);
int storage_can_access(void);

/* Items */
int storage_deposit_item(u32 item_id, u16 count);
int storage_withdraw_item(int slot, u16 count);
int storage_get_item_count(int slot);
u32 storage_get_item_id(int slot);
int storage_find_item(u32 item_id);
int storage_find_free_slot(void);

/* Gold */
int storage_deposit_gold(u32 amount);
int storage_withdraw_gold(u32 amount);
u32 storage_get_gold(void);
u32 storage_get_max_gold(void);

/* Sorting/filtering */
void storage_set_filter(StorageCategory category);
void storage_set_search(const char* text);
void storage_sort(int mode, int ascending);
int storage_get_filtered_count(void);
StorageSlot* storage_get_filtered_item(int index);

/* Packet handlers */
void storage_handle_open(void* data, u32 size);
void storage_handle_item_list(void* data, u32 size);
void storage_handle_deposit_result(void* data, u32 size);
void storage_handle_withdraw_result(void* data, u32 size);
void storage_handle_gold_result(void* data, u32 size);

/* Expansion */
int storage_expand(void);
int storage_get_expansion_cost(void);
int storage_get_max_slots(void);

/* Query */
int storage_get_item_count_total(void);
int storage_has_space_for(u32 item_id, u16 count);
int storage_get_used_slots(void);

/* Selection */
void storage_select(int index);
int storage_get_selected(void);
StorageSlot* storage_get_selected_item(void);

/* Quick operations */
int storage_deposit_all(u32 item_id);
int storage_withdraw_all(int slot);
int storage_swap_with_inventory(int inv_slot, int storage_slot);

/* Utility */
void storage_refresh(void);
void storage_clear(void);

#endif /* STORAGE_H */
