/*
 * Stone Age Client - Miscellaneous Stub Functions
 * Split from stubs.c to reduce file size
 */

#include <windows.h>
#include "types.h"

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

void fade_render(void) {}
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

/* Additional UI render functions */
void FUN_0043b980(int param_1, int param_2, int param_3, int param_4) {
    (void)param_1; (void)param_2; (void)param_3; (void)param_4;
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
 * Text render queue globals
 * Max 1024 entries (0x400), stride 0x110 (272 bytes)
 */
#define TEXT_QUEUE_MAX 1024
extern u32 DAT_005ab6f8;      /* Text queue count */

/*
 * FUN_0041d7c0 - Add Text to Render Queue
 *
 * Binary analysis:
 * - Adds text entry to render queue at DAT_005676f8
 * - Max 1024 entries (0x400)
 * - Each entry: 0x110 bytes (stride)
 * - param_1: X position
 * - param_2: Y position
 * - param_3: color/type byte
 * - param_4: style byte
 * - param_5: text string
 * - param_6: sprite/color data
 * - param_7: additional flags
 * - Returns queue index on success, -2 if queue full
 */
int FUN_0041d7c0(u16 x, u16 y, u8 type, u8 style, const char* text, u32 sprite, u32 flags) {
    if (DAT_005ab6f8 >= TEXT_QUEUE_MAX) {
        return -2;  /* Queue full */
    }

    /* Entry at DAT_005676f8 + index * 0x110 */
    /* TODO: Actual implementation needs proper memory layout */
    (void)x; (void)y; (void)type; (void)style; (void)text; (void)sprite; (void)flags;
    DAT_005ab6f8++;
    return DAT_005ab6f8 - 1;
}

/*
 * FUN_0041d860 - Add Text to Queue (Simplified)
 *
 * Binary analysis:
 * - Wrapper for FUN_0041d7c0 with flags=0
 */
void FUN_0041d860(u16 x, u16 y, u8 type, u8 style, const char* text, u32 sprite) {
    FUN_0041d7c0(x, y, type, style, text, sprite, 0);
}

void FUN_0048a200(const char* param_1, ...) {
    (void)param_1;
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

/* Battle render functions */
void battle_render_background(void) {}
void battle_render_combatants(void) {}

/* Map functions */
void map_update(void) {}
int map_get_tile(int x, int y) { (void)x; (void)y; return 0; }

/* NPC functions */
void* npc_get_by_id(int id) { (void)id; return NULL; }

/* Config init */
int config_init(void) { return 1; }

/* Window proc */
LRESULT window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    (void)hwnd; (void)msg; (void)wparam; (void)lparam;
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

/* Map collision functions */
int map_is_walkable(int x, int y) { (void)x; (void)y; return 1; }
int map_check_collision(int x, int y) { (void)x; (void)y; return 0; }
int map_get_current_map_id(void) { return 0; }

/* Render function */
void FUN_0047d8c0(void) {}

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
    (void)state_index;
}

/*
 * FUN_0047cb00 - Copy UI Colors
 *
 * Binary analysis:
 * - Copies UI color configuration from global data
 * - DAT_04633343 = DAT_0054a4d0
 * - DAT_04633344 bytes 0-1 = DAT_004a2674/78
 */
void FUN_0047cb00(void) {}

/*
 * FUN_0047cb60 - Copy UI Color 3
 *
 * Binary analysis:
 * - Copies third UI color value
 * - DAT_04633344 byte 2 = DAT_045f1948
 */
void FUN_0047cb60(void) {}

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
