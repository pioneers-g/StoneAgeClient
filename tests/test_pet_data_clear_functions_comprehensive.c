/*
 * Stone Age Client - Pet Data Clear Function Unit Tests
 * Tests for FUN_00478bf0 - Clear pet data slot
 * Clears 68 bytes (0x44) at slot 0 or 1, returns 0xFFFFFFFF for invalid slot
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

/* Simulated pet data slots (0x44 bytes each) */
#define PET_SLOT_SIZE 0x44
#define PET_SLOT_COUNT 2
static u8 DAT_04630a00[PET_SLOT_SIZE * PET_SLOT_COUNT + 0x20] = {0};

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

/* Fill slot with pattern */
static void fill_slot_pattern(int slot, u8 pattern) {
    memset(&DAT_04630a00[slot * PET_SLOT_SIZE], pattern, PET_SLOT_SIZE);
}

/* Check if slot is all zeros */
static int is_slot_cleared(int slot) {
    u8 *ptr = &DAT_04630a00[slot * PET_SLOT_SIZE];
    for (int i = 0; i < PET_SLOT_SIZE; i++) {
        if (ptr[i] != 0) return 0;
    }
    return 1;
}

/* Check if slot has pattern */
static int is_slot_pattern(int slot, u8 pattern) {
    u8 *ptr = &DAT_04630a00[slot * PET_SLOT_SIZE];
    for (int i = 0; i < PET_SLOT_SIZE; i++) {
        if (ptr[i] != pattern) return 0;
    }
    return 1;
}

/*
 * FUN_00478bf0 - Clear pet data slot
 * Clears 68 bytes (17 * 4) at slot index 0 or 1
 * Returns 0 on success, 0xFFFFFFFF if slot is out of range
 */
static u32 FUN_00478bf0(s32 slot_index) {
    if (slot_index >= 0 && slot_index < 2) {
        u32 *puVar2 = (u32 *)(&DAT_04630a00[slot_index * PET_SLOT_SIZE]);
        for (int i = 0x11; i != 0; i--) {
            *puVar2 = 0;
            puVar2++;
        }
        return 0;
    }
    return 0xFFFFFFFF;
}

/* ========================================
 * Test Cases for FUN_00478bf0
 * ======================================== */

TEST(clear_slot_0_success) {
    reset_test_state();
    fill_slot_pattern(0, 0xAA);

    u32 result = FUN_00478bf0(0);

    ASSERT(result == 0);
    ASSERT(is_slot_cleared(0));
}

TEST(clear_slot_1_success) {
    reset_test_state();
    fill_slot_pattern(1, 0xBB);

    u32 result = FUN_00478bf0(1);

    ASSERT(result == 0);
    ASSERT(is_slot_cleared(1));
}

TEST(clear_slot_negative_invalid) {
    reset_test_state();

    u32 result = FUN_00478bf0(-1);

    ASSERT(result == 0xFFFFFFFF);
}

TEST(clear_slot_2_invalid) {
    reset_test_state();

    u32 result = FUN_00478bf0(2);

    ASSERT(result == 0xFFFFFFFF);
}

TEST(clear_slot_large_invalid) {
    reset_test_state();

    u32 result = FUN_00478bf0(100);

    ASSERT(result == 0xFFFFFFFF);
}

TEST(clear_slot_preserves_other) {
    reset_test_state();
    fill_slot_pattern(0, 0xAA);
    fill_slot_pattern(1, 0xBB);

    FUN_00478bf0(0);

    ASSERT(is_slot_cleared(0));
    ASSERT(is_slot_pattern(1, 0xBB));
}

TEST(clear_already_cleared) {
    reset_test_state();
    /* Already cleared by reset */

    u32 result = FUN_00478bf0(0);

    ASSERT(result == 0);
    ASSERT(is_slot_cleared(0));
}

TEST(clear_twice) {
    reset_test_state();
    fill_slot_pattern(0, 0xCC);

    FUN_00478bf0(0);
    u32 result = FUN_00478bf0(0);

    ASSERT(result == 0);
    ASSERT(is_slot_cleared(0));
}

TEST(clear_both_slots) {
    reset_test_state();
    fill_slot_pattern(0, 0xDD);
    fill_slot_pattern(1, 0xEE);

    FUN_00478bf0(0);
    FUN_00478bf0(1);

    ASSERT(is_slot_cleared(0));
    ASSERT(is_slot_cleared(1));
}

TEST(clear_partial_preserves_rest) {
    reset_test_state();
    /* Fill entire buffer */
    memset(DAT_04630a00, 0xFF, sizeof(DAT_04630a00));

    FUN_00478bf0(0);

    ASSERT(is_slot_cleared(0));
    /* Data after slot 0 should still be 0xFF */
    ASSERT(DAT_04630a00[PET_SLOT_SIZE] == 0xFF);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Pet Data Clear Function Unit Tests ===\n\n");

    printf("FUN_00478bf0 (Clear Pet Data Slot) Tests:\n");
    RUN_TEST(clear_slot_0_success);
    RUN_TEST(clear_slot_1_success);
    RUN_TEST(clear_slot_negative_invalid);
    RUN_TEST(clear_slot_2_invalid);
    RUN_TEST(clear_slot_large_invalid);
    RUN_TEST(clear_slot_preserves_other);
    RUN_TEST(clear_already_cleared);
    RUN_TEST(clear_twice);
    RUN_TEST(clear_both_slots);
    RUN_TEST(clear_partial_preserves_rest);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
