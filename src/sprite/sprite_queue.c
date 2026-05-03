/*
 * Stone Age Client - Sprite Render Queue
 * Split from sprite.c for code organization
 *
 * Handles render queue for sprite batching
 * Based on FUN_0047e210, FUN_0047e640, FUN_0041fad0, FUN_0041f900
 */

#include <windows.h>
#include <string.h>
#include "types.h"
#include "sprite.h"

/* Global render queue */
SpriteRenderQueue g_sprite_queue = {0};

/* Sprite data lookup tables - from binary addresses */
/* DAT_00a04c64: Standard sprite offset table */
/* DAT_00e8f234: Standard sprite dimension table (0x50 bytes per entry) */
/* DAT_0081c7f4: High-res sprite dimension table */

/* Placeholder tables - would be loaded from data files */
static u32* g_sprite_offset_table = NULL;
static SpriteDimension* g_sprite_dim_table = NULL;
static SpriteDimension* g_sprite_dim_table_hires = NULL;

/* External functions from sprite_lookup.c */
extern int sprite_lookup_offset(u32 sprite_id, u32* offset);
extern int sprite_lookup_dimension(u32 sprite_id, u16* width, u16* height);
extern int sprite_is_highres(u32 sprite_id);
extern int sprite_get_palette_group(int palette);

/*
 * Initialize render queue
 */
void sprite_queue_clear(void) {
    memset(&g_sprite_queue, 0, sizeof(SpriteRenderQueue));
}

/*
 * Get current queue count
 */
u32 sprite_queue_get_count(void) {
    return g_sprite_queue.count;
}

/*
 * Add sprite to render queue - FUN_0047e210 implementation
 * Parameters:
 *   x, y: Position
 *   type: Sprite type (0x68=UI, 0x69=Icon, 0x6d=Button, 0x6e=Effect)
 *   sprite_id: Sprite ID (0-549999)
 *   palette: Palette/style index
 * Returns: Queue index on success, -2 on failure
 */
int sprite_queue_add(int x, int y, u8 type, int sprite_id, int palette) {
    SpriteRenderEntry* entry;
    u32 index;
    u16 dim_x = 0, dim_y = 0;

    /* Check queue limit - from binary: 0xfff limit */
    if (g_sprite_queue.count >= SPRITE_QUEUE_MAX - 1) {
        return -2;
    }

    /* Validate sprite ID - from FUN_0041fad0 */
    if (sprite_id < 100 && sprite_id > 99) {
        /* Special case: negative sprite IDs for system sprites */
    } else if (sprite_id < 100 && sprite_id >= -1) {
        /* Sprite IDs 0-99 need lookup */
        dim_x = 0;
        dim_y = 0;
    } else if (sprite_id >= 100) {
        /* Get dimensions from lookup */
        sprite_lookup_dimension((u32)sprite_id, &dim_x, &dim_y);
    } else {
        return -2;
    }

    index = g_sprite_queue.count;
    entry = &g_sprite_queue.entries[index];

    /* Set entry data - from DAT_04633488 region analysis */
    entry->queue_index = (u16)index;
    entry->sprite_type = type;
    entry->x = x + dim_x;   /* Add dimension offset */
    entry->y = y + dim_y;
    entry->sprite_id = sprite_id;
    entry->flags = 0;

    /* Process palette - FUN_0047e210 pattern */
    if (palette < 10) {
        entry->palette = palette;
        entry->palette_group = 0;
    } else if (palette < 20) {
        entry->palette_group = 1;
        entry->palette = palette - 10;
    } else if (palette < 30) {
        entry->palette_group = 2;
        entry->palette = palette - 20;
    } else if (palette < 40) {
        entry->palette_group = 3;
        entry->palette = palette - 30;
    } else {
        entry->palette_group = 4;
        entry->palette = palette - 40;
    }

    g_sprite_queue.count++;
    g_sprite_queue.total_queued++;

    return (int)index;
}

/*
 * Add scaled sprite to queue - FUN_0047e640 implementation
 * Parameters:
 *   x1, y1: Top-left corner
 *   x2, y2: Bottom-right corner
 *   type: Sprite type
 *   sprite_id: Sprite ID
 *   mode: Render mode (0=normal, 1=additive, 2=subtractive, 3=custom)
 * Returns: Queue index on success
 */
int sprite_queue_scaled(int x1, int y1, int x2, int y2,
                        u8 type, int sprite_id, SpriteRenderMode mode) {
    int modified_id;

    /* Apply render mode flags to sprite_id */
    switch (mode) {
        case SPRITE_RENDER_MODE_ADDITIVE:
            modified_id = sprite_id | 0xa0000000;
            break;
        case SPRITE_RENDER_MODE_SUBTRACTIVE:
            modified_id = sprite_id | 0x90000000;
            break;
        case SPRITE_RENDER_MODE_CUSTOM:
            modified_id = sprite_id | 0xc0000000;
            break;
        default:
            modified_id = sprite_id;
            break;
    }

    return sprite_queue_add(x1, y1, type, modified_id, 0);
}

/*
 * Process render queue - simplified version
 * Full implementation would use DirectDraw surfaces
 */
void sprite_queue_process(void* surface, u32 pitch) {
    u32 i;
    SpriteRenderEntry* entry;

    for (i = 0; i < g_sprite_queue.count; i++) {
        entry = &g_sprite_queue.entries[i];

        /* Skip invalid entries */
        if (entry->sprite_id < 0 && entry->sprite_id != -1) {
            continue;
        }

        /* Render sprite entry */
        /* Full implementation would look up sprite data and render */
        /* For now, this is a placeholder */

        g_sprite_queue.total_rendered++;
    }

    /* Clear queue after processing */
    g_sprite_queue.count = 0;
}
