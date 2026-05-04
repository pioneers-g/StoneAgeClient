/*
 * Stone Age Client - Isometric Coordinate Transformation Unit Tests
 * Tests for FUN_00446e40 (world to screen coordinate conversion)
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

static u32 DAT_004bb424 = 0;     /* Offset X */
static u32 DAT_004bb428 = 0;     /* Offset Y */
static float _DAT_0049c3ec = 0.5f;  /* Isometric scale */
static float _DAT_04582998 = 0.0f;  /* Screen offset X */
static float _DAT_04582994 = 0.0f;  /* Screen offset Y */

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
 * FUN_00446e40 - Isometric Coordinate Transformation
 */
static void FUN_00446e40(float world_x, float world_y, float* screen_x, float* screen_y) {
    float offset_x = (float)DAT_004bb424;
    float iso_factor = (world_y - (float)DAT_004bb428) * _DAT_0049c3ec;

    *screen_x = ((world_x - offset_x) - iso_factor) + _DAT_04582998;
    *screen_y = iso_factor + (world_x - offset_x) + _DAT_04582994;
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(transform_zero_zero) {
    float screen_x, screen_y;
    FUN_00446e40(0.0f, 0.0f, &screen_x, &screen_y);
    ASSERT_FLOAT_EQ(screen_x, 0.0f);
    ASSERT_FLOAT_EQ(screen_y, 0.0f);
}

TEST(transform_positive_x) {
    float screen_x, screen_y;
    FUN_00446e40(100.0f, 0.0f, &screen_x, &screen_y);
    /* screen_x = (100 - 0) - 0 + 0 = 100 */
    /* screen_y = 0 + (100 - 0) + 0 = 100 */
    ASSERT_FLOAT_EQ(screen_x, 100.0f);
    ASSERT_FLOAT_EQ(screen_y, 100.0f);
}

TEST(transform_positive_y) {
    float screen_x, screen_y;
    FUN_00446e40(0.0f, 100.0f, &screen_x, &screen_y);
    /* screen_x = (0 - 0) - 50 + 0 = -50 */
    /* screen_y = 50 + (0 - 0) + 0 = 50 */
    ASSERT_FLOAT_EQ(screen_x, -50.0f);
    ASSERT_FLOAT_EQ(screen_y, 50.0f);
}

TEST(transform_equal_x_y) {
    float screen_x, screen_y;
    FUN_00446e40(50.0f, 50.0f, &screen_x, &screen_y);
    /* screen_x = (50 - 0) - 25 + 0 = 25 */
    /* screen_y = 25 + (50 - 0) + 0 = 75 */
    ASSERT_FLOAT_EQ(screen_x, 25.0f);
    ASSERT_FLOAT_EQ(screen_y, 75.0f);
}

TEST(transform_negative_x) {
    float screen_x, screen_y;
    FUN_00446e40(-50.0f, 0.0f, &screen_x, &screen_y);
    /* screen_x = (-50 - 0) - 0 + 0 = -50 */
    /* screen_y = 0 + (-50 - 0) + 0 = -50 */
    ASSERT_FLOAT_EQ(screen_x, -50.0f);
    ASSERT_FLOAT_EQ(screen_y, -50.0f);
}

TEST(transform_negative_y) {
    float screen_x, screen_y;
    FUN_00446e40(0.0f, -50.0f, &screen_x, &screen_y);
    /* screen_x = (0 - 0) - (-25) + 0 = 25 */
    /* screen_y = -25 + (0 - 0) + 0 = -25 */
    ASSERT_FLOAT_EQ(screen_x, 25.0f);
    ASSERT_FLOAT_EQ(screen_y, -25.0f);
}

TEST(transform_with_offsets) {
    DAT_004bb424 = 10;
    DAT_004bb428 = 20;

    float screen_x, screen_y;
    FUN_00446e40(100.0f, 100.0f, &screen_x, &screen_y);
    /* screen_x = (100 - 10) - 40 + 0 = 50 */
    /* screen_y = 40 + (100 - 10) + 0 = 130 */
    ASSERT_FLOAT_EQ(screen_x, 50.0f);
    ASSERT_FLOAT_EQ(screen_y, 130.0f);

    DAT_004bb424 = 0;
    DAT_004bb428 = 0;
}

TEST(transform_with_screen_offsets) {
    _DAT_04582998 = 100.0f;
    _DAT_04582994 = 200.0f;

    float screen_x, screen_y;
    FUN_00446e40(0.0f, 0.0f, &screen_x, &screen_y);
    /* screen_x = (0 - 0) - 0 + 100 = 100 */
    /* screen_y = 0 + (0 - 0) + 200 = 200 */
    ASSERT_FLOAT_EQ(screen_x, 100.0f);
    ASSERT_FLOAT_EQ(screen_y, 200.0f);

    _DAT_04582998 = 0.0f;
    _DAT_04582994 = 0.0f;
}

TEST(transform_large_values) {
    float screen_x, screen_y;
    FUN_00446e40(1000.0f, 1000.0f, &screen_x, &screen_y);
    /* screen_x = 1000 - 500 = 500 */
    /* screen_y = 500 + 1000 = 1500 */
    ASSERT_FLOAT_EQ(screen_x, 500.0f);
    ASSERT_FLOAT_EQ(screen_y, 1500.0f);
}

TEST(transform_fractional_values) {
    float screen_x, screen_y;
    FUN_00446e40(33.5f, 66.25f, &screen_x, &screen_y);
    /* screen_x = 33.5 - 33.125 = 0.375 */
    /* screen_y = 33.125 + 33.5 = 66.625 */
    ASSERT_FLOAT_EQ(screen_x, 0.375f);
    ASSERT_FLOAT_EQ(screen_y, 66.625f);
}

TEST(transform_isometric_property) {
    /* Moving along world X adds to both screen X and screen Y equally */
    /* Moving along world Y subtracts from screen X and adds to screen Y */

    float base_x, base_y;
    FUN_00446e40(100.0f, 100.0f, &base_x, &base_y);

    float moved_x, moved_y;
    FUN_00446e40(110.0f, 100.0f, &moved_x, &moved_y);

    /* Moving 10 in world X should add 10 to both screen X and Y */
    ASSERT_FLOAT_EQ(moved_x - base_x, 10.0f);
    ASSERT_FLOAT_EQ(moved_y - base_y, 10.0f);
}

TEST(transform_y_affects_x_negatively) {
    float base_x, base_y;
    FUN_00446e40(100.0f, 100.0f, &base_x, &base_y);

    float moved_x, moved_y;
    FUN_00446e40(100.0f, 110.0f, &moved_x, &moved_y);

    /* Moving 10 in world Y should:
       - subtract 5 from screen X (iso_factor increases by 5)
       - add 5 to screen Y (iso_factor) */
    ASSERT_FLOAT_EQ(moved_x - base_x, -5.0f);
    ASSERT_FLOAT_EQ(moved_y - base_y, 5.0f);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Isometric Coordinate Transformation Unit Tests ===\n\n");

    printf("Basic Transform Tests:\n");
    RUN_TEST(transform_zero_zero);
    RUN_TEST(transform_positive_x);
    RUN_TEST(transform_positive_y);
    RUN_TEST(transform_equal_x_y);
    RUN_TEST(transform_negative_x);
    RUN_TEST(transform_negative_y);

    printf("\nOffset Tests:\n");
    RUN_TEST(transform_with_offsets);
    RUN_TEST(transform_with_screen_offsets);

    printf("\nValue Range Tests:\n");
    RUN_TEST(transform_large_values);
    RUN_TEST(transform_fractional_values);

    printf("\nIsometric Property Tests:\n");
    RUN_TEST(transform_isometric_property);
    RUN_TEST(transform_y_affects_x_negatively);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
