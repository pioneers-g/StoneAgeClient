/*
 * Stone Age Client - Protocol Handler Data Parsers
 * Split from protocol_handler.c
 *
 * Contains data parsing handlers for:
 * - 0x42: Inventory/Item data
 * - 0x46: Title/Guild data
 * - 0x49: Character list
 * - 0x4A: Pet status
 * - 0x50: Detailed character stats
 * - 0x53: Skill list
 * - 0x57: Pet data
 * - 0x58: Field update
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "protocol_handler.h"
#include "character.h"
#include "pet_protocol.h"
#include "logger.h"

/* External globals from protocol_handler.c */
extern SkillListEntry g_skill_list[];
extern int g_skill_list_count;
extern PetDataEntry57 g_pet_data_57[];
extern int g_pet_data_57_count;
extern DetailedCharacterStats g_detailed_stats;

/* Helper function */
static void safe_string_copy(char* dest, const char* src, size_t max_len) {
    size_t len = strlen(src);
    if (len >= max_len) {
        len = max_len - 1;
    }
    memcpy(dest, src, len);
    dest[len] = '\0';
}

/* ========================================
 * Case 0x42 - Inventory/Item Data Handler
 * ======================================== */

void protocol_handle_case_42(const char* packet_data) {
    char char_index;
    const char* ptr;
    int field_offset;
    char buffer[256];
    int i;

    if (!packet_data || packet_data[0] != 0x42) {
        return;
    }

    char_index = packet_data[1];
    ptr = packet_data + 2;
    field_offset = 1;

    for (i = 0; i < 7; i++) {
        int name_field = field_offset;
        int data_field = field_offset + 1;

        if (pet_parse_field(ptr, '|', name_field, buffer, sizeof(buffer)) != 0) {
            break;
        }
        pet_unescape_string(buffer);

        if (buffer[0] == '\0') {
            LOG_DEBUG("Case 0x42: Clear item slot %d for char %c", i, char_index);
        } else {
            int item_data = pet_parse_field_int(ptr, '|', data_field);
            LOG_DEBUG("Case 0x42: Item %d: %s, data=%d", i, buffer, item_data);
        }

        field_offset += 14;
    }
}

/* ========================================
 * Case 0x46 - Title/Guild Data Handler
 * ======================================== */

void protocol_handle_case_46(const char* packet_data) {
    const char* ptr;
    char title[18];
    int field2, field3, field4, field5;

    if (!packet_data || packet_data[0] != 0x46) {
        return;
    }

    ptr = packet_data + 1;

    pet_parse_field(ptr, '|', 1, title, sizeof(title));
    pet_unescape_string(title);

    field2 = pet_parse_field_int(ptr, '|', 2);
    field3 = pet_parse_field_int(ptr, '|', 3);
    field4 = pet_parse_field_int(ptr, '|', 4);
    field5 = pet_parse_field_int(ptr, '|', 5);

    LOG_DEBUG("Case 0x46: Title=%s, fields=%d,%d,%d,%d",
              title, field2, field3, field4, field5);

    safe_string_copy(g_detailed_stats.title, title, sizeof(g_detailed_stats.title));
}

/* ========================================
 * Case 0x49 - Character List Handler
 * ======================================== */

void protocol_handle_case_49(const char* packet_data) {
    const char* ptr;
    int field_offset;
    char name[30];
    char account[18];
    char description[86];
    int i;

    if (!packet_data || packet_data[0] != 0x49) {
        return;
    }

    ptr = packet_data + 1;
    field_offset = 1;

    for (i = 0; i < 15 && field_offset < 0x151; i++) {
        if (pet_parse_field(ptr, '|', field_offset, name, sizeof(name)) != 0) {
            break;
        }
        pet_unescape_string(name);

        if (name[0] == '\0') {
            field_offset += 14;
            continue;
        }

        pet_parse_field(ptr, '|', field_offset + 1, account, sizeof(account));
        pet_unescape_string(account);

        pet_parse_field(ptr, '|', field_offset + 4, description, sizeof(description));
        pet_unescape_string(description);

        LOG_DEBUG("Case 0x49: Char %d: %s, account=%s", i, name, account);

        field_offset += 14;
    }
}

/* ========================================
 * Case 0x4A - Pet Status Handler
 * ======================================== */

void protocol_handle_case_4a(const char* packet_data) {
    int char_index;
    const char* ptr;
    int active, hp, max_hp, level;
    char name[26];
    char memo[74];

    if (!packet_data || packet_data[0] != 0x4A) {
        return;
    }

    char_index = packet_data[1] - '0';
    ptr = packet_data + 3;

    active = pet_parse_field_int(ptr, '|', 1);

    if (active == 0) {
        LOG_DEBUG("Case 0x4A: Pet slot %d cleared", char_index);
        return;
    }

    hp = pet_parse_field_int(ptr, '|', 2);
    max_hp = pet_parse_field_int(ptr, '|', 3);
    level = pet_parse_field_int(ptr, '|', 4);

    pet_parse_field(ptr, '|', 5, name, sizeof(name));
    pet_unescape_string(name);

    pet_parse_field(ptr, '|', 6, memo, sizeof(memo));
    pet_unescape_string(memo);

    LOG_DEBUG("Case 0x4A: Pet %d: %s, HP=%d/%d, level=%d",
              char_index, name, hp, max_hp, level);
}

/* ========================================
 * Case 0x50 - Detailed Character Stats Handler
 * ======================================== */

void protocol_handle_case_50(const char* packet_data) {
    if (!packet_data || packet_data[0] != 0x50) {
        return;
    }

    character_parse_detailed_stats(packet_data);
}

/* ========================================
 * Case 0x53 - Skill List Handler
 * ======================================== */

void protocol_handle_case_53(const char* packet_data) {
    const char* ptr;
    int field_offset;
    int skill_count;
    int skill_id, level, type, target_type;
    char name[18];
    char description[86];

    if (!packet_data || packet_data[0] != 0x53) {
        return;
    }

    ptr = packet_data + 1;
    field_offset = 1;
    skill_count = 0;

    memset(g_skill_list, 0, sizeof(g_skill_list));
    g_skill_list_count = 0;

    while (field_offset < 0xEB && skill_count < MAX_PLAYER_SKILLS) {
        skill_id = pet_parse_field_int(ptr, '|', field_offset);
        level = pet_parse_field_int(ptr, '|', field_offset + 1);
        type = pet_parse_field_int(ptr, '|', field_offset + 2);
        target_type = pet_parse_field_int(ptr, '|', field_offset + 3);

        pet_parse_field(ptr, '|', field_offset + 7, name, sizeof(name));
        pet_unescape_string(name);

        pet_parse_field(ptr, '|', field_offset + 8, description, sizeof(description));
        pet_unescape_string(description);

        if (name[0] != '\0') {
            g_skill_list[skill_count].skill_id = (u16)skill_id;
            g_skill_list[skill_count].level = (u16)level;
            g_skill_list[skill_count].type = (u16)type;
            g_skill_list[skill_count].target_type = (u16)target_type;
            safe_string_copy(g_skill_list[skill_count].name, name, sizeof(g_skill_list[skill_count].name));

            LOG_DEBUG("Case 0x53: Skill %d: id=%d, level=%d, type=%d, name=%s",
                      skill_count, skill_id, level, type, name);

            skill_count++;
        }

        field_offset += 9;
    }

    g_skill_list_count = skill_count;
    LOG_INFO("Parsed %d skills from case 0x53", skill_count);
}

/* ========================================
 * Case 0x57 - Pet Data Handler
 * ======================================== */

void protocol_handle_case_57(const char* packet_data) {
    int char_index;
    const char* ptr;
    int field_offset;
    int pet_count;
    char name[26];
    char memo[74];
    int field1, field2, field3;

    if (!packet_data || packet_data[0] != 0x57) {
        return;
    }

    char_index = packet_data[1] - '0';
    ptr = packet_data + 3;
    field_offset = 1;
    pet_count = 0;

    memset(g_pet_data_57, 0, sizeof(g_pet_data_57));
    g_pet_data_57_count = 0;

    while (field_offset <= 0x23 && pet_count < MAX_PETS_PER_CHAR) {
        pet_parse_field(ptr, '|', field_offset + 3, name, sizeof(name));
        pet_unescape_string(name);

        if (name[0] == '\0') {
            field_offset += 5;
            continue;
        }

        field1 = pet_parse_field_int(ptr, '|', field_offset);
        field2 = pet_parse_field_int(ptr, '|', field_offset + 1);
        field3 = pet_parse_field_int(ptr, '|', field_offset + 2);

        pet_parse_field(ptr, '|', field_offset + 4, memo, sizeof(memo));
        pet_unescape_string(memo);

        g_pet_data_57[pet_count].active = 1;
        g_pet_data_57[pet_count].field_1 = (u16)field1;
        g_pet_data_57[pet_count].field_2 = (u16)field2;
        g_pet_data_57[pet_count].field_3 = (u16)field3;
        safe_string_copy(g_pet_data_57[pet_count].name, name,
                         sizeof(g_pet_data_57[pet_count].name));
        safe_string_copy(g_pet_data_57[pet_count].memo, memo,
                         sizeof(g_pet_data_57[pet_count].memo));

        LOG_DEBUG("Case 0x57: Pet %d: %s, fields=%d,%d,%d",
                  pet_count, name, field1, field2, field3);

        pet_count++;
        field_offset += 5;
    }

    g_pet_data_57_count = pet_count;
    LOG_INFO("Parsed %d pets for char %d from case 0x57", pet_count, char_index);
}

/* ========================================
 * Case 0x58 - Field Update Handler
 * ======================================== */

void protocol_handle_case_58(const char* packet_data) {
    const char* ptr;
    int field2;

    if (!packet_data || packet_data[0] != 0x58) {
        return;
    }

    ptr = packet_data + 1;
    field2 = pet_parse_field_int(ptr, '|', 2);

    LOG_DEBUG("Case 0x58: Field update, value=%d", field2);
}
