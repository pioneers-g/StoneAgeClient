/*
 * Stone Age Client - Flag Manipulation Functions Unit Tests
 * Tests for FUN_00478090, FUN_004780a0, FUN_004780b0, FUN_004780c0,
 *          FUN_004780d0, FUN_004780e0, FUN_00478150, FUN_004781e0
 * These functions manipulate a global flag variable (DAT_0462bf2c)
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;
typedef unsigned char u8;
typedef unsigned short u16;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Global flag variable */
static u32 DAT_0462bf2c = 0;

/* Object pointer for object-related functions */
static u8 g_object_data[0x200] = {0};
static u32 DAT_0462e3ac = 0;

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
    memset(g_object_data, 0, sizeof(g_object_data));
}

/*
 * FUN_00478090 - Set flag 0x100
 * Sets bit 8 of the flag variable
 */
static void FUN_00478090(void) {
    DAT_0462bf2c = DAT_0462bf2c | 0x100;
}

/*
 * FUN_004780a0 - Clear flag 0x100
 * Clears bit 8 of the flag variable
 */
static void FUN_004780a0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xfffffeff;
}

/*
 * FUN_004780b0 - Set flag 0x200
 * Sets bit 9 of the flag variable
 */
static void FUN_004780b0(void) {
    DAT_0462bf2c = DAT_0462bf2c | 0x200;
}

/*
 * FUN_004780c0 - Clear flag 0x200
 * Clears bit 9 of the flag variable
 */
static void FUN_004780c0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xfffffdff;
}

/*
 * FUN_004780d0 - Set flag 0x4000
 * Sets bit 14 of the flag variable
 */
static void FUN_004780d0(void) {
    DAT_0462bf2c = DAT_0462bf2c | 0x4000;
}

/*
 * FUN_004780e0 - Clear flag 0x4000
 * Clears bit 14 of the flag variable
 */
static void FUN_004780e0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xffffbfff;
}

/*
 * FUN_00478150 - Clear flag 0x800
 * Clears bit 11 of the flag variable
 */
static void FUN_00478150(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xfffff7ff;
}

/*
 * FUN_004781e0 - Clear flag 0x10000
 * Clears bit 16 of the flag variable
 */
static void FUN_004781e0(void) {
    DAT_0462bf2c = DAT_0462bf2c & 0xfffeffff;
}

/*
 * FUN_004780f0 - Set object flag at offset 0x112 to 1
 */
static void FUN_004780f0(void) {
    if (DAT_0462e3ac != 0) {
        *(u16*)(DAT_0462e3ac + 0x112) = 1;
    }
}

/*
 * FUN_00478110 - Set object flag at offset 0x112 to 0
 */
static void FUN_00478110(void) {
    if (DAT_0462e3ac != 0) {
        *(u16*)(DAT_0462e3ac + 0x112) = 0;
    }
}

/*
 * FUN_00478130 - Get object flag at offset 0x112
 */
static int FUN_00478130(void) {
    if (DAT_0462e3ac == 0) {
        return 0;
    }
    return (int)*(short*)(DAT_0462e3ac + 0x112);
}

/* ========================================
 * Test Cases for FUN_00478090 (Set 0x100)
 * ======================================== */

TEST(set_flag_100_basic) {
    reset_test_state();

    FUN_00478090();

    ASSERT(DAT_0462bf2c == 0x100);
}

TEST(set_flag_100_preserves_other_bits) {
    reset_test_state();

    DAT_0462bf2c = 0x200;
    FUN_00478090();

    ASSERT(DAT_0462bf2c == 0x300);  /* 0x200 | 0x100 */
}

TEST(set_flag_100_idempotent) {
    reset_test_state();

    FUN_00478090();
    FUN_00478090();

    ASSERT(DAT_0462bf2c == 0x100);
}

/* ========================================
 * Test Cases for FUN_004780a0 (Clear 0x100)
 * ======================================== */

TEST(clear_flag_100_basic) {
    reset_test_state();

    DAT_0462bf2c = 0x100;
    FUN_004780a0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_100_preserves_other_bits) {
    reset_test_state();

    DAT_0462bf2c = 0x300;
    FUN_004780a0();

    ASSERT(DAT_0462bf2c == 0x200);
}

TEST(clear_flag_100_idempotent) {
    reset_test_state();

    DAT_0462bf2c = 0x100;
    FUN_004780a0();
    FUN_004780a0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_100_already_clear) {
    reset_test_state();

    DAT_0462bf2c = 0;
    FUN_004780a0();

    ASSERT(DAT_0462bf2c == 0);
}

/* ========================================
 * Test Cases for FUN_004780b0 (Set 0x200)
 * ======================================== */

TEST(set_flag_200_basic) {
    reset_test_state();

    FUN_004780b0();

    ASSERT(DAT_0462bf2c == 0x200);
}

TEST(set_flag_200_preserves_other_bits) {
    reset_test_state();

    DAT_0462bf2c = 0x100;
    FUN_004780b0();

    ASSERT(DAT_0462bf2c == 0x300);
}

TEST(set_flag_200_idempotent) {
    reset_test_state();

    FUN_004780b0();
    FUN_004780b0();

    ASSERT(DAT_0462bf2c == 0x200);
}

/* ========================================
 * Test Cases for FUN_004780c0 (Clear 0x200)
 * ======================================== */

TEST(clear_flag_200_basic) {
    reset_test_state();

    DAT_0462bf2c = 0x200;
    FUN_004780c0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_200_preserves_other_bits) {
    reset_test_state();

    DAT_0462bf2c = 0x300;
    FUN_004780c0();

    ASSERT(DAT_0462bf2c == 0x100);
}

TEST(clear_flag_200_idempotent) {
    reset_test_state();

    DAT_0462bf2c = 0x200;
    FUN_004780c0();
    FUN_004780c0();

    ASSERT(DAT_0462bf2c == 0);
}

/* ========================================
 * Test Cases for FUN_004780d0 (Set 0x4000)
 * ======================================== */

TEST(set_flag_4000_basic) {
    reset_test_state();

    FUN_004780d0();

    ASSERT(DAT_0462bf2c == 0x4000);
}

TEST(set_flag_4000_preserves_other_bits) {
    reset_test_state();

    DAT_0462bf2c = 0x100;
    FUN_004780d0();

    ASSERT(DAT_0462bf2c == 0x4100);
}

TEST(set_flag_4000_idempotent) {
    reset_test_state();

    FUN_004780d0();
    FUN_004780d0();

    ASSERT(DAT_0462bf2c == 0x4000);
}

/* ========================================
 * Test Cases for FUN_004780e0 (Clear 0x4000)
 * ======================================== */

TEST(clear_flag_4000_basic) {
    reset_test_state();

    DAT_0462bf2c = 0x4000;
    FUN_004780e0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_4000_preserves_other_bits) {
    reset_test_state();

    DAT_0462bf2c = 0x4100;
    FUN_004780e0();

    ASSERT(DAT_0462bf2c == 0x100);
}

TEST(clear_flag_4000_idempotent) {
    reset_test_state();

    DAT_0462bf2c = 0x4000;
    FUN_004780e0();
    FUN_004780e0();

    ASSERT(DAT_0462bf2c == 0);
}

/* ========================================
 * Test Cases for FUN_00478150 (Clear 0x800)
 * ======================================== */

TEST(clear_flag_800_basic) {
    reset_test_state();

    DAT_0462bf2c = 0x800;
    FUN_00478150();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_800_preserves_other_bits) {
    reset_test_state();

    DAT_0462bf2c = 0x900;
    FUN_00478150();

    ASSERT(DAT_0462bf2c == 0x100);
}

TEST(clear_flag_800_idempotent) {
    reset_test_state();

    DAT_0462bf2c = 0x800;
    FUN_00478150();
    FUN_00478150();

    ASSERT(DAT_0462bf2c == 0);
}

/* ========================================
 * Test Cases for FUN_004781e0 (Clear 0x10000)
 * ======================================== */

TEST(clear_flag_10000_basic) {
    reset_test_state();

    DAT_0462bf2c = 0x10000;
    FUN_004781e0();

    ASSERT(DAT_0462bf2c == 0);
}

TEST(clear_flag_10000_preserves_other_bits) {
    reset_test_state();

    DAT_0462bf2c = 0x10100;
    FUN_004781e0();

    ASSERT(DAT_0462bf2c == 0x100);
}

TEST(clear_flag_10000_idempotent) {
    reset_test_state();

    DAT_0462bf2c = 0x10000;
    FUN_004781e0();
    FUN_004781e0();

    ASSERT(DAT_0462bf2c == 0);
}

/* ========================================
 * Test Cases for FUN_004780f0 (Set object flag)
 * ======================================== */

TEST(set_object_flag_no_object) {
    reset_test_state();

    DAT_0462e3ac = 0;
    FUN_004780f0();

    /* Should not crash, no change */
    ASSERT(1);
}

TEST(set_object_flag_with_object) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    FUN_004780f0();

    ASSERT(*(u16*)(g_object_data + 0x112) == 1);
}

/* ========================================
 * Test Cases for FUN_00478110 (Clear object flag)
 * ======================================== */

TEST(clear_object_flag_no_object) {
    reset_test_state();

    DAT_0462e3ac = 0;
    FUN_00478110();

    /* Should not crash, no change */
    ASSERT(1);
}

TEST(clear_object_flag_with_object) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    *(u16*)(g_object_data + 0x112) = 1;
    FUN_00478110();

    ASSERT(*(u16*)(g_object_data + 0x112) == 0);
}

/* ========================================
 * Test Cases for FUN_00478130 (Get object flag)
 * ======================================== */

TEST(get_object_flag_no_object) {
    reset_test_state();

    DAT_0462e3ac = 0;
    int result = FUN_00478130();

    ASSERT(result == 0);
}

TEST(get_object_flag_with_object_zero) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    *(u16*)(g_object_data + 0x112) = 0;
    int result = FUN_00478130();

    ASSERT(result == 0);
}

TEST(get_object_flag_with_object_one) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;
    *(u16*)(g_object_data + 0x112) = 1;
    int result = FUN_00478130();

    ASSERT(result == 1);
}

/* ========================================
 * Round-trip Tests
 * ======================================== */

TEST(flag_100_roundtrip) {
    reset_test_state();

    FUN_00478090();
    ASSERT(DAT_0462bf2c == 0x100);

    FUN_004780a0();
    ASSERT(DAT_0462bf2c == 0);
}

TEST(flag_200_roundtrip) {
    reset_test_state();

    FUN_004780b0();
    ASSERT(DAT_0462bf2c == 0x200);

    FUN_004780c0();
    ASSERT(DAT_0462bf2c == 0);
}

TEST(flag_4000_roundtrip) {
    reset_test_state();

    FUN_004780d0();
    ASSERT(DAT_0462bf2c == 0x4000);

    FUN_004780e0();
    ASSERT(DAT_0462bf2c == 0);
}

TEST(multiple_flags_independent) {
    reset_test_state();

    FUN_00478090();  /* Set 0x100 */
    FUN_004780b0();  /* Set 0x200 */
    FUN_004780d0();  /* Set 0x4000 */

    ASSERT(DAT_0462bf2c == 0x4300);

    FUN_004780a0();  /* Clear 0x100 */
    ASSERT(DAT_0462bf2c == 0x4200);

    FUN_004780c0();  /* Clear 0x200 */
    ASSERT(DAT_0462bf2c == 0x4000);

    FUN_004780e0();  /* Clear 0x4000 */
    ASSERT(DAT_0462bf2c == 0);
}

TEST(object_flag_roundtrip) {
    reset_test_state();

    DAT_0462e3ac = (u32)g_object_data;

    FUN_004780f0();
    ASSERT(FUN_00478130() == 1);

    FUN_00478110();
    ASSERT(FUN_00478130() == 0);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Flag Manipulation Functions Unit Tests ===\n\n");

    printf("FUN_00478090 (Set 0x100) Tests:\n");
    RUN_TEST(set_flag_100_basic);
    RUN_TEST(set_flag_100_preserves_other_bits);
    RUN_TEST(set_flag_100_idempotent);

    printf("\nFUN_004780a0 (Clear 0x100) Tests:\n");
    RUN_TEST(clear_flag_100_basic);
    RUN_TEST(clear_flag_100_preserves_other_bits);
    RUN_TEST(clear_flag_100_idempotent);
    RUN_TEST(clear_flag_100_already_clear);

    printf("\nFUN_004780b0 (Set 0x200) Tests:\n");
    RUN_TEST(set_flag_200_basic);
    RUN_TEST(set_flag_200_preserves_other_bits);
    RUN_TEST(set_flag_200_idempotent);

    printf("\nFUN_004780c0 (Clear 0x200) Tests:\n");
    RUN_TEST(clear_flag_200_basic);
    RUN_TEST(clear_flag_200_preserves_other_bits);
    RUN_TEST(clear_flag_200_idempotent);

    printf("\nFUN_004780d0 (Set 0x4000) Tests:\n");
    RUN_TEST(set_flag_4000_basic);
    RUN_TEST(set_flag_4000_preserves_other_bits);
    RUN_TEST(set_flag_4000_idempotent);

    printf("\nFUN_004780e0 (Clear 0x4000) Tests:\n");
    RUN_TEST(clear_flag_4000_basic);
    RUN_TEST(clear_flag_4000_preserves_other_bits);
    RUN_TEST(clear_flag_4000_idempotent);

    printf("\nFUN_00478150 (Clear 0x800) Tests:\n");
    RUN_TEST(clear_flag_800_basic);
    RUN_TEST(clear_flag_800_preserves_other_bits);
    RUN_TEST(clear_flag_800_idempotent);

    printf("\nFUN_004781e0 (Clear 0x10000) Tests:\n");
    RUN_TEST(clear_flag_10000_basic);
    RUN_TEST(clear_flag_10000_preserves_other_bits);
    RUN_TEST(clear_flag_10000_idempotent);

    printf("\nFUN_004780f0 (Set Object Flag) Tests:\n");
    RUN_TEST(set_object_flag_no_object);
    RUN_TEST(set_object_flag_with_object);

    printf("\nFUN_00478110 (Clear Object Flag) Tests:\n");
    RUN_TEST(clear_object_flag_no_object);
    RUN_TEST(clear_object_flag_with_object);

    printf("\nFUN_00478130 (Get Object Flag) Tests:\n");
    RUN_TEST(get_object_flag_no_object);
    RUN_TEST(get_object_flag_with_object_zero);
    RUN_TEST(get_object_flag_with_object_one);

    printf("\nRound-trip Tests:\n");
    RUN_TEST(flag_100_roundtrip);
    RUN_TEST(flag_200_roundtrip);
    RUN_TEST(flag_4000_roundtrip);
    RUN_TEST(multiple_flags_independent);
    RUN_TEST(object_flag_roundtrip);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
