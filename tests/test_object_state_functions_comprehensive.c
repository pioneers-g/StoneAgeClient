/*
 * Stone Age Client - Object State Functions Unit Tests
 * Tests for FUN_00477d70, FUN_00477d90
 * These functions handle object state management
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;
typedef unsigned char u8;
typedef unsigned short u16;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Global variables */
static u32 DAT_0462be94 = 0;
static u32 DAT_0462e3ac = 0;
static u8 g_object_data[0x200] = {0};

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
    DAT_0462be94 = 0;
    DAT_0462e3ac = 0;
    memset(g_object_data, 0, sizeof(g_object_data));
}

/*
 * FUN_00477d70 - Set direction value
 * Sets global and object field at offset 0x150
 */
static void FUN_00477d70(u32 param_1) {
    DAT_0462be94 = param_1;
    if (DAT_0462e3ac != 0) {
        *(u32*)(DAT_0462e3ac + 0x150) = param_1;
    }
}

/*
 * FUN_00477d90 - Set object state
 * Sets object field at offset 0x148, conditionally sets 0x14c to -1
 */
static void FUN_00477d90(u32 param_1) {
    if (DAT_0462e3ac != 0) {
        *(u32*)(DAT_0462e3ac + 0x148) = param_1;
        if (*(s32*)(DAT_0462e3ac + 0x148) != 4) {
            *(u32*)(DAT_0462e3ac + 0x14c) = 0xFFFFFFFF;
        }
    }
}

/* ========================================
 * Test Cases for FUN_00477d70 (Set Direction)
 * ======================================== */

TEST(set_direction_no_object) {
    reset_test_state();

    DAT_0462e3ac = 0;
    FUN_00477d70(8);

    ASSERT(DAT_0462be94 == 8);
}

TEST(set_direction_with_object) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    FUN_00477d70(5);

    ASSERT(DAT_0462be94 == 5);
    ASSERT(*(u32*)(g_object_data + 0x150) == 5);
}

TEST(set_direction_zero) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    DAT_0462be94 = 0xFF;

    FUN_00477d70(0);

    ASSERT(DAT_0462be94 == 0);
    ASSERT(*(u32*)(g_object_data + 0x150) == 0);
}

TEST(set_direction_large_value) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    FUN_00477d70(0x12345678);

    ASSERT(DAT_0462be94 == 0x12345678);
    ASSERT(*(u32*)(g_object_data + 0x150) == 0x12345678);
}

TEST(set_direction_overwrites) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    *(u32*)(g_object_data + 0x150) = 0xFFFFFFFF;

    FUN_00477d70(1);

    ASSERT(*(u32*)(g_object_data + 0x150) == 1);
}

/* ========================================
 * Test Cases for FUN_00477d90 (Set Object State)
 * ======================================== */

TEST(set_object_state_no_object) {
    reset_test_state();

    DAT_0462e3ac = 0;
    FUN_00477d90(3);

    /* Should not crash */
    ASSERT(1);
}

TEST(set_object_state_not_four) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    FUN_00477d90(3);

    ASSERT(*(u32*)(g_object_data + 0x148) == 3);
    ASSERT(*(u32*)(g_object_data + 0x14c) == 0xFFFFFFFF);
}

TEST(set_object_state_is_four) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    *(u32*)(g_object_data + 0x14c) = 0x12345678;

    FUN_00477d90(4);

    ASSERT(*(u32*)(g_object_data + 0x148) == 4);
    /* Should NOT change 0x14c when state is 4 */
    ASSERT(*(u32*)(g_object_data + 0x14c) == 0x12345678);
}

TEST(set_object_state_zero) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    FUN_00477d90(0);

    ASSERT(*(u32*)(g_object_data + 0x148) == 0);
    ASSERT(*(u32*)(g_object_data + 0x14c) == 0xFFFFFFFF);
}

TEST(set_object_state_large_value) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    FUN_00477d90(100);

    ASSERT(*(u32*)(g_object_data + 0x148) == 100);
    ASSERT(*(u32*)(g_object_data + 0x14c) == 0xFFFFFFFF);
}

TEST(set_object_state_five) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    FUN_00477d90(5);

    ASSERT(*(u32*)(g_object_data + 0x148) == 5);
    ASSERT(*(u32*)(g_object_data + 0x14c) == 0xFFFFFFFF);
}

/* ========================================
 * Combined Tests
 * ======================================== */

TEST(set_direction_then_state) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;

    FUN_00477d70(7);
    FUN_00477d90(3);

    ASSERT(DAT_0462be94 == 7);
    ASSERT(*(u32*)(g_object_data + 0x150) == 7);
    ASSERT(*(u32*)(g_object_data + 0x148) == 3);
    ASSERT(*(u32*)(g_object_data + 0x14c) == 0xFFFFFFFF);
}

TEST(set_state_then_direction) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;

    FUN_00477d90(4);
    FUN_00477d70(2);

    ASSERT(DAT_0462be94 == 2);
    ASSERT(*(u32*)(g_object_data + 0x148) == 4);
    ASSERT(*(u32*)(g_object_data + 0x150) == 2);
}

TEST(null_object_all_safe) {
    reset_test_state();

    DAT_0462e3ac = 0;

    /* Both should be safe with null object */
    FUN_00477d70(10);
    FUN_00477d90(5);

    ASSERT(DAT_0462be94 == 10);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Object State Functions Unit Tests ===\n\n");

    printf("FUN_00477d70 (Set Direction) Tests:\n");
    RUN_TEST(set_direction_no_object);
    RUN_TEST(set_direction_with_object);
    RUN_TEST(set_direction_zero);
    RUN_TEST(set_direction_large_value);
    RUN_TEST(set_direction_overwrites);

    printf("\nFUN_00477d90 (Set Object State) Tests:\n");
    RUN_TEST(set_object_state_no_object);
    RUN_TEST(set_object_state_not_four);
    RUN_TEST(set_object_state_is_four);
    RUN_TEST(set_object_state_zero);
    RUN_TEST(set_object_state_large_value);
    RUN_TEST(set_object_state_five);

    printf("\nCombined Tests:\n");
    RUN_TEST(set_direction_then_state);
    RUN_TEST(set_state_then_direction);
    RUN_TEST(null_object_all_safe);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
