/*
 * Stone Age Client - Performance Monitor Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include "types.h"

/* Constants */
#define MAX_PERF_COUNTERS    32
#define MAX_PERF_METRICS     32

/* Performance counter */
typedef struct {
    char name[32];
    u32 start_time;
    u32 total_time;
    u32 call_count;
    u32 min_time;
    u32 max_time;
    u8 in_progress;

} PerfCounter;

/* Performance metric */
typedef struct {
    char name[32];
    u32 value;

} PerfMetric;

/* Performance context */
typedef struct {
    /* Frame tracking */
    u32 frame_start;
    u32 frame_time;
    u32 frame_count;
    u32 last_frame_count;
    u32 total_frame_time;
    u32 min_frame_time;
    u32 max_frame_time;
    u32 fps;
    u32 last_fps_update;
    u8 in_frame;

    /* FPS limiting */
    u32 fps_limit;
    u32 frame_limit_ms;

    /* Counters */
    PerfCounter counters[MAX_PERF_COUNTERS];
    int counter_count;

    /* Metrics */
    PerfMetric metrics[MAX_PERF_METRICS];
    int metric_count;

    /* System stats */
    u32 cpu_usage;
    ULONGLONG last_cpu_time;
    ULONGLONG last_sys_time;

    /* State */
    int enabled;

} PerfContext;

/* Global performance context */
extern PerfContext g_perf;

/* Initialization */
int perf_init(void);
void perf_shutdown(void);

/* Frame tracking */
void perf_begin_frame(void);
void perf_end_frame(void);

/* FPS */
u32 perf_get_fps(void);
void perf_set_fps_limit(u32 fps);
u32 perf_get_fps_limit(void);
int perf_should_limit_fps(void);
u32 perf_get_sleep_time(void);

/* Frame time */
u32 perf_get_frame_time(void);
u32 perf_get_avg_frame_time(void);
u32 perf_get_min_frame_time(void);
u32 perf_get_max_frame_time(void);

/* Statistics */
void perf_reset_stats(void);

/* Counters */
void perf_begin(const char* name);
void perf_end(const char* name);
PerfCounter* perf_get_counter(const char* name);
PerfCounter* perf_get_counter_by_index(int index);
int perf_get_counter_count(void);
void perf_reset_counters(void);

/* Metrics */
void perf_record_metric(const char* name, u32 value);
u32 perf_get_metric(const char* name);

/* System */
u32 perf_get_memory_usage(void);
u32 perf_get_available_memory(void);
u32 perf_get_cpu_usage(void);
void perf_update_cpu(void);

/* Enable/disable */
void perf_set_enabled(int enabled);
int perf_is_enabled(void);

/* Reporting */
void perf_print_report(void);
const char* perf_get_summary(void);

#endif /* PERFORMANCE_H */
