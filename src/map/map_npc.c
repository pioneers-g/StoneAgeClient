/*
 * Stone Age Client - Map NPC Management Implementation
 * Reverse engineered from sa_9061.exe
 *
 * FUN_0040ee70 - NPC creation
 * FUN_0040f310 - Find free NPC slot
 * FUN_0040f3f0 - Get NPC type value
 * FUN_0040f3c0 - Get render flags
 * FUN_0040e830 - Find NPC by ID
 * FUN_0040f600 - Clear all NPCs
 *
 * Note: This module handles NPCs on the game map (field entities).
 * The npc.c module handles NPC dialog interactions.
 */

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "map_npc.h"
#include "map.h"
#include "animation.h"
#include "render.h"
#include "logger.h"

/* Global map NPC context - matches DAT_004e2bxx region */
MapNPCContext g_map_npcs = {0};

/*
 * Initialize map NPC system
 */
int map_npc_init(void) {
    memset(&g_map_npcs, 0, sizeof(MapNPCContext));
    LOG_INFO("Map NPC system initialized");
    return 1;
}

/*
 * Shutdown map NPC system
 */
void map_npc_shutdown(void) {
    memset(&g_map_npcs, 0, sizeof(MapNPCContext));
    LOG_INFO("Map NPC system shutdown");
}

/*
 * Clear all map NPCs - FUN_0040f600
 */
void map_npc_clear(void) {
    u32 i;

    g_map_npcs.count = 0;
    g_map_npcs.high_water = 0;
    g_map_npcs.next_index = 0;
    g_map_npcs.search_index = 0;

    /* Clear all object slots - 0x10c bytes each */
    for (i = 0; i < MAX_MAP_OBJECTS; i++) {
        g_map_npcs.objects[i].type = 0;
        g_map_npcs.objects[i].anim_state = 0;
        g_map_npcs.objects[i].id = 0;
        g_map_npcs.objects[i].render_flags = 0xFFFFFFFF;
        g_map_npcs.objects[i].obj_flags = 0;
    }
}

/*
 * Find free map NPC slot - FUN_0040f310
 * Max NPCs is 0x5dc (1500)
 */
static int map_npc_find_free_slot(void) {
    int index;
    int start;

    /* Check if at max capacity - from FUN_0040f310: 0x5db < DAT_004e2b14 */
    if (g_map_npcs.count >= MAX_MAP_OBJECTS - 1) {
        return -1;
    }

    start = g_map_npcs.next_index;
    g_map_npcs.count++;

    /* Track high water mark */
    if (g_map_npcs.next_index + 1 > g_map_npcs.high_water) {
        g_map_npcs.high_water = g_map_npcs.next_index + 1;
    }

    /* Search for free slot starting from next_index */
    for (index = g_map_npcs.next_index; index < MAX_MAP_OBJECTS; index++) {
        if (g_map_npcs.objects[index].type == 0) {
            g_map_npcs.next_index = index;
            return start;  /* Return original index as per original */
        }
    }

    /* Search from beginning */
    for (index = 0; index < g_map_npcs.next_index; index++) {
        if (g_map_npcs.objects[index].type == 0) {
            g_map_npcs.next_index = index;
            return index;
        }
    }

    return g_map_npcs.next_index;
}

/*
 * Get type-specific value - FUN_0040f3f0
 */
u16 map_npc_get_type_value(u32 obj_type) {
    switch (obj_type) {
        case OBJ_TYPE_NORMAL:
            return NPC_TYPEVAL_NORMAL;
        case OBJ_TYPE_NPC:
            return NPC_TYPEVAL_NPC;
        case OBJ_TYPE_PLAYER:
        case OBJ_TYPE_PET:
        case OBJ_TYPE_ENEMY:
            return NPC_TYPEVAL_CHARACTER;
        case OBJ_TYPE_EFFECT:
            return NPC_TYPEVAL_EFFECT;
        default:
            return NPC_TYPEVAL_NORMAL;
    }
}

/*
 * Get render flags by type - FUN_0040f3c0
 */
u32 map_npc_get_render_flags(u32 obj_type) {
    switch (obj_type) {
        case OBJ_TYPE_NORMAL:
        case OBJ_TYPE_NPC:
            return 0x100;    /* 256 - NPC render layer */
        case OBJ_TYPE_PLAYER:
            return 0x200;    /* 512 - Player render layer */
        case OBJ_TYPE_EFFECT:
            return 0x100;    /* 256 - Effect layer */
        default:
            return 0x1000;   /* 4096 - Default layer */
    }
}

/*
 * Create map NPC - FUN_0040ee70
 */
int map_npc_create(u32 id, u32 x, u32 y, u32 param1, u32 param2, u32 param3,
                   const char* name, const char* title, u32 extra_param,
                   u16 sprite_id, int hidden, int invisible) {
    int index;
    MapNPC* npc;

    /* Check if ID already exists */
    index = map_npc_find_by_id(id);
    if (index >= 0) {
        return 0;  /* Already exists */
    }

    /* Find free slot */
    index = map_npc_find_free_slot();
    if (index < 0) {
        return 0;  /* No free slots */
    }

    npc = &g_map_npcs.objects[index];

    /* Initialize NPC structure */
    npc->type = 2;  /* Active type */
    npc->anim_state = map_npc_get_type_value(extra_param);
    npc->id = id;
    npc->x = x;
    npc->y = y;
    npc->param1 = param1;
    npc->param2 = param2;
    npc->param3 = param3;
    npc->extra_param = extra_param;
    npc->sprite_id = sprite_id;

    /* Set flags */
    if (hidden) {
        npc->obj_flags |= OBJ_FLAG_HIDDEN;
    }
    if (invisible) {
        npc->obj_flags |= OBJ_FLAG_INVISIBLE;
    }

    /* Copy name (max 16 chars) */
    if (name && strlen(name) < MAX_OBJ_NAME_LEN) {
        strncpy(npc->name, name, MAX_OBJ_NAME_LEN);
        npc->name[MAX_OBJ_NAME_LEN] = '\0';
    }

    /* Copy title (max 32 chars) */
    if (title && strlen(title) < MAX_OBJ_TITLE_LEN) {
        strncpy(npc->title, title, MAX_OBJ_TITLE_LEN);
        npc->title[MAX_OBJ_TITLE_LEN] = '\0';
    }

    /* Set render flags */
    npc->render_flags = map_npc_get_render_flags(extra_param);

    return 1;
}

/*
 * Find map NPC by ID - FUN_0040e830
 */
int map_npc_find_by_id(u32 id) {
    int index;

    /* Search from last found index for optimization */
    if (g_map_npcs.search_index < g_map_npcs.high_water) {
        if (g_map_npcs.objects[g_map_npcs.search_index].type != 0 &&
            g_map_npcs.objects[g_map_npcs.search_index].id == id) {
            return g_map_npcs.search_index;
        }
    }

    /* Linear search */
    for (index = 0; index < g_map_npcs.high_water; index++) {
        if (g_map_npcs.objects[index].type != 0 &&
            g_map_npcs.objects[index].id == id) {
            g_map_npcs.search_index = index;
            return index;
        }
    }

    return -1;
}

/*
 * Get map NPC by ID
 */
MapNPC* map_npc_get_by_id(u32 id) {
    int index = map_npc_find_by_id(id);
    if (index >= 0) {
        return &g_map_npcs.objects[index];
    }
    return NULL;
}

/*
 * Get map NPC by index
 */
MapNPC* map_npc_get_by_index(u32 index) {
    if (index >= MAX_MAP_OBJECTS) {
        return NULL;
    }
    return &g_map_npcs.objects[index];
}

/*
 * Remove map NPC
 */
void map_npc_remove(u32 id) {
    int index = map_npc_find_by_id(id);
    if (index >= 0) {
        memset(&g_map_npcs.objects[index], 0, sizeof(MapNPC));
        g_map_npcs.objects[index].render_flags = 0xFFFFFFFF;
        if (g_map_npcs.count > 0) {
            g_map_npcs.count--;
        }
    }
}

/*
 * Update all map NPCs
 */
void map_npc_update_all(void) {
    u32 i;
    MapNPC* npc;

    for (i = 0; i < g_map_npcs.high_water; i++) {
        npc = &g_map_npcs.objects[i];
        if (npc->type == 0) continue;

        /* Update animation state */
        if (npc->sprite_id != 0) {
            /* Use animation system to update entity animation */
            anim_update_entity(npc->id, 16);  /* ~16ms per frame at 60fps */
        }
    }
}

/*
 * Render all map NPCs
 */
void map_npc_render_all(void) {
    u32 i;
    MapNPC* npc;
    int screen_x, screen_y;

    for (i = 0; i < g_map_npcs.high_water; i++) {
        npc = &g_map_npcs.objects[i];
        if (npc->type == 0) continue;
        if (npc->obj_flags & OBJ_FLAG_HIDDEN) continue;

        /* Convert world to screen coordinates */
        map_world_to_screen(npc->x, npc->y, &screen_x, &screen_y);

        /* Apply camera offset */
        screen_x -= g_map.camera_x;
        screen_y -= g_map.camera_y;

        /* Render sprite */
        if (npc->sprite_id != 0) {
            render_sprite(npc->sprite_id, screen_x, screen_y);
        }
    }
}
