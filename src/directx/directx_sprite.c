/*
 * Stone Age Client - DirectX Sprite Cache
 * Sprite surface caching, pixel conversion, tile management
 * Split from directx.c - FUN_00480740, FUN_00480130, FUN_00412d60, FUN_00412b80
 */

#include <windows.h>
#include <ddraw.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "directx.h"
#include "directx_internal.h"
#include "assets.h"
#include "logger.h"

/* Sprite cache - matches DAT_0464f7c0 region */
static SpriteCacheEntry g_sprite_cache[SPRITE_CACHE_SIZE];
static int g_sprite_cache_count = 0;      /* DAT_0466b7cc */
static int g_sprite_surface_count = 0;    /* DAT_04cb6d20 */
static int g_sprite_alpha_count = 0;      /* DAT_0466b7d8 */

/* Sprite dimensions - DAT_0466b7d4, DAT_0466b7d0 */
static int g_cache_sprite_width = 0;
static int g_cache_sprite_height = 0;

/* Global sprite data buffer - DAT_0464f4fc */
static uint8_t* g_sprite_data_buffer = NULL;
static int g_sprite_data_pitch = 0;       /* DAT_0464f6c0 */
static int g_sprite_data_height = 0;      /* DAT_0464f6bc */

/* Current cache state - matches DAT_0466b7xx region */
int g_cache_current_index = 0;            /* DAT_0466b7c8 */

/* Alpha mode flag - DAT_0054c83c */
int g_alpha_mode = 0;

/*
 * Set alpha mode - DAT_0054c83c
 */
void graphics_set_alpha_mode(int enabled) {
    g_alpha_mode = enabled;
}

int graphics_get_alpha_mode(void) {
    return g_alpha_mode;
}

/*
 * Set sprite data buffer for cache operations
 */
void sprite_set_data_buffer(void* buffer, int pitch, int height) {
    g_sprite_data_buffer = (uint8_t*)buffer;
    g_sprite_data_pitch = pitch;
    g_sprite_data_height = height;
}

/*
 * Create sprite cache surfaces - FUN_00480740
 * Creates array of surfaces for sprite rendering
 */
int sprite_cache_init(int width, int height, int count, int alpha_mode) {
    int i;
    int created = 0;

    if (count > SPRITE_CACHE_SIZE) {
        count = SPRITE_CACHE_SIZE;
    }

    g_cache_sprite_width = width;
    g_cache_sprite_height = height;
    g_sprite_cache_count = 0;
    g_sprite_surface_count = 0;
    g_sprite_alpha_count = 0;
    g_cache_current_index = 0;

    memset(g_sprite_cache, 0, sizeof(g_sprite_cache));

    for (i = 0; i < count; i++) {
        SpriteCacheEntry* entry = &g_sprite_cache[i];

        entry->sprite_id = -1;  /* -1 indicates empty slot */
        entry->sprite_data = NULL;
        entry->x_offset = 0;
        entry->y_offset = 0;
        entry->ref_count = 0;

        /* Try video memory first (0x4000 flag) */
        entry->surface = graphics_create_offscreen(width, height, SURFACE_FLAG_VIDEO_MEMORY);
        if (!entry->surface) {
            /* Fallback to system memory (0x800 flag) */
            entry->surface = graphics_create_offscreen(width, height, SURFACE_FLAG_SYSTEM_MEMORY);
            if (!entry->surface) {
                LOG_WARN("Failed to create sprite surface %d", i);
                continue;
            }
        }

        entry->flags = entry->surface ? SURFACE_FLAG_VIDEO_MEMORY : SURFACE_FLAG_SYSTEM_MEMORY;
        g_sprite_surface_count++;

        /* Create alpha buffer if alpha mode enabled - DAT_0054c83c */
        if (alpha_mode) {
            entry->alpha_buffer = malloc(width * height);
            if (entry->alpha_buffer) {
                memset(entry->alpha_buffer, 0, width * height);
            }

            entry->alpha_surface = graphics_create_offscreen(width, height, SURFACE_FLAG_SYSTEM_MEMORY);
            if (entry->alpha_surface) {
                g_sprite_alpha_count++;
            }
        }

        created++;
    }

    g_sprite_cache_count = created;
    g_alpha_mode = alpha_mode;

    LOG_INFO("Sprite cache initialized: %d surfaces, %d alpha surfaces",
             g_sprite_surface_count, g_sprite_alpha_count);

    return created;
}

/*
 * Shutdown sprite cache
 */
void sprite_cache_shutdown(void) {
    int i;

    for (i = 0; i < SPRITE_CACHE_SIZE; i++) {
        SpriteCacheEntry* entry = &g_sprite_cache[i];

        if (entry->surface) {
            IDirectDrawSurface_Release(entry->surface);
            entry->surface = NULL;
        }

        if (entry->alpha_buffer) {
            free(entry->alpha_buffer);
            entry->alpha_buffer = NULL;
        }

        if (entry->alpha_surface) {
            IDirectDrawSurface_Release(entry->alpha_surface);
            entry->alpha_surface = NULL;
        }

        entry->sprite_id = -1;
        entry->sprite_data = NULL;
        entry->x_offset = 0;
        entry->y_offset = 0;
        entry->ref_count = 0;
        entry->flags = 0;
    }

    memset(g_sprite_cache, 0, sizeof(g_sprite_cache));
    g_sprite_cache_count = 0;
    g_sprite_surface_count = 0;
    g_sprite_alpha_count = 0;
    g_cache_current_index = 0;
}

/*
 * Get sprite cache entry
 */
SpriteCacheEntry* sprite_cache_get_entry(int index) {
    if (index >= 0 && index < SPRITE_CACHE_SIZE) {
        return &g_sprite_cache[index];
    }
    return NULL;
}

/*
 * Get sprite cache count
 */
int sprite_cache_get_count(void) {
    return g_sprite_cache_count;
}

/*
 * Copy sprite data to surface - FUN_00412d60
 * Handles RGB555/RGB565 conversion and color key transparency
 */
int sprite_copy_to_surface(IDirectDrawSurface* surface, int src_x, void* src_data,
                           uint32_t width, int height) {
    DDSURFACEDESC2 ddsd;
    HRESULT hr;
    uint8_t* dst_pixels;
    uint8_t* src_pixels;
    int dst_pitch;
    int src_pitch;
    int y;
    int is_555_mode;

    if (!surface || !src_data) return 0;

    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);

    hr = IDirectDrawSurface_Lock(surface, NULL, &ddsd,
                                  DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
    if (FAILED(hr)) {
        LOG_WARN("sprite_copy_to_surface: Lock failed 0x%08X", hr);
        return 0;
    }

    dst_pixels = (uint8_t*)ddsd.lpSurface;
    dst_pitch = ddsd.lPitch;
    src_pixels = (uint8_t*)src_data + src_x * 2;  /* 16-bit pixels */
    src_pitch = g_sprite_data_pitch;
    is_555_mode = (g_graphics.bpp == 15);  /* 15bpp = RGB555 */

    /* Handle 16-bit pixel format - FUN_00412d60 pattern */
    if (ddsd.ddpfPixelFormat.dwRGBBitCount == 16) {
        uint16_t* dst_row;
        uint16_t* src_row;

        for (y = 0; y < height; y++) {
            dst_row = (uint16_t*)(dst_pixels + y * dst_pitch);
            src_row = (uint16_t*)(src_pixels + y * src_pitch);

            /* Check pixel format */
            if (is_555_mode) {
                /* RGB555 mode: swap bytes and convert */
                for (uint32_t x = 0; x < width; x++) {
                    uint16_t pixel = src_row[x];
                    /* Color key: 0 = transparent */
                    if (pixel != 0) {
                        /* Convert from 565 to 555 if needed */
                        uint16_t r = (pixel >> 11) & 0x1F;
                        uint16_t g = (pixel >> 5) & 0x3F;
                        uint16_t b = pixel & 0x1F;
                        /* Convert to 555: r kept, g >> 1, b kept */
                        dst_row[x] = (r << 10) | ((g >> 1) << 5) | b;
                    } else {
                        dst_row[x] = 0;  /* Transparent */
                    }
                }
            } else {
                /* RGB565 mode: direct copy with color key */
                for (uint32_t x = 0; x < width; x++) {
                    uint16_t pixel = src_row[x];
                    /* Color key: 0 = transparent, keep background */
                    if (pixel != 0) {
                        dst_row[x] = pixel;
                    }
                    /* If pixel == 0, leave dst as-is (transparent) */
                }
            }
        }
    } else if (ddsd.ddpfPixelFormat.dwRGBBitCount == 8) {
        /* 8-bit paletted mode - from FUN_00412d60 */
        uint8_t* dst_row;
        uint8_t* src_row;

        for (y = 0; y < height; y++) {
            dst_row = dst_pixels + y * dst_pitch;
            src_row = (uint8_t*)(src_pixels + y * src_pitch);

            for (uint32_t x = 0; x < width; x++) {
                uint8_t pal_index = src_row[x * 2];  /* 16-bit source to 8-bit dest */
                if (pal_index == 0) {
                    dst_row[x] = 0;  /* Transparent */
                } else {
                    dst_row[x] = pal_index;
                }
            }
        }
    }

    IDirectDrawSurface_Unlock(surface, NULL);
    return 1;
}

/*
 * Copy sprite data to dual surfaces (primary + alpha) - FUN_00412b80
 * Used for alpha mode rendering where both color and alpha surfaces need updating
 */
int sprite_copy_to_surfaces(IDirectDrawSurface* primary, IDirectDrawSurface* alpha,
                            int src_x, void* src_data, uint32_t width, int height) {
    DDSURFACEDESC2 ddsd_primary, ddsd_alpha;
    HRESULT hr;
    uint8_t *dst_pixels, *alpha_pixels;
    int dst_pitch, alpha_pitch;
    int src_pitch;
    int y;
    int is_555_mode;

    if (!primary || !src_data) return 0;

    /* Lock primary surface */
    memset(&ddsd_primary, 0, sizeof(DDSURFACEDESC2));
    ddsd_primary.dwSize = sizeof(DDSURFACEDESC2);

    hr = IDirectDrawSurface_Lock(primary, NULL, &ddsd_primary,
                                  DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
    if (FAILED(hr)) {
        LOG_WARN("sprite_copy_to_surfaces: Primary lock failed 0x%08X", hr);
        return 0;
    }

    dst_pixels = (uint8_t*)ddsd_primary.lpSurface;
    dst_pitch = ddsd_primary.lPitch;

    /* Lock alpha surface if provided */
    if (alpha) {
        memset(&ddsd_alpha, 0, sizeof(DDSURFACEDESC2));
        ddsd_alpha.dwSize = sizeof(DDSURFACEDESC2);
        hr = IDirectDrawSurface_Lock(alpha, NULL, &ddsd_alpha,
                                      DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
        if (FAILED(hr)) {
            IDirectDrawSurface_Unlock(primary, NULL);
            LOG_WARN("sprite_copy_to_surfaces: Alpha lock failed 0x%08X", hr);
            return 0;
        }
        alpha_pixels = (uint8_t*)ddsd_alpha.lpSurface;
        alpha_pitch = ddsd_alpha.lPitch;
    } else {
        alpha_pixels = NULL;
        alpha_pitch = 0;
    }

    src_pitch = g_sprite_data_pitch;
    is_555_mode = (g_graphics.bpp == 15);

    /* Copy pixels to both surfaces */
    uint8_t* src_pixels = (uint8_t*)src_data + src_x * 2;

    for (y = 0; y < height; y++) {
        uint16_t* src_row = (uint16_t*)(src_pixels + y * src_pitch);
        uint16_t* dst_row = (uint16_t*)(dst_pixels + y * dst_pitch);
        uint16_t* alpha_row = alpha_pixels ?
                              (uint16_t*)(alpha_pixels + y * alpha_pitch) : NULL;

        for (uint32_t x = 0; x < width; x++) {
            uint16_t pixel = src_row[x];

            /* Copy to primary surface with color key */
            if (pixel != 0) {
                if (is_555_mode) {
                    /* Convert 565 to 555 */
                    uint16_t r = (pixel >> 11) & 0x1F;
                    uint16_t g = (pixel >> 5) & 0x3F;
                    uint16_t b = pixel & 0x1F;
                    dst_row[x] = (r << 10) | ((g >> 1) << 5) | b;
                } else {
                    dst_row[x] = pixel;
                }
            }

            /* Copy to alpha surface */
            if (alpha_row) {
                if (pixel != 0) {
                    if (is_555_mode) {
                        uint16_t r = (pixel >> 11) & 0x1F;
                        uint16_t g = (pixel >> 5) & 0x3F;
                        uint16_t b = pixel & 0x1F;
                        alpha_row[x] = (r << 10) | ((g >> 1) << 5) | b;
                    } else {
                        alpha_row[x] = pixel;
                    }
                } else {
                    alpha_row[x] = 0;
                }
            }
        }
    }

    IDirectDrawSurface_Unlock(primary, NULL);
    if (alpha) {
        IDirectDrawSurface_Unlock(alpha, NULL);
    }

    return 1;
}

/*
 * Sprite surface cache manager - FUN_00480130
 * Manages circular buffer of sprite surfaces for tiling large sprites
 */
int sprite_cache_manager(uint32_t sprite_id, void* sprite_data) {
    int width, height;
    int total_tiles_x, total_tiles_y;
    int tile_width, tile_height;
    int remaining_height, remaining_width;
    int tile_x, tile_y;
    int cache_idx;
    int entries_added = 0;
    int total_entries;
    SpriteCacheEntry* entry;
    int alpha_enabled;
    int src_x, src_y;
    int is_standard_sprite;

    alpha_enabled = g_alpha_mode;
    is_standard_sprite = (sprite_id < SPRITE_ID_STANDARD_MAX);

    /* Get sprite dimensions from global buffer state */
    width = g_sprite_data_pitch;
    height = g_sprite_data_height;

    if (width <= 0 || height <= 0) {
        return 0;
    }

    /* Calculate tile sizes based on sprite type - from FUN_00480130 */
    if (is_standard_sprite) {
        /* Standard sprites: 64x48 tiles (0x40 x 0x30) */
        tile_width = 64;
        tile_height = 48;
    } else {
        /* Extended sprites: 64x64 tiles */
        tile_width = 64;
        tile_height = 64;
    }

    /* Calculate number of tiles needed */
    total_tiles_x = (width + tile_width - 1) / tile_width;
    total_tiles_y = (height + tile_height - 1) / tile_height;
    total_entries = total_tiles_x * total_tiles_y;

    /* Align width for standard sprites if not aligned */
    if (is_standard_sprite && (width & 3) != 0) {
        width = (width + 3) & ~3;  /* Align to 4-pixel boundary */
    }

    remaining_width = width;
    remaining_height = height;
    tile_x = 0;
    tile_y = 0;

    /* Start from current cache index - DAT_0466b7c8 */
    cache_idx = g_cache_current_index;

    LOG_DEBUG("sprite_cache_manager: sprite=%u, size=%dx%d, tiles=%dx%d",
              sprite_id, width, height, total_tiles_x, total_tiles_y);

    /* Process each tile */
    while (entries_added < total_entries) {
        int current_tile_w, current_tile_h;
        int found_slot = 0;
        int iterations = 0;

        /* Find available cache slot */
        while (iterations < g_sprite_cache_count) {
            entry = &g_sprite_cache[cache_idx];

            if (entry->sprite_id == -1 || entry->ref_count <= 0) {
                found_slot = 1;
                break;
            }

            cache_idx++;
            if (cache_idx >= g_sprite_cache_count) {
                cache_idx = 0;
            }
            iterations++;
        }

        if (!found_slot) {
            /* Cache is full, evict from current position */
            entry = &g_sprite_cache[cache_idx];

            if (entry->sprite_id != -1 && entry->sprite_id != (int)sprite_id) {
                /* Clear alpha buffer if present */
                if (alpha_enabled && entry->alpha_buffer) {
                    memset(entry->alpha_buffer, 0,
                           g_cache_sprite_width * g_cache_sprite_height);
                }
            }
        }

        /* Calculate current tile dimensions */
        if (is_standard_sprite) {
            current_tile_w = tile_width;
            current_tile_h = (remaining_height > tile_height) ? tile_height : remaining_height;
            src_x = 0;
            src_y = tile_y * tile_height;
        } else {
            current_tile_w = (remaining_width > tile_width) ? tile_width : remaining_width;
            current_tile_h = (remaining_height > tile_height) ? tile_height : remaining_height;
            src_x = tile_x * tile_width;
            src_y = tile_y * tile_height;
        }

        /* Ensure surface exists */
        if (!entry->surface) {
            entry->surface = graphics_create_offscreen(
                g_cache_sprite_width > 0 ? g_cache_sprite_width : 64,
                g_cache_sprite_height > 0 ? g_cache_sprite_height : 48,
                SURFACE_FLAG_VIDEO_MEMORY
            );
            if (!entry->surface) {
                entry->surface = graphics_create_offscreen(
                    g_cache_sprite_width > 0 ? g_cache_sprite_width : 64,
                    g_cache_sprite_height > 0 ? g_cache_sprite_height : 48,
                    SURFACE_FLAG_SYSTEM_MEMORY
                );
            }
        }

        if (!entry->surface) {
            LOG_WARN("Failed to create sprite cache surface");
            cache_idx++;
            if (cache_idx >= g_sprite_cache_count) cache_idx = 0;
            continue;
        }

        /* Create alpha surface if needed */
        if (alpha_enabled && !entry->alpha_surface) {
            entry->alpha_surface = graphics_create_offscreen(
                g_cache_sprite_width > 0 ? g_cache_sprite_width : 64,
                g_cache_sprite_height > 0 ? g_cache_sprite_height : 48,
                SURFACE_FLAG_SYSTEM_MEMORY
            );
            if (entry->alpha_buffer) {
                memset(entry->alpha_buffer, 0,
                       g_cache_sprite_width * g_cache_sprite_height);
            }
        }

        /* Calculate offsets */
        if (g_graphics.bpp == 16) {
            entry->x_offset = (short)(src_x / 2);
            if (is_standard_sprite) {
                entry->y_offset = (short)(height - src_y - current_tile_h);
            } else {
                entry->y_offset = (short)current_tile_h;
            }
        } else {
            entry->x_offset = (short)src_x;
            entry->y_offset = (short)src_y;
        }

        /* Set entry properties */
        entry->sprite_id = sprite_id;
        entry->sprite_data = sprite_data;
        entry->ref_count = 0;
        entry->flags = 0;

        /* Copy sprite data to surface */
        if (alpha_enabled && entry->alpha_surface) {
            sprite_copy_to_surfaces(entry->surface, entry->alpha_surface,
                                    src_x, sprite_data,
                                    current_tile_w, current_tile_h);
        } else {
            sprite_copy_to_surface(entry->surface, src_x, sprite_data,
                                   current_tile_w, current_tile_h);
        }

        entries_added++;

        /* Update tile position for next iteration */
        if (is_standard_sprite) {
            remaining_height -= tile_height;
            tile_y++;
        } else {
            remaining_width -= tile_width;
            tile_x++;
            if (remaining_width <= 0) {
                remaining_width = width;
                tile_x = 0;
                remaining_height -= tile_height;
                tile_y++;
            }
        }

        /* Advance cache index (circular) */
        cache_idx++;
        if (cache_idx >= g_sprite_cache_count) {
            cache_idx = 0;
        }
    }

    /* Update global cache index for next allocation */
    g_cache_current_index = cache_idx;

    return 1;
}

/*
 * Sprite loading and caching - FUN_004808e0
 * Loads sprite from disk and caches in surface cache
 */
int sprite_load_and_cache(uint32_t sprite_id) {
    int width, height;
    void* sprite_data;
    u32 out_width, out_height;
    int result;

    /* Validate sprite ID range */
    if (sprite_id >= SPRITE_ID_INVALID) {
        LOG_WARN("Invalid sprite ID: %u", sprite_id);
        return 0;
    }

    /* Check if already cached */
    if (sprite_id < SPRITE_ID_STANDARD_MAX) {
        u32 cache_lookup_index = sprite_id * 3;
        if (cache_lookup_index < (u32)g_sprite_cache_count) {
            SpriteCacheEntry* entry = &g_sprite_cache[cache_lookup_index];
            if (entry->sprite_id == (int)sprite_id) {
                return 1;  /* Already cached */
            }
        }
    }

    /* Load sprite data from asset system */
    sprite_data = assets_load_and_decode_sprite(sprite_id, &width, &height);
    if (!sprite_data) {
        LOG_WARN("Failed to load sprite %u", sprite_id);
        return 0;
    }

    /* Set global sprite data buffer for cache manager */
    out_width = (u32)width;
    out_height = (u32)height;
    sprite_set_data_buffer(sprite_data, out_width * 2, out_height);

    /* Cache the sprite in surface cache */
    result = sprite_cache_manager(sprite_id, sprite_data);

    return result;
}
