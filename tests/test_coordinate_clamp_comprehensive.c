/*
 * Stone Age Client - Coordinate Clamp Function Unit Tests
 * Tests for FUN_004423d0 (clamp coordinates to map bounds)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;
typedef short s16;

/* ========================================
 * Test Data and Constants
 * ======================================== */

#define MAP_WIDTH 100
#define MAP_HEIGHT 80

static u32 DAT_0458119c = MAP_WIDTH;
static u32 DAT_04569b70 = MAP_HEIGHT;

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
 * FUN_004423d0 - Clamp Coordinates to Map Bounds
 */
static void FUN_004423d0(short* left, short* top, short* right, short* bottom) {
    if (*left < 0) {
        *left = 0;
    }
    if (*top < 0) {
        *top = 0;
    }
    if ((u32)*right > DAT_0458119c) {
        *right = (short)DAT_0458119c;
    }
    if ((u32)*bottom > DAT_04569b70) {
        *bottom = (short)DAT_04569b70;
    }
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(clamp_negative_left) {
    short left = -10, top = 5, right = 50, bottom = 50;
    FUN_004423d0(&left, &top, &right, &bottom);
    ASSERT(left == 0);
    ASSERT(top == 5);
    ASSERT(right == 50);
    ASSERT(bottom == 50);
}

TEST(clamp_negative_top) {
    short left = 5, top = -10, right = 50, bottom = 50;
    FUN_004423d0(&left, &top, &right, &bottom);
    ASSERT(left == 5);
    ASSERT(top == 0);
    ASSERT(right == 50);
    ASSERT(bottom == 50);
}

TEST(clamp_exceed_right) {
    short left = 5, top = 5, right = 150, bottom = 50;
    FUN_004423d0(&left, &top, &right, &bottom);
    ASSERT(left == 5);
    ASSERT(top == 5);
    ASSERT(right == MAP_WIDTH);
    ASSERT(bottom == 50);
}

TEST(clamp_exceed_bottom) {
    short left = 5, top = 5, right = 50, bottom = 150;
    FUN_004423d0(&left, &top, &right, &bottom);
    ASSERT(left == 5);
    ASSERT(top == 5);
    ASSERT(right == 50);
    ASSERT(bottom == MAP_HEIGHT);
}

TEST(clamp_all_in_bounds) {
    short left = 10, top = 10, right = 50, bottom = 50;
    FUN_004423d0(&left, &top, &right, &bottom);
    ASSERT(left == 10);
    ASSERT(top == 10);
    ASSERT(right == 50);
    ASSERT(bottom == 50);
}

TEST(clamp_all_out_of_bounds) {
    short left = -10, top = -10, right = 150, bottom = 150;
    FUN_004423d0(&left, &top, &right, &bottom);
    ASSERT(left == 0);
    ASSERT(top == 0);
    ASSERT(right == MAP_WIDTH);
    ASSERT(bottom == MAP_HEIGHT);
}

TEST(clamp_at_boundaries) {
    short left = 0, top = 0, right = MAP_WIDTH, bottom = MAP_HEIGHT;
    FUN_004423d0(&left, &top, &right, &bottom);
    ASSERT(left == 0);
    ASSERT(top == 0);
    ASSERT(right == MAP_WIDTH);
    ASSERT(bottom == MAP_HEIGHT);
}

TEST(clamp_just_outside) {
    short left = -1, top = -1, right = MAP_WIDTH + 1, bottom = MAP_HEIGHT + 1;
    FUN_004423d0(&left, &top, &right, &bottom);
    ASSERT(left == 0);
    ASSERT(top == 0);
    ASSERT(right == MAP_WIDTH);
    ASSERT(bottom == MAP_HEIGHT);
}

TEST(clamp_zero_rect) {
    short left = 0, top = 0, right = 0, bottom = 0;
    FUN_004423d0(&left, &top, &right, &bottom);
    ASSERT(left == 0);
    ASSERT(top == 0);
    ASSERT(right == 0);
    ASSERT(bottom == 0);
}

TEST(clamp_large_negative) {
    short left = -1000, top = -1000, right = 50, bottom = 50;
    FUN_004423d0(&left, &top, &right, &bottom);
    ASSERT(left == 0);
    ASSERT(top == 0);
    ASSERT(right == 50);
    ASSERT(bottom == 50);
}

TEST(clamp_large_positive) {
    short left = 5, top = 5, right = 10000, bottom = 10000;
    FUN_004423d0(&left, &top, &right, &bottom);
    ASSERT(left == 5);
    ASSERT(top == 5);
    ASSERT(right == MAP_WIDTH);
    ASSERT(bottom == MAP_HEIGHT);
}

TEST(clamp_partial_out_of_bounds) {
    /* Left negative, right in bounds */
    short left = -5, top = 10, right = 50, bottom = 50;
    FUN_004423d0(&left, &top, &right, &bottom);
    ASSERT(left == 0);
    ASSERT(top == 10);
    ASSERT(right == 50);
    ASSERT(bottom == 50);

    /* Left in bounds, right exceeds */
    left = 10; top = 10; right = MAP_WIDTH + 10; bottom = 50;
    FUN_004423d0(&left, &top, &right, &bottom);
    ASSERT(left == 10);
    ASSERT(right == MAP_WIDTH);
}

TEST(clamp_preserves_valid_rect) {
    /* A valid rectangle should remain unchanged */
    short left = 10, top = 10, right = 90, bottom = 70;
    short orig_left = left, orig_top = top, orig_right = right, orig_bottom = bottom;
    FUN_004423d0(&left, &top, &right, &bottom);
    ASSERT(left == orig_left);
    ASSERT(top == orig_top);
    ASSERT(right == orig_right);
    ASSERT(bottom == orig_bottom);
}

TEST(clamp_with_exact_bounds) {
    /* Test exact boundary values */
    short left = 0, top = 0, right = MAP_WIDTH, bottom = MAP_HEIGHT;
    FUN_004423d0(&left, &top, &right, &bottom);
    ASSERT(left == 0);
    ASSERT(top == 0);
    ASSERT(right == MAP_WIDTH);
    ASSERT(bottom == MAP_HEIGHT);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Coordinate Clamp Function Unit Tests ===\n\n");

    printf("Single Boundary Tests:\n");
    RUN_TEST(clamp_negative_left);
    RUN_TEST(clamp_negative_top);
    RUN_TEST(clamp_exceed_right);
    RUN_TEST(clamp_exceed_bottom);

    printf("\nMultiple Boundary Tests:\n");
    RUN_TEST(clamp_all_in_bounds);
    RUN_TEST(clamp_all_out_of_bounds);
    RUN_TEST(clamp_at_boundaries);
    RUN_TEST(clamp_just_outside);

    printf("\nEdge Case Tests:\n");
    RUN_TEST(clamp_zero_rect);
    RUN_TEST(clamp_large_negative);
    RUN_TEST(clamp_large_positive);
    RUN_TEST(clamp_partial_out_of_bounds);

    printf("\nPreservation Tests:\n");
    RUN_TEST(clamp_preserves_valid_rect);
    RUN_TEST(clamp_with_exact_bounds);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
