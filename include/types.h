#ifndef TYPES_H
#define TYPES_H

#include <windows.h>
#include <stdint.h>

/* Basic type definitions matching the original binary */
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;

/* Global state structure - matches DAT_0455xxxx region */
typedef struct {
    HINSTANCE hInstance;        /* DAT_045600f8 */
    HWND hWnd;                  /* Main window handle */
    char* cmdline;              /* DAT_0455fb40 */
    int nCmdShow;               /* DAT_045600f0 */
    DWORD process_id;           /* DAT_0455f8f8 */
    DWORD parent_process_id;    /* For anti-tamper check */
    char parent_name[MAX_PATH]; /* Parent process name */
    HANDLE process_snapshot;    /* DAT_0456021c */
    /* Windows version info - FUN_004936c3 */
    u8 win_ver_major;
    u8 win_ver_minor;
    u16 win_ver_build;
    /* Screen dimensions */
    int screen_width;
    int screen_height;
} GlobalState;

/* External global variables */
extern GlobalState g_state;
/* g_config is declared in gameconfig.h */

/* Buff flags - used in battle and item systems */
#define BUFF_ATTACK_UP      0x0001
#define BUFF_DEFENSE_UP     0x0002
#define BUFF_SPEED_UP       0x0004
#define BUFF_ATTACK_DOWN    0x0010
#define BUFF_DEFENSE_DOWN   0x0020
#define BUFF_SPEED_DOWN     0x0040
#define BUFF_POISON         0x0100
#define BUFF_STUN           0x0200
#define BUFF_SLEEP          0x0400
#define BUFF_CONFUSE        0x0800

#endif /* TYPES_H */
