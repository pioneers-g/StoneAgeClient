/*
 * Stone Age Client - Sprite Data Cache
 * Split from sprite.c for code organization
 *
 * Handles sprite data caching (not surface cache, which is in directx.c)
 */

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "sprite.h"
#include "assets.h"  /* For DecodedSpriteCacheEntry */

#define MAX_SPRITE_DATA_CACHE 1000

/* Global sprite data cache */
static DecodedSpriteCacheEntry g_sprite_data_cache[MAX_SPRITE_DATA_CACHE];
static u32 g_sprite_data_cache_count = 0;

/*
 * Get sprite from data cache
 */
SpriteCacheEntry* sprite_cache_get(u32 sprite_id) {
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
SpriteCacheEntry* sprite_cache_add(u32 sprite_id, void* data, u32 size, u16 width, u16 height) {
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

    return NULL;  /* Design mismatch - use decoded_sprite_cache_get from assets.h */
}

/*
 * Invalidate sprite data cache entry
 */
void sprite_cache_invalidate(u32 sprite_id) {
    DecodedSpriteCacheEntry* entry = decoded_sprite_cache_get(sprite_id);
    if (entry) {
        if (entry->decoded_data) {
            free(entry->decoded_data);
        }
        memset(entry, 0, sizeof(DecodedSpriteCacheEntry));
    }
}

/*
 * Get sprite cache entry (alias for compatibility)
 */
SpriteCacheEntry* sprite_cache_get_entry(u32 sprite_id) {
    /* Design mismatch - use decoded_sprite_cache_get from assets.h */
    (void)sprite_id;
    return NULL;
}

/*
 * Clear entire sprite data cache
 */
void sprite_cache_clear(void) {
    u32 i;

    for (i = 0; i < g_sprite_data_cache_count; i++) {
        if (g_sprite_data_cache[i].decoded_data) {
            free(g_sprite_data_cache[i].decoded_data);
        }
    }

    memset(g_sprite_data_cache, 0, sizeof(g_sprite_data_cache));
    g_sprite_data_cache_count = 0;
}
