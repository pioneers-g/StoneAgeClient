/*
 * Stone Age Client - Battle Action Item Module
 * Reverse engineered from sa_9061.exe (FUN_00425bb0)
 *
 * Handles item usage in battle:
 * - Action 9: Use consumable item
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
 * Item usage handler - Action 9
 * Based on FUN_00425bb0
 *
 * Item ID ranges:
 * - 3000-3999: HP restore items (potions)
 * - 4000-4999: MP restore items
 * - 5000-5999: Buff items
 * - 6000-6999: Special items
 */
void battle_action_item(const ActionContext* ctx) {
    u32 user_id = ctx->param1;
    u32 item_id = 0;
    u32 target_id = ctx->param2;
    BattleUnit* target;

    /* Get item ID from parsed message */
    if (ctx->parse_count > 0) {
        item_id = ctx->parsed_int[0];
    }

    LOG_DEBUG("Item action: user=%d, item=%d, target=%d", user_id, item_id, target_id);

    target = battle_get_unit(target_id);
    if (!target) {
        target = battle_get_unit(user_id);  /* Self-target */
    }

    if (!target || !target->is_alive) {
        LOG_WARN("Invalid target for item");
        return;
    }

    /* Apply item effect based on item type */
    if (item_id >= 3000 && item_id < 4000) {
        /* HP restore items (potions) */
        u16 heal_amount = 50;  /* Default heal */

        /* Different heal amounts by item ID range */
        if (item_id >= 3100 && item_id < 3200) {
            heal_amount = 100;  /* Medium potion */
        } else if (item_id >= 3200 && item_id < 3300) {
            heal_amount = 200;  /* Large potion */
        } else if (item_id >= 3300 && item_id < 3400) {
            heal_amount = target->max_hp;  /* Full restore */
        } else if (item_id >= 3400 && item_id < 3500) {
            heal_amount = target->max_hp / 2;  /* Half restore */
        }

        /* Apply healing */
        target->hp += heal_amount;
        if (target->hp > target->max_hp) {
            target->hp = target->max_hp;
        }

        action_show_effect(2, target->x, target->y);
        LOG_DEBUG("Healed %d HP to unit %d", heal_amount, target->id);
    }
    else if (item_id >= 4000 && item_id < 5000) {
        /* MP restore items */
        u16 mp_restore = 30;  /* Default MP restore */

        if (item_id >= 4100 && item_id < 4200) {
            mp_restore = 60;
        } else if (item_id >= 4200 && item_id < 4300) {
            mp_restore = 100;
        } else if (item_id >= 4300 && item_id < 4400) {
            mp_restore = target->max_mp;  /* Full MP */
        }

        target->mp += mp_restore;
        if (target->mp > target->max_mp) {
            target->mp = target->max_mp;
        }

        action_show_effect(2, target->x, target->y);
        LOG_DEBUG("Restored %d MP to unit %d", mp_restore, target->id);
    }
    else if (item_id >= 5000 && item_id < 6000) {
        /* Buff items */
        target->buff_flags |= BUFF_ATTACK_UP;
        target->buff_turns[0] = 3;

        action_show_effect(7, target->x, target->y);
        LOG_DEBUG("Applied buff to unit %d", target->id);
    }
    else if (item_id >= 6000 && item_id < 7000) {
        /* Special items (antidote, awaken, etc.) */
        /* Clear negative status */
        target->buff_flags &= ~(BUFF_POISON | BUFF_SLEEP | BUFF_CONFUSE);

        action_show_effect(2, target->x, target->y);
        LOG_DEBUG("Cleared status from unit %d", target->id);
    }

    action_play_sound(3);
}
