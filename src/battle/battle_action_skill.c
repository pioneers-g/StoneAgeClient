/*
 * Stone Age Client - Battle Action Skill Module
 * Reverse engineered from sa_9061.exe (FUN_004253d0, FUN_004364e0)
 *
 * Handles skill-related battle actions:
 * - Action 2: Generic skill usage
 * - Action 0x12-0x15: Skill type variations
 * - Action 0x1f: Berserk skill
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "battle_action.h"
#include "battle.h"
#include "character.h"
#include "logger.h"

/* External state from core module */
extern BattleActionState g_battle_action;

/* External helper functions */
extern void action_show_effect(u32 effect_id, u32 x, u32 y);
extern void action_play_sound(u32 sound_id);

/* Forward declarations */
static void action_handle_skill_internal(const ActionContext* ctx, u32 skill_type);

/*
 * Skill action handler - Action 2
 * Based on FUN_004253d0
 */
void battle_action_skill(const ActionContext* ctx) {
    u32 attacker_id = ctx->param1;
    u32 target_id = ctx->param2;
    u32 skill_id = 0;
    BattleUnit* attacker;
    BattleUnit* target;

    /* Get skill ID from parsed message or param3 */
    if (ctx->parse_count > 0) {
        skill_id = ctx->parsed_int[0];
    } else {
        skill_id = ctx->param3;
    }

    LOG_DEBUG("Skill action: attacker=%d, target=%d, skill=%d", attacker_id, target_id, skill_id);

    g_battle_action.skill_type = skill_id;

    attacker = battle_get_unit(attacker_id);
    target = battle_get_unit(target_id);

    if (!attacker) {
        LOG_WARN("Invalid attacker for skill");
        return;
    }

    /* Use skill */
    if (target && target->is_alive) {
        battle_use_skill(attacker, target, (u16)skill_id);

        /* Show skill effect - different effects by skill type */
        u32 effect_id = 2;  /* Default skill effect */
        if (skill_id >= 100 && skill_id < 200) {
            effect_id = 5;  /* Attack skill */
        } else if (skill_id >= 200 && skill_id < 300) {
            effect_id = 6;  /* Heal skill */
        } else if (skill_id >= 300 && skill_id < 400) {
            effect_id = 7;  /* Buff skill */
        }
        action_show_effect(effect_id, target->x, target->y);
        action_play_sound(2);
    }
}

/*
 * Berserk action handler - Action 0x1f
 * Based on FUN_004364e0
 * Increases attack but reduces defense
 */
void battle_action_berserk(const ActionContext* ctx) {
    u32 unit_id = ctx->param1;
    BattleUnit* unit = battle_get_unit(unit_id);

    LOG_DEBUG("Berserk action: unit=%d", unit_id);

    if (unit && unit->is_alive) {
        /* Berserk increases attack but reduces defense */
        unit->buff_flags |= BUFF_ATTACK_UP | BUFF_DEFENSE_DOWN;
        unit->buff_turns[1] = 3;  /* Attack up for 3 turns */
        unit->buff_turns[2] = 3;  /* Defense down for 3 turns */

        action_show_effect(7, unit->x, unit->y);
        action_play_sound(2);

        LOG_DEBUG("Unit %d enters berserk state", unit_id);
    }
}

/*
 * Skill type 1 handler - Action 0x12
 * Based on FUN_00430700
 */
void battle_action_skill_type1(const ActionContext* ctx) {
    LOG_DEBUG("Skill type 1 action (0x12)");
    action_handle_skill_internal(ctx, 1);
}

/*
 * Skill type 2 handler - Action 0x13
 * Based on FUN_00431390
 */
void battle_action_skill_type2(const ActionContext* ctx) {
    LOG_DEBUG("Skill type 2 action (0x13)");
    action_handle_skill_internal(ctx, 2);
}

/*
 * Skill type 3 handler - Action 0x14
 * Based on FUN_00431390
 */
void battle_action_skill_type3(const ActionContext* ctx) {
    LOG_DEBUG("Skill type 3 action (0x14)");
    action_handle_skill_internal(ctx, 3);
}

/*
 * Skill type 4 handler - Action 0x15
 * Based on FUN_00431ad0
 */
void battle_action_skill_type4(const ActionContext* ctx) {
    LOG_DEBUG("Skill type 4 action (0x15)");
    action_handle_skill_internal(ctx, 4);
}

/*
 * Internal skill handler
 * Routes to main skill handler with skill type
 */
static void action_handle_skill_internal(const ActionContext* ctx, u32 skill_type) {
    g_battle_action.skill_type = skill_type;

    /* Route to main skill handler */
    battle_action_skill(ctx);
}
