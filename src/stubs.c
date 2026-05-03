/*
 * Stone Age Client - Missing Symbols Stub Implementation
 * Only symbols that don't exist elsewhere should be here.
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
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

/* Mouse globals */
s32 g_mouse_x = 0;
s32 g_mouse_y = 0;
int g_mouse_pressed = 0;
u32 g_last_click_sprite = 0;

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

/* Pet data */
void* s_pet_data = NULL;
void* s_pet_skills = NULL;
u32 s_pet_skill_count = 0;
u32 s_selected_pet = 0;
void* s_battle_pet = NULL;

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

/* ========================================
 * Missing Function Implementations
 * ======================================== */

/* Character handlers */
void character_handle_remove_text(int char_id) { (void)char_id; }
void character_handle_update_text(int char_id, const char* text) { (void)char_id; (void)text; }
void character_handle_spawn_text(const char* data) { (void)data; }

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

/* Input functions */
int input_get_key(void) { return 0; }
int input_get_mouse_pos(int* x, int* y) { if (x) *x = 0; if (y) *y = 0; return 0; }
u32 input_get_current_key(void) { return 0; }

/* Render functions */
void render_rect(int x, int y, int w, int h, u32 c) { (void)x; (void)y; (void)w; (void)h; (void)c; }
void fade_render(void) {}
void fade_out(int d) { (void)d; }
void render_process_queue(void) {}

/* Battle render functions */
void battle_render_units(void) {}
void battle_render_ui(void) {}
void battle_render_effects(void) {}
void battle_render_field(void) {}
void battle_render_background(void) {}
void battle_render_combatants(void) {}

/* Battle unit */
void* battle_get_player_unit(void) { return NULL; }

/* Parse functions */
int parse_text_field(const char** data, char delim, char* out, int max_len) {
    (void)data; (void)delim; (void)out; (void)max_len; return 0;
}
int parse_field_int(const char** data, char delim) { (void)data; (void)delim; return 0; }

/* NPC handlers */
int npc_handle_dialog_text(const char* data) { (void)data; return 0; }
int npc_handle_shop_text(const char* data) { (void)data; return 0; }

/* Party handlers */
int party_handle_update(const char* data) { (void)data; return 0; }

/* Trade handlers */
int trade_handle_start(u32 id) { (void)id; return 0; }

/* Network functions */
void network_send_pvp_ride_request(u32 id) { (void)id; }
void network_send_ride_request(u32 id) { (void)id; }
void network_send_pvp_special_command(int cmd) { (void)cmd; }
void network_send_special_command(int cmd) { (void)cmd; }
void network_send_pvp_battle_end(void) {}
void network_send_battle_end(void) {}

/* UI window */
void* ui_window_create(int x, int y, int w, int h, const char* t) {
    (void)x; (void)y; (void)w; (void)h; (void)t; return NULL;
}

/* Graphics functions */
void graphics_set_resolution(int w, int h) { (void)w; (void)h; }
void graphics_set_fullscreen(int f) { (void)f; }
void graphics_set_gamma(int g) { (void)g; }
void graphics_set_vsync(int v) { (void)v; }

/* Audio functions */
void audio_set_master_volume(int v) { (void)v; }
void audio_set_music_volume(int v) { (void)v; }
void audio_set_sfx_volume(int v) { (void)v; }
void audio_set_ambient_volume(int v) { (void)v; }
void audio_set_mute(int m) { (void)m; }

/* Map handle enter field */
int map_handle_enter_field(const char* data) { (void)data; return 0; }

/* Protocol send */
int protocol_send_text_command(const char* cmd) { (void)cmd; return 0; }

/* Window proc */
LRESULT window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

/* Item functions */
const char* item_get_name(u32 id) { (void)id; return "Unknown"; }

/* ========================================
 * Placeholder functions from original binary
 * These are implemented as stubs pending full reverse engineering
 * ======================================== */

/* FUN_00418370 - Main game loop state machine
 * This is the central game update function handling all game states.
 *
 * Key operations:
 * - Network heartbeat (500ms interval via timeGetTime)
 * - Battle UI state checks via FUN_0044b030
 * - Game state transitions based on DAT_04581190 (map ID)
 * - Input handling for states 0-13
 *
 * State values (iVar3 = input state):
 * - 0,10: Normal/idle
 * - 1: Movement
 * - 2: Action mode
 * - 3: Item use
 * - 4: Pet interaction
 * - 5: Menu
 * - 6: Interaction
 * - 7: Dialog
 * - 8: Special action
 * - 9: Capture mode
 * - 11: Battle wait
 * - 12: Battle action
 * - 13: Battle result
 *
 * Map-specific behaviors (DAT_04581190):
 * - 0x7bda, 0x27dc: Special maps
 * - 0x2968-0x296e: Event maps
 * - 0x2aa7, 0x2aa8, 0x50e7, 0x50e8: Battle maps
 * - 0x3f0, 0x3fd: Dialog maps
 */
void FUN_00418370(void) {
    /* Complex state machine - see Ghidra decompilation for full logic */
}

/* FUN_00424f50 - Battle action executor dispatcher
 * Routes battle actions to specific UI/action handlers based on DAT_004b83ec
 *
 * Case mapping:
 * 0,10,0x2d: Attack UI (FUN_00426380) with mode parameter
 * 1,0xb: Attack variant (FUN_00426380)
 * 2: Skill UI (FUN_00426850)
 * 3: Item UI (FUN_00426cc0)
 * 4: Pet UI (FUN_00427190)
 * 5: Escape (FUN_004276a0)
 * 6-8: Pet actions (FUN_00428280)
 * 9: Capture (FUN_0042acf0)
 * 0xc,0xd: Capture UI (FUN_0042ce40)
 * 0xe: Pet skill 1 (FUN_0042e8f0)
 * 0xf: Pet skill 2 (FUN_0042f370)
 * 0x12: Special action (FUN_00430b50)
 * 0x13,0x14: Combo (FUN_00431560)
 * 0x15: Team action (FUN_00431d60)
 * 0x16,0x2c: Formation 0/1 (FUN_00432a10)
 * 0x17,0x2b: Special 0/1 (FUN_00433030)
 * 0x18: Field action (FUN_00436af0)
 * 0x19: Wait (FUN_004338d0)
 * 0x1a: Defend (FUN_004340a0)
 * 0x1b: Counter (FUN_00434610)
 * 0x1d: Summon (FUN_00434d60)
 * 0x1e: Recall (FUN_004354f0)
 * 0x1f: Pet switch (FUN_004366b0)
 * 0x20: Pet skill 3 (FUN_00435b40)
 * 0x21: Pet skill 4 (FUN_00435f70)
 * 0x22: Pet skill 5 (FUN_00436220)
 * 0x23: Pet special (FUN_00437200)
 * 0x24,0x27: Ride action (FUN_00438080)
 * 0x25,0x26: Dismount (FUN_00438880)
 * 0x28: Pet item (FUN_00427cc0)
 * 0x29: Pet heal (FUN_0042bb40)
 * 0x2a: Pet buff (FUN_0042b150)
 * 0x67-0x69: Extended actions
 */
void FUN_00424f50(void) {
    /* Dispatcher based on DAT_004b83ec action type */
}

/* FUN_00411990 - Dialog/UI cleanup */
void FUN_00411990(void) {
    /* TODO: Implementation needed */
}

/* FUN_0041adf0 - Set delay value to 1000 */
void FUN_0041adf0(void) {
    /* Sets DAT_0054cbac = 1000 (delay/sleep value) */
}

/* FUN_0041a8d0 - Set counter value to 1000 */
void FUN_0041a8d0(void) {
    /* Sets DAT_0054dbd8 = 1000 (counter/timer value) */
}

/* FUN_0041bba0 - Clear game state flags and free resources */
void FUN_0041bba0(void) {
    /* TODO: Clears multiple state flags and frees UI resources
     * - DAT_004aa070 = 0
     * - Calls FUN_004011c0 on multiple handles
     */
}

/* FUN_0044ac00 - Clear UI handles array
 * Iterates through DAT_0458f4c0 array and marks each handle for deletion
 */
void FUN_0044ac00(void) {
    /* Clear UI handles from DAT_0458f4c0 to DAT_0458f4d0 */
    /* Each entry is freed via FUN_004011c0 then zeroed */
}

/* FUN_004445c0 - Clear render state
 * Resets various render-related counters and flags
 */
void FUN_004445c0(void) {
    /* DAT_0456b17c = 0; _DAT_04560284 = 0; DAT_04560288 = 0; */
}

/* FUN_00445070 - Render update with timing
 * Calls multiple render functions and handles fade timing
 */
void FUN_00445070(void) {
    /* Calls: FUN_00445100, FUN_004452f0, FUN_00445650, FUN_00446320,
     * FUN_00446b30, FUN_00446e80
     * Also handles fade state (DAT_0456b178) with 2500ms timeout
     */
}

/* FUN_004419a0 - Reset map scroll position
 * Sets scroll coordinates to -1 (invalid/offscreen)
 */
void FUN_004419a0(void) {
    /* DAT_004bb414 = 0xffffffff; DAT_004bb418 = 0xffffffff; */
}

/* FUN_00440e90 - Isometric map tile rendering
 * Complex function that renders map tiles in isometric view
 * Handles tile types, sprites, and viewport positioning
 */
void FUN_00440e90(void) {
    /* TODO: Complex isometric tile rendering:
     * - Checks if map position changed (DAT_04581d3c vs DAT_004bb414)
     * - Calls FUN_00440aa0 for tile data
     * - Iterates tiles in diamond pattern
     * - Renders sprites via FUN_0047e210
     * - Handles tile types 0-99, 20-39, 40-59
     */
}

/* FUN_00410850 - Game state flag processing
 * Handles various game state flags from DAT_04ebe494
 * - Bit 25: Call FUN_00410bf0
 * - Bit 23: Call FUN_004119a0
 * - Bit 31: Clear UI state, set fade flag
 * - Bits 0-7: Process UI elements based on context
 */
void FUN_00410850(void) {
    /* Complex state machine handling:
     * - Checks DAT_04ebe494 bitmask
     * - Processes UI context (DAT_0054a4f4)
     * - Handles message log scrolling
     */
}

/* FUN_004117e0 - Chat/message log update
 * Updates scrolling message display
 * Uses DAT_0054a4f8 for position, DAT_0054a500 for counter
 */
void FUN_004117e0(void) {
    /* Iterates through message entries (20 max)
     * - Each entry is 0x10c bytes at DAT_00544d88
     * - Calculates scroll position and timing
     * - Calls FUN_0041d7c0 for rendering
     */
}

/* FUN_0047d8c0 - Render queue initialization
 * Initializes sprite render queue via FUN_0049320c
 */
void FUN_0047d8c0(void) {
    /* FUN_0049320c(&DAT_0464b488, DAT_0464f488, 4, &LAB_0047e6e0); */
}

/* FUN_00412a40 - Animation update */
void FUN_00412a40(void) {
    /* TODO: Implementation needed */
}

/* FUN_0047dc60 - Render queue processor (main sprite rendering loop)
 * Processes the sprite render queue for each frame
 *
 * Mode selection (DAT_005ab6fc):
 * 0: Normal field rendering - calls FUN_00412a40, FUN_0047d850, FUN_0047e720
 * 3: Battle mode - clears scroll, calls FUN_00404e20
 * Other: Quick update path
 *
 * Entity loop (DAT_0464f488 entities):
 * - Each entity has position at DAT_04633488 + index*0x18
 * - Checks flags at DAT_04633490 for render mode
 * - Alpha mode (DAT_0054c83c) uses different blitter
 *
 * Render modes (flags):
 * - Bit 31: Special handling via FUN_00412eb0
 * - Bit 29: Direction-based animation
 * - Bit 28: Alt animation
 * - Bit 30: Extended render
 *
 * Blitter functions:
 * - FUN_004142f0: Normal blit (no alpha)
 * - FUN_00414190: Alpha blit
 * - FUN_0047fae0: RLE compressed sprite
 * - FUN_0047e970: 16-bit alpha blend
 * - FUN_0047f170: 8-bit alpha
 * - FUN_0047f710: Extended blend
 *
 * Cleanup: Clears DAT_04633488 array (0x7001 entries)
 */
void FUN_0047dc60(void) {
    /* Complex sprite rendering - see Ghidra decompilation */
}

/* FUN_00401170 - Field entity update loop */
void FUN_00401170(void) {
    /* Iterates entity list and calls update callbacks
     * - Uses DAT_004d7e3c as entity list head
     * - Checks entity state at offset +0x24
     */
}

/* FUN_00448610 - Window widget creation (9-sprite grid) */
void FUN_00448610(void) {
    /* Implemented in ui_window.c - window_create() */
}

/* FUN_004011c0 - Set entity delete flag */
void FUN_004011c0(intptr_t entity) {
    if (entity != 0) {
        *(int*)(entity + 0x24) = 1;  /* Mark for deletion */
    }
}

/* FUN_0041d860 - UI layout wrapper
 * Calls FUN_0041d7c0 with default parameter 0
 */
void FUN_0041d860(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6) {
    /* FUN_0041d7c0(param_1, param_2, param_3, param_4, param_5, param_6, 0); */
}

/* FUN_00421080 - Find character index by ID
 * Searches character array for matching ID
 * Returns index if found, -1 otherwise
 */
int FUN_00421080(int* char_array, int count) {
    int i;
    /* Check if search enabled (DAT_045f1bc4 & 1) */
    for (i = 0; i < count; i++) {
        /* if (char_array[i] == DAT_045f1b90) return i; */
    }
    return -1;
}

/* FUN_0044b030 - Battle UI active check
 * Returns non-zero if battle UI is active (menus, dialogs, etc.)
 */
int FUN_0044b030(void) {
    /* Check: DAT_045e19b0 > 0 || DAT_004e21dc > 0 || DAT_045e8ce0 != 0 */
    /* Also checks DAT_045f1a3b and DAT_045f1a3a flags */
    return 0;  /* TODO: Implement with proper global checks */
}

/* FUN_0048a200 - Protocol function */
/* FUN_0048a200 - String escape for protocol
 * Escapes special characters in string for network transmission
 * Uses DAT_004d5828 lookup table for escape sequences
 * Handles DBCS characters properly
 */
void FUN_0048a200(char* src, int dst, int max_len) {
    /* Escapes characters like \n, \t, etc. using backslash
     * - Iterates through source string
     * - Checks DAT_004d5828 for characters needing escape
     * - Uses DAT_004d5829 for escape character mapping
     * - DBCS lead bytes are handled specially
     */
}

/* FUN_004923a7 - String format with prefix
 * Formats string with 'B' prefix (0x42)
 * Calls FUN_0049528d for actual formatting
 */
void FUN_004923a7(char* output, void* format) {
    /* Sets prefix = 'B' (0x42)
     * Calls FUN_0049528d for string formatting
     * Max length = 0x7fffffff
     */
}

/* FUN_00488190 - BGM/Sound loading and playback
 * Loads and plays background music or sound effects
 *
 * Parameters:
 *   param_1: Sound ID (0-500, maps to data/bgm/*.wav files)
 *
 * Returns: 0 on success, -1 on error
 *
 * Error conditions:
 * - DAT_04ebe25c == -1 (sound system not initialized)
 * - param_1 > 500 (invalid sound ID)
 * - DAT_04cb84d0[param_1*8] == -1 (sound not loaded)
 *
 * Operations:
 * - Checks sound index tables at DAT_04cb84d0
 * - Creates DirectSound buffer via vtable call at +0x14
 * - Sets volume via vtable +0x3c
 * - Sets pan/position via vtable +0x40
 * - Starts playback via vtable +0x30
 */
void FUN_00488190(int sound_id) {
    /* Complex DirectSound buffer management */
}

/* FUN_0043b980 - Network protocol packet builder
 * Builds a protocol packet with 6 integer fields
 * Uses DAT_004b9ff0 as output buffer, DAT_004b9fec for encoding
 */
void FUN_0043b980(void* param_1, int p2, int p3, int p4, int p5, int p6, int p7) {
    /* Calls FUN_0043e170 to initialize buffer
     * Uses FUN_0043dce0 to encode each integer field
     * Uses FUN_0043dd20 for last field
     * Calls FUN_0043e100 to finalize packet
     */
}

/* FUN_0048fdc0 - Protocol checksum calculator
 * Calculates checksum for protocol message
 * Uses FUN_0048d1f0 for field hashing
 */
void FUN_0048fdc0(void) {
    /* FUN_00492d80 initializes state
     * Multiple FUN_0048d1f0 calls hash fields
     * FUN_0048c8c0 builds final packet with opcode 0x43
     */
}

/* FUN_00492cd2 - String format with 'I' prefix
 * Formats string with 'I' prefix (0x49) for inventory protocol
 */
void FUN_00492cd2(char* param_1, void* param_2) {
    /* Sets prefix = 'I' (0x49)
     * Gets string length via _strlen
     * Calls FUN_004969ef for formatting
     */
}

/* Character position */
void character_get_position(u16* x, u16* y) { if (x) *x = 0; if (y) *y = 0; }

/* Network functions */
void network_send_action_complete(void) {}
void network_send_action_response(int r) { (void)r; }

/* NPC functions */
void* npc_get_by_id(u32 id) { (void)id; return NULL; }

/* Battle handlers */
int battle_handle_start_text(const char* data) { (void)data; return 0; }
int battle_handle_result_text(const char* data) { (void)data; return 0; }

/* Friend handlers */
int friend_handle_message(const char* data) { (void)data; return 0; }

/* Skill handlers */
int skill_handle_effect_text(const char* data) { (void)data; return 0; }

/* Map functions */
u32 map_get_current_map_id(void) { return 0; }
void map_update(void) {}
int map_get_tile(int x, int y) { (void)x; (void)y; return 0; }

/* Inventory */
void* g_inventory = NULL;

/* Map context */
void* g_map = NULL;

/* Render functions */
int render_lock_surface(void* surface) { (void)surface; return 0; }
int render_unlock_surface(void* surface) { (void)surface; return 0; }
void* render_create_surface(int w, int h) { (void)w; (void)h; return NULL; }
void render_blit_scaled(void* src, void* dst, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh) {
    (void)src; (void)dst; (void)sx; (void)sy; (void)sw; (void)sh; (void)dx; (void)dy; (void)dw; (void)dh;
}

/* NPC functions */
void npc_send_ack_binary_impl(int type) { (void)type; }

/* Item functions */
void* item_get(u32 id) { (void)id; return NULL; }

/* Map functions */
int map_is_walkable(int x, int y) { (void)x; (void)y; return 1; }
int map_check_collision(int x, int y) { (void)x; (void)y; return 0; }

/* UI functions */
int ui_init_sprites(void) { return 1; }
int ui_init(void) { return 1; }
void ui_shutdown(void) {}
void ui_update(void) {}
void ui_render(void) {}

/* UI globals */
int g_alpha_mode = 0;
void* g_ui = NULL;

/* Battle functions */
void* battle_get_unit(int index) { (void)index; return NULL; }
void* battle_get_unit_by_index(int index) { (void)index; return NULL; }
int battle_add_unit(void* unit) { (void)unit; return 0; }
void action_show_effect(int effect_id, int x, int y) { (void)effect_id; (void)x; (void)y; }
void action_play_sound(int sound_id) { (void)sound_id; }
void battle_update_unit_display(int unit_idx) { (void)unit_idx; }
void battle_update_unit_displays(void) {}
void* battle_find_rideable_pet(void) { return NULL; }
int battle_check_end_condition(void) { return 0; }

/* Config functions */
int config_init(void) { return 1; }
