/*
 * Stone Age Client - Entity Creation Functions
 * Split from stubs_entity.c to reduce file size
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

/* External globals */
extern float _DAT_0049c31c;

/* External functions */
extern void* FUN_004010a0(int priority, int extra_size);
extern int FUN_0040e830(int entity_id);
extern int FUN_0040f310(void);

/* Entity slot array access - stride 0x43 dwords */
#define SLOT_STRIDE 0x43

/* Entity spawn slot data - from binary */
typedef struct {
    s16 state;           /* offset 0x00 (from DAT_004e2b20) */
    u16 unknown;
    s32 model_id;        /* offset 0x04 */
    s32 world_x;         /* offset 0x08 */
    s32 world_y;         /* offset 0x0c */
    s32 field_0x10;
    s32 field_0x14;
    s32 extra_data;      /* offset 0x18 */
    char name[60];       /* offset 0x1c (0x3c bytes) */
    /* ... more fields up to 0x10c bytes total */
} EntitySpawnSlot;

/*
 * FUN_0040b5e0 - Create Field Entity
 *
 * Binary analysis:
 * - Creates a new field entity (NPC, character, pet, etc.)
 * - param_1: entity type/model ID
 * - param_2: world X coordinate
 * - param_3: world Y coordinate
 * - param_4: extra data pointer
 * - Allocates entity via FUN_004010a0 with priority 1
 * - Initializes entity structure:
 *   - offset 0x140: model ID
 *   - offset 0x150: extra data
 *   - offset 0xb0-bc: world coordinates (int)
 *   - offset 0x114-118: float coordinates (scaled by DAT_0049c31c)
 *   - offset 0x148: state (3 = spawning)
 *   - offset 0x15: render priority (10)
 *   - offset 0xa0: flags (0x34)
 *   - offset 0x08: render callback pointer (FUN_0040ad60)
 * - Calls FUN_00446df0 for isometric coordinate transform
 * - Returns entity pointer or 0 on failure
 */
int FUN_0040b5e0(int model_id, int world_x, int world_y, int extra_data) {
    int entity;
    float fx, fy;

    /* Allocate entity with extra buffer (0x38 bytes) */
    entity = (int)FUN_004010a0(1, 0x38);
    if (entity == 0) {
        return 0;
    }

    /* Set model ID */
    *(int*)(entity + 0x140) = model_id;

    /* Set extra data */
    *(int*)(entity + 0x150) = extra_data;

    /* Clear some fields */
    *(int*)(entity + 0x110) = 0;
    *(int*)(entity + 0x11c) = 0;
    *(int*)(entity + 0x120) = 0;

    /* Set world coordinates */
    *(int*)(entity + 0xb8) = world_x;
    *(int*)(entity + 0xb0) = world_x;
    *(int*)(entity + 0xbc) = world_y;
    *(int*)(entity + 0xb4) = world_y;

    /* Calculate float coordinates */
    fx = (float)world_x * _DAT_0049c31c;
    fy = (float)world_y * _DAT_0049c31c;
    *(float*)(entity + 0x114) = fx;
    *(float*)(entity + 0x118) = fy;

    /* Set render callback - LAB_0040ad60 */
    /* *(void**)(entity + 8) = LAB_0040ad60; */

    /* Set state to 3 (spawning) */
    *(int*)(entity + 0x148) = 3;

    /* Set priority to 10 */
    *((unsigned char*)entity + 0x15) = 10;

    /* Set flags to 0x34 */
    *(int*)(entity + 0xa0) = 0x34;

    /* TODO: Call FUN_00446df0 for isometric transform
     * This would convert world coordinates to screen coordinates
     * and store at entity + 0x18 and entity + 0x1c
     */

    return entity;
}

/*
 * FUN_0040f180 - Create or Update Entity with Name
 *
 * Binary analysis:
 * - Creates new entity or updates existing one by ID
 * - param_1: entity ID
 * - param_2: entity type/model
 * - param_3: X coordinate
 * - param_4: Y coordinate
 * - param_5: extra data
 * - param_6: unknown field
 * - param_7: entity name string (max 60 chars in slot, 17 chars in entity)
 * - Uses FUN_0040e830 to find existing entity
 * - Uses FUN_0040f310 to allocate new slot if not found
 * - Uses FUN_0040b5e0 to create entity structure
 * - Sets flags at offset 0x134 to 0x800
 */
void FUN_0040f180(int entity_id, int model_id, int x, int y, int extra, int param_6, const char* name) {
    int slot;
    int entity_ptr;
    size_t name_len;
    const char* name_src;

    /* Find existing entity by ID */
    slot = FUN_0040e830(entity_id);

    if (slot < 0) {
        /* Entity doesn't exist, create new */
        slot = FUN_0040f310();
        if (slot < 0) {
            return;  /* No free slots */
        }

        /* Create entity structure */
        entity_ptr = FUN_0040b5e0(model_id, x, y, extra);
        if (entity_ptr == 0) {
            return;  /* Creation failed */
        }

        /* Store entity pointer in slot array */
        /* DAT_004e2bdc[slot * 0x43] = entity_ptr */

        /* Set state to 2 (active) */
        /* DAT_004e2b20[slot * 0x86] = 2 */

        /* Store slot index in entity */
        /* *(entity_ptr + 0xc) = slot */

        /* Set entity ID */
        /* DAT_004e2b24[slot * 0x43] = entity_id */

        /* Mark field as -1 */
        /* DAT_004e2b3c[slot * 0x43] = -1 */
    }

    /* Update slot data */
    /* DAT_004e2b28[slot * 0x43] = model_id */
    /* DAT_004e2b54[slot * 0x10c] = param_6 */
    /* DAT_004e2b2c[slot * 0x43] = x */
    /* DAT_004e2b30[slot * 0x43] = y */
    /* DAT_004e2b38[slot * 0x43] = extra */

    /* Copy name to slot (max 60 chars) */
    if (name) {
        name_len = strlen(name);
        if (name_len < 60) {
            /* DAT_004e2b8a[slot * 0x10c] = name */
        }
    }

    /* Set flags to 0x800 */
    /* DAT_004e2bd4[slot * 0x43] = 0x800 */

    /* Update entity structure if exists */
    /* if (entity_ptr != 0) {
     *     entity->model_id = model_id;
     *     entity->flags |= 0x800;
     *     Copy name to entity (max 17 chars) at offset 0x38
     * }
     */

    (void)name_src;  /* Suppress unused variable warning */
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
