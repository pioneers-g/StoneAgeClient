/*
 * Stone Age Client - Sprite Cache Functions Unit Tests
 * Tests for FUN_00480870, FUN_004808c0
 * These functions manage sprite cache initialization and clearing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;
typedef unsigned short ushort;
typedef unsigned char byte;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Sprite cache size constant */
#define SPRITE_CACHE_MAX_ENTRIES 1000
#define SPRITE_ENTRY_SIZE 0x1c  /* 28 bytes per entry */

/* Sprite cache state */
static u32 DAT_0466b7c8 = 0;        /* Current cache index */
static u32 DAT_0466b7cc = 0;        /* Cache size/count */

/* Sprite entry arrays - simplified for testing */
typedef struct {
    u32 field_00;    /* DAT_0464f7c8 + i*0x1c */
    u32 field_04;    /* DAT_0464f7cc + i*0x1c */
    u32 field_08;    /* DAT_0464f7d0 + i*0x1c */
    u32 field_0c;    /* DAT_0464f7d4 + i*0x1c */
    u32 field_10;    /* DAT_0464f7d8 + i*0x1c */
    u32 field_14;    /* DAT_0465d7d0 + i*0x1c */
    u32 field_18;    /* DAT_0465d7d8 + i*0x1c */
} SpriteEntry;

static SpriteEntry sprite_entries[SPRITE_CACHE_MAX_ENTRIES];

/* Linked list arrays for FUN_004808c0 */
#define LINKED_LIST_SIZE 1000
static u32 linked_list_prev[LINKED_LIST_SIZE];  /* DAT_0466b7e0 area */
static u32 linked_list_next[LINKED_LIST_SIZE];  /* DAT_0466b7e4 area */

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
    DAT_0466b7c8 = 0;
    DAT_0466b7cc = 0;
    memset(sprite_entries, 0, sizeof(sprite_entries));
    memset(linked_list_prev, 0, sizeof(linked_list_prev));
    memset(linked_list_next, 0, sizeof(linked_list_next));
}

/*
 * FUN_00480870 - Reset sprite cache
 * Clears the cache entry arrays with -1 and 0 values
 * Iterates through DAT_0466b7cc entries, each 0x1c (28) bytes apart
 */
static void FUN_00480870(void) {
    u32 i;
    DAT_0466b7c8 = 0;

    for (i = 0; i < DAT_0466b7cc; i++) {
        sprite_entries[i].field_04 = 0xFFFFFFFF;  /* -1 */
        sprite_entries[i].field_14 = 0xFFFFFFFF;  /* -1 */
        sprite_entries[i].field_08 = 0;
        sprite_entries[i].field_18 = 0;
        sprite_entries[i].field_10 = 0;
        sprite_entries[i].field_00 = 0;
        sprite_entries[i].field_0c = 0;
    }
}

/*
 * FUN_004808c0 - Clear sprite linked lists
 * Clears pairs of values in linked list arrays
 * Iterates with stride of 3 u32 values (12 bytes)
 */
static void FUN_004808c0(void) {
    int i;
    for (i = 0; i < LINKED_LIST_SIZE; i++) {
        linked_list_prev[i] = 0;
        linked_list_next[i] = 0;
    }
}

/* ========================================
 * Test Cases for FUN_00480870
 * ======================================== */

TEST(reset_cache_basic) {
    reset_test_state();
    DAT_0466b7cc = 5;

    /* Set some non-zero values */
    for (int i = 0; i < 5; i++) {
        sprite_entries[i].field_00 = 12345;
        sprite_entries[i].field_04 = 99999;
        sprite_entries[i].field_08 = 11111;
    }

    FUN_00480870();

    /* Check that entries are cleared */
    for (int i = 0; i < 5; i++) {
        ASSERT(sprite_entries[i].field_00 == 0);
        ASSERT(sprite_entries[i].field_04 == 0xFFFFFFFF);
        ASSERT(sprite_entries[i].field_08 == 0);
    }
    ASSERT(DAT_0466b7c8 == 0);
}

TEST(reset_cache_zero_count) {
    reset_test_state();
    DAT_0466b7cc = 0;
    DAT_0466b7c8 = 100;

    FUN_00480870();

    /* Should do nothing except reset the index */
    ASSERT(DAT_0466b7c8 == 0);
}

TEST(reset_cache_single_entry) {
    reset_test_state();
    DAT_0466b7cc = 1;

    sprite_entries[0].field_00 = 0xDEADBEEF;
    sprite_entries[0].field_04 = 0xCAFEBABE;
    sprite_entries[0].field_08 = 0x12345678;
    sprite_entries[0].field_0c = 0x87654321;
    sprite_entries[0].field_10 = 0xFFFFFFFF;
    sprite_entries[0].field_14 = 0x00000000;
    sprite_entries[0].field_18 = 0x11111111;

    FUN_00480870();

    ASSERT(sprite_entries[0].field_00 == 0);
    ASSERT(sprite_entries[0].field_04 == 0xFFFFFFFF);
    ASSERT(sprite_entries[0].field_08 == 0);
    ASSERT(sprite_entries[0].field_0c == 0);
    ASSERT(sprite_entries[0].field_10 == 0);
    ASSERT(sprite_entries[0].field_14 == 0xFFFFFFFF);
    ASSERT(sprite_entries[0].field_18 == 0);
}

TEST(reset_cache_multiple_entries) {
    reset_test_state();
    DAT_0466b7cc = 10;

    /* Fill entries with various values */
    for (int i = 0; i < 10; i++) {
        sprite_entries[i].field_00 = i * 100 + 1;
        sprite_entries[i].field_04 = i * 100 + 2;
        sprite_entries[i].field_08 = i * 100 + 3;
        sprite_entries[i].field_0c = i * 100 + 4;
        sprite_entries[i].field_10 = i * 100 + 5;
        sprite_entries[i].field_14 = i * 100 + 6;
        sprite_entries[i].field_18 = i * 100 + 7;
    }

    FUN_00480870();

    /* Verify all entries cleared correctly */
    for (int i = 0; i < 10; i++) {
        ASSERT(sprite_entries[i].field_00 == 0);
        ASSERT(sprite_entries[i].field_04 == 0xFFFFFFFF);
        ASSERT(sprite_entries[i].field_08 == 0);
        ASSERT(sprite_entries[i].field_0c == 0);
        ASSERT(sprite_entries[i].field_10 == 0);
        ASSERT(sprite_entries[i].field_14 == 0xFFFFFFFF);
        ASSERT(sprite_entries[i].field_18 == 0);
    }
}

TEST(reset_cache_preserves_beyond_count) {
    reset_test_state();
    DAT_0466b7cc = 3;

    /* Set values beyond the count */
    sprite_entries[5].field_00 = 99999;
    sprite_entries[5].field_04 = 88888;

    FUN_00480870();

    /* Entries beyond count should be unchanged */
    ASSERT(sprite_entries[5].field_00 == 99999);
    ASSERT(sprite_entries[5].field_04 == 88888);
}

TEST(reset_cache_clears_current_index) {
    reset_test_state();
    DAT_0466b7cc = 5;
    DAT_0466b7c8 = 100;

    FUN_00480870();

    ASSERT(DAT_0466b7c8 == 0);
}

/* ========================================
 * Test Cases for FUN_004808c0
 * ======================================== */

TEST(clear_linked_lists_basic) {
    reset_test_state();

    /* Set some non-zero values */
    for (int i = 0; i < 10; i++) {
        linked_list_prev[i] = i * 100;
        linked_list_next[i] = i * 200;
    }

    FUN_004808c0();

    /* Check all are cleared */
    for (int i = 0; i < 10; i++) {
        ASSERT(linked_list_prev[i] == 0);
        ASSERT(linked_list_next[i] == 0);
    }
}

TEST(clear_linked_lists_full_range) {
    reset_test_state();

    /* Fill entire arrays with non-zero values */
    for (int i = 0; i < LINKED_LIST_SIZE; i++) {
        linked_list_prev[i] = 0xDEADBEEF;
        linked_list_next[i] = 0xCAFEBABE;
    }

    FUN_004808c0();

    /* Verify all cleared */
    for (int i = 0; i < LINKED_LIST_SIZE; i++) {
        ASSERT(linked_list_prev[i] == 0);
        ASSERT(linked_list_next[i] == 0);
    }
}

TEST(clear_linked_lists_already_zero) {
    reset_test_state();

    /* Arrays are already zero from reset */
    FUN_004808c0();

    /* Should still be zero */
    for (int i = 0; i < 100; i++) {
        ASSERT(linked_list_prev[i] == 0);
        ASSERT(linked_list_next[i] == 0);
    }
}

TEST(clear_linked_lists_max_values) {
    reset_test_state();

    /* Set max values */
    linked_list_prev[0] = 0xFFFFFFFF;
    linked_list_next[0] = 0xFFFFFFFF;
    linked_list_prev[500] = 0xFFFFFFFF;
    linked_list_next[500] = 0xFFFFFFFF;

    FUN_004808c0();

    ASSERT(linked_list_prev[0] == 0);
    ASSERT(linked_list_next[0] == 0);
    ASSERT(linked_list_prev[500] == 0);
    ASSERT(linked_list_next[500] == 0);
}

/* ========================================
 * Combined Tests
 * ======================================== */

TEST(combined_reset_and_clear) {
    reset_test_state();
    DAT_0466b7cc = 5;

    /* Fill both structures with data */
    for (int i = 0; i < 5; i++) {
        sprite_entries[i].field_00 = i + 1;
        sprite_entries[i].field_04 = i + 100;
    }
    for (int i = 0; i < 10; i++) {
        linked_list_prev[i] = i * 10;
        linked_list_next[i] = i * 20;
    }

    FUN_00480870();
    FUN_004808c0();

    /* Both should be cleared */
    for (int i = 0; i < 5; i++) {
        ASSERT(sprite_entries[i].field_00 == 0);
        ASSERT(sprite_entries[i].field_04 == 0xFFFFFFFF);
    }
    for (int i = 0; i < 10; i++) {
        ASSERT(linked_list_prev[i] == 0);
        ASSERT(linked_list_next[i] == 0);
    }
}

TEST(multiple_resets) {
    reset_test_state();
    DAT_0466b7cc = 3;

    /* First reset */
    FUN_00480870();

    /* Fill with data again */
    for (int i = 0; i < 3; i++) {
        sprite_entries[i].field_00 = 111;
        sprite_entries[i].field_04 = 222;
    }

    /* Second reset */
    FUN_00480870();

    /* Should still be cleared */
    for (int i = 0; i < 3; i++) {
        ASSERT(sprite_entries[i].field_00 == 0);
        ASSERT(sprite_entries[i].field_04 == 0xFFFFFFFF);
    }
}

TEST(large_cache_size) {
    reset_test_state();
    DAT_0466b7cc = 500;

    /* Fill large cache */
    for (int i = 0; i < 500; i++) {
        sprite_entries[i].field_00 = i;
        sprite_entries[i].field_04 = 500 - i;
        sprite_entries[i].field_08 = i * 2;
        sprite_entries[i].field_14 = i * 3;
    }

    FUN_00480870();

    /* Verify all cleared */
    for (int i = 0; i < 500; i++) {
        ASSERT(sprite_entries[i].field_00 == 0);
        ASSERT(sprite_entries[i].field_04 == 0xFFFFFFFF);
        ASSERT(sprite_entries[i].field_08 == 0);
        ASSERT(sprite_entries[i].field_14 == 0xFFFFFFFF);
    }
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Sprite Cache Functions Unit Tests ===\n\n");

    printf("FUN_00480870 (Reset Cache) Tests:\n");
    RUN_TEST(reset_cache_basic);
    RUN_TEST(reset_cache_zero_count);
    RUN_TEST(reset_cache_single_entry);
    RUN_TEST(reset_cache_multiple_entries);
    RUN_TEST(reset_cache_preserves_beyond_count);
    RUN_TEST(reset_cache_clears_current_index);

    printf("\nFUN_004808c0 (Clear Linked Lists) Tests:\n");
    RUN_TEST(clear_linked_lists_basic);
    RUN_TEST(clear_linked_lists_full_range);
    RUN_TEST(clear_linked_lists_already_zero);
    RUN_TEST(clear_linked_lists_max_values);

    printf("\nCombined Tests:\n");
    RUN_TEST(combined_reset_and_clear);
    RUN_TEST(multiple_resets);
    RUN_TEST(large_cache_size);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
