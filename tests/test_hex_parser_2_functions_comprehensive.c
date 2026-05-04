/*
 * Stone Age Client - Hex Parser 2 Function Unit Tests
 * Tests for FUN_0046b8a0 - Parse hex string to integer (alternate buffer)
 * This function parses hex characters from a different buffer than FUN_00476860
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

/* Simulated data buffer (different from FUN_00476860) */
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
 * FUN_0046b8a0 - Parse hex string to integer (alternate buffer)
 * Parses hex characters (0-9, A-F) from DAT_004d803c buffer
 * Returns -1 if no valid hex character found
 */
static s32 FUN_0046b8a0(void) {
    s32 iVar1;
    s32 iVar2;
    s32 iVar3;

    /* Skip non-hex characters */
    do {
        iVar1 = (s32)(u8)DAT_004d803c[DAT_0461c6c0];
        DAT_0461c6c0 = DAT_0461c6c0 + 1;
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
        iVar2 = (s32)(u8)DAT_004d803c[DAT_0461c6c0];
        iVar3 = DAT_0461c6c0 + 1;

        if (iVar2 == 0) {
            break;
        }
        if (((iVar2 < '0') || (iVar2 > '9')) &&
            ((iVar2 < 'A') || (iVar2 > 'F'))) {
            DAT_0461c6c0 = iVar3;
            break;
        }

        if (iVar2 < 'A') {
            iVar1 = iVar1 * 16 + (iVar2 - '0');
        } else {
            iVar1 = iVar1 * 16 + (iVar2 - 'A' + 10);
        }
    }

    return iVar1;
}

/* ========================================
 * Test Cases for FUN_0046b8a0 (Hex Parser 2)
 * ======================================== */

TEST(parse2_single_digit_0) {
    reset_test_state();
    strcpy(DAT_004d803c, "0");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046b8a0();

    ASSERT(result == 0);
}

TEST(parse2_single_digit_F) {
    reset_test_state();
    strcpy(DAT_004d803c, "F");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046b8a0();

    ASSERT(result == 15);
}

TEST(parse2_two_digits_FF) {
    reset_test_state();
    strcpy(DAT_004d803c, "FF");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046b8a0();

    ASSERT(result == 255);
}

TEST(parse2_four_digits) {
    reset_test_state();
    strcpy(DAT_004d803c, "ABCD");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046b8a0();

    ASSERT(result == 0xABCD);
}

TEST(parse2_eight_digits) {
    reset_test_state();
    strcpy(DAT_004d803c, "CAFEBABE");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046b8a0();

    ASSERT(result == 0xCAFEBABE);
}

TEST(parse2_empty_string) {
    reset_test_state();
    DAT_004d803c[0] = '\0';
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046b8a0();

    ASSERT(result == -1);
}

TEST(parse2_skip_nonhex) {
    reset_test_state();
    strcpy(DAT_004d803c, "!!!ABC");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046b8a0();

    ASSERT(result == 0xABC);
}

TEST(parse2_stop_at_nonhex) {
    reset_test_state();
    strcpy(DAT_004d803c, "123XYZ");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046b8a0();

    ASSERT(result == 0x123);
}

TEST(parse2_mixed_digits) {
    reset_test_state();
    strcpy(DAT_004d803c, "9A5B");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046b8a0();

    ASSERT(result == 0x9A5B);
}

TEST(parse2_max_value) {
    reset_test_state();
    strcpy(DAT_004d803c, "FFFFFFFF");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046b8a0();

    ASSERT(result == 0xFFFFFFFF);
}

TEST(parse2_updates_position) {
    reset_test_state();
    strcpy(DAT_004d803c, "ABCD");
    DAT_0461c6c0 = 0;

    FUN_0046b8a0();

    ASSERT(DAT_0461c6c0 == 4);
}

TEST(parse2_partial_updates_position) {
    reset_test_state();
    strcpy(DAT_004d803c, "12GH");
    DAT_0461c6c0 = 0;

    FUN_0046b8a0();

    /* Should stop at 'G', position at 'G' */
    ASSERT(DAT_0461c6c0 == 2);
}

TEST(parse2_continue_from_position) {
    reset_test_state();
    strcpy(DAT_004d803c, "1234ABCD");
    DAT_0461c6c0 = 0;

    s32 result1 = FUN_0046b8a0();
    s32 result2 = FUN_0046b8a0();

    ASSERT(result1 == 0x1234);
    ASSERT(result2 == 0xABCD);
}

TEST(parse2_with_separator) {
    reset_test_state();
    strcpy(DAT_004d803c, "FF|00");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046b8a0();

    ASSERT(result == 255);
}

TEST(parse2_lowercase_stops) {
    reset_test_state();
    strcpy(DAT_004d803c, "1a2b");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046b8a0();

    /* Should parse '1' then stop at 'a' */
    ASSERT(result == 1);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Hex Parser 2 Function Unit Tests ===\n\n");

    printf("FUN_0046b8a0 (Hex Parser 2) Tests:\n");
    RUN_TEST(parse2_single_digit_0);
    RUN_TEST(parse2_single_digit_F);
    RUN_TEST(parse2_two_digits_FF);
    RUN_TEST(parse2_four_digits);
    RUN_TEST(parse2_eight_digits);
    RUN_TEST(parse2_empty_string);
    RUN_TEST(parse2_skip_nonhex);
    RUN_TEST(parse2_stop_at_nonhex);
    RUN_TEST(parse2_mixed_digits);
    RUN_TEST(parse2_max_value);
    RUN_TEST(parse2_updates_position);
    RUN_TEST(parse2_partial_updates_position);
    RUN_TEST(parse2_continue_from_position);
    RUN_TEST(parse2_with_separator);
    RUN_TEST(parse2_lowercase_stops);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
