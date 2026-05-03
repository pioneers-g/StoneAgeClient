/*
 * Stone Age Client - Random Number Generator Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "types.h"
#include "random.h"
#include "logger.h"

/* Global RNG state */
RandomContext g_random = {0};

/*
 * Initialize random system
 */
int random_init(void) {
    memset(&g_random, 0, sizeof(RandomContext));

    /* Seed with current time */
    random_seed((u32)time(NULL));

    LOG_INFO("Random system initialized");
    return 1;
}

/*
 * Shutdown random system
 */
void random_shutdown(void) {
    memset(&g_random, 0, sizeof(RandomContext));
    LOG_INFO("Random system shutdown");
}

/*
 * Seed the random number generator
 */
void random_seed(u32 seed) {
    g_random.seed = seed;
    g_random.state = seed;

    /* Warm up the generator */
    int i;
    for (i = 0; i < 100; i++) {
        random_next();
    }
}

/*
 * Generate next random number (LCG algorithm)
 */
u32 random_next(void) {
    /* Linear Congruential Generator */
    g_random.state = g_random.state * 1103515245 + 12345;
    return (g_random.state >> 16) & 0x7FFF;
}

/*
 * Generate random 32-bit number
 */
u32 random_u32(void) {
    return (random_next() << 17) | (random_next() << 2) | (random_next() & 3);
}

/*
 * Generate random number in range [0, max)
 */
u32 random_range(u32 max) {
    if (max == 0) return 0;
    return random_next() % max;
}

/*
 * Generate random number in range [min, max]
 */
u32 random_range_ex(u32 min, u32 max) {
    if (min >= max) return min;
    return min + random_range(max - min + 1);
}

/*
 * Generate random float [0.0, 1.0)
 */
float random_float(void) {
    return (float)random_next() / 32768.0f;
}

/*
 * Generate random float in range [min, max]
 */
float random_float_range(float min, float max) {
    return min + random_float() * (max - min);
}

/*
 * Generate random boolean with probability
 */
int random_chance(u32 percent) {
    if (percent >= 100) return 1;
    if (percent == 0) return 0;
    return random_range(100) < percent;
}

/*
 * Generate random boolean with probability (floating point)
 */
int random_chance_float(float probability) {
    if (probability >= 1.0f) return 1;
    if (probability <= 0.0f) return 0;
    return random_float() < probability;
}

/*
 * Roll dice (count x sides)
 */
u32 random_dice(u32 count, u32 sides) {
    u32 total = 0;
    u32 i;

    if (sides == 0 || count == 0) return 0;

    for (i = 0; i < count; i++) {
        total += random_range(sides) + 1;
    }

    return total;
}

/*
 * Roll dice with modifier
 */
u32 random_dice_ex(u32 count, u32 sides, s32 modifier) {
    s32 result = (s32)random_dice(count, sides) + modifier;
    return result > 0 ? (u32)result : 0;
}

/*
 * Pick random element from array
 */
void* random_pick(void** array, u32 count) {
    if (!array || count == 0) return NULL;
    return array[random_range(count)];
}

/*
 * Pick random index weighted by probabilities
 */
u32 random_weighted(u32* weights, u32 count) {
    u32 total;
    u32 roll;
    u32 cumulative;
    u32 i;

    if (!weights || count == 0) return 0;

    /* Calculate total weight */
    total = 0;
    for (i = 0; i < count; i++) {
        total += weights[i];
    }

    if (total == 0) return random_range(count);

    /* Roll */
    roll = random_range(total);

    /* Find selected index */
    cumulative = 0;
    for (i = 0; i < count; i++) {
        cumulative += weights[i];
        if (roll < cumulative) {
            return i;
        }
    }

    return count - 1;
}

/*
 * Shuffle array
 */
void random_shuffle(void** array, u32 count) {
    u32 i, j;
    void* temp;

    if (!array || count <= 1) return;

    /* Fisher-Yates shuffle */
    for (i = count - 1; i > 0; i--) {
        j = random_range(i + 1);
        temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

/*
 * Shuffle integer array
 */
void random_shuffle_int(int* array, u32 count) {
    u32 i, j;
    int temp;

    if (!array || count <= 1) return;

    for (i = count - 1; i > 0; i--) {
        j = random_range(i + 1);
        temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

/*
 * Generate random point in circle
 */
void random_point_in_circle(int cx, int cy, int radius, int* x, int* y) {
    float angle = random_float() * 2.0f * 3.14159f;
    float r = random_float() * radius;

    if (x) *x = cx + (int)(r * cosf(angle));
    if (y) *y = cy + (int)(r * sinf(angle));
}

/*
 * Generate random point in rectangle
 */
void random_point_in_rect(int left, int top, int right, int bottom, int* x, int* y) {
    if (x) *x = left + random_range(right - left);
    if (y) *y = top + random_range(bottom - top);
}

/*
 * Generate random direction (0-7)
 */
u8 random_direction(void) {
    return (u8)random_range(8);
}

/*
 * Generate random angle (0-359)
 */
u16 random_angle(void) {
    return (u16)random_range(360);
}

/*
 * Generate random color
 */
u32 random_color(void) {
    return random_range(256) | (random_range(256) << 8) | (random_range(256) << 16);
}

/*
 * Generate random name
 */
int random_name(char* buffer, int size, int min_len, int max_len) {
    static const char* vowels = "aeiou";
    static const char* consonants = "bcdfghjklmnpqrstvwxyz";

    int len;
    int i;
    int use_vowel;

    if (!buffer || size < 2) return 0;

    len = random_range_ex(min_len, max_len);
    if (len >= size) len = size - 1;

    use_vowel = random_chance(50);

    for (i = 0; i < len; i++) {
        if (use_vowel) {
            buffer[i] = vowels[random_range(5)];
        } else {
            buffer[i] = consonants[random_range(21)];
        }
        use_vowel = !use_vowel;
    }

    /* Capitalize first letter */
    buffer[0] = buffer[0] - 'a' + 'A';

    buffer[len] = '\0';
    return 1;
}

/*
 * Get random state
 */
u32 random_get_state(void) {
    return g_random.state;
}

/*
 * Set random state
 */
void random_set_state(u32 state) {
    g_random.state = state;
}

/*
 * Create random generator with seed
 */
void random_init_seeded(RandomState* state, u32 seed) {
    if (!state) return;
    state->seed = seed;
    state->state = seed;
}

/*
 * Generate random with custom state
 */
u32 random_next_seeded(RandomState* state) {
    if (!state) return 0;
    state->state = state->state * 1103515245 + 12345;
    return (state->state >> 16) & 0x7FFF;
}

/*
 * Generate Gaussian random number (Box-Muller transform)
 */
float random_gaussian(float mean, float stddev) {
    float u1, u2;
    float z0;

    u1 = random_float();
    u2 = random_float();

    /* Avoid log(0) */
    if (u1 < 0.0001f) u1 = 0.0001f;

    z0 = sqrtf(-2.0f * logf(u1)) * cosf(2.0f * 3.14159f * u2);

    return mean + z0 * stddev;
}

/*
 * Check if random hit succeeds (for game mechanics)
 */
int random_hit_check(u32 hit_rate, u32 dodge_rate) {
    u32 effective_rate;

    if (hit_rate >= 100) return 1;
    if (dodge_rate >= 100) return 0;

    effective_rate = hit_rate;
    if (dodge_rate > hit_rate) {
        effective_rate = 0;
    } else {
        effective_rate = hit_rate - dodge_rate;
    }

    return random_chance(effective_rate);
}

/*
 * Critical hit check
 */
int random_critical_check(u32 crit_rate, u32 luck_bonus) {
    return random_chance(crit_rate + luck_bonus);
}
