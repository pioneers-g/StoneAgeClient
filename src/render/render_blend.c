/*
 * Stone Age Client - Render Blend Operations
 * Reverse engineered from sa_9061.exe
 * FUN_0047e970 - Alpha blend
 * FUN_0047f170 - Translucent blend
 * FUN_0047f710 - Gray tint blend
 *
 * Split from render_queue.c for better code organization
 */

#include <windows.h>
#include <ddraw.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "render_blend.h"
#include "render.h"
#include "directx.h"
#include "logger.h"

/* Pixel format - DAT_0054bdec: 2 = RGB565, other = RGB555 */
static int s_pixel_format = 2;

/* RGB565 macros */
#define RGB565_R(x) (((x) >> 11) & 0x1F)
#define RGB565_G(x) (((x) >> 5) & 0x3F)
#define RGB565_B(x) ((x) & 0x1F)
#define RGB565_PACK(r, g, b) (((r) << 11) | ((g) << 5) | (b))

/* RGB555 macros */
#define RGB555_R(x) (((x) >> 10) & 0x1F)
#define RGB555_G(x) (((x) >> 5) & 0x1F)
#define RGB555_B(x) ((x) & 0x1F)
#define RGB555_PACK(r, g, b) (((r) << 10) | ((g) << 5) | (b))

/*
 * Set pixel format for blend operations
 */
void render_blend_set_pixel_format(int format) {
    s_pixel_format = format;
}

/*
 * Alpha blend single pixel - RGB565 format
 * From FUN_0047e970 pattern
 */
u16 alpha_blend_rgb565(u16 src, u16 dst, int alpha) {
    int sr = RGB565_R(src);
    int sg = RGB565_G(src);
    int sb = RGB565_B(src);
    int dr = RGB565_R(dst);
    int dg = RGB565_G(dst);
    int db = RGB565_B(dst);

    /* Alpha blend formula: result = src * alpha + dst * (32 - alpha) */
    int r = ((sr - dr) * alpha >> 5) + dr;
    int g = ((sg - dg) * alpha >> 6) + dg;
    int b = ((sb - db) * alpha >> 5) + db;

    return RGB565_PACK(r, g, b);
}

/*
 * Alpha blend single pixel - RGB555 format
 */
u16 alpha_blend_rgb555(u16 src, u16 dst, int alpha) {
    int sr = RGB555_R(src);
    int sg = RGB555_G(src);
    int sb = RGB555_B(src);
    int dr = RGB555_R(dst);
    int dg = RGB555_G(dst);
    int db = RGB555_B(dst);

    int r = ((sr - dr) * alpha >> 5) + dr;
    int g = ((sg - dg) * alpha >> 5) + dg;
    int b = ((sb - db) * alpha >> 5) + db;

    return RGB555_PACK(r, g, b);
}

/*
 * Alpha blend blit - FUN_0047e970
 * Software implementation for per-pixel alpha blending
 */
void render_blit_alpha_blend(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                             int dst_x, int dst_y, int w, int h, int alpha) {
    DDSURFACEDESC2 src_desc, dst_desc;
    HRESULT hr;
    u16 *src_pixels, *dst_pixels;
    int src_pitch, dst_pitch;
    int x, y;
    int screen_w, screen_h;

    /* Lock surfaces */
    memset(&src_desc, 0, sizeof(DDSURFACEDESC2));
    src_desc.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(src, NULL, &src_desc, DDLOCK_WAIT, NULL);
    if (FAILED(hr)) return;

    memset(&dst_desc, 0, sizeof(DDSURFACEDESC2));
    dst_desc.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(dst, NULL, &dst_desc, DDLOCK_WAIT, NULL);
    if (FAILED(hr)) {
        IDirectDrawSurface_Unlock(src, NULL);
        return;
    }

    src_pixels = (u16*)src_desc.lpSurface;
    dst_pixels = (u16*)dst_desc.lpSurface;
    src_pitch = src_desc.lPitch / 2;
    dst_pitch = dst_desc.lPitch / 2;

    screen_w = g_graphics.width;
    screen_h = g_graphics.height;

    /* Clip to screen bounds */
    if (dst_x < 0) { w += dst_x; src_pixels -= dst_x; dst_x = 0; }
    if (dst_y < 0) { h += dst_y; src_pixels -= dst_y * src_pitch; dst_y = 0; }
    if (dst_x + w > screen_w) w = screen_w - dst_x;
    if (dst_y + h > screen_h) h = screen_h - dst_y;

    if (w <= 0 || h <= 0) {
        IDirectDrawSurface_Unlock(dst, NULL);
        IDirectDrawSurface_Unlock(src, NULL);
        return;
    }

    /* Alpha blend each pixel */
    if (s_pixel_format == 2) {
        for (y = 0; y < h; y++) {
            u16* src_row = src_pixels + y * src_pitch;
            u16* dst_row = dst_pixels + (dst_y + y) * dst_pitch + dst_x;

            for (x = 0; x < w; x++) {
                u16 src_pixel = src_row[x];
                if (src_pixel == 0) continue;
                dst_row[x] = alpha_blend_rgb565(src_pixel, dst_row[x], alpha);
            }
        }
    } else {
        for (y = 0; y < h; y++) {
            u16* src_row = src_pixels + y * src_pitch;
            u16* dst_row = dst_pixels + (dst_y + y) * dst_pitch + dst_x;

            for (x = 0; x < w; x++) {
                u16 src_pixel = src_row[x];
                if (src_pixel == 0) continue;
                dst_row[x] = alpha_blend_rgb555(src_pixel, dst_row[x], alpha);
            }
        }
    }

    IDirectDrawSurface_Unlock(dst, NULL);
    IDirectDrawSurface_Unlock(src, NULL);
}

/*
 * Additive blend blit
 * Used for glowing effects and highlights
 */
void render_blit_additive(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                          int dst_x, int dst_y, int w, int h) {
    DDSURFACEDESC2 src_desc, dst_desc;
    HRESULT hr;
    u16 *src_pixels, *dst_pixels;
    int src_pitch, dst_pitch;
    int x, y;
    int screen_w, screen_h;

    memset(&src_desc, 0, sizeof(DDSURFACEDESC2));
    src_desc.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(src, NULL, &src_desc, DDLOCK_WAIT, NULL);
    if (FAILED(hr)) return;

    memset(&dst_desc, 0, sizeof(DDSURFACEDESC2));
    dst_desc.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(dst, NULL, &dst_desc, DDLOCK_WAIT, NULL);
    if (FAILED(hr)) {
        IDirectDrawSurface_Unlock(src, NULL);
        return;
    }

    src_pixels = (u16*)src_desc.lpSurface;
    dst_pixels = (u16*)dst_desc.lpSurface;
    src_pitch = src_desc.lPitch / 2;
    dst_pitch = dst_desc.lPitch / 2;

    screen_w = g_graphics.width;
    screen_h = g_graphics.height;

    /* Clip to screen bounds */
    if (dst_x < 0) { w += dst_x; src_pixels -= dst_x; dst_x = 0; }
    if (dst_y < 0) { h += dst_y; src_pixels -= dst_y * src_pitch; dst_y = 0; }
    if (dst_x + w > screen_w) w = screen_w - dst_x;
    if (dst_y + h > screen_h) h = screen_h - dst_y;

    if (w <= 0 || h <= 0) {
        IDirectDrawSurface_Unlock(dst, NULL);
        IDirectDrawSurface_Unlock(src, NULL);
        return;
    }

    /* Additive blend each pixel */
    if (s_pixel_format == 2) {
        for (y = 0; y < h; y++) {
            u16* src_row = src_pixels + y * src_pitch;
            u16* dst_row = dst_pixels + (dst_y + y) * dst_pitch + dst_x;

            for (x = 0; x < w; x++) {
                u16 src_pixel = src_row[x];
                if (src_pixel == 0) continue;

                int sr = RGB565_R(src_pixel);
                int sg = RGB565_G(src_pixel);
                int sb = RGB565_B(src_pixel);
                int dr = RGB565_R(dst_row[x]);
                int dg = RGB565_G(dst_row[x]);
                int db = RGB565_B(dst_row[x]);

                int r = sr + dr; if (r > 31) r = 31;
                int g = sg + dg; if (g > 63) g = 63;
                int b = sb + db; if (b > 31) b = 31;

                dst_row[x] = RGB565_PACK(r, g, b);
            }
        }
    } else {
        for (y = 0; y < h; y++) {
            u16* src_row = src_pixels + y * src_pitch;
            u16* dst_row = dst_pixels + (dst_y + y) * dst_pitch + dst_x;

            for (x = 0; x < w; x++) {
                u16 src_pixel = src_row[x];
                if (src_pixel == 0) continue;

                int sr = RGB555_R(src_pixel);
                int sg = RGB555_G(src_pixel);
                int sb = RGB555_B(src_pixel);
                int dr = RGB555_R(dst_row[x]);
                int dg = RGB555_G(dst_row[x]);
                int db = RGB555_B(dst_row[x]);

                int r = sr + dr; if (r > 31) r = 31;
                int g = sg + dg; if (g > 31) g = 31;
                int b = sb + db; if (b > 31) b = 31;

                dst_row[x] = RGB555_PACK(r, g, b);
            }
        }
    }

    IDirectDrawSurface_Unlock(dst, NULL);
    IDirectDrawSurface_Unlock(src, NULL);
}

/*
 * Subtractive blend blit
 * Used for shadow/darkening effects
 */
void render_blit_subtractive(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                             int dst_x, int dst_y, int w, int h) {
    DDSURFACEDESC2 src_desc, dst_desc;
    HRESULT hr;
    u16 *src_pixels, *dst_pixels;
    int src_pitch, dst_pitch;
    int x, y;
    int screen_w, screen_h;

    memset(&src_desc, 0, sizeof(DDSURFACEDESC2));
    src_desc.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(src, NULL, &src_desc, DDLOCK_WAIT, NULL);
    if (FAILED(hr)) return;

    memset(&dst_desc, 0, sizeof(DDSURFACEDESC2));
    dst_desc.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(dst, NULL, &dst_desc, DDLOCK_WAIT, NULL);
    if (FAILED(hr)) {
        IDirectDrawSurface_Unlock(src, NULL);
        return;
    }

    src_pixels = (u16*)src_desc.lpSurface;
    dst_pixels = (u16*)dst_desc.lpSurface;
    src_pitch = src_desc.lPitch / 2;
    dst_pitch = dst_desc.lPitch / 2;

    screen_w = g_graphics.width;
    screen_h = g_graphics.height;

    /* Clip to screen bounds */
    if (dst_x < 0) { w += dst_x; src_pixels -= dst_x; dst_x = 0; }
    if (dst_y < 0) { h += dst_y; src_pixels -= dst_y * src_pitch; dst_y = 0; }
    if (dst_x + w > screen_w) w = screen_w - dst_x;
    if (dst_y + h > screen_h) h = screen_h - dst_y;

    if (w <= 0 || h <= 0) {
        IDirectDrawSurface_Unlock(dst, NULL);
        IDirectDrawSurface_Unlock(src, NULL);
        return;
    }

    /* Subtractive blend each pixel */
    if (s_pixel_format == 2) {
        for (y = 0; y < h; y++) {
            u16* src_row = src_pixels + y * src_pitch;
            u16* dst_row = dst_pixels + (dst_y + y) * dst_pitch + dst_x;

            for (x = 0; x < w; x++) {
                u16 src_pixel = src_row[x];
                if (src_pixel == 0) continue;

                int sr = RGB565_R(src_pixel);
                int sg = RGB565_G(src_pixel);
                int sb = RGB565_B(src_pixel);
                int dr = RGB565_R(dst_row[x]);
                int dg = RGB565_G(dst_row[x]);
                int db = RGB565_B(dst_row[x]);

                int r = dr - sr; if (r < 0) r = 0;
                int g = dg - sg; if (g < 0) g = 0;
                int b = db - sb; if (b < 0) b = 0;

                dst_row[x] = RGB565_PACK(r, g, b);
            }
        }
    } else {
        for (y = 0; y < h; y++) {
            u16* src_row = src_pixels + y * src_pitch;
            u16* dst_row = dst_pixels + (dst_y + y) * dst_pitch + dst_x;

            for (x = 0; x < w; x++) {
                u16 src_pixel = src_row[x];
                if (src_pixel == 0) continue;

                int sr = RGB555_R(src_pixel);
                int sg = RGB555_G(src_pixel);
                int sb = RGB555_B(src_pixel);
                int dr = RGB555_R(dst_row[x]);
                int dg = RGB555_G(dst_row[x]);
                int db = RGB555_B(dst_row[x]);

                int r = dr - sr; if (r < 0) r = 0;
                int g = dg - sg; if (g < 0) g = 0;
                int b = db - sb; if (b < 0) b = 0;

                dst_row[x] = RGB555_PACK(r, g, b);
            }
        }
    }

    IDirectDrawSurface_Unlock(dst, NULL);
    IDirectDrawSurface_Unlock(src, NULL);
}

/*
 * Translucent blend blit - FUN_0047f170
 * Uses max(src, dst) for each color channel
 */
void render_blit_translucent(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                             int dst_x, int dst_y, int w, int h) {
    DDSURFACEDESC2 src_desc, dst_desc;
    HRESULT hr;
    u16 *src_pixels, *dst_pixels;
    int src_pitch, dst_pitch;
    int x, y;
    int screen_w, screen_h;

    memset(&src_desc, 0, sizeof(DDSURFACEDESC2));
    src_desc.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(src, NULL, &src_desc, DDLOCK_WAIT, NULL);
    if (FAILED(hr)) return;

    memset(&dst_desc, 0, sizeof(DDSURFACEDESC2));
    dst_desc.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(dst, NULL, &dst_desc, DDLOCK_WAIT, NULL);
    if (FAILED(hr)) {
        IDirectDrawSurface_Unlock(src, NULL);
        return;
    }

    src_pixels = (u16*)src_desc.lpSurface;
    dst_pixels = (u16*)dst_desc.lpSurface;
    src_pitch = src_desc.lPitch / 2;
    dst_pitch = dst_desc.lPitch / 2;

    screen_w = g_graphics.width;
    screen_h = g_graphics.height;

    /* Clip to screen bounds */
    if (dst_x < 0) { w += dst_x; dst_x = 0; }
    if (dst_y < 0) { h += dst_y; dst_y = 0; }
    if (dst_x + w > screen_w) w = screen_w - dst_x;
    if (dst_y + h > screen_h) h = screen_h - dst_y;

    if (w <= 0 || h <= 0) {
        IDirectDrawSurface_Unlock(dst, NULL);
        IDirectDrawSurface_Unlock(src, NULL);
        return;
    }

    /* Translucent blend - FUN_0047f170: max(src, dst) */
    if (s_pixel_format == 2) {
        for (y = 0; y < h; y++) {
            u16* src_row = src_pixels + y * src_pitch;
            u16* dst_row = dst_pixels + (dst_y + y) * dst_pitch + dst_x;

            for (x = 0; x < w; x++) {
                u16 sp = src_row[x];
                if (sp == 0) continue;

                u16 dp = dst_row[x];
                int sr = RGB565_R(sp), sg = RGB565_G(sp), sb = RGB565_B(sp);
                int dr = RGB565_R(dp), dg = RGB565_G(dp), db = RGB565_B(dp);

                dst_row[x] = RGB565_PACK(
                    (sr > dr) ? sr : dr,
                    (sg > dg) ? sg : dg,
                    (sb > db) ? sb : db
                );
            }
        }
    } else {
        for (y = 0; y < h; y++) {
            u16* src_row = src_pixels + y * src_pitch;
            u16* dst_row = dst_pixels + (dst_y + y) * dst_pitch + dst_x;

            for (x = 0; x < w; x++) {
                u16 sp = src_row[x];
                if (sp == 0) continue;

                u16 dp = dst_row[x];
                int sr = RGB555_R(sp), sg = RGB555_G(sp), sb = RGB555_B(sp);
                int dr = RGB555_R(dp), dg = RGB555_G(dp), db = RGB555_B(dp);

                dst_row[x] = RGB555_PACK(
                    (sr > dr) ? sr : dr,
                    (sg > dg) ? sg : dg,
                    (sb > db) ? sb : db
                );
            }
        }
    }

    IDirectDrawSurface_Unlock(dst, NULL);
    IDirectDrawSurface_Unlock(src, NULL);
}

/*
 * Gray tint blend - FUN_0047f710
 * Applies grayscale with tint effect
 */
void render_blit_gray_tint(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                           int dst_x, int dst_y, int w, int h, int tint) {
    DDSURFACEDESC2 src_desc, dst_desc;
    HRESULT hr;
    u16 *src_pixels, *dst_pixels;
    int src_pitch, dst_pitch;
    int x, y;
    int screen_w, screen_h;

    memset(&src_desc, 0, sizeof(DDSURFACEDESC2));
    src_desc.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(src, NULL, &src_desc, DDLOCK_WAIT, NULL);
    if (FAILED(hr)) return;

    memset(&dst_desc, 0, sizeof(DDSURFACEDESC2));
    dst_desc.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(dst, NULL, &dst_desc, DDLOCK_WAIT, NULL);
    if (FAILED(hr)) {
        IDirectDrawSurface_Unlock(src, NULL);
        return;
    }

    src_pixels = (u16*)src_desc.lpSurface;
    dst_pixels = (u16*)dst_desc.lpSurface;
    src_pitch = src_desc.lPitch / 2;
    dst_pitch = dst_desc.lPitch / 2;

    screen_w = g_graphics.width;
    screen_h = g_graphics.height;

    /* Clip to screen bounds */
    if (dst_x < 0) { w += dst_x; dst_x = 0; }
    if (dst_y < 0) { h += dst_y; dst_y = 0; }
    if (dst_x + w > screen_w) w = screen_w - dst_x;
    if (dst_y + h > screen_h) h = screen_h - dst_y;

    if (w <= 0 || h <= 0) {
        IDirectDrawSurface_Unlock(dst, NULL);
        IDirectDrawSurface_Unlock(src, NULL);
        return;
    }

    /* Gray tint: gray = (R*3 + G*6 + B) / 10 */
    if (s_pixel_format == 2) {
        for (y = 0; y < h; y++) {
            u16* src_row = src_pixels + y * src_pitch;
            u16* dst_row = dst_pixels + (dst_y + y) * dst_pitch + dst_x;

            for (x = 0; x < w; x++) {
                u16 sp = src_row[x];
                if (sp == 0) continue;

                int r = RGB565_R(sp);
                int g = RGB565_G(sp);
                int b = RGB565_B(sp);
                int gray = (r * 3 + g * 6 + b) / 10;

                dst_row[x] = RGB565_PACK(gray << tint, (gray << 6) | gray, gray);
            }
        }
    } else {
        for (y = 0; y < h; y++) {
            u16* src_row = src_pixels + y * src_pitch;
            u16* dst_row = dst_pixels + (dst_y + y) * dst_pitch + dst_x;

            for (x = 0; x < w; x++) {
                u16 sp = src_row[x];
                if (sp == 0) continue;

                int r = RGB555_R(sp);
                int g = RGB555_G(sp);
                int b = RGB555_B(sp);
                int gray = (r * 3 + g * 6 + b) / 10;

                dst_row[x] = RGB555_PACK(gray << tint, (gray << 5) | gray, gray);
            }
        }
    }

    IDirectDrawSurface_Unlock(dst, NULL);
    IDirectDrawSurface_Unlock(src, NULL);
}

/*
 * Per-pixel alpha blend blit with alpha channel - FUN_0047fae0
 * Uses separate alpha buffer for per-pixel alpha values
 * Each pixel uses its own alpha from the alpha_surface
 */
void render_blit_per_pixel_alpha(IDirectDrawSurface* src, IDirectDrawSurface* alpha_surf,
                                  IDirectDrawSurface* dst, int dst_x, int dst_y,
                                  int src_x, int src_y, int w, int h, int flip) {
    DDSURFACEDESC2 src_desc, dst_desc, alpha_desc;
    HRESULT hr;
    u16 *src_pixels, *dst_pixels;
    u8 *alpha_pixels;
    int src_pitch, dst_pitch, alpha_pitch;
    int x, y;
    int screen_w, screen_h;
    int src_offset_x = 0, src_offset_y = 0;

    screen_w = g_graphics.width;
    screen_h = g_graphics.height;

    /* Handle flipping */
    if (flip) {
        src_offset_x = w - src_x;
        src_offset_y = h - src_y;
    }

    /* Clip to screen bounds - FUN_0047fae0 pattern */
    if (dst_x < 0) {
        w += dst_x;
        if (w <= 0) return;
        src_offset_x = -dst_x;
        dst_x = 0;
    }
    if (dst_y < 0) {
        h += dst_y;
        if (h <= 0) return;
        src_offset_y = -dst_y;
        dst_y = 0;
    }
    if (dst_x + w > screen_w) {
        w = screen_w - dst_x;
        if (w <= 0) return;
    }
    if (dst_y + h > screen_h) {
        h = screen_h - dst_y;
        if (h <= 0) return;
    }

    /* Lock surfaces */
    memset(&src_desc, 0, sizeof(DDSURFACEDESC2));
    src_desc.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(src, NULL, &src_desc, DDLOCK_WAIT, NULL);
    if (FAILED(hr)) return;

    memset(&alpha_desc, 0, sizeof(DDSURFACEDESC2));
    alpha_desc.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(alpha_surf, NULL, &alpha_desc, DDLOCK_WAIT, NULL);
    if (FAILED(hr)) {
        IDirectDrawSurface_Unlock(src, NULL);
        return;
    }

    memset(&dst_desc, 0, sizeof(DDSURFACEDESC2));
    dst_desc.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(dst, NULL, &dst_desc, DDLOCK_WAIT, NULL);
    if (FAILED(hr)) {
        IDirectDrawSurface_Unlock(alpha_surf, NULL);
        IDirectDrawSurface_Unlock(src, NULL);
        return;
    }

    src_pixels = (u16*)src_desc.lpSurface;
    alpha_pixels = (u8*)alpha_desc.lpSurface;
    dst_pixels = (u16*)dst_desc.lpSurface;
    src_pitch = src_desc.lPitch / 2;
    alpha_pitch = alpha_desc.lPitch;
    dst_pitch = dst_desc.lPitch / 2;

    /* Per-pixel alpha blend - FUN_0047fae0 pattern */
    if (s_pixel_format == 2) {
        for (y = 0; y < h; y++) {
            int src_row_idx = (y + src_offset_y) * src_pitch + src_offset_x;
            int dst_row_idx = (dst_y + y) * dst_pitch + dst_x;
            int alpha_row_idx = (y + src_offset_y) * alpha_pitch + src_offset_x;

            u16* src_row = src_pixels + src_row_idx;
            u16* dst_row = dst_pixels + dst_row_idx;
            u8* alpha_row = alpha_pixels + alpha_row_idx;

            for (x = 0; x < w; x++) {
                u16 sp = src_row[x];
                u8 alpha = alpha_row[x];

                if (sp == 0 || alpha == 0) continue;

                /* Alpha blend: result = src * alpha + dst * (32 - alpha) */
                int sr = RGB565_R(sp);
                int sg = RGB565_G(sp);
                int sb = RGB565_B(sp);
                int dr = RGB565_R(dst_row[x]);
                int dg = RGB565_G(dst_row[x]);
                int db = RGB565_B(dst_row[x]);

                int r = ((sr - dr) * alpha >> 5) + dr;
                int g = ((sg - dg) * alpha >> 6) + dg;
                int b = ((sb - db) * alpha >> 5) + db;

                dst_row[x] = RGB565_PACK(r, g, b);
            }
        }
    } else {
        for (y = 0; y < h; y++) {
            int src_row_idx = (y + src_offset_y) * src_pitch + src_offset_x;
            int dst_row_idx = (dst_y + y) * dst_pitch + dst_x;
            int alpha_row_idx = (y + src_offset_y) * alpha_pitch + src_offset_x;

            u16* src_row = src_pixels + src_row_idx;
            u16* dst_row = dst_pixels + dst_row_idx;
            u8* alpha_row = alpha_pixels + alpha_row_idx;

            for (x = 0; x < w; x++) {
                u16 sp = src_row[x];
                u8 alpha = alpha_row[x];

                if (sp == 0 || alpha == 0) continue;

                int sr = RGB555_R(sp);
                int sg = RGB555_G(sp);
                int sb = RGB555_B(sp);
                int dr = RGB555_R(dst_row[x]);
                int dg = RGB555_G(dst_row[x]);
                int db = RGB555_B(dst_row[x]);

                int r = ((sr - dr) * alpha >> 5) + dr;
                int g = ((sg - dg) * alpha >> 5) + dg;
                int b = ((sb - db) * alpha >> 5) + db;

                dst_row[x] = RGB555_PACK(r, g, b);
            }
        }
    }

    IDirectDrawSurface_Unlock(dst, NULL);
    IDirectDrawSurface_Unlock(alpha_surf, NULL);
    IDirectDrawSurface_Unlock(src, NULL);
}

/*
 * Fill rectangle with color
 */
void render_fill_rect(IDirectDrawSurface* surface, int x, int y, int w, int h, u32 color) {
    DDBLTFX bltfx;
    RECT rect;
    HRESULT hr;
    extern RenderContext g_render;

    if (!surface) surface = g_render.target;
    if (!surface) return;

    rect.left = x;
    rect.top = y;
    rect.right = x + w;
    rect.bottom = y + h;

    memset(&bltfx, 0, sizeof(bltfx));
    bltfx.dwSize = sizeof(bltfx);
    bltfx.dwFillColor = color;

    hr = IDirectDrawSurface_Blt(surface, &rect, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
    (void)hr;
}

/*
 * Blit with transparency (color key) - FUN_004142f0
 * Uses IDirectDrawSurface::Blt with DDBLT_WAIT|DDBLT_KEYSRC (0x01008000)
 * Matches original binary's clipping and rect calculation exactly
 */
int render_blit_transparent(IDirectDrawSurface* src, IDirectDrawSurface* dst,
                             int src_x, int src_y, int w, int h, int dst_x, int dst_y, u32 color_key) {
    RECT dest_rect, src_rect;
    int clip_src_x, clip_src_y, clip_w, clip_h;
    HRESULT hr;

    if (!src || !dst) return 0;

    /* FUN_004142f0 clipping: clip source rect against dest surface bounds */
    clip_src_x = 0;
    clip_src_y = 0;
    clip_w = w;
    clip_h = h;

    if (dst_x < 0) {
        clip_src_x = -dst_x;
        dst_x = 0;
    }
    if (dst_x + clip_w > g_graphics.width) {
        clip_w = g_graphics.width - dst_x;
    }
    if (dst_y < 0) {
        clip_src_y = -dst_y;
        dst_y = 0;
    }
    if (dst_y + clip_h > g_graphics.height) {
        clip_h = g_graphics.height - dst_y;
    }

    if (clip_w <= 0 || clip_h <= 0) return 0;

    /* Build rects matching FUN_004142f0: dest[left,top,right,bottom], src[left,top,right,bottom] */
    dest_rect.left = dst_x;
    dest_rect.top = dst_y;
    dest_rect.right = (dst_x - clip_src_x) + clip_w;
    dest_rect.bottom = (dst_y - clip_src_y) + clip_h;

    src_rect.left = src_x + clip_src_x;
    src_rect.top = src_y + clip_src_y;
    src_rect.right = src_rect.left + clip_w;
    src_rect.bottom = src_rect.top + clip_h;

    /* DDBLT_WAIT|DDBLT_KEYSRC = 0x01008000 - matches DAT_01008000 in FUN_004142f0 */
    hr = IDirectDrawSurface_Blt(dst, &dest_rect, src, &src_rect,
                                 DDBLT_WAIT | DDBLT_KEYSRC, NULL);
    (void)hr;
    (void)color_key;

    return 1;
}

/*
 * Render primitive rectangle - FUN_00412eb0
 * Draws outlined/filled rectangles directly to the surface via lock
 *
 * rect: pointer to {left, top, right, bottom}
 * color: RGB565 color value (extracted from sprite_id lower bits)
 * mode: 0 = outline only, 1 = filled, 2 = filled (alternate style)
 *
 * Binary behavior:
 * - Clamps rect to surface bounds
 * - Mode 0: draws 2-pixel-wide outline border
 * - Mode 1: fills entire rect with solid color
 * - Mode 2: fills rect with pattern
 */
void render_primitive_rect(RECT* rect, u32 sprite_id, int mode) {
    DDSURFACEDESC2 ddsd;
    HRESULT hr;
    int left, top, right, bottom;
    int w, h;
    u16 color565;
    u16* pixels;
    int pitch;
    int x, y;

    if (!rect || !g_graphics.offscreen_surface) return;

    left = rect->left;
    top = rect->top;
    right = rect->right;
    bottom = rect->bottom;

    /* Clamp to surface bounds */
    if (left < 0) left = 0;
    if (right > g_graphics.width) right = g_graphics.width;
    if (top < 0) top = 0;
    if (bottom > g_graphics.height) bottom = g_graphics.height;

    w = right - left;
    h = bottom - top;

    /* Minimum size check from FUN_00412eb0: mode 2 allows width 0, others need 3x3 minimum */
    if (mode != 2) {
        if (w < 3 || h < 3) return;
    } else {
        if (w <= 0) return;
    }

    /* Extract color from sprite_id - lower 16 bits in DAT_0054c628 color table lookup */
    color565 = (u16)(sprite_id & 0xFFFF);
    if (color565 == 0) color565 = 0xFFFF;

    /* Lock surface */
    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    hr = IDirectDrawSurface_Lock(g_graphics.offscreen_surface, NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (FAILED(hr)) return;

    pixels = (u16*)ddsd.lpSurface;
    pitch = ddsd.lPitch / 2;

    if (mode == 0) {
        /* Outline mode - 2 pixel wide border */
        for (y = 0; y < h && (top + y) < g_graphics.height; y++) {
            for (x = 0; x < w && (left + x) < g_graphics.width; x++) {
                int px = left + x;
                int py = top + y;
                /* Draw border: top/bottom 2 rows, left/right 2 columns */
                if (y < 2 || y >= h - 2 || x < 2 || x >= w - 2) {
                    pixels[py * pitch + px] = color565;
                }
            }
        }
    } else {
        /* Fill modes (1 and 2) - solid fill */
        for (y = 0; y < h && (top + y) < g_graphics.height; y++) {
            for (x = 0; x < w && (left + x) < g_graphics.width; x++) {
                pixels[(top + y) * pitch + (left + x)] = color565;
            }
        }
    }

    IDirectDrawSurface_Unlock(g_graphics.offscreen_surface, NULL);
}
