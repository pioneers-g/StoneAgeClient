/*
 * Stone Age Client - Character Protocol Parsing
 * Reverse engineered from sa_9061.exe (FUN_0045ffb0, FUN_00464670, FUN_00464190)
 *
 * Protocol parsing for character data packets
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "character.h"
#include "pet_protocol.h"
#include "logger.h"

/* Character entry array - DAT_0462c028 region */
static CharacterEntry s_char_entries[MAX_CHARACTERS];
static u32 s_char_entry_count = 0;

/* Global detailed stats from server - FUN_0045ffb0 case 0x50 */
DetailedCharacterStats g_detailed_stats = {0};

/*
 * Get character entry by index
 */
CharacterEntry* character_get_entry(u32 index) {
    if (index >= MAX_CHARACTERS) return NULL;
    return &s_char_entries[index];
}

/*
 * Parse character list from packet - FUN_00464670 pattern
 * Packet format: |char_id|name|field3|...|fieldN|
 */
void character_parse_list(const char* packet_data) {
    const char* ptr = packet_data;
    char buffer[256];
    char name_buffer[64];
    int char_id;
    int field_offset;
    s16 level_val;
    int result;

    if (!packet_data) return;

    /* Parse first character ID - FUN_00464670 pattern */
    char_id = pet_parse_field_int(ptr, '|', 1);
    if (char_id < 0) return;

    /* Parse character name */
    result = pet_parse_field(ptr, '|', 2, buffer, sizeof(buffer));
    if (result != 0) return;
    pet_unescape_string(buffer);

    /* Check if empty - marks entry as inactive */
    if (buffer[0] == '\0') {
        s_char_entries[char_id].name[0] = '\0';
        return;
    }

    /* Store name (max 29 bytes = 0x1d from FUN_00464670) */
    strncpy(s_char_entries[char_id].name, buffer, 28);
    s_char_entries[char_id].name[28] = '\0';

    /* Field 3: account name (max 17 bytes = 0x11) */
    pet_parse_field(ptr, '|', 3, buffer, sizeof(buffer));
    pet_unescape_string(buffer);
    if (strlen(buffer) < 17) {
        strncpy(s_char_entries[char_id].field_0x3a, buffer, 16);
        s_char_entries[char_id].field_0x3a[16] = '\0';
    }

    /* Field 4: some integer (stored at DAT_0462bf4c offset) */
    s_char_entries[char_id].some_field = pet_parse_field_int(ptr, '|', 4);
    if (s_char_entries[char_id].some_field < 0) {
        s_char_entries[char_id].some_field = 0;
    }

    /* Field 5: description (max 85 bytes = 0x55) */
    pet_parse_field(ptr, '|', 5, buffer, sizeof(buffer));
    pet_unescape_string(buffer);
    if (strlen(buffer) < 85) {
        strncpy(s_char_entries[char_id].description, buffer, 84);
        s_char_entries[char_id].description[84] = '\0';
    }

    /* Field 6: some ID (stored at DAT_0462bf50 offset) */
    s_char_entries[char_id].field_0x0c = (u32)pet_parse_field_int(ptr, '|', 6);

    /* Field 7: direction (stored at DAT_0462c02a offset) */
    s_char_entries[char_id].direction = (u16)pet_parse_field_int(ptr, '|', 7);

    /* Field 8: level (stored at DAT_0462c02c, with mod 100 handling) */
    level_val = (s16)pet_parse_field_int(ptr, '|', 8);
    if (level_val < 100) {
        s_char_entries[char_id].level = level_val;
        s_char_entries[char_id].level_over_100 = 0;
    } else {
        s_char_entries[char_id].level = level_val % 100;
        s_char_entries[char_id].level_over_100 = 1;
    }

    /* Field 9: Y position (stored at DAT_0462bf54 offset) */
    s_char_entries[char_id].y = (u16)pet_parse_field_int(ptr, '|', 9);

    /* Field 10: sprite ID (stored at DAT_0462c030 offset) */
    s_char_entries[char_id].sprite_id = (u16)pet_parse_field_int(ptr, '|', 10);

    /* Field 11: short string (max 17 bytes, stored at DAT_0462c0b5) */
    pet_parse_field(ptr, '|', 11, buffer, sizeof(buffer));
    pet_unescape_string(buffer);
    if (strlen(buffer) < 17) {
        strncpy(s_char_entries[char_id].field_0x10, buffer, 16);
        s_char_entries[char_id].field_0x10[16] = '\0';
    }

    /* Field 12: another string value */
    pet_parse_field(ptr, '|', 12, buffer, sizeof(buffer));
    pet_unescape_string(buffer);
    /* This is stored but we need to add a field for it */

    /* Field 13: portrait/guild short string (max 7 bytes) */
    pet_parse_field(ptr, '|', 13, buffer, 8);
    pet_unescape_string(buffer);

    /* Field 14: title string (max 9 bytes) */
    pet_parse_field(ptr, '|', 14, buffer, 10);
    pet_unescape_string(buffer);

    LOG_DEBUG("Parsed character %d: %s at (%d,%d), level %d, sprite %d",
              char_id, s_char_entries[char_id].name,
              s_char_entries[char_id].x, s_char_entries[char_id].y,
              s_char_entries[char_id].level, s_char_entries[char_id].sprite_id);

    if (char_id >= (int)s_char_entry_count) {
        s_char_entry_count = char_id + 1;
    }

    /* Check for additional characters in packet (multi-character parsing) */
    field_offset = 15;
    while (1) {
        /* Parse next character index */
        int next_id = pet_parse_field_int(ptr, '|', field_offset);
        if (next_id < 0) break;

        /* Parse next character name */
        result = pet_parse_field(ptr, '|', field_offset + 1, buffer, sizeof(buffer));
        if (result != 0) break;
        pet_unescape_string(buffer);

        if (buffer[0] == '\0') break;

        /* Recursively parse the remaining fields by adjusting the offset */
        /* In the original, this loops through all character entries in one packet */
        field_offset += 14;  /* Each character entry has ~14 fields */

        if (next_id < MAX_CHARACTERS) {
            strncpy(s_char_entries[next_id].name, buffer, 28);
            s_char_entries[next_id].name[28] = '\0';
            if (next_id >= (int)s_char_entry_count) {
                s_char_entry_count = next_id + 1;
            }
        }
    }
}

/*
 * Handle character teleport - FUN_00464190 pattern
 */
void character_teleport(u32 char_index, const char* packet_data) {
    int char_id;
    char buffer[256];

    if (!packet_data) return;

    /* Parse character ID */
    char_id = pet_parse_field_int(packet_data, '|', 1);

    if (char_id == -1 || char_id == 0) {
        /* Clear character entry */
        if (s_char_entries[char_index].name[0] != '\0') {
            memset(&s_char_entries[char_index], 0, sizeof(CharacterEntry));
        }
        return;
    }

    /* Parse name */
    pet_parse_field(packet_data, '|', 2, buffer, sizeof(buffer));
    pet_unescape_string(buffer);

    if (buffer[0] != '\0') {
        strncpy(s_char_entries[char_index].name, buffer, 28);
        s_char_entries[char_index].name[28] = '\0';
    }

    /* Parse position */
    s_char_entries[char_index].x = (u16)pet_parse_field_int(packet_data, '|', 3);
    s_char_entries[char_index].y = (u16)pet_parse_field_int(packet_data, '|', 4);
    s_char_entries[char_index].direction = (u16)pet_parse_field_int(packet_data, '|', 5);
    s_char_entries[char_index].sprite_id = (u16)pet_parse_field_int(packet_data, '|', 6);

    LOG_DEBUG("Teleported character %d to (%d,%d)", char_index,
              s_char_entries[char_index].x, s_char_entries[char_index].y);
}

/*
 * Handle move result - FUN_00465170 pattern
 */
void character_handle_move_result(u32 char_index, const char* packet_data) {
    Character* ch;
    int field_val;

    if (!packet_data || char_index >= MAX_CHARACTERS) return;

    ch = &g_chars.characters[char_index];

    /* Parse movement fields */
    field_val = pet_parse_field_int(packet_data, '|', 1);
    if (field_val == -1) {
        /* Movement failed or invalid */
        return;
    }

    /* Update position */
    ch->dest_x = (u16)field_val;
    ch->dest_y = (u16)pet_parse_field_int(packet_data, '|', 2);
    ch->is_moving = 1;

    LOG_DEBUG("Character %d moving to (%d,%d)", char_index, ch->dest_x, ch->dest_y);
}

/*
 * Parse detailed character stats - FUN_0045ffb0 case 0x50
 * Packet format: 0x50|field1|field2|...|field31|
 *
 * Field 1: Bitmask indicating which fields are present
 * Fields 2-29: Integer values (u32)
 * Field 30: String field (max 0x11 = 17 bytes) - title
 * Field 31: String field (max 0x21 = 33 bytes) - guild name
 *
 * If bitmask == 1: all fields present, parse sequentially
 * Otherwise: use bitmask to determine which fields to parse
 */
void character_parse_detailed_stats(const char* packet_data) {
    const char* ptr = packet_data;
    char str_buffer[64];
    u32 field_mask;
    int field_index;
    u32 bit_flag;

    if (!packet_data) return;

    /* Skip packet type byte (0x50) */
    ptr += 1;

    /* Field 1: Bitmask - FUN_0048a120 uses Base-62 decoding */
    field_mask = (u32)pet_parse_field_base62(ptr, '|', 1);
    g_detailed_stats.field_mask = field_mask;

    if (field_mask == 1) {
        /* All fields present - parse sequentially from field 2 */
        g_detailed_stats.field_02 = (u32)pet_parse_field_int(ptr, '|', 2);
        g_detailed_stats.field_03 = (u32)pet_parse_field_int(ptr, '|', 3);
        g_detailed_stats.field_04 = (u32)pet_parse_field_int(ptr, '|', 4);
        g_detailed_stats.field_05 = (u32)pet_parse_field_int(ptr, '|', 5);
        g_detailed_stats.field_06 = (u32)pet_parse_field_int(ptr, '|', 6);
        g_detailed_stats.field_07 = (u32)pet_parse_field_int(ptr, '|', 7);
        g_detailed_stats.field_08 = (u32)pet_parse_field_int(ptr, '|', 8);
        g_detailed_stats.field_09 = (u32)pet_parse_field_int(ptr, '|', 9);
        g_detailed_stats.field_10 = (u32)pet_parse_field_int(ptr, '|', 10);
        g_detailed_stats.field_11 = (u32)pet_parse_field_int(ptr, '|', 11);
        g_detailed_stats.field_12 = (u32)pet_parse_field_int(ptr, '|', 12);
        g_detailed_stats.field_13 = (u32)pet_parse_field_int(ptr, '|', 13);
        g_detailed_stats.field_14 = (u32)pet_parse_field_int(ptr, '|', 14);
        g_detailed_stats.field_15 = (u32)pet_parse_field_int(ptr, '|', 15);
        g_detailed_stats.field_16 = (u32)pet_parse_field_int(ptr, '|', 16);
        g_detailed_stats.field_17 = (u32)pet_parse_field_int(ptr, '|', 17);
        g_detailed_stats.field_18 = (u32)pet_parse_field_int(ptr, '|', 18);
        g_detailed_stats.field_19 = (u32)pet_parse_field_int(ptr, '|', 19);
        g_detailed_stats.field_20 = (u32)pet_parse_field_int(ptr, '|', 20);
        g_detailed_stats.field_21 = (u32)pet_parse_field_int(ptr, '|', 21);
        g_detailed_stats.field_22 = (u32)pet_parse_field_int(ptr, '|', 22);
        g_detailed_stats.field_23 = (u32)pet_parse_field_int(ptr, '|', 23);
        g_detailed_stats.field_24 = (u32)pet_parse_field_int(ptr, '|', 24);
        g_detailed_stats.field_25 = (u32)pet_parse_field_int(ptr, '|', 25);

        /* Field 26 (0x1a): stored internally */
        g_detailed_stats.field_26 = (u32)pet_parse_field_int(ptr, '|', 26);

        /* Field 27 (0x1b): stored internally */
        g_detailed_stats.field_27 = (u32)pet_parse_field_int(ptr, '|', 27);

        /* Field 28 (0x1c): stored internally */
        g_detailed_stats.field_28 = (u32)pet_parse_field_int(ptr, '|', 28);

        /* Field 29 (0x1d): stored internally */
        g_detailed_stats.field_29 = (u32)pet_parse_field_int(ptr, '|', 29);

        /* Field 30 (0x1e): Title string (max 0x11 = 17 bytes) */
        pet_parse_field(ptr, '|', 30, str_buffer, sizeof(str_buffer));
        pet_unescape_string(str_buffer);
        strncpy(g_detailed_stats.title, str_buffer, 16);
        g_detailed_stats.title[16] = '\0';

        /* Field 31 (0x1f): Guild name (max 0x21 = 33 bytes) */
        pet_parse_field(ptr, '|', 31, str_buffer, sizeof(str_buffer));
        pet_unescape_string(str_buffer);
        strncpy(g_detailed_stats.guild_name, str_buffer, 32);
        g_detailed_stats.guild_name[32] = '\0';

    } else {
        /* Bitmask mode - parse only fields indicated by bits */
        field_index = 2;  /* Field index starts at 2 */
        bit_flag = 0x2;   /* Start with bit 1 (value 0x2) */

        while (bit_flag != 0) {
            if (field_mask & bit_flag) {
                /* This field is present */
                u32 value = (u32)pet_parse_field_int(ptr, '|', field_index);

                /* Store value based on bit flag */
                switch (bit_flag) {
                    case STATS_FIELD_02: g_detailed_stats.field_02 = value; break;
                    case STATS_FIELD_03: g_detailed_stats.field_03 = value; break;
                    case STATS_FIELD_04: g_detailed_stats.field_04 = value; break;
                    case STATS_FIELD_05: g_detailed_stats.field_05 = value; break;
                    case STATS_FIELD_06: g_detailed_stats.field_06 = value; break;
                    case STATS_FIELD_07: g_detailed_stats.field_07 = value; break;
                    case STATS_FIELD_08: g_detailed_stats.field_08 = value; break;
                    case STATS_FIELD_09: g_detailed_stats.field_09 = value; break;
                    case STATS_FIELD_10: g_detailed_stats.field_10 = value; break;
                    case STATS_FIELD_11: g_detailed_stats.field_11 = value; break;
                    case STATS_FIELD_12: g_detailed_stats.field_12 = value; break;
                    case STATS_FIELD_13: g_detailed_stats.field_13 = value; break;
                    case STATS_FIELD_14: g_detailed_stats.field_14 = value; break;
                    case STATS_FIELD_15: g_detailed_stats.field_15 = value; break;
                    case STATS_FIELD_16: g_detailed_stats.field_16 = value; break;
                    case STATS_FIELD_17: g_detailed_stats.field_17 = value; break;
                    case STATS_FIELD_18: g_detailed_stats.field_18 = value; break;
                    case STATS_FIELD_19: g_detailed_stats.field_19 = value; break;
                    case STATS_FIELD_20: g_detailed_stats.field_20 = value; break;
                    case STATS_FIELD_21: g_detailed_stats.field_21 = value; break;
                    case STATS_FIELD_22: g_detailed_stats.field_22 = value; break;
                    case STATS_FIELD_23: g_detailed_stats.field_23 = value; break;
                    case STATS_FIELD_24: g_detailed_stats.field_24 = value; break;
                    case STATS_FIELD_25: g_detailed_stats.field_25 = value; break;
                    case STATS_FIELD_26_STR:
                        /* String field - title */
                        pet_parse_field(ptr, '|', field_index, str_buffer, sizeof(str_buffer));
                        pet_unescape_string(str_buffer);
                        strncpy(g_detailed_stats.title, str_buffer, 16);
                        g_detailed_stats.title[16] = '\0';
                        break;
                    case STATS_FIELD_27_STR:
                        /* String field - guild name */
                        pet_parse_field(ptr, '|', field_index, str_buffer, sizeof(str_buffer));
                        pet_unescape_string(str_buffer);
                        strncpy(g_detailed_stats.guild_name, str_buffer, 32);
                        g_detailed_stats.guild_name[32] = '\0';
                        break;
                    case STATS_FIELD_28: g_detailed_stats.field_26 = value; break;
                    case STATS_FIELD_29: g_detailed_stats.field_27 = value; break;
                    case STATS_FIELD_30: g_detailed_stats.field_28 = value; break;
                    case STATS_FIELD_31: g_detailed_stats.field_29 = value; break;
                    default: break;
                }

                field_index++;
            }

            bit_flag <<= 1;
        }
    }

    LOG_DEBUG("Parsed detailed stats: mask=0x%08X, f2=%u, f3=%u, f4=%u",
              field_mask, g_detailed_stats.field_02, g_detailed_stats.field_03,
              g_detailed_stats.field_04);
}

/*
 * Update character positions from server
 * Called when server sends PKT_SV_CHAR_POSITIONS (0x1C)
 */
void character_update_positions(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 i;
    u32 char_id;
    u16 x, y;
    u16 direction;
    u16 action;
    u16 sprite_id;

    /* Parse position updates */
    while (ptr + 12 <= (u8*)data + size) {
        char_id = *(u32*)ptr; ptr += 4;
        x = *(u16*)ptr; ptr += 2;
        y = *(u16*)ptr; ptr += 2;
        direction = *(u16*)ptr; ptr += 2;
        action = *(u16*)ptr; ptr += 2;

        /* Find character and update */
        for (i = 0; i < g_chars.character_count; i++) {
            if (g_chars.characters[i].id == char_id) {
                g_chars.characters[i].x = x;
                g_chars.characters[i].y = y;
                g_chars.characters[i].direction = (u8)direction;
                g_chars.characters[i].action = (u8)action;
                break;
            }
        }
    }
}

/*
 * Handle action result from server
 * Called when server sends PKT_SV_ACTION_RESULT (0x27)
 */
void character_handle_action_result(u32 char_id, u16 action, u16 param1, u16 param2, u16 param3, u32 target_id) {
    Character* ch = character_find_by_id(char_id);

    if (!ch) return;

    switch (action) {
        case ACTION_ATTACK:
        case ACTION_CAST:
            ch->action = (u8)action;
            ch->frame = 0;
            break;
        case ACTION_HURT:
            ch->action = ACTION_HURT;
            ch->frame = 0;
            break;
        case ACTION_DIE:
            ch->action = ACTION_DIE;
            ch->frame = 0;
            break;
        default:
            break;
    }

    LOG_DEBUG("Character %u action %d result", char_id, action);
}

/*
 * Gain experience points
 * Called from battle system when exp is awarded
 */
void character_gain_exp(u32 exp) {
    Character* player = character_get_player();

    if (!player) return;

    player->stats.experience += exp;

    /* Check for level up */
    while (player->stats.experience >= player->stats.next_level_exp) {
        player->stats.experience -= player->stats.next_level_exp;
        player->stats.level++;
        player->stats.next_level_exp = (u32)(player->stats.next_level_exp * 1.5);

        LOG_INFO("Level up! Now level %d", player->stats.level);
    }
}

/*
 * Handle text protocol character update
 */
void character_handle_text_update(u32 char_id, const char* name, u32 param) {
    Character* ch;

    if (char_id == 0) {
        ch = character_get_player();
    } else {
        ch = character_find_by_id(char_id);
    }

    if (ch && name && name[0]) {
        strncpy(ch->name, name, sizeof(ch->name) - 1);
    }

    LOG_DEBUG("Character %u updated: %s", char_id, name ? name : "");
}

/*
 * Handle text protocol position update
 */
void character_handle_position_text(u32 char_id, u16 x, u16 y, u16 dir, u16 action, u16 sprite, u16 param6, u16 param7) {
    Character* ch;

    if (char_id == 0) {
        ch = character_get_player();
        if (ch) {
            ch->x = x;
            ch->y = y;
            ch->direction = (u8)dir;
            ch->action = (u8)action;
        }
    } else {
        ch = character_find_by_id(char_id);
        if (ch) {
            ch->x = x;
            ch->y = y;
            ch->direction = (u8)dir;
            ch->action = (u8)action;
            ch->appearance.base_sprite = sprite;
        }
    }
}

/*
 * Handle text protocol teleport
 */
void character_handle_teleport_text(u32 char_id, const char* map_name, u16 param) {
    Character* ch = (char_id == 0) ? character_get_player() : character_find_by_id(char_id);

    if (ch) {
        LOG_DEBUG("Character %u teleported to %s", char_id, map_name ? map_name : "unknown");
    }
}

/*
 * Handle text protocol character create result
 */
void character_handle_create_text(u32 param1, u32 param2, u32 param3, u32 param4,
                                   u32 param5, u32 param6, u32 param7, u32 param8,
                                   const char* name) {
    LOG_INFO("Character create result: %s", name ? name : "");
}

/*
 * Handle text protocol status update
 */
void character_handle_status_text(const char* status_data) {
    if (!status_data) return;

    /* Parse status fields */
    LOG_DEBUG("Status update: %s", status_data);
}
