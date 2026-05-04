/*
 * Stone Age Client - Party Member Check Function Unit Tests
 * Tests for FUN_004781b0 - Check if any party member has specific sprite
 * This function checks if any party member has sprite ID 0x89c4
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

/* Simulated party member data (5 members, each 0x184 bytes apart) */
static s32 DAT_0462bf50 = 0;
static s32 DAT_0462bf94 = 0;  /* +0x44 */
static s32 DAT_0462bfd8 = 0;  /* +0x88 */
static s32 DAT_0462c01c = 0;  /* +0xCC */
static s32 DAT_0462c060 = 0;  /* +0x110 */

/* Flag to set */
static s32 DAT_0462bf2c = 0;

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
    DAT_0462bf50 = 0;
    DAT_0462bf94 = 0;
    DAT_0462bfd8 = 0;
    DAT_0462c01c = 0;
    DAT_0462c060 = 0;
    DAT_0462bf2c = 0;
}

/*
 * FUN_004781b0 - Check party member sprite
 * Checks if any party member (at offsets 0x184 = 388 apart)
 * has sprite ID 0x89c4. If found, sets flag 0x10000.
 */
static void FUN_004781b0(void) {
    int found = 0;
    s32 *piVar2 = &DAT_0462bf50;

    do {
        if (*piVar2 == 0x89c4) {
            found = 1;
        }
        piVar2 = piVar2 + 0x61;  /* +0x184 / 4 = +0x61 */
    } while ((int)piVar2 < 0x462ccf4);

    if (found) {
        DAT_0462bf2c = DAT_0462bf2c | 0x10000;
    }
}

/* ========================================
 * Test Cases for FUN_004781b0
 * ======================================== */

TEST(check_no_members) {
    reset_test_state();
    DAT_0462bf50 = 0;
    DAT_0462bf94 = 0;
    DAT_0462bfd8 = 0;
    DAT_0462c01c = 0;
    DAT_0462c060 = 0;

    FUN_004781b0();

    ASSERT((DAT_0462bf2c & 0x10000) == 0);
}

TEST(check_first_member_matches) {
    reset_test_state();
    DAT_0462bf50 = 0x89c4;

    FUN_004781b0();

    ASSERT((DAT_0462bf2c & 0x10000) != 0);
}

TEST(check_second_member_matches) {
    reset_test_state();
    DAT_0462bf94 = 0x89c4;

    FUN_004781b0();

    ASSERT((DAT_0462bf2c & 0x10000) != 0);
}

TEST(check_third_member_matches) {
    reset_test_state();
    DAT_0462bfd8 = 0x89c4;

    FUN_004781b0();

    ASSERT((DAT_0462bf2c & 0x10000) != 0);
}

TEST(check_fourth_member_matches) {
    reset_test_state();
    DAT_0462c01c = 0x89c4;

    FUN_004781b0();

    ASSERT((DAT_0462bf2c & 0x10000) != 0);
}

TEST(check_fifth_member_matches) {
    reset_test_state();
    DAT_0462c060 = 0x89c4;

    FUN_004781b0();

    ASSERT((DAT_0462bf2c & 0x10000) != 0);
}

TEST(check_no_match_different_sprite) {
    reset_test_state();
    DAT_0462bf50 = 0x1234;
    DAT_0462bf94 = 0x5678;
    DAT_0462bfd8 = 0xABCD;
    DAT_0462c01c = 0xEF01;
    DAT_0462c060 = 0x2345;

    FUN_004781b0();

    ASSERT((DAT_0462bf2c & 0x10000) == 0);
}

TEST(check_flag_preserved_if_not_set) {
    reset_test_state();
    DAT_0462bf2c = 0x00010000;  /* Already has a different flag */
    DAT_0462bf50 = 0;

    FUN_004781b0();

    ASSERT(DAT_0462bf2c == 0x00010000);  /* Should not change */
}

TEST(check_flag_or_operation) {
    reset_test_state();
    DAT_0462bf2c = 0x00010000;
    DAT_0462bf50 = 0x89c4;

    FUN_004781b0();

    ASSERT(DAT_0462bf2c == 0x00020000);  /* 0x10000 | 0x10000 */
}

TEST(check_multiple_members_same_sprite) {
    reset_test_state();
    DAT_0462bf50 = 0x89c4;
    DAT_0462bf94 = 0x89c4;

    FUN_004781b0();

    ASSERT((DAT_0462bf2c & 0x10000) != 0);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Party Member Check Function Unit Tests ===\n\n");

    printf("FUN_004781b0 (Check Party Member Sprite) Tests:\n");
    RUN_TEST(check_no_members);
    RUN_TEST(check_first_member_matches);
    RUN_TEST(check_second_member_matches);
    RUN_TEST(check_third_member_matches);
    RUN_TEST(check_fourth_member_matches);
    RUN_TEST(check_fifth_member_matches);
    RUN_TEST(check_no_match_different_sprite);
    RUN_TEST(check_flag_preserved_if_not_set);
    RUN_TEST(check_flag_or_operation);
    RUN_TEST(check_multiple_members_same_sprite);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
