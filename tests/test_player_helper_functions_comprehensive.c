/*
 * Stone Age Client - Player Helper Functions Unit Tests
 * Tests for FUN_00477d70, FUN_00477d90, FUN_00478130, FUN_00478190
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

/* Simulated player entity structure */
typedef struct {
    char padding[0x148];
    int state;          /* offset 0x148 */
    int move_target;    /* offset 0x14c */
    char padding2[4];
    u32 color;          /* offset 0x98 - needs adjustment */
    char padding3[0x68];
    u32 direction;      /* offset 0x150 */
    char padding4[0x1e];
    short move_flag;    /* offset 0x112 - needs adjustment */
} TestEntity;

static TestEntity g_test_entity;
static void* DAT_0462e3ac = NULL;
static u32 DAT_0462be94 = 0;
static u32 DAT_0462e3b0 = 0;

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

/* Reset test state */
static void reset_test_state(void) {
    memset(&g_test_entity, 0, sizeof(g_test_entity));
    DAT_0462e3ac = NULL;
    DAT_0462be94 = 0;
    DAT_0462e3b0 = 0;
}

/*
 * Player helper functions
 */
static void FUN_00477d70(u32 direction) {
    DAT_0462be94 = direction;
    if (DAT_0462e3ac != NULL) {
        ((TestEntity*)DAT_0462e3ac)->direction = direction;
    }
}

static void FUN_00477d90(int state) {
    if (DAT_0462e3ac != NULL) {
        ((TestEntity*)DAT_0462e3ac)->state = state;
        if (((TestEntity*)DAT_0462e3ac)->state != 4) {
            ((TestEntity*)DAT_0462e3ac)->move_target = -1;
        }
    }
}

static int FUN_00478130(void) {
    if (DAT_0462e3ac == NULL) {
        return 0;
    }
    return (int)((TestEntity*)DAT_0462e3ac)->move_flag;
}

static void FUN_00478190(u32 color) {
    DAT_0462e3b0 = color;
    if (DAT_0462e3ac != NULL) {
        ((TestEntity*)DAT_0462e3ac)->color = color;
    }
}

/* ========================================
 * Test Cases
 * ======================================== */

/* FUN_00477d70 tests */
TEST(set_direction_no_entity) {
    reset_test_state();

    FUN_00477d70(5);

    ASSERT(DAT_0462be94 == 5);
}

TEST(set_direction_with_entity) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;

    FUN_00477d70(3);

    ASSERT(DAT_0462be94 == 3);
    ASSERT(g_test_entity.direction == 3);
}

TEST(set_direction_zero) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;
    g_test_entity.direction = 5;

    FUN_00477d70(0);

    ASSERT(DAT_0462be94 == 0);
    ASSERT(g_test_entity.direction == 0);
}

TEST(set_direction_max) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;

    FUN_00477d70(7);

    ASSERT(DAT_0462be94 == 7);
    ASSERT(g_test_entity.direction == 7);
}

/* FUN_00477d90 tests */
TEST(set_state_no_entity) {
    reset_test_state();

    FUN_00477d90(3);

    /* Should not crash, state unchanged */
    ASSERT(1);
}

TEST(set_state_with_entity) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;

    FUN_00477d90(3);

    ASSERT(g_test_entity.state == 3);
}

TEST(set_state_moving_preserves_target) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;
    g_test_entity.move_target = 100;

    FUN_00477d90(4);  /* Moving state */

    ASSERT(g_test_entity.state == 4);
    ASSERT(g_test_entity.move_target == 100);  /* Should not change */
}

TEST(set_state_nonmoving_clears_target) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;
    g_test_entity.move_target = 100;

    FUN_00477d90(3);  /* Idle state */

    ASSERT(g_test_entity.state == 3);
    ASSERT(g_test_entity.move_target == -1);  /* Should be cleared */
}

TEST(set_state_zero_clears_target) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;
    g_test_entity.move_target = 50;

    FUN_00477d90(0);

    ASSERT(g_test_entity.state == 0);
    ASSERT(g_test_entity.move_target == -1);
}

/* FUN_00478130 tests */
TEST(get_move_flag_no_entity) {
    reset_test_state();

    int result = FUN_00478130();

    ASSERT(result == 0);
}

TEST(get_move_flag_zero) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;
    g_test_entity.move_flag = 0;

    int result = FUN_00478130();

    ASSERT(result == 0);
}

TEST(get_move_flag_one) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;
    g_test_entity.move_flag = 1;

    int result = FUN_00478130();

    ASSERT(result == 1);
}

TEST(get_move_flag_negative) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;
    g_test_entity.move_flag = -1;

    int result = FUN_00478130();

    ASSERT(result == -1);
}

/* FUN_00478190 tests */
TEST(set_color_no_entity) {
    reset_test_state();

    FUN_00478190(0xFF0000);

    ASSERT(DAT_0462e3b0 == 0xFF0000);
}

TEST(set_color_with_entity) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;

    FUN_00478190(0x00FF00);

    ASSERT(DAT_0462e3b0 == 0x00FF00);
    ASSERT(g_test_entity.color == 0x00FF00);
}

TEST(set_color_zero) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;
    g_test_entity.color = 0xFFFFFF;

    FUN_00478190(0);

    ASSERT(DAT_0462e3b0 == 0);
    ASSERT(g_test_entity.color == 0);
}

TEST(set_color_max) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;

    FUN_00478190(0xFFFFFFFF);

    ASSERT(DAT_0462e3b0 == 0xFFFFFFFF);
    ASSERT(g_test_entity.color == 0xFFFFFFFF);
}

TEST(set_color_overwrites) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;
    DAT_0462e3b0 = 0x111111;
    g_test_entity.color = 0x222222;

    FUN_00478190(0x333333);

    ASSERT(DAT_0462e3b0 == 0x333333);
    ASSERT(g_test_entity.color == 0x333333);
}

/* Integration tests */
TEST(direction_and_state_sequence) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;

    /* Set initial state */
    FUN_00477d90(3);
    ASSERT(g_test_entity.state == 3);

    /* Set direction */
    FUN_00477d70(2);
    ASSERT(g_test_entity.direction == 2);

    /* Change to moving state */
    FUN_00477d90(4);
    ASSERT(g_test_entity.state == 4);

    /* Change direction while moving */
    FUN_00477d70(5);
    ASSERT(g_test_entity.direction == 5);
}

TEST(color_and_direction_independent) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;

    FUN_00477d70(3);
    FUN_00478190(0xFF0000);

    ASSERT(g_test_entity.direction == 3);
    ASSERT(g_test_entity.color == 0xFF0000);

    FUN_00477d70(7);

    ASSERT(g_test_entity.direction == 7);
    ASSERT(g_test_entity.color == 0xFF0000);  /* Color should not change */
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Player Helper Functions Unit Tests ===\n\n");

    printf("FUN_00477d70 (Set Direction) Tests:\n");
    RUN_TEST(set_direction_no_entity);
    RUN_TEST(set_direction_with_entity);
    RUN_TEST(set_direction_zero);
    RUN_TEST(set_direction_max);

    printf("\nFUN_00477d90 (Set State) Tests:\n");
    RUN_TEST(set_state_no_entity);
    RUN_TEST(set_state_with_entity);
    RUN_TEST(set_state_moving_preserves_target);
    RUN_TEST(set_state_nonmoving_clears_target);
    RUN_TEST(set_state_zero_clears_target);

    printf("\nFUN_00478130 (Get Move Flag) Tests:\n");
    RUN_TEST(get_move_flag_no_entity);
    RUN_TEST(get_move_flag_zero);
    RUN_TEST(get_move_flag_one);
    RUN_TEST(get_move_flag_negative);

    printf("\nFUN_00478190 (Set Color) Tests:\n");
    RUN_TEST(set_color_no_entity);
    RUN_TEST(set_color_with_entity);
    RUN_TEST(set_color_zero);
    RUN_TEST(set_color_max);
    RUN_TEST(set_color_overwrites);

    printf("\nIntegration Tests:\n");
    RUN_TEST(direction_and_state_sequence);
    RUN_TEST(color_and_direction_independent);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
