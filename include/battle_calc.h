/*
 * Stone Age Client - Battle Calculations Header
 * Split from battle.h for code organization
 */

#ifndef BATTLE_CALC_H
#define BATTLE_CALC_H

#include "types.h"

/* Forward declarations to avoid circular dependency */
struct BattleUnit;
struct BattleSkill;

/* Damage calculation */
u16 battle_calc_damage(struct BattleUnit* attacker, struct BattleUnit* target, struct BattleSkill* skill);
u16 battle_calc_skill_damage(struct BattleUnit* attacker, struct BattleUnit* target, struct BattleSkill* skill);

/* Hit and critical calculation */
int battle_calc_hit(struct BattleUnit* attacker, struct BattleUnit* target);
int battle_calc_critical(struct BattleUnit* attacker, struct BattleUnit* target);

/* Capture rate calculation */
int battle_calc_capture_rate(int enemy_hp_percent, int enemy_level, int capture_item_id);

/* Escape chance calculation */
int battle_calc_escape_chance(struct BattleUnit* player, struct BattleUnit* fastest_enemy);

/* Skill functions - BattleSkill defined in battle.h which includes this file */
BattleSkill* battle_get_skill(u16 skill_id);
int battle_use_skill(struct BattleUnit* user, struct BattleUnit* target, u16 skill_id);
int battle_can_use_skill(struct BattleUnit* unit, u16 skill_id);

/* Item usage in battle */
int battle_use_item(struct BattleUnit* user, struct BattleUnit* target, u16 item_id);

/* Buff management */
void battle_apply_buff(struct BattleUnit* unit, u32 buff_id, u8 turns);
void battle_remove_buff(struct BattleUnit* unit, u32 buff_id);
void battle_update_buffs(void);

#endif /* BATTLE_CALC_H */
