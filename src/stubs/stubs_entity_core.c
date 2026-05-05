/*
 * Stone Age Client - Entity Core Functions
 * Core entity allocation, deallocation, and update loop
 *
 * FUN_004010a0, FUN_00401170, FUN_004011d0, FUN_004011f0, FUN_004012d0
 * are now implemented in entity/entity_list.c
 */

#include <windows.h>
#include <stdlib.h>
#include "types.h"

/* External globals */
extern u32 s_entity_count;

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

/* Entity query stubs - movement functions moved to stubs_entity_movement.c */
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
