/*
 * Stone Age Client - Sprite Core System
 * Split from sprite.c for code organization
 *
 * Handles initialization, color conversion, and core functionality
 */

#include <windows.h>
#include <string.h>
#include "types.h"
#include "sprite.h"
#include "logger.h"

/* Global sprite context */
SpriteContext g_sprite_ctx = {0};

/* Pixel format detection - from DAT_0054bdec */
static int g_pixel_format = PIXEL_FORMAT_565;  /* Default to 565 */

/* Transparent color key (magic pink) */
#define TRANSPARENT_COLOR_565   0xF81F
#define TRANSPARENT_COLOR_555   0x7C1F

/*
 * Initialize sprite system
 */
int sprite_init(void) {
    memset(&g_sprite_ctx, 0, sizeof(SpriteContext));
    g_sprite_ctx.pixel_format = PIXEL_FORMAT_565;

    LOG_INFO("Sprite system initialized");
    return 1;
}

/*
 * Shutdown sprite system
 */
void sprite_shutdown(void) {
    memset(&g_sprite_ctx, 0, sizeof(SpriteContext));
    LOG_INFO("Sprite system shutdown");
}

/*
 * Set pixel format
 */
void sprite_set_pixel_format(int format) {
    g_pixel_format = format;
    g_sprite_ctx.pixel_format = format;
}

/*
 * Get pixel format
 */
int sprite_get_pixel_format(void) {
    return g_pixel_format;
}

/*
 * Convert RGB to 565 format
 */
u16 sprite_color_565(u8 r, u8 g, u8 b) {
    return ((u16)(r & 0xF8) << 8) | ((u16)(g & 0xFC) << 3) | ((u16)(b & 0xF8) >> 3);
}

/*
 * Convert RGB to 555 format
 */
u16 sprite_color_555(u8 r, u8 g, u8 b) {
    return ((u16)(r & 0xF8) << 7) | ((u16)(g & 0xF8) << 2) | ((u16)(b & 0xF8) >> 3);
}

/*
 * Split 565 color to RGB
 */
void sprite_color_split_565(u16 color, u8* r, u8* g, u8* b) {
    if (r) *r = (u8)((color >> 8) & 0xF8);
    if (g) *g = (u8)((color >> 3) & 0xFC);
    if (b) *b = (u8)((color << 3) & 0xF8);
}

/*
 * Split 555 color to RGB
 */
void sprite_color_split_555(u16 color, u8* r, u8* g, u8* b) {
    if (r) *r = (u8)((color >> 7) & 0xF8);
    if (g) *g = (u8)((color >> 2) & 0xF8);
    if (b) *b = (u8)((color << 3) & 0xF8);
}

/*
 * Get transparent color for current pixel format
 */
u16 sprite_get_transparent_color(void) {
    return (g_pixel_format == PIXEL_FORMAT_565) ?
           TRANSPARENT_COLOR_565 : TRANSPARENT_COLOR_555;
}
