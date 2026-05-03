/*
 * Stone Age Client - Shop Protocol Header
 * Protocol handling functions for shop system
 */

#ifndef SHOP_PROTOCOL_H
#define SHOP_PROTOCOL_H

#include "types.h"

/* Protocol formats:
 * B|T|%d - Buy item
 * B|G|%d - Buy gold
 * S|D|%s|%d|%d - Sell item
 */

/* Network request functions */
int shop_buy_network(u32 item_id, u32 price);
int shop_sell_network(u32 slot_index, u16 count);
int shop_buy_gold_network(u32 amount);

/* Packet handlers */
void shop_handle_open(void* data, u32 size);
void shop_handle_item_list(void* data, u32 size);
void shop_handle_buy_result(void* data, u32 size);
void shop_handle_sell_result(void* data, u32 size);

/* Main packet dispatcher */
void shop_process_packet(const char* data, u32 size);

#endif /* SHOP_PROTOCOL_H */
