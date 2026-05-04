/*
 * Stone Age Client - Player Value Setter Function Unit Tests
 * Tests for FUN_00478190 - Set player value
 * This function sets a global variable and optionally updates a field in an object
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;
typedef unsigned char u8;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Simulated global variables */
static s32 DAT_0462e3b0 = 0;
static s32 DAT_0462e3ac = 0;

/* Simulated object buffer */
static u8 object_buffer[512] = {0};

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
    DAT_0462e3b0 = 0;
    DAT_0462e3ac = 0;
    memset(object_buffer, 0, sizeof(object_buffer));
}

/*
 * FUN_00478190 - Set player value
 * Sets DAT_0462e3b0 = param_1
 * If DAT_0462e3ac != 0, also sets *(DAT_0462e3ac + 0x98) = param_1
 */
static void FUN_00478190(s32 param_1) {
    DAT_0462e3b0 = param_1;
    if (DAT_0462e3ac != 0) {
        *(s32 *)(DAT_0462e3ac + 0x98) = param_1;
    }
}

/* ========================================
 * Test Cases for FUN_00478190
 * ======================================== */

TEST(set_value_zero) {
    reset_test_state();
    FUN_00478190(0);

    ASSERT(DAT_0462e3b0 == 0);
}

TEST(set_value_positive) {
    reset_test_state();
    FUN_00478190(100);

    ASSERT(DAT_0462e3b0 == 100);
}

TEST(set_value_negative) {
    reset_test_state();
    FUN_00478190(-50);

    ASSERT(DAT_0462e3b0 == -50);
}

TEST(set_value_large) {
    reset_test_state();
    FUN_00478190(1000000);

    ASSERT(DAT_0462e3b0 == 1000000);
}

TEST(set_value_with_object) {
    reset_test_state();
    DAT_0462e3ac = (s32)object_buffer;
    FUN_00478190(42);

    ASSERT(DAT_0462e3b0 == 42);
    ASSERT(*(s32 *)(object_buffer + 0x98) == 42);
}

TEST(set_value_no_object) {
    reset_test_state();
    DAT_0462e3ac = 0;
    FUN_00478190(99);

    ASSERT(DAT_0462e3b0 == 99);
}

TEST(set_value_overwrites) {
    reset_test_state();
    DAT_0462e3b0 = 1000;
    FUN_00478190(200);

    ASSERT(DAT_0462e3b0 == 200);
}

TEST(set_value_object_overwrites) {
    reset_test_state();
    DAT_0462e3ac = (s32)object_buffer;
    *(s32 *)(object_buffer + 0x98) = 500;
    FUN_00478190(300);

    ASSERT(DAT_0462e3b0 == 300);
    ASSERT(*(s32 *)(object_buffer + 0x98) == 300);
}

TEST(set_value_max_int) {
    reset_test_state();
    FUN_00478190(0x7FFFFFFF);

    ASSERT(DAT_0462e3b0 == 0x7FFFFFFF);
}

TEST(set_value_min_int) {
    reset_test_state();
    FUN_00478190(0x80000000);

    ASSERT(DAT_0462e3b0 == (s32)0x80000000);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Player Value Setter Function Unit Tests ===\n\n");

    printf("FUN_00478190 (Set Player Value) Tests:\n");
    RUN_TEST(set_value_zero);
    RUN_TEST(set_value_positive);
    RUN_TEST(set_value_negative);
    RUN_TEST(set_value_large);
    RUN_TEST(set_value_with_object);
    RUN_TEST(set_value_no_object);
    RUN_TEST(set_value_overwrites);
    RUN_TEST(set_value_object_overwrites);
    RUN_TEST(set_value_max_int);
    RUN_TEST(set_value_min_int);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
