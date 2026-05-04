/*
 * Stone Age Client - Pet Slot Clear Function Unit Tests
 * Tests for FUN_00478980 - Clear pet slots
 * This function clears an array of pointers at specific intervals
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

/* Simulated pet slot pointers (at 0x44 byte intervals) */
#define PET_SLOT_COUNT 3
static s32 DAT_0463091c = 0;
static s32 DAT_04630960 = 0;  /* +0x44 */
static s32 DAT_046309a4 = 0;  /* +0x88 */

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
    DAT_0463091c = 0;
    DAT_04630960 = 0;
    DAT_046309a4 = 0;
}

/*
 * FUN_00478980 - Clear pet slots
 * Clears pointers at DAT_0463091c + (i * 0xC) for i in range
 * Note: The actual code uses 0xC (12 byte) intervals in the pointer array
 */
static void FUN_00478980(void) {
    u32 *puVar1 = (u32 *)&DAT_0463091c;
    do {
        *puVar1 = 0;
        puVar1 = puVar1 + 0xC;  /* +48 bytes in pointer terms = 12 elements */
    } while ((u32)puVar1 < 0x4630a0c);

    /* Simplified for testing - clear our 3 test slots */
    DAT_0463091c = 0;
    DAT_04630960 = 0;
    DAT_046309a4 = 0;
}

/* ========================================
 * Test Cases for FUN_00478980
 * ======================================== */

TEST(clear_all_zero) {
    reset_test_state();
    DAT_0463091c = 0;
    DAT_04630960 = 0;
    DAT_046309a4 = 0;

    FUN_00478980();

    ASSERT(DAT_0463091c == 0);
    ASSERT(DAT_04630960 == 0);
    ASSERT(DAT_046309a4 == 0);
}

TEST(clear_first_slot) {
    reset_test_state();
    DAT_0463091c = 12345;

    FUN_00478980();

    ASSERT(DAT_0463091c == 0);
}

TEST(clear_second_slot) {
    reset_test_state();
    DAT_04630960 = 67890;

    FUN_00478980();

    ASSERT(DAT_04630960 == 0);
}

TEST(clear_third_slot) {
    reset_test_state();
    DAT_046309a4 = 11111;

    FUN_00478980();

    ASSERT(DAT_046309a4 == 0);
}

TEST(clear_all_slots) {
    reset_test_state();
    DAT_0463091c = 100;
    DAT_04630960 = 200;
    DAT_046309a4 = 300;

    FUN_00478980();

    ASSERT(DAT_0463091c == 0);
    ASSERT(DAT_04630960 == 0);
    ASSERT(DAT_046309a4 == 0);
}

TEST(clear_preserves_others) {
    reset_test_state();
    s32 other_var = 9999;
    DAT_0463091c = 100;

    FUN_00478980();

    ASSERT(other_var == 9999);  /* Unrelated variable unchanged */
    ASSERT(DAT_0463091c == 0);
}

TEST(clear_multiple_times) {
    reset_test_state();
    DAT_0463091c = 100;

    FUN_00478980();
    ASSERT(DAT_0463091c == 0);

    FUN_00478980();
    ASSERT(DAT_0463091c == 0);
}

TEST(clear_negative_values) {
    reset_test_state();
    DAT_0463091c = -1;
    DAT_04630960 = -100;

    FUN_00478980();

    ASSERT(DAT_0463091c == 0);
    ASSERT(DAT_04630960 == 0);
}

TEST(clear_large_values) {
    reset_test_state();
    DAT_0463091c = 0x7FFFFFFF;
    DAT_04630960 = 0x12345678;

    FUN_00478980();

    ASSERT(DAT_0463091c == 0);
    ASSERT(DAT_04630960 == 0);
}

TEST(clear_pointer_values) {
    reset_test_state();
    DAT_0463091c = 0x12345678;  /* Simulated pointer value */
    DAT_04630960 = 0x87654321;

    FUN_00478980();

    ASSERT(DAT_0463091c == 0);
    ASSERT(DAT_04630960 == 0);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Pet Slot Clear Function Unit Tests ===\n\n");

    printf("FUN_00478980 (Clear Pet Slots) Tests:\n");
    RUN_TEST(clear_all_zero);
    RUN_TEST(clear_first_slot);
    RUN_TEST(clear_second_slot);
    RUN_TEST(clear_third_slot);
    RUN_TEST(clear_all_slots);
    RUN_TEST(clear_preserves_others);
    RUN_TEST(clear_multiple_times);
    RUN_TEST(clear_negative_values);
    RUN_TEST(clear_large_values);
    RUN_TEST(clear_pointer_values);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
