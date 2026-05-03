/*
 * Stone Age Client - Cursor System Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "cursor.h"
#include "logger.h"

/* Global cursor context */
CursorContext g_cursor = {0};

/*
 * Initialize cursor system - FUN_00488ca0 pattern
 * Parameters: hWnd - window handle, hInstance - module instance
 */
int cursor_init(HWND hWnd, HINSTANCE hInstance) {
    memset(&g_cursor, 0, sizeof(CursorContext));

    g_cursor.visible = 1;
    g_cursor.current_type = CURSOR_ARROW;

    /* Load default cursors */
    cursor_load_defaults();

    /* Load game cursors from resources - FUN_00488ca0 */
    /* Resource IDs 0x68 and 0x70 are used in FUN_0043f1f0 for cursor/icon */

    LOG_INFO("Cursor system initialized");
    return 1;
}

/*
 * Shutdown cursor system
 */
void cursor_shutdown(void) {
    int i;

    /* Destroy custom cursors */
    for (i = 0; i < MAX_CURSORS; i++) {
        if (g_cursor.cursors[i].handle && !g_cursor.cursors[i].is_system) {
            DestroyCursor(g_cursor.cursors[i].handle);
        }
    }

    memset(&g_cursor, 0, sizeof(CursorContext));
    LOG_INFO("Cursor system shutdown");
}

/*
 * Load default cursors
 */
void cursor_load_defaults(void) {
    /* System cursors */
    g_cursor.cursors[CURSOR_ARROW].handle = LoadCursorA(NULL, IDC_ARROW);
    g_cursor.cursors[CURSOR_ARROW].is_system = 1;
    g_cursor.cursors[CURSOR_ARROW].type = CURSOR_ARROW;
    strcpy(g_cursor.cursors[CURSOR_ARROW].name, "arrow");

    g_cursor.cursors[CURSOR_IBEAM].handle = LoadCursorA(NULL, IDC_IBEAM);
    g_cursor.cursors[CURSOR_IBEAM].is_system = 1;
    g_cursor.cursors[CURSOR_IBEAM].type = CURSOR_IBEAM;
    strcpy(g_cursor.cursors[CURSOR_IBEAM].name, "ibeam");

    g_cursor.cursors[CURSOR_WAIT].handle = LoadCursorA(NULL, IDC_WAIT);
    g_cursor.cursors[CURSOR_WAIT].is_system = 1;
    g_cursor.cursors[CURSOR_WAIT].type = CURSOR_WAIT;
    strcpy(g_cursor.cursors[CURSOR_WAIT].name, "wait");

    g_cursor.cursors[CURSOR_CROSS].handle = LoadCursorA(NULL, IDC_CROSS);
    g_cursor.cursors[CURSOR_CROSS].is_system = 1;
    g_cursor.cursors[CURSOR_CROSS].type = CURSOR_CROSS;
    strcpy(g_cursor.cursors[CURSOR_CROSS].name, "cross");

    g_cursor.cursors[CURSOR_UPARROW].handle = LoadCursorA(NULL, IDC_UPARROW);
    g_cursor.cursors[CURSOR_UPARROW].is_system = 1;
    g_cursor.cursors[CURSOR_UPARROW].type = CURSOR_UPARROW;
    strcpy(g_cursor.cursors[CURSOR_UPARROW].name, "uparrow");

    g_cursor.cursors[CURSOR_SIZENWSE].handle = LoadCursorA(NULL, IDC_SIZENWSE);
    g_cursor.cursors[CURSOR_SIZENWSE].is_system = 1;
    g_cursor.cursors[CURSOR_SIZENWSE].type = CURSOR_SIZENWSE;
    strcpy(g_cursor.cursors[CURSOR_SIZENWSE].name, "sizenwse");

    g_cursor.cursors[CURSOR_SIZENESW].handle = LoadCursorA(NULL, IDC_SIZENESW);
    g_cursor.cursors[CURSOR_SIZENESW].is_system = 1;
    g_cursor.cursors[CURSOR_SIZENESW].type = CURSOR_SIZENESW;
    strcpy(g_cursor.cursors[CURSOR_SIZENESW].name, "sizenesw");

    g_cursor.cursors[CURSOR_SIZEWE].handle = LoadCursorA(NULL, IDC_SIZEWE);
    g_cursor.cursors[CURSOR_SIZEWE].is_system = 1;
    g_cursor.cursors[CURSOR_SIZEWE].type = CURSOR_SIZEWE;
    strcpy(g_cursor.cursors[CURSOR_SIZEWE].name, "sizewe");

    g_cursor.cursors[CURSOR_SIZENS].handle = LoadCursorA(NULL, IDC_SIZENS);
    g_cursor.cursors[CURSOR_SIZENS].is_system = 1;
    g_cursor.cursors[CURSOR_SIZENS].type = CURSOR_SIZENS;
    strcpy(g_cursor.cursors[CURSOR_SIZENS].name, "sizens");

    g_cursor.cursors[CURSOR_SIZEALL].handle = LoadCursorA(NULL, IDC_SIZEALL);
    g_cursor.cursors[CURSOR_SIZEALL].is_system = 1;
    g_cursor.cursors[CURSOR_SIZEALL].type = CURSOR_SIZEALL;
    strcpy(g_cursor.cursors[CURSOR_SIZEALL].name, "sizeall");

    g_cursor.cursors[CURSOR_NO].handle = LoadCursorA(NULL, IDC_NO);
    g_cursor.cursors[CURSOR_NO].is_system = 1;
    g_cursor.cursors[CURSOR_NO].type = CURSOR_NO;
    strcpy(g_cursor.cursors[CURSOR_NO].name, "no");

    g_cursor.cursors[CURSOR_HAND].handle = LoadCursorA(NULL, IDC_HAND);
    g_cursor.cursors[CURSOR_HAND].is_system = 1;
    g_cursor.cursors[CURSOR_HAND].type = CURSOR_HAND;
    strcpy(g_cursor.cursors[CURSOR_HAND].name, "hand");

    g_cursor.cursor_count = CURSOR_HAND + 1;
}

/*
 * Load cursor from file
 */
int cursor_load(CursorType type, const char* filename) {
    if (type >= MAX_CURSORS) return 0;

    /* Destroy existing cursor */
    if (g_cursor.cursors[type].handle && !g_cursor.cursors[type].is_system) {
        DestroyCursor(g_cursor.cursors[type].handle);
    }

    /* Load from file */
    g_cursor.cursors[type].handle = LoadCursorFromFileA(filename);
    if (!g_cursor.cursors[type].handle) {
        LOG_ERROR("Failed to load cursor: %s", filename);
        return 0;
    }

    g_cursor.cursors[type].is_system = 0;
    g_cursor.cursors[type].type = type;
    strncpy(g_cursor.cursors[type].name, filename, 31);
    g_cursor.cursors[type].name[31] = '\0';

    if (type >= g_cursor.cursor_count) {
        g_cursor.cursor_count = type + 1;
    }

    return 1;
}

/*
 * Load cursor from resource
 */
int cursor_load_resource(CursorType type, HINSTANCE hinst, u32 resource_id) {
    if (type >= MAX_CURSORS) return 0;

    /* Destroy existing cursor */
    if (g_cursor.cursors[type].handle && !g_cursor.cursors[type].is_system) {
        DestroyCursor(g_cursor.cursors[type].handle);
    }

    /* Load from resource */
    g_cursor.cursors[type].handle = LoadCursorA(hinst, MAKEINTRESOURCEA(resource_id));
    if (!g_cursor.cursors[type].handle) {
        LOG_ERROR("Failed to load cursor resource: %u", resource_id);
        return 0;
    }

    g_cursor.cursors[type].is_system = 0;
    g_cursor.cursors[type].type = type;

    if (type >= g_cursor.cursor_count) {
        g_cursor.cursor_count = type + 1;
    }

    return 1;
}

/*
 * Create cursor from bitmap
 */
int cursor_create(CursorType type, void* and_mask, void* xor_mask,
                   int width, int height, int hot_x, int hot_y) {
    if (type >= MAX_CURSORS) return 0;

    /* Destroy existing cursor */
    if (g_cursor.cursors[type].handle && !g_cursor.cursors[type].is_system) {
        DestroyCursor(g_cursor.cursors[type].handle);
    }

    /* Create cursor */
    g_cursor.cursors[type].handle = CreateCursor(
        GetModuleHandleA(NULL),
        hot_x, hot_y,
        width, height,
        and_mask, xor_mask
    );

    if (!g_cursor.cursors[type].handle) {
        LOG_ERROR("Failed to create cursor");
        return 0;
    }

    g_cursor.cursors[type].is_system = 0;
    g_cursor.cursors[type].type = type;
    g_cursor.cursors[type].hotspot_x = (u16)hot_x;
    g_cursor.cursors[type].hotspot_y = (u16)hot_y;

    if (type >= g_cursor.cursor_count) {
        g_cursor.cursor_count = type + 1;
    }

    return 1;
}

/*
 * Set current cursor
 */
void cursor_set(CursorType type) {
    if (type >= MAX_CURSORS) return;
    if (!g_cursor.cursors[type].handle) return;

    g_cursor.current_type = type;
    g_cursor.current_cursor = g_cursor.cursors[type].handle;

    if (g_cursor.visible) {
        SetCursor(g_cursor.current_cursor);
    }
}

/*
 * Set cursor from handle
 */
void cursor_set_handle(HCURSOR handle) {
    if (!handle) return;

    g_cursor.current_cursor = handle;

    if (g_cursor.visible) {
        SetCursor(handle);
    }
}

/*
 * Get current cursor
 */
CursorType cursor_get_current(void) {
    return g_cursor.current_type;
}

/*
 * Get cursor handle
 */
HCURSOR cursor_get_handle(CursorType type) {
    if (type >= MAX_CURSORS) return NULL;
    return g_cursor.cursors[type].handle;
}

/*
 * Show/hide cursor
 */
void cursor_show(int show) {
    if (g_cursor.visible != show) {
        g_cursor.visible = show;
        ShowCursor(show);
    }
}

/*
 * Toggle cursor visibility
 */
int cursor_toggle(void) {
    g_cursor.visible = !g_cursor.visible;
    ShowCursor(g_cursor.visible);
    return g_cursor.visible;
}

/*
 * Check if cursor visible
 */
int cursor_is_visible(void) {
    return g_cursor.visible;
}

/*
 * Get cursor position
 */
void cursor_get_position(int* x, int* y) {
    POINT pt;
    GetCursorPos(&pt);
    if (x) *x = pt.x;
    if (y) *y = pt.y;
}

/*
 * Set cursor position
 */
void cursor_set_position(int x, int y) {
    SetCursorPos(x, y);
}

/*
 * Get cursor position relative to window
 */
void cursor_get_window_position(HWND hwnd, int* x, int* y) {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(hwnd, &pt);
    if (x) *x = pt.x;
    if (y) *y = pt.y;
}

/*
 * Clip cursor to region
 */
void cursor_clip(int left, int top, int right, int bottom) {
    RECT rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    ClipCursor(&rect);
}

/*
 * Clip cursor to window
 */
void cursor_clip_to_window(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    ClientToScreen(hwnd, (POINT*)&rect.left);
    ClientToScreen(hwnd, (POINT*)&rect.right);
    ClipCursor(&rect);
}

/*
 * Release cursor clip
 */
void cursor_unclip(void) {
    ClipCursor(NULL);
}

/*
 * Set game cursor mode
 */
void cursor_set_game_mode(CursorGameMode mode) {
    g_cursor.game_mode = mode;

    switch (mode) {
        case CURSOR_MODE_NORMAL:
            cursor_set(CURSOR_ARROW);
            break;
        case CURSOR_MODE_TARGET:
            cursor_set(CURSOR_CROSS);
            break;
        case CURSOR_MODE_ATTACK:
            cursor_set(CURSOR_HAND);
            break;
        case CURSOR_MODE_TALK:
            cursor_set(CURSOR_HAND);
            break;
        case CURSOR_MODE_PICKUP:
            cursor_set(CURSOR_HAND);
            break;
        case CURSOR_MODE_MOVE:
            cursor_set(CURSOR_SIZEALL);
            break;
        default:
            cursor_set(CURSOR_ARROW);
            break;
    }
}

/*
 * Get game cursor mode
 */
CursorGameMode cursor_get_game_mode(void) {
    return g_cursor.game_mode;
}

/*
 * Update cursor based on context
 */
void cursor_update_context(int over_npc, int over_item, int over_monster, int can_attack) {
    if (over_npc) {
        cursor_set_game_mode(CURSOR_MODE_TALK);
    } else if (over_item) {
        cursor_set_game_mode(CURSOR_MODE_PICKUP);
    } else if (over_monster && can_attack) {
        cursor_set_game_mode(CURSOR_MODE_ATTACK);
    } else {
        cursor_set_game_mode(CURSOR_MODE_NORMAL);
    }
}

/*
 * Set cursor hotspot
 */
void cursor_set_hotspot(CursorType type, int x, int y) {
    if (type >= MAX_CURSORS) return;
    g_cursor.cursors[type].hotspot_x = (u16)x;
    g_cursor.cursors[type].hotspot_y = (u16)y;
}

/*
 * Get cursor hotspot
 */
void cursor_get_hotspot(CursorType type, int* x, int* y) {
    if (type >= MAX_CURSORS) return;
    if (x) *x = g_cursor.cursors[type].hotspot_x;
    if (y) *y = g_cursor.cursors[type].hotspot_y;
}

/*
 * Destroy cursor
 */
int cursor_destroy(CursorType type) {
    if (type >= MAX_CURSORS) return 0;
    if (g_cursor.cursors[type].is_system) return 0;

    if (g_cursor.cursors[type].handle) {
        DestroyCursor(g_cursor.cursors[type].handle);
        g_cursor.cursors[type].handle = NULL;
    }

    return 1;
}
