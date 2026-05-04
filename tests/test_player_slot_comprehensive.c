/*
 * Stone Age Client - Player Slot Functions Unit Tests
 * Tests for FUN_004789a0, FUN_004789d0 (player slot functions)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;

/* ========================================
 * Test Data and Mock Implementations
 * ======================================== */

#define SLOT_COUNT 2
#define SLOT_STRIDE 0x44

static char test_slot_data[SLOT_COUNT * SLOT_STRIDE];

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

/* Reset slot data */
static void reset_slots(void) {
    memset(test_slot_data, 0, sizeof(test_slot_data));
}

/*
 * FUN_004789a0 - Check Player Slot Occupied
 */
static u32 FUN_004789a0(int slot_index) {
    if (slot_index < 0 || slot_index >= SLOT_COUNT) {
        return 0xffffffff;  /* Invalid slot */
    }

    return (test_slot_data[slot_index * SLOT_STRIDE] != '\0') ? 1 : 0;
}

/*
 * FUN_004789d0 - Get Player Slot Name Pointer
 */
static char* FUN_004789d0(int slot_index) {
    if (slot_index < 0 || slot_index >= SLOT_COUNT) {
        return NULL;
    }

    return &test_slot_data[slot_index * SLOT_STRIDE];
}

/*
 * FUN_00478bf0 - Clear Player Slot Data
 */
static int FUN_00478bf0(int slot_index) {
    u32* slot_data;
    int i;

    if (slot_index < 0 || slot_index >= SLOT_COUNT) {
        return -1;
    }

    /* Clear 17 dwords (0x44 bytes) */
    slot_data = (u32*)&test_slot_data[slot_index * SLOT_STRIDE];
    for (i = 0x11; i > 0; i--) {
        *slot_data = 0;
        slot_data++;
    }

    return 0;
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(slot_empty_initially) {
    reset_slots();

    ASSERT(FUN_004789a0(0) == 0);
    ASSERT(FUN_004789a0(1) == 0);
}

TEST(slot_occupied_after_set) {
    reset_slots();

    test_slot_data[0] = 'A';
    ASSERT(FUN_004789a0(0) == 1);
    ASSERT(FUN_004789a0(1) == 0);
}

TEST(invalid_slot_index_negative) {
    reset_slots();

    ASSERT(FUN_004789a0(-1) == 0xffffffff);
}

TEST(invalid_slot_index_too_high) {
    reset_slots();

    ASSERT(FUN_004789a0(2) == 0xffffffff);
    ASSERT(FUN_004789a0(100) == 0xffffffff);
}

TEST(get_slot_name_pointer_slot0) {
    reset_slots();

    char* name = FUN_004789d0(0);
    ASSERT(name != NULL);
    ASSERT(name == &test_slot_data[0]);
}

TEST(get_slot_name_pointer_slot1) {
    reset_slots();

    char* name = FUN_004789d0(1);
    ASSERT(name != NULL);
    ASSERT(name == &test_slot_data[SLOT_STRIDE]);
}

TEST(get_slot_name_pointer_invalid) {
    reset_slots();

    ASSERT(FUN_004789d0(-1) == NULL);
    ASSERT(FUN_004789d0(2) == NULL);
    ASSERT(FUN_004789d0(100) == NULL);
}

TEST(set_and_check_slot_name) {
    reset_slots();

    char* name0 = FUN_004789d0(0);
    strcpy(name0, "Player1");

    ASSERT(FUN_004789a0(0) == 1);
    ASSERT(FUN_004789a0(1) == 0);
    ASSERT(strcmp(name0, "Player1") == 0);
}

TEST(both_slots_independent) {
    reset_slots();

    char* name0 = FUN_004789d0(0);
    char* name1 = FUN_004789d0(1);

    strcpy(name0, "PlayerA");
    strcpy(name1, "PlayerB");

    ASSERT(FUN_004789a0(0) == 1);
    ASSERT(FUN_004789a0(1) == 1);
    ASSERT(strcmp(name0, "PlayerA") == 0);
    ASSERT(strcmp(name1, "PlayerB") == 0);
}

TEST(clear_slot) {
    reset_slots();

    char* name0 = FUN_004789d0(0);
    strcpy(name0, "TestPlayer");

    ASSERT(FUN_004789a0(0) == 1);

    name0[0] = '\0';  /* Clear slot */

    ASSERT(FUN_004789a0(0) == 0);
}

TEST(slot_stride_correct) {
    reset_slots();

    /* Write to slot 1 and verify it doesn't affect slot 0 */
    char* name1 = FUN_004789d0(1);
    strcpy(name1, "LongNameForSlot1");

    ASSERT(FUN_004789a0(0) == 0);
    ASSERT(FUN_004789a0(1) == 1);
}

/* ========================================
 * Clear Slot Function Tests
 * ======================================== */

TEST(clear_slot_function_valid) {
    reset_slots();

    char* name0 = FUN_004789d0(0);
    strcpy(name0, "PlayerToClear");

    ASSERT(FUN_004789a0(0) == 1);

    int result = FUN_00478bf0(0);
    ASSERT(result == 0);
    ASSERT(FUN_004789a0(0) == 0);
}

TEST(clear_slot_function_invalid) {
    reset_slots();

    ASSERT(FUN_00478bf0(-1) == -1);
    ASSERT(FUN_00478bf0(2) == -1);
    ASSERT(FUN_00478bf0(100) == -1);
}

TEST(clear_slot_preserves_other) {
    reset_slots();

    char* name0 = FUN_004789d0(0);
    char* name1 = FUN_004789d0(1);
    strcpy(name0, "Slot0");
    strcpy(name1, "Slot1");

    FUN_00478bf0(0);

    ASSERT(FUN_004789a0(0) == 0);
    ASSERT(FUN_004789a0(1) == 1);
    ASSERT(strcmp(name1, "Slot1") == 0);
}

TEST(clear_both_slots) {
    reset_slots();

    char* name0 = FUN_004789d0(0);
    char* name1 = FUN_004789d0(1);
    strcpy(name0, "Slot0");
    strcpy(name1, "Slot1");

    FUN_00478bf0(0);
    FUN_00478bf0(1);

    ASSERT(FUN_004789a0(0) == 0);
    ASSERT(FUN_004789a0(1) == 0);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Player Slot Functions Unit Tests ===\n\n");

    printf("Slot Occupied Tests:\n");
    RUN_TEST(slot_empty_initially);
    RUN_TEST(slot_occupied_after_set);

    printf("\nInvalid Index Tests:\n");
    RUN_TEST(invalid_slot_index_negative);
    RUN_TEST(invalid_slot_index_too_high);

    printf("\nGet Name Pointer Tests:\n");
    RUN_TEST(get_slot_name_pointer_slot0);
    RUN_TEST(get_slot_name_pointer_slot1);
    RUN_TEST(get_slot_name_pointer_invalid);

    printf("\nIntegration Tests:\n");
    RUN_TEST(set_and_check_slot_name);
    RUN_TEST(both_slots_independent);
    RUN_TEST(clear_slot);
    RUN_TEST(slot_stride_correct);

    printf("\nClear Slot Function Tests:\n");
    RUN_TEST(clear_slot_function_valid);
    RUN_TEST(clear_slot_function_invalid);
    RUN_TEST(clear_slot_preserves_other);
    RUN_TEST(clear_both_slots);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
