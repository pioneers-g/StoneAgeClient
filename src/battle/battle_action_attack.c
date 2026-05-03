/*
 * Stone Age Client - Battle Action Attack Module
 * Reverse engineered from sa_9061.exe (FUN_00425380, FUN_00425420, FUN_004254e0)
 *
 * Handles attack-related battle actions:
 * - Action 0, 1, 0x2d: Basic attacks
 * - Action 6: Counter attack
 * - Action 7, 8: Combo attacks
 * - Action 0x22: Chain attack
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
static void action_handle_attack_internal(const ActionContext* ctx, int is_counter);

/*
 * Attack action handler - Action 0, 1, 0x2d
 * Based on FUN_00425380
 *
 * TODO: Verify attack state values match binary:
 * - DAT_045541dc should be 7
 * - DAT_045541a4 should be 5
 * - DAT_04552fb0 should be 0x28 (40)
 */
void battle_action_attack(const ActionContext* ctx) {
    u32 attacker_id = ctx->param1;
    u32 target_id = ctx->param2;
    u16 damage = 0;
    BattleUnit* attacker;
    BattleUnit* target;

    LOG_DEBUG("Attack action: attacker=%d, target=%d", attacker_id, target_id);

    attacker = battle_get_unit(attacker_id);
    target = battle_get_unit(target_id);

    if (!attacker || !target) {
        LOG_WARN("Invalid attacker or target for attack");
        return;
    }

    /* Calculate and apply damage */
    if (target->is_alive) {
        damage = battle_calc_damage(attacker, target, NULL);
        target->hp -= damage;

        /* Show damage effect */
        action_show_effect(1, target->x, target->y);
        action_play_sound(1);

        /* Check for death */
        if (target->hp <= 0) {
            target->hp = 0;
            target->is_alive = 0;
            g_battle_action.death_flag = 1;
        }

        LOG_DEBUG("Attack dealt %d damage to target %d", damage, target_id);
    }
}

/*
 * Counter attack handler - Action 6
 * Based on FUN_00425420
 */
void battle_action_counter(const ActionContext* ctx) {
    LOG_DEBUG("Counter attack action");

    /* Counter uses same logic as attack but triggered by being attacked */
    action_handle_attack_internal(ctx, 1);
}

/*
 * Combo attack handler - Action 7, 8
 * Based on FUN_004254e0
 *
 * TODO: Verify combo flag behavior from binary:
 * - Action 0x08 should set DAT_0454f108 = 1
 * - Action 0x07 should NOT set this flag
 * - Need to determine what DAT_0454f108 controls
 */
void battle_action_combo(const ActionContext* ctx) {
    g_battle_action.is_combo = 1;

    LOG_DEBUG("Combo attack action: type=%d", ctx->action_type);

    /* Combo attack follows previous attack */
    action_handle_attack_internal(ctx, 0);

    /* For action 0x08, set combo follow-up flag */
    if (ctx->action_type == 0x08) {
        /* DAT_0454f108 = 1 in original binary */
    }

    g_battle_action.is_combo = 0;
}

/*
 * Chain attack handler - Action 0x22
 * Based on FUN_00436190
 */
void battle_action_chain(const ActionContext* ctx) {
    LOG_DEBUG("Chain attack action");

    g_battle_action.is_combo = 1;
    action_handle_attack_internal(ctx, 0);
    g_battle_action.is_combo = 0;
}

/*
 * Internal attack handler
 * Shared logic for all attack variants
 */
static void action_handle_attack_internal(const ActionContext* ctx, int is_counter) {
    u32 attacker_id = ctx->param1;
    u32 target_id = ctx->param2;
    BattleUnit* attacker;
    BattleUnit* target;
    u16 damage;

    attacker = battle_get_unit(attacker_id);
    target = battle_get_unit(target_id);

    if (!attacker || !target || !target->is_alive) {
        return;
    }

    /* Calculate damage */
    damage = battle_calc_damage(attacker, target, NULL);

    /* Counter attacks do reduced damage (70%) */
    if (is_counter) {
        damage = damage * 7 / 10;
    }

    /* Combo attacks do bonus damage (120%) */
    if (g_battle_action.is_combo) {
        damage = damage * 12 / 10;
    }

    /* Apply damage */
    target->hp -= damage;
    if (target->hp <= 0) {
        target->hp = 0;
        target->is_alive = 0;
        g_battle_action.death_flag = 1;
    }

    /* Show effects */
    action_show_effect(1, target->x, target->y);
    action_play_sound(1);

    LOG_DEBUG("Attack: %d damage to unit %d (counter=%d, combo=%d)",
              damage, target_id, is_counter, g_battle_action.is_combo);
}
