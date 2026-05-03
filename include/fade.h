/*
 * Stone Age Client - Fade Effect System
 * Reverse engineered from sa_9061.exe (FUN_0047bde0, FUN_0047a730, FUN_0047b7e0)
 * Handles screen fade and transition effects
 *
 * FUN_0047bde0: Main dispatcher with 30 fade types (0-0x1d)
 * FUN_0047a730: Standard fade effects (horizontal/vertical alpha)
 * FUN_0047aac0: Accelerated fade effects with deceleration
 * FUN_0047aea0: Box expand/contract effects
 * FUN_0047b180: Block dissolve effects (8x8 grid)
 * FUN_0047b7e0: Pixelate/block wipe effect
 * FUN_0047b9f0: Horizontal blind draw
 * FUN_0047bb30: Horizontal blind erase
 * FUN_0047bc80: Vertical blind effect
 */

#ifndef FADE_H
#define FADE_H

#include "types.h"

/*
 * Fade types from FUN_0047bde0 switch
 * Values directly match the switch case numbers in the binary
 *
 * Binary mapping pattern:
 * - Even cases 0,2,4,6,8,10: standard fade with variant 0-5
 * - Odd cases 1,3,5,7,9,11: accelerated fade with variant 0-5
 * - Cases 12-13: box expand/contract
 * - Cases 14-17: block dissolve with variants 0-3
 * - Cases 18-19: pixelate wipe
 * - Cases 20-22,27: horizontal blind draw
 * - Cases 23-26: horizontal blind erase
 * - Cases 28-29: vertical blind
 */
typedef enum {
    /* Cases 0-11: Alternating standard/accelerated fades */
    FADE_ALPHA_OUT_0 = 0,       /* case 0: standard(0) - alpha decrease */
    FADE_ACCEL_1 = 1,           /* case 1: accelerated(0) */
    FADE_STANDARD_2 = 2,        /* case 2: standard(1) */
    FADE_ACCEL_3 = 3,           /* case 3: accelerated(1) */
    FADE_STANDARD_4 = 4,        /* case 4: standard(2) */
    FADE_ACCEL_5 = 5,           /* case 5: accelerated(2) */
    FADE_STANDARD_6 = 6,        /* case 6: standard(3) */
    FADE_ACCEL_7 = 7,           /* case 7: accelerated(3) */
    FADE_STANDARD_8 = 8,        /* case 8: standard(4) */
    FADE_ACCEL_9 = 9,           /* case 9: accelerated(4) */
    FADE_STANDARD_10 = 10,      /* case 10: standard(5) */
    FADE_ACCEL_11 = 11,         /* case 0xb: accelerated(5) */

    /* FUN_0047aea0 - Box expand/contract (cases 0xc-0xd) */
    FADE_BOX_EXPAND = 12,       /* case 0xc: box(0) - expand from center */
    FADE_BOX_CONTRACT = 13,     /* case 0xd: box(1) - contract to center */

    /* FUN_0047b180 - Block dissolve (cases 0xe-0x11) */
    FADE_BLOCK_0 = 14,          /* case 0xe: block(0) */
    FADE_BLOCK_1 = 15,          /* case 0xf: block(1) */
    FADE_BLOCK_2 = 16,          /* case 0x10: block(2) */
    FADE_BLOCK_3 = 17,          /* case 0x11: block(3) */

    /* FUN_0047b7e0 - Pixelate wipe (cases 0x12-0x13) */
    FADE_PIXELATE_REVERSE = 18, /* case 0x12: pixelate_wipe(-1) */
    FADE_PIXELATE_FORWARD = 19, /* case 0x13: pixelate_wipe(1) */

    /* FUN_0047b9f0 - Horizontal blind draw (cases 0x14-0x16, 0x1b) */
    FADE_BLIND_H_DRAW_0 = 20,   /* case 0x14: blind_h_draw(-1) */
    FADE_BLIND_H_DRAW_1 = 21,   /* case 0x15: blind_h_draw(1) */
    FADE_BLIND_H_DRAW_2 = 22,   /* case 0x16: blind_h_draw(-1) */
    FADE_BLIND_H_DRAW_3 = 27,   /* case 0x1b: blind_h_draw(1) */

    /* FUN_0047bb30 - Horizontal blind erase (cases 0x17-0x1a) */
    FADE_BLIND_H_ERASE_0 = 23,  /* case 0x17: blind_h_erase(1) */
    FADE_BLIND_H_ERASE_1 = 24,  /* case 0x18: blind_h_erase(-1) */
    FADE_BLIND_H_ERASE_2 = 25,  /* case 0x19: blind_h_erase(1) */
    FADE_BLIND_H_ERASE_3 = 26,  /* case 0x1a: blind_h_erase(-1) */

    /* FUN_0047bc80 - Vertical blind (cases 0x1c-0x1d) */
    FADE_BLIND_V_REVERSE = 28,  /* case 0x1c: blind_vertical(0) */
    FADE_BLIND_V_FORWARD = 29   /* case 0x1d: blind_vertical(1) */

} FadeType;

/* Convenience aliases for common fade types */
#define FADE_WIPE_EXPAND    FADE_BOX_EXPAND      /* 12 - Box expand */
#define FADE_WIPE_CONTRACT  FADE_BOX_CONTRACT    /* 13 - Box contract */
#define FADE_BLIND          FADE_BLIND_V_REVERSE /* 28 - Vertical blind */
#define FADE_BLIND_REVERSE  FADE_BLIND_V_FORWARD /* 29 - Vertical blind reverse */
#define FADE_WIPE_RIGHT     FADE_PIXELATE_FORWARD /* 19 - Pixelate wipe forward */
#define FADE_WIPE_LEFT      FADE_PIXELATE_REVERSE /* 18 - Pixelate wipe reverse */

/* Fade state - matches DAT_005ab710 values */
typedef enum {
    FADE_STATE_IDLE = 0,
    FADE_STATE_FADING = 1,
    FADE_STATE_TRANSITION = 2,    /* Transition phase */
    FADE_STATE_ACTIVE = 3,        /* Active rendering */
    FADE_STATE_COMPLETE = 4
} FadeState;

/* Block dissolve cell states for FUN_0047b180 */
#define FADE_BLOCK_HIDDEN    0
#define FADE_BLOCK_APPEARING 1
#define FADE_BLOCK_VISIBLE   2

/* Screen dimensions for fade calculations */
#define FADE_SCREEN_WIDTH    640   /* 0x280 */
#define FADE_SCREEN_HEIGHT   480   /* 0x1E0 */
#define FADE_SCREEN_CENTER_X 320   /* 0x140 */
#define FADE_SCREEN_CENTER_Y 240   /* 0xF0 */

/* Block grid dimensions for FUN_0047b180 */
#define FADE_BLOCK_COLS      8
#define FADE_BLOCK_ROWS      8
#define FADE_BLOCK_COUNT     64    /* 0x40 */
#define FADE_BLOCK_WIDTH     80    /* 0x50 */
#define FADE_BLOCK_HEIGHT    60    /* 0x3C */

/* Wipe grid dimensions for FUN_0047b7e0 */
#define FADE_WIPE_COLS       42    /* 0x14A / 5 */
#define FADE_WIPE_ROWS       30    /* 0xF8 / 8 */

/*
 * Fade context - matches DAT_04633xxx region
 * Size: ~0x300 bytes
 */
typedef struct {
    /* Floating point fade values - DAT_046332dc..ec */
    float fade_x;           /* DAT_046332dc - horizontal offset */
    float fade_y;           /* DAT_046332e0 - vertical offset */
    float fade_alpha;       /* DAT_046332e4 - alpha/opacity */
    float fade_aux;         /* DAT_046332e8 - auxiliary value */
    float fade_speed;       /* DAT_046332ec - current speed */

    /* Accelerated fade values - DAT_046332f0..fc */
    float accel_pos;        /* DAT_046332f0 - position for accel fade */
    float accel_aux1;       /* DAT_046332f4 - auxiliary 1 */
    float accel_alpha;      /* DAT_046332f8 - alpha for accel fade */
    float accel_aux2;       /* DAT_046332fc - auxiliary 2 */

    /* Box effect values - DAT_046311b0..332d8 */
    int box_x1;             /* DAT_046311b0 */
    int box_y1;             /* DAT_046332cc */
    int box_x2;             /* DAT_046311b4 */
    int box_y2;             /* DAT_046332d0 */
    int box_x3;             /* DAT_046311b8 */
    int box_y3;             /* DAT_046332d4 */
    int box_x4;             /* DAT_046311bc */
    int box_y4;             /* DAT_046332d8 */

    /* Wipe effect state - DAT_04630e00, DAT_04633300 */
    int wipe_progress;      /* DAT_04630e00 - wipe position (0-64) */
    int wipe_active;        /* DAT_04633300 - wipe in progress flag */

    /* Block dissolve state - DAT_04630e0c..3100c */
    u8 block_state[FADE_BLOCK_COUNT];    /* DAT_04630e0c - cell states */
    u16 block_x[FADE_BLOCK_COUNT];       /* DAT_04630f0c - cell X positions */
    u16 block_y[FADE_BLOCK_COUNT];       /* DAT_0463108c - cell Y positions */
    int block_progress[FADE_BLOCK_COUNT]; /* DAT_046331cc - cell progress */
    int block_offset[FADE_BLOCK_COUNT];   /* DAT_046330c8 - cell offset */
    int block_complete_count;            /* DAT_0463100c - completed cells */
    int block_active_count;              /* DAT_04631088 - active cells */

    /* Effect state */
    FadeState state;
    FadeType current_type;
    u32 start_time;
    int init_flag;          /* DAT_004cf830 - init on first frame */
    int accel_init_flag;    /* DAT_004cf834 - accel fade init flag */
    int box_init_flag;      /* DAT_004cf838 - box effect init flag */
    int block_init_flag;    /* DAT_004cf83c - block dissolve init flag */

} FadeContext;

/* Global fade context */
extern FadeContext g_fade;

/* Initialize fade system */
int fade_init(void);

/* Shutdown fade system */
void fade_shutdown(void);

/*
 * Process fade effect - FUN_0047bde0 dispatcher
 * Returns 1 when fade is complete, 0 while fading
 */
int fade_process(FadeType type);

/*
 * Standard fade effects - FUN_0047a730
 * param_1: 0-5 for different fade directions
 * Returns 1 when complete, 0 while fading
 */
int fade_standard(int variant);

/*
 * Accelerated fade effects - FUN_0047aac0
 * param_1: 0-5 for different fade types
 * Returns 1 when complete, 0 while fading
 */
int fade_accelerated(int variant);

/*
 * Box expand/contract - FUN_0047aea0
 * param_1: 0 = expand, 1 = contract
 * Returns 1 when complete, 0 while fading
 */
int fade_box(int variant);

/*
 * Block dissolve effects - FUN_0047b180
 * param_1: 0-3 for different dissolve modes
 * Returns 1 when complete, 0 while fading
 */
int fade_block_dissolve(int variant);

/*
 * Pixelate wipe effect - FUN_0047b7e0
 * param_1: -1 for reverse, 1 for forward
 * Returns 1 when complete, 0 while fading
 */
int fade_pixelate_wipe(int direction);

/*
 * Horizontal blind draw - FUN_0047b9f0
 * param_1: -1 for reverse, 1 for forward
 * Returns 1 when complete, 0 while fading
 */
int fade_blind_h_draw(int direction);

/*
 * Horizontal blind erase - FUN_0047bb30
 * param_1: -1 for reverse, 1 for forward
 * Returns 1 when complete, 0 while fading
 */
int fade_blind_h_erase(int direction);

/*
 * Vertical blind effect - FUN_0047bc80
 * param_1: 0 = reverse (top to bottom), 1 = forward (bottom to top)
 * Returns 1 when complete, 0 while fading
 */
int fade_blind_vertical(int direction);

/*
 * Check if fade is complete
 */
int fade_is_complete(void);

/*
 * Reset fade state
 */
void fade_reset(void);

/*
 * Render fade overlay
 */
void fade_render(void);

/*
 * Set fade speed multiplier
 */
void fade_set_speed(float speed);

/*
 * Set/get render state - DAT_005ab710
 */
void fade_set_render_state(int state);
int fade_get_render_state(void);

/*
 * Random number generator for block dissolve
 */
int fade_rand(void);

#endif /* FADE_H */
