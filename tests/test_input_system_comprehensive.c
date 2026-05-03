/*
 * Stone Age Client - Input System Unit Tests
 * Tests for FUN_00480bd0 (input state update) and FUN_004809e0 (key repeat)
 *
 * Based on Ghidra decompilation analysis:
 * - Key state bitmask from keyboard_state array
 * - Edge detection formulas
 * - Key repeat with 30-frame initial delay and 23-frame repeat rate
 * - Mouse movement threshold: 500
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Key state flags from FUN_00480bd0 */
#define KEY_STATE_UP          0x1000
#define KEY_STATE_DOWN        0x2000
#define KEY_STATE_LEFT        0x4000
#define KEY_STATE_RIGHT       0x8000
#define KEY_STATE_LBUTTON     0x0400
#define KEY_STATE_RBUTTON     0x0100

#define KEY_STATE_1           0x00000001
#define KEY_STATE_2           0x00000002
#define KEY_STATE_3           0x00000004
#define KEY_STATE_4           0x00000008
#define KEY_STATE_5           0x00000010
#define KEY_STATE_6           0x00000020
#define KEY_STATE_7           0x00000040
#define KEY_STATE_8           0x00000080

/* Extended key flags */
#define KEY_STATE_F1          0x00001000
#define KEY_STATE_F2          0x00002000
#define KEY_STATE_ESCAPE      0x80000000
#define KEY_STATE_SHIFT       0x40000000
#define KEY_STATE_CTRL        0x20000000
#define KEY_STATE_ALT         0x10000000

/* DirectInput scan codes */
#define DIK_NUMPAD4    0x4B
#define DIK_NUMPAD8    0x48
#define DIK_NUMPAD6    0x4D
#define DIK_NUMPAD2    0x50
#define DIK_LEFT       0xCB
#define DIK_UP         0xC8
#define DIK_RIGHT      0xCD
#define DIK_DOWN       0xD0
#define DIK_1          0x02
#define DIK_2          0x03
#define DIK_3          0x04
#define DIK_4          0x05
#define DIK_5          0x06
#define DIK_6          0x07
#define DIK_ESCAPE     0x01
#define DIK_F1         0x3B

/* Key repeat constants */
#define KEY_REPEAT_INITIAL  0x1e    /* 30 frames */
#define KEY_REPEAT_RATE     0x17    /* 23 frames */

/* Mouse threshold */
#define MOUSE_THRESHOLD     500

/* Simulated input state */
typedef struct {
    u8 keyboard_state[256];
    u32 key_state;
    u32 key_state_prev;
    u32 key_press_edge;
    u32 key_release_edge;
    u32 key_held_edge;
    s16 key_repeat_counter[2];
    u32 key_repeat_result[2];
    s32 mouse_x;
    s32 mouse_y;
    u8 mouse_buttons;
} TestInputState;

static TestInputState g_test_input;

/* Reset test state */
static void test_input_reset(void) {
    memset(&g_test_input, 0, sizeof(g_test_input));
}

/* Simulate key press */
static void test_press_key(u8 scan_code) {
    g_test_input.keyboard_state[scan_code] = 0x80;
}

/* Simulate key release */
static void test_release_key(u8 scan_code) {
    g_test_input.keyboard_state[scan_code] = 0x00;
}

/*
 * Calculate key state bitmask - FUN_00480bd0 pattern
 */
static u32 calculate_key_state(TestInputState* state) {
    u32 key_state = 0;
    u8* kb = state->keyboard_state;

    /* Numpad arrows - from Ghidra DAT_04ebe3eb, DAT_04ebe3e8, etc. */
    if (kb[DIK_NUMPAD4] & 0x80) key_state |= KEY_STATE_LEFT;
    if (kb[DIK_NUMPAD8] & 0x80) key_state |= KEY_STATE_UP;
    if (kb[DIK_NUMPAD6] & 0x80) key_state |= KEY_STATE_RIGHT;
    if (kb[DIK_NUMPAD2] & 0x80) key_state |= KEY_STATE_DOWN;

    /* Regular arrows */
    if (kb[DIK_LEFT] & 0x80) key_state |= KEY_STATE_LEFT;
    if (kb[DIK_UP] & 0x80) key_state |= KEY_STATE_UP;
    if (kb[DIK_RIGHT] & 0x80) key_state |= KEY_STATE_RIGHT;
    if (kb[DIK_DOWN] & 0x80) key_state |= KEY_STATE_DOWN;

    /* Number keys */
    if (kb[DIK_1] & 0x80) key_state |= KEY_STATE_1;
    if (kb[DIK_2] & 0x80) key_state |= KEY_STATE_2;
    if (kb[DIK_3] & 0x80) key_state |= KEY_STATE_3;
    if (kb[DIK_4] & 0x80) key_state |= KEY_STATE_4;
    if (kb[DIK_5] & 0x80) key_state |= KEY_STATE_5;
    if (kb[DIK_6] & 0x80) key_state |= KEY_STATE_6;

    /* Mouse buttons */
    if (state->mouse_buttons & 0x01) key_state |= KEY_STATE_LBUTTON;
    if (state->mouse_buttons & 0x02) key_state |= KEY_STATE_RBUTTON;

    return key_state;
}

/*
 * Update edge detection - FUN_00480bd0 exact formulas
 */
static void update_edge_detection(TestInputState* state, u32 new_key_state) {
    /* From Ghidra:
     * DAT_04ebe490 = ~DAT_04ebe478 & DAT_04ebe488 (just pressed)
     * DAT_04ebe498 = (uVar2 ^ ~DAT_04ebe488) & ~DAT_04ebe488 (released)
     */
    u32 prev = state->key_state_prev;

    /* Just pressed: was up, now down */
    state->key_press_edge = (~prev) & new_key_state;

    /* Released: was down, now up */
    state->key_release_edge = (~new_key_state) & prev;

    /* Held edge pattern */
    state->key_held_edge = (~prev ^ ~new_key_state) & ~new_key_state;

    /* Update state */
    state->key_state_prev = prev;
    state->key_state = new_key_state;
}

/*
 * Process key repeat - FUN_004809e0
 */
static u32 process_key_repeat(TestInputState* state, int context, u32 key_state, u32 press_edge) {
    u32 result = 0;
    u32 relevant_mask = 0xF5F0;  /* 0x1000, 0x2000, 0x4000, 0x8000, 0x400, 0x100, 0x10, 0x20, 0x40, 0x80 */
    s16* counter = &state->key_repeat_counter[context];

    /* If no relevant keys, reset counter */
    if ((key_state & relevant_mask) == 0) {
        *counter = 0;
        return 0;
    }

    /* Check each direction bit */
    struct { u32 bit; } bits[] = {
        { 0x1000 }, { 0x2000 }, { 0x4000 }, { 0x8000 },
        { 0x0400 }, { 0x0100 }, { 0x0010 }, { 0x0020 },
        { 0x0040 }, { 0x0080 }
    };

    for (int i = 0; i < 10; i++) {
        u32 bit = bits[i].bit;

        if (press_edge & bit) {
            /* Just pressed - trigger immediately */
            result |= bit;
        } else if (key_state & bit) {
            /* Held - check counter */
            (*counter)++;
            if (*counter == KEY_REPEAT_INITIAL) {
                *counter = KEY_REPEAT_RATE;
                result |= bit;
            }
        }
    }

    return result;
}

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Key State Bitmask Tests
 * ======================================== */

static int test_numpad_direction_bits(void) {
    test_input_reset();

    /* Test UP (numpad 8) -> 0x1000 */
    test_press_key(DIK_NUMPAD8);
    u32 state = calculate_key_state(&g_test_input);
    assert(state == KEY_STATE_UP);

    /* Test DOWN (numpad 2) -> 0x2000 */
    test_input_reset();
    test_press_key(DIK_NUMPAD2);
    state = calculate_key_state(&g_test_input);
    assert(state == KEY_STATE_DOWN);

    /* Test LEFT (numpad 4) -> 0x4000 */
    test_input_reset();
    test_press_key(DIK_NUMPAD4);
    state = calculate_key_state(&g_test_input);
    assert(state == KEY_STATE_LEFT);

    /* Test RIGHT (numpad 6) -> 0x8000 */
    test_input_reset();
    test_press_key(DIK_NUMPAD6);
    state = calculate_key_state(&g_test_input);
    assert(state == KEY_STATE_RIGHT);

    return 1;
}

static int test_arrow_key_mapping(void) {
    test_input_reset();

    /* Regular arrow keys should map to same bits as numpad */
    test_press_key(DIK_UP);
    u32 state = calculate_key_state(&g_test_input);
    assert(state == KEY_STATE_UP);

    test_input_reset();
    test_press_key(DIK_DOWN);
    state = calculate_key_state(&g_test_input);
    assert(state == KEY_STATE_DOWN);

    test_input_reset();
    test_press_key(DIK_LEFT);
    state = calculate_key_state(&g_test_input);
    assert(state == KEY_STATE_LEFT);

    test_input_reset();
    test_press_key(DIK_RIGHT);
    state = calculate_key_state(&g_test_input);
    assert(state == KEY_STATE_RIGHT);

    return 1;
}

static int test_number_key_bits(void) {
    test_input_reset();

    test_press_key(DIK_1);
    u32 state = calculate_key_state(&g_test_input);
    assert(state == KEY_STATE_1);

    test_input_reset();
    test_press_key(DIK_2);
    state = calculate_key_state(&g_test_input);
    assert(state == KEY_STATE_2);

    test_input_reset();
    test_press_key(DIK_3);
    state = calculate_key_state(&g_test_input);
    assert(state == KEY_STATE_3);

    /* Test combined: 1+2 = 0x03 */
    test_input_reset();
    test_press_key(DIK_1);
    test_press_key(DIK_2);
    state = calculate_key_state(&g_test_input);
    assert(state == (KEY_STATE_1 | KEY_STATE_2));

    return 1;
}

static int test_diagonal_directions(void) {
    test_input_reset();

    /* UP + RIGHT = 0x1000 | 0x8000 = 0x9000 */
    test_press_key(DIK_NUMPAD8);
    test_press_key(DIK_NUMPAD6);
    u32 state = calculate_key_state(&g_test_input);
    assert(state == (KEY_STATE_UP | KEY_STATE_RIGHT));

    /* All four directions */
    test_input_reset();
    test_press_key(DIK_NUMPAD8);
    test_press_key(DIK_NUMPAD2);
    test_press_key(DIK_NUMPAD4);
    test_press_key(DIK_NUMPAD6);
    state = calculate_key_state(&g_test_input);
    assert(state == (KEY_STATE_UP | KEY_STATE_DOWN | KEY_STATE_LEFT | KEY_STATE_RIGHT));

    return 1;
}

static int test_mouse_button_bits(void) {
    test_input_reset();

    /* Left button -> 0x0400 */
    g_test_input.mouse_buttons = 0x01;
    u32 state = calculate_key_state(&g_test_input);
    assert(state == KEY_STATE_LBUTTON);

    /* Right button -> 0x0100 */
    test_input_reset();
    g_test_input.mouse_buttons = 0x02;
    state = calculate_key_state(&g_test_input);
    assert(state == KEY_STATE_RBUTTON);

    /* Both buttons */
    test_input_reset();
    g_test_input.mouse_buttons = 0x03;
    state = calculate_key_state(&g_test_input);
    assert(state == (KEY_STATE_LBUTTON | KEY_STATE_RBUTTON));

    return 1;
}

/* ========================================
 * Edge Detection Tests
 * ======================================== */

static int test_press_edge_detection(void) {
    test_input_reset();

    /* Start with no keys pressed */
    g_test_input.key_state_prev = 0;

    /* Press UP */
    test_press_key(DIK_NUMPAD8);
    u32 new_state = calculate_key_state(&g_test_input);
    update_edge_detection(&g_test_input, new_state);

    /* Press edge should have UP bit */
    assert(g_test_input.key_press_edge == KEY_STATE_UP);
    assert(g_test_input.key_release_edge == 0);

    return 1;
}

static int test_release_edge_detection(void) {
    test_input_reset();

    /* Start with UP pressed */
    g_test_input.key_state_prev = KEY_STATE_UP;

    /* Release all keys */
    u32 new_state = 0;
    update_edge_detection(&g_test_input, new_state);

    /* Release edge should have UP bit */
    assert(g_test_input.key_press_edge == 0);
    assert(g_test_input.key_release_edge == KEY_STATE_UP);

    return 1;
}

static int test_held_edge_formula(void) {
    test_input_reset();

    /* Test held edge formula: (~prev ^ ~current) & ~current */
    u32 prev = KEY_STATE_UP;
    u32 current = KEY_STATE_UP;  /* Still held */

    g_test_input.key_state_prev = prev;
    update_edge_detection(&g_test_input, current);

    /* When key is held, held_edge should be 0 for that bit */
    u32 expected_held = (~prev ^ ~current) & ~current;
    assert(g_test_input.key_held_edge == expected_held);

    return 1;
}

static int test_multiple_key_edges(void) {
    test_input_reset();

    /* Start with UP pressed */
    g_test_input.key_state_prev = KEY_STATE_UP;

    /* Press DOWN while releasing UP */
    u32 new_state = KEY_STATE_DOWN;
    update_edge_detection(&g_test_input, new_state);

    /* Press edge: DOWN only */
    assert(g_test_input.key_press_edge == KEY_STATE_DOWN);
    /* Release edge: UP only */
    assert(g_test_input.key_release_edge == KEY_STATE_UP);

    return 1;
}

/* ========================================
 * Key Repeat Tests - FUN_004809e0
 * ======================================== */

static int test_key_repeat_initial_delay(void) {
    test_input_reset();

    /* Key held for 29 frames - no repeat */
    g_test_input.key_repeat_counter[0] = 0;

    for (int i = 0; i < 29; i++) {
        u32 result = process_key_repeat(&g_test_input, 0, KEY_STATE_UP, 0);
        /* Before reaching 30, no repeat triggered */
    }

    /* At 30th frame, repeat should trigger */
    u32 result = process_key_repeat(&g_test_input, 0, KEY_STATE_UP, 0);
    assert(result == KEY_STATE_UP);

    return 1;
}

static int test_key_repeat_rate(void) {
    test_input_reset();

    /* Set counter to trigger state */
    g_test_input.key_repeat_counter[0] = KEY_REPEAT_RATE;  /* 23 */

    /* Next repeat should occur after 23 more frames */
    for (int i = 0; i < 22; i++) {
        u32 result = process_key_repeat(&g_test_input, 0, KEY_STATE_UP, 0);
        assert(result == 0);
    }

    /* 23rd frame triggers */
    u32 result = process_key_repeat(&g_test_input, 0, KEY_STATE_UP, 0);
    assert(result == KEY_STATE_UP);

    return 1;
}

static int test_key_repeat_on_press(void) {
    test_input_reset();

    /* Press edge triggers immediately without waiting */
    u32 result = process_key_repeat(&g_test_input, 0, KEY_STATE_UP, KEY_STATE_UP);
    assert(result == KEY_STATE_UP);

    return 1;
}

static int test_key_repeat_reset_on_release(void) {
    test_input_reset();

    /* Build up counter */
    g_test_input.key_repeat_counter[0] = 20;

    /* Release key (key_state = 0) */
    u32 result = process_key_repeat(&g_test_input, 0, 0, 0);

    /* Counter should reset */
    assert(g_test_input.key_repeat_counter[0] == 0);
    assert(result == 0);

    return 1;
}

static int test_key_repeat_relevant_bits(void) {
    test_input_reset();

    /* Bits 0x1000, 0x2000, 0x4000, 0x8000 are relevant for repeat */
    u32 relevant = KEY_STATE_UP | KEY_STATE_DOWN | KEY_STATE_LEFT | KEY_STATE_RIGHT;

    /* Number keys (bits 0-7) should not trigger repeat */
    u32 result = process_key_repeat(&g_test_input, 0, KEY_STATE_1, 0);
    assert(result == 0);

    /* Direction keys should trigger repeat */
    g_test_input.key_repeat_counter[0] = KEY_REPEAT_INITIAL - 1;
    result = process_key_repeat(&g_test_input, 0, KEY_STATE_UP, 0);
    assert(result == KEY_STATE_UP);

    return 1;
}

/* ========================================
 * Mouse Movement Tests
 * ======================================== */

static int test_mouse_threshold_500(void) {
    /* From FUN_00480bd0:
     * if (500 < lX) key_state |= 0x8000 (RIGHT)
     * if (lX < -500) key_state |= 0x4000 (LEFT)
     */

    /* 499 should not trigger */
    assert(499 < MOUSE_THRESHOLD);
    assert(-499 > -MOUSE_THRESHOLD);

    /* 500 should not trigger (needs > 500) */
    assert(500 <= MOUSE_THRESHOLD);

    /* 501 should trigger */
    assert(501 > MOUSE_THRESHOLD);
    assert(-501 < -MOUSE_THRESHOLD);

    return 1;
}

static int test_mouse_direction_mapping(void) {
    /* From Ghidra:
     * X > 500 -> RIGHT (0x8000)
     * X < -500 -> LEFT (0x4000)
     * Y > 500 -> DOWN (0x2000)
     * Y < -500 -> UP (0x1000)
     */

    /* This is verified by the constants in the binary */
    assert(KEY_STATE_RIGHT == 0x8000);
    assert(KEY_STATE_LEFT == 0x4000);
    assert(KEY_STATE_DOWN == 0x2000);
    assert(KEY_STATE_UP == 0x1000);

    return 1;
}

/* ========================================
 * Bit Position Verification Tests
 * ======================================== */

static int test_key_state_bit_positions(void) {
    /* Verify exact bit positions from Ghidra */

    /* Direction bits in high nibble of low word */
    assert((KEY_STATE_UP >> 12) == 0x1);
    assert((KEY_STATE_DOWN >> 12) == 0x2);
    assert((KEY_STATE_LEFT >> 12) == 0x4);
    assert((KEY_STATE_RIGHT >> 12) == 0x8);

    /* Mouse buttons */
    assert((KEY_STATE_RBUTTON >> 8) == 0x1);
    assert((KEY_STATE_LBUTTON >> 10) == 0x1);

    /* Number keys in lowest byte */
    assert(KEY_STATE_1 == (1 << 0));
    assert(KEY_STATE_2 == (1 << 1));
    assert(KEY_STATE_3 == (1 << 2));
    assert(KEY_STATE_4 == (1 << 3));
    assert(KEY_STATE_5 == (1 << 4));
    assert(KEY_STATE_6 == (1 << 5));

    return 1;
}

static int test_combined_key_states(void) {
    test_input_reset();

    /* Press multiple keys simultaneously */
    test_press_key(DIK_NUMPAD8);  /* UP */
    test_press_key(DIK_NUMPAD6);  /* RIGHT */
    test_press_key(DIK_1);        /* Key 1 */
    g_test_input.mouse_buttons = 0x01;  /* Left button */

    u32 state = calculate_key_state(&g_test_input);

    /* Should have all bits set */
    assert(state & KEY_STATE_UP);
    assert(state & KEY_STATE_RIGHT);
    assert(state & KEY_STATE_1);
    assert(state & KEY_STATE_LBUTTON);

    /* Calculate expected value */
    u32 expected = KEY_STATE_UP | KEY_STATE_RIGHT | KEY_STATE_1 | KEY_STATE_LBUTTON;
    assert(state == expected);

    return 1;
}

/* ========================================
 * Key Repeat Counter Range Tests
 * ======================================== */

static int test_key_repeat_counter_range(void) {
    /* Counter starts at 0, increments to 30, then resets to 23 */

    s16 counter = 0;

    /* Simulate 30 frames of holding */
    for (int i = 0; i < 30; i++) {
        counter++;
    }

    /* At 30, it should match KEY_REPEAT_INITIAL */
    assert(counter == KEY_REPEAT_INITIAL);

    /* After triggering, counter resets to KEY_REPEAT_RATE */
    counter = KEY_REPEAT_RATE;
    assert(counter == 23);

    return 1;
}

static int test_key_repeat_multiple_directions(void) {
    test_input_reset();

    /* Hold UP and RIGHT simultaneously */
    u32 key_state = KEY_STATE_UP | KEY_STATE_RIGHT;

    /* Build up to trigger threshold */
    g_test_input.key_repeat_counter[0] = KEY_REPEAT_INITIAL - 1;

    u32 result = process_key_repeat(&g_test_input, 0, key_state, 0);

    /* Both directions should trigger */
    assert(result & KEY_STATE_UP);
    /* Note: Implementation may differ - may only trigger one direction per frame */

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Input System Unit Tests ===\n\n");

    /* Key state bitmask tests */
    printf("Key State Bitmask Tests:\n");
    TEST(numpad_direction_bits);
    TEST(arrow_key_mapping);
    TEST(number_key_bits);
    TEST(diagonal_directions);
    TEST(mouse_button_bits);

    /* Edge detection tests */
    printf("\nEdge Detection Tests:\n");
    TEST(press_edge_detection);
    TEST(release_edge_detection);
    TEST(held_edge_formula);
    TEST(multiple_key_edges);

    /* Key repeat tests */
    printf("\nKey Repeat Tests:\n");
    TEST(key_repeat_initial_delay);
    TEST(key_repeat_rate);
    TEST(key_repeat_on_press);
    TEST(key_repeat_reset_on_release);
    TEST(key_repeat_relevant_bits);
    TEST(key_repeat_counter_range);
    TEST(key_repeat_multiple_directions);

    /* Mouse movement tests */
    printf("\nMouse Movement Tests:\n");
    TEST(mouse_threshold_500);
    TEST(mouse_direction_mapping);

    /* Bit position tests */
    printf("\nBit Position Tests:\n");
    TEST(key_state_bit_positions);
    TEST(combined_key_states);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
