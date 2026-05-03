/*
 * Stone Age Client - Character System Implementation
 * Reverse engineered from sa_9061.exe (FUN_00464670, FUN_00446df0, FUN_00440df0)
 *
 * Core character management: player, movement, animation, stats
 * Integrated with animation system for sprite frame sequencing
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "types.h"
#include "character.h"
#include "animation.h"
#include "map.h"
#include "render.h"
#include "pet.h"
#include "logger.h"

/* Global character context */
CharacterContext g_chars = {0};

/* Animation frame timing */
#define ANIM_FRAME_TIME    100   /* ms per frame */
#define WALK_SPEED         4
#define RUN_SPEED          8

/* Next animation entity ID */
static u32 s_next_anim_entity_id = 1;

/* Isometric transformation constants - from DAT_0049c334, DAT_0049c3e4, DAT_0049c3e8 */
#define ISO_TILE_WIDTH     32.0f
#define ISO_TILE_HEIGHT    16.0f
#define ISO_OFFSET_X       0.5f
#define ISO_OFFSET_Y       0.25f

/* Reference coordinates - DAT_04582998, DAT_04582994 */
static float s_ref_x = 0.0f;
static float s_ref_y = 0.0f;

/* Screen offset - DAT_004bb424, DAT_004bb428 */
static float s_screen_offset_x = 400.0f;
static float s_screen_offset_y = 300.0f;

/*
 * Initialize character system
 */
int character_init(void) {
    memset(&g_chars, 0, sizeof(CharacterContext));

    /* Initialize animation system */
    anim_init();

    /* Assign animation entity ID to player */
    g_chars.player.anim_entity_id = s_next_anim_entity_id++;

    LOG_INFO("Character system initialized");
    return 1;
}

/*
 * Shutdown character system
 */
void character_shutdown(void) {
    anim_shutdown();
    memset(&g_chars, 0, sizeof(CharacterContext));
    LOG_INFO("Character system shutdown");
}

/*
 * Get player character
 */
Character* character_get_player(void) {
    return &g_chars.player;
}

/*
 * Set player info
 */
void character_set_player_info(Character* info) {
    if (info) {
        memcpy(&g_chars.player, info, sizeof(Character));
    }
}

/*
 * Set player position
 */
void character_set_position(u16 x, u16 y) {
    g_chars.player.x = x;
    g_chars.player.y = y;
    g_chars.player.dest_x = x;
    g_chars.player.dest_y = y;
    g_chars.player.is_moving = 0;
}

/*
 * Set player direction
 */
void character_set_direction(Direction dir) {
    Character* player = &g_chars.player;
    player->direction = (u8)dir;

    /* Update animation direction */
    if (player->anim_entity_id != 0 && player->anim_playing) {
        anim_set_direction(player->anim_entity_id, (u8)dir);
    }
}

/*
 * Set player action
 */
void character_set_action(CharacterAction action) {
    g_chars.player.action = (u8)action;
    g_chars.player.frame = 0;
    g_chars.player.frame_time = 0;
}

/*
 * Move character to position
 */
int character_move_to(u16 x, u16 y) {
    Character* player = &g_chars.player;
    int dx, dy;
    u8 new_dir;

    if (!map_is_walkable(x, y)) {
        return 0;
    }

    /* Calculate direction */
    dx = (int)x - (int)player->x;
    dy = (int)y - (int)player->y;

    /* Use animation system direction calculation */
    new_dir = anim_direction_from_delta((s16)dx, (s16)dy);
    player->direction = new_dir;

    player->dest_x = x;
    player->dest_y = y;
    player->action = ACTION_WALK;
    player->is_moving = 1;

    /* Start walk animation */
    if (player->anim_entity_id != 0) {
        anim_play_loop(player->anim_entity_id,
                      g_anim.default_anims[ANIM_TYPE_WALK],
                      new_dir, -1);
        player->anim_playing = 1;
    }

    return 1;
}

/*
 * Move character in direction - simple one-step movement
 */
void character_move(int direction) {
    Character* player = &g_chars.player;
    u16 new_x = player->x;
    u16 new_y = player->y;

    player->direction = (u8)direction;

    switch (direction) {
        case DIR_NORTH:     new_y--; break;
        case DIR_SOUTH:     new_y++; break;
        case DIR_WEST:      new_x--; break;
        case DIR_EAST:      new_x++; break;
        case DIR_NORTHEAST: new_x++; new_y--; break;
        case DIR_SOUTHEAST: new_x++; new_y++; break;
        case DIR_SOUTHWEST: new_x--; new_y++; break;
        case DIR_NORTHWEST: new_x--; new_y--; break;
    }

    if (map_is_walkable(new_x, new_y)) {
        character_move_to(new_x, new_y);
    }
}

/*
 * Update character movement
 */
void character_update_movement(void) {
    Character* player = &g_chars.player;
    int dx, dy, move_speed;

    if (!player->is_moving) return;

    /* Calculate distance to destination */
    dx = (int)player->dest_x - (int)player->x;
    dy = (int)player->dest_y - (int)player->y;

    /* Check if arrived */
    if (dx == 0 && dy == 0) {
        player->is_moving = 0;
        player->action = ACTION_STAND;
        return;
    }

    /* Move towards destination */
    move_speed = (player->action == ACTION_RUN) ? RUN_SPEED : WALK_SPEED;

    if (abs(dx) > move_speed) {
        player->x += (dx > 0) ? move_speed : -move_speed;
    } else {
        player->x = player->dest_x;
    }

    if (abs(dy) > move_speed) {
        player->y += (dy > 0) ? move_speed : -move_speed;
    } else {
        player->y = player->dest_y;
    }
}

/*
 * Stop movement
 */
void character_stop_movement(void) {
    Character* player = &g_chars.player;

    player->is_moving = 0;
    player->action = ACTION_STAND;

    /* Switch to idle animation */
    if (player->anim_entity_id != 0) {
        anim_play_loop(player->anim_entity_id,
                      g_anim.default_anims[ANIM_TYPE_IDLE],
                      player->direction, -1);
    }
}

/*
 * Update animation - integrates with animation system
 */
void character_update_animation(void) {
    Character* player = &g_chars.player;
    DWORD current_time;
    static DWORD last_update = 0;
    u32 delta_time;

    current_time = timeGetTime();
    delta_time = current_time - last_update;

    if (delta_time < ANIM_FRAME_TIME) {
        return;
    }
    last_update = current_time;

    /* Update animation through animation system */
    if (player->anim_playing && player->anim_entity_id != 0) {
        anim_update_entity(player->anim_entity_id, delta_time);

        /* Sync frame from animation system */
        player->frame = anim_get_current_frame(player->anim_entity_id);
    } else {
        /* Fallback: simple frame advancement */
        player->frame++;
        if (player->frame >= 8) {
            player->frame = 0;
        }
    }
}

/*
 * Play action animation - uses animation system
 */
void character_play_action(CharacterAction action) {
    Character* player = &g_chars.player;
    int anim_type = character_action_to_anim_type(action);

    player->action = (u8)action;
    player->frame = 0;
    player->animation_loop = (action == ACTION_STAND || action == ACTION_SIT) ? 1 : 0;

    /* Play animation through animation system */
    if (player->anim_entity_id != 0) {
        if (player->animation_loop) {
            anim_play_loop(player->anim_entity_id,
                          g_anim.default_anims[anim_type],
                          player->direction, -1);
        } else {
            anim_play_type(player->anim_entity_id, anim_type, player->direction);
        }
        player->anim_playing = 1;
    }
}

/*
 * Render player character
 */
void character_render_player(void) {
    Character* player = &g_chars.player;
    int screen_x, screen_y;

    /* Convert world to screen coordinates */
    map_world_to_screen(player->x, player->y, &screen_x, &screen_y);

    character_render(player, screen_x, screen_y);
}

/*
 * Render all characters
 */
void character_render_all(void) {
    u32 i;
    Character* ch;
    int screen_x, screen_y;

    /* Render other characters */
    for (i = 0; i < g_chars.character_count; i++) {
        ch = &g_chars.characters[i];
        if (!ch->visible) continue;

        map_world_to_screen(ch->x, ch->y, &screen_x, &screen_y);
        character_render(ch, screen_x, screen_y);
    }

    /* Render player last (on top) */
    character_render_player();
}

/*
 * Render single character - uses animation system for sprite data
 */
void character_render(Character* ch, int screen_x, int screen_y) {
    u16 sprite_id;
    AnimFrame* frame_data;

    if (!ch || !ch->visible) return;

    /* Try to get frame data from animation system */
    if (ch->anim_entity_id != 0 && ch->anim_playing) {
        frame_data = anim_get_current_frame_data(ch->anim_entity_id);
        if (frame_data) {
            sprite_id = frame_data->sprite_id;
            /* Render with offset from animation frame */
            render_sprite(sprite_id,
                         screen_x + frame_data->offset_x,
                         screen_y + frame_data->offset_y);
        } else {
            /* Fallback to base sprite */
            sprite_id = ch->appearance.base_sprite;
            render_sprite_frame(sprite_id, ch->direction * 8 + ch->frame,
                               screen_x, screen_y);
        }
    } else {
        /* No animation active - use simple rendering */
        sprite_id = ch->appearance.base_sprite;
        render_sprite_frame(sprite_id, ch->direction * 8 + ch->frame,
                           screen_x, screen_y);
    }

    /* Render name above character */
    if (ch->name[0] != '\0') {
        render_text_centered(screen_x, screen_y - 20, ch->name, COLOR_WHITE);
    }
}

/*
 * Find character by ID
 */
Character* character_find_by_id(u32 id) {
    u32 i;

    if (g_chars.player.id == id) {
        return &g_chars.player;
    }

    for (i = 0; i < g_chars.character_count; i++) {
        if (g_chars.characters[i].id == id) {
            return &g_chars.characters[i];
        }
    }

    return NULL;
}

/*
 * Add character to list
 */
void character_add(Character* ch) {
    if (g_chars.character_count >= MAX_CHARACTERS) return;

    memcpy(&g_chars.characters[g_chars.character_count], ch, sizeof(Character));

    /* Assign animation entity ID */
    g_chars.characters[g_chars.character_count].anim_entity_id = s_next_anim_entity_id++;

    g_chars.character_count++;
}

/*
 * Remove character from list
 */
void character_remove(u32 id) {
    u32 i;

    for (i = 0; i < g_chars.character_count; i++) {
        if (g_chars.characters[i].id == id) {
            memmove(&g_chars.characters[i], &g_chars.characters[i + 1],
                (g_chars.character_count - i - 1) * sizeof(Character));
            g_chars.character_count--;
            break;
        }
    }
}

/*
 * Update all characters
 */
void character_update_all(void) {
    u32 i;
    DWORD current_time;
    static DWORD last_update = 0;
    u32 delta_time;

    current_time = timeGetTime();
    delta_time = current_time - last_update;
    if (delta_time > 100) delta_time = 100;  /* Cap delta */

    /* Update player */
    character_update_movement();
    character_update_animation();

    /* Update other characters */
    for (i = 0; i < g_chars.character_count; i++) {
        Character* ch = &g_chars.characters[i];

        /* Update movement */
        if (ch->is_moving) {
            int dx = (int)ch->dest_x - (int)ch->x;
            int dy = (int)ch->dest_y - (int)ch->y;

            if (dx == 0 && dy == 0) {
                ch->is_moving = 0;
                ch->action = ACTION_STAND;

                /* Switch to idle animation */
                if (ch->anim_entity_id != 0) {
                    anim_play_type(ch->anim_entity_id, 0, ch->direction);  /* ANIM_TYPE_IDLE */
                }
            } else {
                if (abs(dx) > WALK_SPEED) {
                    ch->x += (dx > 0) ? WALK_SPEED : -WALK_SPEED;
                } else {
                    ch->x = ch->dest_x;
                }
                if (abs(dy) > WALK_SPEED) {
                    ch->y += (dy > 0) ? WALK_SPEED : -WALK_SPEED;
                } else {
                    ch->y = ch->dest_y;
                }
            }
        }

        /* Update animation through animation system */
        if (ch->anim_entity_id != 0 && ch->anim_playing) {
            anim_update_entity(ch->anim_entity_id, delta_time);
            ch->frame = anim_get_current_frame(ch->anim_entity_id);
        } else {
            /* Fallback: simple frame advancement */
            ch->frame++;
            if (ch->frame >= 8) ch->frame = 0;
        }
    }

    last_update = current_time;
}

/*
 * NPC functions (simple NPC list in CharacterContext)
 */
NPC* character_npc_find_by_id(u32 id) {
    u32 i;

    for (i = 0; i < g_chars.npc_count; i++) {
        if (g_chars.npcs[i].id == id) {
            return &g_chars.npcs[i];
        }
    }

    return NULL;
}

NPC* character_npc_find_at_position(u16 x, u16 y) {
    u32 i;

    for (i = 0; i < g_chars.npc_count; i++) {
        if (g_chars.npcs[i].x == x && g_chars.npcs[i].y == y) {
            return &g_chars.npcs[i];
        }
    }

    return NULL;
}

void npc_add(NPC* npc) {
    if (g_chars.npc_count >= MAX_NPCS) return;

    memcpy(&g_chars.npcs[g_chars.npc_count], npc, sizeof(NPC));
    g_chars.npc_count++;
}

void npc_remove(u32 id) {
    u32 i;

    for (i = 0; i < g_chars.npc_count; i++) {
        if (g_chars.npcs[i].id == id) {
            memmove(&g_chars.npcs[i], &g_chars.npcs[i + 1],
                (g_chars.npc_count - i - 1) * sizeof(NPC));
            g_chars.npc_count--;
            break;
        }
    }
}

void npc_render_all(void) {
    u32 i;
    NPC* npc;
    int screen_x, screen_y;

    for (i = 0; i < g_chars.npc_count; i++) {
        npc = &g_chars.npcs[i];

        map_world_to_screen(npc->x, npc->y, &screen_x, &screen_y);

        /* Render NPC sprite */
        render_sprite(npc->sprite_id, screen_x, screen_y);

        /* Render NPC name */
        if (npc->name[0] != '\0') {
            render_text_centered(screen_x, screen_y - 20, npc->name, COLOR_YELLOW);
        }
    }
}

/*
 * Pet functions
 */
Pet* pet_get_by_index(int index) {
    if (index < 0 || index >= MAX_PETS) return NULL;
    return &g_chars.pets[index];
}

void pet_update_all(void) {
    int i;
    Character* player = &g_chars.player;

    for (i = 0; i < MAX_PETS; i++) {
        Pet* pet = &g_chars.pets[i];
        if (pet->id == 0) continue;

        /* Pet follows player */
        pet->offset_x = (i == 0) ? -2 : 2;
        pet->offset_y = 1;
    }
}

/*
 * Recalculate character stats
 */
void character_recalculate_stats(Character* ch) {
    if (!ch) return;

    /* Calculate derived stats */
    ch->stats.attack = ch->stats.strength * 2 + ch->stats.level;
    ch->stats.defense = ch->stats.vitality * 2 + ch->stats.level;
    ch->stats.magic_attack = ch->stats.intelligence * 2 + ch->stats.level;
    ch->stats.magic_defense = ch->stats.intelligence + ch->stats.level;
    ch->stats.hit_rate = ch->stats.dexterity + ch->stats.level;
    ch->stats.dodge_rate = ch->stats.agility + ch->stats.level;
}

/*
 * Check if character can level up
 */
int character_can_level_up(Character* ch) {
    if (!ch) return 0;
    return ch->stats.experience >= ch->stats.next_level_exp;
}

/*
 * Level up character
 */
void character_level_up(Character* ch) {
    if (!ch || !character_can_level_up(ch)) return;

    ch->stats.level++;
    ch->stats.experience -= ch->stats.next_level_exp;
    ch->stats.next_level_exp = ch->stats.level * ch->stats.level * 100;

    /* Increase stats */
    ch->stats.max_hp += 10;
    ch->stats.max_mp += 5;
    ch->stats.hp = ch->stats.max_hp;
    ch->stats.mp = ch->stats.max_mp;

    character_recalculate_stats(ch);

    LOG_INFO("Character leveled up to %d", ch->stats.level);
}

/*
 * World to screen coordinate transformation - FUN_00446df0
 * Converts isometric world coordinates to screen coordinates
 */
void world_to_screen(float world_x, float world_y, float* screen_x, float* screen_y) {
    float dx, dy;

    /* Calculate offset from reference point */
    dx = (world_x - s_ref_x) * ISO_TILE_WIDTH;
    dy = (world_y - s_ref_y) * ISO_TILE_WIDTH;

    /* Isometric transformation */
    *screen_x = (dy + dx) * ISO_OFFSET_X + s_screen_offset_x;
    *screen_y = (dy - dx) * ISO_OFFSET_Y + s_screen_offset_y;
}

/*
 * Screen to world coordinate transformation - FUN_00446e40
 * Converts screen coordinates back to isometric world coordinates
 */
void screen_to_world(float screen_x, float screen_y, float* world_x, float* world_y) {
    float dx, dy;

    /* Calculate offset from screen center */
    dx = screen_x - s_screen_offset_x;
    dy = screen_y - s_screen_offset_y;

    /* Reverse isometric transformation */
    *world_x = ((dx * ISO_OFFSET_Y) - (dy * ISO_OFFSET_X)) + s_ref_x;
    *world_y = ((dx * ISO_OFFSET_Y) + (dy * ISO_OFFSET_X)) + s_ref_y;
}

/*
 * Setup character position - FUN_00440df0
 */
void character_setup_position(u16 x, u16 y) {
    g_chars.player.x = x;
    g_chars.player.y = y;
    g_chars.player.dest_x = x;
    g_chars.player.dest_y = y;
    g_chars.player.is_moving = 0;

    /* Update float positions for rendering */
    /* In the original: DAT_0456a644, DAT_0456a648 */
}
