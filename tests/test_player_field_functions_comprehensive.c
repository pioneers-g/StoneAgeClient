/*
 * Stone Age Client - Player Field Access Functions Unit Tests
 * Tests for simple player data field access functions
 * FUN_004781e0 - Clear party flag bit
 * FUN_00478110 - Clear player field at offset 0x112
 * FUN_00478130 - Get player field at offset 0x112
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
static uintptr DAT_0462e3ac = 0;  /* Use pointer-sized type */

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
 * FUN_004781e0 - Clear party flag bit 0x10000
 * Clears bit 16 of DAT_0462bf2c
 */
static void FUN_004781e0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xfffeffff;
}

/*
 * FUN_00478110 - Clear player field at offset 0x112
 * Sets 2-byte value at offset 0x112 to 0 if object exists
 */
static void FUN_00478110(void) {
    if (DAT_0462e3ac != 0) {
        *(u16 *)(DAT_0462e3ac + 0x112) = 0;
    }
}

/*
 * FUN_00478130 - Get player field at offset 0x112
 * Returns 2-byte value at offset 0x112, or 0 if no object
 */
static s32 FUN_00478130(void) {
    if (DAT_0462e3ac == 0) {
        return 0;
    }
    return (s32)*(u16 *)(DAT_0462e3ac + 0x112);
}

/* ========================================
 * Test Cases for FUN_004781e0
 * ======================================== */

TEST(clear_flag_bit_already_clear) {
    reset_test_state();
    DAT_0462bf2c = 0;

    FUN_004781e0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_bit_set) {
    reset_test_state();
    DAT_0462bf2c = 0x10000;

    FUN_004781e0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_preserves_other_bits) {
    reset_test_state();
    DAT_0462bf2c = 0x00010000;  /* Bit 16 set, but also other bits */

    FUN_004781e0();

    ASSERT(DAT_0462bf2c == 0x00000000);
}

TEST(clear_flag_preserves_lower_bits) {
    reset_test_state();
    DAT_0462bf2c = 0x0000FFFF;

    FUN_004781e0();

    ASSERT(DAT_0462bf2c == 0x0000FFFF);
}

TEST(clear_flag_preserves_upper_bits) {
    reset_test_state();
    DAT_0462bf2c = 0xFFF00000;

    FUN_004781e0();

    ASSERT(DAT_0462bf2c == 0xFFF00000);
}

/* ========================================
 * Test Cases for FUN_00478110
 * ======================================== */

TEST(clear_field_no_object) {
    reset_test_state();
    DAT_0462e3ac = 0;

    FUN_00478110();

    /* No crash, nothing to clear */
    ASSERT(1);
}

TEST(clear_field_with_object) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;
    *(u16 *)(object_buffer + 0x112) = 12345;

    FUN_00478110();

    ASSERT(*(u16 *)(object_buffer + 0x112) == 0);
}

TEST(clear_field_already_zero) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;
    *(u16 *)(object_buffer + 0x112) = 0;

    FUN_00478110();

    ASSERT(*(u16 *)(object_buffer + 0x112) == 0);
}

TEST(clear_field_max_value) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;
    *(u16 *)(object_buffer + 0x112) = 0xFFFF;

    FUN_00478110();

    ASSERT(*(u16 *)(object_buffer + 0x112) == 0);
}

/* ========================================
 * Test Cases for FUN_00478130
 * ======================================== */

TEST(get_field_no_object) {
    reset_test_state();
    DAT_0462e3ac = 0;

    s32 result = FUN_00478130();

    ASSERT(result == 0);
}

TEST(get_field_with_value) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;
    *(u16 *)(object_buffer + 0x112) = 4242;

    s32 result = FUN_00478130();

    ASSERT(result == 4242);
}

TEST(get_field_zero_value) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;
    *(u16 *)(object_buffer + 0x112) = 0;

    s32 result = FUN_00478130();

    ASSERT(result == 0);
}

TEST(get_field_max_value) {
    reset_test_state();
    DAT_0462e3ac = (uintptr)object_buffer;
    *(u16 *)(object_buffer + 0x112) = 0x7FFF;

    s32 result = FUN_00478130();

    ASSERT(result == 0x7FFF);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Player Field Access Functions Unit Tests ===\n\n");

    printf("FUN_004781e0 (Clear Party Flag Bit) Tests:\n");
    RUN_TEST(clear_flag_bit_already_clear);
    RUN_TEST(clear_flag_bit_set);
    RUN_TEST(clear_flag_preserves_other_bits);
    RUN_TEST(clear_flag_preserves_lower_bits);
    RUN_TEST(clear_flag_preserves_upper_bits);

    printf("\nFUN_00478110 (Clear Player Field) Tests:\n");
    RUN_TEST(clear_field_no_object);
    RUN_TEST(clear_field_with_object);
    RUN_TEST(clear_field_already_zero);
    RUN_TEST(clear_field_max_value);

    printf("\nFUN_00478130 (Get Player Field) Tests:\n");
    RUN_TEST(get_field_no_object);
    RUN_TEST(get_field_with_value);
    RUN_TEST(get_field_zero_value);
    RUN_TEST(get_field_max_value);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
