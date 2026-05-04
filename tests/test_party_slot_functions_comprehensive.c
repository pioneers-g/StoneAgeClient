/*
 * Stone Age Client - Party Slot Functions Unit Tests
 * Tests for FUN_00478bf0, FUN_004789a0
 * These functions handle party slot data manipulation
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

/* Party slot data - 2 slots, each 0x44 bytes */
static u8 DAT_04630a00[0x88] = {0};  /* 2 * 0x44 = 0x88 */

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
 * FUN_00478bf0 - Clear party slot data
 * Clears 0x44 bytes of party slot data at the given index
 * Returns 0 on success, 0xFFFFFFFF on invalid param
 */
static u32 FUN_00478bf0(int param_1) {
    int i;
    u32 *puVar2;

    if ((param_1 >= 0) && (param_1 < 2)) {
        puVar2 = (u32 *)(&DAT_04630a00[0] + param_1 * 0x44);
        for (i = 0x11; i != 0; i = i - 1) {
            *puVar2 = 0;
            puVar2 = puVar2 + 1;
        }
        return 0;
    }
    return 0xFFFFFFFF;
}

/*
 * FUN_004789a0 - Check if party slot has data
 * Returns 1 if slot has data (first byte non-zero), 0 if empty, 0xFFFFFFFF if invalid
 */
static u32 FUN_004789a0(int param_1) {
    if ((param_1 >= 0) && (param_1 < 2)) {
        return (u32)(DAT_04630a00[param_1 * 0x44] != 0);
    }
    return 0xFFFFFFFF;
}

/* ========================================
 * Test Cases for FUN_00478bf0 (Clear Party Slot)
 * ======================================== */

TEST(clear_party_slot_0_basic) {
    reset_test_state();

    /* Fill slot 0 with data */
    memset(&DAT_04630a00[0], 0xFF, 0x44);

    u32 result = FUN_00478bf0(0);

    ASSERT(result == 0);
    /* Check first 4 bytes are cleared */
    ASSERT(DAT_04630a00[0] == 0);
    ASSERT(DAT_04630a00[1] == 0);
    ASSERT(DAT_04630a00[2] == 0);
    ASSERT(DAT_04630a00[3] == 0);
}

TEST(clear_party_slot_1_basic) {
    reset_test_state();

    /* Fill slot 1 with data */
    memset(&DAT_04630a00[0x44], 0xFF, 0x44);

    u32 result = FUN_00478bf0(1);

    ASSERT(result == 0);
    /* Check first 4 bytes of slot 1 are cleared */
    ASSERT(DAT_04630a00[0x44] == 0);
    ASSERT(DAT_04630a00[0x45] == 0);
    ASSERT(DAT_04630a00[0x46] == 0);
    ASSERT(DAT_04630a00[0x47] == 0);
}

TEST(clear_party_slot_invalid_negative) {
    reset_test_state();

    u32 result = FUN_00478bf0(-1);

    ASSERT(result == 0xFFFFFFFF);
}

TEST(clear_party_slot_invalid_positive) {
    reset_test_state();

    u32 result = FUN_00478bf0(2);

    ASSERT(result == 0xFFFFFFFF);
}

TEST(clear_party_slot_preserves_other_slot) {
    reset_test_state();

    /* Fill both slots with different data */
    memset(&DAT_04630a00[0], 0xAA, 0x44);
    memset(&DAT_04630a00[0x44], 0xBB, 0x44);

    FUN_00478bf0(0);

    /* Slot 0 should be cleared */
    ASSERT(DAT_04630a00[0] == 0);
    /* Slot 1 should be preserved */
    ASSERT(DAT_04630a00[0x44] == 0xBB);
}

TEST(clear_party_slot_full_clear) {
    reset_test_state();

    /* Fill slot 0 with all 0xFF */
    memset(&DAT_04630a00[0], 0xFF, 0x44);

    FUN_00478bf0(0);

    /* Check all bytes are cleared */
    int i;
    for (i = 0; i < 0x44; i++) {
        ASSERT(DAT_04630a00[i] == 0);
    }
}

/* ========================================
 * Test Cases for FUN_004789a0 (Check Party Slot)
 * ======================================== */

TEST(check_party_slot_empty) {
    reset_test_state();

    u32 result = FUN_004789a0(0);

    ASSERT(result == 0);
}

TEST(check_party_slot_has_data) {
    reset_test_state();

    DAT_04630a00[0] = 1;

    u32 result = FUN_004789a0(0);

    ASSERT(result == 1);
}

TEST(check_party_slot_1_empty) {
    reset_test_state();

    u32 result = FUN_004789a0(1);

    ASSERT(result == 0);
}

TEST(check_party_slot_1_has_data) {
    reset_test_state();

    DAT_04630a00[0x44] = 1;

    u32 result = FUN_004789a0(1);

    ASSERT(result == 1);
}

TEST(check_party_slot_invalid_negative) {
    reset_test_state();

    u32 result = FUN_004789a0(-1);

    ASSERT(result == 0xFFFFFFFF);
}

TEST(check_party_slot_invalid_positive) {
    reset_test_state();

    u32 result = FUN_004789a0(2);

    ASSERT(result == 0xFFFFFFFF);
}

TEST(check_party_slot_independence) {
    reset_test_state();

    /* Slot 0 has data, slot 1 is empty */
    DAT_04630a00[0] = 1;

    ASSERT(FUN_004789a0(0) == 1);
    ASSERT(FUN_004789a0(1) == 0);
}

/* ========================================
 * Round-trip Tests
 * ======================================== */

TEST(party_slot_clear_then_check) {
    reset_test_state();

    /* Set slot 0 to have data */
    DAT_04630a00[0] = 1;
    ASSERT(FUN_004789a0(0) == 1);

    /* Clear slot 0 */
    FUN_00478bf0(0);
    ASSERT(FUN_004789a0(0) == 0);
}

TEST(party_slot_both_operations) {
    reset_test_state();

    /* Set both slots */
    DAT_04630a00[0] = 1;
    DAT_04630a00[0x44] = 1;

    ASSERT(FUN_004789a0(0) == 1);
    ASSERT(FUN_004789a0(1) == 1);

    /* Clear slot 0 */
    FUN_00478bf0(0);

    ASSERT(FUN_004789a0(0) == 0);
    ASSERT(FUN_004789a0(1) == 1);

    /* Clear slot 1 */
    FUN_00478bf0(1);

    ASSERT(FUN_004789a0(0) == 0);
    ASSERT(FUN_004789a0(1) == 0);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Party Slot Functions Unit Tests ===\n\n");

    printf("FUN_00478bf0 (Clear Party Slot) Tests:\n");
    RUN_TEST(clear_party_slot_0_basic);
    RUN_TEST(clear_party_slot_1_basic);
    RUN_TEST(clear_party_slot_invalid_negative);
    RUN_TEST(clear_party_slot_invalid_positive);
    RUN_TEST(clear_party_slot_preserves_other_slot);
    RUN_TEST(clear_party_slot_full_clear);

    printf("\nFUN_004789a0 (Check Party Slot) Tests:\n");
    RUN_TEST(check_party_slot_empty);
    RUN_TEST(check_party_slot_has_data);
    RUN_TEST(check_party_slot_1_empty);
    RUN_TEST(check_party_slot_1_has_data);
    RUN_TEST(check_party_slot_invalid_negative);
    RUN_TEST(check_party_slot_invalid_positive);
    RUN_TEST(check_party_slot_independence);

    printf("\nRound-trip Tests:\n");
    RUN_TEST(party_slot_clear_then_check);
    RUN_TEST(party_slot_both_operations);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
