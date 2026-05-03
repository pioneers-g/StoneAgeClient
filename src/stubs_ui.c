/*
 * Stone Age Client - UI Stub Functions
 * Split from stubs.c to reduce file size
 */

#include <windows.h>
#include "types.h"

/* UI Functions */
void FUN_00411990(void* param_1) { (void)param_1; }
void FUN_0041adf0(void) {}
void FUN_0041a8d0(void) {}
void FUN_0041bba0(void) {}
void FUN_0044ac00(void) {}

void* ui_window_create(int x, int y, int w, int h, const char* t) {
    (void)x; (void)y; (void)w; (void)h; (void)t;
    return NULL;
}

/* Game state UI stubs */
void FUN_00410850(void) {}
void FUN_004117e0(void) {}

/* Shop/UI Functions */
void FUN_00416be0(void) {}
/* FUN_00462f60 is in stubs.c */
void FUN_00465d20(void) {}

/* Menu Functions */
void FUN_00418330(void) {}
void FUN_00418370(void) {}

/* Input Functions */
int FUN_004808e0(int param_1) {
    (void)param_1;
    return 1;
}

/*
 * FUN_00480bd0 - Main Input State Reader
 *
 * Binary analysis:
 * - Reads all input device states (keyboard, mouse, joystick)
 * - Stores results in DAT_04ebe488 (key states) and DAT_04ebe47c (mouse/joystick)
 * - Calculates delta states (changed from last frame) in DAT_04ebe490/494
 * - Bit flags for keys:
 *   - 0x0001-0x0020: F1-F10 keys
 *   - 0x0100: Left mouse button
 *   - 0x0400: Right mouse button
 *   - 0x1000: Up arrow
 *   - 0x2000: Down arrow
 *   - 0x4000: Left arrow
 *   - 0x8000: Right arrow
 *   - Higher bits: Various game-specific inputs
 * - Also handles gamepad/joystick input via DirectInput
 */
void FUN_00480bd0(void) {
    /* Input state processing */
    /* TODO: Full implementation with DirectInput */
}

/*
 * FUN_004809e0 - Input Key Repeat Handler
 *
 * Binary analysis:
 * - Handles key repeat detection for held keys
 * - param_1: player index (0 or 1 for multiplayer)
 * - Returns bitmask of keys that triggered repeat
 * - Uses counter at DAT_04ebe420 for repeat timing
 * - Repeat threshold: 0x1e (30 frames) initial, 0x17 (23 frames) for subsequent
 * - Key repeat masks: 0x1000, 0x2000, 0x4000, 0x8000 (arrows), 0x100, 0x400 (mouse)
 */
unsigned int FUN_004809e0(unsigned int player_index) {
    (void)player_index;
    return 0;
}

/* Additional UI stubs */
void FUN_00414820(int param_1) { (void)param_1; }
void FUN_0047d5b0(void) {}
void FUN_00445050(void) {}
void FUN_004449e0(int param_1, int param_2) { (void)param_1; (void)param_2; }
void FUN_0047d150(void) {}
void FUN_00444980(void) {}
void FUN_00488680(int param_1) { (void)param_1; }
void FUN_004888f0(int param_1, int param_2, int param_3) { (void)param_1; (void)param_2; (void)param_3; }
void FUN_004445e0(int param_1, int param_2, int param_3, int param_4, int param_5, float param_6, float param_7, int param_8) {
    (void)param_1; (void)param_2; (void)param_3; (void)param_4; (void)param_5;
    (void)param_6; (void)param_7; (void)param_8;
}
int FUN_0041f980(int param_1, short* param_2, short* param_3) {
    (void)param_1; (void)param_2; (void)param_3;
    return 1;
}

/*
 * FUN_00448270 - UI Window Render Callback
 *
 * Binary analysis:
 * - Render callback for window widgets
 * - param_1: window entity pointer (500-byte structure)
 * - Switch on animation state at offset 0xa8:
 *   - Case 0: Initial animation (fade/slide in)
 *   - Case 1: Complete (set flag at offset 0x78)
 *   - Case 2: Render 9-sprite grid for window border
 *   - Case 3: Button rendering with click detection
 * - Uses FUN_0047e210 to queue sprites for rendering
 * - 9-sprite grid layout: corners (0,2,6,8), edges (1,3,5,7), center (4)
 */
void FUN_00448270(void* window_entity) {
    (void)window_entity;
    /* TODO: Full window rendering implementation */
}

/*
 * FUN_00448610 - UI Window Creation (9-Sprite Grid)
 *
 * Binary analysis:
 * - Creates a window widget with 9-sprite grid layout
 * - param_1: x position
 * - param_2: y position
 * - param_3: width (in tiles)
 * - param_4: height (in tiles)
 * - param_5: center sprite ID
 * - param_6: style/type (0-5, -1)
 * - Returns entity pointer or 0 on failure
 * - Allocates 500-byte entity structure via FUN_004010a0
 * - Sets render callback to FUN_00448270
 * - Sprite sets from DAT_0054b194 (normal) or DAT_0054c208 (pressed)
 *
 * 9-sprite indices:
 *   0 = top-left corner, 1 = top edge, 2 = top-right corner
 *   3 = left edge, 4 = center, 5 = right edge
 *   6 = bottom-left corner, 7 = bottom edge, 8 = bottom-right corner
 */
int FUN_00448610(int x, int y, int width, int height, int center_sprite, int style) {
    (void)x; (void)y; (void)width; (void)height; (void)center_sprite; (void)style;
    /* TODO: Full window creation implementation */
    return 0;
}

/*
 * FUN_0047bde0 - Fade Effect Dispatcher
 *
 * Binary analysis:
 * - Dispatches fade/transition effects based on mode
 * - param_1: fade mode (0-29)
 * - Returns 1 when fade is complete, 0 otherwise
 * - Modes:
 *   - 0-11: Fade in/out for different layers (0,1 = layer 0, 2,3 = layer 1, etc.)
 *   - 12-15: Special fade effects (FUN_0047aea0, FUN_0047b180)
 *   - 16-19: Pattern fades (FUN_0047b7e0, FUN_0047b9f0, FUN_0047bb30)
 *   - 20-29: Various transition effects
 * - Calls FUN_00412a40 to prepare surfaces
 * - Calls FUN_00412ab0 to flip surfaces when complete
 */
int FUN_0047bde0(int fade_mode) {
    (void)fade_mode;
    /* TODO: Full fade effect implementation */
    return 1;
}

/*
 * FUN_0047a730 - Scroll/Slide Transition
 *
 * Binary analysis:
 * - Implements scroll/slide screen transition with 6 directional modes
 * - param_1: scroll direction (0-5)
 *   - 0: Scroll up (Y decreases, velocity increases)
 *   - 1: Scroll down (Y increases)
 *   - 2: Scroll left (X decreases)
 *   - 3: Scroll right (X increases)
 *   - 4: Diagonal right-up (X increases, Y decreases)
 *   - 5: Diagonal right-down (Y increases, another Y decreases)
 * - Uses acceleration for smooth deceleration
 * - Position tracked in _DAT_046332dc/e0/e4/e8
 * - Velocity in _DAT_046332ec, incremented by _DAT_0049c454/448
 * - Returns 1 when transition reaches boundary, 0 otherwise
 * - Initializes when DAT_004cf830 == 1
 * - Updates DAT_005ab710 state machine (2->3->4)
 */
int FUN_0047a730(int param_1) {
    extern float _DAT_046332dc, _DAT_046332e0, _DAT_046332e4;
    extern float _DAT_046332e8, _DAT_046332ec;
    extern u32 DAT_004cf830;
    extern u32 DAT_005ab710;

    int complete = 0;

    if (DAT_004cf830 == 1) {
        DAT_004cf830 = 0;
        _DAT_046332dc = 0.0f;
        _DAT_046332e0 = 0.0f;
        _DAT_046332e4 = 0.0f;
        _DAT_046332ec = 0.0f;
    }

    switch (param_1) {
        case 0: /* Scroll up */
            _DAT_046332e4 -= _DAT_046332ec;
            _DAT_046332ec += 1.0f;  /* Acceleration */
            if (_DAT_046332e4 <= 0.0f) complete = 1;
            break;
        case 1: /* Scroll down */
            _DAT_046332e4 += _DAT_046332ec;
            _DAT_046332ec += 1.0f;
            if (_DAT_046332e4 >= 480.0f) complete = 1;
            break;
        case 2: /* Scroll left */
            _DAT_046332dc -= _DAT_046332ec;
            _DAT_046332ec += 1.0f;
            if (_DAT_046332dc <= 0.0f) complete = 1;
            break;
        case 3: /* Scroll right */
            _DAT_046332dc += _DAT_046332ec;
            _DAT_046332ec += 1.0f;
            if (_DAT_046332dc >= 640.0f) complete = 1;
            break;
        case 4: /* Diagonal right-up */
            _DAT_046332dc += _DAT_046332ec;
            _DAT_046332e0 -= _DAT_046332ec;
            _DAT_046332ec += 1.0f;
            if (_DAT_046332dc >= 640.0f) complete = 1;
            break;
        case 5: /* Diagonal right-down */
            _DAT_046332e4 += _DAT_046332ec;
            _DAT_046332e8 -= _DAT_046332ec;
            _DAT_046332ec += 1.0f;
            if (_DAT_046332e4 >= 480.0f) complete = 1;
            break;
    }

    if (complete) {
        _DAT_046332dc = 0.0f;
        _DAT_046332e0 = 0.0f;
        _DAT_046332e4 = 0.0f;
        _DAT_046332e8 = 0.0f;
        _DAT_046332ec = 0.0f;
        if (DAT_005ab710 == 2) DAT_005ab710 = 3;
        else if (DAT_005ab710 == 3) DAT_005ab710 = 4;
        return 1;
    }
    return 0;
}

/*
 * FUN_0047aac0 - Accelerated Scroll Transition
 *
 * Binary analysis:
 * - Scroll transition with deceleration (starts fast, slows down)
 * - param_1: scroll direction (0-5), same as FUN_0047a730
 * - Uses separate position (_DAT_046332f0/f4/f8/fc) and velocity (_DAT_04630e04)
 * - Initial velocity: 17.4 or 22.8 depending on direction
 * - Decelerates each frame by _DAT_0049c454/448
 * - Completes when velocity drops below _DAT_0049c318 threshold
 * - State tracked in DAT_004cf834 (-1 = needs init, 0 = running, 1 = complete)
 */
int FUN_0047aac0(int param_1) {
    extern float _DAT_046332f0, _DAT_046332f4, _DAT_046332f8, _DAT_046332fc;
    extern float _DAT_04630e04;
    extern s32 DAT_004cf834;
    extern u32 DAT_005ab710;

    if (DAT_004cf834 == -1) {
        /* Initialize based on direction */
        switch (param_1) {
            case 0:
                _DAT_046332f0 = 0.0f;
                _DAT_04630e04 = 17.4f;
                _DAT_046332f8 = 0.0f;
                break;
            case 1:
                _DAT_046332f0 = 0.0f;
                _DAT_04630e04 = 17.4f;
                _DAT_046332f8 = 480.0f;
                break;
            case 2:
                _DAT_04630e04 = 22.8f;
                _DAT_046332f8 = 0.0f;
                _DAT_046332f0 = 480.0f - 22.8f;
                break;
            case 3:
                _DAT_046332f8 = 0.0f;
                _DAT_04630e04 = 22.8f;
                _DAT_046332f0 = 0.0f;
                break;
            case 4:
                _DAT_046332f4 = 661.0f;
                _DAT_046332f8 = 0.0f;
                _DAT_04630e04 = 22.8f;
                _DAT_046332f0 = 0.0f;
                break;
            case 5:
                _DAT_046332f0 = 0.0f;
                _DAT_046332fc = 513.2f;
                _DAT_04630e04 = 17.4f;
                _DAT_046332f8 = 480.0f;
                break;
        }
        DAT_004cf834 = 0;
    }

    /* Update position and decelerate */
    switch (param_1) {
        case 0:
            _DAT_046332f8 -= _DAT_04630e04;
            _DAT_04630e04 -= 1.0f;
            break;
        case 1:
            _DAT_046332f8 += _DAT_04630e04;
            _DAT_04630e04 -= 1.0f;
            break;
        case 2:
            _DAT_046332f0 -= _DAT_04630e04;
            _DAT_04630e04 -= 1.0f;
            break;
        case 3:
            _DAT_046332f0 += _DAT_04630e04;
            _DAT_04630e04 -= 1.0f;
            break;
        case 4:
            _DAT_046332f0 += _DAT_04630e04;
            _DAT_046332f4 -= _DAT_04630e04;
            _DAT_04630e04 -= 1.0f;
            break;
        case 5:
            _DAT_046332f8 += _DAT_04630e04;
            _DAT_046332fc -= _DAT_04630e04;
            _DAT_04630e04 -= 1.0f;
            break;
    }

    /* Check if velocity below threshold */
    if (_DAT_04630e04 <= 0.0f) {
        DAT_004cf834 = 1;
    }

    if (DAT_004cf834 == 1) {
        if (DAT_005ab710 == 2) DAT_005ab710 = 3;
        else if (DAT_005ab710 == 3) DAT_005ab710 = 4;
        DAT_004cf834 = -1;
        return 1;
    }
    return 0;
}

/*
 * FUN_0047aea0 - Box Wipe Transition
 *
 * Binary analysis:
 * - Expanding or contracting box wipe effect
 * - param_1: wipe mode (0 or 1)
 *   - 0: Box expands outward from center (open)
 *   - 1: Box contracts toward center (close)
 * - Box defined by 4 rectangles with corners at:
 *   - Left box: (0, 0) to (320, 240) and (0, 240) to (320, 480)
 *   - Right box: (320, 0) to (640, 240) and (320, 240) to (640, 480)
 * - Each frame moves edges by 4 pixels
 * - For mode 0 (expand): edges move toward screen edges
 * - For mode 1 (contract): edges move toward center
 * - State tracked in DAT_004cf838 (-1 = needs init, 0 = running, 1 = complete)
 * - Uses DAT_04560214 to check for half-resolution mode
 */
int FUN_0047aea0(int param_1) {
    extern s32 DAT_046311b0, DAT_046311b4, DAT_046311b8, DAT_046311bc;
    extern u16 DAT_046332cc, DAT_046332d0, DAT_046332d4, DAT_046332d8;
    extern s32 DAT_004cf838;
    extern u32 DAT_005ab710;
    extern u32 DAT_04560214;

    if (DAT_004cf838 == -1) {
        if (param_1 == 0) {
            /* Expanding box - start at center */
            DAT_046311b0 = 0;
            DAT_046332cc = 0;
            DAT_046311b4 = 320;
            DAT_046332d0 = 0;
            DAT_046311b8 = 0;
            DAT_046332d4 = 240;
            DAT_046311bc = 320;
            DAT_046332d8 = 240;
        } else {
            /* Contracting box - start at edges */
            DAT_046311b0 = -280;
            DAT_046332cc = -280;
            DAT_046311b4 = 600;
            DAT_046332d0 = -280;
            DAT_046311b8 = -280;
            DAT_046332d4 = 520;
            DAT_046311bc = 600;
            DAT_046332d8 = 520;
        }
        DAT_004cf838 = 0;
    }

    if (param_1 == 0) {
        /* Expand: move edges outward */
        DAT_046332cc -= 4;
        DAT_046311b4 += 4;
        DAT_046332d0 -= 4;
        DAT_046311b8 -= 4;
        DAT_046311b0 -= 4;
        DAT_046332d4 += 4;
        DAT_046311bc += 4;
        DAT_046332d8 += 4;
        if (DAT_046311b0 <= -320) {
            DAT_004cf838 = 1;
        }
    } else {
        /* Contract: move edges inward */
        DAT_046332cc += 4;
        DAT_046311b4 -= 4;
        DAT_046332d0 += 4;
        DAT_046311b8 += 4;
        DAT_046311b0 += 4;
        DAT_046332d4 -= 4;
        DAT_046311bc -= 4;
        DAT_046332d8 -= 4;
        if (DAT_046311b0 >= 0) {
            DAT_004cf838 = 1;
        }
    }

    if (DAT_004cf838 == 1) {
        if (DAT_005ab710 == 2) DAT_005ab710 = 3;
        else if (DAT_005ab710 == 3) DAT_005ab710 = 4;
        DAT_004cf838 = -1;
        return 1;
    }
    return 0;
}

/*
 * FUN_0047b180 - Block/Tile Dissolve Transition
 *
 * Binary analysis:
 * - 8x8 grid of tiles (64 total) that dissolve randomly
 * - param_1: dissolve mode (0-3)
 *   - 0: Random dissolve in (tiles appear randomly)
 *   - 1: Random dissolve out (tiles disappear randomly)
 *   - 2: Progressive dissolve in (tiles fill progressively)
 *   - 3: Progressive dissolve out (tiles clear progressively)
 * - Each tile is 80x60 pixels (0x50 x 0x3c)
 * - Uses DAT_04630e0c array (64 entries) for tile states:
 *   - 0: Hidden/inactive
 *   - 1: Visible/active
 *   - 2: Transitioning (fading in/out)
 * - DAT_046331cc: Progress counter per tile
 * - DAT_046330c8: Current offset for tile animation
 * - Uses FUN_004472e0 for random tile selection
 * - State in DAT_004cf83c (-1 = needs init, 0 = running, 1 = complete)
 * - Counter DAT_0463100c tracks completed tiles
 */
int FUN_0047b180(int param_1) {
    /* TODO: Full implementation with 8x8 tile grid dissolve */
    (void)param_1;
    return 1;
}

/*
 * FUN_0047b7e0 - Pixel Dissolve Transition
 *
 * Binary analysis:
 * - Creates a pixelated dissolve effect using a grid of dots
 * - param_1: direction/speed (positive = dissolve in, negative = dissolve out)
 * - Uses 0x1d0 (464) Y positions and 0x14 (20) pixel spacing
 * - Grid positions stored in DAT_046311c0 (X) and following
 * - Progress tracked in DAT_04630e00 (0-64 scale)
 * - Each frame draws dots at grid positions with scaling
 * - Half-resolution mode supported via DAT_04560214
 * - State flag: DAT_04633300 (0 = not initialized, 1 = running)
 */
int FUN_0047b7e0(int param_1) {
    /* TODO: Full implementation with pixel dissolve grid */
    (void)param_1;
    return 1;
}

/*
 * FUN_0047b9f0 - Horizontal Blind Transition
 *
 * Binary analysis:
 * - Creates horizontal blind/slit effect (like window blinds)
 * - param_1: direction (positive = close blinds, negative = open blinds)
 * - Progress tracked in DAT_04630e00 (0-480 range)
 * - Each frame adds 8 pixels to blind height
 * - Full screen: 640x480 (0x280 x 0x1e0)
 * - Blind strips drawn from top to bottom progressively
 * - State flag: DAT_04633300 (0 = not initialized, 1 = running)
 */
int FUN_0047b9f0(int param_1) {
    extern u32 DAT_04630e00;
    extern u32 DAT_04633300;
    extern u32 DAT_005ab710;
    extern u32 DAT_004cf830;

    if (DAT_004cf830 == 1) {
        DAT_004cf830 = 0;
    } else if (DAT_04633300 == 0) {
        DAT_04633300 = 1;
        DAT_04630e00 = (param_1 >= 0) ? 0 : 480;
    }

    if (param_1 < 0) {
        /* Open blinds (decrease blind height) */
        if (DAT_04630e00 > 0) {
            DAT_04630e00 -= 8;
            return 0;
        }
        if (DAT_005ab710 == 2) {
            DAT_04633300 = 0;
            DAT_005ab710 = 3;
            return 1;
        }
    } else {
        /* Close blinds (increase blind height) */
        if (DAT_04630e00 < 480) {
            DAT_04630e00 += 8;
            return 0;
        }
        if (DAT_005ab710 == 2) {
            DAT_04633300 = 0;
            DAT_005ab710 = 3;
            return 1;
        }
    }

    if (DAT_005ab710 == 3) {
        DAT_005ab710 = 4;
    }
    DAT_04633300 = 0;
    return 1;
}

/*
 * FUN_0047bb30 - Vertical Blind Transition
 *
 * Binary analysis:
 * - Creates vertical blind/slit effect (like window blinds turned sideways)
 * - param_1: direction (positive = close blinds, negative = open blinds)
 * - Progress tracked in DAT_04630e00 (0-480 range)
 * - Each frame adds 8 pixels to blind width
 * - Full screen: 640x480 (0x280 x 0x1e0)
 * - Blind strips drawn from left to right progressively
 * - State flag: DAT_04633300 (0 = not initialized, 1 = running)
 * - Same pattern as FUN_0047b9f0 but vertical orientation
 */
int FUN_0047bb30(int param_1) {
    extern u32 DAT_04630e00;
    extern u32 DAT_04633300;
    extern u32 DAT_005ab710;
    extern u32 DAT_004cf830;

    if (DAT_004cf830 == 1) {
        DAT_004cf830 = 0;
    } else if (DAT_04633300 == 0) {
        DAT_04633300 = 1;
        DAT_04630e00 = (param_1 >= 0) ? 0 : 480;
    }

    if (param_1 < 0) {
        /* Open blinds (decrease blind width) */
        if (DAT_04630e00 > 0) {
            DAT_04630e00 -= 8;
            return 0;
        }
        if (DAT_005ab710 == 2) {
            DAT_04633300 = 0;
            DAT_005ab710 = 3;
            return 1;
        }
    } else {
        /* Close blinds (increase blind width) */
        if (DAT_04630e00 < 480) {
            DAT_04630e00 += 8;
            return 0;
        }
        if (DAT_005ab710 == 2) {
            DAT_04633300 = 0;
            DAT_005ab710 = 3;
            return 1;
        }
    }

    if (DAT_005ab710 == 3) {
        DAT_005ab710 = 4;
    }
    DAT_04633300 = 0;
    return 1;
}
