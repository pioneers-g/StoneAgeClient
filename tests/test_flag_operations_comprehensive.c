/*
 * Stone Age Client - Flag Operations Unit Tests
 * Tests for FUN_00478090, FUN_004780a0, FUN_004780b0, FUN_004780c0,
 *          FUN_004780d0, FUN_004780e0, FUN_004780f0, FUN_00478110, FUN_00478130
 * These functions set/clear bits in the flags variable DAT_0462bf2c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;
typedef unsigned short ushort;
typedef unsigned char byte;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Main flag variable */
static u32 DAT_0462bf2c = 0;

/* Object pointer for FUN_004780f0, FUN_00478110, FUN_00478130 */
static byte* DAT_0462e3ac = NULL;
static ushort object_flag = 0;  /* Simulates offset 0x112 */

/* Flag bit definitions */
#define FLAG_0x0100   0x0100
#define FLAG_0x0200   0x0200
#define FLAG_0x4000   0x4000

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
    DAT_0462e3ac = NULL;
    object_flag = 0;
}

/*
 * FUN_00478090 - Set flag 0x0100
 */
static void FUN_00478090(void) {
    DAT_0462bf2c = DAT_0462bf2c | FLAG_0x0100;
}

/*
 * FUN_004780a0 - Clear flag 0x0100
 */
static void FUN_004780a0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xFFFFFEFF;
}

/*
 * FUN_004780b0 - Set flag 0x0200
 */
static void FUN_004780b0(void) {
    DAT_0462bf2c = DAT_0462bf2c | FLAG_0x0200;
}

/*
 * FUN_004780c0 - Clear flag 0x0200
 */
static void FUN_004780c0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xFFFFFDFF;
}

/*
 * FUN_004780d0 - Set flag 0x4000
 */
static void FUN_004780d0(void) {
    DAT_0462bf2c = DAT_0462bf2c | FLAG_0x4000;
}

/*
 * FUN_004780e0 - Clear flag 0x4000
 */
static void FUN_004780e0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xFFFFBFFF;
}

/*
 * FUN_004780f0 - Set object flag to 1
 */
static void FUN_004780f0(void) {
    if (DAT_0462e3ac != NULL) {
        object_flag = 1;
    }
}

/*
 * FUN_00478110 - Set object flag to 0
 */
static void FUN_00478110(void) {
    if (DAT_0462e3ac != NULL) {
        object_flag = 0;
    }
}

/*
 * FUN_00478130 - Get object flag
 */
static s32 FUN_00478130(void) {
    if (DAT_0462e3ac == NULL) {
        return 0;
    }
    return (s32)object_flag;
}

/* ========================================
 * Test Cases
 * ======================================== */

/* FUN_00478090 tests */
TEST(set_flag_0100_basic) {
    reset_test_state();

    FUN_00478090();

    ASSERT((DAT_0462bf2c & FLAG_0x0100) != 0);
}

TEST(set_flag_0100_already_set) {
    reset_test_state();
    DAT_0462bf2c = FLAG_0x0100;

    FUN_00478090();

    ASSERT((DAT_0462bf2c & FLAG_0x0100) != 0);
}

TEST(set_flag_0100_preserves_other) {
    reset_test_state();
    DAT_0462bf2c = 0xFFFF0000;

    FUN_00478090();

    ASSERT((DAT_0462bf2c & FLAG_0x0100) != 0);
    ASSERT((DAT_0462bf2c & 0xFFFF0000) == 0xFFFF0000);
}

/* FUN_004780a0 tests */
TEST(clear_flag_0100_basic) {
    reset_test_state();
    DAT_0462bf2c = FLAG_0x0100;

    FUN_004780a0();

    ASSERT((DAT_0462bf2c & FLAG_0x0100) == 0);
}

TEST(clear_flag_0100_already_clear) {
    reset_test_state();

    FUN_004780a0();

    ASSERT((DAT_0462bf2c & FLAG_0x0100) == 0);
}

TEST(clear_flag_0100_preserves_other) {
    reset_test_state();
    DAT_0462bf2c = FLAG_0x0100 | FLAG_0x0200 | FLAG_0x4000;

    FUN_004780a0();

    ASSERT((DAT_0462bf2c & FLAG_0x0100) == 0);
    ASSERT((DAT_0462bf2c & FLAG_0x0200) != 0);
    ASSERT((DAT_0462bf2c & FLAG_0x4000) != 0);
}

/* FUN_004780b0 tests */
TEST(set_flag_0200_basic) {
    reset_test_state();

    FUN_004780b0();

    ASSERT((DAT_0462bf2c & FLAG_0x0200) != 0);
}

TEST(set_flag_0200_preserves_other) {
    reset_test_state();
    DAT_0462bf2c = FLAG_0x0100;

    FUN_004780b0();

    ASSERT((DAT_0462bf2c & FLAG_0x0100) != 0);
    ASSERT((DAT_0462bf2c & FLAG_0x0200) != 0);
}

/* FUN_004780c0 tests */
TEST(clear_flag_0200_basic) {
    reset_test_state();
    DAT_0462bf2c = FLAG_0x0200;

    FUN_004780c0();

    ASSERT((DAT_0462bf2c & FLAG_0x0200) == 0);
}

TEST(clear_flag_0200_preserves_other) {
    reset_test_state();
    DAT_0462bf2c = FLAG_0x0100 | FLAG_0x0200;

    FUN_004780c0();

    ASSERT((DAT_0462bf2c & FLAG_0x0100) != 0);
    ASSERT((DAT_0462bf2c & FLAG_0x0200) == 0);
}

/* FUN_004780d0 tests */
TEST(set_flag_4000_basic) {
    reset_test_state();

    FUN_004780d0();

    ASSERT((DAT_0462bf2c & FLAG_0x4000) != 0);
}

TEST(set_flag_4000_already_set) {
    reset_test_state();
    DAT_0462bf2c = FLAG_0x4000;

    FUN_004780d0();

    ASSERT((DAT_0462bf2c & FLAG_0x4000) != 0);
}

/* FUN_004780e0 tests */
TEST(clear_flag_4000_basic) {
    reset_test_state();
    DAT_0462bf2c = FLAG_0x4000;

    FUN_004780e0();

    ASSERT((DAT_0462bf2c & FLAG_0x4000) == 0);
}

TEST(clear_flag_4000_preserves_other) {
    reset_test_state();
    DAT_0462bf2c = FLAG_0x4000 | 0x00003FFF;  /* 0x3FFF doesn't overlap with 0x4000 */

    FUN_004780e0();

    ASSERT((DAT_0462bf2c & FLAG_0x4000) == 0);
    ASSERT((DAT_0462bf2c & 0x00003FFF) == 0x00003FFF);
}

/* Combined flag tests */
TEST(set_clear_cycle_0100) {
    reset_test_state();

    FUN_00478090();
    ASSERT((DAT_0462bf2c & FLAG_0x0100) != 0);

    FUN_004780a0();
    ASSERT((DAT_0462bf2c & FLAG_0x0100) == 0);

    FUN_00478090();
    ASSERT((DAT_0462bf2c & FLAG_0x0100) != 0);
}

TEST(all_flags_set_and_clear) {
    reset_test_state();

    FUN_00478090();
    FUN_004780b0();
    FUN_004780d0();

    ASSERT((DAT_0462bf2c & FLAG_0x0100) != 0);
    ASSERT((DAT_0462bf2c & FLAG_0x0200) != 0);
    ASSERT((DAT_0462bf2c & FLAG_0x4000) != 0);

    FUN_004780a0();
    FUN_004780c0();
    FUN_004780e0();

    ASSERT((DAT_0462bf2c & FLAG_0x0100) == 0);
    ASSERT((DAT_0462bf2c & FLAG_0x0200) == 0);
    ASSERT((DAT_0462bf2c & FLAG_0x4000) == 0);
}

/* FUN_004780f0 tests */
TEST(set_object_flag_null) {
    reset_test_state();
    DAT_0462e3ac = NULL;

    FUN_004780f0();

    ASSERT(object_flag == 0);
}

TEST(set_object_flag_valid) {
    reset_test_state();
    DAT_0462e3ac = (byte*)1;  /* Non-NULL pointer */

    FUN_004780f0();

    ASSERT(object_flag == 1);
}

/* FUN_00478110 tests */
TEST(clear_object_flag_null) {
    reset_test_state();
    DAT_0462e3ac = NULL;
    object_flag = 1;

    FUN_00478110();

    ASSERT(object_flag == 1);  /* Should not change when NULL */
}

TEST(clear_object_flag_valid) {
    reset_test_state();
    DAT_0462e3ac = (byte*)1;
    object_flag = 1;

    FUN_00478110();

    ASSERT(object_flag == 0);
}

/* FUN_00478130 tests */
TEST(get_object_flag_null) {
    reset_test_state();
    DAT_0462e3ac = NULL;

    s32 result = FUN_00478130();

    ASSERT(result == 0);
}

TEST(get_object_flag_zero) {
    reset_test_state();
    DAT_0462e3ac = (byte*)1;
    object_flag = 0;

    s32 result = FUN_00478130();

    ASSERT(result == 0);
}

TEST(get_object_flag_one) {
    reset_test_state();
    DAT_0462e3ac = (byte*)1;
    object_flag = 1;

    s32 result = FUN_00478130();

    ASSERT(result == 1);
}

/* Object flag round-trip tests */
TEST(object_flag_roundtrip) {
    reset_test_state();
    DAT_0462e3ac = (byte*)1;

    FUN_004780f0();
    ASSERT(FUN_00478130() == 1);

    FUN_00478110();
    ASSERT(FUN_00478130() == 0);

    FUN_004780f0();
    ASSERT(FUN_00478130() == 1);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Flag Operations Unit Tests ===\n\n");

    printf("FUN_00478090 (Set Flag 0x0100) Tests:\n");
    RUN_TEST(set_flag_0100_basic);
    RUN_TEST(set_flag_0100_already_set);
    RUN_TEST(set_flag_0100_preserves_other);

    printf("\nFUN_004780a0 (Clear Flag 0x0100) Tests:\n");
    RUN_TEST(clear_flag_0100_basic);
    RUN_TEST(clear_flag_0100_already_clear);
    RUN_TEST(clear_flag_0100_preserves_other);

    printf("\nFUN_004780b0 (Set Flag 0x0200) Tests:\n");
    RUN_TEST(set_flag_0200_basic);
    RUN_TEST(set_flag_0200_preserves_other);

    printf("\nFUN_004780c0 (Clear Flag 0x0200) Tests:\n");
    RUN_TEST(clear_flag_0200_basic);
    RUN_TEST(clear_flag_0200_preserves_other);

    printf("\nFUN_004780d0 (Set Flag 0x4000) Tests:\n");
    RUN_TEST(set_flag_4000_basic);
    RUN_TEST(set_flag_4000_already_set);

    printf("\nFUN_004780e0 (Clear Flag 0x4000) Tests:\n");
    RUN_TEST(clear_flag_4000_basic);
    RUN_TEST(clear_flag_4000_preserves_other);

    printf("\nCombined Flag Tests:\n");
    RUN_TEST(set_clear_cycle_0100);
    RUN_TEST(all_flags_set_and_clear);

    printf("\nFUN_004780f0 (Set Object Flag) Tests:\n");
    RUN_TEST(set_object_flag_null);
    RUN_TEST(set_object_flag_valid);

    printf("\nFUN_00478110 (Clear Object Flag) Tests:\n");
    RUN_TEST(clear_object_flag_null);
    RUN_TEST(clear_object_flag_valid);

    printf("\nFUN_00478130 (Get Object Flag) Tests:\n");
    RUN_TEST(get_object_flag_null);
    RUN_TEST(get_object_flag_zero);
    RUN_TEST(get_object_flag_one);

    printf("\nObject Flag Round-trip Tests:\n");
    RUN_TEST(object_flag_roundtrip);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
