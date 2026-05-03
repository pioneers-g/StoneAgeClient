/*
 * Stone Age Client - Network Protocol Binary Dispatcher
 * Split from network_protocol.c
 *
 * Reverse engineered from sa_9061.exe (FUN_0045ffb0)
 * Binary packet dispatcher handling cases 0x42-0x58
 */

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "network.h"
#include "protocol.h"
#include "character_protocol.h"
#include "pet_protocol.h"
#include "skill.h"
#include "item.h"
#include "logger.h"

/* Include shared data structures */
#include "network_protocol_data.c"

/* ========================================
 * Case 0x42 - Item/Inventory Data
 * ======================================== */

/*
 * Parse inventory item list - FUN_0045ffb0 case 0x42
 * Format: 0x42<char_index>|fields...
 * Entry stride: 0x184 bytes, max 97 items (0x61)
 */
static void protocol_handle_inventory_data(const u8* data, u32 len) {
    char char_idx;
    const char* packet_data;
    int item_offset;
    int base_offset;
    char field_buf[256];
    int field_idx;

    if (len < 2) return;

    char_idx = data[1];
    packet_data = (const char*)(data + 2);

    /* Item offset starts at 0, increments by 0x184 bytes */
    item_offset = 0;
    field_idx = 2;  /* Start at field 2 */

    /* Parse up to 97 items (0x61) */
    while (field_idx < 100) {
        /* Parse item name field */
        pet_parse_field(packet_data, '|', field_idx - 1, field_buf, sizeof(field_buf));
        pet_unescape_string(field_buf);

        if (strlen(field_buf) == 0) {
            /* Empty slot - clear item data */
            /* memset(&g_inventory[item_offset], 0, 0x184); */
        } else {
            /* Parse item fields */
            base_offset = (char_idx - '0') * 0xb18 + item_offset;

            /* Field: item name */
            /* strncpy(g_inventory[base_offset].name, field_buf, 0x1d); */

            /* Field: item count */
            field_idx++;
            /* g_inventory[base_offset].count = pet_parse_field_int(packet_data, '|', field_idx); */

            /* Field: item icon */
            field_idx++;
            /* g_inventory[base_offset].icon = pet_parse_field_int(packet_data, '|', field_idx); */

            /* Continue parsing remaining fields... */
            field_idx += 11;  /* Skip to next item */
        }

        item_offset += 0x184;
        field_idx += 14;  /* 14 fields per item */

        if (item_offset >= 0xb18) break;  /* Max items per character */
    }

    LOG_DEBUG("Parsed inventory for char %c", char_idx);
}

/* ========================================
 * Case 0x43 - Map Entry
 * ======================================== */

/*
 * Map entry handler - FUN_0045ffb0 case 0x43
 * Handles map transitions and battle initialization
 */
void protocol_handle_map_entry(const char* packet_data) {
    int map_id, width, height, param1, param2;

    if (!packet_data) return;

    /* Skip the 'C' packet type identifier */
    if (packet_data[0] == 'C') {
        packet_data++;
    }

    /* Parse fields: map_id, width, height, param1, param2 */
    map_id = pet_parse_field_int(packet_data, '|', 1);
    width = pet_parse_field_int(packet_data, '|', 2);
    height = pet_parse_field_int(packet_data, '|', 3);
    param1 = pet_parse_field_int(packet_data, '|', 4);
    param2 = pet_parse_field_int(packet_data, '|', 5);

    LOG_DEBUG("Map entry: id=%d, size=%dx%d, params=%d,%d",
              map_id, width, height, param1, param2);

    /* Set map flag - DAT_045829c8 = 1 */
    s_map_entry_flag = 1;

    /* Check if in battle state (game state 9) - DAT_04630dd8 */
    if (s_battle_flag == 0 && s_game_state == 9) {
        /* Initialize battle transition */
        if (s_battle_substate == 0) {
            s_game_substate = 200;  /* DAT_04630df0 = 200 */
            /* FUN_0047a3f0: battle transition init */
        }
        /* FUN_00477b90: clear player entity */
        s_player_clear_flag = 0;
        s_battle_active_flag = 1;
    }

    /* Store map dimensions - FUN_00440dd0 pattern */
    s_map_id = map_id;           /* DAT_04581190 */
    s_map_width = width;         /* DAT_0458119c */
    s_map_height = height;       /* DAT_04569b70 */

    /* Set current position from params */
    s_map_pos_x = param1;
    s_map_pos_y = param2;

    /* Clear entity list */
    s_entity_count = 0;
    s_npc_count = 0;

    /* Update global map context */
    s_last_map_id = map_id;
    s_map_param = 0;

    /* Calculate isometric camera positions */
    s_iso_camera_x = (float)param1 * ISO_TILE_RATIO;
    s_iso_camera_y = (float)param2 * ISO_TILE_RATIO;

    /* Initialize map view boundaries */
    s_view_min_x = param1 - 20;
    s_view_min_y = param2 - 16;
    s_view_max_x = param1 + 17;
    s_view_max_y = param2 + 21;

    /* Clamp view boundaries */
    if (s_view_min_x < 0) s_view_min_x = 0;
    if (s_view_min_y < 0) s_view_min_y = 0;
    if (s_view_max_x > width) s_view_max_x = width;
    if (s_view_max_y > height) s_view_max_y = height;

    /* Calculate view dimensions */
    s_view_width = s_view_max_x - s_view_min_x;
    s_view_height = s_view_max_y - s_view_min_y;

    /* Clear render state */
    s_render_queue_count = 0;
    s_fade_active = 0;

    LOG_INFO("Map loaded: id=%d, view=(%d,%d)-(%d,%d)",
             map_id, s_view_min_x, s_view_min_y, s_view_max_x, s_view_max_y);
}

/* ========================================
 * Case 0x4E - Battle Unit Status
 * ======================================== */

/*
 * Battle unit status handler - FUN_0045ffb0 case 0x4E
 * Handles battle unit creation, updates, and removal
 */
void protocol_handle_battle_unit_status(const char* packet_data) {
    int unit_index;
    u32 field_mask;
    int field_offset;

    if (!packet_data) return;

    /* Skip the 'N' packet type identifier */
    if (packet_data[0] == 'N') {
        packet_data++;
    }

    /* Get unit index from first character (ASCII '0'-'9') */
    unit_index = packet_data[0] - '0';
    if (unit_index < 0 || unit_index >= MAX_BATTLE_UNITS) {
        LOG_WARN("Invalid battle unit index: %d", unit_index);
        return;
    }

    /* Move past the index character and delimiter */
    packet_data += 2;

    /* Parse bitmask field - FUN_0048a120 uses Base-62 decoding */
    field_mask = (u32)pet_parse_field_base62(packet_data, '|', 1);

    LOG_DEBUG("Battle unit %d: mask=0x%08X", unit_index, field_mask);

    /* If mask is 0, remove unit */
    if (field_mask == 0) {
        /* Check if unit exists before removing */
        if (s_battle_units[unit_index].active != 0) {
            /* Clear unit from selection if selected */
            if (unit_index == s_selected_unit) {
                s_selected_unit = -1;
            }
            if (unit_index == s_target_unit) {
                s_target_unit = -1;
            }
            /* Clear active flag */
            s_battle_unit_flags[unit_index] = 0;
        }
        /* Clear unit active flag */
        s_battle_units[unit_index].active = 0;
        LOG_DEBUG("Battle unit %d removed", unit_index);
        return;
    }

    /* Mark unit as active */
    s_battle_units[unit_index].active = 1;

    /* Parse fields based on bitmask */
    field_offset = 2;  /* Start after bitmask field */

    /* Bit 1 (0x01): Unit ID */
    if (field_mask & 0x01) {
        s_battle_units[unit_index].unit_id = pet_parse_field_int(packet_data, '|', field_offset);
        field_offset++;
    }

    /* Bit 2 (0x02): X position */
    if (field_mask & 0x02) {
        s_battle_units[unit_index].pos_x = pet_parse_field_int(packet_data, '|', field_offset);
        field_offset++;
    }

    /* Bit 3 (0x04): Y position */
    if (field_mask & 0x04) {
        s_battle_units[unit_index].pos_y = pet_parse_field_int(packet_data, '|', field_offset);
        field_offset++;
    }

    /* Bit 4 (0x08): Direction */
    if (field_mask & 0x08) {
        s_battle_units[unit_index].direction = pet_parse_field_int(packet_data, '|', field_offset);
        field_offset++;
    }

    /* Bit 5 (0x10): Level */
    if (field_mask & 0x10) {
        int level = pet_parse_field_int(packet_data, '|', field_offset);
        s_battle_units[unit_index].level = level;
        s_battle_units[unit_index].level_display = (level >= 100) ? 1 : 0;
        if (level >= 100) {
            s_battle_units[unit_index].level = level % 100;
        }
        field_offset++;
    }

    /* Bit 6 (0x20): HP */
    if (field_mask & 0x20) {
        s_battle_units[unit_index].hp = pet_parse_field_int(packet_data, '|', field_offset);
        field_offset++;
    }

    /* Bit 7 (0x40): Max HP */
    if (field_mask & 0x40) {
        s_battle_units[unit_index].max_hp = pet_parse_field_int(packet_data, '|', field_offset);
        field_offset++;
    }

    /* Bit 8 (0x80): MP */
    if (field_mask & 0x80) {
        s_battle_units[unit_index].mp = pet_parse_field_int(packet_data, '|', field_offset);
        field_offset++;
    }

    /* Bit 9 (0x100): Sprite ID */
    if (field_mask & 0x100) {
        s_battle_units[unit_index].sprite_id = pet_parse_field_int(packet_data, '|', field_offset);
        field_offset++;
    }

    /* Bit 10 (0x200): Status flags */
    if (field_mask & 0x200) {
        s_battle_units[unit_index].status = pet_parse_field_int(packet_data, '|', field_offset);
        field_offset++;
    }

    /* Bit 11 (0x400): Action type */
    if (field_mask & 0x400) {
        s_battle_units[unit_index].action_type = pet_parse_field_int(packet_data, '|', field_offset);
        field_offset++;
    }

    /* Bit 20 (0x80000): Name field */
    if (field_mask & 0x80000) {
        char name_buffer[32];
        pet_parse_field(packet_data, '|', field_offset, name_buffer, sizeof(name_buffer));
        pet_unescape_string(name_buffer);
        strncpy(s_battle_units[unit_index].name, name_buffer, sizeof(s_battle_units[unit_index].name) - 1);
        field_offset++;
    }

    /* Bit 21 (0x100000): Title field */
    if (field_mask & 0x100000) {
        char title_buffer[32];
        pet_parse_field(packet_data, '|', field_offset, title_buffer, sizeof(title_buffer));
        pet_unescape_string(title_buffer);
        strncpy(s_battle_units[unit_index].title, title_buffer, sizeof(s_battle_units[unit_index].title) - 1);
        field_offset++;
    }

    LOG_INFO("Battle unit %d: id=%d, pos=(%d,%d), hp=%d/%d",
             unit_index, s_battle_units[unit_index].unit_id,
             s_battle_units[unit_index].pos_x, s_battle_units[unit_index].pos_y,
             s_battle_units[unit_index].hp, s_battle_units[unit_index].max_hp);

    s_battle_unit_count++;
}

/* ========================================
 * Binary Protocol Dispatcher - FUN_0045ffb0
 * ======================================== */

/*
 * Main binary packet dispatcher
 * Dispatches based on first byte of packet data
 */
void packet_dispatch_binary(SOCKET sock) {
    u8* data = g_net.recv_buffer;
    u32 len = g_net.recv_len;
    char* packet_data;

    (void)sock;

    if (len < 2) {
        return;
    }

    /* Get packet data (skip the type byte) */
    packet_data = (char*)(data + 1);

    /* Switch on first byte - FUN_0045ffb0 pattern */
    u8 packet_type = data[0];

    switch (packet_type) {
        case 0x42:
            /* Item/Inventory data */
            LOG_DEBUG("Binary packet: Item data (0x42)");
            protocol_handle_inventory_data(data, len);
            break;

        case 0x43:
            /* Map entry/Battle start */
            LOG_DEBUG("Binary packet: Map entry (0x43)");
            protocol_handle_map_entry(packet_data);
            break;

        case 0x44:
            /* Timing/Movement sync */
            LOG_DEBUG("Binary packet: Movement sync (0x44)");
            {
                s_last_move_time = timeGetTime();
                s_move_target = (u32)pet_parse_field_int(packet_data, '|', 1);
                s_move_timer_active = 1;
            }
            break;

        case 0x45:
            /* Position/Field ID update */
            LOG_DEBUG("Binary packet: Field update (0x45)");
            {
                s_current_field_id = pet_parse_field_int(packet_data, '|', 1);
                s_field_sub_id = pet_parse_field_int(packet_data, '|', 2);
                s_last_field_id = s_current_field_id;
            }
            break;

        case 0x46:
            /* Character name/data */
            LOG_DEBUG("Binary packet: Character data (0x46)");
            {
                char name_buffer[32];
                pet_parse_field(packet_data, '|', 1, name_buffer, sizeof(name_buffer));
                pet_unescape_string(name_buffer);
                strncpy(s_player_name, name_buffer, sizeof(s_player_name) - 1);
                s_player_name[sizeof(s_player_name) - 1] = '\0';

                s_player_field1 = pet_parse_field_int(packet_data, '|', 2);
                s_player_field2 = pet_parse_field_int(packet_data, '|', 3);
                s_player_field3 = pet_parse_field_int(packet_data, '|', 4);
                s_player_field4 = pet_parse_field_int(packet_data, '|', 5);

                if (s_player_field1 == -1) {
                    s_party_flag &= ~0x80;
                    s_in_party = 0;
                }
            }
            break;

        case 0x49:
            /* Party/Group data */
            LOG_DEBUG("Binary packet: Party data (0x49)");
            {
                int entry_offset = 2;
                int char_idx = 0;
                char field_buf[256];

                while (entry_offset < 0x152 && char_idx < MAX_PARTY_MEMBERS) {
                    pet_parse_field(packet_data, '|', entry_offset - 1, field_buf, sizeof(field_buf));
                    pet_unescape_string(field_buf);

                    if (strlen(field_buf) == 0) {
                        s_party_entries[char_idx].active = 0;
                    } else {
                        s_party_entries[char_idx].active = 1;
                        strncpy(s_party_entries[char_idx].name, field_buf, 29);

                        pet_parse_field(packet_data, '|', entry_offset, field_buf, sizeof(field_buf));
                        pet_unescape_string(field_buf);
                        strncpy(s_party_entries[char_idx].title, field_buf, 17);

                        s_party_entries[char_idx].field1 = pet_parse_field_int(packet_data, '|', entry_offset + 1);
                        s_party_entries[char_idx].level = (u16)pet_parse_field_int(packet_data, '|', entry_offset + 4);
                        s_party_entries[char_idx].sprite_id = (u16)pet_parse_field_int(packet_data, '|', entry_offset + 5);

                        if (s_party_entries[char_idx].level >= 100) {
                            s_party_entries[char_idx].level_mod = s_party_entries[char_idx].level % 100;
                            s_party_entries[char_idx].level_over_100 = 1;
                        }
                    }

                    entry_offset += 14;
                    char_idx++;
                }
                s_party_count = char_idx;
            }
            break;

        case 0x4A:
            /* Pet data */
            LOG_DEBUG("Binary packet: Pet data (0x4A)");
            {
                int pet_idx = (int)data[1] - '0';
                if (pet_idx >= 0 && pet_idx < MAX_PETS) {
                    u32 field1 = (u32)pet_parse_field_int(packet_data, '|', 1);

                    if (field1 == 0) {
                        s_pet_data[pet_idx].active = 0;
                    } else {
                        s_pet_data[pet_idx].active = 1;
                        s_pet_data[pet_idx].hp = (u16)pet_parse_field_int(packet_data, '|', 2);
                        s_pet_data[pet_idx].max_hp = (u16)pet_parse_field_int(packet_data, '|', 3);
                        s_pet_data[pet_idx].level = (u16)pet_parse_field_int(packet_data, '|', 4);

                        if (s_pet_data[pet_idx].level >= 100) {
                            s_pet_data[pet_idx].level_display = s_pet_data[pet_idx].level % 100;
                            s_pet_data[pet_idx].level_over_100 = 1;
                        }

                        {
                            char name_buf[32];
                            pet_parse_field(packet_data, '|', 5, name_buf, sizeof(name_buf));
                            pet_unescape_string(name_buf);
                            strncpy(s_pet_data[pet_idx].name, name_buf, 25);
                        }
                    }
                }
            }
            break;

        case 0x4B:
            /* Pet status update with bitmask */
            LOG_DEBUG("Binary packet: Pet status (0x4B)");
            {
                int pet_idx = (int)data[1] - '0';
                u32 bitmask = (u32)pet_parse_field_base62(packet_data, '|', 1);

                if (pet_idx >= 0 && pet_idx < MAX_PETS) {
                    if (bitmask == 0) {
                        if (s_pet_data[pet_idx].active) {
                            s_pet_data[pet_idx].active = 0;
                            if (s_selected_pet == pet_idx) s_selected_pet = -1;
                            if (s_battle_pet == pet_idx) s_battle_pet = -1;
                        }
                    } else {
                        s_pet_data[pet_idx].active = 1;
                        /* Parse bitmask fields - similar to case 0x50 */
                    }
                }
            }
            break;

        case 0x4D:
            /* Battle ready/pet auto */
            LOG_DEBUG("Binary packet: Battle ready (0x4D)");
            {
                s_battle_field1 = pet_parse_field_int(packet_data, '|', 1);
                s_battle_field2 = pet_parse_field_int(packet_data, '|', 2);
                s_battle_field3 = pet_parse_field_int(packet_data, '|', 3);
            }
            break;

        case 0x4E:
            /* Battle unit status */
            LOG_DEBUG("Binary packet: Battle unit (0x4E)");
            protocol_handle_battle_unit_status(packet_data);
            break;

        case 0x50:
            /* Character detailed stats */
            LOG_DEBUG("Binary packet: Character stats (0x50)");
            character_parse_detailed_stats((const char*)data);
            break;

        case 0x53:
            /* Skill list */
            LOG_DEBUG("Binary packet: Skill list (0x53)");
            skill_parse_list_text(packet_data);
            break;

        case 0x57:
            /* Pet skill data */
            LOG_DEBUG("Binary packet: Pet skill (0x57)");
            {
                int pet_idx = (int)data[1] - '0';
                if (pet_idx >= 0 && pet_idx < MAX_PETS) {
                    memset(&s_pet_skills[pet_idx], 0, sizeof(PetSkillEntry) * MAX_PET_SKILLS);

                    int skill_offset = 1;
                    int skill_idx = 0;

                    while (skill_offset < 0x24 && skill_idx < MAX_PET_SKILLS) {
                        char skill_name[32];
                        pet_parse_field(packet_data, '|', skill_offset + 3, skill_name, sizeof(skill_name));
                        pet_unescape_string(skill_name);

                        if (skill_name[0] != '\0') {
                            strncpy(s_pet_skills[pet_idx][skill_idx].name, skill_name, 25);
                            s_pet_skills[pet_idx][skill_idx].field1 = (u16)pet_parse_field_int(packet_data, '|', skill_offset);
                            s_pet_skills[pet_idx][skill_idx].field2 = (u16)pet_parse_field_int(packet_data, '|', skill_offset + 1);
                            s_pet_skills[pet_idx][skill_idx].field3 = (u16)pet_parse_field_int(packet_data, '|', skill_offset + 2);
                            skill_idx++;
                        }

                        skill_offset += 5;
                    }
                    s_pet_skill_count[pet_idx] = skill_idx;
                }
            }
            break;

        case 0x58:
            /* Value update */
            LOG_DEBUG("Binary packet: Value update (0x58)");
            {
                s_update_value = pet_parse_field_int(packet_data, '|', 2);
            }
            break;

        default:
            LOG_DEBUG("Binary packet: Unknown type 0x%02X", packet_type);
            break;
    }

    network_buffer_remove(len);
}
