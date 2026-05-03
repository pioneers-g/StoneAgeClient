/*
 * Stone Age Client - Surface Operations
 * Reverse engineered from sa_9061.exe
 * FUN_00412af0, FUN_004142f0, FUN_00414420, FUN_00414190
 *
 * This module handles:
 * - Surface creation and management
 * - Blit operations (normal, scaled, transparent)
 * - Surface locking for pixel access
 */

#include <windows.h>
#include <ddraw.h>
#include <string.h>
#include "types.h"
#include "render.h"
#include "directx.h"
#include "logger.h"

/* Surface creation flags from original binary */
#define SURFACE_FLAG_SYSTEM_MEM  0x800
#define SURFACE_FLAG_VIDEO_MEM   0x4000

/*
 * Create offscreen surface - FUN_00412af0
 */
IDirectDrawSurface* render_create_surface(int width, int height, int flags) {
    DDSURFACEDESC2 ddsd;
    IDirectDrawSurface* surface;
    HRESULT hr;

    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    ddsd.dwWidth = width;
    ddsd.dwHeight = height;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

    /* Flag handling from original */
    if (flags & SURFACE_FLAG_SYSTEM_MEM) {
        ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
    }
    if (flags & SURFACE_FLAG_VIDEO_MEM) {
        ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    }

    hr = IDirectDraw4_CreateSurface(g_graphics.ddraw4, &ddsd, &surface, NULL);
    if (FAILED(hr)) {
        /* Fallback to system memory */
        ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
        hr = IDirectDraw4_CreateSurface(g_graphics.ddraw4, &ddsd, &surface, NULL);
        if (FAILED(hr)) {
            LOG_WARN("Failed to create surface %dx%d: 0x%08X", width, height, hr);
            return NULL;
        }
    }

    return surface;
}

/*
 * Release surface
 */
void render_release_surface(IDirectDrawSurface* surface) {
    if (surface) {
        IDirectDrawSurface_Release(surface);
    }
}

/*
 * Lock surface for pixel access
 */
int render_lock_surface(IDirectDrawSurface* surface, void** pixels, int* pitch) {
    DDSURFACEDESC2 ddsd;
    HRESULT hr;

    if (!surface) return 0;

    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);

    hr = IDirectDrawSurface_Lock(surface, NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (FAILED(hr)) {
        return 0;
    }

    if (pixels) *pixels = ddsd.lpSurface;
    if (pitch) *pitch = ddsd.lPitch;

    return 1;
}

/*
 * Unlock surface
 */
void render_unlock_surface(IDirectDrawSurface* surface) {
    if (surface) {
        IDirectDrawSurface_Unlock(surface, NULL);
    }
}

/*
 * Blit operation - FUN_004142f0 / FUN_00414420 pattern
 */
int render_blit(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                int src_x, int src_y, int src_w, int src_h,
                int dst_x, int dst_y) {
    RECT src_rect, dst_rect;
    HRESULT hr;

    if (!src || !dst) return 0;

    /* Clipping - from original binary */
    if (dst_x < 0) {
        src_x -= dst_x;
        src_w += dst_x;
        dst_x = 0;
    }
    if (dst_y < 0) {
        src_y -= dst_y;
        src_h += dst_y;
        dst_y = 0;
    }
    if (dst_x + src_w > g_graphics.width) {
        src_w = g_graphics.width - dst_x;
    }
    if (dst_y + src_h > g_graphics.height) {
        src_h = g_graphics.height - dst_y;
    }

    if (src_w <= 0 || src_h <= 0) return 0;

    src_rect.left = src_x;
    src_rect.top = src_y;
    src_rect.right = src_x + src_w;
    src_rect.bottom = src_y + src_h;

    dst_rect.left = dst_x;
    dst_rect.top = dst_y;
    dst_rect.right = dst_x + src_w;
    dst_rect.bottom = dst_y + src_h;

    hr = IDirectDrawSurface_Blt(dst, &dst_rect, src, &src_rect, DDBLT_WAIT, NULL);
    return SUCCEEDED(hr);
}

/*
 * Scaled blit operation
 */
int render_blit_scaled(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                       int src_x, int src_y, int src_w, int src_h,
                       int dst_x, int dst_y, int dst_w, int dst_h) {
    RECT src_rect, dst_rect;
    HRESULT hr;

    if (!src || !dst) return 0;
    if (dst_w <= 0 || dst_h <= 0) return 0;

    src_rect.left = src_x;
    src_rect.top = src_y;
    src_rect.right = src_x + src_w;
    src_rect.bottom = src_y + src_h;

    dst_rect.left = dst_x;
    dst_rect.top = dst_y;
    dst_rect.right = dst_x + dst_w;
    dst_rect.bottom = dst_y + dst_h;

    hr = IDirectDrawSurface_Blt(dst, &dst_rect, src, &src_rect, DDBLT_WAIT, NULL);
    return SUCCEEDED(hr);
}

/*
 * Transparent blit with color key - FUN_00414190 pattern
 */
int render_blit_transparent(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                            int src_x, int src_y, int src_w, int src_h,
                            int dst_x, int dst_y, u32 color_key) {
    RECT src_rect, dst_rect;
    DDBLTFX bltfx;
    DDCOLORKEY ck;
    HRESULT hr;

    if (!src || !dst) return 0;

    /* Set color key on source surface */
    ck.dwColorSpaceLowValue = color_key;
    ck.dwColorSpaceHighValue = color_key;
    IDirectDrawSurface_SetColorKey(src, DDCKEY_SRCBLT, &ck);

    src_rect.left = src_x;
    src_rect.top = src_y;
    src_rect.right = src_x + src_w;
    src_rect.bottom = src_y + src_h;

    dst_rect.left = dst_x;
    dst_rect.top = dst_y;
    dst_rect.right = dst_x + src_w;
    dst_rect.bottom = dst_y + src_h;

    memset(&bltfx, 0, sizeof(DDBLTFX));
    bltfx.dwSize = sizeof(DDBLTFX);
    bltfx.ddckSrcColorkey.dwColorSpaceLowValue = color_key;
    bltfx.ddckSrcColorkey.dwColorSpaceHighValue = color_key;

    hr = IDirectDrawSurface_Blt(dst, &dst_rect, src, &src_rect,
        DDBLT_WAIT | DDBLT_KEYSRC, &bltfx);

    return SUCCEEDED(hr);
}

/*
 * Fast blit without scaling (BltFast)
 */
int render_blit_fast(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                     int src_x, int src_y, int src_w, int src_h,
                     int dst_x, int dst_y) {
    RECT src_rect;
    HRESULT hr;

    if (!src || !dst) return 0;

    src_rect.left = src_x;
    src_rect.top = src_y;
    src_rect.right = src_x + src_w;
    src_rect.bottom = src_y + src_h;

    hr = IDirectDrawSurface_BltFast(dst, dst_x, dst_y, src, &src_rect, 0x11);
    return SUCCEEDED(hr);
}

/*
 * Fill rectangle
 */
void render_fill_rect(IDirectDrawSurface* surface, int x, int y, int w, int h, u32 color) {
    DDBLTFX bltfx;
    RECT rect;
    HRESULT hr;

    if (!surface) return;

    rect.left = x;
    rect.top = y;
    rect.right = x + w;
    rect.bottom = y + h;

    memset(&bltfx, 0, sizeof(DDBLTFX));
    bltfx.dwSize = sizeof(DDBLTFX);
    bltfx.dwFillColor = color;

    hr = IDirectDrawSurface_Blt(surface, &rect, NULL, NULL,
        DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
}

/*
 * Check if surface is lost and restore it
 */
int render_check_surface(IDirectDrawSurface* surface) {
    HRESULT hr;

    if (!surface) return 0;

    hr = IDirectDrawSurface_IsLost(surface);
    if (hr == DDERR_SURFACELOST) {
        hr = IDirectDrawSurface_Restore(surface);
        return SUCCEEDED(hr);
    }

    return 1;
}
