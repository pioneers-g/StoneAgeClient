/*
 * Stone Age Client - Resource Cache Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "resource.h"
#include "logger.h"

/* Global resource context */
ResourceContext g_resource = {0};

/*
 * Initialize resource system
 */
int resource_init(void) {
    memset(&g_resource, 0, sizeof(ResourceContext));

    g_resource.max_sprites = MAX_CACHED_SPRITES;
    g_resource.max_textures = MAX_CACHED_TEXTURES;
    g_resource.max_sounds = MAX_CACHED_SOUNDS;

    InitializeCriticalSection(&g_resource.lock);

    LOG_INFO("Resource system initialized");
    return 1;
}

/*
 * Shutdown resource system
 */
void resource_shutdown(void) {
    resource_clear_all();

    DeleteCriticalSection(&g_resource.lock);
    memset(&g_resource, 0, sizeof(ResourceContext));
    LOG_INFO("Resource system shutdown");
}

/*
 * Load sprite
 */
CachedSprite* resource_load_sprite(u16 sprite_id) {
    CachedSprite* sprite;
    char path[256];
    FILE* fp;
    int i;

    /* Check cache first */
    for (i = 0; i < g_resource.sprite_count; i++) {
        if (g_resource.sprites[i].sprite_id == sprite_id && g_resource.sprites[i].valid) {
            g_resource.sprites[i].last_access = timeGetTime();
            g_resource.sprites[i].access_count++;
            return &g_resource.sprites[i];
        }
    }

    /* Check if cache full */
    if (g_resource.sprite_count >= g_resource.max_sprites) {
        resource_evict_sprite();
    }

    /* Load new sprite */
    sprite = &g_resource.sprites[g_resource.sprite_count];
    sprite->sprite_id = sprite_id;
    sprite->last_access = timeGetTime();
    sprite->access_count = 1;
    sprite->valid = 1;

    /* Build path */
    snprintf(path, sizeof(path), "data\\spr\\%05d.spr", sprite_id);
    strncpy(sprite->path, path, sizeof(sprite->path) - 1);

    /* Load sprite file */
    fp = fopen(path, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        sprite->data_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        sprite->data = (u8*)malloc(sprite->data_size);
        if (sprite->data) {
            fread(sprite->data, 1, sprite->data_size, fp);
            g_resource.total_sprite_memory += sprite->data_size;
        }

        fclose(fp);
    } else {
        sprite->valid = 0;
        LOG_WARN("Failed to load sprite: %s", path);
        return NULL;
    }

    g_resource.sprite_count++;
    g_resource.cache_misses++;

    LOG_DEBUG("Loaded sprite: %s (%u bytes)", path, sprite->data_size);
    return sprite;
}

/*
 * Get cached sprite
 */
CachedSprite* resource_get_sprite(u16 sprite_id) {
    int i;

    for (i = 0; i < g_resource.sprite_count; i++) {
        if (g_resource.sprites[i].sprite_id == sprite_id && g_resource.sprites[i].valid) {
            return &g_resource.sprites[i];
        }
    }

    return NULL;
}

/*
 * Unload sprite
 */
void resource_unload_sprite(u16 sprite_id) {
    int i;

    for (i = 0; i < g_resource.sprite_count; i++) {
        if (g_resource.sprites[i].sprite_id == sprite_id) {
            if (g_resource.sprites[i].data) {
                free(g_resource.sprites[i].data);
                g_resource.total_sprite_memory -= g_resource.sprites[i].data_size;
            }
            memset(&g_resource.sprites[i], 0, sizeof(CachedSprite));

            /* Compact array */
            if (i < g_resource.sprite_count - 1) {
                g_resource.sprites[i] = g_resource.sprites[g_resource.sprite_count - 1];
                memset(&g_resource.sprites[g_resource.sprite_count - 1], 0, sizeof(CachedSprite));
            }
            g_resource.sprite_count--;

            return;
        }
    }
}

/*
 * Evict least recently used sprite
 */
void resource_evict_sprite(void) {
    int i;
    int lru_index = -1;
    u32 oldest_time = 0xFFFFFFFF;

    for (i = 0; i < g_resource.sprite_count; i++) {
        if (g_resource.sprites[i].valid && g_resource.sprites[i].last_access < oldest_time) {
            oldest_time = g_resource.sprites[i].last_access;
            lru_index = i;
        }
    }

    if (lru_index >= 0) {
        LOG_DEBUG("Evicting sprite %d (LRU)", g_resource.sprites[lru_index].sprite_id);
        resource_unload_sprite(g_resource.sprites[lru_index].sprite_id);
    }
}

/*
 * Load texture
 */
CachedTexture* resource_load_texture(u16 texture_id) {
    CachedTexture* texture;
    int i;

    /* Check cache */
    for (i = 0; i < g_resource.texture_count; i++) {
        if (g_resource.textures[i].texture_id == texture_id && g_resource.textures[i].valid) {
            g_resource.textures[i].last_access = timeGetTime();
            return &g_resource.textures[i];
        }
    }

    /* Check capacity */
    if (g_resource.texture_count >= g_resource.max_textures) {
        resource_evict_texture();
    }

    /* Load new texture */
    texture = &g_resource.textures[g_resource.texture_count];
    texture->texture_id = texture_id;
    texture->last_access = timeGetTime();
    texture->valid = 1;

    /* Would load actual texture data here */
    snprintf(texture->path, sizeof(texture->path), "data\\tex\\%05d.tex", texture_id);

    g_resource.texture_count++;
    return texture;
}

/*
 * Get cached texture
 */
CachedTexture* resource_get_texture(u16 texture_id) {
    int i;

    for (i = 0; i < g_resource.texture_count; i++) {
        if (g_resource.textures[i].texture_id == texture_id && g_resource.textures[i].valid) {
            return &g_resource.textures[i];
        }
    }

    return NULL;
}

/*
 * Unload texture
 */
void resource_unload_texture(u16 texture_id) {
    int i;

    for (i = 0; i < g_resource.texture_count; i++) {
        if (g_resource.textures[i].texture_id == texture_id) {
            memset(&g_resource.textures[i], 0, sizeof(CachedTexture));

            if (i < g_resource.texture_count - 1) {
                g_resource.textures[i] = g_resource.textures[g_resource.texture_count - 1];
            }
            g_resource.texture_count--;
            return;
        }
    }
}

/*
 * Evict least recently used texture
 */
void resource_evict_texture(void) {
    int i;
    int lru_index = -1;
    u32 oldest_time = 0xFFFFFFFF;

    for (i = 0; i < g_resource.texture_count; i++) {
        if (g_resource.textures[i].valid && g_resource.textures[i].last_access < oldest_time) {
            oldest_time = g_resource.textures[i].last_access;
            lru_index = i;
        }
    }

    if (lru_index >= 0) {
        resource_unload_texture(g_resource.textures[lru_index].texture_id);
    }
}

/*
 * Preload sprites
 */
int resource_preload_sprites(u16* sprite_ids, int count) {
    int i;
    int loaded = 0;

    for (i = 0; i < count; i++) {
        if (resource_load_sprite(sprite_ids[i])) {
            loaded++;
        }
    }

    LOG_INFO("Preloaded %d/%d sprites", loaded, count);
    return loaded;
}

/*
 * Clear all cached resources
 */
void resource_clear_all(void) {
    int i;

    /* Clear sprites */
    for (i = 0; i < g_resource.sprite_count; i++) {
        if (g_resource.sprites[i].data) {
            free(g_resource.sprites[i].data);
        }
    }
    g_resource.sprite_count = 0;
    g_resource.total_sprite_memory = 0;

    /* Clear textures */
    g_resource.texture_count = 0;

    /* Clear sounds */
    g_resource.sound_count = 0;
}

/*
 * Get sprite memory usage
 */
u32 resource_get_sprite_memory(void) {
    return g_resource.total_sprite_memory;
}

/*
 * Get sprite count
 */
int resource_get_sprite_count(void) {
    return g_resource.sprite_count;
}

/*
 * Get texture count
 */
int resource_get_texture_count(void) {
    return g_resource.texture_count;
}

/*
 * Set max sprites
 */
void resource_set_max_sprites(int max) {
    g_resource.max_sprites = max;
}

/*
 * Set max textures
 */
void resource_set_max_textures(int max) {
    g_resource.max_textures = max;
}

/*
 * Get cache statistics
 */
void resource_get_stats(ResourceStats* stats) {
    if (!stats) return;

    stats->sprite_count = g_resource.sprite_count;
    stats->texture_count = g_resource.texture_count;
    stats->sound_count = g_resource.sound_count;
    stats->sprite_memory = g_resource.total_sprite_memory;
    stats->cache_hits = g_resource.cache_hits;
    stats->cache_misses = g_resource.cache_misses;

    if (stats->cache_hits + stats->cache_misses > 0) {
        stats->hit_rate = (stats->cache_hits * 100) / (stats->cache_hits + stats->cache_misses);
    } else {
        stats->hit_rate = 0;
    }
}

/*
 * Reset cache statistics
 */
void resource_reset_stats(void) {
    g_resource.cache_hits = 0;
    g_resource.cache_misses = 0;
}

/*
 * Load sound
 */
CachedSound* resource_load_sound(u16 sound_id) {
    CachedSound* sound;
    int i;

    /* Check cache */
    for (i = 0; i < g_resource.sound_count; i++) {
        if (g_resource.sounds[i].sound_id == sound_id && g_resource.sounds[i].valid) {
            return &g_resource.sounds[i];
        }
    }

    if (g_resource.sound_count >= g_resource.max_sounds) {
        return NULL;
    }

    sound = &g_resource.sounds[g_resource.sound_count];
    sound->sound_id = sound_id;
    sound->valid = 1;

    snprintf(sound->path, sizeof(sound->path), "data\\se\\%05d.wav", sound_id);

    g_resource.sound_count++;
    return sound;
}

/*
 * Get cached sound
 */
CachedSound* resource_get_sound(u16 sound_id) {
    int i;

    for (i = 0; i < g_resource.sound_count; i++) {
        if (g_resource.sounds[i].sound_id == sound_id && g_resource.sounds[i].valid) {
            return &g_resource.sounds[i];
        }
    }

    return NULL;
}

/*
 * Update resource system
 */
void resource_update(void) {
    /* Check memory limits and evict if necessary */
    if (g_resource.total_sprite_memory > MAX_SPRITE_MEMORY) {
        resource_evict_sprite();
    }
}
