/*
 * Stone Age Client - Performance Monitor Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "performance.h"
#include "logger.h"

/* Global performance context */
PerfContext g_perf = {0};

/*
 * Initialize performance monitor
 */
int perf_init(void) {
    memset(&g_perf, 0, sizeof(PerfContext));

    g_perf.enabled = 1;
    g_perf.frame_count = 0;
    g_perf.fps = 0;
    g_perf.last_fps_update = timeGetTime();

    /* Initialize CPU usage tracking */
    g_perf.cpu_usage = 0;
    g_perf.last_cpu_time = 0;
    g_perf.last_sys_time = 0;

    LOG_INFO("Performance monitor initialized");
    return 1;
}

/*
 * Shutdown performance monitor
 */
void perf_shutdown(void) {
    memset(&g_perf, 0, sizeof(PerfContext));
    LOG_INFO("Performance monitor shutdown");
}

/*
 * Begin frame
 */
void perf_begin_frame(void) {
    if (!g_perf.enabled) return;

    g_perf.frame_start = timeGetTime();
    g_perf.in_frame = 1;
}

/*
 * End frame
 */
void perf_end_frame(void) {
    u32 frame_time;
    u32 current_time;

    if (!g_perf.enabled || !g_perf.in_frame) return;

    current_time = timeGetTime();
    frame_time = current_time - g_perf.frame_start;

    /* Update frame statistics */
    g_perf.frame_time = frame_time;
    g_perf.total_frame_time += frame_time;
    g_perf.frame_count++;

    /* Track min/max */
    if (frame_time < g_perf.min_frame_time || g_perf.min_frame_time == 0) {
        g_perf.min_frame_time = frame_time;
    }
    if (frame_time > g_perf.max_frame_time) {
        g_perf.max_frame_time = frame_time;
    }

    /* Update FPS every second */
    if (current_time - g_perf.last_fps_update >= 1000) {
        g_perf.fps = g_perf.frame_count - g_perf.last_frame_count;
        g_perf.last_frame_count = g_perf.frame_count;
        g_perf.last_fps_update = current_time;
    }

    g_perf.in_frame = 0;
}

/*
 * Get FPS
 */
u32 perf_get_fps(void) {
    return g_perf.fps;
}

/*
 * Get frame time in milliseconds
 */
u32 perf_get_frame_time(void) {
    return g_perf.frame_time;
}

/*
 * Get average frame time
 */
u32 perf_get_avg_frame_time(void) {
    if (g_perf.frame_count == 0) return 0;
    return g_perf.total_frame_time / g_perf.frame_count;
}

/*
 * Get minimum frame time
 */
u32 perf_get_min_frame_time(void) {
    return g_perf.min_frame_time;
}

/*
 * Get maximum frame time
 */
u32 perf_get_max_frame_time(void) {
    return g_perf.max_frame_time;
}

/*
 * Reset frame statistics
 */
void perf_reset_stats(void) {
    g_perf.frame_count = 0;
    g_perf.total_frame_time = 0;
    g_perf.min_frame_time = 0;
    g_perf.max_frame_time = 0;
    g_perf.fps = 0;
    g_perf.last_frame_count = 0;
    g_perf.last_fps_update = timeGetTime();
}

/*
 * Begin performance counter
 */
void perf_begin(const char* name) {
    PerfCounter* counter;
    int i;

    if (!g_perf.enabled || !name) return;

    /* Find existing counter */
    for (i = 0; i < g_perf.counter_count; i++) {
        if (strcmp(g_perf.counters[i].name, name) == 0) {
            counter = &g_perf.counters[i];
            counter->start_time = timeGetTime();
            counter->in_progress = 1;
            return;
        }
    }

    /* Create new counter */
    if (g_perf.counter_count < MAX_PERF_COUNTERS) {
        counter = &g_perf.counters[g_perf.counter_count];
        strncpy(counter->name, name, 31);
        counter->name[31] = '\0';
        counter->start_time = timeGetTime();
        counter->total_time = 0;
        counter->call_count = 0;
        counter->in_progress = 1;
        g_perf.counter_count++;
    }
}

/*
 * End performance counter
 */
void perf_end(const char* name) {
    PerfCounter* counter;
    int i;
    u32 elapsed;

    if (!g_perf.enabled || !name) return;

    for (i = 0; i < g_perf.counter_count; i++) {
        if (strcmp(g_perf.counters[i].name, name) == 0) {
            counter = &g_perf.counters[i];
            if (counter->in_progress) {
                elapsed = timeGetTime() - counter->start_time;
                counter->total_time += elapsed;
                counter->call_count++;
                counter->in_progress = 0;

                /* Track min/max */
                if (elapsed < counter->min_time || counter->min_time == 0) {
                    counter->min_time = elapsed;
                }
                if (elapsed > counter->max_time) {
                    counter->max_time = elapsed;
                }
            }
            return;
        }
    }
}

/*
 * Get counter by name
 */
PerfCounter* perf_get_counter(const char* name) {
    int i;

    for (i = 0; i < g_perf.counter_count; i++) {
        if (strcmp(g_perf.counters[i].name, name) == 0) {
            return &g_perf.counters[i];
        }
    }

    return NULL;
}

/*
 * Get counter count
 */
int perf_get_counter_count(void) {
    return g_perf.counter_count;
}

/*
 * Get counter by index
 */
PerfCounter* perf_get_counter_by_index(int index) {
    if (index < 0 || index >= g_perf.counter_count) {
        return NULL;
    }
    return &g_perf.counters[index];
}

/*
 * Reset all counters
 */
void perf_reset_counters(void) {
    int i;

    for (i = 0; i < g_perf.counter_count; i++) {
        g_perf.counters[i].total_time = 0;
        g_perf.counters[i].call_count = 0;
        g_perf.counters[i].min_time = 0;
        g_perf.counters[i].max_time = 0;
        g_perf.counters[i].in_progress = 0;
    }
}

/*
 * Enable/disable performance monitoring
 */
void perf_set_enabled(int enabled) {
    g_perf.enabled = enabled;
}

/*
 * Check if enabled
 */
int perf_is_enabled(void) {
    return g_perf.enabled;
}

/*
 * Get memory usage
 */
u32 perf_get_memory_usage(void) {
    MEMORYSTATUS status;
    GlobalMemoryStatus(&status);
    return (u32)(status.dwMemoryLoad);
}

/*
 * Get available memory
 */
u32 perf_get_available_memory(void) {
    MEMORYSTATUS status;
    GlobalMemoryStatus(&status);
    return (u32)(status.dwAvailPhys / 1024 / 1024);
}

/*
 * Get CPU usage
 */
u32 perf_get_cpu_usage(void) {
    return g_perf.cpu_usage;
}

/*
 * Update CPU usage
 */
void perf_update_cpu(void) {
    FILETIME idle_time, kernel_time, user_time;
    ULONGLONG idle, kernel, user;
    ULONGLONG total, sys;

    if (GetSystemTimes(&idle_time, &kernel_time, &user_time)) {
        idle = ((ULONGLONG)idle_time.dwHighDateTime << 32) | idle_time.dwLowDateTime;
        kernel = ((ULONGLONG)kernel_time.dwHighDateTime << 32) | kernel_time.dwLowDateTime;
        user = ((ULONGLONG)user_time.dwHighDateTime << 32) | user_time.dwLowDateTime;

        total = kernel + user;
        sys = total - idle;

        if (g_perf.last_cpu_time > 0) {
            ULONGLONG cpu_delta = total - g_perf.last_cpu_time;
            ULONGLONG sys_delta = sys - g_perf.last_sys_time;

            if (cpu_delta > 0) {
                g_perf.cpu_usage = (u32)((sys_delta * 100) / cpu_delta);
            }
        }

        g_perf.last_cpu_time = total;
        g_perf.last_sys_time = sys;
    }
}

/*
 * Print performance report
 */
void perf_print_report(void) {
    int i;

    LOG_INFO("=== Performance Report ===");
    LOG_INFO("FPS: %u", g_perf.fps);
    LOG_INFO("Frame Time: %u ms (avg: %u, min: %u, max: %u)",
             g_perf.frame_time,
             perf_get_avg_frame_time(),
             g_perf.min_frame_time,
             g_perf.max_frame_time);
    LOG_INFO("Memory Usage: %u%%", perf_get_memory_usage());
    LOG_INFO("CPU Usage: %u%%", g_perf.cpu_usage);
    LOG_INFO("Total Frames: %u", g_perf.frame_count);

    if (g_perf.counter_count > 0) {
        LOG_INFO("--- Performance Counters ---");
        for (i = 0; i < g_perf.counter_count; i++) {
            PerfCounter* c = &g_perf.counters[i];
            if (c->call_count > 0) {
                LOG_INFO("  %s: %u calls, %u ms total, %u ms avg",
                         c->name, c->call_count, c->total_time,
                         c->total_time / c->call_count);
            }
        }
    }
}

/*
 * Get performance summary string
 */
const char* perf_get_summary(void) {
    static char buffer[256];

    snprintf(buffer, sizeof(buffer),
             "FPS: %u | Frame: %u ms | Mem: %u%% | CPU: %u%%",
             g_perf.fps, g_perf.frame_time,
             perf_get_memory_usage(), g_perf.cpu_usage);

    return buffer;
}

/*
 * Set FPS limit
 */
void perf_set_fps_limit(u32 fps) {
    g_perf.fps_limit = fps;
    g_perf.frame_limit_ms = fps > 0 ? 1000 / fps : 0;
}

/*
 * Get FPS limit
 */
u32 perf_get_fps_limit(void) {
    return g_perf.fps_limit;
}

/*
 * Check if should limit FPS
 */
int perf_should_limit_fps(void) {
    return g_perf.fps_limit > 0;
}

/*
 * Get sleep time for FPS limiting
 */
u32 perf_get_sleep_time(void) {
    if (g_perf.fps_limit == 0) return 0;

    u32 elapsed = timeGetTime() - g_perf.frame_start;
    if (elapsed < g_perf.frame_limit_ms) {
        return g_perf.frame_limit_ms - elapsed;
    }

    return 0;
}

/*
 * Record custom metric
 */
void perf_record_metric(const char* name, u32 value) {
    PerfMetric* metric;
    int i;

    if (!name) return;

    /* Find existing metric */
    for (i = 0; i < g_perf.metric_count; i++) {
        if (strcmp(g_perf.metrics[i].name, name) == 0) {
            g_perf.metrics[i].value = value;
            return;
        }
    }

    /* Add new metric */
    if (g_perf.metric_count < MAX_PERF_METRICS) {
        metric = &g_perf.metrics[g_perf.metric_count];
        strncpy(metric->name, name, 31);
        metric->name[31] = '\0';
        metric->value = value;
        g_perf.metric_count++;
    }
}

/*
 * Get metric value
 */
u32 perf_get_metric(const char* name) {
    int i;

    for (i = 0; i < g_perf.metric_count; i++) {
        if (strcmp(g_perf.metrics[i].name, name) == 0) {
            return g_perf.metrics[i].value;
        }
    }

    return 0;
}
