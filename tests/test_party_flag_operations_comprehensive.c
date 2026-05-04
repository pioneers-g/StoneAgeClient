/*
 * Stone Age Client - Party Flag Bit Operations Unit Tests
 * Tests for simple flag set/clear functions
 * FUN_00478090 - Set bit 8 (0x100)
 * FUN_004780a0 - Clear bit 8 (0x100)
 * FUN_004780c0 - Clear bit 9 (0x200)
 * FUN_004780d0 - Set bit 14 (0x4000)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Simulated global flag variable */
static s32 DAT_0462bf2c = 0;

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
    DAT_0462bf2c = 0;
}

/*
 * FUN_00478090 - Set bit 8 (0x100)
 */
static void FUN_00478090(void) {
    DAT_0462bf2c = DAT_0462bf2c | 0x100;
}

/*
 * FUN_004780a0 - Clear bit 8 (0x100)
 */
static void FUN_004780a0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xfffffeff;
}

/*
 * FUN_004780c0 - Clear bit 9 (0x200)
 */
static void FUN_004780c0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xfffffdff;
}

/*
 * FUN_004780d0 - Set bit 14 (0x4000)
 */
static void FUN_004780d0(void) {
    DAT_0462bf2c = DAT_0462bf2c | 0x4000;
}

/* ========================================
 * Test Cases for FUN_00478090 (Set Bit 8)
 * ======================================== */

TEST(set_bit_8_from_zero) {
    reset_test_state();

    FUN_00478090();

    ASSERT(DAT_0462bf2c == 0x100);
}

TEST(set_bit_8_already_set) {
    reset_test_state();
    DAT_0462bf2c = 0x100;

    FUN_00478090();

    ASSERT(DAT_0462bf2c == 0x100);
}

TEST(set_bit_8_preserves_other_bits) {
    reset_test_state();
    DAT_0462bf2c = 0xFFFFFEFF;

    FUN_00478090();

    ASSERT(DAT_0462bf2c == 0xFFFFFFFF);
}

/* ========================================
 * Test Cases for FUN_004780a0 (Clear Bit 8)
 * ======================================== */

TEST(clear_bit_8_from_zero) {
    reset_test_state();

    FUN_004780a0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_bit_8_set) {
    reset_test_state();
    DAT_0462bf2c = 0x100;

    FUN_004780a0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_bit_8_preserves_other_bits) {
    reset_test_state();
    DAT_0462bf2c = 0xFFFFFFFF;

    FUN_004780a0();

    ASSERT(DAT_0462bf2c == 0xFFFFFEFF);
}

/* ========================================
 * Test Cases for FUN_004780c0 (Clear Bit 9)
 * ======================================== */

TEST(clear_bit_9_from_zero) {
    reset_test_state();

    FUN_004780c0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_bit_9_set) {
    reset_test_state();
    DAT_0462bf2c = 0x200;

    FUN_004780c0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_bit_9_preserves_other_bits) {
    reset_test_state();
    DAT_0462bf2c = 0xFFFFFFFF;

    FUN_004780c0();

    ASSERT(DAT_0462bf2c == 0xFFFFFDFF);
}

/* ========================================
 * Test Cases for FUN_004780d0 (Set Bit 14)
 * ======================================== */

TEST(set_bit_14_from_zero) {
    reset_test_state();

    FUN_004780d0();

    ASSERT(DAT_0462bf2c == 0x4000);
}

TEST(set_bit_14_already_set) {
    reset_test_state();
    DAT_0462bf2c = 0x4000;

    FUN_004780d0();

    ASSERT(DAT_0462bf2c == 0x4000);
}

TEST(set_bit_14_preserves_other_bits) {
    reset_test_state();
    DAT_0462bf2c = 0xFFFFBFFF;

    FUN_004780d0();

    ASSERT(DAT_0462bf2c == 0xFFFFFFFF);
}

/* ========================================
 * Combined Tests
 * ======================================== */

TEST(set_and_clear_bit_8) {
    reset_test_state();

    FUN_00478090();
    ASSERT(DAT_0462bf2c == 0x100);

    FUN_004780a0();
    ASSERT(DAT_0462bf2c == 0);
}

TEST(multiple_bit_operations) {
    reset_test_state();

    FUN_00478090();  /* Set bit 8 */
    FUN_004780d0();  /* Set bit 14 */
    ASSERT(DAT_0462bf2c == 0x4100);

    FUN_004780a0();  /* Clear bit 8 */
    ASSERT(DAT_0462bf2c == 0x4000);

    FUN_004780c0();  /* Clear bit 9 (already clear) */
    ASSERT(DAT_0462bf2c == 0x4000);
}

TEST(set_clear_multiple_times) {
    reset_test_state();

    for (int i = 0; i < 5; i++) {
        FUN_00478090();
        ASSERT(DAT_0462bf2c == 0x100);
        FUN_004780a0();
        ASSERT(DAT_0462bf2c == 0);
    }
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Party Flag Bit Operations Unit Tests ===\n\n");

    printf("FUN_00478090 (Set Bit 8) Tests:\n");
    RUN_TEST(set_bit_8_from_zero);
    RUN_TEST(set_bit_8_already_set);
    RUN_TEST(set_bit_8_preserves_other_bits);

    printf("\nFUN_004780a0 (Clear Bit 8) Tests:\n");
    RUN_TEST(clear_bit_8_from_zero);
    RUN_TEST(clear_bit_8_set);
    RUN_TEST(clear_bit_8_preserves_other_bits);

    printf("\nFUN_004780c0 (Clear Bit 9) Tests:\n");
    RUN_TEST(clear_bit_9_from_zero);
    RUN_TEST(clear_bit_9_set);
    RUN_TEST(clear_bit_9_preserves_other_bits);

    printf("\nFUN_004780d0 (Set Bit 14) Tests:\n");
    RUN_TEST(set_bit_14_from_zero);
    RUN_TEST(set_bit_14_already_set);
    RUN_TEST(set_bit_14_preserves_other_bits);

    printf("\nCombined Tests:\n");
    RUN_TEST(set_and_clear_bit_8);
    RUN_TEST(multiple_bit_operations);
    RUN_TEST(set_clear_multiple_times);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
