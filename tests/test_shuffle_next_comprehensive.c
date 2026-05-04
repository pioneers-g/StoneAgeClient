/*
 * Stone Age Client - Shuffle Next Value Function Unit Tests
 * Tests for FUN_00447340 (get next value from shuffle table)
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

#define TABLE_SIZE 100

static int DAT_04582a40[TABLE_SIZE];
static int DAT_04582bd0 = 0;

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

/*
 * FUN_00447340 - Get Next Shuffle Value
 */
static int FUN_00447340(void) {
    int* ptr = &DAT_04582a40[DAT_04582bd0];
    int result = *ptr;

    DAT_04582bd0++;
    if (DAT_04582bd0 > 99) {
        DAT_04582bd0 = 0;
    }

    return result;
}

/*
 * FUN_00447310 - Initialize Shuffle Table (helper)
 */
static void FUN_00447310(void) {
    int start = rand() % TABLE_SIZE;
    int i;

    for (i = 0; i < TABLE_SIZE; i++) {
        DAT_04582a40[start] = i;
        start++;
        if (start > 99) {
            start = 0;
        }
    }
    DAT_04582bd0 = 0;
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(returns_valid_value) {
    FUN_00447310();
    int val = FUN_00447340();
    ASSERT(val >= 0 && val < TABLE_SIZE);
}

TEST(increments_index) {
    FUN_00447310();
    int index_before = DAT_04582bd0;
    FUN_00447340();
    ASSERT(DAT_04582bd0 == index_before + 1);
}

TEST(wraps_at_100) {
    FUN_00447310();
    /* Set index to 99 */
    DAT_04582bd0 = 99;
    FUN_00447340();
    ASSERT(DAT_04582bd0 == 0);
}

TEST(returns_all_values) {
    FUN_00447310();
    int seen[TABLE_SIZE] = {0};
    int i;

    for (i = 0; i < TABLE_SIZE; i++) {
        int val = FUN_00447340();
        ASSERT(val >= 0 && val < TABLE_SIZE);
        seen[val] = 1;
    }

    /* All values should have been seen exactly once */
    for (i = 0; i < TABLE_SIZE; i++) {
        ASSERT(seen[i] == 1);
    }
}

TEST(cycle_repeats) {
    FUN_00447310();

    /* Get first 100 values */
    int first_cycle[TABLE_SIZE];
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        first_cycle[i] = FUN_00447340();
    }

    /* Get second 100 values - should be the same sequence */
    for (i = 0; i < TABLE_SIZE; i++) {
        int val = FUN_00447340();
        ASSERT(val == first_cycle[i]);
    }
}

TEST(index_wraps_correctly) {
    FUN_00447310();

    /* Set index near wrap point */
    DAT_04582bd0 = 98;

    int val1 = FUN_00447340();
    ASSERT(DAT_04582bd0 == 99);

    int val2 = FUN_00447340();
    ASSERT(DAT_04582bd0 == 0);

    int val3 = FUN_00447340();
    ASSERT(DAT_04582bd0 == 1);

    /* All values should be valid */
    ASSERT(val1 >= 0 && val1 < TABLE_SIZE);
    ASSERT(val2 >= 0 && val2 < TABLE_SIZE);
    ASSERT(val3 >= 0 && val3 < TABLE_SIZE);
}

TEST(sequential_after_init) {
    /* Initialize with deterministic seed */
    srand(42);
    FUN_00447310();

    /* Find where 0 is */
    int zero_pos = -1;
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        if (DAT_04582a40[i] == 0) {
            zero_pos = i;
            break;
        }
    }

    /* Set index to zero_pos */
    DAT_04582bd0 = zero_pos;

    /* Values should be sequential from this point */
    for (i = 0; i < TABLE_SIZE; i++) {
        int val = FUN_00447340();
        ASSERT(val == i);
    }
}

TEST(multiple_cycles) {
    FUN_00447310();

    int cycles = 5;
    int total = cycles * TABLE_SIZE;
    int* results = (int*)malloc(total * sizeof(int));

    int i;
    for (i = 0; i < total; i++) {
        results[i] = FUN_00447340();
    }

    /* Each cycle should repeat the same pattern */
    for (i = 0; i < TABLE_SIZE; i++) {
        int expected = results[i];
        int j;
        for (j = 1; j < cycles; j++) {
            ASSERT(results[i + j * TABLE_SIZE] == expected);
        }
    }

    free(results);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Shuffle Next Value Function Unit Tests ===\n\n");

    printf("Basic Tests:\n");
    RUN_TEST(returns_valid_value);
    RUN_TEST(increments_index);
    RUN_TEST(wraps_at_100);

    printf("\nSequence Tests:\n");
    RUN_TEST(returns_all_values);
    RUN_TEST(cycle_repeats);
    RUN_TEST(index_wraps_correctly);

    printf("\nPattern Tests:\n");
    RUN_TEST(sequential_after_init);
    RUN_TEST(multiple_cycles);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
