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
