/*
 * Stone Age Client - Slot Check Functions Unit Tests
 * Tests for FUN_004789a0, FUN_00478bf0, FUN_00478980
 * These functions check and clear slot data
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

/* Slot data arrays - 2 slots, each 0x44 = 68 bytes */
#define SLOT_SIZE 0x44
#define SLOT_COUNT 2

static byte slot_data[SLOT_COUNT][SLOT_SIZE];  /* DAT_04630a00 */

/* Entity pointers for FUN_00478980 - 12 entries per slot */
#define ENTITY_PTR_COUNT (SLOT_COUNT * 12)
static u32 entity_ptrs[ENTITY_PTR_COUNT];  /* DAT_0463091c area */

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
    memset(slot_data, 0, sizeof(slot_data));
    memset(entity_ptrs, 0, sizeof(entity_ptrs));
}

/*
 * FUN_004789a0 - Check if slot is used
 * param_1: slot index (0 or 1)
 * Returns: 1 if slot has data, 0 if empty, 0xFFFFFFFF if invalid
 */
static u32 FUN_004789a0(int param_1) {
    if ((-1 < param_1) && (param_1 < 2)) {
        return (u32)(slot_data[param_1][0] != '\0');
    }
    return 0xFFFFFFFF;
}

/*
 * FUN_00478bf0 - Clear slot data
 * param_1: slot index (0 or 1)
 * Returns: 0 on success, 0xFFFFFFFF if invalid
 */
static u32 FUN_00478bf0(int param_1) {
    int i;
    if ((-1 < param_1) && (param_1 < 2)) {
        for (i = 0; i < SLOT_SIZE; i++) {
            slot_data[param_1][i] = 0;
        }
        return 0;
    }
    return 0xFFFFFFFF;
}

/*
 * FUN_00478980 - Clear entity pointers
 */
static void FUN_00478980(void) {
    int i;
    for (i = 0; i < ENTITY_PTR_COUNT; i++) {
        entity_ptrs[i] = 0;
    }
}

/* ========================================
 * Test Cases for FUN_004789a0
 * ======================================== */

TEST(slot_check_empty) {
    reset_test_state();

    u32 result = FUN_004789a0(0);

    ASSERT(result == 0);
}

TEST(slot_check_used) {
    reset_test_state();
    slot_data[0][0] = 'A';

    u32 result = FUN_004789a0(0);

    ASSERT(result == 1);
}

TEST(slot_check_slot1_empty) {
    reset_test_state();

    u32 result = FUN_004789a0(1);

    ASSERT(result == 0);
}

TEST(slot_check_slot1_used) {
    reset_test_state();
    slot_data[1][0] = 'B';

    u32 result = FUN_004789a0(1);

    ASSERT(result == 1);
}

TEST(slot_check_invalid_negative) {
    reset_test_state();

    u32 result = FUN_004789a0(-1);

    ASSERT(result == 0xFFFFFFFF);
}

TEST(slot_check_invalid_positive) {
    reset_test_state();

    u32 result = FUN_004789a0(2);

    ASSERT(result == 0xFFFFFFFF);
}

TEST(slot_check_invalid_large) {
    reset_test_state();

    u32 result = FUN_004789a0(100);

    ASSERT(result == 0xFFFFFFFF);
}

TEST(slot_check_independence) {
    reset_test_state();
    slot_data[0][0] = 'A';
    slot_data[1][0] = 0;

    u32 result0 = FUN_004789a0(0);
    u32 result1 = FUN_004789a0(1);

    ASSERT(result0 == 1);
    ASSERT(result1 == 0);
}

/* ========================================
 * Test Cases for FUN_00478bf0
 * ======================================== */

TEST(slot_clear_basic) {
    reset_test_state();
    memset(slot_data[0], 0xFF, SLOT_SIZE);

    u32 result = FUN_00478bf0(0);

    ASSERT(result == 0);
    for (int i = 0; i < SLOT_SIZE; i++) {
        ASSERT(slot_data[0][i] == 0);
    }
}

TEST(slot_clear_slot1) {
    reset_test_state();
    memset(slot_data[1], 'X', SLOT_SIZE);

    u32 result = FUN_00478bf0(1);

    ASSERT(result == 0);
    for (int i = 0; i < SLOT_SIZE; i++) {
        ASSERT(slot_data[1][i] == 0);
    }
}

TEST(slot_clear_already_empty) {
    reset_test_state();

    u32 result = FUN_00478bf0(0);

    ASSERT(result == 0);
}

TEST(slot_clear_invalid_negative) {
    reset_test_state();

    u32 result = FUN_00478bf0(-1);

    ASSERT(result == 0xFFFFFFFF);
}

TEST(slot_clear_invalid_positive) {
    reset_test_state();

    u32 result = FUN_00478bf0(2);

    ASSERT(result == 0xFFFFFFFF);
}

TEST(slot_clear_preserves_other) {
    reset_test_state();
    memset(slot_data[0], 'A', SLOT_SIZE);
    memset(slot_data[1], 'B', SLOT_SIZE);

    FUN_00478bf0(0);

    for (int i = 0; i < SLOT_SIZE; i++) {
        ASSERT(slot_data[0][i] == 0);
        ASSERT(slot_data[1][i] == 'B');
    }
}

TEST(slot_clear_all_bytes) {
    reset_test_state();
    for (int i = 0; i < SLOT_SIZE; i++) {
        slot_data[0][i] = (byte)(i + 1);
    }

    FUN_00478bf0(0);

    for (int i = 0; i < SLOT_SIZE; i++) {
        ASSERT(slot_data[0][i] == 0);
    }
}

/* ========================================
 * Test Cases for FUN_00478980
 * ======================================== */

TEST(entity_clear_all) {
    reset_test_state();
    for (int i = 0; i < ENTITY_PTR_COUNT; i++) {
        entity_ptrs[i] = 0xDEADBEEF;
    }

    FUN_00478980();

    for (int i = 0; i < ENTITY_PTR_COUNT; i++) {
        ASSERT(entity_ptrs[i] == 0);
    }
}

TEST(entity_clear_already_zero) {
    reset_test_state();

    FUN_00478980();

    for (int i = 0; i < ENTITY_PTR_COUNT; i++) {
        ASSERT(entity_ptrs[i] == 0);
    }
}

TEST(entity_clear_partial) {
    reset_test_state();
    entity_ptrs[0] = 0x11111111;
    entity_ptrs[5] = 0x22222222;
    entity_ptrs[ENTITY_PTR_COUNT - 1] = 0x33333333;

    FUN_00478980();

    ASSERT(entity_ptrs[0] == 0);
    ASSERT(entity_ptrs[5] == 0);
    ASSERT(entity_ptrs[ENTITY_PTR_COUNT - 1] == 0);
}

/* ========================================
 * Combined Tests
 * ======================================== */

TEST(slot_check_clear_cycle) {
    reset_test_state();

    /* Initially empty */
    ASSERT(FUN_004789a0(0) == 0);

    /* Set data */
    slot_data[0][0] = 'X';
    ASSERT(FUN_004789a0(0) == 1);

    /* Clear */
    FUN_00478bf0(0);
    ASSERT(FUN_004789a0(0) == 0);

    /* Set again */
    slot_data[0][0] = 'Y';
    ASSERT(FUN_004789a0(0) == 1);
}

TEST(both_slots_independent) {
    reset_test_state();

    slot_data[0][0] = 'A';
    slot_data[1][0] = 'B';

    ASSERT(FUN_004789a0(0) == 1);
    ASSERT(FUN_004789a0(1) == 1);

    FUN_00478bf0(0);

    ASSERT(FUN_004789a0(0) == 0);
    ASSERT(FUN_004789a0(1) == 1);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Slot Check Functions Unit Tests ===\n\n");

    printf("FUN_004789a0 (Slot Check) Tests:\n");
    RUN_TEST(slot_check_empty);
    RUN_TEST(slot_check_used);
    RUN_TEST(slot_check_slot1_empty);
    RUN_TEST(slot_check_slot1_used);
    RUN_TEST(slot_check_invalid_negative);
    RUN_TEST(slot_check_invalid_positive);
    RUN_TEST(slot_check_invalid_large);
    RUN_TEST(slot_check_independence);

    printf("\nFUN_00478bf0 (Slot Clear) Tests:\n");
    RUN_TEST(slot_clear_basic);
    RUN_TEST(slot_clear_slot1);
    RUN_TEST(slot_clear_already_empty);
    RUN_TEST(slot_clear_invalid_negative);
    RUN_TEST(slot_clear_invalid_positive);
    RUN_TEST(slot_clear_preserves_other);
    RUN_TEST(slot_clear_all_bytes);

    printf("\nFUN_00478980 (Entity Clear) Tests:\n");
    RUN_TEST(entity_clear_all);
    RUN_TEST(entity_clear_already_zero);
    RUN_TEST(entity_clear_partial);

    printf("\nCombined Tests:\n");
    RUN_TEST(slot_check_clear_cycle);
    RUN_TEST(both_slots_independent);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
