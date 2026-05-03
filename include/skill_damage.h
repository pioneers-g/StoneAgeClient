/*
 * Stone Age Client - Skill Damage/Effect System Header
 * Split from skill.h for code organization
 */

#ifndef SKILL_DAMAGE_H
#define SKILL_DAMAGE_H

#include "types.h"
#include "skill.h"

/* Element system */
float skill_get_element_multiplier(u8 attack_elem, u8 defend_elem);

/* Damage calculation */
u32 skill_calculate_damage(u32 skill_id, u32 attacker_id, u32 target_id);
SkillDamageResult skill_calculate_damage_ex(SkillDamageParams* params);
u32 skill_calculate_heal(u32 skill_id, u8 skill_level, u16 matk);

/* Hit/Critical rolls */
int skill_roll_critical(u16 luck, u8 skill_level);
int skill_roll_hit(u16 attacker_hit, u16 target_dodge);

/* Effect application */
void skill_apply_effect(SkillEffect* effect);

/* Status effects */
void skill_apply_status(u32 target_id, StatusEffect status, u8 duration, u8 power);
void skill_remove_status(u32 target_id, StatusEffect status);
int skill_has_status(u32 target_id, StatusEffect status);

/* Buffs */
void skill_apply_buff(u32 target_id, BuffType buff, u8 duration, u16 power);
void skill_remove_buff(u32 target_id, BuffType buff);
int skill_has_buff(u32 target_id, BuffType buff);

#endif /* SKILL_DAMAGE_H */
