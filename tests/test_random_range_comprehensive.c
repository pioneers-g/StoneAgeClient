/*
 * Stone Age Client - Random Range Function Unit Tests
 * Tests for FUN_004472e0 (random number in range)
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
 * FUN_004472e0 - Random Number in Range
 */
static int FUN_004472e0(int min_val, int max_val) {
    if (max_val == -1) {
        return 0;
    }
    return rand() % (max_val - min_val + 1) + min_val;
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(returns_zero_for_invalid_max) {
    int val = FUN_004472e0(0, -1);
    ASSERT(val == 0);
}

TEST(single_value_range) {
    /* Range [5, 5] should always return 5 */
    int i;
    for (i = 0; i < 100; i++) {
        int val = FUN_004472e0(5, 5);
        ASSERT(val == 5);
    }
}

TEST(range_zero_to_nine) {
    /* Range [0, 9] should return values 0-9 */
    int seen[10] = {0};
    int i;
    for (i = 0; i < 1000; i++) {
        int val = FUN_004472e0(0, 9);
        ASSERT(val >= 0 && val <= 9);
        seen[val] = 1;
    }

    /* All values should have been seen */
    for (i = 0; i < 10; i++) {
        ASSERT(seen[i] == 1);
    }
}

TEST(range_one_to_ten) {
    /* Range [1, 10] should return values 1-10 */
    int seen[11] = {0};
    int i;
    for (i = 0; i < 1000; i++) {
        int val = FUN_004472e0(1, 10);
        ASSERT(val >= 1 && val <= 10);
        if (val >= 0 && val <= 10) {
            seen[val] = 1;
        }
    }

    /* Values 1-10 should have been seen */
    int j;
    for (j = 1; j <= 10; j++) {
        ASSERT(seen[j] == 1);
    }
}

TEST(negative_range) {
    /* Range [-5, 5] should return values -5 to 5 */
    int i;
    for (i = 0; i < 100; i++) {
        int val = FUN_004472e0(-5, 5);
        ASSERT(val >= -5 && val <= 5);
    }
}

TEST(large_range) {
    /* Range [0, 1000] should return values in that range */
    int i;
    for (i = 0; i < 100; i++) {
        int val = FUN_004472e0(0, 1000);
        ASSERT(val >= 0 && val <= 1000);
    }
}

TEST(distribution_reasonable) {
    /* Test that distribution is roughly uniform */
    int counts[10] = {0};
    int i;
    int samples = 10000;

    for (i = 0; i < samples; i++) {
        int val = FUN_004472e0(0, 9);
        if (val >= 0 && val <= 9) {
            counts[val]++;
        }
    }

    /* Each value should appear roughly 1000 times (±30%) */
    int expected = samples / 10;
    int min_expected = expected * 0.7;
    int max_expected = expected * 1.3;

    for (i = 0; i < 10; i++) {
        ASSERT(counts[i] >= min_expected);
        ASSERT(counts[i] <= max_expected);
    }
}

TEST(deterministic_with_seed) {
    /* Same seed should produce same sequence */
    srand(12345);
    int val1 = FUN_004472e0(0, 100);

    srand(12345);
    int val2 = FUN_004472e0(0, 100);

    ASSERT(val1 == val2);
}

TEST(ascending_range) {
    /* Test that min < max works correctly */
    int min_val = 10;
    int max_val = 20;
    int i;
    for (i = 0; i < 100; i++) {
        int val = FUN_004472e0(min_val, max_val);
        ASSERT(val >= min_val && val <= max_val);
    }
}

TEST(invalid_max_returns_zero) {
    ASSERT(FUN_004472e0(100, -1) == 0);
    ASSERT(FUN_004472e0(-100, -1) == 0);
    ASSERT(FUN_004472e0(0, -1) == 0);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Random Range Function Unit Tests ===\n\n");

    printf("Edge Case Tests:\n");
    RUN_TEST(returns_zero_for_invalid_max);
    RUN_TEST(single_value_range);
    RUN_TEST(invalid_max_returns_zero);

    printf("\nRange Tests:\n");
    RUN_TEST(range_zero_to_nine);
    RUN_TEST(range_one_to_ten);
    RUN_TEST(negative_range);
    RUN_TEST(large_range);
    RUN_TEST(ascending_range);

    printf("\nDistribution Tests:\n");
    RUN_TEST(distribution_reasonable);
    RUN_TEST(deterministic_with_seed);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
