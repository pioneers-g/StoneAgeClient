/*
 * Stone Age Client - Performance Monitor Comprehensive Tests
 * Tests for performance.c implementation
 *
 * Covers:
 * - Performance monitor initialization and shutdown
 * - Frame timing (begin/end frame)
 * - FPS calculation and frame statistics
 * - Performance counters (begin/end)
 * - FPS limiting
 * - Custom metrics
 * - Configuration settings
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long ULONGLONG;

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
 * Constants
 * ======================================== */

#define MAX_PERF_COUNTERS 32
#define MAX_PERF_METRICS 64

/* ========================================
 * Structures
 * ======================================== */

typedef struct {
    char name[32];
    u32 start_time;
    u32 total_time;
    u32 call_count;
    u32 min_time;
    u32 max_time;
    int in_progress;
} PerfCounter;

typedef struct {
    char name[32];
    u32 value;
} PerfMetric;

typedef struct {
    int enabled;
    int in_frame;
    u32 frame_start;
    u32 frame_time;
    u32 total_frame_time;
    u32 frame_count;
    u32 min_frame_time;
    u32 max_frame_time;
    u32 fps;
    u32 last_frame_count;
    u32 last_fps_update;
    u32 fps_limit;
    u32 frame_limit_ms;
    u32 cpu_usage;
    ULONGLONG last_cpu_time;
    ULONGLONG last_sys_time;
    int counter_count;
    PerfCounter counters[MAX_PERF_COUNTERS];
    int metric_count;
    PerfMetric metrics[MAX_PERF_METRICS];
} PerfContext;

/* ========================================
 * Global State
 * ======================================== */

static PerfContext g_perf = {0};
static u32 g_simulated_time = 0;

/* ========================================
 * Implementation Functions (Simplified for Testing)
 * ======================================== */

static u32 time_get_time(void) {
    return g_simulated_time;
}

static int perf_init(void) {
    memset(&g_perf, 0, sizeof(PerfContext));

    g_perf.enabled = 1;
    g_perf.frame_count = 0;
    g_perf.fps = 0;
    g_perf.last_fps_update = time_get_time();

    g_perf.cpu_usage = 0;
    g_perf.last_cpu_time = 0;
    g_perf.last_sys_time = 0;

    return 1;
}

static void perf_shutdown(void) {
    memset(&g_perf, 0, sizeof(PerfContext));
}

static void perf_begin_frame(void) {
    if (!g_perf.enabled) return;

    g_perf.frame_start = time_get_time();
    g_perf.in_frame = 1;
}

static void perf_end_frame(void) {
    u32 frame_time;
    u32 current_time;

    if (!g_perf.enabled || !g_perf.in_frame) return;

    current_time = time_get_time();
    frame_time = current_time - g_perf.frame_start;

    g_perf.frame_time = frame_time;
    g_perf.total_frame_time += frame_time;
    g_perf.frame_count++;

    if (frame_time < g_perf.min_frame_time || g_perf.min_frame_time == 0) {
        g_perf.min_frame_time = frame_time;
    }
    if (frame_time > g_perf.max_frame_time) {
        g_perf.max_frame_time = frame_time;
    }

    if (current_time - g_perf.last_fps_update >= 1000) {
        g_perf.fps = g_perf.frame_count - g_perf.last_frame_count;
        g_perf.last_frame_count = g_perf.frame_count;
        g_perf.last_fps_update = current_time;
    }

    g_perf.in_frame = 0;
}

static u32 perf_get_fps(void) {
    return g_perf.fps;
}

static u32 perf_get_frame_time(void) {
    return g_perf.frame_time;
}

static u32 perf_get_avg_frame_time(void) {
    if (g_perf.frame_count == 0) return 0;
    return g_perf.total_frame_time / g_perf.frame_count;
}

static u32 perf_get_min_frame_time(void) {
    return g_perf.min_frame_time;
}

static u32 perf_get_max_frame_time(void) {
    return g_perf.max_frame_time;
}

static void perf_reset_stats(void) {
    g_perf.frame_count = 0;
    g_perf.total_frame_time = 0;
    g_perf.min_frame_time = 0;
    g_perf.max_frame_time = 0;
    g_perf.fps = 0;
    g_perf.last_frame_count = 0;
    g_perf.last_fps_update = time_get_time();
}

static void perf_begin(const char* name) {
    PerfCounter* counter;
    int i;

    if (!g_perf.enabled || !name) return;

    for (i = 0; i < g_perf.counter_count; i++) {
        if (strcmp(g_perf.counters[i].name, name) == 0) {
            counter = &g_perf.counters[i];
            counter->start_time = time_get_time();
            counter->in_progress = 1;
            return;
        }
    }

    if (g_perf.counter_count < MAX_PERF_COUNTERS) {
        counter = &g_perf.counters[g_perf.counter_count];
        strncpy(counter->name, name, 31);
        counter->name[31] = '\0';
        counter->start_time = time_get_time();
        counter->total_time = 0;
        counter->call_count = 0;
        counter->in_progress = 1;
        g_perf.counter_count++;
    }
}

static void perf_end(const char* name) {
    PerfCounter* counter;
    int i;
    u32 elapsed;

    if (!g_perf.enabled || !name) return;

    for (i = 0; i < g_perf.counter_count; i++) {
        if (strcmp(g_perf.counters[i].name, name) == 0) {
            counter = &g_perf.counters[i];
            if (counter->in_progress) {
                elapsed = time_get_time() - counter->start_time;
                counter->total_time += elapsed;
                counter->call_count++;
                counter->in_progress = 0;

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

static PerfCounter* perf_get_counter(const char* name) {
    int i;

    for (i = 0; i < g_perf.counter_count; i++) {
        if (strcmp(g_perf.counters[i].name, name) == 0) {
            return &g_perf.counters[i];
        }
    }

    return NULL;
}

static int perf_get_counter_count(void) {
    return g_perf.counter_count;
}

static PerfCounter* perf_get_counter_by_index(int index) {
    if (index < 0 || index >= g_perf.counter_count) {
        return NULL;
    }
    return &g_perf.counters[index];
}

static void perf_reset_counters(void) {
    int i;

    for (i = 0; i < g_perf.counter_count; i++) {
        g_perf.counters[i].total_time = 0;
        g_perf.counters[i].call_count = 0;
        g_perf.counters[i].min_time = 0;
        g_perf.counters[i].max_time = 0;
        g_perf.counters[i].in_progress = 0;
    }
}

static void perf_set_enabled(int enabled) {
    g_perf.enabled = enabled;
}

static int perf_is_enabled(void) {
    return g_perf.enabled;
}

static u32 perf_get_cpu_usage(void) {
    return g_perf.cpu_usage;
}

static void perf_set_fps_limit(u32 fps) {
    g_perf.fps_limit = fps;
    g_perf.frame_limit_ms = fps > 0 ? 1000 / fps : 0;
}

static u32 perf_get_fps_limit(void) {
    return g_perf.fps_limit;
}

static int perf_should_limit_fps(void) {
    return g_perf.fps_limit > 0;
}

static u32 perf_get_sleep_time(void) {
    if (g_perf.fps_limit == 0) return 0;

    u32 elapsed = time_get_time() - g_perf.frame_start;
    if (elapsed < g_perf.frame_limit_ms) {
        return g_perf.frame_limit_ms - elapsed;
    }

    return 0;
}

static void perf_record_metric(const char* name, u32 value) {
    PerfMetric* metric;
    int i;

    if (!name) return;

    for (i = 0; i < g_perf.metric_count; i++) {
        if (strcmp(g_perf.metrics[i].name, name) == 0) {
            g_perf.metrics[i].value = value;
            return;
        }
    }

    if (g_perf.metric_count < MAX_PERF_METRICS) {
        metric = &g_perf.metrics[g_perf.metric_count];
        strncpy(metric->name, name, 31);
        metric->name[31] = '\0';
        metric->value = value;
        g_perf.metric_count++;
    }
}

static u32 perf_get_metric(const char* name) {
    int i;

    for (i = 0; i < g_perf.metric_count; i++) {
        if (strcmp(g_perf.metrics[i].name, name) == 0) {
            return g_perf.metrics[i].value;
        }
    }

    return 0;
}

/* ========================================
 * Test Cases - Initialization
 * ======================================== */

static int test_init_basic(void) {
    int result = perf_init();
    return result == 1 && g_perf.enabled == 1;
}

static int test_init_frame_count_zero(void) {
    perf_init();
    return g_perf.frame_count == 0;
}

static int test_init_fps_zero(void) {
    perf_init();
    return g_perf.fps == 0;
}

static int test_init_counter_count_zero(void) {
    perf_init();
    return g_perf.counter_count == 0;
}

static int test_shutdown_clears_state(void) {
    perf_init();
    perf_begin_frame();
    perf_shutdown();
    return g_perf.enabled == 0 && g_perf.frame_count == 0;
}

/* ========================================
 * Test Cases - Frame Timing
 * ======================================== */

static int test_begin_frame_sets_flag(void) {
    perf_init();
    perf_begin_frame();
    return g_perf.in_frame == 1;
}

static int test_end_frame_clears_flag(void) {
    perf_init();
    g_simulated_time = 0;
    perf_begin_frame();
    g_simulated_time = 16;
    perf_end_frame();
    return g_perf.in_frame == 0;
}

static int test_end_frame_increments_count(void) {
    perf_init();
    g_simulated_time = 0;
    perf_begin_frame();
    g_simulated_time = 16;
    perf_end_frame();
    return g_perf.frame_count == 1;
}

static int test_end_frame_tracks_time(void) {
    perf_init();
    g_simulated_time = 0;
    perf_begin_frame();
    g_simulated_time = 16;
    perf_end_frame();
    return g_perf.frame_time == 16;
}

static int test_begin_frame_disabled(void) {
    perf_init();
    perf_set_enabled(0);
    perf_begin_frame();
    return g_perf.in_frame == 0;
}

static int test_end_frame_not_in_frame(void) {
    perf_init();
    g_perf.in_frame = 0;
    g_simulated_time = 0;
    perf_end_frame();
    return g_perf.frame_count == 0;
}

/* ========================================
 * Test Cases - Frame Statistics
 * ======================================== */

static int test_min_frame_time(void) {
    perf_init();
    g_simulated_time = 0;
    perf_begin_frame();
    g_simulated_time = 10;
    perf_end_frame();
    perf_begin_frame();
    g_simulated_time = 20;
    perf_end_frame();
    return g_perf.min_frame_time == 10;
}

static int test_max_frame_time(void) {
    perf_init();
    g_simulated_time = 0;
    perf_begin_frame();
    g_simulated_time = 10;
    perf_end_frame();
    perf_begin_frame();
    g_simulated_time = 25;
    perf_end_frame();
    return g_perf.max_frame_time == 15;
}

static int test_avg_frame_time(void) {
    perf_init();
    g_simulated_time = 0;
    perf_begin_frame();
    g_simulated_time = 10;
    perf_end_frame();
    perf_begin_frame();
    g_simulated_time = 20;
    perf_end_frame();
    /* Total time = 10 + 10 = 20, frames = 2, avg = 10 */
    return perf_get_avg_frame_time() == 10;
}

static int test_avg_frame_time_zero_frames(void) {
    perf_init();
    return perf_get_avg_frame_time() == 0;
}

static int test_reset_stats(void) {
    perf_init();
    g_simulated_time = 0;
    perf_begin_frame();
    g_simulated_time = 16;
    perf_end_frame();
    perf_reset_stats();
    return g_perf.frame_count == 0 && g_perf.total_frame_time == 0;
}

/* ========================================
 * Test Cases - FPS Calculation
 * ======================================== */

static int test_get_fps_initial(void) {
    perf_init();
    return perf_get_fps() == 0;
}

static int test_fps_updates_after_second(void) {
    perf_init();
    g_simulated_time = 0;

    /* Set last_fps_update to a time in the past so condition triggers */
    g_perf.last_fps_update = 0;

    /* Simulate 10 frames */
    int i;
    for (i = 0; i < 10; i++) {
        perf_begin_frame();
        g_simulated_time += 16;
        perf_end_frame();
    }

    /* Move time past 1 second to trigger FPS update */
    g_simulated_time = 1100;
    perf_begin_frame();
    g_simulated_time += 16;
    perf_end_frame();

    /* FPS should now be set (frames since last_frame_count) */
    return g_perf.fps == 11;  /* 10 frames + 1 for the trigger frame */
}

/* ========================================
 * Test Cases - Performance Counters
 * ======================================== */

static int test_begin_counter_creates_new(void) {
    perf_init();
    perf_begin("render");
    return g_perf.counter_count == 1;
}

static int test_begin_counter_sets_name(void) {
    perf_init();
    perf_begin("render");
    return strcmp(g_perf.counters[0].name, "render") == 0;
}

static int test_begin_counter_null_name(void) {
    perf_init();
    perf_begin(NULL);
    return g_perf.counter_count == 0;
}

static int test_begin_counter_finds_existing(void) {
    perf_init();
    perf_begin("render");
    perf_end("render");
    perf_begin("render");
    /* Should find existing counter, not create new one */
    return g_perf.counter_count == 1;
}

static int test_end_counter_records_time(void) {
    perf_init();
    g_simulated_time = 0;
    perf_begin("render");
    g_simulated_time = 100;
    perf_end("render");
    return g_perf.counters[0].total_time == 100;
}

static int test_end_counter_increments_count(void) {
    perf_init();
    g_simulated_time = 0;
    perf_begin("render");
    g_simulated_time = 100;
    perf_end("render");
    return g_perf.counters[0].call_count == 1;
}

static int test_end_counter_null_name(void) {
    perf_init();
    perf_begin("test");
    perf_end(NULL);
    /* Counter should not have been updated */
    return g_perf.counters[0].call_count == 0;
}

static int test_counter_multiple_calls(void) {
    perf_init();
    g_simulated_time = 0;

    perf_begin("render");
    g_simulated_time = 50;
    perf_end("render");

    perf_begin("render");
    g_simulated_time = 150;
    perf_end("render");

    return g_perf.counters[0].call_count == 2 &&
           g_perf.counters[0].total_time == 150;
}

static int test_counter_min_max(void) {
    perf_init();
    g_simulated_time = 0;

    perf_begin("render");
    g_simulated_time = 50;
    perf_end("render");

    perf_begin("render");
    g_simulated_time = 200;
    perf_end("render");

    return g_perf.counters[0].min_time == 50 &&
           g_perf.counters[0].max_time == 150;
}

static int test_get_counter_found(void) {
    perf_init();
    perf_begin("render");
    PerfCounter* counter = perf_get_counter("render");
    return counter != NULL && strcmp(counter->name, "render") == 0;
}

static int test_get_counter_not_found(void) {
    perf_init();
    perf_begin("render");
    return perf_get_counter("update") == NULL;
}

static int test_get_counter_count(void) {
    perf_init();
    perf_begin("render");
    perf_begin("update");
    perf_begin("physics");
    return perf_get_counter_count() == 3;
}

static int test_get_counter_by_index_valid(void) {
    perf_init();
    perf_begin("render");
    perf_begin("update");
    PerfCounter* counter = perf_get_counter_by_index(1);
    return counter != NULL && strcmp(counter->name, "update") == 0;
}

static int test_get_counter_by_index_invalid(void) {
    perf_init();
    perf_begin("render");
    return perf_get_counter_by_index(5) == NULL;
}

static int test_reset_counters(void) {
    perf_init();
    g_simulated_time = 0;
    perf_begin("render");
    g_simulated_time = 50;
    perf_end("render");

    perf_reset_counters();

    return g_perf.counters[0].call_count == 0 &&
           g_perf.counters[0].total_time == 0;
}

static int test_max_counters(void) {
    char name[16];
    int i;

    perf_init();

    for (i = 0; i < MAX_PERF_COUNTERS + 5; i++) {
        sprintf(name, "counter%d", i);
        perf_begin(name);
    }

    return g_perf.counter_count == MAX_PERF_COUNTERS;
}

/* ========================================
 * Test Cases - Configuration
 * ======================================== */

static int test_set_enabled(void) {
    perf_init();
    perf_set_enabled(0);
    return perf_is_enabled() == 0;
}

static int test_is_enabled_initial(void) {
    perf_init();
    return perf_is_enabled() == 1;
}

static int test_counter_disabled(void) {
    perf_init();
    perf_set_enabled(0);
    perf_begin("render");
    return g_perf.counter_count == 0;
}

/* ========================================
 * Test Cases - FPS Limiting
 * ======================================== */

static int test_set_fps_limit(void) {
    perf_init();
    perf_set_fps_limit(60);
    return g_perf.fps_limit == 60 && g_perf.frame_limit_ms == 16;
}

static int test_set_fps_limit_zero(void) {
    perf_init();
    perf_set_fps_limit(0);
    return g_perf.fps_limit == 0 && g_perf.frame_limit_ms == 0;
}

static int test_get_fps_limit(void) {
    perf_init();
    perf_set_fps_limit(30);
    return perf_get_fps_limit() == 30;
}

static int test_should_limit_fps_true(void) {
    perf_init();
    perf_set_fps_limit(60);
    return perf_should_limit_fps() == 1;
}

static int test_should_limit_fps_false(void) {
    perf_init();
    perf_set_fps_limit(0);
    return perf_should_limit_fps() == 0;
}

static int test_get_sleep_time_needed(void) {
    perf_init();
    perf_set_fps_limit(60);  /* ~16ms per frame */
    g_simulated_time = 0;
    perf_begin_frame();
    g_simulated_time = 10;  /* 10ms elapsed */
    /* Need 16-10 = 6ms more */
    return perf_get_sleep_time() == 6;
}

static int test_get_sleep_time_not_needed(void) {
    perf_init();
    perf_set_fps_limit(60);
    g_simulated_time = 0;
    perf_begin_frame();
    g_simulated_time = 20;  /* 20ms elapsed, already past 16ms target */
    return perf_get_sleep_time() == 0;
}

/* ========================================
 * Test Cases - Custom Metrics
 * ======================================== */

static int test_record_metric_creates_new(void) {
    perf_init();
    perf_record_metric("draw_calls", 150);
    return g_perf.metric_count == 1;
}

static int test_record_metric_sets_value(void) {
    perf_init();
    perf_record_metric("draw_calls", 150);
    return g_perf.metrics[0].value == 150;
}

static int test_record_metric_updates_existing(void) {
    perf_init();
    perf_record_metric("draw_calls", 150);
    perf_record_metric("draw_calls", 200);
    return g_perf.metrics[0].value == 200 && g_perf.metric_count == 1;
}

static int test_record_metric_null_name(void) {
    perf_init();
    perf_record_metric(NULL, 100);
    return g_perf.metric_count == 0;
}

static int test_get_metric_found(void) {
    perf_init();
    perf_record_metric("draw_calls", 150);
    return perf_get_metric("draw_calls") == 150;
}

static int test_get_metric_not_found(void) {
    perf_init();
    return perf_get_metric("nonexistent") == 0;
}

static int test_max_metrics(void) {
    char name[16];
    int i;

    perf_init();

    for (i = 0; i < MAX_PERF_METRICS + 5; i++) {
        sprintf(name, "metric%d", i);
        perf_record_metric(name, i);
    }

    return g_perf.metric_count == MAX_PERF_METRICS;
}

/* ========================================
 * Test Cases - CPU Usage
 * ======================================== */

static int test_get_cpu_usage_initial(void) {
    perf_init();
    return perf_get_cpu_usage() == 0;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Performance Monitor Comprehensive Tests ===\n\n");

    printf("Initialization Tests:\n");
    TEST(init_basic);
    TEST(init_frame_count_zero);
    TEST(init_fps_zero);
    TEST(init_counter_count_zero);
    TEST(shutdown_clears_state);

    printf("\nFrame Timing Tests:\n");
    TEST(begin_frame_sets_flag);
    TEST(end_frame_clears_flag);
    TEST(end_frame_increments_count);
    TEST(end_frame_tracks_time);
    TEST(begin_frame_disabled);
    TEST(end_frame_not_in_frame);

    printf("\nFrame Statistics Tests:\n");
    TEST(min_frame_time);
    TEST(max_frame_time);
    TEST(avg_frame_time);
    TEST(avg_frame_time_zero_frames);
    TEST(reset_stats);

    printf("\nFPS Calculation Tests:\n");
    TEST(get_fps_initial);
    TEST(fps_updates_after_second);

    printf("\nPerformance Counter Tests:\n");
    TEST(begin_counter_creates_new);
    TEST(begin_counter_sets_name);
    TEST(begin_counter_null_name);
    TEST(begin_counter_finds_existing);
    TEST(end_counter_records_time);
    TEST(end_counter_increments_count);
    TEST(end_counter_null_name);
    TEST(counter_multiple_calls);
    TEST(counter_min_max);
    TEST(get_counter_found);
    TEST(get_counter_not_found);
    TEST(get_counter_count);
    TEST(get_counter_by_index_valid);
    TEST(get_counter_by_index_invalid);
    TEST(reset_counters);
    TEST(max_counters);

    printf("\nConfiguration Tests:\n");
    TEST(set_enabled);
    TEST(is_enabled_initial);
    TEST(counter_disabled);

    printf("\nFPS Limiting Tests:\n");
    TEST(set_fps_limit);
    TEST(set_fps_limit_zero);
    TEST(get_fps_limit);
    TEST(should_limit_fps_true);
    TEST(should_limit_fps_false);
    TEST(get_sleep_time_needed);
    TEST(get_sleep_time_not_needed);

    printf("\nCustom Metrics Tests:\n");
    TEST(record_metric_creates_new);
    TEST(record_metric_sets_value);
    TEST(record_metric_updates_existing);
    TEST(record_metric_null_name);
    TEST(get_metric_found);
    TEST(get_metric_not_found);
    TEST(max_metrics);

    printf("\nCPU Usage Tests:\n");
    TEST(get_cpu_usage_initial);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
