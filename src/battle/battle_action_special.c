/*
 * Stone Age Client - Battle Action Special Module
 * Reverse engineered from sa_9061.exe
 *
 * Handles special battle actions:
 * - Action 0x0a, 0x0b: Wait/Skip turn
 * - Action 0x0c: Capture pet
 * - Action 0x0d: Battle end
 * - Action 0x19-0x1b: Special combo actions
 * - Action 0x1c: Death
 * - Action 0x1e: Revive
 * - Action 0x20: Transform
 * - Action 0x24-0x27: Target selection
 * - Action 0x29: Defend
 * - Action 0x2a: Escape
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "battle_action.h"
#include "battle.h"
#include "logger.h"

/* External state from core module */
extern BattleActionState g_battle_action;

/* External helper functions */
extern void action_show_effect(u32 effect_id, u32 x, u32 y);
extern void action_play_sound(u32 sound_id);

/*
 * Wait action handler - Action 10, 0x0b
 * Unit skips their turn
 */
void battle_action_wait(const ActionContext* ctx) {
    u32 unit_id = ctx->param1;

    LOG_DEBUG("Wait action: unit=%d", unit_id);

    /* Unit skips turn, no action required */
}

/*
 * Capture action handler - Action 0x0c
 * Based on FUN_004262f0
 *
 * TODO: Verify capture name parsing matches binary:
 * - Field 1 (index 1) goes to DAT_045529ec (capturer name, 26 chars max)
 * - Field 2 (index 2) goes to DAT_0454e000 (pet name, 26 chars max)
 */
void battle_action_capture(const ActionContext* ctx) {
    u32 target_id = ctx->param1;
    BattleUnit* target;
    int capture_chance;

    LOG_DEBUG("Capture action: target=%d", target_id);

    target = battle_get_unit(target_id);

    if (!target || !target->is_alive) {
        LOG_WARN("Invalid capture target");
        return;
    }

    /* Store capture data */
    strncpy(g_battle_action.capture_target, "Enemy", sizeof(g_battle_action.capture_target) - 1);

    /* Calculate capture chance based on HP */
    capture_chance = 100 - (target->hp * 100 / target->max_hp);
    capture_chance = capture_chance / 2;  /* Max 50% */

    /* Attempt capture */
    if ((rand() % 100) < capture_chance) {
        LOG_INFO("Capture successful! Captured enemy %d", target->char_id);

        /* Add to pet list - pet.c handles this */
        /* pet_capture(target->char_id); */

        target->is_alive = 0;
        action_show_effect(5, target->x, target->y);
        action_play_sound(4);
    } else {
        LOG_DEBUG("Capture failed");
        action_play_sound(5);
    }
}

/*
 * Battle end action handler - Action 0x0d
 */
void battle_action_end(const ActionContext* ctx) {
    LOG_DEBUG("Battle end action");

    g_battle_action.action_ended = 1;
    battle_end();
}

/*
 * Special action 1 handler - Action 0x19
 * Combo attack setup
 * Based on FUN_00433700
 */
void battle_action_special1(const ActionContext* ctx) {
    u32 unit_id = ctx->param1;
    BattleUnit* unit = battle_get_unit(unit_id);

    LOG_DEBUG("Special action 1 (combo setup): unit=%d", unit_id);

    if (unit && unit->is_alive) {
        /* Set combo flag */
        g_battle_action.combo_unit = unit_id;
        g_battle.is_combo = 1;
        unit->buff_flags |= BUFF_ATTACK_UP;  /* Combo bonus */

        action_show_effect(7, unit->x, unit->y);
    }
}

/*
 * Special action 2 handler - Action 0x1a
 * Combo follow-up attack
 * Based on FUN_00433fb0
 */
void battle_action_special2(const ActionContext* ctx) {
    u32 unit_id = ctx->param1;
    u32 target_id = ctx->param2;
    BattleUnit* unit = battle_get_unit(unit_id);
    BattleUnit* target = battle_get_unit(target_id);

    LOG_DEBUG("Special action 2 (combo follow): unit=%d -> target=%d", unit_id, target_id);

    if (unit && target && unit->is_alive && target->is_alive) {
        /* Combo attack does extra damage */
        u16 damage = battle_calc_damage(unit, target, NULL);
        damage = damage * 150 / 100;  /* 150% damage */

        target->hp -= damage;
        if (target->hp <= 0) {
            target->hp = 0;
            target->is_alive = 0;
        }

        action_show_effect(1, target->x, target->y);
        action_play_sound(1);

        /* Clear combo state */
        g_battle_action.combo_unit = 0;
        g_battle.is_combo = 0;
    }
}

/*
 * Special action 3 handler - Action 0x1b
 * Counter attack setup
 * Based on FUN_004344c0
 */
void battle_action_special3(const ActionContext* ctx) {
    u32 unit_id = ctx->param1;
    BattleUnit* unit = battle_get_unit(unit_id);

    LOG_DEBUG("Special action 3 (counter setup): unit=%d", unit_id);

    if (unit && unit->is_alive) {
        /* Set counter stance */
        g_battle_action.counter_unit = unit_id;
        unit->buff_flags |= BUFF_DEFENSE_UP;  /* Defense boost while countering */

        action_show_effect(7, unit->x, unit->y);
    }
}

/*
 * Death action handler - Action 0x1c
 */
void battle_action_death(const ActionContext* ctx) {
    u32 unit_id = ctx->param1;
    BattleUnit* unit = battle_get_unit(unit_id);

    LOG_DEBUG("Death action: unit=%d", unit_id);

    if (unit) {
        unit->is_alive = 0;
        unit->hp = 0;
        g_battle_action.death_flag = 1;

        action_show_effect(3, unit->x, unit->y);
        action_play_sound(8);
    }
}

/*
 * Revive action handler - Action 0x1e
 * Based on FUN_00435450
 */
void battle_action_revive(const ActionContext* ctx) {
    u32 unit_id = ctx->param1;
    u32 hp_restore = 50;  /* Default HP restore */
    BattleUnit* unit = battle_get_unit(unit_id);

    LOG_DEBUG("Revive action: unit=%d", unit_id);

    if (unit) {
        unit->is_alive = 1;
        unit->hp = hp_restore;
        if (unit->hp > unit->max_hp) {
            unit->hp = unit->max_hp;
        }

        action_show_effect(4, unit->x, unit->y);
        action_play_sound(9);

        LOG_INFO("Revived unit %d with %d HP", unit_id, unit->hp);
    }
}

/*
 * Transform action handler - Action 0x20
 * Based on FUN_00435a00
 */
void battle_action_transform(const ActionContext* ctx) {
    u32 unit_id = ctx->param1;
    u32 new_sprite = ctx->param2;
    BattleUnit* unit = battle_get_unit(unit_id);

    LOG_DEBUG("Transform action: unit=%d, sprite=%d", unit_id, new_sprite);

    if (unit) {
        unit->sprite_id = new_sprite;
        action_show_effect(5, unit->x, unit->y);
        action_play_sound(2);
    }
}

/*
 * Target selection handler - Action 0x24, 0x25, 0x26, 0x27
 */
void battle_action_target_select(const ActionContext* ctx) {
    u32 unit_id = ctx->param1;
    u32 target_id = ctx->param2;

    LOG_DEBUG("Target select: unit=%d, target=%d, type=0x%02x",
              unit_id, target_id, ctx->action_type);

    /* Update unit's target */
    BattleUnit* unit = battle_get_unit(unit_id);
    if (unit) {
        unit->target_id = target_id;
    }
}

/*
 * Defend action handler - Action 0x29
 * Based on FUN_00425b90
 */
void battle_action_defend(const ActionContext* ctx) {
    u32 unit_id = ctx->param1;
    BattleUnit* unit = battle_get_unit(unit_id);

    LOG_DEBUG("Defend action: unit=%d", unit_id);

    if (unit && unit->is_alive) {
        /* Set defense buff for this turn */
        unit->buff_flags |= BUFF_DEFENSE_UP;
        unit->buff_turns[0] = 1;

        action_show_effect(7, unit->x, unit->y);
    }
}

/*
 * Escape action handler - Action 0x2a
 * Based on FUN_0042af40
 *
 * TODO: Verify escape logic matches binary:
 * - DAT_0455b370 should be 0x50 (80) max escape count
 * - Parse escape message format: field1|player|result|...
 */
void battle_action_escape(const ActionContext* ctx) {
    int escape_chance;
    int escape_result;

    LOG_DEBUG("Escape action");

    g_battle_action.escape_attempt++;

    /* Calculate escape chance */
    escape_chance = 50;  /* Base 50% */

    /* Reduce chance with each failed attempt */
    escape_chance -= g_battle_action.escape_count * 10;

    /* Cap at escape_max */
    if (g_battle_action.escape_attempt > g_battle_action.escape_max) {
        escape_chance = 0;
    }

    /* Roll for escape */
    escape_result = (rand() % 100) < escape_chance;

    if (escape_result) {
        LOG_INFO("Escape successful!");
        g_battle_action.action_ended = 1;
        /* Signal battle end - escaped */
        battle_end();
    } else {
        LOG_DEBUG("Escape failed (attempt %d)", g_battle_action.escape_attempt);
        g_battle_action.escape_count++;
    }
}
