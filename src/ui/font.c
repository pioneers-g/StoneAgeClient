/*
 * Stone Age Client - Font System Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "font.h"
#include "logger.h"

/* Global font context */
FontContext g_font = {0};

/*
 * Initialize font system
 */
int font_init(void) {
    memset(&g_font, 0, sizeof(FontContext));

    /* Initialize default fonts */
    font_create_font("Arial", 12, 0, FONT_DEFAULT);
    font_create_font("Arial", 14, 0, FONT_TITLE);
    font_create_font("Arial", 10, 0, FONT_SMALL);

    LOG_INFO("Font system initialized");
    return 1;
}

/*
 * Shutdown font system
 */
void font_shutdown(void) {
    int i;

    for (i = 0; i < MAX_FONTS; i++) {
        if (g_font.fonts[i].handle) {
            DeleteObject(g_font.fonts[i].handle);
        }
    }

    memset(&g_font, 0, sizeof(FontContext));
    LOG_INFO("Font system shutdown");
}

/*
 * Create font
 */
int font_create_font(const char* face, int size, int bold, FontSlot slot) {
    HDC hdc;
    int height;
    HFONT hfont;

    if (slot >= MAX_FONTS) return 0;

    /* Delete existing font */
    if (g_font.fonts[slot].handle) {
        DeleteObject(g_font.fonts[slot].handle);
    }

    hdc = GetDC(NULL);
    height = -MulDiv(size, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(NULL, hdc);

    hfont = CreateFontA(
        height,                    /* Height */
        0,                         /* Width */
        0,                         /* Escapement */
        0,                         /* Orientation */
        bold ? FW_BOLD : FW_NORMAL,/* Weight */
        FALSE,                     /* Italic */
        FALSE,                     /* Underline */
        FALSE,                     /* StrikeOut */
        DEFAULT_CHARSET,           /* CharSet */
        OUT_DEFAULT_PRECIS,        /* OutputPrecision */
        CLIP_DEFAULT_PRECIS,       /* ClipPrecision */
        DEFAULT_QUALITY,           /* Quality */
        DEFAULT_PITCH | FF_DONTCARE,/* PitchAndFamily */
        face                       /* FaceName */
    );

    if (!hfont) {
        LOG_ERROR("Failed to create font: %s", face);
        return 0;
    }

    g_font.fonts[slot].handle = hfont;
    g_font.fonts[slot].size = (u16)size;
    g_font.fonts[slot].bold = (u8)bold;
    strncpy(g_font.fonts[slot].face, face, 31);
    g_font.fonts[slot].face[31] = '\0';

    if (slot >= g_font.font_count) {
        g_font.font_count = slot + 1;
    }

    return 1;
}

/*
 * Get font handle
 */
HFONT font_get(FontSlot slot) {
    if (slot >= MAX_FONTS) return NULL;
    return g_font.fonts[slot].handle;
}

/*
 * Set font for DC
 */
HFONT font_select(HDC hdc, FontSlot slot) {
    HFONT old_font;

    if (slot >= MAX_FONTS || !g_font.fonts[slot].handle) {
        return NULL;
    }

    old_font = (HFONT)SelectObject(hdc, g_font.fonts[slot].handle);
    return old_font;
}

/*
 * Draw text
 */
int font_draw_text(HDC hdc, int x, int y, const char* text, u32 color) {
    int len;
    RECT rect;
    HFONT old_font;
    COLORREF old_color;

    if (!text) return 0;

    len = strlen(text);

    old_font = font_select(hdc, FONT_DEFAULT);
    old_color = SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);

    rect.left = x;
    rect.top = y;
    rect.right = x + 1000;
    rect.bottom = y + 100;

    ExtTextOutA(hdc, x, y, 0, &rect, text, len, NULL);

    SetTextColor(hdc, old_color);
    if (old_font) SelectObject(hdc, old_font);

    return 1;
}

/*
 * Draw text with font
 */
int font_draw_text_ex(HDC hdc, FontSlot slot, int x, int y, const char* text, u32 color) {
    int len;
    RECT rect;
    HFONT old_font;
    COLORREF old_color;

    if (!text || slot >= MAX_FONTS) return 0;

    len = strlen(text);

    old_font = font_select(hdc, slot);
    old_color = SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);

    rect.left = x;
    rect.top = y;
    rect.right = x + 1000;
    rect.bottom = y + 100;

    ExtTextOutA(hdc, x, y, 0, &rect, text, len, NULL);

    SetTextColor(hdc, old_color);
    if (old_font) SelectObject(hdc, old_font);

    return 1;
}

/*
 * Draw text in rect
 */
int font_draw_text_rect(HDC hdc, FontSlot slot, RECT* rect, const char* text,
                         u32 color, u32 align) {
    HFONT old_font;
    COLORREF old_color;
    int format;

    if (!text || !rect || slot >= MAX_FONTS) return 0;

    old_font = font_select(hdc, slot);
    old_color = SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);

    format = DT_NOPREFIX;
    if (align & FONT_ALIGN_CENTER) format |= DT_CENTER;
    else if (align & FONT_ALIGN_RIGHT) format |= DT_RIGHT;
    else format |= DT_LEFT;

    if (align & FONT_ALIGN_VCENTER) format |= DT_VCENTER | DT_SINGLELINE;
    else if (align & FONT_ALIGN_BOTTOM) format |= DT_BOTTOM | DT_SINGLELINE;

    DrawTextA(hdc, text, -1, rect, format);

    SetTextColor(hdc, old_color);
    if (old_font) SelectObject(hdc, old_font);

    return 1;
}

/*
 * Draw text with shadow
 */
int font_draw_text_shadow(HDC hdc, FontSlot slot, int x, int y,
                            const char* text, u32 color, u32 shadow_color) {
    /* Draw shadow */
    font_draw_text_ex(hdc, slot, x + 1, y + 1, text, shadow_color);
    /* Draw main text */
    font_draw_text_ex(hdc, slot, x, y, text, color);

    return 1;
}

/*
 * Draw text with outline
 */
int font_draw_text_outline(HDC hdc, FontSlot slot, int x, int y,
                             const char* text, u32 color, u32 outline_color) {
    /* Draw outline */
    font_draw_text_ex(hdc, slot, x - 1, y, text, outline_color);
    font_draw_text_ex(hdc, slot, x + 1, y, text, outline_color);
    font_draw_text_ex(hdc, slot, x, y - 1, text, outline_color);
    font_draw_text_ex(hdc, slot, x, y + 1, text, outline_color);
    /* Draw main text */
    font_draw_text_ex(hdc, slot, x, y, text, color);

    return 1;
}

/*
 * Get text size
 */
int font_get_text_size(HDC hdc, FontSlot slot, const char* text, int* width, int* height) {
    SIZE size;
    HFONT old_font;

    if (!text || slot >= MAX_FONTS) return 0;

    old_font = font_select(hdc, slot);

    if (GetTextExtentPoint32A(hdc, text, strlen(text), &size)) {
        if (width) *width = size.cx;
        if (height) *height = size.cy;
    }

    if (old_font) SelectObject(hdc, old_font);

    return 1;
}

/*
 * Get text width
 */
int font_get_text_width(HDC hdc, FontSlot slot, const char* text) {
    int width = 0;
    font_get_text_size(hdc, slot, text, &width, NULL);
    return width;
}

/*
 * Get text height
 */
int font_get_text_height(HDC hdc, FontSlot slot) {
    TEXTMETRIC tm;
    HFONT old_font;
    int height = 0;

    if (slot >= MAX_FONTS) return 0;

    old_font = font_select(hdc, slot);
    if (GetTextMetricsA(hdc, &tm)) {
        height = tm.tmHeight;
    }
    if (old_font) SelectObject(hdc, old_font);

    return height;
}

/*
 * Word wrap text
 */
int font_word_wrap(HDC hdc, FontSlot slot, const char* text, int max_width,
                    char* output, int output_size) {
    const char* ptr;
    const char* word_start;
    const char* last_space;
    int current_width;
    int word_width;
    int line_width;
    char* out_ptr;
    int remaining;

    if (!text || !output || output_size < 1) return 0;

    out_ptr = output;
    remaining = output_size - 1;

    ptr = text;
    line_width = 0;

    while (*ptr && remaining > 0) {
        /* Find word boundary */
        word_start = ptr;
        last_space = NULL;

        while (*ptr && *ptr != '\n') {
            if (*ptr == ' ') {
                last_space = ptr;
            }
            ptr++;
        }

        /* Calculate word width */
        word_width = font_get_text_width(hdc, slot, word_start);

        if (line_width + word_width > max_width && last_space) {
            /* Word wrap at last space */
            int copy_len = last_space - word_start;
            if (copy_len >= remaining) copy_len = remaining - 1;

            memcpy(out_ptr, word_start, copy_len);
            out_ptr += copy_len;
            remaining -= copy_len;

            *out_ptr++ = '\n';
            remaining--;

            ptr = last_space + 1;
            line_width = 0;
        } else {
            /* Copy the line */
            int copy_len = ptr - word_start;
            if (*ptr == '\n') {
                copy_len++;
                line_width = 0;
            }

            if (copy_len >= remaining) copy_len = remaining - 1;

            memcpy(out_ptr, word_start, copy_len);
            out_ptr += copy_len;
            remaining -= copy_len;

            if (*ptr == '\n') {
                ptr++;
            }
        }
    }

    *out_ptr = '\0';
    return 1;
}

/*
 * Set current font
 */
void font_set_current(FontSlot slot) {
    if (slot < MAX_FONTS) {
        g_font.current_font = slot;
    }
}

/*
 * Get current font
 */
FontSlot font_get_current(void) {
    return g_font.current_font;
}

/*
 * Set default color
 */
void font_set_default_color(u32 color) {
    g_font.default_color = color;
}

/*
 * Get default color
 */
u32 font_get_default_color(void) {
    return g_font.default_color;
}

/*
 * Convert RGB to color
 */
u32 font_make_color(u8 r, u8 g, u8 b) {
    return RGB(r, g, b);
}

/*
 * Draw text to surface
 */
int font_draw_to_surface(void* surface, int width, int height, int pitch,
                          int x, int y, const char* text, u32 color, FontSlot slot) {
    HDC hdc;
    HDC mem_dc;
    HBITMAP hbitmap;
    HBITMAP old_bmp;
    RECT rect;
    HFONT old_font;
    COLORREF old_color;

    if (!surface || !text) return 0;

    /* Create memory DC */
    hdc = GetDC(NULL);
    mem_dc = CreateCompatibleDC(hdc);

    hbitmap = CreateCompatibleBitmap(hdc, width, height);
    old_bmp = (HBITMAP)SelectObject(mem_dc, hbitmap);

    /* Copy surface to bitmap */
    /* Note: This is simplified - real implementation would need proper surface handling */

    /* Draw text */
    old_font = font_select(mem_dc, slot);
    old_color = SetTextColor(mem_dc, color);
    SetBkMode(mem_dc, TRANSPARENT);

    rect.left = x;
    rect.top = y;
    rect.right = x + 1000;
    rect.bottom = y + 100;

    ExtTextOutA(mem_dc, x, y, 0, &rect, text, strlen(text), NULL);

    SetTextColor(mem_dc, old_color);
    if (old_font) SelectObject(mem_dc, old_font);

    /* Copy back to surface */
    /* Note: Would need proper surface copy here */

    /* Cleanup */
    SelectObject(mem_dc, old_bmp);
    DeleteObject(hbitmap);
    DeleteDC(mem_dc);
    ReleaseDC(NULL, hdc);

    return 1;
}

/*
 * Get character width
 */
int font_get_char_width(HDC hdc, FontSlot slot, char c) {
    char str[2] = {c, '\0'};
    return font_get_text_width(hdc, slot, str);
}

/*
 * Get font metrics
 */
int font_get_metrics(HDC hdc, FontSlot slot, int* ascent, int* descent, int* height) {
    TEXTMETRIC tm;
    HFONT old_font;
    int result = 0;

    if (slot >= MAX_FONTS) return 0;

    old_font = font_select(hdc, slot);

    if (GetTextMetricsA(hdc, &tm)) {
        if (ascent) *ascent = tm.tmAscent;
        if (descent) *descent = tm.tmDescent;
        if (height) *height = tm.tmHeight;
        result = 1;
    }

    if (old_font) SelectObject(hdc, old_font);

    return result;
}

/* ========================================
 * DBCS (Double Byte Character Set) Support
 * Reverse engineered from FUN_0044a940
 * ======================================== */

/*
 * Check if character is a DBCS lead byte
 * Uses Windows API for proper locale support
 */
int font_is_dbcs_lead_byte(BYTE c) {
    return IsDBCSLeadByteEx(CP_ACP, c);
}

/*
 * Get previous character position in DBCS string
 * Returns pointer to previous character
 */
const char* font_prev_char(const char* str, const char* pos) {
    const char* p;

    if (!str || !pos || pos <= str) return str;

    p = pos - 1;

    /* Check if previous byte is a trail byte */
    if (p > str && font_is_dbcs_lead_byte(*(p - 1))) {
        return p - 1;
    }

    return p;
}

/*
 * Get next character position in DBCS string
 * Returns pointer to next character (skip trail byte if DBCS)
 */
const char* font_next_char(const char* str) {
    if (!str || !*str) return str;

    if (font_is_dbcs_lead_byte(*str)) {
        return str + 2;
    }

    return str + 1;
}

/*
 * Get character byte length at position
 * Returns 2 for DBCS, 1 for SBCS, 0 for null
 */
int font_char_len(const char* str) {
    if (!str || !*str) return 0;

    if (font_is_dbcs_lead_byte(*str)) {
        return 2;
    }

    return 1;
}

/*
 * Count characters in DBCS string
 * Returns character count (not byte count)
 */
int font_char_count(const char* str) {
    int count = 0;

    if (!str) return 0;

    while (*str) {
        if (font_is_dbcs_lead_byte(*str)) {
            str += 2;
        } else {
            str++;
        }
        count++;
    }

    return count;
}

/*
 * String truncation with ellipsis - FUN_0044a940
 * Truncates string to fit within max_len bytes, adding ellipsis if needed
 * Handles DBCS characters correctly
 */
int font_truncate_string(const char* src, char* dest, int max_len) {
    int src_len;
    int copy_len;
    int ellipsis_len;
    int i;
    const char* p;

    if (!src || !dest || max_len <= 0) return 0;

    src_len = (int)strlen(src);

    /* If source fits, just copy */
    if (src_len <= max_len) {
        strncpy(dest, src, max_len);
        dest[max_len] = '\0';
        return src_len;
    }

    /* Calculate how much we can copy with ellipsis */
    ellipsis_len = 3;  /* "..." */

    if (max_len <= ellipsis_len) {
        /* Too short for ellipsis, just truncate */
        strncpy(dest, src, max_len);
        dest[max_len] = '\0';
        return max_len;
    }

    /* Find safe truncation point (don't cut DBCS character) */
    copy_len = max_len - ellipsis_len;

    /* Check if we'd cut a DBCS character */
    if (copy_len > 0 && font_is_dbcs_lead_byte(src[copy_len - 1])) {
        copy_len--;
    }

    /* Copy truncated portion */
    strncpy(dest, src, copy_len);

    /* Add ellipsis */
    dest[copy_len] = '.';
    dest[copy_len + 1] = '.';
    dest[copy_len + 2] = '.';
    dest[copy_len + 3] = '\0';

    return copy_len + ellipsis_len;
}

/*
 * Truncate string to fit width - FUN_0044a9c0 pattern
 * Uses DC to measure actual text width
 */
int font_truncate_to_width(HDC hdc, FontSlot slot, const char* src, char* dest, int max_width) {
    int text_width;
    int len;
    int i;
    const char* p;

    if (!src || !dest || max_width <= 0) return 0;

    /* Check if full text fits */
    text_width = font_get_text_width(hdc, slot, src);
    if (text_width <= max_width) {
        strcpy(dest, src);
        return text_width;
    }

    /* Binary search for truncation point */
    len = (int)strlen(src);
    for (i = len - 1; i > 0; i--) {
        /* Don't cut DBCS character */
        if (font_is_dbcs_lead_byte(src[i - 1])) {
            i--;
            continue;
        }

        /* Check width of truncated string */
        strncpy(dest, src, i);
        dest[i] = '\0';

        text_width = font_get_text_width(hdc, slot, dest);
        text_width += font_get_text_width(hdc, slot, "...");

        if (text_width <= max_width) {
            strcat(dest, "...");
            return text_width;
        }
    }

    /* Very short, just ellipsis */
    strcpy(dest, "...");
    return font_get_text_width(hdc, slot, dest);
}

/*
 * Get text width with DBCS support
 * Properly measures mixed-width text
 */
int font_get_text_width_dbcs(HDC hdc, FontSlot slot, const char* text) {
    SIZE size;
    HFONT old_font;
    int width = 0;
    const char* p;
    char buf[3];

    if (!text || slot >= MAX_FONTS) return 0;

    old_font = font_select(hdc, slot);

    /* Use GDI for proper measurement */
    if (GetTextExtentPoint32A(hdc, text, (int)strlen(text), &size)) {
        width = size.cx;
    }

    if (old_font) SelectObject(hdc, old_font);

    return width;
}

/*
 * Create Chinese font - FUN_00415a70 pattern
 * Creates font with GB2312 charset for proper Chinese display
 */
HFONT font_create_chinese(int size) {
    return CreateFontA(
        size,                    /* Height */
        0,                       /* Width */
        0,                       /* Escapement */
        0,                       /* Orientation */
        FW_NORMAL,               /* Weight (400) */
        FALSE,                   /* Italic */
        FALSE,                   /* Underline */
        FALSE,                   /* StrikeOut */
        GB2312_CHARSET,          /* CharSet (0x86 = 134) */
        OUT_DEFAULT_PRECIS,      /* OutputPrecision */
        CLIP_DEFAULT_PRECIS,     /* ClipPrecision */
        0x11,                    /* Quality (PROOF_QUALITY | CLEARTYPE) */
        DEFAULT_PITCH | FF_DONTCARE, /* PitchAndFamily */
        "黑体"                   /* FaceName - SimHei */
    );
}

/*
 * Get DBCS-safe substring
 * Copies specified number of characters (not bytes)
 */
int font_substr_dbcs(const char* src, int start_char, int char_count, char* dest, int dest_size) {
    const char* p;
    int i;
    int char_len;

    if (!src || !dest || dest_size <= 0) return 0;

    p = src;

    /* Skip to start position */
    for (i = 0; i < start_char && *p; i++) {
        if (font_is_dbcs_lead_byte(*p)) {
            p += 2;
        } else {
            p++;
        }
    }

    /* Copy characters */
    for (i = 0; i < char_count && *p && dest_size > 0; i++) {
        char_len = font_char_len(p);

        if (char_len >= dest_size) break;

        memcpy(dest, p, char_len);
        dest += char_len;
        dest_size -= char_len;
        p += char_len;
    }

    *dest = '\0';
    return i;
}
