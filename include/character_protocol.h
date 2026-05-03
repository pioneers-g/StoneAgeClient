/*
 * Stone Age Client - Character Protocol Parsing Header
 * Reverse engineered from sa_9061.exe (FUN_0045ffb0, FUN_00464670, FUN_00464190)
 */

#ifndef CHARACTER_PROTOCOL_H
#define CHARACTER_PROTOCOL_H

#include "types.h"
#include "character.h"

/*
 * Protocol parsing functions for character data
 */

/*
 * Parse character list from packet - FUN_00464670
 * Packet format: |char_id|name|field3|...|fieldN|
 */
void character_parse_list(const char* packet_data);

/*
 * Handle character teleport - FUN_00464190
 */
void character_teleport(u32 char_index, const char* packet_data);

/*
 * Handle move result - FUN_00465170
 */
void character_handle_move_result(u32 char_index, const char* packet_data);

/*
 * Get character entry by index
 */
CharacterEntry* character_get_entry(u32 index);

/*
 * Parse detailed character stats - FUN_0045ffb0 case 0x50
 * Packet format: 0x50|bitmask|fields...
 * Uses bitmask to determine which fields are present
 */
void character_parse_detailed_stats(const char* packet_data);

/* Bitmask field indices for case 0x50 */
#define STATS_FIELD_02      (1 << 1)    /* 0x00000002 */
#define STATS_FIELD_03      (1 << 2)    /* 0x00000004 */
#define STATS_FIELD_04      (1 << 3)    /* 0x00000008 */
#define STATS_FIELD_05      (1 << 4)    /* 0x00000010 */
#define STATS_FIELD_06      (1 << 5)    /* 0x00000020 */
#define STATS_FIELD_07      (1 << 6)    /* 0x00000040 */
#define STATS_FIELD_08      (1 << 7)    /* 0x00000080 */
#define STATS_FIELD_09      (1 << 8)    /* 0x00000100 */
#define STATS_FIELD_10      (1 << 9)    /* 0x00000200 */
#define STATS_FIELD_11      (1 << 10)   /* 0x00000400 */
#define STATS_FIELD_12      (1 << 11)   /* 0x00000800 */
#define STATS_FIELD_13      (1 << 12)   /* 0x00001000 */
#define STATS_FIELD_14      (1 << 13)   /* 0x00002000 */
#define STATS_FIELD_15      (1 << 14)   /* 0x00004000 */
#define STATS_FIELD_16      (1 << 15)   /* 0x00008000 */
#define STATS_FIELD_17      (1 << 16)   /* 0x00010000 */
#define STATS_FIELD_18      (1 << 17)   /* 0x00020000 */
#define STATS_FIELD_19      (1 << 18)   /* 0x00040000 */
#define STATS_FIELD_20      (1 << 19)   /* 0x00080000 */
#define STATS_FIELD_21      (1 << 20)   /* 0x00100000 */
#define STATS_FIELD_22      (1 << 21)   /* 0x00200000 */
#define STATS_FIELD_23      (1 << 22)   /* 0x00400000 */
#define STATS_FIELD_24      (1 << 23)   /* 0x00800000 */
#define STATS_FIELD_25      (1 << 24)   /* 0x01000000 */
#define STATS_FIELD_26_STR  (1 << 25)   /* 0x02000000 - string field */
#define STATS_FIELD_27_STR  (1 << 26)   /* 0x04000000 - string field */
#define STATS_FIELD_28      (1 << 27)   /* 0x08000000 */
#define STATS_FIELD_29      (1 << 28)   /* 0x10000000 */
#define STATS_FIELD_30      (1 << 29)   /* 0x20000000 */
#define STATS_FIELD_31      (1 << 30)   /* 0x40000000 */

/*
 * Update character positions from server
 * Called when server sends PKT_SV_CHAR_POSITIONS (0x1C)
 */
void character_update_positions(void* data, u32 size);

/*
 * Handle action result from server
 * Called when server sends PKT_SV_ACTION_RESULT (0x27)
 */
void character_handle_action_result(u32 char_id, u16 action, u16 param1, u16 param2, u16 param3, u32 target_id);

/*
 * Gain experience points
 */
void character_gain_exp(u32 exp);

/*
 * Text protocol handlers
 */
void character_handle_text_update(u32 char_id, const char* name, u32 param);
void character_handle_position_text(u32 char_id, u16 x, u16 y, u16 dir, u16 action, u16 sprite, u16 param6, u16 param7);
void character_handle_teleport_text(u32 char_id, const char* map_name, u16 param);
void character_handle_create_text(u32 param1, u32 param2, u32 param3, u32 param4,
                                   u32 param5, u32 param6, u32 param7, u32 param8,
                                   const char* name);
void character_handle_status_text(const char* status_data);

#endif /* CHARACTER_PROTOCOL_H */
