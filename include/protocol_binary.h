/*
 * Stone Age Client - Binary Protocol Dispatcher
 * Reverse engineered from sa_9061.exe (FUN_0048d3e0)
 */

#ifndef PROTOCOL_BINARY_H
#define PROTOCOL_BINARY_H

#include "types.h"
#include "protocol.h"

/* Binary packet dispatcher - FUN_0048d3e0 pattern */
int packet_dispatch(u16 opcode, void* data, u32 size);

/* Binary packet dispatcher for network buffer */
void packet_dispatch_binary(SOCKET sock);

/* Packet handlers */
void packet_handle_login_ok(void* data, int size);
void packet_handle_login_fail(void* data, int size);
void packet_handle_char_list(void* data, int size);
void packet_handle_map_data(void* data, int size);
void packet_handle_battle_start(void* data, int size);

#endif /* PROTOCOL_BINARY_H */
