/*
 * Stone Age Client - Battle System Core
 * Reverse engineered from sa_9061.exe
 *
 * Contains:
 * - Battle initialization and shutdown
 * - Turn management
 * - Action handling
 *
 * Split modules:
 * - battle_state.c: State machine functions
 * - battle_unit.c: Unit management
 * - battle_util.c: Utility functions
 * - battle_render.c: Rendering functions
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "battle.h"
#include "battle_state.h"
#include "battle_action.h"
#include "character.h"
#include "render.h"
#include "assets.h"
#include "pet.h"
#include "petai.h"
#include "map.h"
#include "logger.h"
#include "network.h"
#include "sound.h"
#include "gamestate.h"
#include "ui.h"

/* Global battle context */
BattleContext g_battle = {0};

/* Forward declarations */
static void battle_show_message(const char* msg, int type);
static void battle_show_result_animation(u32 p2, u32 p3, u32 p4, int result);
static void battle_show_damage(u32 attacker, u32 target, u16 damage, u16 flags, u32 skill_id);
static void battle_clear_capture_ui(void);

/*
 * Initialize battle system
 */
int battle_init(void) {
    memset(&g_battle, 0, sizeof(BattleContext));
    g_battle.state = BATTLE_STATE_NONE;

    LOG_INFO("Battle system initialized");
    return 1;
}

/*
 * Shutdown battle system
 */
void battle_shutdown(void) {
    memset(&g_battle, 0, sizeof(BattleContext));
    LOG_INFO("Battle system shutdown");
}

/*
 * Start battle - FUN_00405080 pattern
 */
int battle_start(u32 battle_id, u32 enemy_count, u32* enemy_ids) {
    u32 i;

    LOG_INFO("Starting battle %d", battle_id);

    /* Reset battle context */
    memset(&g_battle, 0, sizeof(BattleContext));
    g_battle.battle_id = battle_id;
    g_battle.state = BATTLE_STATE_INIT;
    g_battle.can_escape = 1;

    /* Load battle field */
    battle_load_field(battle_id);

    /* Add player units */
    Character* player = character_get_player();
    if (player) {
        BattleUnit unit = {0};
        unit.id = player->id;
        unit.char_id = player->id;
        unit.sprite_id = player->appearance.base_sprite;
        unit.side = 0;
        unit.position = BATTLE_POS_LEFT_FRONT;
        unit.is_pet = 0;
        unit.is_alive = 1;
        unit.is_active = 1;
        unit.hp = player->stats.hp;
        unit.max_hp = player->stats.max_hp;
        unit.mp = player->stats.mp;
        unit.max_mp = player->stats.max_mp;
        unit.attack = player->stats.attack;
        unit.defense = player->stats.defense;
        unit.speed = player->stats.agility;

        battle_add_unit(&unit, 0);
    }

    /* Add enemy units */
    for (i = 0; i < enemy_count && i < 10; i++) {
        BattleUnit unit = {0};
        unit.id = 1000 + i;
        unit.char_id = enemy_ids[i];
        unit.sprite_id = enemy_ids[i];
        unit.side = 1;
        unit.position = BATTLE_POS_RIGHT_FRONT;
        unit.is_pet = 0;
        unit.is_alive = 1;
        unit.is_active = 1;

        /* Load enemy stats from sprite/asset data */
        SpriteEntry* sprite = assets_get_sprite(enemy_ids[i]);
        if (sprite) {
            unit.max_hp = 50 + (sprite->width * sprite->height) / 10;
            unit.hp = unit.max_hp;
            unit.attack = 5 + sprite->width / 8;
            unit.defense = 3 + sprite->height / 16;
            unit.speed = 3 + (rand() % 5);
        } else {
            unit.hp = 100;
            unit.max_hp = 100;
            unit.attack = 10;
            unit.defense = 5;
            unit.speed = 5;
        }

        battle_add_unit(&unit, 1);
    }

    g_battle.state = BATTLE_STATE_START;
    g_battle.turn_count = 1;

    return 1;
}

/*
 * End battle
 */
void battle_end(void) {
    g_battle.state = BATTLE_STATE_END;
    g_battle.ended = 1;
}

/*
 * Battle victory
 */
void battle_victory(void) {
    LOG_INFO("Battle victory!");
    g_battle.victory = 1;
    battle_end();
}

/*
 * Battle defeat
 */
void battle_defeat(void) {
    LOG_INFO("Battle defeat!");
    g_battle.victory = 0;
    battle_end();
}

/*
 * Update battle state - FUN_0040a1a0 pattern
 * 9-state machine: 0=init, 1=intro fade, 2=main loop, 3=execute action,
 *                  4=show result, 5=next turn, 6=message wait, 7=end start, 8=end complete
 */
void battle_update(void) {
    static u32 internal_state = 0;
    static u32 frame_counter = 0;
    static u32 action_result = 0;
    int fade_result;

    frame_counter++;

    switch (internal_state) {
        case 0: /* BATTLE_STATE_INIT */
            battle_state_init(&internal_state, &action_result);
            break;

        case 1: /* BATTLE_STATE_INTRO */
            fade_result = battle_fade_check();
            if (fade_result == 1) {
                battle_load_field(g_battle.field_id);
                battle_render_queue_process();
                g_battle.animation_timer = timeGetTime();
                g_battle.intro_phase = 3;
                battle_ui_init();
                battle_cursor_init();
                battle_field_render();
                internal_state = 2;
            } else {
                battle_cursor_init();
                battle_field_render();
            }
            break;

        case 2: /* BATTLE_STATE_MAIN */
            battle_state_main(&internal_state, &action_result);
            break;

        case 3: /* BATTLE_STATE_ACTION */
            battle_state_action(&internal_state, &action_result);
            break;

        case 4: /* BATTLE_STATE_EXECUTE */
            battle_state_execute(&internal_state);
            break;

        case 5: /* BATTLE_STATE_RESULT */
            battle_state_result(&internal_state, &action_result);
            break;

        case 6: /* BATTLE_STATE_MESSAGE */
            battle_state_message(&internal_state, &action_result);
            break;

        case 7: /* BATTLE_STATE_END_START */
            battle_state_end_start(&internal_state);
            break;

        case 8: /* BATTLE_STATE_END_COMPLETE */
            battle_state_end_complete(&internal_state);
            break;

        default:
            break;
    }

    /* Handle escape request */
    if (g_battle.intro_phase != 2 && g_battle.escape_request) {
        internal_state = 7;
        g_battle.escape_request = 0;
    }

    battle_check_special_conditions(&internal_state);
}

/*
 * Render battle
 */
void battle_render(void) {
    battle_render_field();
    battle_render_units();
    battle_render_ui();
    battle_render_effects();
}

/*
 * Select action for unit
 */
void battle_select_action(u32 unit_id, BattleAction action, u32 target_id, u16 skill_id) {
    BattleUnit* unit = battle_get_unit(unit_id);
    if (!unit || !unit->is_alive) return;

    unit->action = action;
    unit->target_id = target_id;
    unit->skill_id = skill_id;

    LOG_DEBUG("Unit %d selected action %d targeting %d", unit_id, action, target_id);

    /* Check if all units have selected */
    u32 ready = 0;
    u32 i;
    for (i = 0; i < g_battle.unit_count; i++) {
        if (g_battle.units[i].is_alive && g_battle.units[i].action != BATTLE_ACTION_NONE) {
            ready++;
        }
    }

    if (ready >= g_battle.unit_count) {
        battle_sort_actions();
        g_battle.state = BATTLE_STATE_EXECUTE;
        g_battle.current_actor = 0;
    }
}

/*
 * Execute actions
 */
void battle_execute_actions(void) {
    if (g_battle.current_actor >= g_battle.action_count) {
        g_battle.state = BATTLE_STATE_RESULT;
        return;
    }

    u32 unit_id = g_battle.action_queue[g_battle.current_actor];
    BattleUnit* unit = battle_get_unit(unit_id);

    if (unit && unit->is_alive) {
        battle_execute_action(unit);
    }

    g_battle.current_actor++;
    g_battle.animation_timer = 30;
    g_battle.state = BATTLE_STATE_ANIMATION;
}

/*
 * Execute single action
 */
void battle_execute_action(BattleUnit* attacker) {
    BattleUnit* target;
    BattleResult result = {0};
    u16 damage;

    if (!attacker || !attacker->is_alive) return;

    target = battle_get_unit(attacker->target_id);
    if (!target) {
        u32 i;
        for (i = 0; i < g_battle.unit_count; i++) {
            if (g_battle.units[i].is_alive && g_battle.units[i].side != attacker->side) {
                target = &g_battle.units[i];
                break;
            }
        }
    }

    switch (attacker->action) {
        case BATTLE_ACTION_ATTACK:
            if (target && target->is_alive) {
                damage = battle_calc_damage(attacker, target, NULL);
                target->hp -= damage;
                if (target->hp <= 0) {
                    target->hp = 0;
                    target->is_alive = 0;
                }

                result.attacker_id = attacker->id;
                result.target_id = target->id;
                result.damage = damage;
                result.critical = battle_calc_critical(attacker, target);
                result.miss = !battle_calc_hit(attacker, target);

                battle_process_result(&result);
            }
            break;

        case BATTLE_ACTION_SKILL:
            battle_use_skill(attacker, target, attacker->skill_id);
            break;

        case BATTLE_ACTION_DEFEND:
            attacker->buff_flags |= BUFF_DEFENSE_UP;
            break;

        case BATTLE_ACTION_ESCAPE:
            if (battle_try_escape()) {
                battle_end();
            }
            break;

        case BATTLE_ACTION_CAPTURE:
            if (target && target->is_alive) {
                battle_try_capture(target);
            }
            break;

        default:
            break;
    }

    attacker->action = BATTLE_ACTION_NONE;
    attacker->target_id = 0;
    attacker->skill_id = 0;
}

/*
 * Process battle result
 */
void battle_process_result(BattleResult* result) {
    g_battle.results[g_battle.result_count++] = *result;
    LOG_DEBUG("Battle result: %d -> %d, damage: %d",
        result->attacker_id, result->target_id, result->damage);
}

/*
 * Try to escape
 */
int battle_try_escape(void) {
    Character* player = character_get_player();
    int escape_chance = 50;

    if (player) {
        u32 player_agility = player->stats.agility;
        u32 enemy_agility = 0;
        u32 enemy_count = 0;
        u32 i;

        for (i = 0; i < g_battle.unit_count; i++) {
            if (g_battle.units[i].is_alive && g_battle.units[i].side == 1) {
                enemy_agility += g_battle.units[i].speed;
                enemy_count++;
            }
        }

        if (enemy_count > 0) {
            enemy_agility /= enemy_count;

            if (player_agility > enemy_agility) {
                escape_chance += (player_agility - enemy_agility) * 2;
            } else {
                escape_chance -= (enemy_agility - player_agility);
            }
        }
    }

    if (escape_chance < 10) escape_chance = 10;
    if (escape_chance > 90) escape_chance = 90;

    LOG_DEBUG("Escape chance: %d%%", escape_chance);
    return (rand() % 100) < escape_chance;
}

/*
 * Try to capture enemy
 */
int battle_try_capture(BattleUnit* target) {
    int capture_chance;
    PetData new_pet;

    if (!target || !target->is_alive) return 0;

    capture_chance = 100 - (target->hp * 100 / target->max_hp);
    capture_chance = capture_chance / 2;

    if ((rand() % 100) < capture_chance) {
        LOG_INFO("Captured enemy %d!", target->char_id);

        memset(&new_pet, 0, sizeof(PetData));
        new_pet.id = target->char_id;
        new_pet.type_id = target->char_id;
        new_pet.sprite_id = target->sprite_id;
        new_pet.level = target->level > 0 ? target->level : 1;
        new_pet.hp = target->hp;
        new_pet.max_hp = target->max_hp;
        new_pet.attack = target->attack;
        new_pet.defense = target->defense;
        new_pet.speed = target->speed;

        _snprintf(new_pet.name, sizeof(new_pet.name), "Pet_%u", target->char_id);

        if (pet_add(&new_pet)) {
            LOG_INFO("Pet added to slot");
            g_battle.capture_result[0] = 1;
            g_battle.capture_result[1] = (u16)new_pet.id;
            g_battle.capture_pet_id = new_pet.id;
        }

        target->is_alive = 0;
        return 1;
    }

    return 0;
}

/*
 * Sort actions by speed
 */
void battle_sort_actions(void) {
    u32 i, j;
    u32 temp;

    g_battle.action_count = 0;

    for (i = 0; i < g_battle.unit_count; i++) {
        if (g_battle.units[i].is_alive) {
            g_battle.action_queue[g_battle.action_count++] = g_battle.units[i].id;
        }
    }

    for (i = 0; i < g_battle.action_count - 1; i++) {
        for (j = i + 1; j < g_battle.action_count; j++) {
            BattleUnit* u1 = battle_get_unit(g_battle.action_queue[i]);
            BattleUnit* u2 = battle_get_unit(g_battle.action_queue[j]);

            if (u1 && u2 && u1->speed < u2->speed) {
                temp = g_battle.action_queue[i];
                g_battle.action_queue[i] = g_battle.action_queue[j];
                g_battle.action_queue[j] = temp;
            }
        }
    }
}

/*
 * Next turn
 */
void battle_next_turn(void) {
    u32 i;

    g_battle.turn_count++;
    g_battle.state = BATTLE_STATE_SELECT_ACTION;
    g_battle.action_count = 0;
    g_battle.result_count = 0;
    g_battle.current_actor = 0;

    for (i = 0; i < g_battle.unit_count; i++) {
        g_battle.units[i].action = BATTLE_ACTION_NONE;
        g_battle.units[i].is_active = g_battle.units[i].is_alive;
    }

    battle_update_buffs();
}

/*
 * End turn
 */
void battle_end_turn(void) {
    battle_next_turn();
}

/*
 * AI select action - integrated with petai decision engine
 */
void battle_ai_select_action(BattleUnit* unit) {
    u32 i;
    BattleUnit* target = NULL;
    PetData* pet;
    AIDecision decision;
    u8 pet_slot;

    if (!unit || !unit->is_alive) return;

    if (unit->is_pet && g_petai.ai_mode != 0) {
        pet = pet_get_by_id(unit->char_id);
        if (pet) {
            pet_slot = pet_get_slot_index(unit->char_id);
            if (pet_slot < MAX_PET_SLOTS) {
                decision = petai_decide_action(pet_slot, &g_battle, pet);

                switch (decision.action) {
                    case AI_ACTION_ATTACK:
                        target = battle_get_unit_by_index(decision.target_index);
                        if (target) {
                            battle_select_action(unit->id, BATTLE_ACTION_ATTACK, target->id, 0);
                        }
                        return;

                    case AI_ACTION_SKILL:
                        target = battle_get_unit_by_index(decision.target_index);
                        if (target) {
                            battle_select_action(unit->id, BATTLE_ACTION_SKILL, target->id, decision.skill_id);
                        }
                        return;

                    case AI_ACTION_DEFEND:
                        battle_select_action(unit->id, BATTLE_ACTION_DEFEND, 0, 0);
                        return;

                    case AI_ACTION_FLEE:
                        battle_select_action(unit->id, BATTLE_ACTION_ESCAPE, 0, 0);
                        return;

                    case AI_ACTION_ITEM:
                        battle_select_action(unit->id, BATTLE_ACTION_ITEM, decision.target_index, decision.item_id);
                        return;

                    default:
                        break;
                }
            }
        }
    }

    /* Fallback: Find random enemy target */
    for (i = 0; i < g_battle.unit_count; i++) {
        if (g_battle.units[i].is_alive && g_battle.units[i].side != unit->side) {
            target = &g_battle.units[i];
            break;
        }
    }

    if (target) {
        if ((rand() % 100) < 80) {
            battle_select_action(unit->id, BATTLE_ACTION_ATTACK, target->id, 0);
        } else {
            battle_select_action(unit->id, BATTLE_ACTION_DEFEND, 0, 0);
        }
    }
}

/*
 * Load battle field - FUN_00404850
 */
int battle_load_field(u32 field_id) {
    char path[260];
    FILE* fp;
    u32 header;
    u16 tiles[400];
    size_t read_count;

    if (field_id > 219) field_id = 0;

    snprintf(path, sizeof(path), "data/battleMap/battle%03d.sab", field_id);

    LOG_INFO("Loading battle field: %s", path);

    fp = fopen(path, "rb");
    if (!fp) {
        LOG_WARN("Failed to load battle field: %s", path);
        g_battle.field_sprite = BATTLE_SPRITE_DEFAULT;
        return 0;
    }

    if (fread(&header, 4, 1, fp) != 1) {
        fclose(fp);
        g_battle.field_sprite = BATTLE_SPRITE_DEFAULT;
        return 0;
    }

    read_count = fread(tiles, 2, 400, fp);
    fclose(fp);

    if (read_count != 400) {
        LOG_WARN("Incomplete battle field data: %s", path);
        g_battle.field_sprite = BATTLE_SPRITE_DEFAULT;
        return 0;
    }

    memcpy(g_battle.field_tiles, tiles, sizeof(tiles));
    g_battle.field_loaded = 1;

    g_battle.field_sprite = map_get_battle_sprite();

    LOG_DEBUG("Battle field %d loaded, sprite: 0x%04x", field_id, g_battle.field_sprite);
    return 1;
}

/*
 * Handle battle start - FUN_00465390
 */
void battle_on_start(u32 battle_id, u32 field_id) {
    g_battle.field_id = 0;

    if (field_id == 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Battle Start!");
        battle_show_message(msg, 0);
    }

    LOG_INFO("Battle %d started on field %d", battle_id, field_id);
}

/*
 * Handle battle end - FUN_004653d0
 */
void battle_on_end(u32 param1, u32 param2, u32 param3, u32 param4, int result) {
    if (g_battle.is_pvp == 0 && result != 0) {
        battle_show_result_animation(param2, param3, param4, result);
    }
}

/*
 * Set battle turn - FUN_00465160
 */
void battle_set_turn(u32 turn) {
    g_battle.turn_count = turn;
    LOG_DEBUG("Battle turn set to %d", turn);
}

/*
 * Handle battle result - FUN_00464ac0
 */
void battle_handle_result(u32 attacker, u32 target, u16 damage, u16 flags, u32 skill_id) {
    if (g_battle.is_pvp) return;
    battle_show_damage(attacker, target, damage, flags, skill_id);
}

/*
 * Handle capture result - FUN_00464db0
 */
void battle_handle_capture(u32 flags, u32 result) {
    if (flags == 0) {
        g_battle.capture_flags = 0;
        g_battle.capture_result[0] = 0;
        g_battle.capture_result[1] = 0;
        g_battle.capture_pet_id = 0;
        battle_clear_capture_ui();
        return;
    }

    if (flags & 1) g_battle.capture_flags = (u16)result;
    if (flags & 2) g_battle.capture_result[0] = (u16)result;
    if (flags & 4) g_battle.capture_result[1] = (u16)result;
    if (flags & 8) g_battle.capture_pet_id = result;
}

/*
 * Handle unit action response
 */
void battle_handle_unit_action_response(u32 unit_index) {
    if (unit_index >= g_battle.unit_count) return;

    BattleUnit* unit = &g_battle.units[unit_index];
    if (unit->action_state == 2) {
        network_send_action_complete(unit->id);
        unit->action_state = 0;
    } else {
        s32 result = unit->action_result;
        if (result == -1) {
            network_send_action_response(unit->id, 0);
        } else if (result == -2) {
            network_send_action_response(unit->id, 1);
        } else if (result == -3) {
            network_send_action_response(unit->id, 2);
        }
    }
}

/*
 * Play animation
 */
void battle_play_animation(u32 animation_id, u32 source_id, u32 target_id) {
    BattleUnit* source = battle_get_unit(source_id);
    BattleUnit* target = battle_get_unit(target_id);

    if (source) {
        source->animation = (u16)animation_id;
        source->frame = 0;
        source->effect_timer = 30;
    }

    if (target && target != source) {
        target->effect_id = (u16)animation_id;
        target->effect_timer = 30;
    }

    g_battle.animation_timer = 30;
    LOG_DEBUG("Play animation %d: %d -> %d", animation_id, source_id, target_id);
}

/* Internal helper functions */

static void battle_show_message(const char* msg, int type) {
    strncpy(g_battle.message, msg, sizeof(g_battle.message) - 1);
    g_battle.message_type = (u8)type;
    g_battle.message_timer = 180;
    LOG_INFO("Battle message: %s", msg);
}

static void battle_show_result_animation(u32 p2, u32 p3, u32 p4, int result) {
    g_battle.result_animation = (u8)result;
    g_battle.result_timer = 120;

    if (result == 1) {
        battle_show_message("Victory!", 1);
    } else {
        battle_show_message("Defeat!", 2);
    }

    LOG_INFO("Battle result: %s", result ? "Victory" : "Defeat");
}

static void battle_show_damage(u32 attacker, u32 target, u16 damage, u16 flags, u32 skill_id) {
    BattleUnit* target_unit = battle_get_unit(target);
    if (!target_unit) return;

    if (g_battle.damage_count < 20) {
        DamageNumber* dmg = &g_battle.damage_numbers[g_battle.damage_count++];
        dmg->value = damage;
        dmg->x = target_unit->x;
        dmg->y = target_unit->y - 20;
        dmg->timer = 60;
        dmg->flags = (u8)flags;
        dmg->is_critical = (flags & 0x01) ? 1 : 0;
        dmg->is_heal = (flags & 0x02) ? 1 : 0;
    }

    LOG_DEBUG("Damage: %d -> %d: %d", attacker, target, damage);
}

static void battle_clear_capture_ui(void) {
    g_battle.capture_flags = 0;
    g_battle.capture_result[0] = 0;
    g_battle.capture_result[1] = 0;
    g_battle.capture_pet_id = 0;
    g_battle.capture_timer = 0;
}
