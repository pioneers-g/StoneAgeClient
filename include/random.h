/*
 * Stone Age Client - Random Number Generator Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef RANDOM_H
#define RANDOM_H

#include "types.h"

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

/* Global random context */
extern RandomContext g_random;

/* Initialization */
int random_init(void);
void random_shutdown(void);

/* Seeding */
void random_seed(u32 seed);

/* Basic random */
u32 random_next(void);
u32 random_u32(void);
u32 random_range(u32 max);
u32 random_range_ex(u32 min, u32 max);

/* Float random */
float random_float(void);
float random_float_range(float min, float max);

/* Boolean/chance */
int random_chance(u32 percent);
int random_chance_float(float probability);

/* Dice */
u32 random_dice(u32 count, u32 sides);
u32 random_dice_ex(u32 count, u32 sides, s32 modifier);

/* Selection */
void* random_pick(void** array, u32 count);
u32 random_weighted(u32* weights, u32 count);

/* Shuffle */
void random_shuffle(void** array, u32 count);
void random_shuffle_int(int* array, u32 count);

/* Geometry */
void random_point_in_circle(int cx, int cy, int radius, int* x, int* y);
void random_point_in_rect(int left, int top, int right, int bottom, int* x, int* y);

/* Game-specific */
u8 random_direction(void);
u16 random_angle(void);
u32 random_color(void);
int random_name(char* buffer, int size, int min_len, int max_len);

/* State management */
u32 random_get_state(void);
void random_set_state(u32 state);

/* Custom state */
void random_init_seeded(RandomState* state, u32 seed);
u32 random_next_seeded(RandomState* state);

/* Advanced */
float random_gaussian(float mean, float stddev);

/* Game mechanics */
int random_hit_check(u32 hit_rate, u32 dodge_rate);
int random_critical_check(u32 crit_rate, u32 luck_bonus);

#endif /* RANDOM_H */
