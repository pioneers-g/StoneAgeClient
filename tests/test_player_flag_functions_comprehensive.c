/*
 * Stone Age Client - Player Flag Functions Unit Tests
 * Tests for FUN_00478090, FUN_004780a0, FUN_004780b0, FUN_004780c0,
 * FUN_004780d0, FUN_004780e0, FUN_004780f0, FUN_00478110
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

static u32 DAT_0462bf2c = 0;
static void* DAT_0462e3ac = NULL;

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

/* Simulated entity structure */
typedef struct {
    char padding[0x112];
    short move_flag;
} TestEntity;

static TestEntity g_test_entity;

/* Reset test state */
static void reset_test_state(void) {
    DAT_0462bf2c = 0;
    DAT_0462e3ac = NULL;
    memset(&g_test_entity, 0, sizeof(g_test_entity));
}

/*
 * Flag manipulation functions
 */

/* FUN_00478090 - Set flag 0x100 */
static void FUN_00478090(void) {
    DAT_0462bf2c = DAT_0462bf2c | 0x100;
}

/* FUN_004780a0 - Clear flag 0x100 */
static void FUN_004780a0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xfffffeff;
}

/* FUN_004780b0 - Set flag 0x200 */
static void FUN_004780b0(void) {
    DAT_0462bf2c = DAT_0462bf2c | 0x200;
}

/* FUN_004780c0 - Clear flag 0x200 */
static void FUN_004780c0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xfffffdff;
}

/* FUN_004780d0 - Set flag 0x4000 */
static void FUN_004780d0(void) {
    DAT_0462bf2c = DAT_0462bf2c | 0x4000;
}

/* FUN_004780e0 - Clear flag 0x4000 */
static void FUN_004780e0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xffffbfff;
}

/* FUN_004780f0 - Set move flag to 1 */
static void FUN_004780f0(void) {
    if (DAT_0462e3ac != NULL) {
        ((TestEntity*)DAT_0462e3ac)->move_flag = 1;
    }
}

/* FUN_00478110 - Set move flag to 0 */
static void FUN_00478110(void) {
    if (DAT_0462e3ac != NULL) {
        ((TestEntity*)DAT_0462e3ac)->move_flag = 0;
    }
}

/* ========================================
 * Test Cases
 * ======================================== */

/* FUN_00478090 tests */
TEST(set_flag_100_from_zero) {
    reset_test_state();

    FUN_00478090();

    ASSERT(DAT_0462bf2c == 0x100);
}

TEST(set_flag_100_already_set) {
    reset_test_state();
    DAT_0462bf2c = 0x100;

    FUN_00478090();

    ASSERT(DAT_0462bf2c == 0x100);
}

TEST(set_flag_100_preserves_other) {
    reset_test_state();
    DAT_0462bf2c = 0x500;

    FUN_00478090();

    ASSERT(DAT_0462bf2c == 0x500);
}

/* FUN_004780a0 tests */
TEST(clear_flag_100_from_set) {
    reset_test_state();
    DAT_0462bf2c = 0x100;

    FUN_004780a0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_100_already_clear) {
    reset_test_state();

    FUN_004780a0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_100_preserves_other) {
    reset_test_state();
    DAT_0462bf2c = 0x500;

    FUN_004780a0();

    ASSERT(DAT_0462bf2c == 0x400);
}

/* FUN_004780b0 tests */
TEST(set_flag_200_from_zero) {
    reset_test_state();

    FUN_004780b0();

    ASSERT(DAT_0462bf2c == 0x200);
}

TEST(set_flag_200_already_set) {
    reset_test_state();
    DAT_0462bf2c = 0x200;

    FUN_004780b0();

    ASSERT(DAT_0462bf2c == 0x200);
}

TEST(set_flag_200_preserves_other) {
    reset_test_state();
    DAT_0462bf2c = 0x100;

    FUN_004780b0();

    ASSERT(DAT_0462bf2c == 0x300);
}

/* FUN_004780c0 tests */
TEST(clear_flag_200_from_set) {
    reset_test_state();
    DAT_0462bf2c = 0x200;

    FUN_004780c0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_200_already_clear) {
    reset_test_state();

    FUN_004780c0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_200_preserves_other) {
    reset_test_state();
    DAT_0462bf2c = 0x300;

    FUN_004780c0();

    ASSERT(DAT_0462bf2c == 0x100);
}

/* FUN_004780d0 tests */
TEST(set_flag_4000_from_zero) {
    reset_test_state();

    FUN_004780d0();

    ASSERT(DAT_0462bf2c == 0x4000);
}

TEST(set_flag_4000_already_set) {
    reset_test_state();
    DAT_0462bf2c = 0x4000;

    FUN_004780d0();

    ASSERT(DAT_0462bf2c == 0x4000);
}

TEST(set_flag_4000_preserves_other) {
    reset_test_state();
    DAT_0462bf2c = 0x100;

    FUN_004780d0();

    ASSERT(DAT_0462bf2c == 0x4100);
}

/* FUN_004780e0 tests */
TEST(clear_flag_4000_from_set) {
    reset_test_state();
    DAT_0462bf2c = 0x4000;

    FUN_004780e0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_4000_already_clear) {
    reset_test_state();

    FUN_004780e0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_4000_preserves_other) {
    reset_test_state();
    DAT_0462bf2c = 0x4100;

    FUN_004780e0();

    ASSERT(DAT_0462bf2c == 0x100);
}

/* FUN_004780f0 tests */
TEST(set_move_flag_no_entity) {
    reset_test_state();

    FUN_004780f0();

    /* Should not crash */
    ASSERT(1);
}

TEST(set_move_flag_with_entity) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;
    g_test_entity.move_flag = 0;

    FUN_004780f0();

    ASSERT(g_test_entity.move_flag == 1);
}

TEST(set_move_flag_already_set) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;
    g_test_entity.move_flag = 1;

    FUN_004780f0();

    ASSERT(g_test_entity.move_flag == 1);
}

/* FUN_00478110 tests */
TEST(clear_move_flag_no_entity) {
    reset_test_state();

    FUN_00478110();

    /* Should not crash */
    ASSERT(1);
}

TEST(clear_move_flag_with_entity) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;
    g_test_entity.move_flag = 1;

    FUN_00478110();

    ASSERT(g_test_entity.move_flag == 0);
}

TEST(clear_move_flag_already_clear) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;
    g_test_entity.move_flag = 0;

    FUN_00478110();

    ASSERT(g_test_entity.move_flag == 0);
}

/* Combination tests */
TEST(set_clear_flag_100_cycle) {
    reset_test_state();

    FUN_00478090();
    ASSERT(DAT_0462bf2c == 0x100);

    FUN_004780a0();
    ASSERT(DAT_0462bf2c == 0);

    FUN_00478090();
    ASSERT(DAT_0462bf2c == 0x100);
}

TEST(set_clear_flag_200_cycle) {
    reset_test_state();

    FUN_004780b0();
    ASSERT(DAT_0462bf2c == 0x200);

    FUN_004780c0();
    ASSERT(DAT_0462bf2c == 0);

    FUN_004780b0();
    ASSERT(DAT_0462bf2c == 0x200);
}

TEST(set_clear_flag_4000_cycle) {
    reset_test_state();

    FUN_004780d0();
    ASSERT(DAT_0462bf2c == 0x4000);

    FUN_004780e0();
    ASSERT(DAT_0462bf2c == 0);

    FUN_004780d0();
    ASSERT(DAT_0462bf2c == 0x4000);
}

TEST(multiple_flags_independent) {
    reset_test_state();

    FUN_00478090();
    FUN_004780b0();
    FUN_004780d0();

    ASSERT(DAT_0462bf2c == 0x4300);

    FUN_004780a0();
    ASSERT(DAT_0462bf2c == 0x4200);

    FUN_004780c0();
    ASSERT(DAT_0462bf2c == 0x4000);

    FUN_004780e0();
    ASSERT(DAT_0462bf2c == 0);
}

TEST(move_flag_toggle) {
    reset_test_state();
    DAT_0462e3ac = &g_test_entity;

    FUN_004780f0();
    ASSERT(g_test_entity.move_flag == 1);

    FUN_00478110();
    ASSERT(g_test_entity.move_flag == 0);

    FUN_004780f0();
    ASSERT(g_test_entity.move_flag == 1);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Player Flag Functions Unit Tests ===\n\n");

    printf("FUN_00478090 (Set Flag 0x100) Tests:\n");
    RUN_TEST(set_flag_100_from_zero);
    RUN_TEST(set_flag_100_already_set);
    RUN_TEST(set_flag_100_preserves_other);

    printf("\nFUN_004780a0 (Clear Flag 0x100) Tests:\n");
    RUN_TEST(clear_flag_100_from_set);
    RUN_TEST(clear_flag_100_already_clear);
    RUN_TEST(clear_flag_100_preserves_other);

    printf("\nFUN_004780b0 (Set Flag 0x200) Tests:\n");
    RUN_TEST(set_flag_200_from_zero);
    RUN_TEST(set_flag_200_already_set);
    RUN_TEST(set_flag_200_preserves_other);

    printf("\nFUN_004780c0 (Clear Flag 0x200) Tests:\n");
    RUN_TEST(clear_flag_200_from_set);
    RUN_TEST(clear_flag_200_already_clear);
    RUN_TEST(clear_flag_200_preserves_other);

    printf("\nFUN_004780d0 (Set Flag 0x4000) Tests:\n");
    RUN_TEST(set_flag_4000_from_zero);
    RUN_TEST(set_flag_4000_already_set);
    RUN_TEST(set_flag_4000_preserves_other);

    printf("\nFUN_004780e0 (Clear Flag 0x4000) Tests:\n");
    RUN_TEST(clear_flag_4000_from_set);
    RUN_TEST(clear_flag_4000_already_clear);
    RUN_TEST(clear_flag_4000_preserves_other);

    printf("\nFUN_004780f0 (Set Move Flag) Tests:\n");
    RUN_TEST(set_move_flag_no_entity);
    RUN_TEST(set_move_flag_with_entity);
    RUN_TEST(set_move_flag_already_set);

    printf("\nFUN_00478110 (Clear Move Flag) Tests:\n");
    RUN_TEST(clear_move_flag_no_entity);
    RUN_TEST(clear_move_flag_with_entity);
    RUN_TEST(clear_move_flag_already_clear);

    printf("\nCombination Tests:\n");
    RUN_TEST(set_clear_flag_100_cycle);
    RUN_TEST(set_clear_flag_200_cycle);
    RUN_TEST(set_clear_flag_4000_cycle);
    RUN_TEST(multiple_flags_independent);
    RUN_TEST(move_flag_toggle);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
