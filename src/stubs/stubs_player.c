/*
 * Stone Age Client - Player Entity Functions
 * Split from stubs_entity.c to reduce file size
 */

#include <windows.h>
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
extern void FUN_00477cb0(int x, int y);
extern void FUN_00477cd0(void);

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
