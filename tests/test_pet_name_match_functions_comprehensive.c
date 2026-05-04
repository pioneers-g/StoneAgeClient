/*
 * Stone Age Client - Pet Name Match Function Unit Tests
 * Tests for FUN_004789d0 - Check if pet name matches any in array
 * Compares input string against array of strings at 0x44 byte intervals
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

/* Simulated pet name array (2 entries, 0x44 bytes each) */
#define PET_NAME_ENTRY_SIZE 0x44
#define PET_NAME_COUNT 2
static u8 DAT_04630a00[PET_NAME_ENTRY_SIZE * PET_NAME_COUNT + 0x10] = {0};

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

/* Set pet name at slot index */
static void set_pet_name(int slot, const char *name) {
    strncpy((char *)&DAT_04630a00[slot * PET_NAME_ENTRY_SIZE], name, PET_NAME_ENTRY_SIZE - 1);
}

/*
 * FUN_004789d0 - Check if name matches any pet in array
 * Compares param_1 against strings at DAT_04630a00 + (i * 0x44)
 * Returns 1 if match found, 0 if no match
 */
static u32 FUN_004789d0(const char *param_1) {
    u8 *pbVar4;
    const char *pbVar2;
    u8 *pbVar5;
    int iVar3;

    pbVar4 = &DAT_04630a00[0];
    pbVar5 = pbVar4;

    while (1) {
        pbVar2 = param_1;

        /* String compare loop */
        while (1) {
            u8 bVar1 = (u8)*pbVar2;
            int bVar6 = bVar1 < *pbVar4;

            if (bVar1 != *pbVar4) {
                iVar3 = (1 - bVar6) - (bVar6 != 0);
                break;
            }

            if (bVar1 == 0) {
                iVar3 = 0;
                break;
            }

            pbVar4 = pbVar4 + 1;
            pbVar2 = pbVar2 + 1;
        }

        if (iVar3 == 0) {
            return 1;  /* Match found */
        }

        pbVar4 = pbVar5 + PET_NAME_ENTRY_SIZE;
        pbVar5 = pbVar4;

        /* Check bounds (simplified for test) */
        if (pbVar4 >= &DAT_04630a00[PET_NAME_ENTRY_SIZE * PET_NAME_COUNT]) {
            return 0;  /* No match */
        }
    }
}

/* ========================================
 * Test Cases for FUN_004789d0
 * ======================================== */

TEST(match_empty_names) {
    reset_test_state();
    set_pet_name(0, "");
    set_pet_name(1, "");

    u32 result = FUN_004789d0("");

    ASSERT(result == 1);
}

TEST(match_first_slot) {
    reset_test_state();
    set_pet_name(0, "Fluffy");
    set_pet_name(1, "Spot");

    u32 result = FUN_004789d0("Fluffy");

    ASSERT(result == 1);
}

TEST(match_second_slot) {
    reset_test_state();
    set_pet_name(0, "Fluffy");
    set_pet_name(1, "Spot");

    u32 result = FUN_004789d0("Spot");

    ASSERT(result == 1);
}

TEST(no_match_different_name) {
    reset_test_state();
    set_pet_name(0, "Fluffy");
    set_pet_name(1, "Spot");

    u32 result = FUN_004789d0("Rex");

    ASSERT(result == 0);
}

TEST(no_match_partial) {
    reset_test_state();
    set_pet_name(0, "Fluffy");
    set_pet_name(1, "Spot");

    u32 result = FUN_004789d0("Fluf");

    ASSERT(result == 0);
}

TEST(no_match_longer) {
    reset_test_state();
    set_pet_name(0, "Fluffy");
    set_pet_name(1, "Spot");

    u32 result = FUN_004789d0("FluffyCat");

    ASSERT(result == 0);
}

TEST(match_case_sensitive) {
    reset_test_state();
    set_pet_name(0, "Fluffy");
    set_pet_name(1, "Spot");

    /* Should not match if case differs */
    u32 result = FUN_004789d0("fluffy");

    ASSERT(result == 0);
}

TEST(match_first_empty) {
    reset_test_state();
    set_pet_name(0, "");
    set_pet_name(1, "Spot");

    u32 result = FUN_004789d0("Spot");

    ASSERT(result == 1);
}

TEST(match_second_empty) {
    reset_test_state();
    set_pet_name(0, "Fluffy");
    set_pet_name(1, "");

    u32 result = FUN_004789d0("Fluffy");

    ASSERT(result == 1);
}

TEST(match_both_same_name) {
    reset_test_state();
    set_pet_name(0, "Pet");
    set_pet_name(1, "Pet");

    u32 result = FUN_004789d0("Pet");

    ASSERT(result == 1);  /* Matches first */
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Pet Name Match Function Unit Tests ===\n\n");

    printf("FUN_004789d0 (Check Pet Name Match) Tests:\n");
    RUN_TEST(match_empty_names);
    RUN_TEST(match_first_slot);
    RUN_TEST(match_second_slot);
    RUN_TEST(no_match_different_name);
    RUN_TEST(no_match_partial);
    RUN_TEST(no_match_longer);
    RUN_TEST(match_case_sensitive);
    RUN_TEST(match_first_empty);
    RUN_TEST(match_second_empty);
    RUN_TEST(match_both_same_name);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
