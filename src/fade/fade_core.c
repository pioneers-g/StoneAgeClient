/*
 * Stone Age Client - Fade Effect Core System
 * Split from fade.c for code organization
 *
 * Handles initialization, shutdown, and main dispatcher
 * FUN_0047bde0 - fade_process() dispatcher with 30 fade types
 */

#include <windows.h>
#include <string.h>
#include "types.h"
#include "fade.h"
#include "logger.h"

/* Global fade context */
FadeContext g_fade = {0};

/* Fade speed constants from DAT_0049c44x region (extracted from binary) */
static const float s_fade_screen_width_f = 640.0f;     /* DAT_0049c440 */
static const float s_fade_screen_width_neg = -640.0f; /* DAT_0049c444 */
static const float s_fade_speed_alt = 0.4f;          /* DAT_0049c448 */
static const float s_fade_screen_height_f = 480.0f;    /* DAT_0049c44c */
static const float s_fade_screen_height_neg = -480.0f; /* DAT_0049c450 */
static const float s_fade_speed_base = 0.3f;         /* DAT_0049c454 */
static const float s_fade_threshold_458 = -600.0f;   /* DAT_0049c458 */
static const float s_fade_threshold_45c = 600.0f;    /* DAT_0049c45c */
static const float s_fade_accel_start_neg = -512.8f; /* DAT_0049c460 */
static const float s_fade_accel_start = 512.8f;      /* DAT_0049c464 */
static const float s_fade_val_468 = -0.5f;           /* DAT_0049c468 */
static const float s_fade_val_46c = 160.0f;          /* DAT_0049c46c */
const float FADE_SPEED_MIN = 0.0f;          /* DAT_0049c318 */

/* Internal constants for accelerated fade */
const float FADE_ACCEL_INITIAL = 17.4f;     /* Initial accel value */
const float FADE_BOX_SPEED = 22.8f;         /* Box effect speed */

/* Render mode state - DAT_005ab710 */
static int s_fade_render_state = 0;

/* Random number generator for block dissolve - LCG */
int fade_rand(void) {
    static u32 seed = 0x12345678;
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
}

/*
 * Initialize fade system
 */
int fade_init(void) {
    memset(&g_fade, 0, sizeof(FadeContext));
    g_fade.state = FADE_STATE_IDLE;
    g_fade.init_flag = 0;
    g_fade.accel_init_flag = -1;
    g_fade.box_init_flag = -1;
    g_fade.block_init_flag = -1;

    LOG_INFO("Fade system initialized");
    return 1;
}

/*
 * Shutdown fade system
 */
void fade_shutdown(void) {
    memset(&g_fade, 0, sizeof(FadeContext));
    LOG_INFO("Fade system shutdown");
}

/*
 * Process fade effect - FUN_0047bde0 dispatcher
 * Maps fade type to appropriate handler function
 */
int fade_process(FadeType type) {
    int result = 0;

    switch (type) {
        /* FUN_0047a730 - Standard fades (even cases 0-10) */
        case FADE_ALPHA_OUT_0:      /* case 0 */
            result = fade_standard(0);
            break;
        case FADE_ACCEL_1:          /* case 1 - accelerated */
            result = fade_accelerated(0);
            break;
        case FADE_STANDARD_2:       /* case 2 */
            result = fade_standard(1);
            break;
        case FADE_ACCEL_3:          /* case 3 - accelerated */
            result = fade_accelerated(1);
            break;
        case FADE_STANDARD_4:       /* case 4 */
            result = fade_standard(2);
            break;
        case FADE_ACCEL_5:          /* case 5 - accelerated */
            result = fade_accelerated(2);
            break;
        case FADE_STANDARD_6:       /* case 6 */
            result = fade_standard(3);
            break;
        case FADE_ACCEL_7:          /* case 7 - accelerated */
            result = fade_accelerated(3);
            break;
        case FADE_STANDARD_8:       /* case 8 */
            result = fade_standard(4);
            break;
        case FADE_ACCEL_9:          /* case 9 - accelerated */
            result = fade_accelerated(4);
            break;
        case FADE_STANDARD_10:      /* case 10 */
            result = fade_standard(5);
            break;
        case FADE_ACCEL_11:         /* case 11 - accelerated */
            result = fade_accelerated(5);
            break;

        /* FUN_0047aea0 - Box expand/contract (cases 12-13) */
        case FADE_BOX_EXPAND:       /* case 12 */
            result = fade_box(0);
            break;
        case FADE_BOX_CONTRACT:     /* case 13 */
            result = fade_box(1);
            break;

        /* FUN_0047b180 - Block dissolve (cases 14-17) */
        case FADE_BLOCK_0:          /* case 14 */
            result = fade_block_dissolve(0);
            break;
        case FADE_BLOCK_1:          /* case 15 */
            result = fade_block_dissolve(1);
            break;
        case FADE_BLOCK_2:          /* case 16 */
            result = fade_block_dissolve(2);
            break;
        case FADE_BLOCK_3:          /* case 17 */
            result = fade_block_dissolve(3);
            break;

        /* FUN_0047b7e0 - Pixelate wipe (cases 18-19) */
        case FADE_PIXELATE_REVERSE: /* case 18 */
            result = fade_pixelate_wipe(-1);
            break;
        case FADE_PIXELATE_FORWARD: /* case 19 */
            result = fade_pixelate_wipe(1);
            break;

        /* FUN_0047b9f0 - Horizontal blind draw (cases 20-22, 27) */
        case FADE_BLIND_H_DRAW_0:   /* case 20 */
            result = fade_blind_h_draw(-1);
            break;
        case FADE_BLIND_H_DRAW_1:   /* case 21 */
            result = fade_blind_h_draw(1);
            break;
        case FADE_BLIND_H_DRAW_2:   /* case 22 */
            result = fade_blind_h_draw(-1);
            break;
        case FADE_BLIND_H_DRAW_3:   /* case 27 */
            result = fade_blind_h_draw(1);
            break;

        /* FUN_0047bb30 - Horizontal blind erase (cases 23-26) */
        case FADE_BLIND_H_ERASE_0:  /* case 23 */
            result = fade_blind_h_erase(1);
            break;
        case FADE_BLIND_H_ERASE_1:  /* case 24 */
            result = fade_blind_h_erase(-1);
            break;
        case FADE_BLIND_H_ERASE_2:  /* case 25 */
            result = fade_blind_h_erase(1);
            break;
        case FADE_BLIND_H_ERASE_3:  /* case 26 */
            result = fade_blind_h_erase(-1);
            break;

        /* FUN_0047bc80 - Vertical blind (cases 28-29) */
        case FADE_BLIND_V_REVERSE:  /* case 28 */
            result = fade_blind_vertical(0);
            break;
        case FADE_BLIND_V_FORWARD:  /* case 29 */
            result = fade_blind_vertical(1);
            break;

        default:
            result = 1;
            break;
    }

    /* Complete fade - clear surfaces */
    if (result) {
        g_fade.state = FADE_STATE_COMPLETE;
    }

    return result;
}

/*
 * Check if fade is complete
 */
int fade_is_complete(void) {
    return g_fade.state == FADE_STATE_COMPLETE;
}

/*
 * Reset fade state
 */
void fade_reset(void) {
    g_fade.fade_x = 0.0f;
    g_fade.fade_y = 0.0f;
    g_fade.fade_alpha = 0.0f;
    g_fade.fade_aux = 0.0f;
    g_fade.fade_speed = 0.0f;
    g_fade.wipe_progress = 0;
    g_fade.wipe_active = 0;
    g_fade.state = FADE_STATE_IDLE;
    g_fade.init_flag = 1;
    g_fade.accel_init_flag = -1;
    g_fade.box_init_flag = -1;
    g_fade.block_init_flag = -1;
}

/*
 * Set fade speed multiplier
 */
void fade_set_speed(float speed) {
    g_fade.fade_speed = speed;
}

/*
 * Set the render state for fade transitions - DAT_005ab710
 */
void fade_set_render_state(int state) {
    s_fade_render_state = state;
}

/*
 * Get the render state
 */
int fade_get_render_state(void) {
    return s_fade_render_state;
}

/*
 * Check and update render state on completion
 */
int fade_check_complete(void) {
    if (s_fade_render_state == 2) {
        s_fade_render_state = 3;
        return 1;
    }
    if (s_fade_render_state == 3) {
        s_fade_render_state = 4;
    }
    return 0;
}
