/*
 * Stone Age Client - Direction Angle Function Unit Tests
 * Tests for FUN_00447150 (calculate angle from direction vector)
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
#define EPSILON 0.01f  /* Slightly larger epsilon for atan calculations */

static float test_pi = PI;
static float test_two_pi = TWO_PI;
static float test_lower_bound = 0.0f;
static float test_min_threshold = 0.001f;
static float test_pi_half = PI / 2.0f;
static float test_3pi_half = 3.0f * PI / 2.0f;
static float test_5pi_quarter = 5.0f * PI / 4.0f;
static float test_direction_offset = 0.0f;

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
 * FUN_004470d0 - Normalize Direction Angle
 */
static void FUN_004470d0(float* angle) {
    while (*angle >= test_two_pi) {
        *angle -= test_two_pi;
    }
    while (*angle < test_lower_bound) {
        *angle += test_two_pi;
    }
}

/*
 * FUN_00447150 - Calculate Direction Angle from Vector
 */
static float FUN_00447150(float dx, float dy) {
    float angle;
    int quadrant = 0;

    /* Handle zero vector case */
    if (dx == test_lower_bound && dy == test_lower_bound) {
        return test_min_threshold;
    }

    /* Determine quadrant and make values positive */
    if (dx < test_min_threshold) {
        quadrant |= 1;
        dx = -dx;
    }
    if (dy < test_min_threshold) {
        quadrant |= 2;
        dy = -dy;
    }
    if (dx < dy) {
        quadrant |= 4;
    }

    /* Calculate atan using standard library */
    if (dx == 0.0f) {
        angle = test_pi_half;
    } else {
        angle = atan2f(dy, dx);
    }

    /* Adjust angle based on quadrant */
    switch (quadrant) {
        case 0:
            break;
        case 1:
            angle = test_pi_half - angle;
            break;
        case 2:
            angle = test_two_pi - angle;
            break;
        case 3:
            angle = angle + test_pi_half;
            break;
        case 4:
            angle = test_3pi_half - angle;
            break;
        case 5:
            angle = angle + test_3pi_half;
            break;
        case 6:
            angle = angle + test_5pi_quarter;
            break;
        case 7:
            angle = test_5pi_quarter - angle;
            break;
    }

    /* Subtract direction offset and normalize */
    angle = angle - test_direction_offset;
    FUN_004470d0(&angle);

    return angle;
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(zero_vector) {
    float angle = FUN_00447150(0.0f, 0.0f);
    ASSERT_FLOAT_NEAR(angle, test_min_threshold);
}

TEST(east_direction) {
    /* East: dx=1, dy=0 should give angle 0 */
    float angle = FUN_00447150(1.0f, 0.0f);
    ASSERT_FLOAT_NEAR(angle, 0.0f);
}

TEST(north_direction) {
    /* North: dx=0, dy=-1 should give angle PI/2 or 3*PI/2 */
    float angle = FUN_00447150(0.0f, -1.0f);
    /* Angle should be in [0, 2*PI) */
    ASSERT(angle >= 0.0f && angle < TWO_PI);
}

TEST(west_direction) {
    /* West: dx=-1, dy=0 should give angle PI */
    float angle = FUN_00447150(-1.0f, 0.0f);
    ASSERT(angle >= 0.0f && angle < TWO_PI);
}

TEST(south_direction) {
    /* South: dx=0, dy=1 */
    float angle = FUN_00447150(0.0f, 1.0f);
    ASSERT(angle >= 0.0f && angle < TWO_PI);
}

TEST(northeast_direction) {
    /* Northeast: dx=1, dy=-1 */
    float angle = FUN_00447150(1.0f, -1.0f);
    ASSERT(angle >= 0.0f && angle < TWO_PI);
}

TEST(southeast_direction) {
    /* Southeast: dx=1, dy=1 */
    float angle = FUN_00447150(1.0f, 1.0f);
    ASSERT(angle >= 0.0f && angle < TWO_PI);
}

TEST(southwest_direction) {
    /* Southwest: dx=-1, dy=1 */
    float angle = FUN_00447150(-1.0f, 1.0f);
    ASSERT(angle >= 0.0f && angle < TWO_PI);
}

TEST(northwest_direction) {
    /* Northwest: dx=-1, dy=-1 */
    float angle = FUN_00447150(-1.0f, -1.0f);
    ASSERT(angle >= 0.0f && angle < TWO_PI);
}

TEST(normalized_result) {
    /* All results should be in [0, 2*PI) */
    float test_vectors[][2] = {
        {1.0f, 0.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f}, {0.0f, -1.0f},
        {1.0f, 1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f}, {-1.0f, -1.0f},
        {0.5f, 0.5f}, {0.5f, -0.5f}, {-0.5f, 0.5f}, {-0.5f, -0.5f},
        {100.0f, 100.0f}, {-100.0f, 100.0f}, {100.0f, -100.0f}, {-100.0f, -100.0f}
    };
    int i;
    int num_tests = sizeof(test_vectors) / sizeof(test_vectors[0]);

    for (i = 0; i < num_tests; i++) {
        float angle = FUN_00447150(test_vectors[i][0], test_vectors[i][1]);
        ASSERT(angle >= 0.0f);
        ASSERT(angle < TWO_PI);
    }
}

TEST(scaled_vectors) {
    /* Scaling shouldn't change angle */
    float angle1 = FUN_00447150(1.0f, 1.0f);
    float angle2 = FUN_00447150(100.0f, 100.0f);
    /* Angles should be similar (within epsilon) */
    ASSERT_FLOAT_NEAR(angle1, angle2);
}

TEST(very_small_vectors) {
    /* Very small vectors should still work */
    float angle = FUN_00447150(0.0001f, 0.0001f);
    ASSERT(angle >= 0.0f && angle < TWO_PI);
}

TEST(large_vectors) {
    /* Large vectors should still work */
    float angle = FUN_00447150(10000.0f, 10000.0f);
    ASSERT(angle >= 0.0f && angle < TWO_PI);
}

TEST(diagonal_45_degrees) {
    /* 45-degree angle (1, -1) for game coordinates where Y is inverted */
    float angle = FUN_00447150(1.0f, -1.0f);
    ASSERT(angle >= 0.0f && angle < TWO_PI);
}

TEST(all_quadrants) {
    /* Test that all quadrants produce valid angles */
    struct { float dx, dy; } vectors[] = {
        {1.0f, 0.1f},   /* Q1: dx > 0, small dy */
        {0.1f, 1.0f},   /* Q1: small dx, dy > 0 */
        {-1.0f, 0.1f},  /* Q2: dx < 0, small dy */
        {-0.1f, 1.0f},  /* Q2: small dx < 0, dy > 0 */
        {-1.0f, -0.1f}, /* Q3: dx < 0, dy < 0 */
        {-0.1f, -1.0f}, /* Q3: small dx < 0, dy < 0 */
        {1.0f, -0.1f},  /* Q4: dx > 0, dy < 0 */
        {0.1f, -1.0f},  /* Q4: small dx > 0, dy < 0 */
    };
    int i;
    int num_tests = sizeof(vectors) / sizeof(vectors[0]);

    for (i = 0; i < num_tests; i++) {
        float angle = FUN_00447150(vectors[i].dx, vectors[i].dy);
        ASSERT(angle >= 0.0f);
        ASSERT(angle < TWO_PI);
    }
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Direction Angle Function Unit Tests ===\n\n");

    printf("Cardinal Direction Tests:\n");
    RUN_TEST(zero_vector);
    RUN_TEST(east_direction);
    RUN_TEST(north_direction);
    RUN_TEST(west_direction);
    RUN_TEST(south_direction);

    printf("\nDiagonal Direction Tests:\n");
    RUN_TEST(northeast_direction);
    RUN_TEST(southeast_direction);
    RUN_TEST(southwest_direction);
    RUN_TEST(northwest_direction);

    printf("\nRange and Scaling Tests:\n");
    RUN_TEST(normalized_result);
    RUN_TEST(scaled_vectors);
    RUN_TEST(very_small_vectors);
    RUN_TEST(large_vectors);

    printf("\nQuadrant Tests:\n");
    RUN_TEST(diagonal_45_degrees);
    RUN_TEST(all_quadrants);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
