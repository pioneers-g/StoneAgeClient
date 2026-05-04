/*
 * Stone Age Client - Object Property Setters Unit Tests
 * Tests for FUN_00477c70, FUN_00477d70, FUN_00477cd0
 * These functions set properties on the main game object
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

/* Global variables that the functions modify */
static u32 DAT_0462be88 = 0;
static u32 DAT_0462be94 = 0;

/* Object pointer */
static u32* DAT_0462e3ac = NULL;

/* Object fields - simulate offsets */
#define OBJ_OFFSET_0x140  0x50  /* Simulated offset index */
#define OBJ_OFFSET_0x150  0x54
#define OBJ_OFFSET_0x114  0x45
#define OBJ_OFFSET_0x118  0x46
#define OBJ_OFFSET_0xB0   0x2C
#define OBJ_OFFSET_0xB4   0x2D
#define OBJ_OFFSET_0x11C  0x47
#define OBJ_OFFSET_0x120  0x48
#define OBJ_OFFSET_0xB8   0x2E
#define OBJ_OFFSET_0xBC   0x2F

#define OBJ_SIZE 256
static u32 test_object[OBJ_SIZE];

/* Source data for FUN_00477cd0 */
static u32 DAT_0456a644 = 0;
static u32 DAT_0456a648 = 0;
static u32 DAT_04581d3c = 0;
static u32 DAT_04581d40 = 0;
static u32 DAT_045827fc = 0;
static u32 DAT_04582800 = 0;
static u32 DAT_0458118c = 0;
static u32 DAT_04581184 = 0;

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
    DAT_0462be88 = 0;
    DAT_0462be94 = 0;
    DAT_0462e3ac = NULL;
    memset(test_object, 0, sizeof(test_object));

    DAT_0456a644 = 0;
    DAT_0456a648 = 0;
    DAT_04581d3c = 0;
    DAT_04581d40 = 0;
    DAT_045827fc = 0;
    DAT_04582800 = 0;
    DAT_0458118c = 0;
    DAT_04581184 = 0;
}

/*
 * FUN_00477c70 - Set two properties
 */
static void FUN_00477c70(u32 param_1, u32 param_2) {
    DAT_0462be88 = param_1;
    DAT_0462be94 = param_2;
    if (DAT_0462e3ac != NULL) {
        test_object[OBJ_OFFSET_0x140] = param_1;
        test_object[OBJ_OFFSET_0x150] = param_2;
    }
}

/*
 * FUN_00477d70 - Set single property
 */
static void FUN_00477d70(u32 param_1) {
    DAT_0462be94 = param_1;
    if (DAT_0462e3ac != NULL) {
        test_object[OBJ_OFFSET_0x150] = param_1;
    }
}

/*
 * FUN_00477cd0 - Copy multiple properties from source variables
 */
static void FUN_00477cd0(void) {
    if (DAT_0462e3ac != NULL) {
        test_object[OBJ_OFFSET_0x114] = DAT_0456a644;
        test_object[OBJ_OFFSET_0x118] = DAT_0456a648;
        test_object[OBJ_OFFSET_0xB0] = DAT_04581d3c;
        test_object[OBJ_OFFSET_0xB4] = DAT_04581d40;
        test_object[OBJ_OFFSET_0x11C] = DAT_045827fc;
        test_object[OBJ_OFFSET_0x120] = DAT_04582800;
        test_object[OBJ_OFFSET_0xB8] = DAT_0458118c;
        test_object[OBJ_OFFSET_0xBC] = DAT_04581184;
    }
}

/* ========================================
 * Test Cases for FUN_00477c70
 * ======================================== */

TEST(set_two_props_basic) {
    reset_test_state();
    DAT_0462e3ac = test_object;

    FUN_00477c70(100, 200);

    ASSERT(DAT_0462be88 == 100);
    ASSERT(DAT_0462be94 == 200);
    ASSERT(test_object[OBJ_OFFSET_0x140] == 100);
    ASSERT(test_object[OBJ_OFFSET_0x150] == 200);
}

TEST(set_two_props_null_object) {
    reset_test_state();
    DAT_0462e3ac = NULL;

    FUN_00477c70(100, 200);

    /* Globals should still be set */
    ASSERT(DAT_0462be88 == 100);
    ASSERT(DAT_0462be94 == 200);
}

TEST(set_two_props_zero) {
    reset_test_state();
    DAT_0462e3ac = test_object;
    test_object[OBJ_OFFSET_0x140] = 999;
    test_object[OBJ_OFFSET_0x150] = 888;

    FUN_00477c70(0, 0);

    ASSERT(DAT_0462be88 == 0);
    ASSERT(DAT_0462be94 == 0);
    ASSERT(test_object[OBJ_OFFSET_0x140] == 0);
    ASSERT(test_object[OBJ_OFFSET_0x150] == 0);
}

TEST(set_two_props_overwrites) {
    reset_test_state();
    DAT_0462e3ac = test_object;
    DAT_0462be88 = 111;
    DAT_0462be94 = 222;
    test_object[OBJ_OFFSET_0x140] = 333;
    test_object[OBJ_OFFSET_0x150] = 444;

    FUN_00477c70(555, 666);

    ASSERT(DAT_0462be88 == 555);
    ASSERT(DAT_0462be94 == 666);
    ASSERT(test_object[OBJ_OFFSET_0x140] == 555);
    ASSERT(test_object[OBJ_OFFSET_0x150] == 666);
}

TEST(set_two_props_max_values) {
    reset_test_state();
    DAT_0462e3ac = test_object;

    FUN_00477c70(0xFFFFFFFF, 0xFFFFFFFF);

    ASSERT(DAT_0462be88 == 0xFFFFFFFF);
    ASSERT(DAT_0462be94 == 0xFFFFFFFF);
}

/* ========================================
 * Test Cases for FUN_00477d70
 * ======================================== */

TEST(set_single_prop_basic) {
    reset_test_state();
    DAT_0462e3ac = test_object;

    FUN_00477d70(12345);

    ASSERT(DAT_0462be94 == 12345);
    ASSERT(test_object[OBJ_OFFSET_0x150] == 12345);
}

TEST(set_single_prop_null_object) {
    reset_test_state();
    DAT_0462e3ac = NULL;

    FUN_00477d70(12345);

    ASSERT(DAT_0462be94 == 12345);
}

TEST(set_single_prop_zero) {
    reset_test_state();
    DAT_0462e3ac = test_object;
    DAT_0462be94 = 99999;

    FUN_00477d70(0);

    ASSERT(DAT_0462be94 == 0);
}

TEST(set_single_prop_preserves_be88) {
    reset_test_state();
    DAT_0462e3ac = test_object;
    DAT_0462be88 = 77777;

    FUN_00477d70(12345);

    ASSERT(DAT_0462be88 == 77777);  /* Should not change */
    ASSERT(DAT_0462be94 == 12345);
}

/* ========================================
 * Test Cases for FUN_00477cd0
 * ======================================== */

TEST(copy_props_basic) {
    reset_test_state();
    DAT_0462e3ac = test_object;

    DAT_0456a644 = 111;
    DAT_0456a648 = 222;
    DAT_04581d3c = 333;
    DAT_04581d40 = 444;
    DAT_045827fc = 555;
    DAT_04582800 = 666;
    DAT_0458118c = 777;
    DAT_04581184 = 888;

    FUN_00477cd0();

    ASSERT(test_object[OBJ_OFFSET_0x114] == 111);
    ASSERT(test_object[OBJ_OFFSET_0x118] == 222);
    ASSERT(test_object[OBJ_OFFSET_0xB0] == 333);
    ASSERT(test_object[OBJ_OFFSET_0xB4] == 444);
    ASSERT(test_object[OBJ_OFFSET_0x11C] == 555);
    ASSERT(test_object[OBJ_OFFSET_0x120] == 666);
    ASSERT(test_object[OBJ_OFFSET_0xB8] == 777);
    ASSERT(test_object[OBJ_OFFSET_0xBC] == 888);
}

TEST(copy_props_null_object) {
    reset_test_state();
    DAT_0462e3ac = NULL;

    DAT_0456a644 = 111;
    DAT_04581d3c = 333;

    FUN_00477cd0();

    /* Should not crash, just do nothing */
    ASSERT(1);
}

TEST(copy_props_zero_values) {
    reset_test_state();
    DAT_0462e3ac = test_object;

    /* Set all source values to 0 */
    FUN_00477cd0();

    ASSERT(test_object[OBJ_OFFSET_0x114] == 0);
    ASSERT(test_object[OBJ_OFFSET_0x118] == 0);
    ASSERT(test_object[OBJ_OFFSET_0xB0] == 0);
    ASSERT(test_object[OBJ_OFFSET_0xB4] == 0);
}

TEST(copy_props_overwrites) {
    reset_test_state();
    DAT_0462e3ac = test_object;

    /* Set initial values */
    test_object[OBJ_OFFSET_0x114] = 9999;
    test_object[OBJ_OFFSET_0xB0] = 8888;

    DAT_0456a644 = 111;
    DAT_04581d3c = 333;

    FUN_00477cd0();

    ASSERT(test_object[OBJ_OFFSET_0x114] == 111);
    ASSERT(test_object[OBJ_OFFSET_0xB0] == 333);
}

TEST(copy_props_max_values) {
    reset_test_state();
    DAT_0462e3ac = test_object;

    DAT_0456a644 = 0xFFFFFFFF;
    DAT_0456a648 = 0xFFFFFFFF;
    DAT_04581d3c = 0xFFFFFFFF;
    DAT_04581d40 = 0xFFFFFFFF;
    DAT_045827fc = 0xFFFFFFFF;
    DAT_04582800 = 0xFFFFFFFF;
    DAT_0458118c = 0xFFFFFFFF;
    DAT_04581184 = 0xFFFFFFFF;

    FUN_00477cd0();

    ASSERT(test_object[OBJ_OFFSET_0x114] == 0xFFFFFFFF);
    ASSERT(test_object[OBJ_OFFSET_0x118] == 0xFFFFFFFF);
    ASSERT(test_object[OBJ_OFFSET_0xB0] == 0xFFFFFFFF);
    ASSERT(test_object[OBJ_OFFSET_0xB4] == 0xFFFFFFFF);
    ASSERT(test_object[OBJ_OFFSET_0x11C] == 0xFFFFFFFF);
    ASSERT(test_object[OBJ_OFFSET_0x120] == 0xFFFFFFFF);
    ASSERT(test_object[OBJ_OFFSET_0xB8] == 0xFFFFFFFF);
    ASSERT(test_object[OBJ_OFFSET_0xBC] == 0xFFFFFFFF);
}

/* ========================================
 * Combined Tests
 * ======================================== */

TEST(set_then_copy) {
    reset_test_state();
    DAT_0462e3ac = test_object;

    FUN_00477c70(100, 200);

    DAT_04581d3c = 555;
    DAT_04581d40 = 666;
    FUN_00477cd0();

    ASSERT(test_object[OBJ_OFFSET_0x140] == 100);
    ASSERT(test_object[OBJ_OFFSET_0x150] == 200);
    ASSERT(test_object[OBJ_OFFSET_0xB0] == 555);
    ASSERT(test_object[OBJ_OFFSET_0xB4] == 666);
}

TEST(set_single_after_double) {
    reset_test_state();
    DAT_0462e3ac = test_object;

    FUN_00477c70(100, 200);
    FUN_00477d70(300);

    ASSERT(DAT_0462be88 == 100);  /* Unchanged */
    ASSERT(DAT_0462be94 == 300);  /* Overwritten */
    ASSERT(test_object[OBJ_OFFSET_0x150] == 300);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Object Property Setters Unit Tests ===\n\n");

    printf("FUN_00477c70 (Set Two Props) Tests:\n");
    RUN_TEST(set_two_props_basic);
    RUN_TEST(set_two_props_null_object);
    RUN_TEST(set_two_props_zero);
    RUN_TEST(set_two_props_overwrites);
    RUN_TEST(set_two_props_max_values);

    printf("\nFUN_00477d70 (Set Single Prop) Tests:\n");
    RUN_TEST(set_single_prop_basic);
    RUN_TEST(set_single_prop_null_object);
    RUN_TEST(set_single_prop_zero);
    RUN_TEST(set_single_prop_preserves_be88);

    printf("\nFUN_00477cd0 (Copy Props) Tests:\n");
    RUN_TEST(copy_props_basic);
    RUN_TEST(copy_props_null_object);
    RUN_TEST(copy_props_zero_values);
    RUN_TEST(copy_props_overwrites);
    RUN_TEST(copy_props_max_values);

    printf("\nCombined Tests:\n");
    RUN_TEST(set_then_copy);
    RUN_TEST(set_single_after_double);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
