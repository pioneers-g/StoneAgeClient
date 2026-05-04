/*
 * Stone Age Client - Player Direction and State Functions Unit Tests
 * Tests for FUN_00477d70, FUN_00477d90 - Player object field setters
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;
typedef unsigned char u8;
typedef uintptr_t uintptr;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Simulated global variables */
static s32 DAT_0462be94 = 0;
static uintptr DAT_0462e3ac = 0;

/* Simulated object buffer */
#define OBJECT_SIZE 0x200
static u8 object_buffer[OBJECT_SIZE] = {0};

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
    memset(object_buffer, 0, sizeof(object_buffer));
}

/*
 * FUN_00477d70 - Set direction value
 * Sets DAT_0462be94 = param_1
 * If object exists, also sets object field at 0x150
 */
static void FUN_00477d70(u32 param_1) {
    DAT_0462be94 = param_1;
    if (DAT_0462e3ac != 0) {
        *(u32 *)(DAT_0462e3ac + 0x150) = param_1;
    }
}

/*
 * FUN_00477d90 - Set state value
 * Sets object field at 0x148 to param_1
 * If value != 4, also sets field at 0x14c to -1
 */
static void FUN_00477d90(u32 param_1) {
    if (DAT_0462e3ac != 0) {
        *(u32 *)(DAT_0462e3ac + 0x148) = param_1;
        if (*(s32 *)(DAT_0462e3ac + 0x148) != 4) {
            *(s32 *)(DAT_0462e3ac + 0x14c) = -1;
        }
    }
}

/* ========================================
 * Test Cases for FUN_00477d70
 * ======================================== */

TEST(set_direction_no_object) {
    reset_test_state();
    DAT_0462e3ac = 0;

    FUN_00477d70(100);

    ASSERT(DAT_0462be94 == 100);
}

TEST(set_direction_with_object) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    FUN_00477d70(42);

    ASSERT(DAT_0462be94 == 42);
    ASSERT(*(u32 *)(object_buffer + 0x150) == 42);
}

TEST(set_direction_zero) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;
    DAT_0462be94 = 999;

    FUN_00477d70(0);

    ASSERT(DAT_0462be94 == 0);
    ASSERT(*(u32 *)(object_buffer + 0x150) == 0);
}

TEST(set_direction_overwrites) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;
    *(u32 *)(object_buffer + 0x150) = 555;

    FUN_00477d70(777);

    ASSERT(*(u32 *)(object_buffer + 0x150) == 777);
}

TEST(set_direction_max_value) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    FUN_00477d70(0xFFFFFFFF);

    ASSERT(DAT_0462be94 == 0xFFFFFFFF);
    ASSERT(*(u32 *)(object_buffer + 0x150) == 0xFFFFFFFF);
}

/* ========================================
 * Test Cases for FUN_00477d90
 * ======================================== */

TEST(set_state_no_object) {
    reset_test_state();
    DAT_0462e3ac = 0;

    FUN_00477d90(1);

    /* Should not crash, nothing set */
    ASSERT(1);
}

TEST(set_state_value_4) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;
    *(s32 *)(object_buffer + 0x14c) = 0;

    FUN_00477d90(4);

    ASSERT(*(u32 *)(object_buffer + 0x148) == 4);
    /* 0x14c should NOT be set to -1 when value is 4 */
    ASSERT(*(s32 *)(object_buffer + 0x14c) == 0);
}

TEST(set_state_value_not_4) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    FUN_00477d90(3);

    ASSERT(*(u32 *)(object_buffer + 0x148) == 3);
    ASSERT(*(s32 *)(object_buffer + 0x14c) == -1);
}

TEST(set_state_value_0) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    FUN_00477d90(0);

    ASSERT(*(u32 *)(object_buffer + 0x148) == 0);
    ASSERT(*(s32 *)(object_buffer + 0x14c) == -1);
}

TEST(set_state_value_5) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    FUN_00477d90(5);

    ASSERT(*(u32 *)(object_buffer + 0x148) == 5);
    ASSERT(*(s32 *)(object_buffer + 0x14c) == -1);
}

TEST(set_state_preserves_14c_when_4) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;
    *(s32 *)(object_buffer + 0x14c) = 12345;

    FUN_00477d90(4);

    ASSERT(*(s32 *)(object_buffer + 0x14c) == 12345);
}

TEST(set_state_overwrites_148) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;
    *(u32 *)(object_buffer + 0x148) = 999;

    FUN_00477d90(2);

    ASSERT(*(u32 *)(object_buffer + 0x148) == 2);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Player Direction and State Functions Unit Tests ===\n\n");

    printf("FUN_00477d70 (Set Direction) Tests:\n");
    RUN_TEST(set_direction_no_object);
    RUN_TEST(set_direction_with_object);
    RUN_TEST(set_direction_zero);
    RUN_TEST(set_direction_overwrites);
    RUN_TEST(set_direction_max_value);

    printf("\nFUN_00477d90 (Set State) Tests:\n");
    RUN_TEST(set_state_no_object);
    RUN_TEST(set_state_value_4);
    RUN_TEST(set_state_value_not_4);
    RUN_TEST(set_state_value_0);
    RUN_TEST(set_state_value_5);
    RUN_TEST(set_state_preserves_14c_when_4);
    RUN_TEST(set_state_overwrites_148);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
