/*
 * Stone Age Client - Character NPC Entry System
 * Reverse engineered from sa_9061.exe (FUN_0040ee70, FUN_0040f310, FUN_0040e830)
 *
 * NPC entry management for map entities
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "character.h"
#include "map.h"
#include "render.h"
#include "pet_protocol.h"
#include "logger.h"

/* NPC entry array - DAT_004e2b20 region - from FUN_0040ee70 */
/* Max NPCs = 0x5dc (1500) from FUN_0040f310 */
#define MAX_NPC_ENTRIES 1500

static NPCEntry s_npc_entries[MAX_NPC_ENTRIES];
static u32 s_npc_count = 0;
static u32 s_npc_last_index = 0;  /* DAT_004e2b0c - for caching */
static u32 s_npc_max_used = 0;    /* DAT_00544d70 */

/*
 * Find free NPC slot - FUN_0040f310 pattern
 * Returns index of available NPC slot, or -1 if full
 */
static int npc_find_free_slot(void) {
    u32 i;
    u32 start_index;

    if (s_npc_count >= MAX_NPC_ENTRIES) {
        return -1;
    }

    start_index = s_npc_last_index;

    /* Search from last used index */
    for (i = start_index; i < s_npc_max_used + 1 && i < MAX_NPC_ENTRIES; i++) {
        if (s_npc_entries[i].type == 0) {
            s_npc_last_index = i;
            if (i >= s_npc_max_used) {
                s_npc_max_used = i + 1;
            }
            return (int)i;
        }
    }

    /* Search from beginning if nothing found */
    if (start_index > 0) {
        for (i = 0; i < start_index; i++) {
            if (s_npc_entries[i].type == 0) {
                s_npc_last_index = i;
                return (int)i;
            }
        }
    }

    s_npc_last_index = s_npc_count;
    s_npc_count++;
    return (int)s_npc_last_index;
}

/*
 * Get NPC direction from type - FUN_0040f3f0 pattern
 */
static u16 npc_get_direction_from_type(u32 npc_type) {
    switch (npc_type) {
        case 1:
            return 8;
        case 6:
        case 0x0D:
        case 0x1B:
            return 17;
        case 0x20:
            return 33;
        default:
            return 1;
    }
}

/*
 * Get NPC sprite base from type - FUN_0040f3c0 pattern
 */
static u16 npc_get_sprite_from_type(u32 npc_type) {
    switch (npc_type) {
        case 1:
            return 0x100;
        case 3:
            return 0x200;
        case 0x20:
            return 0x100;
        default:
            return 0x1000;
    }
}

/*
 * Create NPC entry - FUN_0040ee70 pattern
 * param_1-5: Various NPC parameters
 * param_6: NPC name (max 17 bytes)
 * param_7: NPC description (max 33 bytes)
 * param_8: Extra parameter
 * param_9: Some flags
 * param_10: Flag for bit 0x40
 * param_11: Flag for bit 0x80
 * param_12: NPC type
 */
int npc_create_entry(u32 param1, u32 param2, u32 param3, u32 param4, u32 param5,
                     const char* name, const char* description, u32 param8,
                     u16 param9, int flag_0x40, int flag_0x80, u32 npc_type) {
    int slot;
    NPCEntry* entry;

    /* Check if NPC already exists */
    slot = npc_find_by_id_internal(param1);
    if (slot >= 0) {
        return 0;  /* Already exists */
    }

    /* Find free slot */
    slot = npc_find_free_slot();
    if (slot < 0) {
        return 0;  /* No free slots */
    }

    entry = &s_npc_entries[slot];

    /* Clear entry */
    memset(entry, 0, sizeof(NPCEntry));

    /* Set type to 2 - from FUN_0040ee70 */
    entry->type = 2;

    /* Set direction from NPC type */
    entry->direction = npc_get_direction_from_type(npc_type);

    /* Set parameters */
    entry->id = param1;
    entry->param1 = param2;
    entry->param2 = param3;
    entry->param3 = param4;
    entry->param4 = param5;
    entry->extra_param = param8;

    /* Set field at offset 0x1c */
    entry->field_0x1c = param9;

    /* Set flags - bits 6 and 7 */
    if (flag_0x40) {
        entry->flags |= 0x40;
    }
    if (flag_0x80) {
        entry->flags |= 0x80;
    }

    /* Copy name (max 0x11 = 17 bytes) */
    if (name && strlen(name) < 0x11) {
        strncpy(entry->name, name, 16);
        entry->name[16] = '\0';
    }

    /* Copy description (max 0x21 = 33 bytes) */
    if (description && strlen(description) < 0x21) {
        strncpy(entry->description, description, 32);
        entry->description[32] = '\0';
    }

    /* Set sprite ID from NPC type */
    entry->sprite_id = npc_get_sprite_from_type(npc_type);

    /* Mark as active */
    entry->state = 1;

    LOG_DEBUG("Created NPC %d: %s at slot %d", entry->id, entry->name, slot);

    return 1;
}

/*
 * Find NPC by ID (internal) - FUN_0040e830 pattern
 * Returns slot index or -1 if not found
 */
int npc_find_by_id_internal(u32 npc_id) {
    u32 i;

    /* Search from last found index */
    for (i = s_npc_last_index; i < s_npc_max_used && i < MAX_NPC_ENTRIES; i++) {
        if (s_npc_entries[i].type != 0 && s_npc_entries[i].id == npc_id) {
            s_npc_last_index = i;
            return (int)i;
        }
    }

    /* Search from beginning */
    for (i = 0; i < s_npc_last_index; i++) {
        if (s_npc_entries[i].type != 0 && s_npc_entries[i].id == npc_id) {
            s_npc_last_index = i;
            return (int)i;
        }
    }

    return -1;
}

/*
 * Get NPC entry by index
 */
NPCEntry* npc_get_entry(u32 index) {
    if (index >= MAX_NPC_ENTRIES) return NULL;
    return &s_npc_entries[index];
}

/*
 * Get NPC sprite ID by ID - FUN_0040f460 pattern
 */
u16 npc_get_sprite_by_id(u32 npc_id) {
    int slot = npc_find_by_id_internal(npc_id);
    if (slot < 0) {
        return 0;
    }
    return s_npc_entries[slot].sprite_id;
}

/*
 * Remove NPC entry
 */
void npc_remove_entry(u32 npc_id) {
    int slot = npc_find_by_id_internal(npc_id);
    if (slot >= 0) {
        memset(&s_npc_entries[slot], 0, sizeof(NPCEntry));
        if ((u32)slot < s_npc_last_index) {
            s_npc_last_index = slot;
        }
    }
}

/*
 * Update NPC position
 */
void npc_set_position(u32 npc_id, u16 x, u16 y) {
    int slot = npc_find_by_id_internal(npc_id);
    if (slot >= 0) {
        s_npc_entries[slot].x = x;
        s_npc_entries[slot].y = y;
    }
}

/*
 * Render all NPC entries
 */
void npc_render_entries(void) {
    u32 i;
    NPCEntry* entry;
    int screen_x, screen_y;

    for (i = 0; i < s_npc_max_used && i < MAX_NPC_ENTRIES; i++) {
        entry = &s_npc_entries[i];

        if (entry->type == 0 || entry->state == 0) {
            continue;
        }

        /* Convert world to screen coordinates */
        map_world_to_screen(entry->x, entry->y, &screen_x, &screen_y);

        /* Render NPC sprite */
        render_sprite(entry->sprite_id, screen_x, screen_y);

        /* Render NPC name */
        if (entry->name[0] != '\0') {
            render_text_centered(screen_x, screen_y - 20, entry->name, COLOR_YELLOW);
        }
    }
}

/*
 * Parse shop handler - FUN_00463380 pattern
 * Parses comma-separated item IDs
 */
void npc_parse_shop_items(const char* item_list) {
    int item_id;
    int index = 1;

    if (!item_list) return;

    /* Parse comma-separated items */
    item_id = pet_parse_field_int(item_list, ',', index);
    while (item_id != -1) {
        LOG_DEBUG("Shop item %d: %d", index, item_id);
        index++;
        item_id = pet_parse_field_int(item_list, ',', index);
    }
}
