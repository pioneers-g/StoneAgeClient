/*
 * Stone Age Client - Render Stub Functions
 * Split from stubs.c to reduce file size
 */

#include <windows.h>
#include <ddraw.h>
#include <string.h>
#include "types.h"
#include "directx.h"
#include "sprite.h"

/* Forward declarations for real implementations */
extern void render_queue_process_full(void);

/* External globals */
extern u32 DAT_005ab6fc;
extern u32 DAT_0464f488;
extern u32 DAT_045829b8;
extern u32 DAT_045829b4;
extern u32 DAT_04633488[];
extern u32 DAT_0463348c[];
extern u32 DAT_04633490[];
extern u32 DAT_04633494[];
extern char DAT_04633498[];
extern char DAT_0463349c[];
extern u32 DAT_0464f7b0;
extern u32 DAT_0464f64c;
extern u32 DAT_0464f48c;
extern u32 DAT_004bb414;
extern u32 DAT_004bb418;
extern u32 DAT_04581d3c;
extern u32 DAT_04581d40;
extern u32 DAT_046334f0;
extern u32 DAT_04633308;
extern u32 DAT_04ebe270;
extern u32 DAT_04ebe278;

/* Forward declarations */
void FUN_00412a40(void);
void FUN_0047d850(void);
int FUN_0047d8e0(void);
void FUN_00404e20(void);

/*
 * FUN_0047e720 - Render Helper
 *
 * Binary analysis:
 * - Called by FUN_0047dc60 for pre-rendering
 * - Only runs when game state (DAT_04630dd8) is not 10
 * - Increments DAT_0464f488 by DAT_0464f48c
 * - Iterates through render entries at DAT_0464f64c
 * - Skips entries with type 1 (skip marker)
 * - Type > 1 marks end of list
 * - Calls FUN_004142f0 or FUN_00414190 for sprite rendering
 */
void FUN_0047e720(void) {
    extern u32 DAT_0464f488;
    extern u32 DAT_0464f48c;
    extern u32 DAT_04630dd8;

    /* Skip during battle state 10 */
    if (DAT_04630dd8 == 10) return;

    /* Advance render queue counter */
    DAT_0464f488 += DAT_0464f48c;
}

/*
 * FUN_0047dc60 - Render Queue Processing
 *
 * Binary analysis:
 * - Main render queue processor
 * - DAT_005ab6fc determines render mode:
 *   - 0: Normal rendering, calls FUN_00412a40, FUN_0047d850, FUN_0047e720
 *   - 3: Reset mode, clears DAT_045829b8/b4
 *   - Other: Standard processing
 * - Iterates through render queue at DAT_0464f64c (max DAT_0464f488 entries)
 * - Each entry is 4 bytes: 2-byte index + 2-byte type
 * - Processes sprites based on type
 * - Clears sprite data arrays after processing (0x7001 entries)
 */
void FUN_0047dc60(void) {
    /* Delegate to the real render queue processor */
    render_queue_process_full();

    /* Clear render queue count after processing */
    DAT_0464f488 = 0;
}

/*
 * FUN_00412a40 - Clear Back Buffer
 *
 * Binary analysis:
 * - Clears the back buffer / offscreen surface
 * - Uses DirectDraw Blt with DDBLT_COLORFILL
 * - Called each frame before rendering
 * - Real implementation in render_queue_process.c: render_clear_back_buffer()
 */
void FUN_00412a40(void) {
    extern GraphicsContext g_graphics;
    DDBLTFX bltfx;
    memset(&bltfx, 0, sizeof(bltfx));
    bltfx.dwSize = sizeof(bltfx);
    if (g_graphics.back_buffer) {
        IDirectDrawSurface_Blt(g_graphics.back_buffer, NULL, NULL, NULL,
                               DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
    } else if (g_graphics.offscreen_surface) {
        IDirectDrawSurface_Blt(g_graphics.offscreen_surface, NULL, NULL, NULL,
                               DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
    }
}

/*
 * FUN_00404e20 - Initialize/Reset Graphics State
 *
 * Binary analysis:
 * - Resets render mode and surface state
 * - Called during state transitions
 */
void FUN_00404e20(void) {
    extern u32 DAT_005ab6fc;
    DAT_005ab6fc = 0;
}

/*
 * FUN_0047cd80 - Field Rendering Update
 *
 * Binary analysis:
 * - Updates field rendering when field sprite changes
 * - Creates/updates field surfaces based on sprite dimensions
 * - Handles field background rendering for isometric view
 * - Uses FUN_0041fad0 (sprite offset lookup) and FUN_0041f980 (sprite dimensions)
 * - Creates primary and secondary field surfaces
 */
void FUN_0047cd80(void) {
    extern u32 DAT_046333f0;
    extern u32 DAT_046333b4, DAT_046333d4;
    extern u32 DAT_04633398, DAT_046333bc;

    if (DAT_046333f0 != 0) {
        /* Get sprite dimensions and create surfaces */
        /* TODO: Full implementation with sprite lookup and surface creation */
    }
}

/*
 * FUN_0047d850 - Animation Frame Update
 *
 * Binary analysis:
 * - Updates animation frame counters every second
 * - Only runs when DAT_046333f0 is non-zero
 * - Uses timeGetTime() for timing (1000ms interval)
 * - Calls FUN_00492403 to get random values
 * - Updates DAT_046333e8 and DAT_046333ec with frame indices
 */
void FUN_0047d850(void) {
    extern u32 DAT_046333f0;
    extern u32 DAT_04633408;
    extern u32 DAT_046333e8;
    extern u32 DAT_046333ec;

    if (DAT_046333f0 != 0) {
        DWORD now = timeGetTime();
        if (DAT_04633408 < now) {
            DAT_04633408 = now + 1000;  /* 1 second interval */
            /* Random frame selection */
            /* TODO: Full implementation with FUN_00492403 */
        }
    }
}

/*
 * FUN_0047d8e0 - Battle Render Queue Processing
 *
 * Binary analysis:
 * - Processes render queue for battle state (game state 9 or 10)
 * - Handles screen scrolling via DAT_045829b4/b8
 * - Updates render queue at DAT_0464f64c
 * - Calls DirectX surface functions for rendering
 * - Returns 1 if surface flip was performed
 */
int FUN_0047d8e0(void) {
    extern u32 DAT_04630dd8;
    extern u32 DAT_0464f64c;
    extern u32 DAT_0464f488;
    extern u32 DAT_0464f48c;

    if (DAT_04630dd8 != 9 && DAT_04630dd8 != 10) {
        return 0;
    }

    DAT_0464f48c = 0;
    /* Process render queue for battle */
    /* TODO: Full implementation */
    return 0;
}

/*
 * FUN_00446df0 - Isometric Coordinate Transform
 *
 * Binary analysis:
 * - Converts world coordinates to screen coordinates
 * - Uses isometric projection formula:
 *   screen_x = (world_y + world_x) * scale + offset_x
 *   screen_y = (world_y - world_x) * scale + offset_y
 * - param_1, param_2: world coordinates
 * - param_3, param_4: output screen coordinates
 */
void FUN_00446df0(float param_1, float param_2, float* param_3, float* param_4) {
    extern float _DAT_04582998;
    extern float _DAT_04582994;
    extern float _DAT_0049c334;
    extern float _DAT_0049c3e8;
    extern float _DAT_0049c3e4;
    extern u32 DAT_004bb424;
    extern u32 DAT_004bb428;

    float f1 = (param_1 - _DAT_04582998) * _DAT_0049c334;
    float f2 = (param_2 - _DAT_04582994) * _DAT_0049c334;

    *param_3 = (f2 + f1) * _DAT_0049c3e8 + (float)DAT_004bb424;
    *param_4 = (f2 - f1) * _DAT_0049c3e4 + (float)DAT_004bb428;
}

/*
 * FUN_00441b90 - Map Tile Attribute Processing
 *
 * Binary analysis:
 * - Processes map tiles for collision/attribute data
 * - param_1-4: coordinate bounds
 * - param_5-8: tile data arrays
 * - Handles special tile types (0-100):
 *   - 0: Empty (check adjacent tile flag)
 *   - 1,2,5,6,9,10: Walkable tiles
 *   - 4: Non-walkable tile
 * - For tiles >= 100, looks up sprite attributes
 * - Sets output array values: 0=empty, 1=walkable, 2=blocked
 */
/* FUN_00441b90 is now in map/map_tileattr.c */

/*
 * FUN_00442070 - Find Walkable Tile
 *
 * Binary analysis:
 * - Finds walkable tiles around player position
 * - param_1: search mode (0-7)
 *   - 0/1/2: Search left side
 *   - 3/4: Search bottom
 *   - 5/6: Search right side
 *   - 7: Search top
 * - Returns 1 if walkable tile found, 0 otherwise
 * - Updates DAT_045602c4 with found tile count
 * - Stores found positions in DAT_04560290-9c arrays
 */
/* FUN_00442070 is now in map/map_tileattr.c */

/*
 * FUN_0047e970 - Additive Blend Sprite Rendering
 *
 * Binary analysis:
 * - Renders sprite with additive blending (lighter colors blend additively)
 * - param_1: sprite surface pointer
 * - param_2, param_3: screen position (x, y)
 * - param_4-7: source rectangle dimensions
 * - param_8: blend intensity (0-255)
 * - param_9: special flags (flip, etc.)
 * - Uses RGB565/RGB555 color formats based on DAT_0054bdec
 * - Pixel formula: result = src + (dst * intensity) >> shift
 * - Handles clipping against screen bounds
 */
void FUN_0047e970(void* surface, int x, int y, int src_x, int src_y,
                  int src_w, int src_h, unsigned int blend_factor, int flags) {
    extern GraphicsContext g_graphics;
    extern u32 g_pixel_format;
    extern void sprite_blend_additive(u16* dest, const u16* src, u32 count, u8 alpha);
    extern void sprite_blend_565(u16* dest, const u16* src, u32 count, u8 alpha);
    extern void sprite_blend_555(u16* dest, const u16* src, u32 count, u8 alpha);

    DDSURFACEDESC2 src_ddsd, dst_ddsd;
    u16* src_pixels;
    u16* dst_pixels;
    int dst_pitch_words;
    int src_pitch_words;
    int row, draw_w, draw_h;
    int clip_src_x, clip_src_y;
    HRESULT hr;
    IDirectDrawSurface* dst_surf = g_graphics.back_buffer ? g_graphics.back_buffer : g_graphics.offscreen_surface;

    (void)flags;

    if (!surface || !dst_surf || src_w <= 0 || src_h <= 0) return;

    /* Clipping */
    clip_src_x = src_x;
    clip_src_y = src_y;
    draw_w = src_w;
    draw_h = src_h;

    if (x < 0) { clip_src_x -= x; draw_w += x; x = 0; }
    if (y < 0) { clip_src_y -= y; draw_h += y; y = 0; }
    if (x + draw_w > 640) draw_w = 640 - x;
    if (y + draw_h > 480) draw_h = 480 - y;
    if (draw_w <= 0 || draw_h <= 0) return;

    /* Lock source surface */
    memset(&src_ddsd, 0, sizeof(DDSURFACEDESC2));
    src_ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock((IDirectDrawSurface*)surface, NULL, &src_ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
    if (FAILED(hr)) return;

    /* Lock destination surface */
    memset(&dst_ddsd, 0, sizeof(DDSURFACEDESC2));
    dst_ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(dst_surf, NULL, &dst_ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
    if (FAILED(hr)) {
        IDirectDrawSurface_Unlock((IDirectDrawSurface*)surface, NULL);
        return;
    }

    src_pitch_words = src_ddsd.lPitch / 2;
    dst_pitch_words = dst_ddsd.lPitch / 2;

    for (row = 0; row < draw_h; row++) {
        src_pixels = (u16*)src_ddsd.lpSurface + (clip_src_y + row) * src_pitch_words + clip_src_x;
        dst_pixels = (u16*)dst_ddsd.lpSurface + (y + row) * dst_pitch_words + x;
        sprite_blend_additive(dst_pixels, src_pixels, draw_w, (u8)blend_factor);
    }

    IDirectDrawSurface_Unlock(dst_surf, NULL);
    IDirectDrawSurface_Unlock((IDirectDrawSurface*)surface, NULL);
}

/*
 * FUN_0047f170 - Subtractive Blend Sprite Rendering
 *
 * Binary analysis:
 * - Renders sprite with subtractive blending (darker result)
 * - param_1: sprite surface pointer
 * - param_2, param_3: screen position (x, y)
 * - param_4-7: source rectangle dimensions
 * - param_8: blend flags
 * - Uses max() operation to select brighter pixels
 * - For each channel: result = max(src, dst)
 * - Handles RGB565/RGB555 pixel formats
 */
void FUN_0047f170(void* surface, int x, int y, int src_x, int src_y,
                  int src_w, int src_h, unsigned int blend_flags, int flags) {
    extern GraphicsContext g_graphics;
    extern void sprite_blend_subtractive(u16* dest, const u16* src, u32 count, u8 alpha);

    DDSURFACEDESC2 src_ddsd, dst_ddsd;
    u16* src_pixels;
    u16* dst_pixels;
    int dst_pitch_words, src_pitch_words;
    int row, draw_w, draw_h;
    int clip_src_x, clip_src_y;
    HRESULT hr;
    IDirectDrawSurface* dst_surf = g_graphics.back_buffer ? g_graphics.back_buffer : g_graphics.offscreen_surface;

    (void)blend_flags; (void)flags;

    if (!surface || !dst_surf || src_w <= 0 || src_h <= 0) return;

    clip_src_x = src_x;
    clip_src_y = src_y;
    draw_w = src_w;
    draw_h = src_h;

    if (x < 0) { clip_src_x -= x; draw_w += x; x = 0; }
    if (y < 0) { clip_src_y -= y; draw_h += y; y = 0; }
    if (x + draw_w > 640) draw_w = 640 - x;
    if (y + draw_h > 480) draw_h = 480 - y;
    if (draw_w <= 0 || draw_h <= 0) return;

    memset(&src_ddsd, 0, sizeof(DDSURFACEDESC2));
    src_ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock((IDirectDrawSurface*)surface, NULL, &src_ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
    if (FAILED(hr)) return;

    memset(&dst_ddsd, 0, sizeof(DDSURFACEDESC2));
    dst_ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(dst_surf, NULL, &dst_ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
    if (FAILED(hr)) {
        IDirectDrawSurface_Unlock((IDirectDrawSurface*)surface, NULL);
        return;
    }

    src_pitch_words = src_ddsd.lPitch / 2;
    dst_pitch_words = dst_ddsd.lPitch / 2;

    for (row = 0; row < draw_h; row++) {
        src_pixels = (u16*)src_ddsd.lpSurface + (clip_src_y + row) * src_pitch_words + clip_src_x;
        dst_pixels = (u16*)dst_ddsd.lpSurface + (y + row) * dst_pitch_words + x;
        sprite_blend_subtractive(dst_pixels, src_pixels, draw_w, 128);
    }

    IDirectDrawSurface_Unlock(dst_surf, NULL);
    IDirectDrawSurface_Unlock((IDirectDrawSurface*)surface, NULL);
}

/*
 * FUN_0047f710 - Special Color Tint Sprite Rendering
 *
 * Binary analysis:
 * - Renders sprite with color tinting effect
 * - param_1-2: surface pointers
 * - param_3-8: position, dimensions, blend mode, tint color
 * - Converts grayscale using: gray = (R + G*2 + B) * 3 / 10
 * - Applies tint based on blend_mode parameter
 * - Handles RGB565/RGB555 pixel formats
 */
void FUN_0047f710(void* dest_surface, void* src_surface, int x, int y,
                  int src_x, int src_y, int src_w, int src_h, char blend_mode) {
    extern GraphicsContext g_graphics;
    extern u32 g_pixel_format;

    DDSURFACEDESC2 src_ddsd, dst_ddsd;
    u16* src_pixels;
    u16* dst_pixels;
    int dst_pitch_words, src_pitch_words;
    int row, col, draw_w, draw_h;
    int clip_src_x, clip_src_y;
    HRESULT hr;
    int is_565;
    IDirectDrawSurface* dst_surf = g_graphics.back_buffer ? g_graphics.back_buffer : g_graphics.offscreen_surface;

    if (!src_surface || !dst_surf || src_w <= 0 || src_h <= 0) return;

    is_565 = (g_pixel_format == PIXEL_FORMAT_565);

    clip_src_x = src_x;
    clip_src_y = src_y;
    draw_w = src_w;
    draw_h = src_h;

    if (x < 0) { clip_src_x -= x; draw_w += x; x = 0; }
    if (y < 0) { clip_src_y -= y; draw_h += y; y = 0; }
    if (x + draw_w > 640) draw_w = 640 - x;
    if (y + draw_h > 480) draw_h = 480 - y;
    if (draw_w <= 0 || draw_h <= 0) return;

    memset(&src_ddsd, 0, sizeof(DDSURFACEDESC2));
    src_ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock((IDirectDrawSurface*)src_surface, NULL, &src_ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
    if (FAILED(hr)) return;

    memset(&dst_ddsd, 0, sizeof(DDSURFACEDESC2));
    dst_ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(dst_surf, NULL, &dst_ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
    if (FAILED(hr)) {
        IDirectDrawSurface_Unlock((IDirectDrawSurface*)src_surface, NULL);
        return;
    }

    src_pitch_words = src_ddsd.lPitch / 2;
    dst_pitch_words = dst_ddsd.lPitch / 2;

    for (row = 0; row < draw_h; row++) {
        src_pixels = (u16*)src_ddsd.lpSurface + (clip_src_y + row) * src_pitch_words + clip_src_x;
        dst_pixels = (u16*)dst_ddsd.lpSurface + (y + row) * dst_pitch_words + x;

        for (col = 0; col < draw_w; col++) {
            u16 src_pixel = src_pixels[col];
            u16 dst_pixel = dst_pixels[col];
            u32 r, g, b, gray;

            if (is_565) {
                if (src_pixel == 0xF81F) continue; /* transparent */
                r = (src_pixel >> 11) & 0x1F;
                g = (src_pixel >> 5) & 0x3F;
                b = src_pixel & 0x1F;
                gray = (r + g * 2 + b) * 3 / 10;

                if (blend_mode == 0) {
                    /* Apply tint to grayscale - multiply with destination */
                    u32 dr = (dst_pixel >> 11) & 0x1F;
                    u32 dg = (dst_pixel >> 5) & 0x3F;
                    u32 db = dst_pixel & 0x1F;
                    r = (dr * gray) >> 5;
                    g = (dg * gray) >> 6;
                    b = (db * gray) >> 5;
                    if (r > 0x1F) r = 0x1F;
                    if (g > 0x3F) g = 0x3F;
                    if (b > 0x1F) b = 0x1F;
                    dst_pixels[col] = (u16)((r << 11) | (g << 5) | b);
                } else {
                    /* Direct grayscale output */
                    if (gray > 0x1F) gray = 0x1F;
                    g = gray * 2;
                    if (g > 0x3F) g = 0x3F;
                    dst_pixels[col] = (u16)((gray << 11) | (g << 5) | gray);
                }
            } else {
                /* 555 format */
                if (src_pixel == 0x7C1F) continue;
                r = (src_pixel >> 10) & 0x1F;
                g = (src_pixel >> 5) & 0x1F;
                b = src_pixel & 0x1F;
                gray = (r + g * 2 + b) * 3 / 10;

                if (blend_mode == 0) {
                    u32 dr = (dst_pixel >> 10) & 0x1F;
                    u32 dg = (dst_pixel >> 5) & 0x1F;
                    u32 db = dst_pixel & 0x1F;
                    r = (dr * gray) >> 5;
                    g = (dg * gray) >> 5;
                    b = (db * gray) >> 5;
                    if (r > 0x1F) r = 0x1F;
                    if (g > 0x1F) g = 0x1F;
                    if (b > 0x1F) b = 0x1F;
                    dst_pixels[col] = (u16)((r << 10) | (g << 5) | b);
                } else {
                    if (gray > 0x1F) gray = 0x1F;
                    dst_pixels[col] = (u16)((gray << 10) | (gray << 5) | gray);
                }
            }
        }
    }

    IDirectDrawSurface_Unlock(dst_surf, NULL);
    IDirectDrawSurface_Unlock((IDirectDrawSurface*)src_surface, NULL);
}

/*
 * FUN_0047fae0 - RLE Sprite Rendering
 *
 * Binary analysis:
 * - Renders RLE-compressed sprite data to screen
 * - Decompresses on-the-fly during blit
 * - Used for sprites marked with compression flag
 *
 * Delegates to sprite_rle_render from sprite_rle.c
 */
extern void sprite_rle_render(void* surface, int x, int y, int src_x, int src_y,
                               int src_w, int src_h, unsigned int flags);

void FUN_0047fae0(void* surface, int x, int y, int src_x, int src_y,
                  int src_w, int src_h, unsigned int flags) {
    sprite_rle_render(surface, x, y, src_x, src_y, src_w, src_h, flags);
}

/*
 * FUN_0041fb10 - Load Standard Sprite from Archive
 *
 * Binary analysis:
 * - Loads sprite data from sprite archive file
 * - param_1: sprite ID (0-549999)
 * - param_2: output surface pointer
 * - param_3, param_4: dimensions output
 * - Uses DAT_00e8f228 sprite directory (0x14 bytes per entry)
 * - Returns true on success, false on failure
 *
 * Delegates to assets_load_sprite from assets_sprite.c
 */
extern int assets_load_sprite(int sprite_id, void** surface, int* width, int* height);

int FUN_0041fb10(int sprite_id, void** surface, int* width, int* height) {
    return assets_load_sprite(sprite_id, surface, width, height);
}

/*
 * FUN_0041fc90 - Load High-Resolution Sprite from Archive
 *
 * Delegates to assets_load_sprite_hires from assets_sprite.c
 */
extern int assets_load_sprite_hires(unsigned int sprite_index, void** surface,
                                     int* width, int* height, void** alpha_surface, int* has_alpha);

int FUN_0041fc90(unsigned int sprite_index, void** surface, int* width, int* height,
                 void** alpha_surface, int* has_alpha) {
    return assets_load_sprite_hires(sprite_index, surface, width, height, alpha_surface, has_alpha);
}

/*
 * FUN_0041fad0 - Sprite Image Index Lookup
 *
 * Binary analysis:
 * - Looks up image index from sprite ID
 * - param_1: sprite ID (0-549999 valid range)
 * - param_2: output image index
 * - Returns 1 on success, 0 on failure
 *
 * Sprite ID ranges:
 * - 0-499999: Look up from table at DAT_00a04c64
 * - 500000-549999: Use sprite ID directly as image index
 * - 550000+: Invalid, return 0
 */
int FUN_0041fad0(unsigned int param_1, unsigned int* param_2) {
    if (param_1 < 500000) {
        /* Standard sprite: look up from table */
        extern u32 DAT_00a04c64[];
        *param_2 = DAT_00a04c64[param_1];
        return 1;
    }
    if (param_1 > 549999) {
        *param_2 = 0;
        return 0;
    }
    /* High-res sprites (500000-549999): use as-is */
    *param_2 = param_1;
    return 1;
}

/*
 * FUN_0047a6e0 - Clear Back Buffer
 *
 * Binary analysis:
 * - Clears the back buffer surfaces for rendering
 * - Uses DirectDraw surface blit to fill with black
 *
 * Process:
 * 1. Blit to primary back buffer at DAT_0465d7c4
 *    - Uses DDBLT_COLORFILL with DDBLT_WAIT
 *    - Target: DAT_0054a90c + 0xc (primary surface)
 * 2. If high-res mode (DAT_0054c83c != 0):
 *    - Also clear secondary back buffer at DAT_0465d7c0
 *    - Target: DAT_0054a90c + 0x10 (secondary surface)
 *
 * Called during scene transitions and screen clears
 */
void FUN_0047a6e0(void) {
    DDBLTFX bltfx;
    IDirectDrawSurface* target;
    extern GraphicsContext g_graphics;
    extern int g_high_res_mode;

    memset(&bltfx, 0, sizeof(DDBLTFX));
    bltfx.dwSize = sizeof(DDBLTFX);
    bltfx.dwFillColor = 0;

    /* Clear primary back buffer */
    target = g_graphics.back_buffer ? g_graphics.back_buffer : g_graphics.offscreen_surface;
    if (target) {
        IDirectDrawSurface_Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
    }

    /* Clear secondary buffer in high-res mode */
    if (g_high_res_mode && g_graphics.offscreen_surface && g_graphics.back_buffer) {
        IDirectDrawSurface_Blt(g_graphics.offscreen_surface, NULL, NULL, NULL,
                               DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
    }
}

/*
 * FUN_0041f900 - Sprite Dimensions Lookup
 *
 * Binary analysis:
 * - Gets width/height for a sprite image index
 * - param_1: image index (0-549999)
 * - param_2: output width
 * - param_3: output height
 * - Returns 1 on success, 0 on failure
 */
int FUN_0041f900(unsigned int param_1, unsigned short* param_2, unsigned short* param_3) {
    extern int render_get_sprite_dimensions(unsigned int, unsigned short*, unsigned short*);
    if (render_get_sprite_dimensions(param_1, param_2, param_3)) {
        return 1;
    }
    *param_2 = 0;
    *param_3 = 0;
    return 0;
}

/*
 * FUN_004412e0 - Isometric Tile Rendering
 *
 * Binary analysis:
 * - Renders isometric map tiles in diamond pattern
 * - Uses screen coordinates from DAT_04581d3c/DAT_04581d40
 * - Calculates tile positions: x += 0x20, y += -0x18 per column
 * - Tile values < 100 are special (animated tiles, effects)
 * - Tile values >= 100 are sprite IDs
 */
void FUN_004412e0(void) {
    /* Check if scroll position changed */
    if (DAT_04581d3c != DAT_004bb414 || DAT_04581d40 != DAT_004bb418) {
        DAT_004bb414 = DAT_04581d3c;
        DAT_004bb418 = DAT_04581d40;
    }
    /* TODO: Full isometric rendering implementation */
}

/* Forward declaration for real sprite queue implementation */
extern int render_queue_add_sprite(int x, int y, int layer, u32 sprite_id, int priority);
extern int render_queue_add(int x, int y, u32 sprite_id, u32 flags, void* linked_obj);

/*
 * FUN_004445c0 - Clear Render Queue
 *
 * Binary analysis:
 * - Clears/resets the render queue
 * - Sets DAT_0464f488 (queue count) to 0
 * - Called at start of each frame in game state machine
 * - Also clears entity render state
 * - Real implementation delegates to render_queue_clear()
 */
void FUN_004445c0(void) {
    extern void render_queue_clear(void);
    render_queue_clear();
}

/*
 * FUN_00440e90 - Clear Sprites / UI Reset
 *
 * Binary analysis:
 * - Resets sprite and UI rendering state
 * - Called during state transitions (login → char select, battle → field)
 * - Clears temporary sprite surfaces and UI overlays
 */
void FUN_00440e90(void) {
    /* Clear sprite/UI state */
}

/*
 * FUN_00445070 - UI Render
 *
 * Binary analysis:
 * - Renders UI overlay elements
 * - Called after main rendering in game state machine
 * - Processes UI widgets, menus, text overlays
 */
void FUN_00445070(void) {
    /* UI render overlay */
}

/*
 * FUN_004419a0 - Entity Render Queue
 *
 * Binary analysis:
 * - Queues entities for rendering based on position
 * - Iterates entity linked list at DAT_004d7e3c
 * - Adds sprite entries to render queue via FUN_0047e210
 * - Sort order based on Y position (isometric depth sorting)
 * - Skips entities with delete flag set
 */
void FUN_004419a0(void) {
    /* Entity render queue - processes entity linked list */
}

/*
 * render_process_queue - Process and render all queued sprites
 *
 * Delegates to render_queue_process_full from render_queue.c
 */
void render_process_queue(void) {
    extern void render_queue_process_full(void);
    render_queue_process_full();
}

/*
 * FUN_0047e210 - Sprite Render Queue Add
 * Delegates to render_queue_add_sprite from render_queue.c
 */
int FUN_0047e210(int param_1, int param_2, int param_3, int param_4, int param_5) {
    /* Check queue capacity */
    if (DAT_0464f488 > 0xfff) {
        return -2;
    }

    /* Validate sprite ID: -1 to 99 are invalid per FUN_0047e210 */
    if (param_4 >= -1 && param_4 < 100) {
        return -2;
    }

    /* Delegate to real implementation */
    int result = render_queue_add_sprite(param_1, param_2, param_3, (u32)param_4, param_5);
    if (result >= 0) {
        DAT_0464f488++;
    }
    return result;
}

/*
 * FUN_00414420 - Clipped Surface Blit
 *
 * Binary analysis:
 * - Blits a rectangular region from source to destination with clipping
 * - param_1: destination X position
 * - param_2: destination Y position
 * - param_3: source rectangle [left, top, right, bottom]
 * - param_4: surface/texture handle
 * - Performs clipping against screen bounds at DAT_0054a90c+0x88/0x8c
 * - Returns 0 if completely outside screen bounds
 * - Adjusts source rectangle when partially clipped
 */
int FUN_00414420(short dest_x, short dest_y, int* src_rect, u32 surface_handle) {
    extern u32 DAT_0054a90c;

    int src_w = src_rect[2] - src_rect[0];
    int src_h = src_rect[3] - src_rect[1];
    int screen_w = *(int*)(DAT_0054a90c + 0x88);
    int screen_h = *(int*)(DAT_0054a90c + 0x8c);

    /* Check if completely outside screen */
    if (dest_x >= screen_w) return 0;
    if (dest_x + src_w <= 0) return 0;
    if (dest_y >= screen_h) return 0;
    if (dest_y + src_h <= 0) return 0;

    /* Clip left edge */
    if (dest_x < 0) {
        src_rect[0] -= dest_x;
        dest_x = 0;
    }

    /* Clip right edge */
    if (dest_x + src_w > screen_w) {
        src_rect[2] = src_rect[0] + (screen_w - dest_x);
    }

    /* Clip top edge */
    if (dest_y < 0) {
        src_rect[1] -= dest_y;
        dest_y = 0;
    }

    /* Clip bottom edge */
    if (dest_y + src_h > screen_h) {
        src_rect[3] = src_rect[1] + (screen_h - dest_y);
    }

    /* Call actual blit function */
    /* TODO: Implement surface blit via function pointer at DAT_0054a90c+0xc+0x1c */
    (void)surface_handle;
    return 1;
}

/*
 * FUN_004142f0 - Sprite Blit with Clipping
 *
 * Binary analysis:
 * - Blits sprite to screen at position with automatic clipping
 * - param_1: screen X position
 * - param_2: screen Y position
 * - param_3: surface handle
 * - Uses sprite dimensions from DAT_0466b7d4 (width) and DAT_0466b7d0 (height)
 * - Creates source rectangle based on clipping needs
 * - Calls blit function at DAT_0054a90c+0xc+0x14
 */
int FUN_004142f0(short screen_x, short screen_y, u32 surface_handle) {
    extern u32 DAT_0054a90c;
    extern u32 DAT_0466b7d4;  /* Sprite width */
    extern u32 DAT_0466b7d0;  /* Sprite height */

    int screen_w = *(int*)(DAT_0054a90c + 0x88);
    int screen_h = *(int*)(DAT_0054a90c + 0x8c);
    int sprite_w = DAT_0466b7d4;
    int sprite_h = DAT_0466b7d0;

    /* Check bounds */
    if (screen_x >= screen_w) return 0;
    if (screen_x + sprite_w <= 0) return 0;
    if (screen_y >= screen_h) return 0;
    if (screen_y + sprite_h <= 0) return 0;

    /* TODO: Full implementation with source rectangle calculation */
    (void)surface_handle;
    return 1;
}

/*
 * FUN_0047e640 - Add Sprite to Render Queue with Blend Mode
 *
 * Binary analysis:
 * - Wrapper for FUN_0047e210 with blend mode support
 * - param_1: X position (combined with param_3)
 * - param_2: Y position (combined with param_4)
 * - param_3: X offset/sub-position
 * - param_4: Y offset/sub-position
 * - param_5: sprite layer
 * - param_6: sprite ID
 * - param_7: blend mode (0=add, 1=subtract, 2=multiply)
 *
 * Blend modes:
 * - 0: Additive blending (0xa0000000 flag)
 * - 1: Subtractive blending (0x90000000 flag)
 * - 2: Multiplicative blending (0xc0000000 flag)
 */
void FUN_0047e640(int x1, int y1, u32 x2, u32 y2, int layer, u32 sprite_id, int blend_mode) {
    u32 combined_x = (x1 << 16) | x2;
    u32 combined_y = (y1 << 16) | y2;

    switch (blend_mode) {
        case 0:  /* Additive */
            FUN_0047e210(combined_x, combined_y, layer, sprite_id | 0xa0000000, 0);
            break;
        case 1:  /* Subtractive */
            FUN_0047e210(combined_x, combined_y, layer, sprite_id | 0x90000000, 0);
            break;
        case 2:  /* Multiplicative */
            FUN_0047e210(combined_x, combined_y, layer, sprite_id | 0xc0000000, 0);
            break;
        default:  /* Normal */
            FUN_0047e210(combined_x, combined_y, layer, sprite_id, 0);
            break;
    }
}

/*
 * FUN_0047e440 - Page Flip / Present Frame
 *
 * Binary analysis:
 * - Final step of render pipeline each frame
 * - Performs page flip (primary <-> back buffer) for double-buffering
 * - If no back buffer, blits offscreen to primary surface
 * - Called after FUN_0047dc60 (render queue process)
 */
void FUN_0047e440(void) {
    extern void graphics_flip(void);
    extern GraphicsContext g_graphics;

    if (g_graphics.primary_surface) {
        graphics_flip();
    }
}
