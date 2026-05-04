/*
 * Stone Age Client - Sprite Range Check Function Unit Tests
 * Tests for FUN_004473f0 (check if sprite ID is in special effect range)
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
 * FUN_004473f0 - Check Sprite ID Range
 * Range: (0x623e, 0x6299) - exclusive bounds
 */
static int FUN_004473f0(int sprite_id) {
    if (sprite_id > 0x623e && sprite_id < 0x6299) {
        return 1;
    }
    return 0;
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(below_range) {
    ASSERT(FUN_004473f0(0) == 0);
    ASSERT(FUN_004473f0(1000) == 0);
    ASSERT(FUN_004473f0(0x6000) == 0);
}

TEST(at_lower_bound) {
    /* Lower bound is exclusive */
    ASSERT(FUN_004473f0(0x623e) == 0);
}

TEST(just_above_lower_bound) {
    /* Just above lower bound should pass */
    ASSERT(FUN_004473f0(0x623f) == 1);
}

TEST(in_range_middle) {
    /* Middle of range */
    ASSERT(FUN_004473f0(0x6269) == 1);
}

TEST(just_below_upper_bound) {
    /* Just below upper bound should pass */
    ASSERT(FUN_004473f0(0x6298) == 1);
}

TEST(at_upper_bound) {
    /* Upper bound is exclusive */
    ASSERT(FUN_004473f0(0x6299) == 0);
}

TEST(above_range) {
    ASSERT(FUN_004473f0(0x629a) == 0);
    ASSERT(FUN_004473f0(0x6300) == 0);
    ASSERT(FUN_004473f0(0x10000) == 0);
}

TEST(negative_sprite_id) {
    ASSERT(FUN_004473f0(-1) == 0);
    ASSERT(FUN_004473f0(-100) == 0);
    ASSERT(FUN_004473f0(-0x623f) == 0);
}

TEST(range_boundaries) {
    /* Test all boundary conditions */
    ASSERT(FUN_004473f0(0x623d) == 0);  /* Below lower */
    ASSERT(FUN_004473f0(0x623e) == 0);  /* At lower (exclusive) */
    ASSERT(FUN_004473f0(0x623f) == 1);  /* Just above lower */
    ASSERT(FUN_004473f0(0x6298) == 1);  /* Just below upper */
    ASSERT(FUN_004473f0(0x6299) == 0);  /* At upper (exclusive) */
    ASSERT(FUN_004473f0(0x629a) == 0);  /* Above upper */
}

TEST(full_range_scan) {
    /* Count how many values are in range */
    int count = 0;
    int i;
    for (i = 0; i < 0x10000; i++) {
        if (FUN_004473f0(i)) count++;
    }
    /* Range is (0x623e, 0x6299) = 0x6299 - 0x623e - 1 = 90 */
    ASSERT(count == 90);
}

TEST(special_values) {
    /* Test specific values that might be special effects */
    ASSERT(FUN_004473f0(0x6240) == 1);
    ASSERT(FUN_004473f0(0x6250) == 1);
    ASSERT(FUN_004473f0(0x6260) == 1);
    ASSERT(FUN_004473f0(0x6270) == 1);
    ASSERT(FUN_004473f0(0x6280) == 1);
    ASSERT(FUN_004473f0(0x6290) == 1);
}

TEST(returns_boolean) {
    /* Verify function only returns 0 or 1 */
    int result;
    int i;
    for (i = -10; i < 0x6400; i++) {
        result = FUN_004473f0(i);
        ASSERT(result == 0 || result == 1);
    }
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Sprite Range Check Function Unit Tests ===\n\n");

    printf("Out of Range Tests:\n");
    RUN_TEST(below_range);
    RUN_TEST(above_range);
    RUN_TEST(negative_sprite_id);

    printf("\nBoundary Tests:\n");
    RUN_TEST(at_lower_bound);
    RUN_TEST(just_above_lower_bound);
    RUN_TEST(just_below_upper_bound);
    RUN_TEST(at_upper_bound);
    RUN_TEST(range_boundaries);

    printf("\nIn-Range Tests:\n");
    RUN_TEST(in_range_middle);
    RUN_TEST(special_values);

    printf("\nComprehensive Tests:\n");
    RUN_TEST(full_range_scan);
    RUN_TEST(returns_boolean);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
