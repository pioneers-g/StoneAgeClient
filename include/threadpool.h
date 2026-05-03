/*
 * Stone Age Client - Thread Pool System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "types.h"

/* Constants */
#define MAX_THREAD_POOLS    4
#define MAX_THREADS         8
#define MAX_TASKS           256

/* Task callback function type */
typedef void (*TaskCallback)(void* data);

/* Task structure */
typedef struct {
    TaskCallback callback;
    void* data;
    u8 priority;
    u8 reserved[3];

} Task;

/* Thread pool structure */
typedef struct {
    char name[32];
    HANDLE threads[MAX_THREADS];
    u32 thread_count;
    CRITICAL_SECTION lock;
    HANDLE semaphore;
    int running;
    int active;

    /* Task queue */
    Task tasks[MAX_TASKS];
    u32 task_head;
    u32 task_tail;
    u32 task_count;

    /* Statistics */
    volatile u32 submitted_tasks;
    volatile u32 completed_tasks;

} ThreadPool;

/* Thread pool context */
typedef struct {
    ThreadPool pools[MAX_THREAD_POOLS];
    int pool_count;
    u32 default_thread_count;
    int enabled;

} ThreadPoolContext;

/* Global thread pool context */
extern ThreadPoolContext g_threadpool;

/* Initialization */
int threadpool_init(void);
void threadpool_shutdown(void);

/* Pool management */
ThreadPool* threadpool_create(const char* name, u32 thread_count);
void threadpool_destroy(ThreadPool* pool);
ThreadPool* threadpool_get(const char* name);
int threadpool_get_pool_count(void);

/* Task submission */
int threadpool_submit(ThreadPool* pool, TaskCallback callback, void* data);
int threadpool_submit_priority(ThreadPool* pool, TaskCallback callback, void* data, u8 priority);
int threadpool_run_async(TaskCallback callback, void* data);
int threadpool_schedule(ThreadPool* pool, TaskCallback callback, void* data, u32 delay_ms);

/* Waiting */
void threadpool_wait(ThreadPool* pool);
int threadpool_wait_timeout(ThreadPool* pool, u32 timeout_ms);

/* Query */
u32 threadpool_get_pending_count(ThreadPool* pool);
u32 threadpool_get_completed_count(ThreadPool* pool);
u32 threadpool_get_thread_count(ThreadPool* pool);
void threadpool_get_stats(ThreadPool* pool, u32* submitted, u32* completed, u32* pending);

/* Settings */
void threadpool_set_default_threads(u32 count);
void threadpool_set_enabled(int enabled);
int threadpool_is_enabled(void);

#endif /* THREADPOOL_H */
