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
 * - Main game state machine with 20+ states
 * - State dispatch via DAT_04630df0 (sub-state)
 * - Main game state in DAT_04630dd8
 *
 * Sub-states (DAT_04630df0):
 * - 0: Initialize - Load album, init systems, set state 100
 * - 100 (0x64): Splash/Loading screen
 * - 101 (0x65): Wait for flag DAT_04560254
 * - 102 (0x66): Main game render setup, fade in
 * - 1: Login screen - fade transition after DAT_04630de4==1
 * - 2: Fade transition wait
 * - 3: Main game loop - render field, entities, UI
 * - 4: Pre-battle fade out
 * - 5: Battle transition fade
 * - 20 (0x14): Return to field from battle
 * - 101 (0x65): Wait state
 * - 102 (0x66): Render setup with fade
 * - 103 (0x67): Fade in after transition
 * - 150 (0x96): Set sub-state to 100
 * - 200 (0xc8): Logout/exit transition
 * - 201-204 (0xc9-0xcc): Logout fade sequence
 *
 * Key functions called:
 * - FUN_004779d0: Update UI elements
 * - FUN_00418330: Menu rendering
 * - FUN_0040f7d0: Character select UI
 * - FUN_004445c0: Entity render queue
 * - FUN_00401170: Field entity update
 * - FUN_0047dc60: Main render loop
 * - FUN_0047bde0: Fade effect dispatcher
 */
void FUN_00479c40(void) {
    /* State machine implementation in gamestate.c */
}

/*
 * FUN_0041db40 - Main Game Loop
 *
 * Binary analysis:
 * - Main game loop with timing obfuscation and anti-tamper
 * - Frame rate limiting using timeGetTime()
 * - Anti-tamper checks using XOR obfuscation
 *
 * Key operations per frame:
 * 1. Initialize timing (first call only)
 * 2. Process network I/O via FUN_0045e880()
 * 3. Update input state via FUN_00480bd0()
 * 4. Run game state machine via FUN_004799b0()
 * 5. Process render queue via FUN_0047dc60()
 * 6. Frame rate limiting with Sleep()
 *
 * Anti-tamper mechanism:
 * - Uses XOR with 0xffffbcde for timing values
 * - Checks DAT_005ab728 for integrity
 * - Rotates buffer memory every 2 minutes
 * - Detects debuggers via DAT_04ebe3f1/e9 flags
 *
 * Memory rotation:
 * - Every 120 seconds, swaps DAT_0461b41c and gBuffer
 * - Uses VirtualAlloc/VirtualFree for secure memory
 * - Toggles between two buffers via DAT_004ab7d4
 *
 * Screenshot detection:
 * - Checks DAT_0054bdf4 for screenshot request
 * - If detected, processes via FUN_004808c0/80870
 *
 * Returns: 0 on failure, continues looping otherwise
 */
int FUN_0041db40(void) {
    /* Main game loop implementation in gameloop.c */
    return 1;
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
 * - Main text-based protocol command dispatcher
 * - Commands are newline-delimited strings parsed at DAT_004ba014
 * - Uses FUN_0043dbe0 for initial parsing into local_400 buffer
 * - Compares first token against command strings at DAT_004b9xxx
 * - Routes to handler functions with parsed parameters
 *
 * Command categories (50+ commands total):
 * - Battle commands: FUN_00465400, FUN_00463e70, FUN_00464e10
 * - Party commands: FUN_004643f0, FUN_00464610, FUN_00464ef0, FUN_00464670
 * - Guild commands: FUN_00464650, FUN_00465170, FUN_00464af0, FUN_00463f00
 * - Trade commands: FUN_00464190, FUN_00463790, FUN_00462010, FUN_00462200
 * - Chat commands: FUN_00462590, FUN_00462f60, FUN_00463380, FUN_00465460
 * - NPC commands: FUN_0045ffb0, FUN_00463ee0, FUN_00464ee0, FUN_00463d80
 * - Login commands: FUN_0045fa40, FUN_00463c20, FUN_00463d20, FUN_0045fdc0
 * - Character commands: FUN_0045fb80, FUN_0045ff50, FUN_00465440
 * - Mail commands: FUN_0045a9a0, FUN_00465470
 *
 * Uses FUN_0043dd50 to parse integer parameters
 * Uses FUN_0043dd70 to parse string parameters
 * Uses FUN_0043e1b0 for string unescaping/processing
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

/*
 * FUN_004872b0 - WAV File Header Parser
 *
 * Binary analysis:
 * - Parses WAV file format header using Windows multimedia I/O
 * - param_1: HMMIO file handle (from mmioOpen)
 * - param_2: output WAVEFORMATEX structure (18 bytes)
 * - param_3: output data chunk size
 * - Returns 1 on success, 0 on failure
 *
 * Process:
 * 1. Descend into 'WAVE' chunk (fccType = 0x45564157 = "WAVE")
 * 2. Descend into 'fmt ' chunk (ckid = 0x20746d66 = "fmt ")
 * 3. Read 18 bytes of format data (WAVEFORMATEX)
 * 4. Validate format tag is 1 (PCM)
 * 5. Ascend from fmt chunk
 * 6. Descend into 'data' chunk (ckid = 0x61746164 = "data")
 * 7. Return data chunk size
 *
 * WAVEFORMATEX structure (param_2):
 * - offset 0: wFormatTag (1 = PCM)
 * - offset 2: nChannels
 * - offset 4: nSamplesPerSec
 * - offset 8: nAvgBytesPerSec
 * - offset 12: nBlockAlign
 * - offset 14: wBitsPerSample
 * - offset 16: cbSize
 */
int FUN_004872b0(void* hmmio, void* format, u32* data_size) {
    (void)hmmio; (void)format; (void)data_size;
    /* TODO: Full WAV parsing implementation with mmioDescend/mmioRead */
    return 0;
}

/*
 * FUN_00420590 - Login Screen State Machine
 *
 * Binary analysis:
 * - Main login screen state machine with multiple states
 * - State tracked via DAT_04630df0
 *
 * States (DAT_04630df0):
 * - 0: Initialize - Close socket, terminate thread, init UI
 * - 1: DES encryption setup - Copy credentials, encrypt
 * - 2: Wait for login window creation
 * - 3: Display login window, play sound
 * - 4: Wait for user input
 * - 100 (0x64): Error state - show message, retry
 * - 101 (0x65): Wait for retry after error
 *
 * Key operations:
 * - State 0: closesocket(), TerminateThread(), FUN_00412710()
 * - State 1: DES encrypt via FUN_0048bb90 with key "f;encor1c"
 * - State 2: FUN_00420b70() for window creation
 * - State 3: Create window via FUN_00448610(), sound via FUN_00488190()
 * - State 4: Check login result via FUN_0045ef60()
 *
 * DES encryption:
 * - Key: s_f_encor1c_0049e48c = "f;encor1c" (9 bytes)
 * - Encrypts username and password separately (32 bytes each)
 * - Uses FUN_0048bb90 for DES cipher
 *
 * Returns from FUN_0045ef60:
 * - 1: Login successful, transition to game state 2
 * - 2: Cancel/exit, post WM_CLOSE message
 * - -7: Connection error, show DAT_04ebe4d8 message
 * - -8: Server error, show DAT_04ebe4d8 message
 * - Other negative: Show DAT_0461b65c error message
 */
void FUN_00420590(void) {
    /* Login state machine implementation */
    /* TODO: Full login state machine implementation */
}

/*
 * FUN_0048bb90 - DES Cipher Block Operation
 *
 * Binary analysis:
 * - Encrypts/decrypts data using DES cipher in ECB mode
 * - param_1: key string (typically "f;encor1c")
 * - param_2: data buffer to encrypt/decrypt
 * - param_3: data length in bytes
 * - param_4: operation mode (bit flags)
 *   - bit 0: 0=encrypt, 1=decrypt
 *   - bit 1: 1=don't return boolean (internal use)
 *
 * Process:
 * 1. Initialize key schedule via FUN_0048b620(param_1)
 * 2. Calculate padded length: (length + 7) & ~7 (round up to 8)
 * 3. Process 8-byte blocks via FUN_0048b7c0(block, mode)
 * 4. Return true unless bit 1 of mode is set
 *
 * Used by:
 * - FUN_00420590: Login credential encryption
 * - Various protocol encryption/decryption
 */
int FUN_0048bb90(const char* key, void* data, int length, int mode) {
    (void)key; (void)data; (void)length; (void)mode;
    /* TODO: Full DES cipher implementation */
    return 1;
}

/*
 * FUN_0048d3e0 - Binary Protocol Dispatcher
 *
 * Binary analysis:
 * - Main binary protocol command dispatcher
 * - Dispatches to handler functions based on opcode
 * - Uses anti-tamper checksum validation
 *
 * Process:
 * 1. Initialize via FUN_00492d80(), FUN_0048c720(), FUN_0048c5a0()
 * 2. Get packet count via FUN_0048c7f0()
 * 3. For each packet, read opcode from stack (in_stack_00000024)
 * 4. Validate checksum: sum of parameters must equal return address
 * 5. Dispatch to appropriate handler
 *
 * Opcodes (partial list):
 * - 0x02: Battle action (FUN_00465400)
 * - 0x04: Party update (FUN_00463e70)
 * - 0x07: Pet update (FUN_00464e10)
 * - 0x0C: Party member join (FUN_004643f0)
 * - 0x0D: Party member leave (FUN_00464610)
 * - 0x0F: Friend update (FUN_00465460, FUN_00464ef0)
 * - 0x18: Guild member update (FUN_00464650)
 * - 0x1A: Guild info (FUN_00465170)
 * - 0x1C: Guild member list (FUN_00464af0)
 * - 0x2A: NPC dialog (FUN_00462f60)
 * - 0x48: Login server response (FUN_0045fa40)
 * - 0x50: Character list response (FUN_0045fb80)
 * - 0x5C: Mail receive (FUN_0045a9a0)
 * - 0x75: Shop protocol (FUN_00465d20)
 * - 0x78: Skill protocol (FUN_00465de0)
 * - 0x85: Guild member data (FUN_004665b0)
 * - 0x86: Guild info (FUN_00466550)
 */
int FUN_0048d3e0(int param_1) {
    (void)param_1;
    /* TODO: Full binary protocol dispatcher implementation */
    return 0;
}

/*
 * FUN_0043dbe0 - Parse Integer Parameter with Default
 *
 * Binary analysis:
 * - Parses integer from protocol buffer, returns default if not present
 * - param_1: output integer pointer
 * - param_2: destination buffer for string copy
 * - param_3: buffer size
 * - param_4: source data (2 dwords: [0]=value or 0, [1]=string or 0)
 *
 * Process:
 * - If param_4[0] and param_4[1] are both non-zero:
 *   - Convert param_4[0] to integer via FUN_004930a0 (base 10)
 *   - Copy string via FUN_0043dc40
 * - Otherwise: set output to 0 and copy default string from DAT_004e220c
 */
void FUN_0043dbe0(int* output, char* dest, int dest_size, int* source) {
    (void)output; (void)dest; (void)dest_size; (void)source;
    /* TODO: Full implementation */
}

/*
 * FUN_0043dc40 - String Copy with Length Limit
 *
 * Binary analysis:
 * - Copies string with length limit (null-terminated)
 * - param_1: destination buffer
 * - param_2: source string
 * - param_3: maximum size including null terminator
 * - Copies until null byte or max_size-1 characters
 * - Always null-terminates the result
 */
void FUN_0043dc40(char* dest, const char* src, int max_size) {
    (void)dest; (void)src; (void)max_size;
    /* TODO: Full implementation with safe string copy */
}

/*
 * FUN_0043dd50 - Parse String Field
 *
 * Binary analysis:
 * - Parses string field from protocol buffer
 * - param_1: source pointer (or 0 for empty)
 * - Returns pointer to parsed string
 * - If param_1 == 0: copies default from DAT_004e220c to DAT_004b9ff8
 * - Otherwise: calls FUN_0043ded0 to extract field
 */
const char* FUN_0043dd50(const char* source) {
    (void)source;
    /* TODO: Full implementation */
    return "";
}

/*
 * FUN_0043dd70 - Get String Field with Default
 *
 * Binary analysis:
 * - Similar to FUN_0043dd50 but with default handling
 * - param_1: source pointer (or 0 for default)
 * - Returns pointer to result string
 * - If param_1 == 0: copies default string
 * - Otherwise: extracts via FUN_0043ded0
 */
const char* FUN_0043dd70(const char* source) {
    (void)source;
    /* TODO: Full implementation */
    return "";
}

/*
 * FUN_0043e1b0 - String Field Copy
 *
 * Binary analysis:
 * - Simple wrapper for FUN_0043dc40
 * - param_1: destination buffer
 * - param_2: buffer size
 * - param_3: source string
 * - Returns param_1
 */
char* FUN_0043e1b0(char* dest, int size, const char* src) {
    (void)dest; (void)size; (void)src;
    /* TODO: Full implementation */
    return dest;
}

/*
 * FUN_0048a170 - String Unescape Processing
 *
 * Binary analysis:
 * - Processes escape sequences in protocol strings
 * - param_1: string to process (in-place)
 * - Returns pointer to processed string
 *
 * Escape sequences (backslash-prefixed):
 * - DAT_004d5829 contains escape characters to find
 * - DAT_004d5828 contains replacement characters
 * - Table has 4 entries (8 bytes total)
 *
 * Process:
 * 1. Calculate string length
 * 2. Iterate through characters
 * 3. For DBCS lead bytes: copy both bytes as-is
 * 4. For backslash: look up next char in escape table
 *    - If found: replace with corresponding character from DAT_004d5828
 *    - If not found: copy the character as-is
 * 5. Null-terminate the result
 *
 * Common escape sequences:
 * - \n -> newline
 * - \t -> tab
 * - \\ -> backslash
 * - \| -> pipe (field delimiter)
 */
char* FUN_0048a170(char* str) {
    (void)str;
    /* TODO: Full implementation with escape sequence processing */
    return str;
}

/*
 * FUN_004799b0 - Main Game State Dispatcher
 *
 * Binary analysis:
 * - Main game state machine dispatcher
 * - Handles state transitions via DAT_04630df8 (pending state)
 * - Switch on DAT_04630dd8 (current game state)
 *
 * Game states (DAT_04630dd8):
 * - 0: Initialize - Load resources, init sprites
 * - 1: Login screen - FUN_00420590, render, update
 * - 2: Character select - FUN_00421110
 * - 3: Character create - FUN_00421c00
 * - 4: Server select - FUN_00422aa0
 * - 5: Pre-game setup - Clear entities, init field
 * - 6: Main menu/lobby - FUN_00424610
 * - 7: Settings/config - FUN_00424880
 * - 9: Main game loop - FUN_00479c40 (state machine)
 * - 10: Battle - FUN_0040a1a0
 * - 11 (0xb): Logout/transition - Fade out, cleanup
 *
 * State transition:
 * - If DAT_04630df8 >= 0: transition to pending state
 * - Sets DAT_04630dd8 = DAT_04630df8
 * - Sets DAT_04630df0 = DAT_04630de8
 * - Resets DAT_04630df8 to -1
 *
 * Called each frame from FUN_0041db40 main loop
 */
void FUN_004799b0(void) {
    extern s32 DAT_04630df8;
    extern u32 DAT_04630dd8;
    extern u32 DAT_04630de8;
    extern u32 DAT_04630df0;

    /* Check for pending state transition */
    if (DAT_04630df8 >= 0) {
        DAT_04630dd8 = DAT_04630df8;
        DAT_04630df8 = -1;
        DAT_04630df0 = DAT_04630de8;
    }

    /* State dispatch - each state has its own handler */
    /* TODO: Full implementation with all state handlers */
}

/*
 * FUN_0047bfc0 - Character Select Screen Animation
 *
 * Binary analysis:
 * - Animates character selection screen with 8 character slots
 * - param_1: float parameter (unused?)
 *
 * Animation phases (DAT_04633304):
 * - 0: Initialize - Clear arrays, load sprites 0x714a-0x715a
 * - 1: Start animation - Set phase to 2
 * - 2: Animate - Update positions, move sprites
 * - 3: Complete - Finalize animation, show character slots
 *
 * Animation data (8 slots):
 * - DAT_0463118c: Animation phase per slot (0-4)
 * - DAT_04631038: Position/velocity per slot
 * - DAT_04631010: Target position per slot
 * - DAT_046330c0: Counter for slot activation
 * - DAT_046311ac: Number of active slots
 * - DAT_046331c8: Completion counter
 *
 * Process:
 * 1. Load 17 sprites (0x714a-0x715a) for character slots
 * 2. Animate each slot independently
 * 3. Render using FUN_0047e210
 * 4. When all 8 slots complete, show final character
 */
void FUN_0047bfc0(float param_1) {
    (void)param_1;
    /* TODO: Full character select animation implementation */
}

/*
 * FUN_0048a0a0 - Base-62 String to Integer
 *
 * Binary analysis:
 * - Converts Base-62 encoded string to integer
 * - param_1: null-terminated string
 * - Returns: decoded integer value
 *
 * Base-62 character set:
 * - '0'-'9': values 0-9
 * - 'a'-'z': values 10-35
 * - 'A'-'Z': values 36-61
 * - '-': negative sign (only at start)
 *
 * Formula: result = result * 62 + digit_value
 * Used for bitmask encoding in protocol messages
 */
int FUN_0048a0a0(const char* str) {
    (void)str;
    /* TODO: Full Base-62 decode implementation */
    return 0;
}

/*
 * FUN_00421110 - Character Select Screen State Machine
 *
 * Binary analysis:
 * - Main character selection screen with 6 sub-states
 * - State tracked via DAT_04630df0
 *
 * States (DAT_04630df0):
 * - 0: Initialize - Set window title, check character count
 *   - If no characters (DAT_04ebe8d8 empty): go to state 200 (create new)
 * - 1: Wait for server response via FUN_0045ebd0
 *   - On success: init character grid via FUN_00421410
 *   - On failure: show error, go to state 100
 * - 2: Character selection via FUN_00421420
 *   - Return 1: character selected, continue
 *   - Return 2: back to login
 *   - Return 3: create new character
 * - 3: Wait for character data via FUN_00421770
 * - 4: Enter game via FUN_00421a30
 * - 5: Character data validation via FUN_00421a40
 *   - Success: transition to game state 3
 *   - Failure: show error, go to state 100
 * - 100: Error state - show message, retry
 * - 101 (0x65): Wait for retry
 * - 200: Create new character
 * - 201 (0xc9): Wait for creation
 *
 * Key functions:
 * - FUN_00421410: Initialize character select UI
 * - FUN_00421420: Handle character selection
 * - FUN_00421770: Load character data
 * - FUN_00421a30: Enter game with selected character
 * - FUN_00421a40: Validate character data
 */
void FUN_00421110(void) {
    /* Character select state machine implementation */
    /* TODO: Full implementation */
}

/*
 * FUN_00421420 - Character Selection Handler
 *
 * Binary analysis:
 * - Handles character selection from list
 * - Returns: 0=continue, 1=selected, 2=back, 3=create new
 *
 * Layout:
 * - Up to 3 characters displayed in grid
 * - Grid layout depends on character count:
 *   - 1-3 chars: 3 columns (DAT_04552fa8 = 0x82, increment = 0x19)
 *   - 4-18 chars: 5 columns (increment = 0x50)
 *   - 19+ chars: 9 columns (increment = 0x1e)
 *
 * Process:
 * 1. Create selection window via FUN_00448610
 * 2. Display character names from DAT_04ebed30 (0x12 bytes each)
 * 3. Handle click via FUN_00421080 (search clicked character)
 * 4. Return result based on selection
 *
 * Data structures:
 * - DAT_004b8024: Click regions (60 entries, 0xfffffffe = empty)
 * - DAT_04ebed28: Character availability flags
 * - DAT_0455ef24: Character count
 * - DAT_0455ef28: Selected character index
 */
int FUN_00421420(void) {
    /* TODO: Full character selection implementation */
    return 0;
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

/* ========================================
 * Save Data Functions
 * ======================================== */

/*
 * FUN_00449e00 - Load Save Data
 *
 * Binary analysis:
 * - Loads game save data from file
 * - Opens file via FUN_00492394
 * - Reads records, decrypts with XOR key at DAT_004c107c (16 bytes)
 * - Key bytes: applied to 16-byte chunks
 * - Validates record header against expected value
 * - Copies data to DAT_045837c8 (max 0x8980-0x9010 range)
 * - Calls FUN_00449910 for each record
 * - Returns 1 on success, 0 on failure
 * - Uses stack buffer of 0x50 bytes per record
 */
int FUN_00449e00(void) {
    return 0;
}

/*
 * FUN_00401300 - Load Pet AI Settings
 *
 * Binary analysis:
 * - Loads pet AI configuration from "data/AISetting.dat"
 * - param_1: 16-byte key for validation (XOR encrypted)
 * - Decrypts key using DAT_004c10bc (reverse order)
 * - Searches for matching record in file
 * - Loads settings to:
 *   - DAT_004d9050: AI mode (4 bytes)
 *   - DAT_004d7ea4-eb4: 5 AI parameters
 *   - DAT_004d7f30-3c: 5 AI targets
 *   - DAT_004d7f1c-20: 5 AI thresholds
 *   - DAT_004d7f18: Extra setting
 *   - DAT_004d7f54: Enable flag
 * - Default values: mode=9, targets=1, thresholds=0x1e
 * - Returns 1 if found, 0 otherwise
 */
int FUN_00401300(void* key) {
    (void)key;
    return 0;
}
