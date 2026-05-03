/*
 * Stone Age Client - Skill Protocol System
 * Split from skill.c for code organization
 *
 * Handles protocol parsing and network communication
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "types.h"
#include "skill.h"
#include "skill_protocol.h"
#include "skill_damage.h"
#include "network.h"
#include "pet_protocol.h"
#include "logger.h"

/* External skill context */
extern SkillContext g_skill;
extern SkillListEntry g_skill_list[MAX_PLAYER_SKILLS];
extern int g_skill_list_count;

/* External skill functions */
extern SkillData* skill_get_data(u32 skill_id);
extern LearnedSkill* skill_get_learned(u32 skill_id);
extern int skill_learn(u32 skill_id);
extern int skill_set_level(u32 skill_id, u8 level);
extern void skill_start_cooldown(u32 skill_id);

/*
 * Send skill protocol packet - helper function
 */
static void skill_send_packet(const char* cmd, const char* params) {
    char packet[512];

    if (params && params[0]) {
        _snprintf(packet, sizeof(packet), "%s|%s", cmd, params);
    } else {
        strncpy(packet, cmd, sizeof(packet) - 1);
    }

    network_send(packet, strlen(packet));
    network_send("\n", 1);
}

/*
 * Send formatted skill packet - helper function
 */
static void skill_send_formatted(const char* fmt, ...) {
    char packet[1024];
    va_list args;

    va_start(args, fmt);
    _vsnprintf(packet, sizeof(packet), fmt, args);
    va_end(args);

    network_send(packet, strlen(packet));
    network_send("\n", 1);
}

/*
 * Handle skill result
 */
void skill_handle_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;
    u32 skill_id;
    u32 target_id;

    result = *ptr++;
    skill_id = *(u32*)ptr; ptr += 4;
    target_id = *(u32*)ptr;

    if (result == 0) {
        LOG_DEBUG("Skill %u used on target %u", skill_id, target_id);
    } else {
        LOG_WARN("Skill %u failed: result %d", skill_id, result);
    }
}

/*
 * Handle skill effect
 */
void skill_handle_effect(void* data, u32 size) {
    u8* ptr = (u8*)data;
    SkillEffect effect;
    int i, count;

    count = *(u8*)ptr++;

    for (i = 0; i < count; i++) {
        effect.target_id = *(u32*)ptr; ptr += 4;
        effect.damage = *(s32*)ptr; ptr += 4;
        effect.heal = *(s32*)ptr; ptr += 4;
        effect.critical = *(u8*)ptr; ptr += 1;
        effect.miss = *(u8*)ptr; ptr += 1;
        effect.status_effect = *(u8*)ptr; ptr += 1;
        effect.status_duration = *(u8*)ptr; ptr += 1;

        LOG_DEBUG("Skill effect on %u: dmg=%d heal=%d",
                  effect.target_id, effect.damage, effect.heal);
    }
}

/*
 * Handle skill list packet
 */
void skill_handle_list(void* data, u32 size) {
    u8* ptr = (u8*)data;
    int i;
    LearnedSkill* skill;

    g_skill.skill_count = 0;

    for (i = 0; i < MAX_PLAYER_SKILLS && ptr < (u8*)data + size; i++) {
        skill = &g_skill.skills[i];

        skill->skill_id = *(u32*)ptr; ptr += 4;
        if (skill->skill_id == 0) continue;

        skill->level = *(u8*)ptr; ptr += 1;
        skill->slot = *(u8*)ptr; ptr += 1;
        skill->exp = *(u16*)ptr; ptr += 2;

        g_skill.skill_count++;
    }

    g_skill.loaded = 1;
    LOG_INFO("Loaded %d skills", g_skill.skill_count);
}

/*
 * Parse skill list from text protocol - FUN_0045ffb0 case 0x53
 * Packet format: 0x53|field1|field2|...| (9 fields per skill)
 *
 * Memory layout from Ghidra:
 * - Base address: DAT_04624042
 * - Entry size: 0x60 (96) bytes
 * - Fields: skill_id, level, type, target, field5, field6, field7, name, desc
 */
void skill_parse_list_text(const char* packet_data) {
    const char* ptr = packet_data;
    char field_buffer[256];
    char name_buffer[32];
    char desc_buffer[128];
    int field_index;
    int skill_index;
    s32 int_val;
    int i;

    if (!packet_data) return;

    /* Skip packet type byte (0x53) */
    ptr += 1;

    /* Clear skill list - matches Ghidra clear loop at DAT_04624046 */
    memset(g_skill_list, 0, sizeof(g_skill_list));
    g_skill_list_count = 0;

    /* Field index starts at 2 in binary (iVar19 = 2) */
    field_index = 2;

    /* Parse skills - loop ends when field_index > 0xEB (235) */
    /* Max ~26 skills with 9 fields each */
    for (skill_index = 0; skill_index < MAX_PLAYER_SKILLS; skill_index++) {
        /* Field 1 (field_index - 1): Skill ID (u16) */
        int_val = pet_parse_field_int(ptr, '|', field_index - 1);
        if (int_val <= 0) {
            /* Empty slot or end of list */
            break;
        }
        g_skill_list[skill_index].skill_id = (u16)int_val;

        /* Field 2: Level (u16) */
        int_val = pet_parse_field_int(ptr, '|', field_index);
        g_skill_list[skill_index].level = (u16)int_val;

        /* Field 3: Type (u16) */
        int_val = pet_parse_field_int(ptr, '|', field_index + 1);
        g_skill_list[skill_index].type = (u16)int_val;

        /* Field 4: Target type (u16) */
        int_val = pet_parse_field_int(ptr, '|', field_index + 2);
        g_skill_list[skill_index].target_type = (u16)int_val;

        /* Field 5: Additional data (u32) */
        int_val = pet_parse_field_int(ptr, '|', field_index + 3);
        g_skill_list[skill_index].field_10 = (u32)int_val;

        /* Field 6: Additional data (u32) */
        int_val = pet_parse_field_int(ptr, '|', field_index + 4);
        g_skill_list[skill_index].field_14 = (u32)int_val;

        /* Field 7: Additional data (u32) */
        int_val = pet_parse_field_int(ptr, '|', field_index + 5);
        g_skill_list[skill_index].field_18 = (u32)int_val;

        /* Field 8: Name string (max 0x11 = 17 bytes) */
        pet_parse_field(ptr, '|', field_index + 6, name_buffer, sizeof(name_buffer));
        pet_unescape_string(name_buffer);
        strncpy(g_skill_list[skill_index].name, name_buffer, 16);
        g_skill_list[skill_index].name[16] = '\0';

        /* Field 9: Description string (max 0x55 = 85 bytes) */
        pet_parse_field(ptr, '|', field_index + 7, desc_buffer, sizeof(desc_buffer));
        pet_unescape_string(desc_buffer);
        strncpy(g_skill_list[skill_index].description, desc_buffer, 84);
        g_skill_list[skill_index].description[84] = '\0';

        LOG_DEBUG("Parsed skill %d: id=%u, level=%u, name=%s",
                  skill_index, g_skill_list[skill_index].skill_id,
                  g_skill_list[skill_index].level, g_skill_list[skill_index].name);

        /* Move to next skill (9 fields per skill) */
        field_index += 9;
        g_skill_list_count++;

        /* Check loop termination condition from Ghidra */
        if (field_index > 0xEB) break;
    }

    /* Update learned skills from parsed list */
    g_skill.skill_count = 0;
    for (i = 0; i < g_skill_list_count && i < MAX_PLAYER_SKILLS; i++) {
        g_skill.skills[i].skill_id = g_skill_list[i].skill_id;
        g_skill.skills[i].level = (u8)g_skill_list[i].level;
        g_skill.skill_count++;
    }

    g_skill.loaded = 1;
    LOG_INFO("Parsed %d skills from text protocol", g_skill_list_count);
}

/*
 * Handle skill learn result
 */
void skill_handle_learn_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;
    u32 skill_id;

    result = *ptr++;
    skill_id = *(u32*)ptr;

    if (result == 0) {
        skill_learn(skill_id);
    } else {
        LOG_WARN("Failed to learn skill %u", skill_id);
    }
}

/*
 * Handle skill level up
 */
void skill_handle_level_up(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 skill_id;
    u8 new_level;

    skill_id = *(u32*)ptr; ptr += 4;
    new_level = *ptr;

    skill_set_level(skill_id, new_level);
}

/*
 * Parse effect string from server
 * Format: "target1,dmg1,heal1,crit1,miss1|target2,dmg2,..."
 */
void skill_parse_effect_string(const char* effect_str, SkillEffect* effects, int* count) {
    const char* ptr = effect_str;
    char field[64];
    int field_idx;
    int effect_idx = 0;
    int field_pos = 0;
    int in_field = 1;

    if (!effect_str || !effects || !count) return;

    *count = 0;

    /* Parse each target's effects */
    while (*ptr && effect_idx < MAX_SKILL_TARGETS) {
        field_idx = 0;
        field_pos = 0;

        /* Parse fields separated by comma */
        while (*ptr && *ptr != '|' && field_idx < 8) {
            if (*ptr == ',') {
                field[field_pos] = '\0';

                switch (field_idx) {
                    case 0: effects[effect_idx].target_id = atoi(field); break;
                    case 1: effects[effect_idx].damage = atoi(field); break;
                    case 2: effects[effect_idx].heal = atoi(field); break;
                    case 3: effects[effect_idx].critical = (u8)atoi(field); break;
                    case 4: effects[effect_idx].miss = (u8)atoi(field); break;
                    case 5: effects[effect_idx].status_effect = (u8)atoi(field); break;
                    case 6: effects[effect_idx].status_duration = (u8)atoi(field); break;
                }

                field_idx++;
                field_pos = 0;
            } else {
                if (field_pos < 63) {
                    field[field_pos++] = *ptr;
                }
            }
            ptr++;
        }

        /* Handle last field */
        if (field_idx == 6) {
            field[field_pos] = '\0';
            effects[effect_idx].status_duration = (u8)atoi(field);
        }

        effect_idx++;

        if (*ptr == '|') ptr++;
    }

    *count = effect_idx;
}

/*
 * Request skill list from server
 */
void skill_request_list(void) {
    skill_send_packet(SKILL_CMD_LIST, "");
    LOG_DEBUG("Requested skill list");
}

/*
 * Request to learn skill
 */
int skill_request_learn(u32 skill_id) {
    char params[32];

    _snprintf(params, sizeof(params), "%u", skill_id);
    skill_send_packet(SKILL_CMD_LEARN, params);

    LOG_DEBUG("Requesting to learn skill %u", skill_id);
    return 1;
}

/*
 * Request to forget skill
 */
int skill_request_forget(u32 skill_id) {
    char params[32];

    _snprintf(params, sizeof(params), "%u", skill_id);
    skill_send_packet(SKILL_CMD_FORGET, params);

    LOG_DEBUG("Requesting to forget skill %u", skill_id);
    return 1;
}

/*
 * Request skill level up
 */
int skill_request_level_up(u32 skill_id) {
    char params[32];

    if (g_skill.skill_points <= 0) {
        LOG_WARN("No skill points available");
        return 0;
    }

    _snprintf(params, sizeof(params), "%u", skill_id);
    skill_send_packet(SKILL_CMD_LEVEL_UP, params);

    LOG_DEBUG("Requesting skill level up for %u", skill_id);
    return 1;
}

/*
 * Set skill slot (hotkey) - sends to server
 */
int skill_set_slot_network(int slot, u32 skill_id) {
    char params[32];

    if (slot < 0 || slot >= 10) return 0;

    _snprintf(params, sizeof(params), "%d|%u", slot, skill_id);
    skill_send_packet(SKILL_CMD_SET_SLOT, params);

    /* Also update locally */
    g_skill.skill_slots[slot] = skill_id;
    g_skill.dirty = 1;

    LOG_DEBUG("Set skill slot %d to %u", slot, skill_id);
    return 1;
}
