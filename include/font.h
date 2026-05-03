/*
 * Stone Age Client - Font System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef FONT_H
#define FONT_H

#include "types.h"

/* Constants */
#define MAX_FONTS       16

/* Font slots */
typedef enum {
    FONT_DEFAULT = 0,
    FONT_TITLE = 1,
    FONT_SMALL = 2,
    FONT_CHAT = 3,
    FONT_ITEM = 4,
    FONT_DIALOG = 5,
    FONT_BUTTON = 6
} FontSlot;

/* Font alignment */
typedef enum {
    FONT_ALIGN_LEFT = 0,
    FONT_ALIGN_CENTER = 1,
    FONT_ALIGN_RIGHT = 2,
    FONT_ALIGN_TOP = 0,
    FONT_ALIGN_VCENTER = 4,
    FONT_ALIGN_BOTTOM = 8
} FontAlignment;

/* Font entry */
typedef struct {
    HFONT handle;
    char face[32];
    u16 size;
    u8 bold;
    u8 italic;
    u8 underline;

} FontEntry;

/* Font context */
typedef struct {
    /* Loaded fonts */
    FontEntry fonts[MAX_FONTS];
    int font_count;

    /* Current font */
    FontSlot current_font;

    /* Default color */
    u32 default_color;

} FontContext;

/* Global font context */
extern FontContext g_font;

/* Initialization */
int font_init(void);
void font_shutdown(void);

/* Font creation */
int font_create_font(const char* face, int size, int bold, FontSlot slot);

/* Font access */
HFONT font_get(FontSlot slot);
HFONT font_select(HDC hdc, FontSlot slot);

/* Text drawing */
int font_draw_text(HDC hdc, int x, int y, const char* text, u32 color);
int font_draw_text_ex(HDC hdc, FontSlot slot, int x, int y, const char* text, u32 color);
int font_draw_text_rect(HDC hdc, FontSlot slot, RECT* rect, const char* text, u32 color, u32 align);
int font_draw_text_shadow(HDC hdc, FontSlot slot, int x, int y, const char* text, u32 color, u32 shadow_color);
int font_draw_text_outline(HDC hdc, FontSlot slot, int x, int y, const char* text, u32 color, u32 outline_color);

/* Text measurement */
int font_get_text_size(HDC hdc, FontSlot slot, const char* text, int* width, int* height);
int font_get_text_width(HDC hdc, FontSlot slot, const char* text);
int font_get_text_height(HDC hdc, FontSlot slot);
int font_get_char_width(HDC hdc, FontSlot slot, char c);
int font_get_metrics(HDC hdc, FontSlot slot, int* ascent, int* descent, int* height);

/* Word wrap */
int font_word_wrap(HDC hdc, FontSlot slot, const char* text, int max_width, char* output, int output_size);

/* Current font */
void font_set_current(FontSlot slot);
FontSlot font_get_current(void);

/* Colors */
void font_set_default_color(u32 color);
u32 font_get_default_color(void);
u32 font_make_color(u8 r, u8 g, u8 b);

/* Surface rendering */
int font_draw_to_surface(void* surface, int width, int height, int pitch, int x, int y, const char* text, u32 color, FontSlot slot);

/* ========================================
 * DBCS (Double Byte Character Set) Support
 * Reverse engineered from FUN_0044a940
 * ======================================== */

/* DBCS character handling */
int font_is_dbcs_lead_byte(BYTE c);
const char* font_prev_char(const char* str, const char* pos);
const char* font_next_char(const char* str);
int font_char_len(const char* str);
int font_char_count(const char* str);

/* String truncation with ellipsis */
int font_truncate_string(const char* src, char* dest, int max_len);
int font_truncate_to_width(HDC hdc, FontSlot slot, const char* src, char* dest, int max_width);

/* DBCS-aware text measurement */
int font_get_text_width_dbcs(HDC hdc, FontSlot slot, const char* text);

/* Chinese font creation - FUN_00415a70 */
HFONT font_create_chinese(int size);

/* DBCS substring */
int font_substr_dbcs(const char* src, int start_char, int char_count, char* dest, int dest_size);

#endif /* FONT_H */
