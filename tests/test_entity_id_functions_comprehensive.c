/*
 * Stone Age Client - Entity ID Functions Unit Tests
 * Tests for FUN_00477ca0 and related ID setter functions
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

static u32 DAT_0462be90 = 0;  /* Player entity ID */
static u32 DAT_0462be88 = 0;  /* Player model/sprite ID */
static u32 DAT_0462be94 = 0;  /* Player direction */

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
    DAT_0462be90 = 0;
    DAT_0462be88 = 0;
    DAT_0462be94 = 0;
}

/*
 * ID setter functions
 */
static void FUN_00477ca0(u32 entity_id) {
    DAT_0462be90 = entity_id;
}

static void set_player_model(u32 model_id) {
    DAT_0462be88 = model_id;
}

static void set_player_direction(u32 direction) {
    DAT_0462be94 = direction;
}

/*
 * ID getter functions
 */
static u32 get_player_entity_id(void) {
    return DAT_0462be90;
}

static u32 get_player_model(void) {
    return DAT_0462be88;
}

static u32 get_player_direction(void) {
    return DAT_0462be94;
}

/* ========================================
 * Test Cases
 * ======================================== */

/* FUN_00477ca0 tests */
TEST(set_entity_id_basic) {
    reset_test_state();

    FUN_00477ca0(12345);

    ASSERT(DAT_0462be90 == 12345);
}

TEST(set_entity_id_zero) {
    reset_test_state();
    DAT_0462be90 = 999;

    FUN_00477ca0(0);

    ASSERT(DAT_0462be90 == 0);
}

TEST(set_entity_id_max) {
    reset_test_state();

    FUN_00477ca0(0xFFFFFFFF);

    ASSERT(DAT_0462be90 == 0xFFFFFFFF);
}

TEST(set_entity_id_overwrite) {
    reset_test_state();

    FUN_00477ca0(100);
    ASSERT(DAT_0462be90 == 100);

    FUN_00477ca0(200);
    ASSERT(DAT_0462be90 == 200);

    FUN_00477ca0(300);
    ASSERT(DAT_0462be90 == 300);
}

/* Model setter tests */
TEST(set_model_basic) {
    reset_test_state();

    set_player_model(0x1234);

    ASSERT(DAT_0462be88 == 0x1234);
}

TEST(set_model_zero) {
    reset_test_state();
    DAT_0462be88 = 0x5678;

    set_player_model(0);

    ASSERT(DAT_0462be88 == 0);
}

/* Direction setter tests */
TEST(set_direction_basic) {
    reset_test_state();

    set_player_direction(5);

    ASSERT(DAT_0462be94 == 5);
}

TEST(set_direction_all_values) {
    reset_test_state();

    for (u32 i = 0; i < 8; i++) {
        set_player_direction(i);
        ASSERT(DAT_0462be94 == i);
    }
}

/* Getter tests */
TEST(get_entity_id_basic) {
    reset_test_state();
    DAT_0462be90 = 500;

    u32 result = get_player_entity_id();

    ASSERT(result == 500);
}

TEST(get_entity_id_zero) {
    reset_test_state();

    u32 result = get_player_entity_id();

    ASSERT(result == 0);
}

TEST(get_model_basic) {
    reset_test_state();
    DAT_0462be88 = 0xABCD;

    u32 result = get_player_model();

    ASSERT(result == 0xABCD);
}

TEST(get_direction_basic) {
    reset_test_state();
    DAT_0462be94 = 7;

    u32 result = get_player_direction();

    ASSERT(result == 7);
}

/* Round-trip tests */
TEST(set_get_entity_id) {
    reset_test_state();

    FUN_00477ca0(42);
    u32 result = get_player_entity_id();

    ASSERT(result == 42);
}

TEST(set_get_model) {
    reset_test_state();

    set_player_model(0x9999);
    u32 result = get_player_model();

    ASSERT(result == 0x9999);
}

TEST(set_get_direction) {
    reset_test_state();

    set_player_direction(3);
    u32 result = get_player_direction();

    ASSERT(result == 3);
}

/* Multiple field tests */
TEST(set_all_fields) {
    reset_test_state();

    FUN_00477ca0(111);
    set_player_model(222);
    set_player_direction(333);

    ASSERT(DAT_0462be90 == 111);
    ASSERT(DAT_0462be88 == 222);
    ASSERT(DAT_0462be94 == 333);
}

TEST(set_fields_independently) {
    reset_test_state();

    FUN_00477ca0(100);
    ASSERT(DAT_0462be90 == 100);
    ASSERT(DAT_0462be88 == 0);
    ASSERT(DAT_0462be94 == 0);

    set_player_model(200);
    ASSERT(DAT_0462be90 == 100);
    ASSERT(DAT_0462be88 == 200);
    ASSERT(DAT_0462be94 == 0);

    set_player_direction(300);
    ASSERT(DAT_0462be90 == 100);
    ASSERT(DAT_0462be88 == 200);
    ASSERT(DAT_0462be94 == 300);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Entity ID Functions Unit Tests ===\n\n");

    printf("FUN_00477ca0 (Set Entity ID) Tests:\n");
    RUN_TEST(set_entity_id_basic);
    RUN_TEST(set_entity_id_zero);
    RUN_TEST(set_entity_id_max);
    RUN_TEST(set_entity_id_overwrite);

    printf("\nModel Setter Tests:\n");
    RUN_TEST(set_model_basic);
    RUN_TEST(set_model_zero);

    printf("\nDirection Setter Tests:\n");
    RUN_TEST(set_direction_basic);
    RUN_TEST(set_direction_all_values);

    printf("\nGetter Tests:\n");
    RUN_TEST(get_entity_id_basic);
    RUN_TEST(get_entity_id_zero);
    RUN_TEST(get_model_basic);
    RUN_TEST(get_direction_basic);

    printf("\nRound-Trip Tests:\n");
    RUN_TEST(set_get_entity_id);
    RUN_TEST(set_get_model);
    RUN_TEST(set_get_direction);

    printf("\nMultiple Field Tests:\n");
    RUN_TEST(set_all_fields);
    RUN_TEST(set_fields_independently);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
