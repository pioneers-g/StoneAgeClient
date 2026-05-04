/*
 * Stone Age Client - Direction Character Functions Unit Tests
 * Tests for FUN_00444580 - Get direction character
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

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
 * FUN_00444580 - Get direction character
 * Returns character for direction (0-7) and case flag
 * Directions 0-2: return 'f'/'F' + direction (fgh / FGH)
 * Directions 3-7: return '^'/'>' + direction
 */
static char FUN_00444580(s32 param_1, s32 param_2) {
    char cVar2;
    char cVar1;

    cVar2 = 'f';
    cVar1 = (char)param_1;
    if ((param_1 >= 0) && (param_1 < 3)) {
        if (param_2 == 0) {
            return cVar1 + 'f';
        }
        return cVar1 + 'F';
    }
    if (param_1 < 8) {
        if (param_2 == 0) {
            return cVar1 + '^';
        }
        cVar2 = cVar1 + '>';
    }
    return cVar2;
}

/* ========================================
 * Test Cases for FUN_00444580
 * ======================================== */

TEST(dir_0_lower) {
    char result = FUN_00444580(0, 0);
    ASSERT(result == 'f');
}

TEST(dir_0_upper) {
    char result = FUN_00444580(0, 1);
    ASSERT(result == 'F');
}

TEST(dir_1_lower) {
    char result = FUN_00444580(1, 0);
    ASSERT(result == 'g');
}

TEST(dir_1_upper) {
    char result = FUN_00444580(1, 1);
    ASSERT(result == 'G');
}

TEST(dir_2_lower) {
    char result = FUN_00444580(2, 0);
    ASSERT(result == 'h');
}

TEST(dir_2_upper) {
    char result = FUN_00444580(2, 1);
    ASSERT(result == 'H');
}

TEST(dir_3_lower) {
    char result = FUN_00444580(3, 0);
    ASSERT(result == (3 + '^'));  /* 3 + 94 = 97 = 'a' */
}

TEST(dir_3_upper) {
    char result = FUN_00444580(3, 1);
    ASSERT(result == (3 + '>'));  /* 3 + 62 = 65 = 'A' */
}

TEST(dir_4_lower) {
    char result = FUN_00444580(4, 0);
    ASSERT(result == (4 + '^'));
}

TEST(dir_4_upper) {
    char result = FUN_00444580(4, 1);
    ASSERT(result == (4 + '>'));
}

TEST(dir_5_lower) {
    char result = FUN_00444580(5, 0);
    ASSERT(result == (5 + '^'));
}

TEST(dir_5_upper) {
    char result = FUN_00444580(5, 1);
    ASSERT(result == (5 + '>'));
}

TEST(dir_6_lower) {
    char result = FUN_00444580(6, 0);
    ASSERT(result == (6 + '^'));
}

TEST(dir_6_upper) {
    char result = FUN_00444580(6, 1);
    ASSERT(result == (6 + '>'));
}

TEST(dir_7_lower) {
    char result = FUN_00444580(7, 0);
    ASSERT(result == (7 + '^'));
}

TEST(dir_7_upper) {
    char result = FUN_00444580(7, 1);
    ASSERT(result == (7 + '>'));
}

TEST(dir_negative) {
    char result = FUN_00444580(-1, 0);
    /* -1 < 8 so enters second branch, returns (char)-1 + '^' */
    ASSERT(result == (char)(-1 + '^'));
}

TEST(dir_too_large) {
    char result = FUN_00444580(8, 0);
    ASSERT(result == 'f');  /* Default return */
}

TEST(dir_negative_upper) {
    char result = FUN_00444580(-1, 1);
    /* -1 < 8 so enters second branch, returns (char)-1 + '>' */
    ASSERT(result == (char)(-1 + '>'));
}

TEST(dir_8_upper) {
    char result = FUN_00444580(8, 1);
    ASSERT(result == 'f');  /* Default return */
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Direction Character Functions Unit Tests ===\n\n");

    printf("FUN_00444580 (Get Direction Character) Tests:\n");
    RUN_TEST(dir_0_lower);
    RUN_TEST(dir_0_upper);
    RUN_TEST(dir_1_lower);
    RUN_TEST(dir_1_upper);
    RUN_TEST(dir_2_lower);
    RUN_TEST(dir_2_upper);
    RUN_TEST(dir_3_lower);
    RUN_TEST(dir_3_upper);
    RUN_TEST(dir_4_lower);
    RUN_TEST(dir_4_upper);
    RUN_TEST(dir_5_lower);
    RUN_TEST(dir_5_upper);
    RUN_TEST(dir_6_lower);
    RUN_TEST(dir_6_upper);
    RUN_TEST(dir_7_lower);
    RUN_TEST(dir_7_upper);
    RUN_TEST(dir_negative);
    RUN_TEST(dir_too_large);
    RUN_TEST(dir_negative_upper);
    RUN_TEST(dir_8_upper);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
