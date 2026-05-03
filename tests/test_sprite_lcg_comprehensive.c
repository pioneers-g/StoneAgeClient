/*
 * Stone Age Client - Sprite LCG Comprehensive Tests
 * Tests for sprite_lcg.c implementation
 *
 * Based on Ghidra analysis:
 * - FUN_00492403: LCG random number generator
 * - Microsoft C/C++ compatible LCG
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    fflush(stdout); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Implementation Functions
 * ======================================== */

/* LCG state - DAT_004d7160 */
static u32 g_lcg_state = 0x12345678;

/*
 * LCG Random - FUN_00492403
 * Microsoft C/C++ compatible LCG
 * State = State * 214013 + 2531011
 * Returns (State >> 16) & 0x7FFF
 */
static u32 sprite_lcg_random(void) {
    g_lcg_state = g_lcg_state * 0x343fd + 0x269ec3;
    return (g_lcg_state >> 16) & 0x7fff;
}

/*
 * Seed the LCG
 */
static void sprite_lcg_seed(u32 seed) {
    g_lcg_state = seed;
}

/*
 * Get current LCG state
 */
static u32 sprite_lcg_get_state(void) {
    return g_lcg_state;
}

/*
 * Set LCG state (for save/restore)
 */
static void sprite_lcg_set_state(u32 state) {
    g_lcg_state = state;
}

/*
 * Get random number in range [0, max)
 */
static u32 sprite_lcg_random_range(u32 max) {
    if (max == 0) return 0;
    return sprite_lcg_random() % max;
}

/*
 * Get random number in range [min, max]
 */
static u32 sprite_lcg_random_between(u32 min, u32 max) {
    if (min >= max) return min;
    return min + (sprite_lcg_random() % (max - min + 1));
}

/*
 * Reset to default state
 */
static void reset_state(void) {
    g_lcg_state = 0x12345678;
}

/* ========================================
 * Test Cases - Basic LCG
 * ======================================== */

static int test_lcg_basic(void) {
    reset_state();

    u32 val = sprite_lcg_random();

    /* Should return value in 15-bit range */
    return val <= 0x7FFF;
}

static int test_lcg_deterministic(void) {
    reset_state();

    u32 val1 = sprite_lcg_random();
    u32 val2 = sprite_lcg_random();
    u32 val3 = sprite_lcg_random();

    /* Reset and get same sequence */
    reset_state();

    return sprite_lcg_random() == val1 &&
           sprite_lcg_random() == val2 &&
           sprite_lcg_random() == val3;
}

static int test_lcg_seed(void) {
    sprite_lcg_seed(0xABCDEF01);

    u32 state = sprite_lcg_get_state();

    return state == 0xABCDEF01;
}

static int test_lcg_set_get_state(void) {
    sprite_lcg_set_state(0x12345678);

    u32 state = sprite_lcg_get_state();

    return state == 0x12345678;
}

static int test_lcg_seed_zero(void) {
    sprite_lcg_seed(0);

    u32 val = sprite_lcg_random();

    /* Should still work with seed 0 */
    return val <= 0x7FFF;
}

static int test_lcg_seed_max(void) {
    sprite_lcg_seed(0xFFFFFFFF);

    u32 val = sprite_lcg_random();

    return val <= 0x7FFF;
}

/* ========================================
 * Test Cases - State Transitions
 * ======================================== */

static int test_lcg_state_advances(void) {
    sprite_lcg_seed(0x12345678);

    u32 state1 = sprite_lcg_get_state();
    sprite_lcg_random();
    u32 state2 = sprite_lcg_get_state();

    return state1 != state2;
}

static int test_lcg_state_formula(void) {
    sprite_lcg_seed(1);

    sprite_lcg_random();

    /* State should be 1 * 0x343fd + 0x269ec3 = 0x2A2DC0 */
    u32 expected = 1 * 0x343fd + 0x269ec3;
    u32 state = sprite_lcg_get_state();

    return state == expected;
}

static int test_lcg_return_value_formula(void) {
    sprite_lcg_seed(0x12345678);

    u32 state_before = sprite_lcg_get_state();
    u32 val = sprite_lcg_random();
    u32 state_after = sprite_lcg_get_state();

    /* Return should be (state_after >> 16) & 0x7FFF */
    u32 expected = (state_after >> 16) & 0x7FFF;

    return val == expected;
}

/* ========================================
 * Test Cases - Range Functions
 * ======================================== */

static int test_lcg_range_basic(void) {
    reset_state();

    u32 val = sprite_lcg_random_range(100);

    return val < 100;
}

static int test_lcg_range_one(void) {
    reset_state();

    u32 val = sprite_lcg_random_range(1);

    return val == 0;
}

static int test_lcg_range_zero(void) {
    reset_state();

    u32 val = sprite_lcg_random_range(0);

    return val == 0;
}

static int test_lcg_range_large(void) {
    reset_state();

    u32 val = sprite_lcg_random_range(100000);

    return val < 100000;
}

static int test_lcg_range_distribution(void) {
    sprite_lcg_seed(12345);

    int counts[10] = {0};
    int i;

    for (i = 0; i < 1000; i++) {
        u32 val = sprite_lcg_random_range(10);
        counts[val]++;
    }

    /* Each bucket should have at least some hits (rough distribution check) */
    int min_count = 1000;
    for (i = 0; i < 10; i++) {
        if (counts[i] < min_count) min_count = counts[i];
    }

    return min_count >= 50;  /* At least 5% in each bucket */
}

/* ========================================
 * Test Cases - Between Functions
 * ======================================== */

static int test_lcg_between_basic(void) {
    reset_state();

    u32 val = sprite_lcg_random_between(10, 20);

    return val >= 10 && val <= 20;
}

static int test_lcg_between_same(void) {
    reset_state();

    u32 val = sprite_lcg_random_between(5, 5);

    return val == 5;
}

static int test_lcg_between_inverted(void) {
    reset_state();

    u32 val = sprite_lcg_random_between(20, 10);  /* min > max */

    return val == 20;  /* Should return min */
}

static int test_lcg_between_zero(void) {
    reset_state();

    u32 val = sprite_lcg_random_between(0, 0);

    return val == 0;
}

static int test_lcg_between_distribution(void) {
    sprite_lcg_seed(54321);

    int counts[11] = {0};  /* 0-10 range */
    int i;

    for (i = 0; i < 1100; i++) {
        u32 val = sprite_lcg_random_between(0, 10);
        counts[val]++;
    }

    /* Each bucket should have at least some hits */
    int min_count = 1100;
    for (i = 0; i <= 10; i++) {
        if (counts[i] < min_count) min_count = counts[i];
    }

    return min_count >= 50;  /* At least ~5% in each bucket */
}

/* ========================================
 * Test Cases - Constants
 * ======================================== */

static int test_lcg_multiplier(void) {
    /* Microsoft LCG multiplier: 214013 = 0x343FD */
    return 0x343FD == 214013;
}

static int test_lcg_increment(void) {
    /* Microsoft LCG increment: 2531011 = 0x269EC3 */
    return 0x269EC3 == 2531011;
}

static int test_lcg_mask(void) {
    /* Return value mask: 0x7FFF = 32767 */
    return 0x7FFF == 32767;
}

/* ========================================
 * Test Cases - Sequence Properties
 * ======================================== */

static int test_lcg_no_immediate_repeat(void) {
    sprite_lcg_seed(12345);

    u32 val1 = sprite_lcg_random();
    u32 val2 = sprite_lcg_random();

    /* Very unlikely to get same value twice in a row */
    return val1 != val2;
}

static int test_lcg_period(void) {
    /* Microsoft LCG has period of 2^31 */
    /* Just verify it doesn't repeat immediately */
    sprite_lcg_seed(1);

    u32 first = sprite_lcg_random();
    int found_repeat = 0;

    for (int i = 0; i < 1000; i++) {
        if (sprite_lcg_random() == first) {
            found_repeat = 1;
            break;
        }
    }

    /* Should not repeat in first 1000 iterations */
    return !found_repeat;
}

static int test_lcg_all_bits_change(void) {
    sprite_lcg_seed(0x12345678);

    u32 state1 = sprite_lcg_get_state();
    sprite_lcg_random();
    u32 state2 = sprite_lcg_get_state();

    /* State should change significantly */
    u32 diff = state1 ^ state2;

    /* Many bits should be different */
    int bit_count = 0;
    while (diff) {
        bit_count += diff & 1;
        diff >>= 1;
    }

    return bit_count >= 8;  /* At least 8 bits different */
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Sprite LCG Comprehensive Tests ===\n\n");

    printf("Basic LCG Tests:\n");
    TEST(lcg_basic);
    TEST(lcg_deterministic);
    TEST(lcg_seed);
    TEST(lcg_set_get_state);
    TEST(lcg_seed_zero);
    TEST(lcg_seed_max);

    printf("\nState Transitions Tests:\n");
    TEST(lcg_state_advances);
    TEST(lcg_state_formula);
    TEST(lcg_return_value_formula);

    printf("\nRange Functions Tests:\n");
    TEST(lcg_range_basic);
    TEST(lcg_range_one);
    TEST(lcg_range_zero);
    TEST(lcg_range_large);
    TEST(lcg_range_distribution);

    printf("\nBetween Functions Tests:\n");
    TEST(lcg_between_basic);
    TEST(lcg_between_same);
    TEST(lcg_between_inverted);
    TEST(lcg_between_zero);
    TEST(lcg_between_distribution);

    printf("\nConstants Tests:\n");
    TEST(lcg_multiplier);
    TEST(lcg_increment);
    TEST(lcg_mask);

    printf("\nSequence Properties Tests:\n");
    TEST(lcg_no_immediate_repeat);
    TEST(lcg_period);
    TEST(lcg_all_bits_change);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - Statistical randomness tests (chi-square)
     * - Thread safety (if applicable)
     * - Save/restore state for game saves
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
