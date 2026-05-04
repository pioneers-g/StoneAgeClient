/*
 * Stone Age Client - Input Flag Processing Unit Tests
 * Tests for FUN_004809e0
 * This function processes keyboard input flags with bitmask operations
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

/* Input flag arrays - 256 entries for key codes */
#define INPUT_ARRAY_SIZE 256

static u32 input_flags[INPUT_ARRAY_SIZE];     /* DAT_04ebe488 */
static u32 input_flags2[INPUT_ARRAY_SIZE];    /* DAT_04ebe490 */
static ushort key_timers[INPUT_ARRAY_SIZE];   /* DAT_04ebe420 */

/* Flag bit definitions */
#define FLAG_0x0010   0x0010
#define FLAG_0x0020   0x0020
#define FLAG_0x0040   0x0040
#define FLAG_0x0080   0x0080
#define FLAG_0x0100   0x0100
#define FLAG_0x0400   0x0400
#define FLAG_0x1000   0x1000
#define FLAG_0x2000   0x2000
#define FLAG_0x4000   0x4000
#define FLAG_0x8000   0x8000
#define FLAG_MASK     0xF5F0

#define TIMER_THRESHOLD 0x1E  /* 30 */
#define TIMER_RESET     0x17  /* 23 */

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
    memset(input_flags, 0, sizeof(input_flags));
    memset(input_flags2, 0, sizeof(input_flags2));
    memset(key_timers, 0, sizeof(key_timers));
}

/*
 * FUN_004809e0 - Process input flags for a key
 * Matches the actual Ghidra decompilation behavior
 * param_1: key code (0-255)
 * Returns: combined flag result
 */
static u32 FUN_004809e0(u32 param_1) {
    u32 result = 0;
    param_1 = param_1 & 0xFF;

    u32 flag1 = input_flags[param_1];

    /* If flag1 has no relevant bits, clear timer and return 0 */
    if ((flag1 & FLAG_MASK) == 0) {
        key_timers[param_1] = 0;
        return result;
    }

    u32 flag2 = input_flags2[param_1];

    /* Process flag 0x1000 */
    if ((flag2 & FLAG_0x1000) == 0) {
        if ((flag1 & FLAG_0x1000) != 0) {
            key_timers[param_1]++;
            if (key_timers[param_1] == TIMER_THRESHOLD) {
                key_timers[param_1] = TIMER_RESET;
                result |= FLAG_0x1000;
            }
        }
    } else {
        result |= FLAG_0x1000;
    }

    /* Process flag 0x2000 */
    if ((flag2 & FLAG_0x2000) == 0) {
        if ((flag1 & FLAG_0x2000) != 0) {
            key_timers[param_1]++;
            if (key_timers[param_1] == TIMER_THRESHOLD) {
                key_timers[param_1] = TIMER_RESET;
                result |= FLAG_0x2000;
            }
        }
    } else {
        result |= FLAG_0x2000;
    }

    /* Process flag 0x8000 */
    if ((flag2 & FLAG_0x8000) == 0) {
        if ((flag1 & FLAG_0x8000) != 0) {
            key_timers[param_1]++;
            if (key_timers[param_1] == TIMER_THRESHOLD) {
                key_timers[param_1] = TIMER_RESET;
                result |= FLAG_0x8000;
            }
        }
    } else {
        result |= FLAG_0x8000;
    }

    /* Process flag 0x4000 */
    if ((flag2 & FLAG_0x4000) == 0) {
        if ((flag1 & FLAG_0x4000) != 0) {
            key_timers[param_1]++;
            if (key_timers[param_1] == TIMER_THRESHOLD) {
                key_timers[param_1] = TIMER_RESET;
                result |= FLAG_0x4000;
            }
        }
    } else {
        result |= FLAG_0x4000;
    }

    /* Process flag 0x0400 */
    if ((flag2 & FLAG_0x0400) == 0) {
        if ((flag1 & FLAG_0x0400) != 0) {
            key_timers[param_1]++;
            if (key_timers[param_1] == TIMER_THRESHOLD) {
                key_timers[param_1] = TIMER_RESET;
                result |= FLAG_0x0400;
            }
        }
    } else {
        result |= FLAG_0x0400;
    }

    /* Process flag 0x0100 */
    if ((flag2 & FLAG_0x0100) == 0) {
        if ((flag1 & FLAG_0x0100) != 0) {
            key_timers[param_1]++;
            if (key_timers[param_1] == TIMER_THRESHOLD) {
                key_timers[param_1] = TIMER_RESET;
                result |= FLAG_0x0100;
            }
        }
    } else {
        result |= FLAG_0x0100;
    }

    /* Process flag 0x0010 */
    if ((flag2 & FLAG_0x0010) == 0) {
        if ((flag1 & FLAG_0x0010) != 0) {
            key_timers[param_1]++;
            if (key_timers[param_1] == TIMER_THRESHOLD) {
                key_timers[param_1] = TIMER_RESET;
                result |= FLAG_0x0010;
            }
        }
    } else {
        result |= FLAG_0x0010;
    }

    /* Process flag 0x0020 */
    if ((flag2 & FLAG_0x0020) == 0) {
        if ((flag1 & FLAG_0x0020) != 0) {
            key_timers[param_1]++;
            if (key_timers[param_1] == TIMER_THRESHOLD) {
                key_timers[param_1] = TIMER_RESET;
                result |= FLAG_0x0020;
            }
        }
    } else {
        result |= FLAG_0x0020;
    }

    /* Process flag 0x0040 */
    if ((flag2 & FLAG_0x0040) == 0) {
        if ((flag1 & FLAG_0x0040) != 0) {
            key_timers[param_1]++;
            if (key_timers[param_1] == TIMER_THRESHOLD) {
                key_timers[param_1] = TIMER_RESET;
                result |= FLAG_0x0040;
            }
        }
    } else {
        result |= FLAG_0x0040;
    }

    /* Process flag 0x0080 */
    if ((flag2 & FLAG_0x0080) == 0) {
        if ((flag1 & FLAG_0x0080) != 0) {
            key_timers[param_1]++;
            if (key_timers[param_1] == TIMER_THRESHOLD) {
                key_timers[param_1] = TIMER_RESET;
                result |= FLAG_0x0080;
            }
        }
    } else {
        result |= FLAG_0x0080;
    }

    return result;
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(no_flags_set) {
    reset_test_state();

    u32 result = FUN_004809e0(0);

    ASSERT(result == 0);
    ASSERT(key_timers[0] == 0);
}

TEST(flag1_no_relevant_bits) {
    reset_test_state();
    input_flags[0] = 0x000F;  /* Not in FLAG_MASK */
    input_flags2[0] = FLAG_0x1000;

    u32 result = FUN_004809e0(0);

    /* Should return 0 because flag1 has no relevant bits */
    ASSERT(result == 0);
    ASSERT(key_timers[0] == 0);
}

TEST(flag2_immediate_0x1000) {
    reset_test_state();
    input_flags[0] = FLAG_0x1000;  /* Need flag1 to have bits */
    input_flags2[0] = FLAG_0x1000;

    u32 result = FUN_004809e0(0);

    ASSERT(result == FLAG_0x1000);
}

TEST(flag2_immediate_0x2000) {
    reset_test_state();
    input_flags[65] = FLAG_0x2000;  /* Need flag1 to have bits */
    input_flags2[65] = FLAG_0x2000;

    u32 result = FUN_004809e0(65);

    ASSERT(result == FLAG_0x2000);
}

TEST(flag1_timer_triggers) {
    reset_test_state();
    input_flags[0] = FLAG_0x1000;
    key_timers[0] = TIMER_THRESHOLD - 1;  /* 29 */

    u32 result = FUN_004809e0(0);

    ASSERT(result == FLAG_0x1000);
    ASSERT(key_timers[0] == TIMER_RESET);  /* Reset to 23 */
}

TEST(flag1_timer_not_yet) {
    reset_test_state();
    input_flags[0] = FLAG_0x1000;
    key_timers[0] = 10;

    u32 result = FUN_004809e0(0);

    ASSERT(result == 0);
    ASSERT(key_timers[0] == 11);
}

TEST(multiple_flags_flag2) {
    reset_test_state();
    input_flags[0] = FLAG_MASK;
    input_flags2[0] = FLAG_0x1000 | FLAG_0x2000 | FLAG_0x4000;

    u32 result = FUN_004809e0(0);

    ASSERT(result == (FLAG_0x1000 | FLAG_0x2000 | FLAG_0x4000));
}

TEST(multiple_flags_mixed) {
    reset_test_state();
    input_flags[0] = FLAG_MASK;
    input_flags2[0] = FLAG_0x2000;
    key_timers[0] = TIMER_THRESHOLD - 1;

    u32 result = FUN_004809e0(0);

    /* Flag 0x2000 from flag2 (immediate), Flag 0x1000 from timer */
    ASSERT((result & FLAG_0x2000) != 0);
    ASSERT((result & FLAG_0x1000) != 0);
}

TEST(all_flag_bits_flag2) {
    reset_test_state();
    input_flags[0] = FLAG_MASK;
    input_flags2[0] = FLAG_MASK;

    u32 result = FUN_004809e0(0);

    ASSERT((result & FLAG_MASK) == FLAG_MASK);
}

TEST(key_code_masked) {
    reset_test_state();
    input_flags[0] = FLAG_0x1000;
    input_flags2[0] = FLAG_0x1000;
    input_flags[1] = FLAG_0x2000;
    input_flags2[1] = FLAG_0x2000;

    u32 result0 = FUN_004809e0(0);
    u32 result256 = FUN_004809e0(256);  /* Key code masked to 0 */

    ASSERT(result0 == FLAG_0x1000);
    ASSERT(result256 == FLAG_0x1000);  /* Same as key 0 */
}

TEST(timer_increments_multiple_calls) {
    reset_test_state();
    input_flags[0] = FLAG_0x1000;

    FUN_004809e0(0);
    ASSERT(key_timers[0] == 1);

    FUN_004809e0(0);
    ASSERT(key_timers[0] == 2);

    FUN_004809e0(0);
    ASSERT(key_timers[0] == 3);
}

TEST(timer_reset_continues) {
    reset_test_state();
    input_flags[0] = FLAG_0x1000;
    key_timers[0] = TIMER_RESET;  /* 23 */

    /* After reset, timer continues from 23 */
    for (int i = 0; i < (TIMER_THRESHOLD - TIMER_RESET - 1); i++) {
        FUN_004809e0(0);
    }

    /* Timer should have triggered again */
    u32 result = FUN_004809e0(0);
    ASSERT((result & FLAG_0x1000) != 0);
}

TEST(flag_0x0010_processing) {
    reset_test_state();
    input_flags[0] = FLAG_0x0010;
    input_flags2[0] = FLAG_0x0010;

    u32 result = FUN_004809e0(0);

    ASSERT(result == FLAG_0x0010);
}

TEST(flag_0x0080_processing) {
    reset_test_state();
    input_flags[0] = FLAG_0x0080;
    key_timers[0] = TIMER_THRESHOLD - 1;

    u32 result = FUN_004809e0(0);

    ASSERT((result & FLAG_0x0080) != 0);
}

TEST(different_keys_independent) {
    reset_test_state();
    input_flags[0] = FLAG_0x1000;
    input_flags[1] = FLAG_0x2000;
    input_flags2[0] = FLAG_0x1000;
    input_flags2[1] = FLAG_0x2000;
    key_timers[0] = 100;
    key_timers[1] = 200;

    u32 result0 = FUN_004809e0(0);
    u32 result1 = FUN_004809e0(1);

    ASSERT(result0 == FLAG_0x1000);
    ASSERT(result1 == FLAG_0x2000);
    ASSERT(key_timers[0] == 100);  /* Unchanged */
    ASSERT(key_timers[1] == 200);  /* Unchanged */
}

TEST(no_relevant_flags_clears_timer) {
    reset_test_state();
    key_timers[0] = 100;
    input_flags[0] = 0x000F;  /* Not in FLAG_MASK */
    input_flags2[0] = 0;

    u32 result = FUN_004809e0(0);

    ASSERT(result == 0);
    ASSERT(key_timers[0] == 0);  /* Timer cleared */
}

TEST(flag2_overrides_timer_check) {
    reset_test_state();
    input_flags[0] = FLAG_0x1000;
    input_flags2[0] = FLAG_0x1000;
    key_timers[0] = 0;  /* Timer at 0, would not trigger normally */

    u32 result = FUN_004809e0(0);

    /* flag2 bit set means immediate result, timer not incremented */
    ASSERT(result == FLAG_0x1000);
    ASSERT(key_timers[0] == 0);  /* Timer not changed */
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Input Flag Processing Unit Tests ===\n\n");

    printf("FUN_004809e0 (Input Flag Processing) Tests:\n");
    RUN_TEST(no_flags_set);
    RUN_TEST(flag1_no_relevant_bits);
    RUN_TEST(flag2_immediate_0x1000);
    RUN_TEST(flag2_immediate_0x2000);
    RUN_TEST(flag1_timer_triggers);
    RUN_TEST(flag1_timer_not_yet);
    RUN_TEST(multiple_flags_flag2);
    RUN_TEST(multiple_flags_mixed);
    RUN_TEST(all_flag_bits_flag2);
    RUN_TEST(key_code_masked);
    RUN_TEST(timer_increments_multiple_calls);
    RUN_TEST(timer_reset_continues);
    RUN_TEST(flag_0x0010_processing);
    RUN_TEST(flag_0x0080_processing);
    RUN_TEST(different_keys_independent);
    RUN_TEST(no_relevant_flags_clears_timer);
    RUN_TEST(flag2_overrides_timer_check);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
