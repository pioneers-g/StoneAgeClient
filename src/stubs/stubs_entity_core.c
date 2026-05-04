/*
 * Stone Age Client - Entity Core Functions
 * Core entity allocation, deallocation, and update loop
 */

#include <windows.h>
#include <stdlib.h>
#include "types.h"

/* External globals */
extern u32 s_entity_count;

/* Entity structure size from binary */
#define ENTITY_STRUCT_SIZE 500

/* External allocation functions - implemented in memory module */
extern void* FUN_00491f70(int alignment, int size);  /* 16-byte aligned allocation */
extern void FUN_00491fed(void* ptr);  /* Aligned free */

/*
 * FUN_004010a0 - Entity Allocation and List Insertion
 *
 * Binary analysis:
 * - Allocates a 500-byte entity structure via FUN_00491f70
 * - param_1: priority/sorting value (stored at offset 5)
 * - param_2: optional extra buffer size (stored at offset 0xc)
 * - Initializes:
 *   - offset 0x08: render callback pointer (initially 0)
 *   - offset 0x15: priority byte
 *   - offset 0x9c: render order (-1)
 *   - offset 0x20: state (-2)
 * - Inserts into sorted linked list at DAT_04630fa0
 * - List sorted by priority (lower value = later in list)
 * - Returns pointer to allocated entity or NULL on failure
 * - Shows error message on allocation failure
 */
void* FUN_004010a0(int param_1, int param_2) {
    int* entity;
    int* extra_buffer;

    /* Allocate 500-byte entity structure (16-byte aligned) */
    entity = (int*)FUN_00491f70(1, ENTITY_STRUCT_SIZE);
    if (entity == NULL) {
        MessageBoxA(NULL, "Failed to allocate entity", "GetAction Error", MB_OK);
        return NULL;
    }

    /* Allocate optional extra buffer */
    if (param_2 != 0) {
        extra_buffer = (int*)FUN_00491f70(1, param_2);
        entity[3] = (int)extra_buffer;  /* offset 0x0c */
        if (extra_buffer == NULL) {
            FUN_00491fed(entity);
            MessageBoxA(NULL, "Failed to allocate extra buffer", "GetYobi Error", MB_OK);
            return NULL;
        }
    }

    /* Initialize fields */
    entity[2] = 0;                    /* offset 0x08: render callback */
    *((unsigned char*)entity + 0x15) = (unsigned char)param_1;  /* priority */
    entity[0x27] = -1;                /* offset 0x9c: render order */
    entity[8] = -2;                   /* offset 0x20: state */

    /* TODO: Insert into sorted linked list at DAT_04630fa0 */

    s_entity_count++;
    return entity;
}

/*
 * FUN_004011d0 - Entity Free
 *
 * Binary analysis:
 * - Frees entity and its extra buffer
 * - param_1: entity pointer
 * - Does not decrement global count (done in original binary)
 */
void FUN_004011d0(intptr_t entity_ptr) {
    int* entity = (int*)entity_ptr;

    if (entity == 0) {
        return;
    }

    /* Free extra buffer at offset 0x0c if present */
    if (entity[3] != 0) {
        FUN_00491fed((void*)entity[3]);
    }

    /* Free entity itself */
    FUN_00491fed(entity);

    if (s_entity_count > 0) {
        s_entity_count--;
    }
}

/*
 * FUN_00401170 - Field Entity Update Loop
 *
 * Binary analysis:
 * - Iterates through entity linked list
 * - Calls render callback for each active entity
 * - Removes entities marked for deletion (offset 0x24 != 0)
 * - Entity list head at DAT_004d7e3c
 * - Entity list tail at DAT_004d7e38
 */
void FUN_00401170(void) {
    int **entity_list_head = (int**)0x004d7e3c;
    int **entity_list_tail = (int**)0x004d7e38;
    int *current;
    int *next;

    /* Start from head->next (first actual entity) */
    current = (int*)*entity_list_head;
    if (current == NULL) return;

    current = (int*)current[1];  /* Skip list head node */

    while (current != (int*)*entity_list_tail) {
        if (current == NULL) break;

        /* Check deletion flag at offset 0x24 */
        if (current[9] == 0) {
            /* Entity is active - call render callback at offset 0x08 */
            if (current[2] != 0) {
                typedef void (*render_callback_t)(int*);
                render_callback_t callback = (render_callback_t)current[2];
                callback(current);
            }
            /* Move to next entity */
            current = (int*)current[1];
        } else {
            /* Entity marked for deletion - remove from list */
            int *prev = (int*)current[0];
            next = (int*)current[1];

            /* Unlink from doubly-linked list */
            prev[1] = (int)next;
            next[0] = (int)prev;

            /* Free entity */
            FUN_004011d0((intptr_t)current);

            /* Continue with next entity */
            current = next;
        }
    }
}

/*
 * FUN_004011f0 - Mark All Entities for Deletion
 *
 * Binary analysis:
 * - Terminates explorer process if DAT_0455f8f8 is set
 * - Uses Process32First/Process32Next to find process
 * - Checks if module name contains "explorer.exe"
 * - Terminates process with TerminateProcess if found
 * - Iterates through entity list at DAT_004d7e3c
 * - Sets deletion flag (offset 0x24) to 1 for each entity
 * - Clears DAT_0455f8f8 after cleanup
 */
void FUN_004011f0(void) {
    /* TODO: Full implementation with process handling and entity list */
}

/*
 * FUN_004012d0 - Full Entity Cleanup
 *
 * Binary analysis:
 * - Complete cleanup of entity system
 * - Calls FUN_004011f0 to mark all entities for deletion
 * - Calls FUN_00401170 to update/remove entities
 * - Frees entity list globals DAT_004d7e3c and DAT_004d7e38
 */
void FUN_004012d0(void) {
    /* TODO: Full implementation with global cleanup */
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
