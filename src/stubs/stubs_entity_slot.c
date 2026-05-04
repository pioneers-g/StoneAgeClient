/*
 * Stone Age Client - Entity Slot Management Functions
 * Entity slot array allocation, lookup, and management
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

/* Entity slot array constants - from binary analysis */
#define ENTITY_SLOT_STRIDE   0x43    /* 67 dwords per slot */
#define ENTITY_SLOT_MAX      1500    /* Maximum entities (0x5dc) */

/* Entity slot structure - offsets from DAT_004e2b24 base
 * Each slot is 0x43 * 4 = 268 bytes (0x10c)
 */
typedef struct {
    s16 active;          /* offset -0xbc from base (-1 slot) = -2 bytes from id */
    u16 padding;
    s32 entity_id;       /* offset 0x00: entity ID */
    void* entity_ptr;    /* offset 0x04: entity pointer */
    void* extra_data1;   /* offset 0x08: extra data 1 */
    void* extra_data2;   /* offset 0x0c: extra data 2 */
    s32 field_0x10;
    s32 field_0x14;
    s32 world_x;         /* offset 0x18: world X */
    s32 world_y;         /* offset 0x1c: world Y */
    s32 field_0x20;
    s32 field_0x24;
    s32 field_0x28;
    s32 field_0x2c;
    s32 field_0x30;
    s32 field_0x34;
    s32 field_0x38;
    s32 field_0x3c;
    s32 field_0x40;
    u16 flags;           /* offset 0x44: entity flags */
    u16 type;            /* offset 0x46: entity type */
} EntitySlot;

/* Global entity slot array - DAT_004e2b24 */
static EntitySlot s_entity_slots[ENTITY_SLOT_MAX];

/* Entity slot globals - from binary */
static s32 s_entity_count = 0;        /* DAT_004e2b14 */
static s32 s_next_free_slot = 0;      /* DAT_004e2b10 */
static s32 s_high_water_slot = 0;     /* DAT_00544d70 */
static s32 s_cached_slot = 0;         /* DAT_004e2b0c - cache for lookups */
static s32 s_player_entity_id = 0;    /* DAT_0462be90 */

/* External functions */
extern void FUN_00491fed(void* ptr);
extern void FUN_004011c0(void* ptr);

/*
 * FUN_0040e830 - Find Entity Slot by ID
 *
 * Binary analysis:
 * - Searches for entity with matching ID in entity array
 * - param_1: entity ID to find
 * - Returns: slot index (0-1500) or -1 if not found
 * - Uses cache at DAT_004e2b0c for optimization
 * - Entity array at DAT_004e2b24 (stride 0x43 = 67 dwords)
 */
int FUN_0040e830(int entity_id) {
    int slot;
    int found_slot;
    EntitySlot* entry;

    found_slot = -1;

    /* First search from cached slot to high water mark */
    if (s_cached_slot < s_high_water_slot) {
        entry = &s_entity_slots[s_cached_slot];
        for (slot = s_cached_slot; slot < s_high_water_slot; slot++) {
            /* Check if slot is active and ID matches */
            if (entry->active != 0 && entry->entity_id == entity_id) {
                found_slot = slot;
                s_cached_slot = slot;
                return slot;
            }
            entry++;
        }
    }

    /* Then search from 0 to cached slot */
    if (s_cached_slot > 0) {
        entry = &s_entity_slots[0];
        for (slot = 0; slot < s_cached_slot; slot++) {
            if (entry->active != 0 && entry->entity_id == entity_id) {
                found_slot = slot;
                break;
            }
            entry++;
        }
    }

    if (found_slot >= 0) {
        s_cached_slot = found_slot;
    }

    return found_slot;
}

/*
 * FUN_0040f310 - Allocate Entity Slot
 *
 * Binary analysis:
 * - Finds next available entity slot
 * - Returns: slot index (0-1500) or -1 if full
 * - Max 1500 entities (0x5dc)
 * - Updates DAT_004e2b14 (count) and DAT_004e2b10 (next free)
 * - Entity array at DAT_004e2b20 (stride 0x86 = 134 shorts)
 */
int FUN_0040f310(void) {
    int slot;
    EntitySlot* entry;

    /* Search from next_free_slot to max */
    for (slot = s_next_free_slot; slot < ENTITY_SLOT_MAX; slot++) {
        entry = &s_entity_slots[slot];
        if (entry->active == 0) {
            /* Found free slot */
            s_next_free_slot = slot + 1;
            s_entity_count++;

            /* Update high water mark */
            if (slot >= s_high_water_slot) {
                s_high_water_slot = slot + 1;
            }

            /* Initialize slot */
            memset(entry, 0, sizeof(EntitySlot));
            entry->active = 1;

            return slot;
        }
    }

    /* Search from 0 to next_free_slot */
    for (slot = 0; slot < s_next_free_slot; slot++) {
        entry = &s_entity_slots[slot];
        if (entry->active == 0) {
            s_next_free_slot = slot + 1;
            s_entity_count++;

            if (slot >= s_high_water_slot) {
                s_high_water_slot = slot + 1;
            }

            memset(entry, 0, sizeof(EntitySlot));
            entry->active = 1;

            return slot;
        }
    }

    /* No free slots */
    return -1;
}

/*
 * FUN_0040f460 - Get Entity Pointer by ID
 *
 * Binary analysis:
 * - Returns entity pointer for given ID
 * - param_1: entity ID
 * - Uses FUN_0040e830 to find slot
 * - Returns pointer from DAT_004e2bdc[slot * 0x43] or 0 if not found
 */
void* FUN_0040f460(int entity_id) {
    int slot = FUN_0040e830(entity_id);
    if (slot < 0) {
        return NULL;
    }
    return s_entity_slots[slot].entity_ptr;
}

/*
 * FUN_0040f3c0 - Get Entity Type Flags
 *
 * Binary analysis:
 * - Returns flags based on entity type
 * - param_1: entity type
 * - Returns:
 *   - type 1: 0x100
 *   - type 3: 0x200
 *   - type 0x20: 0x100
 *   - default: 0x1000
 */
int FUN_0040f3c0(int entity_type) {
    if (entity_type == 1) {
        return 0x100;
    } else if (entity_type == 3) {
        return 0x200;
    } else if (entity_type == 0x20) {
        return 0x100;
    }
    return 0x1000;
}

/*
 * FUN_0040e8c0 - Check Entity at Position with Flags
 *
 * Binary analysis:
 * - Checks if an entity exists at the given position with matching flags
 * - param_1: X coordinate
 * - param_2: Y coordinate
 * - param_3: entity flags mask (checked at offset 0x46 from slot)
 * - Returns: 1 if found, 0 if not found
 * - Entity array at DAT_004e2bdc (stride 0x43 = 67 dwords)
 * - Checks: active flag (offset -0x5e), pointer valid, position match, flag match
 */
int FUN_0040e8c0(int x, int y, u16 flags) {
    int slot;
    EntitySlot* entry;

    for (slot = 0; slot < s_high_water_slot; slot++) {
        entry = &s_entity_slots[slot];

        /* Check if slot is active */
        if (entry->active == 0) continue;

        /* Check if entity pointer is valid */
        if (entry->entity_ptr == NULL) continue;

        /* Check position match */
        if (entry->world_x != x || entry->world_y != y) continue;

        /* Check flags match */
        if ((entry->flags & flags) != 0) {
            return 1;
        }
    }

    return 0;
}

/*
 * FUN_0040e930 - Check Entity at Position with Flags and Type
 *
 * Binary analysis:
 * - Checks if an entity exists at position with flags AND type match
 * - param_1: X coordinate
 * - param_2: Y coordinate
 * - param_3: entity flags mask (offset 0x46)
 * - param_4: entity type mask (offset 0x28 from slot)
 * - Returns: 1 if found, 0 if not found
 * - Same as FUN_0040e8c0 but with additional type check
 */
int FUN_0040e930(int x, int y, u16 flags, u32 type_mask) {
    int slot;
    EntitySlot* entry;

    for (slot = 0; slot < s_high_water_slot; slot++) {
        entry = &s_entity_slots[slot];

        if (entry->active == 0) continue;
        if (entry->entity_ptr == NULL) continue;
        if (entry->world_x != x || entry->world_y != y) continue;

        /* Check both flags and type */
        if ((entry->flags & flags) != 0 && (entry->type & type_mask) != 0) {
            return 1;
        }
    }

    return 0;
}

/*
 * FUN_0040f490 - Remove Entity by ID
 *
 * Binary analysis:
 * - Removes an entity from the slot array and frees its resources
 * - param_1: entity ID to remove
 * - Checks if ID matches DAT_0462be90 (special player entity)
 * - Uses FUN_0040e830 to find slot
 * - Frees entity extra data at offsets 4, 8, 0x14, 0x10, 0x1c, 0x20, 0x24-0x30
 * - Calls FUN_004011c0 for each extra data pointer
 * - Clears slot in DAT_004e2bdc array
 * - Decrements DAT_004e2b14 (entity count)
 * - Updates DAT_004e2b10 (next free slot)
 * - Updates DAT_00544d70 (highest used slot)
 */
void FUN_0040f490(int entity_id) {
    int slot;
    EntitySlot* entry;

    /* Check if this is the player entity */
    if (entity_id == s_player_entity_id) {
        s_player_entity_id = 0;
    }

    slot = FUN_0040e830(entity_id);
    if (slot < 0) return;

    entry = &s_entity_slots[slot];

    /* Free extra data pointers */
    if (entry->extra_data1) {
        FUN_004011c0(entry->extra_data1);
        entry->extra_data1 = NULL;
    }
    if (entry->extra_data2) {
        FUN_004011c0(entry->extra_data2);
        entry->extra_data2 = NULL;
    }
    if (entry->field_0x10) {
        FUN_004011c0((void*)entry->field_0x10);
        entry->field_0x10 = 0;
    }
    if (entry->field_0x14) {
        FUN_004011c0((void*)entry->field_0x14);
        entry->field_0x14 = 0;
    }

    /* Clear the slot */
    memset(entry, 0, sizeof(EntitySlot));
    entry->active = 0;

    /* Update counters */
    if (s_entity_count > 0) {
        s_entity_count--;
    }

    /* Update next free slot if this is earlier */
    if (slot < s_next_free_slot) {
        s_next_free_slot = slot;
    }
}

/*
 * FUN_0040f600 - Clear All Entity Slots
 *
 * Binary analysis:
 * - Initializes/resets all entity slot arrays to empty state
 * - Sets DAT_004e2b14 (count), DAT_00544d70 (high water), DAT_004e2b10 (next free) to 0
 * - Iterates through all slots (up to 0x544e2c)
 * - Clears each slot's:
 *   - offset -0xbc: active flag (short)
 *   - offset 0x00: entity pointer
 *   - offset -0x90: unknown field
 *   - offset -0xb8: unknown field
 *   - offset -0xa0: -1 (field ID)
 *   - offset -0x84: 0 (byte)
 * - Sets DAT_004e2b0c (cache) to 0
 */
void FUN_0040f600(void) {
    int slot;
    EntitySlot* entry;

    /* Clear all slots */
    for (slot = 0; slot < ENTITY_SLOT_MAX; slot++) {
        entry = &s_entity_slots[slot];
        memset(entry, 0, sizeof(EntitySlot));
        entry->active = 0;
    }

    /* Reset counters */
    s_entity_count = 0;
    s_next_free_slot = 0;
    s_high_water_slot = 0;
    s_cached_slot = 0;
    s_player_entity_id = 0;
}

/*
 * FUN_0040f650 - Clear All Entities with Cleanup
 *
 * Binary analysis:
 * - Clears all entities and frees their resources
 * - Same as FUN_0040f600 but also frees entity data
 * - Iterates through all slots
 * - For each active slot, frees extra data at multiple offsets
 * - Calls FUN_004011c0 for each extra data pointer
 * - Clears slot after freeing
 */
void FUN_0040f650(void) {
    int slot;
    EntitySlot* entry;

    /* Free all entity data */
    for (slot = 0; slot < ENTITY_SLOT_MAX; slot++) {
        entry = &s_entity_slots[slot];

        if (entry->active == 0) continue;

        /* Free extra data pointers */
        if (entry->extra_data1) {
            FUN_004011c0(entry->extra_data1);
        }
        if (entry->extra_data2) {
            FUN_004011c0(entry->extra_data2);
        }
        if (entry->field_0x10) {
            FUN_004011c0((void*)entry->field_0x10);
        }
        if (entry->field_0x14) {
            FUN_004011c0((void*)entry->field_0x14);
        }
        if (entry->entity_ptr) {
            FUN_004011c0(entry->entity_ptr);
        }

        /* Clear slot */
        memset(entry, 0, sizeof(EntitySlot));
    }

    /* Reset counters */
    s_entity_count = 0;
    s_next_free_slot = 0;
    s_high_water_slot = 0;
    s_cached_slot = 0;
    s_player_entity_id = 0;
}
