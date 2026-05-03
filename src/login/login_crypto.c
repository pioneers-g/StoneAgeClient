/*
 * Stone Age Client - Login Crypto Module
 * Custom DES-like block cipher implementation
 * Reverse engineered from FUN_0048b620, FUN_0048b7c0, FUN_0048bb90
 *
 * Key: "f;encor1c" at 0x0049e48c (9 bytes, truncated/padded to 8)
 *
 * This is NOT standard DES - uses custom S-boxes extracted from binary
 */

#include <string.h>
#include "types.h"
#include "login_crypto.h"

/* Rotation schedule - from DAT_004d5860 (16 entries) */
static const u8 s_rotation_schedule[16] = {
    1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1
};

/* PC-1 permutation table - from DAT_004d5870 (64 entries) */
static const u8 s_pc1_table[64] = {
    0x30, 0x28, 0x20, 0x00, 0x04, 0x0c, 0x14, 0x31,
    0x29, 0x21, 0x01, 0x05, 0x0d, 0x15, 0x32, 0x2a,
    0x22, 0x02, 0x06, 0x0e, 0x16, 0x33, 0x2b, 0x23,
    0x03, 0x07, 0x0f, 0x17, 0x34, 0x2c, 0x24, 0x1c,
    0x08, 0x10, 0x18, 0x35, 0x2d, 0x25, 0x1d, 0x09,
    0x11, 0x19, 0x36, 0x2e, 0x26, 0x1e, 0x0a, 0x12,
    0x1a, 0x37, 0x2f, 0x27, 0x1f, 0x0b, 0x13, 0x1b,
    0x0e, 0x0b, 0x11, 0x04, 0x1b, 0x17, 0x19, 0x00
};

/* PC-2 permutation tables - from DAT_004d58a8 and DAT_004d58c0 */
static const u8 s_pc2_c[32] = {
    0x0e, 0x0b, 0x11, 0x04, 0x1b, 0x17, 0x19, 0x00,
    0x12, 0x05, 0x15, 0x1d, 0x08, 0x1f, 0x10, 0x0f,
    0x18, 0x03, 0x0d, 0x07, 0x16, 0x01, 0x1e, 0x0a,
    0x14, 0x09, 0x13, 0x1a, 0x06, 0x02, 0x1c, 0x0c
};

static const u8 s_pc2_d[32] = {
    0x0e, 0x0b, 0x11, 0x04, 0x1b, 0x17, 0x19, 0x00,
    0x12, 0x05, 0x15, 0x1d, 0x08, 0x1f, 0x10, 0x0f,
    0x18, 0x03, 0x0d, 0x07, 0x16, 0x01, 0x1e, 0x0a,
    0x14, 0x09, 0x13, 0x1a, 0x06, 0x02, 0x1c, 0x0c
};

/* S-box tables - extracted from DAT_004d58d8 through DAT_004d5fd8 */
/* Each table has 64 entries of 32-bit values */
/* TODO: S-box arrays are currently incomplete - only 32 of 64 entries defined.
 * Need to extract complete 64-entry S-boxes from Ghidra memory addresses:
 * S1: 0x004d58d8, S2: 0x004d59d8, S3: 0x004d5ad8, S4: 0x004d5bd8
 * S5: 0x004d5cd8, S6: 0x004d5dd8, S7: 0x004d5ed8, S8: 0x004d5fd8
 * Each S-box spans 256 bytes (64 × 4 bytes). The remaining 32 entries
 * per S-box need to be extracted from the binary. */

/* S1 - DAT_004d58d8 */
static const u32 s_sbox1[64] = {
    0x00828000, 0x00000000, 0x00800000, 0x02828000,
    0x02808000, 0x02820000, 0x02000000, 0x00800000,
    0x00020000, 0x00828000, 0x02828000, 0x00020000,
    0x02028000, 0x02808000, 0x00008000, 0x02000000,
    0x02020000, 0x00028000, 0x00028000, 0x00820000,
    0x00820000, 0x00808000, 0x00808000, 0x02028000,
    0x02800000, 0x02008000, 0x02008000, 0x02800000,
    0x00000000, 0x02020000, 0x02820000, 0x00008000
};

/* S2 - DAT_004d59d8 */
static const u32 s_sbox2[64] = {
    0x10400840, 0x00400040, 0x00400000, 0x10400800,
    0x00000800, 0x10000000, 0x10000840, 0x00400840,
    0x10000040, 0x10400000, 0x10400040, 0x00400000,
    0x10000800, 0x10400840, 0x00000040, 0x10400040,
    0x00400800, 0x10000040, 0x10400000, 0x00000840,
    0x00400040, 0x00000000, 0x00000800, 0x10400800,
    0x10000840, 0x00400800, 0x10400040, 0x10000000,
    0x00400840, 0x00000040, 0x00000000, 0x10000800
};

/* S3 - DAT_004d5ad8 */
static const u32 s_sbox3[64] = {
    0x04010000, 0x00010104, 0x00000000, 0x04000104,
    0x00010004, 0x04000000, 0x04010100, 0x00010004,
    0x04000100, 0x04000004, 0x04000004, 0x00000100,
    0x04000104, 0x04010000, 0x04000100, 0x00000104,
    0x00000004, 0x04000000, 0x00010100, 0x00010104,
    0x00010100, 0x00000104, 0x04000100, 0x04010004,
    0x04000000, 0x00010100, 0x00000100, 0x04010104,
    0x00010004, 0x04010100, 0x04010004, 0x00000000
};

/* S4 - DAT_004d5bd8 */
static const u32 s_sbox4[64] = {
    0x00104080, 0x40100080, 0x40100080, 0x40000000,
    0x40104000, 0x40004080, 0x00004080, 0x00104080,
    0x00000000, 0x00104000, 0x00104000, 0x40104080,
    0x40008000, 0x00000000, 0x40004000, 0x00004080,
    0x40000080, 0x00100000, 0x00004000, 0x00104080,
    0x40004000, 0x00004080, 0x00100080, 0x40100000,
    0x40104080, 0x00100080, 0x40100000, 0x00004000,
    0x00104000, 0x40000080, 0x40008000, 0x40104000
};

/* S5 - DAT_004d5cd8 */
static const u32 s_sbox5[64] = {
    0x80000000, 0x80000401, 0x00000401, 0x80000021,
    0x00000400, 0x80000000, 0x80000000, 0x00000020,
    0x80000401, 0x00000400, 0x80000000, 0x80000001,
    0x80000420, 0x00000400, 0x80000001, 0x80000420,
    0x80000021, 0x00000421, 0x80000400, 0x00000020,
    0x00000001, 0x00000420, 0x00000420, 0x00000000,
    0x80000200, 0x80000421, 0x80000421, 0x80000001,
    0x00000421, 0x80000200, 0x00000000, 0x80000021
};

/* S6 - DAT_004d5dd8 */
static const u32 s_sbox6[64] = {
    0x08000010, 0x00002010, 0x00200000, 0x08202010,
    0x00002010, 0x00000000, 0x08202010, 0x00200010,
    0x00200010, 0x08000000, 0x08000000, 0x00202000,
    0x08002000, 0x00200010, 0x00202000, 0x08000010,
    0x08200000, 0x00201000, 0x00201000, 0x00000010,
    0x00000010, 0x08002000, 0x08201000, 0x00200000,
    0x00202010, 0x08201000, 0x08002010, 0x00000000,
    0x00200000, 0x08200000, 0x08002010, 0x00202010
};

/* S7 - DAT_004d5ed8 */
static const u32 s_sbox7[64] = {
    0x00001000, 0x01001002, 0x01040002, 0x00000000,
    0x00040000, 0x01001002, 0x00001002, 0x01040000,
    0x01001000, 0x00040002, 0x01040002, 0x00001000,
    0x01000002, 0x01001000, 0x00000000, 0x01000002,
    0x00040002, 0x01000000, 0x00001002, 0x00040000,
    0x01040000, 0x01000002, 0x01001000, 0x00001002,
    0x01000000, 0x00040002, 0x00000000, 0x01040002,
    0x01001002, 0x00000000, 0x01040000, 0x01001002
};

/* S8 - DAT_004d5fd8 */
static const u32 s_sbox8[64] = {
    0x20080008, 0x00080008, 0x00080000, 0x20000000,
    0x00000200, 0x20080008, 0x20080208, 0x00000000,
    0x00080008, 0x20080008, 0x20000000, 0x20080200,
    0x20000008, 0x00080000, 0x00000000, 0x00080008,
    0x20000000, 0x00000200, 0x20080000, 0x00080208,
    0x20080200, 0x20000008, 0x00000208, 0x20080000,
    0x00080008, 0x20080208, 0x00000000, 0x20000008,
    0x00000208, 0x20080000, 0x20080200, 0x00000200
};

/* Initial permutation table A - DAT_004d60d8 (256 entries) */
static const u32 s_init_perm_a[256] = {
    0x00000000, 0x80000000, 0x00000000, 0x80000000,
    0x00800000, 0x80800000, 0x00800000, 0x80800000,
    0x00000000, 0x80000000, 0x00000000, 0x80000000,
    0x00800000, 0x80800000, 0x00800000, 0x80800000,
    0x00008000, 0x80008000, 0x00008000, 0x80008000,
    0x00808000, 0x80808000, 0x00808000, 0x80808000,
    0x00008000, 0x80008000, 0x00008000, 0x80008000,
    0x00808000, 0x80808000, 0x00808000, 0x80808000,
    0x00000000, 0x80000000, 0x00000000, 0x80000000,
    0x00800000, 0x80800000, 0x00800000, 0x80800000,
    0x00000000, 0x80000000, 0x00000000, 0x80000000,
    0x00800000, 0x80800000, 0x00800000, 0x80800000,
    0x00008000, 0x80008000, 0x00008000, 0x80008000,
    0x00808000, 0x80808000, 0x00808000, 0x80808000,
    0x00008000, 0x80008000, 0x00008000, 0x80008000,
    0x00808000, 0x80808000, 0x00808000, 0x80808000,
    0x00000020, 0x80000020, 0x00000020, 0x80000020,
    0x00800020, 0x80800020, 0x00800020, 0x80800020,
    0x00000020, 0x80000020, 0x00000020, 0x80000020,
    0x00800020, 0x80800020, 0x00800020, 0x80800020,
    0x00008020, 0x80008020, 0x00008020, 0x80008020,
    0x00808020, 0x80808020, 0x00808020, 0x80808020,
    0x00008020, 0x80008020, 0x00008020, 0x80008020,
    0x00808020, 0x80808020, 0x00808020, 0x80808020,
    0x00000020, 0x80000020, 0x00000020, 0x80000020,
    0x00800020, 0x80800020, 0x00800020, 0x80800020,
    0x00000020, 0x80000020, 0x00000020, 0x80000020,
    0x00800020, 0x80800020, 0x00800020, 0x80800020,
    0x00008020, 0x80008020, 0x00008020, 0x80008020,
    0x00808020, 0x80808020, 0x00808020, 0x80808020,
    0x00008020, 0x80008020, 0x00008020, 0x80008020,
    0x00808020, 0x80808020, 0x00808020, 0x80808020
};

/* Initial permutation table B - DAT_004d64d8 (256 entries) */
static const u32 s_init_perm_b[256] = {
    0x00000000, 0x00000001, 0x00000002, 0x00000004,
    0x00000008, 0x00000010, 0x00000020, 0x00000040,
    0x00000080, 0x00000100, 0x00000200, 0x00000400,
    0x00000800, 0x00001000, 0x00002000, 0x00004000,
    0x00008000, 0x00010000, 0x00020000, 0x00040000,
    0x00080000, 0x00100000, 0x00200000, 0x00400000,
    0x00800000, 0x01000000, 0x02000000, 0x04000000,
    0x08000000, 0x10000000, 0x20000000, 0x40000000,
    0x00000001, 0x00000002, 0x00000004, 0x00000008,
    0x00000010, 0x00000020, 0x00000040, 0x00000080,
    0x00000100, 0x00000200, 0x00000400, 0x00000800,
    0x00001000, 0x00002000, 0x00004000, 0x00008000,
    0x00010000, 0x00020000, 0x00040000, 0x00080000,
    0x00100000, 0x00200000, 0x00400000, 0x00800000,
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
    0x00000002, 0x00000004, 0x00000008, 0x00000010,
    0x00000020, 0x00000040, 0x00000080, 0x00000100,
    0x00000200, 0x00000400, 0x00000800, 0x00001000,
    0x00002000, 0x00004000, 0x00008000, 0x00010000,
    0x00020000, 0x00040000, 0x00080000, 0x00100000,
    0x00200000, 0x00400000, 0x00800000, 0x01000000,
    0x02000000, 0x04000000, 0x08000000, 0x10000000,
    0x20000000, 0x40000000, 0x80000000, 0x00000001,
    0x00000004, 0x00000008, 0x00000010, 0x00000020,
    0x00000040, 0x00000080, 0x00000100, 0x00000200,
    0x00000400, 0x00000800, 0x00001000, 0x00002000,
    0x00004000, 0x00008000, 0x00010000, 0x00020000,
    0x00040000, 0x00080000, 0x00100000, 0x00200000,
    0x00400000, 0x00800000, 0x01000000, 0x02000000,
    0x04000000, 0x08000000, 0x10000000, 0x20000000,
    0x40000000, 0x80000000, 0x00000001, 0x00000002
};

/* Final permutation table A - DAT_004d68d8 (256 entries) */
static const u32 s_final_perm_a[256] = {
    0x00000000, 0x00000001, 0x00000002, 0x00000004,
    0x00000008, 0x00000010, 0x00000020, 0x00000040,
    0x00000080, 0x00000100, 0x00000200, 0x00000400,
    0x00000800, 0x00001000, 0x00002000, 0x00004000,
    0x00008000, 0x00010000, 0x00020000, 0x00040000,
    0x00080000, 0x00100000, 0x00200000, 0x00400000,
    0x00800000, 0x01000000, 0x02000000, 0x04000000,
    0x08000000, 0x10000000, 0x20000000, 0x40000000,
    0x00000001, 0x00000002, 0x00000004, 0x00000008,
    0x00000010, 0x00000020, 0x00000040, 0x00000080,
    0x00000100, 0x00000200, 0x00000400, 0x00000800,
    0x00001000, 0x00002000, 0x00004000, 0x00008000,
    0x00010000, 0x00020000, 0x00040000, 0x00080000,
    0x00100000, 0x00200000, 0x00400000, 0x00800000,
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
    0x00000002, 0x00000004, 0x00000008, 0x00000010,
    0x00000020, 0x00000040, 0x00000080, 0x00000100,
    0x00000200, 0x00000400, 0x00000800, 0x00001000,
    0x00002000, 0x00004000, 0x00008000, 0x00010000,
    0x00020000, 0x00040000, 0x00080000, 0x00100000,
    0x00200000, 0x00400000, 0x00800000, 0x01000000,
    0x02000000, 0x04000000, 0x08000000, 0x10000000,
    0x20000000, 0x40000000, 0x80000000, 0x00000001,
    0x00000004, 0x00000008, 0x00000010, 0x00000020,
    0x00000040, 0x00000080, 0x00000100, 0x00000200,
    0x00000400, 0x00000800, 0x00001000, 0x00002000,
    0x00004000, 0x00008000, 0x00010000, 0x00020000,
    0x00040000, 0x00080000, 0x00100000, 0x00200000,
    0x00400000, 0x00800000, 0x01000000, 0x02000000,
    0x04000000, 0x08000000, 0x10000000, 0x20000000,
    0x40000000, 0x80000000, 0x00000001, 0x00000002
};

/* Final permutation table B - DAT_004d6cd8 (256 entries) */
static const u32 s_final_perm_b[256] = {
    0x00000000, 0x00000001, 0x00000002, 0x00000004,
    0x00000008, 0x00000010, 0x00000020, 0x00000040,
    0x00000080, 0x00000100, 0x00000200, 0x00000400,
    0x00000800, 0x00001000, 0x00002000, 0x00004000,
    0x00008000, 0x00010000, 0x00020000, 0x00040000,
    0x00080000, 0x00100000, 0x00200000, 0x00400000,
    0x00800000, 0x01000000, 0x02000000, 0x04000000,
    0x08000000, 0x10000000, 0x20000000, 0x40000000,
    0x00000000, 0x00000001, 0x00000002, 0x00000004,
    0x00000008, 0x00000010, 0x00000020, 0x00000040,
    0x00000080, 0x00000100, 0x00000200, 0x00000400,
    0x00000800, 0x00001000, 0x00002000, 0x00004000,
    0x00008000, 0x00010000, 0x00020000, 0x00040000,
    0x00080000, 0x00100000, 0x00200000, 0x00400000,
    0x00800000, 0x01000000, 0x02000000, 0x04000000,
    0x08000000, 0x10000000, 0x20000000, 0x40000000,
    0x00000000, 0x00000001, 0x00000002, 0x00000004,
    0x00000008, 0x00000010, 0x00000020, 0x00000040,
    0x00000080, 0x00000100, 0x00000200, 0x00000400,
    0x00000800, 0x00001000, 0x00002000, 0x00004000,
    0x00008000, 0x00010000, 0x00020000, 0x00040000,
    0x00080000, 0x00100000, 0x00200000, 0x00400000,
    0x00800000, 0x01000000, 0x02000000, 0x04000000,
    0x08000000, 0x10000000, 0x20000000, 0x40000000,
    0x00000000, 0x00000001, 0x00000002, 0x00000004,
    0x00000008, 0x00000010, 0x00000020, 0x00000040,
    0x00000080, 0x00000100, 0x00000200, 0x00000400,
    0x00000800, 0x00001000, 0x00002000, 0x00004000,
    0x00008000, 0x00010000, 0x00020000, 0x00040000,
    0x00080000, 0x00100000, 0x00200000, 0x00400000,
    0x00800000, 0x01000000, 0x02000000, 0x04000000,
    0x08000000, 0x10000000, 0x20000000, 0x40000000
};

/* Subkey storage - DAT_04ebffe4 (32 values = 16 rounds x 2) */
static u32 s_subkeys[32];

/*
 * Key schedule function - FUN_0048b620
 * Generates 16 round subkeys from 8-byte key
 */
void des_key_schedule(const u8* key) {
    u32 c = 0, d = 0;
    const u8* pc1 = s_pc1_table;
    int i, j, round;
    u8 byte;
    u8 bit_pos;

    /* Apply PC1 permutation to generate C and D halves */
    for (i = 0; i < 8; i++) {
        byte = key[i];
        for (j = 7; j >= 0; j--) {
            if (byte & 0x80) {
                bit_pos = *pc1;
                if (bit_pos < 28) {
                    c |= (1U << bit_pos);
                } else {
                    d |= (1U << (bit_pos - 28));
                }
            }
            byte <<= 1;
            pc1++;
        }
    }

    /* Generate 16 round keys */
    for (round = 0; round < 16; round++) {
        u8 rotation = s_rotation_schedule[round];
        u8 c_bytes[4], d_bytes[4];
        u32 key_c, key_d;
        int k;
        u32 mask;

        /* Rotate C and D left by rotation amount */
        c = ((c << rotation) | (c >> (28 - rotation))) & 0x0FFFFFFF;
        d = ((d << rotation) | (d >> (28 - rotation))) & 0x0FFFFFFF;

        /* Apply PC2 permutation using lookup tables */
        /* Generate C part of subkey */
        c_bytes[0] = 0; c_bytes[1] = 0; c_bytes[2] = 0; c_bytes[3] = 0;
        for (k = 0; k < 32; k++) {
            mask = 0x20 >> (k & 7);
            if (c & (1U << s_pc2_c[k])) {
                c_bytes[k >> 3] |= mask;
            }
        }

        /* Generate D part of subkey */
        d_bytes[0] = 0; d_bytes[1] = 0; d_bytes[2] = 0; d_bytes[3] = 0;
        for (k = 0; k < 32; k++) {
            mask = 0x20 >> (k & 7);
            if (d & (1U << s_pc2_d[k])) {
                d_bytes[k >> 3] |= mask;
            }
        }

        /* Store subkey pair */
        key_c = ((u32)c_bytes[0] << 24) | ((u32)c_bytes[1] << 16) |
                ((u32)c_bytes[2] << 8) | c_bytes[3];
        key_d = ((u32)d_bytes[0] << 24) | ((u32)d_bytes[1] << 16) |
                ((u32)d_bytes[2] << 8) | d_bytes[3];

        s_subkeys[round * 2] = key_c;
        s_subkeys[round * 2 + 1] = key_d;
    }
}

/*
 * Block cipher function - FUN_0048b7c0
 * Encrypts or decrypts 8 bytes using generated subkeys
 */
void des_block_cipher(u8* data, int decrypt) {
    u32 left, right;
    u32 temp_l, temp_r;
    int round;
    u32* subkey;
    u32 old_r;

    /* Initial permutation using lookup tables */
    left = s_init_perm_a[data[0]] >> 1 | s_init_perm_a[data[1]] >> 1 |
           s_init_perm_a[data[2]] >> 1 | s_init_perm_a[data[3]] >> 1 |
           s_init_perm_a[data[4]] >> 1 | s_init_perm_a[data[5]] >> 1 |
           s_init_perm_a[data[6]] >> 1 | s_init_perm_a[data[7]];

    right = s_init_perm_b[data[0]] >> 1 | s_init_perm_b[data[1]] >> 1 |
            s_init_perm_b[data[2]] >> 1 | s_init_perm_b[data[3]] >> 1 |
            s_init_perm_b[data[4]] >> 1 | s_init_perm_b[data[5]] >> 1 |
            s_init_perm_b[data[6]] >> 1 | s_init_perm_b[data[7]];

    /* 16 Feistel rounds */
    if (decrypt) {
        /* Decrypt: use subkeys in reverse order */
        subkey = &s_subkeys[30];  /* Start from last subkey */
        for (round = 0; round < 16; round++) {
            old_r = right;

            /* Rotate right by 1 */
            right = (right >> 31) | (right << 1);

            /* Apply round function */
            temp_l = (right << 28) | (old_r >> 4);
            temp_r = right;

            /* XOR with subkeys */
            temp_l ^= subkey[-1];  /* C part */
            temp_r ^= subkey[0];   /* D part */

            /* S-box substitution */
            right = s_sbox1[temp_l >> 24 & 0x3F] ^
                    s_sbox2[temp_r >> 24 & 0x3F] ^
                    s_sbox3[temp_l >> 16 & 0x3F] ^
                    s_sbox4[temp_r >> 16 & 0x3F] ^
                    s_sbox5[temp_l >> 8 & 0x3F] ^
                    s_sbox6[temp_r >> 8 & 0x3F] ^
                    s_sbox7[temp_l & 0x3F] ^
                    s_sbox8[temp_r & 0x3F] ^
                    left;

            left = old_r;
            subkey -= 2;
        }
    } else {
        /* Encrypt: use subkeys in forward order */
        subkey = s_subkeys;
        for (round = 0; round < 16; round++) {
            old_r = right;

            /* Rotate right by 1 */
            right = (right >> 31) | (right << 1);

            /* Apply round function */
            temp_l = (right << 28) | (old_r >> 4);
            temp_r = right;

            /* XOR with subkeys */
            temp_l ^= subkey[-1];  /* C part (from previous iteration) */
            temp_r ^= subkey[0];   /* D part */

            /* S-box substitution */
            right = s_sbox1[temp_l >> 24 & 0x3F] ^
                    s_sbox2[temp_r >> 24 & 0x3F] ^
                    s_sbox3[temp_l >> 16 & 0x3F] ^
                    s_sbox4[temp_r >> 16 & 0x3F] ^
                    s_sbox5[temp_l >> 8 & 0x3F] ^
                    s_sbox6[temp_r >> 8 & 0x3F] ^
                    s_sbox7[temp_l & 0x3F] ^
                    s_sbox8[temp_r & 0x3F] ^
                    left;

            left = old_r;
            subkey += 2;
        }
    }

    /* Final permutation */
    temp_l = s_final_perm_a[(right >> 24) & 0xFF] >> 1 |
             s_final_perm_a[(right >> 16) & 0xFF] >> 1 |
             s_final_perm_a[(right >> 8) & 0xFF] >> 1 |
             s_final_perm_a[right & 0xFF] >> 1 |
             s_final_perm_a[(left >> 24) & 0xFF] >> 1 |
             s_final_perm_a[(left >> 16) & 0xFF] >> 1 |
             s_final_perm_a[(left >> 8) & 0xFF] >> 1 |
             s_final_perm_a[left & 0xFF];

    temp_r = s_final_perm_b[(right >> 24) & 0xFF] >> 1 |
             s_final_perm_b[(right >> 16) & 0xFF] >> 1 |
             s_final_perm_b[(right >> 8) & 0xFF] >> 1 |
             s_final_perm_b[right & 0xFF] >> 1 |
             s_final_perm_b[(left >> 24) & 0xFF] >> 1 |
             s_final_perm_b[(left >> 16) & 0xFF] >> 1 |
             s_final_perm_b[(left >> 8) & 0xFF] >> 1 |
             s_final_perm_b[left & 0xFF];

    /* Output result */
    data[0] = (u8)(temp_l >> 24);
    data[1] = (u8)(temp_l >> 16);
    data[2] = (u8)(temp_l >> 8);
    data[3] = (u8)temp_l;
    data[4] = (u8)(temp_r >> 24);
    data[5] = (u8)(temp_r >> 16);
    data[6] = (u8)(temp_r >> 8);
    data[7] = (u8)temp_r;
}

/*
 * DES wrapper function - FUN_0048bb90
 * Encrypts or decrypts data in 8-byte blocks
 *
 * Parameters:
 *   key: 8-byte encryption key
 *   data: data to encrypt/decrypt
 *   size: size of data
 *   mode: 0 = decrypt, 1 = encrypt, 2 = don't return success flag
 *
 * Returns: true on success (unless mode & 2)
 */
int des_encrypt(const char* key, void* data, int size, u8 mode) {
    u8 key_buf[8];
    u8* data_ptr = (u8*)data;
    int blocks;
    int i;

    if (!key || !data || size <= 0) {
        return 0;
    }

    /* Copy and pad key to 8 bytes */
    memset(key_buf, 0, sizeof(key_buf));
    strncpy((char*)key_buf, key, 8);

    /* Generate key schedule */
    des_key_schedule(key_buf);

    /* Calculate number of blocks (round up to 8 bytes) */
    blocks = (size + 7) & ~7;

    /* Process each 8-byte block */
    for (i = 0; i < blocks; i += 8) {
        des_block_cipher(data_ptr + i, mode & 1);
    }

    /* Return success flag unless mode & 2 */
    return (mode & 2) != 2;
}

/*
 * Default key for Stone Age client
 * Located at 0x0049e48c: "f;encor1c"
 */
static const char* s_default_key = "f;encor1c";

/*
 * Encrypt login credentials using default key
 */
void login_encrypt_credentials(void* data, int size) {
    des_encrypt(s_default_key, data, size, 1);
}

/*
 * Decrypt login credentials using default key
 */
void login_decrypt_credentials(void* data, int size) {
    des_encrypt(s_default_key, data, size, 0);
}
