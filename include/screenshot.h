/*
 * Stone Age Client - Screenshot System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include "types.h"

/* Constants */
#define SCREENSHOT_MAX_FILES    1000

/* Screenshot format */
typedef enum {
    SCREENSHOT_BMP = 0,
    SCREENSHOT_JPG = 1,
    SCREENSHOT_PNG = 2
} ScreenshotFormat;

/* Screenshot context */
typedef struct {
    int enabled;
    ScreenshotFormat format;
    u8 quality;         /* JPEG quality 1-100 */
    u32 counter;
    u32 total_captures;
    u32 last_capture;
    char last_file[256];

} ScreenshotContext;

/* Global screenshot context */
extern ScreenshotContext g_screenshot;

/* Initialization */
int screenshot_init(void);
void screenshot_shutdown(void);

/* Capture */
int screenshot_capture(const char* filename);
int screenshot_capture_game(void);
int screenshot_capture_region(int x, int y, int width, int height, const char* filename);
int screenshot_capture_directx(void* surface, int width, int height);

/* Save */
int screenshot_save_bmp(const char* filename, void* bits, int width, int height);
int screenshot_save_memory(void* buffer, int* size, void* bits, int width, int height);

/* Settings */
void screenshot_set_format(ScreenshotFormat format);
void screenshot_set_quality(u8 quality);
void screenshot_set_enabled(int enabled);
int screenshot_is_enabled(void);

/* Query */
const char* screenshot_get_last_file(void);
int screenshot_get_total_captures(void);
const char* screenshot_get_directory(void);

/* Utility */
int screenshot_clean_old(int max_age_days);
void screenshot_generate_filename(char* buffer, int size);

#endif /* SCREENSHOT_H */
