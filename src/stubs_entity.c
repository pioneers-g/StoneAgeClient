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
 * FUN_004010a0 - Entity Allocation
 *
 * Binary analysis:
 * - Allocates a 500-byte entity structure
 * - Maintains sorted linked list at DAT_04630fa0
 * - Returns pointer to allocated entity
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
 * FUN_0040b6e0 - Entity Movement
 *
 * Binary analysis:
 * - Handles entity movement along path queue
 * - Updates position and direction
 */
void FUN_0040b6e0(void* entity) {
    (void)entity;
    /* TODO: Movement implementation */
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
