/*
 * Stone Age Client - Input System
 * Reverse engineered from sa_9061.exe
 * FUN_004813f0 - DirectInput initialization
 * FUN_00480bd0 - Input state update
 * FUN_004809e0 - Key repeat handling
 * FUN_004814f0 - DirectInput cleanup
 */

#ifndef INPUT_H
#define INPUT_H

#include "types.h"

/* DirectInput version */
#define DIRECTINPUT_VERSION 0x0700

/* Key state flags - from FUN_00480bd0 bitmask
 * These are combined into key_state bitmask
 * Note: Binary uses numpad arrows for movement primarily
 */
#define KEY_STATE_UP          0x1000    /* Numpad 8 / Up arrow */
#define KEY_STATE_DOWN        0x2000    /* Numpad 2 / Down arrow */
#define KEY_STATE_LEFT        0x4000    /* Numpad 4 / Left arrow */
#define KEY_STATE_RIGHT       0x8000    /* Numpad 6 / Right arrow */
#define KEY_STATE_LBUTTON     0x0400    /* Left mouse button */
#define KEY_STATE_RBUTTON     0x0100    /* Right mouse button */

/* Number key flags - key_state lower bits */
#define KEY_STATE_1           0x00000001
#define KEY_STATE_2           0x00000002
#define KEY_STATE_3           0x00000004
#define KEY_STATE_4           0x00000008
#define KEY_STATE_5           0x00000010
#define KEY_STATE_6           0x00000020
#define KEY_STATE_7           0x00000040
#define KEY_STATE_8           0x00000080

/* Extended key state flags - key_state_ext bitmask */
#define KEY_STATE_F1          0x00001000
#define KEY_STATE_F2          0x00002000
#define KEY_STATE_F3          0x00004000
#define KEY_STATE_F4          0x00008000
#define KEY_STATE_F5          0x00010000
#define KEY_STATE_F6          0x00020000
#define KEY_STATE_F7          0x00040000
#define KEY_STATE_F8          0x00080000
#define KEY_STATE_F9          0x00100000
#define KEY_STATE_F10         0x00200000
#define KEY_STATE_F11         0x00400000
#define KEY_STATE_F12         0x00800000

/* Modifier key flags */
#define KEY_STATE_SHIFT       0x40000000
#define KEY_STATE_CTRL        0x20000000
#define KEY_STATE_ALT         0x10000000
#define KEY_STATE_ESCAPE      0x80000000

/* Convenience aliases for function keys */
#define KEY_F1                KEY_STATE_F1

/* ASCII key codes for Y/N confirmation */
#define KEY_Y                 0x59    /* 'Y' key */
#define KEY_N                 0x4E    /* 'N' key */
#define KEY_F2                KEY_STATE_F2
#define KEY_F3                KEY_STATE_F3
#define KEY_F4                KEY_STATE_F4
#define KEY_F5                KEY_STATE_F5
#define KEY_F6                KEY_STATE_F6
#define KEY_F7                KEY_STATE_F7
#define KEY_F8                KEY_STATE_F8
#define KEY_F9                KEY_STATE_F9
#define KEY_F10               KEY_STATE_F10
#define KEY_F11               KEY_STATE_F11
#define KEY_F12               KEY_STATE_F12

/* Mouse button aliases */
#define MOUSE_BUTTON_LEFT     KEY_STATE_LBUTTON
#define MOUSE_BUTTON_RIGHT    KEY_STATE_RBUTTON

/* Keyboard scancode constants - matching DirectInput */
#define DIK_ESCAPE          0x01
#define DIK_1               0x02
#define DIK_2               0x03
#define DIK_3               0x04
#define DIK_4               0x05
#define DIK_5               0x06
#define DIK_6               0x07
#define DIK_7               0x08
#define DIK_8               0x09
#define DIK_9               0x0A
#define DIK_0               0x0B
#define DIK_MINUS           0x0C
#define DIK_EQUALS          0x0D
#define DIK_BACK            0x0E
#define DIK_TAB             0x0F
#define DIK_Q               0x10
#define DIK_W               0x11
#define DIK_E               0x12
#define DIK_R               0x13
#define DIK_T               0x14
#define DIK_Y               0x15
#define DIK_U               0x16
#define DIK_I               0x17
#define DIK_O               0x18
#define DIK_P               0x19
#define DIK_LBRACKET        0x1A
#define DIK_RBRACKET        0x1B
#define DIK_RETURN          0x1C
#define DIK_LCONTROL        0x1D
#define DIK_A               0x1E
#define DIK_S               0x1F
#define DIK_D               0x20
#define DIK_F               0x21
#define DIK_G               0x22
#define DIK_H               0x23
#define DIK_J               0x24
#define DIK_K               0x25
#define DIK_L               0x26
#define DIK_SEMICOLON       0x27
#define DIK_APOSTROPHE      0x28
#define DIK_GRAVE           0x29
#define DIK_LSHIFT          0x2A
#define DIK_BACKSLASH       0x2B
#define DIK_Z               0x2C
#define DIK_X               0x2D
#define DIK_C               0x2E
#define DIK_V               0x2F
#define DIK_B               0x30
#define DIK_N               0x31
#define DIK_M               0x32
#define DIK_COMMA           0x33
#define DIK_PERIOD          0x34
#define DIK_SLASH           0x35
#define DIK_RSHIFT          0x36
#define DIK_MULTIPLY        0x37
#define DIK_LMENU           0x38
#define DIK_SPACE           0x39
#define DIK_CAPITAL         0x3A
#define DIK_F1              0x3B
#define DIK_F2              0x3C
#define DIK_F3              0x3D
#define DIK_F4              0x3E
#define DIK_F5              0x3F
#define DIK_F6              0x40
#define DIK_F7              0x41
#define DIK_F8              0x42
#define DIK_F9              0x43
#define DIK_F10             0x44
#define DIK_NUMPAD7         0x47
#define DIK_NUMPAD8         0x48
#define DIK_NUMPAD9         0x49
#define DIK_SUBTRACT        0x4A
#define DIK_NUMPAD4         0x4B
#define DIK_NUMPAD5         0x4C
#define DIK_NUMPAD6         0x4D
#define DIK_ADD             0x4E
#define DIK_NUMPAD1         0x4F
#define DIK_NUMPAD2         0x50
#define DIK_NUMPAD3         0x51
#define DIK_NUMPAD0         0x52
#define DIK_DECIMAL         0x53
#define DIK_F11             0x57
#define DIK_F12             0x58
#define DIK_NUMPADENTER     0x9C
#define DIK_RCONTROL        0x9D
#define DIK_DIVIDE          0xB5
#define DIK_RMENU           0xB8
#define DIK_HOME            0xC7
#define DIK_UP              0xC8
#define DIK_PRIOR           0xC9
#define DIK_LEFT            0xCB
#define DIK_RIGHT           0xCD
#define DIK_END             0xCF
#define DIK_DOWN            0xD0
#define DIK_NEXT            0xD1
#define DIK_INSERT          0xD2
#define DIK_DELETE          0xD3

/* Mouse button constants */
#define MOUSE_LBUTTON       0x01
#define MOUSE_RBUTTON       0x02
#define MOUSE_MBUTTON       0x04

/* Mouse movement threshold from FUN_00480bd0 */
#define MOUSE_THRESHOLD     500

/* Key repeat constants from FUN_004809e0 */
#define KEY_REPEAT_INITIAL  0x1e    /* 30 frames before repeat starts */
#define KEY_REPEAT_RATE     0x17    /* 23 frames between repeats */

/* Convenience key macros for common operations */
#define KEY_BACKSPACE       DIK_BACK
#define KEY_TAB             DIK_TAB
#define KEY_RETURN          DIK_RETURN
#define KEY_ENTER           DIK_RETURN
#define KEY_LEFT            DIK_LEFT
#define KEY_RIGHT           DIK_RIGHT
#define KEY_UP              DIK_UP
#define KEY_DOWN            DIK_DOWN
#define KEY_HOME            DIK_HOME
#define KEY_END             DIK_END
#define KEY_DELETE          DIK_DELETE
#define KEY_INSERT          DIK_INSERT
#define KEY_ESCAPE          DIK_ESCAPE
#define KEY_SPACE           DIK_SPACE

/* Input state structure - matches DAT_04ebe320 region layout */
typedef struct {
    /* Keyboard state - 256 bytes (DAT_04ebe320) */
    u8 keyboard_state[256];

    /* Current key state bitmask (DAT_04ebe488) */
    u32 key_state;

    /* Previous key state bitmask (DAT_04ebe478) */
    u32 key_state_prev;

    /* Key press edge - just pressed (DAT_04ebe490) */
    u32 key_press_edge;

    /* Key release edge (DAT_04ebe498) */
    u32 key_release_edge;

    /* Key held edge */
    u32 key_held_edge;

    /* Extended key press edge (DAT_04ebe494) */
    u32 key_ext_press_edge;

    /* Extended key release edge (DAT_04ebe49c) */
    u32 key_ext_release_edge;

    /* Extended key state (DAT_04ebe48c) */
    u32 key_state_ext;

    /* Extended key state previous (DAT_04ebe47c) */
    u32 key_state_ext_prev;

    /* Mouse state */
    s32 mouse_x;
    s32 mouse_y;
    s32 mouse_rel_x;
    s32 mouse_rel_y;
    u8 mouse_buttons;
    u8 mouse_buttons_prev;

    /* Cursor position for rendering */
    s32 cursor_x;
    s32 cursor_y;

    /* Timestamps */
    u32 last_update_time;

    /* DirectInput device flags */
    u8 keyboard_active;    /* DAT_04ebe4a9 */
    u8 mouse_active;       /* DAT_04ebe4a8 */
    u8 reserved[2];

} InputState;

/* Global input state */
extern InputState g_input;

/* Input initialization - FUN_004813f0 */
int input_init(HINSTANCE hInstance, HWND hWnd);

/* Input cleanup - FUN_004814f0 */
void input_shutdown(void);

/* Update input state - FUN_00480bd0 */
void input_update(void);

/* Acquire/unacquire devices */
void input_acquire(void);
void input_unacquire(void);

/* Key state queries using bitmask flags */
int input_is_key_down(u32 key_flag);
int input_is_key_up(u32 key_flag);
int input_key_pressed(u32 key_flag);
int input_key_released(u32 key_flag);

/* Convenience key down check for game input */
#define input_key_down(flag) input_is_key_down(flag)

/* Additional key definitions for WASD movement */
#define KEY_W               DIK_W
#define KEY_A               DIK_A
#define KEY_S               DIK_S
#define KEY_D               DIK_D

/* Key repeat - from FUN_004809e0 */
u32 input_get_key_repeat(int context);

/* Direct keyboard scancode queries */
int input_is_key_down_direct(u8 scan_code);
int input_key_pressed_direct(u8 scan_code);

/* Mouse state queries */
int input_is_mouse_button_down(u8 button);
int input_mouse_button_pressed(u8 button);
int input_mouse_button_released(u8 button);
void input_get_mouse_position(s32* x, s32* y);
void input_get_mouse_relative(s32* dx, s32* dy);
void input_set_mouse_position(s32 x, s32 y);

/* Mouse click detection in rectangle */
int input_mouse_clicked(int x1, int y1, int x2, int y2);

/* Mouse button pressed check */
#define input_mouse_pressed(button) input_mouse_button_pressed(button)

/* Mouse position getters */
#define input_get_mouse_x() (g_input.cursor_x)
#define input_get_mouse_y() (g_input.cursor_y)

/* Field action codes - for input_get_main_action() */
#define FIELD_ACTION_NONE       0
#define FIELD_ACTION_WALK       1
#define FIELD_ACTION_RUN        2
#define FIELD_ACTION_BATTLE     3
#define FIELD_ACTION_MENU       4
#define FIELD_ACTION_SKILL      5
#define FIELD_ACTION_WAIT       6
#define FIELD_ACTION_ATTACK     7
#define FIELD_ACTION_BATTLE_MENU 8
#define FIELD_ACTION_ITEM       9
#define FIELD_ACTION_ESCAPE     10
#define FIELD_ACTION_SPECIAL    11
#define FIELD_ACTION_DEFEND     12
#define FIELD_ACTION_PET        13

/* Character input buffer */
int input_get_char(void);
void input_clear_char_buffer(void);

/* Key state access for field_update */
u32 input_get_key_state(void);
int input_get_main_action(void);
int input_get_sub_action(void);

/* Cursor control */
void input_show_cursor(int show);

/* ========================================
 * IME System - FUN_00491780, FUN_00491c40
 * ======================================== */

typedef struct {
    HIMC context;
    HIMC old_context;
    HWND hwnd;
    DWORD param;
    HANDLE heap;
    char* buffer_main;
    char* buffer_aux;
    char* buffer_comp;
    char* buffer_cand;
    char* buffer_desc;
    char* buffer_reading;
    HKL keyboard_layout;
    int is_ime;
    int initialized;
} IMEContext;

extern IMEContext g_ime;

int ime_init(HWND hwnd, DWORD param);
void ime_shutdown(void);
void ime_update_layout(void);
int ime_is_active(void);
const char* ime_get_description(void);
void ime_set_conversion_mode(int mode);
int ime_get_conversion_mode(void);

#endif /* INPUT_H */
