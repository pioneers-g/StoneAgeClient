/*
 * Stone Age Client - LSRPC Encryption Header
 * Split from network.h for code organization
 */

#ifndef NETWORK_LSRPC_H
#define NETWORK_LSRPC_H

#include "types.h"

/*
 * LSRPC encryption/decryption - FUN_0043e320, FUN_0043e810, FUN_0043e540
 */

/* Decode base64-like encoded data */
int lsrpc_decode(const char* input, char* output);

/* Decrypt data using position-based XOR and bit flipping */
void lsrpc_decrypt(char* data, int len, u32 key, char* output, int* out_len);

/* Full decryption: decode base64 then decrypt */
int lsrpc_decrypt_packet(const char* input, char* output);

/* Initialize LSRPC output buffer */
void lsrpc_init_buffer(void* buffer);

/* Get LSRPC output buffer */
char* lsrpc_get_output(void);

#endif /* NETWORK_LSRPC_H */
