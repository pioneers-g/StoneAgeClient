/*
 * Stone Age Client - UI Skin/Animation System Implementation
 * Reverse engineered from sa_9061.exe
 *
 * FUN_00488ca0 - Skin animation initialization (creates 40 frames)
 * FUN_00489070 - Skin image loading (loads 5 BMP files)
 * FUN_00488dc0 - Skin system shutdown
 *
 * This system manages a linked list of animation frames for UI skins.
 * Each frame is 0x118 bytes with prev/next pointers at offset 0x10c/0x110.
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "ui_skin.h"
#include "logger.h"

/* Skin image filenames - from DAT_004d4a90 area */
static const char* s_skin_filenames[SKIN_IMAGE_COUNT] = {
    "\\down_arrow_r.bmp",    /* 0x004d4a90 */
    "\\down_arrow_g.bmp",    /* 0x004d4aa4 */
    "\\base.bmp",            /* 0x004d4ad8 */
    "\\skin3.bmp",           /* Placeholder */
    "\\skin4.bmp"            /* Placeholder */
};

/* Global skin context */
UISkinContext g_ui_skin = {0};

/*
 * Initialize UI skin system - FUN_00488ca0
 *
 * Binary analysis:
 * - Allocates head frame with malloc(0x118)
 * - Creates linked list of 40 (0x28) frames
 * - Each frame linked via offset 0x10c (prev) and 0x44/next
 * - Calls FUN_00489070 with skin path
 */
int ui_skin_init(HWND hwnd, const char* skin_path) {
    SkinFrame* frame;
    SkinFrame* prev_frame;
    int i;

    memset(&g_ui_skin, 0, sizeof(UISkinContext));

    /* Store window handle */
    g_ui_skin.hwnd = hwnd;

    /* Initialize state fields from FUN_00488ca0 */
    g_ui_skin.field_12 = 0;      /* param_1[0x12] */
    g_ui_skin.field_14 = 0;      /* param_1[0x14] */
    g_ui_skin.field_13 = 0;      /* param_1[0x13] */
    g_ui_skin.field_17 = 0;      /* param_1[0x17] */
    g_ui_skin.field_15 = 0;      /* param_1[0x15] */
    g_ui_skin.field_16 = 0;      /* param_1[0x16] */
    g_ui_skin.field_18 = 0;      /* param_1[0x18] */
    g_ui_skin.field_04 = 0;      /* param_1[1] */

    /* Clear image handles - loop at 0x00488cca */
    for (i = 0; i < 5; i++) {
        g_ui_skin.images[i] = NULL;
    }

    /* Allocate first frame - malloc(0x118) at 0x00488ce0 */
    frame = (SkinFrame*)malloc(SKIN_FRAME_SIZE);
    if (!frame) {
        MessageBoxA(hwnd, "Failed to allocate skin frame", "Error", MB_OK);
        return 0;
    }

    /* Store frame pointers */
    g_ui_skin.frame_head = frame;
    g_ui_skin.frame_current = frame;
    g_ui_skin.frame_tail = frame;

    /* Zero-initialize first frame */
    memset(frame, 0, SKIN_FRAME_SIZE);

    /* Create linked list of 40 frames */
    prev_frame = frame;
    for (i = 0; i < SKIN_MAX_FRAMES; i++) {
        /* Allocate next frame */
        frame = (SkinFrame*)malloc(SKIN_FRAME_SIZE);
        if (!frame) {
            MessageBoxA(hwnd, "Failed to allocate skin frame", "Error", MB_OK);
            ui_skin_shutdown();
            return 0;
        }

        /* Zero-initialize */
        memset(frame, 0, SKIN_FRAME_SIZE);

        /* Link frames - offset 0x10c = prev, offset 0x44 in binary = next */
        /* At 0x00488d4a: *(undefined4 **)(puVar1[0x44] + 0x10c) = puVar1 */
        frame->prev = prev_frame;
        prev_frame->next = frame;

        prev_frame = frame;
    }

    /* Terminate linked list */
    frame->next = NULL;
    g_ui_skin.frame_tail = frame;
    g_ui_skin.frame_count = SKIN_MAX_FRAMES;
    g_ui_skin.current_frame = 0;

    /* Mark initialized */
    g_ui_skin.initialized = 1;

    /* Load skin images */
    if (skin_path) {
        ui_skin_load_images(skin_path);
    } else {
        ui_skin_load_images("data\\skin\\default");
    }

    LOG_INFO("UI Skin system initialized with %d frames", SKIN_MAX_FRAMES);
    return 1;
}

/*
 * Shutdown UI skin system - FUN_00488dc0
 */
void ui_skin_shutdown(void) {
    SkinFrame* frame;
    SkinFrame* next;
    int i;

    /* Free all frames in linked list */
    frame = g_ui_skin.frame_head;
    while (frame) {
        next = frame->next;

        /* Free frame data */
        free(frame);

        frame = next;
    }

    /* Free image resources */
    for (i = 0; i < SKIN_IMAGE_COUNT; i++) {
        if (g_ui_skin.image_dcs[i]) {
            SelectObject(g_ui_skin.image_dcs[i], g_ui_skin.original_objs[i]);
            DeleteDC(g_ui_skin.image_dcs[i]);
        }
        if (g_ui_skin.images[i]) {
            DeleteObject(g_ui_skin.images[i]);
        }
    }

    /* Free composite resources */
    if (g_ui_skin.composite_dc) {
        SelectObject(g_ui_skin.composite_dc, g_ui_skin.composite_orig_obj);
        DeleteDC(g_ui_skin.composite_dc);
    }
    if (g_ui_skin.composite_bitmap) {
        DeleteObject(g_ui_skin.composite_bitmap);
    }

    /* Free animation resources */
    if (g_ui_skin.anim_dc) {
        SelectObject(g_ui_skin.anim_dc, g_ui_skin.anim_orig_obj);
        DeleteDC(g_ui_skin.anim_dc);
    }
    if (g_ui_skin.anim_bitmap) {
        DeleteObject(g_ui_skin.anim_bitmap);
    }

    memset(&g_ui_skin, 0, sizeof(UISkinContext));
    LOG_INFO("UI Skin system shutdown");
}

/*
 * Load skin images - FUN_00489070
 *
 * Binary analysis:
 * - Loads 5 images using LoadImageA with LR_LOADFROMFILE (0x50)
 * - Creates compatible DC for each image
 * - Selects bitmap into DC
 * - Creates composite bitmap (645x155 pixels)
 */
int ui_skin_load_images(const char* skin_path) {
    char filepath[256];
    HBITMAP hbitmap;
    HDC hdc;
    int i;

    for (i = 0; i < SKIN_IMAGE_COUNT; i++) {
        /* Build filepath: skin_path + filename */
        _snprintf(filepath, sizeof(filepath), "%s%s", skin_path, s_skin_filenames[i]);

        /* Load image - LoadImageA with LR_LOADFROMFILE (0x50) */
        hbitmap = (HBITMAP)LoadImageA(
            NULL,
            filepath,
            IMAGE_BITMAP,
            0, 0,
            LR_LOADFROMFILE
        );

        if (!hbitmap) {
            LOG_WARN("Failed to load skin image: %s", filepath);
            continue;
        }

        /* Store image handle */
        g_ui_skin.images[i] = hbitmap;

        /* Create compatible DC */
        hdc = CreateCompatibleDC(NULL);
        if (!hdc) {
            LOG_WARN("Failed to create DC for skin image: %s", filepath);
            continue;
        }

        g_ui_skin.image_dcs[i] = hdc;

        /* Select bitmap into DC */
        g_ui_skin.original_objs[i] = SelectObject(hdc, hbitmap);

        LOG_DEBUG("Loaded skin image: %s", filepath);
    }

    /* Create composite DC for rendering */
    g_ui_skin.composite_dc = CreateCompatibleDC(NULL);
    if (g_ui_skin.composite_dc) {
        /* Create composite bitmap - 645x155 pixels (0x285 x 0x9b) */
        g_ui_skin.composite_bitmap = CreateCompatibleBitmap(
            g_ui_skin.image_dcs[0] ? g_ui_skin.image_dcs[0] : GetDC(NULL),
            SKIN_COMPOSITE_WIDTH,
            SKIN_COMPOSITE_HEIGHT
        );

        if (g_ui_skin.composite_bitmap) {
            g_ui_skin.composite_orig_obj = SelectObject(
                g_ui_skin.composite_dc,
                g_ui_skin.composite_bitmap
            );
        }
    }

    LOG_INFO("Loaded %d skin images from %s", SKIN_IMAGE_COUNT, skin_path);
    return 1;
}

/*
 * Get skin frame by index
 */
SkinFrame* ui_skin_get_frame(int index) {
    SkinFrame* frame;
    int i;

    if (index < 0 || index >= SKIN_MAX_FRAMES) {
        return NULL;
    }

    frame = g_ui_skin.frame_head;
    for (i = 0; i < index && frame; i++) {
        frame = frame->next;
    }

    return frame;
}

/*
 * Get current frame
 */
SkinFrame* ui_skin_get_current_frame(void) {
    return g_ui_skin.frame_current;
}

/*
 * Advance to next frame
 */
void ui_skin_next_frame(void) {
    if (g_ui_skin.frame_current && g_ui_skin.frame_current->next) {
        g_ui_skin.frame_current = g_ui_skin.frame_current->next;
        g_ui_skin.current_frame++;
    } else {
        /* Loop back to start */
        g_ui_skin.frame_current = g_ui_skin.frame_head;
        g_ui_skin.current_frame = 0;
    }
}

/*
 * Reset to first frame
 */
void ui_skin_reset(void) {
    g_ui_skin.frame_current = g_ui_skin.frame_head;
    g_ui_skin.current_frame = 0;
}

/*
 * Get skin image by type
 */
HBITMAP ui_skin_get_image(SkinImageType type) {
    if (type < 0 || type >= SKIN_IMAGE_COUNT) {
        return NULL;
    }
    return g_ui_skin.images[type];
}

/*
 * Get skin image DC
 */
HDC ui_skin_get_image_dc(SkinImageType type) {
    if (type < 0 || type >= SKIN_IMAGE_COUNT) {
        return NULL;
    }
    return g_ui_skin.image_dcs[type];
}

/*
 * Get composite DC
 */
HDC ui_skin_get_composite_dc(void) {
    return g_ui_skin.composite_dc;
}

/*
 * Render skin frame
 */
void ui_skin_render_frame(HDC dest_dc, int x, int y) {
    SkinFrame* frame;

    if (!dest_dc) return;

    frame = g_ui_skin.frame_current;
    if (!frame) return;

    /* Render from composite DC if available */
    if (g_ui_skin.composite_dc && g_ui_skin.composite_bitmap) {
        BitBlt(
            dest_dc,
            x, y,
            SKIN_COMPOSITE_WIDTH,
            SKIN_COMPOSITE_HEIGHT,
            g_ui_skin.composite_dc,
            0, 0,
            SRCCOPY
        );
    }
}

/*
 * Update animation
 */
void ui_skin_update(void) {
    /* Advance frame counter */
    ui_skin_next_frame();
}
