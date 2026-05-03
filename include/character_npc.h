/*
 * Stone Age Client - Character NPC Entry System Header
 * Reverse engineered from sa_9061.exe (FUN_0040ee70, FUN_0040f310, FUN_0040e830)
 */

#ifndef CHARACTER_NPC_H
#define CHARACTER_NPC_H

#include "types.h"
#include "character.h"  /* For NPCEntry type */

/*
 * NPC entry functions - from FUN_0040ee70, FUN_0040f310, FUN_0040e830
 *
 * These functions manage the NPCEntry array (DAT_004e2b20 region)
 * separate from the simple NPC list in CharacterContext.
 */

/*
 * Create NPC entry - FUN_0040ee70
 * Returns 1 on success, 0 on failure
 */
int npc_create_entry(u32 param1, u32 param2, u32 param3, u32 param4, u32 param5,
                     const char* name, const char* description, u32 param8,
                     u16 param9, int flag_0x40, int flag_0x80, u32 npc_type);

/*
 * Find NPC by ID (internal) - FUN_0040e830
 * Returns slot index or -1 if not found
 */
int npc_find_by_id_internal(u32 npc_id);

/*
 * Get NPC entry by index
 */
NPCEntry* npc_get_entry(u32 index);

/*
 * Get NPC sprite ID by ID - FUN_0040f460
 */
u16 npc_get_sprite_by_id(u32 npc_id);

/*
 * Remove NPC entry
 */
void npc_remove_entry(u32 npc_id);

/*
 * Update NPC position
 */
void npc_set_position(u32 npc_id, u16 x, u16 y);

/*
 * Render all NPC entries
 */
void npc_render_entries(void);

/*
 * Parse shop items - FUN_00463380
 * Parses comma-separated item IDs
 */
void npc_parse_shop_items(const char* item_list);

#endif /* CHARACTER_NPC_H */
