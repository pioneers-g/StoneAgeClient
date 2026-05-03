/*
 * Stone Age Client - Battle Protocol Handlers
 * Reverse engineered from sa_9061.exe
 * Split from battle.c for code organization
 *
 * Handles text protocol battle commands:
 * - Battle start/end
 * - Turn management
 * - Damage/results
 * - Capture
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "battle.h"
#include "network.h"
#include "logger.h"
#include "protocol_util.h"

/* External battle context */
extern BattleContext g_battle;

/* Battle unit slots - corresponds to DAT_046308f0 region (up to 20 units) */
/* Each slot has: active flag (2 bytes), char_id (4 bytes), etc. */
#define MAX_BATTLE_UNIT_SLOTS 20

/* Battle unit slot data - from FUN_0045ffb0 case 0x4e */
typedef struct {
    u16 active;           /* DAT_046308f0 + slot*0x30 */
    u32 char_id;          /* DAT_046308f4 + slot*0xc */
    u32 field_08;         /* DAT_046308f8 + slot*0x30 */
    u16 level;            /* DAT_046308fc + slot*0xc */
    u16 hp;               /* DAT_04630900 + slot*0xc */
    u16 status;           /* DAT_04630904 + slot*0xc */
    char name[17];        /* DAT_04630908 + slot*0xc (16 chars + null) */
    u32 entity_ptr;       /* DAT_0463091c + slot*0xc */
} BattleUnitSlot;

/* Global battle unit slots - corresponds to DAT_046308f0 region */
static BattleUnitSlot s_battle_slots[MAX_BATTLE_UNIT_SLOTS];
static u32 s_battle_slots_active = 0;  /* DAT_04630b24 */
static u32 s_battle_slots_count = 0;   /* DAT_04630b1c */

/* Player character ID - DAT_0462be90 */
static u32 s_player_char_id = 0;

/* Forward declarations */
static u16 packet_checksum(const u8* data, u16 len);

/*
 * Handle text protocol battle start
 */
void battle_handle_text_start(u32 param1, u32 param2, u32 param3) {
    g_battle.battle_id = param1;
    g_battle.state = BATTLE_STATE_INIT;
    battle_on_start(param1, param2);

    LOG_DEBUG("Battle start text: id=%d, field=%d, p3=%d", param1, param2, param3);
}

/*
 * Handle text protocol battle action
 */
void battle_handle_action_text(u32 attacker, u32 action, u32 target, u32 param, const char* extra) {
    g_battle.current_actor = attacker;
    g_battle.current_action = (BattleAction)action;
    g_battle.action_target = target;
    g_battle.action_param1 = param;

    LOG_DEBUG("Battle action text: %u -> %u, action=%u", attacker, target, action);
}

/*
 * Handle text protocol battle turn
 */
void battle_handle_turn_text(u32 turn, u32 param2, u32 param3, u32 param4) {
    g_battle.turn_count = turn;
    LOG_DEBUG("Battle turn %u (p2=%u, p3=%u, p4=%u)", turn, param2, param3, param4);
}

/*
 * Handle text protocol battle end
 */
void battle_handle_end_text(u32 result, u32 exp, u32 gold, u32 items) {
    g_battle.victory = (result == 0) ? 1 : 0;
    g_battle.exp_reward = exp;
    g_battle.gold_reward = gold;
    g_battle.ended = 1;

    battle_on_end(result, exp, gold, items, result);

    LOG_DEBUG("Battle end: result=%u, exp=%u, gold=%u", result, exp, gold);
}

/*
 * Handle text protocol capture result
 */
void battle_handle_capture_text(u32 pet_id, u32 result) {
    g_battle.capture_pet_id = pet_id;
    g_battle.capture_flags = (u16)result;

    LOG_DEBUG("Capture result: pet=%u, result=%u", pet_id, result);
}

/*
 * Handle text protocol damage
 */
void battle_handle_damage_text(const char* damage_data) {
    /* Parse damage data format: "attacker|target|damage|flags|skill_id" */
    char buf[256];
    char* token;
    char* saveptr = NULL;
    int values[5] = {0};
    int count = 0;

    if (!damage_data) return;

    strncpy(buf, damage_data, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

#ifdef _WIN32
    token = strtok_s(buf, "|", &saveptr);
    while (token && count < 5) {
        values[count++] = atoi(token);
        token = strtok_s(NULL, "|", &saveptr);
    }
#else
    token = strtok_r(buf, "|", &saveptr);
    while (token && count < 5) {
        values[count++] = atoi(token);
        token = strtok_r(NULL, "|", &saveptr);
    }
#endif

    if (count >= 3) {
        battle_handle_result(
            (u32)values[0],  /* attacker */
            (u32)values[1],  /* target */
            (u16)values[2],  /* damage */
            (u16)(count > 3 ? values[3] : 0),  /* flags */
            (u32)(count > 4 ? values[4] : 0)   /* skill_id */
        );
    }

    LOG_DEBUG("Damage text: %s", damage_data);
}

/*
 * Handle text protocol level up
 */
void battle_handle_level_text(const char* level_data) {
    /* Parse level up data */
    LOG_INFO("Level up: %s", level_data ? level_data : "(none)");
}

/*
 * Handle text protocol exp gain
 */
void battle_handle_exp_text(const char* exp_data) {
    /* Parse exp gain data */
    LOG_DEBUG("Exp text: %s", exp_data ? exp_data : "(none)");
}

/*
 * Handle text protocol magic/skill
 */
void battle_handle_magic_text(u32 skill_id, u32 param) {
    LOG_DEBUG("Magic text: skill=%u, param=%u", skill_id, param);
}

/*
 * Handle text protocol turn begin
 */
void battle_handle_turn_begin_text(u32 turn) {
    g_battle.turn_count = turn;
    g_battle.state = BATTLE_STATE_MAIN;

    LOG_DEBUG("Turn %u begin", turn);
}

/*
 * Handle text protocol counter attack
 */
void battle_handle_counter_text(u32 attacker, u32 target, const char* extra) {
    LOG_DEBUG("Counter text: %u -> %u, extra=%s", attacker, target, extra ? extra : "(none)");
}

/*
 * Handle text protocol battle command
 */
void battle_handle_text_command(u32 param1, u32 param2) {
    LOG_DEBUG("Battle command: %u, %u", param1, param2);
}

/*
 * Send text battle command - FUN_0043b980
 */
void battle_send_text_command(SOCKET sock, u32 x, u32 y, u32 param1, u32 param2, u32 flags, const char* extra) {
    char buffer[1024];

    /* Build command string */
    if (extra && extra[0] != '\0') {
        snprintf(buffer, sizeof(buffer), "B %d %d %d %d %d %s",
                 x, y, param1, param2, flags, extra);
    } else {
        snprintf(buffer, sizeof(buffer), "B %d %d %d %d %d",
                 x, y, param1, param2, flags);
    }

    /* Send via network */
    network_send(buffer, strlen(buffer));
    network_send("\n", 1);

    LOG_DEBUG("Sent battle command: %s", buffer);
}

/*
 * Send binary battle command - FUN_0048fdc0
 */
void battle_send_binary_command(SOCKET sock, u32 opcode, u32 param1, u32 param2,
                                 u32 param3, u32 param4, u32 param5, u32 checksum) {
    u8 buffer[32];
    u16 size = 28;

    /* Build packet header */
    *(u16*)(buffer + 0) = size;
    *(u16*)(buffer + 2) = (u16)opcode;
    *(u16*)(buffer + 4) = 0;  /* Checksum placeholder */

    /* Build payload */
    *(u32*)(buffer + 6) = param1;
    *(u32*)(buffer + 10) = param2;
    *(u32*)(buffer + 14) = param3;
    *(u32*)(buffer + 18) = param4;
    *(u32*)(buffer + 22) = param5;
    *(u32*)(buffer + 26) = checksum;

    /* Calculate checksum */
    *(u16*)(buffer + 4) = packet_checksum(buffer + 6, size - 6);

    /* Send packet */
    network_send(buffer, size);

    LOG_DEBUG("Sent binary battle command: opcode=%u", opcode);
}

/*
 * Simple checksum calculation
 */
static u16 packet_checksum(const u8* data, u16 len) {
    u16 sum = 0;
    u16 i;

    for (i = 0; i < len; i += 2) {
        if (i + 1 < len) {
            sum += *(u16*)(data + i);
        } else {
            sum += data[i];
        }
    }

    return sum;
}

/*
 * Find battle unit slot by character ID
 * Returns slot index or -1 if not found
 *
 * Binary analysis from FUN_0040e830:
 * - Searches through slots array starting from last accessed slot
 * - Compares char_id field
 * - Caches result in DAT_004e2b0c for optimization
 */
int battle_find_unit_slot(u32 char_id) {
    int i;

    /* Search starting from last position */
    for (i = 0; i < MAX_BATTLE_UNIT_SLOTS; i++) {
        if (s_battle_slots[i].active && s_battle_slots[i].char_id == char_id) {
            s_battle_slots_count = i;  /* Cache result */
            return i;
        }
    }

    return -1;
}

/*
 * Get entity pointer by character ID
 * FUN_0040f460
 */
void* battle_get_entity_by_char_id(u32 char_id) {
    int slot = battle_find_unit_slot(char_id);
    if (slot < 0) {
        return NULL;
    }
    return (void*)s_battle_slots[slot].entity_ptr;
}

/*
 * Set entity active flag
 * FUN_0040c080 - Sets flag 0x200
 */
void battle_entity_set_active(void* entity) {
    if (entity) {
        /* Set flag at offset based on entity structure */
        /* In binary: (&DAT_004e2b4c)[slot*0x43] |= 0x200 */
        BattleUnit* unit = (BattleUnit*)entity;
        unit->is_active = 1;
    }
}

/*
 * Clear entity active flag
 * FUN_0040c0b0 - Clears flag 0x200
 */
void battle_entity_clear_active(void* entity) {
    if (entity) {
        BattleUnit* unit = (BattleUnit*)entity;
        unit->is_active = 0;
    }
}

/*
 * Remove battle unit from slot
 *
 * Binary analysis from FUN_0045ffb0 case 0x4e when bitmask == 0:
 * - Checks if slot has entity pointer
 * - If entity belongs to player, clears player reference
 * - Frees entity memory via FUN_004011c0
 * - Clears slot active flag
 * - Updates active count
 * - If count < 2, clears battle flag and calls FUN_00478910
 */
void battle_unit_remove(int slot_index) {
    BattleUnitSlot* slot;
    int active_count;
    int i;

    if (slot_index < 0 || slot_index >= MAX_BATTLE_UNIT_SLOTS) {
        return;
    }

    slot = &s_battle_slots[slot_index];

    if (!slot->active) {
        return;  /* Already inactive */
    }

    /* Clear entity if exists */
    if (slot->entity_ptr) {
        /* Check if this is player's entity */
        if (slot->char_id == s_player_char_id) {
            s_player_char_id = 0;
        }
        /* Free entity memory - in real implementation would call free() */
        slot->entity_ptr = 0;
    }

    /* Clear slot */
    slot->active = 0;
    slot->char_id = 0;
    slot->entity_ptr = 0;
    memset(slot->name, 0, sizeof(slot->name));

    /* Count remaining active units */
    active_count = 0;
    for (i = 0; i < MAX_BATTLE_UNIT_SLOTS; i++) {
        if (s_battle_slots[i].active) {
            active_count++;
        }
    }

    /* If less than 2 units, battle should end */
    if (active_count < 2) {
        s_battle_slots_active = 0;
        g_battle.active = 0;
        g_battle.ended = 1;
        LOG_DEBUG("Battle units removed, remaining: %d", active_count);
    }
}

/*
 * Update battle unit status
 *
 * Binary analysis from FUN_0045ffb0 case 0x4e:
 *
 * bitmask == 0: Remove unit
 * bitmask == 1: Update all fields (fields 2-7)
 * bitmask bits:
 *   Bit 1 (0x02): char_id (field 2)
 *   Bit 2 (0x04): field_08 (field 3)
 *   Bit 3 (0x08): level (field 4)
 *   Bit 4 (0x10): hp (field 5)
 *   Bit 5 (0x20): status (field 6)
 *   Bit 6 (0x40): name (field 7)
 *
 * After parsing, if char_id matches player, updates player entity reference
 */
void battle_unit_status_update(int slot_index, u32 bitmask, const char* data) {
    BattleUnitSlot* slot;
    int field_offset;
    char name_buf[32];

    if (slot_index < 0 || slot_index >= MAX_BATTLE_UNIT_SLOTS) {
        return;
    }

    slot = &s_battle_slots[slot_index];

    /* Handle removal */
    if (bitmask == 0) {
        battle_unit_remove(slot_index);
        return;
    }

    /* Mark slot as active */
    slot->active = 1;
    s_battle_slots_active = 1;
    s_battle_slots_count = 0;  /* Reset counter */

    /* Update player reference if this is player's char_id */
    if (slot->char_id == s_player_char_id) {
        slot->entity_ptr = s_player_char_id;  /* Placeholder */
    }

    /* Parse fields based on bitmask */
    field_offset = 2;  /* Field 1 is bitmask, fields start at 2 */

    if (bitmask == 1) {
        /* All fields present - sequential parse */
        slot->char_id = parse_field_int(data, '|', 2);
        slot->field_08 = parse_field_int(data, '|', 3);
        slot->level = (u16)parse_field_int(data, '|', 4);
        slot->hp = (u16)parse_field_int(data, '|', 5);
        slot->status = (u16)parse_field_int(data, '|', 6);

        /* Parse name */
        parse_text_field(data, '|', 7, 16, slot->name);
        slot->name[16] = '\0';

        LOG_DEBUG("Battle unit slot %d: char=%u, level=%u, hp=%u, name=%s",
                 slot_index, slot->char_id, slot->level, slot->hp, slot->name);
    } else {
        /* Selective field parse based on bitmask */
        if (bitmask & BATTLE_UNIT_UPDATE_ID) {
            slot->char_id = parse_field_int(data, '|', field_offset);
            field_offset++;
        }
        if (bitmask & BATTLE_UNIT_UPDATE_HP) {
            slot->field_08 = parse_field_int(data, '|', field_offset);
            field_offset++;
        }
        if (bitmask & BATTLE_UNIT_UPDATE_MP) {
            slot->level = (u16)parse_field_int(data, '|', field_offset);
            field_offset++;
        }
        if (bitmask & BATTLE_UNIT_UPDATE_LEVEL) {
            slot->hp = (u16)parse_field_int(data, '|', field_offset);
            field_offset++;
        }
        if (bitmask & BATTLE_UNIT_UPDATE_STATUS) {
            slot->status = (u16)parse_field_int(data, '|', field_offset);
            field_offset++;
        }
        if (bitmask & BATTLE_UNIT_UPDATE_NAME) {
            parse_text_field(data, '|', field_offset, 16, slot->name);
            slot->name[16] = '\0';
        }
    }

    /* Update g_battle unit array */
    if (slot_index < 20) {
        BattleUnit* unit = &g_battle.units[slot_index];
        unit->id = slot_index;
        unit->char_id = slot->char_id;
        unit->level = slot->level;
        unit->hp = slot->hp;
        unit->is_alive = (slot->hp > 0) ? 1 : 0;
        strncpy(unit->name, slot->name, 16);
        unit->name[16] = '\0';
    }
}

/*
 * Initialize battle slots
 */
void battle_slots_init(void) {
    memset(s_battle_slots, 0, sizeof(s_battle_slots));
    s_battle_slots_active = 0;
    s_battle_slots_count = 0;
}

/*
 * Set player character ID
 */
void battle_set_player_char_id(u32 char_id) {
    s_player_char_id = char_id;
}

/*
 * Get battle unit slot data
 */
BattleUnitSlot* battle_get_slot(int index) {
    if (index < 0 || index >= MAX_BATTLE_UNIT_SLOTS) {
        return NULL;
    }
    return &s_battle_slots[index];
}
