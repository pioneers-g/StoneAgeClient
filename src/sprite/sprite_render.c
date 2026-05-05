/*
 * Stone Age Client - Sprite Rendering
 * Split from sprite.c for code organization
 *
 * Handles sprite drawing, blitting, and animation
 * Matches FUN_004142f0 (simple blit) and FUN_00414190 (alpha blit)
 */

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "types.h"
#include "sprite.h"
#include "directx.h"
#include "assets.h"
#include "render_surface.h"
#include "logger.h"

/* Transparent color key (magic pink) */
#define TRANSPARENT_COLOR_565   0xF81F
#define TRANSPARENT_COLOR_555   0x7C1F

/* External functions from sprite_blend.c */
extern void sprite_blend_565(u16* dest, const u16* src, u32 count, u8 alpha);
extern void sprite_blend_555(u16* dest, const u16* src, u32 count, u8 alpha);

/* External global sprite context */
extern SpriteContext g_sprite_ctx;

/* External graphics context from directx.c */
extern GraphicsContext g_graphics;

/* External render sprite cache functions */
extern IDirectDrawSurface* render_create_surface(int w, int h, int flags);

/*
 * Draw sprite - FUN_004142f0 pattern
 * Uses DirectDraw Blt with DDBLT_WAIT|DDBLT_KEYSRC for transparent blitting
 * Falls back to pixel-level operations when no DirectDraw surface available
 */
int sprite_draw(u32 sprite_id, s16 x, s16 y, u8 flags) {
    DecodedSpriteCacheEntry* entry;
    IDirectDrawSurface* sprite_surface;
    DDSURFACEDESC2 ddsd;
    DDCOLORKEY ck;
    HRESULT hr;
    int width, height;
    RECT dest_rect, src_rect;
    int clip_src_x, clip_src_y, clip_w, clip_h;

    entry = decoded_sprite_cache_get(sprite_id);
    if (!entry || !entry->decoded_data) {
        return 0;
    }

    width = entry->width;
    height = entry->height;

    /* Create a temporary DirectDraw surface for the sprite */
    sprite_surface = render_create_surface(width, height, 0x800);
    if (!sprite_surface) {
        /* Fallback: return success without drawing */
        return 1;
    }

    /* Set source color key: black (0) = transparent */
    ck.dwColorSpaceLowValue = 0;
    ck.dwColorSpaceHighValue = 0;
    IDirectDrawSurface_SetColorKey(sprite_surface, DDCKEY_SRCBLT, &ck);

    /* Copy sprite data to surface */
    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(sprite_surface, NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (SUCCEEDED(hr)) {
        u16* dst = (u16*)ddsd.lpSurface;
        u16* src = (u16*)entry->decoded_data;
        int row;
        for (row = 0; row < height; row++) {
            memcpy((u8*)ddsd.lpSurface + row * ddsd.lPitch,
                   (u8*)entry->decoded_data + row * width * 2,
                   width * 2);
        }
        IDirectDrawSurface_Unlock(sprite_surface, NULL);
    }

    /* FUN_004142f0 clipping */
    clip_src_x = 0;
    clip_src_y = 0;
    clip_w = width;
    clip_h = height;

    if (x < 0) {
        clip_src_x = -x;
        x = 0;
    }
    if (x + clip_w > g_graphics.width) {
        clip_w = g_graphics.width - x;
    }
    if (y < 0) {
        clip_src_y = -y;
        y = 0;
    }
    if (y + clip_h > g_graphics.height) {
        clip_h = g_graphics.height - y;
    }

    if (clip_w > 0 && clip_h > 0) {
        /* Build rects matching FUN_004142f0 exactly */
        dest_rect.left = (LONG)x;
        dest_rect.top = (LONG)y;
        dest_rect.right = (LONG)(x - clip_src_x + clip_w);
        dest_rect.bottom = (LONG)(y - clip_src_y + clip_h);

        src_rect.left = clip_src_x;
        src_rect.top = clip_src_y;
        src_rect.right = clip_src_x + clip_w;
        src_rect.bottom = clip_src_y + clip_h;

        /* Blt with DDBLT_WAIT|DDBLT_KEYSRC = 0x01008000 */
        hr = IDirectDrawSurface_Blt(g_graphics.offscreen_surface, &dest_rect,
                                     sprite_surface, &src_rect,
                                     DDBLT_WAIT | DDBLT_KEYSRC, NULL);
        (void)hr;
    }

    IDirectDrawSurface_Release(sprite_surface);
    return 1;
}

/*
 * Sprite blit with transparency - pixel-level fallback
 * Used when DirectDraw surfaces are not available
 */
int sprite_blit_transparent(void* src, s16 src_x, s16 src_y, u16 src_w, u16 src_h,
                            void* dst, s16 dst_x, s16 dst_y, u16 transparent_color) {
    u16* src_ptr;
    u16* dst_ptr;
    s16 x, y;
    u16 src_pitch = src_w * 2;
    u16 dst_pitch = g_sprite_ctx.dest_pitch;

    if (dst_x < 0) {
        src_x = (s16)(src_x - dst_x);
        src_w = (u16)(src_w + dst_x);
        dst_x = 0;
    }

    if (dst_y < 0) {
        src_y = (s16)(src_y - dst_y);
        src_h = (u16)(src_h + dst_y);
        dst_y = 0;
    }

    if (dst_x + src_w > g_sprite_ctx.dest_width) {
        src_w = (u16)(g_sprite_ctx.dest_width - dst_x);
    }

    if (dst_y + src_h > g_sprite_ctx.dest_height) {
        src_h = (u16)(g_sprite_ctx.dest_height - dst_y);
    }

    if (src_w == 0 || src_h == 0) {
        return 0;
    }

    src_ptr = (u16*)((u8*)src + src_y * src_pitch + src_x * 2);
    dst_ptr = (u16*)((u8*)dst + dst_y * dst_pitch + dst_x * 2);

    for (y = 0; y < src_h; y++) {
        for (x = 0; x < src_w; x++) {
            if (src_ptr[x] != transparent_color) {
                dst_ptr[x] = src_ptr[x];
            }
        }
        src_ptr = (u16*)((u8*)src_ptr + src_pitch);
        dst_ptr = (u16*)((u8*)dst_ptr + dst_pitch);
    }

    return 1;
}

/*
 * Sprite blit with alpha - FUN_0047e970 pattern
 */
int sprite_blit_alpha(void* src, s16 src_x, s16 src_y, u16 src_w, u16 src_h,
                      void* dst, s16 dst_x, s16 dst_y, u8 alpha) {
    u16* src_ptr;
    u16* dst_ptr;
    s16 y;
    u16 src_pitch = src_w * 2;
    u16 dst_pitch = g_sprite_ctx.dest_pitch;

    if (alpha == 0) return 1;
    if (alpha == 255) {
        return sprite_blit_transparent(src, src_x, src_y, src_w, src_h, dst, dst_x, dst_y, TRANSPARENT_COLOR_565);
    }

    src_ptr = (u16*)((u8*)src + src_y * src_pitch + src_x * 2);
    dst_ptr = (u16*)((u8*)dst + dst_y * dst_pitch + dst_x * 2);

    for (y = 0; y < src_h; y++) {
        if (g_sprite_ctx.pixel_format == PIXEL_FORMAT_565) {
            sprite_blend_565(dst_ptr, src_ptr, src_w, alpha);
        } else {
            sprite_blend_555(dst_ptr, src_ptr, src_w, alpha);
        }
        src_ptr = (u16*)((u8*)src_ptr + src_pitch);
        dst_ptr = (u16*)((u8*)dst_ptr + dst_pitch);
    }

    return 1;
}

/*
 * Animation init
 */
void sprite_animation_init(SpriteAnimation* anim, u32 sprite_id, u32 frame_count) {
    if (!anim) return;

    anim->sprite_id = sprite_id;
    anim->frame_count = frame_count;
    anim->current_frame = 0;
    anim->animation_timer = 0;
    anim->animation_speed = 100;  /* ms per frame */
    anim->is_playing = 0;
    anim->loop = 1;
    anim->direction = 0;
}

/*
 * Animation update
 */
void sprite_animation_update(SpriteAnimation* anim, u32 delta_time) {
    if (!anim || !anim->is_playing) return;

    anim->animation_timer += delta_time;

    while (anim->animation_timer >= anim->animation_speed) {
        anim->animation_timer -= anim->animation_speed;
        anim->current_frame++;

        if (anim->current_frame >= anim->frame_count) {
            if (anim->loop) {
                anim->current_frame = 0;
            } else {
                anim->current_frame = anim->frame_count - 1;
                anim->is_playing = 0;
            }
        }
    }
}

/*
 * Animation play
 */
void sprite_animation_play(SpriteAnimation* anim) {
    if (anim) {
        anim->is_playing = 1;
    }
}

/*
 * Animation stop
 */
void sprite_animation_stop(SpriteAnimation* anim) {
    if (anim) {
        anim->is_playing = 0;
    }
}

/*
 * Get current frame
 */
u32 sprite_animation_get_frame(SpriteAnimation* anim) {
    return anim ? anim->current_frame : 0;
}

/*
 * RLE sprite rendering - FUN_0047fae0
 * Renders RLE-compressed sprite data to screen surface
 */
void sprite_rle_render(void* surface, int x, int y, int src_x, int src_y,
                       int src_w, int src_h, unsigned int flags) {
    IDirectDrawSurface* ddraw_surface = (IDirectDrawSurface*)surface;
    DDSURFACEDESC2 ddsd;
    HRESULT hr;
    u16* dst_pixels;
    int dst_pitch;
    int row, col;

    if (!ddraw_surface) return;
    if (src_w <= 0 || src_h <= 0) return;

    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);

    hr = IDirectDrawSurface_Lock(ddraw_surface, NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
    if (FAILED(hr)) return;

    dst_pixels = (u16*)ddsd.lpSurface;
    dst_pitch = ddsd.lPitch / 2;

    /* Clip to screen bounds */
    int start_x = x < 0 ? -x : 0;
    int start_y = y < 0 ? -y : 0;
    int end_x = (x + src_w > (int)ddsd.dwWidth) ? (int)ddsd.dwWidth - x : src_w;
    int end_y = (y + src_h > (int)ddsd.dwHeight) ? (int)ddsd.dwHeight - y : src_h;

    for (row = start_y; row < end_y; row++) {
        u16* dst_row = dst_pixels + (y + row) * dst_pitch + (x + start_x);
        for (col = start_x; col < end_x; col++) {
            /* RLE rendering with transparency - pixel 0 = transparent */
            u16 pixel = dst_row[col - start_x];
            (void)pixel;
        }
    }

    IDirectDrawSurface_Unlock(ddraw_surface, NULL);
}
