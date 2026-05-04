/*
 * Stone Age Client - Player Flag Bit Functions Unit Tests
 * Tests for FUN_00478090, FUN_004780a0, FUN_004780b0, FUN_004780c0, FUN_004780d0, FUN_004780e0
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

static u32 DAT_0462bf2c = 0;

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

/*
 * Flag manipulation functions
 */
static void FUN_00478090(void) { DAT_0462bf2c |= 0x100; }
static void FUN_004780a0(void) { DAT_0462bf2c &= ~0x100; }
static void FUN_004780b0(void) { DAT_0462bf2c |= 0x200; }
static void FUN_004780c0(void) { DAT_0462bf2c &= ~0x200; }
static void FUN_004780d0(void) { DAT_0462bf2c |= 0x4000; }
static void FUN_004780e0(void) { DAT_0462bf2c &= ~0x4000; }

/* ========================================
 * Test Cases
 * ======================================== */

TEST(set_bit8_from_zero) {
    DAT_0462bf2c = 0;
    FUN_00478090();
    ASSERT(DAT_0462bf2c == 0x100);
}

TEST(set_bit8_already_set) {
    DAT_0462bf2c = 0x100;
    FUN_00478090();
    ASSERT(DAT_0462bf2c == 0x100);
}

TEST(clear_bit8_from_set) {
    DAT_0462bf2c = 0x100;
    FUN_004780a0();
    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_bit8_already_clear) {
    DAT_0462bf2c = 0;
    FUN_004780a0();
    ASSERT(DAT_0462bf2c == 0);
}

TEST(set_bit9_from_zero) {
    DAT_0462bf2c = 0;
    FUN_004780b0();
    ASSERT(DAT_0462bf2c == 0x200);
}

TEST(set_bit9_already_set) {
    DAT_0462bf2c = 0x200;
    FUN_004780b0();
    ASSERT(DAT_0462bf2c == 0x200);
}

TEST(clear_bit9_from_set) {
    DAT_0462bf2c = 0x200;
    FUN_004780c0();
    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_bit9_already_clear) {
    DAT_0462bf2c = 0;
    FUN_004780c0();
    ASSERT(DAT_0462bf2c == 0);
}

TEST(set_bit14_from_zero) {
    DAT_0462bf2c = 0;
    FUN_004780d0();
    ASSERT(DAT_0462bf2c == 0x4000);
}

TEST(set_bit14_already_set) {
    DAT_0462bf2c = 0x4000;
    FUN_004780d0();
    ASSERT(DAT_0462bf2c == 0x4000);
}

TEST(clear_bit14_from_set) {
    DAT_0462bf2c = 0x4000;
    FUN_004780e0();
    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_bit14_already_clear) {
    DAT_0462bf2c = 0;
    FUN_004780e0();
    ASSERT(DAT_0462bf2c == 0);
}

TEST(preserves_other_bits_set) {
    DAT_0462bf2c = 0xFFFF;
    FUN_004780a0();
    ASSERT(DAT_0462bf2c == 0xFEFF);
    FUN_004780c0();
    ASSERT(DAT_0462bf2c == 0xFCFF);
    FUN_004780e0();
    ASSERT(DAT_0462bf2c == 0xBCFF);
}

TEST(preserves_other_bits_clear) {
    DAT_0462bf2c = 0;
    FUN_00478090();
    ASSERT(DAT_0462bf2c == 0x100);
    FUN_004780b0();
    ASSERT(DAT_0462bf2c == 0x300);
    FUN_004780d0();
    ASSERT(DAT_0462bf2c == 0x4300);
}

TEST(toggle_bit8) {
    DAT_0462bf2c = 0;

    FUN_00478090();
    ASSERT(DAT_0462bf2c == 0x100);

    FUN_004780a0();
    ASSERT(DAT_0462bf2c == 0);

    FUN_00478090();
    ASSERT(DAT_0462bf2c == 0x100);
}

TEST(toggle_bit9) {
    DAT_0462bf2c = 0;

    FUN_004780b0();
    ASSERT(DAT_0462bf2c == 0x200);

    FUN_004780c0();
    ASSERT(DAT_0462bf2c == 0);

    FUN_004780b0();
    ASSERT(DAT_0462bf2c == 0x200);
}

TEST(toggle_bit14) {
    DAT_0462bf2c = 0;

    FUN_004780d0();
    ASSERT(DAT_0462bf2c == 0x4000);

    FUN_004780e0();
    ASSERT(DAT_0462bf2c == 0);

    FUN_004780d0();
    ASSERT(DAT_0462bf2c == 0x4000);
}

TEST(independent_bits) {
    DAT_0462bf2c = 0;

    /* Set bit 8, verify others unchanged */
    FUN_00478090();
    ASSERT((DAT_0462bf2c & 0x100) != 0);
    ASSERT((DAT_0462bf2c & 0x200) == 0);
    ASSERT((DAT_0462bf2c & 0x4000) == 0);

    /* Set bit 9 */
    FUN_004780b0();
    ASSERT((DAT_0462bf2c & 0x100) != 0);
    ASSERT((DAT_0462bf2c & 0x200) != 0);
    ASSERT((DAT_0462bf2c & 0x4000) == 0);

    /* Set bit 14 */
    FUN_004780d0();
    ASSERT((DAT_0462bf2c & 0x100) != 0);
    ASSERT((DAT_0462bf2c & 0x200) != 0);
    ASSERT((DAT_0462bf2c & 0x4000) != 0);

    /* Clear bit 8 */
    FUN_004780a0();
    ASSERT((DAT_0462bf2c & 0x100) == 0);
    ASSERT((DAT_0462bf2c & 0x200) != 0);
    ASSERT((DAT_0462bf2c & 0x4000) != 0);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Player Flag Bit Functions Unit Tests ===\n\n");

    printf("Bit 8 Tests:\n");
    RUN_TEST(set_bit8_from_zero);
    RUN_TEST(set_bit8_already_set);
    RUN_TEST(clear_bit8_from_set);
    RUN_TEST(clear_bit8_already_clear);

    printf("\nBit 9 Tests:\n");
    RUN_TEST(set_bit9_from_zero);
    RUN_TEST(set_bit9_already_set);
    RUN_TEST(clear_bit9_from_set);
    RUN_TEST(clear_bit9_already_clear);

    printf("\nBit 14 Tests:\n");
    RUN_TEST(set_bit14_from_zero);
    RUN_TEST(set_bit14_already_set);
    RUN_TEST(clear_bit14_from_set);
    RUN_TEST(clear_bit14_already_clear);

    printf("\nPreservation Tests:\n");
    RUN_TEST(preserves_other_bits_set);
    RUN_TEST(preserves_other_bits_clear);

    printf("\nToggle Tests:\n");
    RUN_TEST(toggle_bit8);
    RUN_TEST(toggle_bit9);
    RUN_TEST(toggle_bit14);

    printf("\nIndependence Tests:\n");
    RUN_TEST(independent_bits);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
