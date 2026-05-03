/*
 * Stone Age Client - Screenshot System Implementation
 * Reverse engineered from sa_9061.exe FUN_00414e30
 * Captures from DirectDraw back buffer for screenshots
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "types.h"
#include "screenshot.h"
#include "directx.h"
#include "fileutil.h"
#include "config.h"
#include "logger.h"

/* Global screenshot context */
ScreenshotContext g_screenshot = {0};

/* Pixel format detection - from DAT_0054bdec */
#define PIXEL_FORMAT_RGB565 2
#define PIXEL_FORMAT_RGB555 0

/* External globals from directx module */
extern int g_pixel_format;      /* DAT_0054bdec - pixel format (2=RGB565, 0=RGB555) */

/* External globals from config/module */
extern int g_resolution_mode;   /* DAT_04560214 - resolution mode (1=320x240, else 640x480) */

/*
 * Initialize screenshot system
 */
int screenshot_init(void) {
    memset(&g_screenshot, 0, sizeof(ScreenshotContext));

    g_screenshot.enabled = 1;
    g_screenshot.format = SCREENSHOT_BMP;
    g_screenshot.quality = 90;
    g_screenshot.counter = 0;

    /* Create screenshot directory - FUN_00414e30 pattern */
    CreateDirectoryA("screenshot", NULL);

    LOG_INFO("Screenshot system initialized");
    return 1;
}

/*
 * Shutdown screenshot system
 */
void screenshot_shutdown(void) {
    memset(&g_screenshot, 0, sizeof(ScreenshotContext));
    LOG_INFO("Screenshot system shutdown");
}

/*
 * Convert 16-bit RGB565 to 24-bit BGR - from FUN_00414e30
 * RGB565 format: RRRRRGGG GGGBBBBB
 */
static void convert_rgb565_to_bgr(const u16* src, u8* dst, int width, int height, int src_pitch) {
    const u16* src_row;
    u8* dst_row;
    int x, y;
    u16 pixel;

    /* BMP is bottom-up, so start from last row */
    for (y = 0; y < height; y++) {
        src_row = (const u16*)((const u8*)src + y * src_pitch);
        dst_row = dst + (height - 1 - y) * width * 3;

        for (x = 0; x < width; x++) {
            pixel = src_row[x];
            /* RGB565 extraction: R5, G6, B5 */
            dst_row[x * 3 + 0] = (u8)((pixel << 3) & 0xF8);      /* Blue */
            dst_row[x * 3 + 1] = (u8)((pixel >> 3) & 0xFC);      /* Green */
            dst_row[x * 3 + 2] = (u8)((pixel >> 8) & 0xF8);      /* Red */
        }
    }
}

/*
 * Convert 16-bit RGB555 to 24-bit BGR - from FUN_00414e30
 * RGB555 format: 0RRRRRGG GGGBBBBB
 */
static void convert_rgb555_to_bgr(const u16* src, u8* dst, int width, int height, int src_pitch) {
    const u16* src_row;
    u8* dst_row;
    int x, y;
    u16 pixel;

    for (y = 0; y < height; y++) {
        src_row = (const u16*)((const u8*)src + y * src_pitch);
        dst_row = dst + (height - 1 - y) * width * 3;

        for (x = 0; x < width; x++) {
            pixel = src_row[x];
            /* RGB555 extraction: R5, G5, B5 */
            dst_row[x * 3 + 0] = (u8)((pixel << 3) & 0xF8);      /* Blue */
            dst_row[x * 3 + 1] = (u8)((pixel >> 2) & 0xF8);      /* Green */
            dst_row[x * 3 + 2] = (u8)((pixel >> 7) & 0xF8);      /* Red */
        }
    }
}

/*
 * Get screen dimensions based on resolution mode - from FUN_00414e30
 * DAT_04560214: Resolution mode (1 = 320x240, else 640x480)
 */
static void get_screen_dimensions(int* width, int* height) {
    if (g_resolution_mode == 1) {
        *width = 320;
        *height = 240;
    } else {
        *width = 640;
        *height = 480;
    }
}

/*
 * Capture screen to file
 */
int screenshot_capture(const char* filename) {
    HWND hwnd;
    HDC hdc_window;
    HDC hdc_mem;
    HBITMAP hbitmap;
    BITMAPINFO bmi;
    void* bits;
    RECT rect;
    int width, height;
    int result = 0;

    if (!g_screenshot.enabled) {
        return 0;
    }

    hwnd = GetDesktopWindow();
    if (!hwnd) return 0;

    hdc_window = GetDC(hwnd);
    if (!hdc_window) return 0;

    GetWindowRect(hwnd, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    hdc_mem = CreateCompatibleDC(hdc_window);
    if (!hdc_mem) {
        ReleaseDC(hwnd, hdc_window);
        return 0;
    }

    hbitmap = CreateCompatibleBitmap(hdc_window, width, height);
    if (!hbitmap) {
        DeleteDC(hdc_mem);
        ReleaseDC(hwnd, hdc_window);
        return 0;
    }

    SelectObject(hdc_mem, hbitmap);
    BitBlt(hdc_mem, 0, 0, width, height, hdc_window, 0, 0, SRCCOPY);

    /* Setup bitmap info */
    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;  /* Top-down */
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    /* Allocate bits */
    bits = malloc(width * height * 3);
    if (bits) {
        if (GetDIBits(hdc_mem, hbitmap, 0, height, bits, &bmi, DIB_RGB_COLORS)) {
            result = screenshot_save_bmp(filename, bits, width, height);
        }
        free(bits);
    }

    DeleteObject(hbitmap);
    DeleteDC(hdc_mem);
    ReleaseDC(hwnd, hdc_window);

    return result;
}

/*
 * Capture game window using DirectX back buffer - FUN_00414e30
 * This is the main screenshot function matching original binary behavior
 */
int screenshot_capture_game(void) {
    char filename[MAX_PATH];
    SYSTEMTIME st;
    void* pixels;
    int width, height;
    int pitch;
    int bpp;
    u8* bgr_buffer;
    u32 bgr_size;
    int result;
    int index;

    if (!g_screenshot.enabled) {
        return 0;
    }

    /* Generate filename - FUN_00414e30 format: sa_MMDDHHMM_NNN.bmp */
    GetLocalTime(&st);

    for (index = g_screenshot.counter; index < 1000; index++) {
        snprintf(filename, sizeof(filename),
                 "screenshot\\sa_%02d%02d%02d_%03d.bmp",
                 st.wMonth, st.wDay, st.wHour * 100 + st.wMinute,
                 index);

        /* Check if file exists */
        if (GetFileAttributesA(filename) == INVALID_FILE_ATTRIBUTES) {
            g_screenshot.counter = index + 1;
            break;
        }
    }

    if (index >= 1000) {
        /* Fallback */
        snprintf(filename, sizeof(filename), "screenshot\\sa_%u.bmp", (u32)time(NULL));
    }

    /* Get screen dimensions */
    get_screen_dimensions(&width, &height);

    /* Lock back buffer */
    if (!graphics_lock_surface(g_graphics.back_buffer, &pixels, &pitch)) {
        LOG_WARN("Failed to lock back buffer for screenshot");
        return 0;
    }

    bpp = g_graphics.bpp;

    if (bpp == 16) {
        /* 16-bit color mode - convert to 24-bit BGR */
        bgr_size = width * height * 3;
        bgr_buffer = (u8*)malloc(bgr_size);
        if (!bgr_buffer) {
            graphics_unlock_surface(g_graphics.back_buffer);
            LOG_WARN("Failed to allocate screenshot buffer");
            return 0;
        }

        /* Check pixel format - DAT_0054bdec */
        if (g_pixel_format == PIXEL_FORMAT_RGB565) {
            convert_rgb565_to_bgr((const u16*)pixels, bgr_buffer, width, height, pitch);
        } else {
            convert_rgb555_to_bgr((const u16*)pixels, bgr_buffer, width, height, pitch);
        }

        /* Write 24-bit BMP using fileutil function */
        result = file_write_bmp_24bit(filename, bgr_buffer, width, height);

        free(bgr_buffer);
    } else if (bpp == 8) {
        /* 8-bit paletted mode */
        PALETTEENTRY palette[256];
        int num_colors = 256;

        /* Get palette */
        if (g_graphics.palette) {
            IDirectDrawPalette_GetEntries(g_graphics.palette, 0, 0, 256, palette);
        }

        /* Write paletted BMP */
        result = file_write_bmp_paletted(filename, (const u8*)pixels, 0, height,
                                         width, height, pitch, palette, num_colors);
    } else {
        LOG_WARN("Unsupported bit depth for screenshot: %d", bpp);
        result = 0;
    }

    graphics_unlock_surface(g_graphics.back_buffer);

    if (result) {
        g_screenshot.total_captures++;
        strncpy(g_screenshot.last_file, filename, sizeof(g_screenshot.last_file) - 1);
        g_screenshot.last_capture = timeGetTime();
        LOG_INFO("Screenshot saved: %s", filename);
    } else {
        LOG_WARN("Failed to save screenshot: %s", filename);
    }

    return result;
}

/*
 * Capture specific region
 */
int screenshot_capture_region(int x, int y, int width, int height, const char* filename) {
    HWND hwnd;
    HDC hdc_window;
    HDC hdc_mem;
    HBITMAP hbitmap;
    BITMAPINFO bmi;
    void* bits;
    int result = 0;

    if (!g_screenshot.enabled) {
        return 0;
    }

    hwnd = GetDesktopWindow();
    if (!hwnd) return 0;

    hdc_window = GetDC(hwnd);
    if (!hdc_window) return 0;

    hdc_mem = CreateCompatibleDC(hdc_window);
    if (!hdc_mem) {
        ReleaseDC(hwnd, hdc_window);
        return 0;
    }

    hbitmap = CreateCompatibleBitmap(hdc_window, width, height);
    if (!hbitmap) {
        DeleteDC(hdc_mem);
        ReleaseDC(hwnd, hdc_window);
        return 0;
    }

    SelectObject(hdc_mem, hbitmap);
    BitBlt(hdc_mem, 0, 0, width, height, hdc_window, x, y, SRCCOPY);

    /* Setup bitmap info */
    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    bits = malloc(width * height * 3);
    if (bits) {
        if (GetDIBits(hdc_mem, hbitmap, 0, height, bits, &bmi, DIB_RGB_COLORS)) {
            result = screenshot_save_bmp(filename, bits, width, height);
        }
        free(bits);
    }

    DeleteObject(hbitmap);
    DeleteDC(hdc_mem);
    ReleaseDC(hwnd, hdc_window);

    return result;
}

/*
 * Save BMP file
 */
int screenshot_save_bmp(const char* filename, void* bits, int width, int height) {
    FILE* fp;
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
    int row_size;
    int pixel_size;
    int i;
    u8* row_buffer;
    u8* src;
    u8* dst;

    fp = fopen(filename, "wb");
    if (!fp) {
        LOG_ERROR("Failed to create screenshot file: %s", filename);
        return 0;
    }

    row_size = (width * 3 + 3) & ~3;  /* Align to 4 bytes */
    pixel_size = row_size * height;

    /* Setup file header */
    memset(&bfh, 0, sizeof(BITMAPFILEHEADER));
    bfh.bfType = 0x4D42;  /* "BM" */
    bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + pixel_size;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    /* Setup info header */
    memset(&bih, 0, sizeof(BITMAPINFOHEADER));
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = pixel_size;

    /* Write headers */
    fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, fp);
    fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, fp);

    /* Write pixel data (with row alignment and BGR conversion) */
    row_buffer = (u8*)malloc(row_size);
    if (row_buffer) {
        for (i = 0; i < height; i++) {
            src = (u8*)bits + i * width * 3;
            dst = row_buffer;

            /* Copy and convert RGB to BGR */
            memcpy(dst, src, width * 3);

            /* Swap R and B */
            for (int j = 0; j < width; j++) {
                u8 temp = dst[j * 3];
                dst[j * 3] = dst[j * 3 + 2];
                dst[j * 3 + 2] = temp;
            }

            fwrite(row_buffer, row_size, 1, fp);
        }
        free(row_buffer);
    }

    fclose(fp);

    g_screenshot.total_captures++;
    strncpy(g_screenshot.last_file, filename, sizeof(g_screenshot.last_file) - 1);
    g_screenshot.last_capture = timeGetTime();

    LOG_INFO("Screenshot saved: %s", filename);
    return 1;
}

/*
 * Save to memory buffer
 */
int screenshot_save_memory(void* buffer, int* size, void* bits, int width, int height) {
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
    int row_size;
    int pixel_size;
    int total_size;

    if (!buffer || !size) return 0;

    row_size = (width * 3 + 3) & ~3;
    pixel_size = row_size * height;
    total_size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + pixel_size;

    if (*size < total_size) {
        *size = total_size;
        return 0;
    }

    *size = total_size;

    /* Setup headers */
    memset(&bfh, 0, sizeof(BITMAPFILEHEADER));
    bfh.bfType = 0x4D42;
    bfh.bfSize = total_size;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    memset(&bih, 0, sizeof(BITMAPINFOHEADER));
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = pixel_size;

    /* Write to buffer */
    memcpy(buffer, &bfh, sizeof(BITMAPFILEHEADER));
    memcpy((u8*)buffer + sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));

    /* Copy pixel data */
    u8* dst = (u8*)buffer + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    for (int i = 0; i < height; i++) {
        u8* src = (u8*)bits + i * width * 3;
        u8* row = dst + i * row_size;
        memcpy(row, src, width * 3);

        /* Swap R and B for BMP format */
        for (int j = 0; j < width; j++) {
            u8 temp = row[j * 3];
            row[j * 3] = row[j * 3 + 2];
            row[j * 3 + 2] = temp;
        }
    }

    return 1;
}

/*
 * Set screenshot format
 */
void screenshot_set_format(ScreenshotFormat format) {
    g_screenshot.format = format;
}

/*
 * Set screenshot quality (for JPEG)
 */
void screenshot_set_quality(u8 quality) {
    g_screenshot.quality = quality > 100 ? 100 : quality;
}

/*
 * Enable/disable screenshots
 */
void screenshot_set_enabled(int enabled) {
    g_screenshot.enabled = enabled;
}

/*
 * Check if enabled
 */
int screenshot_is_enabled(void) {
    return g_screenshot.enabled;
}

/*
 * Get last screenshot file
 */
const char* screenshot_get_last_file(void) {
    return g_screenshot.last_file;
}

/*
 * Get total capture count
 */
int screenshot_get_total_captures(void) {
    return g_screenshot.total_captures;
}

/*
 * Get screenshot directory
 */
const char* screenshot_get_directory(void) {
    return "screenshot";
}

/*
 * Clean old screenshots
 */
int screenshot_clean_old(int max_age_days) {
    WIN32_FIND_DATAA find_data;
    HANDLE hfind;
    char path[256];
    char filepath[256];
    time_t now;
    int deleted = 0;

    time(&now);

    snprintf(path, sizeof(path), "screenshot\\*.bmp");
    hfind = FindFirstFileA(path, &find_data);

    if (hfind != INVALID_HANDLE_VALUE) {
        do {
            /* Check file age */
            FILETIME ft_write;
            ULARGE_INTEGER uli;
            time_t file_time;

            ft_write = find_data.ftLastWriteTime;
            uli.LowPart = ft_write.dwLowDateTime;
            uli.HighPart = ft_write.dwHighDateTime;
            file_time = (time_t)((uli.QuadPart - 116444736000000000LL) / 10000000);

            if ((now - file_time) > (max_age_days * 24 * 60 * 60)) {
                snprintf(filepath, sizeof(filepath), "screenshot\\%s", find_data.cFileName);
                if (DeleteFileA(filepath)) {
                    deleted++;
                }
            }
        } while (FindNextFileA(hfind, &find_data));

        FindClose(hfind);
    }

    return deleted;
}

/*
 * Capture using DirectX surface
 */
int screenshot_capture_directx(void* surface, int width, int height) {
    char filename[256];
    time_t now;
    struct tm* tm_info;

    if (!g_screenshot.enabled || !surface) {
        return 0;
    }

    time(&now);
    tm_info = localtime(&now);

    snprintf(filename, sizeof(filename),
             "screenshot\\sa%02d%02d%02d_%03d.bmp",
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
             g_screenshot.counter % 1000);

    g_screenshot.counter++;

    return screenshot_save_bmp(filename, surface, width, height);
}

/*
 * Generate screenshot filename
 */
void screenshot_generate_filename(char* buffer, int size) {
    time_t now;
    struct tm* tm_info;

    time(&now);
    tm_info = localtime(&now);

    snprintf(buffer, size,
             "screenshot\\sa%02d%02d%02d_%03d.bmp",
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
             g_screenshot.counter % 1000);

    g_screenshot.counter++;
}
