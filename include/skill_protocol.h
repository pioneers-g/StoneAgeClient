/*
 * Stone Age Client - Skill Protocol System Header
 * Split from skill.h for code organization
 */

#ifndef SKILL_PROTOCOL_H
#define SKILL_PROTOCOL_H

#include "types.h"
#include "skill.h"

/* Protocol parsing */
void skill_parse_list_text(const char* packet_data);
void skill_parse_effect_string(const char* effect_str, SkillEffect* effects, int* count);

/* Packet handlers */
void skill_handle_list(void* data, u32 size);
void skill_handle_result(void* data, u32 size);
void skill_handle_effect(void* data, u32 size);
void skill_handle_learn_result(void* data, u32 size);
void skill_handle_level_up(void* data, u32 size);

/* Network requests */
void skill_request_list(void);
int skill_request_learn(u32 skill_id);
int skill_request_forget(u32 skill_id);
int skill_request_level_up(u32 skill_id);
int skill_set_slot_network(int slot, u32 skill_id);

#endif /* SKILL_PROTOCOL_H */
