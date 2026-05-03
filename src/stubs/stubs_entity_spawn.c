/*
 * Stone Age Client - Entity Creation Functions
 * Split from stubs_entity.c to reduce file size
 */

#include <windows.h>
#include "types.h"

/* External globals */
extern float _DAT_0049c31c;

/* Forward declaration */
extern void* FUN_004010a0(int priority, int extra_size);

/*
 * FUN_0040db20 - Set Entity Direction
 *
 * Binary analysis:
 * - Sets direction for entity and syncs with slot data
 * - param_1: entity pointer
 * - param_2: direction value
 * - Entity ID retrieved from offset 0xc
 * - Stores in entity offset 0x98 and DAT_004e2be0[slot * 0x10c]
 */
void FUN_0040db20(void* entity, int direction) {
    int* ent = (int*)entity;
    int entity_id;

    if (entity == NULL) return;

    entity_id = *(int*)(ent[3]);  /* offset 0xc */
    *(int*)((char*)entity + 0x98) = direction;
    /* DAT_004e2be0[entity_id * 0x43] = direction; */
    (void)entity_id;
}

/*
 * FUN_0040db50 - Create Simple Entity
 *
 * Binary analysis:
 * - Creates a basic entity with sprite and position
 * - param_1: sprite ID
 * - param_2: X position
 * - param_3: Y position
 * - param_4: state
 * - param_5: extra data
 * - param_6: render priority
 * - Returns entity pointer or 0 on failure
 * - Uses FUN_004010a0 for allocation
 * - Calls FUN_00446df0 for isometric transform
 */
int FUN_0040db50(int sprite_id, int x, int y, int state, int extra, char priority) {
    int entity;
    float fx, fy;
    int screen_x, screen_y;

    entity = (int)FUN_004010a0(1, 0);
    if (entity == 0) return 0;

    *(int*)(entity + 0x140) = sprite_id;
    *(int*)(entity + 0x148) = state;
    *((char*)entity + 0x15) = priority;
    *(int*)(entity + 0xbc) = y;
    *(int*)(entity + 0xb8) = x;
    *(int*)(entity + 0xb0) = x;
    *(int*)(entity + 0xb4) = y;
    *(int*)(entity + 0x150) = extra;
    *(int*)(entity + 0xa0) = 0x20;  /* flags */
    *(int*)(entity + 0x11c) = 0;    /* velocity X */
    *(int*)(entity + 0x120) = 0;    /* velocity Y */

    fx = (float)x * _DAT_0049c31c;
    fy = (float)y * _DAT_0049c31c;
    *(float*)(entity + 0x114) = fx;
    *(float*)(entity + 0x118) = fy;

    /* TODO: Call FUN_00446df0 for isometric transform */
    screen_x = x;
    screen_y = y;
    *(int*)(entity + 0x18) = screen_x;
    *(int*)(entity + 0x1c) = screen_y;

    return entity;
}

/*
 * FUN_0040dcd0 - Create Entity with Callback
 *
 * Binary analysis:
 * - Same as FUN_0040db50 but sets render callback
 * - Callback set to LAB_0040dc40
 */
int FUN_0040dcd0(int sprite_id, int x, int y, int state, int extra, char priority) {
    int entity;
    float fx, fy;
    int screen_x, screen_y;

    entity = (int)FUN_004010a0(1, 0);
    if (entity == 0) return 0;

    *(int*)(entity + 0x140) = sprite_id;
    *(int*)(entity + 0x148) = state;
    *((char*)entity + 0x15) = priority;
    *(int*)(entity + 0xbc) = y;
    *(int*)(entity + 0xb8) = x;
    *(int*)(entity + 0xb0) = x;
    *(int*)(entity + 0xb4) = y;
    *(int*)(entity + 0x150) = extra;
    *(int*)(entity + 0xa0) = 0x20;
    *(float*)(entity + 0x11c) = 0.0f;
    *(float*)(entity + 0x120) = 0.0f;

    fx = (float)x * _DAT_0049c31c;
    fy = (float)y * _DAT_0049c31c;
    *(float*)(entity + 0x114) = fx;
    *(float*)(entity + 0x118) = fy;

    /* Set render callback */
    /* *(void**)(entity + 8) = LAB_0040dc40; */

    /* TODO: Call FUN_00446df0 for isometric transform */
    screen_x = x;
    screen_y = y;
    *(int*)(entity + 0x18) = screen_x;
    *(int*)(entity + 0x1c) = screen_y;

    return entity;
}
