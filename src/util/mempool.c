/*
 * Stone Age Client - Memory Pool System Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "mempool.h"
#include "logger.h"

/* Global memory pool context */
MemPoolContext g_mempool = {0};

/*
 * Initialize memory pool system
 */
int mempool_init(void) {
    memset(&g_mempool, 0, sizeof(MemPoolContext));

    g_mempool.enabled = 1;
    g_mempool.debug = 0;

    LOG_INFO("Memory pool system initialized");
    return 1;
}

/*
 * Shutdown memory pool system
 */
void mempool_shutdown(void) {
    int i;

    /* Release all pools */
    for (i = 0; i < MAX_MEM_POOLS; i++) {
        if (g_mempool.pools[i].memory) {
            free(g_mempool.pools[i].memory);
            g_mempool.pools[i].memory = NULL;
        }
    }

    memset(&g_mempool, 0, sizeof(MemPoolContext));
    LOG_INFO("Memory pool system shutdown");
}

/*
 * Create a memory pool
 */
MemPool* mempool_create(u32 block_size, u32 block_count, const char* name) {
    MemPool* pool;
    u32 total_size;
    u32 i;
    u8* ptr;
    MemBlock* block;

    if (g_mempool.pool_count >= MAX_MEM_POOLS) {
        LOG_ERROR("Maximum pool count reached");
        return NULL;
    }

    pool = &g_mempool.pools[g_mempool.pool_count];

    /* Calculate total size */
    total_size = block_size * block_count;

    /* Allocate memory */
    pool->memory = (u8*)malloc(total_size);
    if (!pool->memory) {
        LOG_ERROR("Failed to allocate pool memory");
        return NULL;
    }

    /* Initialize pool */
    pool->block_size = block_size;
    pool->block_count = block_count;
    pool->free_count = block_count;
    pool->free_list = NULL;

    if (name) {
        strncpy(pool->name, name, 31);
        pool->name[31] = '\0';
    }

    /* Build free list */
    ptr = pool->memory;
    for (i = 0; i < block_count; i++) {
        block = (MemBlock*)ptr;
        block->next = pool->free_list;
        block->pool = pool;
        pool->free_list = block;
        ptr += block_size;
    }

    g_mempool.pool_count++;
    g_mempool.total_allocated += total_size;

    LOG_DEBUG("Created pool '%s': %u blocks x %u bytes", name, block_count, block_size);
    return pool;
}

/*
 * Destroy a memory pool
 */
void mempool_destroy(MemPool* pool) {
    int i;

    if (!pool) return;

    /* Find and remove pool */
    for (i = 0; i < g_mempool.pool_count; i++) {
        if (&g_mempool.pools[i] == pool) {
            if (pool->memory) {
                g_mempool.total_allocated -= pool->block_size * pool->block_count;
                free(pool->memory);
            }
            memset(pool, 0, sizeof(MemPool));

            /* Shift remaining pools */
            for (; i < g_mempool.pool_count - 1; i++) {
                g_mempool.pools[i] = g_mempool.pools[i + 1];
            }
            g_mempool.pool_count--;
            return;
        }
    }
}

/*
 * Allocate from pool
 */
void* mempool_alloc(MemPool* pool) {
    MemBlock* block;

    if (!pool || !pool->free_list) {
        return NULL;
    }

    /* Get block from free list */
    block = pool->free_list;
    pool->free_list = block->next;
    pool->free_count--;

    g_mempool.alloc_count++;

    return (void*)((u8*)block + sizeof(MemBlock));
}

/*
 * Free to pool
 */
void mempool_free(void* ptr) {
    MemBlock* block;
    MemPool* pool;

    if (!ptr) return;

    /* Get block header */
    block = (MemBlock*)((u8*)ptr - sizeof(MemBlock));
    pool = block->pool;

    if (!pool) {
        LOG_WARN("Invalid pool free");
        return;
    }

    /* Add back to free list */
    block->next = pool->free_list;
    pool->free_list = block;
    pool->free_count++;

    g_mempool.free_count++;
}

/*
 * Get pool by name
 */
MemPool* mempool_get(const char* name) {
    int i;

    for (i = 0; i < g_mempool.pool_count; i++) {
        if (strcmp(g_mempool.pools[i].name, name) == 0) {
            return &g_mempool.pools[i];
        }
    }

    return NULL;
}

/*
 * Get pool statistics
 */
void mempool_get_stats(MemPool* pool, u32* total, u32* used, u32* free) {
    if (!pool) return;

    if (total) *total = pool->block_count;
    if (used) *used = pool->block_count - pool->free_count;
    if (free) *free = pool->free_count;
}

/*
 * Check if pointer is from pool
 */
int mempool_is_from_pool(MemPool* pool, void* ptr) {
    u8* p = (u8*)ptr;

    if (!pool || !ptr) return 0;

    return (p >= pool->memory && p < pool->memory + pool->block_size * pool->block_count);
}

/*
 * Compact pool (defragment)
 */
void mempool_compact(MemPool* pool) {
    /* Simple pools don't need compaction */
}

/*
 * Reset pool (free all blocks)
 */
void mempool_reset(MemPool* pool) {
    u32 i;
    u8* ptr;
    MemBlock* block;

    if (!pool) return;

    pool->free_list = NULL;
    pool->free_count = pool->block_count;

    ptr = pool->memory;
    for (i = 0; i < pool->block_count; i++) {
        block = (MemBlock*)ptr;
        block->next = pool->free_list;
        block->pool = pool;
        pool->free_list = block;
        ptr += pool->block_size;
    }
}

/*
 * Get total allocated memory
 */
u32 mempool_get_total_allocated(void) {
    return g_mempool.total_allocated;
}

/*
 * Get allocation count
 */
u32 mempool_get_alloc_count(void) {
    return g_mempool.alloc_count;
}

/*
 * Get free count
 */
u32 mempool_get_free_count(void) {
    return g_mempool.free_count;
}

/*
 * Enable debug mode
 */
void mempool_set_debug(int enabled) {
    g_mempool.debug = enabled;
}

/*
 * Print pool statistics
 */
void mempool_print_stats(void) {
    int i;

    LOG_INFO("=== Memory Pool Statistics ===");
    LOG_INFO("Total pools: %d", g_mempool.pool_count);
    LOG_INFO("Total allocated: %u bytes", g_mempool.total_allocated);
    LOG_INFO("Allocations: %u, Frees: %u", g_mempool.alloc_count, g_mempool.free_count);

    for (i = 0; i < g_mempool.pool_count; i++) {
        MemPool* pool = &g_mempool.pools[i];
        LOG_INFO("Pool '%s': %u/%u blocks used",
                 pool->name,
                 pool->block_count - pool->free_count,
                 pool->block_count);
    }
}

/* Object Pool Implementation */

/*
 * Initialize object pool
 */
int objpool_init(ObjectPool* pool, u32 object_size, u32 capacity, const char* name) {
    if (!pool) return 0;

    memset(pool, 0, sizeof(ObjectPool));

    pool->objects = (u8*)malloc(object_size * capacity);
    if (!pool->objects) {
        return 0;
    }

    pool->object_size = object_size;
    pool->capacity = capacity;
    pool->count = 0;

    if (name) {
        strncpy(pool->name, name, 31);
        pool->name[31] = '\0';
    }

    /* Build free list */
    pool->free_list = NULL;
    for (u32 i = 0; i < capacity; i++) {
        ObjectEntry* entry = (ObjectEntry*)(pool->objects + i * object_size);
        entry->next = pool->free_list;
        entry->active = 0;
        pool->free_list = entry;
    }

    return 1;
}

/*
 * Shutdown object pool
 */
void objpool_shutdown(ObjectPool* pool) {
    if (!pool) return;

    if (pool->objects) {
        free(pool->objects);
        pool->objects = NULL;
    }

    memset(pool, 0, sizeof(ObjectPool));
}

/*
 * Allocate object from pool
 */
void* objpool_alloc(ObjectPool* pool) {
    ObjectEntry* entry;

    if (!pool || !pool->free_list) {
        return NULL;
    }

    entry = pool->free_list;
    pool->free_list = entry->next;
    entry->active = 1;
    pool->count++;

    return (void*)entry;
}

/*
 * Free object to pool
 */
void objpool_free(ObjectPool* pool, void* obj) {
    ObjectEntry* entry;

    if (!pool || !obj) return;

    entry = (ObjectEntry*)obj;
    entry->active = 0;
    entry->next = pool->free_list;
    pool->free_list = entry;
    pool->count--;
}

/*
 * Get object by index
 */
void* objpool_get(ObjectPool* pool, u32 index) {
    if (!pool || index >= pool->capacity) {
        return NULL;
    }

    return pool->objects + index * pool->object_size;
}

/*
 * Iterate active objects
 */
void* objpool_first(ObjectPool* pool, u32* index) {
    u32 i;

    if (!pool) return NULL;

    for (i = 0; i < pool->capacity; i++) {
        ObjectEntry* entry = (ObjectEntry*)(pool->objects + i * pool->object_size);
        if (entry->active) {
            if (index) *index = i;
            return entry;
        }
    }

    return NULL;
}

/*
 * Iterate next active object
 */
void* objpool_next(ObjectPool* pool, u32* index) {
    u32 i;

    if (!pool || !index) return NULL;

    for (i = *index + 1; i < pool->capacity; i++) {
        ObjectEntry* entry = (ObjectEntry*)(pool->objects + i * pool->object_size);
        if (entry->active) {
            *index = i;
            return entry;
        }
    }

    return NULL;
}

/*
 * Get active count
 */
u32 objpool_count(ObjectPool* pool) {
    return pool ? pool->count : 0;
}

/*
 * Clear all objects
 */
void objpool_clear(ObjectPool* pool) {
    u32 i;

    if (!pool) return;

    pool->free_list = NULL;
    pool->count = 0;

    for (i = 0; i < pool->capacity; i++) {
        ObjectEntry* entry = (ObjectEntry*)(pool->objects + i * pool->object_size);
        entry->next = pool->free_list;
        entry->active = 0;
        pool->free_list = entry;
    }
}
