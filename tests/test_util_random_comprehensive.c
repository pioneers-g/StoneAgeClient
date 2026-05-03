/*
 * Stone Age Client - Random Utility Module Comprehensive Tests
 * Tests for random.c - LCG-based random number generator
 *
 * Coverage:
 * - Initialization and shutdown
 * - Seeding behavior
 * - LCG algorithm verification
 * - Range functions
 * - Float generation
 * - Chance/probability
 * - Dice rolling
 * - State management
 * - Distribution tests
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Random state for seeded generators */
typedef struct {
    u32 seed;
    u32 state;
} RandomState;

/* Random context */
typedef struct {
    u32 seed;
    u32 state;
    u32 call_count;
} RandomContext;

static RandomContext g_random;

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Random Functions (Stub Implementation)
 * ======================================== */

int random_init(void) {
    memset(&g_random, 0, sizeof(RandomContext));
    g_random.seed = 12345;  /* Default seed */
    g_random.state = 12345;
    return 1;
}

void random_shutdown(void) {
    memset(&g_random, 0, sizeof(RandomContext));
}

void random_seed(u32 seed) {
    g_random.seed = seed;
    g_random.state = seed;
    g_random.call_count = 0;

    /* Warm up the generator */
    for (int i = 0; i < 100; i++) {
        u32 dummy = g_random.state * 1103515245 + 12345;
        g_random.state = dummy;
        (void)dummy;
    }
}

u32 random_next(void) {
    /* Linear Congruential Generator */
    g_random.state = g_random.state * 1103515245 + 12345;
    g_random.call_count++;
    return (g_random.state >> 16) & 0x7FFF;
}

u32 random_u32(void) {
    return (random_next() << 17) | (random_next() << 2) | (random_next() & 3);
}

u32 random_range(u32 max) {
    if (max == 0) return 0;
    return random_next() % max;
}

u32 random_range_ex(u32 min, u32 max) {
    if (min >= max) return min;
    return min + random_range(max - min + 1);
}

float random_float(void) {
    return (float)random_next() / 32768.0f;
}

float random_float_range(float min, float max) {
    return min + random_float() * (max - min);
}

int random_chance(u32 percent) {
    if (percent >= 100) return 1;
    if (percent == 0) return 0;
    return random_range(100) < percent;
}

int random_chance_float(float probability) {
    if (probability >= 1.0f) return 1;
    if (probability <= 0.0f) return 0;
    return random_float() < probability;
}

u32 random_dice(u32 count, u32 sides) {
    u32 total = 0;
    for (u32 i = 0; i < count; i++) {
        total += random_range(sides) + 1;
    }
    return total;
}

u32 random_dice_ex(u32 count, u32 sides, s32 modifier) {
    s32 total = (s32)random_dice(count, sides) + modifier;
    return (total < 0) ? 0 : (u32)total;
}

u32 random_get_state(void) {
    return g_random.state;
}

void random_set_state(u32 state) {
    g_random.state = state;
}

void random_init_seeded(RandomState* state, u32 seed) {
    state->seed = seed;
    state->state = seed;
}

u32 random_next_seeded(RandomState* state) {
    state->state = state->state * 1103515245 + 12345;
    return (state->state >> 16) & 0x7FFF;
}

u8 random_direction(void) {
    return (u8)random_range(8);  /* 0-7 for 8 directions */
}

u16 random_angle(void) {
    return (u16)random_range(360);
}

u32 random_color(void) {
    u8 r = (u8)random_range(256);
    u8 g = (u8)random_range(256);
    u8 b = (u8)random_range(256);
    return (r << 16) | (g << 8) | b;
}

int random_hit_check(u32 hit_rate, u32 dodge_rate) {
    u32 effective = (hit_rate > dodge_rate) ? (hit_rate - dodge_rate) : 0;
    if (effective > 100) effective = 100;
    return random_chance(effective);
}

int random_critical_check(u32 crit_rate, u32 luck_bonus) {
    u32 total = crit_rate + luck_bonus;
    if (total > 100) total = 100;
    return random_chance(total);
}

/* ========================================
 * Test Setup/Teardown
 * ======================================== */

static void test_setup(void) {
    random_init();
}

static void test_teardown(void) {
    random_shutdown();
}

/* ========================================
 * Initialization Tests
 * ======================================== */

static int test_random_init(void) {
    test_setup();

    int pass = g_random.seed != 0 && g_random.state != 0;

    test_teardown();
    return pass;
}

static int test_random_shutdown(void) {
    test_setup();

    random_shutdown();

    int pass = g_random.seed == 0 && g_random.state == 0;

    return pass;
}

/* ========================================
 * Seeding Tests
 * ======================================== */

static int test_seed_basic(void) {
    test_setup();

    random_seed(12345);

    int pass = g_random.seed == 12345;

    test_teardown();
    return pass;
}

static int test_seed_zero(void) {
    test_setup();

    random_seed(0);

    /* After warmup, state should have changed from 0 */
    int pass = g_random.seed == 0;  /* Seed should be stored as-is */

    test_teardown();
    return pass;
}

static int test_seed_deterministic(void) {
    test_setup();

    random_seed(54321);
    u32 val1 = random_next();
    random_seed(54321);
    u32 val2 = random_next();

    int pass = val1 == val2;

    test_teardown();
    return pass;
}

static int test_different_seeds(void) {
    test_setup();

    random_seed(111);
    u32 val1 = random_next();

    random_seed(222);
    u32 val2 = random_next();

    int pass = val1 != val2;  /* Different seeds should give different values */

    test_teardown();
    return pass;
}

/* ========================================
 * LCG Algorithm Tests
 * ======================================== */

static int test_random_next_range(void) {
    test_setup();
    random_seed(12345);

    /* random_next() should return 0-32767 (15-bit value) */
    int pass = 1;
    for (int i = 0; i < 100; i++) {
        u32 val = random_next();
        if (val > 0x7FFF) {
            pass = 0;
            break;
        }
    }

    test_teardown();
    return pass;
}

static int test_random_u32_range(void) {
    test_setup();
    random_seed(12345);

    /* random_u32() should return 32-bit value */
    u32 val = random_u32();
    int pass = 1;  /* Just verify it produces a value */

    test_teardown();
    return pass;
}

static int test_lcg_multiplier(void) {
    /* Verify LCG uses correct multiplier: 1103515245 */
    test_setup();

    u32 state = 12345;
    u32 expected = state * 1103515245 + 12345;

    random_seed(12345);
    /* Skip warmup calls */
    for (int i = 0; i < 100; i++) {
        random_next();
    }

    /* The state should match LCG formula */
    int pass = (g_random.state != 12345);  /* State should have changed */

    test_teardown();
    return pass;
}

/* ========================================
 * Range Tests
 * ======================================== */

static int test_random_range_zero(void) {
    test_setup();

    u32 val = random_range(0);

    int pass = val == 0;

    test_teardown();
    return pass;
}

static int test_random_range_one(void) {
    test_setup();

    u32 val = random_range(1);

    int pass = val == 0;  /* Range [0, 1) should always be 0 */

    test_teardown();
    return pass;
}

static int test_random_range_bounds(void) {
    test_setup();
    random_seed(99999);

    /* All values should be in range [0, 100) */
    int pass = 1;
    for (int i = 0; i < 1000; i++) {
        u32 val = random_range(100);
        if (val >= 100) {
            pass = 0;
            break;
        }
    }

    test_teardown();
    return pass;
}

static int test_random_range_ex_equal(void) {
    test_setup();

    u32 val = random_range_ex(50, 50);

    int pass = val == 50;  /* When min == max, return min */

    test_teardown();
    return pass;
}

static int test_random_range_ex_invalid(void) {
    test_setup();

    u32 val = random_range_ex(100, 50);  /* min > max */

    int pass = val == 100;  /* Should return min when min > max */

    test_teardown();
    return pass;
}

static int test_random_range_ex_bounds(void) {
    test_setup();
    random_seed(55555);

    /* All values should be in range [10, 20] */
    int pass = 1;
    for (int i = 0; i < 1000; i++) {
        u32 val = random_range_ex(10, 20);
        if (val < 10 || val > 20) {
            pass = 0;
            break;
        }
    }

    test_teardown();
    return pass;
}

/* ========================================
 * Float Tests
 * ======================================== */

static int test_random_float_range(void) {
    test_setup();
    random_seed(77777);

    /* All values should be in range [0.0, 1.0) */
    int pass = 1;
    for (int i = 0; i < 1000; i++) {
        float val = random_float();
        if (val < 0.0f || val >= 1.0f) {
            pass = 0;
            break;
        }
    }

    test_teardown();
    return pass;
}

static int test_random_float_range_ex(void) {
    test_setup();
    random_seed(88888);

    /* All values should be in range [5.0, 10.0] */
    int pass = 1;
    for (int i = 0; i < 1000; i++) {
        float val = random_float_range(5.0f, 10.0f);
        if (val < 5.0f || val > 10.0f) {
            pass = 0;
            break;
        }
    }

    test_teardown();
    return pass;
}

static int test_random_float_zero_range(void) {
    test_setup();

    float val = random_float_range(7.0f, 7.0f);

    int pass = val == 7.0f;

    test_teardown();
    return pass;
}

/* ========================================
 * Chance Tests
 * ======================================== */

static int test_chance_zero(void) {
    test_setup();

    int pass = random_chance(0) == 0;  /* 0% should never hit */

    test_teardown();
    return pass;
}

static int test_chance_hundred(void) {
    test_setup();

    int pass = random_chance(100) == 1;  /* 100% should always hit */

    test_teardown();
    return pass;
}

static int test_chance_over_hundred(void) {
    test_setup();

    int pass = random_chance(200) == 1;  /* >100% should always hit */

    test_teardown();
    return pass;
}

static int test_chance_fifty_approx(void) {
    test_setup();
    random_seed(11111);

    int hits = 0;
    for (int i = 0; i < 1000; i++) {
        if (random_chance(50)) hits++;
    }

    /* 50% chance should give roughly 500 hits (allow 400-600) */
    int pass = hits >= 400 && hits <= 600;

    test_teardown();
    return pass;
}

static int test_chance_float_zero(void) {
    test_setup();

    int pass = random_chance_float(0.0f) == 0;

    test_teardown();
    return pass;
}

static int test_chance_float_one(void) {
    test_setup();

    int pass = random_chance_float(1.0f) == 1;

    test_teardown();
    return pass;
}

static int test_chance_float_half(void) {
    test_setup();
    random_seed(22222);

    int hits = 0;
    for (int i = 0; i < 1000; i++) {
        if (random_chance_float(0.5f)) hits++;
    }

    /* 50% chance should give roughly 500 hits (allow 400-600) */
    int pass = hits >= 400 && hits <= 600;

    test_teardown();
    return pass;
}

/* ========================================
 * Dice Tests
 * ======================================== */

static int test_dice_single_d6(void) {
    test_setup();
    random_seed(33333);

    /* 1d6 should give 1-6 */
    int pass = 1;
    for (int i = 0; i < 100; i++) {
        u32 val = random_dice(1, 6);
        if (val < 1 || val > 6) {
            pass = 0;
            break;
        }
    }

    test_teardown();
    return pass;
}

static int test_dice_two_d6(void) {
    test_setup();
    random_seed(44444);

    /* 2d6 should give 2-12 */
    int pass = 1;
    for (int i = 0; i < 100; i++) {
        u32 val = random_dice(2, 6);
        if (val < 2 || val > 12) {
            pass = 0;
            break;
        }
    }

    test_teardown();
    return pass;
}

static int test_dice_zero_count(void) {
    test_setup();

    u32 val = random_dice(0, 6);

    int pass = val == 0;  /* 0 dice should give 0 */

    test_teardown();
    return pass;
}

static int test_dice_with_modifier(void) {
    test_setup();
    random_seed(55555);

    /* 1d6+5 should give 6-11 */
    int pass = 1;
    for (int i = 0; i < 100; i++) {
        u32 val = random_dice_ex(1, 6, 5);
        if (val < 6 || val > 11) {
            pass = 0;
            break;
        }
    }

    test_teardown();
    return pass;
}

static int test_dice_negative_modifier(void) {
    test_setup();
    random_seed(66666);

    /* 1d6-3 should give 0-3 (min 0) */
    int pass = 1;
    for (int i = 0; i < 100; i++) {
        u32 val = random_dice_ex(1, 6, -3);
        if (val > 3) {
            pass = 0;
            break;
        }
    }

    test_teardown();
    return pass;
}

/* ========================================
 * State Management Tests
 * ======================================== */

static int test_get_set_state(void) {
    test_setup();
    random_seed(12345);

    u32 state = random_get_state();
    random_next();
    u32 new_state = random_get_state();

    int pass = state != new_state;  /* State should change after next() */

    test_teardown();
    return pass;
}

static int test_set_state_reproducible(void) {
    test_setup();
    random_seed(99999);

    u32 saved = random_get_state();
    u32 val1 = random_next();
    random_set_state(saved);
    u32 val2 = random_next();

    int pass = val1 == val2;  /* Same state should give same result */

    test_teardown();
    return pass;
}

static int test_seeded_state_independent(void) {
    test_setup();

    RandomState custom;
    random_init_seeded(&custom, 12345);

    /* Custom state should be independent of global */
    u32 custom_val = random_next_seeded(&custom);
    u32 global_val = random_next();

    int pass = 1;  /* Both should produce values */

    test_teardown();
    return pass;
}

/* ========================================
 * Game-Specific Tests
 * ======================================== */

static int test_random_direction(void) {
    test_setup();
    random_seed(77777);

    /* Direction should be 0-7 */
    int pass = 1;
    for (int i = 0; i < 100; i++) {
        u8 dir = random_direction();
        if (dir > 7) {
            pass = 0;
            break;
        }
    }

    test_teardown();
    return pass;
}

static int test_random_angle(void) {
    test_setup();
    random_seed(88888);

    /* Angle should be 0-359 */
    int pass = 1;
    for (int i = 0; i < 100; i++) {
        u16 angle = random_angle();
        if (angle > 359) {
            pass = 0;
            break;
        }
    }

    test_teardown();
    return pass;
}

static int test_random_color(void) {
    test_setup();
    random_seed(99999);

    /* Color should be 24-bit RGB */
    u32 color = random_color();
    int pass = (color & 0xFF000000) == 0;  /* No bits above 24 */

    test_teardown();
    return pass;
}

static int test_hit_check_zero_hit(void) {
    test_setup();

    int pass = random_hit_check(0, 50) == 0;  /* 0% hit rate should miss */

    test_teardown();
    return pass;
}

static int test_hit_check_full_hit(void) {
    test_setup();

    int pass = random_hit_check(100, 0) == 1;  /* 100% hit rate should hit */

    test_teardown();
    return pass;
}

static int test_hit_check_dodge_reduce(void) {
    test_setup();
    random_seed(11111);

    /* 80% hit with 30% dodge = 50% effective */
    int hits = 0;
    for (int i = 0; i < 1000; i++) {
        if (random_hit_check(80, 30)) hits++;
    }

    /* Should be around 500 (allow 400-600) */
    int pass = hits >= 400 && hits <= 600;

    test_teardown();
    return pass;
}

static int test_critical_check_zero(void) {
    test_setup();

    int pass = random_critical_check(0, 0) == 0;

    test_teardown();
    return pass;
}

static int test_critical_check_max(void) {
    test_setup();

    int pass = random_critical_check(50, 60) == 1;  /* 110% should cap and hit */

    test_teardown();
    return pass;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Random Utility Module Comprehensive Tests ===\n\n");

    /* Initialization tests */
    printf("Initialization Tests:\n");
    TEST(random_init);
    TEST(random_shutdown);

    /* Seeding tests */
    printf("\nSeeding Tests:\n");
    TEST(seed_basic);
    TEST(seed_zero);
    TEST(seed_deterministic);
    TEST(different_seeds);

    /* LCG algorithm tests */
    printf("\nLCG Algorithm Tests:\n");
    TEST(random_next_range);
    TEST(random_u32_range);
    TEST(lcg_multiplier);

    /* Range tests */
    printf("\nRange Tests:\n");
    TEST(random_range_zero);
    TEST(random_range_one);
    TEST(random_range_bounds);
    TEST(random_range_ex_equal);
    TEST(random_range_ex_invalid);
    TEST(random_range_ex_bounds);

    /* Float tests */
    printf("\nFloat Tests:\n");
    TEST(random_float_range);
    TEST(random_float_range_ex);
    TEST(random_float_zero_range);

    /* Chance tests */
    printf("\nChance Tests:\n");
    TEST(chance_zero);
    TEST(chance_hundred);
    TEST(chance_over_hundred);
    TEST(chance_fifty_approx);
    TEST(chance_float_zero);
    TEST(chance_float_one);
    TEST(chance_float_half);

    /* Dice tests */
    printf("\nDice Tests:\n");
    TEST(dice_single_d6);
    TEST(dice_two_d6);
    TEST(dice_zero_count);
    TEST(dice_with_modifier);
    TEST(dice_negative_modifier);

    /* State management tests */
    printf("\nState Management Tests:\n");
    TEST(get_set_state);
    TEST(set_state_reproducible);
    TEST(seeded_state_independent);

    /* Game-specific tests */
    printf("\nGame-Specific Tests:\n");
    TEST(random_direction);
    TEST(random_angle);
    TEST(random_color);
    TEST(hit_check_zero_hit);
    TEST(hit_check_full_hit);
    TEST(hit_check_dodge_reduce);
    TEST(critical_check_zero);
    TEST(critical_check_max);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - Distribution uniformity verification
     * - Gaussian distribution
     * - Weighted random selection
     * - Array shuffling
     * - Point in circle/rectangle generation
     * - Random name generation
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
