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

/* FUN_0043b980 - Send Protocol Packet with Parameters
 * Constructs and sends a binary protocol packet with multiple fields
 *
 * Parameters:
 * - param_1: Socket/connection handle
 * - param_2: First integer field (encoded via FUN_0043dce0)
 * - param_3: Second integer field
 * - param_4: Third integer field
 * - param_5: Fourth integer field
 * - param_6: Fifth integer field
 * - param_7: String/data field (encoded via FUN_0043dd20)
 *
 * Processing:
 * 1. Initialize buffer via FUN_0043e170(DAT_004b9ff0, &DAT_004b9f34)
 * 2. Encode each integer parameter via FUN_0043dce0 (Base-62)
 * 3. Append each encoded field via FUN_0043dc90
 * 4. Encode string parameter via FUN_0043dd20
 * 5. Send via FUN_0043e100
 *
 * Buffer locations:
 * - DAT_004b9ff0: Output buffer
 * - DAT_004b9fec: Encoding table/format
 * - DAT_004b9f34: Protocol format string
 */
void FUN_0043b980(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6, void* param_7) {
    /* Send protocol packet with 5 int fields + 1 string field */
}

/* FUN_0048fdc0 - Send Checksum Verification Packet
 * Sends a verification/checksum packet to the server
 *
 * Stack-based parameter passing (uses 7 parameters from stack):
 * - Parameters passed via stack: 0x4004-0x401c
 *
 * Processing:
 * 1. Call FUN_00492d80 (initialization/clear)
 * 2. Calculate checksum via FUN_0048d1f0 for each field
 * 3. Sum all checksums
 * 4. Send via FUN_0048c8c0 with opcode 0x43 ('C')
 *
 * Used for: Server-side verification of client data integrity
 */
void FUN_0048fdc0(void) {
    /* Send checksum verification packet */
}

/* FUN_004929fe - String to Integer Wrapper
 * Wrapper function for FUN_00492973 (string to int conversion)
 *
 * Parameter: param_1 - String to convert
 *
 * Simply calls FUN_00492973 which:
 * - Skips leading whitespace
 * - Handles +/- signs
 * - Converts decimal digits to integer
 *
 * Returns: Parsed integer value
 */
int FUN_004929fe(const char* param_1) {
    /* Convert string to integer */
    return 0;
}

/* FUN_0043dce0 - Encode Integer to Base-62 String
 * Converts an integer to Base-62 encoded string for protocol
 *
 * Parameter: param_1 - Integer value to encode
 *
 * Processing:
 * 1. Call FUN_0043e190 to encode using DAT_004b9fec table
 * 2. Clear output buffer via FUN_0043dc40(DAT_004b9ffc, result)
 * 3. Append to buffer via FUN_0043dc90
 *
 * Output: DAT_004b9ffc - Buffer containing encoded string
 *
 * Returns: Pointer to encoded string in DAT_004b9ffc
 */
int FUN_0043dce0(int param_1) {
    /* Encode integer to Base-62 string */
    return 0;
}

/* FUN_0043dc90 - Append String to Buffer with Limit
 * Appends a string to buffer with maximum size limit
 *
 * Parameters:
 * - param_1: Destination buffer
 * - param_2: Source string to append
 * - param_3: Maximum buffer size
 *
 * Processing:
 * - Find null terminator in destination
 * - Append source string until null or limit reached
 * - Respects (param_3 - 1) maximum characters
 *
 * Note: This is a safe string concatenation function
 */
void FUN_0043dc90(char* param_1, const char* param_2, int param_3) {
    /* Append string to buffer with size limit */
}

/* FUN_0043e170 - Initialize Protocol Buffer
 * Initializes a protocol output buffer with format string
 *
 * Parameters:
 * - param_1: Buffer to initialize
 * - param_2: Format specifier
 *
 * Processing:
 * 1. Call FUN_0043e0f0 to clear format state
 * 2. Initialize with "%u %s" format via FUN_004923a7
 *
 * Format string: "%u %s" at DAT_004ba3ac
 */
void FUN_0043e170(char* param_1, const char* param_2) {
    /* Initialize protocol buffer with format */
}

/* FUN_0049b108 - Integer to String Conversion
 * Converts integer to string representation in given base
 *
 * Parameters:
 * - param_1: Integer value to convert
 * - param_2: Output buffer
 * - param_3: Number base (typically 10 for decimal)
 *
 * Special handling:
 * - If base is 10 and value is negative, sets sign flag
 * - Calls FUN_0049b135 for actual conversion
 *
 * Returns: Pointer to output buffer (param_2)
 */
int FUN_0049b108(int param_1, char* param_2, int param_3) {
    /* Convert integer to string in given base */
    return 0;
}

/* FUN_004923a7 - String Format/Initialize
 * Formats a string with printf-style arguments
 *
 * Parameters:
 * - param_1: Output buffer
 * - param_2: Format string
 * - ...: Variable arguments (on stack)
 *
 * Processing:
 * - Uses FUN_0049528d for formatting
 * - Maximum length: 0x7fffffff - 1 characters
 * - Null-terminates result
 *
 * Returns: Result from format function
 */
int FUN_004923a7(char* param_1, const char* param_2, ...) {
    /* Format string with variable arguments */
    return 0;
}

/* FUN_0043e100 - Send Protocol Message
 * Sends a protocol message to the server
 *
 * Parameters:
 * - param_1: Socket/connection handle
 * - param_2: Message string to send
 *
 * Processing:
 * - If DAT_0461b658 == 0: Encrypt via FUN_0043e1d0
 * - Calculate string length
 * - Append newline if not present
 * - Send via function pointer at DAT_004b9fe8
 *
 * Message format: "data\n" (newline-terminated)
 * Encryption: XOR-based using DAT_004b9fec * 3 table
 */
void FUN_0043e100(int param_1, char* param_2) {
    /* Send protocol message to server */
}

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

/* FUN_0040a1a0 - Battle State Machine (Main Loop)
 * Central state machine managing all battle phases and transitions
 *
 * State machine (DAT_04630df0):
 * - State 0: Initialize battle - cleanup, load map, set BGM
 * - State 1: Fade transition
 * - State 2: Wait for action input, process packets
 * - State 3: Execute actions, update entities
 * - State 4: Process special modes
 * - State 5: Handle results, display outcomes
 * - State 6: Show battle result UI
 * - State 7: Fade out, cleanup
 * - State 8: Exit battle, return to field
 *
 * State 0 initialization:
 * - Clears handles: DAT_0461c2c0, DAT_0461c680, DAT_0461c684
 * - Calls: FUN_004011f0, FUN_00419ac0, FUN_00419a40, FUN_0040daf0
 * - Loads battle map via FUN_00404850
 * - Initializes render queue (DAT_0464f488 = 0, DAT_005ab6f8 = 0)
 *
 * BGM selection (based on DAT_04581190 map ID):
 * - Special maps (0x331, 0x1f47, 0x1fa5, etc.): BGM 0x18
 * - Arena map (0x2147): BGM 0x0e
 * - Day time (15-21 hours): BGM 0x0d or 0x0c
 * - Night/other: BGM 0x06 or 0x05
 *
 * Special map IDs for arena battles:
 * - 0x331, 0x1f47, 0x1fa5, 0x1fa4, 0x1f5b-0x1f5d, 0x1f4f, 0x1fb1, 0x1fb2
 *
 * Entity counter: DAT_04ebe31c (incremented each frame)
 * Frame counter: DAT_004d7f7c (0-63 circular)
 */
void FUN_0040a1a0(void) {
    /* Battle state machine - 9 states */
}

/* FUN_00424b70 - Battle Action Dispatcher
 * Processes incoming battle action and sets up appropriate state
 *
 * Parameters:
 * - param_1: Action type (0-0x68)
 * - param_2: Action parameter bitmask
 * - param_3: X coordinate or secondary parameter
 * - param_4: Y coordinate or tertiary parameter
 * - param_5: String parameter (target name, skill name, etc.)
 *
 * Special action types:
 * - 0x65: Observation mode (sets DAT_00564e70, copies string to DAT_00564e74)
 *   - With param_3=0x208: Sets target coordinates in DAT_0054ccd8
 *
 * Global state changes:
 * - DAT_004b83ec: Current action type
 * - DAT_0455ef94: Action parameter bitmask
 * - DAT_04558c34: X/secondary parameter
 * - DAT_0455b5ac: Y/tertiary parameter
 * - DAT_045528c8, DAT_0454fe98: Player coordinates
 *
 * Action type to handler mapping:
 * - 0,1,0x2d: Attack (FUN_00425380)
 * - 2: Skill (FUN_004253d0)
 * - 6: Pet summon (FUN_00425420)
 * - 7,8: Pet action (FUN_004254e0)
 * - 9: Capture (FUN_00425bb0)
 * - 0xa,0xb: Multi-target (FUN_00425b50)
 * - 0xc: Item (FUN_004262f0)
 * - 0xd: Capture data (FUN_00425dc0)
 * - 0xe: Pet skill 1 (FUN_0042e870)
 * - 0xf: Pet skill 2 (FUN_0042f130)
 * - 0x12: Special action (FUN_00430700)
 * - 0x15: Team action (FUN_00431ad0)
 * - 0x16,0x2c: Formation (FUN_004327b0)
 * - 0x17,0x2b: Extended action (FUN_00432ec0)
 * - 0x18: Gold adjustment (parses param_5 as int)
 * - 0x19: Wait (FUN_00433700)
 * - 0x1a: Defend (FUN_00433fb0)
 * - 0x1b: Counter (FUN_004344c0)
 * - 0x1c: Pet name storage (copies to DAT_04554270)
 * - 0x1e: Pet recall (FUN_00435450)
 * - 0x1f: Pet switch (FUN_004364e0)
 * - 0x20: Extended UI (FUN_00435a00)
 * - 0x21: Close UI (FUN_00436190)
 * - 0x22-0x27: Ride/dismount state setup
 * - 0x67: Duel request (FUN_00439810)
 * - 0x68: Duel response (FUN_0043a020)
 *
 * Cleanup: Calls FUN_0044aff0 if battle state active
 */
void FUN_00424b70(int param_1, int param_2, int param_3, int param_4, char* param_5) {
    /* Dispatch battle action to appropriate handler */
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

/* FUN_00426cc0 - Battle Item UI
 * Handles item selection UI during battle
 *
 * Window creation (first call only):
 * - Creates window via FUN_00448610(0x60, 0x6c, 7, 5, 0, 1)
 * - Window size: 7 tiles wide, 5 tiles tall
 * - Position: (0x60, 0x6c) = (96, 108)
 *
 * Button layout:
 * - 6 buttons at DAT_004b8420 (initialized to -2 = unassigned)
 * - Button labels from local_218: "OK", "CANCEL", button texts
 * - Buttons positioned based on item count (DAT_0455ef94 bitmask)
 *
 * Item display:
 * - Iterates through inventory (DAT_046274ce array, 0x58c stride)
 * - Each item: name at +0x11, icon, level info
 * - Displays item name, "LV. %d", "MaxHP %d"
 * - Click detection via FUN_0044aba0
 *
 * Input handling:
 * - Button clicks processed via FUN_00421080
 * - Item selection via mouse click (DAT_045f1bc4 & 1)
 * - Returns selection in local_21c (item index 1-7)
 *
 * Protocol output:
 * - Uses FUN_004923a7 to format with 'B' prefix
 * - Calls FUN_0048a200 for string escaping
 * - Sends via FUN_0043b980 (normal) or FUN_0048fdc0 (checksum mode)
 * - Sets DAT_004b83ec = -1 to close UI
 *
 * Cleanup:
 * - FUN_004011c0 destroys window when action complete
 * - Sets DAT_0455ef98 = 0
 */
void FUN_00426cc0(void) {
    /* Battle item selection UI */
}

/* FUN_00427190 - Battle Pet UI
 * Handles pet selection/management UI during battle
 *
 * Window creation (first call only):
 * - Creates window via FUN_00448610(0x60, 0x6c, 7, 5, 0, 1)
 * - Same layout as item UI
 *
 * Pet display:
 * - 5 pet slots (loop iVar8 = 1 to 6)
 * - Pet names from DAT_046308d8 + iVar8 * 0x30
 * - First pet uses DAT_0462bef8 if DAT_04630b24 == 0
 * - Empty slots show nothing (DAT_045566dc = 0)
 *
 * Selection:
 * - Button selection via FUN_00421080 -> local_21c
 * - Pet selection via FUN_0044aba0 click detection -> iVar3
 * - Pet indices: 1-5 are player pets, 6-10 are enemy pets (iVar3 = iVar8 + 5)
 *
 * Protocol output:
 * - Formats selection with FUN_004923a7, FUN_0048a200
 * - Sends pet selection via FUN_0043b980 or FUN_0048fdc0
 * - Bitmask: 1 << button_index for action
 *
 * Display loop:
 * - Centers pet names ((0x10 - name_len) * 9) / 2
 * - Position: x = 0x98 + context, y = (index + 2) * 0x15
 */
void FUN_00427190(void) {
    /* Battle pet selection UI */
}

/* FUN_004276a0 - Battle Escape/Reposition UI
 * Handles escape and pet repositioning during battle
 *
 * Window creation (first call only):
 * - Creates window via FUN_00448610(0x60, 0x6c, 7, 5, 0, 1)
 * - Title from DAT_004b924c
 *
 * Two selection areas:
 * 1. Left side: Character actions (DAT_046274ce array)
 *    - Shows character names with offset 0x11 check
 *    - Click detection at (0x29, y) to (0x197, y+0x14)
 *    - Returns character selection in iVar4
 *
 * 2. Right side: Pet actions (DAT_046308c0 + iVar8 * 0xc)
 *    - Pet names from DAT_046308d8 + iVar8 * 0x30
 *    - Click detection at (0x101, y) to (0x196, y+0x14)
 *    - Returns pet selection in iVar4 = iVar8 + 5
 *
 * Button handling:
 * - 6 buttons at DAT_004b8450
 * - Button mask: 1 << (button_index & 0x1f)
 *
 * Protocol output:
 * - Sends escape/reposition command
 * - Parameter: button_mask for action, target_index for target
 *
 * Display:
 * - Left side: x = 0x2c + context
 * - Right side: x = 0x104 + context
 * - Row height: 0x15 pixels
 */
void FUN_004276a0(void) {
    /* Battle escape/reposition UI */
}

/* FUN_00426380 - Battle Attack UI
 * Handles attack target selection during battle
 *
 * Parameter: param_1 = mode
 *   0: Normal attack mode
 *   1: Special attack with targeting cursor
 *   2: Extended attack mode
 *
 * Window creation (first call only):
 * - Position calculated from DAT_045541dc (width) and DAT_045541a4 (height)
 * - Window dimensions: 0x30 pixel rows, variable columns
 * - X position: ((10 - width) * 0x40) / 2
 * - Y position: (height * -0x30 + 0x1c8) / 2
 *
 * Target grid:
 * - Row count in DAT_0455b0dc = 0x14 (20 rows)
 * - Cell height: (window_height - 0x38) / row_count
 * - Up to 4 visible targets based on DAT_0455ef94 bitmask
 *
 * Button handling:
 * - 6 buttons at DAT_004b83f0
 * - Button positions stored in DAT_04552e44 array
 * - Returns button mask: 1 << (button_index & 0x1f)
 *
 * Target selection (mode 1):
 * - Uses FUN_00420fb0 for cursor positioning
 * - Cursor position: DAT_04552fb0 * 9 offset
 * - Displays target names from DAT_04556678 array (100 bytes each)
 *
 * Protocol output:
 * - Sends attack command via FUN_0043b980 or FUN_0048fdc0
 * - Button mask indicates target selection
 */
void FUN_00426380(int mode) {
    /* Battle attack target selection UI */
}

/* FUN_00426850 - Battle Skill UI
 * Handles skill selection and target selection during battle
 *
 * Window creation (first call only):
 * - Fixed size: 7 tiles wide, 5 tiles tall
 * - Position: (0x60, 0x6c) = (96, 108)
 *
 * Skill display:
 * - Skills stored in DAT_04556678 array (100 bytes per skill)
 * - Up to 10 skills displayed (DAT_0454efcc = 10)
 * - Starting index: DAT_0454f5b0 (for scrolling)
 * - Row height: 0x15 pixels
 *
 * Selection handling:
 * - Button click via FUN_00421080 -> iVar3
 * - Skill click via FUN_0044aba0 -> iVar10
 * - Skill index: (row - scroll_offset) + 1
 *
 * Protocol output:
 * - If cancel (iVar3 == 100): sends with mode 2
 * - If button selected: sends button_mask = 1 << button_index
 * - Target parameter: skill_index
 *
 * Protocol variants:
 * - Normal: FUN_0043b980
 * - Checksum: FUN_0048fdc0
 * - With offset: FUN_0048fdc0 with subtracted parameters
 */
void FUN_00426850(void) {
    /* Battle skill selection UI */
}

/* FUN_0042ce40 - Battle Pet Capture UI
 * Complex state machine for pet capture during battle
 *
 * State machine (DAT_0455a0fc):
 * 0: Initialize -> calls FUN_0042d570, advance to 1
 * 1: Display -> calls FUN_0042d580
 * 10: Capture init -> calls FUN_0042d880, advance to 11
 * 11 (0xb): Wait for input via FUN_0042d890
 *    - Return 1: Send cancel, close UI
 *    - Return 2: Advance to 20 (0x14)
 * 20 (0x14): Pet list init -> calls FUN_0042df30, advance to 21
 * 21 (0x15): Wait for selection via FUN_0042df40
 *    - Return 1: Send capture, update pet slots
 *    - Return 2: Return to state 10
 * 100 (0x64): Extended capture init -> calls FUN_0042e110, advance to 101
 * 101 (0x65): Wait for input via FUN_0042e120
 *    - Return 1: Send cancel, close UI
 *    - Return 2: Advance to 110
 *    - Return 3: Advance to 120
 * 110 (0x6e): Pet list init -> calls FUN_0042df30, advance to 111
 * 111 (0x6f): Wait for selection via FUN_0042df40
 *    - Return 1: Send capture, update pet slots
 *    - Return 2: Return to state 100
 * 120 (0x78): Extended state
 *
 * Pet slot management:
 * - Pet data at DAT_0455b5e0 (0x88 bytes per pet, 104 slots)
 * - Pet count: DAT_04558c98
 * - Selected pet: DAT_0454f25c
 * - Each pet has: name, stats, skills
 *
 * Slot compaction:
 * - When pet captured, slots are compacted
 * - Empty slots (name[0] == 0) are skipped
 * - Pet data copied with stride 0x88
 *
 * Protocol output:
 * - Cancel: FUN_004923a7 with DAT_004b92e8 format
 * - Capture: FUN_004923a7 with pet_id parameter
 */
void FUN_0042ce40(void) {
    /* Battle pet capture state machine */
}

/* FUN_0042acf0 - Battle Pet Capture UI (Simplified)
 * State machine for pet capture during battle (shorter flow)
 *
 * State machine (DAT_0455b0f0):
 * 0: Initialize -> calls FUN_0042bee0, advance to 1
 * 1: Wait for input via FUN_0042bef0
 *    - Return 2: Advance to 10
 * 10: Capture list init -> calls FUN_0042c580, advance to 11
 * 11 (0xb): Wait for selection via FUN_0042c590
 *    - Return 1: Reset to state 0
 *    - Return 2: Advance to 20
 * 20 (0x14): Pet detail init -> calls FUN_0042c8a0, advance to 21
 * 21 (0x15): Wait for confirmation via FUN_0042c8b0
 *    - Return 1: Return to state 10
 *    - Return 2: Advance to 30
 * 30 (0x1e): Final init -> calls FUN_0042cb90, advance to 31
 * 31 (0x1f): Wait for final input via FUN_0042cba0
 *    - Return 1: Send capture command with parameters
 *    - Return 2: Return to state 20
 *
 * Protocol output format:
 * - Uses format string "%d %d %d %d" with 4 parameters
 * - Parameters: capture_index+1, DAT_04556410, DAT_04558c40, sprite_id
 * - Decrements pet counter (DAT_0462bee8) on success
 */
void FUN_0042acf0(void) {
    /* Simplified battle pet capture UI */
}

/* FUN_0042e8f0 - Pet Skill 1 UI
 * Complex pet skill management interface during battle
 *
 * Window creation:
 * - Position: (0x22, 0x5c) = (34, 92)
 * - Size: 0x23b x 0x127 = 571 x 295 pixels
 * - Uses FUN_00448610 with parameter -1 (special mode)
 *
 * UI elements:
 * - Pet portrait area at offset (0x5a, 0x112)
 * - Pet name display from DAT_0462bef8
 * - Skill buttons: 4 sprites (0x6644-0x6647)
 * - Navigation arrows: left/right for pet selection
 *
 * Pet selection:
 * - Index stored in DAT_0455efa8 (0-4 range)
 * - Navigate with button indices 2 (left) and 3 (right)
 * - Pet data array at DAT_046274ce with stride 0x58c
 * - Pet icon at DAT_0462745c with stride 0x2c6
 *
 * Button handlers (via FUN_004210b0):
 * 0: Confirm skill selection
 * 1: Cancel/close
 * 2: Previous pet
 * 3: Next pet
 *
 * Skill modes:
 * - Mode stored in DAT_004b848c (0 or 1)
 * - Mode changes via button index 1
 *
 * Protocol output:
 * - Format: "A %s %d %d %s" (skill name, parameters)
 * - Uses FUN_0043bf40 (text) or FUN_00490420 (binary)
 *
 * Special checks:
 * - DAT_0455ed18: First skill name buffer
 * - DAT_045562a0: Second skill name buffer
 * - DAT_0454de9c: Pet icon entity handle
 */
void FUN_0042e8f0(void) {
    /* Pet skill selection UI (skill slot 1) */
}

/* FUN_00430b50 - Special Battle Action UI
 * Complex multi-target action selection interface
 *
 * Extended mode (DAT_0455effc == 1):
 * - Calls FUN_004322a0 for sub-menu
 * - Uses DAT_0455b22c as target index
 *
 * Window creation (normal mode):
 * - Size: 9 tiles wide, 8 tiles tall
 * - Position: (0x20, 0x24) = (32, 36)
 *
 * Target list:
 * - Up to 11 targets (DAT_045528d0 = 0xb)
 * - Names in DAT_04556678 array (100 bytes each)
 * - Scroll offset: DAT_0454f5b0
 * - Target types in DAT_04556600 array
 *
 * Target types:
 * - -1: Dead/unavailable
 * - 0: Normal
 * - 1: Special target (different color)
 * - 2: Protected target
 * - 4: Highlighted target
 *
 * Click detection:
 * - Uses FUN_0044aba0 for hit testing
 * - Scroll timing check via FUN_00492750
 * - Row 10 has different click zones (extended area)
 *
 * Selection handling:
 * - Selected index stored temporarily in local_634
 * - Values 1-10: Normal targets
 * - Value 11 (0xb): Special toggle (DAT_04556628)
 *
 * Protocol output:
 * - Format: "%d %d %d" (mode, target_index, target_type)
 * - Extended info: "M %s %d %d" via FUN_0043bf40
 * - Toggle notification: Special format from DAT_004b9650
 *
 * Button handling:
 * - 6 buttons at DAT_004b84b4
 * - Button mask: 1 << (button_index & 0x1f)
 * - Cancel returns 2 as action type
 */
void FUN_00430b50(void) {
    /* Special battle action UI */
}

/* FUN_00431560 - Combo Attack UI
 * Handles combo attack selection during battle
 *
 * Window creation:
 * - Position calculated from DAT_045541dc (width) and DAT_045541a4 (height)
 * - Same layout as attack UI (FUN_00426380)
 *
 * Display:
 * - Two-column target display
 * - Left column: target names from DAT_04556934 - 700
 * - Right column: combo partner names from DAT_04556934
 * - Each column has entries every 100 bytes
 * - 30 total entries (0x4556bf0 - 0x4556934) / 100 = 30
 *
 * Target selection:
 * - Combo cursor at DAT_0454f110 (text buffer)
 * - Cursor position via FUN_00420fb0
 * - Cursor initialized to row_count - 2
 *
 * Button handling:
 * - 6 buttons at DAT_004b84cc
 * - Button mask: 1 << (button_index & 0x1f)
 *
 * Protocol output:
 * - Format: "%d %s" (button_mask, target_name)
 * - Target name escaped via FUN_0048a200
 *
 * Display layout:
 * - Left column: (width * 0x40 - cursor * 9) / 2 + x_offset
 * - Right column: same + 300
 * - Row height: cell_height * 2
 */
void FUN_00431560(void) {
    /* Combo attack selection UI */
}

/* FUN_004338d0 - Battle Wait/Defend UI
 * Displays wait action options with party status
 *
 * Window creation:
 * - Fixed size: 8 tiles wide, 8 tiles tall
 * - Position: (0x40, 0x24) = (64, 36)
 *
 * Button layout:
 * - 10 buttons at DAT_004b8584 (initialized to -2)
 * - Buttons 0-5: Standard actions (OK, Cancel, etc.)
 * - Buttons 6-9: Wait type selection
 *
 * Wait types (buttons 6-9):
 * - Data from DAT_04552a18 + (button - 6) * 0x10
 * - Each wait type has a 16-byte string
 *
 * Party status display:
 * - Party count: DAT_04557968
 * - Format: "%d" (party_count / 100, party_count % 100)
 * - Column headers at DAT_004b9a24: "%10s%16s%16s%8s%8s"
 *
 * Party member data:
 * - Array at DAT_04552a4c with stride 0x10
 * - 4 members (0x4552b4c - 0x4552a4c) / 0x10 = 4
 * - Member status at offset 4 (puVar7[1]):
 *   - -1: Empty slot
 *   - 0-1: Normal member
 *   - 2-3: Special status
 *   - 4: Leader/flagged
 *
 * Member display colors:
 * - Empty: Different format (DAT_004b99d0)
 * - Normal: Standard format (DAT_004b99e8, DAT_004b99d8)
 * - Status affects row color (puVar9 parameter)
 *
 * Special mode check:
 * - DAT_0462e3c8 == 3: PvP mode (different display rules)
 *
 * Protocol output:
 * - Standard: FUN_0043b980 with button_mask
 * - Wait type: FUN_0043b980 with wait_type_string
 * - Checksum mode: FUN_0048fdc0
 */
void FUN_004338d0(void) {
    /* Battle wait/defend UI */
}

/* FUN_00434d60 - Pet Summon/Gold Adjustment UI
 * Handles pet summoning with gold payment during battle
 *
 * Window creation:
 * - Size: 5 tiles wide, 4 tiles tall
 * - Position: (0xa0, 0x84) = (160, 132)
 *
 * Button layout:
 * - 4 buttons at DAT_004b8614 (initialized to -2)
 * - Button 0: Confirm summon
 * - Button 1: Cancel
 * - Button 2: Increase gold amount
 * - Button 3: Decrease gold amount
 *
 * Gold adjustment (buttons 2/3):
 * - Current gold: DAT_0454f270
 * - Total gold: DAT_0462bee8 + DAT_0454f3a4
 * - Increment: DAT_0455f01c (increases by 5x each 29 clicks)
 * - Decrement: DAT_0455f020 (increases by 5x each 29 clicks)
 * - Acceleration counter: DAT_0455f024
 * - Max gold: 100,000,000
 *
 * Display:
 * - Pet icon at DAT_0462e3b6
 * - Title: "Summon Pet" (from DAT_004b9b8c)
 * - Current gold display
 * - Amount to pay display
 * - Sprite buttons: 0x664c/0x664e for increment/decrement
 *
 * PvP mode (DAT_0462e3c8 == 3 or 4):
 * - Different sprite set for increment button
 * - Restricted gold adjustment
 *
 * Protocol output:
 * - Confirm: "B T %d" format via FUN_0043bf40 or FUN_00490420
 * - Sound effects: 0xd9 (click), 0xdc (limit)
 */
void FUN_00434d60(void) {
    /* Pet summon with gold payment UI */
}

/* FUN_004354f0 - Pet Recall UI
 * Handles recalling (storing) pets during battle
 *
 * Window creation:
 * - Size: 7 tiles wide, 6 tiles tall
 * - Position: (0x60, 0x54) = (96, 84)
 *
 * Button layout:
 * - 2 buttons at DAT_004b8634
 * - Buttons 0-5: Standard actions
 * - Buttons 6-13: Pet selection (via iVar2 * 3 + 0x12)
 *
 * Pet display:
 * - Pet icons at DAT_0455ee80, DAT_0455ee84, DAT_0455ee88
 * - Icon handles stored in DAT_0454bb4c, DAT_0454bb54, DAT_0454bb44
 * - Pet names from DAT_04553df8 array
 *
 * Selection handling:
 * - Pet index stored via (button - 6) * 3 + 0x12
 * - Each pet has 6 bytes of data (3 shorts)
 * - Total 8 pets selectable (buttons 6-13)
 *
 * Protocol output:
 * - Format: "%d %s" (action_type, pet_name)
 * - Action mask: 1 << (button_index & 0x1f)
 * - Pet name escaped via FUN_0048a200
 *
 * Display layout:
 * - Pet icons at positions (0x73, 0xb4), (0x14f, 0xb4), (0xe1, 0xb4)
 * - Pet names in two columns
 * - Title from DAT_004a145c
 */
void FUN_004354f0(void) {
    /* Pet recall UI */
}

/* FUN_00432a10 - Formation UI
 * Handles battle formation selection (two formation types)
 *
 * Parameter: param_1
 *   0: Formation type A (10 tiles wide, 7 tiles tall)
 *   1: Formation type B (8 tiles wide, 7 tiles tall)
 *
 * Window creation:
 * - Position calculated based on formation type:
 *   - Type A: (10 tiles, 7 tiles)
 *   - Type B: (8 tiles, 7 tiles)
 * - X position: (width * -0x30 + 0x1c8) / 2
 * - Y position: ((10 - width) * 0x40) / 2
 *
 * Button layout:
 * - 6 buttons at DAT_004b852c (initialized to -2 = 0xfffffffe)
 * - Button text from local_218 array:
 *   - [0]: DAT_004b91dc (confirm)
 *   - [1]: s_Cancel (DAT_004b91d4)
 *   - [2-5]: Formation options
 *
 * Formation member display:
 * - Member count from DAT_0455ef94 (bitmask of active members)
 * - Max 4 members per formation
 * - Column positions stored in DAT_04552b58 array
 * - Row height: 0x127 (295) pixels
 * - Each member has width: (window_width << 6) / (member_count + 1)
 *
 * Member data:
 * - Names from DAT_04556678 array (100 bytes per entry)
 * - Max 10 members (DAT_0455a104)
 * - Row spacing: 0x15 (21) pixels
 * - Highlight color: 7 (selected) or 0 (normal)
 *
 * Protocol output:
 * - Format: "%d %d %s" via FUN_0048a200
 * - Button mask: 2 for cancel, 1 << button_index otherwise
 * - Sent via FUN_0043b980 or FUN_0048fdc0
 *
 * Special states checked:
 * - DAT_0455ef98: Window handle (0 = not created)
 * - DAT_04ebe490 & 0x80000000: Invalid state check
 * - DAT_005676a0, DAT_00564e62, DAT_0461b420: Mode flags
 * - DAT_0455ef9c: Battle state flag
 */
void FUN_00432a10(int param_1) {
    /* Formation selection UI */
}

/* FUN_00438080 - Pet Ride Action UI
 * Handles the pet riding system with pet selection
 *
 * State management:
 * - DAT_0455ef38: Ride state (1 = riding, 0 = not riding)
 * - DAT_0455f038: Window handle
 * - DAT_04553200: Current pet name buffer (700 bytes)
 * - DAT_04552d24: Button state array (20 entries)
 *
 * Window creation (first call or DAT_0455f038 == 0):
 * - Size: 0x21c x 0x1c2 pixels (540 x 450)
 * - Position: (0x32, 0xf) = (50, 15)
 * - Skin: 0x669e
 * - Button count: 0x14 (20)
 *
 * Pet slots:
 * - Slot 1: DAT_04552f10 (bit 4 of DAT_0455ef94)
 * - Slot 2: DAT_04552f18 (bit 5 of DAT_0455ef94)
 * - Value 2 = no pet, 0 = pet available
 *
 * Password verification:
 * - XOR key at DAT_0454f3d4 (from DAT_0455703c)
 * - Uses FUN_0048bb90 with key "f;encor1c"
 * - 16-byte key buffer
 *
 * Pet list display:
 * - Pet data at DAT_04553204, stride 0x118 (280 bytes)
 * - Max 5 pets (0x4553cf4 - 0x4553204) / 0x118 = 5
 * - Each pet: name at +0x11, password hash check
 *
 * Scroll handling:
 * - DAT_04554098: Scroll offset
 * - Increment/decrement by 10 per scroll
 * - Update sent to server via FUN_0043b980
 * - Scroll mask: 0x10 (up) or 0x20 (down)
 *
 * Button states:
 * - DAT_04552f00: Confirm button (0=off, 1=hover, 2=normal)
 * - DAT_04552f08: Cancel button
 * - DAT_04552f10: Pet slot 1
 * - DAT_04552f18: Pet slot 2
 * - DAT_04552f20: Close button
 *
 * Protocol output:
 * - Ride confirm: mask 1 or 4
 * - Scroll: mask 0x10 or 0x20
 * - Format via FUN_0048a200 with pet name
 *
 * Sound effects:
 * - 0xca: Window open
 * - 0xd9: Window close
 */
void FUN_00438080(void) {
    /* Pet ride action UI */
}

/* FUN_00438880 - Pet Dismount UI
 * Handles getting off a ridden pet with confirmation
 *
 * State management:
 * - DAT_0455f038: Window handle
 * - DAT_04557978: Pet data buffer (280 bytes, 0x46 entries)
 * - DAT_0454e0a0-0x454e0b4: Button states (5 entries)
 *
 * Window creation:
 * - Size: 0x21c x 0x1c2 pixels (540 x 450)
 * - Position: (0x32, 0xf) = (50, 15)
 * - Skin: 0x66b4
 *
 * Pet info display:
 * - Pet name at DAT_0455798d, copied to DAT_04552be8
 * - Name length stored in DAT_04552cef, DAT_04552cf1
 * - Pet level: DAT_04552cec (default 100)
 * - Pet stats: DAT_04552ced (24), DAT_04552cee (20)
 *
 * Secondary pet:
 * - Name at DAT_04557a03, copied to DAT_0454dea0
 * - Level at DAT_0454dfa4 (default 100)
 * - Stats: DAT_0454dfa5 (28), DAT_0454dfa6 (20)
 *
 * Tertiary pet (if DAT_04557a0c == 2):
 * - Name at DAT_04557a50, copied to DAT_0455ae58
 * - Level at DAT_0455af5c (default 100)
 * - Stats: DAT_0455af5d (28), DAT_0455af5e (20)
 *
 * Button positions:
 * - Dismount confirm: (window_x + 0xe6, window_y + 0x109)
 * - Secondary dismount: (window_x + 0x10e, window_y + 0x181)
 * - Tertiary dismount: (window_x + 0xdc, window_y + 0xc8)
 *
 * Display elements:
 * - Pet portrait sprite: 0x66b1, 0x66b3
 * - Text labels at DAT_045579f2, DAT_04557a10
 * - Pet image handle: DAT_0455f044 (via FUN_00449510)
 *
 * Pet stats display (if DAT_04557a0c == 1):
 * - Stats at DAT_04557a34, DAT_04557a38, DAT_04557a3c, DAT_04557a40, DAT_04557a44, DAT_04557a48
 * - Displayed as decimal via FUN_0049b108
 *
 * Protocol output:
 * - Dismount confirm: mask 1
 * - Format: "%d %s" with pet name
 * - Sent via FUN_0043b980 or FUN_0048fdc0
 *
 * Cleanup:
 * - FUN_004011c0 on window handle
 * - FUN_00411990 on dialog (DAT_0054a3c8)
 * - Clear DAT_0455f044 (pet image)
 */
void FUN_00438880(void) {
    /* Pet dismount UI */
}

/* FUN_0043bf90 - Text Protocol Command Dispatcher
 * Central dispatcher for all text-based protocol commands
 *
 * Parses incoming text commands and routes to appropriate handlers.
 * Uses DAT_004b9ff0 as command buffer, DAT_004b9fec for encoding.
 *
 * Command format: "COMMAND|param1|param2|...\n"
 * - Fields separated by pipe '|' or space
 * - First field is command name
 * - Subsequent fields are parameters
 *
 * Command handlers (40+ commands):
 * - DAT_004b9fe4: Party join -> FUN_00465400
 * - DAT_004b9eb8: Party leave -> FUN_00463e70
 * - DAT_004b9ebc: Party kick -> FUN_00464e10
 * - DAT_004b9fe0: Party invite -> FUN_004643f0
 * - DAT_004b9fdc: Party accept -> FUN_00464610
 * - DAT_004b9ecc: Party reject -> FUN_00464ef0
 * - DAT_004b9fd8: Party change leader -> FUN_00464670
 * - DAT_004b9fd4: Party settings -> FUN_00464650
 * - DAT_004b9ee4: Guild create -> FUN_00465170
 * - DAT_004b9fd0: Guild settings -> FUN_00464af0
 * - DAT_004b9ef0: Guild join -> FUN_00463f00
 * - DAT_004b9fcc: Guild leave -> FUN_00464190
 * - DAT_004b9f00: Guild kick -> FUN_00463790
 * - DAT_004b9fc8: Guild update -> FUN_00462010
 * - DAT_004b9f04: Guild settings2 -> FUN_00462200
 * - DAT_004a2624: Guild info -> FUN_00462590
 * - DAT_004b9fc4: NPC dialog -> FUN_00462f60
 * - DAT_004b9fc0: Shop open -> FUN_00463380
 * - DAT_004b9fbc: Trade start -> FUN_00465460
 * - DAT_004a262c: Battle start -> FUN_0045ffb0
 * - DAT_004a2628: Battle action -> FUN_00465460
 * - DAT_004b9f08: Mail send -> FUN_00463ee0
 * - DAT_004b9f0c: Mail read -> FUN_00464ee0
 * - DAT_004b9f10: Mail delete -> FUN_00463d80
 * - DAT_004b9f14: Mail list -> FUN_00465060
 * - DAT_004b9f20: Friend add -> FUN_00465390
 * - DAT_004b9f28: Friend remove -> FUN_00465160
 * - DAT_004b9f34: Friend list -> FUN_00464ac0
 * - DAT_004b9fb8: Whisper -> FUN_00464db0
 * - DAT_004b9fb4: Chat -> FUN_004653d0
 * - DAT_004b9f3c: ClientLogin -> FUN_0045fa40
 * - DAT_004b9f48: CreateNewChar -> FUN_00463c20
 * - DAT_004b9f58: CharDelete -> FUN_00463d20
 * - DAT_004b9f64: CharLogin -> FUN_0045fdc0
 * - DAT_004b9f70: CharList -> FUN_0045fb80
 * - DAT_004b9f7c: CharLogout -> FUN_0045ff50
 * - DAT_004b9f88: ProcGet -> FUN_00465460
 * - DAT_004b9f90: PlayerNumGet -> FUN_00465460
 * - DAT_004b9fa0: System message -> FUN_00465460
 * - DAT_004b9fb0: Server info -> FUN_00465440
 * - DAT_004b9fa8: Mail notification -> FUN_0045a9a0
 * - DAT_004b9fac: Ping/Pong -> FUN_00465470
 *
 * Parameter parsing:
 * - FUN_0043dd50: Parse integer parameter
 * - FUN_0043dd70: Parse string parameter (Base-62 encoded)
 * - FUN_0043e1b0: Decode string parameter
 *
 * Returns: 0 on success, -1 if no handler found
 */
int FUN_0043bf90(void* socket, const char* data) {
    /* Text protocol command dispatcher */
    return 0;
}

/* FUN_00439310 - Validate and Normalize Numeric String
 * Validates string contains only digits, replaces non-digits with '0'
 *
 * Parameters:
 * - param_1: String buffer to validate (modified in place)
 * - param_2: Maximum value (-1 for check against current gold)
 *
 * Processing:
 * - Iterates through string, finds length
 * - Replaces any non-digit characters ('0'-'9') with '0'
 * - Validates resulting number against maximum
 *
 * Value validation:
 * - If param_2 < 0: Compare against FUN_004792c0() result (current gold)
 * - If param_2 >= 0: Compare against param_2 directly
 * - If value exceeds maximum, reset to DAT_004aa1c4 (likely "0")
 *
 * Used by: FUN_00438880 (dismount UI) for numeric input validation
 */
void FUN_00439310(char* param_1, int param_2) {
    /* Validate numeric string in place */
}

/* FUN_00438f70 - Parse Pet Data from Protocol String
 * Parses pipe-delimited pet data into structured format
 *
 * Parameters:
 * - param_1: Mode (0 = multiple pets, 1 = single pet with stats)
 * - param_2: Output buffer for parsed pet data
 *
 * Mode 0 (multiple pets):
 * - Parses 10 pet entries from DAT_0455ef34
 * - Each entry: field1 (int), field2 (17 bytes), field3 (101 bytes)
 * - Stride: 0x118 (280 bytes) per pet
 *
 * Mode 1 (single pet with full stats):
 * - Parses from DAT_0455ef34 with 6+ fields
 * - Field 1: Pet ID (int)
 * - Field 2: Pet name (17 bytes) at offset 0x7a
 * - Field 3: Pet nickname (101 bytes) at offset 0x15
 * - Field 4: Pet level (8 bytes) at offset 0x8b
 * - Field 5: Pet type (int) at offset 0x25
 * - If type == 1: Parse additional stats (fields 6-8 for 7 values)
 * - Otherwise: Parse description string (field 4, 64 bytes)
 *
 * Data source:
 * - DAT_0455ef34: Raw pipe-delimited string from server
 * - Uses FUN_00489f70 for field extraction
 * - Uses FUN_0048a170 for string unescaping
 * - Uses FUN_004929fe for string-to-int conversion
 */
void FUN_00438f70(int param_1, void* param_2) {
    /* Parse pet data from protocol string */
}

/* FUN_00449510 - Create Pet Portrait Entity
 * Creates an entity for displaying pet portrait in UI
 *
 * Parameters:
 * - param_1: X position
 * - param_2: Y position
 * - param_3: Sprite/image ID
 * - param_4: Mode (0 = standard, 2 = with fade effect)
 *
 * Entity creation:
 * - Calls FUN_004010a0(3, 0) to allocate entity
 * - Sets entity type flag: *(entity + 0xa0) |= 4
 * - Sets callback to LAB_00448780
 * - Sets sprite at offset 0x140
 * - Sets position at offset 0x18 (x) and 0x1c (y)
 *
 * Mode-specific settings:
 * - Mode 0: Standard display, state 0xc at offset 0x2c
 * - Mode 2: Fade effect, state 0x1 at offset 0x2c, timer 0x1e at offset 0x130
 *
 * Returns: Entity handle or 0 on failure
 */
int FUN_00449510(int param_1, int param_2, int param_3, int param_4) {
    /* Create pet portrait entity */
    return 0;
}

/* FUN_0041d890 - Draw Multi-line Text with Highlighting
 * Renders text with support for line wrapping and cursor highlighting
 *
 * Parameter: param_1 - Text layout structure
 *   - offset 0x00: Text string (null-terminated)
 *   - offset 0x105: Character width per line
 *   - offset 0x106: Line height
 *   - offset 0x107: Indent character count
 *   - offset 0x108: Color/flags
 *   - offset 0x109: Cursor position (character index)
 *   - offset 0x10c: X position
 *   - offset 0x110: Y position
 *   - offset 0x11c: Color
 *   - offset 0x120: Mode (1 = use asterisks)
 *
 * Processing:
 * - If character count > 0x3ff: Mark as invalid (-2, -1 markers)
 * - If offset 0x105 == 0: Register text in global list
 * - Otherwise: Multi-line rendering with line breaks
 *
 * Text registration (offset 0x105 == 0):
 * - Stores position in DAT_005676f8 array
 * - Stores dimensions in DAT_005676fa array
 * - If mode == 1: Fill with asterisks for password masking
 * - Increments DAT_005ab6f8 counter
 *
 * Multi-line rendering:
 * - Splits text by character width (param_1[0x105])
 * - Renders each line with proper positioning
 * - Highlights line containing cursor position
 * - Uses FUN_0041d860 for actual text rendering
 *
 * Cursor tracking:
 * - Calculates cursor pixel position from character index
 * - Stores in offset 0x114 (x) and 0x118 (y)
 */
void FUN_0041d890(char* param_1) {
    /* Draw multi-line text with highlighting */
}

/* FUN_0044aba0 - Check Click Within Rectangle
 * Tests if mouse click is within a rectangular region
 *
 * Parameters:
 * - param_1: Left edge (x1)
 * - param_2: Top edge (y1)
 * - param_3: Right edge (x2)
 * - param_4: Bottom edge (y2)
 * - param_5: Sprite/surface index (or -1 for no render)
 *
 * Check conditions:
 * - DAT_045f1b98: Mouse X position
 * - DAT_045f1b9c: Mouse Y position
 * - Validates: x1 <= mouseX <= x2 AND y1 <= mouseY <= y2
 *
 * If param_5 >= 0 and click is inside:
 * - Calls FUN_0047e640 to render highlight sprite
 * - Uses sprite from DAT_045f1bec
 *
 * Returns: 1 if click inside rectangle, 0 otherwise
 */
int FUN_0044aba0(int param_1, int param_2, int param_3, int param_4, int param_5) {
    /* Check if click is within rectangle */
    return 0;
}

/* FUN_0047e210 - Add Sprite to Render Queue
 * Core sprite rendering function, adds sprite to display queue
 *
 * Parameters:
 * - param_1: X position (pixel coordinate)
 * - param_2: Y position (pixel coordinate)
 * - param_3: Layer/depth (affects render order)
 * - param_4: Sprite ID (negative values are special)
 * - param_5: Additional flags/mode
 *
 * Queue management:
 * - Maximum queue size: 0x1000 (4096) entries
 * - Returns -2 if queue is full
 * - Returns queue index on success
 *
 * Queue entry size: 0x18 (24) bytes per entry
 * - Offset 0x00: Layer (param_3)
 * - Offset 0x04: X position
 * - Offset 0x08: Y position
 * - Offset 0x0c: Sprite ID
 * - Offset 0x10: Flags
 * - Offset 0x14: Layer mode (for sorting)
 *
 * Layer processing:
 * - Values -1 to 99: Reserved/error range
 * - Values 100+: Sprite lookup via FUN_0041fad0 and FUN_0041f900
 *   - Returns sprite dimensions (width, height)
 *
 * Layer mode categorization (param_5):
 * - 0-9: Mode 0 (normal)
 * - 10-19: Mode 1, adjusted to 0-9
 * - 20-29: Mode 2, adjusted to 0-9
 * - 30-39: Mode 3, adjusted to 0-9
 * - 40-49: Mode 4, adjusted to 0-9
 * - 50+: Mode 0 (default)
 *
 * Global state:
 * - DAT_0464f488: Queue counter
 * - DAT_0464b488: Queue index array
 * - DAT_0464b48a: Layer array
 * - DAT_04633488: X position array
 * - DAT_0463348c: Y position array
 * - DAT_04633490: Sprite ID array
 * - DAT_04633494: Unknown (always 0)
 * - DAT_04633498: Flags array
 * - DAT_0463349c: Layer mode array
 *
 * Returns: Queue index on success, -2 on failure
 */
int FUN_0047e210(int param_1, int param_2, int param_3, int param_4, int param_5) {
    /* Add sprite to render queue */
    return -2;
}

/* FUN_0041fad0 - Get Sprite Image Index
 * Looks up sprite image index from sprite ID
 *
 * Parameters:
 * - param_1: Sprite ID (0-549999)
 * - param_2: Output pointer for image index
 *
 * Lookup ranges:
 * - 0-499999: Look up in DAT_00a04c64 table (stride 4)
 * - 500000-549999: Return sprite ID directly (identity mapping)
 * - 550000+: Invalid, return 0
 *
 * Returns: 1 on success, 0 on failure
 */
int FUN_0041fad0(unsigned int param_1, unsigned int* param_2) {
    /* Get sprite image index from ID */
    return 0;
}

/* FUN_0041f900 - Get Sprite Dimensions
 * Looks up sprite width and height from sprite ID
 *
 * Parameters:
 * - param_1: Sprite ID (0-549999)
 * - param_2: Output pointer for width
 * - param_3: Output pointer for height
 *
 * Lookup ranges:
 * - 0-499999: Look up in DAT_00e8f234 table (stride 0x50)
 *   - Width at offset 0, height at offset 4
 * - 500000-550000: Look up in DAT_0081c7f4 table
 *   - Uses formula: (param_1 * 5 - 2500000) * 8
 * - 550000+: Invalid, return 0 for both dimensions
 *
 * Returns: 1 on success, 0 on failure
 */
int FUN_0041f900(unsigned int param_1, unsigned short* param_2, unsigned short* param_3) {
    /* Get sprite dimensions from ID */
    return 0;
}

/* FUN_00488190 - Play Sound Effect
 * Plays a sound effect by index with volume and pan settings
 *
 * Parameter: param_1 - Sound effect index (0-500)
 *
 * Sound table:
 * - DAT_04cb84d0: Sound data handles (index * 8)
 * - DAT_04cb84e4: Sound state (0 = not loaded, 5 = playing)
 * - DAT_04cb84dc: Sound duration
 * - DAT_04cb84e0: Sound data offset
 * - DAT_04cb84e8: Sound data size
 *
 * Volume calculation:
 * - Uses DAT_004d36dc (master volume) / 15
 * - Scaled by sound duration (0x7f range)
 * - DAT_004d36d8: Pan setting (0 = center, else calculated)
 *
 * DirectSound interface:
 * - Creates sound buffer from DAT_04cb743c entry
 * - Sets volume via offset 0x3c method
 * - Sets pan via offset 0x40 method
 * - Sets data via offset 0x44 method
 * - Plays via offset 0x30 method
 *
 * Channel management:
 * - DAT_04ebe294: Current channel index (0-63)
 * - DAT_04cb834c: Channel handles array
 * - Skips occupied channels
 *
 * Returns: 0 on success, -1 on failure
 */
int FUN_00488190(int param_1) {
    /* Play sound effect */
    return -1;
}

/* FUN_0041d860 - Draw Text String
 * Renders text string at specified position with color
 *
 * Parameters:
 * - param_1: X position
 * - param_2: Y position
 * - param_3: Layer/depth
 * - param_4: Color/style
 * - param_5: Text string pointer
 * - param_6: Additional flags
 *
 * This is a wrapper that calls FUN_0041d7c0 with last parameter 0
 */
void FUN_0041d860(int param_1, int param_2, int param_3, int param_4, const char* param_5, int param_6) {
    /* Draw text string - wrapper for FUN_0041d7c0 */
}

/* FUN_0048a200 - Escape String for Protocol
 * Escapes special characters in string for network transmission
 *
 * Parameters:
 * - param_1: Source string
 * - param_2: Destination buffer
 * - param_3: Buffer size
 *
 * Escape table at DAT_004d5828:
 * - Characters that need escaping (source bytes)
 * - Mapped to escape sequences at DAT_004d5829
 *
 * Processing:
 * - Iterates through source string
 * - Uses IsDBCSLeadByte for DBCS awareness
 * - For SBCS: Check if character needs escaping
 *   - If match found: output backslash (0x5c) + escape char
 *   - Otherwise: copy character as-is
 * - For DBCS: Copy both bytes unchanged
 *
 * Buffer management:
 * - Stops if buffer would overflow (param_3 check)
 * - Null-terminates result
 *
 * Escape sequences (from DAT_004d5828/DAT_004d5829):
 * - Specific characters mapped to protocol-safe equivalents
 */
void FUN_0048a200(const char* param_1, char* param_2, int param_3) {
    /* Escape string for protocol transmission */
}

/* FUN_0047e640 - Render Sprite with Blend Mode
 * Renders sprite with specified blending/transparency mode
 *
 * Parameters:
 * - param_1: X position 1 (left)
 * - param_2: Y position 1 (top)
 * - param_3: X position 2 (right)
 * - param_4: Y position 2 (bottom)
 * - param_5: Sprite ID
 * - param_6: Base sprite flags
 * - param_7: Blend mode
 *
 * Coordinate packing:
 * - param_3: Combined as (param_1 << 16) | param_3
 * - param_4: Combined as (param_2 << 16) | param_4
 *
 * Blend modes (param_7):
 * - 0: Additive alpha (0xa0000000 flag)
 * - 1: Subtractive alpha (0x90000000 flag)
 * - 2: Custom blend (0xc0000000 flag)
 * - Other: No additional flags
 *
 * Calls FUN_0047e210 for actual sprite rendering
 */
void FUN_0047e640(int param_1, int param_2, unsigned int param_3, unsigned int param_4,
                  int param_5, unsigned int param_6, int param_7) {
    /* Render sprite with blend mode */
}

/* FUN_00411900 - Get Last Character Width
 * Returns the byte width of the last character in a DBCS string
 *
 * Parameter: param_1 - Null-terminated string
 *
 * Processing:
 * - Iterates through string using IsDBCSLeadByte
 * - For DBCS characters: advances 2 bytes, returns 2 at end
 * - For SBCS characters: advances 1 byte, returns 1 at end
 *
 * Returns:
 * - 0: Empty string
 * - 1: Last character is single-byte
 * - 2: Last character is double-byte (DBCS)
 * - 3: Incomplete DBCS sequence at end (lead byte without trail)
 *
 * Used by: FUN_0041d890 for proper text wrapping
 */
int FUN_00411900(const unsigned char* param_1) {
    /* Get last character byte width */
    return 0;
}

/* FUN_004792c0 - Calculate Gold Limit
 * Returns maximum gold based on VIP level
 *
 * Formula: DAT_0462e3b4 * 1800000 + 1000000
 *
 * VIP gold limits:
 * - VIP 0: 1,000,000 gold
 * - VIP 1: 2,800,000 gold
 * - VIP 2: 4,600,000 gold
 * - etc. (1.8M per VIP level)
 *
 * Returns: Maximum gold the player can hold
 */
int FUN_004792c0(void) {
    /* Calculate gold limit from VIP level */
    return 1000000;
}

/* FUN_00411990 - Dialog/UI cleanup */
 * - DAT_045f1b9c: Mouse Y position
 * - Validates: x1 <= mouseX <= x2 AND y1 <= mouseY <= y2
 *
 * If param_5 >= 0 and click is inside:
 * - Calls FUN_0047e640 to render highlight sprite
 * - Uses sprite from DAT_045f1bec
 *
 * Returns: 1 if click inside rectangle, 0 otherwise
 */
int FUN_0044aba0(int param_1, int param_2, int param_3, int param_4, int param_5) {
    /* Check if click is within rectangle */
    return 0;
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

/* FUN_00412a40 - DirectDraw primary surface restoration
 * Restores lost DirectDraw surfaces and resets state
 *
 * Operations:
 * - Initializes local buffer (100 bytes at offset 0)
 * - Sets restoration rect to DAT_01000400
 * - Calls surface restore via vtable +0x14 on DAT_0054a90c+0xc
 * - If alpha mode (DAT_0054c83c), also restores secondary surface
 */
void FUN_00412a40(void) {
    /* DirectDraw surface restoration */
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

/* FUN_00401170 - Field entity update loop
 * Iterates through entity linked list and processes each entity
 *
 * Structure:
 * - Entity list at DAT_004d7e3c, ends at DAT_004d7e38
 * - Each entity has: prev at +0, next at +4, callback at +8
 * - State flag at offset +0x24 (9*4): 0=active, else=delete
 *
 * Operations:
 * - If state==0: Call callback at +0x8 if not NULL
 * - If state!=0: Remove from list, call FUN_004011d0 to free
 */
void FUN_00401170(void) {
    /* Entity list iteration with callback dispatch */
}

/* FUN_004010a0 - Entity Allocation and List Insertion
 * Allocates a new entity and inserts it into the sorted linked list
 *
 * Parameters:
 * - param_1: Entity type/priority (determines sort order)
 * - param_2: Extra data size (0 for no extra allocation)
 *
 * Entity structure (500 bytes base):
 * - Offset 0x00: prev pointer
 * - Offset 0x04: next pointer
 * - Offset 0x08: callback function pointer
 * - Offset 0x0c: extra data pointer (if param_2 != 0)
 * - Offset 0x10: unknown
 * - Offset 0x14: entity type (param_1)
 * - Offset 0x18-0x1c: position data
 * - Offset 0x24: delete flag (0=active, 1=delete)
 * - Offset 0x27: current state (-1 initially)
 *
 * Memory allocation:
 * - Base: 500 bytes via FUN_00491f70(1, 500)
 * - Extra: param_2 bytes if specified
 * - Shows error MessageBox on failure
 *
 * List insertion:
 * - Inserts in sorted order by entity type (param_1)
 * - Lower type values inserted earlier in list
 * - List head at DAT_004d7e3c, tail at DAT_004d7e38
 *
 * Returns: Entity pointer or NULL on failure
 */
int* FUN_004010a0(unsigned char param_1, int param_2) {
    /* Allocate entity and insert into sorted list */
    return NULL;
}

/* FUN_00448610 - Window Widget Creation (9-Sprite Grid)
 * Creates a window widget with 9-sprite grid layout for UI
 *
 * Parameters:
 * - param_1: X position (left edge)
 * - param_2: Y position (top edge)
 * - param_3: Width in tiles
 * - param_4: Height in tiles
 * - param_5: Sprite ID for window skin
 * - param_6: Window style/mode (0-4 or -1)
 *
 * Entity creation:
 * - Calls FUN_004010a0(3, 0x40) for 64-byte extra data
 * - Sets callback to FUN_00448270
 * - Entity type flag: *(entity + 0xa0) |= 6
 *
 * Window modes (param_6):
 * - 0: Standard window with DAT_0054b194 skin
 * - 1: Alternate window with DAT_0054c208 skin
 * - 2: Standard window (same as 0)
 * - 3: Alternate window (same as 1)
 * - 4: Special mode with 0x6d sprite, no borders
 * - -1: Custom window with param_5 sprite
 *
 * Window dimensions:
 * - Width in pixels: param_3 << 6 (tiles * 64)
 * - Half-width used for centering: (width) / 2
 * - Height scaled by 0x30 for certain modes
 *
 * Extra data layout (64 bytes at entity+0xc):
 * - [0]: Width (param_3)
 * - [1]: Height (param_4)
 * - [2]: Sprite ID (param_5)
 * - [3]: X position right edge
 * - [4]: Y position bottom edge
 * - [5]: Center X
 * - [6]: Center Y
 * - [10]: Window skin sprite
 * - [11]: Draw mode (1 or 0)
 * - [14]: Corner sprite ID (0x68 or 0x6d)
 * - [15]: Always -1
 *
 * Returns: Entity handle or 0 on failure
 */
int FUN_00448610(int param_1, int param_2, int param_3, int param_4, int param_5, int param_6) {
    /* Create window widget with 9-sprite grid */
    return 0;
}

/* FUN_00448270 - Window Render Callback
 * Renders a window widget using 9-sprite grid system
 *
 * Parameter: param_1 - Entity pointer
 *
 * State machine (entity + 0xa8):
 * - State 0: Opening animation (expand from center)
 * - State 1: Complete/hidden
 * - State 2: Normal render (9-sprite grid)
 * - State 3: Minimal mode (close button only)
 *
 * State 0 - Opening Animation:
 * - Uses FUN_0047e640 to render expanding rectangle
 * - Expands by offsets at entity+0x28 (x) and entity+0x2c (y)
 * - Animation counter at extra_data[9]
 * - After 10 steps, transitions to state 1 or 2
 *
 * State 2 - 9-Sprite Grid Render:
 * Sprite offsets from base (piVar1[10]):
 * - +0: Top-left corner
 * - +1: Top edge
 * - +2: Top-right corner
 * - +3: Left edge
 * - +4: Center fill
 * - +5: Right edge
 * - +6: Bottom-left corner
 * - +7: Bottom edge
 * - +8: Bottom-right corner
 *
 * Grid positions:
 * - Tile size: 0x40 (64) pixels wide, 0x30 (48) pixels tall
 * - Row 0: Corners at offsets 0, 1, 2
 * - Row 1-(n-1): Edges at offsets 3, 4, 5
 * - Row (n-1): Corners at offsets 6, 7, 8
 *
 * Alpha mode (DAT_0054c83c):
 * - Affects sprite selection for transparency effects
 * - Uses different skin variants from DAT_0054b194 or DAT_0054c208
 *
 * Extra data access:
 * - piVar1[0]: Width in tiles
 * - piVar1[1]: Height in tiles
 * - piVar1[2]: Optional sprite overlay
 * - piVar1[5-6]: Center position
 * - piVar1[7-8]: Animation offsets
 * - piVar1[10]: Base skin sprite
 * - piVar1[11]: Draw mode
 * - piVar1[12-13]: Button sprite handles (state 3)
 * - piVar1[14]: Corner sprite (0x68 or 0x6d)
 * - piVar1[15]: Hover state
 */
void FUN_00448270(int param_1) {
    /* Window render callback - 9-sprite grid */
}

/* FUN_004011d0 - Free Entity
 * Frees an entity and its extra data
 *
 * Parameter: param_1 - Entity pointer
 *
 * If entity has extra data (offset 0xc != 0):
 * - Frees extra data via FUN_00491fed
 * - Then frees entity itself via FUN_00491fed
 */
void FUN_004011d0(int param_1) {
    /* Free entity and extra data */
}

/* FUN_00491f70 - Memory Allocation
 * Allocates memory with 16-byte alignment
 *
 * Parameters:
 * - param_1: Count of elements
 * - param_2: Size of each element
 *
 * Total size calculation:
 * - If size < 0xffffffe1: Round up to 16-byte boundary
 * - If size == 0: Use minimum 1 byte
 *
 * Allocation strategy:
 * 1. If size <= DAT_004d7414 threshold: Try pool allocator (FUN_00493c53)
 * 2. Otherwise: Use HeapAlloc with HEAP_ZERO_MEMORY (flag 8)
 * 3. On failure: Call garbage collector (FUN_00493868) and retry
 *
 * Returns: Pointer to zero-initialized memory, or NULL on failure
 */
void* FUN_00491f70(int param_1, int param_2) {
    /* Allocate aligned memory */
    return NULL;
}

/* FUN_00491fed - Free Memory
 * Frees memory allocated by FUN_00491f70
 *
 * Parameter: param_1 - Memory pointer to free
 *
 * Processing:
 * - NULL check: returns immediately if pointer is NULL
 * - Try pool deallocator FUN_004938fd first
 * - If not from pool, use HeapFree on DAT_04ec0f9c
 */
void FUN_00491fed(void* param_1) {
    /* Free allocated memory */
}

/* FUN_0044b030 - Check Game/Battle State
 * Returns current game state flags for UI updates
 *
 * Checks multiple state variables:
 * - DAT_045e19b0: Battle counter (active if > 0)
 * - DAT_004e21dc: Secondary counter (active if > 0)
 * - DAT_045e8ce0: Battle state flag
 * - DAT_045f1a3b: Override state flag
 * - DAT_045f1a3a: Primary state flag
 *
 * Priority:
 * 1. If DAT_045f1a3a != 0, return its value
 * 2. If DAT_045f1a3b != 0, return its value
 * 3. Otherwise return combined flags
 *
 * Returns: State value (0 = normal, non-zero = special state)
 */
char FUN_0044b030(void) {
    /* Check game/battle state flags */
    return 0;
}

/* FUN_004011c0 - Mark Entity for Deletion
 * Sets the delete flag on an entity for cleanup
 *
 * Parameter: param_1 - Entity pointer
 *
 * Simply sets *(entity + 0x24) = 1
 * Entity will be freed on next FUN_00401170 iteration
 */
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

/* FUN_00421080 - Find Index by Value in Array
 * Searches integer array for matching value
 *
 * Parameters:
 * - param_1: Integer array to search
 * - param_2: Array length
 *
 * Search value: DAT_045f1b90 (current selection/target ID)
 * Search condition: (DAT_045f1bc4 & 1) must be set
 *
 * Returns: Index of match, or -1 if not found or disabled
 */
int FUN_00421080(int* param_1, int param_2) {
    /* Find index by matching DAT_045f1b90 */
    return -1;
}

/* FUN_0044b030 - Battle UI active check (duplicate - see earlier definition) */
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

/* FUN_0045e880 - Network I/O Loop
 * Core network loop handling send/receive operations
 *
 * Socket: gSocket (global socket handle)
 * Buffer: DAT_045f1bf8 (8KB receive buffer)
 * Send buffer: gBuffer (16KB at DAT_0461b41c)
 *
 * Operations:
 * 1. Check connection state (DAT_0461b3f8, DAT_0461b420)
 * 2. Use select() to check for pending data
 * 3. recv() data into buffer (max 0x1fff bytes)
 * 4. Process received data via FUN_0045ec80
 * 5. Send pending data from gBuffer
 * 6. Handle heartbeat (30 second timeout)
 *
 * Large packet handling:
 * - If packet is 200-0x1127 bytes: Sleep(2000), try recv again
 * - Sets DAT_0461b414 flag for large packets
 *
 * Protocol mode selection:
 * - DAT_04ebffd8 == 3: Binary protocol via FUN_0043bf90
 * - Otherwise: Text protocol via FUN_0048a6f0
 *
 * Heartbeat:
 * - Sent every 30 seconds (30000ms)
 * - Binary mode: FUN_0043bea0
 * - Checksum mode: FUN_004901f0
 * - Counter: DAT_005ab714
 *
 * Error handling:
 * - WSAEWOULDBLOCK (0x2733): Non-blocking, continue
 * - Other errors: Close socket, set DAT_0461b420 = 1
 */
void FUN_0045e880(void) {
    /* Core network I/O loop with select/recv/send */
}

/* FUN_0045f4d0 - Connection Handler
 * Manages server connection establishment and handshake
 *
 * Connection states (DAT_0461c008):
 * - 0: Initial state, start connection
 * - 1-70: Wait for connection
 * - 71-80: Send handshake packet
 * - 81-98: Wait for response
 * - 99: Connected
 *
 * Connection process:
 * 1. Resolve hostname via FUN_0045e820
 * 2. Create socket with socket(AF_INET, SOCK_STREAM, 0)
 * 3. Set non-blocking mode via ioctlsocket
 * 4. Call connect() - may return WSAEWOULDBLOCK
 * 5. Wait for connection completion
 * 6. Send encryption key packet
 *
 * Server info:
 * - DAT_004c4288: Server address string
 * - Port from DAT_04557040, DAT_0455703c
 *
 * Encryption setup (state 0x47):
 * - XOR key: "f;encor1c" via FUN_0048bb90
 * - Sends encrypted handshake via FUN_0043bb10 or FUN_0048ff00
 * - Key stored in DAT_04557040-DAT_04557048
 *
 * Timeout: 600000ms (10 minutes) for connection
 *
 * Returns: 0 on success, negative error code on failure
 */
int FUN_0045f4d0(void) {
    /* Handle connection establishment and handshake */
    return 0;
}

/* FUN_0045ffb0 - Binary Packet Dispatcher
 * Dispatches binary protocol packets to appropriate handlers
 *
 * Parameters:
 * - param_1: Socket handle
 * - param_2: Packet data (first byte is opcode)
 *
 * Opcode handlers (case values):
 * - 0x42 ('B'): Inventory update
 * - 0x4b ('K'): Character stats bitmask
 * - 0x4e ('N'): Equipment data
 * - 0x50 ('P'): Full character stats
 * - 0x53 ('S'): Party data
 * - 0x57 ('W'): Mail data
 * - 0x58 ('X'): Skill list
 * - 0x59 ('Y'): Pet list
 * - 0x5a ('Z'): Quest list
 * - 0x5c ('\'): Storage data
 * - 0x5d (']'): Album data
 * - 0x5e ('^'): Ranking data
 * - 0x5f ('_'): Buff data
 *
 * Processing:
 * - Each opcode has specific field parsing
 * - Uses FUN_00489f70 for field extraction
 * - Uses FUN_0048a170 for string unescaping
 * - Updates corresponding global data structures
 *
 * Data structures:
 * - DAT_046274d4: Character data (0xb18 bytes per character)
 * - DAT_0462bf34: Pet data array
 * - DAT_04630df0: Battle state
 */
void FUN_0045ffb0(int param_1, unsigned char* param_2) {
    /* Dispatch binary packet to handler based on opcode */
}

/* FUN_0045ec80 - Append Data to Receive Buffer
 * Appends received data to the protocol buffer
 *
 * Parameters:
 * - param_1: Source data pointer
 * - param_2: Number of bytes to append
 *
 * Buffer: DAT_0461b41c (16KB)
 * Current position: DAT_0461b408
 * Max size: 0x4000 (16384 bytes)
 *
 * Processing:
 * - Check if DAT_0461b3fc != 0 (connection active)
 * - Check if new size would exceed 0x4000
 * - Copy data in 4-byte chunks, then remaining bytes
 * - Update DAT_0461b408 with new position
 *
 * Returns: 0 on success, -1 on buffer overflow, -100 on not connected
 */
int FUN_0045ec80(void* param_1, unsigned int param_2) {
    /* Append data to receive buffer */
    return 0;
}

/* FUN_0045ee40 - Extract Line from Buffer
 * Extracts a newline-terminated line from receive buffer
 *
 * Parameters:
 * - param_1: Output buffer for extracted line
 * - param_2: Maximum output size
 *
 * Processing:
 * - Search for '\n' in buffer
 * - Copy data up to newline to output
 * - Null-terminate output
 * - Strip trailing '\r' if present
 * - Remove extracted data from buffer via FUN_0045ed80
 *
 * Returns: 0 on success, -1 if no newline found or buffer empty
 */
int FUN_0045ee40(void* param_1, int param_2) {
    /* Extract line from receive buffer */
    return -1;
}

/* FUN_0045ede0 - Remove Sent Data from Send Buffer
 * Advances send buffer position after successful send
 *
 * Parameter: param_1 - Number of bytes sent
 *
 * Buffer: gBuffer (global send buffer)
 * Length: len (global variable)
 *
 * Processing:
 * - If bytes_sent < total_length: Shift remaining data to front
 * - Decrease len by bytes_sent
 * - If bytes_sent >= total_length: Just clear length
 *
 * Returns: 0 on success, -1 if bytes_sent > total_length
 */
int FUN_0045ede0(int param_1) {
    /* Remove sent bytes from send buffer */
    return 0;
}

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

/* FUN_004017a0 - Pet AI Settings Loader
 * Loads pet AI configuration from AISetting.dat file
 *
 * Processing:
 * 1. Opens "AISetting.dat" via FUN_00492d40
 * 2. Reads 4-byte entries in loop via FUN_00493240
 * 3. Each entry: [command_id:2][target_id:2]
 * 4. Stops when FUN_00493260 returns EOF indicator
 * 5. Closes file via FUN_00493270
 *
 * Data storage:
 * - DAT_04552904: AI command array (command IDs)
 * - DAT_04552908: AI target array (target IDs)
 * - DAT_04552900: Entry count (max 256 entries)
 *
 * Entry format (4 bytes each):
 * - Offset 0-1: Command ID (AI action type)
 * - Offset 2-3: Target ID (action parameter)
 *
 * AI commands (command_id values):
 * - 0: No action / idle
 * - 1: Attack nearest enemy
 * - 2: Attack lowest HP enemy
 * - 3: Attack highest threat enemy
 * - 4: Use skill on enemy
 * - 5: Use skill on ally
 * - 6: Defend owner
 * - 7: Flee
 * - 8: Follow owner
 * - 9-255: Extended AI behaviors
 *
 * File format:
 * - Binary file with 4-byte records
 * - No header, just raw entries
 * - Typically 50-100 entries for game AI
 *
 * Returns: 0 on success, non-zero on file error
 */
int FUN_004017a0(void) {
    /* Load pet AI settings from AISetting.dat */
    return 0;
}

/* FUN_00425bb0 - Pet Capture Action Handler
 * Handles pet capture during battle, parsing pet data from protocol
 *
 * Parameters:
 * - param_1: Pet index in capture list
 * - param_2: Capture mode (0 = normal, 1 = forced)
 *
 * Processing:
 * 1. Get pet count from DAT_04556420
 * 2. Validate pet index (1 to pet_count)
 * 3. Retrieve pet sprite ID from DAT_04556400 array
 * 4. Store pet name from DAT_04556678 to DAT_04554270
 * 5. Calculate capture parameters via FUN_00425dc0
 *
 * Pet capture data:
 * - DAT_04556400: Pet sprite ID array (100 bytes per entry)
 * - DAT_04556678: Pet name buffer (100 bytes per entry)
 * - DAT_04556420: Number of capturable pets
 * - DAT_04554270: Selected pet name output buffer
 * - DAT_04556410: Selected pet sprite ID
 *
 * Capture validation:
 * - pet_index must be > 0 and <= DAT_04556420
 * - If invalid: returns without action
 * - If valid: stores selection for capture confirmation
 *
 * Protocol output:
 * - After selection, capture command sent via FUN_0043b980
 * - Format: "capture %d %s" (pet_index, pet_name)
 * - Server validates and returns capture result
 *
 * Called by: FUN_00424b70 (action dispatcher) for action type 9
 *
 * Returns: 0 on success, -1 if invalid pet index
 */
int FUN_00425bb0(int param_1, int param_2) {
    /* Handle pet capture action during battle */
    return 0;
}

/* FUN_0044a100 - Album Data Loader with XOR Decryption
 * Loads and decrypts pet album data files
 *
 * Processing:
 * 1. Iterates through 36 album data files (indices 0-35)
 * 2. Constructs filename: "data/album%d.dat" via FUN_004923a7
 * 3. Opens each file via FUN_00492d40
 * 4. Reads encrypted data via FUN_00493240
 * 5. Decrypts using XOR cipher via FUN_00449e00
 * 6. Stores decrypted data in album array
 *
 * File format (each album file):
 * - Encrypted with XOR cipher
 * - Key: The encryption key used by FUN_00449e00
 * - Contains pet album entries
 *
 * XOR decryption (FUN_00449e00):
 * - Key stored in DAT_0455ef9c
 * - Iterates bytes: decrypted[i] = encrypted[i] ^ key[i % key_len]
 * - Standard XOR cipher implementation
 *
 * Data storage:
 * - DAT_04630b80: Album data array
 * - DAT_04630b84: Album entry count
 * - Each entry: 104 bytes (0x68 bytes)
 * - Entry structure: pet_id, name, stats, capture_info
 *
 * Album entry layout (104 bytes):
 * - Offset 0x00: Pet ID (2 bytes)
 * - Offset 0x02: Pet name (16 bytes)
 * - Offset 0x12: Pet nickname (32 bytes)
 * - Offset 0x32: Level (1 byte)
 * - Offset 0x33: HP (2 bytes)
 * - Offset 0x35: Attack (2 bytes)
 * - Offset 0x37: Defense (2 bytes)
 * - Offset 0x39: Speed (2 bytes)
 * - Offset 0x3b: Element (1 byte)
 * - Offset 0x3c: Skills (20 bytes)
 * - Offset 0x50: Capture date (8 bytes)
 * - Offset 0x58: Reserved (16 bytes)
 *
 * Total album capacity: 36 * (entry_count_per_file)
 * Typical: 36 files * 104 pets = ~3744 pets maximum
 *
 * Error handling:
 * - Missing files: Skips silently
 * - Corrupt files: Partial load, continues to next
 * - Memory failure: Returns early
 *
 * Returns: Total number of album entries loaded, or -1 on error
 */
int FUN_0044a100(void) {
    /* Load and decrypt all album data files */
    return 0;
}

/* FUN_00449e00 - XOR Decryption Function
 * Decrypts data using XOR cipher with given key
 *
 * Parameters:
 * - param_1: Encrypted data buffer (modified in place)
 * - param_2: Data length
 * - param_3: XOR key buffer
 * - param_4: Key length
 *
 * Processing:
 * - For each byte i in data: data[i] ^= key[i % key_length]
 * - Simple XOR cipher implementation
 *
 * Note: XOR is symmetric - same function encrypts/decrypts
 */
void FUN_00449e00(unsigned char* param_1, int param_2,
                  const unsigned char* param_3, int param_4) {
    /* XOR decrypt data with key */
}

/* FUN_00425dc0 - Pet Data Parser for Capture
 * Parses pet data from protocol string for capture handling
 *
 * Parameters:
 * - param_1: Raw protocol string from server
 * - param_2: Output pet data structure
 *
 * Protocol format:
 * "pet_id|sprite_id|name|level|hp|atk|def|spd|element|skill1|skill2|..."
 *
 * Field parsing:
 * - Uses FUN_00489f70 for field extraction
 * - Uses FUN_004929fe for integer conversion
 * - Uses FUN_0048a170 for string unescaping
 *
 * Pet data structure:
 * - Offset 0x00: Pet ID (4 bytes)
 * - Offset 0x04: Sprite ID (4 bytes)
 * - Offset 0x08: Name (32 bytes)
 * - Offset 0x28: Level (4 bytes)
 * - Offset 0x2c: HP (4 bytes)
 * - Offset 0x30: Attack (4 bytes)
 * - Offset 0x34: Defense (4 bytes)
 * - Offset 0x38: Speed (4 bytes)
 * - Offset 0x3c: Element (4 bytes)
 * - Offset 0x40: Skills (40 bytes, 10 slots)
 *
 * Returns: 0 on success, -1 on parse error
 */
int FUN_00425dc0(const char* param_1, void* param_2) {
    /* Parse pet capture data from protocol */
    return 0;
}

/* FUN_00401300 - Pet AI Settings Loader with XOR Decryption
 * Loads and decrypts pet AI configuration from AISetting.dat
 *
 * Parameter: param_1 - Parameter array (4 elements)
 *   - [0]: AI mode setting
 *   - [1]: Attack priority mode
 *   - [2]: Skill usage mode
 *   - [3]: Target selection mode
 *
 * File: "data/AISetting.dat"
 *
 * Processing:
 * 1. Opens file via FUN_00492394
 * 2. Reads 16-byte header entry (XOR encrypted)
 * 3. Decrypts header using XOR key at DAT_004c10bc
 * 4. Reads AI settings entries in loop
 * 5. Each entry: 16 bytes, XOR encrypted
 * 6. Compares entry with parameter values
 * 7. If match found, reads additional settings
 *
 * XOR decryption key:
 * - Key stored at DAT_004c10bc (descending addresses)
 * - Key length: 17 bytes (0x4c10bc - 0x4c107c = 0x40 / 4)
 * - Each entry byte XORed with key[i % key_len]
 *
 * AI settings structure:
 * - DAT_004d9050: AI mode (0 = off, 1 = on)
 * - DAT_004d7ea4: Attack priority (9 = default)
 * - DAT_004d7f30: Skill mode (1 = default)
 * - DAT_004d7f1c: Target mode (0x1e = 30 = default)
 * - DAT_004d7ea8: Attack threshold (-1 = unlimited)
 * - DAT_004d7f34: Secondary mode (1)
 * - DAT_004d7f20: Secondary threshold (0x1e = 30)
 * - DAT_004d7eac: Secondary attack threshold (-1)
 * - DAT_004d7eb0: Tertiary priority (9)
 * - DAT_004d7f3c: Tertiary mode (1)
 * - DAT_004d7eb4: Quaternary setting (0x1a = 26)
 * - DAT_004d7f40: Quaternary mode (4)
 * - DAT_004d7f18: Final threshold
 * - DAT_004d7f54: AI enabled flag
 *
 * Default values (if file not found):
 * - Mode: 0 (disabled)
 * - Priority: 9
 * - All thresholds: -1 (unlimited)
 *
 * Returns: 0 if settings loaded, 1 if defaults used
 */
int FUN_00401300(int* param_1) {
    /* Load pet AI settings from AISetting.dat with XOR decryption */
    return 0;
}

/* FUN_0044aff0 - Battle UI Cleanup
 * Clears battle UI state and flags when exiting battle
 *
 * Processing:
 * 1. Clear battle state flag: DAT_045e8ce0 = 0
 * 2. Call FUN_0044ac50 to clear UI elements
 * 3. Call FUN_0044adc0 to clear additional UI state
 * 4. Clear counter: DAT_004e21dc = 0
 * 5. Set invalid values: DAT_045f1be0 = -1
 * 6. Set invalid values: DAT_004bd3d8 = -1
 * 7. Set invalid values: DAT_004bd3d4 = -1
 *
 * State flags cleared:
 * - DAT_045e8ce0: Main battle state (0 = not in battle)
 * - DAT_004e21dc: Battle UI counter
 * - DAT_045f1be0: Selection index (-1 = none)
 * - DAT_004bd3d8/d4: Target indices
 *
 * Called by: FUN_0040a1a0 (battle state machine) on state 7/8
 *
 * Used for: Resetting battle UI when returning to field
 */
void FUN_0044aff0(void) {
    /* Clear battle UI state and flags */
}

/* FUN_0040daf0 - Battle Entity Cleanup
 * Frees battle-specific entity and clears related state
 *
 * Processing:
 * 1. Check if entity exists: DAT_004e2b18 != 0
 * 2. If exists, call FUN_004011c0 to mark for deletion
 * 3. Clear entity handle: DAT_004e2b18 = 0
 * 4. Clear state flags: DAT_00544d74 = 0
 * 5. Clear state flags: DAT_00544d75 = 0
 *
 * Entity at DAT_004e2b18:
 * - Battle UI container entity
 * - Created at battle start
 * - Contains all battle UI elements
 *
 * State flags:
 * - DAT_00544d74: Battle UI active flag
 * - DAT_00544d75: Battle UI render flag
 *
 * Called by: FUN_0040a1a0 (battle state machine) on initialization
 *
 * Used for: Clearing previous battle UI before creating new one
 */
void FUN_0040daf0(void) {
    /* Free battle UI entity and clear state */
}

/* FUN_0044ac50 - Battle UI Element Cleanup (Primary)
 * Frees all primary battle UI elements and clears state flags
 *
 * Processing (marks entities for deletion via FUN_004011c0):
 * 1. DAT_045e7af0: Main battle UI entity, set DAT_00564e5c=0, DAT_045f1a3b=0
 * 2. DAT_0458f620: Secondary UI entity, set DAT_045f1a3a=0
 * 3. DAT_045e859c: Tertiary UI entity
 * 4. DAT_045e19b4: Quaternary UI entity
 * 5. Call FUN_00411990 on DAT_0054a3c8 (dialog cleanup)
 * 6. DAT_045e1850: Fifth UI entity
 * 7. DAT_045e7b14: Sixth UI entity
 * 8. DAT_04583108: Seventh UI entity
 * 9. Call FUN_0044ac00 to clear UI handles array
 * 10. Conditional cleanup (if non-zero):
 *     - DAT_045e19b8: Extended UI entity
 *     - DAT_04583240: Additional UI entity
 *     - DAT_045e85a0: Pet UI entity
 *     - DAT_04582cb4: Target UI entity
 *     - DAT_0458f718: Skill UI entity
 *     - DAT_0458f714: Item UI entity
 *     - DAT_045967a0: Menu UI entity
 *     - DAT_045e7c4c: Final UI entity
 *
 * Called by: FUN_0044aff0 (battle UI cleanup)
 *
 * Used for: Cleaning up all battle UI entities when exiting battle
 */
void FUN_0044ac50(void) {
    /* Free all primary battle UI elements */
}

/* FUN_0044adc0 - Battle UI Element Cleanup (Secondary)
 * Frees secondary battle UI elements and resets state flags
 *
 * Processing (marks entities for deletion via FUN_004011c0):
 * 1. DAT_045e19bc: Primary entity, clear DAT_045f194e, set to 0
 * 2. Call FUN_00411990 on DAT_0054a3c8 (dialog cleanup)
 * 3. DAT_0458f70c: Secondary entity, set DAT_004bd3dc=-1
 * 4. Conditional cleanup (similar to FUN_0044ac50):
 *    - DAT_045e19b8: Clear DAT_00564e5c, DAT_045f1a3b
 *    - DAT_0458f620: Clear DAT_045f1a3a
 *    - DAT_045e859c, DAT_04583240, DAT_045e85a0, DAT_04582cb4
 *    - Set DAT_045f194f=0
 *    - DAT_0458f718, DAT_0458f714, DAT_045967a0, DAT_045e7c4c
 *
 * State flags cleared:
 * - DAT_045f194e: UI mode flag
 * - DAT_045f194f: Secondary mode flag
 * - DAT_004bd3dc: Selection index (-1 = none)
 *
 * Called by: FUN_0044aff0 (battle UI cleanup)
 *
 * Used for: Additional cleanup for extended battle UI elements
 */
void FUN_0044adc0(void) {
    /* Free secondary battle UI elements */
}

/* FUN_004011f0 - Entity Cleanup and Process Termination
 * Terminates external process and marks all entities for deletion
 *
 * Processing:
 * 1. Check if external process tracking active: DAT_0455f8f8 != 0
 * 2. If active, use Process32First/Next to find process by ID
 * 3. If process name is NOT "explorer.exe", terminate it:
 *    - OpenProcess with PROCESS_ALL_ACCESS (0x1f0fff)
 *    - TerminateProcess
 *    - CloseHandle
 * 4. Clear process ID: DAT_0455f8f8 = 0
 * 5. Mark all entities in linked list for deletion
 *
 * Process tracking:
 * - DAT_0455f8f8: Tracked process ID (0 = not tracking)
 * - DAT_0456021c: Snapshot handle from CreateToolhelp32Snapshot
 * - DAT_0455fa18: PROCESSENTRY32 structure (size set to 0x128)
 * - DAT_0455fa20: Process ID from enumeration
 * - DAT_0455fa3c: Process name from enumeration
 *
 * Entity list cleanup:
 * - List head at DAT_004d7e3c + 4 (first entity)
 * - List tail at DAT_004d7e38
 * - For each entity: set *(entity + 0x24) = 1 (mark for deletion)
 *
 * Anti-cheat/Anti-debug:
 * - Purpose appears to be terminating external processes
 * - Exception for "explorer.exe" (Windows shell)
 * - Used in anti-tamper mechanism
 *
 * Called by: FUN_0040a1a0 (battle state machine) state 0 initialization
 *
 * Used for: Cleanup before battle, terminate any tracked external process
 */
void FUN_004011f0(void) {
    /* Terminate external process and mark all entities for deletion */
}

/* FUN_00419ac0 - UI Handle Array Cleanup
 * Clears array of UI handles by marking each for deletion
 *
 * Processing:
 * - Iterates from DAT_0054ca20 to DAT_0054cae8
 * - Array size: 0xcae8 - 0xca20 = 0xc8 = 200 bytes
 * - Each entry: 4 bytes (entity handle)
 * - Total entries: 200 / 4 = 50 UI handles
 * - For each non-zero handle: call FUN_004011c0 to mark for deletion
 * - Set handle to 0 after marking
 *
 * Called by: FUN_0040a1a0 (battle state machine) state 0 initialization
 *
 * Used for: Clearing UI handle array before battle
 */
void FUN_00419ac0(void) {
    /* Clear UI handle array (50 entries) */
}

/* FUN_00419a40 - Single UI Entity Cleanup
 * Frees a single UI entity and clears related state
 *
 * Processing:
 * 1. Check if entity exists: DAT_0054c998 != 0
 * 2. If exists, call FUN_004011c0 to mark for deletion
 * 3. Clear entity handle: DAT_0054c998 = 0
 * 4. Clear state flag: DAT_00564c61 = 0
 *
 * Called by: FUN_0040a1a0 (battle state machine) state 0 initialization
 *
 * Used for: Clearing specific UI entity before battle
 */
void FUN_00419a40(void) {
    /* Free single UI entity and clear state */
}

/* FUN_00465d20 - Shop Protocol Command Dispatcher
 * Dispatches shop-related protocol commands to appropriate handlers
 *
 * Parameters:
 * - param_1: Socket/connection handle
 * - param_2: Protocol command string (pipe-delimited)
 *
 * Processing:
 * 1. Extract first field via FUN_00489f70 (max 0x1f=31 bytes)
 * 2. Switch on first character of extracted field:
 *
 * Command characters:
 * - 'B' (0x42): Set buy mode flag DAT_00564e3e = 1, fall through to 'D'
 * - 'D' (0x44): Open shop UI via FUN_00416be0
 * - 'C' (0x43): Set sell mode flag DAT_00564e3e = 0
 * - 'O' (0x4f): Set special mode DAT_00564e3c = 1, DAT_046308b0 = 1
 * - 'S' (0x53): Set extended mode DAT_00564e3c = 3, call FUN_00415c50
 *
 * Mode flags:
 * - DAT_00564e3e: Buy/sell mode (1 = buy, 0 = sell)
 * - DAT_00564e3c: Shop mode (1 = special, 3 = extended)
 * - DAT_046308b0: Special shop flag
 *
 * Protocol format: "command|shop_data|item_list|..."
 * - First field: Single character command
 * - Subsequent fields: Shop-specific data
 *
 * Called by: Protocol dispatcher when receiving shop commands
 */
void FUN_00465d20(int param_1, const char* param_2) {
    /* Dispatch shop protocol command */
}

/* FUN_00416be0 - Shop UI Main Handler
 * Complex function handling shop display, item selection, and purchases
 *
 * Parameter: param_1 - Protocol data string (or NULL for UI update)
 *
 * Window creation (first call, DAT_00564e38 == 0):
 * - Creates window via FUN_00448610(0xc, 0, 0x269, 0x195, 0x89a8, -1)
 * - Size: 617 x 405 pixels
 * - Initializes data arrays: DAT_0054dbe0 (0x1b08 bytes)
 * - Initializes UI handles: DAT_0054c88c (20 entries)
 * - Counts inventory items and pet slots
 *
 * Button sprites (rendered via FUN_0047e210):
 * - 0x66b1: Left scroll button
 * - 0x66b3: Right scroll button
 * - 0x66db/0x66dc: Up/down scroll buttons
 * - 0x6767/0x6768: Toggle mode button
 *
 * UI states:
 * - DAT_00564e38: Window handle
 * - DAT_0054ccfc: Scroll offset (0-13)
 * - DAT_0054cd10: Selected item index
 * - DAT_00564e58: Buy/sell mode (0=buy, 1=sell)
 * - DAT_00564e48: Item portrait entity
 *
 * Item data (per item, stride 0x15a = 346 bytes):
 * - DAT_0054dbe0: Item sprite ID
 * - DAT_0054dbe8: Item count
 * - DAT_0054dc1c: Item available flag
 * - DAT_0054dc22: Item in cart flag
 * - DAT_0054dc23: Item name
 * - DAT_0054dc40: Item description
 * - DAT_0054dd71: Item category (0=pet, 1=item)
 *
 * Shop cart:
 * - DAT_0054c87c: Item count in cart
 * - DAT_0054c880: Pet count in cart
 * - DAT_0054c908: Total cart value
 *
 * Protocol handling:
 * - 'B' command: Parse item list from protocol
 * - 'D' command: Parse item details from protocol
 *
 * Click handling (via FUN_0044aba0):
 * - Case 0: Buy/confirm button
 * - Case 1: Cancel button
 * - Case 2: Scroll up
 * - Case 3: Scroll down
 * - Case 4: Toggle buy/sell mode
 *
 * Protocol output:
 * - Buy: "B %d %s" format via FUN_00490560
 * - Sell: "D %d" format
 *
 * Called by: FUN_00465d20 (shop protocol dispatcher)
 */
void FUN_00416be0(const char* param_1) {
    /* Main shop UI handler */
}

/* FUN_00415c50 - Pet Shop / Extended Shop UI
 * Handles pet selection and special shop modes
 *
 * Parameters:
 * - param_1: Mode (0 = normal, non-zero = special)
 * - param_2: Protocol data string (or NULL for UI update)
 *
 * Window creation (first call, DAT_00564e34 == 0):
 * - Creates window via FUN_00448610(9, 0, 0x26e, 0x19d, DAT_0054ad1c, -1)
 * - Size: 622 x 413 pixels
 * - Initializes scroll buttons and navigation
 *
 * UI states:
 * - DAT_00564e34: Window handle
 * - DAT_0054c9b4: Scroll offset (0-10)
 * - DAT_0054c884: Selected pet index (0-4)
 * - DAT_00564e50: Selection mode active flag
 * - DAT_00564e48: Pet portrait entity
 *
 * Pet data (per pet, stride 0x4c = 76 bytes):
 * - DAT_00564658: Pet ID
 * - DAT_0056465c: Pet type (0=pet, 1=character)
 * - DAT_00564660: Pet level
 * - DAT_00564664: Selection flag
 * - DAT_00564683: Pet name
 * - DAT_005646a0: Pet category
 *
 * Pet selection array:
 * - DAT_0054c90c: Selected pets (5 slots)
 * - DAT_0054c9b0: Selected characters (5 slots)
 * - Values: -1 = not selected, index = selected
 *
 * Button handling:
 * - Case 0: Confirm selection
 * - Case 1: Cancel
 * - Case 2: Scroll up
 * - Case 3: Scroll down
 * - Case 4: Select current pet
 * - Case 5: Next pet
 * - Case 6: Previous pet
 *
 * Protocol output:
 * - "S %d %s %s" format for pet selection
 * - "R P 1" format for special mode
 *
 * Called by: FUN_00465d20 for 'S' command
 */
void FUN_00415c50(int param_1, const char* param_2) {
    /* Pet shop / extended shop UI */
}

/* FUN_00404850 - Battle Map Loader (SAB Format)
 * Loads and renders battle map from SAB file
 *
 * Parameter: param_1 - Map ID (0-219, values > 0xdb reset to 0)
 *
 * File format: "data/battleMap/battle%02d.sab"
 * - 4-byte header: "SAB1" signature
 * - 800-byte tile data (20x20 grid, 2 bytes per tile)
 *
 * Header validation:
 * - FUN_0049212c reads 4-byte header
 * - FUN_00492020 checks for "SAB1" signature
 * - Shows error MessageBox if invalid
 *
 * Tile data:
 * - 20x20 = 400 tiles
 * - Each tile: 2 bytes (sprite ID)
 * - Read via FUN_00492421 (little-endian 16-bit)
 *
 * Map rendering:
 * - Isometric tile positioning
 * - Start position: (-288, 240) = (-0x120, 0xf0)
 * - Tile offset: (32, -24) = (0x20, -0x18)
 * - 20 tiles per row, 20 rows total
 *
 * Special maps (DAT_04581190):
 * - 0x331, 0x1f47, 0x1fa5, etc.: Arena maps (single sprite)
 * - 0x2147: Arena map with sprite 0x718d
 * - 0x7546-0x7549: Special event maps with random backgrounds
 * - 0x4e-0x63, 0x1fab-0x1fb0: Special dungeon maps
 *
 * Background sprite selection (via FUN_00492403 random):
 * - Default: 0x7149 + variant
 * - Arena maps: Single background sprite
 * - Special maps: Random from sprite tables at DAT_0049ea64+
 *
 * Data storage:
 * - DAT_004d7f78: Current map ID
 * - DAT_004d7f74: Current background sprite
 * - DAT_04581190: Current field/map ID for special handling
 *
 * Returns: 1 on success, 0 on file error
 */
int FUN_00404850(int param_1) {
    /* Load and render battle map from SAB file */
    return 0;
}

/* FUN_00404e20 - Battle Background Renderer
 * Renders battle background with special effects per map type
 *
 * Processing modes:
 * - Mode 0: Standard rendering with surface blit
 * - Mode 1: Arena/special map - horizontal stripe effect
 * - Mode 2: Wave/distortion effect
 *
 * Mode selection:
 * - Normal map (DAT_004d7f78 not 0x94-0x96): Mode 0
 * - Arena maps (DAT_04581190 special values): Mode 1
 * - Special maps with DAT_0461c7d4 == 2: Mode 1
 * - Otherwise: Mode 2
 *
 * Special map IDs for arena rendering:
 * - 0x331, 0x1f47, 0x1fa5, 0x1fa4, 0x1f5b-0x1f5d, 0x1f4f, 0x1fb1, 0x1fb2
 *
 * Surface info (DAT_0054a90c):
 * - Offset 0x88: Surface width
 * - Offset 0x8c: Surface height
 * - Default size: 640x480 (0x280 x 0x1e0)
 *
 * Mode 0 - Standard:
 * - Uses surface dimensions (clamped to max 640x480)
 * - Blits from primary surface at offset 0xc
 * - If alpha mode (DAT_0054c83c): Also blit secondary surface at 0x10
 *
 * Mode 1 - Arena stripes:
 * - Horizontal stripe pattern
 * - Y offset: DAT_0461c7d8 + DAT_004d7f7c
 * - Stripe data from DAT_0049ea94 table (64 entries)
 * - 8-pixel stripes, total 0x1e0 (480) pixels height
 *
 * Mode 2 - Wave effect:
 * - Wave distortion using sine table
 * - Amplitude from DAT_0049ea94 table
 * - 4-pixel rows with wave offset
 * - Creates rippling water/magic effect
 *
 * Alpha mode (DAT_0054c83c):
 * - Enables secondary surface rendering
 * - Uses DAT_0465d7c0 for alpha buffer
 * - Renders both surfaces with transparency
 *
 * Called by: FUN_0047dc60 (render queue processor) in battle mode
 */
void FUN_00404e20(void) {
    /* Render battle background with special effects */
}

/* FUN_00462f60 - NPC Dialog Handler
 * Handles NPC dialog text and player responses
 *
 * Processing:
 * 1. Initialize via FUN_00492d80
 * 2. Check dialog state (DAT_0455ef2c and DAT_04560258)
 * 3. Parse dialog text via FUN_00489f70
 * 4. Extract NPC ID, dialog type, and options
 *
 * Dialog parsing (via FUN_00489f70):
 * - Field 1: NPC ID (via FUN_0048a0a0 Base-62 decode)
 * - Field 2: Dialog type (via FUN_004929fe)
 * - Field 3: Response type (via FUN_004929fe)
 * - Field 4: Action code (via FUN_004929fe)
 * - Field 5+: Additional parameters
 *
 * Dialog types:
 * - 0x29: Text response with custom string (max 21 chars)
 * - Other: Standard dialog options
 *
 * Current NPC tracking:
 * - DAT_0462be90: Current NPC ID
 * - DAT_0462e3ac: Current NPC entity
 * - DAT_046308b0: Special mode flag
 *
 * Entity handling:
 * - If NPC ID matches current: Update existing entity
 * - If different: Create new via FUN_0040f460 or queue action
 * - Uses DAT_0462e3ac entity fields at offsets 0x1d4-0x1e8
 *
 * Response handling:
 * - Special action 0x29: Store custom response text
 * - FUN_004781f0: Update NPC state
 * - FUN_0043b490/FUN_0048f900: Send response to server
 *
 * Player actions:
 * - FUN_0040ddd0: Execute NPC action
 * - FUN_0043b660: Process dialog response
 * - FUN_00477d90: Update display
 *
 * Dialog queue:
 * - Stack buffer at 0x1038 holds seen NPC IDs
 * - Max 0x1000 (4096) entries tracked
 * - Prevents duplicate processing
 *
 * Called by: Protocol dispatcher for NPC dialog commands
 */
void FUN_00462f60(void) {
    /* Handle NPC dialog text and responses */
}

/* FUN_00411990 - Set Dialog Context
 * Sets the current dialog/UI context for text input
 *
 * Parameter: param_1 - Dialog context pointer
 *
 * Simply stores the context in DAT_0054a4f4
 * Used to track active dialog for text rendering
 *
 * Context structure (at DAT_0054a4f4):
 * - Text buffer pointer
 * - Cursor position
 * - Input mode flags
 * - Character limit
 *
 * Called by: Various UI functions to set active input context
 */
void FUN_00411990(void* param_1) {
    /* Set dialog context */
    DAT_0054a4f4 = param_1;
}

/* FUN_0047e720 - Sprite Render Queue Processor
 * Processes sorted sprite render queue and blits sprites to surface
 *
 * Processing:
 * 1. Update render counter: DAT_0464f488 += DAT_0464f48c
 * 2. Iterate through sorted queue (sorted by layer)
 * 3. For each sprite, check visibility and blit
 *
 * Queue structure (at DAT_0464b488):
 * - Each entry: 24 bytes (0x18)
 * - Offset 0: Layer value (determines sort order)
 * - Entry count in DAT_0464f488
 *
 * Visibility check (via FUN_004808e0):
 * - Returns non-zero if sprite is visible on screen
 * - Skips rendering if not visible
 *
 * Rendering modes:
 * - Alpha mode off (DAT_0054c83c == 0): Single surface blit
 * - Alpha mode on: Dual surface blit with transparency
 *
 * Blit functions:
 * - FUN_004142f0: Normal blit (no alpha)
 * - FUN_00414190: Alpha blit with secondary surface
 *
 * High-resolution mode (DAT_04560214 == 1):
 * - Shifts coordinates by 1 pixel for sub-pixel positioning
 * - Creates smoother scrolling
 *
 * Sprite data array (at DAT_0466b7e0, stride 12):
 * - Offset 0: Sprite surface
 * - Offset 4: X offset
 * - Offset 8: Y offset
 * - Offset 16: Next sprite pointer
 *
 * Error detection:
 * - Return value -0x7789fe52 indicates blit failure
 * - Sets DAT_0464f7b0 = 1 to flag error
 *
 * Skip condition: DAT_04630dd8 == 10 (special state)
 *
 * Called by: FUN_0047dc60 (main render processor)
 */
void FUN_0047e720(void) {
    /* Process sprite render queue and blit to surface */
}

/* FUN_0040f460 - Find Entity by ID
 * Looks up entity pointer from entity ID
 *
 * Parameter: param_1 - Entity ID to find
 *
 * Processing:
 * 1. Call FUN_0040e830 to get entity index from ID
 * 2. If index < 0, return 0 (not found)
 * 3. Return entity pointer from DAT_004e2bdc array
 *
 * Entity array (at DAT_004e2bdc):
 * - Stride: 0x43 * 4 = 268 bytes per entity
 * - Each entry: entity pointer
 *
 * Returns: Entity pointer or 0 if not found
 */
int FUN_0040f460(int param_1) {
    /* Find entity by ID */
    return 0;
}

/* FUN_0040ddd0 - Entity Action Dispatcher
 * Dispatches entity actions based on action type (30+ action types)
 *
 * Parameters:
 * - param_1: Entity pointer
 * - param_2: X coordinate
 * - param_3: Y coordinate
 * - param_4: Target/parameter
 * - param_5: Action type
 * - param_6: Additional parameter
 * - param_7: Sprite ID
 * - param_8: Mode/variant
 *
 * Action types (param_5) and handlers:
 * - 0: Set idle state (state=3, target=-1)
 * - 1: Walk/move to position (FUN_0040b6e0 or FUN_0040b740)
 * - 2: Set state 0 (stand)
 * - 3: Set state 12 (action)
 * - 4: Set state 1 (walk)
 * - 5, 10: Set state 2
 * - 0xb: Set state 5
 * - 0xc: Set state 6
 * - 0xd: Set state 7
 * - 0xe: Set state 8
 * - 0xf: Set state 9
 * - 0x10: Set state 10
 * - 0x11: Set state 4
 * - 0x12: Set state 11
 * - 0x13: Set state 3
 * - 0x14: Emote effect (FUN_0040c1a0 or FUN_0040c210)
 * - 0x15: Special effect (FUN_0040c050 or FUN_0040c020)
 * - 0x16: Effect mode (FUN_0040c0e0 or FUN_0040c110)
 * - 0x17: Set entity property (FUN_0040db20)
 * - 0x1e: Set target and calculate direction
 * - 0x1f: Reset to position with state 3
 * - 0x20: Set animation (FUN_0040c240 or FUN_0040c270)
 * - 0x22: Play animation (FUN_0040c2a0 or FUN_0040c2d0)
 * - 0x28: Set direction (FUN_0040c140)
 * - 0x29: Create child entity
 * - 0x2a: Remove child entity
 * - 0x32: Set direction or dispatch
 * - 0x33: Create effect entity with parameters
 * - 0x3c: Create emote entities (4 sprites)
 *
 * Entity structure offsets:
 * - 0xc: Extra data pointer (contains entity info)
 * - 0x110: Animation counter
 * - 0x148: Action state
 * - 0x14c: Previous state (-1 = none)
 * - 0x150: Target parameter
 * - 0xb0-0xbc: Position data
 *
 * Movement functions:
 * - FUN_0040b6e0: Start movement to target
 * - FUN_0040b740: Continue movement
 * - FUN_0040bfc0: Update position
 *
 * Called by: Various entity handlers for NPC, player, and pet actions
 */
void FUN_0040ddd0(int entity, int x, int y, int target, int action_type,
                  int param6, int sprite_id, int mode) {
    /* Dispatch entity action based on type */
}
