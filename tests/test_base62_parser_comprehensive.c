/*
 * Stone Age Client - Base62 Parser Unit Tests
 * Tests for FUN_0048a0a0 - Parse Base62 string to integer
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
 * FUN_0048a0a0 - Parse Base62 string to integer
 *
 * Base62 uses characters: 0-9, a-z, A-Z
 * - '0'-'9' = 0-9
 * - 'a'-'z' = 10-35
 * - 'A'-'Z' = 36-61
 * - '-' = negative sign
 *
 * Returns: parsed integer value, or 0 on invalid input
 */
static int FUN_0048a0a0(char *param_1) {
    char cVar1;
    int iVar2;
    int iVar3;

    iVar3 = 0;
    cVar1 = *param_1;
    iVar2 = 1;
    do {
        if (cVar1 == '\0') {
            return iVar2 * iVar3;
        }
        iVar3 = iVar3 * 0x3e;  /* 62 * current value */
        if ((cVar1 < '0') || ('9' < cVar1)) {
            if ((cVar1 < 'a') || ('z' < cVar1)) {
                if ((cVar1 < 'A') || ('Z' < cVar1)) {
                    if (cVar1 != '-') {
                        return 0;  /* Invalid character */
                    }
                    iVar2 = -1;  /* Negative sign */
                }
                else {
                    iVar3 = iVar3 + -0x1d + (int)cVar1;  /* A-Z: 36-61 */
                }
            }
            else {
                iVar3 = iVar3 + -0x57 + (int)cVar1;  /* a-z: 10-35 */
            }
        }
        else {
            iVar3 = iVar3 + -0x30 + (int)cVar1;  /* 0-9: 0-9 */
        }
        cVar1 = param_1[1];
        param_1 = param_1 + 1;
    } while(1);
}

/* ========================================
 * Test Cases
 * ======================================== */

/* Single digit tests */
TEST(parse_digit_0) {
    char input[] = "0";
    int result = FUN_0048a0a0(input);
    ASSERT(result == 0);
}

TEST(parse_digit_9) {
    char input[] = "9";
    int result = FUN_0048a0a0(input);
    ASSERT(result == 9);
}

TEST(parse_letter_a) {
    char input[] = "a";
    int result = FUN_0048a0a0(input);
    ASSERT(result == 10);
}

TEST(parse_letter_z) {
    char input[] = "z";
    int result = FUN_0048a0a0(input);
    ASSERT(result == 35);
}

TEST(parse_letter_A) {
    char input[] = "A";
    int result = FUN_0048a0a0(input);
    ASSERT(result == 36);
}

TEST(parse_letter_Z) {
    char input[] = "Z";
    int result = FUN_0048a0a0(input);
    ASSERT(result == 61);
}

/* Multi-digit tests */
TEST(parse_two_digits_10) {
    char input[] = "10";
    int result = FUN_0048a0a0(input);
    ASSERT(result == 62);  /* 1*62 + 0 */
}

TEST(parse_two_digits_1a) {
    char input[] = "1a";
    int result = FUN_0048a0a0(input);
    ASSERT(result == 72);  /* 1*62 + 10 */
}

TEST(parse_two_digits_ZZ) {
    char input[] = "ZZ";
    int result = FUN_0048a0a0(input);
    ASSERT(result == 61*62 + 61);  /* 61*62 + 61 = 3843 */
}

TEST(parse_three_digits_100) {
    char input[] = "100";
    int result = FUN_0048a0a0(input);
    ASSERT(result == 62*62);  /* 1*62^2 + 0*62 + 0 = 3844 */
}

/* Empty string test */
TEST(parse_empty_string) {
    char input[] = "";
    int result = FUN_0048a0a0(input);
    ASSERT(result == 0);
}

/* Invalid character tests */
TEST(parse_invalid_char) {
    char input[] = "1@2";
    int result = FUN_0048a0a0(input);
    ASSERT(result == 0);  /* Returns 0 on invalid */
}

TEST(parse_space_invalid) {
    char input[] = "1 2";
    int result = FUN_0048a0a0(input);
    ASSERT(result == 0);  /* Space is invalid */
}

/* Negative number tests */
TEST(parse_negative_1) {
    char input[] = "-1";
    int result = FUN_0048a0a0(input);
    ASSERT(result == -1);
}

TEST(parse_negative_abc) {
    char input[] = "-abc";
    int result = FUN_0048a0a0(input);
    /* -a = -10, but we need to parse properly */
    /* Actually the '-' makes iVar2 = -1, then parses 'abc' */
    /* a=10, b=11, c=12 -> 10*62^2 + 11*62 + 12 = 38440 + 682 + 12 = 39134 */
    /* Result: -1 * 39134 = -39134 */
    ASSERT(result < 0);  /* Should be negative */
}

/* Edge case tests */
TEST(parse_mixed_case) {
    char input[] = "aA";
    int result = FUN_0048a0a0(input);
    /* a=10, A=36 -> 10*62 + 36 = 656 */
    ASSERT(result == 656);
}

TEST(parse_all_digit_types) {
    char input[] = "0aA";
    int result = FUN_0048a0a0(input);
    /* 0*62^2 + 10*62 + 36 = 0 + 620 + 36 = 656 */
    ASSERT(result == 656);
}

/* Large value tests */
TEST(parse_large_value) {
    char input[] = "1000";
    int result = FUN_0048a0a0(input);
    /* 1*62^3 = 238328 */
    ASSERT(result == 238328);
}

TEST(parse_max_single) {
    char input[] = "Z";
    int result = FUN_0048a0a0(input);
    ASSERT(result == 61);
}

TEST(parse_sequence_10) {
    char input[] = "10";
    int result = FUN_0048a0a0(input);
    ASSERT(result == 62);
}

TEST(parse_sequence_1Z) {
    char input[] = "1Z";
    int result = FUN_0048a0a0(input);
    /* 1*62 + 61 = 123 */
    ASSERT(result == 123);
}

/* Base conversion verification */
TEST(base62_verify_formula) {
    /* Verify: digit * 62^position + ... */
    char input[] = "2";
    int result = FUN_0048a0a0(input);
    ASSERT(result == 2);

    char input2[] = "20";
    result = FUN_0048a0a0(input2);
    ASSERT(result == 2*62 + 0);  /* 124 */

    char input3[] = "200";
    result = FUN_0048a0a0(input3);
    ASSERT(result == 2*62*62);  /* 7688 */
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Base62 Parser Unit Tests ===\n\n");

    printf("Single Digit Tests:\n");
    RUN_TEST(parse_digit_0);
    RUN_TEST(parse_digit_9);
    RUN_TEST(parse_letter_a);
    RUN_TEST(parse_letter_z);
    RUN_TEST(parse_letter_A);
    RUN_TEST(parse_letter_Z);

    printf("\nMulti-Digit Tests:\n");
    RUN_TEST(parse_two_digits_10);
    RUN_TEST(parse_two_digits_1a);
    RUN_TEST(parse_two_digits_ZZ);
    RUN_TEST(parse_three_digits_100);

    printf("\nEdge Case Tests:\n");
    RUN_TEST(parse_empty_string);
    RUN_TEST(parse_invalid_char);
    RUN_TEST(parse_space_invalid);

    printf("\nNegative Number Tests:\n");
    RUN_TEST(parse_negative_1);
    RUN_TEST(parse_negative_abc);

    printf("\nMixed Case Tests:\n");
    RUN_TEST(parse_mixed_case);
    RUN_TEST(parse_all_digit_types);

    printf("\nLarge Value Tests:\n");
    RUN_TEST(parse_large_value);
    RUN_TEST(parse_max_single);
    RUN_TEST(parse_sequence_10);
    RUN_TEST(parse_sequence_1Z);

    printf("\nBase Conversion Verification:\n");
    RUN_TEST(base62_verify_formula);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
