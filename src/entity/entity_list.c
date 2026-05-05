/*
 * Stone Age Client - Entity Linked List Manager
 * Replaces hardcoded address-based entity list (DAT_004d7e3c/DAT_004d7e38)
 *
 * Functions:
 * - FUN_004010a0: Allocate entity, insert into sorted list
 * - FUN_00401170: Update loop (call callbacks, remove deleted)
 * - FUN_004011d0: Free entity and extra buffer
 * - FUN_004011f0: Mark all entities for deletion
 * - FUN_004012d0: Full entity cleanup
 *
 * Entity layout (original x86 offsets in comments):
 *   prev, next: linked list pointers
 *   callback:   render callback function pointer
 *   extra:      extra buffer pointer
 *   priority:   sort key (1 byte)
 *   state:      entity state (init -2)
 *   delete_flag: 0=active, nonzero=delete
 *   render_order: sort order (init -1)
 *   Remaining bytes: entity-specific data (position, sprite, etc.)
 */

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "types.h"

/* Total entity allocation size */
#define ENTITY_ALLOC_SIZE 512

/* Linked list node header - matches original x86 layout conceptually */
typedef struct EntityNode {
    struct EntityNode* prev;       /* offset 0x00 */
    struct EntityNode* next;       /* offset 0x04 (x86) / 0x08 (x64) */
    void (*callback)(void*);       /* render callback */
    void* extra;                   /* extra buffer */
    u8 priority;                   /* sort key */
    u8 pad1[3];
    int state;                     /* entity state, init -2 */
    int delete_flag;               /* 0=active, nonzero=delete */
    u8 data[ENTITY_ALLOC_SIZE - sizeof(struct EntityNode*) * 4
                              - sizeof(void (*)(void*))
                              - sizeof(void*)
                              - 4 - 4 - 4];
    int render_order;              /* init -1 */
} EntityNode;

/* Sentinel list head and tail */
static EntityNode s_entity_head;
static EntityNode s_entity_tail;
static int s_entity_list_init = 0;
static int s_entity_count = 0;

/* External allocation */
extern void* FUN_00491f70(int alignment, int size);
extern void FUN_00491fed(void* ptr);

/* Forward declaration */
void FUN_004011d0(intptr_t entity_ptr);

static void entity_list_init(void) {
    if (s_entity_list_init) return;
    memset(&s_entity_head, 0, sizeof(EntityNode));
    memset(&s_entity_tail, 0, sizeof(EntityNode));
    s_entity_head.next = &s_entity_tail;
    s_entity_tail.prev = &s_entity_head;
    s_entity_list_init = 1;
    s_entity_count = 0;
}

static void entity_list_insert(EntityNode* node) {
    EntityNode* cur;

    entity_list_init();

    cur = s_entity_head.next;
    while (cur != &s_entity_tail) {
        if (cur->priority > node->priority) break;
        cur = cur->next;
    }

    node->prev = cur->prev;
    node->next = cur;
    cur->prev->next = node;
    cur->prev = node;
    s_entity_count++;
}

static void entity_list_unlink(EntityNode* node) {
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
    node->prev = NULL;
    node->next = NULL;
}

/*
 * FUN_004010a0 - Entity Allocation and List Insertion
 */
void* FUN_004010a0(int param_1, int param_2) {
    EntityNode* node;
    void* extra;

    entity_list_init();

    node = (EntityNode*)FUN_00491f70(1, sizeof(EntityNode));
    if (!node) {
        MessageBoxA(NULL, "Failed to allocate entity", "GetAction Error", MB_OK);
        return NULL;
    }
    memset(node, 0, sizeof(EntityNode));

    if (param_2 != 0) {
        extra = FUN_00491f70(1, param_2);
        node->extra = extra;
        if (!extra) {
            FUN_00491fed(node);
            MessageBoxA(NULL, "Failed to allocate extra buffer", "GetYobi Error", MB_OK);
            return NULL;
        }
    }

    node->callback = NULL;
    node->priority = (u8)param_1;
    node->render_order = -1;
    node->state = -2;
    node->delete_flag = 0;

    entity_list_insert(node);
    return node;
}

/*
 * FUN_00401170 - Field Entity Update Loop
 */
void FUN_00401170(void) {
    EntityNode* cur;
    EntityNode* next;

    entity_list_init();
    cur = s_entity_head.next;

    while (cur != &s_entity_tail) {
        if (cur->delete_flag == 0) {
            if (cur->callback) {
                cur->callback(cur);
            }
            cur = cur->next;
        } else {
            next = cur->next;
            entity_list_unlink(cur);
            FUN_004011d0((intptr_t)cur);
            cur = next;
        }
    }
}

/*
 * FUN_004011d0 - Entity Free
 */
void FUN_004011d0(intptr_t entity_ptr) {
    EntityNode* node = (EntityNode*)entity_ptr;
    if (!node) return;

    if (node->extra) {
        FUN_00491fed(node->extra);
    }
    FUN_00491fed(node);

    if (s_entity_count > 0) s_entity_count--;
}

/*
 * FUN_004011f0 - Mark All Entities for Deletion
 */
void FUN_004011f0(void) {
    EntityNode* cur;
    entity_list_init();
    cur = s_entity_head.next;
    while (cur != &s_entity_tail) {
        cur->delete_flag = 1;
        cur = cur->next;
    }
}

/*
 * FUN_004012d0 - Full Entity Cleanup
 */
void FUN_004012d0(void) {
    FUN_004011f0();
    FUN_00401170();
    s_entity_list_init = 0;
}

int entity_list_count(void) { return s_entity_count; }

int entity_list_empty(void) {
    entity_list_init();
    return s_entity_head.next == &s_entity_tail;
}

void* entity_list_get_head(void) { entity_list_init(); return &s_entity_head; }
void* entity_list_get_tail(void) { entity_list_init(); return &s_entity_tail; }
