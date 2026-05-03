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
