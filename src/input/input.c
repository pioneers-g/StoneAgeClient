/*
 * Stone Age Client - Input System Implementation
 * Reverse engineered from sa_9061.exe
 * FUN_004813f0 - DirectInput initialization
 * FUN_00480bd0 - Input state update (key bitmask calculation)
 * FUN_004809e0 - Key repeat handling
 * FUN_004814f0 - DirectInput cleanup
 */

#include <windows.h>
#include <dinput.h>
#include <imm.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "input.h"
#include "logger.h"

/* Key repeat constants from FUN_004809e0 */
#define KEY_REPEAT_INITIAL  0x1e    /* 30 frames before repeat starts */
#define KEY_REPEAT_RATE     0x17    /* 23 frames between repeats */

/* Global input state - matches DAT_04ebe320 region */
InputState g_input = {0};

/* Key repeat counters - DAT_04ebe420 (array of 16-bit values) */
static s16 g_key_repeat_counter[2] = {0, 0};

/* Key repeat results - DAT_04ebe4a0, DAT_04ebe4a4 */
static u32 g_key_repeat_result[2] = {0, 0};

/* DirectInput objects - matches DAT_04cb6d24 region */
static LPDIRECTINPUT7 g_di_ctx = NULL;            /* DAT_04cb6d24 */
static LPDIRECTINPUTDEVICE7 g_di_keyboard = NULL; /* DAT_04ebe424 */
static LPDIRECTINPUTDEVICE7 g_di_mouse = NULL;    /* DAT_04ebe480 */

/* Function pointer type for DirectInputCreateA */
typedef HRESULT (WINAPI* DirectInputCreateAFunc)(HINSTANCE, DWORD, LPDIRECTINPUTA*, LPUNKNOWN);

/* Module handle */
static HMODULE g_dinput_module = NULL;

/*
 * Initialize input system - FUN_004813f0 pattern
 *
 * Binary flow:
 * 1. Check GetAsyncKeyState(VK_CONTROL) - skip if Ctrl pressed
 * 2. DirectInputCreateA(hInstance, 0x700, &g_di_ctx, NULL)
 * 3. CreateDevice(GUID_SysKeyboard, &g_di_keyboard, NULL)
 * 4. SetDataFormat(&c_dfDIKeyboard)
 * 5. SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)
 * 6. Acquire keyboard
 * 7. Create mouse device
 */
int input_init(HINSTANCE hInstance, HWND hWnd) {
    HRESULT hr;
    DirectInputCreateAFunc pDirectInputCreateA;
    DWORD coop_flags;

    LOG_INFO("Initializing input system...");

    /* Check for Ctrl key - from FUN_004813f0 */
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
        LOG_DEBUG("Ctrl key pressed, skipping DirectInput init");
        return 0;
    }

    /* Load dinput.dll */
    g_dinput_module = LoadLibraryA("dinput.dll");
    if (!g_dinput_module) {
        LOG_WARN("Failed to load dinput.dll");
        return 0;
    }

    pDirectInputCreateA = (DirectInputCreateAFunc)GetProcAddress(g_dinput_module, "DirectInputCreateA");
    if (!pDirectInputCreateA) {
        LOG_WARN("Failed to get DirectInputCreateA");
        FreeLibrary(g_dinput_module);
        g_dinput_module = NULL;
        return 0;
    }

    /* Create DirectInput context - DAT_04cb6d24
     * Binary uses version 0x700 (DirectInput 7.0)
     */
    hr = pDirectInputCreateA(hInstance, 0x700, &g_di_ctx, NULL);
    if (FAILED(hr)) {
        LOG_WARN("DirectInputCreateA failed: 0x%08X", hr);
        FreeLibrary(g_dinput_module);
        g_dinput_module = NULL;
        return 0;
    }

    /* Create keyboard device - DAT_04ebe424 */
    hr = IDirectInput_CreateDevice(g_di_ctx, &GUID_SysKeyboard, &g_di_keyboard, NULL);
    if (FAILED(hr)) {
        LOG_WARN("CreateDevice (keyboard) failed: 0x%08X", hr);
        IDirectInput_Release(g_di_ctx);
        FreeLibrary(g_dinput_module);
        return 0;
    }

    /* Set keyboard data format */
    hr = IDirectInputDevice_SetDataFormat(g_di_keyboard, &c_dfDIKeyboard);
    if (FAILED(hr)) {
        LOG_WARN("SetDataFormat (keyboard) failed: 0x%08X", hr);
        IDirectInputDevice_Release(g_di_keyboard);
        IDirectInput_Release(g_di_ctx);
        FreeLibrary(g_dinput_module);
        return 0;
    }

    /* Set cooperative level - 6 = FOREGROUND | NONEXCLUSIVE */
    coop_flags = DISCL_FOREGROUND | DISCL_NONEXCLUSIVE;
    hr = IDirectInputDevice_SetCooperativeLevel(g_di_keyboard, hWnd, coop_flags);
    if (FAILED(hr)) {
        LOG_DEBUG("SetCooperativeLevel (keyboard) failed: 0x%08X", hr);
    }

    /* Acquire keyboard device */
    IDirectInputDevice_Acquire(g_di_keyboard);
    g_input.keyboard_active = 1;  /* DAT_04ebe4a9 */

    /* Create mouse device - DAT_04ebe480 */
    hr = IDirectInput_CreateDevice(g_di_ctx, &GUID_SysMouse, &g_di_mouse, NULL);
    if (SUCCEEDED(hr)) {
        hr = IDirectInputDevice_SetDataFormat(g_di_mouse, &c_dfDIMouse);
        if (SUCCEEDED(hr)) {
            hr = IDirectInputDevice_SetCooperativeLevel(g_di_mouse, hWnd, coop_flags);
            IDirectInputDevice_Acquire(g_di_mouse);
            g_input.mouse_active = 1;  /* DAT_04ebe4a8 */
        }
    }

    LOG_INFO("Input system initialized (keyboard=%d, mouse=%d)",
             g_input.keyboard_active, g_input.mouse_active);
    return 1;
}

/*
 * Shutdown input system - FUN_004814f0 pattern
 */
void input_shutdown(void) {
    LOG_INFO("Shutting down input system...");

    /* Release keyboard - DAT_04ebe4a9 check */
    if (g_input.keyboard_active) {
        if (g_di_keyboard) {
            IDirectInputDevice_Unacquire(g_di_keyboard);
            IDirectInputDevice_Release(g_di_keyboard);
            g_di_keyboard = NULL;
        }
    }

    /* Release mouse - DAT_04ebe4a8 check */
    if (g_input.mouse_active) {
        if (g_di_mouse) {
            IDirectInputDevice_Unacquire(g_di_mouse);
            IDirectInputDevice_Release(g_di_mouse);
            g_di_mouse = NULL;
        }
    }

    if (g_di_ctx) {
        IDirectInput_Release(g_di_ctx);
        g_di_ctx = NULL;
    }

    if (g_dinput_module) {
        FreeLibrary(g_dinput_module);
        g_dinput_module = NULL;
    }

    memset(&g_input, 0, sizeof(InputState));
    LOG_INFO("Input system shutdown");
}

/*
 * Process key repeat for a context - FUN_004809e0
 *
 * Binary logic:
 * - If no relevant keys pressed (mask 0xf5f0), reset counter to 0
 * - If key just pressed (edge), set bit in result
 * - If key held and counter reaches 30, reset to 23 and set bit (repeat)
 * - Mask 0xf5f0 covers: arrows (0x1000-0x8000), mouse (0x100, 0x400), bits 0x10-0x80
 */
static u32 input_process_key_repeat(int context, u32 key_state, u32 key_press_edge) {
    u32 result = 0;
    u32 relevant_mask;
    s16 counter;
    u32 check_bits;
    int i;

    /* Bits that trigger repeat: 0x1000, 0x2000, 0x4000, 0x8000, 0x400, 0x100, 0x10, 0x20, 0x40, 0x80 */
    relevant_mask = 0x1000 | 0x2000 | 0x4000 | 0x8000 | 0x0400 | 0x0100 | 0x0010 | 0x0020 | 0x0040 | 0x0080;

    /* If no relevant keys pressed, reset counter */
    if ((key_state & relevant_mask) == 0) {
        g_key_repeat_counter[context] = 0;
        return 0;
    }

    counter = g_key_repeat_counter[context];

    /* Check each relevant bit */
    check_bits = relevant_mask;
    while (check_bits) {
        u32 bit = check_bits & -check_bits;  /* Extract lowest bit */
        check_bits ^= bit;  /* Remove lowest bit */

        if (key_press_edge & bit) {
            /* Key just pressed - set result bit */
            result |= bit;
        } else if (key_state & bit) {
            /* Key held - check counter */
            counter++;
            if (counter == KEY_REPEAT_INITIAL) {
                counter = KEY_REPEAT_RATE;
                result |= bit;
            }
        }
    }

    g_key_repeat_counter[context] = counter;
    return result;
}

/*
 * Update input state - FUN_00480bd0 exact pattern
 *
 * Binary calculates key_state bitmask from keyboard_state array:
 * - Numpad arrows: UP=0x1000, DOWN=0x2000, LEFT=0x4000, RIGHT=0x8000
 * - Mouse movement over threshold: same bits
 * - Number keys 1-6: bits 0-5
 * - F-keys in extended state
 */
void input_update(void) {
    HRESULT hr;
    DIMOUSESTATE mouse_state;
    POINT pt;
    u32 key_state = 0;
    u32 key_state_ext = 0;
    u8* kb;

    if (!g_input.keyboard_active) return;

    /* Store previous state - DAT_04ebe478, DAT_04ebe47c */
    g_input.key_state_prev = g_input.key_state;
    g_input.key_state_ext_prev = g_input.key_state_ext;
    g_input.mouse_buttons_prev = g_input.mouse_buttons;

    /* Get keyboard state - 256 bytes (0x100) from DAT_04ebe320 */
    memset(g_input.keyboard_state, 0, 256);
    hr = IDirectInputDevice_GetDeviceState(g_di_keyboard, 256, g_input.keyboard_state);
    if (FAILED(hr)) {
        if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
            hr = IDirectInputDevice_Acquire(g_di_keyboard);
            if (SUCCEEDED(hr)) {
                hr = IDirectInputDevice_GetDeviceState(g_di_keyboard, 256, g_input.keyboard_state);
            }
        }
    }

    kb = g_input.keyboard_state;

    /* Get mouse state if active */
    if (g_input.mouse_active && g_di_mouse) {
        memset(&mouse_state, 0, sizeof(mouse_state));
        hr = IDirectInputDevice_GetDeviceState(g_di_mouse, sizeof(mouse_state), &mouse_state);
        if (FAILED(hr)) {
            hr = IDirectInputDevice_Acquire(g_di_mouse);
            if (SUCCEEDED(hr)) {
                hr = IDirectInputDevice_GetDeviceState(g_di_mouse, sizeof(mouse_state), &mouse_state);
            }
        }

        if (SUCCEEDED(hr)) {
            /* Mouse relative movement */
            g_input.mouse_rel_x = mouse_state.lX;
            g_input.mouse_rel_y = mouse_state.lY;

            /* Mouse movement threshold check from FUN_00480bd0:
             * if (500 < lX) key_state |= 0x8000 (RIGHT)
             * if (lX < -500) key_state |= 0x4000 (LEFT)
             * if (500 < lY) key_state |= 0x2000 (DOWN)
             * if (lY < -500) key_state |= 0x1000 (UP)
             */
            if (mouse_state.lX > MOUSE_THRESHOLD) key_state |= KEY_STATE_RIGHT;
            if (mouse_state.lX < -MOUSE_THRESHOLD) key_state |= KEY_STATE_LEFT;
            if (mouse_state.lY > MOUSE_THRESHOLD) key_state |= KEY_STATE_DOWN;
            if (mouse_state.lY < -MOUSE_THRESHOLD) key_state |= KEY_STATE_UP;

            /* Mouse button flags - from binary:
             * (rgbButtons[0] & 0x80) -> 0x400 (LBUTTON)
             * (rgbButtons[1] & 0x80) -> 0x100 (RBUTTON)
             */
            if (mouse_state.rgbButtons[0] & 0x80) key_state |= KEY_STATE_LBUTTON;
            if (mouse_state.rgbButtons[1] & 0x80) key_state |= KEY_STATE_RBUTTON;

            g_input.mouse_buttons = 0;
            if (mouse_state.rgbButtons[0] & 0x80) g_input.mouse_buttons |= MOUSE_LBUTTON;
            if (mouse_state.rgbButtons[1] & 0x80) g_input.mouse_buttons |= MOUSE_RBUTTON;
            if (mouse_state.rgbButtons[2] & 0x80) g_input.mouse_buttons |= MOUSE_MBUTTON;
        }
    }

    /* Arrow keys - from FUN_00480bd0 exact offsets:
     * DAT_04ebe3eb (DIK_NUMPAD4/LEFT at 0xCB) -> 0x4000
     * DAT_04ebe3e8 (DIK_NUMPAD8/UP at 0xC8) -> 0x1000
     * DAT_04ebe3ed (DIK_NUMPAD6/RIGHT at 0xCD) -> 0x8000
     * DAT_04ebe3f0 (DIK_NUMPAD2/DOWN at 0xD0) -> 0x2000
     */
    if (kb[DIK_NUMPAD4] & 0x80) key_state |= KEY_STATE_LEFT;
    if (kb[DIK_NUMPAD8] & 0x80) key_state |= KEY_STATE_UP;
    if (kb[DIK_NUMPAD6] & 0x80) key_state |= KEY_STATE_RIGHT;
    if (kb[DIK_NUMPAD2] & 0x80) key_state |= KEY_STATE_DOWN;

    /* Regular arrow keys - map to same bits */
    if (kb[DIK_LEFT] & 0x80) key_state |= KEY_STATE_LEFT;
    if (kb[DIK_UP] & 0x80) key_state |= KEY_STATE_UP;
    if (kb[DIK_RIGHT] & 0x80) key_state |= KEY_STATE_RIGHT;
    if (kb[DIK_DOWN] & 0x80) key_state |= KEY_STATE_DOWN;

    /* Escape key - DAT_04ebe320._1_1_ (offset 0x01) -> 0x80000000 */
    if (kb[DIK_ESCAPE] & 0x80) key_state_ext |= KEY_STATE_ESCAPE;

    /* Number keys 1-6 - from FUN_00480bd0:
     * DAT_04ebe3e9 (DIK_1 at 0x02) -> bit 0
     * DAT_04ebe3f1 (DIK_2 at 0x03) -> bit 1
     * DAT_04ebe3e7 (DIK_3 at 0x04) -> bit 2
     * DAT_04ebe3ef (DIK_4 at 0x05) -> bit 3
     * DAT_04ebe3f2 (DIK_5 at 0x06) -> bit 4
     * DAT_04ebe3f3 (DIK_6 at 0x07) -> bit 5
     */
    if (kb[DIK_1] & 0x80) key_state |= KEY_STATE_1;
    if (kb[DIK_2] & 0x80) key_state |= KEY_STATE_2;
    if (kb[DIK_3] & 0x80) key_state |= KEY_STATE_3;
    if (kb[DIK_4] & 0x80) key_state |= KEY_STATE_4;
    if (kb[DIK_5] & 0x80) key_state |= KEY_STATE_5;
    if (kb[DIK_6] & 0x80) key_state |= KEY_STATE_6;

    /* Function keys F1-F12 - in key_state_ext
     * From FUN_00480bd0: F1=0x1000, F2=0x2000, etc.
     */
    if (kb[DIK_F1] & 0x80) key_state_ext |= KEY_STATE_F1;
    if (kb[DIK_F2] & 0x80) key_state_ext |= KEY_STATE_F2;
    if (kb[DIK_F3] & 0x80) key_state_ext |= KEY_STATE_F3;
    if (kb[DIK_F4] & 0x80) key_state_ext |= KEY_STATE_F4;
    if (kb[DIK_F5] & 0x80) key_state_ext |= KEY_STATE_F5;
    if (kb[DIK_F6] & 0x80) key_state_ext |= KEY_STATE_F6;
    if (kb[DIK_F7] & 0x80) key_state_ext |= KEY_STATE_F7;
    if (kb[DIK_F8] & 0x80) key_state_ext |= KEY_STATE_F8;
    if (kb[DIK_F9] & 0x80) key_state_ext |= KEY_STATE_F9;
    if (kb[DIK_F10] & 0x80) key_state_ext |= KEY_STATE_F10;
    if (kb[DIK_F11] & 0x80) key_state_ext |= KEY_STATE_F11;
    if (kb[DIK_F12] & 0x80) key_state_ext |= KEY_STATE_F12;

    /* Modifier keys */
    if (kb[DIK_LSHIFT] & 0x80) key_state_ext |= KEY_STATE_SHIFT;
    if (kb[DIK_RSHIFT] & 0x80) key_state_ext |= KEY_STATE_SHIFT;
    if (kb[DIK_LCONTROL] & 0x80) key_state_ext |= KEY_STATE_CTRL;
    if (kb[DIK_RCONTROL] & 0x80) key_state_ext |= KEY_STATE_CTRL;
    if (kb[DIK_LMENU] & 0x80) key_state_ext |= KEY_STATE_ALT;
    if (kb[DIK_RMENU] & 0x80) key_state_ext |= KEY_STATE_ALT;

    /* Store current state - DAT_04ebe488 */
    g_input.key_state = key_state;
    g_input.key_state_ext = key_state_ext;

    /* Calculate edge detection - from FUN_00480bd0 exact pattern:
     * DAT_04ebe490 = ~DAT_04ebe478 & DAT_04ebe488 (just pressed)
     * DAT_04ebe498 = (uVar2 ^ ~DAT_04ebe488) & ~DAT_04ebe488 (released)
     * DAT_04ebe494 = ~DAT_04ebe47c & uVar3 (ext just pressed)
     * DAT_04ebe49c = (~DAT_04ebe47c ^ ~uVar3) & ~uVar3 (ext released)
     */
    g_input.key_press_edge = (~g_input.key_state_prev) & key_state;
    g_input.key_release_edge = (~key_state) & g_input.key_state_prev;
    g_input.key_held_edge = (~g_input.key_state_prev ^ ~key_state) & ~key_state;

    /* Extended key edge detection */
    g_input.key_ext_press_edge = (~g_input.key_state_ext_prev) & key_state_ext;
    g_input.key_ext_release_edge = (~key_state_ext) & g_input.key_state_ext_prev;

    /* Process key repeat for two contexts - DAT_04ebe4a0, DAT_04ebe4a4 */
    g_key_repeat_result[0] = input_process_key_repeat(0, key_state, g_input.key_press_edge);
    g_key_repeat_result[1] = input_process_key_repeat(1, key_state, g_input.key_press_edge);

    /* Get mouse position via Windows API for absolute coords */
    GetCursorPos(&pt);
    ScreenToClient(GetActiveWindow(), &pt);
    g_input.cursor_x = pt.x;
    g_input.cursor_y = pt.y;

    /* Update timestamp */
    g_input.last_update_time = timeGetTime();
}

/*
 * Get key repeat result for context
 */
u32 input_get_key_repeat(int context) {
    if (context >= 0 && context < 2) {
        return g_key_repeat_result[context];
    }
    return 0;
}

/*
 * Check if key flag is set in key_state
 */
int input_is_key_down(u32 key_flag) {
    return (g_input.key_state & key_flag) != 0;
}

/*
 * Check if key flag is not set
 */
int input_is_key_up(u32 key_flag) {
    return (g_input.key_state & key_flag) == 0;
}

/*
 * Check if key was just pressed (edge detection)
 */
int input_key_pressed(u32 key_flag) {
    return (g_input.key_press_edge & key_flag) != 0;
}

/*
 * Check if key was just released
 */
int input_key_released(u32 key_flag) {
    return (g_input.key_release_edge & key_flag) != 0;
}

/*
 * Check if key is down using direct scan code
 */
int input_is_key_down_direct(u8 scan_code) {
    if (scan_code >= 256) return 0;
    return (g_input.keyboard_state[scan_code] & 0x80) != 0;
}

/*
 * Check if key was just pressed using direct scan code
 */
int input_key_pressed_direct(u8 scan_code) {
    static u8 prev_state[256] = {0};
    int result;

    if (scan_code >= 256) return 0;

    result = ((g_input.keyboard_state[scan_code] & 0x80) != 0) &&
             ((prev_state[scan_code] & 0x80) == 0);

    prev_state[scan_code] = g_input.keyboard_state[scan_code];
    return result;
}

/*
 * Check if mouse was clicked within a rectangle
 * Returns 1 if left mouse button was just pressed within the rectangle, 0 otherwise
 */
int input_mouse_clicked(int x1, int y1, int x2, int y2) {
    s32 mx, my;

    /* Check if left mouse button was just pressed */
    if (!input_mouse_button_pressed(MOUSE_LBUTTON)) {
        return 0;
    }

    /* Get mouse position */
    input_get_mouse_position(&mx, &my);

    /* Check if within rectangle */
    return (mx >= x1 && mx <= x2 && my >= y1 && my <= y2);
}

/*
 * Check if mouse button is down
 */
int input_is_mouse_button_down(u8 button) {
    return (g_input.mouse_buttons & button) != 0;
}

/*
 * Check if mouse button was just pressed
 */
int input_mouse_button_pressed(u8 button) {
    return ((g_input.mouse_buttons & button) != 0) &&
           ((g_input.mouse_buttons_prev & button) == 0);
}

/*
 * Check if mouse button was just released
 */
int input_mouse_button_released(u8 button) {
    return ((g_input.mouse_buttons & button) == 0) &&
           ((g_input.mouse_buttons_prev & button) != 0);
}

/*
 * Get mouse position
 */
void input_get_mouse_position(s32* x, s32* y) {
    if (x) *x = g_input.cursor_x;
    if (y) *y = g_input.cursor_y;
}

/*
 * Get mouse relative movement
 */
void input_get_mouse_relative(s32* dx, s32* dy) {
    if (dx) *dx = g_input.mouse_rel_x;
    if (dy) *dy = g_input.mouse_rel_y;
}

/*
 * Set mouse position
 */
void input_set_mouse_position(s32 x, s32 y) {
    POINT pt;
    pt.x = x;
    pt.y = y;
    ClientToScreen(GetActiveWindow(), &pt);
    SetCursorPos(pt.x, pt.y);
    g_input.cursor_x = x;
    g_input.cursor_y = y;
}

/*
 * Show/hide cursor
 */
void input_show_cursor(int show) {
    ShowCursor(show ? TRUE : FALSE);
}

/*
 * Acquire input devices
 */
void input_acquire(void) {
    if (g_di_keyboard) {
        IDirectInputDevice_Acquire(g_di_keyboard);
    }
    if (g_di_mouse) {
        IDirectInputDevice_Acquire(g_di_mouse);
    }
}

/*
 * Unacquire input devices
 */
void input_unacquire(void) {
    if (g_di_keyboard) {
        IDirectInputDevice_Unacquire(g_di_keyboard);
    }
    if (g_di_mouse) {
        IDirectInputDevice_Unacquire(g_di_mouse);
    }
}

/*
 * Get character from keyboard (for text input)
 */
int input_get_char(void) {
    u32 i;
    static const u8 key_to_char[256] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    /* Check for newly pressed keys */
    for (i = 0; i < 256; i++) {
        if ((g_input.keyboard_state[i] & 0x80) && key_to_char[i]) {
            if (g_input.key_press_edge) {
                return key_to_char[i];
            }
        }
    }

    return 0;
}

/*
 * Clear character buffer
 */
void input_clear_char_buffer(void) {
    /* Nothing to do for now */
}

/*
 * Get current key state bitmask - for field_update.c
 */
u32 input_get_key_state(void) {
    return g_input.key_state;
}

/*
 * Get main action from current input state
 * Returns action code for field_update processing
 */
int input_get_main_action(void) {
    u32 key_state = g_input.key_state;
    u32 key_press = g_input.key_press_edge;

    /* Movement keys */
    if (key_state & (KEY_STATE_UP | KEY_STATE_DOWN | KEY_STATE_LEFT | KEY_STATE_RIGHT)) {
        /* Running if shift held */
        if (key_state & KEY_STATE_SHIFT) {
            return FIELD_ACTION_RUN;
        }
        return FIELD_ACTION_WALK;
    }

    /* Menu keys */
    if (key_press & KEY_STATE_ESCAPE) {
        return FIELD_ACTION_MENU;
    }

    /* Number keys for skills/items */
    if (key_press & KEY_STATE_1) return FIELD_ACTION_SKILL;
    if (key_press & KEY_STATE_2) return FIELD_ACTION_ITEM;
    if (key_press & KEY_STATE_3) return FIELD_ACTION_PET;
    if (key_press & KEY_STATE_4) return FIELD_ACTION_SPECIAL;
    if (key_press & KEY_STATE_5) return FIELD_ACTION_DEFEND;
    if (key_press & KEY_STATE_6) return FIELD_ACTION_ATTACK;
    if (key_press & KEY_STATE_7) return FIELD_ACTION_ESCAPE;
    if (key_press & KEY_STATE_8) return FIELD_ACTION_WAIT;

    /* No action */
    return FIELD_ACTION_NONE;
}

/*
 * Get sub action from current input state
 */
int input_get_sub_action(void) {
    u32 key_state = g_input.key_state;

    /* Check for modifier combinations */
    if (key_state & KEY_STATE_CTRL) {
        return FIELD_ACTION_SPECIAL;
    }
    if (key_state & KEY_STATE_ALT) {
        return FIELD_ACTION_MENU;
    }

    return FIELD_ACTION_NONE;
}

