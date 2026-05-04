/*
 * Stone Age Client - Hex Parser Function Unit Tests
 * Tests for FUN_00476860 - Parse hex string to integer
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

static char DAT_004e1118[256] = {0};  /* Input buffer */
static u32 DAT_0461c678 = 0;          /* Current position in buffer */

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

/*
 * FUN_00476860 - Parse Hex String to Integer
 *
 * Binary analysis:
 * - Parses hex string from DAT_004e1118 starting at DAT_0461c678
 * - Supports digits 0-9 and letters A-F (uppercase)
 * - Returns -1 if no valid hex digit found
 * - Returns parsed integer value
 */
static int FUN_00476860(void) {
    char c;
    int result;
    u32 pos = DAT_0461c678;

    /* Skip non-hex characters */
    while (1) {
        c = DAT_004e1118[pos];
        if (c == '\0') {
            DAT_0461c678 = pos;
            return -1;
        }
        pos++;

        /* Check if valid hex digit */
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')) {
            break;
        }
    }

    /* Convert first digit */
    if (c >= '0' && c <= '9') {
        result = c - '0';
    } else {
        result = c - 'A' + 10;
    }

    /* Parse remaining digits */
    while (1) {
        c = DAT_004e1118[pos];
        if (c == '\0') {
            break;
        }

        /* Check if valid hex digit */
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
            break;
        }

        pos++;

        if (c >= '0' && c <= '9') {
            result = result * 16 + (c - '0');
        } else {
            result = result * 16 + (c - 'A' + 10);
        }
    }

    DAT_0461c678 = pos;
    return result;
}

/* Reset test state */
static void reset_test_state(void) {
    memset(DAT_004e1118, 0, sizeof(DAT_004e1118));
    DAT_0461c678 = 0;
}

/* Set input buffer */
static void set_input(const char* str) {
    reset_test_state();
    strncpy(DAT_004e1118, str, sizeof(DAT_004e1118) - 1);
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(parse_single_digit_0) {
    set_input("0");

    int result = FUN_00476860();

    ASSERT(result == 0);
}

TEST(parse_single_digit_9) {
    set_input("9");

    int result = FUN_00476860();

    ASSERT(result == 9);
}

TEST(parse_single_letter_A) {
    set_input("A");

    int result = FUN_00476860();

    ASSERT(result == 10);
}

TEST(parse_single_letter_F) {
    set_input("F");

    int result = FUN_00476860();

    ASSERT(result == 15);
}

TEST(parse_two_digits_10) {
    set_input("10");

    int result = FUN_00476860();

    ASSERT(result == 16);
}

TEST(parse_two_digits_FF) {
    set_input("FF");

    int result = FUN_00476860();

    ASSERT(result == 255);
}

TEST(parse_two_digits_AB) {
    set_input("AB");

    int result = FUN_00476860();

    ASSERT(result == 171);
}

TEST(parse_four_digits_1234) {
    set_input("1234");

    int result = FUN_00476860();

    ASSERT(result == 0x1234);
}

TEST(parse_four_digits_ABCD) {
    set_input("ABCD");

    int result = FUN_00476860();

    ASSERT(result == 0xABCD);
}

TEST(parse_max_value) {
    set_input("FFFFFFFF");

    int result = FUN_00476860();

    ASSERT(result == 0xFFFFFFFF);
}

TEST(parse_empty_string) {
    set_input("");

    int result = FUN_00476860();

    ASSERT(result == -1);
}

TEST(parse_no_hex_digits) {
    set_input("GHIJ");

    int result = FUN_00476860();

    ASSERT(result == -1);
}

TEST(parse_skip_prefix) {
    set_input("0x1234");

    int result = FUN_00476860();

    /* '0' is valid hex, so parses as 0, stops at 'x' */
    ASSERT(result == 0);
}

TEST(parse_skip_whitespace) {
    set_input("   ABC");

    int result = FUN_00476860();

    ASSERT(result == 0xABC);
}

TEST(parse_lowercase_rejected) {
    set_input("ab");

    int result = FUN_00476860();

    /* Lowercase 'a' is rejected, should return -1 */
    ASSERT(result == -1);
}

TEST(parse_mixed_valid_invalid) {
    set_input("12GH34");

    int result = FUN_00476860();

    /* Should parse 12 and stop at G */
    ASSERT(result == 0x12);
    ASSERT(DAT_0461c678 == 2);
}

TEST(parse_multiple_calls) {
    set_input("1234ABCD");

    int result1 = FUN_00476860();
    ASSERT(result1 == 0x1234ABCD);

    int result2 = FUN_00476860();
    ASSERT(result2 == -1);
}

TEST(parse_with_separator) {
    set_input("12|34");

    int result = FUN_00476860();

    ASSERT(result == 0x12);
}

TEST(parse_position_update) {
    set_input("ABCDEF");

    ASSERT(DAT_0461c678 == 0);

    FUN_00476860();

    ASSERT(DAT_0461c678 == 6);
}

TEST(parse_zero_after_value) {
    set_input("1230");

    int result = FUN_00476860();

    ASSERT(result == 0x1230);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Hex Parser Function Unit Tests ===\n\n");

    printf("Single Digit Tests:\n");
    RUN_TEST(parse_single_digit_0);
    RUN_TEST(parse_single_digit_9);
    RUN_TEST(parse_single_letter_A);
    RUN_TEST(parse_single_letter_F);

    printf("\nTwo Digit Tests:\n");
    RUN_TEST(parse_two_digits_10);
    RUN_TEST(parse_two_digits_FF);
    RUN_TEST(parse_two_digits_AB);

    printf("\nFour Digit Tests:\n");
    RUN_TEST(parse_four_digits_1234);
    RUN_TEST(parse_four_digits_ABCD);

    printf("\nEdge Case Tests:\n");
    RUN_TEST(parse_max_value);
    RUN_TEST(parse_empty_string);
    RUN_TEST(parse_no_hex_digits);
    RUN_TEST(parse_skip_prefix);
    RUN_TEST(parse_skip_whitespace);
    RUN_TEST(parse_lowercase_rejected);

    printf("\nComplex Tests:\n");
    RUN_TEST(parse_mixed_valid_invalid);
    RUN_TEST(parse_multiple_calls);
    RUN_TEST(parse_with_separator);
    RUN_TEST(parse_position_update);
    RUN_TEST(parse_zero_after_value);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
