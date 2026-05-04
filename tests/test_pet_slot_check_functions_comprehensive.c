/*
 * Stone Age Client - Pet Slot Check Function Unit Tests
 * Tests for FUN_004789a0 - Check if pet slot is occupied
 * Returns 1 if slot has data, 0 if empty, 0xFFFFFFFF if invalid slot
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

/* Simulated pet slot data (at 0x44 byte intervals) */
#define PET_SLOT_DATA_SIZE 0x100
static u8 DAT_04630a00[PET_SLOT_DATA_SIZE] = {0};

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
    memset(DAT_04630a00, 0, sizeof(DAT_04630a00));
}

/*
 * FUN_004789a0 - Check if pet slot is occupied
 * Returns 1 if slot 0 or 1 has non-zero first byte
 * Returns 0 if slot is empty (zero first byte)
 * Returns 0xFFFFFFFF if slot index is out of range (not 0 or 1)
 */
static u32 FUN_004789a0(s32 slot_index) {
    if (slot_index >= 0 && slot_index < 2) {
        return (DAT_04630a00[slot_index * 0x44] != 0) ? 1 : 0;
    }
    return 0xFFFFFFFF;
}

/* ========================================
 * Test Cases for FUN_004789a0
 * ======================================== */

TEST(check_slot_0_empty) {
    reset_test_state();
    DAT_04630a00[0] = 0;

    u32 result = FUN_004789a0(0);

    ASSERT(result == 0);
}

TEST(check_slot_0_occupied) {
    reset_test_state();
    DAT_04630a00[0] = 1;

    u32 result = FUN_004789a0(0);

    ASSERT(result == 1);
}

TEST(check_slot_1_empty) {
    reset_test_state();
    DAT_04630a00[0x44] = 0;

    u32 result = FUN_004789a0(1);

    ASSERT(result == 0);
}

TEST(check_slot_1_occupied) {
    reset_test_state();
    DAT_04630a00[0x44] = 0x42;

    u32 result = FUN_004789a0(1);

    ASSERT(result == 1);
}

TEST(check_slot_negative_invalid) {
    reset_test_state();

    u32 result = FUN_004789a0(-1);

    ASSERT(result == 0xFFFFFFFF);
}

TEST(check_slot_2_invalid) {
    reset_test_state();

    u32 result = FUN_004789a0(2);

    ASSERT(result == 0xFFFFFFFF);
}

TEST(check_slot_large_invalid) {
    reset_test_state();

    u32 result = FUN_004789a0(100);

    ASSERT(result == 0xFFFFFFFF);
}

TEST(check_slot_0_various_values) {
    reset_test_state();

    /* Test with various non-zero values */
    DAT_04630a00[0] = 0x01;
    ASSERT(FUN_004789a0(0) == 1);

    DAT_04630a00[0] = 0xFF;
    ASSERT(FUN_004789a0(0) == 1);

    DAT_04630a00[0] = 0x80;
    ASSERT(FUN_004789a0(0) == 1);
}

TEST(check_slot_1_various_values) {
    reset_test_state();

    /* Test with various non-zero values at slot 1 */
    DAT_04630a00[0x44] = 0x01;
    ASSERT(FUN_004789a0(1) == 1);

    DAT_04630a00[0x44] = 0xFF;
    ASSERT(FUN_004789a0(1) == 1);

    DAT_04630a00[0x44] = 0x80;
    ASSERT(FUN_004789a0(1) == 1);
}

TEST(check_independent_slots) {
    reset_test_state();
    DAT_04630a00[0] = 0x55;
    DAT_04630a00[0x44] = 0;

    ASSERT(FUN_004789a0(0) == 1);
    ASSERT(FUN_004789a0(1) == 0);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Pet Slot Check Function Unit Tests ===\n\n");

    printf("FUN_004789a0 (Check Pet Slot Occupied) Tests:\n");
    RUN_TEST(check_slot_0_empty);
    RUN_TEST(check_slot_0_occupied);
    RUN_TEST(check_slot_1_empty);
    RUN_TEST(check_slot_1_occupied);
    RUN_TEST(check_slot_negative_invalid);
    RUN_TEST(check_slot_2_invalid);
    RUN_TEST(check_slot_large_invalid);
    RUN_TEST(check_slot_0_various_values);
    RUN_TEST(check_slot_1_various_values);
    RUN_TEST(check_independent_slots);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
