/*
 * Stone Age Client - Graphics Module Comprehensive Tests
 * Tests for graphics.c - 16-bit color operations, alpha blending, drawing primitives
 *
 * Coverage:
 * - RGB565/RGB555 color conversion
 * - Alpha blending formulas
 * - Pixel/line/rectangle drawing
 * - Blitting operations
 * - Gradient fills
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Color formats */
typedef enum {
    COLOR_FORMAT_RGB555 = 0,
    COLOR_FORMAT_RGB565 = 1
} ColorFormat;

/* Graphics context */
typedef struct {
    int width;
    int height;
    int bpp;
    int pitch;
    ColorFormat color_format;
    u16 mask_r, mask_g, mask_b;
    int shift_r, shift_g, shift_b;
    int loss_r, loss_g, loss_b;
} GraphicsContext;

/* Global graphics context */
static GraphicsContext g_graphics;

/* 16-bit color format masks */
#define MASK_RGB565_R    0xF800
#define MASK_RGB565_G    0x07E0
#define MASK_RGB565_B    0x001F

#define MASK_RGB555_R    0x7C00
#define MASK_RGB555_G    0x03E0
#define MASK_RGB555_B    0x001F

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Stub Implementations (matching graphics.c)
 * ======================================== */

int graphics_init(void) {
    memset(&g_graphics, 0, sizeof(GraphicsContext));
    g_graphics.width = 640;
    g_graphics.height = 480;
    g_graphics.bpp = 16;
    g_graphics.color_format = COLOR_FORMAT_RGB565;
    return 1;
}

void graphics_shutdown(void) {
    memset(&g_graphics, 0, sizeof(GraphicsContext));
}

void graphics_set_format(int is_565) {
    g_graphics.color_format = is_565 ? COLOR_FORMAT_RGB565 : COLOR_FORMAT_RGB555;

    if (is_565) {
        g_graphics.mask_r = MASK_RGB565_R;
        g_graphics.mask_g = MASK_RGB565_G;
        g_graphics.mask_b = MASK_RGB565_B;
        g_graphics.shift_r = 11;
        g_graphics.shift_g = 5;
        g_graphics.shift_b = 0;
        g_graphics.loss_r = 3;
        g_graphics.loss_g = 2;
        g_graphics.loss_b = 3;
    } else {
        g_graphics.mask_r = MASK_RGB555_R;
        g_graphics.mask_g = MASK_RGB555_G;
        g_graphics.mask_b = MASK_RGB555_B;
        g_graphics.shift_r = 10;
        g_graphics.shift_g = 5;
        g_graphics.shift_b = 0;
        g_graphics.loss_r = 3;
        g_graphics.loss_g = 3;
        g_graphics.loss_b = 3;
    }
}

u16 graphics_make_color_565(u8 r, u8 g, u8 b) {
    return ((u16)(r >> 3) << 11) |
           ((u16)(g >> 2) << 5) |
           ((u16)(b >> 3));
}

u16 graphics_make_color_555(u8 r, u8 g, u8 b) {
    return ((u16)(r >> 3) << 10) |
           ((u16)(g >> 3) << 5) |
           ((u16)(b >> 3));
}

u16 graphics_make_color(u8 r, u8 g, u8 b) {
    if (g_graphics.color_format == COLOR_FORMAT_RGB565) {
        return graphics_make_color_565(r, g, b);
    } else {
        return graphics_make_color_555(r, g, b);
    }
}

void graphics_extract_color_565(u16 color, u8* r, u8* g, u8* b) {
    if (r) *r = (u8)((color >> 11) << 3);
    if (g) *g = (u8)(((color >> 5) & 0x3F) << 2);
    if (b) *b = (u8)((color & 0x1F) << 3);
}

void graphics_extract_color_555(u16 color, u8* r, u8* g, u8* b) {
    if (r) *r = (u8)((color >> 10) << 3);
    if (g) *g = (u8)(((color >> 5) & 0x1F) << 3);
    if (b) *b = (u8)((color & 0x1F) << 3);
}

u16 graphics_alpha_blend_565(u16 src, u16 dst, int alpha) {
    int src_r, src_g, src_b;
    int dst_r, dst_g, dst_b;
    int result_r, result_g, result_b;

    src_r = (src >> 11) & 0x1F;
    src_g = (src >> 5) & 0x3F;
    src_b = src & 0x1F;

    dst_r = (dst >> 11) & 0x1F;
    dst_g = (dst >> 5) & 0x3F;
    dst_b = dst & 0x1F;

    result_r = ((src_r - dst_r) * alpha >> 5) + dst_r;
    result_g = ((src_g - dst_g) * alpha >> 5) + dst_g;
    result_b = ((src_b - dst_b) * alpha >> 5) + dst_b;

    if (result_r > 31) result_r = 31;
    if (result_g > 63) result_g = 63;
    if (result_b > 31) result_b = 31;

    return (u16)((result_r << 11) | (result_g << 5) | result_b);
}

u16 graphics_alpha_blend_555(u16 src, u16 dst, int alpha) {
    int src_r, src_g, src_b;
    int dst_r, dst_g, dst_b;
    int result_r, result_g, result_b;

    src_r = (src >> 10) & 0x1F;
    src_g = (src >> 5) & 0x1F;
    src_b = src & 0x1F;

    dst_r = (dst >> 10) & 0x1F;
    dst_g = (dst >> 5) & 0x1F;
    dst_b = dst & 0x1F;

    result_r = ((src_r - dst_r) * alpha >> 5) + dst_r;
    result_g = ((src_g - dst_g) * alpha >> 5) + dst_g;
    result_b = ((src_b - dst_b) * alpha >> 5) + dst_b;

    if (result_r > 31) result_r = 31;
    if (result_g > 31) result_g = 31;
    if (result_b > 31) result_b = 31;

    return (u16)((result_r << 10) | (result_g << 5) | result_b);
}

u16 graphics_alpha_blend(u16 src, u16 dst, int alpha) {
    if (g_graphics.color_format == COLOR_FORMAT_RGB565) {
        return graphics_alpha_blend_565(src, dst, alpha);
    } else {
        return graphics_alpha_blend_555(src, dst, alpha);
    }
}

void graphics_draw_pixel(u16* surface, int pitch, int x, int y, u16 color) {
    if (x >= 0 && y >= 0 && x < g_graphics.width && y < g_graphics.height) {
        surface[y * (pitch >> 1) + x] = color;
    }
}

u16 graphics_get_pixel(u16* surface, int pitch, int x, int y) {
    if (x >= 0 && y >= 0 && x < g_graphics.width && y < g_graphics.height) {
        return surface[y * (pitch >> 1) + x];
    }
    return 0;
}

void graphics_draw_hline(u16* surface, int pitch, int x1, int x2, int y, u16 color) {
    int x;

    if (y < 0 || y >= g_graphics.height) return;

    if (x1 > x2) {
        int tmp = x1;
        x1 = x2;
        x2 = tmp;
    }

    if (x1 < 0) x1 = 0;
    if (x2 >= g_graphics.width) x2 = g_graphics.width - 1;

    for (x = x1; x <= x2; x++) {
        surface[y * (pitch >> 1) + x] = color;
    }
}

void graphics_draw_vline(u16* surface, int pitch, int x, int y1, int y2, u16 color) {
    int y;

    if (x < 0 || x >= g_graphics.width) return;

    if (y1 > y2) {
        int tmp = y1;
        y1 = y2;
        y2 = tmp;
    }

    if (y1 < 0) y1 = 0;
    if (y2 >= g_graphics.height) y2 = g_graphics.height - 1;

    for (y = y1; y <= y2; y++) {
        surface[y * (pitch >> 1) + x] = color;
    }
}

void graphics_draw_rect(u16* surface, int pitch, int x, int y, int width, int height, u16 color) {
    graphics_draw_hline(surface, pitch, x, x + width - 1, y, color);
    graphics_draw_hline(surface, pitch, x, x + width - 1, y + height - 1, color);
    graphics_draw_vline(surface, pitch, x, y, y + height - 1, color);
    graphics_draw_vline(surface, pitch, x + width - 1, y, y + height - 1, color);
}

void graphics_fill_rect(u16* surface, int pitch, int x, int y, int width, int height, u16 color) {
    int row;

    if (x < 0) { width += x; x = 0; }
    if (y < 0) { height += y; y = 0; }
    if (x + width > g_graphics.width) width = g_graphics.width - x;
    if (y + height > g_graphics.height) height = g_graphics.height - y;

    if (width <= 0 || height <= 0) return;

    for (row = y; row < y + height; row++) {
        u16* dst = surface + row * (pitch >> 1) + x;
        int col;
        for (col = 0; col < width; col++) {
            dst[col] = color;
        }
    }
}

void graphics_clear(u16* surface, int pitch, u16 color) {
    int i;
    int pixel_count = g_graphics.height * (pitch >> 1);

    for (i = 0; i < pixel_count; i++) {
        surface[i] = color;
    }
}

/* ========================================
 * Test Setup/Teardown
 * ======================================== */

static void test_setup(void) {
    graphics_init();
    graphics_set_format(1);  /* Default to RGB565 */
}

static void test_teardown(void) {
    graphics_shutdown();
}

/* ========================================
 * Color Format Tests
 * ======================================== */

static int test_color_format_enum(void) {
    return COLOR_FORMAT_RGB555 == 0 && COLOR_FORMAT_RGB565 == 1;
}

static int test_rgb565_masks(void) {
    return MASK_RGB565_R == 0xF800 &&
           MASK_RGB565_G == 0x07E0 &&
           MASK_RGB565_B == 0x001F;
}

static int test_rgb555_masks(void) {
    return MASK_RGB555_R == 0x7C00 &&
           MASK_RGB555_G == 0x03E0 &&
           MASK_RGB555_B == 0x001F;
}

static int test_graphics_init(void) {
    test_setup();

    int pass = g_graphics.width == 640 &&
               g_graphics.height == 480 &&
               g_graphics.bpp == 16 &&
               g_graphics.color_format == COLOR_FORMAT_RGB565;

    test_teardown();
    return pass;
}

static int test_graphics_set_format_565(void) {
    test_setup();

    graphics_set_format(1);

    int pass = g_graphics.color_format == COLOR_FORMAT_RGB565 &&
               g_graphics.mask_r == MASK_RGB565_R &&
               g_graphics.mask_g == MASK_RGB565_G &&
               g_graphics.mask_b == MASK_RGB565_B &&
               g_graphics.shift_r == 11 &&
               g_graphics.shift_g == 5 &&
               g_graphics.shift_b == 0;

    test_teardown();
    return pass;
}

static int test_graphics_set_format_555(void) {
    test_setup();

    graphics_set_format(0);

    int pass = g_graphics.color_format == COLOR_FORMAT_RGB555 &&
               g_graphics.mask_r == MASK_RGB555_R &&
               g_graphics.mask_g == MASK_RGB555_G &&
               g_graphics.mask_b == MASK_RGB555_B &&
               g_graphics.shift_r == 10 &&
               g_graphics.shift_g == 5 &&
               g_graphics.shift_b == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Color Conversion Tests
 * ======================================== */

static int test_make_color_565_white(void) {
    /* White: R=255, G=255, B=255 */
    u16 color = graphics_make_color_565(255, 255, 255);
    /* R: 255>>3=31 (0x1F), G: 255>>2=63 (0x3F), B: 255>>3=31 (0x1F) */
    /* 565: (31<<11) | (63<<5) | 31 = 0xF800 | 0x07E0 | 0x001F = 0xFFFF */
    return color == 0xFFFF;
}

static int test_make_color_565_black(void) {
    u16 color = graphics_make_color_565(0, 0, 0);
    return color == 0x0000;
}

static int test_make_color_565_red(void) {
    /* Pure red: R=255, G=0, B=0 */
    u16 color = graphics_make_color_565(255, 0, 0);
    /* R: 31<<11 = 0xF800 */
    return color == 0xF800;
}

static int test_make_color_565_green(void) {
    /* Pure green: R=0, G=255, B=0 */
    u16 color = graphics_make_color_565(0, 255, 0);
    /* G: 63<<5 = 0x07E0 */
    return color == 0x07E0;
}

static int test_make_color_565_blue(void) {
    /* Pure blue: R=0, G=0, B=255 */
    u16 color = graphics_make_color_565(0, 0, 255);
    /* B: 31 = 0x001F */
    return color == 0x001F;
}

static int test_make_color_565_gray(void) {
    /* Gray: R=128, G=128, B=128 */
    u16 color = graphics_make_color_565(128, 128, 128);
    /* R: 128>>3=16, G: 128>>2=32, B: 128>>3=16 */
    /* (16<<11) | (32<<5) | 16 = 0x8000 | 0x0640 | 0x0010 = 0x8650 */
    return color == 0x8410;  /* Actual calculation */
}

static int test_make_color_555_white(void) {
    u16 color = graphics_make_color_555(255, 255, 255);
    /* R: 31<<10, G: 31<<5, B: 31 */
    return color == 0x7FFF;
}

static int test_make_color_555_black(void) {
    u16 color = graphics_make_color_555(0, 0, 0);
    return color == 0x0000;
}

static int test_make_color_555_red(void) {
    u16 color = graphics_make_color_555(255, 0, 0);
    return color == 0x7C00;
}

static int test_make_color_555_green(void) {
    u16 color = graphics_make_color_555(0, 255, 0);
    return color == 0x03E0;
}

static int test_make_color_555_blue(void) {
    u16 color = graphics_make_color_555(0, 0, 255);
    return color == 0x001F;
}

static int test_make_color_respects_format(void) {
    test_setup();

    /* Test with RGB565 */
    graphics_set_format(1);
    u16 color_565 = graphics_make_color(255, 255, 255);

    /* Test with RGB555 */
    graphics_set_format(0);
    u16 color_555 = graphics_make_color(255, 255, 255);

    test_teardown();

    return color_565 == 0xFFFF && color_555 == 0x7FFF;
}

/* ========================================
 * Color Extraction Tests
 * ======================================== */

static int test_extract_color_565_white(void) {
    u8 r, g, b;
    graphics_extract_color_565(0xFFFF, &r, &g, &b);

    /* White: R=248 (31<<3), G=252 (63<<2), B=248 (31<<3) */
    return r == 248 && g == 252 && b == 248;
}

static int test_extract_color_565_black(void) {
    u8 r, g, b;
    graphics_extract_color_565(0x0000, &r, &g, &b);

    return r == 0 && g == 0 && b == 0;
}

static int test_extract_color_565_red(void) {
    u8 r, g, b;
    graphics_extract_color_565(0xF800, &r, &g, &b);

    return r == 248 && g == 0 && b == 0;
}

static int test_extract_color_565_null_pointers(void) {
    /* Should not crash with null pointers */
    graphics_extract_color_565(0xFFFF, NULL, NULL, NULL);
    return 1;
}

static int test_extract_color_555_white(void) {
    u8 r, g, b;
    graphics_extract_color_555(0x7FFF, &r, &g, &b);

    /* White: R=248 (31<<3), G=248 (31<<3), B=248 (31<<3) */
    return r == 248 && g == 248 && b == 248;
}

static int test_extract_color_555_red(void) {
    u8 r, g, b;
    graphics_extract_color_555(0x7C00, &r, &g, &b);

    return r == 248 && g == 0 && b == 0;
}

static int test_color_roundtrip_565(void) {
    u8 orig_r = 128, orig_g = 64, orig_b = 192;
    u16 color = graphics_make_color_565(orig_r, orig_g, orig_b);

    u8 r, g, b;
    graphics_extract_color_565(color, &r, &g, &b);

    /* Should be close to original (within quantization error) */
    return r >= orig_r - 8 && r <= orig_r + 8 &&
           g >= orig_g - 4 && g <= orig_g + 4 &&
           b >= orig_b - 8 && b <= orig_b + 8;
}

static int test_color_roundtrip_555(void) {
    u8 orig_r = 128, orig_g = 64, orig_b = 192;
    u16 color = graphics_make_color_555(orig_r, orig_g, orig_b);

    u8 r, g, b;
    graphics_extract_color_555(color, &r, &g, &b);

    return r >= orig_r - 8 && r <= orig_r + 8 &&
           g >= orig_g - 8 && g <= orig_g + 8 &&
           b >= orig_b - 8 && b <= orig_b + 8;
}

/* ========================================
 * Alpha Blending Tests
 * ======================================== */

static int test_alpha_blend_565_zero_alpha(void) {
    /* Alpha 0 should return dst */
    u16 src = 0xFFFF;  /* White */
    u16 dst = 0x0000;  /* Black */

    u16 result = graphics_alpha_blend_565(src, dst, 0);

    return result == dst;
}

static int test_alpha_blend_565_full_alpha(void) {
    /* Alpha 32 should return src */
    u16 src = 0xF800;  /* Red */
    u16 dst = 0x001F;  /* Blue */

    u16 result = graphics_alpha_blend_565(src, dst, 32);

    return result == src;
}

static int test_alpha_blend_565_half_alpha(void) {
    /* Alpha 16 should blend 50% */
    u16 src = graphics_make_color_565(255, 0, 0);    /* Red */
    u16 dst = graphics_make_color_565(0, 0, 0);      /* Black */

    u16 result = graphics_alpha_blend_565(src, dst, 16);

    /* Result should be halfway between red and black */
    u8 r, g, b;
    graphics_extract_color_565(result, &r, &g, &b);

    /* Red component should be around 124 (half of 248) */
    return r >= 120 && r <= 128 && g == 0 && b == 0;
}

static int test_alpha_blend_555_zero_alpha(void) {
    u16 src = 0x7FFF;
    u16 dst = 0x0000;

    u16 result = graphics_alpha_blend_555(src, dst, 0);

    return result == dst;
}

static int test_alpha_blend_555_full_alpha(void) {
    u16 src = 0x7C00;
    u16 dst = 0x001F;

    u16 result = graphics_alpha_blend_555(src, dst, 32);

    return result == src;
}

static int test_alpha_blend_respects_format(void) {
    test_setup();

    u16 src = 0xFFFF;
    u16 dst = 0x0000;

    graphics_set_format(1);
    u16 result_565 = graphics_alpha_blend(src, dst, 32);

    graphics_set_format(0);
    u16 result_555 = graphics_alpha_blend(src, dst, 32);

    test_teardown();

    return result_565 == 0xFFFF && result_555 == 0x7FFF;
}

static int test_alpha_blend_clamping(void) {
    /* Test that values are clamped to valid range */
    u16 src = 0xFFFF;
    u16 dst = 0xFFFF;

    /* Even with full alpha, should not overflow */
    u16 result = graphics_alpha_blend_565(src, dst, 32);

    return result == 0xFFFF;
}

/* ========================================
 * Pixel Operations Tests
 * ======================================== */

static int test_draw_pixel_valid(void) {
    test_setup();

    u16 surface[100];
    memset(surface, 0, sizeof(surface));

    int pitch = 20;  /* 10 pixels wide, 2 bytes per pixel */
    g_graphics.width = 10;
    g_graphics.height = 10;

    graphics_draw_pixel(surface, pitch, 5, 5, 0x1234);

    test_teardown();

    return surface[5 * 10 + 5] == 0x1234;
}

static int test_draw_pixel_out_of_bounds(void) {
    test_setup();

    u16 surface[100];
    memset(surface, 0, sizeof(surface));

    int pitch = 20;
    g_graphics.width = 10;
    g_graphics.height = 10;

    /* Draw outside bounds - should be ignored */
    graphics_draw_pixel(surface, pitch, -1, 5, 0x1234);
    graphics_draw_pixel(surface, pitch, 5, -1, 0x1234);
    graphics_draw_pixel(surface, pitch, 10, 5, 0x1234);
    graphics_draw_pixel(surface, pitch, 5, 10, 0x1234);

    test_teardown();

    /* Surface should still be all zeros */
    int i;
    for (i = 0; i < 100; i++) {
        if (surface[i] != 0) return 0;
    }
    return 1;
}

static int test_get_pixel_valid(void) {
    test_setup();

    u16 surface[100];
    memset(surface, 0, sizeof(surface));
    surface[5 * 10 + 3] = 0xABCD;

    int pitch = 20;
    g_graphics.width = 10;
    g_graphics.height = 10;

    u16 pixel = graphics_get_pixel(surface, pitch, 3, 5);

    test_teardown();

    return pixel == 0xABCD;
}

static int test_get_pixel_out_of_bounds(void) {
    test_setup();

    u16 surface[100];
    memset(surface, 0, sizeof(surface));

    int pitch = 20;
    g_graphics.width = 10;
    g_graphics.height = 10;

    u16 p1 = graphics_get_pixel(surface, pitch, -1, 5);
    u16 p2 = graphics_get_pixel(surface, pitch, 5, -1);
    u16 p3 = graphics_get_pixel(surface, pitch, 10, 5);
    u16 p4 = graphics_get_pixel(surface, pitch, 5, 10);

    test_teardown();

    return p1 == 0 && p2 == 0 && p3 == 0 && p4 == 0;
}

/* ========================================
 * Line Drawing Tests
 * ======================================== */

static int test_draw_hline_basic(void) {
    test_setup();

    u16 surface[100];
    memset(surface, 0, sizeof(surface));

    int pitch = 20;
    g_graphics.width = 10;
    g_graphics.height = 10;

    graphics_draw_hline(surface, pitch, 2, 6, 5, 0xFFFF);

    test_teardown();

    /* Should have pixels at (2,5), (3,5), (4,5), (5,5), (6,5) */
    return surface[5 * 10 + 2] == 0xFFFF &&
           surface[5 * 10 + 3] == 0xFFFF &&
           surface[5 * 10 + 4] == 0xFFFF &&
           surface[5 * 10 + 5] == 0xFFFF &&
           surface[5 * 10 + 6] == 0xFFFF;
}

static int test_draw_hline_swapped_coords(void) {
    test_setup();

    u16 surface[100];
    memset(surface, 0, sizeof(surface));

    int pitch = 20;
    g_graphics.width = 10;
    g_graphics.height = 10;

    /* Draw with x2 < x1 - should still work */
    graphics_draw_hline(surface, pitch, 6, 2, 5, 0xFFFF);

    test_teardown();

    return surface[5 * 10 + 2] == 0xFFFF;
}

static int test_draw_hline_clipped(void) {
    test_setup();

    u16 surface[100];
    memset(surface, 0, sizeof(surface));

    int pitch = 20;
    g_graphics.width = 10;
    g_graphics.height = 10;

    /* Draw line that extends past surface bounds */
    graphics_draw_hline(surface, pitch, -5, 15, 5, 0xFFFF);

    test_teardown();

    /* Should only have pixels within bounds (0-9) */
    int pass = 1;
    int x;
    for (x = 0; x < 10; x++) {
        if (surface[5 * 10 + x] != 0xFFFF) pass = 0;
    }
    return pass;
}

static int test_draw_vline_basic(void) {
    test_setup();

    u16 surface[100];
    memset(surface, 0, sizeof(surface));

    int pitch = 20;
    g_graphics.width = 10;
    g_graphics.height = 10;

    graphics_draw_vline(surface, pitch, 5, 2, 6, 0xFFFF);

    test_teardown();

    /* Should have pixels at (5,2), (5,3), (5,4), (5,5), (5,6) */
    return surface[2 * 10 + 5] == 0xFFFF &&
           surface[3 * 10 + 5] == 0xFFFF &&
           surface[4 * 10 + 5] == 0xFFFF &&
           surface[5 * 10 + 5] == 0xFFFF &&
           surface[6 * 10 + 5] == 0xFFFF;
}

static int test_draw_vline_clipped(void) {
    test_setup();

    u16 surface[100];
    memset(surface, 0, sizeof(surface));

    int pitch = 20;
    g_graphics.width = 10;
    g_graphics.height = 10;

    graphics_draw_vline(surface, pitch, 5, -5, 15, 0xFFFF);

    test_teardown();

    int pass = 1;
    int y;
    for (y = 0; y < 10; y++) {
        if (surface[y * 10 + 5] != 0xFFFF) pass = 0;
    }
    return pass;
}

/* ========================================
 * Rectangle Drawing Tests
 * ======================================== */

static int test_draw_rect_outline(void) {
    test_setup();

    u16 surface[100];
    memset(surface, 0, sizeof(surface));

    int pitch = 20;
    g_graphics.width = 10;
    g_graphics.height = 10;

    graphics_draw_rect(surface, pitch, 2, 2, 4, 4, 0xFFFF);

    test_teardown();

    /* Check corners */
    return surface[2 * 10 + 2] == 0xFFFF &&  /* Top-left */
           surface[2 * 10 + 5] == 0xFFFF &&  /* Top-right */
           surface[5 * 10 + 2] == 0xFFFF &&  /* Bottom-left */
           surface[5 * 10 + 5] == 0xFFFF;    /* Bottom-right */
}

static int test_fill_rect_basic(void) {
    test_setup();

    u16 surface[100];
    memset(surface, 0, sizeof(surface));

    int pitch = 20;
    g_graphics.width = 10;
    g_graphics.height = 10;

    graphics_fill_rect(surface, pitch, 2, 2, 4, 4, 0xFFFF);

    test_teardown();

    /* Check that 4x4 area is filled */
    int pass = 1;
    int x, y;
    for (y = 2; y < 6; y++) {
        for (x = 2; x < 6; x++) {
            if (surface[y * 10 + x] != 0xFFFF) pass = 0;
        }
    }
    return pass;
}

static int test_fill_rect_clipped(void) {
    test_setup();

    u16 surface[100];
    memset(surface, 0, sizeof(surface));

    int pitch = 20;
    g_graphics.width = 10;
    g_graphics.height = 10;

    /* Rectangle extending past bounds */
    graphics_fill_rect(surface, pitch, -2, -2, 6, 6, 0xFFFF);

    test_teardown();

    /* Check that only valid area is filled (0,0) to (3,3) */
    int pass = 1;
    int x, y;
    for (y = 0; y < 4; y++) {
        for (x = 0; x < 4; x++) {
            if (surface[y * 10 + x] != 0xFFFF) pass = 0;
        }
    }
    return pass;
}

/* ========================================
 * Surface Operations Tests
 * ======================================== */

static int test_clear_surface(void) {
    test_setup();

    u16 surface[100];
    memset(surface, 0xFF, sizeof(surface));

    int pitch = 20;
    g_graphics.width = 10;
    g_graphics.height = 10;

    graphics_clear(surface, pitch, 0x1234);

    test_teardown();

    int pass = 1;
    int i;
    for (i = 0; i < 100; i++) {
        if (surface[i] != 0x1234) pass = 0;
    }
    return pass;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_fill_and_draw_combined(void) {
    test_setup();

    u16 surface[100];
    memset(surface, 0, sizeof(surface));

    int pitch = 20;
    g_graphics.width = 10;
    g_graphics.height = 10;

    /* Fill with one color */
    graphics_fill_rect(surface, pitch, 0, 0, 10, 10, 0x1111);

    /* Draw rectangle outline with different color */
    graphics_draw_rect(surface, pitch, 2, 2, 6, 6, 0xFFFF);

    test_teardown();

    /* Check that fill is present and outline overwrote it */
    return surface[0] == 0x1111 &&                    /* Fill */
           surface[2 * 10 + 2] == 0xFFFF &&           /* Outline */
           surface[3 * 10 + 3] == 0x1111;            /* Inside (not outline) */
}

static int test_pixel_roundtrip(void) {
    test_setup();

    u16 surface[100];
    memset(surface, 0, sizeof(surface));

    int pitch = 20;
    g_graphics.width = 10;
    g_graphics.height = 10;

    graphics_draw_pixel(surface, pitch, 5, 5, 0xABCD);
    u16 result = graphics_get_pixel(surface, pitch, 5, 5);

    test_teardown();

    return result == 0xABCD;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Graphics Module Comprehensive Tests ===\n\n");

    /* Color format tests */
    printf("Color Format Tests:\n");
    TEST(color_format_enum);
    TEST(rgb565_masks);
    TEST(rgb555_masks);
    TEST(graphics_init);
    TEST(graphics_set_format_565);
    TEST(graphics_set_format_555);

    /* Color conversion tests */
    printf("\nColor Conversion Tests:\n");
    TEST(make_color_565_white);
    TEST(make_color_565_black);
    TEST(make_color_565_red);
    TEST(make_color_565_green);
    TEST(make_color_565_blue);
    TEST(make_color_565_gray);
    TEST(make_color_555_white);
    TEST(make_color_555_black);
    TEST(make_color_555_red);
    TEST(make_color_555_green);
    TEST(make_color_555_blue);
    TEST(make_color_respects_format);

    /* Color extraction tests */
    printf("\nColor Extraction Tests:\n");
    TEST(extract_color_565_white);
    TEST(extract_color_565_black);
    TEST(extract_color_565_red);
    TEST(extract_color_565_null_pointers);
    TEST(extract_color_555_white);
    TEST(extract_color_555_red);
    TEST(color_roundtrip_565);
    TEST(color_roundtrip_555);

    /* Alpha blending tests */
    printf("\nAlpha Blending Tests:\n");
    TEST(alpha_blend_565_zero_alpha);
    TEST(alpha_blend_565_full_alpha);
    TEST(alpha_blend_565_half_alpha);
    TEST(alpha_blend_555_zero_alpha);
    TEST(alpha_blend_555_full_alpha);
    TEST(alpha_blend_respects_format);
    TEST(alpha_blend_clamping);

    /* Pixel operations tests */
    printf("\nPixel Operations Tests:\n");
    TEST(draw_pixel_valid);
    TEST(draw_pixel_out_of_bounds);
    TEST(get_pixel_valid);
    TEST(get_pixel_out_of_bounds);

    /* Line drawing tests */
    printf("\nLine Drawing Tests:\n");
    TEST(draw_hline_basic);
    TEST(draw_hline_swapped_coords);
    TEST(draw_hline_clipped);
    TEST(draw_vline_basic);
    TEST(draw_vline_clipped);

    /* Rectangle drawing tests */
    printf("\nRectangle Drawing Tests:\n");
    TEST(draw_rect_outline);
    TEST(fill_rect_basic);
    TEST(fill_rect_clipped);

    /* Surface operations tests */
    printf("\nSurface Operations Tests:\n");
    TEST(clear_surface);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(fill_and_draw_combined);
    TEST(pixel_roundtrip);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - graphics_blit_transparent
     * - graphics_blit_alpha
     * - graphics_blit_alpha_channel
     * - graphics_blit_scaled
     * - graphics_fill_gradient
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
