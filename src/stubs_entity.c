/*
 * Stone Age Client - Entity Stub Functions
 * Split from stubs.c to reduce file size
 */

#include <windows.h>
#include <stdlib.h>
#include "types.h"

/* External globals */
extern u32 s_entity_count;
extern void* s_battle_units;
extern u32 s_battle_unit_count;

/*
 * FUN_004010a0 - Entity Allocation and List Insertion
 *
 * Binary analysis:
 * - Allocates a 500-byte entity structure via FUN_00491f70
 * - param_1: priority/sorting value (stored at offset 5)
 * - param_2: optional extra buffer size (stored at offset 0xc)
 * - Initializes:
 *   - offset 0x14: render callback pointer (initially 0)
 *   - offset 0x15: priority byte
 *   - offset 0x9c: render order (-1)
 *   - offset 0x20: state (-2)
 * - Inserts into sorted linked list at DAT_04630fa0
 * - List sorted by priority (lower value = later in list)
 * - Returns pointer to allocated entity or NULL on failure
 * - Shows error message on allocation failure
 */
void* FUN_004010a0(int param_1) {
    (void)param_1;
    void* entity = calloc(1, 500);  /* 500-byte struct */
    if (entity) {
        s_entity_count++;
    }
    return entity;
}

/*
 * FUN_004011d0 - Entity Free
 *
 * Binary analysis:
 * - Frees entity and removes from linked list
 * - Decrements entity count
 */
void FUN_004011d0(intptr_t entity_ptr) {
    if (entity_ptr) {
        free((void*)entity_ptr);
        if (s_entity_count > 0) {
            s_entity_count--;
        }
    }
}

/*
 * FUN_00401170 - Field Entity Update Loop
 *
 * Binary analysis:
 * - Updates all entities in the field
 * - Handles movement, animation, actions
 */
void FUN_00401170(void) {
    /* Update all field entities */
    /* TODO: Full implementation with entity iteration */
}

/*
 * FUN_0040ddd0 - Entity Action Dispatcher
 *
 * Binary analysis:
 * - Dispatches entity actions (30+ action types)
 * - param_1: entity pointer
 * - param_2: action type
 */
void FUN_0040ddd0(void* entity, int action) {
    (void)entity; (void)action;
    /* TODO: Full action dispatch implementation */
}

/*
 * FUN_0040b6e0 - Entity Movement Queue Add
 *
 * Binary analysis:
 * - Adds a movement target to entity's path queue
 * - param_1: entity pointer
 * - param_2: target X coordinate
 * - param_3: target Y coordinate
 * - Queue stored at offset 0xc0 (X) and 0xe8 (Y), max 10 entries
 * - Queue count at offset 0x110
 * - If queue is full (10 entries), resets count to 0 and calls FUN_0040bfc0
 * - This allows for path queueing up to 10 waypoints
 */
void FUN_0040b6e0(void* entity, int target_x, int target_y) {
    (void)entity; (void)target_x; (void)target_y;
    /* TODO: Movement queue implementation */
}

/* Entity query stubs */
void* entity_get_by_id(int id) {
    (void)id;
    return NULL;
}

int entity_get_position(void* entity, int* x, int* y) {
    (void)entity;
    *x = 0;
    *y = 0;
    return 0;
}

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
    (void)model_id; (void)world_x; (void)world_y; (void)extra_data;
    return 0;
}

/*
 * FUN_0040f600 - Reset All Field Entities
 *
 * Binary analysis:
 * - Clears all field entity data
 * - Sets DAT_004e2b14, DAT_00544d70, DAT_004e2b10 to 0 (counters)
 * - Iterates through entity array at DAT_004e2bdc
 * - Clears each entity entry (0x43 dwords = 268 bytes per entry):
 *   - offset -0xBC: state (2 bytes) = 0
 *   - offset 0x00: pointer = 0
 *   - offset -0x90: flags = 0
 *   - offset -0xB8: index = 0
 *   - offset -0xA0: value = -1
 *   - offset -0x84: type (1 byte) = 0
 * - Array ends at 0x544e2c
 * - Sets DAT_004e2b0c to 0
 */
void FUN_0040f600(void) {}

/*
 * FUN_0040f7a0 - Clear Entity References
 *
 * Binary analysis:
 * - Clears entity pointers for entities with state != 0
 * - Iterates through DAT_004e2bdc array
 * - For each entry with state at offset -0xBC != 0:
 *   - Sets entity pointer at offset 0x00 to 0
 *   - Sets index at offset -0xA0 to -1
 * - Used during scene transitions or entity removal
 */
void FUN_0040f7a0(void) {}

/*
 * FUN_0040f7d0 - Spawn Field Entities
 *
 * Binary analysis:
 * - Spawns entities from spawn data array
 * - Iterates through DAT_004e2b30 spawn entries
 * - For each entry with state != 0 and entity pointer == 0:
 *   - Calls FUN_0040b5e0 to create entity
 *   - Sets state to 2 (active)
 *   - Copies name string to entity at offset 0x38
 *   - Copies extra data based on spawn type:
 *     - Type 1 or 8: from offset 0x28 in spawn data
 *     - Type 2: from offset 0x5a (max 28 chars)
 *     - Type 4: from offset 0x5a (max 16 chars)
 *   - Sets entity flags from spawn data offset 0xA4
 * - Spawn entry size: 0x10C bytes (0x43 dwords)
 */
void FUN_0040f7d0(void) {}
