/*
 * Stone Age Client - Linear Interpolation Function Unit Tests
 * Tests for FUN_00447520 (lerp function)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

/* Type definitions */
typedef float f32;

/* ========================================
 * Test Data and Constants
 * ======================================== */

#define EPSILON 0.0001f

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

#define ASSERT_FLOAT_NEAR(a, b) ASSERT(fabsf((a) - (b)) < EPSILON)

/*
 * FUN_00447520 - Linear Interpolation
 */
static float FUN_00447520(float a, float b, float t) {
    return a + (b - a) * t;
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(lerp_at_zero) {
    /* t=0 should return start value */
    ASSERT_FLOAT_NEAR(FUN_00447520(0.0f, 10.0f, 0.0f), 0.0f);
    ASSERT_FLOAT_NEAR(FUN_00447520(5.0f, 15.0f, 0.0f), 5.0f);
    ASSERT_FLOAT_NEAR(FUN_00447520(-5.0f, 5.0f, 0.0f), -5.0f);
}

TEST(lerp_at_one) {
    /* t=1 should return end value */
    ASSERT_FLOAT_NEAR(FUN_00447520(0.0f, 10.0f, 1.0f), 10.0f);
    ASSERT_FLOAT_NEAR(FUN_00447520(5.0f, 15.0f, 1.0f), 15.0f);
    ASSERT_FLOAT_NEAR(FUN_00447520(-5.0f, 5.0f, 1.0f), 5.0f);
}

TEST(lerp_at_half) {
    /* t=0.5 should return midpoint */
    ASSERT_FLOAT_NEAR(FUN_00447520(0.0f, 10.0f, 0.5f), 5.0f);
    ASSERT_FLOAT_NEAR(FUN_00447520(0.0f, 100.0f, 0.5f), 50.0f);
    ASSERT_FLOAT_NEAR(FUN_00447520(-10.0f, 10.0f, 0.5f), 0.0f);
}

TEST(lerp_quarter) {
    /* t=0.25 should return quarter point */
    ASSERT_FLOAT_NEAR(FUN_00447520(0.0f, 100.0f, 0.25f), 25.0f);
    ASSERT_FLOAT_NEAR(FUN_00447520(0.0f, 4.0f, 0.25f), 1.0f);
}

TEST(lerp_three_quarter) {
    /* t=0.75 should return three-quarter point */
    ASSERT_FLOAT_NEAR(FUN_00447520(0.0f, 100.0f, 0.75f), 75.0f);
    ASSERT_FLOAT_NEAR(FUN_00447520(0.0f, 4.0f, 0.75f), 3.0f);
}

TEST(lerp_negative_values) {
    ASSERT_FLOAT_NEAR(FUN_00447520(-10.0f, -5.0f, 0.5f), -7.5f);
    ASSERT_FLOAT_NEAR(FUN_00447520(-100.0f, 100.0f, 0.5f), 0.0f);
    ASSERT_FLOAT_NEAR(FUN_00447520(10.0f, -10.0f, 0.5f), 0.0f);
}

TEST(lerp_same_values) {
    /* Lerp between same values should return that value */
    ASSERT_FLOAT_NEAR(FUN_00447520(5.0f, 5.0f, 0.0f), 5.0f);
    ASSERT_FLOAT_NEAR(FUN_00447520(5.0f, 5.0f, 0.5f), 5.0f);
    ASSERT_FLOAT_NEAR(FUN_00447520(5.0f, 5.0f, 1.0f), 5.0f);
}

TEST(lerp_extrapolate_beyond_one) {
    /* t > 1 extrapolates beyond end */
    ASSERT_FLOAT_NEAR(FUN_00447520(0.0f, 10.0f, 1.5f), 15.0f);
    ASSERT_FLOAT_NEAR(FUN_00447520(0.0f, 10.0f, 2.0f), 20.0f);
}

TEST(lerp_extrapolate_negative) {
    /* t < 0 extrapolates before start */
    ASSERT_FLOAT_NEAR(FUN_00447520(0.0f, 10.0f, -0.5f), -5.0f);
    ASSERT_FLOAT_NEAR(FUN_00447520(10.0f, 20.0f, -1.0f), 0.0f);
}

TEST(lerp_small_values) {
    ASSERT_FLOAT_NEAR(FUN_00447520(0.0f, 0.001f, 0.5f), 0.0005f);
    ASSERT_FLOAT_NEAR(FUN_00447520(0.001f, 0.002f, 0.5f), 0.0015f);
}

TEST(lerp_large_values) {
    ASSERT_FLOAT_NEAR(FUN_00447520(0.0f, 10000.0f, 0.1f), 1000.0f);
    ASSERT_FLOAT_NEAR(FUN_00447520(-10000.0f, 10000.0f, 0.5f), 0.0f);
}

TEST(lerp_precision) {
    /* Test that precision is maintained */
    float result = FUN_00447520(0.0f, 1.0f, 0.333333f);
    ASSERT(result > 0.33f && result < 0.34f);
}

TEST(lerp_symmetry) {
    /* Lerp from a to b at t should equal lerp from b to a at (1-t) */
    float r1 = FUN_00447520(0.0f, 10.0f, 0.3f);
    float r2 = FUN_00447520(10.0f, 0.0f, 0.7f);
    ASSERT_FLOAT_NEAR(r1, r2);
}

TEST(lerp_additivity) {
    /* Test that lerp is linear (homogeneity property) */
    /* lerp(a, b, k*t) = lerp(a, lerp(a, b, k), t) for certain cases */
    float a = 0.0f, b = 10.0f;

    /* For t=0.5, midpoint should be exactly half */
    float mid = FUN_00447520(a, b, 0.5f);
    ASSERT_FLOAT_NEAR(mid, 5.0f);

    /* Quarter point */
    float quarter = FUN_00447520(a, b, 0.25f);
    ASSERT_FLOAT_NEAR(quarter, 2.5f);

    /* Three quarter point */
    float three_quarter = FUN_00447520(a, b, 0.75f);
    ASSERT_FLOAT_NEAR(three_quarter, 7.5f);
}

TEST(lerp_animation_use_case) {
    /* Simulate animation frame interpolation */
    float start_pos = 100.0f;
    float end_pos = 200.0f;
    float positions[11];
    int i;

    for (i = 0; i <= 10; i++) {
        positions[i] = FUN_00447520(start_pos, end_pos, i / 10.0f);
    }

    ASSERT_FLOAT_NEAR(positions[0], 100.0f);
    ASSERT_FLOAT_NEAR(positions[5], 150.0f);
    ASSERT_FLOAT_NEAR(positions[10], 200.0f);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Linear Interpolation Function Unit Tests ===\n\n");

    printf("Standard Interpolation Tests:\n");
    RUN_TEST(lerp_at_zero);
    RUN_TEST(lerp_at_one);
    RUN_TEST(lerp_at_half);
    RUN_TEST(lerp_quarter);
    RUN_TEST(lerp_three_quarter);

    printf("\nEdge Value Tests:\n");
    RUN_TEST(lerp_negative_values);
    RUN_TEST(lerp_same_values);
    RUN_TEST(lerp_small_values);
    RUN_TEST(lerp_large_values);

    printf("\nExtrapolation Tests:\n");
    RUN_TEST(lerp_extrapolate_beyond_one);
    RUN_TEST(lerp_extrapolate_negative);

    printf("\nMathematical Property Tests:\n");
    RUN_TEST(lerp_precision);
    RUN_TEST(lerp_symmetry);
    RUN_TEST(lerp_additivity);

    printf("\nApplication Tests:\n");
    RUN_TEST(lerp_animation_use_case);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
