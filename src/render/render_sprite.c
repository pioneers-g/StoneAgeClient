/*
 * Stone Age Client - Sprite Cache and Rendering
 * Reverse engineered from sa_9061.exe
 * FUN_0041fad0, FUN_0041f900, FUN_004808e0, FUN_0041fc90
 *
 * This module handles:
 * - Sprite surface cache management
 * - Sprite loading and decoding
 * - Sprite dimension lookup
 * - Extended sprite (500000+) handling
 */

#include <windows.h>
#include <ddraw.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "render.h"
#include "render_surface.h"
#include "directx.h"
#include "assets.h"
#include "logger.h"

/* Sprite cache storage - matching DAT_0466b7e0 region
 * Two regions: 0-499999 for regular sprites, 500000+ for extended sprites
 */
static SpriteSurfaceEntry s_sprite_cache[SPRITE_SURFACE_CACHE_SIZE];
static int s_sprite_cache_initialized = 0;

/* Sprite table - matching DAT_0466b7e0 (12 bytes per entry, stride 3 dwords)
 * Stores head pointers to linked lists of SpriteNode per sprite_id
 */
static SpriteTableEntry s_sprite_table[SPRITE_SURFACE_CACHE_SIZE];

/* Node pool - matching DAT_0464f7c0 (28 bytes per node)
 * Fixed-size pool with LRU eviction for sprite surface nodes
 */
#define SPRITE_NODE_POOL_SIZE 4096
static SpriteNode s_node_pool[SPRITE_NODE_POOL_SIZE];
static int s_node_pool_initialized = 0;

/* Extended sprite file handle - DAT_00a04c60 pattern */
static HANDLE s_extended_sprite_file = INVALID_HANDLE_VALUE;

/* Alpha mode flag - DAT_0054c83c */
static int s_alpha_mode = 0;

/*
 * Initialize sprite cache
 */
void render_sprite_cache_init(void) {
    if (s_sprite_cache_initialized) {
        return;
    }

    memset(s_sprite_cache, 0, sizeof(s_sprite_cache));
    memset(s_sprite_table, 0, sizeof(s_sprite_table));
    memset(s_node_pool, 0, sizeof(s_node_pool));
    s_sprite_cache_initialized = 1;
    s_node_pool_initialized = 1;
    LOG_INFO("Sprite cache initialized (%d entries, %d nodes)", SPRITE_SURFACE_CACHE_SIZE, SPRITE_NODE_POOL_SIZE);
}

/*
 * Get sprite node head from table - DAT_0466b7e0 lookup
 * Returns head of linked list for given sprite_id
 */
SpriteNode* render_get_sprite_node(u32 sprite_id) {
    if (sprite_id >= SPRITE_SURFACE_CACHE_SIZE) return NULL;
    return s_sprite_table[sprite_id].head;
}

/*
 * Get alpha node head from table - DAT_0466b7e4 lookup
 */
SpriteNode* render_get_alpha_node(u32 sprite_id) {
    if (sprite_id >= SPRITE_SURFACE_CACHE_SIZE) return NULL;
    return s_sprite_table[sprite_id].alpha_head;
}

/*
 * Get sprite surface from cache
 */
SpriteSurfaceEntry* render_get_sprite_surface(u32 sprite_id) {
    if (sprite_id >= SPRITE_SURFACE_CACHE_SIZE) {
        return NULL;
    }

    if (s_sprite_cache[sprite_id].surface) {
        return &s_sprite_cache[sprite_id];
    }

    return NULL;
}

/*
 * Get sprite dimensions - FUN_0041f900
 * Returns width and height for the given sprite ID
 *
 * Binary uses two lookup tables:
 * - DAT_00e8f234: Standard sprite dimension table (sprite_id * 0x50)
 *   Entry at offset 0x00: width (u16)
 *   Entry at offset 0x02: height (u16)
 * - DAT_0081c7f4: Extended sprite dimension table
 *   Formula: (sprite_id - 500000) * 40
 *   Entry at offset 0x00: width (u16)
 *   Entry at offset 0x04: height (u16)
 *
 * Returns 1 on success, 0 if sprite_id >= 550000
 */
int render_get_sprite_dimensions(u32 sprite_id, u16* width, u16* height) {
    int iwidth, iheight;

    if (!width || !height) return 0;

    /* Check cache first for already-loaded sprites */
    if (sprite_id < SPRITE_SURFACE_CACHE_SIZE && s_sprite_cache[sprite_id].surface) {
        *width = s_sprite_cache[sprite_id].width;
        *height = s_sprite_cache[sprite_id].height;
        return 1;
    }

    /* Use assets module for dimension lookup */
    if (assets_get_sprite_dimensions(sprite_id, &iwidth, &iheight)) {
        *width = (u16)iwidth;
        *height = (u16)iheight;
        return 1;
    }

    /* Standard sprite range: 0 - 499999 */
    if (sprite_id < SPRITE_ID_STANDARD_MAX) {
        /* Default dimensions for standard sprites */
        *width = 64;
        *height = 64;
        return 1;
    }

    /* Extended sprite range: 500000 - 549999 */
    if (sprite_id >= SPRITE_ID_STANDARD_MAX && sprite_id < SPRITE_ID_EXTENDED_MAX) {
        /* Default dimensions for extended sprites */
        *width = 64;
        *height = 64;
        return 1;
    }

    /* Invalid sprite ID: >= 550000 */
    *width = 0;
    *height = 0;
    return 0;
}

/*
 * Get sprite data pointer - FUN_0041fad0
 * Returns pointer/index for sprite data
 *
 * Binary behavior:
 * - sprite_id < 500000: look up in DAT_00a04c64 (pointer table)
 * - sprite_id 500000-549999: return sprite_id directly
 * - sprite_id >= 550000: return 0 (invalid)
 */
u32 render_get_sprite_data_pointer(u32 sprite_id) {
    if (sprite_id < 500000) {
        /* Would look up in DAT_00a04c64 + sprite_id * 4 */
        return sprite_id; /* Placeholder */
    }

    if (sprite_id > 549999) {
        return 0;
    }

    /* Extended sprites return the ID as-is */
    return sprite_id;
}

/*
 * Get sprite pointer from ID - FUN_0041fad0
 * For sprites < 500000: look up pointer in table
 * For sprites 500000-549999: return ID as-is
 * For sprites >= 550000: return 0 (invalid)
 */
u32 render_get_sprite_pointer(u32 sprite_id) {
    if (sprite_id < 500000) {
        /* Look up in sprite pointer table at DAT_00a04c64 */
        return sprite_id;
    }

    if (sprite_id > 549999) {
        return 0;  /* Invalid */
    }

    /* Extended sprites return the ID directly */
    return sprite_id;
}

/*
 * Load sprite into cache - FUN_004808e0 pattern
 *
 * Binary behavior:
 * - sprite_id < 500000: load from sprite info table (DAT_00e8f228)
 * - sprite_id 500000-549999: load extended sprite via FUN_0041fc90
 * - sprite_id >= 550000: return 0 (invalid)
 *
 * Cache entry at DAT_0466b7e0 has 12 bytes per entry:
 * - +0x00: surface pointer (4 bytes)
 * - +0x04: width (2 bytes)
 * - +0x06: height (2 bytes)
 * - +0x08: timestamp/other (4 bytes)
 */
int render_load_sprite(u32 sprite_id) {
    SpriteSurfaceEntry* entry;
    int width, height;
    void* sprite_data;
    DDSURFACEDESC2 ddsd;
    HRESULT hr;

    /* Check sprite ID validity - FUN_004808e0 pattern */
    if (sprite_id >= SPRITE_ID_EXTENDED_MAX) {
        return 0;  /* Invalid: >= 550000 */
    }

    if (sprite_id >= SPRITE_SURFACE_CACHE_SIZE) {
        return 0;
    }

    /* Initialize cache if needed */
    if (!s_sprite_cache_initialized) {
        render_sprite_cache_init();
    }

    entry = &s_sprite_cache[sprite_id];

    if (entry->surface) {
        return 1;  /* Already loaded - FUN_004808e0 checks DAT_0466b7e0[param_1*3] */
    }

    /* Check if this is an extended sprite - FUN_004808e0 pattern */
    if (sprite_id >= SPRITE_ID_STANDARD_MAX) {
        /* Extended sprite: delegate to render_load_extended_sprite */
        return render_load_extended_sprite(sprite_id);
    }

    /* Standard sprite: load from assets */
    sprite_data = assets_load_and_decode_sprite(sprite_id, &width, &height);
    if (!sprite_data) {
        /* Try to get basic sprite info from dimension table */
        if (!assets_get_sprite_dimensions(sprite_id, &width, &height)) {
            width = 64;
            height = 64;
        }
        /* Create empty surface even without data */
    }

    /* Create surface with 0x800 flag (system memory + video memory compatible) */
    entry->surface = render_create_surface(width, height, 0x800);
    if (!entry->surface) {
        if (sprite_data) free(sprite_data);
        return 0;
    }

    /* Set source color key: black (0) = transparent - FUN_004142f0 uses DDBLT_KEYSRC */
    {
        DDCOLORKEY ck;
        ck.dwColorSpaceLowValue = 0;
        ck.dwColorSpaceHighValue = 0;
        IDirectDrawSurface_SetColorKey(entry->surface, DDCKEY_SRCBLT, &ck);
    }

    entry->width = (u16)width;
    entry->height = (u16)height;
    entry->timestamp = timeGetTime();

    /* Update sprite table (DAT_0466b7e0) - single-node for complete sprite */
    s_sprite_table[sprite_id].width = (u16)width;
    s_sprite_table[sprite_id].height = (u16)height;

    /* Allocate a node from pool for this sprite */
    if (!s_sprite_table[sprite_id].head) {
        int ni;
        for (ni = 0; ni < SPRITE_NODE_POOL_SIZE; ni++) {
            if (s_node_pool[ni].surface == NULL) {
                SpriteNode* node = &s_node_pool[ni];
                node->surface = entry->surface;
                node->alpha_data = entry->alpha_surface;
                node->frame_type = 1;
                node->sprite_id = sprite_id;
                node->timestamp = timeGetTime();
                node->x_offset = 0;
                node->y_offset = 0;
                node->next = NULL;
                s_sprite_table[sprite_id].head = node;
                break;
            }
        }
    }

    /* Copy sprite data to surface if available */
    if (sprite_data) {
        memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
        ddsd.dwSize = sizeof(DDSURFACEDESC2);

        hr = IDirectDrawSurface_Lock(entry->surface, NULL, &ddsd, DDLOCK_WAIT, NULL);
        if (SUCCEEDED(hr)) {
            u8* dst = (u8*)ddsd.lpSurface;
            u8* src = (u8*)sprite_data;
            int y;
            u32 src_pitch = width * 2;
            u32 dst_pitch = ddsd.lPitch;

            for (y = 0; y < height; y++) {
                memcpy(dst + y * dst_pitch, src + y * src_pitch, src_pitch);
            }

            IDirectDrawSurface_Unlock(entry->surface, NULL);
        }

        free(sprite_data);
    }

    return 1;
}

/*
 * Load extended sprite (ID >= 500000) - FUN_0041fc90 pattern
 * These sprites are stored in a separate file (extsprite.bin)
 *
 * Binary behavior from FUN_0041fc90:
 * 1. Check ext_id < 50000 (sprite_id - 500000)
 * 2. Seek to file offset from DAT_0081c7e4[ext_id * 10]
 * 3. Read palette data (DAT_0081c7e8 entry + 4 bytes)
 * 4. Read sprite data (DAT_0081c7ec entry bytes)
 * 5. Decode sprite via FUN_0048a550
 * 6. Optionally read extra data (DAT_0081c7f0 entry bytes)
 */
int render_load_extended_sprite(u32 sprite_id) {
    SpriteSurfaceEntry* entry;
    int width, height;
    void* sprite_data;
    DDSURFACEDESC2 ddsd;
    HRESULT hr;
    u32 ext_id;

    /* Extended sprites are indexed 500000+ but stored as 0-50000 */
    ext_id = sprite_id - SPRITE_ID_STANDARD_MAX;
    if (ext_id >= 50000) {
        return 0;
    }

    if (sprite_id >= SPRITE_SURFACE_CACHE_SIZE) {
        return 0;
    }

    entry = &s_sprite_cache[sprite_id];

    if (entry->surface) {
        return 1;  /* Already loaded */
    }

    /* Load extended sprite from assets module */
    sprite_data = assets_load_and_decode_sprite(sprite_id, &width, &height);
    if (!sprite_data) {
        /* Use default dimensions for extended sprites */
        width = 64;
        height = 64;
    }

    /* Create surface */
    entry->surface = render_create_surface(width, height, 0x800);
    if (!entry->surface) {
        if (sprite_data) free(sprite_data);
        return 0;
    }

    /* Set source color key: black = transparent */
    {
        DDCOLORKEY ck;
        ck.dwColorSpaceLowValue = 0;
        ck.dwColorSpaceHighValue = 0;
        IDirectDrawSurface_SetColorKey(entry->surface, DDCKEY_SRCBLT, &ck);
    }

    entry->width = (u16)width;
    entry->height = (u16)height;
    entry->timestamp = timeGetTime();

    /* Copy sprite data to surface if available */
    if (sprite_data) {
        memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
        ddsd.dwSize = sizeof(DDSURFACEDESC2);

        hr = IDirectDrawSurface_Lock(entry->surface, NULL, &ddsd, DDLOCK_WAIT, NULL);
        if (SUCCEEDED(hr)) {
            u8* dst = (u8*)ddsd.lpSurface;
            u8* src = (u8*)sprite_data;
            int y;
            u32 src_pitch = width * 2;
            u32 dst_pitch = ddsd.lPitch;

            for (y = 0; y < height; y++) {
                memcpy(dst + y * dst_pitch, src + y * src_pitch, src_pitch);
            }

            IDirectDrawSurface_Unlock(entry->surface, NULL);
        }

        free(sprite_data);
    }

    return 1;
}

/*
 * Unload sprite from cache
 */
void render_unload_sprite(u32 sprite_id) {
    SpriteSurfaceEntry* entry;

    if (sprite_id >= SPRITE_SURFACE_CACHE_SIZE) {
        return;
    }

    entry = &s_sprite_cache[sprite_id];

    if (entry->surface) {
        IDirectDrawSurface_Release(entry->surface);
        entry->surface = NULL;
    }

    if (entry->alpha_surface) {
        IDirectDrawSurface_Release(entry->alpha_surface);
        entry->alpha_surface = NULL;
    }

    entry->width = 0;
    entry->height = 0;
}

/*
 * Clear all sprite cache entries
 */
void render_clear_sprite_cache(void) {
    u32 i;

    for (i = 0; i < SPRITE_SURFACE_CACHE_SIZE; i++) {
        if (s_sprite_cache[i].surface) {
            IDirectDrawSurface_Release(s_sprite_cache[i].surface);
            s_sprite_cache[i].surface = NULL;
        }
        if (s_sprite_cache[i].alpha_surface) {
            IDirectDrawSurface_Release(s_sprite_cache[i].alpha_surface);
            s_sprite_cache[i].alpha_surface = NULL;
        }
        s_sprite_cache[i].width = 0;
        s_sprite_cache[i].height = 0;
    }

    LOG_INFO("Sprite cache cleared");
}

/*
 * Set alpha mode for sprite rendering
 */
void render_sprite_set_alpha_mode(int enabled) {
    s_alpha_mode = enabled;
}

/*
 * Get alpha mode
 */
int render_sprite_get_alpha_mode(void) {
    return s_alpha_mode;
}

/*
 * Render sprite at position
 */
int render_sprite(u32 sprite_id, int x, int y) {
    SpriteSurfaceEntry* sprite;
    u16 width, height;

    /* Get sprite from cache or load it */
    sprite = render_get_sprite_surface(sprite_id);
    if (!sprite) {
        if (!render_load_sprite(sprite_id)) {
            return 0;
        }
        sprite = render_get_sprite_surface(sprite_id);
    }

    if (!sprite || !sprite->surface) {
        return 0;
    }

    /* Render with transparency */
    return render_blit_transparent(sprite->surface, g_graphics.offscreen_surface,
        0, 0, sprite->width, sprite->height, x, y, 0);
}

/*
 * Render sprite frame
 */
int render_sprite_frame(u32 sprite_id, int frame, int x, int y) {
    SpriteSurfaceEntry* sprite;
    int frame_x, frame_y;
    int frame_width, frame_height;
    int frames_per_row;

    /* Get sprite from cache */
    sprite = render_get_sprite_surface(sprite_id);
    if (!sprite) {
        if (!render_load_sprite(sprite_id)) {
            return 0;
        }
        sprite = render_get_sprite_surface(sprite_id);
    }

    if (!sprite || !sprite->surface) {
        return 0;
    }

    /* Calculate frame dimensions (typically 64x64 or sprite size / frames_per_row) */
    frame_width = sprite->width;
    frame_height = sprite->height;

    /* Standard animation: 8 frames per row, 8 directions */
    /* Frame index = direction * 8 + animation_frame */
    if (sprite->width >= 512 && sprite->height >= 64) {
        /* Multi-frame sprite sheet */
        frames_per_row = sprite->width / 64;
        frame_width = 64;
        frame_height = 64;

        frame_x = (frame % frames_per_row) * frame_width;
        frame_y = (frame / frames_per_row) * frame_height;
    } else if (sprite->width >= 128 && sprite->height >= 128) {
        /* 2x2 frame layout */
        frames_per_row = sprite->width / 64;
        frame_width = 64;
        frame_height = 64;

        frame_x = (frame % frames_per_row) * frame_width;
        frame_y = (frame / frames_per_row) * frame_height;
    } else {
        /* Single frame sprite */
        frame_x = 0;
        frame_y = 0;
    }

    /* Blit the frame */
    return render_blit_transparent(sprite->surface, g_graphics.offscreen_surface,
        frame_x, frame_y, frame_width, frame_height, x, y, 0);
}

/*
 * Render scaled sprite
 */
int render_sprite_scaled(u32 sprite_id, int x, int y, int w, int h) {
    SpriteSurfaceEntry* sprite;

    sprite = render_get_sprite_surface(sprite_id);
    if (!sprite) {
        if (!render_load_sprite(sprite_id)) {
            return 0;
        }
        sprite = render_get_sprite_surface(sprite_id);
    }

    if (!sprite || !sprite->surface) {
        return 0;
    }

    return render_blit_scaled(sprite->surface, g_graphics.offscreen_surface,
        0, 0, sprite->width, sprite->height, x, y, w, h);
}
