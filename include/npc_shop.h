/*
 * Stone Age Client - NPC Shop System Header
 * Split from npc.h for code organization
 */

#ifndef NPC_SHOP_H
#define NPC_SHOP_H

#include "types.h"
#include "npc.h"

/* Shop system functions */
int npc_open_shop(u32 npc_id, const char* shop_name);
void npc_close_shop(void);
int npc_add_shop_item(u32 item_id, u32 price, u32 stock);
int npc_find_shop_item(u32 item_id);
void npc_update_shop(void);
int npc_buy_item(u32 item_id, u32 count);
int npc_sell_item(u32 item_id, u32 count);

/* Packet handlers */
void npc_handle_shop_packet(void* data, u32 size);

#endif /* NPC_SHOP_H */
