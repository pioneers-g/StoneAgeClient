/*
 * Stone Age Client - Animation Direction System
 * Split from animation.c for code organization
 *
 * Handles direction calculation, delta conversion, and angle computation
 * Based on FUN_00443e80, FUN_00447150 analysis
 *
 * Direction system: Isometric 8-direction movement
 * - Even directions (0,2,4,6): Diagonal movements
 * - Odd directions (1,3,5,7): Cardinal movements
 */

#include <windows.h>
#include <math.h>
#include "types.h"
#include "animation.h"

/* Direction angle table - for isometric coordinate conversion
 * Angles in degrees, starting from Southwest (0)
 */
static const float s_dir_angles[] = {
    225.0f,    /* SW (0) - Southwest diagonal */
    180.0f,    /* W (1)  - Pure West */
    135.0f,    /* NW (2) - Northwest diagonal */
    90.0f,     /* N (3)  - Pure North */
    45.0f,     /* NE (4) - Northeast diagonal */
    0.0f,      /* E (5)  - Pure East */
    315.0f,    /* SE (6) - Southeast diagonal */
    270.0f     /* S (7)  - Pure South */
};

/* Delta movement for each direction - FUN_00443e80 verified
 *
 * From binary FUN_00443e80 decompilation:
 *   Case 0: *param_2 = 0xffffffff (-1), *param_3 = 1
 *   Case 1: *param_2 = 0xffffffff (-1), *param_3 = 0
 *   Case 2: *param_2 = 0xffffffff (-1), *param_3 = 0xffffffff (-1)
 *   Case 3: *param_2 = 0, *param_3 = 0xffffffff (-1)
 *   Case 4: *param_2 = 1, *param_3 = 0xffffffff (-1)
 *   Case 5: *param_2 = 1, *param_3 = 0
 *   Case 6: *param_2 = 1, *param_3 = 1
 *   Case 7: *param_2 = 0, *param_3 = 1
 */
static const s16 s_dir_delta_x[] = { -1, -1, -1, 0, 1, 1, 1, 0 };
static const s16 s_dir_delta_y[] = { 1, 0, -1, -1, -1, 0, 1, 1 };

/*
 * Get direction delta for movement - FUN_00443e80 implementation
 */
void anim_get_direction_delta(u8 direction, s16* dx, s16* dy) {
    if (direction < 8) {
        if (dx) *dx = s_dir_delta_x[direction];
        if (dy) *dy = s_dir_delta_y[direction];
    } else {
        if (dx) *dx = 0;
        if (dy) *dy = 0;
    }
}

/*
 * Get direction from delta movement
 * Returns the closest direction for given delta values
 */
u8 anim_direction_from_delta(s16 dx, s16 dy) {
    int i;

    /* Handle zero movement */
    if (dx == 0 && dy == 0) {
        return ANIM_DIR_S;  /* Default to South */
    }

    /* Find matching direction */
    for (i = 0; i < 8; i++) {
        if (s_dir_delta_x[i] == dx && s_dir_delta_y[i] == dy) {
            return (u8)i;
        }
    }

    /* For non-exact matches, calculate angle-based direction */
    float angle = atan2f((float)dx, (float)-dy) * 180.0f / 3.14159265f;
    if (angle < 0) angle += 360.0f;

    /* Map angle to direction
     * E=0°, NE=45°, N=90°, NW=135°, W=180°, SW=225°, S=270°, SE=315°
     */
    u8 dir;
    if (angle >= 337.5f || angle < 22.5f) {
        dir = ANIM_DIR_E;      /* 0° - Pure East */
    } else if (angle >= 22.5f && angle < 67.5f) {
        dir = ANIM_DIR_NE;     /* 45° - Northeast diagonal */
    } else if (angle >= 67.5f && angle < 112.5f) {
        dir = ANIM_DIR_N;      /* 90° - Pure North */
    } else if (angle >= 112.5f && angle < 157.5f) {
        dir = ANIM_DIR_NW;     /* 135° - Northwest diagonal */
    } else if (angle >= 157.5f && angle < 202.5f) {
        dir = ANIM_DIR_W;      /* 180° - Pure West */
    } else if (angle >= 202.5f && angle < 247.5f) {
        dir = ANIM_DIR_SW;     /* 225° - Southwest diagonal */
    } else if (angle >= 247.5f && angle < 292.5f) {
        dir = ANIM_DIR_S;      /* 270° - Pure South */
    } else {
        dir = ANIM_DIR_SE;     /* 315° - Southeast diagonal */
    }

    return dir;
}

/*
 * Get direction to target
 */
u8 anim_direction_to_target(s16 from_x, s16 from_y, s16 to_x, s16 to_y) {
    return anim_direction_from_delta(to_x - from_x, to_y - from_y);
}

/*
 * Calculate direction from source to target - FUN_00447150 style
 * Uses isometric coordinate system
 */
u8 anim_calculate_direction(s16 src_x, s16 src_y, s16 dst_x, s16 dst_y) {
    s16 dx = dst_x - src_x;
    s16 dy = dst_y - src_y;

    return anim_direction_from_delta(dx, dy);
}

/*
 * Get angle for direction (for rotation calculations)
 */
float anim_get_direction_angle(u8 direction) {
    if (direction < 8) {
        return s_dir_angles[direction];
    }
    return 0.0f;
}
