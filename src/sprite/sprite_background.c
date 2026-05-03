/*
 * Stone Age Client - Background Rendering
 * Split from sprite.c for code organization
 *
 * Handles background surfaces and rendering
 * Based on FUN_0047cd80
 */

#include <windows.h>
#include "types.h"
#include "sprite.h"
#include "ddraw.h"

/* Background surfaces - DAT_046333xx region */
static IDirectDrawSurface* g_bg_surface1 = NULL;  /* DAT_046333f8 */
static IDirectDrawSurface* g_bg_surface2 = NULL;  /* DAT_046333fc */
static IDirectDrawSurface* g_bg_surface3 = NULL;  /* DAT_04633400 */

/* Background dimensions */
static u16 g_bg_width = 0;      /* DAT_046333b4 */
static u16 g_bg_height = 0;     /* DAT_046333d4 */
static s32 g_bg_offset_x = 0;   /* DAT_04633398 */
static s32 g_bg_offset_y = 0;   /* DAT_046333bc */

/* Background sprite IDs */
static u32 g_bg_sprite_id = 0;  /* DAT_046333f0 */

/* External functions from sprite_lookup.c */
extern int sprite_lookup_offset(u32 sprite_id, u32* offset);
extern int sprite_lookup_dimension(u32 sprite_id, u16* width, u16* height);

/* External function from sprite_lcg.c */
extern u32 sprite_lcg_random(void);

/*
 * Initialize background surfaces - FUN_0047cd80
 * Creates background layers for field rendering
 *
 * TODO: Verify background surface creation logic against FUN_0047cd80
 * Binary uses:
 * - DAT_046333f8, DAT_046333fc, DAT_04633400 for surface pointers
 * - DAT_046333b4, DAT_046333d4 for dimensions
 * - DAT_0458119c, DAT_04569b70 for offset calculations
 * - LCG random (FUN_00492403) for sprite selection
 */
void sprite_init_background(u32 sprite_id) {
    u32 data_offset;
    u16 width, height;
    u32 random_val;
    u16 bg_width, bg_height;

    if (sprite_id == 0) {
        return;
    }

    g_bg_sprite_id = sprite_id;

    /* Get sprite data offset - FUN_0041fad0 */
    if (!sprite_lookup_offset(sprite_id & 0xFFFF, &data_offset)) {
        return;
    }

    /* Get sprite dimensions - FUN_0041f980 */
    if (!sprite_lookup_dimension(data_offset, &width, &height)) {
        return;
    }

    /* Check if dimensions changed */
    if (width != g_bg_width || height != g_bg_height) {
        /* Calculate new offsets */
        g_bg_offset_x = 0;  /* Would be calculated from DAT_0458119c + DAT_04569b70 */
        g_bg_offset_y = 0;

        g_bg_width = width;
        g_bg_height = height;

        /* Release old surface */
        if (g_bg_surface1) {
            IDirectDrawSurface_Release(g_bg_surface1);
            g_bg_surface1 = NULL;
        }

        /* Create new offscreen surface - FUN_00412af0 */
        /* In real implementation, this calls DirectDraw CreateSurface */
        /* g_bg_surface1 = graphics_create_offscreen(width, height, 0x800); */
    }

    if (g_bg_surface1) {
        /* Blit sprite to background surface - FUN_0047cb80 */
        /* sprite_blit_to_surface(data_offset, g_bg_surface1, width, height); */

        /* Initialize secondary background layers if not done */
        if (!g_bg_surface2) {
            /* Use LCG random for sprite selection */
            random_val = sprite_lcg_random();
            sprite_lookup_offset((random_val & 3) + 0x9e34, &data_offset);
            sprite_lookup_dimension(data_offset, &bg_width, &bg_height);

            /* Create surface and blit */
            /* g_bg_surface2 = graphics_create_offscreen(640, 480, 0, 0x800); */
        }

        if (!g_bg_surface3) {
            random_val = sprite_lcg_random();
            sprite_lookup_offset((random_val & 3) + 0x9e34, &data_offset);
            sprite_lookup_dimension(data_offset, &bg_width, &bg_height);

            /* Create surface and blit */
            /* g_bg_surface3 = graphics_create_offscreen(640, 480, 0, 0x800); */
        }
    }
}

/*
 * Shutdown background surfaces
 */
void sprite_shutdown_background(void) {
    if (g_bg_surface1) {
        IDirectDrawSurface_Release(g_bg_surface1);
        g_bg_surface1 = NULL;
    }
    if (g_bg_surface2) {
        IDirectDrawSurface_Release(g_bg_surface2);
        g_bg_surface2 = NULL;
    }
    if (g_bg_surface3) {
        IDirectDrawSurface_Release(g_bg_surface3);
        g_bg_surface3 = NULL;
    }

    g_bg_width = 0;
    g_bg_height = 0;
    g_bg_sprite_id = 0;
}

/*
 * Get background surface
 */
IDirectDrawSurface* sprite_get_background_surface(int layer) {
    switch (layer) {
        case 0: return g_bg_surface1;
        case 1: return g_bg_surface2;
        case 2: return g_bg_surface3;
        default: return NULL;
    }
}

/*
 * Get background dimensions
 */
void sprite_get_background_dimension(u16* width, u16* height) {
    if (width) *width = g_bg_width;
    if (height) *height = g_bg_height;
}

/*
 * Get background offset
 */
void sprite_get_background_offset(s32* x, s32* y) {
    if (x) *x = g_bg_offset_x;
    if (y) *y = g_bg_offset_y;
}

/*
 * Set background offset
 */
void sprite_set_background_offset(s32 x, s32 y) {
    g_bg_offset_x = x;
    g_bg_offset_y = y;
}
