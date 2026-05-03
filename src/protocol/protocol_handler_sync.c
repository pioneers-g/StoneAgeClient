/*
 * Stone Age Client - Protocol Handler State Sync
 * Split from protocol_handler.c
 *
 * Contains state synchronization handlers for:
 * - 0x43: Map enter
 * - 0x44: Time sync
 * - 0x45: Player ID setup
 * - 0x4D: HP/MP update
 * - 0x4B: Pet detailed stats (bitmask)
 * - 0x4E: Battle pet status (bitmask)
 */

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "protocol_handler.h"
#include "pet_protocol.h"
#include "map.h"
#include "minimap.h"
#include "ui.h"
#include "logger.h"

/* External globals */
extern PetDataEntry57 g_pet_data_57[];
extern BattleUnit g_battle_units[];
extern GameStateSync g_game_state;
extern DetailedCharacterStats g_detailed_stats;

/* External flags from protocol_handler.c */
extern u32 g_battle_state_flag;
extern u32 g_game_state_field;
extern u32 g_battle_sub_state;
extern u32 g_map_enter_flag;
extern u32 g_battle_transition;
extern u32 g_field_state_flag;
extern u32 g_player_y_coord;
extern u32 g_player_map_param;

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
 * Case 0x43 - Map Enter Handler
 * ======================================== */

void protocol_handle_case_43(const char* packet_data) {
    const char* ptr;
    int map_id, x, y, param1, param2;
    extern u32 g_current_map_id;
    extern u32 g_player_x_coord;
    extern u32 g_special_flag;
    extern u32 g_flag_045e19b0;
    extern u32 g_field_counter;
    extern u32 g_player_ref;
    extern u32 g_field_counter2;

    if (!packet_data || packet_data[0] != 0x43) {
        return;
    }

    g_map_enter_flag = 1;

    if (g_battle_state_flag == 0 && g_game_state_field == 9) {
        if (g_battle_transition == 0) {
            g_battle_sub_state = 200;
            battle_transition_cleanup();

            if ((g_special_flag & 0x40000000) != 0) {
                g_flag_045e19b0 = 1;
            }
        }

        field_clear_state();

        g_battle_transition = 0;
        g_field_state_flag = 1;
    }

    ptr = packet_data + 1;

    map_id = pet_parse_field_int(ptr, '|', 1);
    param1 = pet_parse_field_int(ptr, '|', 2);
    param2 = pet_parse_field_int(ptr, '|', 3);
    x = pet_parse_field_int(ptr, '|', 4);
    y = pet_parse_field_int(ptr, '|', 5);

    LOG_DEBUG("Case 0x43: Map enter - map_id=%d, pos=(%d,%d), params=(%d,%d)",
              map_id, x, y, param1, param2);

    character_setup_map_position(map_id, param2, y);
    character_set_map_position(map_id, param1, param2);

    g_player_y_coord = param2;
    g_player_map_param = param1;

    map_npc_clear();

    g_field_counter = 0;
    g_player_ref = g_current_map_id;
    g_player_x_coord = 0;

    field_init_state();

    g_field_counter2 = 0;

    map_load(map_id);

    minimap_set_center((u16)param2, (u16)param1);
    minimap_set_map(map_id, 0, 0);
}

/* ========================================
 * Case 0x44 - Time Sync Handler
 * ======================================== */

void protocol_handle_case_44(const char* packet_data) {
    const char* ptr;
    u32 timestamp;
    int param;
    u32 sync_result;

    if (!packet_data || packet_data[0] != 0x44) {
        return;
    }

    ptr = packet_data + 1;

    timestamp = (u32)pet_parse_field_int(ptr, '|', 1);
    param = pet_parse_field_int(ptr, '|', 2);

    game_time_sync(timestamp);

    g_game_state.last_sync_time = param;
    g_game_state.last_sync_tick = timeGetTime();

    game_state_update(&g_game_state);
    sync_result = game_state_calculate(&g_game_state);

    timer_update(sync_result, 0);

    LOG_DEBUG("Case 0x44: Time sync - timestamp=%u, param=%d, result=%u",
              timestamp, param, sync_result);
}

/* ========================================
 * Case 0x45 - Player ID Setup Handler
 * ======================================== */

void protocol_handle_case_45(const char* packet_data) {
    const char* ptr;
    int player_id, field2;

    if (!packet_data || packet_data[0] != 0x45) {
        return;
    }

    ptr = packet_data + 1;

    player_id = pet_parse_field_int(ptr, '|', 1);
    field2 = pet_parse_field_int(ptr, '|', 2);

    g_detailed_stats.player_id = player_id;
    g_detailed_stats.field_06 = field2;

    LOG_DEBUG("Case 0x45: Player ID setup - id=%d, field2=%d", player_id, field2);
}

/* ========================================
 * Case 0x4D - HP/MP Update Handler
 * ======================================== */

void protocol_handle_case_4d(const char* packet_data) {
    const char* ptr;
    int hp, mp, stamina;

    if (!packet_data || packet_data[0] != 0x4D) {
        return;
    }

    ptr = packet_data + 1;

    hp = pet_parse_field_int(ptr, '|', 1);
    mp = pet_parse_field_int(ptr, '|', 2);
    stamina = pet_parse_field_int(ptr, '|', 3);

    g_detailed_stats.field_02 = hp;
    g_detailed_stats.field_04 = mp;
    g_detailed_stats.field_10 = stamina;

    LOG_DEBUG("Case 0x4D: HP=%d, MP=%d, Stamina=%d", hp, mp, stamina);

    ui_update();
}

/* ========================================
 * Case 0x4B - Pet Detailed Stats Handler (Bitmask)
 * ======================================== */

void protocol_handle_case_4b(const char* packet_data) {
    int pet_index;
    const char* ptr;
    u32 bitmask;
    int field_idx;
    u32 field_value;
    int current_field;
    char buffer[256];

    if (!packet_data || packet_data[0] != 0x4B) {
        return;
    }

    pet_index = packet_data[1] - '0';
    ptr = packet_data + 3;

    bitmask = (u32)pet_parse_field_int(ptr, '|', 1);

    if (bitmask == 0) {
        LOG_DEBUG("Case 0x4B: Clear pet slot %d", pet_index);
        return;
    }

    field_idx = 2;
    current_field = 0;

    while (bitmask != 0 && current_field < 20) {
        u32 bit = 1u << current_field;

        if (bitmask & bit) {
            field_value = (u32)pet_parse_field_int(ptr, '|', field_idx);
            field_idx++;

            switch (bit) {
                case 0x00000002:
                    g_pet_data_57[pet_index].field_1 = (u16)field_value;
                    break;
                case 0x00000004:
                    g_pet_data_57[pet_index].field_2 = (u16)field_value;
                    break;
                case 0x00000008:
                    g_pet_data_57[pet_index].field_3 = (u16)field_value;
                    break;
                case 0x00080000:
                    pet_parse_field(ptr, '|', field_idx - 1, buffer, sizeof(buffer));
                    pet_unescape_string(buffer);
                    safe_string_copy(g_pet_data_57[pet_index].name, buffer,
                                     sizeof(g_pet_data_57[pet_index].name));
                    break;
                case 0x00100000:
                    pet_parse_field(ptr, '|', field_idx - 1, buffer, sizeof(buffer));
                    pet_unescape_string(buffer);
                    safe_string_copy(g_pet_data_57[pet_index].memo, buffer,
                                     sizeof(g_pet_data_57[pet_index].memo));
                    break;
                default:
                    break;
            }
        }

        current_field++;
        bitmask &= ~bit;
    }

    LOG_DEBUG("Case 0x4B: Pet %d detailed stats parsed", pet_index);
}

/* ========================================
 * Case 0x4E - Battle Pet Status Handler (Bitmask)
 * ======================================== */

void protocol_handle_case_4e(const char* packet_data) {
    int unit_index;
    const char* ptr;
    u32 bitmask;
    int field_idx;
    u32 field_value;
    int current_field;
    char buffer[256];

    if (!packet_data || packet_data[0] != 0x4E) {
        return;
    }

    unit_index = packet_data[1] - '0';
    ptr = packet_data + 3;

    bitmask = (u32)pet_parse_field_int(ptr, '|', 1);

    if (bitmask == 0) {
        LOG_DEBUG("Case 0x4E: Clear battle unit slot %d", unit_index);
        return;
    }

    field_idx = 2;
    current_field = 0;

    while (bitmask != 0 && current_field < 7) {
        u32 bit = 1u << current_field;

        if (bitmask & bit) {
            field_value = (u32)pet_parse_field_int(ptr, '|', field_idx);
            field_idx++;

            switch (bit) {
                case 0x00000002:
                    g_battle_units[unit_index].entity_id = field_value;
                    break;
                case 0x00000004:
                    g_battle_units[unit_index].hp = (u16)field_value;
                    break;
                case 0x00000008:
                    g_battle_units[unit_index].max_hp = (u16)field_value;
                    break;
                case 0x00000010:
                    g_battle_units[unit_index].mp = (u16)field_value;
                    break;
                case 0x00000020:
                    g_battle_units[unit_index].max_mp = (u16)field_value;
                    break;
                case 0x00000040:
                    pet_parse_field(ptr, '|', field_idx - 1, buffer, sizeof(buffer));
                    pet_unescape_string(buffer);
                    safe_string_copy(g_battle_units[unit_index].name, buffer,
                                     sizeof(g_battle_units[unit_index].name));
                    break;
                default:
                    break;
            }
        }

        current_field++;
        bitmask &= ~bit;
    }

    LOG_DEBUG("Case 0x4E: Battle unit %d status parsed", unit_index);
}
