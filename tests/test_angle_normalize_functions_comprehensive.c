/*
 * Stone Age Client - Angle Normalize Functions Unit Tests
 * Tests for FUN_004470d0 - Normalize angle to [0, 360)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;
typedef float f32;

/* Constants */
#define PI 3.14159265358979323846f
#define TWO_PI (2.0f * PI)
#define DEG_360 360.0f
#define DEG_180 180.0f
#define EPSILON 0.0001f

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

#define ASSERT_FLOAT_EQ(a, b) ASSERT(fabsf((a) - (b)) < EPSILON)

/*
 * FUN_004470d0 - Normalize angle to [0, 360) degrees
 * Wraps angle values to be within the range [0, 360)
 */
static void FUN_004470d0(float *param_1) {
    /* Normalize: while angle >= 360, subtract 360 */
    while (DEG_360 <= *param_1) {
        *param_1 = *param_1 - DEG_360;
    }
    /* Normalize: while angle < 0, add 360 */
    while (*param_1 < 0.0f) {
        *param_1 = *param_1 + DEG_360;
    }
}

/* Helper: check if float is in range [0, 360) */
static int is_normalized(float angle) {
    return (angle >= 0.0f) && (angle < DEG_360);
}

/* ========================================
 * Test Cases
 * ======================================== */

/* Basic normalization tests */
TEST(normalize_zero) {
    float angle = 0.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 0.0f);
}

TEST(normalize_90) {
    float angle = 90.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 90.0f);
}

TEST(normalize_180) {
    float angle = 180.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 180.0f);
}

TEST(normalize_270) {
    float angle = 270.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 270.0f);
}

TEST(normalize_359) {
    float angle = 359.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 359.0f);
}

/* Overflow tests */
TEST(normalize_360) {
    float angle = 360.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 0.0f);
}

TEST(normalize_450) {
    float angle = 450.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 90.0f);
}

TEST(normalize_720) {
    float angle = 720.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 0.0f);
}

TEST(normalize_900) {
    float angle = 900.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 180.0f);
}

/* Negative angle tests */
TEST(normalize_negative_90) {
    float angle = -90.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 270.0f);
}

TEST(normalize_negative_180) {
    float angle = -180.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 180.0f);
}

TEST(normalize_negative_360) {
    float angle = -360.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 0.0f);
}

TEST(normalize_negative_450) {
    float angle = -450.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 270.0f);
}

TEST(normalize_negative_720) {
    float angle = -720.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 0.0f);
}

/* Fractional angle tests */
TEST(normalize_45_5) {
    float angle = 45.5f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 45.5f);
}

TEST(normalize_405_5) {
    float angle = 405.5f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 45.5f);
}

TEST(normalize_negative_45_5) {
    float angle = -45.5f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 314.5f);
}

/* Boundary tests */
TEST(normalize_near_360) {
    float angle = 359.999f;
    FUN_004470d0(&angle);
    ASSERT(is_normalized(angle));
    ASSERT_FLOAT_EQ(angle, 359.999f);
}

TEST(normalize_just_over_360) {
    float angle = 360.001f;
    FUN_004470d0(&angle);
    ASSERT(is_normalized(angle));
    ASSERT_FLOAT_EQ(angle, 0.001f);
}

/* Large value tests */
TEST(normalize_large_positive) {
    float angle = 3600.0f;  /* 10 * 360 */
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 0.0f);
}

TEST(normalize_large_negative) {
    float angle = -3600.0f;  /* -10 * 360 */
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 0.0f);
}

/* Verification that result is always normalized */
TEST(result_always_normalized_positive) {
    for (float angle = 0.0f; angle < 1000.0f; angle += 45.0f) {
        float test_angle = angle;
        FUN_004470d0(&test_angle);
        ASSERT(is_normalized(test_angle));
    }
}

TEST(result_always_normalized_negative) {
    for (float angle = -1000.0f; angle < 0.0f; angle += 45.0f) {
        float test_angle = angle;
        FUN_004470d0(&test_angle);
        ASSERT(is_normalized(test_angle));
    }
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Angle Normalize Functions Unit Tests ===\n\n");

    printf("Basic Normalization Tests:\n");
    RUN_TEST(normalize_zero);
    RUN_TEST(normalize_90);
    RUN_TEST(normalize_180);
    RUN_TEST(normalize_270);
    RUN_TEST(normalize_359);

    printf("\nOverflow Tests:\n");
    RUN_TEST(normalize_360);
    RUN_TEST(normalize_450);
    RUN_TEST(normalize_720);
    RUN_TEST(normalize_900);

    printf("\nNegative Angle Tests:\n");
    RUN_TEST(normalize_negative_90);
    RUN_TEST(normalize_negative_180);
    RUN_TEST(normalize_negative_360);
    RUN_TEST(normalize_negative_450);
    RUN_TEST(normalize_negative_720);

    printf("\nFractional Angle Tests:\n");
    RUN_TEST(normalize_45_5);
    RUN_TEST(normalize_405_5);
    RUN_TEST(normalize_negative_45_5);

    printf("\nBoundary Tests:\n");
    RUN_TEST(normalize_near_360);
    RUN_TEST(normalize_just_over_360);

    printf("\nLarge Value Tests:\n");
    RUN_TEST(normalize_large_positive);
    RUN_TEST(normalize_large_negative);

    printf("\nVerification Tests:\n");
    RUN_TEST(result_always_normalized_positive);
    RUN_TEST(result_always_normalized_negative);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
