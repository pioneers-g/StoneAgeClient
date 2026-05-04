/*
 * Stone Age Client - Item Count Function Unit Tests
 * Tests for FUN_0046db90 - Count items in data buffer
 * This function counts items by parsing hex values from the buffer
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
static char DAT_004d803c[256] = {0};
static u32 DAT_0461c6c0 = 0;

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
    memset(DAT_004d803c, 0, sizeof(DAT_004d803c));
    DAT_0461c6c0 = 0;
}

/*
 * FUN_0046b8a0 - Parse hex string to integer (stub for testing)
 * Parses hex characters (0-9, A-F) from DAT_004d803c buffer
 * Returns -1 if no valid hex character found
 */
static s32 FUN_0046b8a0(void) {
    s32 iVar1;
    s32 iVar2;
    s32 iVar3;

    /* Skip non-hex characters */
    do {
        iVar1 = (s32)(u8)DAT_004d803c[DAT_0461c6c0];
        DAT_0461c6c0 = DAT_0461c6c0 + 1;
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
        iVar2 = (s32)(u8)DAT_004d803c[DAT_0461c6c0];
        iVar3 = DAT_0461c6c0 + 1;

        if (iVar2 == 0) {
            break;
        }
        if (((iVar2 < '0') || (iVar2 > '9')) &&
            ((iVar2 < 'A') || (iVar2 > 'F'))) {
            DAT_0461c6c0 = iVar3;
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
 * FUN_0046db90 - Count items in data buffer
 * Each item consists of 3 hex values, with optional 4th if 'p' follows
 * Returns count of items found
 */
static s32 FUN_0046db90(void) {
    s32 iVar1;
    s32 iVar2;
    s32 iVar3;

    iVar1 = DAT_0461c6c0;
    iVar3 = 0;
    while (1) {
        iVar2 = FUN_0046b8a0();
        if ((iVar2 == -1) || (iVar2 == 0xff)) break;
        FUN_0046b8a0();
        FUN_0046b8a0();
        if (DAT_004d803c[DAT_0461c6c0] == 'p') {
            FUN_0046b8a0();
        }
        iVar3 = iVar3 + 1;
    }
    DAT_0461c6c0 = iVar1;
    return iVar3;
}

/* ========================================
 * Test Cases for FUN_0046db90 (Item Count)
 * ======================================== */

TEST(count_empty_buffer) {
    reset_test_state();
    DAT_004d803c[0] = '\0';
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046db90();

    ASSERT(result == 0);
}

TEST(count_single_item) {
    reset_test_state();
    /* Item: 3 hex values = 01 02 03 */
    strcpy(DAT_004d803c, "010203FF");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046db90();

    ASSERT(result == 1);
}

TEST(count_multiple_items) {
    reset_test_state();
    /* 3 items, each with 3 hex values */
    strcpy(DAT_004d803c, "010203040506070809FF");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046db90();

    ASSERT(result == 3);
}

TEST(count_item_with_p_suffix) {
    reset_test_state();
    /* 1 item with 'p' suffix (4 hex values total) */
    strcpy(DAT_004d803c, "010203p04FF");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046db90();

    ASSERT(result == 1);
}

TEST(count_mixed_items) {
    reset_test_state();
    /* 2 items: one regular, one with 'p' suffix */
    strcpy(DAT_004d803c, "010203p04050607FF");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046db90();

    ASSERT(result == 2);
}

TEST(count_stops_at_ff) {
    reset_test_state();
    /* Should stop at FF (0xFF value) */
    strcpy(DAT_004d803c, "010203FF040506");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046db90();

    ASSERT(result == 1);
}

TEST(count_stops_at_negative_one) {
    reset_test_state();
    /* Empty buffer returns -1 from hex parser */
    DAT_004d803c[0] = '\0';
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046db90();

    ASSERT(result == 0);
}

TEST(count_preserves_position) {
    reset_test_state();
    strcpy(DAT_004d803c, "010203FF");
    DAT_0461c6c0 = 0;

    u32 pos_before = DAT_0461c6c0;
    FUN_0046db90();
    u32 pos_after = DAT_0461c6c0;

    ASSERT(pos_after == pos_before);
}

TEST(count_from_mid_buffer) {
    reset_test_state();
    strcpy(DAT_004d803c, "XXXX010203FF");
    DAT_0461c6c0 = 4;  /* Start after XXXX */

    s32 result = FUN_0046db90();

    ASSERT(result == 1);
}

TEST(count_five_items) {
    reset_test_state();
    /* 5 items */
    strcpy(DAT_004d803c, "0102030405060708090A0B0C0D0E0FFF");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046db90();

    ASSERT(result == 5);
}

TEST(count_items_with_separators) {
    reset_test_state();
    /* Items with | separators between hex groups */
    strcpy(DAT_004d803c, "01|02|03|FF");
    DAT_0461c6c0 = 0;

    s32 result = FUN_0046db90();

    /* Should count as 1 item (01, 02, 03 before FF stops) */
    ASSERT(result == 1);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Item Count Function Unit Tests ===\n\n");

    printf("FUN_0046db90 (Item Count) Tests:\n");
    RUN_TEST(count_empty_buffer);
    RUN_TEST(count_single_item);
    RUN_TEST(count_multiple_items);
    RUN_TEST(count_item_with_p_suffix);
    RUN_TEST(count_mixed_items);
    RUN_TEST(count_stops_at_ff);
    RUN_TEST(count_stops_at_negative_one);
    RUN_TEST(count_preserves_position);
    RUN_TEST(count_from_mid_buffer);
    RUN_TEST(count_five_items);
    RUN_TEST(count_items_with_separators);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
