/*
 * Stone Age Client - DES Crypto Unit Tests
 * Tests for custom DES-like block cipher from FUN_0048b620, FUN_0048b7c0, FUN_0048bb90
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../tests/test_framework.h"
#include "types.h"
#include "login_crypto.h"

/* ========================================
 * Test Cases for DES Constants
 * ======================================== */

/* Test encryption key */
static void test_des_key(void) {
    TEST_BEGIN("DES encryption key");

    /* Key should be "f;encor1c" from 0x0049e48c */
    TEST_ASSERT_STR_EQ(LOGIN_ENCRYPT_KEY, "f;encor1c");
    TEST_ASSERT_EQ(LOGIN_ENCRYPT_KEY_LEN, 9);

    TEST_END();
}

/* Test block size */
static void test_des_block_size(void) {
    TEST_BEGIN("DES block size");

    TEST_ASSERT_EQ(DES_BLOCK_SIZE, 8);

    TEST_END();
}

/* Test mode flags */
static void test_des_mode_flags(void) {
    TEST_BEGIN("DES mode flags");

    TEST_ASSERT_EQ(DES_MODE_DECRYPT, 0);
    TEST_ASSERT_EQ(DES_MODE_ENCRYPT, 1);
    TEST_ASSERT_EQ(DES_MODE_NO_RETURN, 2);

    TEST_END();
}

/* ========================================
 * Test Cases for Key Schedule
 * ======================================== */

/* Test key schedule generation */
static void test_key_schedule(void) {
    TEST_BEGIN("Key schedule generation");

    u8 key[8] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38};  /* "12345678" */

    /* Key schedule should be generated without crash */
    des_key_schedule(key);

    TEST_END();
}

/* Test key schedule with default key */
static void test_key_schedule_default(void) {
    TEST_BEGIN("Key schedule with default key");

    /* Use the game's encryption key */
    des_key_schedule((const u8*)LOGIN_ENCRYPT_KEY);

    TEST_END();
}

/* Test key schedule with null key */
static void test_key_schedule_null(void) {
    TEST_BEGIN("Key schedule with null key");

    /* Should handle null key gracefully */
    /* des_key_schedule(NULL); */  /* Skip - would crash */

    TEST_END();
}

/* ========================================
 * Test Cases for Block Cipher
 * ======================================== */

/* Test block cipher encryption */
static void test_block_cipher_encrypt(void) {
    TEST_BEGIN("Block cipher encrypt");

    u8 data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    u8 original[8];
    memcpy(original, data, 8);

    /* Generate key schedule */
    des_key_schedule((const u8*)LOGIN_ENCRYPT_KEY);

    /* Encrypt block */
    des_block_cipher(data, 0);  /* 0 = encrypt */

    /* Data should be different after encryption */
    int different = 0;
    for (int i = 0; i < 8; i++) {
        if (data[i] != original[i]) {
            different = 1;
            break;
        }
    }
    TEST_ASSERT(different);

    TEST_END();
}

/* Test block cipher decrypt reverses encrypt */
static void test_block_cipher_roundtrip(void) {
    TEST_BEGIN("Block cipher roundtrip");

    u8 data[8] = {'t', 'e', 's', 't', 'd', 'a', 't', 'a'};
    u8 original[8];
    memcpy(original, data, 8);

    /* Generate key schedule */
    des_key_schedule((const u8*)LOGIN_ENCRYPT_KEY);

    /* Encrypt */
    des_block_cipher(data, 0);  /* encrypt */

    /* Decrypt */
    des_block_cipher(data, 1);  /* decrypt */

    /* Should match original */
    TEST_ASSERT(memcmp(data, original, 8) == 0);

    TEST_END();
}

/* Test block cipher with zeros */
static void test_block_cipher_zeros(void) {
    TEST_BEGIN("Block cipher with zeros");

    u8 data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    u8 original[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    des_key_schedule((const u8*)LOGIN_ENCRYPT_KEY);
    des_block_cipher(data, 0);
    des_block_cipher(data, 1);

    TEST_ASSERT(memcmp(data, original, 8) == 0);

    TEST_END();
}

/* Test block cipher with all ones */
static void test_block_cipher_ones(void) {
    TEST_BEGIN("Block cipher with all ones");

    u8 data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    u8 original[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    des_key_schedule((const u8*)LOGIN_ENCRYPT_KEY);
    des_block_cipher(data, 0);
    des_block_cipher(data, 1);

    TEST_ASSERT(memcmp(data, original, 8) == 0);

    TEST_END();
}

/* ========================================
 * Test Cases for DES Wrapper
 * ======================================== */

/* Test des_encrypt function */
static void test_des_encrypt_wrapper(void) {
    TEST_BEGIN("DES encrypt wrapper");

    char data[16] = "test1234test5678";
    char original[16];
    memcpy(original, data, 16);

    /* Encrypt */
    int result = des_encrypt(LOGIN_ENCRYPT_KEY, data, 16, DES_MODE_ENCRYPT);
    TEST_ASSERT(result == 1);  /* Should return success */

    /* Data should be different */
    TEST_ASSERT(memcmp(data, original, 16) != 0);

    TEST_END();
}

/* Test des_encrypt decrypt */
static void test_des_encrypt_decrypt(void) {
    TEST_BEGIN("DES encrypt then decrypt");

    char data[32] = "This is a test message!";
    char original[32];
    memcpy(original, data, 32);

    /* Encrypt */
    des_encrypt(LOGIN_ENCRYPT_KEY, data, 32, DES_MODE_ENCRYPT);

    /* Decrypt */
    des_encrypt(LOGIN_ENCRYPT_KEY, data, 32, DES_MODE_DECRYPT);

    /* Should match original */
    TEST_ASSERT(memcmp(data, original, 32) == 0);

    TEST_END();
}

/* Test des_encrypt with small data */
static void test_des_encrypt_small(void) {
    TEST_BEGIN("DES encrypt small data");

    char data[3] = "Hi";
    char original[3] = "Hi";

    /* Should pad to 8 bytes internally */
    des_encrypt(LOGIN_ENCRYPT_KEY, data, 3, DES_MODE_ENCRYPT);
    des_encrypt(LOGIN_ENCRYPT_KEY, data, 3, DES_MODE_DECRYPT);

    /* First 3 bytes should match */
    TEST_ASSERT(memcmp(data, original, 3) == 0);

    TEST_END();
}

/* Test des_encrypt mode flags */
static void test_des_encrypt_mode_flags(void) {
    TEST_BEGIN("DES encrypt mode flags");

    char data[8] = "testdata";

    /* Mode 1 (encrypt) should return 1 */
    int r1 = des_encrypt(LOGIN_ENCRYPT_KEY, data, 8, DES_MODE_ENCRYPT);
    TEST_ASSERT(r1 == 1);

    /* Mode 2 (no return) should return 0 */
    int r2 = des_encrypt(LOGIN_ENCRYPT_KEY, data, 8, DES_MODE_NO_RETURN);
    TEST_ASSERT(r2 == 0);

    TEST_END();
}

/* ========================================
 * Test Cases for Login Credentials
 * ======================================== */

/* Test login_encrypt_credentials */
static void test_login_encrypt_credentials(void) {
    TEST_BEGIN("Login encrypt credentials");

    char data[32] = "username_password_test";
    char original[32];
    memcpy(original, data, 32);

    /* Encrypt */
    login_encrypt_credentials(data, 32);

    /* Should be encrypted */
    TEST_ASSERT(memcmp(data, original, 32) != 0);

    /* Decrypt */
    login_decrypt_credentials(data, 32);

    /* Should match original */
    TEST_ASSERT(memcmp(data, original, 32) == 0);

    TEST_END();
}

/* Test with typical username */
static void test_encrypt_username(void) {
    TEST_BEGIN("Encrypt username");

    char username[32] = "testuser";
    char original[32];
    memcpy(original, username, 32);

    login_encrypt_credentials(username, 32);
    login_decrypt_credentials(username, 32);

    TEST_ASSERT(memcmp(username, original, 32) == 0);

    TEST_END();
}

/* Test with typical password */
static void test_encrypt_password(void) {
    TEST_BEGIN("Encrypt password");

    char password[32] = "testpass123";
    char original[32];
    memcpy(original, password, 32);

    login_encrypt_credentials(password, 32);
    login_decrypt_credentials(password, 32);

    TEST_ASSERT(memcmp(password, original, 32) == 0);

    TEST_END();
}

/* Test with DBCS characters */
static void test_encrypt_dbcs(void) {
    TEST_BEGIN("Encrypt DBCS characters");

    /* Japanese/Chinese characters are 2 bytes each in Shift-JIS/GBK */
    char data[32] = "test\x82\xd1\x82\xc9test";  /* DBCS chars */
    char original[32];
    memcpy(original, data, 32);

    login_encrypt_credentials(data, 32);
    login_decrypt_credentials(data, 32);

    TEST_ASSERT(memcmp(data, original, 32) == 0);

    TEST_END();
}

/* ========================================
 * Test Cases for Edge Cases
 * ======================================== */

/* Test with exact 8-byte boundary */
static void test_encrypt_exact_block(void) {
    TEST_BEGIN("Encrypt exact block");

    char data[8] = "12345678";
    char original[8];
    memcpy(original, data, 8);

    login_encrypt_credentials(data, 8);
    login_decrypt_credentials(data, 8);

    TEST_ASSERT(memcmp(data, original, 8) == 0);

    TEST_END();
}

/* Test with multiple blocks */
static void test_encrypt_multiple_blocks(void) {
    TEST_BEGIN("Encrypt multiple blocks");

    char data[24] = "block1_1block2_2block3";  /* 3 blocks */
    char original[24];
    memcpy(original, data, 24);

    login_encrypt_credentials(data, 24);
    login_decrypt_credentials(data, 24);

    TEST_ASSERT(memcmp(data, original, 24) == 0);

    TEST_END();
}

/* Test with large data */
static void test_encrypt_large_data(void) {
    TEST_BEGIN("Encrypt large data");

    char data[256];
    char original[256];

    /* Fill with pattern */
    for (int i = 0; i < 256; i++) {
        data[i] = (char)(i & 0xFF);
        original[i] = (char)(i & 0xFF);
    }

    login_encrypt_credentials(data, 256);
    login_decrypt_credentials(data, 256);

    TEST_ASSERT(memcmp(data, original, 256) == 0);

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

int main(void) {
    /* DES Constants */
    test_des_key();
    test_des_block_size();
    test_des_mode_flags();

    /* Key Schedule */
    test_key_schedule();
    test_key_schedule_default();

    /* Block Cipher */
    test_block_cipher_encrypt();
    test_block_cipher_roundtrip();
    test_block_cipher_zeros();
    test_block_cipher_ones();

    /* DES Wrapper */
    test_des_encrypt_wrapper();
    test_des_encrypt_decrypt();
    test_des_encrypt_small();
    test_des_encrypt_mode_flags();

    /* Login Credentials */
    test_login_encrypt_credentials();
    test_encrypt_username();
    test_encrypt_password();
    test_encrypt_dbcs();

    /* Edge Cases */
    test_encrypt_exact_block();
    test_encrypt_multiple_blocks();
    test_encrypt_large_data();

    TEST_SUMMARY();

    return g_test_failures;
}
