/*
 * Stone Age Client - Peek Second Hex Function Unit Tests
 * Tests for FUN_00476150 - Peek second hex value from buffer
 * This function reads two hex values and returns the second one,
 * preserving the buffer position
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;
typedef unsigned char u8;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Simulated data buffer */
static char DAT_004e1118[256] = {0};
static u32 DAT_0461c678 = 0;

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
    memset(DAT_004e1118, 0, sizeof(DAT_004e1118));
    DAT_0461c678 = 0;
}

/*
 * FUN_00476860 - Parse hex string to integer (stub for testing)
 * Parses hex characters (0-9, A-F) from DAT_004e1118 buffer
 * Returns -1 if no valid hex character found
 */
static s32 FUN_00476860(void) {
    s32 iVar1;
    s32 iVar2;
    s32 iVar3;

    /* Skip non-hex characters */
    do {
        iVar1 = (s32)(u8)DAT_004e1118[DAT_0461c678];
        DAT_0461c678 = DAT_0461c678 + 1;
        if (iVar1 == 0) {
            return -1;
        }
    } while (((iVar1 < '0') || (iVar1 > '9')) &&
             ((iVar1 < 'A') || (iVar1 > 'F')));

    /* Convert first hex char */
    if (iVar1 < 'A') {
        iVar1 = iVar1 - '0';
    } else {
        iVar1 = iVar1 - 'A' + 10;
    }

    /* Parse remaining hex chars */
    while (1) {
        iVar2 = (s32)(u8)DAT_004e1118[DAT_0461c678];
        iVar3 = DAT_0461c678 + 1;

        if (iVar2 == 0) {
            break;
        }
        if (((iVar2 < '0') || (iVar2 > '9')) &&
            ((iVar2 < 'A') || (iVar2 > 'F'))) {
            DAT_0461c678 = iVar3;
            break;
        }

        if (iVar2 < 'A') {
            iVar1 = iVar1 * 16 + (iVar2 - '0');
        } else {
            iVar1 = iVar1 * 16 + (iVar2 - 'A' + 10);
        }
    }

    return iVar1;
}

/*
 * FUN_00476150 - Peek second hex value
 * Reads two hex values, returns second one, preserves position
 * Returns 0 if first value is -1 or 0xFF
 */
static s32 FUN_00476150(void) {
    u32 uVar1;
    s32 iVar2;
    u32 uVar3;

    uVar1 = DAT_0461c678;
    iVar2 = FUN_00476860();
    if ((iVar2 != -1) && (iVar2 != 0xff)) {
        uVar3 = FUN_00476860();
        DAT_0461c678 = uVar1;
        return uVar3;
    }
    DAT_0461c678 = uVar1;
    return 0;
}

/* ========================================
 * Test Cases for FUN_00476150
 * ======================================== */

TEST(peek_second_empty_buffer) {
    reset_test_state();
    DAT_004e1118[0] = '\0';
    DAT_0461c678 = 0;

    s32 result = FUN_00476150();

    ASSERT(result == 0);
    ASSERT(DAT_0461c678 == 0);  /* Position preserved */
}

TEST(peek_second_single_value) {
    reset_test_state();
    strcpy(DAT_004e1118, "AB");
    DAT_0461c678 = 0;

    s32 result = FUN_00476150();

    /* Only one hex value, can't get second */
    ASSERT(result == 0);
}

TEST(peek_second_two_values) {
    reset_test_state();
    strcpy(DAT_004e1118, "ABCD");
    DAT_0461c678 = 0;

    s32 result = FUN_00476150();

    /* Should return CD (0xCD = 205) */
    ASSERT(result == 0xCD);
    ASSERT(DAT_0461c678 == 0);  /* Position preserved */
}

TEST(peek_second_first_is_ff) {
    reset_test_state();
    strcpy(DAT_004e1118, "FFAB");
    DAT_0461c678 = 0;

    s32 result = FUN_00476150();

    /* First value is 0xFF, should return 0 */
    ASSERT(result == 0);
}

TEST(peek_second_preserves_position) {
    reset_test_state();
    strcpy(DAT_004e1118, "123456");
    DAT_0461c678 = 0;

    u32 pos_before = DAT_0461c678;
    FUN_00476150();
    u32 pos_after = DAT_0461c678;

    ASSERT(pos_after == pos_before);
}

TEST(peek_second_from_offset) {
    reset_test_state();
    strcpy(DAT_004e1118, "XXXXAABB");
    DAT_0461c678 = 4;

    s32 result = FUN_00476150();

    /* Should return BB (0xBB = 187) */
    ASSERT(result == 0xBB);
    ASSERT(DAT_0461c678 == 4);  /* Position preserved */
}

TEST(peek_second_with_separators) {
    reset_test_state();
    strcpy(DAT_004e1118, "12|34");
    DAT_0461c678 = 0;

    s32 result = FUN_00476150();

    /* First hex: 12, then | stops, second hex: 34 */
    ASSERT(result == 0x34);
}

TEST(peek_second_zero_first) {
    reset_test_state();
    strcpy(DAT_004e1118, "00AB");
    DAT_0461c678 = 0;

    s32 result = FUN_00476150();

    /* First value is 0, not 0xFF or -1, so return second */
    ASSERT(result == 0xAB);
}

TEST(peek_second_negative_one_first) {
    reset_test_state();
    DAT_004e1118[0] = '\0';  /* Empty returns -1 */
    DAT_0461c678 = 0;

    s32 result = FUN_00476150();

    ASSERT(result == 0);
}

TEST(peek_second_multiple_calls) {
    reset_test_state();
    strcpy(DAT_004e1118, "AABBCC");
    DAT_0461c678 = 0;

    s32 result1 = FUN_00476150();
    s32 result2 = FUN_00476150();

    /* Both should return BB since position is preserved */
    ASSERT(result1 == 0xBB);
    ASSERT(result2 == 0xBB);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Peek Second Hex Function Unit Tests ===\n\n");

    printf("FUN_00476150 (Peek Second Hex) Tests:\n");
    RUN_TEST(peek_second_empty_buffer);
    RUN_TEST(peek_second_single_value);
    RUN_TEST(peek_second_two_values);
    RUN_TEST(peek_second_first_is_ff);
    RUN_TEST(peek_second_preserves_position);
    RUN_TEST(peek_second_from_offset);
    RUN_TEST(peek_second_with_separators);
    RUN_TEST(peek_second_zero_first);
    RUN_TEST(peek_second_negative_one_first);
    RUN_TEST(peek_second_multiple_calls);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
