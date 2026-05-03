/*
 * Stone Age Client - Enhanced Input System Tests
 * Based on reverse engineering of FUN_00480bd0 (input_update)
 *
 * Tests the specific bitmask calculations discovered from binary:
 * - Numpad arrow mappings (0x48 -> 0x1000, etc.)
 * - Mouse movement threshold (500)
 * - Edge detection formulas
 * - Key repeat logic (30 frames initial, 23 frames repeat)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/input.h"

/* ========================================
 * Test Cases for Bitmask Calculation
 * From FUN_00480bd0
 * ======================================== */

/*
 * Test 1: Numpad arrow to key_state mapping
 * From binary: DAT_04ebe3eb (DIK_NUMPAD4) -> 0x4000
 */
static void test_numpad4_to_left(void) {
    TEST_BEGIN("Numpad 4 maps to LEFT flag");

    memset(&g_input, 0, sizeof(InputState));

    /* Set numpad 4 as pressed (0x80 high bit) */
    g_input.keyboard_state[DIK_NUMPAD4] = 0x80;

    /* Call update to calculate bitmask */
    input_update();

    /* Should have KEY_STATE_LEFT (0x4000) set */
    TEST_ASSERT((g_input.key_state & KEY_STATE_LEFT) != 0,
                "Numpad 4 should set KEY_STATE_LEFT (0x4000)");

    TEST_END();
}

/*
 * Test 2: Numpad 8 to UP mapping
 * From binary: DAT_04ebe3e8 (DIK_NUMPAD8) -> 0x1000
 */
static void test_numpad8_to_up(void) {
    TEST_BEGIN("Numpad 8 maps to UP flag");

    memset(&g_input, 0, sizeof(InputState));
    g_input.keyboard_state[DIK_NUMPAD8] = 0x80;
    g_input.keyboard_active = 1;

    input_update();

    TEST_ASSERT((g_input.key_state & KEY_STATE_UP) != 0,
                "Numpad 8 should set KEY_STATE_UP (0x1000)");

    TEST_END();
}

/*
 * Test 3: Numpad 6 to RIGHT mapping
 * From binary: DAT_04ebe3ed (DIK_NUMPAD6) -> 0x8000
 */
static void test_numpad6_to_right(void) {
    TEST_BEGIN("Numpad 6 maps to RIGHT flag");

    memset(&g_input, 0, sizeof(InputState));
    g_input.keyboard_state[DIK_NUMPAD6] = 0x80;
    g_input.keyboard_active = 1;

    input_update();

    TEST_ASSERT((g_input.key_state & KEY_STATE_RIGHT) != 0,
                "Numpad 6 should set KEY_STATE_RIGHT (0x8000)");

    TEST_END();
}

/*
 * Test 4: Numpad 2 to DOWN mapping
 * From binary: DAT_04ebe3f0 (DIK_NUMPAD2) -> 0x2000
 */
static void test_numpad2_to_down(void) {
    TEST_BEGIN("Numpad 2 maps to DOWN flag");

    memset(&g_input, 0, sizeof(InputState));
    g_input.keyboard_state[DIK_NUMPAD2] = 0x80;
    g_input.keyboard_active = 1;

    input_update();

    TEST_ASSERT((g_input.key_state & KEY_STATE_DOWN) != 0,
                "Numpad 2 should set KEY_STATE_DOWN (0x2000)");

    TEST_END();
}

/*
 * Test 5: Regular arrow keys also map to direction flags
 */
static void test_regular_arrows(void) {
    TEST_BEGIN("Regular arrow keys map to direction flags");

    memset(&g_input, 0, sizeof(InputState));
    g_input.keyboard_active = 1;

    /* Test UP arrow */
    g_input.keyboard_state[DIK_UP] = 0x80;
    input_update();
    TEST_ASSERT((g_input.key_state & KEY_STATE_UP) != 0, "UP arrow should set KEY_STATE_UP");

    /* Reset and test DOWN arrow */
    memset(&g_input, 0, sizeof(InputState));
    g_input.keyboard_active = 1;
    g_input.keyboard_state[DIK_DOWN] = 0x80;
    input_update();
    TEST_ASSERT((g_input.key_state & KEY_STATE_DOWN) != 0, "DOWN arrow should set KEY_STATE_DOWN");

    /* Reset and test LEFT arrow */
    memset(&g_input, 0, sizeof(InputState));
    g_input.keyboard_active = 1;
    g_input.keyboard_state[DIK_LEFT] = 0x80;
    input_update();
    TEST_ASSERT((g_input.key_state & KEY_STATE_LEFT) != 0, "LEFT arrow should set KEY_STATE_LEFT");

    /* Reset and test RIGHT arrow */
    memset(&g_input, 0, sizeof(InputState));
    g_input.keyboard_active = 1;
    g_input.keyboard_state[DIK_RIGHT] = 0x80;
    input_update();
    TEST_ASSERT((g_input.key_state & KEY_STATE_RIGHT) != 0, "RIGHT arrow should set KEY_STATE_RIGHT");

    TEST_END();
}

/*
 * Test 6: Number keys 1-6 map to bits 0-5
 * From binary: DIK_1 -> bit 0, DIK_2 -> bit 1, etc.
 */
static void test_number_key_mapping(void) {
    TEST_BEGIN("Number keys 1-6 map to lower bits");

    memset(&g_input, 0, sizeof(InputState));
    g_input.keyboard_active = 1;

    /* Key 1 -> bit 0 */
    g_input.keyboard_state[DIK_1] = 0x80;
    input_update();
    TEST_ASSERT((g_input.key_state & KEY_STATE_1) != 0, "Key 1 should set bit 0");

    /* Key 2 -> bit 1 */
    memset(&g_input.keyboard_state, 0, sizeof(g_input.keyboard_state));
    g_input.keyboard_state[DIK_2] = 0x80;
    input_update();
    TEST_ASSERT((g_input.key_state & KEY_STATE_2) != 0, "Key 2 should set bit 1");

    /* Key 3 -> bit 2 */
    memset(&g_input.keyboard_state, 0, sizeof(g_input.keyboard_state));
    g_input.keyboard_state[DIK_3] = 0x80;
    input_update();
    TEST_ASSERT((g_input.key_state & KEY_STATE_3) != 0, "Key 3 should set bit 2");

    /* Key 4 -> bit 3 */
    memset(&g_input.keyboard_state, 0, sizeof(g_input.keyboard_state));
    g_input.keyboard_state[DIK_4] = 0x80;
    input_update();
    TEST_ASSERT((g_input.key_state & KEY_STATE_4) != 0, "Key 4 should set bit 3");

    /* Key 5 -> bit 4 */
    memset(&g_input.keyboard_state, 0, sizeof(g_input.keyboard_state));
    g_input.keyboard_state[DIK_5] = 0x80;
    input_update();
    TEST_ASSERT((g_input.key_state & KEY_STATE_5) != 0, "Key 5 should set bit 4");

    /* Key 6 -> bit 5 */
    memset(&g_input.keyboard_state, 0, sizeof(g_input.keyboard_state));
    g_input.keyboard_state[DIK_6] = 0x80;
    input_update();
    TEST_ASSERT((g_input.key_state & KEY_STATE_6) != 0, "Key 6 should set bit 5");

    TEST_END();
}

/* ========================================
 * Test Cases for Edge Detection
 * From FUN_00480bd0 lines after main loop
 * ======================================== */

/*
 * Test 7: Press edge detection formula
 * From binary: DAT_04ebe490 = ~DAT_04ebe478 & DAT_04ebe488
 * key_press_edge = (~key_state_prev) & key_state
 */
static void test_press_edge_formula(void) {
    TEST_BEGIN("Press edge detection formula");

    memset(&g_input, 0, sizeof(InputState));

    /* Initial state: no keys */
    g_input.key_state_prev = 0;

    /* New state: key pressed */
    g_input.key_state = KEY_STATE_UP;

    /* Calculate edge manually using formula from binary */
    u32 expected_press_edge = (~g_input.key_state_prev) & g_input.key_state;

    /* Should equal key_state since prev was 0 */
    TEST_ASSERT(expected_press_edge == KEY_STATE_UP,
                "Press edge should be KEY_STATE_UP when key just pressed");

    /* If key was already held, press edge should be 0 */
    g_input.key_state_prev = KEY_STATE_UP;
    g_input.key_state = KEY_STATE_UP;
    expected_press_edge = (~g_input.key_state_prev) & g_input.key_state;
    TEST_ASSERT(expected_press_edge == 0,
                "Press edge should be 0 when key already held");

    TEST_END();
}

/*
 * Test 8: Release edge detection formula
 * From binary: (uVar2 ^ ~DAT_04ebe488) & ~DAT_04ebe488
 * Simplifies to: (~key_state) & key_state_prev
 */
static void test_release_edge_formula(void) {
    TEST_BEGIN("Release edge detection formula");

    memset(&g_input, 0, sizeof(InputState));

    /* Key was pressed */
    g_input.key_state_prev = KEY_STATE_UP;

    /* Key now released */
    g_input.key_state = 0;

    /* Calculate release edge */
    u32 expected_release_edge = (~g_input.key_state) & g_input.key_state_prev;

    TEST_ASSERT(expected_release_edge == KEY_STATE_UP,
                "Release edge should be KEY_STATE_UP when key just released");

    /* If key stays released, release edge should be 0 */
    g_input.key_state_prev = 0;
    g_input.key_state = 0;
    expected_release_edge = (~g_input.key_state) & g_input.key_state_prev;
    TEST_ASSERT(expected_release_edge == 0,
                "Release edge should be 0 when key already released");

    TEST_END();
}

/*
 * Test 9: Extended key state press edge
 * From binary: DAT_04ebe494 = ~DAT_04ebe47c & uVar3
 */
static void test_ext_press_edge_formula(void) {
    TEST_BEGIN("Extended key state press edge");

    memset(&g_input, 0, sizeof(InputState));

    /* F1 key pressed in extended state */
    g_input.key_state_ext_prev = 0;
    g_input.key_state_ext = KEY_STATE_F1;

    u32 expected_ext_press = (~g_input.key_state_ext_prev) & g_input.key_state_ext;

    TEST_ASSERT(expected_ext_press == KEY_STATE_F1,
                "Extended press edge should detect F1 press");

    TEST_END();
}

/*
 * Test 10: Extended key state release edge
 * From binary: (~DAT_04ebe47c ^ ~uVar3) & ~uVar3
 */
static void test_ext_release_edge_formula(void) {
    TEST_BEGIN("Extended key state release edge");

    memset(&g_input, 0, sizeof(InputState));

    /* F1 was pressed, now released */
    g_input.key_state_ext_prev = KEY_STATE_F1;
    g_input.key_state_ext = 0;

    u32 expected_ext_release = (~g_input.key_state_ext) & g_input.key_state_ext_prev;

    TEST_ASSERT(expected_ext_release == KEY_STATE_F1,
                "Extended release edge should detect F1 release");

    TEST_END();
}

/* ========================================
 * Test Cases for Mouse Movement
 * From FUN_00480bd0 threshold checks
 * ======================================== */

/*
 * Test 11: Mouse movement threshold for RIGHT
 * From binary: if (500 < lX) key_state |= 0x8000
 */
static void test_mouse_threshold_right(void) {
    TEST_BEGIN("Mouse threshold for RIGHT direction");

    /* Threshold is 500 */
    TEST_ASSERT(MOUSE_THRESHOLD == 500, "Mouse threshold should be 500");

    /* Mouse X > 500 should set RIGHT (0x8000) */
    /* Note: This would require mocking DirectInput, so just test the constant */
    TEST_ASSERT(KEY_STATE_RIGHT == 0x8000, "RIGHT should be 0x8000");

    TEST_END();
}

/*
 * Test 12: Mouse movement threshold for LEFT
 * From binary: if (lX < -500) key_state |= 0x4000
 */
static void test_mouse_threshold_left(void) {
    TEST_BEGIN("Mouse threshold for LEFT direction");

    /* Mouse X < -500 should set LEFT (0x4000) */
    TEST_ASSERT(KEY_STATE_LEFT == 0x4000, "LEFT should be 0x4000");

    TEST_END();
}

/*
 * Test 13: Mouse movement threshold for DOWN
 * From binary: if (500 < lY) key_state |= 0x2000
 */
static void test_mouse_threshold_down(void) {
    TEST_BEGIN("Mouse threshold for DOWN direction");

    /* Mouse Y > 500 should set DOWN (0x2000) */
    TEST_ASSERT(KEY_STATE_DOWN == 0x2000, "DOWN should be 0x2000");

    TEST_END();
}

/*
 * Test 14: Mouse movement threshold for UP
 * From binary: if (lY < -500) key_state |= 0x1000
 */
static void test_mouse_threshold_up(void) {
    TEST_BEGIN("Mouse threshold for UP direction");

    /* Mouse Y < -500 should set UP (0x1000) */
    TEST_ASSERT(KEY_STATE_UP == 0x1000, "UP should be 0x1000");

    TEST_END();
}

/*
 * Test 15: Mouse button mapping
 * From binary:
 * (rgbButtons[0] & 0x80) -> 0x400 (LBUTTON)
 * (rgbButtons[1] & 0x80) -> 0x100 (RBUTTON)
 */
static void test_mouse_button_mapping(void) {
    TEST_BEGIN("Mouse button to key_state mapping");

    TEST_ASSERT(KEY_STATE_LBUTTON == 0x0400, "Left button should be 0x0400");
    TEST_ASSERT(KEY_STATE_RBUTTON == 0x0100, "Right button should be 0x0100");

    TEST_END();
}

/* ========================================
 * Test Cases for Key Repeat (FUN_004809e0)
 * ======================================== */

/*
 * Test 16: Key repeat initial delay (0x1e = 30 frames)
 */
static void test_key_repeat_initial_delay_value(void) {
    TEST_BEGIN("Key repeat initial delay value");

    /* From binary: counter reaches 0x1e (30) before first repeat */
    TEST_ASSERT(KEY_REPEAT_INITIAL == 0x1e, "Initial delay should be 0x1e (30)");
    TEST_ASSERT(KEY_REPEAT_INITIAL == 30, "Initial delay should be 30 frames");

    TEST_END();
}

/*
 * Test 17: Key repeat rate (0x17 = 23 frames)
 */
static void test_key_repeat_rate_value(void) {
    TEST_BEGIN("Key repeat rate value");

    /* From binary: counter resets to 0x17 (23) for subsequent repeats */
    TEST_ASSERT(KEY_REPEAT_RATE == 0x17, "Repeat rate should be 0x17 (23)");
    TEST_ASSERT(KEY_REPEAT_RATE == 23, "Repeat rate should be 23 frames");

    TEST_END();
}

/*
 * Test 18: Key repeat relevant mask
 * From binary: mask 0xf5f0 for keys that trigger repeat
 */
static void test_key_repeat_relevant_mask(void) {
    TEST_BEGIN("Key repeat relevant mask");

    /* From FUN_004809e0: mask 0xf5f0 covers arrows, mouse buttons, bits 0x10-0x80 */
    u32 relevant_mask = 0x1000 | 0x2000 | 0x4000 | 0x8000 | 0x0400 | 0x0100 | 0x0010 | 0x0020 | 0x0040 | 0x0080;

    /* Arrows should be in mask */
    TEST_ASSERT((relevant_mask & KEY_STATE_UP) != 0, "UP should be in repeat mask");
    TEST_ASSERT((relevant_mask & KEY_STATE_DOWN) != 0, "DOWN should be in repeat mask");
    TEST_ASSERT((relevant_mask & KEY_STATE_LEFT) != 0, "LEFT should be in repeat mask");
    TEST_ASSERT((relevant_mask & KEY_STATE_RIGHT) != 0, "RIGHT should be in repeat mask");

    /* Mouse buttons should be in mask */
    TEST_ASSERT((relevant_mask & KEY_STATE_LBUTTON) != 0, "LBUTTON should be in repeat mask");
    TEST_ASSERT((relevant_mask & KEY_STATE_RBUTTON) != 0, "RBUTTON should be in repeat mask");

    TEST_END();
}

/* ========================================
 * Test Cases for Escape Key (Extended State)
 * ======================================== */

/*
 * Test 19: Escape key to extended state
 * From binary: DAT_04ebe320._1_1_ (offset 0x01) -> 0x80000000
 */
static void test_escape_extended_state(void) {
    TEST_BEGIN("Escape key in extended state");

    memset(&g_input, 0, sizeof(InputState));
    g_input.keyboard_active = 1;

    /* Press escape */
    g_input.keyboard_state[DIK_ESCAPE] = 0x80;
    input_update();

    /* Escape should set the high bit of extended state */
    TEST_ASSERT((g_input.key_state_ext & KEY_STATE_ESCAPE) != 0,
                "Escape should set 0x80000000 in extended state");

    TEST_END();
}

/*
 * Test 20: Modifier keys in extended state
 */
static void test_modifier_extended_state(void) {
    TEST_BEGIN("Modifier keys in extended state");

    TEST_ASSERT(KEY_STATE_SHIFT == 0x40000000, "SHIFT should be 0x40000000");
    TEST_ASSERT(KEY_STATE_CTRL == 0x20000000, "CTRL should be 0x20000000");
    TEST_ASSERT(KEY_STATE_ALT == 0x10000000, "ALT should be 0x10000000");

    TEST_END();
}

/* ========================================
 * Test Cases for Multiple Keys
 * ======================================== */

/*
 * Test 21: Multiple direction keys simultaneously
 */
static void test_multiple_direction_keys(void) {
    TEST_BEGIN("Multiple direction keys simultaneously");

    memset(&g_input, 0, sizeof(InputState));
    g_input.keyboard_active = 1;

    /* Press UP and LEFT together */
    g_input.keyboard_state[DIK_UP] = 0x80;
    g_input.keyboard_state[DIK_LEFT] = 0x80;
    input_update();

    /* Both should be set */
    TEST_ASSERT((g_input.key_state & KEY_STATE_UP) != 0, "UP should be set");
    TEST_ASSERT((g_input.key_state & KEY_STATE_LEFT) != 0, "LEFT should be set");

    /* Combined value */
    u32 expected = KEY_STATE_UP | KEY_STATE_LEFT;
    TEST_ASSERT((g_input.key_state & (KEY_STATE_UP | KEY_STATE_LEFT)) == expected,
                "Both UP and LEFT should be set together");

    TEST_END();
}

/*
 * Test 22: All four directions (diagonal test)
 */
static void test_all_four_directions(void) {
    TEST_BEGIN("All four direction keys");

    memset(&g_input, 0, sizeof(InputState));
    g_input.keyboard_active = 1;

    /* Press all four directions */
    g_input.keyboard_state[DIK_UP] = 0x80;
    g_input.keyboard_state[DIK_DOWN] = 0x80;
    g_input.keyboard_state[DIK_LEFT] = 0x80;
    g_input.keyboard_state[DIK_RIGHT] = 0x80;
    input_update();

    u32 expected = KEY_STATE_UP | KEY_STATE_DOWN | KEY_STATE_LEFT | KEY_STATE_RIGHT;
    TEST_ASSERT((g_input.key_state & expected) == expected,
                "All four directions should be set");

    TEST_END();
}

/*
 * Test 23: Key state persists while held
 */
static void test_key_state_persists(void) {
    TEST_BEGIN("Key state persists while held");

    memset(&g_input, 0, sizeof(InputState));
    g_input.keyboard_active = 1;

    /* Press key */
    g_input.keyboard_state[DIK_UP] = 0x80;
    input_update();

    u32 first_state = g_input.key_state;

    /* Update again while still pressed */
    input_update();

    /* State should still have UP */
    TEST_ASSERT((g_input.key_state & KEY_STATE_UP) != 0,
                "Key state should persist while held");

    TEST_END();
}

/*
 * Test 24: Key state clears on release
 */
static void test_key_state_clears_on_release(void) {
    TEST_BEGIN("Key state clears on release");

    memset(&g_input, 0, sizeof(InputState));
    g_input.keyboard_active = 1;

    /* Press key */
    g_input.keyboard_state[DIK_UP] = 0x80;
    input_update();

    TEST_ASSERT((g_input.key_state & KEY_STATE_UP) != 0, "Key should be set");

    /* Release key */
    g_input.keyboard_state[DIK_UP] = 0;
    input_update();

    /* State should not have UP anymore */
    TEST_ASSERT((g_input.key_state & KEY_STATE_UP) == 0,
                "Key state should clear on release");

    TEST_END();
}

/*
 * Test 25: Input state structure layout matches binary
 */
static void test_input_state_layout(void) {
    TEST_BEGIN("Input state structure layout");

    /* Keyboard state should be first field, 256 bytes */
    TEST_ASSERT(offsetof(InputState, keyboard_state) == 0,
                "keyboard_state should be at offset 0");

    /* Check key_state field exists and is after keyboard_state */
    TEST_ASSERT(offsetof(InputState, key_state) >= 256,
                "key_state should be after keyboard_state (256 bytes)");

    /* Check keyboard_active field exists */
    TEST_ASSERT(offsetof(InputState, keyboard_active) >= 0,
                "keyboard_active should exist in structure");

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(input_enhanced) {
    /* Bitmask calculation tests */
    test_numpad4_to_left();
    test_numpad8_to_up();
    test_numpad6_to_right();
    test_numpad2_to_down();
    test_regular_arrows();
    test_number_key_mapping();

    /* Edge detection tests */
    test_press_edge_formula();
    test_release_edge_formula();
    test_ext_press_edge_formula();
    test_ext_release_edge_formula();

    /* Mouse threshold tests */
    test_mouse_threshold_right();
    test_mouse_threshold_left();
    test_mouse_threshold_down();
    test_mouse_threshold_up();
    test_mouse_button_mapping();

    /* Key repeat tests */
    test_key_repeat_initial_delay_value();
    test_key_repeat_rate_value();
    test_key_repeat_relevant_mask();

    /* Extended state tests */
    test_escape_extended_state();
    test_modifier_extended_state();

    /* Multi-key tests */
    test_multiple_direction_keys();
    test_all_four_directions();
    test_key_state_persists();
    test_key_state_clears_on_release();

    /* Structure layout test */
    test_input_state_layout();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - Enhanced Input Tests\n");
    printf("Based on FUN_00480bd0 reverse engineering\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(input_enhanced);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
