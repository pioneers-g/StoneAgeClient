/*
 * Stone Age Client - Render Queue Processing
 * Reverse engineered from sa_9061.exe
 *
 * Core rendering pipeline functions for sprite and text rendering.
 * FUN_0047dc60 - Main render queue processor
 * FUN_0047d8e0 - Field background rendering
 * FUN_0047e970 - Alpha blending
 * FUN_0047f170 - Additive blending
 */

#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "render_queue_process.h"
#include "directx.h"
#include "sprite.h"
#include "assets.h"
#include "logger.h"

/* ========================================
 * Global State
 * ======================================== */

RenderState g_render = {0};

/* Render queue arrays - these match DAT_04633488 and DAT_0464b488 */
#define MAX_QUEUE_ENTRIES   0x7000
static RenderQueueEntry s_queue_entries[MAX_QUEUE_ENTRIES];
static RenderQueueIndex s_queue_indices[MAX_QUEUE_ENTRIES];
static u32 s_queue_entry_count = 0;

/* Animation state */
static int s_anim_sprite_id = 0;
static int s_anim_x = 0;
static int s_anim_y = 0;
static int s_anim_vel_x = 0;
static int s_anim_vel_y = 0;
static u32 s_anim_timer = 0;
static u32 s_anim_update_timer = 0;

/* ========================================
 * Helper Functions
 * ======================================== */

/*
 * Check if position is within screen bounds
 */
static int is_on_screen(int x, int y, int width, int height) {
    return (x < g_render.screen_width && x > -width &&
            y < g_render.screen_height && y > -height);
}

/*
 * Get screen dimensions based on resolution mode
 */
static void update_screen_dimensions(void) {
    /* DAT_04560214: 0 = 640x480, 1 = 320x240 */
    if (g_graphics.bpp == 15) {
        g_render.screen_width = 320;
        g_render.screen_height = 240;
    } else {
        g_render.screen_width = 640;
        g_render.screen_height = 480;
    }
}

/* ========================================
 * Core Render Functions
 * ======================================== */

/*
 * Clear back buffer - FUN_00412a40
 */
void render_clear_back_buffer(void) {
    DDBLTFX bltfx;

    memset(&bltfx, 0, sizeof(bltfx));
    bltfx.dwSize = sizeof(bltfx);

    /* Clear primary surface */
    if (g_graphics.back_buffer) {
        IDirectDrawSurface_Blt(g_graphics.back_buffer, NULL, NULL, NULL,
                               DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
    } else if (g_graphics.offscreen_surface) {
        IDirectDrawSurface_Blt(g_graphics.offscreen_surface, NULL, NULL, NULL,
                               DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
    }
}

/*
 * Update animation timer - FUN_0047d850
 */
void render_update_animation_timer(void) {
    DWORD current_time;
    u32 rand_val;

    if (s_anim_sprite_id == 0) {
        return;
    }

    current_time = timeGetTime();

    if (s_anim_update_timer >= current_time) {
        return;
    }

    s_anim_update_timer = current_time + 1000;

    /* Random velocity update */
    rand_val = sprite_lcg_random();

    if ((rand_val & 3) == 0) {
        s_anim_vel_x = 4 - (sprite_lcg_random() & 2);
    } else if ((rand_val & 3) != 1) {
        s_anim_vel_y = 2 + (sprite_lcg_random() & 2);
    }
}

/*
 * Render dynamic background - FUN_0047d5b0
 * Handles minimap cursor and animated elements
 */
void render_dynamic_background(void) {
    DWORD current_time;

    if (s_anim_sprite_id == 0) {
        return;
    }

    current_time = timeGetTime();

    if (s_anim_timer < current_time) {
        /* Update position */
        s_anim_x += s_anim_vel_x;
        s_anim_y += s_anim_vel_y;
        s_anim_timer = current_time + 73;  /* 0x49 ms */

        /* Check bounds */
        if (s_anim_x > 639 || s_anim_y > 479) {
            /* Reset with new random sprite */
            u32 rand_val = sprite_lcg_random();
            s_anim_sprite_id = (rand_val & 3) + 0x9e34;

            /* Randomize velocity */
            rand_val = sprite_lcg_random();
            if (rand_val % 3 == 0) {
                s_anim_vel_x = sprite_lcg_random() & 0x1f;
            } else {
                s_anim_vel_y = sprite_lcg_random() & 0x1f;
            }

            s_anim_x = 40;
            s_anim_y = 30;
        }
    }
}

/*
 * Load sprite into cache - FUN_004808e0
 * Checks if sprite is already cached, loads if not
 */
int sprite_cache_load(u32 sprite_id) {
    return sprite_load_and_cache(sprite_id);
}

/*
 * Render field background - FUN_0047d8e0
 */
int render_field_background(void) {
    int scroll_x = 0, scroll_y = 0;
    int clip_x = 0, clip_y = 0;
    int screen_w, screen_h;
    int i;
    RenderQueueIndex* sprite_index;

    g_render.queue_ptr = s_queue_indices;
    g_render.queue_skip = 0;

    /* Only process for field (state 9) or battle (state 10) */
    /* This check would use game state from gamestate module */

    update_screen_dimensions();
    screen_w = g_render.screen_width;
    screen_h = g_render.screen_height;

    /* Handle scrolling offsets */
    if (g_render.scroll_x < 1) {
        if (g_render.scroll_x < 0) {
            clip_x = -g_render.scroll_x;
        }
    } else {
        screen_w -= g_render.scroll_x;
        scroll_x = g_render.scroll_x;
    }

    if (g_render.scroll_y < 1) {
        if (g_render.scroll_y < 0) {
            clip_y = -g_render.scroll_y;
        }
    } else {
        screen_h -= g_render.scroll_y;
        scroll_y = g_render.scroll_y;
    }

    /* Process render queue */
    if (g_render.queue_count > 0) {
        sprite_index = s_queue_indices;

        for (i = 0; i < (int)g_render.queue_count; i++) {
            u8 priority = sprite_index->priority;

            /* Skip priority 0 entries */
            if (priority == 0) {
                sprite_index++;
                continue;
            }

            /* Priority > 1 means stop */
            if (priority > 1) {
                g_render.queue_count -= i;
                g_render.queue_skip = i;
                break;
            }

            /* Get queue entry */
            u16 entry_index = sprite_index->queue_index;
            RenderQueueEntry* entry = &s_queue_entries[entry_index];
            u32 sprite_id = entry->sprite_id;

            /* Check if hidden */
            if (entry->entity_ptr != 0) {
                /* Check entity hidden flag at +0xa0 */
                u8* entity = (u8*)entry->entity_ptr;
                if (entity && (entity[0xa0] & 0x20)) {
                    sprite_index++;
                    continue;
                }
            }

            /* Load sprite */
            if (sprite_cache_load(sprite_id)) {
                /* Render sprite chain */
                /* This would traverse the linked list at sprite_id * 3 */
                /* and call render_sprite_blit for each entry */
            }

            sprite_index++;
        }
    }

    return 1;
}

/*
 * Render sprites for field mode
 */
int render_sprites_field(void) {
    return render_field_background();
}

/*
 * Render sprites for normal mode - FUN_0047e720
 */
void render_sprites_normal(void) {
    int i;
    RenderQueueIndex* sprite_index;

    /* Adjust queue count */
    g_render.queue_count += g_render.queue_skip;
    g_render.queue_ptr = s_queue_indices;

    if (g_render.queue_count <= 0) {
        return;
    }

    sprite_index = g_render.queue_ptr;

    for (i = 0; i < (int)g_render.queue_count; i++) {
        u8 priority = sprite_index->priority;

        /* Skip priority 1 (already rendered) */
        if (priority == 1) {
            sprite_index++;
            continue;
        }

        if (priority > 1) {
            g_render.queue_count -= i;
            break;
        }

        /* Get queue entry */
        u16 entry_index = sprite_index->queue_index;
        RenderQueueEntry* entry = &s_queue_entries[entry_index];
        u32 sprite_id = entry->sprite_id;

        /* Check hidden flag */
        if (entry->entity_ptr != 0) {
            u8* entity = (u8*)entry->entity_ptr;
            if (entity && (entity[0xa0] & 0x20)) {
                sprite_index++;
                continue;
            }
        }

        /* Load and render sprite */
        if (sprite_cache_load(sprite_id)) {
            /* Render with appropriate blend mode */
        }

        sprite_index++;
    }
}

/*
 * Render text queue - FUN_00414820
 */
void render_text_queue(int layer) {
    /* Text rendering would be handled by the text system */
    /* This is called at specific priority thresholds */
    (void)layer;
}

/*
 * Main render queue processing - FUN_0047dc60
 */
void render_queue_process(void) {
    int text_layer = 0;
    int dynamic_rendered = 0;
    int i;

    update_screen_dimensions();

    /* Handle different render modes */
    if (g_render.mode == RENDER_MODE_NORMAL) {
        /* Normal mode */
        render_clear_back_buffer();
        render_update_animation_timer();

        if (!render_sprites_field()) {
            /* Error handling */
        }

        render_sprites_normal();

    } else if (g_render.mode == RENDER_MODE_BATTLE) {
        /* Battle mode - reset scroll */
        g_render.scroll_x = 0;
        g_render.scroll_y = 0;
        render_sprites_field();
        /* Battle UI rendering would go here */

    } else {
        /* Other modes (loading, etc) */
        render_sprites_field();
    }

    /* Process render queue with priority-based text layer rendering */
    if (g_render.queue_count > 0) {
        RenderQueueIndex* sprite_index = g_render.queue_ptr;

        for (i = 0; i < (int)g_render.queue_count; i++) {
            u8 priority = sprite_index->priority;
            u16 entry_index = sprite_index->queue_index;
            RenderQueueEntry* entry = &s_queue_entries[entry_index];
            u32 sprite_id = entry->sprite_id;

            /* Handle text layer transitions based on priority */
            if (text_layer == 0) {
                if (priority > PRIORITY_LAYER_CHARACTER) {
                    render_text_queue(0);
                    render_text_queue(3);
                    text_layer = 1;

                    if (priority > PRIORITY_LAYER_UI) {
                        render_text_queue(1);
                        text_layer = 2;
                    }
                }
            } else if (text_layer == 1) {
                if (priority > PRIORITY_LAYER_UI) {
                    render_text_queue(1);
                    text_layer = 2;
                }
                if (priority > PRIORITY_LAYER_EFFECT) {
                    render_text_queue(2);
                }
            }

            /* Handle dynamic background for priority > 0x13 */
            if (priority > 0x13) {
                render_dynamic_background();
                dynamic_rendered = 1;
            }

            /* Process sprite based on flags */
            if ((sprite_id & 0x80000000) == 0) {
                /* Normal sprite rendering */
                if (sprite_cache_load(sprite_id)) {
                    int blend_type = entry->blend_type;

                    /* Choose appropriate render function based on blend type */
                    switch (blend_type) {
                        case BLEND_MODE_ALPHA:
                            /* render_blend_alpha(...); */
                            break;
                        case BLEND_MODE_ADDITIVE:
                            /* render_blend_additive(...); */
                            break;
                        case BLEND_MODE_SPECIAL:
                            /* render_blend_special(...); */
                            break;
                        default:
                            /* Normal blit */
                            break;
                    }
                }
            } else if (sprite_id & 0x70000000) {
                /* Special effects rendering */
                int effect_type = 0;
                if (sprite_id & 0x20000000) effect_type = 0;
                else if (sprite_id & 0x10000000) effect_type = 1;
                else if (sprite_id & 0x40000000) effect_type = 2;
                /* Call effect renderer */
            }

            sprite_index++;
        }
    }

    /* Final text layer rendering */
    if (text_layer == 0) {
        render_text_queue(0);
        render_text_queue(3);
    } else if (text_layer == 1) {
        render_text_queue(1);
    }

    if (!dynamic_rendered) {
        /* Final cleanup */
    }

    /* Clear render queue */
    render_queue_clear();
}

/*
 * Alpha blending - FUN_0047e970
 * Blends source onto destination using alpha value
 * Supports both RGB565 and RGB555 pixel formats
 */
void render_blend_alpha(u16* dest, const u16* src,
                        int dest_pitch, int src_pitch,
                        int width, int height,
                        u8 alpha, int flags) {
    int x, y;
    u32 alpha_factor;
    int is_565;

    if (!dest || !src || width <= 0 || height <= 0) {
        return;
    }

    is_565 = (g_graphics.bpp != 15);  /* DAT_0054bdec: 2 = 565 */
    alpha_factor = (alpha * 32) / 256;  /* Scale to 0-31 range */

    for (y = 0; y < height; y++) {
        u16* dest_row = (u16*)((u8*)dest + y * dest_pitch);
        const u16* src_row = (const u16*)((const u8*)src + y * src_pitch);

        for (x = 0; x < width; x++) {
            u16 src_pixel = src_row[x];
            u16 dest_pixel = dest_row[x];

            /* Skip transparent pixels (color key 0) */
            if (src_pixel == 0) {
                continue;
            }

            if (is_565) {
                /* RGB565 blending */
                u32 src_r = (src_pixel >> 11) & 0x1F;
                u32 src_g = (src_pixel >> 5) & 0x3F;
                u32 src_b = src_pixel & 0x1F;

                u32 dest_r = (dest_pixel >> 11) & 0x1F;
                u32 dest_g = (dest_pixel >> 5) & 0x3F;
                u32 dest_b = dest_pixel & 0x1F;

                /* Blend: result = dest + (src - dest) * alpha / 32 */
                u32 result_r = dest_r + (((src_r - dest_r) * alpha_factor) >> 5);
                u32 result_g = dest_g + (((src_g - dest_g) * alpha_factor) >> 5);
                u32 result_b = dest_b + (((src_b - dest_b) * alpha_factor) >> 5);

                /* Clamp */
                if (result_r > 0x1F) result_r = 0x1F;
                if (result_g > 0x3F) result_g = 0x3F;
                if (result_b > 0x1F) result_b = 0x1F;

                dest_row[x] = (u16)((result_r << 11) | (result_g << 5) | result_b);
            } else {
                /* RGB555 blending */
                u32 src_r = (src_pixel >> 10) & 0x1F;
                u32 src_g = (src_pixel >> 5) & 0x1F;
                u32 src_b = src_pixel & 0x1F;

                u32 dest_r = (dest_pixel >> 10) & 0x1F;
                u32 dest_g = (dest_pixel >> 5) & 0x1F;
                u32 dest_b = dest_pixel & 0x1F;

                u32 result_r = dest_r + (((src_r - dest_r) * alpha_factor) >> 5);
                u32 result_g = dest_g + (((src_g - dest_g) * alpha_factor) >> 5);
                u32 result_b = dest_b + (((src_b - dest_b) * alpha_factor) >> 5);

                if (result_r > 0x1F) result_r = 0x1F;
                if (result_g > 0x1F) result_g = 0x1F;
                if (result_b > 0x1F) result_b = 0x1F;

                dest_row[x] = (u16)((result_r << 10) | (result_g << 5) | result_b);
            }
        }
    }
}

/*
 * Additive blending - FUN_0047f170
 * Adds source color to destination (for lighting effects)
 */
void render_blend_additive(u16* dest, const u16* src,
                           int dest_pitch, int src_pitch,
                           int width, int height, int flags) {
    int x, y;
    int is_565;

    if (!dest || !src || width <= 0 || height <= 0) {
        return;
    }

    is_565 = (g_graphics.bpp != 15);

    for (y = 0; y < height; y++) {
        u16* dest_row = (u16*)((u8*)dest + y * dest_pitch);
        const u16* src_row = (const u16*)((const u8*)src + y * src_pitch);

        for (x = 0; x < width; x++) {
            u16 src_pixel = src_row[x];
            u16 dest_pixel = dest_row[x];

            if (src_pixel == 0) {
                continue;
            }

            if (is_565) {
                /* RGB565 additive */
                u32 src_r = (src_pixel >> 11) & 0x1F;
                u32 src_g = (src_pixel >> 5) & 0x3F;
                u32 src_b = src_pixel & 0x1F;

                u32 dest_r = (dest_pixel >> 11) & 0x1F;
                u32 dest_g = (dest_pixel >> 5) & 0x3F;
                u32 dest_b = dest_pixel & 0x1F;

                /* Add with saturation */
                u32 result_r = src_r + dest_r;
                u32 result_g = src_g + dest_g;
                u32 result_b = src_b + dest_b;

                if (result_r > 0x1F) result_r = 0x1F;
                if (result_g > 0x3F) result_g = 0x3F;
                if (result_b > 0x1F) result_b = 0x1F;

                dest_row[x] = (u16)((result_r << 11) | (result_g << 5) | result_b);
            } else {
                /* RGB555 additive */
                u32 src_r = (src_pixel >> 10) & 0x1F;
                u32 src_g = (src_pixel >> 5) & 0x1F;
                u32 src_b = src_pixel & 0x1F;

                u32 dest_r = (dest_pixel >> 10) & 0x1F;
                u32 dest_g = (dest_pixel >> 5) & 0x1F;
                u32 dest_b = dest_pixel & 0x1F;

                u32 result_r = src_r + dest_r;
                u32 result_g = src_g + dest_g;
                u32 result_b = src_b + dest_b;

                if (result_r > 0x1F) result_r = 0x1F;
                if (result_g > 0x1F) result_g = 0x1F;
                if (result_b > 0x1F) result_b = 0x1F;

                dest_row[x] = (u16)((result_r << 10) | (result_g << 5) | result_b);
            }
        }
    }
}

/*
 * Special blending - FUN_0047f710
 */
void render_blend_special(u16* dest, const u16* src,
                          int dest_pitch, int src_pitch,
                          int width, int height,
                          int blend_type, int flags) {
    /* Implementation would handle various special blend modes */
    /* Based on blend_type parameter (4+) */
    (void)blend_type;
    (void)flags;

    /* Default to normal blending for now */
    render_blend_alpha(dest, src, dest_pitch, src_pitch, width, height, 16, 0);
}

/*
 * Sprite blitting without alpha - FUN_004142f0
 */
int render_sprite_blit(int x, int y, void* sprite_data) {
    /* Would use DirectDraw BltFast or Blt with color key */
    (void)x;
    (void)y;
    (void)sprite_data;
    return 1;
}

/*
 * Sprite blitting with alpha surface - FUN_00414190
 */
int render_sprite_blit_alpha(int x, int y, void* sprite_data, void* alpha_data) {
    /* Would use DirectDraw Blt with alpha surface */
    (void)x;
    (void)y;
    (void)sprite_data;
    (void)alpha_data;
    return 1;
}

/*
 * Clear render queue after processing
 */
void render_queue_clear(void) {
    memset(s_queue_entries, 0, sizeof(s_queue_entries));
    memset(s_queue_indices, 0, sizeof(s_queue_indices));
    s_queue_entry_count = 0;
    g_render.queue_count = 0;
    g_render.queue_skip = 0;
}

/*
 * Get render queue count
 */
u32 render_get_queue_count(void) {
    return g_render.queue_count;
}

/*
 * Get current render mode
 */
int render_get_mode(void) {
    return g_render.mode;
}

/*
 * Set render mode
 */
void render_set_mode(int mode) {
    g_render.mode = (RenderMode)mode;
}
