/*
 * Stone Age Client - Missing Symbols Stub Implementation
 * Only symbols that don't exist elsewhere should be here.
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include "types.h"

/* ========================================
 * Missing Global Variables
 * ======================================== */

/* Pixel format - used by render and screenshot */
u32 s_pixel_format = 0;
u32 g_pixel_format = 0;
int g_resolution_mode = 0;

/* Mouse state globals */
s32 g_mouse_state = 0;
u32 g_mouse_state_flags = 0;
void* g_mouse_hover_target = NULL;
u32 g_hover_sprite = 0;
s32 g_mouse_x = 0;
s32 g_mouse_y = 0;
int g_mouse_pressed = 0;
u32 g_last_click_sprite = 0;

/* Game state globals */
int g_skip_enter_game = 0;
int g_dialog_skip_flag = 0;
u32 g_player_char_id = 0;
u32 g_current_npc_id = 0;
void* g_current_npc_context = NULL;
int g_protocol_mode = 0;
int g_in_battle = 0;
int g_game_sub_state = 0;
int g_game_state_flag = 0;
int g_state_changed = 0;

/* Map position variables */
s32 s_map_pos_x = 0;
s32 s_map_pos_y = 0;
u32 s_last_map_id = 0;
u32 s_map_param = 0;
s32 s_view_min_x = 0;
s32 s_view_min_y = 0;
s32 s_view_max_x = 640;
s32 s_view_max_y = 480;
s32 s_view_width = 640;
s32 s_view_height = 480;
u32 s_entity_count = 0;
u32 s_npc_count = 0;
u32 s_render_queue_count = 0;
int s_fade_active = 0;
s32 s_iso_camera_x = 0;
s32 s_iso_camera_y = 0;

/* More map/game state variables */
s32 s_move_target = 0;
int s_move_timer_active = 0;
u32 s_current_field_id = 0;
u32 s_field_sub_id = 0;
u32 s_last_field_id = 0;
u32 s_battle_field1 = 0;
u32 s_battle_field2 = 0;
u32 s_battle_field3 = 0;
u32 s_update_value = 0;
int s_map_entry_flag = 0;
int s_game_state = 0;
int s_game_substate = 0;
int s_battle_flag = 0;
int s_battle_substate = 0;
int s_battle_active_flag = 0;
int s_player_clear_flag = 0;
u32 s_map_id = 0;
u32 s_map_width = 0;
u32 s_map_height = 0;

/* Battle units */
void* s_battle_units = NULL;
u32 s_battle_unit_count = 0;
u32 s_selected_unit = 0;
u32 s_target_unit = 0;
u32 s_battle_unit_flags = 0;
void* s_battle_pet = NULL;

/* Pet data */
void* s_pet_data = NULL;
void* s_pet_skills = NULL;
u32 s_pet_skill_count = 0;
u32 s_selected_pet = 0;

/* Party data */
void* s_party_entries = NULL;
u32 s_party_count = 0;
u32 s_party_flag = 0;
int s_in_party = 0;

/* Player data */
char s_player_name[64] = {0};
u32 s_player_field1 = 0;
u32 s_player_field2 = 0;
u32 s_player_field3 = 0;
u32 s_player_field4 = 0;
u32 s_last_move_time = 0;

/* Network socket */
void* gSocket = NULL;

/* Pet AI */
void* g_petai = NULL;

/* Data placeholders */
u32 DAT_04581d3c = 0;
u32 DAT_04581d40 = 0;
u32 DAT_04558c34 = 0;
u32 DAT_0455b5ac = 0;
u32 DAT_04ebe490 = 0;
u32 DAT_005676a0 = 0;
u32 DAT_00564e62 = 0;
u32 DAT_0461b420 = 0;
u32 DAT_0455ef9c = 0;
u32 DAT_0461b658 = 0;
u32 DAT_046308b8 = 0;
u32 DAT_045829d8 = 0;
u32 DAT_045829dc = 0;
u32 DAT_004b83ec = 0;

/* State check globals for FUN_0044b030 */
s32 DAT_045e19b0 = 0;    /* Battle active flag */
s32 DAT_004e21dc = 0;    /* Dialog active flag */
s32 DAT_045e8ce0 = 0;    /* Menu active flag */
char DAT_045f1a3b = 0;   /* State override 1 */
char DAT_045f1a3a = 0;   /* State override 2 */

/* Search globals for FUN_00421080 */
u32 DAT_045f1b90 = 0;    /* Search target value */
u32 DAT_045f1bc4 = 0;    /* Search enabled flag */

/* VIP level for gold limit */
u32 DAT_0462e3b4 = 0;    /* VIP level */

/* ========================================
 * Missing Function Implementations
 * ======================================== */

/* Character handlers */
void character_handle_remove_text(int char_id) { (void)char_id; }
void character_handle_update_text(int char_id, const char* text) { (void)char_id; (void)text; }
void character_handle_spawn_text(const char* data) { (void)data; }
void character_get_position(u16* x, u16* y) { if (x) *x = 0; if (y) *y = 0; }

/* Pet handlers */
void pet_handle_spawn_text(int pet_id) { (void)pet_id; }

/* Protocol parsing */
int protocol_parse_field(const char** data, const char* delim, char* out, int max_len) {
    if (!data || !*data || !out || max_len <= 0) return 0;
    const char* start = *data;
    const char* end = strstr(start, delim);
    int len;
    if (end) {
        len = (int)(end - start);
        *data = end + (int)strlen(delim);
    } else {
        len = (int)strlen(start);
        *data = start + len;
    }
    if (len >= max_len) len = max_len - 1;
    strncpy(out, start, len);
    out[len] = '\0';
    return len;
}

const char* protocol_unescape_string(const char* str) {
    if (!str) return NULL;
    return str;
}

void protocol_unescape_string_to(const char* src, char* dst, int max_len) {
    if (!src || !dst || max_len <= 0) return;
    strncpy(dst, src, max_len - 1);
    dst[max_len - 1] = '\0';
}

int protocol_decode_int(const char* str) {
    if (!str) return 0;
    return atoi(str);
}

/* Auto save */
int auto_save_load(const char* filename) { (void)filename; return 0; }

/* Map tile type */
int map_get_tile_type(int tile_x, int tile_y) { (void)tile_x; (void)tile_y; return 0; }

/* BMP file writing */
int file_write_bmp_paletted(const char* fn, void* d, int w, int h, void* p) {
    (void)fn; (void)d; (void)w; (void)h; (void)p; return 0;
}
int file_write_bmp_24bit(const char* fn, void* d, int w, int h) {
    (void)fn; (void)d; (void)w; (void)h; return 0;
}

/* Sound functions */
void sound_play_bgm(int id) { (void)id; }
void sound_play_effect(int id) { (void)id; }
void sound_stop_bgm(void) {}

/* FUN_00488190 - Play Sound Effect */
int FUN_00488190(int param_1) {
    /* Play sound effect */
    (void)param_1;
    return -1;
}

/* Parse functions */
int parse_text_field(const char** data, char delim, char* out, int max_len) {
    (void)data; (void)delim; (void)out; (void)max_len; return 0;
}
int parse_field_int(const char** data, char delim) { (void)data; (void)delim; return 0; }

/* NPC handlers */
int npc_handle_dialog_text(const char* data) { (void)data; return 0; }
int npc_handle_shop_text(const char* data) { (void)data; return 0; }
void* npc_get_by_id(u32 id) { (void)id; return NULL; }
void npc_send_ack_binary_impl(int type) { (void)type; }

/* Party handlers */
int party_handle_update(const char* data) { (void)data; return 0; }

/* Trade handlers */
int trade_handle_start(u32 id) { (void)id; return 0; }

/* Battle handlers */
int battle_handle_start_text(const char* data) { (void)data; return 0; }
int battle_handle_result_text(const char* data) { (void)data; return 0; }
void* battle_get_unit(int index) { (void)index; return NULL; }
void* battle_get_unit_by_index(int index) { (void)index; return NULL; }
int battle_add_unit(void* unit) { (void)unit; return 0; }
void action_show_effect(int effect_id, int x, int y) { (void)effect_id; (void)x; (void)y; }
void action_play_sound(int sound_id) { (void)sound_id; }
void battle_update_unit_display(int unit_idx) { (void)unit_idx; }
void battle_update_unit_displays(void) {}
void* battle_find_rideable_pet(void) { return NULL; }
int battle_check_end_condition(void) { return 0; }

/* Friend handlers */
int friend_handle_message(const char* data) { (void)data; return 0; }

/* Skill handlers */
int skill_handle_effect_text(const char* data) { (void)data; return 0; }

/* Map functions */
u32 map_get_current_map_id(void) { return 0; }
void map_update(void) {}
int map_get_tile(int x, int y) { (void)x; (void)y; return 0; }
int map_is_walkable(int x, int y) { (void)x; (void)y; return 1; }
int map_check_collision(int x, int y) { (void)x; (void)y; return 0; }
int map_handle_enter_field(const char* data) { (void)data; return 0; }

/* Inventory */
void* g_inventory = NULL;
void* item_get(u32 id) { (void)id; return NULL; }
const char* item_get_name(u32 id) { (void)id; return "Unknown"; }

/* Map context */
void* g_map = NULL;

/* Config functions */
int config_init(void) { return 1; }

/* Protocol send */
int protocol_send_text_command(const char* cmd) { (void)cmd; return 0; }

/* Window proc */
LRESULT window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

/* ========================================
 * Game State Functions
 * ======================================== */

/* FUN_00418370 - Main game loop state machine */
void FUN_00418370(void) {
    /* Complex state machine - see Ghidra decompilation for full logic */
}

/* FUN_0044b030 - Check Game/Battle State
 * Returns non-zero if any UI/menu/battle is active
 * Binary: checks DAT_045e19b0, DAT_004e21dc, DAT_045e8ce0
 * Also checks DAT_045f1a3b and DAT_045f1a3a for override
 */
char FUN_0044b030(void) {
    extern s32 DAT_045e19b0;   /* Battle active flag */
    extern s32 DAT_004e21dc;   /* Dialog active flag */
    extern s32 DAT_045e8ce0;   /* Menu active flag */
    extern char DAT_045f1a3b;  /* State override 1 */
    extern char DAT_045f1a3a;  /* State override 2 */
    char result = (DAT_045e19b0 > 0) || (DAT_004e21dc > 0) || (DAT_045e8ce0 != 0);
    if (DAT_045f1a3b != 0) result = DAT_045f1a3b;
    if (DAT_045f1a3a != 0) result = DAT_045f1a3a;
    return result;
}

/* FUN_004792c0 - Calculate Gold Limit
 * Returns maximum gold based on VIP level
 * Binary: return DAT_0462e3b4 * 1800000 + 1000000
 * VIP 0 = 1M limit, VIP 1 = 2.8M, VIP 2 = 4.6M, etc.
 */
int FUN_004792c0(void) {
    extern u32 DAT_0462e3b4;  /* VIP level */
    return DAT_0462e3b4 * 1800000 + 1000000;
}

/* FUN_00421080 - Find Index by Value in Array
 * Searches for DAT_045f1b90 value in array
 * param_1: array pointer
 * param_2: array size
 * Returns: index if found, -1 if not found
 */
int FUN_00421080(int* param_1, int param_2) {
    extern u32 DAT_045f1b90;  /* Search target value */
    extern u32 DAT_045f1bc4;  /* Enabled flag */
    int i;
    if ((DAT_045f1bc4 & 1) == 0) return -1;
    for (i = 0; i < param_2; i++) {
        if (param_1[i] == (int)DAT_045f1b90) return i;
    }
    return -1;
}

/* ========================================
 * Memory Functions
 * ======================================== */

/* FUN_00491f70 - Memory Allocation (16-byte aligned)
 * param_1: count
 * param_2: size per element
 * Returns: allocated and zero-initialized memory, or NULL on failure
 * Binary: tries pool allocator first, falls back to HeapAlloc
 */
void* FUN_00491f70(int param_1, int param_2) {
    u32 size = (u32)(param_1 * param_2);
    void* ptr;
    if (size < 0xffffffe1) {
        u32 aligned_size = (size == 0) ? 16 : (size + 0xf) & 0xfffffff0;
        ptr = HeapAlloc(GetProcessHeap(), 8, aligned_size);
        if (ptr) {
            memset(ptr, 0, size);
            return ptr;
        }
    }
    return NULL;
}

/* FUN_00491fed - Free Memory
 * param_1: pointer to free
 * Binary: checks if in pool first, otherwise HeapFree
 */
void FUN_00491fed(void* param_1) {
    if (param_1) {
        HeapFree(GetProcessHeap(), 0, param_1);
    }
}

/* ========================================
 * Pet/Album Functions
 * ======================================== */

/* FUN_004017a0 - Pet AI Settings Loader with XOR Decryption */
int FUN_004017a0(int* param_1) {
    /* Load pet AI settings from AISetting.dat with XOR decryption */
    (void)param_1;
    return 0;
}

/* FUN_00425bb0 - Pet Capture Action Handler */
int FUN_00425bb0(int param_1, int param_2) {
    /* Handle pet capture action during battle */
    (void)param_1; (void)param_2;
    return 0;
}

/* FUN_00425dc0 - Pet Data Parser for Capture */
int FUN_00425dc0(const char* param_1, void* param_2) {
    /* Parse pet capture data from protocol */
    (void)param_1; (void)param_2;
    return 0;
}

/* FUN_0044a100 - Album Data Loader with XOR Decryption */
int FUN_0044a100(void) {
    /* Load and decrypt all album data files */
    return 0;
}

/* FUN_00449e00 - XOR Decryption Function */
void FUN_00449e00(unsigned char* param_1, int param_2,
                  const unsigned char* param_3, int param_4) {
    /* XOR decrypt data with key */
    (void)param_1; (void)param_2; (void)param_3; (void)param_4;
}

/* ========================================
 * NPC State Functions
 * ======================================== */

/* FUN_004781f0 - NPC State Update Handler */
void FUN_004781f0(int param_1, int param_2, int param_3, int param_4,
                  int param_5, int param_6, int param_7) {
    /* Update NPC state based on action type */
    (void)param_1; (void)param_2; (void)param_3; (void)param_4;
    (void)param_5; (void)param_6; (void)param_7;
}

/* ========================================
 * Battle Render Functions
 * ======================================== */

void battle_render_units(void) {}
void battle_render_ui(void) {}
void battle_render_effects(void) {}
void battle_render_field(void) {}
void battle_render_background(void) {}
void battle_render_combatants(void) {}
void* battle_get_player_unit(void) { return NULL; }

/* ========================================
 * Network Send Functions
 * ======================================== */

void network_send_pvp_ride_request(u32 id) { (void)id; }
void network_send_ride_request(u32 id) { (void)id; }
void network_send_pvp_special_command(int cmd) { (void)cmd; }
void network_send_special_command(int cmd) { (void)cmd; }
void network_send_pvp_battle_end(void) {}
void network_send_battle_end(void) {}
void network_send_action_complete(void) {}
void network_send_action_response(int r) { (void)r; }

/* ========================================
 * Protocol Functions
 * ======================================== */

void FUN_0043b980(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6, void* param_7) {
    (void)param_1; (void)param_2; (void)param_3; (void)param_4; (void)param_5; (void)param_6; (void)param_7;
}

void FUN_0048fdc0(void) {}

int FUN_0043dce0(int param_1) { (void)param_1; return 0; }

/* FUN_0043dc90 - Safe String Append
 * param_1: destination buffer
 * param_2: source string to append
 * param_3: max buffer size (including null terminator)
 * Binary: finds end of dest, then appends source with null termination
 */
void FUN_0043dc90(char* param_1, const char* param_2, int param_3) {
    int dest_len;
    int i;
    if (!param_1 || !param_2 || param_3 <= 1) return;
    param_3--;  /* Reserve space for null terminator */
    /* Find end of destination string */
    dest_len = 0;
    while (dest_len < param_3 && param_1[dest_len] != '\0') {
        dest_len++;
    }
    /* Append source string */
    for (i = 0; dest_len < param_3 && param_2[i] != '\0'; i++, dest_len++) {
        param_1[dest_len] = param_2[i];
    }
    param_1[dest_len] = '\0';
}

/* FUN_0043e170 - String Copy
 * param_1: destination buffer
 * param_2: source string
 * Binary: copies source to destination with null termination
 */
void FUN_0043e170(char* param_1, const char* param_2) {
    if (!param_1 || !param_2) return;
    while (*param_2) {
        *param_1++ = *param_2++;
    }
    *param_1 = '\0';
}

/* FUN_0049b108 - Integer to String Conversion
 * param_1: integer value to convert
 * param_2: output buffer
 * param_3: buffer size
 * Returns: number of characters written (excluding null)
 */
int FUN_0049b108(int param_1, char* param_2, int param_3) {
    int len;
    int temp;
    int i;
    int negative;
    unsigned int value;

    if (!param_2 || param_3 <= 0) return 0;

    negative = 0;
    value = (unsigned int)param_1;
    if (param_1 < 0) {
        negative = 1;
        value = (unsigned int)(-param_1);
    }

    /* Calculate digits */
    temp = value;
    len = 0;
    do {
        len++;
        temp /= 10;
    } while (temp > 0);

    if (negative) len++;
    if (len >= param_3) len = param_3 - 1;

    param_2[len] = '\0';
    i = len - 1;

    /* Write digits in reverse */
    do {
        param_2[i--] = '0' + (value % 10);
        value /= 10;
    } while (value > 0 && i >= 0);

    if (negative && i >= 0) {
        param_2[i] = '-';
    }

    return len;
}

/* FUN_004923a7 - Printf-style String Formatting
 * param_1: output buffer
 * param_2: format string
 * ...: variable arguments
 * Returns: number of characters written
 * Note: Simplified implementation - full version supports %s, %d, etc.
 */
int FUN_004923a7(char* param_1, const char* param_2, ...) {
    va_list args;
    int result;
    if (!param_1 || !param_2) return 0;
    va_start(args, param_2);
    result = vsnprintf(param_1, 1024, param_2, args);
    va_end(args);
    return result;
}

void FUN_0043e100(int param_1, char* param_2) {
    (void)param_1; (void)param_2;
}

int FUN_0043bf90(void* socket, const char* data) {
    (void)socket; (void)data; return 0;
}

void FUN_0048a200(const char* param_1, char* param_2, int param_3) {
    (void)param_1; (void)param_2; (void)param_3;
}

int FUN_004929fe(const char* param_1) { (void)param_1; return 0; }

void FUN_0043b490(void* param_1, int param_2) { (void)param_1; (void)param_2; }

/* ========================================
 * Entity Functions
 * ======================================== */

int* FUN_004010a0(unsigned char param_1, int param_2) {
    (void)param_1; (void)param_2; return NULL;
}

void FUN_004011d0(int param_1) { (void)param_1; }

void FUN_004011c0(intptr_t entity) {
    if (entity != 0) {
        *(int*)(entity + 0x24) = 1;
    }
}

void FUN_00401170(void) {}
void FUN_004011f0(void) {}

int FUN_0040f460(int param_1) { (void)param_1; return 0; }

void FUN_0040ddd0(int entity, int x, int y, int target, int action_type,
                  int param6, int sprite_id, int mode) {
    (void)entity; (void)x; (void)y; (void)target; (void)action_type;
    (void)param6; (void)sprite_id; (void)mode;
}

void FUN_0040b6e0(int entity, int x, int y) { (void)entity; (void)x; (void)y; }
void FUN_0040bfc0(int entity, int x, int y) { (void)entity; (void)x; (void)y; }
void FUN_0040b740(int entity, int x, int y) { (void)entity; (void)x; (void)y; }

/* ========================================
 * Render Functions
 * ======================================== */

int FUN_00448610(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6) {
    (void)param_1; (void)param_2; (void)param_3; (void)param_4; (void)param_5; (void)param_6;
    return 0;
}

void FUN_00448270(int param_1) { (void)param_1; }

void FUN_0041d860(int param_1, int param_2, int param_3, int param_4, const char* param_5, int param_6) {
    (void)param_1; (void)param_2; (void)param_3; (void)param_4; (void)param_5; (void)param_6;
}

void FUN_0041d890(char* param_1) { (void)param_1; }

int FUN_00411900(const unsigned char* param_1) { (void)param_1; return 0; }

int FUN_0044aba0(int param_1, int param_2, int param_3, int param_4, int param_5) {
    (void)param_1; (void)param_2; (void)param_3; (void)param_4; (void)param_5;
    return 0;
}

int FUN_0047e210(int param_1, int param_2, int param_3, int param_4, int param_5) {
    (void)param_1; (void)param_2; (void)param_3; (void)param_4; (void)param_5;
    return -2;
}

void FUN_0047e640(int param_1, int param_2, unsigned int param_3, unsigned int param_4,
                  int param_5, unsigned int param_6, int param_7) {
    (void)param_1; (void)param_2; (void)param_3; (void)param_4;
    (void)param_5; (void)param_6; (void)param_7;
}

void FUN_0047e720(void) {}
void FUN_0047dc60(void) {}
void FUN_0047d8c0(void) {}

int FUN_0041fad0(unsigned int param_1, unsigned int* param_2) {
    (void)param_1; (void)param_2; return 0;
}

int FUN_0041f900(unsigned int param_1, unsigned short* param_2, unsigned short* param_3) {
    (void)param_1; (void)param_2; (void)param_3; return 0;
}

void FUN_004412e0(void) {}
void FUN_00440e90(void) {}
void FUN_004445c0(void) {}
void FUN_00445070(void) {}
void FUN_004419a0(void) {}
void FUN_00412a40(void) {}

/* ========================================
 * UI Functions
 * ======================================== */

void FUN_00411990(void* param_1) { (void)param_1; }
void FUN_0041adf0(void) {}
void FUN_0041a8d0(void) {}
void FUN_0041bba0(void) {}
void FUN_0044ac00(void) {}

void* ui_window_create(int x, int y, int w, int h, const char* t) {
    (void)x; (void)y; (void)w; (void)h; (void)t; return NULL;
}

void render_process_queue(void) {}

/* ========================================
 * Game State Functions
 * ======================================== */

void FUN_00410850(void) {}
void FUN_004117e0(void) {}

/* ========================================
 * Battle State Functions
 * ======================================== */

void FUN_0040a1a0(void) {}
void FUN_00424b70(int param_1, int param_2, int param_3, int param_4, char* param_5) {
    (void)param_1; (void)param_2; (void)param_3; (void)param_4; (void)param_5;
}
void FUN_00424f50(void) {}
void FUN_00426cc0(void) {}
void FUN_00427190(void) {}
void FUN_004276a0(void) {}
void FUN_00426380(int mode) { (void)mode; }
void FUN_00426850(void) {}
void FUN_0042ce40(void) {}
void FUN_0042acf0(void) {}
void FUN_0042e8f0(void) {}
void FUN_00430b50(void) {}
void FUN_00431560(void) {}
void FUN_004338d0(void) {}
void FUN_00434d60(void) {}
void FUN_004354f0(void) {}
void FUN_00432a10(int param_1) { (void)param_1; }
void FUN_00438080(void) {}
void FUN_00438880(void) {}
void FUN_00439310(char* param_1, int param_2) { (void)param_1; (void)param_2; }
void FUN_00438f70(int param_1, void* param_2) { (void)param_1; (void)param_2; }
void FUN_0044aff0(void) {}
void FUN_0044ac50(void) {}
void FUN_0044adc0(void) {}
void FUN_0040daf0(void) {}
void FUN_00419ac0(void) {}
void FUN_00419a40(void) {}
void FUN_00404e20(void) {}
int FUN_00404850(int param_1) { (void)param_1; return 0; }

/* ========================================
 * Shop/UI Functions
 * ======================================== */

void FUN_00465d20(int param_1, const char* param_2) { (void)param_1; (void)param_2; }
void FUN_00416be0(const char* param_1) { (void)param_1; }
void FUN_00415c50(int param_1, const char* param_2) { (void)param_1; (void)param_2; }

/* ========================================
 * NPC Functions
 * ======================================== */

void FUN_00462f60(void) {}
int FUN_00449510(int param_1, int param_2, int param_3, int param_4) {
    (void)param_1; (void)param_2; (void)param_3; (void)param_4; return 0;
}

/* ========================================
 * Network I/O Functions
 * ======================================== */

void FUN_0045e880(void) {}
int FUN_0045f4d0(void) { return 0; }
void FUN_0045ffb0(int param_1, unsigned char* param_2) { (void)param_1; (void)param_2; }
int FUN_0045ec80(void* param_1, unsigned int param_2) { (void)param_1; (void)param_2; return 0; }
int FUN_0045ee40(void* param_1, int param_2) { (void)param_1; (void)param_2; return -1; }
int FUN_0045ede0(int param_1) { (void)param_1; return 0; }

/* ========================================
 * Additional Utility Functions
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
