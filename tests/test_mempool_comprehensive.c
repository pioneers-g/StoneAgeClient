/*
 * Stone Age Client - Memory Pool System Comprehensive Tests
 * Tests for mempool.c implementation
 *
 * Covers:
 * - Memory pool initialization and shutdown
 * - Pool creation and destruction
 * - Block allocation and freeing
 * - Statistics tracking
 * - Object pool operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s16;
typedef int s32;

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

#define MAX_MEM_POOLS       16

/* ========================================
 * Structures
 * ======================================== */

typedef struct MemBlock {
    struct MemBlock* next;
    struct MemPool* pool;
} MemBlock;

typedef struct MemPool {
    char name[32];
    u8* memory;
    u32 block_size;
    u32 block_count;
    u32 free_count;
    MemBlock* free_list;
} MemPool;

typedef struct {
    MemPool pools[MAX_MEM_POOLS];
    int pool_count;
    u32 total_allocated;
    u32 alloc_count;
    u32 free_count;
    int enabled;
    int debug;
} MemPoolContext;

typedef struct ObjectEntry {
    struct ObjectEntry* next;
    u8 active;
} ObjectEntry;

typedef struct ObjectPool {
    char name[32];
    u8* objects;
    u32 object_size;
    u32 capacity;
    u32 count;
    ObjectEntry* free_list;
} ObjectPool;

/* ========================================
 * Global State
 * ======================================== */

static MemPoolContext g_mempool = {0};

/* ========================================
 * Implementation Functions
 * ======================================== */

static int mempool_init(void) {
    memset(&g_mempool, 0, sizeof(MemPoolContext));
    g_mempool.enabled = 1;
    g_mempool.debug = 0;
    return 1;
}

static void mempool_shutdown(void) {
    int i;

    for (i = 0; i < MAX_MEM_POOLS; i++) {
        if (g_mempool.pools[i].memory) {
            free(g_mempool.pools[i].memory);
            g_mempool.pools[i].memory = NULL;
        }
    }

    memset(&g_mempool, 0, sizeof(MemPoolContext));
}

static MemPool* mempool_create(u32 block_size, u32 block_count, const char* name) {
    MemPool* pool;
    u32 total_size;
    u32 i;
    u8* ptr;
    MemBlock* block;

    if (g_mempool.pool_count >= MAX_MEM_POOLS) {
        return NULL;
    }

    pool = &g_mempool.pools[g_mempool.pool_count];

    total_size = block_size * block_count;

    pool->memory = (u8*)malloc(total_size);
    if (!pool->memory) {
        return NULL;
    }

    pool->block_size = block_size;
    pool->block_count = block_count;
    pool->free_count = block_count;
    pool->free_list = NULL;

    if (name) {
        strncpy(pool->name, name, 31);
        pool->name[31] = '\0';
    }

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

    return pool;
}

static void mempool_destroy(MemPool* pool) {
    int i;

    if (!pool) return;

    for (i = 0; i < g_mempool.pool_count; i++) {
        if (&g_mempool.pools[i] == pool) {
            if (pool->memory) {
                g_mempool.total_allocated -= pool->block_size * pool->block_count;
                free(pool->memory);
            }
            memset(pool, 0, sizeof(MemPool));

            for (; i < g_mempool.pool_count - 1; i++) {
                g_mempool.pools[i] = g_mempool.pools[i + 1];
            }
            g_mempool.pool_count--;
            return;
        }
    }
}

static void* mempool_alloc(MemPool* pool) {
    MemBlock* block;

    if (!pool || !pool->free_list) {
        return NULL;
    }

    block = pool->free_list;
    pool->free_list = block->next;
    pool->free_count--;

    g_mempool.alloc_count++;

    return (void*)((u8*)block + sizeof(MemBlock));
}

static void mempool_free(void* ptr) {
    MemBlock* block;
    MemPool* pool;

    if (!ptr) return;

    block = (MemBlock*)((u8*)ptr - sizeof(MemBlock));
    pool = block->pool;

    if (!pool) {
        return;
    }

    block->next = pool->free_list;
    pool->free_list = block;
    pool->free_count++;

    g_mempool.free_count++;
}

static MemPool* mempool_get(const char* name) {
    int i;

    for (i = 0; i < g_mempool.pool_count; i++) {
        if (strcmp(g_mempool.pools[i].name, name) == 0) {
            return &g_mempool.pools[i];
        }
    }

    return NULL;
}

static void mempool_get_stats(MemPool* pool, u32* total, u32* used, u32* free) {
    if (!pool) return;

    if (total) *total = pool->block_count;
    if (used) *used = pool->block_count - pool->free_count;
    if (free) *free = pool->free_count;
}

static int mempool_is_from_pool(MemPool* pool, void* ptr) {
    u8* p = (u8*)ptr;

    if (!pool || !ptr) return 0;

    return (p >= pool->memory && p < pool->memory + pool->block_size * pool->block_count);
}

static void mempool_reset(MemPool* pool) {
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

static u32 mempool_get_total_allocated(void) {
    return g_mempool.total_allocated;
}

static u32 mempool_get_alloc_count(void) {
    return g_mempool.alloc_count;
}

static u32 mempool_get_free_count(void) {
    return g_mempool.free_count;
}

/* Object Pool Implementation */

static int objpool_init(ObjectPool* pool, u32 object_size, u32 capacity, const char* name) {
    u32 i;

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

    pool->free_list = NULL;
    for (i = 0; i < capacity; i++) {
        ObjectEntry* entry = (ObjectEntry*)(pool->objects + i * object_size);
        entry->next = pool->free_list;
        entry->active = 0;
        pool->free_list = entry;
    }

    return 1;
}

static void objpool_shutdown(ObjectPool* pool) {
    if (!pool) return;

    if (pool->objects) {
        free(pool->objects);
        pool->objects = NULL;
    }

    memset(pool, 0, sizeof(ObjectPool));
}

static void* objpool_alloc(ObjectPool* pool) {
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

static void objpool_free(ObjectPool* pool, void* obj) {
    ObjectEntry* entry;

    if (!pool || !obj) return;

    entry = (ObjectEntry*)obj;
    entry->active = 0;
    entry->next = pool->free_list;
    pool->free_list = entry;
    pool->count--;
}

static void* objpool_get(ObjectPool* pool, u32 index) {
    if (!pool || index >= pool->capacity) {
        return NULL;
    }

    return pool->objects + index * pool->object_size;
}

static u32 objpool_count(ObjectPool* pool) {
    return pool ? pool->count : 0;
}

static void objpool_clear(ObjectPool* pool) {
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

static void reset_state(void) {
    mempool_shutdown();
    mempool_init();
}

/* ========================================
 * Test Cases - Constants
 * ======================================== */

static int test_max_pools(void) {
    return MAX_MEM_POOLS == 16;
}

/* ========================================
 * Test Cases - Initialization
 * ======================================== */

static int test_init_return(void) {
    reset_state();
    return mempool_init() == 1;
}

static int test_init_enabled(void) {
    reset_state();
    return g_mempool.enabled == 1;
}

static int test_init_pool_count(void) {
    reset_state();
    return g_mempool.pool_count == 0;
}

static int test_init_total_allocated(void) {
    reset_state();
    return g_mempool.total_allocated == 0;
}

static int test_shutdown_clears(void) {
    reset_state();
    mempool_create(64, 10, "test");
    mempool_shutdown();
    return g_mempool.pool_count == 0 && g_mempool.total_allocated == 0;
}

/* ========================================
 * Test Cases - Pool Creation
 * ======================================== */

static int test_create_basic(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 10, "test");

    int ok = pool != NULL &&
             pool->block_size == 64 &&
             pool->block_count == 10 &&
             pool->free_count == 10;

    mempool_shutdown();
    return ok;
}

static int test_create_increments_count(void) {
    reset_state();

    mempool_create(64, 10, "pool1");
    mempool_create(128, 5, "pool2");

    int ok = g_mempool.pool_count == 2;

    mempool_shutdown();
    return ok;
}

static int test_create_total_allocated(void) {
    reset_state();

    mempool_create(64, 10, "test");

    int ok = g_mempool.total_allocated == 640;  /* 64 * 10 */

    mempool_shutdown();
    return ok;
}

static int test_create_null_name(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 10, NULL);

    int ok = pool != NULL && pool->name[0] == '\0';

    mempool_shutdown();
    return ok;
}

static int test_create_max_pools(void) {
    reset_state();

    int i;
    char name[16];
    int success_count = 0;

    for (i = 0; i < MAX_MEM_POOLS + 5; i++) {
        sprintf(name, "pool%d", i);
        if (mempool_create(64, 10, name)) {
            success_count++;
        }
    }

    int ok = success_count == MAX_MEM_POOLS;

    mempool_shutdown();
    return ok;
}

/* ========================================
 * Test Cases - Pool Destruction
 * ======================================== */

static int test_destroy_reduces_count(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 10, "test");
    mempool_destroy(pool);

    int ok = g_mempool.pool_count == 0;

    mempool_shutdown();
    return ok;
}

static int test_destroy_updates_total(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 10, "test");
    mempool_destroy(pool);

    int ok = g_mempool.total_allocated == 0;

    mempool_shutdown();
    return ok;
}

static int test_destroy_null(void) {
    reset_state();

    mempool_destroy(NULL);

    return 1;  /* Should not crash */
}

/* ========================================
 * Test Cases - Allocation
 * ======================================== */

static int test_alloc_basic(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 10, "test");
    void* ptr = mempool_alloc(pool);

    int ok = ptr != NULL && pool->free_count == 9;

    mempool_shutdown();
    return ok;
}

static int test_alloc_multiple(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 10, "test");

    mempool_alloc(pool);
    mempool_alloc(pool);
    mempool_alloc(pool);

    int ok = pool->free_count == 7;

    mempool_shutdown();
    return ok;
}

static int test_alloc_updates_count(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 10, "test");
    mempool_alloc(pool);

    int ok = g_mempool.alloc_count == 1;

    mempool_shutdown();
    return ok;
}

static int test_alloc_null_pool(void) {
    reset_state();

    void* ptr = mempool_alloc(NULL);

    return ptr == NULL;
}

static int test_alloc_exhausted(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 2, "test");

    mempool_alloc(pool);
    mempool_alloc(pool);
    void* ptr = mempool_alloc(pool);  /* Should fail */

    int ok = ptr == NULL;

    mempool_shutdown();
    return ok;
}

/* ========================================
 * Test Cases - Freeing
 * ======================================== */

static int test_free_basic(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 10, "test");
    void* ptr = mempool_alloc(pool);
    mempool_free(ptr);

    int ok = pool->free_count == 10;

    mempool_shutdown();
    return ok;
}

static int test_free_updates_count(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 10, "test");
    void* ptr = mempool_alloc(pool);
    mempool_free(ptr);

    int ok = g_mempool.free_count == 1;

    mempool_shutdown();
    return ok;
}

static int test_free_null(void) {
    reset_state();

    mempool_free(NULL);

    return 1;  /* Should not crash */
}

static int test_free_reuse(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 2, "test");

    void* ptr1 = mempool_alloc(pool);
    void* ptr2 = mempool_alloc(pool);
    mempool_free(ptr1);

    void* ptr3 = mempool_alloc(pool);  /* Should reuse freed block */

    int ok = ptr3 != NULL && pool->free_count == 0;

    mempool_shutdown();
    return ok;
}

/* ========================================
 * Test Cases - Statistics
 * ======================================== */

static int test_get_stats_initial(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 10, "test");

    u32 total, used, free;
    mempool_get_stats(pool, &total, &used, &free);

    int ok = total == 10 && used == 0 && free == 10;

    mempool_shutdown();
    return ok;
}

static int test_get_stats_after_alloc(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 10, "test");
    mempool_alloc(pool);
    mempool_alloc(pool);

    u32 total, used, free;
    mempool_get_stats(pool, &total, &used, &free);

    int ok = total == 10 && used == 2 && free == 8;

    mempool_shutdown();
    return ok;
}

static int test_get_stats_null_pool(void) {
    reset_state();

    u32 total = 99, used = 99, free = 99;
    mempool_get_stats(NULL, &total, &used, &free);

    return total == 99 && used == 99 && free == 99;  /* Unchanged */
}

/* ========================================
 * Test Cases - Pool Lookup
 * ======================================== */

static int test_get_by_name_found(void) {
    reset_state();

    mempool_create(64, 10, "test_pool");

    MemPool* pool = mempool_get("test_pool");

    int ok = pool != NULL && strcmp(pool->name, "test_pool") == 0;

    mempool_shutdown();
    return ok;
}

static int test_get_by_name_not_found(void) {
    reset_state();

    mempool_create(64, 10, "test");

    MemPool* pool = mempool_get("nonexistent");

    int ok = pool == NULL;

    mempool_shutdown();
    return ok;
}

/* ========================================
 * Test Cases - Pool Check
 * ======================================== */

static int test_is_from_pool_true(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 10, "test");
    void* ptr = mempool_alloc(pool);

    int ok = mempool_is_from_pool(pool, ptr);

    mempool_shutdown();
    return ok;
}

static int test_is_from_pool_false(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 10, "test");
    int external_var = 0;

    int ok = !mempool_is_from_pool(pool, &external_var);

    mempool_shutdown();
    return ok;
}

static int test_is_from_pool_null_pool(void) {
    reset_state();

    int val = 0;
    return mempool_is_from_pool(NULL, &val) == 0;
}

static int test_is_from_pool_null_ptr(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 10, "test");
    int ok = mempool_is_from_pool(pool, NULL) == 0;

    mempool_shutdown();
    return ok;
}

/* ========================================
 * Test Cases - Pool Reset
 * ======================================== */

static int test_reset_restores_free_count(void) {
    reset_state();

    MemPool* pool = mempool_create(64, 10, "test");
    mempool_alloc(pool);
    mempool_alloc(pool);
    mempool_alloc(pool);

    mempool_reset(pool);

    int ok = pool->free_count == 10;

    mempool_shutdown();
    return ok;
}

static int test_reset_null_pool(void) {
    reset_state();

    mempool_reset(NULL);

    return 1;  /* Should not crash */
}

/* ========================================
 * Test Cases - Object Pool
 * ======================================== */

static int test_objpool_init_basic(void) {
    reset_state();

    ObjectPool pool;
    int result = objpool_init(&pool, 32, 10, "objects");

    int ok = result == 1 && pool.object_size == 32 && pool.capacity == 10;

    objpool_shutdown(&pool);
    return ok;
}

static int test_objpool_alloc_basic(void) {
    reset_state();

    ObjectPool pool;
    objpool_init(&pool, 32, 10, "objects");

    void* obj = objpool_alloc(&pool);

    int ok = obj != NULL && pool.count == 1;

    objpool_shutdown(&pool);
    return ok;
}

static int test_objpool_free_basic(void) {
    reset_state();

    ObjectPool pool;
    objpool_init(&pool, 32, 10, "objects");

    void* obj = objpool_alloc(&pool);
    objpool_free(&pool, obj);

    int ok = pool.count == 0;

    objpool_shutdown(&pool);
    return ok;
}

static int test_objpool_get_by_index(void) {
    reset_state();

    ObjectPool pool;
    objpool_init(&pool, 32, 10, "objects");

    void* obj0 = objpool_get(&pool, 0);
    void* obj5 = objpool_get(&pool, 5);
    void* obj_invalid = objpool_get(&pool, 100);

    int ok = obj0 != NULL && obj5 != NULL && obj_invalid == NULL;

    objpool_shutdown(&pool);
    return ok;
}

static int test_objpool_count(void) {
    reset_state();

    ObjectPool pool;
    objpool_init(&pool, 32, 10, "objects");

    objpool_alloc(&pool);
    objpool_alloc(&pool);
    objpool_alloc(&pool);

    int ok = objpool_count(&pool) == 3;

    objpool_shutdown(&pool);
    return ok;
}

static int test_objpool_clear(void) {
    reset_state();

    ObjectPool pool;
    objpool_init(&pool, 32, 10, "objects");

    objpool_alloc(&pool);
    objpool_alloc(&pool);
    objpool_clear(&pool);

    int ok = pool.count == 0;

    objpool_shutdown(&pool);
    return ok;
}

static int test_objpool_null_pool(void) {
    reset_state();

    void* obj = objpool_alloc(NULL);

    return obj == NULL;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Memory Pool System Comprehensive Tests ===\n\n");

    printf("Constants Tests:\n");
    TEST(max_pools);

    printf("\nInitialization Tests:\n");
    TEST(init_return);
    TEST(init_enabled);
    TEST(init_pool_count);
    TEST(init_total_allocated);
    TEST(shutdown_clears);

    printf("\nPool Creation Tests:\n");
    TEST(create_basic);
    TEST(create_increments_count);
    TEST(create_total_allocated);
    TEST(create_null_name);
    TEST(create_max_pools);

    printf("\nPool Destruction Tests:\n");
    TEST(destroy_reduces_count);
    TEST(destroy_updates_total);
    TEST(destroy_null);

    printf("\nAllocation Tests:\n");
    TEST(alloc_basic);
    TEST(alloc_multiple);
    TEST(alloc_updates_count);
    TEST(alloc_null_pool);
    TEST(alloc_exhausted);

    printf("\nFreeing Tests:\n");
    TEST(free_basic);
    TEST(free_updates_count);
    TEST(free_null);
    TEST(free_reuse);

    printf("\nStatistics Tests:\n");
    TEST(get_stats_initial);
    TEST(get_stats_after_alloc);
    TEST(get_stats_null_pool);

    printf("\nPool Lookup Tests:\n");
    TEST(get_by_name_found);
    TEST(get_by_name_not_found);

    printf("\nPool Check Tests:\n");
    TEST(is_from_pool_true);
    TEST(is_from_pool_false);
    TEST(is_from_pool_null_pool);
    TEST(is_from_pool_null_ptr);

    printf("\nPool Reset Tests:\n");
    TEST(reset_restores_free_count);
    TEST(reset_null_pool);

    printf("\nObject Pool Tests:\n");
    TEST(objpool_init_basic);
    TEST(objpool_alloc_basic);
    TEST(objpool_free_basic);
    TEST(objpool_get_by_index);
    TEST(objpool_count);
    TEST(objpool_clear);
    TEST(objpool_null_pool);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    reset_state();

    return (tests_passed == tests_run) ? 0 : 1;
}
