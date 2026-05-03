/*
 * Stone Age Client - Item Effect Interface
 */

#ifndef ITEM_EFFECT_H
#define ITEM_EFFECT_H

#include "types.h"
#include "item.h"

/* ========================================
 * Item Usage Functions
 * ======================================== */

int item_use(u32 item_id, void* target);
int item_can_use(u32 item_id);

/* ========================================
 * Item Effect Functions
 * ======================================== */

int item_apply_effect(u32 item_id, void* target);
int item_apply_stat_bonus(Item* item, void* character);
int item_remove_stat_bonus(Item* item, void* character);

#endif /* ITEM_EFFECT_H */
