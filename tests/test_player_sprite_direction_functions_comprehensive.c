/*
 * Stone Age Client - Player Sprite and Direction Functions Unit Tests
 * Tests for FUN_00477c70 - Set sprite and direction values
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;
typedef unsigned char u8;
typedef uintptr_t uintptr;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Simulated global variables */
static s32 DAT_0462be88 = 0;
static s32 DAT_0462be94 = 0;
static uintptr DAT_0462e3ac = 0;

/* Simulated object buffer */
#define OBJECT_SIZE 0x200
static u8 object_buffer[OBJECT_SIZE] = {0};

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
    DAT_0462be88 = 0;
    DAT_0462be94 = 0;
    DAT_0462e3ac = 0;
    memset(object_buffer, 0, sizeof(object_buffer));
}

/*
 * FUN_00477c70 - Set sprite and direction values
 * Sets DAT_0462be88 = param_1, DAT_0462be94 = param_2
 * If object exists, also sets object fields at 0x140 and 0x150
 */
static void FUN_00477c70(u32 param_1, u32 param_2) {
    DAT_0462be88 = param_1;
    DAT_0462be94 = param_2;
    if (DAT_0462e3ac != 0) {
        *(u32 *)(DAT_0462e3ac + 0x140) = param_1;
        *(u32 *)(DAT_0462e3ac + 0x150) = param_2;
    }
}

/* ========================================
 * Test Cases for FUN_00477c70
 * ======================================== */

TEST(set_values_no_object) {
    reset_test_state();
    DAT_0462e3ac = 0;

    FUN_00477c70(100, 200);

    ASSERT(DAT_0462be88 == 100);
    ASSERT(DAT_0462be94 == 200);
}

TEST(set_values_with_object) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    FUN_00477c70(0x18ABC, 42);

    ASSERT(DAT_0462be88 == 0x18ABC);
    ASSERT(DAT_0462be94 == 42);
    ASSERT(*(u32 *)(object_buffer + 0x140) == 0x18ABC);
    ASSERT(*(u32 *)(object_buffer + 0x150) == 42);
}

TEST(set_values_zero) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;
    DAT_0462be88 = 999;
    DAT_0462be94 = 888;
    *(u32 *)(object_buffer + 0x140) = 777;
    *(u32 *)(object_buffer + 0x150) = 666;

    FUN_00477c70(0, 0);

    ASSERT(DAT_0462be88 == 0);
    ASSERT(DAT_0462be94 == 0);
    ASSERT(*(u32 *)(object_buffer + 0x140) == 0);
    ASSERT(*(u32 *)(object_buffer + 0x150) == 0);
}

TEST(set_values_max) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    FUN_00477c70(0xFFFFFFFF, 0xFFFFFFFF);

    ASSERT(DAT_0462be88 == 0xFFFFFFFF);
    ASSERT(DAT_0462be94 == 0xFFFFFFFF);
}

TEST(set_values_overwrites) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;
    *(u32 *)(object_buffer + 0x140) = 11111;
    *(u32 *)(object_buffer + 0x150) = 22222;

    FUN_00477c70(33333, 44444);

    ASSERT(*(u32 *)(object_buffer + 0x140) == 33333);
    ASSERT(*(u32 *)(object_buffer + 0x150) == 44444);
}

TEST(set_values_independent) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    /* First call */
    FUN_00477c70(100, 200);
    ASSERT(DAT_0462be88 == 100);
    ASSERT(DAT_0462be94 == 200);

    /* Second call with different values */
    FUN_00477c70(300, 400);
    ASSERT(DAT_0462be88 == 300);
    ASSERT(DAT_0462be94 == 400);
}

TEST(set_values_sprite_id_pattern) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    /* Test with typical sprite ID values */
    FUN_00477c70(0x18ABC, 3);

    ASSERT(DAT_0462be88 == 0x18ABC);
    ASSERT(DAT_0462be94 == 3);
    ASSERT(*(u32 *)(object_buffer + 0x140) == 0x18ABC);
    ASSERT(*(u32 *)(object_buffer + 0x150) == 3);
}

TEST(set_values_no_object_no_crash) {
    reset_test_state();
    DAT_0462e3ac = 0;

    /* Should not crash when object is null */
    FUN_00477c70(12345, 67890);

    ASSERT(DAT_0462be88 == 12345);
    ASSERT(DAT_0462be94 == 67890);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Player Sprite and Direction Functions Unit Tests ===\n\n");

    printf("FUN_00477c70 (Set Sprite and Direction) Tests:\n");
    RUN_TEST(set_values_no_object);
    RUN_TEST(set_values_with_object);
    RUN_TEST(set_values_zero);
    RUN_TEST(set_values_max);
    RUN_TEST(set_values_overwrites);
    RUN_TEST(set_values_independent);
    RUN_TEST(set_values_sprite_id_pattern);
    RUN_TEST(set_values_no_object_no_crash);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
