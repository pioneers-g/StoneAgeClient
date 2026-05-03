/*
 * Stone Age Client - Battle Protocol Header
 * Split from battle.h for code organization
 *
 * Handles text protocol battle commands
 */

#ifndef BATTLE_PROTOCOL_H
#define BATTLE_PROTOCOL_H

#include <winsock2.h>
#include "types.h"

/* Text protocol handlers */
void battle_handle_text_start(u32 param1, u32 param2, u32 param3);
void battle_handle_action_text(u32 attacker, u32 action, u32 target, u32 param, const char* extra);
void battle_handle_turn_text(u32 turn, u32 param2, u32 param3, u32 param4);
void battle_handle_end_text(u32 result, u32 exp, u32 gold, u32 items);
void battle_handle_capture_text(u32 pet_id, u32 result);
void battle_handle_damage_text(const char* damage_data);
void battle_handle_level_text(const char* level_data);
void battle_handle_exp_text(const char* exp_data);
void battle_handle_magic_text(u32 skill_id, u32 param);
void battle_handle_turn_begin_text(u32 turn);
void battle_handle_counter_text(u32 attacker, u32 target, const char* extra);
void battle_handle_text_command(u32 param1, u32 param2);

/* Command sending */
void battle_send_text_command(SOCKET sock, u32 x, u32 y, u32 param1, u32 param2, u32 flags, const char* extra);
void battle_send_binary_command(SOCKET sock, u32 opcode, u32 param1, u32 param2,
                                 u32 param3, u32 param4, u32 param5, u32 checksum);

#endif /* BATTLE_PROTOCOL_H */
