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
