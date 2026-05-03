/*
 * Stone Age Client - Movement Path Calculation
 * Character movement path system with obstacle avoidance
 * Reverse engineered from FUN_00443fe0, FUN_00443e80, FUN_00447150
 * Split from pathfind.c (1361 lines -> ~450 lines)
 */

#include <windows.h>
#include <math.h>
#include <string.h>
#include "types.h"
#include "pathfind.h"
#include "map.h"
#include "logger.h"

/* Global movement path context */
MovementPath g_movement_path = {0};

/* Direction deltas - matching FUN_00443e80 exactly (VERIFIED CORRECT)
 *
 * Binary decompilation of FUN_00443e80:
 *   case 0: *dx = -1 (0xffffffff), *dy = 1
 *   case 1: *dx = -1, *dy = 0
 *   case 2: *dx = -1, *dy = -1 (0xffffffff)
 *   case 3: *dx = 0, *dy = -1 (0xffffffff)
 *   case 4: *dx = 1, *dy = -1 (0xffffffff)
 *   case 5: *dx = 1, *dy = 0
 *   case 6: *dx = 1, *dy = 1
 *   case 7: *dx = 0, *dy = 1
 *
 * Direction naming (assuming screen coordinates with Y increasing downward):
 *   Direction 0: dx=-1, dy=1  = Southwest
 *   Direction 1: dx=-1, dy=0  = West
 *   Direction 2: dx=-1, dy=-1 = Northwest
 *   Direction 3: dx=0, dy=-1  = North
 *   Direction 4: dx=1, dy=-1  = Northeast
 *   Direction 5: dx=1, dy=0   = East
 *   Direction 6: dx=1, dy=1   = Southeast
 *   Direction 7: dx=0, dy=1   = South
 */
static const s32 s_move_dx[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
static const s32 s_move_dy[8] = {1, 0, -1, -1, -1, 0, 1, 1};

/* Angle lookup table - DAT_004bc2b0
 * 65 entries of 8-byte doubles for atan2 approximation
 */
static const double s_angle_table[65] = {
    0.000000, 0.024679, 0.049462, 0.074443, 0.099669,
    0.125170, 0.150983, 0.177147, 0.203704, 0.230703,
    0.258197, 0.286240, 0.314889, 0.344205, 0.374251,
    0.405091, 0.436791, 0.469420, 0.503048, 0.537750,
    0.573603, 0.610686, 0.649087, 0.688898, 0.730219,
    0.773156, 0.817822, 0.864336, 0.912825, 0.963425,
    1.01628, 1.07154, 1.12940, 1.19007, 1.25380,
    1.32087, 1.39160, 1.46637, 1.54562, 1.62984,
    1.71962, 1.81563, 1.91863, 2.02948, 2.14924,
    2.27921, 2.42095, 2.57636, 2.74780, 2.93828,
    3.15134, 3.39184, 3.66519, 3.97883, 4.34202,
    4.76790, 5.27392, 5.88593, 6.64014, 7.59575,
    8.82802, 10.5043, 12.9545, 16.9354, 25.0169
};

/* Floating point constants from DAT_0049c3xx region */
static const float s_pi = 3.14159265358979323846f;
static const float s_half_pi = 1.57079632679489661923f;
static const float s_two_pi = 6.28318530717958647693f;
static const float s_angle_offset = 0.39269908169872415481f;  /* pi/8 */
static const float s_epsilon = 0.0000001f;

/*
 * Direction to delta conversion - FUN_00443e80
 * Converts direction (0-7) to x/y deltas
 */
void direction_get_delta(u32 direction, s32* dx, s32* dy) {
    if (direction > 7) {
        *dx = 0;
        *dy = 0;
        return;
    }

    *dx = s_move_dx[direction];
    *dy = s_move_dy[direction];
}

/*
 * Angle normalization - FUN_004470d0
 * Normalizes angle to [0, 2*PI) range
 */
void normalize_angle(float* angle) {
    while (*angle >= s_two_pi) {
        *angle -= s_two_pi;
    }
    while (*angle < 0.0f) {
        *angle += s_two_pi;
    }
}

/*
 * Angle calculation - FUN_00447150
 * Calculates angle from dx/dy using atan2 approximation
 *
 * TODO: Verify quadrant handling matches original binary exactly.
 * TODO: Extract angle table directly from DAT_004bc2b0 for accuracy.
 */
float calculate_angle(float dx, float dy) {
    float abs_dx, abs_dy;
    float ratio, angle;
    int index, next_index;
    double t, t1, v1, v2;
    int quadrant = 0;

    if (dx == 0.0f && dy == 0.0f) {
        return 0.0f;
    }

    abs_dx = dx;
    if (dx < 0.0f) {
        quadrant = 1;
        abs_dx = -dx;
    }

    abs_dy = dy;
    if (dy < 0.0f) {
        quadrant += 2;
        abs_dy = -dy;
    }

    if (abs_dx < abs_dy) {
        quadrant += 4;
        float temp = abs_dx;
        abs_dx = abs_dy;
        abs_dy = temp;
    }

    if (abs_dy < s_epsilon) {
        ratio = 0.0f;
    } else {
        ratio = abs_dy / abs_dx;
    }

    index = (int)ratio;
    next_index = index + 1;
    if (next_index > 64) {
        next_index = 64;
    }

    t = ratio - (float)index;
    v1 = s_angle_table[index];
    v2 = s_angle_table[next_index];
    t1 = (1.0 - t) * v1 + t * v2;

    switch (quadrant) {
        case 0:
            angle = (float)t1;
            break;
        case 1:
            angle = s_half_pi - (float)t1;
            break;
        case 2:
            angle = s_half_pi + (float)t1;
            break;
        case 3:
            angle = s_pi - (float)t1;
            break;
        case 4:
            angle = s_half_pi - (float)t1;
            break;
        case 5:
            angle = (float)t1;
            break;
        case 6:
            angle = (float)t1 + s_half_pi;
            break;
        case 7:
            angle = s_half_pi - (float)t1;
            break;
        default:
            angle = (float)t1;
            break;
    }

    angle -= s_pi;
    if (angle < 0.0f) {
        angle += s_two_pi;
    }

    return angle;
}

/*
 * Direction from delta - FUN_00443f80
 */
u32 calculate_direction_from_delta(s32 from_x, s32 from_y, s32 to_x, s32 to_y) {
    float dx, dy, angle;

    dx = (float)(to_x - from_x);
    dy = (float)(to_y - from_y);

    angle = calculate_angle(dx, dy);
    angle += s_angle_offset;
    normalize_angle(&angle);

    return (u32)(angle / (s_two_pi / 8.0f)) & 7;
}

/*
 * Walkability check - FUN_00441ff0
 */
int is_tile_walkable(s32 x, s32 y) {
    s32 rel_x, rel_y;

    if (g_map.in_battle) {
        return 1;
    }

    rel_x = x - g_map.offset_x;
    rel_y = y - g_map.offset_y;

    if (rel_x < 0 || rel_x >= (s32)g_map.width ||
        rel_y < 0 || rel_y >= (s32)g_map.height) {
        return 1;
    }

    if (g_map.collision_map) {
        return g_map.collision_map[rel_y * g_map.width + rel_x] == 1;
    }

    return map_is_walkable((u16)x, (u16)y);
}

/*
 * Main path calculation - FUN_00443fe0
 *
 * TODO: Exact control flow may differ from original.
 * FIX: May not handle all cases where both alternative directions are walkable.
 */
int calculate_movement_path(s32 start_x, s32 start_y, s32 dest_x, s32 dest_y) {
    s32 cur_x, cur_y;
    s32 target_x, target_y;
    u32 primary_dir, current_dir;
    s32 dx, dy;
    u32 step_count;
    s32 next_x, next_y;
    s32 alt_dx1, alt_dy1, alt_dx2, alt_dy2;
    int i;
    float angle;

    g_movement_path.step_count = 0;
    g_movement_path.current_step = 0;
    g_movement_path.last_direction = -1;

    if (start_x == dest_x && start_y == dest_y) {
        return 0;
    }

    dx = dest_x - start_x;
    dy = dest_y - start_y;
    angle = calculate_angle((float)dx, (float)dy);
    angle += s_angle_offset;
    normalize_angle(&angle);
    primary_dir = (u32)(angle / (s_two_pi / 8.0f)) & 7;

    cur_x = start_x;
    cur_y = start_y;
    target_x = dest_x;
    target_y = dest_y;
    step_count = 0;

    while ((cur_x != target_x || cur_y != target_y) && step_count < MAX_PATH_STEPS) {
        dx = target_x - cur_x;
        dy = target_y - cur_y;

        angle = calculate_angle((float)dx, (float)dy);
        angle += s_angle_offset;
        normalize_angle(&angle);
        current_dir = (u32)(angle / (s_two_pi / 8.0f)) & 7;

        for (i = -1; i <= 1; i++) {
            u32 check_dir = (current_dir + i) & 7;

            direction_get_delta(check_dir, &dx, &dy);
            next_x = cur_x + dx;
            next_y = cur_y + dy;

            if (is_tile_walkable(next_x, next_y)) {
                if ((check_dir & 1) == 0) {
                    if (next_x == target_x && next_y == target_y) {
                        g_movement_path.steps[step_count].direction = (s16)check_dir;
                        g_movement_path.steps[step_count].x = (s16)next_x;
                        g_movement_path.steps[step_count].y = (s16)next_y;
                        step_count++;
                        g_movement_path.step_count = step_count;
                        g_movement_path.last_direction = (s16)check_dir;
                        return 1;
                    }
                }

                g_movement_path.steps[step_count].direction = (s16)check_dir;
                g_movement_path.steps[step_count].x = (s16)next_x;
                g_movement_path.steps[step_count].y = (s16)next_y;
                step_count++;

                cur_x = next_x;
                cur_y = next_y;
                break;
            }
        }

        if (i > 1) {
            u32 alt_dir1 = (current_dir + 1) & 7;
            u32 alt_dir2 = (current_dir - 1) & 7;

            direction_get_delta(alt_dir1, &alt_dx1, &alt_dy1);
            direction_get_delta(alt_dir2, &alt_dx2, &alt_dy2);

            s32 walkable1 = is_tile_walkable(cur_x + alt_dx1, cur_y + alt_dy1);
            s32 walkable2 = is_tile_walkable(cur_x + alt_dx2, cur_y + alt_dy2);

            if (walkable1 && !walkable2) {
                g_movement_path.steps[step_count].direction = (s16)alt_dir1;
                g_movement_path.steps[step_count].x = (s16)(cur_x + alt_dx1);
                g_movement_path.steps[step_count].y = (s16)(cur_y + alt_dy1);
                step_count++;
                cur_x += alt_dx1;
                cur_y += alt_dy1;
            } else if (walkable2 && !walkable1) {
                g_movement_path.steps[step_count].direction = (s16)alt_dir2;
                g_movement_path.steps[step_count].x = (s16)(cur_x + alt_dx2);
                g_movement_path.steps[step_count].y = (s16)(cur_y + alt_dy2);
                step_count++;
                cur_x += alt_dx2;
                cur_y += alt_dy2;
            } else if (walkable1 && walkable2) {
                s32 dist1 = abs(target_x - (cur_x + alt_dx1)) +
                           abs(target_y - (cur_y + alt_dy1));
                s32 dist2 = abs(target_x - (cur_x + alt_dx2)) +
                           abs(target_y - (cur_y + alt_dy2));

                if (dist1 <= dist2) {
                    g_movement_path.steps[step_count].direction = (s16)alt_dir1;
                    g_movement_path.steps[step_count].x = (s16)(cur_x + alt_dx1);
                    g_movement_path.steps[step_count].y = (s16)(cur_y + alt_dy1);
                    step_count++;
                    cur_x += alt_dx1;
                    cur_y += alt_dy1;
                } else {
                    g_movement_path.steps[step_count].direction = (s16)alt_dir2;
                    g_movement_path.steps[step_count].x = (s16)(cur_x + alt_dx2);
                    g_movement_path.steps[step_count].y = (s16)(cur_y + alt_dy2);
                    step_count++;
                    cur_x += alt_dx2;
                    cur_y += alt_dy2;
                }
            } else {
                for (i = 0; i < 8; i++) {
                    direction_get_delta(i, &dx, &dy);
                    if (is_tile_walkable(cur_x + dx, cur_y + dy)) {
                        g_movement_path.steps[step_count].direction = (s16)i;
                        g_movement_path.steps[step_count].x = (s16)(cur_x + dx);
                        g_movement_path.steps[step_count].y = (s16)(cur_y + dy);
                        step_count++;
                        cur_x += dx;
                        cur_y += dy;
                        break;
                    }
                }

                if (i == 8) {
                    g_movement_path.step_count = step_count;
                    g_movement_path.last_direction = -1;
                    return 0;
                }
            }
        }

        if (step_count >= MAX_PATH_STEPS) {
            break;
        }
    }

    g_movement_path.step_count = step_count;

    if (step_count > 0 && (cur_x != target_x || cur_y != target_y)) {
        dx = target_x - cur_x;
        dy = target_y - cur_y;
        angle = calculate_angle((float)dx, (float)dy);
        angle += s_angle_offset;
        normalize_angle(&angle);
        g_movement_path.last_direction = (s16)((u32)(angle / (s_two_pi / 8.0f)) & 7);
    } else if (step_count > 0) {
        g_movement_path.last_direction =
            g_movement_path.steps[step_count - 1].direction;
    }

    return step_count > 0 ? 1 : 0;
}

/*
 * Get next step from movement path
 */
int movement_path_get_next(s16* direction, s16* x, s16* y) {
    if (g_movement_path.current_step >= g_movement_path.step_count) {
        return 0;
    }

    *direction = g_movement_path.steps[g_movement_path.current_step].direction;
    *x = g_movement_path.steps[g_movement_path.current_step].x;
    *y = g_movement_path.steps[g_movement_path.current_step].y;

    g_movement_path.current_step++;
    return 1;
}

/*
 * Clear movement path
 */
void movement_path_clear(void) {
    memset(&g_movement_path, 0, sizeof(MovementPath));
    g_movement_path.last_direction = -1;
}

/*
 * Check if movement path is active
 */
int movement_path_is_active(void) {
    return g_movement_path.step_count > 0 &&
           g_movement_path.current_step < g_movement_path.step_count;
}

/*
 * Initialize movement path system
 */
int movement_path_init(void) {
    movement_path_clear();
    LOG_INFO("Movement path system initialized");
    return 1;
}
