/*
 * Stone Age Client - Protocol Handler Module
 * Reverse engineered from sa_9061.exe (FUN_0045ffb0)
 *
 * Main protocol dispatcher handling multiple packet types:
 * - 0x42: Inventory/Item data (protocol_handler_data.c)
 * - 0x43: Map enter (protocol_handler_sync.c)
 * - 0x44: Time sync (protocol_handler_sync.c)
 * - 0x45: Player ID setup (protocol_handler_sync.c)
 * - 0x46: Title/Guild data (protocol_handler_data.c)
 * - 0x49: Character list (protocol_handler_data.c)
 * - 0x4A: Pet status (protocol_handler_data.c)
 * - 0x4B: Pet detailed stats (protocol_handler_sync.c)
 * - 0x4D: HP/MP update (protocol_handler_sync.c)
 * - 0x4E: Battle pet status (protocol_handler_sync.c)
 * - 0x50: Detailed character stats (protocol_handler_data.c)
 * - 0x53: Skill list (protocol_handler_data.c)
 * - 0x57: Pet data (protocol_handler_data.c)
 * - 0x58: Field update (protocol_handler_data.c)
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "protocol_handler.h"
#include "character.h"
#include "logger.h"

/* ========================================
 * Global Data Structures
 * ======================================== */

/* Skill list - DAT_04624042 region */
SkillListEntry g_skill_list[MAX_PLAYER_SKILLS] = {0};
int g_skill_list_count = 0;

/* Pet data from case 0x57 - DAT_0462b000 region */
PetDataEntry57 g_pet_data_57[MAX_PETS_PER_CHAR] = {0};
int g_pet_data_57_count = 0;

/* Battle units from case 0x4E - DAT_046308f0 region */
BattleUnit g_battle_units[MAX_BATTLE_UNITS] = {0};
int g_battle_unit_count = 0;

/* Game state sync data - DAT_005ab738 region */
GameStateSync g_game_state = {0};

/* Detailed stats - DAT_0462be98 region */
DetailedCharacterStats g_detailed_stats = {0};

/* State flags */
u32 g_battle_state_flag = 0;
u32 g_game_state_field = 0;
u32 g_battle_sub_state = 0;
u32 g_map_enter_flag = 0;
u32 g_battle_transition = 0;
u32 g_field_state_flag = 0;
u32 g_player_y_coord = 0;
u32 g_player_map_param = 0;
u32 g_current_map_id = 0;
u32 g_player_x_coord = 0;
u32 g_special_flag = 0;
u32 g_flag_045e19b0 = 0;
u32 g_field_counter = 0;
u32 g_player_ref = 0;
u32 g_field_counter2 = 0;

/* ========================================
 * External Handler Declarations
 * Implemented in protocol_handler_data.c and protocol_handler_sync.c
 * ======================================== */

extern void protocol_handle_case_42(const char* packet_data);
extern void protocol_handle_case_43(const char* packet_data);
extern void protocol_handle_case_44(const char* packet_data);
extern void protocol_handle_case_45(const char* packet_data);
extern void protocol_handle_case_46(const char* packet_data);
extern void protocol_handle_case_49(const char* packet_data);
extern void protocol_handle_case_4a(const char* packet_data);
extern void protocol_handle_case_4b(const char* packet_data);
extern void protocol_handle_case_4d(const char* packet_data);
extern void protocol_handle_case_4e(const char* packet_data);
extern void protocol_handle_case_50(const char* packet_data);
extern void protocol_handle_case_53(const char* packet_data);
extern void protocol_handle_case_57(const char* packet_data);
extern void protocol_handle_case_58(const char* packet_data);

/* ========================================
 * Main Protocol Dispatcher - FUN_0045ffb0
 * ======================================== */

/*
 * Main protocol dispatcher - FUN_0045ffb0
 * Dispatches based on first byte of packet
 */
void protocol_handler_dispatch(const char* packet_data) {
    if (!packet_data || packet_data[0] == '\0') {
        return;
    }

    switch ((u8)packet_data[0]) {
        case 0x42:
            protocol_handle_case_42(packet_data);
            break;

        case 0x43:
            protocol_handle_case_43(packet_data);
            break;

        case 0x44:
            protocol_handle_case_44(packet_data);
            break;

        case 0x45:
            protocol_handle_case_45(packet_data);
            break;

        case 0x46:
            protocol_handle_case_46(packet_data);
            break;

        case 0x49:
            protocol_handle_case_49(packet_data);
            break;

        case 0x4A:
            protocol_handle_case_4a(packet_data);
            break;

        case 0x4B:
            protocol_handle_case_4b(packet_data);
            break;

        case 0x4D:
            protocol_handle_case_4d(packet_data);
            break;

        case 0x4E:
            protocol_handle_case_4e(packet_data);
            break;

        case 0x50:
            protocol_handle_case_50(packet_data);
            break;

        case 0x53:
            protocol_handle_case_53(packet_data);
            break;

        case 0x57:
            protocol_handle_case_57(packet_data);
            break;

        case 0x58:
            protocol_handle_case_58(packet_data);
            break;

        default:
            LOG_DEBUG("Unknown protocol case: 0x%02X", (u8)packet_data[0]);
            break;
    }
}

/* ========================================
 * Utility Functions
 * ======================================== */

/*
 * Get skill list entry by index
 */
SkillListEntry* protocol_get_skill(int index) {
    if (index < 0 || index >= g_skill_list_count) {
        return NULL;
    }
    return &g_skill_list[index];
}

/*
 * Get skill list count
 */
int protocol_get_skill_count(void) {
    return g_skill_list_count;
}

/*
 * Get pet data entry by index
 */
PetDataEntry57* protocol_get_pet_data(int index) {
    if (index < 0 || index >= g_pet_data_57_count) {
        return NULL;
    }
    return &g_pet_data_57[index];
}

/*
 * Get pet data count
 */
int protocol_get_pet_count(void) {
    return g_pet_data_57_count;
}

/*
 * Get battle unit by index
 */
BattleUnit* protocol_get_battle_unit(int index) {
    if (index < 0 || index >= g_battle_unit_count) {
        return NULL;
    }
    return &g_battle_units[index];
}

/*
 * Get battle unit count
 */
int protocol_get_battle_unit_count(void) {
    return g_battle_unit_count;
}

/*
 * Get detailed stats
 */
DetailedCharacterStats* protocol_get_detailed_stats(void) {
    return &g_detailed_stats;
}

/*
 * Get game state sync
 */
GameStateSync* protocol_get_game_state(void) {
    return &g_game_state;
}
