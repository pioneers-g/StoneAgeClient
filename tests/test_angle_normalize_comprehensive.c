/*
 * Stone Age Client - Angle Normalization Function Unit Tests
 * Tests for FUN_004470d0 (normalize angle to [0, 2*PI))
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;
typedef float f32;

/* ========================================
 * Test Data and Constants
 * ======================================== */

#define PI 3.14159265358979323846f
#define TWO_PI (2.0f * PI)

static float _DAT_0049c3d8 = 0.0f;       /* Lower bound */
static float _DAT_0049c3f8 = TWO_PI;     /* Upper bound */
static float _DAT_0049c3f0 = TWO_PI;     /* 2*PI for normalization */

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

#define ASSERT_FLOAT_EQ(a, b) do { \
    float diff = fabsf((a) - (b)); \
    if (diff > 0.0001f) { \
        printf("FAIL at line %d: %s != %s (diff=%f)\n", __LINE__, #a, #b, diff); \
        test_failed++; \
        return; \
    } \
} while(0)

/*
 * FUN_004470d0 - Normalize Angle
 */
static void FUN_004470d0(float* angle) {
    while (*angle >= _DAT_0049c3f8) {
        *angle -= _DAT_0049c3f0;
    }
    while (*angle < _DAT_0049c3d8) {
        *angle += _DAT_0049c3f0;
    }
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(normalize_zero) {
    float angle = 0.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 0.0f);
}

TEST(normalize_pi) {
    float angle = PI;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, PI);
}

TEST(normalize_two_pi) {
    float angle = TWO_PI;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 0.0f);
}

TEST(normalize_just_over_two_pi) {
    float angle = TWO_PI + 0.1f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 0.1f);
}

TEST(normalize_negative_angle) {
    float angle = -0.5f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, TWO_PI - 0.5f);
}

TEST(normalize_large_positive) {
    float angle = TWO_PI * 5.0f + 1.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 1.0f);
}

TEST(normalize_large_negative) {
    float angle = -TWO_PI * 3.0f + 0.5f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 0.5f);
}

TEST(normalize_quarter_pi) {
    float angle = PI / 4.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, PI / 4.0f);
}

TEST(normalize_half_pi) {
    float angle = PI / 2.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, PI / 2.0f);
}

TEST(normalize_three_quarter_pi) {
    float angle = 3.0f * PI / 2.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 3.0f * PI / 2.0f);
}

TEST(normalize_very_small_positive) {
    float angle = 0.0001f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, 0.0001f);
}

TEST(normalize_very_small_negative) {
    float angle = -0.0001f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, TWO_PI - 0.0001f);
}

TEST(normalize_near_two_pi) {
    float angle = TWO_PI - 0.001f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, TWO_PI - 0.001f);
}

TEST(normalize_multiple_rotations_positive) {
    float angle = TWO_PI * 10.0f + PI;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, PI);
}

TEST(normalize_multiple_rotations_negative) {
    float angle = -TWO_PI * 10.0f + PI / 2.0f;
    FUN_004470d0(&angle);
    ASSERT_FLOAT_EQ(angle, PI / 2.0f);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Angle Normalization Function Unit Tests ===\n\n");

    printf("Basic Normalization Tests:\n");
    RUN_TEST(normalize_zero);
    RUN_TEST(normalize_pi);
    RUN_TEST(normalize_two_pi);
    RUN_TEST(normalize_just_over_two_pi);
    RUN_TEST(normalize_negative_angle);

    printf("\nLarge Value Tests:\n");
    RUN_TEST(normalize_large_positive);
    RUN_TEST(normalize_large_negative);

    printf("\nCommon Angle Tests:\n");
    RUN_TEST(normalize_quarter_pi);
    RUN_TEST(normalize_half_pi);
    RUN_TEST(normalize_three_quarter_pi);

    printf("\nEdge Case Tests:\n");
    RUN_TEST(normalize_very_small_positive);
    RUN_TEST(normalize_very_small_negative);
    RUN_TEST(normalize_near_two_pi);

    printf("\nMultiple Rotation Tests:\n");
    RUN_TEST(normalize_multiple_rotations_positive);
    RUN_TEST(normalize_multiple_rotations_negative);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
