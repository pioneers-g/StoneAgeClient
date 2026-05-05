/*
 * Stone Age Client - Miscellaneous Stub Functions
 * Split from stubs.c to reduce file size
 */

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "types.h"
#include "gameconfig.h"

/* Forward declarations for functions in split files */
int FUN_0045ede0(int param_1);

/* ========================================
 * Additional Utility Stubs
 * ======================================== */

void FUN_00492cd2(char* param_1, void* param_2) { (void)param_1; (void)param_2; }

int ui_init_sprites(void) { return 1; }
int ui_init(void) { return 1; }
void ui_shutdown(void) {}
void ui_update(void) {}
void ui_render(void) {}
int g_alpha_mode = 0;
void* g_ui = NULL;

/*
 * fade_render - Render fade effects
 *
 * Binary analysis:
 * - Renders screen fade/dissolve effects during state transitions
 * - Uses DAT_005ab6fc for fade mode:
 *   - 0: No fade
 *   - 2: Fade in (alpha increasing)
 *   - 3: Fade out (alpha decreasing)
 * - Duration tracked by DAT_005ab708 (start time)
 * - Duration: DAT_005ab70c (fade duration in ms)
 * - Uses DirectDraw overlay/fill with varying alpha
 */
void fade_render(void) {
    extern u32 DAT_005ab6fc;
    /* No-op if not fading */
    if (DAT_005ab6fc == 0) return;
    /* Full implementation would fill screen with black at varying alpha */
}
void fade_out(int d) { (void)d; }

void render_rect(int x, int y, int w, int h, u32 c) { (void)x; (void)y; (void)w; (void)h; (void)c; }

int input_get_key(void) { return 0; }
int input_get_mouse_pos(int* x, int* y) { if (x) *x = 0; if (y) *y = 0; return 0; }

int render_lock_surface(void* surface) { (void)surface; return 0; }
int render_unlock_surface(void* surface) { (void)surface; return 0; }
void* render_create_surface(int w, int h) { (void)w; (void)h; return NULL; }
void render_blit_scaled(void* src, void* dst, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh) {
    (void)src; (void)dst; (void)sx; (void)sy; (void)sw; (void)sh; (void)dx; (void)dy; (void)dw; (void)dh;
}

/* ========================================
 * Pet/Album Functions
 * ======================================== */

/*
 * FUN_0044a100 - Album Loader
 *
 * Binary analysis:
 * - Loads pet album data from file
 * - Parses entries and initializes album state
 */
void FUN_0044a100(void) {
    /* TODO: Full album loading implementation */
}

/* ========================================
 * NPC Functions
 * ======================================== */

/*
 * FUN_00462f60 - NPC Dialog Handler
 *
 * Binary analysis:
 * - Handles NPC dialog interactions
 * - 25+ action types for different NPC behaviors
 */
void FUN_00462f60(void) {
    /* TODO: Full NPC dialog implementation */
}

int FUN_00449510(int param_1, int param_2, int param_3, int param_4) {
    (void)param_1; (void)param_2; (void)param_3; (void)param_4;
    return 0;
}

/* Item functions */
void* item_get(int item_id) {
    (void)item_id;
    return NULL;
}

const char* item_get_name(int item_id) { (void)item_id; return ""; }

/* FUN_0043b980 - Send battle action packet to server */
void FUN_0043b980(int socket, int x, int y, int ctx1, int ctx2, int target, void* data) {
    /* Network packet dispatch - stub until network_send is wired up */
    (void)socket; (void)x; (void)y; (void)ctx1; (void)ctx2; (void)target; (void)data;
}

void FUN_0048fdc0(int param_1, int param_2, const char* param_3) {
    (void)param_1; (void)param_2; (void)param_3;
}

void FUN_004011c0(int param_1) {
    if (param_1 != 0) {
        *(int*)(param_1 + 0x24) = 1;
    }
}

/*
 * Text render queue
 * Max 1024 entries (0x400), stride 0x110 (272 bytes) - from FUN_0041d7c0
 */
#define TEXT_QUEUE_MAX 1024
#define TEXT_ENTRY_STRIDE 0x110

typedef struct {
    u16 x;
    u16 y;
    u8 type;
    u8 style;
    char text[256];
    u32 sprite;
    u32 flags;
    int active;
} TextQueueEntry;

static TextQueueEntry s_text_queue[TEXT_QUEUE_MAX];
static u32 s_text_queue_count = 0;

extern u32 DAT_005ab6f8;  /* Defined in stubs_globals.c */

/*
 * FUN_0041d7c0 - Add Text to Render Queue
 * Returns queue index on success, -2 if queue full
 */
int FUN_0041d7c0(u16 x, u16 y, u8 type, u8 style, const char* text, u32 sprite, u32 flags) {
    u32 index;

    if (s_text_queue_count >= TEXT_QUEUE_MAX) {
        return -2;
    }

    index = s_text_queue_count;
    s_text_queue[index].x = x;
    s_text_queue[index].y = y;
    s_text_queue[index].type = type;
    s_text_queue[index].style = style;
    s_text_queue[index].sprite = sprite;
    s_text_queue[index].flags = flags;
    s_text_queue[index].active = 1;

    if (text) {
        strncpy(s_text_queue[index].text, text, sizeof(s_text_queue[index].text) - 1);
        s_text_queue[index].text[sizeof(s_text_queue[index].text) - 1] = '\0';
    } else {
        s_text_queue[index].text[0] = '\0';
    }

    s_text_queue_count++;
    DAT_005ab6f8 = s_text_queue_count;
    return (int)index;
}

void FUN_0041d860(u16 x, u16 y, u8 type, u8 style, const char* text, u32 sprite) {
    FUN_0041d7c0(x, y, type, style, text, sprite, 0);
}

void FUN_0048a200(void* dest, const void* src, int size, ...) {
    if (!dest || !src || size <= 0) return;
    /* Called with 3 args: memcpy; with more: format string */
    memcpy(dest, src, size);
}

/* Battle text handlers */
void battle_handle_start_text(const char* param_1) { (void)param_1; }
void battle_handle_result_text(const char* param_1) { (void)param_1; }

/* Friend handler */
void friend_handle_message(const char* param_1) { (void)param_1; }

/* Party handler */
void party_handle_update(const char* param_1) { (void)param_1; }

/* Skill handler */
void skill_handle_effect_text(const char* param_1) { (void)param_1; }

/* NPC handlers */
void npc_handle_dialog_text(const char* param_1) { (void)param_1; }
void npc_handle_shop_text(const char* param_1) { (void)param_1; }

/* Map handlers */
void map_handle_enter_field(const char* param_1) { (void)param_1; }
int map_get_tile_type(int x, int y) { (void)x; (void)y; return 0; }

/* Pet handler */
void pet_handle_spawn_text(const char* param_1) { (void)param_1; }

/* File operations */
int file_write_bmp_paletted(const char* path, void* data, int w, int h, void* pal) {
    (void)path; (void)data; (void)w; (void)h; (void)pal;
    return 0;
}

int file_write_bmp_24bit(const char* path, void* data, int w, int h) {
    (void)path; (void)data; (void)w; (void)h;
    return 0;
}

/*
 * battle_render_background - Render battle background terrain
 *
 * Binary analysis:
 * - Renders the isometric battle field terrain
 * - Uses sprite data loaded for current battle map
 * - Draws ground tiles, obstacles, decoration
 */
void battle_render_background(void) {
    /* Render battle terrain sprites */
}

/*
 * battle_render_combatants - Render battle units
 *
 * Binary analysis:
 * - Renders all visible battle units (player + enemy)
 * - Draws unit sprites with idle/attack/defend animations
 * - Sorts by Y position for correct depth ordering
 * - Renders HP bars and status icons above units
 */
void battle_render_combatants(void) {
    /* Render battle unit sprites and status */
}

/* Map functions */
void map_update(void) {}
int map_get_tile(int x, int y) { (void)x; (void)y; return 0; }

/* NPC functions */
void* npc_get_by_id(int id) { (void)id; return NULL; }

/* Config init - defaults to windowed mode for development */
int config_init(void) {
    extern GameConfig g_config;
    g_config.window_mode = 1;  /* Windowed for dev */
    return 1;
}

/* Window proc - needed for WinMain registration */
LRESULT CALLBACK window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) PostQuitMessage(0);
            break;
        default:
            return DefWindowProcA(hWnd, message, wParam, lParam);
    }
    return 0;
}

/*
 * window_create - Create main game window (FUN_0043f830)
 *
 * Creates the game window at appropriate resolution:
 * - Fullscreen (mode 0): 640x480, WS_POPUP|WS_VISIBLE
 * - Windowed (mode 1): 320x240, WS_OVERLAPPEDWINDOW|WS_VISIBLE
 */
HWND window_create(HINSTANCE hInstance, int window_mode) {
    DWORD dwStyle;
    RECT rect;
    int width, height;
    HWND hWnd;
    extern GlobalState g_state;

    if (window_mode) {
        dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        width = 640;
        height = 480;
    } else {
        dwStyle = WS_POPUP | WS_VISIBLE;
        width = 640;
        height = 480;
    }

    SetRect(&rect, 0, 0, width, height);
    AdjustWindowRectEx(&rect, dwStyle, 0, 0);

    hWnd = CreateWindowExA(0, "StoneAge", "StoneAge",
        dwStyle, 0, 0,
        rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, hInstance, NULL);

    if (hWnd) {
        ShowWindow(hWnd, g_state.nCmdShow);
        UpdateWindow(hWnd);
    }
    return hWnd;
}

void window_destroy(void) {
    extern GlobalState g_state;
    if (g_state.hWnd) {
        DestroyWindow(g_state.hWnd);
        g_state.hWnd = NULL;
    }
    UnregisterClassA("StoneAge", g_state.hInstance);
}

void window_update(void) {
    extern GlobalState g_state;
    if (g_state.hWnd) UpdateWindow(g_state.hWnd);
}

/* Map collision functions */
int map_is_walkable(int x, int y) { (void)x; (void)y; return 1; }
int map_check_collision(int x, int y) { (void)x; (void)y; return 0; }
int map_get_current_map_id(void) { return 0; }

/*
 * FUN_0047d8c0 - Render/Animation Update
 *
 * Binary analysis:
 * - Updates animation timer and processes queued sprites
 * - Called each frame in game loop after state machine
 * - Advances DAT_0462bf34 animation counters
 * - Triggers sprite update for skin animations
 */
void FUN_0047d8c0(void) {
    extern u32 DAT_04633308;
    if (DAT_04633308 != 0) {
        /* Animation timer tick */
    }
}

/*
 * FUN_0047c9d0 - Animation State Initialization
 *
 * Binary analysis:
 * - Initializes animation state from config arrays
 * - param_1: state index (0 or 1 only)
 * - Copies 5 entries from DAT_04633326[param_1*5] to DAT_0462bf34
 * - Sets DAT_0462bf32 to 0xffff
 * - Finds first entry with bit 2 set and stores index in DAT_0462bf3e
 */
void FUN_0047c9d0(int state_index) {
    extern u32 DAT_04633308;
    extern u16 DAT_04633404;
    (void)state_index;
    DAT_04633404 = 0;
    if (DAT_04633308 == 0) {
        DAT_04633308 = 1;
    }
}

/*
 * FUN_0047cb00 - Copy UI Colors
 *
 * Binary analysis:
 * - Copies UI color configuration from global data
 * - DAT_04633343 = DAT_0054a4d0
 * - DAT_04633344 bytes 0-1 = DAT_004a2674/78
 */
/*
 * FUN_0047cb00 - Copy UI Colors
 *
 * Binary analysis:
 * - Copies UI color configuration from global data
 * - DAT_04633343 = DAT_0054a4d0
 * - DAT_04633344 bytes 0-1 = DAT_004a2674/78
 */
void FUN_0047cb00(void) {
    extern u32 DAT_04633308;
    extern u16 DAT_04633404;
    (void)DAT_04633308;
    (void)DAT_04633404;
}

/*
 * FUN_0047cb60 - Copy UI Color 3
 *
 * Binary analysis:
 * - Copies third UI color value
 * - DAT_04633344 byte 2 = DAT_045f1948
 */
/*
 * FUN_0047cb60 - Copy UI Color 3
 *
 * Binary analysis:
 * - Copies third UI color value
 * - DAT_04633344 byte 2 = DAT_045f1948
 */
void FUN_0047cb60(void) {
    /* Color copy - placeholder */
}

/* ========================================
 * IME (Input Method Editor) Functions
 * ======================================== */

/*
 * FUN_00491b50 - IME Reset
 *
 * Binary analysis:
 * - Resets the IME input state
 * - Checks if IME is open via ImmGetOpenStatus
 * - If open, toggles it off then on
 * - Clears the composition buffer
 */
void FUN_00491b50(void) {
    extern HIMC DAT_04ec08e4;
    extern char* DAT_04ec08cc;
    BOOL is_open;

    is_open = ImmGetOpenStatus(DAT_04ec08e4);
    if (is_open) {
        ImmSetOpenStatus(DAT_04ec08e4, FALSE);
        ImmSetOpenStatus(DAT_04ec08e4, TRUE);
        *DAT_04ec08cc = '\0';
    }
}

/*
 * FUN_00491b90 - Get IME Composition String
 *
 * Binary analysis:
 * - Returns current IME composition string
 * - Checks IME open status first
 * - If flag & 4: returns DAT_04ec08d0
 * - If flag & 2 and composition not empty: returns DAT_04ec08cc
 * - Returns NULL if no composition available
 */
char* FUN_00491b90(void) {
    extern HIMC DAT_04ec08e4;
    extern char* DAT_04ec08cc;
    extern char* DAT_04ec08d0;
    extern DWORD DAT_04ec0900;
    BOOL is_open;

    is_open = ImmGetOpenStatus(DAT_04ec08e4);
    if (is_open) {
        if ((DAT_04ec0900 & 4) != 0) {
            if (*DAT_04ec08d0 != '\0') {
                return DAT_04ec08d0;
            }
        } else if ((DAT_04ec0900 & 2) != 0) {
            if (*DAT_04ec08cc != '\0') {
                return DAT_04ec08cc;
            }
        }
    }
    return NULL;
}

/*
 * FUN_00491bd0 - Get IME Context
 *
 * Binary analysis:
 * - Returns the IME context handle
 */
void* FUN_00491bd0(void) {
    extern void* DAT_04ec08c4;
    return DAT_04ec08c4;
}

/*
 * FUN_0041fbb0 - Load Sprite Data Table
 *
 * Binary analysis:
 * - Loads sprite data from two files into DAT_0081c7e0
 * - Clears the sprite table (500000 * 10 dwords = 20MB)
 * - Opens param_2 (index file) and param_1 (data file)
 * - Reads 40-byte records (10 dwords) from index file
 * - Uses first dword as index to store in table
 * - Returns 0 on success, -1 on index file error, -2 on data file error
 *
 * param_1: data file path
 * param_2: index file path
 */
int FUN_0041fbb0(const char* data_file, const char* index_file) {
    extern HANDLE DAT_005ab7d8;
    extern HANDLE DAT_00a04c60;
    extern u32 DAT_0081c7e0[];

    DWORD bytes_read;
    int record[10];
    int i;

    /* Clear sprite table (50000 entries * 10 dwords = 2000000 bytes) */
    memset(DAT_0081c7e0, 0, 50000 * 40);

    /* Open index file */
    DAT_005ab7d8 = CreateFileA(index_file, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (DAT_005ab7d8 == INVALID_HANDLE_VALUE) {
        return -1;
    }

    /* Open data file */
    DAT_00a04c60 = CreateFileA(data_file, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (DAT_00a04c60 == INVALID_HANDLE_VALUE) {
        CloseHandle(DAT_005ab7d8);
        return -2;
    }

    /* Read index records */
    while (ReadFile(DAT_005ab7d8, record, sizeof(record), &bytes_read, NULL) && bytes_read == sizeof(record)) {
        int index = record[0];
        if (index >= 0 && index < 500000) {
            for (i = 0; i < 10; i++) {
                DAT_0081c7e0[index * 10 + i] = record[i];
            }
        }
    }

    CloseHandle(DAT_005ab7d8);
    return 0;
}
