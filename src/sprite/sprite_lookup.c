/*
 * Stone Age Client - Sprite Lookup Functions
 * Split from sprite.c for code organization
 *
 * Handles sprite offset and dimension lookup
 * Based on FUN_0041fad0, FUN_0041f980
 */

#include <windows.h>
#include "types.h"
#include "sprite.h"

/* Sprite data tables - these would be loaded from data files in the real implementation */
/* Standard sprites: IDs 0-499999, lookup at 0x00a04c64 (offset) and 0x00e8f23c (dimensions) */
/* Extended sprites: IDs 500000-549999, special handling */

/* Placeholder tables - would be loaded from data files */
static u32* g_sprite_offset_table = NULL;
static SpriteDimension* g_sprite_dim_table = NULL;
static SpriteDimension* g_sprite_dim_table_hires = NULL;

/*
 * Get sprite data offset - FUN_0041fad0
 * Returns the data offset for a sprite ID
 * Standard sprites: Look up in table
 * Extended sprites (500000-549999): Return the ID itself
 */
int sprite_lookup_offset(u32 sprite_id, u32* offset) {
    if (sprite_id < SPRITE_ID_HIGHRES_BASE) {
        /* Standard sprite - look up offset in table */
        /* Table at DAT_00a04c64, stride 4 bytes */
        /* In real implementation, this would access the loaded data */
        if (offset) {
            *offset = sprite_id * 0x50;  /* Placeholder calculation */
        }
        return 1;
    }

    if (sprite_id <= SPRITE_ID_HIGHRES_MAX) {
        /* Extended sprite - return ID as offset */
        if (offset) {
            *offset = sprite_id;
        }
        return 1;
    }

    /* Invalid sprite ID */
    if (offset) *offset = 0;
    return 0;
}

/*
 * Get sprite dimensions - FUN_0041f980
 * Returns width and height for a sprite data offset
 *
 * TODO: Verify dimension table loading from actual data files
 * Binary uses:
 * - DAT_00e8f23c for standard sprites (stride 0x50 bytes)
 * - DAT_0081c7fc for high-res sprites
 */
int sprite_lookup_dimension(u32 sprite_id, u16* width, u16* height) {
    if (sprite_id < SPRITE_ID_HIGHRES_BASE) {
        /* Standard sprite - look up in dimension table */
        /* Table at DAT_00e8f23c, stride 0x50 bytes */
        /* Width at +0x00, Height at +0x04 in each entry */
        if (g_sprite_dim_table && width && height) {
            *width = g_sprite_dim_table[sprite_id].width;
            *height = g_sprite_dim_table[sprite_id].height;
            return 1;
        }
        /* Placeholder values */
        if (width) *width = 64;
        if (height) *height = 48;
        return 1;
    }

    if (sprite_id <= SPRITE_ID_HIGHRES_MAX) {
        /* Extended sprite - look up in extended table */
        /* Calculation: (id * 5 - 2500000) * 8 */
        /* Table at DAT_0081c7fc */
        if (g_sprite_dim_table_hires && width && height) {
            u32 index = (sprite_id - SPRITE_ID_HIGHRES_BASE) * 5 + 2500000;
            u32 offset = index * 8;
            /* High-res sprites have 8-byte entries */
            *width = ((SpriteDimension*)((u8*)g_sprite_dim_table_hires + offset))[0].width;
            *height = ((SpriteDimension*)((u8*)g_sprite_dim_table_hires + offset))[0].height;
            return 1;
        }
        /* Placeholder values */
        if (width) *width = 128;
        if (height) *height = 96;
        return 1;
    }

    if (width) *width = 0;
    if (height) *height = 0;
    return 0;
}

/*
 * Check if sprite is high-res - from FUN_0041fad0
 */
int sprite_is_highres(u32 sprite_id) {
    return (sprite_id >= SPRITE_ID_HIGHRES_BASE && sprite_id <= SPRITE_ID_HIGHRES_MAX);
}

/*
 * Get palette group from palette value - FUN_0047e210 pattern
 * Values 0-9: group 0, 10-19: group 1, 20-29: group 2, etc.
 */
int sprite_get_palette_group(int palette) {
    if (palette < 10) {
        return 0;
    } else if (palette < 20) {
        return 1;
    } else if (palette < 30) {
        return 2;
    } else if (palette < 40) {
        return 3;
    } else {
        return 4;
    }
}

/*
 * Set sprite offset table (for loading from data files)
 */
void sprite_set_offset_table(u32* table) {
    g_sprite_offset_table = table;
}

/*
 * Set sprite dimension table (for loading from data files)
 */
void sprite_set_dimension_table(SpriteDimension* table) {
    g_sprite_dim_table = table;
}

/*
 * Set high-res sprite dimension table
 */
void sprite_set_dimension_table_hires(SpriteDimension* table) {
    g_sprite_dim_table_hires = table;
}
