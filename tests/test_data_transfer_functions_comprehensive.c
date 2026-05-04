/*
 * Stone Age Client - Data Transfer Functions Unit Tests
 * Tests for FUN_0047ca50 and FUN_0047caa0 - Configuration data transfer
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

/* Source/Destination variables */
static byte DAT_004d36d8 = 0;
static byte DAT_004d36dc = 0;
static byte DAT_004d36e0 = 0;
static u32 DAT_04ebe2a8 = 0;
static u32 DAT_04ebe2ac = 0;
static u32 DAT_04ebe2b0 = 0;
static u32 DAT_04ebe2b4 = 0;

static byte DAT_04633330_byte0 = 0;
static byte DAT_04633330_byte1 = 0;
static byte DAT_04633330_byte2 = 0;
static u32 DAT_04633333 = 0;
static u32 DAT_04633337 = 0;
static u32 DAT_0463333b = 0;
static u32 DAT_0463333f = 0;

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
    DAT_004d36d8 = 0;
    DAT_004d36dc = 0;
    DAT_004d36e0 = 0;
    DAT_04ebe2a8 = 0;
    DAT_04ebe2ac = 0;
    DAT_04ebe2b0 = 0;
    DAT_04ebe2b4 = 0;

    DAT_04633330_byte0 = 0;
    DAT_04633330_byte1 = 0;
    DAT_04633330_byte2 = 0;
    DAT_04633333 = 0;
    DAT_04633337 = 0;
    DAT_0463333b = 0;
    DAT_0463333f = 0;
}

/* Set source variables to test values */
static void set_source_values(byte b0, byte b1, byte b2, u32 v3, u32 v4, u32 v5, u32 v6) {
    DAT_04633330_byte0 = b0;
    DAT_04633330_byte1 = b1;
    DAT_04633330_byte2 = b2;
    DAT_04633333 = v3;
    DAT_04633337 = v4;
    DAT_0463333b = v5;
    DAT_0463333f = v6;
}

/* Set destination variables to test values */
static void set_dest_values(byte b0, byte b1, byte b2, u32 v3, u32 v4, u32 v5, u32 v6) {
    DAT_004d36d8 = b0;
    DAT_004d36dc = b1;
    DAT_004d36e0 = b2;
    DAT_04ebe2a8 = v3;
    DAT_04ebe2ac = v4;
    DAT_04ebe2b0 = v5;
    DAT_04ebe2b4 = v6;
}

/*
 * FUN_0047caa0 - Copy from 04633330 to 004d36d8
 */
static void FUN_0047caa0(void) {
    DAT_004d36d8 = DAT_04633330_byte0;
    DAT_004d36dc = DAT_04633330_byte1;
    DAT_004d36e0 = DAT_04633330_byte2;
    DAT_04ebe2a8 = DAT_04633333;
    DAT_04ebe2ac = DAT_04633337;
    DAT_04ebe2b0 = DAT_0463333b;
    DAT_04ebe2b4 = DAT_0463333f;
}

/*
 * FUN_0047ca50 - Copy from 004d36d8 to 04633330
 */
static void FUN_0047ca50(void) {
    DAT_04633330_byte0 = DAT_004d36d8;
    DAT_04633330_byte1 = DAT_004d36dc;
    DAT_04633330_byte2 = DAT_004d36e0;
    DAT_04633333 = DAT_04ebe2a8;
    DAT_04633337 = DAT_04ebe2ac;
    DAT_0463333b = DAT_04ebe2b0;
    DAT_0463333f = DAT_04ebe2b4;
}

/* ========================================
 * Test Cases
 * ======================================== */

/* FUN_0047caa0 tests */
TEST(copy_to_dest_basic) {
    reset_test_state();
    set_source_values(0x12, 0x34, 0x56, 0x11111111, 0x22222222, 0x33333333, 0x44444444);

    FUN_0047caa0();

    ASSERT(DAT_004d36d8 == 0x12);
    ASSERT(DAT_004d36dc == 0x34);
    ASSERT(DAT_004d36e0 == 0x56);
    ASSERT(DAT_04ebe2a8 == 0x11111111);
    ASSERT(DAT_04ebe2ac == 0x22222222);
    ASSERT(DAT_04ebe2b0 == 0x33333333);
    ASSERT(DAT_04ebe2b4 == 0x44444444);
}

TEST(copy_to_dest_zero) {
    reset_test_state();

    FUN_0047caa0();

    ASSERT(DAT_004d36d8 == 0);
    ASSERT(DAT_004d36dc == 0);
    ASSERT(DAT_004d36e0 == 0);
    ASSERT(DAT_04ebe2a8 == 0);
    ASSERT(DAT_04ebe2ac == 0);
    ASSERT(DAT_04ebe2b0 == 0);
    ASSERT(DAT_04ebe2b4 == 0);
}

TEST(copy_to_dest_overwrites) {
    reset_test_state();
    set_dest_values(0xFF, 0xFF, 0xFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
    set_source_values(0x01, 0x02, 0x03, 0x100, 0x200, 0x300, 0x400);

    FUN_0047caa0();

    ASSERT(DAT_004d36d8 == 0x01);
    ASSERT(DAT_004d36dc == 0x02);
    ASSERT(DAT_004d36e0 == 0x03);
}

/* FUN_0047ca50 tests */
TEST(copy_to_source_basic) {
    reset_test_state();
    set_dest_values(0xAB, 0xCD, 0xEF, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD);

    FUN_0047ca50();

    ASSERT(DAT_04633330_byte0 == 0xAB);
    ASSERT(DAT_04633330_byte1 == 0xCD);
    ASSERT(DAT_04633330_byte2 == 0xEF);
    ASSERT(DAT_04633333 == 0xAAAAAAAA);
    ASSERT(DAT_04633337 == 0xBBBBBBBB);
    ASSERT(DAT_0463333b == 0xCCCCCCCC);
    ASSERT(DAT_0463333f == 0xDDDDDDDD);
}

TEST(copy_to_source_zero) {
    reset_test_state();

    FUN_0047ca50();

    ASSERT(DAT_04633330_byte0 == 0);
    ASSERT(DAT_04633330_byte1 == 0);
    ASSERT(DAT_04633330_byte2 == 0);
}

TEST(copy_to_source_overwrites) {
    reset_test_state();
    set_source_values(0xFF, 0xFF, 0xFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
    set_dest_values(0x11, 0x22, 0x33, 0x111, 0x222, 0x333, 0x444);

    FUN_0047ca50();

    ASSERT(DAT_04633330_byte0 == 0x11);
    ASSERT(DAT_04633330_byte1 == 0x22);
    ASSERT(DAT_04633330_byte2 == 0x33);
}

/* Round-trip tests */
TEST(roundtrip_source_to_dest_to_source) {
    reset_test_state();
    set_source_values(0x12, 0x34, 0x56, 0x11111111, 0x22222222, 0x33333333, 0x44444444);

    FUN_0047caa0();  /* Source -> Dest */
    set_source_values(0, 0, 0, 0, 0, 0, 0);  /* Clear source */
    FUN_0047ca50();  /* Dest -> Source */

    ASSERT(DAT_04633330_byte0 == 0x12);
    ASSERT(DAT_04633330_byte1 == 0x34);
    ASSERT(DAT_04633330_byte2 == 0x56);
    ASSERT(DAT_04633333 == 0x11111111);
    ASSERT(DAT_04633337 == 0x22222222);
    ASSERT(DAT_0463333b == 0x33333333);
    ASSERT(DAT_0463333f == 0x44444444);
}

TEST(roundtrip_dest_to_source_to_dest) {
    reset_test_state();
    set_dest_values(0xAB, 0xCD, 0xEF, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD);

    FUN_0047ca50();  /* Dest -> Source */
    set_dest_values(0, 0, 0, 0, 0, 0, 0);  /* Clear dest */
    FUN_0047caa0();  /* Source -> Dest */

    ASSERT(DAT_004d36d8 == 0xAB);
    ASSERT(DAT_004d36dc == 0xCD);
    ASSERT(DAT_004d36e0 == 0xEF);
    ASSERT(DAT_04ebe2a8 == 0xAAAAAAAA);
    ASSERT(DAT_04ebe2ac == 0xBBBBBBBB);
    ASSERT(DAT_04ebe2b0 == 0xCCCCCCCC);
    ASSERT(DAT_04ebe2b4 == 0xDDDDDDDD);
}

/* Byte value tests */
TEST(byte_values_min) {
    reset_test_state();
    set_source_values(0x00, 0x00, 0x00, 0, 0, 0, 0);

    FUN_0047caa0();

    ASSERT(DAT_004d36d8 == 0x00);
    ASSERT(DAT_004d36dc == 0x00);
    ASSERT(DAT_004d36e0 == 0x00);
}

TEST(byte_values_max) {
    reset_test_state();
    set_source_values(0xFF, 0xFF, 0xFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);

    FUN_0047caa0();

    ASSERT(DAT_004d36d8 == 0xFF);
    ASSERT(DAT_004d36dc == 0xFF);
    ASSERT(DAT_004d36e0 == 0xFF);
}

/* Independent field tests */
TEST(independent_fields) {
    reset_test_state();

    /* Set only one field in source */
    DAT_04633330_byte0 = 0x42;
    DAT_04633333 = 0x12345678;

    FUN_0047caa0();

    /* Check other fields are still 0 */
    ASSERT(DAT_004d36dc == 0);
    ASSERT(DAT_004d36e0 == 0);
    ASSERT(DAT_04ebe2ac == 0);

    /* Check set fields are copied */
    ASSERT(DAT_004d36d8 == 0x42);
    ASSERT(DAT_04ebe2a8 == 0x12345678);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Data Transfer Functions Unit Tests ===\n\n");

    printf("FUN_0047caa0 (Copy to Dest) Tests:\n");
    RUN_TEST(copy_to_dest_basic);
    RUN_TEST(copy_to_dest_zero);
    RUN_TEST(copy_to_dest_overwrites);

    printf("\nFUN_0047ca50 (Copy to Source) Tests:\n");
    RUN_TEST(copy_to_source_basic);
    RUN_TEST(copy_to_source_zero);
    RUN_TEST(copy_to_source_overwrites);

    printf("\nRound-trip Tests:\n");
    RUN_TEST(roundtrip_source_to_dest_to_source);
    RUN_TEST(roundtrip_dest_to_source_to_dest);

    printf("\nByte Value Tests:\n");
    RUN_TEST(byte_values_min);
    RUN_TEST(byte_values_max);

    printf("\nIndependent Field Tests:\n");
    RUN_TEST(independent_fields);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
