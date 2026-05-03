/*
 * Stone Age Client - LCG Random Number Generator
 * Split from sprite.c for code organization
 *
 * Microsoft C/C++ compatible Linear Congruential Generator
 * Based on FUN_00492403
 */

#include <windows.h>
#include "types.h"
#include "sprite.h"

/* LCG state - DAT_004d7160 */
static u32 g_lcg_state = 0x12345678;

/*
 * LCG Random - FUN_00492403
 * Microsoft C/C++ compatible LCG
 * State = State * 214013 + 2531011
 * Returns (State >> 16) & 0x7FFF
 */
u32 sprite_lcg_random(void) {
    g_lcg_state = g_lcg_state * 0x343fd + 0x269ec3;
    return (g_lcg_state >> 16) & 0x7fff;
}

/*
 * Seed the LCG
 */
void sprite_lcg_seed(u32 seed) {
    g_lcg_state = seed;
}

/*
 * Get current LCG state
 */
u32 sprite_lcg_get_state(void) {
    return g_lcg_state;
}

/*
 * Set LCG state (for save/restore)
 */
void sprite_lcg_set_state(u32 state) {
    g_lcg_state = state;
}

/*
 * Get random number in range [0, max)
 */
u32 sprite_lcg_random_range(u32 max) {
    if (max == 0) return 0;
    return sprite_lcg_random() % max;
}

/*
 * Get random number in range [min, max]
 */
u32 sprite_lcg_random_between(u32 min, u32 max) {
    if (min >= max) return min;
    return min + (sprite_lcg_random() % (max - min + 1));
}
