/*
 * Stone Age Client - Timer System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef TIMER_H
#define TIMER_H

#include "types.h"

/* Constants */
#define MAX_TIMERS       64

/* Timer callback function type */
typedef void (*TimerCallback)(int timer_id, void* user_data);

/* Timer entry */
typedef struct {
    int id;
    u32 interval;
    u32 elapsed;
    int repeating;
    int count;          /* Repeat count, 0 = infinite */
    int paused;

    TimerCallback callback;
    void* user_data;

    int active;

} TimerEntry;

/* Timer context */
typedef struct {
    /* Timer slots */
    TimerEntry timers[MAX_TIMERS];
    int timer_count;

    /* System time tracking */
    u32 base_time;
    u32 last_update;
    u32 elapsed_time;
    u32 pause_start;

    /* System state */
    int paused;

} TimerContext;

/* Global timer context */
extern TimerContext g_timer;

/* Initialization */
int timer_init(void);
void timer_shutdown(void);

/* Update */
void timer_update(void);

/* Timer creation */
int timer_create(u32 interval, int repeating, TimerCallback callback, void* user_data);
int timer_create_oneshot(u32 delay, TimerCallback callback, void* user_data);
int timer_create_repeating(u32 interval, TimerCallback callback, void* user_data);
int timer_create_counted(u32 interval, int count, TimerCallback callback, void* user_data);

/* Timer control */
int timer_destroy(int timer_id);
int timer_pause(int timer_id);
int timer_resume(int timer_id);
int timer_reset(int timer_id);
int timer_set_interval(int timer_id, u32 interval);

/* Timer query */
int timer_is_active(int timer_id);
int timer_is_paused(int timer_id);
u32 timer_get_remaining(int timer_id);
u32 timer_get_elapsed(int timer_id);
u8 timer_get_progress(int timer_id);  /* 0-100 */

/* System time */
u32 timer_get_total_elapsed(void);
u32 timer_get_current_time(void);
u32 timer_get_delta_time(void);

/* System control */
void timer_pause_all(void);
void timer_resume_all(void);
int timer_is_system_paused(void);
void timer_destroy_all(void);
int timer_get_count(void);

/* Convenience functions */
int timer_schedule(u32 delay, TimerCallback callback, void* user_data);
int timer_start_cooldown(u32 duration, TimerCallback callback, void* user_data);
int timer_is_cooldown_active(int timer_id);
u32 timer_get_cooldown_remaining(int timer_id);

/* Utility */
const char* timer_format_time(u32 ms);
u32 timer_ms_to_frames(u32 ms);
u32 timer_frames_to_ms(u32 frames);
void timer_delay(u32 ms);
void timer_delay_precise(u32 ms);
u32 timer_get_ticks(void);
int timer_has_elapsed(u32 start_time, u32 interval);
u32 timer_time_since(u32 timestamp);
u32 timer_calculate_fps(u32 frame_count, u32 elapsed_ms);

#endif /* TIMER_H */
