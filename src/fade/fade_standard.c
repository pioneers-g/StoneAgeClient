/*
 * Stone Age Client - Fade Standard and Accelerated Effects
 * Split from fade.c for code organization
 *
 * FUN_0047a730 - fade_standard() - Standard alpha fades
 * FUN_0047aac0 - fade_accelerated() - Accelerated fades with deceleration
 */

#include <windows.h>
#include "types.h"
#include "fade.h"
#include "render.h"

/* Fade constants - local definitions */
static const float FADE_SPEED_ALT = 0.4f;
static const float FADE_SPEED_BASE = 0.3f;
static const float FADE_SPEED_MIN = 0.01f;
static const float FADE_SCREEN_WIDTH_NEG = -640.0f;
static const float FADE_SCREEN_HEIGHT_NEG = -480.0f;
static const float FADE_ACCEL_START_NEG = -512.8f;
static const float FADE_ACCEL_START = 512.8f;
static const float FADE_VAL_46C = 160.0f;
static const float FADE_THRESHOLD_45C = 600.0f;
static const float FADE_BOX_SPEED = 40.0f;
static const float FADE_ACCEL_INITIAL = 20.0f;

/* External functions from fade_core.c */
extern int fade_check_complete(void);

/* External global */
extern FadeContext g_fade;

/*
 * Standard fade effects - FUN_0047a730
 * Handles basic alpha and positional fade transitions
 */
int fade_standard(int variant) {
    int complete = 0;

    /* Initialize on first frame - DAT_004cf830 check */
    if (g_fade.init_flag) {
        g_fade.init_flag = 0;
        g_fade.fade_x = 0.0f;
        g_fade.fade_y = 0.0f;
        g_fade.fade_alpha = 0.0f;
        g_fade.fade_aux = 0.0f;
        g_fade.fade_speed = 0.0f;
    }

    switch (variant) {
        case 0:
            /* Fade out - decrease alpha, increase speed */
            g_fade.fade_alpha -= g_fade.fade_speed;
            g_fade.fade_speed += FADE_SPEED_BASE;
            if (g_fade.fade_alpha <= FADE_SCREEN_HEIGHT_NEG) {
                complete = 1;
            }
            break;

        case 1:
            /* Fade in - increase alpha and speed */
            g_fade.fade_alpha += g_fade.fade_speed;
            g_fade.fade_speed += FADE_SPEED_BASE;
            if (g_fade.fade_alpha >= FADE_SCREEN_HEIGHT) {
                complete = 1;
            }
            break;

        case 2:
            /* Horizontal fade out */
            g_fade.fade_x -= g_fade.fade_speed;
            g_fade.fade_speed += FADE_SPEED_ALT;
            if (g_fade.fade_x <= FADE_SCREEN_WIDTH_NEG) {
                complete = 1;
            }
            break;

        case 3:
            /* Horizontal fade in */
            g_fade.fade_x += g_fade.fade_speed;
            g_fade.fade_speed += FADE_SPEED_ALT;
            if (g_fade.fade_x >= FADE_SCREEN_WIDTH) {
                complete = 1;
            }
            break;

        case 4:
            /* Combined X/Y fade - striped effect */
            g_fade.fade_x += g_fade.fade_speed;
            g_fade.fade_y -= g_fade.fade_speed;
            g_fade.fade_speed += FADE_SPEED_ALT;
            if (g_fade.fade_x >= FADE_SCREEN_WIDTH) {
                complete = 1;
            }
            break;

        case 5:
            /* Combined alpha/aux fade */
            g_fade.fade_alpha += g_fade.fade_speed;
            g_fade.fade_aux -= g_fade.fade_speed;
            g_fade.fade_speed += FADE_SPEED_BASE;
            if (g_fade.fade_alpha >= FADE_SCREEN_HEIGHT) {
                complete = 1;
            }
            break;

        default:
            break;
    }

    /* Render fade overlay - FUN_00414420 */
    fade_render();

    if (complete) {
        g_fade.fade_x = 0.0f;
        g_fade.fade_y = 0.0f;
        g_fade.fade_alpha = 0.0f;
        g_fade.fade_aux = 0.0f;
        g_fade.fade_speed = 0.0f;

        fade_check_complete();
    }

    return complete;
}

/*
 * Accelerated fade effects - FUN_0047aac0
 * Fades that accelerate then decelerate for smoother transitions
 */
int fade_accelerated(int variant) {
    int complete = 0;

    /* Initialize on first frame - DAT_004cf834 check */
    if (g_fade.accel_init_flag == -1) {
        g_fade.accel_init_flag = 0;

        switch (variant) {
            case 0:
                g_fade.accel_pos = 0.0f;
                g_fade.accel_alpha = FADE_ACCEL_INITIAL;
                g_fade.fade_speed = FADE_ACCEL_START;
                break;
            case 1:
                g_fade.accel_pos = 0.0f;
                g_fade.accel_alpha = FADE_ACCEL_INITIAL;
                g_fade.fade_speed = FADE_ACCEL_START_NEG;
                break;
            case 2:
                g_fade.accel_alpha = FADE_BOX_SPEED;
                g_fade.fade_speed = 0.0f;
                g_fade.accel_pos = FADE_THRESHOLD_45C - FADE_BOX_SPEED;
                break;
            case 3:
                g_fade.fade_speed = 0.0f;
                g_fade.accel_alpha = FADE_BOX_SPEED;
                g_fade.accel_pos = FADE_VAL_46C;
                break;
            case 4:
                g_fade.accel_aux1 = 661.0f;
                g_fade.fade_speed = 0.0f;
                g_fade.accel_alpha = FADE_BOX_SPEED;
                g_fade.accel_pos = FADE_VAL_46C;
                break;
            case 5:
                g_fade.accel_pos = 0.0f;
                g_fade.accel_aux2 = 513.2f;
                g_fade.accel_alpha = FADE_ACCEL_INITIAL;
                g_fade.fade_speed = FADE_ACCEL_START_NEG;
                break;
            default:
                break;
        }
    }

    switch (variant) {
        case 0:
            /* Accelerated fade out - alpha decreases, speed decelerates */
            g_fade.fade_speed -= g_fade.accel_alpha;
            g_fade.accel_alpha -= FADE_SPEED_BASE;
            if (g_fade.accel_alpha <= FADE_SPEED_MIN) {
                g_fade.accel_init_flag = 1;
            }
            break;

        case 1:
            /* Accelerated fade in - alpha increases, speed decelerates */
            g_fade.fade_speed += g_fade.accel_alpha;
            g_fade.accel_alpha -= FADE_SPEED_BASE;
            if (g_fade.accel_alpha <= FADE_SPEED_MIN) {
                g_fade.accel_init_flag = 1;
            }
            break;

        case 2:
            /* Horizontal accelerated fade out */
            g_fade.accel_pos -= g_fade.accel_alpha;
            g_fade.accel_alpha -= FADE_SPEED_ALT;
            if (g_fade.accel_alpha <= FADE_SPEED_MIN) {
                g_fade.accel_init_flag = 1;
            }
            break;

        case 3:
            /* Horizontal accelerated fade in */
            g_fade.accel_pos += g_fade.accel_alpha;
            g_fade.accel_alpha -= FADE_SPEED_ALT;
            if (g_fade.accel_alpha <= FADE_SPEED_MIN) {
                g_fade.accel_init_flag = 1;
            }
            break;

        case 4:
            /* Combined XY accelerated fade */
            g_fade.accel_pos += g_fade.accel_alpha;
            g_fade.accel_aux1 -= g_fade.accel_alpha;
            g_fade.accel_alpha -= FADE_SPEED_ALT;
            if (g_fade.accel_alpha <= FADE_SPEED_MIN) {
                g_fade.accel_init_flag = 1;
            }
            break;

        case 5:
            /* Combined alpha accelerated fade */
            g_fade.fade_speed += g_fade.accel_alpha;
            g_fade.accel_aux2 -= g_fade.accel_alpha;
            g_fade.accel_alpha -= FADE_SPEED_BASE;
            if (g_fade.accel_alpha <= FADE_SPEED_MIN) {
                g_fade.accel_init_flag = 1;
            }
            break;

        default:
            break;
    }

    fade_render();

    if (g_fade.accel_init_flag == 1) {
        fade_check_complete();
        g_fade.accel_init_flag = -1;
        return 1;
    }

    return 0;
}
