/*
 * Stone Age Client - Battle Action Pet Module
 * Reverse engineered from sa_9061.exe (FUN_0042e870, FUN_0042f130, FUN_004327b0, FUN_00432ec0)
 *
 * Handles pet-related battle actions:
 * - Action 0x0e: Pet attack
 * - Action 0x0f: Pet skill
 * - Action 0x16, 0x2c: Summon pet
 * - Action 0x17, 0x2b: Recall pet
 * - Action 0x67: Pet escape
 * - Action 0x68: Pet swap
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "battle_action.h"
#include "battle.h"
#include "pet.h"
#include "logger.h"

/* External state from core module */
extern BattleActionState g_battle_action;

/* External helper functions */
extern void action_show_effect(u32 effect_id, u32 x, u32 y);
extern void action_play_sound(u32 sound_id);

/* Forward declarations */
static void action_handle_attack_internal(const ActionContext* ctx, int is_counter);

/*
 * Pet attack handler - Action 0x0e
 * Based on FUN_0042e870
 */
void battle_action_pet_attack(const ActionContext* ctx) {
    u32 pet_id = ctx->param1;
    u32 target_id = ctx->param2;

    LOG_DEBUG("Pet attack: pet=%d, target=%d", pet_id, target_id);

    /* Pet uses normal attack logic */
    action_handle_attack_internal(ctx, 0);
}

/*
 * Pet skill handler - Action 0x0f
 * Based on FUN_0042f130
 */
void battle_action_pet_skill(const ActionContext* ctx) {
    u32 pet_id = ctx->param1;
    u32 skill_id = 0;

    if (ctx->parse_count > 0) {
        skill_id = ctx->parsed_int[0];
    }

    LOG_DEBUG("Pet skill: pet=%d, skill=%d", pet_id, skill_id);

    /* Pet uses skill logic - route to skill handler */
    {
        ActionContext skill_ctx = *ctx;
        skill_ctx.param1 = pet_id;
        battle_action_skill(&skill_ctx);
    }
}

/*
 * Summon pet handler - Action 0x16, 0x2c
 * Based on FUN_004327b0
 */
void battle_action_summon(const ActionContext* ctx) {
    u32 pet_slot = ctx->param1;
    PetData* pet;
    BattleUnit unit;

    LOG_DEBUG("Summon pet: slot=%d", pet_slot);

    pet = pet_get_slot((int)pet_slot);
    if (!pet) {
        LOG_WARN("No pet in slot %d", pet_slot);
        return;
    }

    /* Create battle unit for pet */
    memset(&unit, 0, sizeof(BattleUnit));
    unit.id = 5000 + pet_slot;  /* Pet ID offset */
    unit.char_id = pet->id;
    unit.sprite_id = pet->sprite_id;
    unit.side = 0;  /* Player side */
    unit.is_pet = 1;
    unit.is_alive = 1;
    unit.is_active = 1;
    unit.level = pet->level;
    unit.hp = pet->hp;
    unit.max_hp = pet->max_hp;
    unit.attack = pet->attack;
    unit.defense = pet->defense;
    unit.speed = pet->speed;

    /* Position behind player */
    {
        BattleUnit* player = battle_get_player_unit();
        if (player) {
            unit.x = player->x - 40;
            unit.y = player->y + 20;
        }
    }

    battle_add_unit(&unit, 0);
    g_battle_action.summoned_pet_slot = (u8)pet_slot;

    action_show_effect(6, unit.x, unit.y);
    action_play_sound(6);

    LOG_INFO("Summoned pet from slot %d (ID: %d)", pet_slot, pet->id);
}

/*
 * Recall pet handler - Action 0x17, 0x2b
 * Based on FUN_00432ec0
 */
void battle_action_recall(const ActionContext* ctx) {
    u32 pet_slot = g_battle_action.summoned_pet_slot;
    u32 i;

    LOG_DEBUG("Recall pet from slot %d", pet_slot);

    if (pet_slot == 0xFF) {
        LOG_WARN("No pet currently summoned");
        return;
    }

    /* Find and remove pet battle unit */
    for (i = 0; i < g_battle.unit_count; i++) {
        if (g_battle.units[i].is_pet && g_battle.units[i].id >= 5000) {
            action_show_effect(7, g_battle.units[i].x, g_battle.units[i].y);

            g_battle.units[i].is_alive = 0;
            g_battle.units[i].is_active = 0;
            break;
        }
    }

    g_battle_action.summoned_pet_slot = 0xFF;  /* No pet summoned */
    action_play_sound(7);

    LOG_INFO("Recalled pet from battle");
}

/*
 * Pet escape handler - Action 0x67
 * Pet flees from battle
 * Based on FUN_00439810
 */
void battle_action_pet_escape(const ActionContext* ctx) {
    u32 pet_id = ctx->param1;
    u32 i;

    LOG_DEBUG("Pet escape action: pet=%d", pet_id);

    /* Find pet unit in battle */
    for (i = 0; i < g_battle.unit_count; i++) {
        if (g_battle.units[i].is_pet &&
            (g_battle.units[i].id == pet_id || g_battle.units[i].char_id == pet_id)) {

            /* Remove pet from battle */
            action_show_effect(7, g_battle.units[i].x, g_battle.units[i].y);

            g_battle.units[i].is_alive = 0;
            g_battle.units[i].is_active = 0;

            LOG_INFO("Pet %d escaped from battle", pet_id);
            break;
        }
    }

    g_battle_action.summoned_pet_slot = 0xFF;
    action_play_sound(7);
}

/*
 * Pet swap handler - Action 0x68
 * Swap active pet in battle
 * Based on FUN_0043a020
 */
void battle_action_pet_swap(const ActionContext* ctx) {
    u32 old_pet_slot = ctx->param1;
    u32 new_pet_slot = ctx->param2;
    PetData* new_pet;
    u32 i;

    LOG_DEBUG("Pet swap: old_slot=%d, new_slot=%d", old_pet_slot, new_pet_slot);

    /* Remove old pet from battle */
    for (i = 0; i < g_battle.unit_count; i++) {
        if (g_battle.units[i].is_pet && g_battle.units[i].id >= 5000) {
            action_show_effect(7, g_battle.units[i].x, g_battle.units[i].y);
            g_battle.units[i].is_alive = 0;
            g_battle.units[i].is_active = 0;
            break;
        }
    }

    /* Summon new pet */
    new_pet = pet_get_slot((int)new_pet_slot);
    if (new_pet) {
        BattleUnit unit;
        memset(&unit, 0, sizeof(BattleUnit));
        unit.id = 5000 + new_pet_slot;
        unit.char_id = new_pet->id;
        unit.sprite_id = new_pet->sprite_id;
        unit.side = 0;
        unit.is_pet = 1;
        unit.is_alive = 1;
        unit.is_active = 1;
        unit.level = new_pet->level;
        unit.hp = new_pet->hp;
        unit.max_hp = new_pet->max_hp;
        unit.attack = new_pet->attack;
        unit.defense = new_pet->defense;
        unit.speed = new_pet->speed;

        battle_add_unit(&unit, 0);
        g_battle_action.summoned_pet_slot = (u8)new_pet_slot;

        action_show_effect(6, unit.x, unit.y);
        action_play_sound(6);

        LOG_INFO("Swapped to pet in slot %d (ID: %d)", new_pet_slot, new_pet->id);
    }
}

/*
 * Internal attack handler (local copy for pet module)
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

    /* Counter attacks do reduced damage */
    if (is_counter) {
        damage = damage * 7 / 10;
    }

    /* Apply damage */
    target->hp -= damage;
    if (target->hp <= 0) {
        target->hp = 0;
        target->is_alive = 0;
        g_battle_action.death_flag = 1;
    }

    action_show_effect(1, target->x, target->y);
    action_play_sound(1);
}
