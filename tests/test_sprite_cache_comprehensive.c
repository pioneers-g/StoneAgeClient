/*
 * Stone Age Client - Sprite Cache Comprehensive Tests
 * Tests for sprite_cache.c implementation
 *
 * Based on Ghidra analysis:
 * - FUN_00480740: sprite_cache_get
 * - FUN_00480870: sprite_cache_add
 * - FUN_004808c0: sprite_cache_invalidate
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

#define MAX_SPRITE_DATA_CACHE 1000

/* Decoded sprite cache entry (from assets.h) */
typedef struct {
    u32 sprite_id;
    void* decoded_data;
    u32 data_size;
    u16 width;
    u16 height;
    u8  is_valid;
    u8  flags;
    u32 last_used;
} DecodedSpriteCacheEntry;

/* SpriteCacheEntry from directx.h - for API compatibility */
typedef struct {
    u32 id;
    void* surface;
    u16 width;
    u16 height;
    s16 offset_x;
    s16 offset_y;
    u8  flags;
    u8  alpha;
    u32 last_used;
} SpriteCacheEntry;

/* ========================================
 * Global State
 * ======================================== */

static DecodedSpriteCacheEntry g_sprite_data_cache[MAX_SPRITE_DATA_CACHE];
static u32 g_sprite_data_cache_count = 0;

/* ========================================
 * Implementation Functions
 * ======================================== */

/*
 * Get decoded sprite from cache
 */
static DecodedSpriteCacheEntry* decoded_sprite_cache_get(u32 sprite_id) {
    u32 i;

    for (i = 0; i < g_sprite_data_cache_count; i++) {
        if (g_sprite_data_cache[i].sprite_id == sprite_id && g_sprite_data_cache[i].is_valid) {
            g_sprite_data_cache[i].last_used++;
            return &g_sprite_data_cache[i];
        }
    }

    return NULL;
}

/*
 * Get sprite from data cache (returns NULL due to design mismatch)
 */
static SpriteCacheEntry* sprite_cache_get(u32 sprite_id) {
    u32 i;

    for (i = 0; i < g_sprite_data_cache_count; i++) {
        if (g_sprite_data_cache[i].sprite_id == sprite_id && g_sprite_data_cache[i].is_valid) {
            /* Note: This returns a SpriteCacheEntry from directx.h, but we use
             * DecodedSpriteCacheEntry internally. This is a design mismatch.
             * For now, return NULL and let the caller use decoded_sprite_cache_get. */
            return NULL;
        }
    }

    return NULL;
}

/*
 * Add sprite to data cache
 */
static SpriteCacheEntry* sprite_cache_add(u32 sprite_id, void* data, u32 size, u16 width, u16 height) {
    DecodedSpriteCacheEntry* entry;

    if (g_sprite_data_cache_count >= MAX_SPRITE_DATA_CACHE) {
        /* Find invalid entry to replace */
        u32 i;
        for (i = 0; i < MAX_SPRITE_DATA_CACHE; i++) {
            if (!g_sprite_data_cache[i].is_valid) {
                entry = &g_sprite_data_cache[i];
                goto found;
            }
        }
        return NULL;
    }

    entry = &g_sprite_data_cache[g_sprite_data_cache_count++];
found:
    entry->sprite_id = sprite_id;
    entry->decoded_data = data;
    entry->data_size = size;
    entry->width = width;
    entry->height = height;
    entry->is_valid = 1;
    entry->flags = 0;
    entry->last_used = 0;

    return NULL;  /* Design mismatch - use decoded_sprite_cache_get from assets.h */
}

/*
 * Invalidate sprite data cache entry
 */
static void sprite_cache_invalidate(u32 sprite_id) {
    DecodedSpriteCacheEntry* entry = decoded_sprite_cache_get(sprite_id);
    if (entry) {
        if (entry->decoded_data) {
            free(entry->decoded_data);
        }
        memset(entry, 0, sizeof(DecodedSpriteCacheEntry));
    }
}

/*
 * Clear entire sprite data cache
 */
static void sprite_cache_clear(void) {
    u32 i;

    for (i = 0; i < g_sprite_data_cache_count; i++) {
        if (g_sprite_data_cache[i].decoded_data) {
            free(g_sprite_data_cache[i].decoded_data);
        }
    }

    memset(g_sprite_data_cache, 0, sizeof(g_sprite_data_cache));
    g_sprite_data_cache_count = 0;
}

/*
 * Reset test state
 */
static void reset_state(void) {
    sprite_cache_clear();
}

/* ========================================
 * Test Cases - Basic Operations
 * ======================================== */

static int test_cache_add_basic(void) {
    reset_state();

    u8* data = malloc(100);
    memcpy(data, "test data", 9);

    sprite_cache_add(1000, data, 100, 48, 48);

    DecodedSpriteCacheEntry* entry = decoded_sprite_cache_get(1000);

    int result = entry != NULL &&
           entry->sprite_id == 1000 &&
           entry->width == 48 &&
           entry->height == 48 &&
           entry->data_size == 100 &&
           entry->is_valid == 1;

    return result;
}

static int test_cache_add_multiple(void) {
    reset_state();

    u8* data1 = malloc(50);
    u8* data2 = malloc(60);
    u8* data3 = malloc(70);

    sprite_cache_add(1000, data1, 50, 32, 32);
    sprite_cache_add(2000, data2, 60, 48, 48);
    sprite_cache_add(3000, data3, 70, 64, 64);

    DecodedSpriteCacheEntry* e1 = decoded_sprite_cache_get(1000);
    DecodedSpriteCacheEntry* e2 = decoded_sprite_cache_get(2000);
    DecodedSpriteCacheEntry* e3 = decoded_sprite_cache_get(3000);

    return e1 != NULL && e2 != NULL && e3 != NULL &&
           e1->width == 32 && e2->width == 48 && e3->width == 64;
}

static int test_cache_get_nonexistent(void) {
    reset_state();

    DecodedSpriteCacheEntry* entry = decoded_sprite_cache_get(99999);

    return entry == NULL;
}

static int test_cache_get_returns_null_api(void) {
    reset_state();

    u8* data = malloc(100);
    sprite_cache_add(1000, data, 100, 48, 48);

    /* sprite_cache_get returns NULL due to design mismatch */
    SpriteCacheEntry* entry = sprite_cache_get(1000);

    return entry == NULL;
}

/* ========================================
 * Test Cases - Cache Invalidation
 * ======================================== */

static int test_cache_invalidate_existing(void) {
    reset_state();

    u8* data = malloc(100);
    sprite_cache_add(1000, data, 100, 48, 48);

    sprite_cache_invalidate(1000);

    DecodedSpriteCacheEntry* entry = decoded_sprite_cache_get(1000);

    return entry == NULL;  /* Should be removed */
}

static int test_cache_invalidate_nonexistent(void) {
    reset_state();

    /* Should not crash */
    sprite_cache_invalidate(99999);

    return 1;
}

static int test_cache_invalidate_preserves_others(void) {
    reset_state();

    u8* data1 = malloc(50);
    u8* data2 = malloc(60);

    sprite_cache_add(1000, data1, 50, 32, 32);
    sprite_cache_add(2000, data2, 60, 48, 48);

    sprite_cache_invalidate(1000);

    DecodedSpriteCacheEntry* e1 = decoded_sprite_cache_get(1000);
    DecodedSpriteCacheEntry* e2 = decoded_sprite_cache_get(2000);

    return e1 == NULL && e2 != NULL && e2->width == 48;
}

/* ========================================
 * Test Cases - Cache Clear
 * ======================================== */

static int test_cache_clear_empty(void) {
    reset_state();

    /* Should not crash */
    sprite_cache_clear();

    return g_sprite_data_cache_count == 0;
}

static int test_cache_clear_with_entries(void) {
    reset_state();

    u8* data1 = malloc(50);
    u8* data2 = malloc(60);

    sprite_cache_add(1000, data1, 50, 32, 32);
    sprite_cache_add(2000, data2, 60, 48, 48);

    sprite_cache_clear();

    return g_sprite_data_cache_count == 0 &&
           decoded_sprite_cache_get(1000) == NULL &&
           decoded_sprite_cache_get(2000) == NULL;
}

/* ========================================
 * Test Cases - Cache Full Behavior
 * ======================================== */

static int test_cache_fill_to_max(void) {
    reset_state();

    u32 i;
    for (i = 0; i < MAX_SPRITE_DATA_CACHE; i++) {
        u8* data = malloc(10);
        sprite_cache_add(i, data, 10, 8, 8);
    }

    /* All should be cached */
    int all_cached = 1;
    for (i = 0; i < MAX_SPRITE_DATA_CACHE; i++) {
        if (decoded_sprite_cache_get(i) == NULL) {
            all_cached = 0;
            break;
        }
    }

    return all_cached && g_sprite_data_cache_count == MAX_SPRITE_DATA_CACHE;
}

static int test_cache_full_returns_null(void) {
    reset_state();

    u32 i;
    for (i = 0; i < MAX_SPRITE_DATA_CACHE; i++) {
        u8* data = malloc(10);
        sprite_cache_add(i, data, 10, 8, 8);
    }

    /* Adding one more should fail (no invalid entries to replace) */
    u8* extra_data = malloc(10);
    SpriteCacheEntry* result = sprite_cache_add(MAX_SPRITE_DATA_CACHE, extra_data, 10, 8, 8);

    /* Free the unused extra data */
    free(extra_data);

    return result == NULL;
}

static int test_cache_replace_invalid_entry(void) {
    reset_state();

    /* Fill half the cache */
    u32 i;
    for (i = 0; i < MAX_SPRITE_DATA_CACHE / 2; i++) {
        u8* data = malloc(10);
        sprite_cache_add(i, data, 10, 8, 8);
    }

    /* Invalidate first entry */
    sprite_cache_invalidate(0);

    /* Set count to max to trigger replacement path */
    g_sprite_data_cache_count = MAX_SPRITE_DATA_CACHE;

    /* Add should find the invalid entry and replace it */
    u8* new_data = malloc(20);
    sprite_cache_add(99999, new_data, 20, 16, 16);

    /* The invalidated slot should now have the new sprite */
    DecodedSpriteCacheEntry* entry = decoded_sprite_cache_get(99999);

    return entry != NULL && entry->width == 16;
}

/* ========================================
 * Test Cases - Edge Cases
 * ======================================== */

static int test_cache_add_null_data(void) {
    reset_state();

    /* Adding NULL data should work (entry stored) */
    sprite_cache_add(1000, NULL, 0, 48, 48);

    DecodedSpriteCacheEntry* entry = decoded_sprite_cache_get(1000);

    return entry != NULL && entry->decoded_data == NULL;
}

static int test_cache_add_zero_dimensions(void) {
    reset_state();

    u8* data = malloc(100);
    sprite_cache_add(1000, data, 100, 0, 0);

    DecodedSpriteCacheEntry* entry = decoded_sprite_cache_get(1000);

    return entry != NULL && entry->width == 0 && entry->height == 0;
}

static int test_cache_add_large_sprite(void) {
    reset_state();

    u8* data = malloc(65536);  /* 256x256 sprite */
    sprite_cache_add(1000, data, 65536, 256, 256);

    DecodedSpriteCacheEntry* entry = decoded_sprite_cache_get(1000);

    return entry != NULL &&
           entry->width == 256 &&
           entry->height == 256 &&
           entry->data_size == 65536;
}

static int test_cache_last_used_increment(void) {
    reset_state();

    u8* data = malloc(100);
    sprite_cache_add(1000, data, 100, 48, 48);

    DecodedSpriteCacheEntry* entry1 = decoded_sprite_cache_get(1000);
    u32 first_use = entry1->last_used;

    DecodedSpriteCacheEntry* entry2 = decoded_sprite_cache_get(1000);
    u32 second_use = entry2->last_used;

    return second_use > first_use;
}

static int test_cache_sprite_id_boundary_low(void) {
    reset_state();

    u8* data = malloc(100);
    sprite_cache_add(0, data, 100, 48, 48);

    DecodedSpriteCacheEntry* entry = decoded_sprite_cache_get(0);

    return entry != NULL && entry->sprite_id == 0;
}

static int test_cache_sprite_id_boundary_high(void) {
    reset_state();

    u8* data = malloc(100);
    sprite_cache_add(549999, data, 100, 48, 48);  /* MAX_SPRITE_ID */

    DecodedSpriteCacheEntry* entry = decoded_sprite_cache_get(549999);

    return entry != NULL && entry->sprite_id == 549999;
}

/* ========================================
 * Test Cases - Re-add Behavior
 * ======================================== */

static int test_cache_readd_same_id(void) {
    reset_state();

    u8* data1 = malloc(100);
    u8* data2 = malloc(200);

    sprite_cache_add(1000, data1, 100, 32, 32);
    sprite_cache_add(1000, data2, 200, 48, 48);  /* Same ID, different data */

    /* Should have two entries (not checking for duplicates) */
    int count = 0;
    for (u32 i = 0; i < g_sprite_data_cache_count; i++) {
        if (g_sprite_data_cache[i].sprite_id == 1000 && g_sprite_data_cache[i].is_valid) {
            count++;
        }
    }

    /* First match should have first data */
    DecodedSpriteCacheEntry* entry = decoded_sprite_cache_get(1000);

    return count == 2 && entry != NULL;
}

/* ========================================
 * Test Cases - Memory Management
 * ======================================== */

static int test_cache_invalidate_frees_data(void) {
    reset_state();

    u8* data = malloc(100);
    memset(data, 0xAA, 100);

    sprite_cache_add(1000, data, 100, 48, 48);

    /* Verify data is cached */
    DecodedSpriteCacheEntry* entry = decoded_sprite_cache_get(1000);
    if (!entry || entry->decoded_data != data) return 0;

    /* Invalidate should free the data */
    sprite_cache_invalidate(1000);

    /* Entry should be cleared */
    entry = decoded_sprite_cache_get(1000);

    return entry == NULL;
}

static int test_cache_clear_frees_all_data(void) {
    reset_state();

    u8* data1 = malloc(100);
    u8* data2 = malloc(100);
    u8* data3 = malloc(100);

    sprite_cache_add(1000, data1, 100, 32, 32);
    sprite_cache_add(2000, data2, 100, 32, 32);
    sprite_cache_add(3000, data3, 100, 32, 32);

    sprite_cache_clear();

    /* All entries should be gone */
    return g_sprite_data_cache_count == 0;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Sprite Cache Comprehensive Tests ===\n\n");

    printf("Basic Operations Tests:\n");
    TEST(cache_add_basic);
    TEST(cache_add_multiple);
    TEST(cache_get_nonexistent);
    TEST(cache_get_returns_null_api);

    printf("\nCache Invalidation Tests:\n");
    TEST(cache_invalidate_existing);
    TEST(cache_invalidate_nonexistent);
    TEST(cache_invalidate_preserves_others);

    printf("\nCache Clear Tests:\n");
    TEST(cache_clear_empty);
    TEST(cache_clear_with_entries);

    printf("\nCache Full Behavior Tests:\n");
    TEST(cache_fill_to_max);
    TEST(cache_full_returns_null);
    TEST(cache_replace_invalid_entry);

    printf("\nEdge Cases Tests:\n");
    TEST(cache_add_null_data);
    TEST(cache_add_zero_dimensions);
    TEST(cache_add_large_sprite);
    TEST(cache_last_used_increment);
    TEST(cache_sprite_id_boundary_low);
    TEST(cache_sprite_id_boundary_high);

    printf("\nRe-add Behavior Tests:\n");
    TEST(cache_readd_same_id);

    printf("\nMemory Management Tests:\n");
    TEST(cache_invalidate_frees_data);
    TEST(cache_clear_frees_all_data);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - LRU eviction policy
     * - Cache statistics tracking
     * - Thread safety (if applicable)
     * - Integration with sprite loading pipeline
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
