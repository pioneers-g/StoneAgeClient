/*
 * Stone Age Client - Configuration Copy Functions Unit Tests
 * Tests for FUN_0047cb00, FUN_0047cb30, FUN_0047cb60, FUN_0047cb70, FUN_0047ccd0
 * These functions copy configuration values between memory regions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;
typedef unsigned char byte;
typedef unsigned short ushort;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Configuration variables - source */
static u32 DAT_0054a4d0 = 0;
static byte DAT_004a2674 = 0;
static byte DAT_004a2678 = 0;
static byte DAT_045f1948 = 0;

/* Configuration variables - local copy */
static u32 DAT_04633343 = 0;
static u32 DAT_04633344 = 0;

/* Additional variables cleared by FUN_0047ccd0 */
static u32 DAT_046333f0 = 0;
static u32 DAT_046333f4 = 0;
static u32 DAT_046333b4 = 0;
static u32 DAT_046333d4 = 0;
static ushort DAT_046333a2 = 0;
static u32 DAT_046333b8 = 0;
static ushort DAT_046333a0 = 0;
static ushort DAT_046333b6 = 0;
static u32 DAT_046333d8 = 0;
static u32 DAT_046333e0 = 0;
static u32 DAT_046333dc = 0;
static u32 DAT_046333e4 = 0;
static u32 DAT_046333ec = 0;
static u32 DAT_046333e8 = 0;

static int test_passed = 0;
static int test_failed = 0;

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    test_##name(); \
    printf("PASS\n"); \
    test_passed++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL at line %d: %s\n", __LINE__, #cond); \
        test_failed++; \
        return; \
    } \
} while(0)

/* Reset test state */
static void reset_test_state(void) {
    DAT_0054a4d0 = 0;
    DAT_004a2674 = 0;
    DAT_004a2678 = 0;
    DAT_045f1948 = 0;
    DAT_04633343 = 0;
    DAT_04633344 = 0;

    DAT_046333f0 = 0;
    DAT_046333f4 = 0;
    DAT_046333b4 = 0;
    DAT_046333d4 = 0;
    DAT_046333a2 = 0;
    DAT_046333b8 = 0;
    DAT_046333a0 = 0;
    DAT_046333b6 = 0;
    DAT_046333d8 = 0;
    DAT_046333e0 = 0;
    DAT_046333dc = 0;
    DAT_046333e4 = 0;
    DAT_046333ec = 0;
    DAT_046333e8 = 0;
}

/*
 * FUN_0047cb00 - Copy configuration from global to local
 * Copies DAT_0054a4d0 to DAT_04633343
 * Copies DAT_004a2674 and DAT_004a2678 to DAT_04633344
 */
static void FUN_0047cb00(void) {
    DAT_04633343 = DAT_0054a4d0;
    DAT_04633344 = (DAT_004a2674 & 0xFF) | ((DAT_004a2678 & 0xFF) << 8);
}

/*
 * FUN_0047cb30 - Copy configuration from local to global
 * Copies DAT_04633343 to DAT_0054a4d0
 * Copies DAT_04633344 bytes to DAT_004a2674 and DAT_004a2678
 */
static void FUN_0047cb30(void) {
    DAT_0054a4d0 = DAT_04633343;
    DAT_004a2678 = (DAT_04633344 >> 8) & 0xFF;
    DAT_004a2674 = DAT_04633344 & 0xFF;
}

/*
 * FUN_0047cb60 - Copy byte from DAT_045f1948 to DAT_04633344 byte 2
 */
static void FUN_0047cb60(void) {
    DAT_04633344 = (DAT_04633344 & 0xFFFF) | ((DAT_045f1948 & 0xFF) << 16);
}

/*
 * FUN_0047cb70 - Copy byte from DAT_04633344 byte 2 to DAT_045f1948
 */
static void FUN_0047cb70(void) {
    DAT_045f1948 = (DAT_04633344 >> 16) & 0xFF;
}

/*
 * FUN_0047ccd0 - Clear multiple configuration variables
 */
static void FUN_0047ccd0(void) {
    DAT_046333f0 = 0;
    DAT_046333f4 = 0;
    DAT_046333b4 = 0;
    DAT_046333d4 = 0;
    DAT_046333a2 = 0;
    DAT_046333b8 = 0;
    DAT_046333a0 = 0;
    DAT_046333b6 = 0;
    DAT_046333d8 = 0;
    DAT_046333e0 = 0;
    DAT_046333dc = 0;
    DAT_046333e4 = 0;
    DAT_046333ec = 0;
    DAT_046333e8 = 0;
}

/* ========================================
 * Test Cases
 * ======================================== */

/* FUN_0047cb00 tests */
TEST(copy_to_local_basic) {
    reset_test_state();
    DAT_0054a4d0 = 12345;
    DAT_004a2674 = 0x12;
    DAT_004a2678 = 0x34;

    FUN_0047cb00();

    ASSERT(DAT_04633343 == 12345);
    ASSERT((DAT_04633344 & 0xFF) == 0x12);
    ASSERT((DAT_04633344 >> 8 & 0xFF) == 0x34);
}

TEST(copy_to_local_zero) {
    reset_test_state();

    FUN_0047cb00();

    ASSERT(DAT_04633343 == 0);
    ASSERT(DAT_04633344 == 0);
}

TEST(copy_to_local_max_values) {
    reset_test_state();
    DAT_0054a4d0 = 0xFFFFFFFF;
    DAT_004a2674 = 0xFF;
    DAT_004a2678 = 0xFF;

    FUN_0047cb00();

    ASSERT(DAT_04633343 == 0xFFFFFFFF);
    ASSERT((DAT_04633344 & 0xFFFF) == 0xFFFF);
}

TEST(copy_to_local_overwrites) {
    reset_test_state();
    DAT_04633343 = 99999;
    DAT_04633344 = 0xFFFFFFFF;
    DAT_0054a4d0 = 100;
    DAT_004a2674 = 0x01;
    DAT_004a2678 = 0x02;

    FUN_0047cb00();

    ASSERT(DAT_04633343 == 100);
    ASSERT((DAT_04633344 & 0xFF) == 0x01);
    ASSERT((DAT_04633344 >> 8 & 0xFF) == 0x02);
}

/* FUN_0047cb30 tests */
TEST(copy_to_global_basic) {
    reset_test_state();
    DAT_04633343 = 54321;
    DAT_04633344 = 0x00003412;

    FUN_0047cb30();

    ASSERT(DAT_0054a4d0 == 54321);
    ASSERT(DAT_004a2674 == 0x12);
    ASSERT(DAT_004a2678 == 0x34);
}

TEST(copy_to_global_zero) {
    reset_test_state();

    FUN_0047cb30();

    ASSERT(DAT_0054a4d0 == 0);
    ASSERT(DAT_004a2674 == 0);
    ASSERT(DAT_004a2678 == 0);
}

TEST(copy_to_global_overwrites) {
    reset_test_state();
    DAT_0054a4d0 = 99999;
    DAT_004a2674 = 0xFF;
    DAT_004a2678 = 0xFF;
    DAT_04633343 = 100;
    DAT_04633344 = 0x00000201;

    FUN_0047cb30();

    ASSERT(DAT_0054a4d0 == 100);
    ASSERT(DAT_004a2674 == 0x01);
    ASSERT(DAT_004a2678 == 0x02);
}

/* FUN_0047cb60 tests */
TEST(copy_byte_to_local_basic) {
    reset_test_state();
    DAT_045f1948 = 0x56;

    FUN_0047cb60();

    ASSERT((DAT_04633344 >> 16 & 0xFF) == 0x56);
}

TEST(copy_byte_to_local_zero) {
    reset_test_state();

    FUN_0047cb60();

    ASSERT((DAT_04633344 >> 16 & 0xFF) == 0);
}

TEST(copy_byte_to_local_preserves_lower) {
    reset_test_state();
    DAT_04633344 = 0x00001234;
    DAT_045f1948 = 0xAB;

    FUN_0047cb60();

    ASSERT((DAT_04633344 & 0xFFFF) == 0x1234);
    ASSERT((DAT_04633344 >> 16 & 0xFF) == 0xAB);
}

TEST(copy_byte_to_local_max) {
    reset_test_state();
    DAT_045f1948 = 0xFF;

    FUN_0047cb60();

    ASSERT((DAT_04633344 >> 16 & 0xFF) == 0xFF);
}

/* FUN_0047cb70 tests */
TEST(copy_byte_to_global_basic) {
    reset_test_state();
    DAT_04633344 = 0x00560000;

    FUN_0047cb70();

    ASSERT(DAT_045f1948 == 0x56);
}

TEST(copy_byte_to_global_zero) {
    reset_test_state();

    FUN_0047cb70();

    ASSERT(DAT_045f1948 == 0);
}

TEST(copy_byte_to_global_ignores_lower) {
    reset_test_state();
    DAT_04633344 = 0x00AB1234;

    FUN_0047cb70();

    ASSERT(DAT_045f1948 == 0xAB);
}

TEST(copy_byte_to_global_max) {
    reset_test_state();
    DAT_04633344 = 0x00FF0000;

    FUN_0047cb70();

    ASSERT(DAT_045f1948 == 0xFF);
}

/* FUN_0047ccd0 tests */
TEST(clear_config_all_zeros) {
    reset_test_state();

    FUN_0047ccd0();

    ASSERT(DAT_046333f0 == 0);
    ASSERT(DAT_046333f4 == 0);
    ASSERT(DAT_046333b4 == 0);
    ASSERT(DAT_046333d4 == 0);
    ASSERT(DAT_046333a2 == 0);
    ASSERT(DAT_046333b8 == 0);
    ASSERT(DAT_046333a0 == 0);
    ASSERT(DAT_046333b6 == 0);
    ASSERT(DAT_046333d8 == 0);
    ASSERT(DAT_046333e0 == 0);
    ASSERT(DAT_046333dc == 0);
    ASSERT(DAT_046333e4 == 0);
    ASSERT(DAT_046333ec == 0);
    ASSERT(DAT_046333e8 == 0);
}

TEST(clear_config_from_nonzero) {
    reset_test_state();

    /* Set all to non-zero values */
    DAT_046333f0 = 0x12345678;
    DAT_046333f4 = 0x87654321;
    DAT_046333b4 = 0xDEADBEEF;
    DAT_046333d4 = 0xCAFEBABE;
    DAT_046333a2 = 0x1234;
    DAT_046333b8 = 0xFFFFFFFF;
    DAT_046333a0 = 0xABCD;
    DAT_046333b6 = 0xEF01;
    DAT_046333d8 = 0x11111111;
    DAT_046333e0 = 0x22222222;
    DAT_046333dc = 0x33333333;
    DAT_046333e4 = 0x44444444;
    DAT_046333ec = 0x55555555;
    DAT_046333e8 = 0x66666666;

    FUN_0047ccd0();

    ASSERT(DAT_046333f0 == 0);
    ASSERT(DAT_046333f4 == 0);
    ASSERT(DAT_046333b4 == 0);
    ASSERT(DAT_046333d4 == 0);
    ASSERT(DAT_046333a2 == 0);
    ASSERT(DAT_046333b8 == 0);
    ASSERT(DAT_046333a0 == 0);
    ASSERT(DAT_046333b6 == 0);
    ASSERT(DAT_046333d8 == 0);
    ASSERT(DAT_046333e0 == 0);
    ASSERT(DAT_046333dc == 0);
    ASSERT(DAT_046333e4 == 0);
    ASSERT(DAT_046333ec == 0);
    ASSERT(DAT_046333e8 == 0);
}

/* Round-trip tests */
TEST(roundtrip_global_to_local_to_global) {
    reset_test_state();
    DAT_0054a4d0 = 99999;
    DAT_004a2674 = 0xAB;
    DAT_004a2678 = 0xCD;

    FUN_0047cb00();  /* Global -> Local */

    /* Clear globals */
    DAT_0054a4d0 = 0;
    DAT_004a2674 = 0;
    DAT_004a2678 = 0;

    FUN_0047cb30();  /* Local -> Global */

    ASSERT(DAT_0054a4d0 == 99999);
    ASSERT(DAT_004a2674 == 0xAB);
    ASSERT(DAT_004a2678 == 0xCD);
}

TEST(roundtrip_byte_copy) {
    reset_test_state();
    DAT_045f1948 = 0x78;

    FUN_0047cb60();  /* Global byte -> Local */

    /* Clear source */
    DAT_045f1948 = 0;

    FUN_0047cb70();  /* Local byte -> Global */

    ASSERT(DAT_045f1948 == 0x78);
}

/* Combined tests */
TEST(full_copy_sequence) {
    reset_test_state();

    /* Set initial values */
    DAT_0054a4d0 = 12345;
    DAT_004a2674 = 0x11;
    DAT_004a2678 = 0x22;
    DAT_045f1948 = 0x33;

    /* Copy all to local */
    FUN_0047cb00();
    FUN_0047cb60();

    /* Clear all globals */
    DAT_0054a4d0 = 0;
    DAT_004a2674 = 0;
    DAT_004a2678 = 0;
    DAT_045f1948 = 0;

    /* Copy all back */
    FUN_0047cb30();
    FUN_0047cb70();

    /* Verify */
    ASSERT(DAT_0054a4d0 == 12345);
    ASSERT(DAT_004a2674 == 0x11);
    ASSERT(DAT_004a2678 == 0x22);
    ASSERT(DAT_045f1948 == 0x33);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Configuration Copy Functions Unit Tests ===\n\n");

    printf("FUN_0047cb00 (Copy to Local) Tests:\n");
    RUN_TEST(copy_to_local_basic);
    RUN_TEST(copy_to_local_zero);
    RUN_TEST(copy_to_local_max_values);
    RUN_TEST(copy_to_local_overwrites);

    printf("\nFUN_0047cb30 (Copy to Global) Tests:\n");
    RUN_TEST(copy_to_global_basic);
    RUN_TEST(copy_to_global_zero);
    RUN_TEST(copy_to_global_overwrites);

    printf("\nFUN_0047cb60 (Copy Byte to Local) Tests:\n");
    RUN_TEST(copy_byte_to_local_basic);
    RUN_TEST(copy_byte_to_local_zero);
    RUN_TEST(copy_byte_to_local_preserves_lower);
    RUN_TEST(copy_byte_to_local_max);

    printf("\nFUN_0047cb70 (Copy Byte to Global) Tests:\n");
    RUN_TEST(copy_byte_to_global_basic);
    RUN_TEST(copy_byte_to_global_zero);
    RUN_TEST(copy_byte_to_global_ignores_lower);
    RUN_TEST(copy_byte_to_global_max);

    printf("\nFUN_0047ccd0 (Clear Config) Tests:\n");
    RUN_TEST(clear_config_all_zeros);
    RUN_TEST(clear_config_from_nonzero);

    printf("\nRound-trip Tests:\n");
    RUN_TEST(roundtrip_global_to_local_to_global);
    RUN_TEST(roundtrip_byte_copy);
    RUN_TEST(full_copy_sequence);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
