/*
 * Stone Age Client - Normalize Angle Function Unit Tests
 * Tests for FUN_004470d0 (normalize direction angle to [0, 2*PI))
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

#define PI 3.14159265358979323846f
#define TWO_PI (2.0f * PI)
#define EPSILON 0.0001f

static f32 test_two_pi = TWO_PI;
static f32 test_upper_bound = TWO_PI;
static f32 test_lower_bound = 0.0f;

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
 * FUN_004470d0 - Normalize Direction Angle
 */
static void FUN_004470d0(float* angle) {
    while (*angle >= test_upper_bound) {
        *angle -= test_two_pi;
    }
    while (*angle < test_lower_bound) {
        *angle += test_two_pi;
    }
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(angle_zero) {
    float angle = 0.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 0.0f);
}

TEST(angle_pi) {
    float angle = PI;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, PI);
}

TEST(angle_half_pi) {
    float angle = PI / 2.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, PI / 2.0f);
}

TEST(angle_two_pi) {
    float angle = TWO_PI;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 0.0f);
}

TEST(angle_just_over_two_pi) {
    float angle = TWO_PI + 0.1f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 0.1f);
}

TEST(angle_negative) {
    float angle = -0.5f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, TWO_PI - 0.5f);
}

TEST(angle_negative_pi) {
    float angle = -PI;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, PI);
}

TEST(angle_large_positive) {
    float angle = 10.0f * TWO_PI + PI / 4.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, PI / 4.0f);
}

TEST(angle_large_negative) {
    float angle = -5.0f * TWO_PI + PI / 3.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, PI / 3.0f);
}

TEST(angle_multiple_rotations) {
    float angle = 10.0f * TWO_PI + 1.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 1.0f);
}

TEST(angle_negative_multiple_rotations) {
    float angle = -10.0f * TWO_PI + 1.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 1.0f);
}

TEST(angle_just_under_two_pi) {
    float angle = TWO_PI - 0.001f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, TWO_PI - 0.001f);
}

TEST(angle_very_negative) {
    float angle = -100.0f;
    FUN_004470d0(&angle);
    ASSERT(angle >= 0.0f);
    ASSERT(angle < TWO_PI);
}

TEST(angle_very_positive) {
    float angle = 100.0f;
    FUN_004470d0(&angle);
    ASSERT(angle >= 0.0f);
    ASSERT(angle < TWO_PI);
}

TEST(result_in_range) {
    /* Test various angles all result in [0, 2*PI) */
    float test_angles[] = {-100.0f, -PI, -0.1f, 0.0f, PI/2, PI, 3*PI/2, TWO_PI, 100.0f};
    int num_tests = sizeof(test_angles) / sizeof(test_angles[0]);
    int i;

    for (i = 0; i < num_tests; i++) {
        float angle = test_angles[i];
        FUN_004470d0(&angle);
        ASSERT(angle >= 0.0f);
        ASSERT(angle < TWO_PI);
    }
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Normalize Angle Function Unit Tests ===\n\n");

    printf("Basic Angle Tests:\n");
    RUN_TEST(angle_zero);
    RUN_TEST(angle_pi);
    RUN_TEST(angle_half_pi);
    RUN_TEST(angle_two_pi);

    printf("\nOverflow Tests:\n");
    RUN_TEST(angle_just_over_two_pi);
    RUN_TEST(angle_negative);
    RUN_TEST(angle_negative_pi);

    printf("\nLarge Angle Tests:\n");
    RUN_TEST(angle_large_positive);
    RUN_TEST(angle_large_negative);
    RUN_TEST(angle_multiple_rotations);
    RUN_TEST(angle_negative_multiple_rotations);

    printf("\nEdge Case Tests:\n");
    RUN_TEST(angle_just_under_two_pi);
    RUN_TEST(angle_very_negative);
    RUN_TEST(angle_very_positive);
    RUN_TEST(result_in_range);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
