/*
 * Stone Age Client - Utility Stub Functions
 * Split from stubs.c to reduce file size
 */

#include <windows.h>
#include <stdlib.h>
#include "types.h"

/* External globals */
extern s32 DAT_045e19b0;
extern s32 DAT_004e21dc;
extern s32 DAT_045e8ce0;
extern char DAT_045f1a3b;
extern char DAT_045f1a3a;
extern u32 DAT_045f1b90;
extern u32 DAT_045f1bc4;
extern u32 DAT_0462e3b4;

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

/*
 * FUN_0044b030 - Game/Battle State Checker
 *
 * Binary analysis:
 * - Returns 1 if game is in a state that blocks certain actions
 * - Checks: battle active (DAT_045e19b0), dialog (DAT_004e21dc), menu (DAT_045e8ce0)
 */
int FUN_0044b030(void) {
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
    return DAT_0462e3b4 * 1800000 + 1000000;
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

/*
 * FUN_00444fb0 - Apply Direction Offset
 *
 * Binary analysis:
 * - Modifies x,y coordinates based on direction (0-7)
 * - Direction is stored in DAT_004bb758
 * - Direction mapping:
 *   0: North (y -= 2)
 *   1: Northeast (x += 1, y -= 1)
 *   2: East (x += 2)
 *   3: Southeast (x += 1, y += 1)
 *   4: South (y += 2)
 *   5: Southwest (x -= 1, y += 1)
 *   6: West (x -= 2)
 *   7: Northwest (x -= 1, y -= 1)
 */
void FUN_00444fb0(int* x, int* y) {
    extern s32 DAT_004bb758;

    if (DAT_004bb758 < 0) return;

    switch (DAT_004bb758) {
        case 0:  /* North */
            *y -= 2;
            break;
        case 1:  /* Northeast */
            *x += 1;
            *y -= 1;
            break;
        case 2:  /* East */
            *x += 2;
            break;
        case 3:  /* Southeast */
            *x += 1;
            *y += 1;
            break;
        case 4:  /* South */
            *y += 2;
            break;
        case 5:  /* Southwest */
            *x -= 1;
            *y += 1;
            break;
        case 6:  /* West */
            *x -= 2;
            break;
        case 7:  /* Northwest */
            *x -= 1;
            *y -= 1;
            break;
    }
}

/* FUN_004777e0 is defined in stubs_ui.c */

/*
 * FUN_00447370 - Distance Squared
 *
 * Binary analysis:
 * - Calculates squared distance between two points
 * - param_1: x1 coordinate
 * - param_2: y1 coordinate
 * - param_3: x2 coordinate
 * - param_4: y2 coordinate
 * - Returns: (x2-x1)^2 + (y2-y1)^2
 * - Used for distance comparisons without sqrt
 */
float FUN_00447370(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return dx * dx + dy * dy;
}

/*
 * FUN_004473f0 - Check Sprite ID Range
 *
 * Binary analysis:
 * - Checks if sprite ID is in specific range (0x623e, 0x6299)
 * - param_1: sprite ID
 * - Returns: 1 if in range, 0 otherwise
 * - Range appears to be special effect sprites
 */
int FUN_004473f0(int sprite_id) {
    if (sprite_id > 0x623e && sprite_id < 0x6299) {
        return 1;
    }
    return 0;
}

/*
 * FUN_00447310 - Initialize Random Shuffle Table
 *
 * Binary analysis:
 * - Initializes a 100-element shuffle table with sequential indices
 * - Uses rand() % 100 for starting position
 * - Table at DAT_04582a40 (100 integers)
 * - Used for randomizing game elements
 */
void FUN_00447310(void) {
    extern int DAT_04582a40[100];
    int start = rand() % 100;
    int i;

    for (i = 0; i < 100; i++) {
        DAT_04582a40[start] = i;
        start++;
        if (start > 99) {
            start = 0;
        }
    }
}

/*
 * FUN_00447340 - Get Next Shuffle Value
 *
 * Binary analysis:
 * - Returns next value from the shuffle table (circular buffer)
 * - Table at DAT_04582a40 (100 integers)
 * - Index at DAT_04582bd0, increments and wraps at 100
 * - Used for randomizing game elements without repetition
 */
int FUN_00447340(void) {
    extern int DAT_04582a40[100];
    extern int DAT_04582bd0;  /* Current index in shuffle table */

    int* ptr = &DAT_04582a40[DAT_04582bd0];
    int result = *ptr;

    DAT_04582bd0++;
    if (DAT_04582bd0 > 99) {
        DAT_04582bd0 = 0;
    }

    return result;
}

/*
 * FUN_004472e0 - Random Number in Range
 *
 * Binary analysis:
 * - Returns random integer in range [param_1, param_2]
 * - Uses rand() internally
 * - Returns 0 if param_2 == -1
 */
int FUN_004472e0(int min_val, int max_val) {
    if (max_val == -1) {
        return 0;
    }
    return rand() % (max_val - min_val + 1) + min_val;
}

/*
 * FUN_00443510 - Check if Tile is Walkable
 *
 * Binary analysis:
 * - Checks if a tile at given coordinates is walkable (type 2)
 * - param_1: x coordinate
 * - param_2: y coordinate
 * - Returns: true if tile type is 2, false otherwise
 * - Uses map data at DAT_04581280 with stride DAT_045602b0
 * - Coordinate offset from DAT_04560e3c and DAT_04560e44
 * - Note: Original data is u32, but treats lower 12 bits as tile type
 */
int FUN_00443510(int x, int y) {
    extern u32 DAT_04581280;
    extern u32 DAT_045602b0;  /* Map stride */
    extern u32 DAT_04560e3c;  /* Map offset Y */
    extern u32 DAT_04560e44;  /* Map offset X */

    /* Calculate index into tile data array */
    /* Original uses u16 pointer arithmetic, we use u32 */
    u16* tile_data = (u16*)&DAT_04581280;
    int index = ((y - (int)DAT_04560e3c) * (int)DAT_045602b0 - (int)DAT_04560e44) + x;
    u16 tile = tile_data[index];

    return (tile & 0xfff) == 2;
}

/*
 * FUN_004474e0 - Clamp Value to Range
 *
 * Binary analysis:
 * - Clamps a value to be within [min, max] range
 * - param_1: value to clamp
 * - param_2: minimum value
 * - param_3: maximum value
 * - Returns: clamped value
 */
int FUN_004474e0(int value, int min_val, int max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

/*
 * FUN_00447520 - Linear Interpolation
 *
 * Binary analysis:
 * - Performs linear interpolation between two values
 * - param_1: start value
 * - param_2: end value
 * - param_3: interpolation factor (0.0 to 1.0)
 * - Returns: interpolated value
 */
float FUN_00447520(float a, float b, float t) {
    return a + (b - a) * t;
}

/*
 * FUN_00492403 - Linear Congruential Generator (LCG) Random
 *
 * Binary analysis:
 * - Standard LCG random number generator
 * - Uses formula: state = state * 0x343fd + 0x269ec3
 * - Returns: (state >> 16) & 0x7fff (15-bit value)
 * - State stored in DAT_004d7160
 */
u32 FUN_00492403(void) {
    extern u32 DAT_004d7160;  /* LCG state */

    DAT_004d7160 = DAT_004d7160 * 0x343fd + 0x269ec3;
    return (DAT_004d7160 >> 16) & 0x7fff;
}

/*
 * FUN_004423d0 - Clamp Coordinates to Map Bounds
 *
 * Binary analysis:
 * - Clamps coordinate rectangle to map boundaries
 * - param_1: pointer to left X (clamped to >= 0)
 * - param_2: pointer to top Y (clamped to >= 0)
 * - param_3: pointer to right X (clamped to DAT_0458119c)
 * - param_4: pointer to bottom Y (clamped to DAT_04569b70)
 * - Used for ensuring coordinates are within valid map range
 */
void FUN_004423d0(short* left, short* top, short* right, short* bottom) {
    extern u32 DAT_0458119c;  /* Map width */
    extern u32 DAT_04569b70;  /* Map height */

    if (*left < 0) {
        *left = 0;
    }
    if (*top < 0) {
        *top = 0;
    }
    if ((u32)*right > DAT_0458119c) {
        *right = (short)DAT_0458119c;
    }
    if ((u32)*bottom > DAT_04569b70) {
        *bottom = (short)DAT_04569b70;
    }
}

/*
 * FUN_00444580 - Direction Index to Character
 *
 * Binary analysis:
 * - Converts direction index (0-7) to a character for display
 * - param_1: direction index (0-7)
 * - param_2: uppercase flag (0 = lowercase, non-zero = uppercase)
 * - Returns: character representing the direction
 * - Directions 0-2: 'f'/'F' (forward variants)
 * - Directions 3-7: '^'/'>' variants
 */
char FUN_00444580(int direction, int uppercase) {
    if (direction >= 0 && direction < 3) {
        if (uppercase == 0) {
            return (char)direction + 'f';
        }
        return (char)direction + 'F';
    }
    if (direction < 8) {
        if (uppercase == 0) {
            return (char)direction + '^';
        }
        return (char)direction + '>';
    }
    return 'f';
}

/*
 * FUN_00444920 - Insert Node After in Linked List
 *
 * Binary analysis:
 * - Inserts a new node after an existing node in a doubly-linked list
 * - param_1: existing node (insert after this)
 * - param_2: new node to insert
 * - Node structure has prev at offset 0x24 and next at offset 0x28
 */
void FUN_00444920(void* existing, void* new_node) {
    void** existing_ptr = (void**)existing;
    void** new_ptr = (void**)new_node;

    if (existing == NULL || new_node == NULL) {
        return;
    }

    /* Set new node's prev to existing */
    new_ptr[9] = existing;  /* offset 0x24 = 9 * 4 */

    /* Set new node's next to existing's next */
    new_ptr[10] = existing_ptr[10];  /* offset 0x28 = 10 * 4 */

    /* Update existing's next's prev to new node */
    if (existing_ptr[10] != NULL) {
        void** next_ptr = (void**)existing_ptr[10];
        next_ptr[9] = new_node;
    }

    /* Set existing's next to new node */
    existing_ptr[10] = new_node;
}

/*
 * FUN_00444950 - Append Node to Linked List Tail
 *
 * Binary analysis:
 * - Appends a new node at the tail of a doubly-linked list
 * - param_1: current tail node
 * - param_2: new node to append
 * - Node structure has prev at offset 0x24 and next at offset 0x28
 */
void FUN_00444950(void* tail, void* new_node) {
    void** tail_ptr = (void**)tail;
    void** new_ptr = (void**)new_node;

    if (tail == NULL || new_node == NULL) {
        return;
    }

    /* Set new node's prev to tail */
    new_ptr[9] = tail;  /* offset 0x24 = 9 * 4 */

    /* Set new node's next to tail's next (usually NULL) */
    new_ptr[10] = tail_ptr[10];  /* offset 0x28 = 10 * 4 */

    /* Update tail's next to new node */
    tail_ptr[10] = new_node;
}

/*
 * FUN_00446e40 - Isometric Coordinate Transformation
 *
 * Binary analysis:
 * - Converts world coordinates (x, y) to screen coordinates (iso_x, iso_y)
 * - Uses isometric projection formula
 * - param_1: world X coordinate
 * - param_2: world Y coordinate
 * - param_3: output screen X
 * - param_4: output screen Y
 * - Uses DAT_004bb424, DAT_004bb428 as offsets
 * - Uses _DAT_0049c3ec as isometric scale factor
 */
void FUN_00446e40(float world_x, float world_y, float* screen_x, float* screen_y) {
    extern u32 DAT_004bb424;  /* Offset X */
    extern u32 DAT_004bb428;  /* Offset Y */
    extern float _DAT_0049c3ec;  /* Isometric scale (0.5) */
    extern float _DAT_04582998;  /* Screen offset X */
    extern float _DAT_04582994;  /* Screen offset Y */

    float offset_x = (float)DAT_004bb424;
    float iso_factor = (world_y - (float)DAT_004bb428) * _DAT_0049c3ec;

    *screen_x = ((world_x - offset_x) - iso_factor) + _DAT_04582998;
    *screen_y = iso_factor + (world_x - offset_x) + _DAT_04582994;
}



