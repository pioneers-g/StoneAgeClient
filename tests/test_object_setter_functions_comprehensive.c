/*
 * Stone Age Client - Object Setter Functions Unit Tests
 * Tests for FUN_00477ca0, FUN_00477c70, FUN_00477cd0
 * These functions set object properties and global variables
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
static u32 DAT_0462be90 = 0;
static u32 DAT_0462be88 = 0;
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
    DAT_0462be90 = 0;
    DAT_0462be88 = 0;
    DAT_0462be94 = 0;
    DAT_0462e3ac = 0;
    memset(g_object_data, 0, sizeof(g_object_data));
}

/*
 * FUN_00477ca0 - Set DAT_0462be90
 * Simple setter for a global variable
 */
static void FUN_00477ca0(u32 param_1) {
    DAT_0462be90 = param_1;
}

/*
 * FUN_00477c70 - Set object properties
 * Sets both the global variables and object fields if object exists
 */
static void FUN_00477c70(u32 param_1, u32 param_2) {
    DAT_0462be88 = param_1;
    DAT_0462be94 = param_2;
    if (DAT_0462e3ac != 0) {
        *(u32*)(DAT_0462e3ac + 0x140) = param_1;
        *(u32*)(DAT_0462e3ac + 0x150) = param_2;
    }
}

/*
 * FUN_00477cd0 - Set object direction
 * Sets direction in object structure
 */
static void FUN_00477cd0(void) {
    if (DAT_0462e3ac != 0) {
        *(u16*)(DAT_0462e3ac + 0x114) = (u16)DAT_0462be90;
    }
}

/* ========================================
 * Test Cases for FUN_00477ca0 (Simple Setter)
 * ======================================== */

TEST(set_be90_basic) {
    reset_test_state();

    FUN_00477ca0(0x12345678);

    ASSERT(DAT_0462be90 == 0x12345678);
}

TEST(set_be90_zero) {
    reset_test_state();

    DAT_0462be90 = 0xFFFFFFFF;
    FUN_00477ca0(0);

    ASSERT(DAT_0462be90 == 0);
}

TEST(set_be90_overwrite) {
    reset_test_state();

    DAT_0462be90 = 0xAAAAAAAA;
    FUN_00477ca0(0x55555555);

    ASSERT(DAT_0462be90 == 0x55555555);
}

/* ========================================
 * Test Cases for FUN_00477c70 (Object Properties Setter)
 * ======================================== */

TEST(set_object_props_no_object) {
    reset_test_state();

    DAT_0462e3ac = 0;
    FUN_00477c70(0x11111111, 0x22222222);

    ASSERT(DAT_0462be88 == 0x11111111);
    ASSERT(DAT_0462be94 == 0x22222222);
}

TEST(set_object_props_with_object) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    FUN_00477c70(0x11111111, 0x22222222);

    ASSERT(DAT_0462be88 == 0x11111111);
    ASSERT(DAT_0462be94 == 0x22222222);
    ASSERT(*(u32*)(g_object_data + 0x140) == 0x11111111);
    ASSERT(*(u32*)(g_object_data + 0x150) == 0x22222222);
}

TEST(set_object_props_preserves_globals) {
    reset_test_state();

    DAT_0462be88 = 0xFFFFFFFF;
    DAT_0462be94 = 0xFFFFFFFF;

    FUN_00477c70(0, 0);

    ASSERT(DAT_0462be88 == 0);
    ASSERT(DAT_0462be94 == 0);
}

TEST(set_object_props_null_object_no_crash) {
    reset_test_state();

    DAT_0462e3ac = 0;
    /* Should not crash when object is null */
    FUN_00477c70(0x12345678, 0x9ABCDEF0);

    ASSERT(DAT_0462be88 == 0x12345678);
    ASSERT(DAT_0462be94 == 0x9ABCDEF0);
}

/* ========================================
 * Test Cases for FUN_00477cd0 (Object Direction Setter)
 * ======================================== */

TEST(set_object_direction_no_object) {
    reset_test_state();

    DAT_0462e3ac = 0;
    DAT_0462be90 = 5;

    FUN_00477cd0();

    /* Should not crash, direction stored in global */
    ASSERT(DAT_0462be90 == 5);
}

TEST(set_object_direction_with_object) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    DAT_0462be90 = 8;

    FUN_00477cd0();

    ASSERT(*(u16*)(g_object_data + 0x114) == 8);
}

TEST(set_object_direction_large_value) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    DAT_0462be90 = 0x1234;

    FUN_00477cd0();

    /* Only lower 16 bits should be stored */
    ASSERT(*(u16*)(g_object_data + 0x114) == 0x1234);
}

TEST(set_object_direction_truncation) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    DAT_0462be90 = 0x12345678;

    FUN_00477cd0();

    /* Only lower 16 bits should be stored */
    ASSERT(*(u16*)(g_object_data + 0x114) == 0x5678);
}

/* ========================================
 * Round-trip Tests
 * ======================================== */

TEST(setter_combo_test) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;

    FUN_00477ca0(10);
    FUN_00477cd0();
    FUN_00477c70(100, 200);

    ASSERT(*(u16*)(g_object_data + 0x114) == 10);
    ASSERT(DAT_0462be88 == 100);
    ASSERT(DAT_0462be94 == 200);
    ASSERT(*(u32*)(g_object_data + 0x140) == 100);
    ASSERT(*(u32*)(g_object_data + 0x150) == 200);
}

TEST(setter_null_object_safe) {
    reset_test_state();

    DAT_0462e3ac = 0;

    /* All these should not crash with null object */
    FUN_00477ca0(42);
    FUN_00477cd0();
    FUN_00477c70(1, 2);

    ASSERT(DAT_0462be90 == 42);
    ASSERT(DAT_0462be88 == 1);
    ASSERT(DAT_0462be94 == 2);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Object Setter Functions Unit Tests ===\n\n");

    printf("FUN_00477ca0 (Set BE90) Tests:\n");
    RUN_TEST(set_be90_basic);
    RUN_TEST(set_be90_zero);
    RUN_TEST(set_be90_overwrite);

    printf("\nFUN_00477c70 (Set Object Properties) Tests:\n");
    RUN_TEST(set_object_props_no_object);
    RUN_TEST(set_object_props_with_object);
    RUN_TEST(set_object_props_preserves_globals);
    RUN_TEST(set_object_props_null_object_no_crash);

    printf("\nFUN_00477cd0 (Set Object Direction) Tests:\n");
    RUN_TEST(set_object_direction_no_object);
    RUN_TEST(set_object_direction_with_object);
    RUN_TEST(set_object_direction_large_value);
    RUN_TEST(set_object_direction_truncation);

    printf("\nRound-trip Tests:\n");
    RUN_TEST(setter_combo_test);
    RUN_TEST(setter_null_object_safe);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
