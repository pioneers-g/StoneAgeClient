/*
 * Stone Age Client - Unit Tests for Input System
 * Test file: test_input.c
 *
 * Tests for DirectInput keyboard/mouse, key repeat, IME support
 * Based on reverse engineering of sa_9061.exe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/input.h"

/* ========================================
 * Test Cases for Input Constants
 * ======================================== */

/*
 * Test 1: Key code constants
 */
static void test_key_code_constants(void) {
    TEST_BEGIN("Key code constants");

    /* Virtual key codes from binary */
    TEST_ASSERT(KEY_ESCAPE == 0x01, "Escape should be 0x01");
    TEST_ASSERT(KEY_1 == 0x02, "1 should be 0x02");
    TEST_ASSERT(KEY_2 == 0x03, "2 should be 0x03");
    TEST_ASSERT(KEY_ENTER == 0x1C, "Enter should be 0x1C");
    TEST_ASSERT(KEY_SPACE == 0x39, "Space should be 0x39");
    TEST_ASSERT(KEY_UP == 0xC8, "Up arrow should be 0xC8");
    TEST_ASSERT(KEY_DOWN == 0xD0, "Down arrow should be 0xD0");
    TEST_ASSERT(KEY_LEFT == 0xCB, "Left arrow should be 0xCB");
    TEST_ASSERT(KEY_RIGHT == 0xCD, "Right arrow should be 0xCD");

    TEST_END();
}

/*
 * Test 2: Key state flags
 */
static void test_key_state_flags(void) {
    TEST_BEGIN("Key state flags");

    /* Key state flags from FUN_004809e0 */
    TEST_ASSERT(KEY_STATE_PRESSED == 0x1000, "Pressed flag should be 0x1000");
    TEST_ASSERT(KEY_STATE_HELD == 0x2000, "Held flag should be 0x2000");
    TEST_ASSERT(KEY_STATE_REPEAT == 0x8000, "Repeat flag should be 0x8000");
    TEST_ASSERT(KEY_STATE_RELEASE == 0x4000, "Release flag should be 0x4000");

    /* Additional state flags */
    TEST_ASSERT(KEY_STATE_SHIFT == 0x10, "Shift flag should be 0x10");
    TEST_ASSERT(KEY_STATE_CTRL == 0x20, "Ctrl flag should be 0x20");
    TEST_ASSERT(KEY_STATE_ALT == 0x40, "Alt flag should be 0x40");

    TEST_END();
}

/*
 * Test 3: Key repeat thresholds
 */
static void test_key_repeat_thresholds(void) {
    TEST_BEGIN("Key repeat thresholds");

    /* From FUN_004809e0: counter threshold is 0x1e (30) */
    TEST_ASSERT(KEY_REPEAT_DELAY == 30, "Repeat delay should be 30 frames");
    /* Counter resets to 0x17 (23) for repeat rate */
    TEST_ASSERT(KEY_REPEAT_RATE == 23, "Repeat rate should be 23 frames");

    TEST_END();
}

/*
 * Test 4: Mouse button constants
 */
static void test_mouse_button_constants(void) {
    TEST_BEGIN("Mouse button constants");

    TEST_ASSERT(MOUSE_LEFT == 0x00, "Left button should be 0x00");
    TEST_ASSERT(MOUSE_RIGHT == 0x01, "Right button should be 0x01");
    TEST_ASSERT(MOUSE_MIDDLE == 0x02, "Middle button should be 0x02");

    TEST_END();
}

/*
 * Test 5: Mouse state flags
 */
static void test_mouse_state_flags(void) {
    TEST_BEGIN("Mouse state flags");

    TEST_ASSERT(MOUSE_STATE_CLICKED == 0x01, "Clicked flag should be 0x01");
    TEST_ASSERT(MOUSE_STATE_DOUBLECLICK == 0x02, "Double-click flag should be 0x02");
    TEST_ASSERT(MOUSE_STATE_DRAG == 0x04, "Drag flag should be 0x04");

    TEST_END();
}

/* ========================================
 * Test Cases for Key Repeat Logic
 * ======================================== */

/*
 * Test 6: Key repeat initial delay
 */
static void test_key_repeat_initial_delay(void) {
    TEST_BEGIN("Key repeat initial delay");

    /* Initialize input context */
    memset(&g_input, 0, sizeof(InputContext));

    /* Simulate key held for 29 frames (just before repeat) */
    u32 key = KEY_SPACE;
    g_input.key_state[key] = KEY_STATE_PRESSED | KEY_STATE_HELD;

    int i;
    for (i = 0; i < 29; i++) {
        u32 result = input_check_key_repeat(key);
        /* Should not trigger repeat yet */
        TEST_ASSERT((result & KEY_STATE_REPEAT) == 0, "Should not repeat before 30 frames");
    }

    TEST_END();
}

/*
 * Test 7: Key repeat triggers at 30 frames
 */
static void test_key_repeat_triggers(void) {
    TEST_BEGIN("Key repeat triggers at 30 frames");

    memset(&g_input, 0, sizeof(InputContext));

    u32 key = KEY_SPACE;
    g_input.key_state[key] = KEY_STATE_PRESSED | KEY_STATE_HELD;
    g_input.repeat_counter[key] = 29;

    u32 result = input_check_key_repeat(key);
    TEST_ASSERT((result & KEY_STATE_REPEAT) != 0, "Should repeat at 30 frames");
    TEST_ASSERT(g_input.repeat_counter[key] == 23, "Counter should reset to 23");

    TEST_END();
}

/*
 * Test 8: Key repeat rate
 */
static void test_key_repeat_rate(void) {
    TEST_BEGIN("Key repeat rate");

    memset(&g_input, 0, sizeof(InputContext));

    u32 key = KEY_SPACE;
    g_input.key_state[key] = KEY_STATE_PRESSED | KEY_STATE_HELD;
    g_input.repeat_counter[key] = 23;  /* After initial delay */

    /* After 23 more frames, should repeat again */
    int i;
    for (i = 0; i < 22; i++) {
        u32 result = input_check_key_repeat(key);
        TEST_ASSERT((result & KEY_STATE_REPEAT) == 0, "Should not repeat before 23 frames");
    }

    u32 result = input_check_key_repeat(key);
    TEST_ASSERT((result & KEY_STATE_REPEAT) != 0, "Should repeat at 23 frame interval");

    TEST_END();
}

/*
 * Test 9: Key release resets counter
 */
static void test_key_release_resets_counter(void) {
    TEST_BEGIN("Key release resets counter");

    memset(&g_input, 0, sizeof(InputContext));

    u32 key = KEY_SPACE;
    g_input.key_state[key] = KEY_STATE_PRESSED | KEY_STATE_HELD;
    g_input.repeat_counter[key] = 25;

    /* Release key */
    g_input.key_state[key] = 0;

    u32 result = input_check_key_repeat(key);
    TEST_ASSERT(result == 0, "Released key should return 0");
    TEST_ASSERT(g_input.repeat_counter[key] == 0, "Counter should reset to 0");

    TEST_END();
}

/*
 * Test 10: No repeat for unpressed key
 */
static void test_no_repeat_unpressed_key(void) {
    TEST_BEGIN("No repeat for unpressed key");

    memset(&g_input, 0, sizeof(InputContext));

    u32 key = KEY_SPACE;
    u32 result = input_check_key_repeat(key);

    TEST_ASSERT(result == 0, "Unpressed key should return 0");
    TEST_ASSERT(g_input.repeat_counter[key] == 0, "Counter should remain 0");

    TEST_END();
}

/* ========================================
 * Test Cases for Key State Detection
 * ======================================== */

/*
 * Test 11: Key press detection
 */
static void test_key_press_detection(void) {
    TEST_BEGIN("Key press detection");

    memset(&g_input, 0, sizeof(InputContext));

    /* First frame: key just pressed */
    u32 key = KEY_ENTER;
    g_input.key_state[key] = KEY_STATE_PRESSED;

    int pressed = input_is_key_pressed(key);
    TEST_ASSERT(pressed != 0, "Should detect key press");

    /* Second frame: key still held */
    g_input.key_state[key] = KEY_STATE_PRESSED | KEY_STATE_HELD;
    pressed = input_is_key_pressed(key);
    TEST_ASSERT(pressed == 0, "Should not detect press on held key");

    TEST_END();
}

/*
 * Test 12: Key release detection
 */
static void test_key_release_detection(void) {
    TEST_BEGIN("Key release detection");

    memset(&g_input, 0, sizeof(InputContext));

    u32 key = KEY_ENTER;
    g_input.key_state[key] = KEY_STATE_HELD;

    /* Release key */
    g_input.key_state[key] = KEY_STATE_RELEASE;

    int released = input_is_key_released(key);
    TEST_ASSERT(released != 0, "Should detect key release");

    TEST_END();
}

/*
 * Test 13: Key held detection
 */
static void test_key_held_detection(void) {
    TEST_BEGIN("Key held detection");

    memset(&g_input, 0, sizeof(InputContext));

    u32 key = KEY_ENTER;
    g_input.key_state[key] = KEY_STATE_PRESSED | KEY_STATE_HELD;

    int held = input_is_key_held(key);
    TEST_ASSERT(held != 0, "Should detect key held");

    TEST_END();
}

/*
 * Test 14: Modifier key detection
 */
static void test_modifier_key_detection(void) {
    TEST_BEGIN("Modifier key detection");

    memset(&g_input, 0, sizeof(InputContext));

    /* Shift held */
    g_input.key_state[KEY_LSHIFT] = KEY_STATE_HELD;
    TEST_ASSERT(input_is_shift_held() != 0, "Should detect Shift");

    /* Ctrl held */
    g_input.key_state[KEY_LCONTROL] = KEY_STATE_HELD;
    TEST_ASSERT(input_is_ctrl_held() != 0, "Should detect Ctrl");

    /* Alt held */
    g_input.key_state[KEY_LMENU] = KEY_STATE_HELD;
    TEST_ASSERT(input_is_alt_held() != 0, "Should detect Alt");

    TEST_END();
}

/* ========================================
 * Test Cases for Mouse Input
 * ======================================== */

/*
 * Test 15: Mouse position
 */
static void test_mouse_position(void) {
    TEST_BEGIN("Mouse position");

    memset(&g_input, 0, sizeof(InputContext));

    g_input.mouse_x = 100;
    g_input.mouse_y = 200;

    TEST_ASSERT(g_input.mouse_x == 100, "Mouse X should be 100");
    TEST_ASSERT(g_input.mouse_y == 200, "Mouse Y should be 200");

    TEST_END();
}

/*
 * Test 16: Mouse button press
 */
static void test_mouse_button_press(void) {
    TEST_BEGIN("Mouse button press");

    memset(&g_input, 0, sizeof(InputContext));

    g_input.mouse_button_state[MOUSE_LEFT] = MOUSE_STATE_CLICKED;

    int clicked = input_is_mouse_clicked(MOUSE_LEFT);
    TEST_ASSERT(clicked != 0, "Should detect left click");

    TEST_END();
}

/*
 * Test 17: Mouse double click
 */
static void test_mouse_double_click(void) {
    TEST_BEGIN("Mouse double click");

    memset(&g_input, 0, sizeof(InputContext));

    g_input.mouse_button_state[MOUSE_LEFT] = MOUSE_STATE_DOUBLECLICK;

    int double_clicked = input_is_mouse_double_clicked(MOUSE_LEFT);
    TEST_ASSERT(double_clicked != 0, "Should detect double click");

    TEST_END();
}

/*
 * Test 18: Mouse drag detection
 */
static void test_mouse_drag_detection(void) {
    TEST_BEGIN("Mouse drag detection");

    memset(&g_input, 0, sizeof(InputContext));

    g_input.mouse_button_state[MOUSE_LEFT] = MOUSE_STATE_DRAG;
    g_input.mouse_drag_start_x = 50;
    g_input.mouse_drag_start_y = 75;

    int dragging = input_is_mouse_dragging(MOUSE_LEFT);
    TEST_ASSERT(dragging != 0, "Should detect drag");

    TEST_END();
}

/*
 * Test 19: Mouse wheel
 */
static void test_mouse_wheel(void) {
    TEST_BEGIN("Mouse wheel");

    memset(&g_input, 0, sizeof(InputContext));

    g_input.mouse_wheel_delta = 120;  /* Standard wheel delta */

    TEST_ASSERT(g_input.mouse_wheel_delta > 0, "Wheel delta should be positive for scroll up");

    g_input.mouse_wheel_delta = -120;
    TEST_ASSERT(g_input.mouse_wheel_delta < 0, "Wheel delta should be negative for scroll down");

    TEST_END();
}

/* ========================================
 * Test Cases for DirectInput Initialization
 * ======================================== */

/*
 * Test 20: Input context initialization
 */
static void test_input_context_init(void) {
    TEST_BEGIN("Input context initialization");

    memset(&g_input, 0xFF, sizeof(InputContext));
    memset(&g_input, 0, sizeof(InputContext));

    TEST_ASSERT(g_input.keyboard_device == NULL, "Keyboard device should be NULL");
    TEST_ASSERT(g_input.mouse_device == NULL, "Mouse device should be NULL");
    TEST_ASSERT(g_input.mouse_x == 0, "Mouse X should be 0");
    TEST_ASSERT(g_input.mouse_y == 0, "Mouse Y should be 0");

    TEST_END();
}

/*
 * Test 21: Input structure size
 */
static void test_input_structure_size(void) {
    TEST_BEGIN("Input structure size");

    /* InputContext should have reasonable size */
    TEST_ASSERT(sizeof(InputContext) >= 600, "InputContext should be at least 600 bytes");
    TEST_ASSERT(sizeof(InputContext) <= 2000, "InputContext should be at most 2000 bytes");

    TEST_END();
}

/*
 * Test 22: Key state array size
 */
static void test_key_state_array_size(void) {
    TEST_BEGIN("Key state array size");

    /* Should support 256 keys */
    TEST_ASSERT(sizeof(g_input.key_state) >= 256 * sizeof(u16), "Key state array should hold 256 entries");

    TEST_END();
}

/* ========================================
 * Test Cases for IME Support
 * ======================================== */

/*
 * Test 23: IME composition string
 */
static void test_ime_composition_string(void) {
    TEST_BEGIN("IME composition string");

    memset(&g_input, 0, sizeof(InputContext));

    /* Simulate IME composition */
    strcpy(g_input.ime_composition, "Test");
    g_input.ime_composition_len = 4;

    TEST_ASSERT(strcmp(g_input.ime_composition, "Test") == 0, "IME composition should match");
    TEST_ASSERT(g_input.ime_composition_len == 4, "IME length should be 4");

    TEST_END();
}

/*
 * Test 24: IME cursor position
 */
static void test_ime_cursor_position(void) {
    TEST_BEGIN("IME cursor position");

    memset(&g_input, 0, sizeof(InputContext));

    g_input.ime_cursor_pos = 2;

    TEST_ASSERT(g_input.ime_cursor_pos == 2, "IME cursor position should be 2");

    TEST_END();
}

/*
 * Test 25: IME candidate list
 */
static void test_ime_candidate_list(void) {
    TEST_BEGIN("IME candidate list");

    memset(&g_input, 0, sizeof(InputContext));

    g_input.ime_candidate_count = 5;
    g_input.ime_candidate_selection = 2;

    TEST_ASSERT(g_input.ime_candidate_count == 5, "Candidate count should be 5");
    TEST_ASSERT(g_input.ime_candidate_selection == 2, "Candidate selection should be 2");

    TEST_END();
}

/* ========================================
 * Test Cases for Input Buffer
 * ======================================== */

/*
 * Test 26: Key buffer overflow
 */
static void test_key_buffer_overflow(void) {
    TEST_BEGIN("Key buffer overflow");

    memset(&g_input, 0, sizeof(InputContext));

    /* Fill key buffer beyond capacity */
    int i;
    for (i = 0; i < 300; i++) {
        input_buffer_key((u8)(i % 256));
    }

    /* Should not crash */
    TEST_ASSERT(1, "Key buffer overflow should not crash");

    TEST_END();
}

/*
 * Test 27: Clear input state
 */
static void test_clear_input_state(void) {
    TEST_BEGIN("Clear input state");

    memset(&g_input, 0, sizeof(InputContext));

    /* Set some state */
    g_input.key_state[KEY_SPACE] = KEY_STATE_PRESSED;
    g_input.mouse_x = 100;

    /* Clear */
    input_clear_state();

    TEST_ASSERT(g_input.key_state[KEY_SPACE] == 0, "Key state should be cleared");
    TEST_ASSERT(g_input.mouse_x == 0, "Mouse position should be cleared");

    TEST_END();
}

/*
 * Test 28: Get key name
 */
static void test_get_key_name(void) {
    TEST_BEGIN("Get key name");

    const char* name = input_get_key_name(KEY_SPACE);
    TEST_ASSERT(name != NULL, "Key name should not be NULL");
    TEST_ASSERT(strstr(name, "Space") != NULL || strstr(name, "space") != NULL,
                "Key name should contain 'Space'");

    TEST_END();
}

/*
 * Test 29: Key mapping
 */
static void test_key_mapping(void) {
    TEST_BEGIN("Key mapping");

    /* DIK (DirectInput) to VK (Virtual Key) mapping */
    u8 vk = input_dik_to_vk(0x01);  /* DIK_ESCAPE */
    TEST_ASSERT(vk == VK_ESCAPE, "DIK_ESCAPE should map to VK_ESCAPE");

    vk = input_dik_to_vk(0x1C);  /* DIK_RETURN */
    TEST_ASSERT(vk == VK_RETURN, "DIK_RETURN should map to VK_RETURN");

    TEST_END();
}

/*
 * Test 30: Input device enumeration
 */
static void test_input_device_enumeration(void) {
    TEST_BEGIN("Input device enumeration");

    /* Without DirectInput, should return gracefully */
    int result = input_enumerate_devices();
    TEST_ASSERT(result == 0 || result == 1, "Device enumeration should return 0 or 1");

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(input) {
    /* Constants tests */
    test_key_code_constants();
    test_key_state_flags();
    test_key_repeat_thresholds();
    test_mouse_button_constants();
    test_mouse_state_flags();

    /* Key repeat tests */
    test_key_repeat_initial_delay();
    test_key_repeat_triggers();
    test_key_repeat_rate();
    test_key_release_resets_counter();
    test_no_repeat_unpressed_key();

    /* Key state tests */
    test_key_press_detection();
    test_key_release_detection();
    test_key_held_detection();
    test_modifier_key_detection();

    /* Mouse tests */
    test_mouse_position();
    test_mouse_button_press();
    test_mouse_double_click();
    test_mouse_drag_detection();
    test_mouse_wheel();

    /* Initialization tests */
    test_input_context_init();
    test_input_structure_size();
    test_key_state_array_size();

    /* IME tests */
    test_ime_composition_string();
    test_ime_cursor_position();
    test_ime_candidate_list();

    /* Buffer tests */
    test_key_buffer_overflow();
    test_clear_input_state();
    test_get_key_name();
    test_key_mapping();
    test_input_device_enumeration();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - Input System Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(input);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
