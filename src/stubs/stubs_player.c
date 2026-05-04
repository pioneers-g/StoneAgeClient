/*
 * Stone Age Client - Player Entity Functions
 * Split from stubs_entity.c to reduce file size
 */

#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include "types.h"

/* External globals */
extern void* DAT_0462e3ac;
extern u32 DAT_0462bf2c;
extern char DAT_0462bef8[];
extern char DAT_0462bf09[];
extern int DAT_0462be98;
extern int DAT_0462be9c;
extern int DAT_0462bec0;
extern u32 DAT_0462be88;
extern u32 DAT_0462be94;
extern u32 DAT_0462be90;
extern u32 DAT_04581d3c;
extern u32 DAT_04581d40;

/* External functions */
extern void* FUN_0040b5e0(int model_id, int x, int y, int extra);
extern void FUN_00477c70(int model_id, int extra);
extern void FUN_00477cd0(void);
extern void FUN_00419a40(void);
extern void FUN_0041cdd0(void);

/*
 * FUN_00440df0 - Set World Position
 *
 * Binary analysis:
 * - Sets world position globals
 * - Updates DAT_04581d3c, DAT_04581d40 (world X, Y)
 * - Updates float position at DAT_0456a644, DAT_0456a648
 * - Calls FUN_00419a40 and FUN_0041cdd0 (not implemented yet)
 */
void FUN_00440df0(int x, int y) {
    extern u32 DAT_04581d3c;
    extern u32 DAT_04581d40;
    extern float DAT_0456a644;
    extern float DAT_0456a648;
    extern float _DAT_0049c31c;
    extern u32 DAT_0458118c;
    extern u32 DAT_04581184;
    extern u32 DAT_004bb414;
    extern u32 DAT_004bb418;
    extern u32 _DAT_004bb41c;
    extern u32 _DAT_004bb420;
    extern float _DAT_045827fc;
    extern float _DAT_04582800;
    extern u32 DAT_0455ef9c;
    extern float _DAT_04582994;
    extern float _DAT_04582998;

    DAT_04581d3c = x;
    DAT_0458118c = x;
    DAT_0456a644 = (float)x * _DAT_0049c31c;
    DAT_04581d40 = y;
    DAT_004bb414 = 0xffffffff;
    DAT_004bb418 = 0xffffffff;
    _DAT_004bb41c = 0xffffffff;
    _DAT_004bb420 = 0xffffffff;
    DAT_04581184 = y;
    _DAT_045827fc = 0;
    _DAT_04582800 = 0;
    DAT_0456a648 = (float)y * _DAT_0049c31c;
    DAT_0455ef9c = 1;
    _DAT_04582994 = DAT_0456a648;
    _DAT_04582998 = DAT_0456a644;
    /* TODO: FUN_00419a40(); FUN_0041cdd0(); - not yet implemented */
}

/*
 * FUN_00477cb0 - Set Player World Position
 *
 * Binary analysis:
 * - Wrapper function that calls FUN_00440df0
 * - Sets player entity position in world coordinates
 */
void FUN_00477cb0(int x, int y) {
    FUN_00440df0(x, y);
}

/*
 * FUN_00477fd0 - Update Player Entity Data
 *
 * Binary analysis:
 * - Copies player name and title from globals to entity
 * - Copies from DAT_0462bef8 to entity offset 0x38 (name, max 17 chars)
 * - Copies from DAT_0462bf09 to entity offset 0x55 (title, max 33 chars)
 * - Sets HP at offset 0x78 from DAT_0462be98
 * - Sets MP at offset 0x80 from DAT_0462be9c
 * - Sets level at offset 0x8c from DAT_0462bec0
 */
void FUN_00477fd0(void) {
    char* entity;
    char* src;
    int len, i;

    if (DAT_0462e3ac == NULL) return;

    entity = (char*)DAT_0462e3ac;

    /* Copy name (max 17 chars) */
    src = DAT_0462bef8;
    for (len = 0; len < 17 && src[len] != '\0'; len++);
    if (len < 17) {
        for (i = 0; i < len; i++) {
            entity[0x38 + i] = src[i];
        }
    }

    /* Copy title (max 33 chars) */
    src = DAT_0462bf09;
    for (len = 0; len < 33 && src[len] != '\0'; len++);
    if (len < 33) {
        for (i = 0; i < len; i++) {
            entity[0x55 + i] = src[i];
        }
    }

    /* Copy stats */
    *(int*)(entity + 0x8c) = DAT_0462bec0;  /* level */
    *(int*)(entity + 0x78) = DAT_0462be98;  /* HP */
    *(int*)(entity + 0x80) = DAT_0462be9c;  /* MP */
}

/*
 * FUN_00478090 - Set Player Ride Flag
 *
 * Binary analysis:
 * - Sets bit 8 (0x100) in DAT_0462bf2c
 * - Used to indicate player is riding a mount
 */
void FUN_00478090(void) {
    DAT_0462bf2c |= 0x100;
}

/*
 * FUN_004780a0 - Clear Player Ride Flag
 *
 * Binary analysis:
 * - Clears bit 8 (0x100) in DAT_0462bf2c
 * - Used when player dismounts
 */
void FUN_004780a0(void) {
    DAT_0462bf2c &= 0xfffffeff;
}

/*
 * FUN_004780b0 - Set Player Highlight Flag
 *
 * Binary analysis:
 * - Sets bit 9 (0x200) in DAT_0462bf2c
 * - Used for player highlight effect
 */
void FUN_004780b0(void) {
    DAT_0462bf2c |= 0x200;
}

/*
 * FUN_004780c0 - Clear Player Highlight Flag
 *
 * Binary analysis:
 * - Clears bit 9 (0x200) in DAT_0462bf2c
 */
void FUN_004780c0(void) {
    DAT_0462bf2c &= 0xfffffdff;
}

/*
 * FUN_004780d0 - Set Player Special Render Flag
 *
 * Binary analysis:
 * - Sets bit 14 (0x4000) in DAT_0462bf2c
 * - Used for special rendering effects
 */
void FUN_004780d0(void) {
    DAT_0462bf2c |= 0x4000;
}

/*
 * FUN_00478110 - Clear Player Movement Flag
 *
 * Binary analysis:
 * - Clears movement flag at offset 0x112 in player entity
 * - Called when player stops moving
 */
void FUN_00478110(void) {
    if (DAT_0462e3ac != NULL) {
        *(unsigned short*)((char*)DAT_0462e3ac + 0x112) = 0;
    }
}

/*
 * FUN_00478150 - Clear Player Animation Flag
 *
 * Binary analysis:
 * - Clears bit 11 (0x800) in DAT_0462bf2c
 * - Used to end special animation state
 */
void FUN_00478150(void) {
    DAT_0462bf2c &= 0xfffff7ff;
}

/*
 * FUN_00477d70 - Set Player Direction with Entity Update
 *
 * Binary analysis:
 * - Sets DAT_0462be94 to param_1
 * - If player entity exists, also sets offset 0x150 (direction)
 */
void FUN_00477d70(u32 direction) {
    DAT_0462be94 = direction;
    if (DAT_0462e3ac != NULL) {
        *(u32*)((char*)DAT_0462e3ac + 0x150) = direction;
    }
}

/*
 * FUN_00477d90 - Set Player State
 *
 * Binary analysis:
 * - Sets player entity state at offset 0x148
 * - If state is not 4 (moving), clears offset 0x14c to -1
 */
void FUN_00477d90(int state) {
    if (DAT_0462e3ac != NULL) {
        *(int*)((char*)DAT_0462e3ac + 0x148) = state;
        if (*(int*)((char*)DAT_0462e3ac + 0x148) != 4) {
            *(int*)((char*)DAT_0462e3ac + 0x14c) = -1;
        }
    }
}

/*
 * FUN_00478130 - Get Player Movement Flag
 *
 * Binary analysis:
 * - Returns the movement flag at offset 0x112 from player entity
 * - Returns 0 if player entity is NULL
 */
int FUN_00478130(void) {
    if (DAT_0462e3ac == NULL) {
        return 0;
    }
    return (int)*(short*)((char*)DAT_0462e3ac + 0x112);
}

/*
 * FUN_00478190 - Set Player Render Color
 *
 * Binary analysis:
 * - Sets global DAT_0462e3b0 and entity offset 0x98 (render color)
 * - param_1: color value
 */
void FUN_00478190(u32 color) {
    extern u32 DAT_0462e3b0;
    DAT_0462e3b0 = color;
    if (DAT_0462e3ac != NULL) {
        *(u32*)((char*)DAT_0462e3ac + 0x98) = color;
    }
}

/*
 * FUN_004781b0 - Check Pet and Set Combat Flag
 *
 * Binary analysis:
 * - Iterates through entity array at DAT_0462bf50
 * - Stride is 0x184 (0x61 dwords) per entry
 * - If any entry has sprite ID 0x89c4, sets combat flag 0x10000
 * - Max 20 entities checked (end at 0x462ccf4)
 */
void FUN_004781b0(void) {
    int found = 0;
    extern u32 DAT_0462bf50[];
    int* entry = (int*)DAT_0462bf50;

    while ((int)entry < 0x462ccf4) {
        if (*entry == 0x89c4) {
            found = 1;
            break;
        }
        entry += 0x61;  /* Stride 0x184 bytes = 0x61 dwords */
    }

    if (found) {
        DAT_0462bf2c |= 0x10000;
    }
}

/*
 * FUN_004781e0 - Clear Player Combat Flag
 *
 * Binary analysis:
 * - Clears bit 16 (0x10000) in DAT_0462bf2c
 */
void FUN_004781e0(void) {
    DAT_0462bf2c &= 0xfffeffff;
}

/*
 * FUN_004781f0 - Player Action Dispatcher
 *
 * Binary analysis:
 * - Large switch statement handling player actions
 * - Actions include: idle, moving, direction change, position update, etc.
 * - Calls FUN_00477d90 (set state), FUN_00477d70 (set direction)
 * - Calls FUN_00477cb0 (set position), FUN_00478190 (set color)
 * - param_1: X position
 * - param_2: Y position
 * - param_3: direction
 * - param_4: action type
 * - param_5-7: additional parameters
 */
void FUN_004781f0(int x, int y, u32 direction, int action, int param5, int param6, int param7) {
    switch (action) {
        case 0:  /* Idle */
            FUN_00477d90(3);
            break;
        case 1:  /* Moving */
            FUN_00477d90(4);
            break;
        case 2:  /* Face direction, state 0 */
            FUN_00477d70(direction);
            FUN_00477d90(0);
            break;
        case 3:  /* Face direction, state 12 */
            FUN_00477d70(direction);
            FUN_00477d90(0xc);
            break;
        case 4:  /* Face direction, state 1 */
            FUN_00477d70(direction);
            FUN_00477d90(1);
            break;
        case 5:  /* Face direction, state 2 */
        case 10:
            FUN_00477d70(direction);
            FUN_00477d90(2);
            break;
        case 0xb:  /* Face direction, state 5 */
            FUN_00477d70(direction);
            FUN_00477d90(5);
            break;
        case 0xc:  /* Face direction, state 6 */
            FUN_00477d70(direction);
            FUN_00477d90(6);
            break;
        case 0xd:  /* Face direction, state 7 */
            FUN_00477d70(direction);
            FUN_00477d90(7);
            break;
        case 0xe:  /* Face direction, state 8 */
            FUN_00477d70(direction);
            FUN_00477d90(8);
            break;
        case 0xf:  /* Face direction, state 9 */
            FUN_00477d70(direction);
            FUN_00477d90(9);
            break;
        case 0x10:  /* Face direction, state 10 */
            FUN_00477d70(direction);
            FUN_00477d90(10);
            break;
        case 0x11:  /* Face direction, moving */
            FUN_00477d70(direction);
            FUN_00477d90(4);
            break;
        case 0x12:  /* Face direction, state 11 */
            FUN_00477d70(direction);
            FUN_00477d90(0xb);
            break;
        case 0x13:  /* Face direction, idle */
            FUN_00477d70(direction);
            FUN_00477d90(3);
            break;
        case 0x14:  /* Set position and direction */
            FUN_00477cb0(x, y);
            FUN_00477d70(direction);
            break;
        case 0x15:  /* Set position, direction, ride flag */
            FUN_00477cb0(x, y);
            FUN_00477d70(direction);
            if (param5 == 1) {
                FUN_00478090();  /* Set ride flag */
            } else {
                FUN_004780a0();  /* Clear ride flag */
            }
            break;
        case 0x16:  /* Set position, direction, special render flag */
            FUN_00477cb0(x, y);
            FUN_00477d70(direction);
            if (param5 == 1) {
                FUN_004780d0();  /* Set special render flag */
            }
            /* FUN_004780e0 would clear it but not implemented */
            break;
        case 0x17:  /* Set render color */
            FUN_00478190(param5);
            break;
        case 0x1e:  /* Set direction only */
            FUN_00477d70(direction);
            break;
        case 0x1f:  /* Set position, direction, idle */
            FUN_00477cb0(x, y);
            FUN_00477d70(direction);
            FUN_00477d90(3);
            break;
        case 0x22:  /* Set position, direction, combat flag */
            FUN_00477cb0(x, y);
            FUN_00477d70(direction);
            if (param5 == 1) {
                FUN_004781b0();  /* Set combat flag via pet check */
            } else {
                FUN_004781e0();  /* Clear combat flag */
            }
            break;
        default:
            break;
    }
}

/*
 * FUN_00478910 - Clear All Player Highlights
 *
 * Binary analysis:
 * - Iterates through player slot array starting at DAT_0463091c
 * - Stride 0x30 (0xc dwords) per entry
 * - For player's own entity: clears highlight flag via FUN_004780c0
 * - For other entities: clears highlight flag via FUN_0040c0b0
 * - Also clears queue count at offset -0xb (0x2c) from entry
 * - Clears entity ID at offset -10 from entry
 * - Clears entity pointer
 * - Finally clears player ride flag
 */
void FUN_00478910(void) {
    extern u32 DAT_0462be90;
    extern u32 DAT_0463091c[];
    int* entry = (int*)DAT_0463091c;

    do {
        if (*(short*)(entry - 0xb) != 0) {  /* Queue count at offset -0x2c */
            if (entry[-10] == (int)DAT_0462be90) {  /* Entity ID matches player */
                if (*entry != 0) {
                    *(unsigned short*)((char*)(size_t)*entry + 0x110) = 0;
                }
                FUN_004780c0();  /* Clear player highlight */
            } else {
                if ((DAT_0462bf2c & 0x100) != 0 && *entry != 0) {
                    *(unsigned short*)((char*)(size_t)*entry + 0x110) = 0;
                }
                /* FUN_0040c0b0(*entry) - clear entity highlight */
            }
        }
        *(unsigned short*)(entry - 0xb) = 0;
        entry[-10] = 0;
        *entry = 0;
        entry += 0xc;
    } while ((size_t)entry < 0x4630a0c);

    FUN_004780a0();  /* Clear player ride flag */
}

/*
 * FUN_004780e0 - Clear Player Special Render Flag
 *
 * Binary analysis:
 * - Clears bit 14 (0x4000) in DAT_0462bf2c
 */
void FUN_004780e0(void) {
    DAT_0462bf2c &= 0xffffbfff;
}

/*
 * FUN_004780f0 - Set Player Movement Flag to 1
 *
 * Binary analysis:
 * - Sets movement flag at offset 0x112 in player entity to 1
 */
void FUN_004780f0(void) {
    if (DAT_0462e3ac != NULL) {
        *(unsigned short*)((char*)DAT_0462e3ac + 0x112) = 1;
    }
}

/*
 * FUN_00478160 - Set Player Timeout Flag and Timer
 *
 * Binary analysis:
 * - Sets bit 13 (0x2000) in DAT_0462bf2c
 * - Sets timeout timer at entity->offset 0xc + 0xc
 * - param_1: timeout duration in milliseconds
 */
void FUN_00478160(int timeout_ms) {
    extern u32 DAT_046308a8;
    DWORD current_time;
    int entity_data;

    if (DAT_0462e3ac != NULL) {
        DAT_0462bf2c |= 0x2000;
        entity_data = *(int*)((char*)DAT_0462e3ac + 0xc);
        if (entity_data != 0) {
            current_time = timeGetTime();
            *(DWORD*)(entity_data + 0xc) = current_time + timeout_ms;
        }
    }
}

/*
 * FUN_00478980 - Clear Player Slot Array
 *
 * Binary analysis:
 * - Clears player slot array starting at DAT_0463091c
 * - Stride 0x30 (0xc dwords) per entry
 * - Iterates until address 0x4630a0c
 */
void FUN_00478980(void) {
    extern u32 DAT_0463091c[];
    u32* entry = DAT_0463091c;

    do {
        *entry = 0;
        entry += 0xc;
    } while ((size_t)entry < 0x4630a0c);
}

/*
 * FUN_004789a0 - Check Player Slot Occupied
 *
 * Binary analysis:
 * - Checks if player slot data exists at DAT_04630a00 + slot * 0x44
 * - param_1: slot index (0 or 1 only)
 * - Returns 1 if slot has data, 0 if empty, -1 if invalid slot
 */
u32 FUN_004789a0(int slot_index) {
    extern char DAT_04630a00[];

    if (slot_index < 0 || slot_index >= 2) {
        return 0xffffffff;  /* Invalid slot */
    }

    return (DAT_04630a00[slot_index * 0x44] != '\0') ? 1 : 0;
}

/*
 * FUN_004789d0 - Get Player Slot Name Pointer
 *
 * Binary analysis:
 * - Returns pointer to player slot name at DAT_04630a00 + slot * 0x44
 * - param_1: slot index (0 or 1 only)
 * - Returns pointer or NULL if invalid
 */
char* FUN_004789d0(int slot_index) {
    extern char DAT_04630a00[];

    if (slot_index < 0 || slot_index >= 2) {
        return NULL;
    }

    return &DAT_04630a00[slot_index * 0x44];
}
