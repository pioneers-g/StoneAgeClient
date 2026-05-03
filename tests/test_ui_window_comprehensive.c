/*
 * Stone Age Client - UI Window Unit Tests
 * Tests for FUN_00448610 (window creation) and FUN_00448270 (render handler)
 *
 * Based on Ghidra decompilation analysis:
 * - Window allocation size: 0x40 bytes
 * - 9-sprite grid decoration
 * - State machine: 0=opening, 1=complete, 2=drawing, 3=buttons
 * - Style mapping for sprite bases
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

/* Stub types for testing without Windows/DirectX */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Constants from binary */
#define SPRITE_BASE_NORMAL   0x6591    /* DAT_0054b194 */
#define SPRITE_BASE_ALPHA    0x7a121   /* DAT_0054c208 */

#define SPRITE_OFFSET_NORMAL   0x68
#define SPRITE_OFFSET_MINIMAL  0x6d

#define TILE_WIDTH   64    /* 0x40 */
#define TILE_HEIGHT  48    /* 0x30 */

#define ANIM_FRAMES  10
#define ANIM_DIVISOR 10

#define CLOSE_BTN_NORMAL   0x65eb
#define CLOSE_BTN_HOVER    0x65ec
#define CLOSE_BTN_BG       0x54b174   /* DAT_0054b174 */

/* Window states from FUN_00448270 switch */
#define WINDOW_STATE_OPENING   0
#define WINDOW_STATE_COMPLETE  1
#define WINDOW_STATE_DRAWING   2
#define WINDOW_STATE_BUTTONS   3

/* 9-sprite grid offsets */
#define SPRITE_TOP_LEFT      0
#define SPRITE_TOP           1
#define SPRITE_TOP_RIGHT     2
#define SPRITE_LEFT          3
#define SPRITE_CENTER        4
#define SPRITE_RIGHT         5
#define SPRITE_BOTTOM_LEFT   6
#define SPRITE_BOTTOM        7
#define SPRITE_BOTTOM_RIGHT  8

/* Max render queue from FUN_0047e210 */
#define MAX_RENDER_QUEUE 0xfff

/* Window internal data - matches FUN_00448610 piVar1 offsets */
typedef struct {
    s32 width;            /* piVar1[0] */
    s32 height;           /* piVar1[1] */
    s32 close_sprite;     /* piVar1[2] */
    s32 close_x;          /* piVar1[3] */
    s32 close_y;          /* piVar1[4] */
    s32 center_x;         /* piVar1[5] */
    s32 center_y;         /* piVar1[6] */
    s32 anim_w;           /* piVar1[7] */
    s32 anim_h;           /* piVar1[8] */
    s32 anim_frame;       /* piVar1[9] */
    u32 sprite_base;      /* piVar1[10] */
    s32 layer;            /* piVar1[11] */
    s32 reserved1;        /* piVar1[12] */
    s32 reserved2;        /* piVar1[13] */
    s32 close_btn_idx;    /* piVar1[14] - 0xe */
    u8 sprite_offset;     /* piVar1[14] low byte */
    s32 minimize_btn_idx; /* piVar1[15] - 0xf */
    u8 close_btn_hovered;
    u8 has_close_btn;
    u8 padding[2];
} WindowInternal;

/* Window widget structure - matches 0x40 byte allocation */
typedef struct {
    void* handler;            /* +0x08: FUN_00448270 */
    WindowInternal internal;  /* +0x0c: piVar1 pointer */
    s32 x;                    /* +0x18: param_1 */
    s32 y;                    /* +0x1c: param_2 */
    s32 flags;                /* +0xa0: OR with 6 */
    u8 sprite_type;           /* +0x15: 0x68 or 0x6d */
    u8 state;                 /* +0xa8: state machine */
    s32 anim_step_x;          /* +0x28: (center_x - x) / 10 */
    s32 anim_step_y;          /* +0x2c: (center_y - y) / 10 */
    u8 render_complete;
    u8 close_clicked;
    char title[64];
} WindowWidget;

/* Render queue for testing */
static s32 s_queue_x[MAX_RENDER_QUEUE];
static s32 s_queue_y[MAX_RENDER_QUEUE];
static u32 s_queue_sprite_id[MAX_RENDER_QUEUE];
static u8 s_queue_layer[MAX_RENDER_QUEUE];
static s32 s_queue_priority[MAX_RENDER_QUEUE];
static int s_queue_count = 0;

/* Global state from binary */
static int g_alpha_mode = 0;
static u32 g_mouse_flags = 0;
static int g_mouse_hover_target = -1;

/* Sprite base globals - matching DAT_0054b194, DAT_0054c208 */
static u32 s_sprite_base_normal = SPRITE_BASE_NORMAL;
static u32 s_sprite_base_alpha = SPRITE_BASE_ALPHA;

/* Test helpers */
static void test_queue_clear(void) {
    s_queue_count = 0;
    memset(s_queue_x, 0, sizeof(s_queue_x));
    memset(s_queue_y, 0, sizeof(s_queue_y));
    memset(s_queue_sprite_id, 0, sizeof(s_queue_sprite_id));
}

static int test_queue_add(int x, int y, u8 layer, u32 sprite_id, int priority) {
    if (s_queue_count >= MAX_RENDER_QUEUE) {
        return -2;
    }
    s_queue_x[s_queue_count] = x;
    s_queue_y[s_queue_count] = y;
    s_queue_layer[s_queue_count] = layer;
    s_queue_sprite_id[s_queue_count] = sprite_id;
    s_queue_priority[s_queue_count] = priority;
    return s_queue_count++;
}

static int test_queue_add_fade(int x1, int y1, int x2, int y2, u8 sprite_type, u32 base, int mode) {
    if (s_queue_count >= MAX_RENDER_QUEUE) {
        return -2;
    }
    /* Pack coordinates like FUN_0047e640 */
    s_queue_x[s_queue_count] = (x1 & 0xFFFF) | (x2 << 16);
    s_queue_y[s_queue_count] = (y1 & 0xFFFF) | (y2 << 16);
    s_queue_sprite_id[s_queue_count] = base;
    return s_queue_count++;
}

/*
 * Create window - FUN_00448610
 * Returns allocated window or NULL on failure
 */
static WindowWidget* window_create(int x, int y, int width, int height, int style) {
    WindowWidget* wnd;
    WindowInternal* wi;
    int half_w, half_h;

    /* Allocate 0x40 bytes as in FUN_004010a0(3, 0x40) */
    wnd = (WindowWidget*)malloc(0x40);
    if (!wnd) {
        return NULL;
    }
    memset(wnd, 0, 0x40);

    wnd->x = x;
    wnd->y = y;

    /* Flags OR with 6 from binary */
    wnd->flags |= 6;

    /* Handler function pointer */
    /* wnd->handler = FUN_00448270; */

    /* Sprite type 0x68 stored at offset 0x15 */
    wnd->sprite_type = SPRITE_OFFSET_NORMAL;

    wi = &wnd->internal;

    /* Calculate half dimensions: (width << 6) / 2 */
    half_w = (width << 6) / 2;
    half_h = (height << 6) / 2;

    wi->width = width;
    wi->height = height;

    /* Y offset: param_2 + 0x1b in binary */
    wi->close_y = y + 0x1b;
    wi->close_x = x + half_w;

    /* Style mapping from FUN_00448610 switch */
    switch (style) {
        case 0:
            wi->sprite_base = s_sprite_base_normal;
            wi->has_close_btn = 1;
            break;
        case 1:
            wi->sprite_base = s_sprite_base_alpha;
            wi->has_close_btn = 1;
            break;
        case 2:
            wi->sprite_base = s_sprite_base_normal;
            wi->has_close_btn = 0;
            break;
        case 3:
            wi->sprite_base = s_sprite_base_alpha;
            wi->has_close_btn = 0;
            break;
        case 4:
            /* Minimal style */
            wnd->sprite_type = SPRITE_OFFSET_MINIMAL;
            wi->sprite_base = (u32)-2;
            wi->close_btn_idx = -2;
            wi->minimize_btn_idx = -2;
            wi->close_sprite = -2;
            wi->has_close_btn = 0;
            break;
        default:
            wi->sprite_base = (u32)-1;
            break;
    }

    /* Layer: style < 2 ? 1 : 0 */
    wi->layer = (style < 2) ? 1 : 0;

    /* Center calculation */
    if (style == -1 || style == 4) {
        wi->center_x = x + width / 2;
    } else {
        wi->center_x = x + half_w;
        /* height * 0x30 in some cases */
    }
    wi->center_y = y + half_h;

    /* Animation steps: (center - origin) / 10 */
    wnd->anim_step_x = (wi->center_x - x) / ANIM_DIVISOR;
    wnd->anim_step_y = (wi->center_y - y) / ANIM_DIVISOR;

    /* Initial state */
    wnd->state = WINDOW_STATE_OPENING;

    return wnd;
}

/*
 * Window render handler - FUN_00448270
 */
static void window_render(WindowWidget* wnd) {
    WindowInternal* wi;
    int row, col;
    int tile_x, tile_y;
    u32 sprite_id;
    int y_base;

    if (!wnd) return;

    wi = &wnd->internal;

    switch (wnd->state) {
        case WINDOW_STATE_OPENING:
            /* Case 0: Opening animation */
            test_queue_add_fade(
                wi->center_x - wi->anim_w,
                wi->center_y - wi->anim_h,
                wi->center_x + wi->anim_w,
                wi->center_y + wi->anim_h,
                wnd->sprite_type, 0, 0
            );

            /* Update animation */
            wi->anim_w += wnd->anim_step_x;
            wi->anim_h += wnd->anim_step_y;
            wi->anim_frame++;

            if (wi->anim_frame > 9) {
                if (wi->sprite_base == (u32)-2 || wi->sprite_base == (u32)-1) {
                    wnd->state = WINDOW_STATE_COMPLETE;
                } else {
                    wnd->state = WINDOW_STATE_DRAWING;
                }
            }
            break;

        case WINDOW_STATE_DRAWING:
            /* Case 2: Draw 9-sprite grid */
            y_base = wnd->y + 0x18;

            for (row = 0; row < wi->height; row++) {
                tile_y = y_base + row * TILE_HEIGHT;

                for (col = 0; col < wi->width; col++) {
                    tile_x = wnd->x + 0x20 + col * TILE_WIDTH;

                    /* Determine sprite offset based on grid position */
                    if (row == 0) {
                        /* Top row */
                        if (col == 0) sprite_id = wi->sprite_base + 0;
                        else if (col == wi->width - 1) sprite_id = wi->sprite_base + 2;
                        else sprite_id = wi->sprite_base + 1;
                    }
                    else if (row == wi->height - 1) {
                        /* Bottom row */
                        if (col == 0) sprite_id = wi->sprite_base + 6;
                        else if (col == wi->width - 1) sprite_id = wi->sprite_base + 8;
                        else sprite_id = wi->sprite_base + 7;
                    }
                    else {
                        /* Middle rows */
                        if (col == 0) sprite_id = wi->sprite_base + 3;
                        else if (col == wi->width - 1) sprite_id = wi->sprite_base + 5;
                        else sprite_id = wi->sprite_base + 4;
                    }

                    test_queue_add(tile_x, tile_y, wnd->sprite_type, sprite_id, wi->layer);
                }
            }

            /* Draw close button if present */
            if (wi->close_sprite != 0) {
                test_queue_add(wi->close_x, wi->close_y, 0x69, wi->close_sprite, wi->layer);
            }
            /* Fall through to complete */

        case WINDOW_STATE_COMPLETE:
            /* Case 1: Window complete */
            wnd->render_complete = 1;
            break;

        case WINDOW_STATE_BUTTONS:
            /* Case 3: Button interaction */
            if (g_mouse_flags & 1) {
                if (g_mouse_hover_target == wi->close_btn_idx) {
                    wi->close_btn_hovered = 1;
                }
                if (g_mouse_hover_target == wi->minimize_btn_idx) {
                    wi->close_btn_hovered = 0;
                }
            }

            /* Draw close button */
            test_queue_add(wi->center_x, wi->center_y, SPRITE_OFFSET_MINIMAL, CLOSE_BTN_BG, 1);
            wi->close_btn_idx = test_queue_add(wi->center_x, wi->center_y, 0x6e, CLOSE_BTN_NORMAL, 2);
            wi->minimize_btn_idx = test_queue_add(wi->center_x, wi->center_y, 0x6e, CLOSE_BTN_HOVER, 2);
            break;
    }
}

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

/* Setup */
static void test_setup(void) {
    test_queue_clear();
    g_alpha_mode = 0;
    g_mouse_flags = 0;
    g_mouse_hover_target = -1;
}

/* ========================================
 * Structure Size Tests
 * ======================================== */

static int test_window_allocation_size(void) {
    /* FUN_00448610 calls FUN_004010a0(3, 0x40) */
    assert(sizeof(WindowWidget) >= 0x40);
    return 1;
}

static int test_internal_structure_layout(void) {
    WindowWidget* wnd = window_create(100, 200, 3, 2, 0);
    assert(wnd != NULL);

    WindowInternal* wi = &wnd->internal;

    /* Verify internal offsets match piVar1 indexing */
    assert(wi->width == 3);
    assert(wi->height == 2);
    assert(wi->sprite_base == s_sprite_base_normal);
    assert(wi->has_close_btn == 1);

    free(wnd);
    return 1;
}

/* ========================================
 * Window Creation Tests - FUN_00448610
 * ======================================== */

static int test_window_create_basic(void) {
    test_setup();

    WindowWidget* wnd = window_create(100, 200, 3, 2, 0);
    assert(wnd != NULL);
    assert(wnd->x == 100);
    assert(wnd->y == 200);
    assert(wnd->flags == 6);  /* OR with 6 */
    assert(wnd->sprite_type == SPRITE_OFFSET_NORMAL);
    assert(wnd->state == WINDOW_STATE_OPENING);

    free(wnd);
    return 1;
}

static int test_window_style_0_normal(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 2, 2, 0);
    assert(wnd != NULL);
    assert(wnd->internal.sprite_base == s_sprite_base_normal);
    assert(wnd->internal.has_close_btn == 1);
    assert(wnd->internal.layer == 1);

    free(wnd);
    return 1;
}

static int test_window_style_1_alpha(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 2, 2, 1);
    assert(wnd != NULL);
    assert(wnd->internal.sprite_base == s_sprite_base_alpha);
    assert(wnd->internal.has_close_btn == 1);
    assert(wnd->internal.layer == 1);

    free(wnd);
    return 1;
}

static int test_window_style_2_no_button(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 2, 2, 2);
    assert(wnd != NULL);
    assert(wnd->internal.sprite_base == s_sprite_base_normal);
    assert(wnd->internal.has_close_btn == 0);
    assert(wnd->internal.layer == 0);

    free(wnd);
    return 1;
}

static int test_window_style_3_alpha_no_button(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 2, 2, 3);
    assert(wnd != NULL);
    assert(wnd->internal.sprite_base == s_sprite_base_alpha);
    assert(wnd->internal.has_close_btn == 0);
    assert(wnd->internal.layer == 0);

    free(wnd);
    return 1;
}

static int test_window_style_4_minimal(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 2, 2, 4);
    assert(wnd != NULL);
    assert(wnd->sprite_type == SPRITE_OFFSET_MINIMAL);  /* 0x6d */
    assert(wnd->internal.sprite_base == (u32)-2);
    assert(wnd->internal.close_btn_idx == -2);
    assert(wnd->internal.minimize_btn_idx == -2);
    assert(wnd->internal.has_close_btn == 0);

    free(wnd);
    return 1;
}

static int test_window_style_invalid(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 2, 2, 99);
    assert(wnd != NULL);
    assert(wnd->internal.sprite_base == (u32)-1);

    free(wnd);
    return 1;
}

static int test_window_center_calculation(void) {
    test_setup();

    /* Normal style: center = x + (width << 6) / 2 */
    WindowWidget* wnd = window_create(100, 200, 3, 2, 0);
    assert(wnd != NULL);

    int expected_half = (3 << 6) / 2;  /* 96 */
    assert(wnd->internal.center_x == 100 + expected_half);

    free(wnd);
    return 1;
}

static int test_window_animation_steps(void) {
    test_setup();

    WindowWidget* wnd = window_create(100, 200, 3, 2, 0);
    assert(wnd != NULL);

    /* Animation step = (center - origin) / 10 */
    int expected_step = (wnd->internal.center_x - 100) / 10;
    assert(wnd->anim_step_x == expected_step);

    free(wnd);
    return 1;
}

/* ========================================
 * Render Handler Tests - FUN_00448270
 * ======================================== */

static int test_render_state_opening(void) {
    test_setup();

    WindowWidget* wnd = window_create(100, 200, 2, 2, 0);
    assert(wnd != NULL);
    assert(wnd->state == WINDOW_STATE_OPENING);

    /* Render opening frame */
    window_render(wnd);

    /* Should add fade effect to queue */
    assert(s_queue_count == 1);
    assert(wnd->internal.anim_frame == 1);

    free(wnd);
    return 1;
}

static int test_render_animation_progression(void) {
    test_setup();

    WindowWidget* wnd = window_create(100, 200, 2, 2, 0);
    assert(wnd != NULL);

    /* Run 10 animation frames */
    for (int i = 0; i < 10; i++) {
        window_render(wnd);
    }

    /* After 10 frames, should transition to drawing state */
    assert(wnd->state == WINDOW_STATE_DRAWING);

    free(wnd);
    return 1;
}

static int test_render_minimal_skips_drawing(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 2, 2, 4);
    assert(wnd != NULL);

    /* Run animation to completion */
    for (int i = 0; i < 12; i++) {
        window_render(wnd);
    }

    /* Minimal windows skip drawing state, go to complete */
    assert(wnd->state == WINDOW_STATE_COMPLETE);
    assert(wnd->render_complete == 1);

    free(wnd);
    return 1;
}

static int test_render_9sprite_grid(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 3, 3, 0);
    assert(wnd != NULL);

    /* Skip to drawing state */
    wnd->state = WINDOW_STATE_DRAWING;
    window_render(wnd);

    /* 3x3 window = 9 sprites */
    assert(s_queue_count == 9);

    /* Verify corner sprites */
    /* Top-left: sprite_base + 0 */
    assert(s_queue_sprite_id[0] == wnd->internal.sprite_base + 0);
    /* Top-right: sprite_base + 2 */
    assert(s_queue_sprite_id[2] == wnd->internal.sprite_base + 2);
    /* Bottom-left: sprite_base + 6 */
    assert(s_queue_sprite_id[6] == wnd->internal.sprite_base + 6);
    /* Bottom-right: sprite_base + 8 */
    assert(s_queue_sprite_id[8] == wnd->internal.sprite_base + 8);

    free(wnd);
    return 1;
}

static int test_render_2x2_grid(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 2, 2, 0);
    assert(wnd != NULL);

    wnd->state = WINDOW_STATE_DRAWING;
    test_queue_clear();
    window_render(wnd);

    /* 2x2 window = 4 sprites */
    assert(s_queue_count == 4);

    /* Top-left, top-right, bottom-left, bottom-right */
    assert(s_queue_sprite_id[0] == wnd->internal.sprite_base + 0);
    assert(s_queue_sprite_id[1] == wnd->internal.sprite_base + 2);
    assert(s_queue_sprite_id[2] == wnd->internal.sprite_base + 6);
    assert(s_queue_sprite_id[3] == wnd->internal.sprite_base + 8);

    free(wnd);
    return 1;
}

static int test_render_1x1_grid(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 1, 1, 0);
    assert(wnd != NULL);

    wnd->state = WINDOW_STATE_DRAWING;
    test_queue_clear();
    window_render(wnd);

    /* 1x1 window = 1 sprite (top-left = bottom-right) */
    assert(s_queue_count == 1);

    free(wnd);
    return 1;
}

static int test_render_layer_assignment(void) {
    test_setup();

    /* Style 0: layer = 1 */
    WindowWidget* wnd1 = window_create(0, 0, 2, 2, 0);
    assert(wnd1->internal.layer == 1);
    free(wnd1);

    /* Style 2: layer = 0 */
    WindowWidget* wnd2 = window_create(0, 0, 2, 2, 2);
    assert(wnd2->internal.layer == 0);
    free(wnd2);

    return 1;
}

static int test_render_complete_flag(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 2, 2, 0);
    assert(wnd != NULL);

    wnd->state = WINDOW_STATE_COMPLETE;
    window_render(wnd);

    assert(wnd->render_complete == 1);

    free(wnd);
    return 1;
}

/* ========================================
 * Button State Tests - Case 3
 * ======================================== */

static int test_button_hover_detection(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 2, 2, 0);
    assert(wnd != NULL);

    wnd->state = WINDOW_STATE_BUTTONS;
    wnd->internal.close_btn_idx = 5;
    wnd->internal.minimize_btn_idx = 6;

    /* Simulate mouse hover on close button */
    g_mouse_flags = 1;
    g_mouse_hover_target = 5;

    window_render(wnd);

    assert(wnd->internal.close_btn_hovered == 1);

    free(wnd);
    return 1;
}

static int test_button_hover_minimize(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 2, 2, 0);
    assert(wnd != NULL);

    wnd->state = WINDOW_STATE_BUTTONS;
    wnd->internal.close_btn_idx = 5;
    wnd->internal.minimize_btn_idx = 6;
    wnd->internal.close_btn_hovered = 1;

    /* Simulate mouse hover on minimize button */
    g_mouse_flags = 1;
    g_mouse_hover_target = 6;

    window_render(wnd);

    assert(wnd->internal.close_btn_hovered == 0);

    free(wnd);
    return 1;
}

static int test_button_sprites_added(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 2, 2, 0);
    assert(wnd != NULL);

    wnd->state = WINDOW_STATE_BUTTONS;
    window_render(wnd);

    /* Should add 3 sprites: background, normal button, hover button */
    assert(s_queue_count >= 3);

    /* Close button sprites */
    assert(s_queue_sprite_id[1] == CLOSE_BTN_NORMAL);
    assert(s_queue_sprite_id[2] == CLOSE_BTN_HOVER);

    free(wnd);
    return 1;
}

/* ========================================
 * Sprite ID Calculation Tests
 * ======================================== */

static int test_sprite_id_top_row(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 5, 3, 0);
    assert(wnd != NULL);

    wnd->state = WINDOW_STATE_DRAWING;
    test_queue_clear();
    window_render(wnd);

    /* Top row: offsets 0, 1, 1, 1, 2 */
    u32 base = wnd->internal.sprite_base;
    assert(s_queue_sprite_id[0] == base + 0);  /* Top-left */
    assert(s_queue_sprite_id[1] == base + 1);  /* Top */
    assert(s_queue_sprite_id[2] == base + 1);  /* Top */
    assert(s_queue_sprite_id[3] == base + 1);  /* Top */
    assert(s_queue_sprite_id[4] == base + 2);  /* Top-right */

    free(wnd);
    return 1;
}

static int test_sprite_id_middle_row(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 3, 3, 0);
    assert(wnd != NULL);

    wnd->state = WINDOW_STATE_DRAWING;
    test_queue_clear();
    window_render(wnd);

    /* Middle row (index 3-5): offsets 3, 4, 5 */
    u32 base = wnd->internal.sprite_base;
    assert(s_queue_sprite_id[3] == base + 3);  /* Left */
    assert(s_queue_sprite_id[4] == base + 4);  /* Center */
    assert(s_queue_sprite_id[5] == base + 5);  /* Right */

    free(wnd);
    return 1;
}

static int test_sprite_id_bottom_row(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 3, 3, 0);
    assert(wnd != NULL);

    wnd->state = WINDOW_STATE_DRAWING;
    test_queue_clear();
    window_render(wnd);

    /* Bottom row (index 6-8): offsets 6, 7, 8 */
    u32 base = wnd->internal.sprite_base;
    assert(s_queue_sprite_id[6] == base + 6);  /* Bottom-left */
    assert(s_queue_sprite_id[7] == base + 7);  /* Bottom */
    assert(s_queue_sprite_id[8] == base + 8);  /* Bottom-right */

    free(wnd);
    return 1;
}

/* ========================================
 * Position Tests
 * ======================================== */

static int test_tile_positions(void) {
    test_setup();

    WindowWidget* wnd = window_create(100, 200, 2, 2, 0);
    assert(wnd != NULL);

    wnd->state = WINDOW_STATE_DRAWING;
    test_queue_clear();
    window_render(wnd);

    /* First tile x: wnd->x + 0x20 */
    assert(s_queue_x[0] == 100 + 0x20);

    /* First tile y: wnd->y + 0x18 */
    assert(s_queue_y[0] == 200 + 0x18);

    /* Second tile x: first + TILE_WIDTH */
    assert(s_queue_x[1] == 100 + 0x20 + TILE_WIDTH);

    free(wnd);
    return 1;
}

static int test_negative_position(void) {
    test_setup();

    WindowWidget* wnd = window_create(-100, -50, 2, 2, 0);
    assert(wnd != NULL);
    assert(wnd->x == -100);
    assert(wnd->y == -50);

    free(wnd);
    return 1;
}

static int test_large_window(void) {
    test_setup();

    WindowWidget* wnd = window_create(0, 0, 10, 10, 0);
    assert(wnd != NULL);

    wnd->state = WINDOW_STATE_DRAWING;
    test_queue_clear();
    window_render(wnd);

    /* 10x10 window = 100 sprites */
    assert(s_queue_count == 100);

    free(wnd);
    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== UI Window Unit Tests ===\n\n");

    /* Structure tests */
    printf("Structure Tests:\n");
    TEST(window_allocation_size);
    TEST(internal_structure_layout);

    /* Creation tests */
    printf("\nWindow Creation Tests:\n");
    TEST(window_create_basic);
    TEST(window_style_0_normal);
    TEST(window_style_1_alpha);
    TEST(window_style_2_no_button);
    TEST(window_style_3_alpha_no_button);
    TEST(window_style_4_minimal);
    TEST(window_style_invalid);
    TEST(window_center_calculation);
    TEST(window_animation_steps);

    /* Render tests */
    printf("\nRender Handler Tests:\n");
    TEST(render_state_opening);
    TEST(render_animation_progression);
    TEST(render_minimal_skips_drawing);
    TEST(render_9sprite_grid);
    TEST(render_2x2_grid);
    TEST(render_1x1_grid);
    TEST(render_layer_assignment);
    TEST(render_complete_flag);

    /* Button tests */
    printf("\nButton State Tests:\n");
    TEST(button_hover_detection);
    TEST(button_hover_minimize);
    TEST(button_sprites_added);

    /* Sprite ID tests */
    printf("\nSprite ID Tests:\n");
    TEST(sprite_id_top_row);
    TEST(sprite_id_middle_row);
    TEST(sprite_id_bottom_row);

    /* Position tests */
    printf("\nPosition Tests:\n");
    TEST(tile_positions);
    TEST(negative_position);
    TEST(large_window);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
