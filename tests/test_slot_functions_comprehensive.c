/*
 * Stone Age Client - Slot Functions Unit Tests
 * Tests for FUN_00478bf0 and related slot manipulation functions
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

/* Slot data structure - 68 bytes per slot (0x44) */
#define SLOT_SIZE 0x44
#define MAX_SLOTS 2

static char DAT_04630a00[MAX_SLOTS * SLOT_SIZE];

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

/* Fill slot with test pattern */
static void fill_slot_pattern(int slot, char pattern) {
    memset(&DAT_04630a00[slot * SLOT_SIZE], pattern, SLOT_SIZE);
}

/* Check if slot is all zeros */
static int is_slot_cleared(int slot) {
    for (int i = 0; i < SLOT_SIZE; i++) {
        if (DAT_04630a00[slot * SLOT_SIZE + i] != 0) {
            return 0;
        }
    }
    return 1;
}

/* Check if slot has pattern */
static int has_slot_pattern(int slot, char pattern) {
    for (int i = 0; i < SLOT_SIZE; i++) {
        if (DAT_04630a00[slot * SLOT_SIZE + i] != pattern) {
            return 0;
        }
    }
    return 1;
}

/*
 * FUN_00478bf0 - Clear slot data
 * param_1: slot index (0 or 1)
 * Returns: 0 on success, 0xFFFFFFFF if invalid index
 */
static u32 FUN_00478bf0(int param_1) {
    int iVar1;
    u32 *puVar2;

    if ((-1 < param_1) && (param_1 < 2)) {
        puVar2 = (u32 *)(&DAT_04630a00[0] + param_1 * SLOT_SIZE);
        for (iVar1 = 0x11; iVar1 != 0; iVar1 = iVar1 - 1) {
            *puVar2 = 0;
            puVar2 = puVar2 + 1;
        }
        return 0;
    }
    return 0xffffffff;
}

/* ========================================
 * Test Cases
 * ======================================== */

/* FUN_00478bf0 tests */
TEST(clear_slot_0_basic) {
    reset_test_state();
    fill_slot_pattern(0, 0xAA);

    u32 result = FUN_00478bf0(0);

    ASSERT(result == 0);
    ASSERT(is_slot_cleared(0));
}

TEST(clear_slot_1_basic) {
    reset_test_state();
    fill_slot_pattern(1, 0xBB);

    u32 result = FUN_00478bf0(1);

    ASSERT(result == 0);
    ASSERT(is_slot_cleared(1));
}

TEST(clear_slot_invalid_negative) {
    reset_test_state();

    u32 result = FUN_00478bf0(-1);

    ASSERT(result == 0xffffffff);
}

TEST(clear_slot_invalid_positive) {
    reset_test_state();

    u32 result = FUN_00478bf0(2);

    ASSERT(result == 0xffffffff);
}

TEST(clear_slot_invalid_large) {
    reset_test_state();

    u32 result = FUN_00478bf0(100);

    ASSERT(result == 0xffffffff);
}

TEST(clear_slot_already_cleared) {
    reset_test_state();

    u32 result = FUN_00478bf0(0);

    ASSERT(result == 0);
    ASSERT(is_slot_cleared(0));
}

TEST(clear_slot_preserves_other) {
    reset_test_state();
    fill_slot_pattern(0, 0xAA);
    fill_slot_pattern(1, 0xBB);

    u32 result = FUN_00478bf0(0);

    ASSERT(result == 0);
    ASSERT(is_slot_cleared(0));
    ASSERT(has_slot_pattern(1, 0xBB));
}

TEST(clear_both_slots) {
    reset_test_state();
    fill_slot_pattern(0, 0xAA);
    fill_slot_pattern(1, 0xBB);

    u32 result0 = FUN_00478bf0(0);
    u32 result1 = FUN_00478bf0(1);

    ASSERT(result0 == 0);
    ASSERT(result1 == 0);
    ASSERT(is_slot_cleared(0));
    ASSERT(is_slot_cleared(1));
}

TEST(clear_slot_multiple_times) {
    reset_test_state();
    fill_slot_pattern(0, 0xAA);

    FUN_00478bf0(0);
    u32 result = FUN_00478bf0(0);

    ASSERT(result == 0);
    ASSERT(is_slot_cleared(0));
}

TEST(clear_slot_alternating) {
    reset_test_state();

    /* Clear slot 0, check slot 1 */
    fill_slot_pattern(0, 0xAA);
    fill_slot_pattern(1, 0xBB);
    FUN_00478bf0(0);
    ASSERT(has_slot_pattern(1, 0xBB));

    /* Fill slot 0, clear slot 1 */
    fill_slot_pattern(0, 0xCC);
    FUN_00478bf0(1);
    ASSERT(has_slot_pattern(0, 0xCC));
    ASSERT(is_slot_cleared(1));
}

/* Size verification tests */
TEST(slot_size_correct) {
    /* Verify SLOT_SIZE is 68 bytes (0x44) */
    ASSERT(SLOT_SIZE == 0x44);
}

TEST(clear_clears_all_bytes) {
    reset_test_state();

    /* Fill entire slot with non-zero */
    for (int i = 0; i < SLOT_SIZE; i++) {
        DAT_04630a00[i] = 0xFF;
    }

    FUN_00478bf0(0);

    /* Check every byte is zero */
    for (int i = 0; i < SLOT_SIZE; i++) {
        ASSERT(DAT_04630a00[i] == 0);
    }
}

TEST(clear_preserves_slot_1_bytes) {
    reset_test_state();

    /* Fill both slots with different patterns */
    for (int i = 0; i < SLOT_SIZE; i++) {
        DAT_04630a00[i] = 0x11;
        DAT_04630a00[SLOT_SIZE + i] = 0x22;
    }

    FUN_00478bf0(0);

    /* Slot 0 should be cleared */
    for (int i = 0; i < SLOT_SIZE; i++) {
        ASSERT(DAT_04630a00[i] == 0);
    }

    /* Slot 1 should be preserved */
    for (int i = 0; i < SLOT_SIZE; i++) {
        ASSERT(DAT_04630a00[SLOT_SIZE + i] == 0x22);
    }
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Slot Functions Unit Tests ===\n\n");

    printf("FUN_00478bf0 (Clear Slot) Tests:\n");
    RUN_TEST(clear_slot_0_basic);
    RUN_TEST(clear_slot_1_basic);
    RUN_TEST(clear_slot_invalid_negative);
    RUN_TEST(clear_slot_invalid_positive);
    RUN_TEST(clear_slot_invalid_large);
    RUN_TEST(clear_slot_already_cleared);
    RUN_TEST(clear_slot_preserves_other);
    RUN_TEST(clear_both_slots);
    RUN_TEST(clear_slot_multiple_times);
    RUN_TEST(clear_slot_alternating);

    printf("\nSize Verification Tests:\n");
    RUN_TEST(slot_size_correct);
    RUN_TEST(clear_clears_all_bytes);
    RUN_TEST(clear_preserves_slot_1_bytes);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
