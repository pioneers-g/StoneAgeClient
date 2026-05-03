/*
 * Stone Age Client - Entity Stub Functions
 * Split from stubs.c to reduce file size
 */

#include <windows.h>
#include <stdlib.h>
#include <math.h>
#include "types.h"

/* External globals */
extern u32 s_entity_count;
extern void* s_battle_units;
extern u32 s_battle_unit_count;

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
void FUN_0040f180(int entity_id, int type, int x, int y, int extra, int param_6, const char* name) {
    /* TODO: Full implementation */
    (void)entity_id;
    (void)type;
    (void)x;
    (void)y;
    (void)extra;
    (void)param_6;
    (void)name;
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

/*
 * FUN_00476150 - Get Random Value with Range Check
 *
 * Binary analysis:
 * - Gets a random value via FUN_0046b8a0
 * - Returns 0 if value is -1 or 0xff
 * - Otherwise returns the random value
 * - Preserves DAT_0461c6c0 across call
 */
int FUN_00476150(void) {
    return 0;
}

/*
 * FUN_00476180 - Spawn Effect Entity
 *
 * Binary analysis:
 * - Spawns visual effect entity (light, glow, etc.)
 * - param_1: effect sprite ID
 * - Sprite ID ranges:
 *   - With DAT_0054c83c: 0x18d8e-0x18da5
 *   - Without: 0x18dc5-0x18ddc
 * - Only spawns if DAT_0461c2c0 == 0
 * - Creates entity at (320, 240) with size 0x140x0xf0
 * - Sets animation state 3, priority 2
 */
void FUN_00476180(int sprite_id) {
    (void)sprite_id;
}

/*
 * FUN_00476200 - Initialize Character Grid Entities
 *
 * Binary analysis:
 * - Creates character entities for the field display
 * - Clears DAT_0461c764 (21 entries) and DAT_04ebe428 (20 entries)
 * - Creates main player entity at DAT_0461c674
 * - Creates camera/view entity at DAT_0461c67c
 * - Creates 20 character slot entities in DAT_04ebe428
 * - Each entity: 0x25c bytes, priority 0x14, sprite 0x186c8/0x1879a
 * - Grid layout: 5 rows x 4 columns starting at (453, 432)
 * - Initializes animation via FUN_00477240
 * - Sets position arrays at DAT_0461c6c4/c6c8/c220/c224
 */
int FUN_00476200(void) {
    return 0;
}

/*
 * FUN_00477240 - Character Animation Update
 *
 * Binary analysis:
 * - Main animation state machine for character entities
 * - param_1: entity pointer
 * - param_2: force frame (0 = use default)
 * - param_3: immediate return flag (1 = return immediately)
 * - Checks if animation state changed via offsets 0xa0-0xaa
 * - Handles sprite ID ranges:
 *   - < 100: Clear animation
 *   - < 100000: Standard sprite lookup
 *   - 0x206a0+: Invalid
 * - Uses animation tables at DAT_038484a8/84ac
 * - Updates frame counter at offset 0xae
 * - Plays sound effects via FUN_00488190 for special sprites
 * - Returns 1 if animation complete, 0 otherwise
 */
int FUN_00477240(void* entity, int force_frame, int immediate) {
    (void)entity; (void)force_frame; (void)immediate;
    return 0;
}

/*
 * FUN_004770c0 - Character Animation State Machine
 *
 * Binary analysis:
 * - Handles animation state transitions for characters
 * - param_1: entity pointer
 * - Uses state at offset 0x1ec (packed: type in high byte, index in bits 16-23)
 * - State types (high byte):
 *   - 1: Forward animation cycle
 *   - 2: Backward animation cycle
 *   - 3: Special effect (toggle between 0x18de2/0x18dad)
 * - Animation frames stored in local_24 array:
 *   - 0x18824, 0x189e9, 0x18b3a, 0x189f6, 0x18cc2
 *   - 0x18801, 0x18d7f, 0x18c30, 0x18c71
 * - Increments frame index at offset 0x1f0 (wraps at 9)
 * - Special sprite 0x18de7: transition frame
 * - Special sprite 0x18e30: alternate transition
 */
void FUN_004770c0(void* entity) {
    (void)entity;
}

/*
 * FUN_0046b800 - Character Bobbing Animation
 *
 * Binary analysis:
 * - Applies vertical bobbing effect to character
 * - param_1: entity pointer
 * - Only active when DAT_0461c7d4 > 1 or DAT_004d7f78 in range 0x94-0x96
 * - Uses sine table at DAT_0049ea94 (64 entries)
 * - Calculates bob offset: (Y >> 3 + DAT_004d7f7c + DAT_0461c7d8) & 0x3f
 * - Adjusts Y coordinate and sprite offset
 * - Stores offset at entity->companion->0x130
 */
void FUN_0046b800(void* entity) {
    (void)entity;
}

/*
 * FUN_0046b8a0 - Parse Hex Number from Buffer
 *
 * Binary analysis:
 * - Parses hexadecimal number from DAT_004d803c at position DAT_0461c6c0
 * - Same logic as FUN_00476860 but different buffer
 * - Skips non-hex characters at start
 * - Accepts: '0'-'9' (0x30-0x39) and 'A'-'F' (0x41-0x46)
 * - Returns -1 if no hex digit found
 * - Updates DAT_0461c6c0 to position after last hex digit
 */
int FUN_0046b8a0(void) {
    return -1;
}

/*
 * FUN_0046b930 - Extract String from Buffer
 *
 * Binary analysis:
 * - Extracts string from DAT_004d803c at position DAT_0461c6c0
 * - Same logic as FUN_004768e0 but different buffer
 * - param_1: destination structure (stores at offset 0x38)
 * - Reads until null or '|' delimiter
 * - Handles DBCS characters via IsDBCSLeadByte()
 * - Calls FUN_0048a170 to process extracted string
 */
void FUN_0046b930(void* dest_struct) {
    (void)dest_struct;
}

/*
 * FUN_0046b9a0 - Check All Characters Ready
 *
 * Binary analysis:
 * - Checks if all character entities are ready for action
 * - Returns true if all characters in party are ready
 * - Checks entities in DAT_04ebe428 (first 10) and DAT_04ebe450 (next 5)
 * - For each entity: checks offset 8 (active) and offset 0x78 (HP)
 * - Special sprites 0x18db5/0x18db6 always considered ready
 * - Also checks companion flag at offset 0xc+0x120
 * - Returns false if any entity is not ready
 */
int FUN_0046b9a0(void) {
    return 1;
}

/*
 * FUN_0046bac0 - Set Character Emote Sprite
 *
 * Binary analysis:
 * - Sets emote sprite for character entity at DAT_0461c67c
 * - param_1: emote type (0-4)
 * - Emote sprites:
 *   - 0: 0x622c (normal)
 *   - 1: 0x622d (happy)
 *   - 2: 0x622e (sad)
 *   - 3: 0x622f (angry)
 *   - 4: 0x622f (angry)
 * - Clears bit 1 from flags at offset 0xa0
 * - Calls FUN_00477240 to update animation
 */
void FUN_0046bac0(int emote_type) {
    (void)emote_type;
}

/*
 * FUN_00468150 - Create Status Effect Entity
 *
 * Binary analysis:
 * - Creates visual status effect above character
 * - param_1: effect type (1-34)
 * - param_2: parent entity pointer
 * - Only creates if entity HP (offset 0x78) > 0
 * - Creates child entity with priority 0x47, size 0x25c bytes
 * - Sets render callback to FUN_00467eb0
 * - Links child to parent via companion->0x110
 * - Effect sprites by type:
 *   - 1: 0x188cb (poison)
 *   - 2, 23, 33: 0x188c7 (paralysis)
 *   - 3: 0x188c9 (sleep)
 *   - 4: 0x188c6 (stone)
 *   - 5: 0x188c8 (drunk)
 *   - 6, 20: 0x188ca (confusion)
 *   - 7: 0x18c2c (attack up)
 *   - 8: 0x18c29 (defense up)
 *   - 9, 32: 0x18c2d (speed up)
 *   - 10: 0x18c2b (accuracy up)
 *   - 11: 0x18d46 (critical up)
 *   - 12: 0x639c (buff indicator)
 *   - 13: 0x8930 (special effect)
 *   - 14: 0x8926 (special effect 2)
 *   - 15, 17: 0x6c2c (shield)
 *   - 16: 0x6795 (barrier)
 *   - 18, 19: 0x6984 (protection)
 *   - 21-22: 33: Special handling for 0x18db6/0x18db7
 *   - 34: Resurrection effect
 */
void FUN_00468150(int effect_type, void* parent_entity) {
    (void)effect_type; (void)parent_entity;
}

/*
 * FUN_00468430 - Create Emote Effect Entity
 *
 * Binary analysis:
 * - Creates emote bubble above character
 * - param_1: parent entity pointer
 * - Creates child entity with priority 0x47
 * - Sets render callback to LAB_004683c0
 * - Links via companion->0x118
 * - Sprite: 0x622c (emote bubble)
 * - Position: same X as parent, Y offset by -0x40
 */
void FUN_00468430(void* parent_entity) {
    (void)parent_entity;
}

/*
 * FUN_00477c70 - Set Entity Model/Sprite
 *
 * Binary analysis:
 * - Updates entity model ID and extra data
 * - param_1: model/sprite ID
 * - param_2: extra data
 * - Sets both global (DAT_0462be88/94) and entity-specific (offset 0x140/150)
 */
void FUN_00477c70(int model_id, int extra_data) {
    (void)model_id; (void)extra_data;
}

/*
 * FUN_00477cd0 - Update Entity Position from Globals
 *
 * Binary analysis:
 * - Copies position data from global variables to entity
 * - Target: entity at DAT_0462e3ac
 * - Copies:
 *   - offset 0x114/118: from DAT_0456a644/48
 *   - offset 0xb0/b4: from DAT_04581d3c/40 (world coords)
 *   - offset 0x11c/120: from DAT_045827fc/800
 *   - offset 0xb8/bc: from DAT_0458118c/84
 */
void FUN_00477cd0(void) {}

/*
 * FUN_00478a30 - Parse Character Data String
 *
 * Binary analysis:
 * - Parses character data from pipe-delimited string
 * - param_1: character name string
 * - param_2: data string (14+ fields separated by '|')
 * - Returns 0 on success, -1 on failure
 * - Extracts fields using FUN_0048a050:
 *   - Field 1: slot index (0 or 1)
 *   - Field 2: level -> DAT_04630a18
 *   - Field 3: HP -> DAT_04630a12
 *   - Field 4-8: stats -> various offsets
 *   - Field 9: status -> DAT_04630a40
 *   - Fields 10-13: values divided by 10
 *   - Field 14: extra data
 * - Character name stored at DAT_04630a00 + slot*0x44
 */
int FUN_00478a30(char* name, char* data) {
    (void)name; (void)data;
    return 0;
}
