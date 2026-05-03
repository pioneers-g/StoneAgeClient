/*
 * Stone Age Client - Entity Flag Manipulation Functions
 * Split from stubs_entity.c to reduce file size
 */

#include <windows.h>
#include "types.h"

/* ========================================
 * Entity Flag Manipulation Functions
 * ======================================== */

/*
 * Entity flag array: DAT_004e2b4c
 * Stride: 0x43 (67 dwords) per entity
 * Entity ID is at offset 0xc from entity pointer
 * Flags are stored at DAT_004e2b4c[entity_id * 0x43]
 */

/* External entity flag array - defined in stubs_globals.c */
extern u32 DAT_004e2b4c[];

/*
 * FUN_0040c020 - Set Entity Flag 0x100 (Hidden/Invisible)
 *
 * Binary analysis:
 * - Sets bit 8 (0x100) in entity flags
 * - param_1: entity pointer
 * - Entity ID retrieved from offset 0xc
 */
void FUN_0040c020(void* entity) {
    int* ent = (int*)entity;
    int entity_id;

    if (entity == NULL) return;

    entity_id = *(int*)(ent[3]);  /* offset 0xc */
    DAT_004e2b4c[entity_id * 0x43] |= 0x100;
}

/*
 * FUN_0040c050 - Clear Entity Flag 0x100 (Make Visible)
 *
 * Binary analysis:
 * - Clears bit 8 (0x100) in entity flags
 * - param_1: entity pointer
 */
void FUN_0040c050(void* entity) {
    int* ent = (int*)entity;
    int entity_id;

    if (entity == NULL) return;

    entity_id = *(int*)(ent[3]);  /* offset 0xc */
    DAT_004e2b4c[entity_id * 0x43] &= 0xfffffeff;
}

/*
 * FUN_0040c080 - Set Entity Flag 0x200 (Highlighted)
 *
 * Binary analysis:
 * - Sets bit 9 (0x200) in entity flags
 * - param_1: entity pointer
 */
void FUN_0040c080(void* entity) {
    int* ent = (int*)entity;
    int entity_id;

    if (entity == NULL) return;

    entity_id = *(int*)(ent[3]);  /* offset 0xc */
    DAT_004e2b4c[entity_id * 0x43] |= 0x200;
}

/*
 * FUN_0040c0b0 - Clear Entity Flag 0x200 (Remove Highlight)
 *
 * Binary analysis:
 * - Clears bit 9 (0x200) in entity flags
 * - param_1: entity pointer
 */
void FUN_0040c0b0(void* entity) {
    int* ent = (int*)entity;
    int entity_id;

    if (entity == NULL) return;

    entity_id = *(int*)(ent[3]);  /* offset 0xc */
    DAT_004e2b4c[entity_id * 0x43] &= 0xfffffdff;
}

/*
 * FUN_0040c0e0 - Set Entity Flag 0x4000 (Special Render)
 *
 * Binary analysis:
 * - Sets bit 14 (0x4000) in entity flags
 * - param_1: entity pointer
 * - Used for special rendering effects (shadow, etc.)
 */
void FUN_0040c0e0(void* entity) {
    int* ent = (int*)entity;
    int entity_id;

    if (entity == NULL) return;

    entity_id = *(int*)(ent[3]);  /* offset 0xc */
    DAT_004e2b4c[entity_id * 0x43] |= 0x4000;
}

/*
 * FUN_0040c110 - Clear Entity Flag 0x4000
 *
 * Binary analysis:
 * - Clears bit 14 (0x4000) in entity flags
 * - param_1: entity pointer
 */
void FUN_0040c110(void* entity) {
    int* ent = (int*)entity;
    int entity_id;

    if (entity == NULL) return;

    entity_id = *(int*)(ent[3]);  /* offset 0xc */
    DAT_004e2b4c[entity_id * 0x43] &= 0xffffbfff;
}

/*
 * FUN_0040c240 - Set Entity Flag 0x8000 (Ride/Mount State)
 *
 * Binary analysis:
 * - Sets bit 15 (0x8000) in entity flags
 * - param_1: entity pointer
 * - Used when entity is riding a mount
 */
void FUN_0040c240(void* entity) {
    int* ent = (int*)entity;
    int entity_id;

    if (entity == NULL) return;

    entity_id = *(int*)(ent[3]);  /* offset 0xc */
    DAT_004e2b4c[entity_id * 0x43] |= 0x8000;
}

/*
 * FUN_0040c270 - Clear Entity Flag 0x8000
 *
 * Binary analysis:
 * - Clears bit 15 (0x8000) in entity flags
 * - param_1: entity pointer
 */
void FUN_0040c270(void* entity) {
    int* ent = (int*)entity;
    int entity_id;

    if (entity == NULL) return;

    entity_id = *(int*)(ent[3]);  /* offset 0xc */
    DAT_004e2b4c[entity_id * 0x43] &= 0xffff7fff;
}

/*
 * FUN_0040c2a0 - Set Entity Flag 0x10000 (Combat/Action State)
 *
 * Binary analysis:
 * - Sets bit 16 (0x10000) in entity flags
 * - param_1: entity pointer
 * - Used during combat or action sequences
 */
void FUN_0040c2a0(void* entity) {
    int* ent = (int*)entity;
    int entity_id;

    if (entity == NULL) return;

    entity_id = *(int*)(ent[3]);  /* offset 0xc */
    DAT_004e2b4c[entity_id * 0x43] |= 0x10000;
}

/*
 * FUN_0040c2d0 - Clear Entity Flag 0x10000
 *
 * Binary analysis:
 * - Clears bit 16 (0x10000) in entity flags
 * - param_1: entity pointer
 */
void FUN_0040c2d0(void* entity) {
    int* ent = (int*)entity;
    int entity_id;

    if (entity == NULL) return;

    entity_id = *(int*)(ent[3]);  /* offset 0xc */
    DAT_004e2b4c[entity_id * 0x43] &= 0xfffeffff;
}

/*
 * FUN_0040c300 - Clear Entity Flag 0x800 (Animation State)
 *
 * Binary analysis:
 * - Clears bit 11 (0x800) in entity flags
 * - param_1: entity pointer
 * - Used to end special animation state
 */
void FUN_0040c300(void* entity) {
    int* ent = (int*)entity;
    int entity_id;

    if (entity == NULL) return;

    entity_id = *(int*)(ent[3]);  /* offset 0xc */
    DAT_004e2b4c[entity_id * 0x43] &= 0xfffff7ff;
}

/*
 * FUN_0040c330 - Set Entity Timer Flag
 *
 * Binary analysis:
 * - Sets bit 13 (0x2000) in entity flags and starts timer
 * - param_1: entity pointer
 * - param_2: timer duration in milliseconds
 * - Timer stored at entity->extra[3] (offset 0xc from extra struct)
 * - Uses timeGetTime() to get current time
 */
void FUN_0040c330(void* entity, int duration_ms) {
    int* ent = (int*)entity;
    int* extra;
    int entity_id;
    DWORD current_time;

    if (entity == NULL) return;

    extra = (int*)ent[3];  /* offset 0xc */
    entity_id = *extra;

    DAT_004e2b4c[entity_id * 0x43] |= 0x2000;

    current_time = timeGetTime();
    extra[3] = current_time + duration_ms;  /* offset 0xc in extra struct */
}

/*
 * FUN_0040c210 - Clear Entity Flag 0x400 (Target Indicator)
 *
 * Binary analysis:
 * - Clears bit 10 (0x400) in entity flags
 * - param_1: entity pointer
 */
void FUN_0040c210(void* entity) {
    int* ent = (int*)entity;
    int entity_id;

    if (entity == NULL) return;

    entity_id = *(int*)(ent[3]);  /* offset 0xc */
    DAT_004e2b4c[entity_id * 0x43] &= 0xffffebff;
}

/*
 * FUN_0040c1a0 - Set Entity Dialog/Target State
 *
 * Binary analysis:
 * - Sets bit 10 (0x400) in entity flags
 * - param_1: entity pointer
 * - param_2: dialog/action type
 * - param_3: dialog parameter 1
 * - param_4: dialog parameter 2 (if non-zero, sets bit 12)
 * - Stores params in DAT_004e2bcc array (stride 0x10c)
 */
void FUN_0040c1a0(void* entity, int dialog_type, u16 param1, short param2) {
    int* ent = (int*)entity;
    int entity_id;

    if (entity == NULL) return;

    entity_id = *(int*)(ent[3]);  /* offset 0xc */

    /* Set dialog flag */
    DAT_004e2b4c[entity_id * 0x43] |= 0x400;

    /* Store dialog parameters at stride 0x10c offsets */
    /* DAT_004e2bcc[entity_id * 0x43] = dialog_type; */
    /* DAT_004e2bd0[entity_id * 0x43] = param1; */
    /* DAT_004e2bd2[entity_id * 0x43] = param2; */

    if (param2 != 0) {
        DAT_004e2b4c[entity_id * 0x43] |= 0x1000;
    } else {
        DAT_004e2b4c[entity_id * 0x43] &= 0xffffefff;
    }
}
