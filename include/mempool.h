/*
 * Stone Age Client - Memory Pool System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef MEMPOOL_H
#define MEMPOOL_H

#include "types.h"

/* Constants */
#define MAX_MEM_POOLS       16

/* Memory block header */
typedef struct MemBlock {
    struct MemBlock* next;
    struct MemPool* pool;
} MemBlock;

/* Memory pool */
typedef struct MemPool {
    char name[32];
    u8* memory;
    u32 block_size;
    u32 block_count;
    u32 free_count;
    MemBlock* free_list;

} MemPool;

/* Memory pool context */
typedef struct {
    MemPool pools[MAX_MEM_POOLS];
    int pool_count;
    u32 total_allocated;
    u32 alloc_count;
    u32 free_count;
    int enabled;
    int debug;

} MemPoolContext;

/* Object entry for object pool */
typedef struct ObjectEntry {
    struct ObjectEntry* next;
    u8 active;
} ObjectEntry;

/* Object pool */
typedef struct ObjectPool {
    char name[32];
    u8* objects;
    u32 object_size;
    u32 capacity;
    u32 count;
    ObjectEntry* free_list;

} ObjectPool;

/* Global memory pool context */
extern MemPoolContext g_mempool;

/* Initialization */
int mempool_init(void);
void mempool_shutdown(void);

/* Pool management */
MemPool* mempool_create(u32 block_size, u32 block_count, const char* name);
void mempool_destroy(MemPool* pool);
MemPool* mempool_get(const char* name);

/* Allocation */
void* mempool_alloc(MemPool* pool);
void mempool_free(void* ptr);

/* Statistics */
void mempool_get_stats(MemPool* pool, u32* total, u32* used, u32* free);
int mempool_is_from_pool(MemPool* pool, void* ptr);
u32 mempool_get_total_allocated(void);
u32 mempool_get_alloc_count(void);
u32 mempool_get_free_count(void);

/* Pool operations */
void mempool_compact(MemPool* pool);
void mempool_reset(MemPool* pool);
void mempool_set_debug(int enabled);
void mempool_print_stats(void);

/* Object Pool */
int objpool_init(ObjectPool* pool, u32 object_size, u32 capacity, const char* name);
void objpool_shutdown(ObjectPool* pool);
void* objpool_alloc(ObjectPool* pool);
void objpool_free(ObjectPool* pool, void* obj);
void* objpool_get(ObjectPool* pool, u32 index);
void* objpool_first(ObjectPool* pool, u32* index);
void* objpool_next(ObjectPool* pool, u32* index);
u32 objpool_count(ObjectPool* pool);
void objpool_clear(ObjectPool* pool);

#endif /* MEMPOOL_H */
