/*
 * Stone Age Client - Game State Functions Unit Tests
 * Tests for FUN_00479bc0, FUN_00479be0, FUN_00479c00, FUN_00479c20
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

static u32 DAT_04630dd8 = 0;  /* Current game state */
static u32 DAT_04630df0 = 0;  /* Current game substate */
static u32 DAT_04630df8 = 0;  /* Queued game state */
static u32 DAT_04630de8 = 0;  /* Queued game substate */

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
 * Game state functions
 */
static void FUN_00479bc0(u32 state) {
    DAT_04630df0 = 0;
    DAT_04630dd8 = state;
}

static void FUN_00479be0(u32 state, u32 substate) {
    DAT_04630dd8 = state;
    DAT_04630df0 = substate;
}

static void FUN_00479c00(u32 state) {
    DAT_04630de8 = 0;
    DAT_04630df8 = state;
}

static void FUN_00479c20(u32 state, u32 substate) {
    DAT_04630df8 = state;
    DAT_04630de8 = substate;
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(set_state_basic) {
    DAT_04630dd8 = 0;
    DAT_04630df0 = 5;

    FUN_00479bc0(3);

    ASSERT(DAT_04630dd8 == 3);
    ASSERT(DAT_04630df0 == 0);
}

TEST(set_state_zero) {
    DAT_04630dd8 = 5;
    DAT_04630df0 = 10;

    FUN_00479bc0(0);

    ASSERT(DAT_04630dd8 == 0);
    ASSERT(DAT_04630df0 == 0);
}

TEST(set_state_large) {
    DAT_04630dd8 = 0;

    FUN_00479bc0(100);

    ASSERT(DAT_04630dd8 == 100);
    ASSERT(DAT_04630df0 == 0);
}

TEST(set_state_substate_basic) {
    DAT_04630dd8 = 0;
    DAT_04630df0 = 0;

    FUN_00479be0(5, 3);

    ASSERT(DAT_04630dd8 == 5);
    ASSERT(DAT_04630df0 == 3);
}

TEST(set_state_substate_zero) {
    DAT_04630dd8 = 10;
    DAT_04630df0 = 20;

    FUN_00479be0(0, 0);

    ASSERT(DAT_04630dd8 == 0);
    ASSERT(DAT_04630df0 == 0);
}

TEST(set_state_substate_preserves_substate) {
    DAT_04630dd8 = 1;
    DAT_04630df0 = 5;

    FUN_00479be0(2, 5);

    ASSERT(DAT_04630dd8 == 2);
    ASSERT(DAT_04630df0 == 5);
}

TEST(queue_state_basic) {
    DAT_04630df8 = 0;
    DAT_04630de8 = 5;

    FUN_00479c00(10);

    ASSERT(DAT_04630df8 == 10);
    ASSERT(DAT_04630de8 == 0);
}

TEST(queue_state_zero) {
    DAT_04630df8 = 5;
    DAT_04630de8 = 3;

    FUN_00479c00(0);

    ASSERT(DAT_04630df8 == 0);
    ASSERT(DAT_04630de8 == 0);
}

TEST(queue_state_clears_substate) {
    DAT_04630df8 = 0;
    DAT_04630de8 = 100;

    FUN_00479c00(5);

    ASSERT(DAT_04630df8 == 5);
    ASSERT(DAT_04630de8 == 0);
}

TEST(queue_state_substate_basic) {
    DAT_04630df8 = 0;
    DAT_04630de8 = 0;

    FUN_00479c20(7, 2);

    ASSERT(DAT_04630df8 == 7);
    ASSERT(DAT_04630de8 == 2);
}

TEST(queue_state_substate_zero) {
    DAT_04630df8 = 10;
    DAT_04630de8 = 10;

    FUN_00479c20(0, 0);

    ASSERT(DAT_04630df8 == 0);
    ASSERT(DAT_04630de8 == 0);
}

TEST(queue_state_substate_large) {
    DAT_04630df8 = 0;
    DAT_04630de8 = 0;

    FUN_00479c20(200, 50);

    ASSERT(DAT_04630df8 == 200);
    ASSERT(DAT_04630de8 == 50);
}

TEST(state_sequence) {
    DAT_04630dd8 = 0;
    DAT_04630df0 = 0;

    /* Login -> Character Select */
    FUN_00479bc0(1);
    ASSERT(DAT_04630dd8 == 1);
    ASSERT(DAT_04630df0 == 0);

    /* Character Select -> Game */
    FUN_00479bc0(5);
    ASSERT(DAT_04630dd8 == 5);
    ASSERT(DAT_04630df0 == 0);

    /* Game -> Battle */
    FUN_00479bc0(10);
    ASSERT(DAT_04630dd8 == 10);
    ASSERT(DAT_04630df0 == 0);
}

TEST(state_with_substate_sequence) {
    DAT_04630dd8 = 0;
    DAT_04630df0 = 0;

    /* Enter game with loading substate */
    FUN_00479be0(5, 100);
    ASSERT(DAT_04630dd8 == 5);
    ASSERT(DAT_04630df0 == 100);

    /* Transition within game */
    FUN_00479be0(5, 101);
    ASSERT(DAT_04630dd8 == 5);
    ASSERT(DAT_04630df0 == 101);
}

TEST(queue_then_set) {
    DAT_04630dd8 = 0;
    DAT_04630df0 = 0;
    DAT_04630df8 = 0;
    DAT_04630de8 = 0;

    /* Queue a state change */
    FUN_00479c20(5, 3);
    ASSERT(DAT_04630df8 == 5);
    ASSERT(DAT_04630de8 == 3);

    /* Apply it */
    FUN_00479be0(DAT_04630df8, DAT_04630de8);
    ASSERT(DAT_04630dd8 == 5);
    ASSERT(DAT_04630df0 == 3);
}

TEST(multiple_queue_overwrites) {
    DAT_04630df8 = 0;
    DAT_04630de8 = 0;

    FUN_00479c20(1, 1);
    ASSERT(DAT_04630df8 == 1);
    ASSERT(DAT_04630de8 == 1);

    FUN_00479c20(2, 2);
    ASSERT(DAT_04630df8 == 2);
    ASSERT(DAT_04630de8 == 2);

    FUN_00479c20(3, 3);
    ASSERT(DAT_04630df8 == 3);
    ASSERT(DAT_04630de8 == 3);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Game State Functions Unit Tests ===\n\n");

    printf("Set State Tests:\n");
    RUN_TEST(set_state_basic);
    RUN_TEST(set_state_zero);
    RUN_TEST(set_state_large);

    printf("\nSet State with Substate Tests:\n");
    RUN_TEST(set_state_substate_basic);
    RUN_TEST(set_state_substate_zero);
    RUN_TEST(set_state_substate_preserves_substate);

    printf("\nQueue State Tests:\n");
    RUN_TEST(queue_state_basic);
    RUN_TEST(queue_state_zero);
    RUN_TEST(queue_state_clears_substate);

    printf("\nQueue State with Substate Tests:\n");
    RUN_TEST(queue_state_substate_basic);
    RUN_TEST(queue_state_substate_zero);
    RUN_TEST(queue_state_substate_large);

    printf("\nSequence Tests:\n");
    RUN_TEST(state_sequence);
    RUN_TEST(state_with_substate_sequence);
    RUN_TEST(queue_then_set);
    RUN_TEST(multiple_queue_overwrites);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
