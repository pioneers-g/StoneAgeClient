/*
 * Stone Age Client - Item Effect Implementation
 * Item usage, effects, and stat bonuses
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "item.h"
#include "character.h"
#include "logger.h"

/* Forward declarations */
Item* item_get(u32 item_id);
int inventory_remove_item(u32 item_id, u16 count);

/*
 * Use item
 */
int item_use(u32 item_id, void* target) {
    Item* item = item_get(item_id);

    if (!item) return 0;
    if (!item_can_use(item_id)) return 0;

    /* Apply effect */
    item_apply_effect(item_id, target);

    /* Remove from inventory if consumable */
    if (item->type == ITEM_TYPE_CONSUMABLE) {
        inventory_remove_item(item_id, 1);
    }

    return 1;
}

/*
 * Check if item can be used
 */
int item_can_use(u32 item_id) {
    Item* item = item_get(item_id);
    if (!item) return 0;
    return item->type == ITEM_TYPE_CONSUMABLE || item->type == ITEM_TYPE_PET_ITEM;
}

/*
 * Apply item effect
 */
int item_apply_effect(u32 item_id, void* target) {
    Item* item = item_get(item_id);
    Character* ch = (Character*)target;

    if (!item || !ch) return 0;

    switch (item->effect_type) {
        case EFFECT_HEAL_HP:
            ch->stats.hp += item->effect_value;
            if (ch->stats.hp > ch->stats.max_hp) {
                ch->stats.hp = ch->stats.max_hp;
            }
            LOG_DEBUG("Healed %d HP", item->effect_value);
            break;

        case EFFECT_HEAL_MP:
            ch->stats.mp += item->effect_value;
            if (ch->stats.mp > ch->stats.max_mp) {
                ch->stats.mp = ch->stats.max_mp;
            }
            LOG_DEBUG("Restored %d MP", item->effect_value);
            break;

        case EFFECT_BUFF_ATTACK:
            /* Apply temporary buff */
            ch->buff_flags |= BUFF_ATTACK_UP;
            break;

        case EFFECT_CURE_POISON:
            ch->buff_flags &= ~BUFF_POISON;
            break;

        default:
            break;
    }

    return 1;
}

/*
 * Apply item stat bonus
 */
int item_apply_stat_bonus(Item* item, void* character) {
    Character* ch = (Character*)character;

    if (!item || !ch) return 0;

    ch->stats.attack += item->attack;
    ch->stats.defense += item->defense;
    ch->stats.magic_attack += item->magic_attack;
    ch->stats.magic_defense += item->magic_defense;
    ch->stats.agility += item->agility;
    ch->stats.max_hp += item->hp_bonus;
    ch->stats.max_mp += item->mp_bonus;

    return 1;
}

/*
 * Remove item stat bonus
 */
int item_remove_stat_bonus(Item* item, void* character) {
    Character* ch = (Character*)character;

    if (!item || !ch) return 0;

    ch->stats.attack -= item->attack;
    ch->stats.defense -= item->defense;
    ch->stats.magic_attack -= item->magic_attack;
    ch->stats.magic_defense -= item->magic_defense;
    ch->stats.agility -= item->agility;
    ch->stats.max_hp -= item->hp_bonus;
    ch->stats.max_mp -= item->mp_bonus;

    return 1;
}
