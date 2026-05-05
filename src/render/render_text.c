/*
 * Stone Age Client - Text Queue Rendering
 * Reverse engineered from sa_9061.exe FUN_0041d7c0, FUN_00414820
 *
 * This module handles queued text rendering with:
 * - Two-pass rendering (shadow + color)
 * - Multiple color types from PTR_DAT_004a2720
 * - Half-resolution mode support (DAT_04560214)
 * - Alpha surface support (DAT_0054c83c)
 * - Custom font size support via param1
 */

#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "render.h"
#include "directx.h"
#include "logger.h"

/*
 * Text color table - PTR_DAT_004a2720
 * 10 color entries from 0x004a2720 to 0x004a2748
 */
static COLORREF s_text_colors[10] = {
    RGB(255, 255, 255),  /* 0: White */
    RGB(255, 255, 0),    /* 1: Yellow */
    RGB(255, 0, 0),      /* 2: Red */
    RGB(0, 255, 0),      /* 3: Green */
    RGB(0, 255, 255),    /* 4: Cyan */
    RGB(255, 0, 255),    /* 5: Magenta */
    RGB(255, 165, 0),    /* 6: Orange */
    RGB(128, 128, 128),  /* 7: Gray */
    RGB(0, 0, 0),        /* 8: Black */
    RGB(64, 64, 64)      /* 9: Dark gray */
};

/* Dialog title strings - DAT_004a34d0 (13 bytes each) */
static const char* s_dialog_titles[] = {
    "Dialog 1",
    "Dialog 2",
    "Dialog 3",
    "Dialog 4",
    "Dialog 5"
};

/* Text queue storage - matching DAT_005676f8 structure */
static TextQueueEntry s_text_queue[MAX_TEXT_QUEUE_ENTRIES];
static int s_text_queue_count = 0;  /* DAT_005ab6f8 */

/* Half-resolution mode flag - DAT_04560214 */
static int s_half_resolution = 0;

/* Dialog state - from original binary */
static int s_dialog_offset = 0;   /* DAT_045f1a14 */
static int s_dialog_flag = 0;     /* DAT_0054c840 */

/* Custom font handles - DAT_0054c854, DAT_0054c85c */
static HFONT s_custom_font1 = NULL;  /* DAT_0054c854 */
static HFONT s_custom_font2 = NULL;  /* DAT_0054c85c */
static HFONT s_prev_font1 = NULL;    /* DAT_0054c858 */
static HFONT s_prev_font2 = NULL;    /* DAT_0054c860 */

/* External references to main render module */
extern HFONT g_font;          /* DAT_0054b188 */
extern int g_font_size;

/* Forward declarations */
static HFONT text_queue_create_font(int font_size);

/*
 * Initialize text queue system
 */
void text_queue_init(void) {
    s_text_queue_count = 0;
    s_half_resolution = 0;
    s_dialog_offset = 0;
    s_dialog_flag = 0;
    s_custom_font1 = NULL;
    s_custom_font2 = NULL;
    memset(s_text_queue, 0, sizeof(s_text_queue));
}

/*
 * Shutdown text queue system
 */
void text_queue_shutdown(void) {
    if (s_custom_font1) {
        DeleteObject(s_custom_font1);
        s_custom_font1 = NULL;
    }
    if (s_custom_font2) {
        DeleteObject(s_custom_font2);
        s_custom_font2 = NULL;
    }
    s_text_queue_count = 0;
}

/*
 * Set half-resolution mode for text rendering - DAT_04560214
 */
void text_queue_set_half_resolution(int enabled) {
    s_half_resolution = enabled ? 1 : 0;
}

/*
 * Set dialog offset for special text positioning - DAT_045f1a14
 */
void text_queue_set_dialog_offset(int offset) {
    s_dialog_offset = offset;
}

/*
 * Set dialog flag for text rendering mode - DAT_0054c840
 */
void text_queue_set_dialog_flag(int flag) {
    s_dialog_flag = flag ? 1 : 0;
}

/*
 * Get current text queue count - DAT_005ab6f8
 */
int text_queue_get_count(void) {
    return s_text_queue_count;
}

/*
 * Create font for text rendering - FUN_00415a70 exact pattern
 * Parameters from decompiled binary:
 * - weight: 400 (FW_NORMAL)
 * - charset: 0x86 (134 = GB2312_CHARSET)
 * - quality: 0x11 (PROOF_QUALITY | CLEARTYPE_NATURAL_QUALITY)
 * - font name: "黑体" (SimHei) at DAT_004a394c
 */
static HFONT text_queue_create_font(int font_size) {
    return CreateFontA(font_size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        0x11, DEFAULT_PITCH | FF_DONTCARE, "黑体");
}

/*
 * Add text entry to queue - FUN_0041d7c0
 * Returns queue index on success, -2 if queue full (>= 0x3ff)
 *
 * Memory layout matches DAT_005676f8 array with 0x110 stride:
 * - Offset 0x00: x (short)
 * - Offset 0x02: y (short)
 * - Offset 0x04: flags (byte)
 * - Offset 0x05: text (256 bytes)
 * - Offset 0x105: color_type (byte)
 * - Offset 0x108: param1 (u32, font_size)
 * - Offset 0x10c: param2 (u32)
 */
int text_queue_add(int x, int y, u8 color_type, u8 flags, const char* text, u32 param1, u32 param2) {
    size_t len;
    int index;
    char* dest;

    /* Check queue limit (0x3ff = 1023 from original) */
    if (s_text_queue_count >= 0x3ff) {
        return -2;
    }

    index = s_text_queue_count;

    /* Fill entry fields matching FUN_0041d7c0 offsets */
    s_text_queue[index].x = (s16)x;
    s_text_queue[index].y = (s16)y;
    s_text_queue[index].flags = flags;

    /* Copy text string to offset 0x05 */
    dest = s_text_queue[index].text;
    if (text) {
        len = strlen(text);
        if (len >= 256) {
            len = 255;
        }
        memcpy(dest, text, len);
        dest[len] = '\0';
    } else {
        dest[0] = '\0';
    }

    /* Set rendering parameters */
    s_text_queue[index].color_type = color_type;
    s_text_queue[index].param1 = param1;
    s_text_queue[index].param2 = param2;

    s_text_queue_count++;
    return index;
}

/*
 * Convenience wrapper - FUN_0041d860
 * Calls text_queue_add with param2=0
 */
int text_queue_add_simple(int x, int y, u8 color_type, u8 flags, const char* text, u32 param1) {
    return text_queue_add(x, y, color_type, flags, text, param1, 0);
}

/*
 * Clear text queue
 */
void text_queue_clear(void) {
    s_text_queue_count = 0;
}

/*
 * Render shadow pass for a single entry
 * From FUN_00414820 first loop iteration
 */
static void text_queue_render_shadow_entry(HDC hdc, HDC hdc_alpha, TextQueueEntry* entry) {
    int x = entry->x;
    int y = entry->y;
    int text_len;

    /* Dialog text (type 3) with dialog flag set: skip */
    if (entry->color_type == 3 && s_dialog_flag) {
        return;
    }

    text_len = (int)strlen(entry->text);

    /* Handle custom font size from param1 (offset 0x107 from text in original) */
    if (entry->param1 >= 1) {
        if (s_custom_font1) {
            DeleteObject(s_custom_font1);
        }
        s_custom_font1 = text_queue_create_font(entry->param1);
        if (s_custom_font1) {
            s_prev_font1 = SelectObject(hdc, s_custom_font1);
            if (hdc_alpha) {
                SelectObject(hdc_alpha, s_custom_font1);
            }
        }
    }

    /* Render shadow based on mode */
    if (s_half_resolution) {
        /* Half-resolution mode: divide coordinates by 2, add +1 for shadow offset */
        TextOutA(hdc, x / 2 + 1, y / 2 + 1, entry->text, text_len);
        if (hdc_alpha) {
            TextOutA(hdc_alpha, x / 2 + 1, y / 2 + 1, entry->text, text_len);
        }
    } else {
        /* Dialog text type 3 gets special X offset (0x17) if dialog_offset is set */
        if (entry->color_type == 3 && s_dialog_offset != 0) {
            TextOutA(hdc, x + 0x17, y + 1, entry->text, text_len);
            if (hdc_alpha) {
                TextOutA(hdc_alpha, x + 0x17, y + 1, entry->text, text_len);
            }
        } else {
            /* Normal shadow at +1,+1 offset */
            TextOutA(hdc, x + 1, y + 1, entry->text, text_len);
            if (hdc_alpha) {
                TextOutA(hdc_alpha, x + 1, y + 1, entry->text, text_len);
            }
        }
    }

    /* Restore font if changed */
    if (entry->param1 >= 1 && s_custom_font1) {
        DeleteObject(s_custom_font1);
        if (s_prev_font1) {
            SelectObject(hdc, s_prev_font1);
        }
        s_custom_font1 = NULL;
    }
}

/*
 * Render color pass for a single entry
 * From FUN_00414820 second loop (color pass iteration)
 */
static void text_queue_render_color_entry(HDC hdc, HDC hdc_alpha, TextQueueEntry* entry) {
    int x = entry->x;
    int y = entry->y;
    int text_len;

    /* Dialog text (type 3) with dialog flag set: skip */
    if (entry->color_type == 3 && s_dialog_flag) {
        return;
    }

    text_len = (int)strlen(entry->text);

    /* Handle custom font size from param1 */
    if (entry->param1 >= 1) {
        if (s_custom_font2) {
            DeleteObject(s_custom_font2);
        }
        s_custom_font2 = text_queue_create_font(entry->param1);
        if (s_custom_font2) {
            s_prev_font2 = SelectObject(hdc, s_custom_font2);
            if (hdc_alpha) {
                SelectObject(hdc_alpha, s_custom_font2);
            }
        }
    }

    /* Render text based on mode */
    if (s_half_resolution) {
        /* Half-resolution mode: divide coordinates by 2 */
        TextOutA(hdc, x / 2, y / 2, entry->text, text_len);
        if (hdc_alpha) {
            TextOutA(hdc_alpha, x / 2, y / 2, entry->text, text_len);
        }
    } else {
        /* Dialog text type 3 gets special X offset (0x16) if dialog_offset is set */
        if (entry->color_type == 3 && s_dialog_offset != 0) {
            TextOutA(hdc, x + 0x16, y, entry->text, text_len);
            if (hdc_alpha) {
                TextOutA(hdc_alpha, x + 0x16, y, entry->text, text_len);
            }
        } else {
            /* Normal rendering */
            TextOutA(hdc, x, y, entry->text, text_len);
            if (hdc_alpha) {
                TextOutA(hdc_alpha, x, y, entry->text, text_len);
            }
        }
    }

    /* Restore font if changed */
    if (entry->param1 >= 1 && s_custom_font2) {
        DeleteObject(s_custom_font2);
        if (s_prev_font2) {
            SelectObject(hdc, s_prev_font2);
        }
        s_custom_font2 = NULL;
    }
}

/*
 * Process and render queued text entries for a specific group - FUN_00414820(group)
 *
 * From Ghidra decompilation of FUN_0047dc60, text rendering is called with:
 *   FUN_00414820(0) - general text (at layer > 0x67)
 *   FUN_00414820(3) - special text / dialog / IME (at layer > 0x67)
 *   FUN_00414820(1) - second text layer (at layer > 0x6c)
 *   FUN_00414820(2) - third text layer (at layer > 0x6e)
 *
 * The group parameter filters entries by their color_type field.
 * Pass -1 to render all groups (used for final flush).
 *
 * Two-pass rendering:
 * - Pass 1: Shadow (black text at +1,+1 offset)
 * - Pass 2: Main text with color from PTR_DAT_004a2720, also checking flags byte
 */
void text_queue_process(int group) {
    int i;
    int color_pass;
    HDC hdc = NULL;
    HDC hdc_alpha = NULL;
    HFONT prev_font = NULL;
    HFONT prev_font_alpha = NULL;
    int alpha_mode;
    TextQueueEntry* entry;
    int text_len;

    if (s_text_queue_count == 0) return;

    alpha_mode = render_get_alpha_mode();

    if (FAILED(IDirectDrawSurface_GetDC(g_graphics.offscreen_surface, &hdc))) {
        return;
    }

    if (alpha_mode && g_graphics.alpha_surface) {
        if (SUCCEEDED(IDirectDrawSurface_GetDC(g_graphics.alpha_surface, &hdc_alpha))) {
            /* Alpha surface DC ready */
        }
    }

    if (g_font) {
        prev_font = SelectObject(hdc, g_font);
        if (hdc_alpha) {
            prev_font_alpha = SelectObject(hdc_alpha, g_font);
        }
    }

    SetBkMode(hdc, TRANSPARENT);
    if (hdc_alpha) {
        SetBkMode(hdc_alpha, TRANSPARENT);
    }

    /*
     * First pass: shadow rendering (black text at +1,+1)
     * From FUN_00414820 first loop - filters by group matching color_type
     */
    SetTextColor(hdc, RGB(0, 0, 0));
    if (hdc_alpha) {
        SetTextColor(hdc_alpha, RGB(0, 0, 0));
    }

    for (i = 0; i < s_text_queue_count; i++) {
        entry = &s_text_queue[i];
        if (group >= 0 && entry->color_type != (u8)group) continue;
        text_queue_render_shadow_entry(hdc, hdc_alpha, entry);
    }

    /*
     * Second pass: colored text rendering
     * From FUN_00414820 second loop:
     * - Outer: iterate 10 colors from PTR_DAT_004a2720
     * - Inner: check BOTH color_type == group AND flags == color_index
     */
    for (color_pass = 0; color_pass < 10; color_pass++) {
        int color_set = 0;

        for (i = 0; i < s_text_queue_count; i++) {
            entry = &s_text_queue[i];

            /* Filter by group (color_type) */
            if (group >= 0 && entry->color_type != (u8)group) continue;
            /* Filter by flags matching current color index */
            if (entry->flags != (u8)color_pass) continue;

            if (!color_set) {
                SetTextColor(hdc, s_text_colors[color_pass]);
                if (hdc_alpha) {
                    SetTextColor(hdc_alpha, s_text_colors[color_pass]);
                }
                color_set = 1;
            }

            text_queue_render_color_entry(hdc, hdc_alpha, entry);
        }
    }

    /* Dialog title for group 3 (dialog text) with dialog offset */
    if ((group < 0 || group == 3) && s_dialog_offset >= 0 && s_dialog_offset < 5) {
        const char* title = s_dialog_titles[s_dialog_offset];
        text_len = (int)strlen(title);

        SetTextColor(hdc, RGB(0, 0, 0));
        TextOutA(hdc, 3, 0x1b1, title, text_len);
        if (hdc_alpha) {
            TextOutA(hdc_alpha, 3, 0x1b1, title, text_len);
        }

        SetTextColor(hdc, s_text_colors[0]);
        TextOutA(hdc, 2, 0x1b0, title, text_len);
        if (hdc_alpha) {
            TextOutA(hdc_alpha, 2, 0x1b0, title, text_len);
        }
    }

    /* Restore DC state */
    if (prev_font) {
        SelectObject(hdc, prev_font);
    }
    if (prev_font_alpha && hdc_alpha) {
        SelectObject(hdc_alpha, prev_font_alpha);
    }

    IDirectDrawSurface_ReleaseDC(g_graphics.offscreen_surface, hdc);
    if (hdc_alpha) {
        IDirectDrawSurface_ReleaseDC(g_graphics.alpha_surface, hdc_alpha);
    }

    /* Clear queue after final flush (group < 0 means render all and clear) */
    if (group < 0) {
        s_text_queue_count = 0;
    }
}
