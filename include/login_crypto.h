/*
 * Stone Age Client - Login Crypto Module Header
 * Custom DES-like block cipher for login credential encryption
 * Reverse engineered from FUN_0048b620, FUN_0048b7c0, FUN_0048bb90
 *
 * Key: "f;encor1c" at 0x0049e48c
 * This is NOT standard DES - uses custom S-boxes
 */

#ifndef LOGIN_CRYPTO_H
#define LOGIN_CRYPTO_H

#include "types.h"

/* Encryption key - from DAT_0049e48c */
#define LOGIN_ENCRYPT_KEY       "f;encor1c"
#define LOGIN_ENCRYPT_KEY_LEN   9

/* Block size for DES cipher */
#define DES_BLOCK_SIZE          8

/* Mode flags for des_encrypt */
#define DES_MODE_DECRYPT        0
#define DES_MODE_ENCRYPT        1
#define DES_MODE_NO_RETURN      2

/* Key schedule function - FUN_0048b620 */
void des_key_schedule(const u8* key);

/* Block cipher function - FUN_0048b7c0 */
void des_block_cipher(u8* data, int decrypt);

/* DES wrapper function - FUN_0048bb90
 * Parameters:
 *   key: 8-byte encryption key (or use LOGIN_ENCRYPT_KEY)
 *   data: data to encrypt/decrypt
 *   size: size of data
 *   mode: DES_MODE_ENCRYPT or DES_MODE_DECRYPT
 * Returns: true on success (unless DES_MODE_NO_RETURN is set)
 */
int des_encrypt(const char* key, void* data, int size, u8 mode);

/* Encrypt login credentials using default key */
void login_encrypt_credentials(void* data, int size);

/* Decrypt login credentials using default key */
void login_decrypt_credentials(void* data, int size);

/* Legacy compatibility functions */
void login_encrypt_data(const char* key, void* data, int len, int encrypt);
void login_encrypt_password(const char* password, char* encrypted);
void login_encrypt_user_pass(const char* user, const char* pass,
                              char* enc_user, char* enc_pass);

#endif /* LOGIN_CRYPTO_H */
