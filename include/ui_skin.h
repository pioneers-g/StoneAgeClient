/*
 * Stone Age Client - UI Skin/Animation System Header
 * Reverse engineered from sa_9061.exe
 *
 * FUN_00488ca0 - Skin animation initialization
 * FUN_00489070 - Skin image loading
 *
 * This system manages UI skin animations with a linked list of frames.
 * It loads 5 skin images from data/skin/default/ and creates 40 animation frames.
 */

#ifndef UI_SKIN_H
#define UI_SKIN_H

#include "types.h"

/* Constants from binary analysis */
#define SKIN_MAX_FRAMES         40      /* 0x28 frames allocated */
#define SKIN_FRAME_SIZE         0x118   /* 280 bytes per frame entry */
#define SKIN_IMAGE_COUNT        5       /* 5 skin images loaded */
#define SKIN_COMPOSITE_WIDTH    645     /* 0x285 pixels */
#define SKIN_COMPOSITE_HEIGHT   155     /* 0x9b pixels */

/* Skin image types - indices into image array */
typedef enum {
    SKIN_IMAGE_DOWN_ARROW_R = 0,    /* Red down arrow */
    SKIN_IMAGE_DOWN_ARROW_G = 1,    /* Green down arrow */
    SKIN_IMAGE_BASE = 2,            /* Base skin */
    SKIN_IMAGE_3 = 3,
    SKIN_IMAGE_4 = 4
} SkinImageType;

/*
 * Skin frame entry - 0x118 (280) bytes
 * Linked list structure for animation frames
 */
#pragma pack(push, 1)
typedef struct SkinFrame {
    /* Offset 0x00: Image handles (5 entries, 4 bytes each = 20 bytes) */
    HBITMAP images[SKIN_IMAGE_COUNT];      /* 0x00-0x13 */

    /* Offset 0x14: Device contexts (5 entries, 4 bytes each = 20 bytes) */
    HDC     image_dcs[SKIN_IMAGE_COUNT];   /* 0x14-0x27 */

    /* Offset 0x28: Original GDI objects (5 entries) */
    HGDIOBJ original_objs[SKIN_IMAGE_COUNT]; /* 0x28-0x3b */

    /* Offset 0x3c: Frame-specific data */
    u32     frame_id;                       /* 0x3c */
    u32     flags;                          /* 0x40 */
    u32     width;                          /* 0x44 */
    u32     height;                         /* 0x48 */

    /* Offset 0x4c-0x10b: Additional frame data (192 bytes) */
    u8      custom_data[192];

    /* Offset 0x10c: Linked list pointers */
    struct SkinFrame* prev;                 /* 0x10c */
    struct SkinFrame* next;                 /* 0x110 */

    /* Offset 0x114: Padding/reserved */
    u32     reserved;

} SkinFrame;
#pragma pack(pop)

/*
 * Skin image entry - 0x20 (32) bytes
 * Describes a skin image to load
 */
#pragma pack(push, 1)
typedef struct {
    char    filename[16];       /* Image filename (relative to skin path) */
    u16     width;              /* Image width */
    u16     height;             /* Image height */
    u32     flags;              /* Image flags */
    u32     reserved[3];
} SkinImageEntry;
#pragma pack(pop)

/*
 * UI Skin context - matches FUN_00488ca0 structure layout
 * Total size: 0x7c (124) bytes based on offset analysis
 */
typedef struct {
    /* Offset 0x00: Window handle */
    HWND    hwnd;                       /* 0x00 */

    /* Offset 0x04: Unknown */
    u32     field_04;                   /* 0x04 */

    /* Offset 0x08: Unknown */
    u32     field_08;                   /* 0x08 */

    /* Offset 0x0c: Instance/skin path pointer */
    u32     instance;                   /* 0x0c */

    /* Offset 0x10-0x1b: Image handles array (5 entries) */
    HBITMAP images[SKIN_IMAGE_COUNT];   /* 0x10-0x23 - Note: overlaps with frame data */

    /* Image DCs for each image */
    HDC     image_dcs[SKIN_IMAGE_COUNT]; /* DCs for each skin image */
    HGDIOBJ original_objs[SKIN_IMAGE_COUNT]; /* Original GDI objects for each DC */

    /* Additional fields accessed by index */
    u32     field_12;                   /* Field 12 */
    u32     field_13;                   /* Field 13 */
    u32     field_14;                   /* Field 14 */
    u32     field_15;                   /* Field 15 */
    u32     field_16;                   /* Field 16 */
    u32     field_17;                   /* Field 17 */
    u32     field_18;                   /* Field 18 */

    /* Offset 0x24-0x47: Various state fields */
    u32     field_24;                   /* 0x24 */
    u32     field_28;                   /* 0x28 */
    u32     field_2c;                   /* 0x2c */
    u32     field_30;                   /* 0x30 */
    HDC     composite_dc;               /* 0x34 - Composite DC */
    HBITMAP composite_bitmap;           /* 0x38 - Composite bitmap */
    HGDIOBJ composite_orig_obj;         /* 0x3c - Original GDI object */
    u32     field_40;                   /* 0x40 */
    u32     field_44;                   /* 0x44 */

    /* Offset 0x48: Animation state */
    HDC     anim_dc;                    /* 0x48 */
    HBITMAP anim_bitmap;                /* 0x4c */
    HGDIOBJ anim_orig_obj;              /* 0x50 */
    u32     field_54;                   /* 0x54 */
    u32     field_58;                   /* 0x58 */
    u32     field_5c;                   /* 0x5c */

    /* Offset 0x60: Frame management */
    u32     field_60;                   /* 0x60 */
    u32     field_64;                   /* 0x64 */
    u32     field_68;                   /* 0x68 */
    u32     field_6c;                   /* 0x6c */
    u32     field_70;                   /* 0x70 */
    u32     field_74;                   /* 0x74 */
    u32     field_78;                   /* 0x78 */

    /* Offset 0x7c: Frame pointers */
    u32     initialized;                /* 0x7c - Set to 1 after init */

    /* Offset 0x80: Frame linked list */
    SkinFrame* frame_head;              /* 0x80 (0x19 * 4 = offset 0x64 from param_1) */
    SkinFrame* frame_current;           /* 0x84 */
    SkinFrame* frame_tail;              /* 0x88 */

    /* Additional frame data */
    u32     frame_count;                /* 0x8c */
    u32     current_frame;              /* 0x90 */

} UISkinContext;

/* Global skin context */
extern UISkinContext g_ui_skin;

/*
 * Initialize UI skin system - FUN_00488ca0
 * Creates linked list of 40 animation frames
 * Loads 5 skin images from data/skin/default/
 */
int ui_skin_init(HWND hwnd, const char* skin_path);

/*
 * Shutdown UI skin system - FUN_00488dc0
 */
void ui_skin_shutdown(void);

/*
 * Load skin images - FUN_00489070
 * Loads 5 BMP files from skin directory
 */
int ui_skin_load_images(const char* skin_path);

/*
 * Get skin frame by index
 */
SkinFrame* ui_skin_get_frame(int index);

/*
 * Get current frame
 */
SkinFrame* ui_skin_get_current_frame(void);

/*
 * Advance to next frame
 */
void ui_skin_next_frame(void);

/*
 * Reset to first frame
 */
void ui_skin_reset(void);

/*
 * Get skin image by type
 */
HBITMAP ui_skin_get_image(SkinImageType type);

/*
 * Get skin image DC
 */
HDC ui_skin_get_image_dc(SkinImageType type);

/*
 * Get composite DC
 */
HDC ui_skin_get_composite_dc(void);

/*
 * Render skin frame
 */
void ui_skin_render_frame(HDC dest_dc, int x, int y);

/*
 * Update animation
 */
void ui_skin_update(void);

#endif /* UI_SKIN_H */
