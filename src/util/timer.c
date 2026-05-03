/*
 * Stone Age Client - Timer System Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "timer.h"
#include "logger.h"

/* Global timer context */
TimerContext g_timer = {0};

/*
 * Initialize timer system
 */
int timer_init(void) {
    memset(&g_timer, 0, sizeof(TimerContext));

    g_timer.base_time = timeGetTime();
    g_timer.last_update = g_timer.base_time;
    g_timer.paused = 0;

    LOG_INFO("Timer system initialized");
    return 1;
}

/*
 * Shutdown timer system
 */
void timer_shutdown(void) {
    int i;

    /* Clear all timers */
    for (i = 0; i < MAX_TIMERS; i++) {
        if (g_timer.timers[i].active) {
            g_timer.timers[i].active = 0;
        }
    }

    memset(&g_timer, 0, sizeof(TimerContext));
    LOG_INFO("Timer system shutdown");
}

/*
 * Update timer system
 */
void timer_update(void) {
    u32 current_time;
    u32 delta;
    int i;

    if (g_timer.paused) {
        return;
    }

    current_time = timeGetTime();
    delta = current_time - g_timer.last_update;
    g_timer.last_update = current_time;

    /* Update elapsed time */
    g_timer.elapsed_time += delta;

    /* Process active timers */
    for (i = 0; i < MAX_TIMERS; i++) {
        if (!g_timer.timers[i].active) {
            continue;
        }

        g_timer.timers[i].elapsed += delta;

        if (g_timer.timers[i].elapsed >= g_timer.timers[i].interval) {
            /* Timer triggered */
            if (g_timer.timers[i].callback) {
                g_timer.timers[i].callback(g_timer.timers[i].id, g_timer.timers[i].user_data);
            }

            if (g_timer.timers[i].repeating) {
                /* Reset for next interval */
                g_timer.timers[i].elapsed = 0;
                if (g_timer.timers[i].count > 0) {
                    g_timer.timers[i].count--;
                    if (g_timer.timers[i].count == 0) {
                        g_timer.timers[i].active = 0;
                    }
                }
            } else {
                /* One-shot timer, deactivate */
                g_timer.timers[i].active = 0;
            }
        }
    }
}

/*
 * Create a new timer
 */
int timer_create(u32 interval, int repeating, TimerCallback callback, void* user_data) {
    int i;
    int id = -1;

    /* Find free slot */
    for (i = 0; i < MAX_TIMERS; i++) {
        if (!g_timer.timers[i].active) {
            id = i;
            break;
        }
    }

    if (id < 0) {
        return -1;
    }

    g_timer.timers[id].id = id;
    g_timer.timers[id].interval = interval;
    g_timer.timers[id].elapsed = 0;
    g_timer.timers[id].repeating = repeating;
    g_timer.timers[id].count = 0;  /* Infinite */
    g_timer.timers[id].callback = callback;
    g_timer.timers[id].user_data = user_data;
    g_timer.timers[id].active = 1;
    g_timer.timers[id].paused = 0;

    g_timer.timer_count++;

    return id;
}

/*
 * Create a one-shot timer
 */
int timer_create_oneshot(u32 delay, TimerCallback callback, void* user_data) {
    return timer_create(delay, 0, callback, user_data);
}

/*
 * Create a repeating timer
 */
int timer_create_repeating(u32 interval, TimerCallback callback, void* user_data) {
    return timer_create(interval, 1, callback, user_data);
}

/*
 * Create a timer with limited repeat count
 */
int timer_create_counted(u32 interval, int count, TimerCallback callback, void* user_data) {
    int id = timer_create(interval, 1, callback, user_data);
    if (id >= 0) {
        g_timer.timers[id].count = count;
    }
    return id;
}

/*
 * Destroy a timer
 */
int timer_destroy(int timer_id) {
    if (timer_id < 0 || timer_id >= MAX_TIMERS) {
        return 0;
    }

    if (!g_timer.timers[timer_id].active) {
        return 0;
    }

    g_timer.timers[timer_id].active = 0;
    g_timer.timer_count--;

    return 1;
}

/*
 * Pause a timer
 */
int timer_pause(int timer_id) {
    if (timer_id < 0 || timer_id >= MAX_TIMERS) {
        return 0;
    }

    if (!g_timer.timers[timer_id].active) {
        return 0;
    }

    g_timer.timers[timer_id].paused = 1;
    return 1;
}

/*
 * Resume a timer
 */
int timer_resume(int timer_id) {
    if (timer_id < 0 || timer_id >= MAX_TIMERS) {
        return 0;
    }

    if (!g_timer.timers[timer_id].active) {
        return 0;
    }

    g_timer.timers[timer_id].paused = 0;
    return 1;
}

/*
 * Reset a timer
 */
int timer_reset(int timer_id) {
    if (timer_id < 0 || timer_id >= MAX_TIMERS) {
        return 0;
    }

    if (!g_timer.timers[timer_id].active) {
        return 0;
    }

    g_timer.timers[timer_id].elapsed = 0;
    return 1;
}

/*
 * Set timer interval
 */
int timer_set_interval(int timer_id, u32 interval) {
    if (timer_id < 0 || timer_id >= MAX_TIMERS) {
        return 0;
    }

    if (!g_timer.timers[timer_id].active) {
        return 0;
    }

    g_timer.timers[timer_id].interval = interval;
    return 1;
}

/*
 * Check if timer is active
 */
int timer_is_active(int timer_id) {
    if (timer_id < 0 || timer_id >= MAX_TIMERS) {
        return 0;
    }

    return g_timer.timers[timer_id].active;
}

/*
 * Check if timer is paused
 */
int timer_is_paused(int timer_id) {
    if (timer_id < 0 || timer_id >= MAX_TIMERS) {
        return 0;
    }

    return g_timer.timers[timer_id].paused;
}

/*
 * Get remaining time
 */
u32 timer_get_remaining(int timer_id) {
    if (timer_id < 0 || timer_id >= MAX_TIMERS) {
        return 0;
    }

    if (!g_timer.timers[timer_id].active) {
        return 0;
    }

    u32 elapsed = g_timer.timers[timer_id].elapsed;
    u32 interval = g_timer.timers[timer_id].interval;

    return elapsed < interval ? interval - elapsed : 0;
}

/*
 * Get elapsed time
 */
u32 timer_get_elapsed(int timer_id) {
    if (timer_id < 0 || timer_id >= MAX_TIMERS) {
        return 0;
    }

    return g_timer.timers[timer_id].elapsed;
}

/*
 * Get progress (0-100)
 */
u8 timer_get_progress(int timer_id) {
    if (timer_id < 0 || timer_id >= MAX_TIMERS) {
        return 0;
    }

    if (!g_timer.timers[timer_id].active || g_timer.timers[timer_id].interval == 0) {
        return 0;
    }

    u32 progress = (g_timer.timers[timer_id].elapsed * 100) / g_timer.timers[timer_id].interval;
    return (u8)(progress > 100 ? 100 : progress);
}

/*
 * Get total elapsed time since start
 */
u32 timer_get_total_elapsed(void) {
    return g_timer.elapsed_time;
}

/*
 * Get current time
 */
u32 timer_get_current_time(void) {
    return timeGetTime();
}

/*
 * Get delta time since last update
 */
u32 timer_get_delta_time(void) {
    return g_timer.last_update - g_timer.base_time;
}

/*
 * Pause all timers
 */
void timer_pause_all(void) {
    g_timer.paused = 1;
}

/*
 * Resume all timers
 */
void timer_resume_all(void) {
    u32 current = timeGetTime();
    u32 paused_duration = current - g_timer.pause_start;

    g_timer.paused = 0;
    g_timer.last_update = current;

    /* Adjust base time to account for pause */
    g_timer.base_time += paused_duration;
}

/*
 * Check if timers are paused
 */
int timer_is_system_paused(void) {
    return g_timer.paused;
}

/*
 * Destroy all timers
 */
void timer_destroy_all(void) {
    int i;

    for (i = 0; i < MAX_TIMERS; i++) {
        g_timer.timers[i].active = 0;
    }

    g_timer.timer_count = 0;
}

/*
 * Get active timer count
 */
int timer_get_count(void) {
    return g_timer.timer_count;
}

/*
 * Schedule a callback for later
 */
int timer_schedule(u32 delay, TimerCallback callback, void* user_data) {
    return timer_create_oneshot(delay, callback, user_data);
}

/*
 * Create cooldown timer
 */
int timer_start_cooldown(u32 duration, TimerCallback callback, void* user_data) {
    return timer_create_oneshot(duration, callback, user_data);
}

/*
 * Check if cooldown is active
 */
int timer_is_cooldown_active(int timer_id) {
    return timer_is_active(timer_id);
}

/*
 * Get cooldown remaining
 */
u32 timer_get_cooldown_remaining(int timer_id) {
    return timer_get_remaining(timer_id);
}

/*
 * Format time as string
 */
const char* timer_format_time(u32 ms) {
    static char buffer[32];
    u32 seconds = ms / 1000;
    u32 minutes = seconds / 60;
    u32 hours = minutes / 60;

    if (hours > 0) {
        snprintf(buffer, sizeof(buffer), "%u:%02u:%02u", hours, minutes % 60, seconds % 60);
    } else if (minutes > 0) {
        snprintf(buffer, sizeof(buffer), "%u:%02u", minutes, seconds % 60);
    } else {
        snprintf(buffer, sizeof(buffer), "%u.%03us", seconds, ms % 1000);
    }

    return buffer;
}

/*
 * Convert milliseconds to frames (at 60 FPS)
 */
u32 timer_ms_to_frames(u32 ms) {
    return (ms * 60) / 1000;
}

/*
 * Convert frames to milliseconds (at 60 FPS)
 */
u32 timer_frames_to_ms(u32 frames) {
    return (frames * 1000) / 60;
}

/*
 * Simple delay
 */
void timer_delay(u32 ms) {
    Sleep(ms);
}

/*
 * High precision delay
 */
void timer_delay_precise(u32 ms) {
    u32 start = timeGetTime();
    while (timeGetTime() - start < ms) {
        Sleep(1);
    }
}

/*
 * Get ticks since start
 */
u32 timer_get_ticks(void) {
    return timeGetTime() - g_timer.base_time;
}

/*
 * Check if time has elapsed
 */
int timer_has_elapsed(u32 start_time, u32 interval) {
    return (timeGetTime() - start_time) >= interval;
}

/*
 * Get time since timestamp
 */
u32 timer_time_since(u32 timestamp) {
    u32 current = timeGetTime();
    return current >= timestamp ? current - timestamp : 0;
}

/*
 * Calculate FPS
 */
u32 timer_calculate_fps(u32 frame_count, u32 elapsed_ms) {
    if (elapsed_ms == 0) return 0;
    return (frame_count * 1000) / elapsed_ms;
}
