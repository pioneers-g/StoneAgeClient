/*
 * Stone Age Client - Battle Unit Management
 * Split from battle.c for code organization
 *
 * Handles unit creation, lookup, position management
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "battle.h"
#include "character.h"
#include "assets.h"
#include "logger.h"

/* External reference to global battle context */
extern BattleContext g_battle;

/*
 * Get unit by ID
 */
BattleUnit* battle_get_unit(u32 id) {
    u32 i;

    for (i = 0; i < g_battle.unit_count; i++) {
        if (g_battle.units[i].id == id) {
            return &g_battle.units[i];
        }
    }

    return NULL;
}

/*
 * Get unit by index
 */
BattleUnit* battle_get_unit_by_index(u32 index) {
    if (index >= g_battle.unit_count) return NULL;
    return &g_battle.units[index];
}

/*
 * Get unit at index (alias)
 */
BattleUnit* battle_get_unit_at(u32 index) {
    if (index >= g_battle.unit_count) {
        return NULL;
    }
    return &g_battle.units[index];
}

/*
 * Add unit to battle
 */
void battle_add_unit(BattleUnit* unit, u8 side) {
    if (g_battle.unit_count >= 20) return;

    memcpy(&g_battle.units[g_battle.unit_count], unit, sizeof(BattleUnit));

    /* Set position based on side */
    if (side == 0) {
        /* Player side - left */
        unit->x = 100 + (g_battle.player_count % 5) * 40;
        unit->y = 200 + (g_battle.player_count / 5) * 60;
        g_battle.player_count++;
    } else {
        /* Enemy side - right */
        unit->x = 400 + (g_battle.enemy_count % 5) * 40;
        unit->y = 200 + (g_battle.enemy_count / 5) * 60;
        g_battle.enemy_count++;
    }

    g_battle.units[g_battle.unit_count].x = unit->x;
    g_battle.units[g_battle.unit_count].y = unit->y;

    g_battle.unit_count++;
}

/*
 * Remove unit from battle
 */
void battle_remove_unit(u32 id) {
    u32 i;

    for (i = 0; i < g_battle.unit_count; i++) {
        if (g_battle.units[i].id == id) {
            /* Shift remaining units */
            for (; i < g_battle.unit_count - 1; i++) {
                g_battle.units[i] = g_battle.units[i + 1];
            }
            g_battle.unit_count--;
            return;
        }
    }
}

/*
 * Find unit by character ID
 */
BattleUnit* battle_find_unit_by_char_id(u32 char_id) {
    u32 i;

    for (i = 0; i < g_battle.unit_count; i++) {
        if (g_battle.units[i].char_id == char_id) {
            return &g_battle.units[i];
        }
    }

    return NULL;
}

/*
 * Find unit by position
 */
BattleUnit* battle_find_unit_at_position(u16 x, u16 y) {
    u32 i;

    for (i = 0; i < g_battle.unit_count; i++) {
        BattleUnit* unit = &g_battle.units[i];
        if (unit->is_alive) {
            /* Check if position is within unit bounds */
            if (x >= unit->x - 30 && x <= unit->x + 30 &&
                y >= unit->y - 40 && y <= unit->y + 40) {
                return unit;
            }
        }
    }

    return NULL;
}

/*
 * Count alive units on side
 */
u32 battle_count_alive_units(u8 side) {
    u32 count = 0;
    u32 i;

    for (i = 0; i < g_battle.unit_count; i++) {
        if (g_battle.units[i].is_alive && g_battle.units[i].side == side) {
            count++;
        }
    }

    return count;
}

/*
 * Find first alive enemy
 */
BattleUnit* battle_find_first_alive_enemy(u8 my_side) {
    u32 i;

    for (i = 0; i < g_battle.unit_count; i++) {
        if (g_battle.units[i].is_alive && g_battle.units[i].side != my_side) {
            return &g_battle.units[i];
        }
    }

    return NULL;
}

/*
 * Find lowest HP enemy
 */
BattleUnit* battle_find_lowest_hp_enemy(u8 my_side) {
    BattleUnit* lowest = NULL;
    u32 lowest_hp = 0xFFFFFFFF;
    u32 i;

    for (i = 0; i < g_battle.unit_count; i++) {
        BattleUnit* unit = &g_battle.units[i];
        if (unit->is_alive && unit->side != my_side) {
            if (unit->hp < lowest_hp) {
                lowest_hp = unit->hp;
                lowest = unit;
            }
        }
    }

    return lowest;
}

/*
 * Find lowest HP ally
 */
BattleUnit* battle_find_lowest_hp_ally(u8 my_side) {
    BattleUnit* lowest = NULL;
    u32 lowest_percent = 100;
    u32 i;

    for (i = 0; i < g_battle.unit_count; i++) {
        BattleUnit* unit = &g_battle.units[i];
        if (unit->is_alive && unit->side == my_side) {
            u32 hp_percent = (unit->hp * 100) / unit->max_hp;
            if (hp_percent < lowest_percent) {
                lowest_percent = hp_percent;
                lowest = unit;
            }
        }
    }

    return lowest;
}

/*
 * Find random alive enemy
 */
BattleUnit* battle_find_random_enemy(u8 my_side) {
    BattleUnit* enemies[20];
    u32 enemy_count = 0;
    u32 i;

    for (i = 0; i < g_battle.unit_count; i++) {
        if (g_battle.units[i].is_alive && g_battle.units[i].side != my_side) {
            enemies[enemy_count++] = &g_battle.units[i];
        }
    }

    if (enemy_count == 0) return NULL;

    return enemies[rand() % enemy_count];
}

/*
 * Setup battle positions - FUN_00440df0
 */
void battle_setup_positions(u32 x, u32 y) {
    g_battle.player_x = (u16)x;
    g_battle.player_y = (u16)y;

    /* Clear target selections */
    g_battle.selected_targets[0] = -1;
    g_battle.selected_targets[1] = -1;
    g_battle.selected_targets[2] = -1;
    g_battle.selected_targets[3] = -1;
    g_battle.selected_targets[4] = -1;

    g_battle.active = 1;
}

/*
 * Initialize turn order based on unit speed
 */
void battle_init_turn_order(void) {
    u32 i, j;
    u32 temp;

    /* Build turn order array based on unit speed */
    g_battle.action_count = 0;

    for (i = 0; i < g_battle.unit_count; i++) {
        if (g_battle.units[i].is_alive) {
            g_battle.turn_order[g_battle.action_count] = i;
            g_battle.action_count++;
        }
    }

    /* Sort by speed (descending) - faster units act first */
    for (i = 0; i < g_battle.action_count - 1; i++) {
        for (j = i + 1; j < g_battle.action_count; j++) {
            BattleUnit* u1 = &g_battle.units[g_battle.turn_order[i]];
            BattleUnit* u2 = &g_battle.units[g_battle.turn_order[j]];

            if (u1->speed < u2->speed) {
                temp = g_battle.turn_order[i];
                g_battle.turn_order[i] = g_battle.turn_order[j];
                g_battle.turn_order[j] = temp;
            }
        }
    }

    LOG_DEBUG("Turn order initialized for %d units", g_battle.action_count);
}

/*
 * Reset unit animations
 */
void battle_reset_animations(void) {
    u32 i;

    /* Reset animation state for all units */
    for (i = 0; i < g_battle.unit_count; i++) {
        g_battle.units[i].animation = 0;
        g_battle.units[i].frame = 0;
        g_battle.units[i].effect_id = 0;
        g_battle.units[i].effect_timer = 0;
    }

    /* Reset battle animation timer */
    g_battle.animation_timer = 0;
    g_battle.current_actor = 0;
}

/*
 * Check if battle is active
 */
int battle_is_active(void) {
    return g_battle.active;
}

/*
 * Check if battle should end
 */
int battle_check_end_condition(void) {
    u32 player_alive = 0;
    u32 enemy_alive = 0;
    u32 i;

    for (i = 0; i < g_battle.unit_count; i++) {
        if (g_battle.units[i].is_alive) {
            if (g_battle.units[i].side == 0) {
                player_alive = 1;
            } else {
                enemy_alive = 1;
            }
        }
    }

    return (!player_alive || !enemy_alive) ? 1 : 0;
}

/*
 * Find rideable pet after battle
 */
int battle_find_rideable_pet(void) {
    u32 i;
    for (i = 0; i < g_battle.unit_count; i++) {
        BattleUnit* unit = &g_battle.units[i];
        if (unit->is_alive && unit->is_pet && unit->side == 0) {
            /* Check if pet is rideable (sprite ID in valid range) */
            if (unit->sprite_id >= 24000 && unit->sprite_id <= 24044) {
                return (int)i;
            }
        }
    }
    return -1;
}

/*
 * Update unit display
 */
void battle_update_unit_display(u32 unit_index) {
    /* FUN_004053e0 pattern */
    if (unit_index >= g_battle.unit_count) return;
}

/*
 * Update all unit displays
 */
void battle_update_unit_displays(void) {
    u32 i;
    for (i = 0; i < g_battle.unit_count; i++) {
        battle_update_unit_display(i);
    }
}
