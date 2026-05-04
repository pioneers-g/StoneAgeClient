/*
 * Stone Age Client - Direction Offset Function Unit Tests
 * Tests for FUN_00444fb0 (apply direction offset to coordinates)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef int s32;

/* ========================================
 * Test Data and Mock Implementations
 * ======================================== */

static s32 test_direction = -1;

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
 * FUN_00444fb0 - Apply Direction Offset
 */
static void FUN_00444fb0(int* x, int* y) {
    if (test_direction < 0) return;

    switch (test_direction) {
        case 0:  /* North */
            *y -= 2;
            break;
        case 1:  /* Northeast */
            *x += 1;
            *y -= 1;
            break;
        case 2:  /* East */
            *x += 2;
            break;
        case 3:  /* Southeast */
            *x += 1;
            *y += 1;
            break;
        case 4:  /* South */
            *y += 2;
            break;
        case 5:  /* Southwest */
            *x -= 1;
            *y += 1;
            break;
        case 6:  /* West */
            *x -= 2;
            break;
        case 7:  /* Northwest */
            *x -= 1;
            *y -= 1;
            break;
    }
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(direction_north) {
    int x = 10, y = 10;
    test_direction = 0;

    FUN_00444fb0(&x, &y);

    ASSERT(x == 10);
    ASSERT(y == 8);
}

TEST(direction_northeast) {
    int x = 10, y = 10;
    test_direction = 1;

    FUN_00444fb0(&x, &y);

    ASSERT(x == 11);
    ASSERT(y == 9);
}

TEST(direction_east) {
    int x = 10, y = 10;
    test_direction = 2;

    FUN_00444fb0(&x, &y);

    ASSERT(x == 12);
    ASSERT(y == 10);
}

TEST(direction_southeast) {
    int x = 10, y = 10;
    test_direction = 3;

    FUN_00444fb0(&x, &y);

    ASSERT(x == 11);
    ASSERT(y == 11);
}

TEST(direction_south) {
    int x = 10, y = 10;
    test_direction = 4;

    FUN_00444fb0(&x, &y);

    ASSERT(x == 10);
    ASSERT(y == 12);
}

TEST(direction_southwest) {
    int x = 10, y = 10;
    test_direction = 5;

    FUN_00444fb0(&x, &y);

    ASSERT(x == 9);
    ASSERT(y == 11);
}

TEST(direction_west) {
    int x = 10, y = 10;
    test_direction = 6;

    FUN_00444fb0(&x, &y);

    ASSERT(x == 8);
    ASSERT(y == 10);
}

TEST(direction_northwest) {
    int x = 10, y = 10;
    test_direction = 7;

    FUN_00444fb0(&x, &y);

    ASSERT(x == 9);
    ASSERT(y == 9);
}

TEST(direction_negative_no_change) {
    int x = 10, y = 10;
    test_direction = -1;

    FUN_00444fb0(&x, &y);

    ASSERT(x == 10);
    ASSERT(y == 10);
}

TEST(direction_invalid_no_change) {
    int x = 10, y = 10;
    test_direction = 8;  /* Invalid direction */

    FUN_00444fb0(&x, &y);

    /* Out of range directions should have no effect */
    ASSERT(x == 10);
    ASSERT(y == 10);
}

TEST(zero_coordinates) {
    int x = 0, y = 0;
    test_direction = 0;  /* North */

    FUN_00444fb0(&x, &y);

    ASSERT(x == 0);
    ASSERT(y == -2);
}

TEST(negative_coordinates) {
    int x = -5, y = -5;
    test_direction = 4;  /* South */

    FUN_00444fb0(&x, &y);

    ASSERT(x == -5);
    ASSERT(y == -3);
}

TEST(multiple_applications) {
    int x = 0, y = 0;
    test_direction = 2;  /* East */

    FUN_00444fb0(&x, &y);
    FUN_00444fb0(&x, &y);
    FUN_00444fb0(&x, &y);

    ASSERT(x == 6);
    ASSERT(y == 0);
}

TEST(all_directions_circle) {
    /* Start at origin, go in circle - should end up back at origin */
    int x = 0, y = 0;

    test_direction = 0; FUN_00444fb0(&x, &y);  /* North: (0, -2) */
    test_direction = 4; FUN_00444fb0(&x, &y);  /* South: (0, 0) */
    test_direction = 2; FUN_00444fb0(&x, &y);  /* East: (2, 0) */
    test_direction = 6; FUN_00444fb0(&x, &y);  /* West: (0, 0) */

    ASSERT(x == 0);
    ASSERT(y == 0);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Direction Offset Function Unit Tests ===\n\n");

    printf("Direction Tests (0-7):\n");
    RUN_TEST(direction_north);
    RUN_TEST(direction_northeast);
    RUN_TEST(direction_east);
    RUN_TEST(direction_southeast);
    RUN_TEST(direction_south);
    RUN_TEST(direction_southwest);
    RUN_TEST(direction_west);
    RUN_TEST(direction_northwest);

    printf("\nEdge Case Tests:\n");
    RUN_TEST(direction_negative_no_change);
    RUN_TEST(direction_invalid_no_change);
    RUN_TEST(zero_coordinates);
    RUN_TEST(negative_coordinates);

    printf("\nMultiple Application Tests:\n");
    RUN_TEST(multiple_applications);
    RUN_TEST(all_directions_circle);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
