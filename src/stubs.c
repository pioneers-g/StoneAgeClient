/*
 * Stone Age Client - Missing Symbols Stub Implementation
 * Split into multiple files:
 * - stubs_globals.c: Global variables
 * - stubs_render.c: Render functions
 * - stubs_network.c: Network functions
 * - stubs_battle.c: Battle functions
 * - stubs_ui.c: UI functions
 * - stubs_entity.c: Entity functions
 * This file: Remaining utility stubs
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include "types.h"

/* Forward declarations for functions in split files */
int FUN_0045ede0(int param_1);

/* ========================================
 * Game State Functions
 * ======================================== */

/*
 * FUN_00479bc0 - Set Game State
 *
 * Binary analysis:
 * - Sets game state (param_1) to DAT_04630dd8
 * - Clears sub-state DAT_04630df0 to 0
 */
void FUN_00479bc0(int param_1) {
    extern u32 DAT_04630dd8;
    extern u32 DAT_04630df0;
    DAT_04630df0 = 0;
    DAT_04630dd8 = param_1;
}

/*
 * FUN_00479c40 - Game State Machine
 *
 * Binary analysis:
 * - Main game state machine with 15+ states
 * - State dispatch via DAT_04630df0
 * - States: 0=init, 1=login, 2=char_select, 3=playing, etc.
 */
void FUN_00479c40(void) {
    /* State machine implementation in gamestate.c */
}

/* ========================================
 * Memory Functions
 * ======================================== */

/*
 * FUN_00491f70 - 16-byte Aligned Memory Allocation
 *
 * Binary analysis:
 * - Allocates zeroed memory with 16-byte alignment
 * - param_1: element count
 * - param_2: element size
 * - Returns: pointer to allocated memory or NULL
 * - Uses HeapAlloc with HEAP_ZERO_MEMORY
 */
void* FUN_00491f70(int param_1, int param_2) {
    u32 size = (u32)(param_1 * param_2);
    void* ptr;

    if (size < 0xffffffe1) {
        u32 aligned_size = (size == 0) ? 16 : (size + 0xf) & 0xfffffff0;
        ptr = HeapAlloc(GetProcessHeap(), 8, aligned_size);  /* HEAP_ZERO_MEMORY = 8 */
        if (ptr) memset(ptr, 0, size);
        return ptr;
    }
    return NULL;
}

/*
 * FUN_00491fed - Memory Free with Heap Fallback
 *
 * Binary analysis:
 * - Frees memory, with fallback to heap if pointer not in known region
 */
void FUN_00491fed(void* param_1) {
    if (param_1) {
        HeapFree(GetProcessHeap(), 0, param_1);
    }
}

/* ========================================
 * Utility Functions
 * ======================================== */

/*
 * FUN_0044b030 - Game/Battle State Checker
 *
 * Binary analysis:
 * - Returns 1 if game is in a state that blocks certain actions
 * - Checks: battle active (DAT_045e19b0), dialog (DAT_004e21dc), menu (DAT_045e8ce0)
 */
int FUN_0044b030(void) {
    extern s32 DAT_045e19b0;
    extern s32 DAT_004e21dc;
    extern s32 DAT_045e8ce0;
    extern char DAT_045f1a3b;
    extern char DAT_045f1a3a;

    /* Check if battle, dialog, or menu is active */
    if (DAT_045e19b0 != 0) return 1;
    if (DAT_004e21dc != 0) return 1;
    if ((DAT_045e8ce0 & 0x40000000) != 0) return 1;
    if (DAT_045f1a3b != 0) return 1;
    if (DAT_045f1a3a != 0) return 1;

    return 0;
}

/*
 * FUN_00421080 - Array Search by Value
 *
 * Binary analysis:
 * - Searches array for target value
 * - param_1: array pointer
 * - param_2: element count
 * - param_3: target value
 * - Returns: index if found, -1 if not
 */
int FUN_00421080(int* param_1, int param_2, int param_3) {
    int i;
    extern u32 DAT_045f1b90;
    extern u32 DAT_045f1bc4;

    DAT_045f1b90 = param_3;
    DAT_045f1bc4 = 1;

    if (!param_1 || param_2 <= 0) return -1;

    for (i = 0; i < param_2; i++) {
        if (param_1[i] == param_3) {
            return i;
        }
    }
    return -1;
}

/*
 * FUN_004792c0 - Calculate Gold Limit
 *
 * Binary analysis:
 * - Calculates maximum gold based on VIP level
 * - Formula: VIP * 1,800,000 + 1,000,000
 * - VIP level from DAT_0462e3b4
 */
int FUN_004792c0(void) {
    extern u32 DAT_0462e3b4;
    return DAT_0462e3b4 * 1800000 + 1000000;
}

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
 * Protocol Functions
 * ======================================== */

/*
 * FUN_0043bf90 - Text Protocol Dispatcher
 *
 * Binary analysis:
 * - Dispatches text-based protocol commands (40+ commands)
 * - Commands delimited by newline
 * - Each command parsed and routed to handlers
 */
void FUN_0043bf90(const char* param_1) {
    (void)param_1;
    /* TODO: Full implementation with command dispatch */
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

/*
 * FUN_0043dc90 - Safe String Append with Buffer Limit
 *
 * Binary analysis:
 * - Appends source string to destination with buffer size limit
 * - param_1: destination buffer
 * - param_2: source string
 * - param_3: buffer size (including null terminator)
 * - Finds end of destination string first
 * - Appends source until buffer limit reached or source exhausted
 * - Always null-terminates the result
 */
void FUN_0043dc90(char* dest, const char* src, int max_size) {
    int dest_len = 0;
    int i;

    if (max_size <= 1) return;

    max_size--;

    /* Find end of destination */
    while (dest[dest_len] != '\0') {
        dest_len++;
        if (dest_len >= max_size) return;
    }

    /* Append source */
    for (i = 0; i < max_size - dest_len && src[i] != '\0'; i++) {
        dest[dest_len + i] = src[i];
    }
    dest[dest_len + i] = '\0';
}

/*
 * FUN_0043e170 - String Copy
 *
 * Binary analysis:
 * - Copies formatted string to destination
 * - Uses FUN_0043e0f0 to process format string
 * - Uses FUN_004923a7 for string formatting
 */
void FUN_0043e170(char* dest, const char* src) {
    (void)dest; (void)src;
    /* TODO: Full implementation */
}

/*
 * FUN_0049b108 - Integer to String Conversion
 *
 * Binary analysis:
 * - Converts integer to string representation
 * - param_1: integer value to convert
 * - param_2: output buffer
 * - param_3: radix (usually 10 for decimal)
 * - Handles negative numbers for radix 10
 * - Returns pointer to output buffer
 */
char* FUN_0049b108(int value, char* buffer, int radix) {
    (void)value; (void)radix;
    if (buffer) buffer[0] = '0';
    return buffer;
}

/*
 * FUN_004923a7 - Printf-Style String Formatting
 *
 * Binary analysis:
 * - Formats string similar to sprintf
 * - param_1: output buffer
 * - param_2: format string with %u, %s, %d etc.
 * - Returns formatted string length
 */
int FUN_004923a7(char* buffer, const char* format, ...) {
    (void)buffer; (void)format;
    return 0;
}

/*
 * FUN_00492973 - String to Integer Conversion
 *
 * Binary analysis:
 * - Locale-aware string to integer conversion
 * - Skips leading whitespace (character type & 8)
 * - Handles + and - signs
 * - Parses decimal digits (character type & 4)
 * - Uses DAT_004d786c to determine locale handling mode
 * - Uses PTR_DAT_004d7660 character type lookup table
 */
int FUN_00492973(const char* str) {
    const unsigned char* ptr = (const unsigned char*)str;
    int result = 0;
    int sign = 1;

    /* Skip whitespace */
    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') {
        ptr++;
    }

    /* Handle sign */
    if (*ptr == '-') {
        sign = -1;
        ptr++;
    } else if (*ptr == '+') {
        ptr++;
    }

    /* Parse digits */
    while (*ptr >= '0' && *ptr <= '9') {
        result = result * 10 + (*ptr - '0');
        ptr++;
    }

    return result * sign;
}

/* Sound/bgm function */
void FUN_00488190(int param_1, int param_2, int param_3) {
    (void)param_1; (void)param_2; (void)param_3;
}

/* Additional UI render functions */
void FUN_0043b980(int param_1, int param_2, int param_3, int param_4) {
    (void)param_1; (void)param_2; (void)param_3; (void)param_4;
}

void FUN_0048fdc0(int param_1, int param_2, const char* param_3) {
    (void)param_1; (void)param_2; (void)param_3;
}

/* FUN_00448610 is in stubs_ui.c */

void FUN_004011c0(void) {}
void FUN_0041d860(void) {}

void FUN_0048a200(const char* param_1, ...) {
    (void)param_1;
}

/* FUN_004923a7 is defined above with documentation */

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

/* Character handlers */
void character_handle_spawn_text(const char* param_1) { (void)param_1; }
void character_handle_remove_text(const char* param_1) { (void)param_1; }
void character_handle_update_text(const char* param_1) { (void)param_1; }

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

/* Auto save */
void auto_save_load(void) {}

/* Battle render functions */
void battle_render_background(void) {}
void battle_render_combatants(void) {}

/* Map functions */
void map_update(void) {}
int map_get_tile(int x, int y) { (void)x; (void)y; return 0; }

/* NPC functions */
void* npc_get_by_id(int id) { (void)id; return NULL; }

/* Protocol send */
void protocol_send_text_command(const char* cmd) { (void)cmd; }

/* Config init */
int config_init(void) { return 1; }

/* Window proc */
LRESULT window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    (void)hwnd; (void)msg; (void)wparam; (void)lparam;
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

/* Sound functions */
void sound_play_bgm(int bgm_id) { (void)bgm_id; }
void sound_play_effect(int effect_id) { (void)effect_id; }

/* Map collision functions */
int map_is_walkable(int x, int y) { (void)x; (void)y; return 1; }
int map_check_collision(int x, int y) { (void)x; (void)y; return 0; }
int map_get_current_map_id(void) { return 0; }

/* Character functions */
void character_get_position(int* x, int* y) {
    if (x) *x = 0;
    if (y) *y = 0;
}

/* Item functions */
const char* item_get_name(int item_id) { (void)item_id; return ""; }

/* Render function */
void FUN_0047d8c0(void) {}
