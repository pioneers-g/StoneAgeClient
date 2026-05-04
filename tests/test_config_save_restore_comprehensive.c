/*
 * Stone Age Client - Config Save/Restore Functions Unit Tests
 * Tests for FUN_0047ca50, FUN_0047caa0, FUN_0047cb00, FUN_0047cb30, FUN_0047ca20
 * These functions handle configuration save and restore operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;
typedef unsigned char u8;
typedef unsigned short u16;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Configuration data arrays */
static u8 DAT_004d36d8 = 0;
static u8 DAT_004d36dc = 0;
static u8 DAT_004d36e0 = 0;
static u32 DAT_04ebe2a8 = 0;
static u32 DAT_04ebe2ac = 0;
static u32 DAT_04ebe2b0 = 0;
static u32 DAT_04ebe2b4 = 0;

/* Runtime configuration */
static u8 DAT_04633330_0 = 0;
static u8 DAT_04633330_1 = 0;
static u8 DAT_04633330_2 = 0;
static u32 ram0x04633333 = 0;
static u32 DAT_04633337 = 0;
static u32 DAT_0463333b = 0;
static u32 DAT_0463333f = 0;

/* Additional config */
static u16 DAT_04633343 = 0;
static u8 DAT_04633344_0 = 0;
static u8 DAT_04633344_1 = 0;

static u16 DAT_0054a4d0 = 0;
static u8 DAT_004a2674 = 0;
static u8 DAT_004a2678 = 0;

/* Party selection state */
static u16 DAT_0462bf32 = 0;
static u8 DAT_04633326[10] = {0};

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
    DAT_004d36d8 = 0;
    DAT_004d36dc = 0;
    DAT_004d36e0 = 0;
    DAT_04ebe2a8 = 0;
    DAT_04ebe2ac = 0;
    DAT_04ebe2b0 = 0;
    DAT_04ebe2b4 = 0;

    DAT_04633330_0 = 0;
    DAT_04633330_1 = 0;
    DAT_04633330_2 = 0;
    ram0x04633333 = 0;
    DAT_04633337 = 0;
    DAT_0463333b = 0;
    DAT_0463333f = 0;

    DAT_04633343 = 0;
    DAT_04633344_0 = 0;
    DAT_04633344_1 = 0;

    DAT_0054a4d0 = 0;
    DAT_004a2674 = 0;
    DAT_004a2678 = 0;

    DAT_0462bf32 = 0;
    memset(DAT_04633326, 0, sizeof(DAT_04633326));
}

/*
 * FUN_0047ca50 - Load config from storage
 * Copies values from DAT_004d36xx/DAT_04ebe2xxx to runtime DAT_046333xx
 */
static void FUN_0047ca50(void) {
    DAT_04633330_0 = DAT_004d36d8;
    DAT_04633330_1 = DAT_004d36dc;
    DAT_04633330_2 = DAT_004d36e0;
    ram0x04633333 = DAT_04ebe2a8;
    DAT_04633337 = DAT_04ebe2ac;
    DAT_0463333b = DAT_04ebe2b0;
    DAT_0463333f = DAT_04ebe2b4;
}

/*
 * FUN_0047caa0 - Save config to storage
 * Copies values from runtime DAT_046333xx to DAT_004d36xx/DAT_04ebe2xxx
 */
static void FUN_0047caa0(void) {
    DAT_004d36d8 = DAT_04633330_0;
    DAT_004d36dc = DAT_04633330_1;
    DAT_004d36e0 = DAT_04633330_2;
    DAT_04ebe2a8 = ram0x04633333;
    DAT_04ebe2ac = DAT_04633337;
    DAT_04ebe2b0 = DAT_0463333b;
    DAT_04ebe2b4 = DAT_0463333f;
}

/*
 * FUN_0047cb00 - Load additional config
 */
static void FUN_0047cb00(void) {
    DAT_04633343 = DAT_0054a4d0;
    DAT_04633344_0 = DAT_004a2674;
    DAT_04633344_1 = DAT_004a2678;
}

/*
 * FUN_0047cb30 - Save additional config
 */
static void FUN_0047cb30(void) {
    DAT_0054a4d0 = DAT_04633343;
    DAT_004a2678 = DAT_04633344_1;
    DAT_004a2674 = DAT_04633344_0;
}

/*
 * FUN_0047ca20 - Clear party selection
 */
static void FUN_0047ca20(int param_1) {
    if (param_1 >= 0 && param_1 < 2) {
        DAT_0462bf32 = 0xFFFF;
        *(u32*)(DAT_04633326 + param_1 * 5) = 0;
        DAT_04633326[param_1 * 5 + 4] = 0;
    }
}

/* ========================================
 * Test Cases for FUN_0047ca50
 * ======================================== */

TEST(load_config_basic) {
    reset_test_state();

    DAT_004d36d8 = 0x11;
    DAT_004d36dc = 0x22;
    DAT_004d36e0 = 0x33;
    DAT_04ebe2a8 = 0x12345678;
    DAT_04ebe2ac = 0x9ABCDEF0;

    FUN_0047ca50();

    ASSERT(DAT_04633330_0 == 0x11);
    ASSERT(DAT_04633330_1 == 0x22);
    ASSERT(DAT_04633330_2 == 0x33);
    ASSERT(ram0x04633333 == 0x12345678);
    ASSERT(DAT_04633337 == 0x9ABCDEF0);
}

TEST(load_config_zero) {
    reset_test_state();

    FUN_0047ca50();

    ASSERT(DAT_04633330_0 == 0);
    ASSERT(DAT_04633330_1 == 0);
    ASSERT(DAT_04633330_2 == 0);
    ASSERT(ram0x04633333 == 0);
}

TEST(load_config_overwrites) {
    reset_test_state();

    DAT_04633330_0 = 0xFF;
    DAT_04633330_1 = 0xFF;
    ram0x04633333 = 0xFFFFFFFF;

    DAT_004d36d8 = 0x01;
    DAT_04ebe2a8 = 0x00000001;

    FUN_0047ca50();

    ASSERT(DAT_04633330_0 == 0x01);
    ASSERT(ram0x04633333 == 0x00000001);
}

/* ========================================
 * Test Cases for FUN_0047caa0
 * ======================================== */

TEST(save_config_basic) {
    reset_test_state();

    DAT_04633330_0 = 0xAA;
    DAT_04633330_1 = 0xBB;
    DAT_04633330_2 = 0xCC;
    ram0x04633333 = 0xDEADBEEF;

    FUN_0047caa0();

    ASSERT(DAT_004d36d8 == 0xAA);
    ASSERT(DAT_004d36dc == 0xBB);
    ASSERT(DAT_004d36e0 == 0xCC);
    ASSERT(DAT_04ebe2a8 == 0xDEADBEEF);
}

TEST(save_config_zero) {
    reset_test_state();

    FUN_0047caa0();

    ASSERT(DAT_004d36d8 == 0);
    ASSERT(DAT_004d36dc == 0);
    ASSERT(DAT_004d36e0 == 0);
}

/* ========================================
 * Test Cases for FUN_0047cb00
 * ======================================== */

TEST(load_additional_config_basic) {
    reset_test_state();

    DAT_0054a4d0 = 0x1234;
    DAT_004a2674 = 0x56;
    DAT_004a2678 = 0x78;

    FUN_0047cb00();

    ASSERT(DAT_04633343 == 0x1234);
    ASSERT(DAT_04633344_0 == 0x56);
    ASSERT(DAT_04633344_1 == 0x78);
}

TEST(load_additional_config_zero) {
    reset_test_state();

    FUN_0047cb00();

    ASSERT(DAT_04633343 == 0);
    ASSERT(DAT_04633344_0 == 0);
    ASSERT(DAT_04633344_1 == 0);
}

/* ========================================
 * Test Cases for FUN_0047cb30
 * ======================================== */

TEST(save_additional_config_basic) {
    reset_test_state();

    DAT_04633343 = 0xABCD;
    DAT_04633344_0 = 0x12;
    DAT_04633344_1 = 0x34;

    FUN_0047cb30();

    ASSERT(DAT_0054a4d0 == 0xABCD);
    ASSERT(DAT_004a2674 == 0x12);
    ASSERT(DAT_004a2678 == 0x34);
}

TEST(save_additional_config_zero) {
    reset_test_state();

    FUN_0047cb30();

    ASSERT(DAT_0054a4d0 == 0);
    ASSERT(DAT_004a2674 == 0);
    ASSERT(DAT_004a2678 == 0);
}

/* ========================================
 * Test Cases for FUN_0047ca20
 * ======================================== */

TEST(clear_party_selection_param0) {
    reset_test_state();

    DAT_0462bf32 = 0x1234;
    DAT_04633326[0] = 0xFF;
    DAT_04633326[1] = 0xFF;
    DAT_04633326[2] = 0xFF;
    DAT_04633326[3] = 0xFF;
    DAT_04633326[4] = 0xFF;

    FUN_0047ca20(0);

    ASSERT(DAT_0462bf32 == 0xFFFF);
    ASSERT(DAT_04633326[0] == 0);
    ASSERT(DAT_04633326[1] == 0);
    ASSERT(DAT_04633326[2] == 0);
    ASSERT(DAT_04633326[3] == 0);
    ASSERT(DAT_04633326[4] == 0);
}

TEST(clear_party_selection_param1) {
    reset_test_state();

    DAT_0462bf32 = 0x1234;
    DAT_04633326[5] = 0xFF;
    DAT_04633326[6] = 0xFF;
    DAT_04633326[7] = 0xFF;
    DAT_04633326[8] = 0xFF;
    DAT_04633326[9] = 0xFF;

    FUN_0047ca20(1);

    ASSERT(DAT_0462bf32 == 0xFFFF);
    ASSERT(DAT_04633326[5] == 0);
    ASSERT(DAT_04633326[6] == 0);
    ASSERT(DAT_04633326[7] == 0);
    ASSERT(DAT_04633326[8] == 0);
    ASSERT(DAT_04633326[9] == 0);
}

TEST(clear_party_selection_invalid_param) {
    reset_test_state();

    DAT_0462bf32 = 0x1234;
    DAT_04633326[0] = 0xFF;

    FUN_0047ca20(2);

    /* Should not change anything for invalid param */
    ASSERT(DAT_0462bf32 == 0x1234);
    ASSERT(DAT_04633326[0] == 0xFF);
}

TEST(clear_party_selection_negative_param) {
    reset_test_state();

    DAT_0462bf32 = 0x1234;

    FUN_0047ca20(-1);

    /* Should not change anything for negative param */
    ASSERT(DAT_0462bf32 == 0x1234);
}

TEST(clear_party_selection_independence) {
    reset_test_state();

    DAT_04633326[0] = 0xFF;
    DAT_04633326[5] = 0xFF;

    FUN_0047ca20(0);

    /* param 0 should only clear first 5 bytes */
    ASSERT(DAT_04633326[0] == 0);
    ASSERT(DAT_04633326[5] == 0xFF);
}

/* ========================================
 * Round-trip Tests
 * ======================================== */

TEST(config_roundtrip) {
    reset_test_state();

    /* Set storage values */
    DAT_004d36d8 = 0x12;
    DAT_004d36dc = 0x34;
    DAT_004d36e0 = 0x56;
    DAT_04ebe2a8 = 0x11111111;
    DAT_04ebe2ac = 0x22222222;
    DAT_04ebe2b0 = 0x33333333;
    DAT_04ebe2b4 = 0x44444444;

    /* Load to runtime */
    FUN_0047ca50();

    /* Modify runtime */
    DAT_04633330_0 = 0xAB;
    ram0x04633333 = 0xABCDEF01;

    /* Save back */
    FUN_0047caa0();

    /* Verify */
    ASSERT(DAT_004d36d8 == 0xAB);
    ASSERT(DAT_004d36dc == 0x34);
    ASSERT(DAT_04ebe2a8 == 0xABCDEF01);
    ASSERT(DAT_04ebe2ac == 0x22222222);
}

TEST(additional_config_roundtrip) {
    reset_test_state();

    /* Set storage values */
    DAT_0054a4d0 = 0x1111;
    DAT_004a2674 = 0x22;
    DAT_004a2678 = 0x33;

    /* Load to runtime */
    FUN_0047cb00();

    /* Modify runtime */
    DAT_04633343 = 0xAAAA;
    DAT_04633344_0 = 0xBB;
    DAT_04633344_1 = 0xCC;

    /* Save back */
    FUN_0047cb30();

    /* Verify */
    ASSERT(DAT_0054a4d0 == 0xAAAA);
    ASSERT(DAT_004a2674 == 0xBB);
    ASSERT(DAT_004a2678 == 0xCC);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Config Save/Restore Functions Unit Tests ===\n\n");

    printf("FUN_0047ca50 (Load Config) Tests:\n");
    RUN_TEST(load_config_basic);
    RUN_TEST(load_config_zero);
    RUN_TEST(load_config_overwrites);

    printf("\nFUN_0047caa0 (Save Config) Tests:\n");
    RUN_TEST(save_config_basic);
    RUN_TEST(save_config_zero);

    printf("\nFUN_0047cb00 (Load Additional Config) Tests:\n");
    RUN_TEST(load_additional_config_basic);
    RUN_TEST(load_additional_config_zero);

    printf("\nFUN_0047cb30 (Save Additional Config) Tests:\n");
    RUN_TEST(save_additional_config_basic);
    RUN_TEST(save_additional_config_zero);

    printf("\nFUN_0047ca20 (Clear Party Selection) Tests:\n");
    RUN_TEST(clear_party_selection_param0);
    RUN_TEST(clear_party_selection_param1);
    RUN_TEST(clear_party_selection_invalid_param);
    RUN_TEST(clear_party_selection_negative_param);
    RUN_TEST(clear_party_selection_independence);

    printf("\nRound-trip Tests:\n");
    RUN_TEST(config_roundtrip);
    RUN_TEST(additional_config_roundtrip);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
