/*
 * Stone Age Client - Player Flag Manipulation Functions Unit Tests
 * Tests for FUN_00478090, FUN_004780a0, FUN_004780b0, FUN_004780c0,
 *          FUN_004780d0, FUN_004780e0 - Flag set/clear operations
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

/* Reset test state */
static void reset_test_state(void) {
    DAT_0462bf2c = 0;
}

/*
 * Flag manipulation functions
 * These functions set or clear specific bits in DAT_0462bf2c
 */

/* FUN_00478090 - Set bit 8 (0x100) */
static void FUN_00478090(void) {
    DAT_0462bf2c = DAT_0462bf2c | 0x100;
}

/* FUN_004780a0 - Clear bit 8 (0x100) */
static void FUN_004780a0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xfffffeff;
}

/* FUN_004780b0 - Set bit 9 (0x200) */
static void FUN_004780b0(void) {
    DAT_0462bf2c = DAT_0462bf2c | 0x200;
}

/* FUN_004780c0 - Clear bit 9 (0x200) */
static void FUN_004780c0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xfffffdff;
}

/* FUN_004780d0 - Set bit 14 (0x4000) */
static void FUN_004780d0(void) {
    DAT_0462bf2c = DAT_0462bf2c | 0x4000;
}

/* FUN_004780e0 - Clear bit 14 (0x4000) */
static void FUN_004780e0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xffffbfff;
}

/* Helper functions */
static int is_flag_100_set(void) {
    return (DAT_0462bf2c & 0x100) != 0;
}

static int is_flag_200_set(void) {
    return (DAT_0462bf2c & 0x200) != 0;
}

static int is_flag_4000_set(void) {
    return (DAT_0462bf2c & 0x4000) != 0;
}

/* ========================================
 * Test Cases
 * ======================================== */

/* FUN_00478090 tests (set 0x100) */
TEST(set_flag_100_from_zero) {
    reset_test_state();

    FUN_00478090();

    ASSERT(is_flag_100_set());
    ASSERT(!is_flag_200_set());
    ASSERT(!is_flag_4000_set());
}

TEST(set_flag_100_already_set) {
    reset_test_state();
    DAT_0462bf2c = 0x100;

    FUN_00478090();

    ASSERT(is_flag_100_set());
    ASSERT(DAT_0462bf2c == 0x100);
}

TEST(set_flag_100_preserves_others) {
    reset_test_state();
    DAT_0462bf2c = 0x200 | 0x4000;

    FUN_00478090();

    ASSERT(is_flag_100_set());
    ASSERT(is_flag_200_set());
    ASSERT(is_flag_4000_set());
}

/* FUN_004780a0 tests (clear 0x100) */
TEST(clear_flag_100_from_set) {
    reset_test_state();
    DAT_0462bf2c = 0x100;

    FUN_004780a0();

    ASSERT(!is_flag_100_set());
}

TEST(clear_flag_100_from_zero) {
    reset_test_state();

    FUN_004780a0();

    ASSERT(!is_flag_100_set());
    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_100_preserves_others) {
    reset_test_state();
    DAT_0462bf2c = 0x100 | 0x200 | 0x4000;

    FUN_004780a0();

    ASSERT(!is_flag_100_set());
    ASSERT(is_flag_200_set());
    ASSERT(is_flag_4000_set());
}

/* FUN_004780b0 tests (set 0x200) */
TEST(set_flag_200_from_zero) {
    reset_test_state();

    FUN_004780b0();

    ASSERT(is_flag_200_set());
    ASSERT(!is_flag_100_set());
    ASSERT(!is_flag_4000_set());
}

TEST(set_flag_200_already_set) {
    reset_test_state();
    DAT_0462bf2c = 0x200;

    FUN_004780b0();

    ASSERT(is_flag_200_set());
    ASSERT(DAT_0462bf2c == 0x200);
}

TEST(set_flag_200_preserves_others) {
    reset_test_state();
    DAT_0462bf2c = 0x100 | 0x4000;

    FUN_004780b0();

    ASSERT(is_flag_100_set());
    ASSERT(is_flag_200_set());
    ASSERT(is_flag_4000_set());
}

/* FUN_004780c0 tests (clear 0x200) */
TEST(clear_flag_200_from_set) {
    reset_test_state();
    DAT_0462bf2c = 0x200;

    FUN_004780c0();

    ASSERT(!is_flag_200_set());
}

TEST(clear_flag_200_from_zero) {
    reset_test_state();

    FUN_004780c0();

    ASSERT(!is_flag_200_set());
    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_200_preserves_others) {
    reset_test_state();
    DAT_0462bf2c = 0x100 | 0x200 | 0x4000;

    FUN_004780c0();

    ASSERT(is_flag_100_set());
    ASSERT(!is_flag_200_set());
    ASSERT(is_flag_4000_set());
}

/* FUN_004780d0 tests (set 0x4000) */
TEST(set_flag_4000_from_zero) {
    reset_test_state();

    FUN_004780d0();

    ASSERT(is_flag_4000_set());
    ASSERT(!is_flag_100_set());
    ASSERT(!is_flag_200_set());
}

TEST(set_flag_4000_already_set) {
    reset_test_state();
    DAT_0462bf2c = 0x4000;

    FUN_004780d0();

    ASSERT(is_flag_4000_set());
    ASSERT(DAT_0462bf2c == 0x4000);
}

TEST(set_flag_4000_preserves_others) {
    reset_test_state();
    DAT_0462bf2c = 0x100 | 0x200;

    FUN_004780d0();

    ASSERT(is_flag_100_set());
    ASSERT(is_flag_200_set());
    ASSERT(is_flag_4000_set());
}

/* FUN_004780e0 tests (clear 0x4000) */
TEST(clear_flag_4000_from_set) {
    reset_test_state();
    DAT_0462bf2c = 0x4000;

    FUN_004780e0();

    ASSERT(!is_flag_4000_set());
}

TEST(clear_flag_4000_from_zero) {
    reset_test_state();

    FUN_004780e0();

    ASSERT(!is_flag_4000_set());
    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_4000_preserves_others) {
    reset_test_state();
    DAT_0462bf2c = 0x100 | 0x200 | 0x4000;

    FUN_004780e0();

    ASSERT(is_flag_100_set());
    ASSERT(is_flag_200_set());
    ASSERT(!is_flag_4000_set());
}

/* Combined tests */
TEST(all_flags_set_clear_cycle) {
    reset_test_state();

    /* Set all flags */
    FUN_00478090();
    FUN_004780b0();
    FUN_004780d0();

    ASSERT(DAT_0462bf2c == (0x100 | 0x200 | 0x4000));

    /* Clear all flags */
    FUN_004780a0();
    FUN_004780c0();
    FUN_004780e0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(flag_toggle_cycles) {
    reset_test_state();

    /* Toggle 0x100 */
    FUN_00478090();
    ASSERT(is_flag_100_set());
    FUN_004780a0();
    ASSERT(!is_flag_100_set());
    FUN_00478090();
    ASSERT(is_flag_100_set());

    /* Toggle 0x200 */
    FUN_004780b0();
    ASSERT(is_flag_200_set());
    FUN_004780c0();
    ASSERT(!is_flag_200_set());

    /* Toggle 0x4000 */
    FUN_004780d0();
    ASSERT(is_flag_4000_set());
    FUN_004780e0();
    ASSERT(!is_flag_4000_set());
}

TEST(independent_flag_operations) {
    reset_test_state();

    /* Set 0x100, check 0x200 and 0x4000 unaffected */
    FUN_00478090();
    ASSERT(is_flag_100_set());
    ASSERT(!is_flag_200_set());
    ASSERT(!is_flag_4000_set());

    /* Set 0x4000, check 0x100 preserved */
    FUN_004780d0();
    ASSERT(is_flag_100_set());
    ASSERT(!is_flag_200_set());
    ASSERT(is_flag_4000_set());

    /* Clear 0x100, check 0x4000 preserved */
    FUN_004780a0();
    ASSERT(!is_flag_100_set());
    ASSERT(!is_flag_200_set());
    ASSERT(is_flag_4000_set());
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Player Flag Manipulation Functions Unit Tests ===\n\n");

    printf("FUN_00478090 (Set 0x100) Tests:\n");
    RUN_TEST(set_flag_100_from_zero);
    RUN_TEST(set_flag_100_already_set);
    RUN_TEST(set_flag_100_preserves_others);

    printf("\nFUN_004780a0 (Clear 0x100) Tests:\n");
    RUN_TEST(clear_flag_100_from_set);
    RUN_TEST(clear_flag_100_from_zero);
    RUN_TEST(clear_flag_100_preserves_others);

    printf("\nFUN_004780b0 (Set 0x200) Tests:\n");
    RUN_TEST(set_flag_200_from_zero);
    RUN_TEST(set_flag_200_already_set);
    RUN_TEST(set_flag_200_preserves_others);

    printf("\nFUN_004780c0 (Clear 0x200) Tests:\n");
    RUN_TEST(clear_flag_200_from_set);
    RUN_TEST(clear_flag_200_from_zero);
    RUN_TEST(clear_flag_200_preserves_others);

    printf("\nFUN_004780d0 (Set 0x4000) Tests:\n");
    RUN_TEST(set_flag_4000_from_zero);
    RUN_TEST(set_flag_4000_already_set);
    RUN_TEST(set_flag_4000_preserves_others);

    printf("\nFUN_004780e0 (Clear 0x4000) Tests:\n");
    RUN_TEST(clear_flag_4000_from_set);
    RUN_TEST(clear_flag_4000_from_zero);
    RUN_TEST(clear_flag_4000_preserves_others);

    printf("\nCombined Tests:\n");
    RUN_TEST(all_flags_set_clear_cycle);
    RUN_TEST(flag_toggle_cycles);
    RUN_TEST(independent_flag_operations);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
