/*
 * Stone Age Client - Entity Slot Management Functions
 * Entity slot array allocation, lookup, and management
 */

#include <windows.h>
#include <stdlib.h>
#include "types.h"

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
    /* TODO: Full implementation with entity array */
    (void)entity_id;
    return -1;
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
    /* TODO: Full implementation with entity array */
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
    /* TODO: Return actual pointer from entity array */
    (void)slot;
    return NULL;
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
    /* TODO: Full implementation with entity array */
    (void)x;
    (void)y;
    (void)flags;
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
    /* TODO: Full implementation with entity array */
    (void)x;
    (void)y;
    (void)flags;
    (void)type_mask;
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
    /* TODO: Full implementation with entity arrays */
    (void)entity_id;
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
    /* TODO: Full implementation with entity arrays */
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
    /* TODO: Full implementation with entity arrays */
}
