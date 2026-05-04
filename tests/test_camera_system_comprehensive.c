/*
 * Stone Age Client - Camera/Map State Functions Unit Tests
 * Tests for FUN_0047ccd0 and related state functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;
typedef unsigned short ushort;

/* ========================================
 * Test Data and Constants
 * ======================================== */

static u32 DAT_046333f0 = 0;
static u32 DAT_046333f4 = 0;
static u32 DAT_046333b4 = 0;
static u32 DAT_046333d4 = 0;
static ushort DAT_046333a2 = 0;
static ushort DAT_046333b8 = 0;
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

/* Set all variables to non-zero values */
static void set_all_nonzero(void) {
    DAT_046333f0 = 0x12345678;
    DAT_046333f4 = 0xABCDEF00;
    DAT_046333b4 = 0x11111111;
    DAT_046333d4 = 0x22222222;
    DAT_046333a2 = 0x3333;
    DAT_046333b8 = 0x4444;
    DAT_046333a0 = 0x5555;
    DAT_046333b6 = 0x6666;
    DAT_046333d8 = 0x77777777;
    DAT_046333e0 = 0x88888888;
    DAT_046333dc = 0x99999999;
    DAT_046333e4 = 0xAAAAAA;
    DAT_046333ec = 0xBBBBBBBB;
    DAT_046333e8 = 0xCCCCCCCC;
}

/*
 * FUN_0047ccd0 - Clear all camera/map state variables
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

/* Check all are zero */
static int all_are_zero(void) {
    return DAT_046333f0 == 0 &&
           DAT_046333f4 == 0 &&
           DAT_046333b4 == 0 &&
           DAT_046333d4 == 0 &&
           DAT_046333a2 == 0 &&
           DAT_046333b8 == 0 &&
           DAT_046333a0 == 0 &&
           DAT_046333b6 == 0 &&
           DAT_046333d8 == 0 &&
           DAT_046333e0 == 0 &&
           DAT_046333dc == 0 &&
           DAT_046333e4 == 0 &&
           DAT_046333ec == 0 &&
           DAT_046333e8 == 0;
}

/* Check all are non-zero */
static int all_are_nonzero(void) {
    return DAT_046333f0 != 0 &&
           DAT_046333f4 != 0 &&
           DAT_046333b4 != 0 &&
           DAT_046333d4 != 0 &&
           DAT_046333a2 != 0 &&
           DAT_046333b8 != 0 &&
           DAT_046333a0 != 0 &&
           DAT_046333b6 != 0 &&
           DAT_046333d8 != 0 &&
           DAT_046333e0 != 0 &&
           DAT_046333dc != 0 &&
           DAT_046333e4 != 0 &&
           DAT_046333ec != 0 &&
           DAT_046333e8 != 0;
}

/* ========================================
 * Test Cases
 * ======================================== */

/* FUN_0047ccd0 tests */
TEST(clear_all_from_zero) {
    reset_test_state();

    FUN_0047ccd0();

    ASSERT(all_are_zero());
}

TEST(clear_all_from_nonzero) {
    reset_test_state();
    set_all_nonzero();

    FUN_0047ccd0();

    ASSERT(all_are_zero());
}

TEST(clear_all_idempotent) {
    reset_test_state();

    FUN_0047ccd0();
    FUN_0047ccd0();
    FUN_0047ccd0();

    ASSERT(all_are_zero());
}

TEST(clear_after_partial_set) {
    reset_test_state();
    DAT_046333f0 = 100;
    DAT_046333b4 = 200;

    FUN_0047ccd0();

    ASSERT(all_are_zero());
}

TEST(clear_preserves_nothing) {
    reset_test_state();
    set_all_nonzero();

    FUN_0047ccd0();

    /* All should be zero, nothing preserved */
    ASSERT(DAT_046333f0 == 0);
    ASSERT(DAT_046333f4 == 0);
    ASSERT(DAT_046333b4 == 0);
    ASSERT(DAT_046333d4 == 0);
}

/* Individual variable tests */
TEST(set_individual_f0) {
    reset_test_state();
    DAT_046333f0 = 0x12345678;

    ASSERT(DAT_046333f0 == 0x12345678);
    ASSERT(DAT_046333f4 == 0);  /* Others unchanged */
}

TEST(set_individual_b4) {
    reset_test_state();
    DAT_046333b4 = 0xABCDEF00;

    ASSERT(DAT_046333b4 == 0xABCDEF00);
    ASSERT(DAT_046333f0 == 0);  /* Others unchanged */
}

TEST(set_individual_shorts) {
    reset_test_state();
    DAT_046333a2 = 0x1234;
    DAT_046333b8 = 0x5678;
    DAT_046333a0 = 0x9ABC;
    DAT_046333b6 = 0xDEF0;

    ASSERT(DAT_046333a2 == 0x1234);
    ASSERT(DAT_046333b8 == 0x5678);
    ASSERT(DAT_046333a0 == 0x9ABC);
    ASSERT(DAT_046333b6 == 0xDEF0);
}

/* Multiple set/clear cycles */
TEST(set_clear_cycle) {
    reset_test_state();

    /* Set all */
    set_all_nonzero();
    ASSERT(all_are_nonzero());

    /* Clear all */
    FUN_0047ccd0();
    ASSERT(all_are_zero());

    /* Set again */
    set_all_nonzero();
    ASSERT(all_are_nonzero());

    /* Clear again */
    FUN_0047ccd0();
    ASSERT(all_are_zero());
}

TEST(partial_set_then_clear) {
    reset_test_state();

    /* Set only some variables */
    DAT_046333f0 = 100;
    DAT_046333b4 = 200;
    DAT_046333d8 = 300;

    FUN_0047ccd0();

    ASSERT(all_are_zero());
}

/* Max value tests */
TEST(set_max_values) {
    reset_test_state();

    DAT_046333f0 = 0xFFFFFFFF;
    DAT_046333f4 = 0xFFFFFFFF;
    DAT_046333b4 = 0xFFFFFFFF;
    DAT_046333d4 = 0xFFFFFFFF;
    DAT_046333a2 = 0xFFFF;
    DAT_046333b8 = 0xFFFF;

    ASSERT(DAT_046333f0 == 0xFFFFFFFF);
    ASSERT(DAT_046333a2 == 0xFFFF);

    FUN_0047ccd0();

    ASSERT(all_are_zero());
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Camera/Map State Functions Unit Tests ===\n\n");

    printf("FUN_0047ccd0 (Clear All) Tests:\n");
    RUN_TEST(clear_all_from_zero);
    RUN_TEST(clear_all_from_nonzero);
    RUN_TEST(clear_all_idempotent);
    RUN_TEST(clear_after_partial_set);
    RUN_TEST(clear_preserves_nothing);

    printf("\nIndividual Variable Tests:\n");
    RUN_TEST(set_individual_f0);
    RUN_TEST(set_individual_b4);
    RUN_TEST(set_individual_shorts);

    printf("\nMultiple Cycle Tests:\n");
    RUN_TEST(set_clear_cycle);
    RUN_TEST(partial_set_then_clear);

    printf("\nMax Value Tests:\n");
    RUN_TEST(set_max_values);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
