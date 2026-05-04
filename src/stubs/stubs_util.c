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
