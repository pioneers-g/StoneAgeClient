/*
 * Stone Age Client - Item Protocol Interface
 */

#ifndef ITEM_PROTOCOL_H
#define ITEM_PROTOCOL_H

#include "types.h"
#include "item.h"

/* ========================================
 * Network Protocol Functions
 * ======================================== */

/* Send item operations to server */
int item_use_network(u32 item_id, u32 target_id);
int item_drop_network(u32 slot_index, u16 count);
int item_buy_network(u32 item_id, u32 price);
int item_sell_network(u32 slot_index, u32 price);
void item_request_inventory(void);
int item_move_slot(u32 from_slot, u32 to_slot);
int item_split_stack(u32 slot_index, u16 count);
int item_equip_network(u32 slot_index, EquipSlot equip_slot);
int item_unequip_network(EquipSlot equip_slot);

/* ========================================
 * Packet Parsing Functions
 * ======================================== */

/* Parse inventory list from packet - FUN_0045ffb0 case 0x42 */
void item_parse_inventory_list(const char* packet_data, int char_index);

/* Handle server responses */
void item_handle_pickup(void* data, u32 size);
void item_handle_drop(void* data, u32 size);
void item_handle_inventory_update(void* data, u32 size);
void item_handle_use_result(void* data, u32 size);
void item_handle_equip_result(void* data, u32 size);
void item_handle_unequip_result(void* data, u32 size);

/* Text protocol handlers */
void item_handle_text(u32 param);
void item_handle_use_text(u32 item_id);
void item_handle_equip_text(u32 item_id, u32 slot);
void item_handle_quantity_text(u32 item_id, u32 count);

#endif /* ITEM_PROTOCOL_H */
