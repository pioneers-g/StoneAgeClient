/*
 * Stone Age Client - Field Entity System Implementation
 * Reverse engineered from sa_9061.exe
 *
 * Key functions:
 * - FUN_004779d0: Field UI initialization
 * - FUN_00477b20: Player entity spawn/update
 * - FUN_00477dc0: Player info display setup
 * - FUN_0040b5e0: Entity creation
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "field_entity.h"
#include "character.h"
#include "map.h"
#include "render.h"
#include "logger.h"

/* Global context */
FieldEntityContext g_field_entities = {0};

/* Isometric transformation constants - from FUN_00446df0 */
#define ISO_CONSTANT_1  0.00390625f   /* _DAT_0049c334 = 1/256 */
#define ISO_CONSTANT_2  0.5f          /* _DAT_0049c3e8 */
#define ISO_CONSTANT_3  0.5f          /* _DAT_0049c3e4 */

/* Screen center offsets - DAT_004bb424, DAT_004bb428 */
#define SCREEN_CENTER_X  400
#define SCREEN_CENTER_Y  300

/* World offset - _DAT_04582998, _DAT_04582994 */
static float s_world_offset_x = 0.0f;
static float s_world_offset_y = 0.0f;

/*
 * Initialize field entity system
 */
int field_entity_init(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    LOG_INFO("Field entity system initialized");
    return 1;
}

/*
 * Shutdown field entity system
 */
void field_entity_shutdown(void) {
    field_entity_clear_all();
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));
    LOG_INFO("Field entity system shutdown");
}

/*
 * Calculate screen position from world coordinates - FUN_00446df0
 * Isometric transformation formula:
 *   screen_x = (world_y - offset_y + world_x - offset_x) * 0.5 + center_x
 *   screen_y = (world_y - offset_y - world_x - offset_x) * 0.5 + center_y
 */
void field_entity_calc_screen_pos(FieldEntity* entity) {
    float world_x, world_y;
    float screen_x, screen_y;

    if (!entity) return;

    /* Convert to float coordinates */
    world_x = entity->render_x;
    world_y = entity->render_y;

    /* Apply isometric transformation - FUN_00446df0 pattern */
    float dx = (world_x - s_world_offset_x) * ISO_CONSTANT_1;
    float dy = (world_y - s_world_offset_y) * ISO_CONSTANT_1;

    screen_x = (dy + dx) * ISO_CONSTANT_2 + (float)SCREEN_CENTER_X;
    screen_y = (dy - dx) * ISO_CONSTANT_3 + (float)SCREEN_CENTER_Y;

    entity->screen_x = (s16)screen_x;
    entity->screen_y = (s16)screen_y;
}

/*
 * Create field entity - FUN_0040b5e0 pattern
 */
FieldEntity* field_entity_create(EntityType type, u16 x, u16 y) {
    FieldEntity* entity;
    int i;

    /* Find free slot */
    for (i = 0; i < MAX_FIELD_ENTITIES; i++) {
        if (g_field_entities.entities[i].entity_kind == 0) {
            entity = &g_field_entities.entities[i];
            break;
        }
    }

    if (i >= MAX_FIELD_ENTITIES) {
        LOG_WARN("Field entity limit reached");
        return NULL;
    }

    /* Initialize entity - FUN_0040b5e0 pattern */
    memset(entity, 0, sizeof(FieldEntity));

    entity->entity_kind = (u8)(type + 1);  /* Type + 1 (0 = unused) */
    entity->index = -2;

    /* Set world position */
    entity->world_x = x;
    entity->world_y = y;
    entity->dest_x = x;
    entity->dest_y = y;

    /* Set render position */
    entity->render_x = (float)x * 0.00390625f;
    entity->render_y = (float)y * 0.00390625f;

    /* Calculate screen position */
    field_entity_calc_screen_pos(entity);

    /* Set flags */
    entity->entity_flags = ENTITY_FLAG_VISIBLE;
    entity->type_data = 10;  /* Default type data from FUN_0040b5e0 */

    /* Add to active list */
    if (g_field_entities.active_tail) {
        g_field_entities.active_tail->next = entity;
        entity->prev = g_field_entities.active_tail;
        g_field_entities.active_tail = entity;
    } else {
        g_field_entities.active_head = entity;
        g_field_entities.active_tail = entity;
    }

    g_field_entities.entity_count++;

    return entity;
}

/*
 * Destroy field entity
 */
void field_entity_destroy(FieldEntity* entity) {
    if (!entity || entity->entity_kind == 0) return;

    /* Remove from active list */
    if (entity->prev) {
        entity->prev->next = entity->next;
    } else {
        g_field_entities.active_head = entity->next;
    }

    if (entity->next) {
        entity->next->prev = entity->prev;
    } else {
        g_field_entities.active_tail = entity->prev;
    }

    /* Clear entity */
    memset(entity, 0, sizeof(FieldEntity));
    g_field_entities.entity_count--;
}

/*
 * Clear all entities
 */
void field_entity_clear_all(void) {
    int i;

    for (i = 0; i < MAX_FIELD_ENTITIES; i++) {
        if (g_field_entities.entities[i].entity_kind != 0) {
            memset(&g_field_entities.entities[i], 0, sizeof(FieldEntity));
        }
    }

    g_field_entities.entity_count = 0;
    g_field_entities.active_head = NULL;
    g_field_entities.active_tail = NULL;
    g_field_entities.player_entity = NULL;
}

/*
 * Create player entity - FUN_00477b20
 * This creates the player character entity on the field
 */
FieldEntity* field_entity_create_player(u32 sprite_id, u16 x, u16 y, u32 param) {
    FieldEntity* entity;

    /* Check if player entity already exists */
    if (g_field_entities.player_entity) {
        /* Update existing entity - FUN_00477c70 pattern */
        field_entity_update_player(sprite_id, param);
        field_entity_set_player_position(x, y);
        return g_field_entities.player_entity;
    }

    /* Create new player entity */
    entity = field_entity_create(ENTITY_TYPE_PLAYER, x, y);
    if (!entity) return NULL;

    /* Set player-specific data - FUN_0040b5e0 pattern */
    entity->sprite_base = sprite_id;
    entity->sprite_flags = param;
    entity->entity_type = 3;  /* Player type */
    entity->entity_flags |= 0x80;  /* Special flag from FUN_0040b5e0 */

    /* Store as player entity */
    g_field_entities.player_entity = entity;

    /* Allocate extended player data */
    entity->entity_data = calloc(1, sizeof(PlayerEntityData));
    if (entity->entity_data) {
        PlayerEntityData* pd = (PlayerEntityData*)entity->entity_data;
        pd->sprite_id = (u16)sprite_id;
    }

    LOG_DEBUG("Player entity created at (%d, %d)", x, y);
    return entity;
}

/*
 * Update player entity - FUN_00477c70
 */
void field_entity_update_player(u32 sprite_id, u32 param) {
    if (!g_field_entities.player_entity) return;

    g_field_entities.player_entity->sprite_base = sprite_id;
    g_field_entities.player_entity->sprite_flags = param;
}

/*
 * Set player position - FUN_00477cb0 calls FUN_00440df0
 */
void field_entity_set_player_position(u16 x, u16 y) {
    if (!g_field_entities.player_entity) return;

    g_field_entities.player_entity->world_x = x;
    g_field_entities.player_entity->world_y = y;
    g_field_entities.player_entity->dest_x = x;
    g_field_entities.player_entity->dest_y = y;

    /* Update render position */
    g_field_entities.player_entity->render_x = (float)x * 0.00390625f;
    g_field_entities.player_entity->render_y = (float)y * 0.00390625f;

    /* Recalculate screen position */
    field_entity_calc_screen_pos(g_field_entities.player_entity);

    /* Update character system */
    character_setup_position(x, y);
}

/*
 * Set player info - FUN_00477dc0
 * Sets player display info (title, guild, etc.)
 */
void field_entity_set_player_info(
    const char* title,
    const char* guild,
    u32 field_0x0c,
    const char* party,
    u32 sprite_id,
    u16 level,
    int flag_0x40,
    int flag_0x80,
    u32 param_9,
    u32 param_10,
    u32 param_11,
    u32 param_12
) {
    size_t len;
    FieldEntity* player = g_field_entities.player_entity;

    /* Set title - max 17 bytes (0x11) */
    if (title) {
        len = strlen(title);
        if (len < 17) {
            strncpy(g_field_entities.player_title, title, 16);
            g_field_entities.player_title[16] = '\0';
        }
    }

    /* Set guild name - max 33 bytes (0x21) */
    if (guild) {
        len = strlen(guild);
        if (len < 33) {
            strncpy(g_field_entities.player_guild, guild, 32);
            g_field_entities.player_guild[32] = '\0';
        }
    }

    /* Set field_0x0c */
    g_field_entities.player_field_0x0c = field_0x0c;

    /* Set party name - max 33 bytes */
    if (party) {
        len = strlen(party);
        if (len < 33) {
            strncpy(g_field_entities.player_party, party, 32);
            g_field_entities.player_party[32] = '\0';
        }
    }

    /* Set sprite and level */
    g_field_entities.player_sprite_id = sprite_id;
    g_field_entities.player_level = level;

    /* Set flags */
    if (flag_0x40) {
        g_field_entities.player_flags |= 0x40;
    }
    if (flag_0x80) {
        g_field_entities.player_flags |= 0x80;
    }

    /* Update player entity data */
    if (player && player->entity_data) {
        PlayerEntityData* pd = (PlayerEntityData*)player->entity_data;

        /* Copy title to entity data - offset 0x38 */
        strncpy(pd->title, g_field_entities.player_title, 16);
        pd->title[16] = '\0';

        /* Copy guild to entity data - offset 0x55 */
        strncpy(pd->guild_name, g_field_entities.player_guild, 32);
        pd->guild_name[32] = '\0';

        /* Set player flags - offset 0x8c */
        pd->player_flags = g_field_entities.player_field_0x0c;

        /* Set sprite - offset 0x94 */
        pd->sprite_id = (u16)sprite_id;

        /* Copy party to entity data - offset 0x194 */
        strncpy(pd->party_name, g_field_entities.player_party, 32);
        pd->party_name[32] = '\0';

        /* Set extended data - offset 0x1a8 */
        pd->guild_data = sprite_id;

        /* Set extended flags - offset 0x1cc */
        pd->ext_flags = param_9;

        /* Store additional params */
        (void)param_10;
        (void)param_11;
        (void)param_12;
    }
}

/*
 * Field UI initialization - FUN_004779d0
 * Initializes player entity and UI state for field gameplay
 */
void field_ui_init(void) {
    Character* player;
    int i;

    /* Get player character data */
    player = character_get_player();
    if (!player) {
        LOG_WARN("field_ui_init: No player character");
        return;
    }

    /* Create player entity if needed - FUN_00477b20 */
    if (!g_field_entities.player_entity) {
        field_entity_create_player(
            player->appearance.base_sprite,
            player->x,
            player->y,
            0
        );
    }

    /* Set player info - FUN_00477dc0 */
    field_entity_set_player_info(
        g_detailed_stats.title,
        g_detailed_stats.guild_name,
        g_detailed_stats.field_12,  /* intelligence as field_0x0c */
        "",  /* party name */
        player->appearance.base_sprite,
        player->stats.level,
        0,  /* flag 0x40 */
        0,  /* flag 0x80 */
        0, 0, 0, 0  /* additional params */
    );

    /* Clear entity slots - from FUN_004779d0 */
    for (i = 0; i < 20; i++) {
        g_field_entities.entity_slots_1[i] = 0;
        g_field_entities.entity_slots_2[i] = 0;
        g_field_entities.entity_slots_3[i] = 0;
    }

    /* Clear entity flags */
    g_field_entities.entity_slot_flags[0] = 0;
    g_field_entities.entity_slot_flags[1] = 0;
    g_field_entities.entity_slot_flags[2] = 0;
    g_field_entities.entity_slot_flags[3] = 0;
    g_field_entities.entity_slot_flags[4] = 0;

    LOG_DEBUG("Field UI initialized");
}

/*
 * Clear field UI state
 */
void field_ui_clear(void) {
    int i;

    /* Clear player entity reference */
    if (g_field_entities.player_entity) {
        g_field_entities.player_entity->entity_kind = 0;
        g_field_entities.player_entity = NULL;
    }

    /* Clear slots */
    for (i = 0; i < 20; i++) {
        g_field_entities.entity_slots_1[i] = 0;
        g_field_entities.entity_slots_2[i] = 0;
        g_field_entities.entity_slots_3[i] = 0;
    }

    /* Clear strings */
    g_field_entities.player_title[0] = '\0';
    g_field_entities.player_guild[0] = '\0';
    g_field_entities.player_party[0] = '\0';
    g_field_entities.player_flags = 0;
}

/*
 * Update all field entities
 */
void field_entity_update_all(void) {
    FieldEntity* entity;
    u32 current_time;

    current_time = timeGetTime();

    for (entity = g_field_entities.active_head; entity; entity = entity->next) {
        /* Skip inactive entities */
        if (entity->entity_kind == 0) continue;

        /* Update animation */
        if (entity->anim_timer > 0 && current_time >= entity->anim_timer) {
            entity->anim_state++;
            entity->anim_timer = current_time + 100;  /* 100ms per frame */
        }

        /* Update moving entities */
        if (entity->entity_flags & ENTITY_FLAG_MOVING) {
            /* Interpolate position */
            field_entity_calc_screen_pos(entity);
        }
    }
}

/*
 * Render all field entities
 * Adds entities to the render queue sorted by Y position for proper layering
 */
void field_entity_render_all(void) {
    FieldEntity* entity;
    int priority;
    u32 sprite_id;
    int direction, frame;
    u16 width, height;

    for (entity = g_field_entities.active_head; entity; entity = entity->next) {
        /* Skip invisible entities */
        if (entity->entity_kind == 0 || !(entity->entity_flags & ENTITY_FLAG_VISIBLE)) {
            continue;
        }

        /* Get sprite ID from entity */
        sprite_id = entity->sprite_base;
        if (sprite_id == 0) {
            continue;
        }

        /* Calculate priority based on Y position for proper layering
         * Entities further down (higher Y) should be drawn last (higher priority)
         * This ensures entities in front appear in front
         */
        priority = (entity->screen_y / 10) + 1;
        if (priority < 0) priority = 0;
        if (priority > 50) priority = 50;

        /* Get direction and animation frame */
        direction = entity->anim_state & 0x07;  /* 8 directions */
        frame = (entity->anim_state >> 3) & 0x0F;  /* Up to 16 frames */

        /* Calculate sprite frame based on direction and animation
         * Sprite ID formula: base_sprite + (direction * frames_per_dir) + current_frame
         */
        sprite_id = entity->sprite_base + (direction * 8) + frame;

        /* Add to render queue - FUN_0047e210 pattern
         * Parameters: x, y, sprite_id, priority, linked_object
         */
        render_queue_add_sprite(
            entity->screen_x,
            entity->screen_y,
            0,              /* Layer 0 for field entities */
            sprite_id,
            priority
        );

        /* Handle entity-specific rendering */
        if (entity->entity_kind == ENTITY_TYPE_PLAYER ||
            (entity->entity_flags & ENTITY_FLAG_GM)) {
            /* Render player name/title above entity */
            PlayerEntityData* pd = (PlayerEntityData*)entity->entity_data;
            if (pd) {
                /* Get sprite dimensions for name positioning */
                render_get_sprite_dimensions(sprite_id, &width, &height);

                /* Name would be rendered above the entity sprite
                 * This uses the text queue system
                 * Text is rendered at screen_y - height - 10
                 */
                /* TODO: Add text rendering for player names */
            }
        }
    }
}

/*
 * Get player entity
 */
FieldEntity* field_entity_get_player(void) {
    return g_field_entities.player_entity;
}

/*
 * Find entity by ID
 */
FieldEntity* field_entity_find_by_id(u32 id) {
    FieldEntity* entity;

    for (entity = g_field_entities.active_head; entity; entity = entity->next) {
        if ((u32)entity->index == id) {
            return entity;
        }
    }

    return NULL;
}

/*
 * Find entity at position
 */
FieldEntity* field_entity_find_at_position(u16 x, u16 y) {
    FieldEntity* entity;

    for (entity = g_field_entities.active_head; entity; entity = entity->next) {
        if (entity->world_x == x && entity->world_y == y) {
            return entity;
        }
    }

    return NULL;
}
