/*
 * Stone Age Client - Player Position Sync Functions Unit Tests
 * Tests for FUN_00477cd0 - Sync position data to player object
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

/* Simulated source position variables */
static s32 DAT_0456a644 = 0;
static s32 DAT_0456a648 = 0;
static s32 DAT_04581d3c = 0;
static s32 DAT_04581d40 = 0;
static s32 DAT_045827fc = 0;
static s32 DAT_04582800 = 0;
static s32 DAT_0458118c = 0;
static s32 DAT_04581184 = 0;

/* Player object pointer */
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
    DAT_0456a644 = 0;
    DAT_0456a648 = 0;
    DAT_04581d3c = 0;
    DAT_04581d40 = 0;
    DAT_045827fc = 0;
    DAT_04582800 = 0;
    DAT_0458118c = 0;
    DAT_04581184 = 0;
    DAT_0462e3ac = 0;
    memset(object_buffer, 0, sizeof(object_buffer));
}

/*
 * FUN_00477cd0 - Sync position data to player object
 * Copies 8 global position values to player object fields
 */
static void FUN_00477cd0(void) {
    if (DAT_0462e3ac != 0) {
        *(u32 *)(DAT_0462e3ac + 0x114) = DAT_0456a644;
        *(u32 *)(DAT_0462e3ac + 0x118) = DAT_0456a648;
        *(u32 *)(DAT_0462e3ac + 0xb0) = DAT_04581d3c;
        *(u32 *)(DAT_0462e3ac + 0xb4) = DAT_04581d40;
        *(u32 *)(DAT_0462e3ac + 0x11c) = DAT_045827fc;
        *(u32 *)(DAT_0462e3ac + 0x120) = DAT_04582800;
        *(u32 *)(DAT_0462e3ac + 0xb8) = DAT_0458118c;
        *(u32 *)(DAT_0462e3ac + 0xbc) = DAT_04581184;
    }
}

/* ========================================
 * Test Cases for FUN_00477cd0
 * ======================================== */

TEST(sync_no_object) {
    reset_test_state();
    DAT_0462e3ac = 0;

    FUN_00477cd0();

    /* Should not crash */
    ASSERT(1);
}

TEST(sync_basic_values) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    DAT_0456a644 = 100;
    DAT_0456a648 = 200;
    DAT_04581d3c = 300;
    DAT_04581d40 = 400;

    FUN_00477cd0();

    ASSERT(*(u32 *)(object_buffer + 0x114) == 100);
    ASSERT(*(u32 *)(object_buffer + 0x118) == 200);
    ASSERT(*(u32 *)(object_buffer + 0xb0) == 300);
    ASSERT(*(u32 *)(object_buffer + 0xb4) == 400);
}

TEST(sync_all_fields) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    DAT_0456a644 = 111;
    DAT_0456a648 = 222;
    DAT_04581d3c = 333;
    DAT_04581d40 = 444;
    DAT_045827fc = 555;
    DAT_04582800 = 666;
    DAT_0458118c = 777;
    DAT_04581184 = 888;

    FUN_00477cd0();

    ASSERT(*(u32 *)(object_buffer + 0x114) == 111);
    ASSERT(*(u32 *)(object_buffer + 0x118) == 222);
    ASSERT(*(u32 *)(object_buffer + 0xb0) == 333);
    ASSERT(*(u32 *)(object_buffer + 0xb4) == 444);
    ASSERT(*(u32 *)(object_buffer + 0x11c) == 555);
    ASSERT(*(u32 *)(object_buffer + 0x120) == 666);
    ASSERT(*(u32 *)(object_buffer + 0xb8) == 777);
    ASSERT(*(u32 *)(object_buffer + 0xbc) == 888);
}

TEST(sync_zero_values) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    /* Pre-fill with non-zero */
    memset(object_buffer, 0xFF, sizeof(object_buffer));

    DAT_0456a644 = 0;
    DAT_0456a648 = 0;
    DAT_04581d3c = 0;
    DAT_04581d40 = 0;
    DAT_045827fc = 0;
    DAT_04582800 = 0;
    DAT_0458118c = 0;
    DAT_04581184 = 0;

    FUN_00477cd0();

    ASSERT(*(u32 *)(object_buffer + 0x114) == 0);
    ASSERT(*(u32 *)(object_buffer + 0x118) == 0);
    ASSERT(*(u32 *)(object_buffer + 0xb0) == 0);
    ASSERT(*(u32 *)(object_buffer + 0xb4) == 0);
}

TEST(sync_overwrites_existing) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    /* Set existing values */
    *(u32 *)(object_buffer + 0x114) = 9999;
    *(u32 *)(object_buffer + 0xb0) = 8888;

    DAT_0456a644 = 100;
    DAT_04581d3c = 200;

    FUN_00477cd0();

    ASSERT(*(u32 *)(object_buffer + 0x114) == 100);
    ASSERT(*(u32 *)(object_buffer + 0xb0) == 200);
}

TEST(sync_negative_values) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    DAT_04581d3c = -100;
    DAT_04581d40 = -200;

    FUN_00477cd0();

    ASSERT((s32)*(u32 *)(object_buffer + 0xb0) == -100);
    ASSERT((s32)*(u32 *)(object_buffer + 0xb4) == -200);
}

TEST(sync_position_coordinates) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    /* Typical game coordinates */
    DAT_04581d3c = 50;
    DAT_04581d40 = 75;

    FUN_00477cd0();

    ASSERT(*(u32 *)(object_buffer + 0xb0) == 50);
    ASSERT(*(u32 *)(object_buffer + 0xb4) == 75);
}

TEST(sync_multiple_calls) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    DAT_04581d3c = 100;
    FUN_00477cd0();
    ASSERT(*(u32 *)(object_buffer + 0xb0) == 100);

    DAT_04581d3c = 200;
    FUN_00477cd0();
    ASSERT(*(u32 *)(object_buffer + 0xb0) == 200);

    DAT_04581d3c = 300;
    FUN_00477cd0();
    ASSERT(*(u32 *)(object_buffer + 0xb0) == 300);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Player Position Sync Functions Unit Tests ===\n\n");

    printf("FUN_00477cd0 (Sync Position Data) Tests:\n");
    RUN_TEST(sync_no_object);
    RUN_TEST(sync_basic_values);
    RUN_TEST(sync_all_fields);
    RUN_TEST(sync_zero_values);
    RUN_TEST(sync_overwrites_existing);
    RUN_TEST(sync_negative_values);
    RUN_TEST(sync_position_coordinates);
    RUN_TEST(sync_multiple_calls);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
