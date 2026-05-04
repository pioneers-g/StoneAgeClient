/*
 * Stone Age Client - Object State Setters Unit Tests
 * Tests for FUN_00477d90, FUN_00477ca0
 * These functions set state values on the game object
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

/* Global variable */
static u32 DAT_0462be90 = 0;

/* Object pointer */
static u32* DAT_0462e3ac = NULL;

/* Object fields - simulate offsets */
#define OBJ_OFFSET_0x148  0x52
#define OBJ_OFFSET_0x14C  0x53

#define OBJ_SIZE 256
static u32 test_object[OBJ_SIZE];

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
    DAT_0462be90 = 0;
    DAT_0462e3ac = NULL;
    memset(test_object, 0, sizeof(test_object));
}

/*
 * FUN_00477d90 - Set state with conditional reset
 * If the value is not 4, also sets offset 0x14C to -1
 */
static void FUN_00477d90(u32 param_1) {
    if (DAT_0462e3ac != NULL) {
        test_object[OBJ_OFFSET_0x148] = param_1;
        if (test_object[OBJ_OFFSET_0x148] != 4) {
            test_object[OBJ_OFFSET_0x14C] = 0xFFFFFFFF;
        }
    }
}

/*
 * FUN_00477ca0 - Simple setter
 */
static void FUN_00477ca0(u32 param_1) {
    DAT_0462be90 = param_1;
}

/* ========================================
 * Test Cases for FUN_00477d90
 * ======================================== */

TEST(set_state_basic) {
    reset_test_state();
    DAT_0462e3ac = test_object;

    FUN_00477d90(1);

    ASSERT(test_object[OBJ_OFFSET_0x148] == 1);
    ASSERT(test_object[OBJ_OFFSET_0x14C] == 0xFFFFFFFF);
}

TEST(set_state_value_4) {
    reset_test_state();
    DAT_0462e3ac = test_object;

    FUN_00477d90(4);

    ASSERT(test_object[OBJ_OFFSET_0x148] == 4);
    ASSERT(test_object[OBJ_OFFSET_0x14C] == 0);  /* Not reset */
}

TEST(set_state_zero) {
    reset_test_state();
    DAT_0462e3ac = test_object;

    FUN_00477d90(0);

    ASSERT(test_object[OBJ_OFFSET_0x148] == 0);
    ASSERT(test_object[OBJ_OFFSET_0x14C] == 0xFFFFFFFF);
}

TEST(set_state_null_object) {
    reset_test_state();
    DAT_0462e3ac = NULL;

    FUN_00477d90(1);

    /* Should not crash */
    ASSERT(1);
}

TEST(set_state_preserves_14c_when_4) {
    reset_test_state();
    DAT_0462e3ac = test_object;
    test_object[OBJ_OFFSET_0x14C] = 12345;

    FUN_00477d90(4);

    ASSERT(test_object[OBJ_OFFSET_0x14C] == 12345);  /* Preserved */
}

TEST(set_state_overwrites_14c_when_not_4) {
    reset_test_state();
    DAT_0462e3ac = test_object;
    test_object[OBJ_OFFSET_0x14C] = 12345;

    FUN_00477d90(5);

    ASSERT(test_object[OBJ_OFFSET_0x14C] == 0xFFFFFFFF);  /* Reset */
}

TEST(set_state_various_values) {
    reset_test_state();
    DAT_0462e3ac = test_object;

    /* Test values 0, 1, 2, 3 - should all reset 14C */
    for (u32 val = 0; val < 4; val++) {
        test_object[OBJ_OFFSET_0x14C] = 0;
        FUN_00477d90(val);
        ASSERT(test_object[OBJ_OFFSET_0x148] == val);
        ASSERT(test_object[OBJ_OFFSET_0x14C] == 0xFFFFFFFF);
    }

    /* Value 4 - should NOT reset 14C */
    test_object[OBJ_OFFSET_0x14C] = 0;
    FUN_00477d90(4);
    ASSERT(test_object[OBJ_OFFSET_0x148] == 4);
    ASSERT(test_object[OBJ_OFFSET_0x14C] == 0);

    /* Values 5+ - should reset 14C */
    for (u32 val = 5; val < 10; val++) {
        test_object[OBJ_OFFSET_0x14C] = 0;
        FUN_00477d90(val);
        ASSERT(test_object[OBJ_OFFSET_0x148] == val);
        ASSERT(test_object[OBJ_OFFSET_0x14C] == 0xFFFFFFFF);
    }
}

TEST(set_state_max_value) {
    reset_test_state();
    DAT_0462e3ac = test_object;

    FUN_00477d90(0xFFFFFFFF);

    ASSERT(test_object[OBJ_OFFSET_0x148] == 0xFFFFFFFF);
    ASSERT(test_object[OBJ_OFFSET_0x14C] == 0xFFFFFFFF);
}

/* ========================================
 * Test Cases for FUN_00477ca0
 * ======================================== */

TEST(set_be90_basic) {
    reset_test_state();

    FUN_00477ca0(12345);

    ASSERT(DAT_0462be90 == 12345);
}

TEST(set_be90_zero) {
    reset_test_state();
    DAT_0462be90 = 99999;

    FUN_00477ca0(0);

    ASSERT(DAT_0462be90 == 0);
}

TEST(set_be90_overwrites) {
    reset_test_state();
    DAT_0462be90 = 11111;

    FUN_00477ca0(22222);

    ASSERT(DAT_0462be90 == 22222);
}

TEST(set_be90_max_value) {
    reset_test_state();

    FUN_00477ca0(0xFFFFFFFF);

    ASSERT(DAT_0462be90 == 0xFFFFFFFF);
}

TEST(set_be90_multiple_times) {
    reset_test_state();

    FUN_00477ca0(100);
    ASSERT(DAT_0462be90 == 100);

    FUN_00477ca0(200);
    ASSERT(DAT_0462be90 == 200);

    FUN_00477ca0(300);
    ASSERT(DAT_0462be90 == 300);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Object State Setters Unit Tests ===\n\n");

    printf("FUN_00477d90 (Set State with Conditional) Tests:\n");
    RUN_TEST(set_state_basic);
    RUN_TEST(set_state_value_4);
    RUN_TEST(set_state_zero);
    RUN_TEST(set_state_null_object);
    RUN_TEST(set_state_preserves_14c_when_4);
    RUN_TEST(set_state_overwrites_14c_when_not_4);
    RUN_TEST(set_state_various_values);
    RUN_TEST(set_state_max_value);

    printf("\nFUN_00477ca0 (Set BE90) Tests:\n");
    RUN_TEST(set_be90_basic);
    RUN_TEST(set_be90_zero);
    RUN_TEST(set_be90_overwrites);
    RUN_TEST(set_be90_max_value);
    RUN_TEST(set_be90_multiple_times);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
