/*
 * Stone Age Client - Distance Squared Function Unit Tests
 * Tests for FUN_00447370 (calculate squared distance between two points)
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

#define ASSERT_FLOAT_EQ(a, b) ASSERT(fabsf((a) - (b)) < EPSILON)

/*
 * FUN_00447370 - Distance Squared
 */
static float FUN_00447370(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return dx * dx + dy * dy;
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(same_point) {
    float dist = FUN_00447370(0.0f, 0.0f, 0.0f, 0.0f);
    ASSERT_FLOAT_EQ(dist, 0.0f);
}

TEST(unit_distance_x) {
    float dist = FUN_00447370(0.0f, 0.0f, 1.0f, 0.0f);
    ASSERT_FLOAT_EQ(dist, 1.0f);
}

TEST(unit_distance_y) {
    float dist = FUN_00447370(0.0f, 0.0f, 0.0f, 1.0f);
    ASSERT_FLOAT_EQ(dist, 1.0f);
}

TEST(diagonal_distance) {
    float dist = FUN_00447370(0.0f, 0.0f, 1.0f, 1.0f);
    ASSERT_FLOAT_EQ(dist, 2.0f);
}

TEST(negative_coordinates) {
    float dist = FUN_00447370(-1.0f, -1.0f, 1.0f, 1.0f);
    ASSERT_FLOAT_EQ(dist, 8.0f);  /* (2)^2 + (2)^2 = 8 */
}

TEST(large_values) {
    float dist = FUN_00447370(0.0f, 0.0f, 100.0f, 100.0f);
    ASSERT_FLOAT_EQ(dist, 20000.0f);  /* 100^2 + 100^2 = 20000 */
}

TEST(small_values) {
    float dist = FUN_00447370(0.0f, 0.0f, 0.001f, 0.001f);
    ASSERT_FLOAT_EQ(dist, 0.000002f);
}

TEST(asymmetric) {
    float dist = FUN_00447370(0.0f, 0.0f, 3.0f, 4.0f);
    ASSERT_FLOAT_EQ(dist, 25.0f);  /* 3^2 + 4^2 = 25 */
}

TEST(reverse_direction) {
    float dist1 = FUN_00447370(0.0f, 0.0f, 5.0f, 5.0f);
    float dist2 = FUN_00447370(5.0f, 5.0f, 0.0f, 0.0f);
    ASSERT_FLOAT_EQ(dist1, dist2);
}

TEST(floating_point_precision) {
    float dist = FUN_00447370(0.1f, 0.1f, 0.2f, 0.2f);
    float expected = 0.02f;  /* 0.1^2 + 0.1^2 = 0.02 */
    ASSERT(fabsf(dist - expected) < EPSILON);
}

TEST(distance_3_4_5) {
    /* Classic 3-4-5 triangle: 3^2 + 4^2 = 5^2 = 25 */
    float dist = FUN_00447370(0.0f, 0.0f, 3.0f, 4.0f);
    ASSERT_FLOAT_EQ(dist, 25.0f);
}

TEST(distance_5_12_13) {
    /* Classic 5-12-13 triangle: 5^2 + 12^2 = 13^2 = 169 */
    float dist = FUN_00447370(0.0f, 0.0f, 5.0f, 12.0f);
    ASSERT_FLOAT_EQ(dist, 169.0f);
}

TEST(arbitrary_points) {
    float dist = FUN_00447370(10.0f, 20.0f, 13.0f, 24.0f);
    /* dx = 3, dy = 4, dist = 9 + 16 = 25 */
    ASSERT_FLOAT_EQ(dist, 25.0f);
}

TEST(mixed_signs) {
    float dist = FUN_00447370(-5.0f, 3.0f, 2.0f, -1.0f);
    /* dx = 7, dy = -4, dist = 49 + 16 = 65 */
    ASSERT_FLOAT_EQ(dist, 65.0f);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Distance Squared Function Unit Tests ===\n\n");

    printf("Basic Tests:\n");
    RUN_TEST(same_point);
    RUN_TEST(unit_distance_x);
    RUN_TEST(unit_distance_y);
    RUN_TEST(diagonal_distance);

    printf("\nCoordinate Tests:\n");
    RUN_TEST(negative_coordinates);
    RUN_TEST(large_values);
    RUN_TEST(small_values);
    RUN_TEST(asymmetric);

    printf("\nMathematical Tests:\n");
    RUN_TEST(reverse_direction);
    RUN_TEST(floating_point_precision);
    RUN_TEST(distance_3_4_5);
    RUN_TEST(distance_5_12_13);

    printf("\nEdge Case Tests:\n");
    RUN_TEST(arbitrary_points);
    RUN_TEST(mixed_signs);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
