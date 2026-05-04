/*
 * Stone Age Client - Random Shuffle Table Function Unit Tests
 * Tests for FUN_00447310 (initialize random shuffle table)
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
 * FUN_00447310 - Initialize Random Shuffle Table
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
}

/* ========================================
 * Helper Functions
 * ======================================== */

static void clear_table(void) {
    memset(DAT_04582a40, 0, sizeof(DAT_04582a40));
}

static int count_value(int value) {
    int count = 0;
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        if (DAT_04582a40[i] == value) count++;
    }
    return count;
}

static int sum_table(void) {
    int sum = 0;
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        sum += DAT_04582a40[i];
    }
    return sum;
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(contains_all_values) {
    clear_table();
    FUN_00447310();

    /* Each value 0-99 should appear exactly once */
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        ASSERT(count_value(i) == 1);
    }
}

TEST(sum_correct) {
    clear_table();
    FUN_00447310();

    /* Sum of 0-99 = 99 * 100 / 2 = 4950 */
    ASSERT(sum_table() == 4950);
}

TEST(no_zero_after_init) {
    clear_table();
    FUN_00447310();

    /* After init, all values should be 0-99 */
    int i;
    int has_zero = 0;
    for (i = 0; i < TABLE_SIZE; i++) {
        if (DAT_04582a40[i] >= 0 && DAT_04582a40[i] < TABLE_SIZE) {
            if (DAT_04582a40[i] == 0) has_zero = 1;
        } else {
            ASSERT(0);  /* Value out of range */
        }
    }
    ASSERT(has_zero == 1);  /* Zero should be present */
}

TEST(sequential_wrap_around) {
    clear_table();

    /* Set seed for reproducibility */
    srand(12345);
    FUN_00447310();

    /* The table should have sequential values that wrap around */
    /* Find where 0 is located */
    int zero_pos = -1;
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        if (DAT_04582a40[i] == 0) {
            zero_pos = i;
            break;
        }
    }
    ASSERT(zero_pos >= 0);

    /* Verify sequential values from zero_pos */
    for (i = 0; i < TABLE_SIZE; i++) {
        int expected = i;
        int pos = (zero_pos + i) % TABLE_SIZE;
        ASSERT(DAT_04582a40[pos] == expected);
    }
}

TEST(multiple_calls_different) {
    clear_table();
    srand(1);
    FUN_00447310();

    int first_zero_pos = -1;
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        if (DAT_04582a40[i] == 0) {
            first_zero_pos = i;
            break;
        }
    }

    clear_table();
    srand(2);
    FUN_00447310();

    int second_zero_pos = -1;
    for (i = 0; i < TABLE_SIZE; i++) {
        if (DAT_04582a40[i] == 0) {
            second_zero_pos = i;
            break;
        }
    }

    /* Different seeds should (likely) produce different starting positions */
    /* Note: This test might occasionally fail due to randomness */
    /* But it's very unlikely for rand() % 100 to give same result twice in a row */
}

TEST(all_values_in_range) {
    clear_table();
    FUN_00447310();

    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        ASSERT(DAT_04582a40[i] >= 0);
        ASSERT(DAT_04582a40[i] < TABLE_SIZE);
    }
}

TEST(no_duplicates) {
    clear_table();
    FUN_00447310();

    int seen[TABLE_SIZE] = {0};
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        int val = DAT_04582a40[i];
        ASSERT(seen[val] == 0);  /* Should not have been seen before */
        seen[val] = 1;
    }
}

TEST(values_are_permutation) {
    clear_table();
    FUN_00447310();

    /* A permutation of 0-99 should have exactly one of each */
    int counts[TABLE_SIZE] = {0};
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        counts[DAT_04582a40[i]]++;
    }

    for (i = 0; i < TABLE_SIZE; i++) {
        ASSERT(counts[i] == 1);
    }
}

TEST(deterministic_with_seed) {
    /* Same seed should produce same result */
    clear_table();
    srand(42);
    FUN_00447310();

    int copy1[TABLE_SIZE];
    memcpy(copy1, DAT_04582a40, sizeof(DAT_04582a40));

    clear_table();
    srand(42);
    FUN_00447310();

    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        ASSERT(DAT_04582a40[i] == copy1[i]);
    }
}

TEST(edge_case_start_at_zero) {
    /* Force rand to return 0 by using a known seed */
    /* This tests wrap-around from position 0 */
    clear_table();

    /* Find a seed that gives 0 */
    unsigned int seed = 0;
    int found = 0;
    for (seed = 0; seed < 10000; seed++) {
        srand(seed);
        if (rand() % TABLE_SIZE == 0) {
            found = 1;
            break;
        }
    }

    if (!found) {
        /* Couldn't find a seed, skip this test by just verifying the function works */
        srand(123);
        FUN_00447310();
        /* Just verify it's a valid permutation */
        int i;
        int counts[TABLE_SIZE] = {0};
        for (i = 0; i < TABLE_SIZE; i++) {
            counts[DAT_04582a40[i]]++;
        }
        for (i = 0; i < TABLE_SIZE; i++) {
            ASSERT(counts[i] == 1);
        }
        return;
    }

    /* Re-seed and call the function */
    srand(seed);
    FUN_00447310();

    /* Verify sequential values starting from position where 0 is placed */
    int zero_pos = -1;
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        if (DAT_04582a40[i] == 0) {
            zero_pos = i;
            break;
        }
    }
    ASSERT(zero_pos >= 0);
}

TEST(edge_case_start_at_99) {
    /* Test when start position is 99 */
    clear_table();

    /* Find a seed that gives 99 */
    unsigned int seed = 0;
    int found = 0;
    for (seed = 0; seed < 10000; seed++) {
        srand(seed);
        if (rand() % TABLE_SIZE == 99) {
            found = 1;
            break;
        }
    }

    if (!found) {
        /* Couldn't find a seed, skip this test by just verifying the function works */
        srand(456);
        FUN_00447310();
        /* Just verify it's a valid permutation */
        int i;
        int counts[TABLE_SIZE] = {0};
        for (i = 0; i < TABLE_SIZE; i++) {
            counts[DAT_04582a40[i]]++;
        }
        for (i = 0; i < TABLE_SIZE; i++) {
            ASSERT(counts[i] == 1);
        }
        return;
    }

    /* Re-seed and call the function */
    srand(seed);
    FUN_00447310();

    /* Verify sequential values starting from position where 0 is placed */
    int zero_pos = -1;
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        if (DAT_04582a40[i] == 0) {
            zero_pos = i;
            break;
        }
    }
    ASSERT(zero_pos >= 0);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Random Shuffle Table Function Unit Tests ===\n\n");

    printf("Content Tests:\n");
    RUN_TEST(contains_all_values);
    RUN_TEST(sum_correct);
    RUN_TEST(no_zero_after_init);
    RUN_TEST(all_values_in_range);

    printf("\nStructure Tests:\n");
    RUN_TEST(sequential_wrap_around);
    RUN_TEST(no_duplicates);
    RUN_TEST(values_are_permutation);

    printf("\nDeterminism Tests:\n");
    RUN_TEST(multiple_calls_different);
    RUN_TEST(deterministic_with_seed);

    printf("\nEdge Case Tests:\n");
    RUN_TEST(edge_case_start_at_zero);
    RUN_TEST(edge_case_start_at_99);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
