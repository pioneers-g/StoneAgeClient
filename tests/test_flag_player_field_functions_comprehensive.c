/*
 * Stone Age Client - Flag and Player Field Functions Unit Tests
 * Tests for additional flag and player field functions
 * FUN_004780e0 - Clear bit 14 (0x4000)
 * FUN_004780f0 - Set player field at offset 0x112 to 1
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
typedef unsigned short u16;
typedef uintptr_t uintptr;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Simulated global variables */
static s32 DAT_0462bf2c = 0;
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
    DAT_0462bf2c = 0;
    DAT_0462e3ac = 0;
    memset(object_buffer, 0, sizeof(object_buffer));
}

/*
 * FUN_004780e0 - Clear bit 14 (0x4000)
 */
static void FUN_004780e0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xffffbfff;
}

/*
 * FUN_004780f0 - Set player field at offset 0x112 to 1
 */
static void FUN_004780f0(void) {
    if (DAT_0462e3ac != 0) {
        *(u16 *)(DAT_0462e3ac + 0x112) = 1;
    }
}

/* ========================================
 * Test Cases for FUN_004780e0
 * ======================================== */

TEST(clear_bit_14_from_zero) {
    reset_test_state();

    FUN_004780e0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_bit_14_set) {
    reset_test_state();
    DAT_0462bf2c = 0x4000;

    FUN_004780e0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_bit_14_preserves_lower_bits) {
    reset_test_state();
    DAT_0462bf2c = 0x3FFF;

    FUN_004780e0();

    ASSERT(DAT_0462bf2c == 0x3FFF);
}

TEST(clear_bit_14_preserves_upper_bits) {
    reset_test_state();
    DAT_0462bf2c = 0xFFFFC000;

    FUN_004780e0();

    ASSERT(DAT_0462bf2c == 0xFFFF8000);
}

TEST(clear_bit_14_all_bits_set) {
    reset_test_state();
    DAT_0462bf2c = 0xFFFFFFFF;

    FUN_004780e0();

    ASSERT(DAT_0462bf2c == 0xFFFFBFFF);
}

/* ========================================
 * Test Cases for FUN_004780f0
 * ======================================== */

TEST(set_field_no_object) {
    reset_test_state();
    DAT_0462e3ac = 0;

    FUN_004780f0();

    /* No crash expected */
    ASSERT(1);
}

TEST(set_field_with_object) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;
    *(u16 *)(object_buffer + 0x112) = 0;

    FUN_004780f0();

    ASSERT(*(u16 *)(object_buffer + 0x112) == 1);
}

TEST(set_field_overwrite_value) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;
    *(u16 *)(object_buffer + 0x112) = 999;

    FUN_004780f0();

    ASSERT(*(u16 *)(object_buffer + 0x112) == 1);
}

TEST(set_field_already_one) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;
    *(u16 *)(object_buffer + 0x112) = 1;

    FUN_004780f0();

    ASSERT(*(u16 *)(object_buffer + 0x112) == 1);
}

TEST(set_field_multiple_times) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;

    for (int i = 0; i < 5; i++) {
        *(u16 *)(object_buffer + 0x112) = i * 100;
        FUN_004780f0();
        ASSERT(*(u16 *)(object_buffer + 0x112) == 1);
    }
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Flag and Player Field Functions Unit Tests ===\n\n");

    printf("FUN_004780e0 (Clear Bit 14) Tests:\n");
    RUN_TEST(clear_bit_14_from_zero);
    RUN_TEST(clear_bit_14_set);
    RUN_TEST(clear_bit_14_preserves_lower_bits);
    RUN_TEST(clear_bit_14_preserves_upper_bits);
    RUN_TEST(clear_bit_14_all_bits_set);

    printf("\nFUN_004780f0 (Set Player Field to 1) Tests:\n");
    RUN_TEST(set_field_no_object);
    RUN_TEST(set_field_with_object);
    RUN_TEST(set_field_overwrite_value);
    RUN_TEST(set_field_already_one);
    RUN_TEST(set_field_multiple_times);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
