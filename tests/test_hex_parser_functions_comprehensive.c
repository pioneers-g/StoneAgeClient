/*
 * Stone Age Client - Hex Parser Function Unit Tests
 * Tests for FUN_00476860 - Parse hex string to integer
 * This function parses hex characters from a buffer
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
static char DAT_004e1118[256] = {0};
static u32 DAT_0461c678 = 0;

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
    memset(DAT_004e1118, 0, sizeof(DAT_004e1118));
    DAT_0461c678 = 0;
}

/*
 * FUN_00476860 - Parse hex string to integer
 * Parses hex characters (0-9, A-F) from buffer and returns the value
 * Returns -1 if no valid hex character found
 */
static s32 FUN_00476860(void) {
    s32 iVar1;
    s32 iVar3;
    s32 iVar4;

    /* Skip non-hex characters */
    do {
        iVar1 = (s32)(u8)DAT_004e1118[DAT_0461c678];
        iVar3 = DAT_0461c678 + 1;
        if (iVar1 == 0) {
            return -1;
        }
    } while (((iVar1 < '0') || (iVar1 > '9')) &&
             ((iVar1 < 'A') || (iVar1 > 'F')));

    /* Convert first hex char */
    if (iVar1 < 'A') {
        iVar1 = iVar1 - '0';
    } else {
        iVar1 = iVar1 - 'A' + 10;
    }

    /* Parse remaining hex chars */
    while (1) {
        s32 iVar2 = (s32)(u8)DAT_004e1118[iVar3];
        iVar4 = iVar3 + 1;

        if (iVar2 == 0) {
            break;
        }
        if (((iVar2 < '0') || (iVar2 > '9')) &&
            ((iVar2 < 'A') || (iVar2 > 'F'))) {
            DAT_0461c678 = iVar3;
            break;
        }

        if (iVar2 < 'A') {
            iVar1 = iVar1 * 16 + (iVar2 - '0');
        } else {
            iVar1 = iVar1 * 16 + (iVar2 - 'A' + 10);
        }
        iVar3 = iVar4;
    }

    DAT_0461c678 = iVar3;
    return iVar1;
}

/* ========================================
 * Test Cases for FUN_00476860 (Hex Parser)
 * ======================================== */

TEST(parse_single_digit_0) {
    reset_test_state();
    strcpy(DAT_004e1118, "0");
    DAT_0461c678 = 0;

    s32 result = FUN_00476860();

    ASSERT(result == 0);
}

TEST(parse_single_digit_9) {
    reset_test_state();
    strcpy(DAT_004e1118, "9");
    DAT_0461c678 = 0;

    s32 result = FUN_00476860();

    ASSERT(result == 9);
}

TEST(parse_single_digit_A) {
    reset_test_state();
    strcpy(DAT_004e1118, "A");
    DAT_0461c678 = 0;

    s32 result = FUN_00476860();

    ASSERT(result == 10);
}

TEST(parse_single_digit_F) {
    reset_test_state();
    strcpy(DAT_004e1118, "F");
    DAT_0461c678 = 0;

    s32 result = FUN_00476860();

    ASSERT(result == 15);
}

TEST(parse_two_digits_10) {
    reset_test_state();
    strcpy(DAT_004e1118, "10");
    DAT_0461c678 = 0;

    s32 result = FUN_00476860();

    ASSERT(result == 16);
}

TEST(parse_two_digits_FF) {
    reset_test_state();
    strcpy(DAT_004e1118, "FF");
    DAT_0461c678 = 0;

    s32 result = FUN_00476860();

    ASSERT(result == 255);
}

TEST(parse_two_digits_AB) {
    reset_test_state();
    strcpy(DAT_004e1118, "AB");
    DAT_0461c678 = 0;

    s32 result = FUN_00476860();

    ASSERT(result == 171);
}

TEST(parse_four_digits) {
    reset_test_state();
    strcpy(DAT_004e1118, "1234");
    DAT_0461c678 = 0;

    s32 result = FUN_00476860();

    ASSERT(result == 0x1234);
}

TEST(parse_eight_digits) {
    reset_test_state();
    strcpy(DAT_004e1118, "DEADBEEF");
    DAT_0461c678 = 0;

    s32 result = FUN_00476860();

    ASSERT(result == 0xDEADBEEF);
}

TEST(parse_empty_string) {
    reset_test_state();
    DAT_004e1118[0] = '\0';
    DAT_0461c678 = 0;

    s32 result = FUN_00476860();

    ASSERT(result == -1);
}

TEST(parse_skip_nonhex) {
    reset_test_state();
    strcpy(DAT_004e1118, "XYZ123");
    DAT_0461c678 = 0;

    s32 result = FUN_00476860();

    ASSERT(result == 0x123);
}

TEST(parse_stop_at_nonhex) {
    reset_test_state();
    strcpy(DAT_004e1118, "FFGHH");
    DAT_0461c678 = 0;

    s32 result = FUN_00476860();

    ASSERT(result == 255);
}

TEST(parse_skip_spaces) {
    reset_test_state();
    strcpy(DAT_004e1118, "   AB");
    DAT_0461c678 = 0;

    s32 result = FUN_00476860();

    ASSERT(result == 0xAB);
}

TEST(parse_lowercase_not_supported) {
    reset_test_state();
    /* Lowercase 'a' is not in the range 'A'-'F', so it should stop */
    strcpy(DAT_004e1118, "1a2b");
    DAT_0461c678 = 0;

    s32 result = FUN_00476860();

    /* Should parse '1' then stop at 'a' */
    ASSERT(result == 1);
}

TEST(parse_mixed_digits) {
    reset_test_state();
    strcpy(DAT_004e1118, "A5B3");
    DAT_0461c678 = 0;

    s32 result = FUN_00476860();

    ASSERT(result == 0xA5B3);
}

TEST(parse_zero_prefix) {
    reset_test_state();
    strcpy(DAT_004e1118, "00FF");
    DAT_0461c678 = 0;

    s32 result = FUN_00476860();

    ASSERT(result == 255);
}

TEST(parse_updates_position) {
    reset_test_state();
    strcpy(DAT_004e1118, "1234");
    DAT_0461c678 = 0;

    FUN_00476860();

    ASSERT(DAT_0461c678 == 4);
}

TEST(parse_partial_updates_position) {
    reset_test_state();
    strcpy(DAT_004e1118, "FFGH");
    DAT_0461c678 = 0;

    FUN_00476860();

    /* Should stop at 'G', position at 'G' */
    ASSERT(DAT_0461c678 == 2);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Hex Parser Function Unit Tests ===\n\n");

    printf("FUN_00476860 (Hex Parser) Tests:\n");
    RUN_TEST(parse_single_digit_0);
    RUN_TEST(parse_single_digit_9);
    RUN_TEST(parse_single_digit_A);
    RUN_TEST(parse_single_digit_F);
    RUN_TEST(parse_two_digits_10);
    RUN_TEST(parse_two_digits_FF);
    RUN_TEST(parse_two_digits_AB);
    RUN_TEST(parse_four_digits);
    RUN_TEST(parse_eight_digits);
    RUN_TEST(parse_empty_string);
    RUN_TEST(parse_skip_nonhex);
    RUN_TEST(parse_stop_at_nonhex);
    RUN_TEST(parse_skip_spaces);
    RUN_TEST(parse_lowercase_not_supported);
    RUN_TEST(parse_mixed_digits);
    RUN_TEST(parse_zero_prefix);
    RUN_TEST(parse_updates_position);
    RUN_TEST(parse_partial_updates_position);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
