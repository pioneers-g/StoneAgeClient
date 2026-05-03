/*
 * Stone Age Client - Fade Effect Special Effects
 * Split from fade.c for code organization
 *
 * FUN_0047aea0 - fade_box() - Box expand/contract effects
 * FUN_0047b180 - fade_block_dissolve() - 8x8 block dissolve
 * FUN_0047b7e0 - fade_pixelate_wipe() - Pixelate block wipe
 * FUN_0047b9f0 - fade_blind_h_draw() - Horizontal blind draw
 * FUN_0047bb30 - fade_blind_h_erase() - Horizontal blind erase
 * FUN_0047bc80 - fade_blind_vertical() - Vertical blind effect
 */

#include <windows.h>
#include "types.h"
#include "fade.h"
#include "render.h"

/* External functions from fade_core.c */
extern int fade_rand(void);
extern int fade_check_complete(void);

/* External global */
extern FadeContext g_fade;

/*
 * Box expand/contract effect - FUN_0047aea0
 * Four rectangles expand from or contract to center
 */
int fade_box(int variant) {
    int complete = 0;
    int i;

    /* Initialize on first frame - DAT_004cf838 check */
    if (g_fade.box_init_flag == -1) {
        g_fade.box_init_flag = 0;

        if (variant == 0) {
            /* Expand: start at center */
            g_fade.box_x1 = 0;
            g_fade.box_x2 = FADE_SCREEN_CENTER_X;
            g_fade.box_x3 = 0;
            g_fade.box_x4 = FADE_SCREEN_CENTER_X;
            g_fade.box_y1 = 0;
            g_fade.box_y2 = 0;
            g_fade.box_y3 = FADE_SCREEN_CENTER_Y;
            g_fade.box_y4 = FADE_SCREEN_CENTER_Y;
        } else {
            /* Contract: start at edges */
            g_fade.box_x1 = -280;
            g_fade.box_x2 = 600;
            g_fade.box_x3 = -280;
            g_fade.box_x4 = 600;
            g_fade.box_y1 = -280;
            g_fade.box_y2 = 520;
            g_fade.box_y3 = -280;
            g_fade.box_y4 = 520;
        }
    }

    if (variant == 0) {
        /* Expand - rectangles grow outward */
        g_fade.box_y1 -= 4;
        g_fade.box_x2 += 4;
        g_fade.box_y2 -= 4;
        g_fade.box_x3 -= 4;
        g_fade.box_x1 -= 4;
        g_fade.box_y3 += 4;
        g_fade.box_x4 += 4;
        g_fade.box_y4 += 4;

        if (g_fade.box_x1 < -FADE_SCREEN_CENTER_X) {
            complete = 1;
        }
    } else {
        /* Contract - rectangles shrink inward */
        g_fade.box_y1 += 4;
        g_fade.box_x2 -= 4;
        g_fade.box_y2 += 4;
        g_fade.box_x3 += 4;
        g_fade.box_x1 += 4;
        g_fade.box_y3 -= 4;
        g_fade.box_x4 -= 4;
        g_fade.box_y4 -= 4;

        if (g_fade.box_x1 >= 0) {
            complete = 1;
        }
    }

    if (complete) {
        g_fade.box_init_flag = 1;
    }

    /* Render 4 rectangles - FUN_00414420 calls */
    for (i = 0; i < 4; i++) {
        int x = *(&g_fade.box_x1 + i);
        int y = *(&g_fade.box_y1 + i);
        /* Render rectangle at (x, y) */
        (void)x; (void)y;
    }

    if (g_fade.box_init_flag == 1) {
        fade_check_complete();
        g_fade.box_init_flag = -1;
        return 1;
    }

    return 0;
}

/*
 * Block dissolve effect - FUN_0047b180
 * 8x8 grid of blocks that appear/disappear randomly or sequentially
 */
int fade_block_dissolve(int variant) {
    int i, idx;
    int complete = 0;

    /* Initialize on first frame - DAT_004cf83c check */
    if (g_fade.block_init_flag == -1) {
        g_fade.block_init_flag = 0;
        g_fade.block_complete_count = 0;
        g_fade.block_active_count = 0;

        /* Initialize all 64 blocks */
        for (i = 0; i < FADE_BLOCK_COUNT; i++) {
            int col = i % 8;
            int row = i / 8;

            switch (variant) {
                case 0:
                    g_fade.block_state[i] = FADE_BLOCK_VISIBLE;
                    break;
                case 1:
                    g_fade.block_state[i] = FADE_BLOCK_HIDDEN;
                    break;
                case 2:
                    g_fade.block_state[i] = FADE_BLOCK_VISIBLE;
                    break;
                case 3:
                    g_fade.block_state[i] = FADE_BLOCK_HIDDEN;
                    break;
            }

            g_fade.block_progress[i] = 0;
            g_fade.block_offset[i] = (variant == 3) ? -60 : 0;
            g_fade.block_x[i] = col * FADE_BLOCK_WIDTH;
            g_fade.block_y[i] = row * FADE_BLOCK_HEIGHT;
        }
    }

    /* Pick random block to transition */
    idx = fade_rand() % FADE_BLOCK_COUNT;

    switch (variant) {
        case 0:
            while (g_fade.block_state[idx] != FADE_BLOCK_VISIBLE) {
                idx = fade_rand() % FADE_BLOCK_COUNT;
            }
            g_fade.block_state[idx] = FADE_BLOCK_APPEARING;
            break;

        case 1:
            while (g_fade.block_state[idx] != FADE_BLOCK_HIDDEN) {
                idx = fade_rand() % FADE_BLOCK_COUNT;
            }
            g_fade.block_state[idx] = FADE_BLOCK_APPEARING;
            break;

        case 2:
            if (g_fade.block_active_count < FADE_BLOCK_COUNT) {
                while (g_fade.block_state[idx] < FADE_BLOCK_APPEARING) {
                    idx = fade_rand() % FADE_BLOCK_COUNT;
                }
                g_fade.block_state[idx] = FADE_BLOCK_APPEARING;
                g_fade.block_active_count++;
            }
            for (i = 0; i < FADE_BLOCK_COUNT; i++) {
                if (g_fade.block_state[i] == FADE_BLOCK_APPEARING) {
                    g_fade.block_progress[i]++;
                    g_fade.block_offset[i] += g_fade.block_progress[i];
                    if (g_fade.block_offset[i] > 499) {
                        g_fade.block_state[i] = FADE_BLOCK_HIDDEN;
                        g_fade.block_complete_count++;
                    }
                }
            }
            break;

        case 3:
            if (g_fade.block_active_count < FADE_BLOCK_COUNT) {
                while (g_fade.block_state[idx] > FADE_BLOCK_HIDDEN) {
                    idx = fade_rand() % FADE_BLOCK_COUNT;
                }
                g_fade.block_state[idx] = FADE_BLOCK_APPEARING;
                g_fade.block_active_count++;
            }
            for (i = 0; i < FADE_BLOCK_COUNT; i++) {
                if (g_fade.block_state[i] == FADE_BLOCK_APPEARING) {
                    g_fade.block_progress[i]++;
                    int offset = g_fade.block_offset[i] + g_fade.block_progress[i];
                    g_fade.block_offset[i] = offset;
                    if (offset >= g_fade.block_y[i]) {
                        g_fade.block_offset[i] = g_fade.block_y[i];
                        g_fade.block_state[i] = FADE_BLOCK_VISIBLE;
                        g_fade.block_complete_count++;
                    }
                }
            }
            break;
    }

    if (variant < 2) {
        g_fade.block_complete_count++;
        if (g_fade.block_complete_count > FADE_BLOCK_COUNT - 1) {
            g_fade.block_init_flag = 1;
        }
    } else if (g_fade.block_complete_count >= FADE_BLOCK_COUNT) {
        g_fade.block_init_flag = 1;
    }

    /* Render all blocks */
    for (i = 0; i < FADE_BLOCK_COUNT; i++) {
        if (g_fade.block_state[i] == FADE_BLOCK_APPEARING ||
            g_fade.block_state[i] == FADE_BLOCK_VISIBLE) {
            int x = g_fade.block_x[i];
            int y = g_fade.block_offset[i];
            (void)x; (void)y;
        }
    }

    if (g_fade.block_init_flag == 1) {
        fade_check_complete();
        g_fade.block_init_flag = -1;
        return 1;
    }

    return 0;
}

/*
 * Pixelate wipe effect - FUN_0047b7e0
 */
int fade_pixelate_wipe(int direction) {
    int x, y;
    int complete = 0;

    if (g_fade.init_flag) {
        g_fade.init_flag = 0;
        g_fade.wipe_active = 1;
        g_fade.wipe_progress = (direction < 0) ? 64 : 0;
    } else if (g_fade.wipe_active) {
    } else {
        g_fade.wipe_active = 1;
        g_fade.wipe_progress = (direction < 0) ? 64 : 0;
    }

    for (y = 0; y < FADE_WIPE_ROWS; y++) {
        for (x = 0; x < FADE_WIPE_COLS; x++) {
            int cell_x = x * 15 - 310;
            int cell_y = y * 16 - 232;
            int transformed_x = (cell_x * g_fade.wipe_progress) / 64 + 310;
            int transformed_y = (cell_y * g_fade.wipe_progress) / 64 + 232;
            (void)transformed_x;
            (void)transformed_y;
        }
    }

    if (direction < 0) {
        g_fade.wipe_progress--;
        if (g_fade.wipe_progress < 0) {
            complete = 1;
        }
    } else {
        g_fade.wipe_progress++;
        if (g_fade.wipe_progress > 64) {
            complete = 1;
        }
    }

    if (complete) {
        g_fade.wipe_active = 0;
        fade_check_complete();
    }

    return complete;
}

/*
 * Horizontal blind draw - FUN_0047b9f0
 */
int fade_blind_h_draw(int direction) {
    int i;
    int complete = 0;
    int progress = g_fade.wipe_progress;

    if (g_fade.init_flag) {
        g_fade.init_flag = 0;
        g_fade.wipe_active = 1;
    }

    if (!g_fade.wipe_active || g_fade.wipe_progress == 0) {
        if (direction >= 0) {
            progress = 0;
        } else {
            progress = (int)FADE_SCREEN_HEIGHT;
        }
    }

    if (progress > 0) {
        int step = (int)FADE_SCREEN_HEIGHT;
        for (i = 0; i < progress; i++) {
            int blind_start = step / progress;
            int blind_end = blind_start + 1;
            (void)blind_start; (void)blind_end;
        }
    }

    if (direction < 0) {
        progress -= 8;
        if (progress < 0) {
            complete = 1;
        }
    } else {
        progress += 8;
        if (progress > (int)FADE_SCREEN_HEIGHT) {
            complete = 1;
        }
    }

    g_fade.wipe_progress = progress;

    if (complete) {
        g_fade.wipe_active = 0;
        fade_check_complete();
        return 1;
    }

    return 0;
}

/*
 * Horizontal blind erase - FUN_0047bb30
 */
int fade_blind_h_erase(int direction) {
    int i;
    int complete = 0;
    int progress = g_fade.wipe_progress;

    if (g_fade.init_flag) {
        g_fade.init_flag = 0;
        g_fade.wipe_active = 1;
    }

    if (!g_fade.wipe_active || g_fade.wipe_progress == 0) {
        if (direction >= 0) {
            progress = 0;
        } else {
            progress = (int)FADE_SCREEN_HEIGHT;
        }
    }

    if (progress > 0) {
        int step = (int)FADE_SCREEN_HEIGHT;
        for (i = 0; i < progress; i++) {
            int blind_start = (int)FADE_SCREEN_HEIGHT - (step / progress);
            int blind_end = blind_start + 1;
            (void)blind_start; (void)blind_end;
        }
    }

    if (direction < 0) {
        progress -= 8;
        if (progress < 0) {
            complete = 1;
        }
    } else {
        progress += 8;
        if (progress > (int)FADE_SCREEN_HEIGHT) {
            complete = 1;
        }
    }

    g_fade.wipe_progress = progress;

    if (complete) {
        g_fade.wipe_active = 0;
        fade_check_complete();
        return 1;
    }

    return 0;
}

/*
 * Vertical blind effect - FUN_0047bc80
 */
int fade_blind_vertical(int direction) {
    int i;
    int complete = 0;
    int progress = g_fade.wipe_progress;
    int step_size = 8;
    int center_y = (int)FADE_SCREEN_CENTER_Y;

    if (g_fade.init_flag) {
        g_fade.init_flag = 0;
        if (direction == 0) {
            progress = center_y;
        } else {
            progress = 0;
        }
    }

    if (progress > 0) {
        for (i = 0; i < progress; i++) {
            int blind_y = center_y - (i * center_y / progress);
            int blind_h = blind_y + 1;
            (void)blind_y; (void)blind_h;
        }
    }

    if (direction == 0) {
        progress -= step_size;
        if (progress < 0) {
            return 1;
        }
    } else {
        progress += step_size;
        if (progress > center_y) {
            return 1;
        }
    }

    g_fade.wipe_progress = progress;

    return 0;
}
