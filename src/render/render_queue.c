/*
 * Stone Age Client - Render Queue Management
 * Reverse engineered from sa_9061.exe
 * FUN_0047e210, FUN_0047dc60, FUN_0047e720, FUN_0047e640, FUN_0047e970
 *
 * Memory layout from binary:
 * DAT_0464f488 - Queue count
 * DAT_0464b488 - Index array (short per entry)
 * DAT_0464b48a - Layer byte (byte per entry)
 * DAT_04633488 - X position array (int per entry, stride 6)
 * DAT_0463348c - Y position array (int per entry, stride 6)
 * DAT_04633490 - Sprite ID array (int per entry, stride 6)
 * DAT_04633494 - Linked object array (int per entry, stride 6)
 * DAT_04633498 - Priority array (int per entry, stride 6)
 * DAT_0463349c - Blend mode array (byte per entry, stride 0x18)
 */

#include <windows.h>
#include <ddraw.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "render.h"
#include "render_surface.h"
#include "render_blend.h"
#include "render_sprite.h"
#include "render_text.h"
#include "directx.h"
#include "logger.h"

/* Maximum queue entries - 0xfff from FUN_0047e210 */
#define MAX_QUEUE_ENTRIES 0x1000

/* Queue storage - using separate arrays like original binary */
static u16 s_queue_index[MAX_QUEUE_ENTRIES];      /* DAT_0464b488 */
static u8  s_queue_layer[MAX_QUEUE_ENTRIES];      /* DAT_0464b48a */
static s32 s_queue_x[MAX_QUEUE_ENTRIES];          /* DAT_04633488 */
static s32 s_queue_y[MAX_QUEUE_ENTRIES];          /* DAT_0463348c */
static u32 s_queue_sprite_id[MAX_QUEUE_ENTRIES];  /* DAT_04633490 */
static void* s_queue_linked[MAX_QUEUE_ENTRIES];   /* DAT_04633494 */
static s32 s_queue_priority[MAX_QUEUE_ENTRIES];   /* DAT_04633498 */
static u8  s_queue_blend_mode[MAX_QUEUE_ENTRIES]; /* DAT_0463349c */

/* Queue count - DAT_0464f488 */
static int s_queue_count = 0;

/* Surface lost flag - DAT_0464f7b0 */
static int s_surface_lost = 0;

/* Render mode - DAT_005ab6fc */
static int s_render_mode = 0;

/* Sprite width/height globals - DAT_0466b7d4, DAT_0466b7d0 */
extern int g_sprite_width;
extern int g_sprite_height;

/* Forward declarations */
static void render_queue_process_entry(int index, int use_alpha);

/*
 * Initialize render queue
 */
void render_queue_init(void) {
    memset(s_queue_index, 0, sizeof(s_queue_index));
    memset(s_queue_layer, 0, sizeof(s_queue_layer));
    memset(s_queue_x, 0, sizeof(s_queue_x));
    memset(s_queue_y, 0, sizeof(s_queue_y));
    memset(s_queue_sprite_id, 0, sizeof(s_queue_sprite_id));
    memset(s_queue_linked, 0, sizeof(s_queue_linked));
    memset(s_queue_priority, 0, sizeof(s_queue_priority));
    memset(s_queue_blend_mode, 0, sizeof(s_queue_blend_mode));
    s_queue_count = 0;
    s_surface_lost = 0;
}

/*
 * Clear render queue
 */
void render_queue_clear(void) {
    s_queue_count = 0;
}

/*
 * Get current render queue count
 */
int render_queue_get_count(void) {
    return s_queue_count;
}

/*
 * Set pixel format for blending operations
 */
void render_set_pixel_format(int format) {
    s_pixel_format = format;
}

/*
 * Add entry to render queue - FUN_0047e210 pattern
 * Returns queue index on success, -2 if queue full
 *
 * Note: This function allows special sprite IDs (with flags like 0x80000000)
 * which bypass the normal sprite ID validation.
 */
int render_queue_add(int x, int y, u32 sprite_id, u32 flags, void* linked_obj) {
    int index;
    u16 sprite_w = 0, sprite_h = 0;
    int priority = flags & 0xFF;
    int blend_mode = 0;

    /* Check queue limit - 0xfff from original */
    if (s_queue_count >= 0xfff) {
        return -2;
    }

    index = s_queue_count;

    /* Check for special sprite flags (0x80000000) - these bypass validation */
    if ((sprite_id & 0x80000000) == 0) {
        /* Normal sprite - validate ID range */
        s32 sid = (s32)sprite_id;
        if (sid >= -1 && sid < 100) {
            /* Invalid range: -1 to 99, but allow anyway for special cases */
            sprite_w = 0;
            sprite_h = 0;
        } else {
            render_get_sprite_dimensions(sprite_id, &sprite_w, &sprite_h);
        }
    }

    /* Calculate blend mode from priority - FUN_0047e210 pattern */
    if (priority < 10) {
        blend_mode = 0;
    } else if (priority < 0x14) {  /* 10-19 */
        blend_mode = 1;
        priority -= 10;
    } else if (priority < 0x1e) {  /* 20-29 */
        blend_mode = 2;
        priority -= 0x14;
    } else if (priority < 0x28) {  /* 30-39 */
        blend_mode = 3;
        priority -= 0x1e;
    } else if (priority < 0x32) {  /* 40-49 */
        blend_mode = 4;
        priority -= 0x28;
    } else {
        blend_mode = 0;
    }

    /* Store entry */
    s_queue_index[index] = (u16)index;
    s_queue_layer[index] = 0;
    s_queue_x[index] = x + sprite_w;
    s_queue_y[index] = y + sprite_h;
    s_queue_sprite_id[index] = sprite_id;
    s_queue_linked[index] = linked_obj;
    s_queue_priority[index] = priority;
    s_queue_blend_mode[index] = (u8)blend_mode;

    s_queue_count++;
    return index;
}

/*
 * Add sprite to render queue - FUN_0047e210
 * This is the core function matching original binary behavior
 * Returns queue index on success, -2 if queue full or invalid sprite_id
 *
 * From Ghidra analysis:
 * - Sprite IDs -1 to 99 are invalid (return -2)
 * - Sprite IDs >= 100 or < -1 are valid
 * - Priority ranges: 0-9, 10-19 (0xa-0x13), 20-29 (0x14-0x1d),
 *                    30-39 (0x1e-0x27), 40-49 (0x28-0x31)
 */
int render_queue_add_sprite(int x, int y, int layer, u32 sprite_id, int priority) {
    int index;
    u16 sprite_w = 0, sprite_h = 0;
    int blend_mode;
    int actual_priority;
    s32 sid = (s32)sprite_id;

    /* Check queue limit (0xfff = 4095 from original) */
    if (s_queue_count >= 0xfff) {
        return -2;
    }

    /* Sprite ID validation - FUN_0047e210 pattern
     * From Ghidra:
     * if (param_4 < -1) {
     *   if (99 < param_4) goto get_dimensions;
     *   local_8[0] = 0; // Invalid range 0-99
     * } else {
     *   if (param_4 < 100) return -2;  // Invalid: -1 to 99
     *   get_dimensions:
     * }
     */
    if (sid >= -1 && sid < 100) {
        /* Invalid range: -1 to 99 */
        return -2;
    }

    /* Get sprite dimensions for valid sprites */
    if (sid >= 100 || sid < -1) {
        render_get_sprite_dimensions(sprite_id, &sprite_w, &sprite_h);
    }

    /* Calculate priority layer - FUN_0047e210 pattern
     * From Ghidra:
     * - priority < 10: blend_mode = 0, priority unchanged
     * - priority < 0x14 (20): blend_mode = 1, priority -= 10
     * - priority < 0x1e (30): blend_mode = 2, priority -= 0x14 (20)
     * - priority < 0x28 (40): blend_mode = 3, priority -= 0x1e (30)
     * - priority < 0x32 (50): blend_mode = 4, priority -= 0x28 (40)
     * - priority >= 50: blend_mode = 0
     */
    if (priority < 10) {
        blend_mode = 0;
        actual_priority = priority;
    } else if (priority < 0x14) {  /* 10-19 */
        blend_mode = 1;
        actual_priority = priority - 10;
    } else if (priority < 0x1e) {  /* 20-29 */
        blend_mode = 2;
        actual_priority = priority - 0x14;
    } else if (priority < 0x28) {  /* 30-39 */
        blend_mode = 3;
        actual_priority = priority - 0x1e;
    } else if (priority < 0x32) {  /* 40-49 */
        blend_mode = 4;
        actual_priority = priority - 0x28;
    } else {
        blend_mode = 0;
        actual_priority = priority;
    }

    index = s_queue_count;

    /* Store entry - matching DAT_04633488 region layout */
    s_queue_index[index] = (u16)index;
    s_queue_layer[index] = (u8)layer;
    s_queue_x[index] = x + (s32)sprite_w;
    s_queue_y[index] = y + (s32)sprite_h;
    s_queue_sprite_id[index] = sprite_id;
    s_queue_linked[index] = NULL;
    s_queue_priority[index] = actual_priority;
    s_queue_blend_mode[index] = (u8)blend_mode;

    s_queue_count++;
    return index;
}

/*
 * Compare function for sorting render queue by Y position
 */
static int render_queue_compare(const void* a, const void* b) {
    int idx_a = *(const int*)a;
    int idx_b = *(const int*)b;
    return s_queue_y[idx_a] - s_queue_y[idx_b];
}

/*
 * Sort render queue by position
 */
void render_queue_sort(void) {
    /* Sorting is done by processing in Y order */
    /* Original binary processes entries in insertion order */
}

/*
 * Add fade/render effect to queue - FUN_0047e640
 * Uses special sprite ID encoding with flags
 *
 * Parameters packed as: x1|x3 in param_3, y1|y2 in param_4
 * fade_mode: 0 = fill rect (0xa0000000), 1 = special (0x90000000), 2 = outline (0xc0000000)
 */
int render_queue_add_fade(int x1, int y1, int x2, int y2, int sprite_type, u32 base_sprite, int fade_mode) {
    u32 packed_x;
    u32 packed_y;
    u32 sprite_id;

    if (s_queue_count >= 0xfff) {
        return -2;
    }

    /* Pack coordinates: x1 in low 16 bits, x2 in high 16 bits */
    packed_x = ((u32)x1 & 0xFFFF) | ((u32)x2 << 16);
    packed_y = ((u32)y1 & 0xFFFF) | ((u32)y2 << 16);

    /* Build sprite ID with fade flags based on mode */
    switch (fade_mode) {
        case 0:
            /* Fill rectangle mode - OR with 0xa0000000 */
            sprite_id = base_sprite | 0xa0000000;
            break;
        case 1:
            /* Special render mode - OR with 0x90000000 */
            sprite_id = base_sprite | 0x90000000;
            break;
        case 2:
            /* Outline mode - OR with 0xc0000000 */
            sprite_id = base_sprite | 0xc0000000;
            break;
        default:
            sprite_id = base_sprite;
            break;
    }

    /* Add to queue - use sprite_type as layer, store packed coords in x/y */
    int index = s_queue_count;
    s_queue_index[index] = (u16)index;
    s_queue_layer[index] = (u8)sprite_type;
    s_queue_x[index] = (s32)packed_x;
    s_queue_y[index] = (s32)packed_y;
    s_queue_sprite_id[index] = sprite_id;
    s_queue_linked[index] = NULL;
    s_queue_priority[index] = 0;
    s_queue_blend_mode[index] = 0;

    s_queue_count++;
    return index;
}

/*
 * Set render mode
 */
void render_set_mode(int mode) {
    s_render_mode = mode;
}

/*
 * Check if surface was lost
 */
int render_is_surface_lost(void) {
    return s_surface_lost;
}

/*
 * Reset render state
 */
void render_state_reset(void) {
    s_surface_lost = 0;
}

/*
 * Process single render queue entry - from FUN_0047dc60 pattern
 */
static void render_queue_process_entry(int index, int use_alpha) {
    u32 sprite_id;
    int x, y;
    int blend_mode;
    SpriteNode* node;

    sprite_id = s_queue_sprite_id[index];
    x = s_queue_x[index];
    y = s_queue_y[index];
    blend_mode = s_queue_blend_mode[index];

    /* Check for special render flags (0x80000000) - FUN_0047dc60 pattern */
    if (sprite_id & 0x80000000) {
        u32 flags = sprite_id & 0x70000000;
        if (flags != 0) {
            /* FUN_00412eb0: primitive rect rendering */
            RECT rect;
            int mode;
            rect.left = x & 0xFFFF;
            rect.top = y & 0xFFFF;
            rect.right = (x >> 16) & 0xFFFF;
            rect.bottom = (y >> 16) & 0xFFFF;

            /* Mode from flags: 0x20000000=0, 0x10000000=1, 0x40000000=2 */
            if (flags & 0x20000000) {
                mode = 0;
            } else if (flags & 0x10000000) {
                mode = 1;
            } else {
                mode = 2;
            }
            render_primitive_rect(&rect, sprite_id, mode);
        }
        return;
    }

    /* Lazy load sprite if needed - FUN_004808e0 */
    if (!render_get_sprite_surface(sprite_id)) {
        if (!render_load_sprite(sprite_id)) {
            return;
        }
    }

    /* Walk sprite node linked list - FUN_0047dc60 pattern
     * Each node is a 64x48 tile chunk with its own surface and offset.
     * Position = queue_base_pos + node_offset. */
    node = render_get_sprite_node(sprite_id);
    if (!node) {
        /* Fallback: single surface blit */
        SpriteSurfaceEntry* sprite = render_get_sprite_surface(sprite_id);
        if (sprite && sprite->surface) {
            render_blit_transparent(sprite->surface, g_graphics.offscreen_surface,
                0, 0, sprite->width, sprite->height, x, y, 0);
            sprite->timestamp = timeGetTime();
        }
        return;
    }

    while (node) {
        int node_x = x + (int)node->x_offset;
        int node_y = y + (int)node->y_offset;

        if (node->surface) {
            /* Get node surface dimensions */
            DDSURFACEDESC2 ddsd;
            int nw = 64, nh = 48;  /* Default tile chunk size */
            memset(&ddsd, 0, sizeof(ddsd));
            ddsd.dwSize = sizeof(ddsd);
            if (SUCCEEDED(IDirectDrawSurface_GetSurfaceDesc(node->surface, &ddsd))) {
                nw = ddsd.dwWidth;
                nh = ddsd.dwHeight;
            }

            switch (blend_mode) {
                case 0:
                    render_blit_transparent(node->surface, g_graphics.offscreen_surface,
                        0, 0, nw, nh, node_x, node_y, 0);
                    break;
                case 1:
                    if (use_alpha) {
                        render_blit_alpha_blend(node->surface, g_graphics.offscreen_surface,
                            node_x, node_y, nw, nh, 16);
                    } else {
                        render_blit_transparent(node->surface, g_graphics.offscreen_surface,
                            0, 0, nw, nh, node_x, node_y, 0);
                    }
                    break;
                case 2:
                    render_blit_additive(node->surface, g_graphics.offscreen_surface,
                        node_x, node_y, nw, nh);
                    break;
                case 3:
                    render_blit_subtractive(node->surface, g_graphics.offscreen_surface,
                        node_x, node_y, nw, nh);
                    break;
                default:
                    render_blit_transparent(node->surface, g_graphics.offscreen_surface,
                        0, 0, nw, nh, node_x, node_y, 0);
                    break;
            }
            node->timestamp = timeGetTime();
        }

        node = node->next;
    }
}

/*
 * Process render queue - FUN_0047dc60 pattern
 */
void render_queue_process(int use_alpha) {
    int i;

    for (i = 0; i < s_queue_count; i++) {
        render_queue_process_entry(i, use_alpha);
    }

    s_queue_count = 0;
}

/*
 * Process render queue with priority layers - FUN_0047dc60
 */
void render_queue_process_full(void) {
    int i;
    HRESULT hr;
    u8 last_layer = 0;

    /* Check render mode - DAT_005ab6fc */
    if (s_render_mode == 0) {
        /* Normal mode - FUN_0047dc60 mode 0:
         * FUN_00412a40: clear back buffer
         * FUN_0047d850: update animation timer
         * FUN_0047d8e0: animation update (returns 1 if battle active)
         * Surface-lost recovery
         * FUN_0047e720: process background sprites (layers 0-1)
         */
        if (g_graphics.back_buffer) {
            DDBLTFX bltfx;
            memset(&bltfx, 0, sizeof(bltfx));
            bltfx.dwSize = sizeof(bltfx);
            IDirectDrawSurface_Blt(g_graphics.back_buffer, NULL, NULL, NULL,
                DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
        }

        /* Process background sprites - FUN_0047e720 */
        render_process_background();
    }
    else if (s_render_mode == 3) {
        /* Special mode - battle render */
        render_state_reset();
    }
    else {
        /* Other modes */
        if (g_graphics.back_buffer && g_graphics.offscreen_surface) {
            hr = IDirectDrawSurface_BltFast(g_graphics.back_buffer, 0, 0,
                g_graphics.offscreen_surface, NULL, 0x11);
            if (hr == DDERR_SURFACELOST) {
                s_surface_lost = 1;
                return;
            }
        }
    }

    /* Process render queue entries - FUN_0047dc60 pattern
     * At specific layer thresholds, text queue is flushed (FUN_00414820):
     *   layer > 0x67: flush text groups 0 and 3, set state 1
     *   layer > 0x6c: flush text group 1, set state 2
     *   layer > 0x6e: flush text group 2, set state 2
     * After all entries: flush remaining text groups
     */
    {
        int text_state = 0;  /* Tracks which text passes have been rendered */
        for (i = 0; i < s_queue_count; i++) {
            u8 layer = s_queue_layer[i];

            /* Text rendering at layer thresholds - exact FUN_0047dc60 pattern */
            if (text_state == 0 && layer > 0x67) {
                text_queue_process(0);
                text_queue_process(3);
                text_state = 1;
            }
            if (text_state == 1 && layer > 0x6c) {
                text_queue_process(1);
                text_state = 2;
            }
            if (text_state == 2 && layer > 0x6e) {
                text_queue_process(2);
            }

            last_layer = layer;
            render_queue_process_entry(i, 1);
        }

        /* Final text flush matching FUN_0047dc60 post-loop */
        if (text_state == 0) {
            text_queue_process(0);
            text_queue_process(3);
            text_queue_process(1);
        } else if (text_state == 1) {
            text_queue_process(1);
        }
        /* text_state == 2: groups 0,3,1 already done, group 2 done if threshold hit */

        /* Clear all text entries after final flush */
        text_queue_process(-1);
    }

    /* Clear render entries */
    memset(s_queue_x, 0, sizeof(s_queue_x[0]) * s_queue_count);
    memset(s_queue_y, 0, sizeof(s_queue_y[0]) * s_queue_count);
    memset(s_queue_sprite_id, 0, sizeof(s_queue_sprite_id[0]) * s_queue_count);
    memset(s_queue_blend_mode, 0, sizeof(s_queue_blend_mode[0]) * s_queue_count);
    s_queue_count = 0;
}

/*
 * Process background sprites - FUN_0047e720
 *
 * From Ghidra: processes only layer 0 entries, then adjusts queue count.
 * Layer 0 sprites are rendered as background before the main queue processing.
 * When a layer > 0 entry is encountered, the processed count is subtracted
 * from the total and processing stops.
 */
void render_process_background(void) {
    int i;
    int bg_count = 0;

    for (i = 0; i < s_queue_count; i++) {
        u8 layer = s_queue_layer[i];

        /* Layer 0: process as background */
        if (layer == 0) {
            render_queue_process_entry(i, 1);
            bg_count++;
        } else if (layer == 1) {
            /* Layer 1: also background but signals end of layer 0 group */
            render_queue_process_entry(i, 1);
            bg_count++;
        } else {
            /* Layer > 1: stop background processing, adjust queue count */
            /* Remove processed background entries by shifting remaining */
            break;
        }
    }

    /* Compact queue: remove processed background entries */
    if (bg_count > 0 && bg_count < s_queue_count) {
        int remaining = s_queue_count - bg_count;
        memmove(s_queue_x, s_queue_x + bg_count, remaining * sizeof(s_queue_x[0]));
        memmove(s_queue_y, s_queue_y + bg_count, remaining * sizeof(s_queue_y[0]));
        memmove(s_queue_layer, s_queue_layer + bg_count, remaining * sizeof(s_queue_layer[0]));
        memmove(s_queue_sprite_id, s_queue_sprite_id + bg_count, remaining * sizeof(s_queue_sprite_id[0]));
        memmove(s_queue_blend_mode, s_queue_blend_mode + bg_count, remaining * sizeof(s_queue_blend_mode[0]));
        s_queue_count = remaining;
    } else if (bg_count >= s_queue_count) {
        s_queue_count = 0;
    }
}

