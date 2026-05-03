/*
 * Stone Age Client - Battle Action Core Module
 * Reverse engineered from sa_9061.exe (FUN_00424b70)
 *
 * Core dispatcher, initialization, message parsing, and utility functions.
 * This file is part of the battle_action module split.
 *
 * Related files:
 * - battle_action_attack.c - Attack actions
 * - battle_action_skill.c - Skill actions
 * - battle_action_item.c - Item actions
 * - battle_action_pet.c - Pet actions
 * - battle_action_special.c - Special actions
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "battle_action.h"
#include "battle.h"
#include "logger.h"

/* Global battle action state - matches DAT_004b83ec region */
BattleActionState g_battle_action = {0};

/* External handlers from other modules */
extern void battle_action_attack(const ActionContext* ctx);
extern void battle_action_skill(const ActionContext* ctx);
extern void battle_action_counter(const ActionContext* ctx);
extern void battle_action_combo(const ActionContext* ctx);
extern void battle_action_item(const ActionContext* ctx);
extern void battle_action_wait(const ActionContext* ctx);
extern void battle_action_capture(const ActionContext* ctx);
extern void battle_action_end(const ActionContext* ctx);
extern void battle_action_pet_attack(const ActionContext* ctx);
extern void battle_action_pet_skill(const ActionContext* ctx);
extern void battle_action_berserk(const ActionContext* ctx);
extern void battle_action_skill_type1(const ActionContext* ctx);
extern void battle_action_skill_type2(const ActionContext* ctx);
extern void battle_action_skill_type3(const ActionContext* ctx);
extern void battle_action_skill_type4(const ActionContext* ctx);
extern void battle_action_summon(const ActionContext* ctx);
extern void battle_action_recall(const ActionContext* ctx);
extern void battle_action_special1(const ActionContext* ctx);
extern void battle_action_special2(const ActionContext* ctx);
extern void battle_action_special3(const ActionContext* ctx);
extern void battle_action_death(const ActionContext* ctx);
extern void battle_action_revive(const ActionContext* ctx);
extern void battle_action_transform(const ActionContext* ctx);
extern void battle_action_chain(const ActionContext* ctx);
extern void battle_action_target_select(const ActionContext* ctx);
extern void battle_action_defend(const ActionContext* ctx);
extern void battle_action_escape(const ActionContext* ctx);
extern void battle_action_pet_escape(const ActionContext* ctx);
extern void battle_action_pet_swap(const ActionContext* ctx);

/*
 * Initialize battle action system
 */
int battle_action_init(void) {
    memset(&g_battle_action, 0, sizeof(BattleActionState));
    g_battle_action.escape_max = 80;  /* DAT_0455b370 = 0x50 */
    g_battle_action.summoned_pet_slot = 0xFF;  /* No pet summoned */

    LOG_INFO("Battle action system initialized");
    return 1;
}

/*
 * Shutdown battle action system
 */
void battle_action_shutdown(void) {
    memset(&g_battle_action, 0, sizeof(BattleActionState));
    LOG_INFO("Battle action system shutdown");
}

/*
 * Main action dispatcher - FUN_00424b70
 * Dispatches battle actions based on action type
 */
void battle_action_dispatch(u32 action, u32 param1, u32 param2, u32 param3, const char* message) {
    ActionContext ctx = {0};

    /* Store action parameters */
    g_battle_action.current_action = action;
    g_battle_action.action_param1 = param1;
    g_battle_action.action_param2 = param2;
    g_battle_action.action_param3 = param3;

    /* Build context */
    ctx.action_type = action;
    ctx.param1 = param1;
    ctx.param2 = param2;
    ctx.param3 = param3;
    ctx.message = message;

    /* Parse message if provided - FUN_00489f70 pattern */
    if (message && message[0] != '\0') {
        battle_action_parse_message(message, &ctx);
        strncpy(g_battle_action.message_buffer, message, sizeof(g_battle_action.message_buffer) - 1);
    }

    LOG_DEBUG("Battle action dispatch: type=0x%02x, p1=%d, p2=%d, p3=%d",
              action, param1, param2, param3);

    /* Main dispatcher switch - matches FUN_00424b70 */
    switch (action) {
        /* Normal attack actions */
        case 0x00:  /* Basic attack */
        case 0x01:  /* Attack variant */
        case 0x2d:  /* Special attack */
            battle_action_attack(&ctx);
            break;

        /* Skill usage */
        case 0x02:
            battle_action_skill(&ctx);
            break;

        /* Counter attack */
        case 0x06:
            battle_action_counter(&ctx);
            break;

        /* Combo attacks */
        case 0x07:
        case 0x08:
            battle_action_combo(&ctx);
            break;

        /* Item usage */
        case 0x09:
            battle_action_item(&ctx);
            break;

        /* Wait/Skip turn */
        case 0x0a:
        case 0x0b:
            battle_action_wait(&ctx);
            break;

        /* Capture pet */
        case 0x0c:
            battle_action_capture(&ctx);
            break;

        /* Battle end */
        case 0x0d:
            battle_action_end(&ctx);
            break;

        /* Pet attack */
        case 0x0e:
            battle_action_pet_attack(&ctx);
            break;

        /* Pet skill */
        case 0x0f:
            battle_action_pet_skill(&ctx);
            break;

        /* Skill type variations */
        case 0x12:
            battle_action_skill_type1(&ctx);
            break;

        case 0x13:
            battle_action_skill_type2(&ctx);
            break;

        case 0x14:
            battle_action_skill_type3(&ctx);
            break;

        case 0x15:
            battle_action_skill_type4(&ctx);
            break;

        /* Summon pet */
        case 0x16:
        case 0x2c:
            battle_action_summon(&ctx);
            break;

        /* Recall pet */
        case 0x17:
        case 0x2b:
            battle_action_recall(&ctx);
            break;

        /* Special actions */
        case 0x19:
            battle_action_special1(&ctx);
            break;

        case 0x1a:
            battle_action_special2(&ctx);
            break;

        case 0x1b:
            battle_action_special3(&ctx);
            break;

        /* Death */
        case 0x1c:
            battle_action_death(&ctx);
            break;

        /* Revive */
        case 0x1e:
            battle_action_revive(&ctx);
            break;

        /* Berserk */
        case 0x1f:
            battle_action_berserk(&ctx);
            break;

        /* Transform */
        case 0x20:
            battle_action_transform(&ctx);
            break;

        /* Chain attack */
        case 0x22:
            battle_action_chain(&ctx);
            break;

        /* Target selection */
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
            battle_action_target_select(&ctx);
            break;

        /* Defend */
        case 0x29:
            battle_action_defend(&ctx);
            break;

        /* Escape */
        case 0x2a:
            battle_action_escape(&ctx);
            break;

        /* Pet escape */
        case 0x67:
            battle_action_pet_escape(&ctx);
            break;

        /* Pet swap */
        case 0x68:
            battle_action_pet_swap(&ctx);
            break;

        default:
            LOG_WARN("Unknown battle action type: 0x%02x", action);
            break;
    }

    /* Clear action state */
    g_battle_action.action_ended = 1;
}

/*
 * Parse pipe-delimited message - FUN_00489f70 pattern
 * Parses "val1|val2|val3|..." format
 */
int battle_action_parse_message(const char* message, ActionContext* ctx) {
    char buffer[512];
    char* token;
    char* saveptr = NULL;
    int count = 0;

    if (!message || !ctx) return 0;

    strncpy(buffer, message, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    ctx->parse_count = 0;

    /* Parse pipe-delimited values */
#ifdef _WIN32
    token = strtok_s(buffer, "|", &saveptr);
#else
    token = strtok_r(buffer, "|", &saveptr);
#endif

    while (token && count < 16) {
        /* Trim whitespace */
        while (*token == ' ') token++;

        /* Try to parse as integer */
        if (token[0] >= '0' && token[0] <= '9') {
            ctx->parsed_int[count] = atoi(token);
        } else if (token[0] == '-' && token[1] >= '0' && token[1] <= '9') {
            ctx->parsed_int[count] = atoi(token);
        } else {
            ctx->parsed_int[count] = 0;
        }

        /* Store as string if slots available */
        if (count < 8) {
            strncpy(ctx->parsed_str[count], token, sizeof(ctx->parsed_str[0]) - 1);
            ctx->parsed_str[count][sizeof(ctx->parsed_str[0]) - 1] = '\0';
        }

        count++;
#ifdef _WIN32
        token = strtok_s(NULL, "|", &saveptr);
#else
        token = strtok_r(NULL, "|", &saveptr);
#endif
    }

    ctx->parse_count = count;
    return count;
}

/*
 * Set player position during action
 */
void battle_action_set_player_pos(u16 x, u16 y) {
    g_battle_action.player_x = x;
    g_battle_action.player_y = y;
}

/*
 * Clear action state
 */
void battle_action_clear_state(void) {
    g_battle_action.current_action = 0;
    g_battle_action.action_param1 = 0;
    g_battle_action.action_param2 = 0;
    g_battle_action.action_param3 = 0;
    g_battle_action.is_combo = 0;
    g_battle_action.action_ended = 0;
    g_battle_action.death_flag = 0;
    g_battle_action.escape_attempt = 0;
    g_battle_action.message_buffer[0] = '\0';
}

/*
 * Check if action is active
 */
int battle_action_is_active(void) {
    return g_battle_action.current_action != 0 && !g_battle_action.action_ended;
}

/*
 * Get current action type
 */
u32 battle_action_get_current(void) {
    return g_battle_action.current_action;
}

/*
 * Get current message
 */
const char* battle_action_get_message(void) {
    return g_battle_action.message_buffer;
}

/*
 * Check if current action is combo
 */
int battle_action_is_combo(void) {
    return g_battle_action.is_combo;
}

/*
 * Check if player position is close to action position - FUN_00424f50 position check
 */
int battle_action_check_position(void) {
    /* Position validation logic */
    return 1;
}
