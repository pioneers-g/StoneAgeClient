/*
 * Stone Age Client - Pattern Search Function Unit Tests
 * Tests for FUN_0046db40 - Search for "B?|" pattern
 * This function searches for a specific pattern in the data buffer
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

/* Simulated data buffer */
static char DAT_004d803c[256] = {0};
static u32 DAT_0461c6c0 = 0;

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
    memset(DAT_004d803c, 0, sizeof(DAT_004d803c));
    DAT_0461c6c0 = 0;
}

/*
 * FUN_0046db40 - Search for "B?|" pattern
 * Searches for pattern "B" followed by any char, then "|" or end
 * Returns the character after 'B', or 0xFF if not found
 */
static u8 FUN_0046db40(void) {
    s32 iVar2;
    char cVar1;

    do {
        iVar2 = DAT_0461c6c0;
        DAT_0461c6c0 = iVar2 + 1;
        if (DAT_004d803c[iVar2] == '\0') {
            return 0xFF;
        }
    } while ((DAT_004d803c[iVar2] != 'B') ||
             (DAT_004d803c[iVar2 + 2] != '|') ||
             ((cVar1 = DAT_004d803c[iVar2 + 3], cVar1 != '|' && cVar1 != '\0')));

    DAT_0461c6c0 = iVar2 + 2;
    return DAT_004d803c[iVar2 + 1];
}

/* ========================================
 * Test Cases for FUN_0046db40 (Pattern Search)
 * ======================================== */

TEST(pattern_not_found_empty) {
    reset_test_state();
    DAT_004d803c[0] = '\0';
    DAT_0461c6c0 = 0;

    u8 result = FUN_0046db40();

    ASSERT(result == 0xFF);
}

TEST(pattern_not_found_no_B) {
    reset_test_state();
    strcpy(DAT_004d803c, "ABC|");
    DAT_0461c6c0 = 0;

    u8 result = FUN_0046db40();

    ASSERT(result == 0xFF);
}

TEST(pattern_found_simple) {
    reset_test_state();
    strcpy(DAT_004d803c, "BX||");
    DAT_0461c6c0 = 0;

    u8 result = FUN_0046db40();

    ASSERT(result == 'X');
}

TEST(pattern_found_digit) {
    reset_test_state();
    strcpy(DAT_004d803c, "B1||");
    DAT_0461c6c0 = 0;

    u8 result = FUN_0046db40();

    ASSERT(result == '1');
}

TEST(pattern_found_after_prefix) {
    reset_test_state();
    strcpy(DAT_004d803c, "PREFIX_BA||");
    DAT_0461c6c0 = 0;

    u8 result = FUN_0046db40();

    ASSERT(result == 'A');
}

TEST(pattern_found_updates_position) {
    reset_test_state();
    strcpy(DAT_004d803c, "BC||");
    DAT_0461c6c0 = 0;

    FUN_0046db40();

    ASSERT(DAT_0461c6c0 == 2);
}

TEST(pattern_not_found_wrong_terminator) {
    reset_test_state();
    strcpy(DAT_004d803c, "BXYZ");
    DAT_0461c6c0 = 0;

    u8 result = FUN_0046db40();

    ASSERT(result == 0xFF);
}

TEST(pattern_multiple_B) {
    reset_test_state();
    strcpy(DAT_004d803c, "B1B2||");
    DAT_0461c6c0 = 0;

    u8 result = FUN_0046db40();

    /* Should find B2|| since first B doesn't have proper terminator */
    ASSERT(result == '2');
}

TEST(pattern_null_terminator) {
    reset_test_state();
    strcpy(DAT_004d803c, "BQ|");
    DAT_004d803c[3] = '\0';  /* Null after | */
    DAT_0461c6c0 = 0;

    u8 result = FUN_0046db40();

    /* BQ| followed by null should match */
    ASSERT(result == 'Q');
}

TEST(pattern_found_space_char) {
    reset_test_state();
    strcpy(DAT_004d803c, "B ||");
    DAT_0461c6c0 = 0;

    u8 result = FUN_0046db40();

    ASSERT(result == ' ');
}

TEST(pattern_continues_from_position) {
    reset_test_state();
    strcpy(DAT_004d803c, "SKIP_BZ||");
    DAT_0461c6c0 = 5;  /* Start after SKIP_ */

    u8 result = FUN_0046db40();

    ASSERT(result == 'Z');
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Pattern Search Function Unit Tests ===\n\n");

    printf("FUN_0046db40 (Pattern Search) Tests:\n");
    RUN_TEST(pattern_not_found_empty);
    RUN_TEST(pattern_not_found_no_B);
    RUN_TEST(pattern_found_simple);
    RUN_TEST(pattern_found_digit);
    RUN_TEST(pattern_found_after_prefix);
    RUN_TEST(pattern_found_updates_position);
    RUN_TEST(pattern_not_found_wrong_terminator);
    RUN_TEST(pattern_multiple_B);
    RUN_TEST(pattern_null_terminator);
    RUN_TEST(pattern_found_space_char);
    RUN_TEST(pattern_continues_from_position);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
