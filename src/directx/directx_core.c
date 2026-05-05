/*
 * Stone Age Client - DirectX Core
 * DirectDraw initialization, surface management, pixel format
 * Split from directx.c - FUN_00411a00, FUN_00412af0, FUN_00412220
 */

#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "directx.h"
#include "directx_internal.h"
#include "config.h"
#include "logger.h"
#include "ui.h"
#include "sound.h"
#include "input.h"

/* Global graphics context */
GraphicsContext g_graphics = {0};

/* High-res mode flag - DAT_0054c83c */
int g_high_res_mode = 0;

/* Pixel format masks */
u32 g_red_mask = 0;
u32 g_green_mask = 0;
u32 g_blue_mask = 0;
int g_red_shift = 0;
int g_green_shift = 0;
int g_blue_shift = 0;

/* Palette data */
static PALETTEENTRY g_palette[256];

/* Offscreen surface cache */
static IDirectDrawSurface* g_offscreen_surfaces[MAX_OFFSCREEN_SURFACES];
static int g_offscreen_count = 0;

/* Sprite dimensions */
int g_sprite_width = 64;
int g_sprite_height = 48;

/* DirectX function pointers */
static HRESULT (WINAPI *p_DirectDrawCreate)(GUID*, LPDIRECTDRAW*, IUnknown*) = NULL;
static HMODULE hDDraw = NULL;

/*
 * Count bits set in mask - FUN_004119e0
 */
int count_bits(u32 mask) {
    int count = 0;
    u32 bit = 1;
    int i;

    for (i = 0; i < 32; i++) {
        if (mask & bit) count++;
        bit <<= 1;
    }
    return count;
}

/*
 * Load DirectX functions dynamically
 */
int load_directx_functions(void) {
    hDDraw = LoadLibraryA("ddraw.dll");
    if (!hDDraw) {
        LOG_ERROR("Failed to load ddraw.dll");
        return 0;
    }
    p_DirectDrawCreate = (void*)GetProcAddress(hDDraw, "DirectDrawCreate");
    if (!p_DirectDrawCreate) {
        LOG_ERROR("Failed to get DirectDrawCreate");
        return 0;
    }
    return 1;
}

/*
 * Set color key for surface - FUN_00412ab0
 */
void surface_set_color_key(IDirectDrawSurface* surface) {
    DDCOLORKEY ck;
    if (!surface) return;

    memset(&ck, 0, sizeof(DDCOLORKEY));
    ck.dwColorSpaceLowValue = 0;
    ck.dwColorSpaceHighValue = 0;
    IDirectDrawSurface_SetColorKey(surface, DDCKEY_SRCBLT, &ck);
}

/*
 * Create offscreen surface - FUN_00412af0
 */
IDirectDrawSurface* graphics_create_offscreen(int width, int height, int flags) {
    DDSURFACEDESC2 ddsd;
    IDirectDrawSurface* surface;
    HRESULT hr;

    if (!g_graphics.ddraw4) return NULL;

    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    ddsd.dwWidth = width;
    ddsd.dwHeight = height;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

    /* Force 16-bit RGB565 pixel format to match original binary's rendering */
    ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
    ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
    ddsd.ddpfPixelFormat.dwRBitMask = 0xF800;
    ddsd.ddpfPixelFormat.dwGBitMask = 0x07E0;
    ddsd.ddpfPixelFormat.dwBBitMask = 0x001F;

    if (flags & SURFACE_FLAG_SYSTEM_MEMORY) {
        ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
    } else if (flags & SURFACE_FLAG_VIDEO_MEMORY) {
        ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    } else {
        ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    }

    hr = IDirectDraw4_CreateSurface(g_graphics.ddraw4, &ddsd, &surface, NULL);
    if (FAILED(hr)) {
        ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
        hr = IDirectDraw4_CreateSurface(g_graphics.ddraw4, &ddsd, &surface, NULL);
        if (FAILED(hr)) {
            LOG_WARN("CreateSurface (16-bit) failed: 0x%08X", hr);
            return NULL;
        }
    }

    surface_set_color_key(surface);
    return surface;
}

/*
 * Initialize pixel format - FUN_00411a00 pattern
 */
int graphics_init_pixel_format(void) {
    DDPIXELFORMAT ddpf;
    HRESULT hr;

    if (!g_graphics.primary_surface) return 0;

    memset(&ddpf, 0, sizeof(DDPIXELFORMAT));
    ddpf.dwSize = sizeof(DDPIXELFORMAT);

    hr = IDirectDrawSurface_GetPixelFormat(g_graphics.primary_surface, &ddpf);
    if (FAILED(hr)) {
        g_red_mask = 0xF800;
        g_green_mask = 0x07E0;
        g_blue_mask = 0x001F;
    } else {
        g_red_mask = ddpf.dwRBitMask;
        g_green_mask = ddpf.dwGBitMask;
        g_blue_mask = ddpf.dwBBitMask;
    }

    g_red_shift = 8 - count_bits(g_red_mask);
    g_green_shift = 8 - count_bits(g_green_mask);
    g_blue_shift = 8 - count_bits(g_blue_mask);

    LOG_INFO("Pixel format: R=%08X(%d), G=%08X(%d), B=%08X(%d)",
        g_red_mask, g_red_shift, g_green_mask, g_green_shift, g_blue_mask, g_blue_shift);

    return 1;
}

/*
 * Initialize default palette - FUN_00412220
 */
void graphics_init_default_palette(void) {
    int i;
    static const u8 default_colors[][3] = {
        {0x00, 0x00, 0x00}, {0x80, 0x00, 0x00}, {0x00, 0x80, 0x00}, {0x80, 0x80, 0x00},
        {0x00, 0x00, 0x80}, {0x80, 0x00, 0x80}, {0x00, 0x80, 0x80}, {0xC0, 0xC0, 0xC0},
        {0x80, 0x80, 0x80}, {0xFF, 0x00, 0x00}, {0x00, 0xFF, 0x00}, {0xFF, 0xFF, 0x00},
        {0x00, 0x00, 0xFF}, {0xFF, 0x00, 0xFF}, {0x00, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF},
    };

    for (i = 0; i < 16; i++) {
        g_palette[i].peRed = default_colors[i][0];
        g_palette[i].peGreen = default_colors[i][1];
        g_palette[i].peBlue = default_colors[i][2];
        g_palette[i].peFlags = PC_NOCOLLAPSE;
    }

    for (i = 16; i < 256; i++) {
        g_palette[i].peRed = (u8)i;
        g_palette[i].peGreen = (u8)i;
        g_palette[i].peBlue = (u8)i;
        g_palette[i].peFlags = PC_NOCOLLAPSE;
    }

    g_palette[0].peFlags = 0;  /* Color 0 is transparent */
}

/*
 * Load palette from file - FUN_00412220
 */
int graphics_load_palette(const char* path) {
    HANDLE hFile;
    DWORD bytes_read;
    u8 rgb[3];
    int i;

    hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        LOG_WARN("Failed to load palette: %s", path);
        return 0;
    }

    for (i = 0; i < 256; i++) {
        if (!ReadFile(hFile, rgb, 3, &bytes_read, NULL) || bytes_read != 3) break;
        g_palette[i].peRed = rgb[0];
        g_palette[i].peGreen = rgb[1];
        g_palette[i].peBlue = rgb[2];
        g_palette[i].peFlags = PC_NOCOLLAPSE;
    }

    CloseHandle(hFile);

    if (g_graphics.ddraw4 && g_graphics.primary_surface) {
        IDirectDraw4_CreatePalette(g_graphics.ddraw4, DDPCAPS_8BIT | DDPCAPS_ALLOW256,
            g_palette, &g_graphics.palette, NULL);
        if (g_graphics.palette) {
            IDirectDrawSurface_SetPalette(g_graphics.primary_surface, g_graphics.palette);
            if (g_graphics.back_buffer)
                IDirectDrawSurface_SetPalette(g_graphics.back_buffer, g_graphics.palette);
            if (g_graphics.offscreen_surface)
                IDirectDrawSurface_SetPalette(g_graphics.offscreen_surface, g_graphics.palette);
        }
    }

    LOG_INFO("Loaded palette from %s", path);
    return 1;
}

/*
 * Allocate graphics context - FUN_00440170
 */
int graphics_allocate_context(int screen_mode) {
    switch (screen_mode) {
        case 0: case 1: case 2:
            g_graphics.width = 640;
            g_graphics.height = 480;
            g_sprite_width = 64;
            g_sprite_height = 48;
            break;
        default:
            g_graphics.width = 640;
            g_graphics.height = 480;
            g_sprite_width = 64;
            g_sprite_height = 48;
            break;
    }

    LOG_INFO("Graphics context: %dx%d, sprite: %dx%d",
             g_graphics.width, g_graphics.height, g_sprite_width, g_sprite_height);
    return 1;
}

/*
 * Graphics Initialization - FUN_00411a00
 */
int graphics_init(HWND hWnd, int window_mode) {
    HRESULT hr;
    HDC hdc;
    DDSURFACEDESC2 ddsd;
    DDSCAPS2 caps;

    memset(&g_graphics, 0, sizeof(GraphicsContext));

    hdc = GetDC(hWnd);
    g_graphics.bpp = GetDeviceCaps(hdc, BITSPIXEL);
    ReleaseDC(hWnd, hdc);

    LOG_INFO("Initializing DirectDraw, color depth: %d, mode: %s",
             g_graphics.bpp, window_mode ? "windowed" : "fullscreen");

    hr = p_DirectDrawCreate(NULL, &g_graphics.ddraw, NULL);
    if (FAILED(hr)) {
        LOG_ERROR("DirectDrawCreate failed: 0x%08X, trying secondary", hr);
        hr = p_DirectDrawCreate((GUID*)2, &g_graphics.ddraw, NULL);
        if (FAILED(hr)) {
            LOG_ERROR("DirectDrawCreate secondary also failed: 0x%08X", hr);
            return 0;
        }
    }
    LOG_INFO("DirectDrawCreate succeeded");

    hr = IDirectDraw_QueryInterface(g_graphics.ddraw, &IID_IDirectDraw4, (void**)&g_graphics.ddraw4);
    if (FAILED(hr)) {
        LOG_ERROR("QueryInterface IDirectDraw4 failed: 0x%08X", hr);
        return 0;
    }
    LOG_INFO("QueryInterface IDirectDraw4 succeeded");

    if (window_mode == SCREEN_MODE_FULLSCREEN) {
        g_graphics.bpp = 16;
        g_graphics.width = 640;
        g_graphics.height = 480;

        hr = IDirectDraw4_SetCooperativeLevel(g_graphics.ddraw4, hWnd,
            DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
        if (FAILED(hr)) {
            MessageBoxA(hWnd, "SetCooperativeLevel Error", "Error", MB_OK | MB_ICONERROR);
            return 0;
        }

        hr = IDirectDraw4_SetDisplayMode(g_graphics.ddraw4, g_graphics.width, g_graphics.height,
            g_graphics.bpp, 0, 0);
        if (FAILED(hr)) {
            MessageBoxA(hWnd, "SetDisplayMode failed", "Error", MB_OK | MB_ICONERROR);
            return 0;
        }
    } else {
        hr = IDirectDraw4_SetCooperativeLevel(g_graphics.ddraw4, hWnd, DDSCL_NORMAL);
        if (FAILED(hr)) {
            MessageBoxA(hWnd, "SetCooperativeLevel Error", "Error", MB_OK | MB_ICONERROR);
            return 0;
        }
        g_graphics.width = 640;
        g_graphics.height = 480;
    }

    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    if (window_mode == SCREEN_MODE_FULLSCREEN) {
        ddsd.dwFlags |= DDSD_BACKBUFFERCOUNT;
        ddsd.dwBackBufferCount = 1;
        ddsd.ddsCaps.dwCaps |= DDSCAPS_COMPLEX | DDSCAPS_FLIP;
    }

    hr = IDirectDraw4_CreateSurface(g_graphics.ddraw4, &ddsd, &g_graphics.primary_surface, NULL);
    if (FAILED(hr)) {
        LOG_ERROR("CreateSurface (primary) failed: 0x%08X", hr);
        return 0;
    }
    LOG_INFO("Primary surface created");

    if (window_mode == SCREEN_MODE_FULLSCREEN) {
        memset(&caps, 0, sizeof(DDSCAPS2));
        caps.dwCaps = DDSCAPS_BACKBUFFER;
        hr = IDirectDrawSurface_GetAttachedSurface(g_graphics.primary_surface, &caps,
            &g_graphics.back_buffer);
        if (FAILED(hr)) {
            MessageBoxA(hWnd, "Create backbuffer error", "Error", MB_OK | MB_ICONERROR);
            return 0;
        }
    } else {
        hr = IDirectDraw4_CreateClipper(g_graphics.ddraw4, 0, &g_graphics.clipper, NULL);
        if (SUCCEEDED(hr)) {
            IDirectDrawClipper_SetHWnd(g_graphics.clipper, 0, hWnd);
            IDirectDrawSurface_SetClipper(g_graphics.primary_surface, g_graphics.clipper);
        }
    }

    graphics_init_pixel_format();
    graphics_init_default_palette();
    graphics_load_palette("data/pal/Palet_1.sap");

    g_graphics.offscreen_surface = graphics_create_offscreen(g_graphics.width, g_graphics.height, SURFACE_FLAG_VIDEO_MEMORY);
    if (!g_graphics.offscreen_surface) {
        LOG_WARN("Video memory offscreen failed, trying system memory");
        g_graphics.offscreen_surface = graphics_create_offscreen(g_graphics.width, g_graphics.height, SURFACE_FLAG_SYSTEM_MEMORY);
        if (!g_graphics.offscreen_surface) {
            LOG_ERROR("Failed to create offscreen surface");
            return 0;
        }
    }
    LOG_INFO("Offscreen surface created (16-bit RGB565)");

    /* Set internal bpp to 16 for rendering code */
    g_graphics.bpp = 16;

    if (g_alpha_mode) {
        g_graphics.alpha_surface = graphics_create_offscreen(g_graphics.width, g_graphics.height, SURFACE_FLAG_SYSTEM_MEMORY);
    }

    LOG_INFO("Graphics initialized: %dx%d %dbpp", g_graphics.width, g_graphics.height, g_graphics.bpp);
    return 1;
}

void graphics_set_palette(void) {
    if (g_graphics.palette && g_graphics.primary_surface) {
        IDirectDrawSurface_SetPalette(g_graphics.primary_surface, g_graphics.palette);
    }
}

void graphics_flip(void) {
    HRESULT hr;

    if (g_graphics.back_buffer) {
        hr = IDirectDrawSurface_Flip(g_graphics.primary_surface, NULL, DDFLIP_WAIT);
        if (FAILED(hr)) LOG_WARN("Flip failed: 0x%08X", hr);
    } else if (g_graphics.offscreen_surface && g_graphics.primary_surface) {
        RECT src_rect = {0, 0, g_graphics.width, g_graphics.height};
        RECT dst_rect;
        HWND hwnd = GetActiveWindow();

        GetClientRect(hwnd, &dst_rect);
        ClientToScreen(hwnd, (POINT*)&dst_rect.left);
        ClientToScreen(hwnd, (POINT*)&dst_rect.right);

        hr = IDirectDrawSurface_Blt(g_graphics.primary_surface, &dst_rect,
            g_graphics.offscreen_surface, &src_rect, DDBLT_WAIT, NULL);
    }
}

void graphics_clear(int color) {
    DDBLTFX bltfx;
    HRESULT hr;

    memset(&bltfx, 0, sizeof(DDBLTFX));
    bltfx.dwSize = sizeof(DDBLTFX);
    bltfx.dwFillColor = color;

    if (g_graphics.back_buffer)
        hr = IDirectDrawSurface_Blt(g_graphics.back_buffer, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
    if (g_graphics.offscreen_surface)
        hr = IDirectDrawSurface_Blt(g_graphics.offscreen_surface, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
}

void graphics_shutdown(void) {
    int i;

    for (i = 0; i < g_offscreen_count; i++) {
        if (g_offscreen_surfaces[i]) {
            IDirectDrawSurface_Release(g_offscreen_surfaces[i]);
            g_offscreen_surfaces[i] = NULL;
        }
    }
    g_offscreen_count = 0;

    if (g_graphics.alpha_surface) {
        IDirectDrawSurface_Release(g_graphics.alpha_surface);
        g_graphics.alpha_surface = NULL;
    }
    if (g_graphics.clipper) {
        IDirectDrawClipper_Release(g_graphics.clipper);
        g_graphics.clipper = NULL;
    }
    if (g_graphics.palette) {
        IDirectDrawPalette_Release(g_graphics.palette);
        g_graphics.palette = NULL;
    }
    if (g_graphics.offscreen_surface) {
        IDirectDrawSurface_Release(g_graphics.offscreen_surface);
        g_graphics.offscreen_surface = NULL;
    }
    if (g_graphics.back_buffer) {
        IDirectDrawSurface_Release(g_graphics.back_buffer);
        g_graphics.back_buffer = NULL;
    }
    if (g_graphics.primary_surface) {
        IDirectDrawSurface_Release(g_graphics.primary_surface);
        g_graphics.primary_surface = NULL;
    }
    if (g_graphics.ddraw4) {
        IDirectDraw4_RestoreDisplayMode(g_graphics.ddraw4);
        IDirectDraw4_Release(g_graphics.ddraw4);
        g_graphics.ddraw4 = NULL;
    }
    if (g_graphics.ddraw) {
        IDirectDraw_Release(g_graphics.ddraw);
        g_graphics.ddraw = NULL;
    }

    if (hDDraw) {
        FreeLibrary(hDDraw);
        hDDraw = NULL;
    }

    LOG_INFO("Graphics shutdown complete");
}

void graphics_blit(IDirectDrawSurface* src, int src_x, int src_y, int src_w, int src_h,
                   IDirectDrawSurface* dst, int dst_x, int dst_y) {
    RECT src_rect, dst_rect;
    HRESULT hr;

    if (!src || !dst) return;

    src_rect.left = src_x; src_rect.top = src_y;
    src_rect.right = src_x + src_w; src_rect.bottom = src_y + src_h;
    dst_rect.left = dst_x; dst_rect.top = dst_y;
    dst_rect.right = dst_x + src_w; dst_rect.bottom = dst_y + src_h;

    hr = IDirectDrawSurface_Blt(dst, &dst_rect, src, &src_rect, DDBLT_WAIT, NULL);
}

int graphics_lock_surface(IDirectDrawSurface* surface, void** pixels, int* pitch) {
    DDSURFACEDESC2 ddsd;
    HRESULT hr;

    if (!surface || !pixels || !pitch) return 0;

    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);

    hr = IDirectDrawSurface_Lock(surface, NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
    if (FAILED(hr)) return 0;

    *pixels = ddsd.lpSurface;
    *pitch = ddsd.lPitch;
    return 1;
}

void graphics_unlock_surface(IDirectDrawSurface* surface) {
    if (surface) IDirectDrawSurface_Unlock(surface, NULL);
}

void graphics_set_alpha_mode(int enabled) {
    g_alpha_mode = enabled;
}

int graphics_get_alpha_mode(void) {
    return g_alpha_mode;
}

/*
 * Get surface pitch
 */
int graphics_get_surface_pitch(IDirectDrawSurface* surface) {
    DDSURFACEDESC2 ddsd;
    HRESULT hr;

    if (!surface) return 0;

    memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);

    hr = IDirectDrawSurface_GetSurfaceDesc(surface, &ddsd);
    if (FAILED(hr)) {
        return 0;
    }

    return ddsd.lPitch;
}

/*
 * Get surface pixels (locks surface temporarily)
 */
void* graphics_get_surface_pixels(IDirectDrawSurface* surface) {
    void* pixels;
    int pitch;

    if (graphics_lock_surface(surface, &pixels, &pitch)) {
        return pixels;
    }
    return NULL;
}

/*
 * Create multiple offscreen surfaces for sprite cache
 */
int graphics_create_offscreen_array(int width, int height, int count, int flags) {
    int i;

    for (i = 0; i < count && i < MAX_OFFSCREEN_SURFACES; i++) {
        g_offscreen_surfaces[i] = graphics_create_offscreen(width, height, flags);
        if (!g_offscreen_surfaces[i]) {
            LOG_WARN("Failed to create offscreen surface %d", i);
            return 0;
        }
    }

    g_offscreen_count = i;
    LOG_INFO("Created %d offscreen surfaces (%dx%d)", i, width, height);
    return 1;
}

/*
 * Create offscreen surface wrapper
 */
void* offscreen_create(int width, int height, int flags) {
    return graphics_create_offscreen(width, height, flags);
}

/*
 * Main DirectX initialization
 */
int directx_init(HWND hWnd, int window_mode) {
    if (!load_directx_functions()) {
        return 0;
    }

    if (!graphics_init(hWnd, window_mode)) {
        return 0;
    }

    /* Initialize sound system */
    sound_init(hWnd);

    /* Initialize input system */
    input_init(GetModuleHandle(NULL), hWnd);

    /* Initialize UI sprites based on resolution mode */
    ui_init_sprites(g_high_res_mode);

    return 1;
}

/*
 * Main DirectX shutdown
 */
void directx_shutdown(void) {
    input_shutdown();
    sound_shutdown();
    graphics_shutdown();

    if (hDDraw) {
        FreeLibrary(hDDraw);
        hDDraw = NULL;
    }
}
