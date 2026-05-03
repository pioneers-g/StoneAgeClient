/*
 * Stone Age Client - Main Rendering Module
 * Reverse engineered from sa_9061.exe
 * FUN_00414780, FUN_00415a70, FUN_004155e0
 *
 * This module provides:
 * - Render context management
 * - Drawing primitives (rect, line, pixel)
 * - Font management
 * - Text rendering
 * - UI element rendering
 * - UI sprite ID management
 *
 * Sub-modules:
 * - render_surface.c: Surface operations and blit functions
 * - render_queue.c: Render queue management
 * - render_sprite.c: Sprite cache and sprite rendering
 * - render_text.c: Text queue management
 */

#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "render.h"
#include "render_sprite.h"
#include "directx.h"
#include "assets.h"
#include "logger.h"

/* Global render context */
RenderContext g_render = {0};

/* Font handles - matching DAT_0054b188, DAT_0054c854, DAT_0054c85c */
HFONT g_font = NULL;          /* Primary font - DAT_0054b188 */
HFONT g_font_custom = NULL;   /* Custom size font - DAT_0054c854 */
HFONT g_font_custom2 = NULL;  /* Secondary custom font - DAT_0054c85c */
int g_font_size = 14;

/* Half-resolution mode flag - DAT_04560214 */
static int s_half_resolution = 0;

/* Alpha mode flag - DAT_0054c83c */
static int s_alpha_mode = 0;

/*
 * Initialize rendering system
 */
int render_init(void) {
    memset(&g_render, 0, sizeof(RenderContext));
    g_render.target = g_graphics.offscreen_surface;
    g_render.color_key = 0;
    g_render.use_color_key = 1;

    /* Set default clip rect */
    g_render.clip_rect.left = 0;
    g_render.clip_rect.top = 0;
    g_render.clip_rect.right = g_graphics.width;
    g_render.clip_rect.bottom = g_graphics.height;

    /* Initialize sub-modules */
    render_queue_init();
    render_sprite_cache_init();

    /* Initialize UI sprites */
    render_init_ui_sprites();

    LOG_INFO("Render system initialized");
    return 1;
}

/*
 * Shutdown rendering system
 */
void render_shutdown(void) {
    if (g_font) {
        DeleteObject(g_font);
        g_font = NULL;
    }
    if (g_font_custom) {
        DeleteObject(g_font_custom);
        g_font_custom = NULL;
    }
    if (g_font_custom2) {
        DeleteObject(g_font_custom2);
        g_font_custom2 = NULL;
    }

    /* Clear sprite cache */
    render_clear_sprite_cache();

    LOG_INFO("Render system shutdown");
}

/*
 * Set font - FUN_00414780 pattern
 * Uses 14px in normal mode, 7px in half-resolution mode (DAT_04560214)
 */
void render_set_font(int font_size) {
    LPCSTR font_name = "SimHei";

    if (g_font) {
        DeleteObject(g_font);
        g_font = NULL;
    }

    g_font_size = font_size;

    /* In half-resolution mode, use smaller font */
    if (s_half_resolution) {
        g_font = CreateFontA(7, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            PROOF_QUALITY, DEFAULT_PITCH | FF_DONTCARE, font_name);
    } else {
        g_font = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            PROOF_QUALITY, DEFAULT_PITCH | FF_DONTCARE, font_name);
    }

    /* Fallback to default font if SimHei not available */
    if (!g_font) {
        g_font = CreateFontA(s_half_resolution ? 7 : 14, 0, 0, 0, FW_NORMAL,
            FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, NULL);
    }
}

/*
 * Create dynamic font - FUN_00415a70 pattern
 */
HFONT render_create_font(int font_size) {
    return CreateFontA(font_size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        PROOF_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "SimHei");
}

/*
 * Set half-resolution mode - affects font size and coordinate scaling
 */
void render_set_half_resolution(int enabled) {
    s_half_resolution = enabled;
    render_sprite_set_alpha_mode(enabled);
}

/*
 * Set alpha mode - affects rendering and sprite IDs
 */
void render_set_alpha_mode(int enabled) {
    s_alpha_mode = enabled;
    render_sprite_set_alpha_mode(enabled);
}

/*
 * Get alpha mode
 */
int render_get_alpha_mode(void) {
    return s_alpha_mode;
}

/* ========================================
 * Drawing Primitives
 * ======================================== */

/*
 * Draw rectangle outline
 */
void render_draw_rect(IDirectDrawSurface* surface, int x, int y, int w, int h, u32 color) {
    render_fill_rect(surface, x, y, w, 1, color);         /* Top */
    render_fill_rect(surface, x, y + h - 1, w, 1, color); /* Bottom */
    render_fill_rect(surface, x, y, 1, h, color);         /* Left */
    render_fill_rect(surface, x + w - 1, y, 1, h, color); /* Right */
}

/*
 * Draw line using Bresenham algorithm
 */
void render_draw_line(IDirectDrawSurface* surface, int x1, int y1, int x2, int y2, u32 color) {
    void* pixels;
    int pitch;
    int dx, dy, sx, sy, err, e2;

    if (!render_lock_surface(surface, &pixels, &pitch)) return;

    dx = abs(x2 - x1);
    dy = abs(y2 - y1);
    sx = (x1 < x2) ? 1 : -1;
    sy = (y1 < y2) ? 1 : -1;
    err = dx - dy;

    /* For 16-bit surfaces */
    u16* pixel_data = (u16*)pixels;

    while (1) {
        if (x1 >= 0 && x1 < g_graphics.width && y1 >= 0 && y1 < g_graphics.height) {
            pixel_data[y1 * (pitch / 2) + x1] = (u16)color;
        }

        if (x1 == x2 && y1 == y2) break;

        e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }

    render_unlock_surface(surface);
}

/*
 * Draw single pixel
 */
void render_draw_pixel(IDirectDrawSurface* surface, int x, int y, u32 color) {
    DDSURFACEDESC2 ddsd;
    HRESULT hr;

    if (!surface) return;
    if (x < 0 || x >= g_graphics.width || y < 0 || y >= g_graphics.height) return;

    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);

    hr = IDirectDrawSurface_Lock(surface, NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (SUCCEEDED(hr)) {
        u16* pixels = (u16*)ddsd.lpSurface;
        pixels[y * (ddsd.lPitch / 2) + x] = (u16)color;
        IDirectDrawSurface_Unlock(surface, NULL);
    }
}

/* ========================================
 * Text Rendering
 * ======================================== */

/*
 * Render text - GDI based
 */
int render_text(int x, int y, const char* text, u32 color) {
    HDC hdc;
    HRESULT hr;
    SIZE size;
    COLORREF old_color;

    if (!g_graphics.offscreen_surface || !text) return 0;

    hr = IDirectDrawSurface_GetDC(g_graphics.offscreen_surface, &hdc);
    if (FAILED(hr)) return 0;

    if (g_font) {
        SelectObject(hdc, g_font);
    }

    SetBkMode(hdc, TRANSPARENT);
    old_color = SetTextColor(hdc, RGB((color >> 11) & 0x1F << 3,
                                       (color >> 5) & 0x3F << 2,
                                       color & 0x1F << 3));

    TextOutA(hdc, x, y, text, (int)strlen(text));

    SetTextColor(hdc, old_color);
    IDirectDrawSurface_ReleaseDC(g_graphics.offscreen_surface, hdc);

    GetTextExtentPoint32A(hdc, text, (int)strlen(text), &size);
    return size.cx;
}

/*
 * Render text with shadow
 */
int render_text_shadowed(int x, int y, const char* text, u32 color, u32 shadow_color) {
    render_text(x + 1, y + 1, text, shadow_color);
    return render_text(x, y, text, color);
}

/*
 * Render centered text
 */
int render_text_centered(int x, int y, const char* text, u32 color) {
    HDC hdc;
    SIZE size;
    int width;

    if (!g_graphics.offscreen_surface || !text) return 0;

    if (SUCCEEDED(IDirectDrawSurface_GetDC(g_graphics.offscreen_surface, &hdc))) {
        if (g_font) SelectObject(hdc, g_font);
        GetTextExtentPoint32A(hdc, text, (int)strlen(text), &size);
        IDirectDrawSurface_ReleaseDC(g_graphics.offscreen_surface, hdc);
        width = size.cx;
    } else {
        width = (int)strlen(text) * 8;
    }

    return render_text(x - width / 2, y, text, color);
}

/*
 * Get text width
 */
int render_text_width(const char* text) {
    HDC hdc;
    SIZE size;

    if (!text) return 0;

    if (SUCCEEDED(IDirectDrawSurface_GetDC(g_graphics.offscreen_surface, &hdc))) {
        if (g_font) SelectObject(hdc, g_font);
        GetTextExtentPoint32A(hdc, text, (int)strlen(text), &size);
        IDirectDrawSurface_ReleaseDC(g_graphics.offscreen_surface, hdc);
        return size.cx;
    }

    return (int)strlen(text) * 8;
}

/*
 * Get text width for first n characters
 */
int render_text_width_n(const char* text, int n) {
    HDC hdc;
    SIZE size;

    if (!text || n <= 0) return 0;

    if (SUCCEEDED(IDirectDrawSurface_GetDC(g_graphics.offscreen_surface, &hdc))) {
        if (g_font) SelectObject(hdc, g_font);
        GetTextExtentPoint32A(hdc, text, n, &size);
        IDirectDrawSurface_ReleaseDC(g_graphics.offscreen_surface, hdc);
        return size.cx;
    }

    return n * 8;
}

/* ========================================
 * Clipping
 * ======================================== */

/*
 * Set clipping rectangle
 */
void render_set_clip(int x, int y, int w, int h) {
    g_render.clip_rect.left = x;
    g_render.clip_rect.top = y;
    g_render.clip_rect.right = x + w;
    g_render.clip_rect.bottom = y + h;
}

/*
 * Reset clipping rectangle
 */
void render_reset_clip(void) {
    g_render.clip_rect.left = 0;
    g_render.clip_rect.top = 0;
    g_render.clip_rect.right = g_graphics.width;
    g_render.clip_rect.bottom = g_graphics.height;
}

/* ========================================
 * UI Rendering
 * ======================================== */

/*
 * Render dialog box
 */
void render_dialog(int x, int y, int w, int h) {
    /* Background */
    render_fill_rect(g_graphics.offscreen_surface, x, y, w, h, 0x0842);

    /* Border */
    render_draw_rect(g_graphics.offscreen_surface, x, y, w, h, COLOR_WHITE);
    render_draw_rect(g_graphics.offscreen_surface, x + 1, y + 1, w - 2, h - 2, 0x8410);
}

/*
 * Render button
 */
void render_button(int x, int y, int w, int h, const char* text, int state) {
    u32 bg_color = state ? 0x1084 : 0x0842;
    u32 border_color = state ? COLOR_YELLOW : COLOR_WHITE;

    render_fill_rect(g_graphics.offscreen_surface, x, y, w, h, bg_color);
    render_draw_rect(g_graphics.offscreen_surface, x, y, w, h, border_color);

    if (text) {
        render_text_centered(x + w / 2, y + (h - 12) / 2, text, COLOR_WHITE);
    }
}

/*
 * Render textbox
 */
void render_textbox(int x, int y, int w, int h, const char* text) {
    /* Background */
    render_fill_rect(g_graphics.offscreen_surface, x, y, w, h, 0x0000);

    /* Border */
    render_draw_rect(g_graphics.offscreen_surface, x, y, w, h, COLOR_WHITE);

    /* Text */
    if (text) {
        render_text(x + 4, y + (h - 12) / 2, text, COLOR_WHITE);
    }
}

/* ========================================
 * UI Sprite ID Management - FUN_004155e0
 * ======================================== */

/* UI sprite ID storage - matches various DAT_0054xxxx globals */
static u32 s_ui_sprite_ids[64] = {0};

/* Alpha mode sprite IDs (0x7a1xx range) - from FUN_004155e0 */
static const u32 s_ui_sprites_alpha[57] = {
    0x7a145, 0x7a155, 0x7a152, 0x7a156, 0x7a157,  /* 0-4 */
    0x7a158, 0x7a153, 0x7a154, 0x7a150, 0x7a151,  /* 5-9 */
    500000,  0x7a121, 0x7a122, 0x7a123, 0x7a124,  /* 10-14: index 10 = extended sprite base */
    0x7a125, 0x7a126, 0x7a127, 0x7a128, 0x7a129,  /* 15-19 */
    0x7a12a, 0x7a12b, 0x7a12c, 0x7a12d, 0x7a12e,  /* 20-24 */
    0x7a12f, 0x7a130, 0x7a131, 0x7a132, 0x7a133,  /* 25-29 */
    0x7a134, 0x7a135, 0x7a136, 0x7a137, 0x7a138,  /* 30-34 */
    0x7a139, 0x7a13a, 0x7a13b, 0x7a13c, 0x7a13d,  /* 35-39 */
    0x7a2e4, 0x7a13e, 0x7a146, 0x7a147, 0x7a148,  /* 40-44 */
    0x7a149, 0x7a14a, 0x7a13f, 0x7a14d, 0x7a14f,  /* 45-49 */
    0x7a140, 0x7a141, 0x7a142, 0x7a143, 0x7a14b,  /* 50-54 */
    0x7a14c, 0x7a144                             /* 55-56 */
};

/* Normal mode sprite IDs (0x65xx - 0x67xx range) - from FUN_004155e0 */
static const u32 s_ui_sprites_normal[57] = {
    0x6650, 0x6715, 0x66f0, 0x673b, 0x8995,  /* 0-4 */
    0x8997, 0x6712, 0x6757, 0x66d8, 0x66d9,  /* 5-9 */
    0x6591, 0x6592, 0x6593, 0x6594, 0x6595,  /* 10-14 */
    0x6596, 0x6597, 0x6598, 0x6599, 0x65a5,  /* 15-19 */
    0x65a6, 0x65a7, 0x65a8, 0x65a9, 0x65aa,  /* 20-24 */
    0x65ab, 0x65ac, 0x65ad, 0x65b5, 0x65b6,  /* 25-29 */
    0x65b7, 0x65b8, 0x65bc, 0x65bd, 0x65c1,  /* 30-34 */
    0x65cd, 0x65d4, 0x65d6, 0x65d7, 0x65e1,  /* 35-39 */
    0x65d9, 0x65e2, 0x6658, 0x6659, 0x665b,  /* 40-44 */
    0x6676, 0x6678, 0x65ea, 0x6692, 0x66b8,  /* 45-49 */
    0x6612, 0x661a, 0x661b, 0x661c, 0x667f,  /* 50-54 */
    0x6680, 0x661d                              /* 55-56 */
};

/*
 * Initialize UI sprite IDs - FUN_004155e0 pattern
 * Different sprite IDs are used based on alpha mode (DAT_0054c83c)
 */
void render_init_ui_sprites(void) {
    if (s_alpha_mode) {
        memcpy(s_ui_sprite_ids, s_ui_sprites_alpha, sizeof(s_ui_sprites_alpha));
    } else {
        memcpy(s_ui_sprite_ids, s_ui_sprites_normal, sizeof(s_ui_sprites_normal));
    }

    LOG_INFO("UI sprites initialized for %s mode", s_alpha_mode ? "alpha" : "normal");
}

/*
 * Get UI sprite ID by index
 */
u32 render_get_ui_sprite_id(int index) {
    if (index >= 0 && index < 64) {
        return s_ui_sprite_ids[index];
    }
    return 0;
}

/* ========================================
 * Map and Character Rendering Stubs
 * ======================================== */

/*
 * Render tile
 */
void render_tile(int tile_id, int x, int y) {
    u32 sprite_id;

    /* Convert tile ID to sprite ID */
    /* Tiles are typically sprites starting from a base offset */
    sprite_id = 0x10000 + tile_id;  /* Tile sprite base offset */

    /* Add to render queue */
    render_queue_add_sprite(x, y, 0, sprite_id, 0);
}

/*
 * Render map layer
 */
void render_map_layer(int layer, int camera_x, int camera_y) {
    int tile_x, tile_y;
    int screen_x, screen_y;
    int start_tile_x, start_tile_y;
    int tile_id;

    /* Calculate visible tile range */
    start_tile_x = camera_x / 32;
    start_tile_y = camera_y / 32;

    /* Render visible tiles */
    for (tile_y = start_tile_y; tile_y < start_tile_y + 20; tile_y++) {
        for (tile_x = start_tile_x; tile_x < start_tile_x + 26; tile_x++) {
            screen_x = tile_x * 32 - camera_x;
            screen_y = tile_y * 32 - camera_y;

            /* Get tile ID from map data (would integrate with map system) */
            tile_id = 0;  /* Placeholder */

            render_tile(tile_id, screen_x, screen_y);
        }
    }
}

/*
 * Render all map layers
 */
void render_map_all(int camera_x, int camera_y) {
    int layer;

    /* Render each layer in order */
    for (layer = 0; layer < 3; layer++) {
        render_map_layer(layer, camera_x, camera_y);
    }
}

/*
 * Render character
 */
void render_character(int char_id, int x, int y, int direction, int action) {
    u32 sprite_id;
    int frame;

    /* Calculate sprite ID from character ID and action */
    /* Base character sprites start at 0x20000 */
    sprite_id = 0x20000 + char_id * 100 + action * 10 + direction;

    /* Get animation frame (would be updated by animation timer) */
    frame = 0;

    /* Render character sprite */
    render_sprite_frame(sprite_id, frame, x, y);
}

/*
 * Render NPC
 */
void render_npc(int npc_id, int x, int y) {
    u32 sprite_id;

    /* Calculate sprite ID from NPC ID */
    /* NPC sprites start at 0x30000 */
    sprite_id = 0x30000 + npc_id;

    render_sprite(sprite_id, x, y);
}

/*
 * Render player
 */
void render_player(int x, int y, int direction, int action) {
    u32 sprite_id;
    int frame;

    /* Player sprite base */
    sprite_id = 0x20000;  /* Player is character 0 */

    /* Add direction and action offset */
    sprite_id += action * 10 + direction;

    /* Get animation frame */
    frame = 0;

    render_sprite_frame(sprite_id, frame, x, y);
}

/* ========================================
 * Effects
 * ======================================== */

/*
 * Fade effect
 */
void render_fade(int alpha) {
    IDirectDrawSurface* surface;
    DDBLTFX bltfx;
    HRESULT hr;

    if (!g_render.target) return;

    /* Create a black overlay with alpha */
    memset(&bltfx, 0, sizeof(bltfx));
    bltfx.dwSize = sizeof(bltfx);
    bltfx.dwDDFX = 0;
    bltfx.dwFillColor = 0;  /* Black */

    /* Alpha blending would require a separate alpha surface */
    /* For now, this creates a solid black overlay */
    if (alpha > 200) {
        hr = IDirectDrawSurface_Blt(g_render.target, NULL, NULL, NULL,
                                    DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
    }
}

/*
 * Flash effect
 */
void render_flash(u32 color, int alpha) {
    DDBLTFX bltfx;
    HRESULT hr;

    if (!g_render.target) return;

    memset(&bltfx, 0, sizeof(bltfx));
    bltfx.dwSize = sizeof(bltfx);
    bltfx.dwFillColor = color;

    /* Flash effect - blend with color */
    if (alpha > 200) {
        hr = IDirectDrawSurface_Blt(g_render.target, NULL, NULL, NULL,
                                    DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
    }
}
