/*
 * Stone Age Client - UI Widget System Core Implementation
 * Widget creation, destruction, and management
 *
 * Reverse engineered from sa_9061.exe:
 *   FUN_00448610 - Window creation with style (91 callers)
 *   FUN_00448270 - Window render callback (9-sprite grid)
 *   FUN_004010a0 - Object allocator with linked list
 *
 * Split modules:
 *   - uiwidget_render.c: Render callbacks
 *   - uiwidget_input.c: Input handling
 *   - uiwidget_props.c: Property setters/getters
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <timeapi.h>
#include "types.h"
#include "uiwidget.h"
#include "logger.h"

/* Global widget manager */
WidgetManagerContext g_widgetmgr = {0};

/* Widget ID counter */
static u32 s_widget_id_counter = 1;

/* Sprite ID constants from DAT_0054b194 and DAT_0054c208 */
#define SPRITE_STANDARD_BASE    0x6591   /* DAT_0054b194 */
#define SPRITE_ALTERNATE_BASE   0x65a5   /* DAT_0054c208 */

/* Alpha mode flag from DAT_0054c83c */
int g_alpha_enabled = 0;

/* Default colors */
static const WidgetColor s_default_bg = {40, 40, 60, 255};
static const WidgetColor s_default_border = {100, 100, 140, 255};
static const WidgetColor s_default_text = {255, 255, 255, 255};

/* Skin file names - from FUN_00489070 analysis */
static const char* s_skin_files[5] = {
    "\\base.bmp",
    "\\down_arrow_r.bmp",
    "\\down_arrow_g.bmp",
    "",  /* Reserved */
    ""   /* Reserved */
};

/* Forward declarations for render callback - implemented in uiwidget_render.c */
extern void widget_window_render_callback(Widget* widget);

/*
 * Initialize widget manager - FUN_004779d0 pattern
 */
int widgetmgr_init(void) {
    memset(&g_widgetmgr, 0, sizeof(WidgetManagerContext));

    LOG_INFO("Widget manager initialized");
    return 1;
}

/*
 * Shutdown widget manager
 */
void widgetmgr_shutdown(void) {
    widget_destroy_all();
    widget_unload_skin();
    memset(&g_widgetmgr, 0, sizeof(WidgetManagerContext));
    LOG_INFO("Widget manager shutdown");
}

/*
 * Allocate widget object - FUN_004010a0 pattern
 * Allocates base object (500 bytes) + optional extra data
 * Inserts into linked list at g_widgetmgr
 */
static Widget* widget_alloc(WidgetType type, u32 extra_size) {
    Widget* widget;
    void* extra_data;

    /* Allocate base widget structure */
    widget = (Widget*)calloc(1, sizeof(Widget));
    if (!widget) {
        MessageBoxA(NULL, "GetAction Error", "Error", MB_OK);
        return NULL;
    }

    /* Allocate extra data if requested */
    if (extra_size > 0) {
        extra_data = calloc(1, extra_size);
        if (!extra_data) {
            free(widget);
            MessageBoxA(NULL, "GetYobi Error", "Error", MB_OK);
            return NULL;
        }
        widget->extended_data = extra_data;
    }

    /* Initialize type */
    widget->type = type;
    widget->id = s_widget_id_counter++;

    /* Insert into linked list */
    widget->next = NULL;
    widget->prev = NULL;

    return widget;
}

/*
 * Create widget - FUN_00477b20 pattern
 */
Widget* widget_create(WidgetType type, int x, int y, int width, int height, Widget* parent) {
    Widget* widget = NULL;
    int i;

    if (g_widgetmgr.widget_count >= 256) {
        LOG_WARN("Widget limit reached");
        return NULL;
    }

    /* Find free slot */
    for (i = 0; i < 256; i++) {
        if (g_widgetmgr.widgets[i].id == 0) {
            widget = &g_widgetmgr.widgets[i];
            break;
        }
    }

    if (!widget) return NULL;

    /* Initialize widget */
    memset(widget, 0, sizeof(Widget));

    widget->id = s_widget_id_counter++;
    widget->type = type;
    widget->flags = WIDGET_FLAG_VISIBLE | WIDGET_FLAG_ENABLED;

    widget->x = x;
    widget->y = y;

    widget->bg_color = s_default_bg;
    widget->border_color = s_default_border;
    widget->text_color = s_default_text;

    widget->create_time = timeGetTime();
    widget->parent = parent;

    /* Add to parent's children */
    if (parent) {
        if (parent->child_count < 16) {
            parent->children[parent->child_count++] = widget;
        }
    } else {
        /* Add to root widgets */
        if (g_widgetmgr.root_count < 64) {
            g_widgetmgr.root_widgets[g_widgetmgr.root_count++] = widget;
        }
    }

    g_widgetmgr.widget_count++;
    g_widgetmgr.total_created++;

    return widget;
}

/*
 * Create window widget - FUN_00448610
 *
 * Parameters from binary:
 *   param_1: X position
 *   param_2: Y position
 *   param_3: Width (in tiles)
 *   param_4: Height (in tiles)
 *   param_5: Extra parameter
 *   param_6: Style (0-4, -1)
 *
 * Memory layout from binary:
 *   offset 0x00-0x03: width (piVar1[0])
 *   offset 0x04-0x07: height (piVar1[1])
 *   offset 0x08-0x0b: extra_param (piVar1[2])
 *   offset 0x0c-0x0f: base_x (piVar1[3])
 *   offset 0x10-0x13: base_y (piVar1[4])
 *   offset 0x14-0x17: center_x (piVar1[5])
 *   offset 0x18-0x1b: center_y (piVar1[6])
 *   offset 0x1c-0x1f: offset_x (piVar1[7])
 *   offset 0x20-0x23: offset_y (piVar1[8])
 *   offset 0x24-0x27: frame_count (piVar1[9])
 *   offset 0x28-0x2b: sprite_base (piVar1[10])
 *   offset 0x2c-0x2f: alpha_mode (piVar1[0xb])
 *   offset 0x30-0x33: close_sprite1 (piVar1[0xc])
 *   offset 0x34-0x37: close_sprite2 (piVar1[0xd])
 *   offset 0x38-0x3b: close_sprite3 (piVar1[0xe])
 *   offset 0x3c-0x3f: hover_flag (piVar1[0xf])
 */
Widget* widget_create_window_ex(int x, int y, int width, int height,
                                 const char* title, int style) {
    Widget* widget;
    WidgetWindowData* wdata;
    s32 center_calc;

    /* Allocate with 0x40 (64) bytes extra - FUN_004010a0(3, 0x40) */
    widget = widget_alloc(WIDGET_TYPE_WINDOW, sizeof(WidgetWindowData));
    if (!widget) {
        return NULL;
    }

    wdata = (WidgetWindowData*)widget->extended_data;
    if (!wdata) {
        free(widget);
        return NULL;
    }

    /* Set render callback - *(code **)(iVar3 + 8) = FUN_00448270 */
    widget->render_callback = widget_window_render_callback;

    /* Set flags - *(uint *)(iVar3 + 0xa0) |= 6 */
    widget->render_state = 0;

    /* Set sprite type - *(undefined1 *)(iVar3 + 0x15) = 0x68 */
    widget->sprite_type = 0x68;

    /* Store position - *(int *)(iVar3 + 0x18) = param_1 */
    widget->x = x;
    widget->y = y;

    /* Initialize WidgetWindowData - piVar1 assignments */
    wdata->width = width;                           /* piVar1[0] = param_3 */
    wdata->height = height;                         /* piVar1[1] = param_4 */
    wdata->extra_param = 0;                         /* piVar1[2] = param_5 */

    /* Calculate base position - iVar4 = (param_3 << 6) / 2 */
    center_calc = (width << 6) / 2;
    wdata->base_x = x + center_calc;                /* piVar1[3] = x + center_calc */
    wdata->base_y = y + 0x1b;                       /* piVar1[4] = y + 0x1b */

    /* Set sprite base based on style - switch(param_6) */
    switch (style) {
        case 0:
        case 2:
            /* Standard window sprites - piVar1[10] = DAT_0054b194 */
            wdata->sprite_base = SPRITE_STANDARD_BASE;
            break;
        case 1:
        case 3:
            /* Alternate window sprites - piVar1[10] = DAT_0054c208 */
            wdata->sprite_base = SPRITE_ALTERNATE_BASE;
            break;
        case 4:
            /* Close button style - piVar1[0xe] = 0x6d, sprites = -2 */
            wdata->close_sprite3 = 0x6d;
            wdata->sprite_base = -2;
            wdata->close_sprite1 = -2;
            wdata->close_sprite2 = -2;
            break;
        case -1:
        default:
            /* Default - piVar1[10] = -1 */
            wdata->sprite_base = -1;
            break;
    }

    /* Initialize hover flag - piVar1[0xf] = -1 */
    wdata->hover_flag = -1;

    /* Set alpha mode - piVar1[0xb] = (param_6 < 2) ? 1 : 0 */
    wdata->alpha_mode = (style < 2) ? 1 : 0;

    /* Calculate center position */
    if (style == -1 || style == 4) {
        /* Centered style - piVar1[5] = param_3 / 2 + x */
        wdata->center_x = width / 2 + widget->x;
    } else {
        /* Other styles - piVar1[5] = x + center_calc, adjust height */
        wdata->center_x = widget->x + center_calc;
        /* height = height * 0x30 - for non-centered windows */
        wdata->center_y = (height * 0x30) / 2 + y;
    }

    /* Calculate animation steps - *(int *)(iVar3 + 0x28/0x2c) */
    widget->anim_offset_x = (wdata->center_x - widget->x) / 10;
    widget->anim_offset_y = (wdata->center_y - widget->y) / 10;

    /* Initialize animation offsets */
    wdata->offset_x = 0;
    wdata->offset_y = 0;
    wdata->frame_count = 0;

    /* Set title if provided */
    if (title) {
        widget_set_text(widget, title);
    }

    /* Add to root widgets */
    if (g_widgetmgr.root_count < 64) {
        g_widgetmgr.root_widgets[g_widgetmgr.root_count++] = widget;
    }
    g_widgetmgr.widget_count++;

    return widget;
}

/*
 * Create window widget (simple wrapper)
 */
Widget* widget_create_window(int x, int y, int width, int height, const char* title) {
    return widget_create_window_ex(x, y, width, height, title, WINDOW_STYLE_STANDARD);
}

/*
 * Create button widget
 */
Widget* widget_create_button(int x, int y, int width, int height, const char* text) {
    Widget* widget = widget_create(WIDGET_TYPE_BUTTON, x, y, width, height, NULL);
    if (widget) {
        if (text) widget_set_text(widget, text);
        widget->flags |= WIDGET_FLAG_CLICKABLE | WIDGET_FLAG_BORDER;
    }
    return widget;
}

/*
 * Create text widget
 */
Widget* widget_create_text(int x, int y, int width, int height, const char* text) {
    Widget* widget = widget_create(WIDGET_TYPE_TEXT, x, y, width, height, NULL);
    if (widget && text) {
        widget_set_text(widget, text);
    }
    return widget;
}

/*
 * Create edit widget
 */
Widget* widget_create_edit(int x, int y, int width, int height, int max_len) {
    Widget* widget = widget_create(WIDGET_TYPE_EDIT, x, y, width, height, NULL);
    if (widget) {
        widget->text = (char*)malloc(max_len + 1);
        if (widget->text) {
            widget->text[0] = '\0';
            widget->text_len = max_len;
        }
        widget->flags |= WIDGET_FLAG_BORDER;
    }
    return widget;
}

/*
 * Create image widget
 */
Widget* widget_create_image(int x, int y, int width, int height, u16 sprite_id) {
    Widget* widget = widget_create(WIDGET_TYPE_IMAGE, x, y, width, height, NULL);
    if (widget) {
        widget->sprite_id = sprite_id;
    }
    return widget;
}

/*
 * Create listbox widget
 */
Widget* widget_create_listbox(int x, int y, int width, int height) {
    Widget* widget = widget_create(WIDGET_TYPE_LISTBOX, x, y, width, height, NULL);
    if (widget) {
        widget->flags |= WIDGET_FLAG_BORDER;
    }
    return widget;
}

/*
 * Create progress bar widget
 */
Widget* widget_create_progress(int x, int y, int width, int height, int max_value) {
    Widget* widget = widget_create(WIDGET_TYPE_PROGRESS, x, y, width, height, NULL);
    if (widget) {
        widget->data = (void*)(size_t)max_value;
        widget->flags |= WIDGET_FLAG_BORDER;
    }
    return widget;
}

/*
 * Create slot widget (inventory/equipment slot)
 */
Widget* widget_create_slot(int x, int y, int slot_index) {
    Widget* widget = widget_create(WIDGET_TYPE_SLOT, x, y, 32, 32, NULL);
    if (widget) {
        widget->data = (void*)(size_t)slot_index;
        widget->flags |= WIDGET_FLAG_CLICKABLE | WIDGET_FLAG_BORDER;
    }
    return widget;
}

/*
 * Destroy widget
 */
void widget_destroy(Widget* widget) {
    int i;

    if (!widget || widget->id == 0) return;

    /* Destroy children first */
    widget_destroy_children(widget);

    /* Free text */
    if (widget->text) {
        free(widget->text);
        widget->text = NULL;
    }

    /* Free extended data */
    if (widget->extended_data) {
        free(widget->extended_data);
        widget->extended_data = NULL;
    }

    /* Free custom data */
    if (widget->data && widget->data_size > 0) {
        free(widget->data);
        widget->data = NULL;
    }

    /* Remove from parent */
    if (widget->parent) {
        for (i = 0; i < widget->parent->child_count; i++) {
            if (widget->parent->children[i] == widget) {
                /* Shift remaining children */
                int j;
                for (j = i; j < widget->parent->child_count - 1; j++) {
                    widget->parent->children[j] = widget->parent->children[j + 1];
                }
                widget->parent->child_count--;
                break;
            }
        }
    } else {
        /* Remove from root widgets */
        for (i = 0; i < g_widgetmgr.root_count; i++) {
            if (g_widgetmgr.root_widgets[i] == widget) {
                int j;
                for (j = i; j < g_widgetmgr.root_count - 1; j++) {
                    g_widgetmgr.root_widgets[j] = g_widgetmgr.root_widgets[j + 1];
                }
                g_widgetmgr.root_count--;
                break;
            }
        }
    }

    /* Clear focus if needed */
    if (g_widgetmgr.focused_widget == widget) {
        g_widgetmgr.focused_widget = NULL;
    }
    if (g_widgetmgr.hovered_widget == widget) {
        g_widgetmgr.hovered_widget = NULL;
    }

    widget->id = 0;
    g_widgetmgr.widget_count--;
    g_widgetmgr.total_destroyed++;
}

/*
 * Destroy all children of a widget
 */
void widget_destroy_children(Widget* widget) {
    if (!widget) return;

    while (widget->child_count > 0) {
        widget_destroy(widget->children[0]);
    }
}

/*
 * Destroy all widgets
 */
void widget_destroy_all(void) {
    int i;

    for (i = 0; i < 256; i++) {
        if (g_widgetmgr.widgets[i].id != 0) {
            widget_destroy(&g_widgetmgr.widgets[i]);
        }
    }

    g_widgetmgr.widget_count = 0;
    g_widgetmgr.root_count = 0;
}

/*
 * Load UI skin - FUN_00489070 pattern
 */
int widget_load_skin(const char* path) {
    char bmp_path[512];
    int i;
    HBITMAP hBitmap;
    HDC hDC;
    HGDIOBJ hOldObj;

    /* Load each skin bitmap */
    for (i = 0; i < 5; i++) {
        if (s_skin_files[i][0] == '\0') continue;

        snprintf(bmp_path, sizeof(bmp_path), "%s%s", path, s_skin_files[i]);

        /* Load bitmap from file */
        hBitmap = (HBITMAP)LoadImageA(NULL, bmp_path, IMAGE_BITMAP,
                                       0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
        if (!hBitmap) {
            LOG_WARN("Failed to load skin: %s", bmp_path);
            continue;
        }

        g_widgetmgr.skin_images[i] = hBitmap;

        /* Create compatible DC */
        hDC = CreateCompatibleDC(NULL);
        if (hDC) {
            hOldObj = SelectObject(hDC, hBitmap);
            g_widgetmgr.skin_dc[i] = hDC;
            (void)hOldObj;
        }
    }

    /* Create back buffer for rendering */
    if (g_widgetmgr.skin_dc[0]) {
        g_widgetmgr.back_dc = CreateCompatibleDC(NULL);
        if (g_widgetmgr.back_dc) {
            /* Create back buffer bitmap (645 x 155) */
            g_widgetmgr.back_buffer = CreateCompatibleBitmap(
                g_widgetmgr.skin_dc[0], 645, 155);
            if (g_widgetmgr.back_buffer) {
                SelectObject(g_widgetmgr.back_dc, g_widgetmgr.back_buffer);
            }
        }
    }

    LOG_INFO("UI skin loaded from: %s", path);
    return 1;
}

/*
 * Unload UI skin
 */
void widget_unload_skin(void) {
    int i;

    /* Clean up back buffer */
    if (g_widgetmgr.back_buffer) {
        DeleteObject(g_widgetmgr.back_buffer);
        g_widgetmgr.back_buffer = NULL;
    }
    if (g_widgetmgr.back_dc) {
        DeleteDC(g_widgetmgr.back_dc);
        g_widgetmgr.back_dc = NULL;
    }

    /* Clean up skin images and DCs */
    for (i = 0; i < 5; i++) {
        if (g_widgetmgr.skin_dc[i]) {
            DeleteDC(g_widgetmgr.skin_dc[i]);
            g_widgetmgr.skin_dc[i] = NULL;
        }
        if (g_widgetmgr.skin_images[i]) {
            DeleteObject(g_widgetmgr.skin_images[i]);
            g_widgetmgr.skin_images[i] = NULL;
        }
    }

    LOG_INFO("UI skin unloaded");
}
