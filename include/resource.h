/*
 * Stone Age Client - Resource Cache Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include "types.h"

/* Constants */
#define MAX_CACHED_SPRITES   500
#define MAX_CACHED_TEXTURES  200
#define MAX_CACHED_SOUNDS    100
#define MAX_SPRITE_MEMORY    (32 * 1024 * 1024)  /* 32MB */

/* Cached sprite */
typedef struct {
    u16 sprite_id;
    char path[256];
    u8* data;
    u32 data_size;
    u32 last_access;
    u32 access_count;
    int valid;

} CachedSprite;

/* Cached texture */
typedef struct {
    u16 texture_id;
    char path[256];
    void* surface;
    u32 last_access;
    int valid;

} CachedTexture;

/* Cached sound */
typedef struct {
    u16 sound_id;
    char path[256];
    void* buffer;
    u32 last_access;
    int valid;

} CachedSound;

/* Resource statistics */
typedef struct {
    int sprite_count;
    int texture_count;
    int sound_count;
    u32 sprite_memory;
    u32 cache_hits;
    u32 cache_misses;
    int hit_rate;

} ResourceStats;

/* Resource context */
typedef struct {
    /* Sprite cache */
    CachedSprite sprites[MAX_CACHED_SPRITES];
    int sprite_count;
    int max_sprites;
    u32 total_sprite_memory;

    /* Texture cache */
    CachedTexture textures[MAX_CACHED_TEXTURES];
    int texture_count;
    int max_textures;

    /* Sound cache */
    CachedSound sounds[MAX_CACHED_SOUNDS];
    int sound_count;
    int max_sounds;

    /* Statistics */
    u32 cache_hits;
    u32 cache_misses;

    /* Synchronization */
    CRITICAL_SECTION lock;

} ResourceContext;

/* Global resource context */
extern ResourceContext g_resource;

/* Initialization */
int resource_init(void);
void resource_shutdown(void);

/* Sprites */
CachedSprite* resource_load_sprite(u16 sprite_id);
CachedSprite* resource_get_sprite(u16 sprite_id);
void resource_unload_sprite(u16 sprite_id);
void resource_evict_sprite(void);

/* Textures */
CachedTexture* resource_load_texture(u16 texture_id);
CachedTexture* resource_get_texture(u16 texture_id);
void resource_unload_texture(u16 texture_id);
void resource_evict_texture(void);

/* Sounds */
CachedSound* resource_load_sound(u16 sound_id);
CachedSound* resource_get_sound(u16 sound_id);

/* Preloading */
int resource_preload_sprites(u16* sprite_ids, int count);

/* Clear */
void resource_clear_all(void);

/* Settings */
void resource_set_max_sprites(int max);
void resource_set_max_textures(int max);

/* Statistics */
u32 resource_get_sprite_memory(void);
int resource_get_sprite_count(void);
int resource_get_texture_count(void);
void resource_get_stats(ResourceStats* stats);
void resource_reset_stats(void);

/* Update */
void resource_update(void);

#endif /* RESOURCE_H */
