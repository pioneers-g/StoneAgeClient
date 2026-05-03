/*
 * Stone Age Client - Thread Pool System Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "threadpool.h"
#include "logger.h"

/* Global thread pool context */
ThreadPoolContext g_threadpool = {0};

/* Worker thread function */
static DWORD WINAPI threadpool_worker(LPVOID param) {
    ThreadPool* pool = (ThreadPool*)param;
    Task* task;

    while (pool->running) {
        /* Wait for task */
        WaitForSingleObject(pool->semaphore, INFINITE);

        if (!pool->running) break;

        /* Get task from queue */
        EnterCriticalSection(&pool->lock);
        if (pool->task_count > 0) {
            task = &pool->tasks[pool->task_head];
            pool->task_head = (pool->task_head + 1) % MAX_TASKS;
            pool->task_count--;
        } else {
            task = NULL;
        }
        LeaveCriticalSection(&pool->lock);

        /* Execute task */
        if (task && task->callback) {
            task->callback(task->data);
            InterlockedIncrement(&pool->completed_tasks);
        }
    }

    return 0;
}

/*
 * Initialize thread pool system
 */
int threadpool_init(void) {
    memset(&g_threadpool, 0, sizeof(ThreadPoolContext));

    g_threadpool.enabled = 1;
    g_threadpool.default_thread_count = 2;

    LOG_INFO("Thread pool system initialized");
    return 1;
}

/*
 * Shutdown thread pool system
 */
void threadpool_shutdown(void) {
    int i;

    for (i = 0; i < g_threadpool.pool_count; i++) {
        if (g_threadpool.pools[i].active) {
            threadpool_destroy(&g_threadpool.pools[i]);
        }
    }

    memset(&g_threadpool, 0, sizeof(ThreadPoolContext));
    LOG_INFO("Thread pool system shutdown");
}

/*
 * Create a thread pool
 */
ThreadPool* threadpool_create(const char* name, u32 thread_count) {
    ThreadPool* pool;
    u32 i;

    if (g_threadpool.pool_count >= MAX_THREAD_POOLS) {
        LOG_ERROR("Maximum thread pool count reached");
        return NULL;
    }

    if (thread_count == 0) {
        thread_count = g_threadpool.default_thread_count;
    }

    pool = &g_threadpool.pools[g_threadpool.pool_count];

    /* Initialize pool */
    memset(pool, 0, sizeof(ThreadPool));
    if (name) {
        strncpy(pool->name, name, 31);
        pool->name[31] = '\0';
    }

    /* Create synchronization objects */
    InitializeCriticalSection(&pool->lock);
    pool->semaphore = CreateSemaphore(NULL, 0, MAX_TASKS, NULL);
    if (!pool->semaphore) {
        LOG_ERROR("Failed to create semaphore");
        DeleteCriticalSection(&pool->lock);
        return NULL;
    }

    /* Create threads */
    pool->thread_count = thread_count;
    for (i = 0; i < thread_count; i++) {
        pool->threads[i] = CreateThread(NULL, 0, threadpool_worker, pool, 0, NULL);
        if (!pool->threads[i]) {
            LOG_ERROR("Failed to create thread %d", i);
            threadpool_destroy(pool);
            return NULL;
        }
    }

    pool->running = 1;
    pool->active = 1;

    g_threadpool.pool_count++;
    LOG_DEBUG("Created thread pool '%s' with %u threads", name, thread_count);

    return pool;
}

/*
 * Destroy a thread pool
 */
void threadpool_destroy(ThreadPool* pool) {
    u32 i;

    if (!pool || !pool->active) return;

    /* Signal threads to stop */
    pool->running = 0;

    /* Release semaphore to wake up threads */
    for (i = 0; i < pool->thread_count; i++) {
        ReleaseSemaphore(pool->semaphore, 1, NULL);
    }

    /* Wait for threads to finish */
    WaitForMultipleObjects(pool->thread_count, pool->threads, TRUE, 5000);

    /* Close handles */
    for (i = 0; i < pool->thread_count; i++) {
        if (pool->threads[i]) {
            CloseHandle(pool->threads[i]);
        }
    }

    if (pool->semaphore) {
        CloseHandle(pool->semaphore);
    }

    DeleteCriticalSection(&pool->lock);

    pool->active = 0;
    LOG_DEBUG("Destroyed thread pool '%s'", pool->name);
}

/*
 * Submit a task to the pool
 */
int threadpool_submit(ThreadPool* pool, TaskCallback callback, void* data) {
    Task* task;

    if (!pool || !pool->active || !pool->running || !callback) {
        return 0;
    }

    EnterCriticalSection(&pool->lock);

    if (pool->task_count >= MAX_TASKS) {
        LeaveCriticalSection(&pool->lock);
        LOG_WARN("Thread pool '%s' task queue full", pool->name);
        return 0;
    }

    /* Add task to queue */
    task = &pool->tasks[pool->task_tail];
    task->callback = callback;
    task->data = data;

    pool->task_tail = (pool->task_tail + 1) % MAX_TASKS;
    pool->task_count++;
    pool->submitted_tasks++;

    LeaveCriticalSection(&pool->lock);

    /* Signal worker */
    ReleaseSemaphore(pool->semaphore, 1, NULL);

    return 1;
}

/*
 * Submit task with priority
 */
int threadpool_submit_priority(ThreadPool* pool, TaskCallback callback, void* data, u8 priority) {
    /* Simple implementation - priority not yet supported */
    return threadpool_submit(pool, callback, data);
}

/*
 * Wait for all tasks to complete
 */
void threadpool_wait(ThreadPool* pool) {
    if (!pool || !pool->active) return;

    while (pool->task_count > 0) {
        Sleep(1);
    }

    /* Wait a bit more for in-progress tasks */
    Sleep(10);
}

/*
 * Wait with timeout
 */
int threadpool_wait_timeout(ThreadPool* pool, u32 timeout_ms) {
    u32 start = timeGetTime();

    if (!pool || !pool->active) return 1;

    while (pool->task_count > 0) {
        if (timeGetTime() - start >= timeout_ms) {
            return 0;
        }
        Sleep(1);
    }

    return 1;
}

/*
 * Get pending task count
 */
u32 threadpool_get_pending_count(ThreadPool* pool) {
    return pool ? pool->task_count : 0;
}

/*
 * Get completed task count
 */
u32 threadpool_get_completed_count(ThreadPool* pool) {
    return pool ? pool->completed_tasks : 0;
}

/*
 * Get thread count
 */
u32 threadpool_get_thread_count(ThreadPool* pool) {
    return pool ? pool->thread_count : 0;
}

/*
 * Get pool by name
 */
ThreadPool* threadpool_get(const char* name) {
    int i;

    for (i = 0; i < g_threadpool.pool_count; i++) {
        if (g_threadpool.pools[i].active &&
            strcmp(g_threadpool.pools[i].name, name) == 0) {
            return &g_threadpool.pools[i];
        }
    }

    return NULL;
}

/*
 * Get pool count
 */
int threadpool_get_pool_count(void) {
    return g_threadpool.pool_count;
}

/*
 * Set default thread count
 */
void threadpool_set_default_threads(u32 count) {
    g_threadpool.default_thread_count = count;
}

/*
 * Enable/disable thread pool
 */
void threadpool_set_enabled(int enabled) {
    g_threadpool.enabled = enabled;
}

/*
 * Check if enabled
 */
int threadpool_is_enabled(void) {
    return g_threadpool.enabled;
}

/*
 * Simple async task
 */
int threadpool_run_async(TaskCallback callback, void* data) {
    static ThreadPool* default_pool = NULL;

    if (!default_pool) {
        default_pool = threadpool_create("default", 0);
    }

    return threadpool_submit(default_pool, callback, data);
}

/*
 * Schedule task for later
 */
int threadpool_schedule(ThreadPool* pool, TaskCallback callback, void* data, u32 delay_ms) {
    /* Would need timer integration - simplified version */
    return threadpool_submit(pool, callback, data);
}

/*
 * Get pool statistics
 */
void threadpool_get_stats(ThreadPool* pool, u32* submitted, u32* completed, u32* pending) {
    if (!pool) return;

    if (submitted) *submitted = pool->submitted_tasks;
    if (completed) *completed = pool->completed_tasks;
    if (pending) *pending = pool->task_count;
}
