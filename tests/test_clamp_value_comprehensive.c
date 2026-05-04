/*
 * Stone Age Client - Clamp Value Function Unit Tests
 * Tests for FUN_004474e0 (clamp value to range)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
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
 * FUN_004474e0 - Clamp Value to Range
 */
static int FUN_004474e0(int value, int min_val, int max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(clamp_below_min) {
    ASSERT(FUN_004474e0(-5, 0, 10) == 0);
    ASSERT(FUN_004474e0(-100, 0, 10) == 0);
    ASSERT(FUN_004474e0(4, 5, 10) == 5);
}

TEST(clamp_above_max) {
    ASSERT(FUN_004474e0(15, 0, 10) == 10);
    ASSERT(FUN_004474e0(100, 0, 10) == 10);
    ASSERT(FUN_004474e0(11, 5, 10) == 10);
}

TEST(clamp_within_range) {
    ASSERT(FUN_004474e0(5, 0, 10) == 5);
    ASSERT(FUN_004474e0(0, 0, 10) == 0);
    ASSERT(FUN_004474e0(10, 0, 10) == 10);
    ASSERT(FUN_004474e0(7, 5, 10) == 7);
}

TEST(clamp_at_boundaries) {
    ASSERT(FUN_004474e0(0, 0, 10) == 0);
    ASSERT(FUN_004474e0(10, 0, 10) == 10);
    ASSERT(FUN_004474e0(-1, 0, 10) == 0);
    ASSERT(FUN_004474e0(11, 0, 10) == 10);
}

TEST(clamp_negative_range) {
    ASSERT(FUN_004474e0(-15, -10, -5) == -10);
    ASSERT(FUN_004474e0(-3, -10, -5) == -5);
    ASSERT(FUN_004474e0(-7, -10, -5) == -7);
}

TEST(clamp_same_min_max) {
    ASSERT(FUN_004474e0(0, 5, 5) == 5);
    ASSERT(FUN_004474e0(10, 5, 5) == 5);
    ASSERT(FUN_004474e0(5, 5, 5) == 5);
}

TEST(clamp_large_values) {
    ASSERT(FUN_004474e0(1000000, 0, 100) == 100);
    ASSERT(FUN_004474e0(-1000000, 0, 100) == 0);
    ASSERT(FUN_004474e0(50000, 0, 100000) == 50000);
}

TEST(clamp_zero_range) {
    ASSERT(FUN_004474e0(-5, 0, 0) == 0);
    ASSERT(FUN_004474e0(5, 0, 0) == 0);
    ASSERT(FUN_004474e0(0, 0, 0) == 0);
}

TEST(clamp_symmetric_range) {
    ASSERT(FUN_004474e0(-15, -10, 10) == -10);
    ASSERT(FUN_004474e0(15, -10, 10) == 10);
    ASSERT(FUN_004474e0(0, -10, 10) == 0);
    ASSERT(FUN_004474e0(5, -10, 10) == 5);
    ASSERT(FUN_004474e0(-5, -10, 10) == -5);
}

TEST(clamp_just_outside) {
    ASSERT(FUN_004474e0(10, 0, 10) == 10);
    ASSERT(FUN_004474e0(11, 0, 10) == 10);
    ASSERT(FUN_004474e0(-1, 0, 10) == 0);
}

TEST(clamp_preserves_type) {
    /* Verify that the function returns the same type as input */
    int result = FUN_004474e0(5, 0, 10);
    ASSERT(result == 5);
    ASSERT(sizeof(result) == sizeof(int));
}

TEST(multiple_clamp_operations) {
    /* Chain multiple clamp operations */
    int value = 50;
    value = FUN_004474e0(value, 0, 100);  /* 50 */
    ASSERT(value == 50);
    value = FUN_004474e0(value, 0, 30);   /* 30 */
    ASSERT(value == 30);
    value = FUN_004474e0(value, 40, 60);  /* 40 */
    ASSERT(value == 40);
}

TEST(clamp_common_ranges) {
    /* Test common game-related ranges */
    /* Health: 0-100 */
    ASSERT(FUN_004474e0(150, 0, 100) == 100);
    ASSERT(FUN_004474e0(-10, 0, 100) == 0);

    /* Level: 1-99 */
    ASSERT(FUN_004474e0(0, 1, 99) == 1);
    ASSERT(FUN_004474e0(100, 1, 99) == 99);
    ASSERT(FUN_004474e0(50, 1, 99) == 50);

    /* Direction: 0-7 */
    ASSERT(FUN_004474e0(-1, 0, 7) == 0);
    ASSERT(FUN_004474e0(8, 0, 7) == 7);
    ASSERT(FUN_004474e0(3, 0, 7) == 3);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Clamp Value Function Unit Tests ===\n\n");

    printf("Boundary Tests:\n");
    RUN_TEST(clamp_below_min);
    RUN_TEST(clamp_above_max);
    RUN_TEST(clamp_within_range);
    RUN_TEST(clamp_at_boundaries);

    printf("\nRange Tests:\n");
    RUN_TEST(clamp_negative_range);
    RUN_TEST(clamp_same_min_max);
    RUN_TEST(clamp_large_values);
    RUN_TEST(clamp_zero_range);
    RUN_TEST(clamp_symmetric_range);

    printf("\nEdge Case Tests:\n");
    RUN_TEST(clamp_just_outside);
    RUN_TEST(clamp_preserves_type);
    RUN_TEST(multiple_clamp_operations);

    printf("\nApplication Tests:\n");
    RUN_TEST(clamp_common_ranges);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
