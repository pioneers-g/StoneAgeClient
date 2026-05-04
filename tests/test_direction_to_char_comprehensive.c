/*
 * Stone Age Client - Direction to Character Function Unit Tests
 * Tests for FUN_00444580 (direction index to character conversion)
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
 * FUN_00444580 - Direction Index to Character
 */
static char FUN_00444580(int direction, int uppercase) {
    if (direction >= 0 && direction < 3) {
        if (uppercase == 0) {
            return (char)direction + 'f';
        }
        return (char)direction + 'F';
    }
    if (direction < 8) {
        if (uppercase == 0) {
            return (char)direction + '^';
        }
        return (char)direction + '>';
    }
    return 'f';
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(direction_0_lowercase) {
    char result = FUN_00444580(0, 0);
    ASSERT(result == 'f');
}

TEST(direction_1_lowercase) {
    char result = FUN_00444580(1, 0);
    ASSERT(result == 'g');
}

TEST(direction_2_lowercase) {
    char result = FUN_00444580(2, 0);
    ASSERT(result == 'h');
}

TEST(direction_0_uppercase) {
    char result = FUN_00444580(0, 1);
    ASSERT(result == 'F');
}

TEST(direction_1_uppercase) {
    char result = FUN_00444580(1, 1);
    ASSERT(result == 'G');
}

TEST(direction_2_uppercase) {
    char result = FUN_00444580(2, 1);
    ASSERT(result == 'H');
}

TEST(direction_3_lowercase) {
    char result = FUN_00444580(3, 0);
    ASSERT(result == (char)(3 + '^'));
}

TEST(direction_4_lowercase) {
    char result = FUN_00444580(4, 0);
    ASSERT(result == (char)(4 + '^'));
}

TEST(direction_5_lowercase) {
    char result = FUN_00444580(5, 0);
    ASSERT(result == (char)(5 + '^'));
}

TEST(direction_6_lowercase) {
    char result = FUN_00444580(6, 0);
    ASSERT(result == (char)(6 + '^'));
}

TEST(direction_7_lowercase) {
    char result = FUN_00444580(7, 0);
    ASSERT(result == (char)(7 + '^'));
}

TEST(direction_3_uppercase) {
    char result = FUN_00444580(3, 1);
    ASSERT(result == (char)(3 + '>'));
}

TEST(direction_4_uppercase) {
    char result = FUN_00444580(4, 1);
    ASSERT(result == (char)(4 + '>'));
}

TEST(direction_7_uppercase) {
    char result = FUN_00444580(7, 1);
    ASSERT(result == (char)(7 + '>'));
}

TEST(direction_negative) {
    char result = FUN_00444580(-1, 0);
    /* For negative, the function calculates (-1 + '^') which is ASCII 93 = ']' */
    ASSERT(result == (char)(-1 + '^'));
}

TEST(direction_negative_uppercase) {
    char result = FUN_00444580(-1, 1);
    /* For negative with uppercase flag, calculates (-1 + '>') which is ASCII 61 = '=' */
    ASSERT(result == (char)(-1 + '>'));
}

TEST(direction_8_lowercase) {
    char result = FUN_00444580(8, 0);
    ASSERT(result == 'f');  /* Default return for out of range */
}

TEST(direction_8_uppercase) {
    char result = FUN_00444580(8, 1);
    ASSERT(result == 'f');  /* Default return for out of range */
}

TEST(direction_large_positive) {
    char result = FUN_00444580(100, 0);
    ASSERT(result == 'f');  /* Default return for out of range */
}

TEST(all_directions_lowercase) {
    /* Test all valid directions 0-7 with lowercase */
    for (int i = 0; i < 3; i++) {
        char result = FUN_00444580(i, 0);
        ASSERT(result == (char)(i + 'f'));
    }
    for (int i = 3; i < 8; i++) {
        char result = FUN_00444580(i, 0);
        ASSERT(result == (char)(i + '^'));
    }
}

TEST(all_directions_uppercase) {
    /* Test all valid directions 0-7 with uppercase */
    for (int i = 0; i < 3; i++) {
        char result = FUN_00444580(i, 1);
        ASSERT(result == (char)(i + 'F'));
    }
    for (int i = 3; i < 8; i++) {
        char result = FUN_00444580(i, 1);
        ASSERT(result == (char)(i + '>'));
    }
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Direction to Character Function Unit Tests ===\n\n");

    printf("Lowercase Direction Tests (0-2):\n");
    RUN_TEST(direction_0_lowercase);
    RUN_TEST(direction_1_lowercase);
    RUN_TEST(direction_2_lowercase);

    printf("\nUppercase Direction Tests (0-2):\n");
    RUN_TEST(direction_0_uppercase);
    RUN_TEST(direction_1_uppercase);
    RUN_TEST(direction_2_uppercase);

    printf("\nLowercase Direction Tests (3-7):\n");
    RUN_TEST(direction_3_lowercase);
    RUN_TEST(direction_4_lowercase);
    RUN_TEST(direction_5_lowercase);
    RUN_TEST(direction_6_lowercase);
    RUN_TEST(direction_7_lowercase);

    printf("\nUppercase Direction Tests (3-7):\n");
    RUN_TEST(direction_3_uppercase);
    RUN_TEST(direction_4_uppercase);
    RUN_TEST(direction_7_uppercase);

    printf("\nEdge Case Tests:\n");
    RUN_TEST(direction_negative);
    RUN_TEST(direction_negative_uppercase);
    RUN_TEST(direction_8_lowercase);
    RUN_TEST(direction_8_uppercase);
    RUN_TEST(direction_large_positive);

    printf("\nAll Directions Tests:\n");
    RUN_TEST(all_directions_lowercase);
    RUN_TEST(all_directions_uppercase);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
