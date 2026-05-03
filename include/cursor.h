/*
 * Stone Age Client - Cursor System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef CURSOR_H
#define CURSOR_H

#include "types.h"

/* Constants */
#define MAX_CURSORS     32

/* Cursor types */
typedef enum {
    CURSOR_ARROW = 0,
    CURSOR_IBEAM = 1,
    CURSOR_WAIT = 2,
    CURSOR_CROSS = 3,
    CURSOR_UPARROW = 4,
    CURSOR_SIZENWSE = 5,
    CURSOR_SIZENESW = 6,
    CURSOR_SIZEWE = 7,
    CURSOR_SIZENS = 8,
    CURSOR_SIZEALL = 9,
    CURSOR_NO = 10,
    CURSOR_HAND = 11,
    /* Game-specific cursors */
    CURSOR_TARGET = 12,
    CURSOR_ATTACK = 13,
    CURSOR_TALK = 14,
    CURSOR_PICKUP = 15,
    CURSOR_MAGIC = 16,
    CURSOR_FORBIDDEN = 17
} CursorType;

/* Game cursor modes */
typedef enum {
    CURSOR_MODE_NORMAL = 0,
    CURSOR_MODE_TARGET = 1,
    CURSOR_MODE_ATTACK = 2,
    CURSOR_MODE_TALK = 3,
    CURSOR_MODE_PICKUP = 4,
    CURSOR_MODE_MOVE = 5,
    CURSOR_MODE_MAGIC = 6
} CursorGameMode;

/* Cursor entry */
typedef struct {
    HCURSOR handle;
    CursorType type;
    int is_system;
    char name[32];
    u16 hotspot_x;
    u16 hotspot_y;

} CursorEntry;

/* Cursor context */
typedef struct {
    /* Loaded cursors */
    CursorEntry cursors[MAX_CURSORS];
    int cursor_count;

    /* Current state */
    CursorType current_type;
    HCURSOR current_cursor;
    int visible;

    /* Game mode */
    CursorGameMode game_mode;

} CursorContext;

/* Global cursor context */
extern CursorContext g_cursor;

/* Initialization - FUN_00488ca0 */
int cursor_init(HWND hWnd, HINSTANCE hInstance);
void cursor_shutdown(void);

/* Load cursors */
void cursor_load_defaults(void);
int cursor_load(CursorType type, const char* filename);
int cursor_load_resource(CursorType type, HINSTANCE hinst, u32 resource_id);
int cursor_create(CursorType type, void* and_mask, void* xor_mask,
                   int width, int height, int hot_x, int hot_y);

/* Set cursor */
void cursor_set(CursorType type);
void cursor_set_handle(HCURSOR handle);

/* Get cursor */
CursorType cursor_get_current(void);
HCURSOR cursor_get_handle(CursorType type);

/* Visibility */
void cursor_show(int show);
int cursor_toggle(void);
int cursor_is_visible(void);

/* Position */
void cursor_get_position(int* x, int* y);
void cursor_set_position(int x, int y);
void cursor_get_window_position(HWND hwnd, int* x, int* y);

/* Clipping */
void cursor_clip(int left, int top, int right, int bottom);
void cursor_clip_to_window(HWND hwnd);
void cursor_unclip(void);

/* Game mode */
void cursor_set_game_mode(CursorGameMode mode);
CursorGameMode cursor_get_game_mode(void);
void cursor_update_context(int over_npc, int over_item, int over_monster, int can_attack);

/* Hotspot */
void cursor_set_hotspot(CursorType type, int x, int y);
void cursor_get_hotspot(CursorType type, int* x, int* y);

/* Destroy */
int cursor_destroy(CursorType type);

#endif /* CURSOR_H */
