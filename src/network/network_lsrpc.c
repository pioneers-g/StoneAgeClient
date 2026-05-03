/*
 * Stone Age Client - LSRPC Encryption Module
 * Split from network.c for code organization
 * Reverse engineered from sa_9061.exe (FUN_0043e320, FUN_0043e810, FUN_0043e540)
 */

#include <windows.h>
#include <string.h>
#include "types.h"
#include "network.h"
#include "logger.h"

/* Base64 alphabet for LSRPC - DAT_004ba01c */
static const char* LSRPC_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* LSRPC key buffer - DAT_004ba008 */
static char s_lsrpc_key[256];

/* LSRPC output buffer - DAT_004ba010 */
static char s_lsrpc_output[4096];

/*
 * LSRPC decode from base64 - FUN_0043e540 pattern
 * Decodes base64-like encoded data using custom alphabet
 */
int lsrpc_decode(const char* input, char* output) {
    int input_pos = 0;
    int output_pos = 0;
    int len = strlen(input);

    while (input_pos < len) {
        u8 b0 = (u8)input[input_pos];
        u8 b1 = (input_pos + 1 < len) ? (u8)input[input_pos + 1] : 0;
        u8 b2 = (input_pos + 2 < len) ? (u8)input[input_pos + 2] : 0;
        u8 b3 = (input_pos + 3 < len) ? (u8)input[input_pos + 3] : 0;

        /* Lookup in alphabet */
        u8 c0 = (b0 >= 0x40) ? LSRPC_ALPHABET[b0 - 0x40] : 0;
        u8 c1 = (b1 >= 0x40) ? LSRPC_ALPHABET[b1 - 0x40] : 0;
        u8 c2 = (b2 >= 0x40) ? LSRPC_ALPHABET[b2 - 0x40] : 0;
        u8 c3 = (b3 >= 0x40) ? LSRPC_ALPHABET[b3 - 0x40] : 0;

        int count = 0;
        if (b1 == 0) {
            c0 = LSRPC_ALPHABET[b0 + 0x40];
            count = 1;
        }
        else if (b2 == 0) {
            c0 = LSRPC_ALPHABET[b0 + 0x40];
            c1 = LSRPC_ALPHABET[b1 + 0x40];
            count = 2;
        }
        else if (b3 == 0) {
            c0 = LSRPC_ALPHABET[b0 + 0x40];
            c1 = LSRPC_ALPHABET[b1 + 0x40];
            c2 = LSRPC_ALPHABET[b2 + 0x40];
            count = 3;
        }
        else {
            c0 = LSRPC_ALPHABET[b0 + 0x40];
            c1 = LSRPC_ALPHABET[b1 + 0x40];
            c2 = LSRPC_ALPHABET[b2 + 0x40];
            c3 = LSRPC_ALPHABET[b3 + 0x40];
            count = 3;
        }

        if (count >= 1) {
            output[output_pos++] = (c0 << 2) | ((c1 >> 4) & 0x03);
        }
        if (count >= 2) {
            output[output_pos++] = (c1 << 4) | ((c2 >> 2) & 0x0F);
        }
        if (count >= 3) {
            output[output_pos++] = (c2 << 6) | (c3 & 0x3F);
        }

        if (count < 3) {
            break;
        }

        input_pos += 4;

        if (input_pos < len && input[input_pos] == '\0') {
            break;
        }
    }

    output[output_pos] = '\0';
    return output_pos;
}

/*
 * LSRPC decrypt - FUN_0043e810 pattern
 * Decrypts data using position-based XOR and bit flipping
 */
void lsrpc_decrypt(char* data, int len, u32 key, char* output, int* out_len) {
    int i;
    char key_char;
    int key_idx;

    *out_len = len - 1;

    if (len <= 1) {
        return;
    }

    /* Get key character */
    key_idx = key % (len - 1);
    key_char = data[key_idx];

    /* Phase 1: Position-based subtraction */
    for (i = 0; i < len; i++) {
        int threshold = key % (len - 1);

        if (i < threshold) {
            /* Subtract based on position */
            output[i] = data[i] - (char)((i * i) % 3) * key_char;
        }
        else if (i > threshold) {
            /* Subtract with different formula */
            output[i] = data[i] - (char)((i * i) % 7) * key_char;
        }
        else {
            output[i] = data[i];
        }
    }

    /* Phase 2: Bit flipping based on key */
    for (i = 0; i < len - 1; i++) {
        if ((key % 7) == (i % 5)) {
            output[i] = ~output[i];
        }
        else if ((key & 1) == (i & 1)) {
            output[i] = ~output[i];
        }
    }

    output[len - 1] = '\0';
}

/*
 * LSRPC full decryption - FUN_0043e320 pattern
 * First decodes from base64, then decrypts
 */
int lsrpc_decrypt_packet(const char* input, char* output) {
    int decoded_len;
    int decrypted_len;
    u32 key;

    /* Remove trailing newlines/CR */
    char temp[4096];
    strncpy(temp, input, sizeof(temp) - 1);

    int len = strlen(temp);
    if (len >= 2 && (temp[len - 2] == '\n' || temp[len - 2] == '\r')) {
        temp[len - 2] = '\0';
    }
    if (len >= 3 && (temp[len - 3] == '\n' || temp[len - 3] == '\r')) {
        temp[len - 3] = '\0';
    }

    /* Decode base64 */
    decoded_len = lsrpc_decode(temp, s_lsrpc_key);
    s_lsrpc_key[decoded_len] = '\0';

    /* Check encryption flag */
    key = *(u32*)s_lsrpc_key;
    int is_encrypted = (key & 0x80000001);

    if (is_encrypted == 0) {
        /* Not encrypted - direct copy */
        int copy_len = decoded_len - 1;
        if (copy_len < 1) {
            output[0] = '\0';
            return 0;
        }
        memcpy(output, s_lsrpc_key + 1, copy_len);
        output[copy_len] = '\0';
        return copy_len;
    }

    /* Decrypt */
    lsrpc_decrypt(s_lsrpc_key + 1, decoded_len - 1, key, output, &decrypted_len);
    output[decrypted_len] = '\0';

    return decrypted_len;
}

/*
 * Initialize LSRPC output buffer - FUN_0043e170 pattern
 */
void lsrpc_init_buffer(void* buffer) {
    (void)buffer;  /* Unused parameter */
    memset(s_lsrpc_output, 0, sizeof(s_lsrpc_output));
}

/*
 * Get LSRPC output buffer
 */
char* lsrpc_get_output(void) {
    return s_lsrpc_output;
}
