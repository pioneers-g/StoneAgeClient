/*
 * Stone Age Client - Pet System Core
 * Reverse engineered from sa_9061.exe (FUN_00463c20, FUN_00463d20, FUN_0045fb80)
 *
 * Split modules:
 * - pet_ops.c: Pet operations (rename, release, feed, capture, skills)
 * - pet_battle.c: Battle-related functions
 * - pet_protocol.c: Protocol handlers
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "types.h"
#include "pet.h"
#include "pet_protocol.h"
#include "character.h"
#include "battle.h"
#include "network.h"
#include "logger.h"

/* Global pet context */
PetContext g_pet = {0};

/* Global pet data from server - FUN_0045ffb0 case 0x57 */
PetDataEntry57 g_pet_data_57[MAX_PETS_PER_CHAR] = {0};
int g_pet_data_57_count = 0;

/* Pet experience table - from FUN_0045fa40 analysis */
static const u32 s_exp_table[] = {
    0, 100, 250, 450, 700, 1000, 1400, 1900, 2500, 3200,
    4000, 4900, 5900, 7000, 8200, 9500, 11000, 12500, 14000, 15500,
    17000, 18500, 20000, 22000, 24000, 26000, 28000, 30000, 32000, 34000,
    37000, 40000, 43000, 46000, 49000, 52000, 55000, 58000, 61000, 64000,
    68000, 72000, 76000, 80000, 84000, 88000, 92000, 96000, 100000, 105000
};

/* Pet protocol commands */
#define PET_CMD_SKILL       "PT"
#define PET_CMD_ACTION      "P"
#define PET_CMD_RENAME      "PETNAME"
#define PET_CMD_RELEASE     "PETRELEASE"
#define PET_CMD_CAPTURE     "PETCAPTURE"
#define PET_CMD_SKILL_REQ   "PETSKILL"

/*
 * Helper: Send pet protocol packet
 */
static void pet_send_packet(const char* cmd, const char* params) {
    char packet[256];

    if (params && params[0]) {
        _snprintf(packet, sizeof(packet), "%s|%s", cmd, params);
    } else {
        strncpy(packet, cmd, sizeof(packet) - 1);
    }

    send_queue_add(packet, 0, 0);
}

/*
 * Helper: Check if data starts with expected string
 */
static int pet_check_response(const u8* data, u32 size, const char* expected) {
    int i;
    for (i = 0; expected[i] && i < (int)size; i++) {
        if (data[i] != (u8)expected[i]) {
            return 0;
        }
    }
    return 1;
}

/*
 * Initialize pet system
 */
int pet_init(void) {
    memset(&g_pet, 0, sizeof(PetContext));
    g_pet.selected_index = -1;
    g_pet.battle_pet_index = -1;

    LOG_INFO("Pet system initialized");
    return 1;
}

/*
 * Shutdown pet system
 */
void pet_shutdown(void) {
    if (g_pet.skills) {
        free(g_pet.skills);
        g_pet.skills = NULL;
    }
    if (g_pet.type_data) {
        free(g_pet.type_data);
        g_pet.type_data = NULL;
    }
    memset(&g_pet, 0, sizeof(PetContext));
    LOG_INFO("Pet system shutdown");
}

/*
 * Get pet by slot
 */
PetData* pet_get_slot(int slot) {
    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return NULL;
    }
    return &g_pet.slots[slot];
}

/*
 * Get selected pet
 */
PetData* pet_get_selected(void) {
    if (g_pet.selected_index < 0 || g_pet.selected_index >= MAX_PET_SLOTS) {
        return NULL;
    }
    return &g_pet.slots[g_pet.selected_index];
}

/*
 * Get battle pet
 */
PetData* pet_get_battle_pet(void) {
    if (g_pet.battle_pet_index < 0 || g_pet.battle_pet_index >= MAX_PET_SLOTS) {
        return NULL;
    }
    return &g_pet.slots[g_pet.battle_pet_index];
}

/*
 * Get pet by ID
 */
PetData* pet_get_by_id(u32 pet_id) {
    int i;

    if (pet_id == 0) {
        return NULL;
    }

    for (i = 0; i < MAX_PET_SLOTS; i++) {
        if (g_pet.slots[i].id == pet_id) {
            return &g_pet.slots[i];
        }
    }

    return NULL;
}

/*
 * Get pet slot index by ID
 */
int pet_get_slot_index(u32 pet_id) {
    int i;

    if (pet_id == 0) {
        return -1;
    }

    for (i = 0; i < MAX_PET_SLOTS; i++) {
        if (g_pet.slots[i].id == pet_id) {
            return i;
        }
    }

    return -1;
}

/*
 * Select pet by slot
 */
int pet_select(int slot) {
    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return 0;
    }
    if (g_pet.slots[slot].id == 0) {
        return 0;
    }

    g_pet.selected_index = slot;
    LOG_DEBUG("Selected pet in slot %d", slot);
    return 1;
}

/*
 * Set battle pet
 */
int pet_set_battle(int slot) {
    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return 0;
    }
    if (g_pet.slots[slot].id == 0) {
        return 0;
    }

    g_pet.battle_pet_index = slot;
    LOG_DEBUG("Set battle pet to slot %d", slot);
    return 1;
}

/*
 * Add pet to first available slot
 */
int pet_add(PetData* pet) {
    int i;

    if (!pet || pet->id == 0) {
        return -1;
    }

    for (i = 0; i < MAX_PET_SLOTS; i++) {
        if (g_pet.slots[i].id == 0) {
            memcpy(&g_pet.slots[i], pet, sizeof(PetData));
            g_pet.count++;
            LOG_INFO("Added pet %s to slot %d", pet->name, i);
            return i;
        }
    }

    LOG_WARN("No empty pet slots available");
    return -1;
}

/*
 * Remove pet from slot
 */
int pet_remove(int slot) {
    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return 0;
    }
    if (g_pet.slots[slot].id == 0) {
        return 0;
    }

    LOG_INFO("Removing pet %s from slot %d", g_pet.slots[slot].name, slot);

    memset(&g_pet.slots[slot], 0, sizeof(PetData));

    if (g_pet.selected_index == slot) {
        g_pet.selected_index = -1;
    }
    if (g_pet.battle_pet_index == slot) {
        g_pet.battle_pet_index = -1;
    }

    g_pet.count--;
    return 1;
}

/*
 * Handle pet list packet - FUN_00463c20 pattern
 */
void pet_handle_list_packet(void* data, u32 size) {
    u8* ptr = (u8*)data;
    int slot;
    PetData* pet;
    int i;
    char* name;
    char* result_str;
    int result;

    result_str = "successful";
    result = 1;
    for (i = 0; result_str[i] && i < (int)size; i++) {
        if (ptr[i] != result_str[i]) {
            result = 0;
            break;
        }
    }

    if (result) {
        g_pet.list_received = 1;
    }
}

/*
 * Handle pet status packet - FUN_00463d20 pattern
 */
void pet_handle_status_packet(void* data, u32 size) {
    if (pet_check_response((const u8*)data, size, "successful")) {
        g_pet.status_received = 1;
    }
}

/*
 * Handle pet name result - FUN_0045fb80 pattern
 */
void pet_handle_name_result(void* data, u32 size) {
    const char* ptr = (const char*)data;
    char buffer[64];
    int slot;
    char* name_start;
    char* name_end;

    if (pet_check_response((const u8*)data, size, "successful")) {
        g_pet.name_result = 1;

        name_start = strchr(ptr, '|');
        if (name_start) {
            name_start++;
            slot = atoi(name_start);

            name_start = strchr(name_start, '|');
            if (name_start) {
                name_start++;
                name_end = strchr(name_start, '|');
                if (name_end) {
                    int name_len = name_end - name_start;
                    if (name_len >= sizeof(buffer)) {
                        name_len = sizeof(buffer) - 1;
                    }
                    strncpy(buffer, name_start, name_len);
                    buffer[name_len] = '\0';

                    if (slot >= 0 && slot < MAX_PET_SLOTS) {
                        strncpy(g_pet.slots[slot].name, buffer,
                                sizeof(g_pet.slots[slot].name) - 1);
                        g_pet.slots[slot].name[sizeof(g_pet.slots[slot].name) - 1] = '\0';
                        LOG_DEBUG("Pet slot %d renamed to %s", slot, buffer);
                    }
                }
            }
        }
    } else if (pet_check_response((const u8*)data, size, "OUTOFSERVICE")) {
        g_pet.name_result = 2;
    }
}

/*
 * Handle pet skill list - FUN_0045fdc0 pattern
 */
void pet_handle_skill_list(void* data, u32 size) {
    if (pet_check_response((const u8*)data, size, "successful")) {
        if (g_pet.skill_thread == NULL) {
            g_pet.skills_received = 1;
            g_pet.skill_thread = CreateThread(NULL, 0, pet_skill_load_thread, &g_pet, 0, &g_pet.thread_id);
        }
    } else if (pet_check_response((const u8*)data, size, "serverbusy")) {
        if (g_pet.skill_thread == NULL) {
            g_pet.skills_received = 0xFE0A;
        }
    }
}

/*
 * Handle pet release result - FUN_0045ff50 pattern
 */
void pet_handle_release_result(void* data, u32 size) {
    if (pet_check_response((const u8*)data, size, "successful")) {
        g_pet.release_result = 1;
    }
}

/*
 * Parse pet data from text protocol - FUN_0045ffb0 case 0x57
 */
void pet_parse_data_57(const char* packet_data, int char_index) {
    const char* ptr = packet_data;
    char name_buffer[64];
    char memo_buffer[128];
    int field_index;
    int pet_index;
    s32 int_val;

    if (!packet_data) return;
    if (char_index < '0' || char_index > '9') return;

    ptr += 3;

    memset(g_pet_data_57, 0, sizeof(g_pet_data_57));
    g_pet_data_57_count = 0;

    field_index = 1;

    for (pet_index = 0; pet_index < MAX_PETS_PER_CHAR; pet_index++) {
        if (pet_parse_field(ptr, '|', field_index + 3, name_buffer, sizeof(name_buffer)) != 0) {
            break;
        }

        pet_unescape_string(name_buffer);

        if (name_buffer[0] == '\0') {
            field_index += 5;
            continue;
        }

        g_pet_data_57[pet_index].active = 1;
        strncpy(g_pet_data_57[pet_index].name, name_buffer, MAX_PET_NAME_57 - 1);
        g_pet_data_57[pet_index].name[MAX_PET_NAME_57 - 1] = '\0';

        int_val = pet_parse_field_int(ptr, '|', field_index);
        g_pet_data_57[pet_index].field_1 = (u16)int_val;

        int_val = pet_parse_field_int(ptr, '|', field_index + 1);
        g_pet_data_57[pet_index].field_2 = (u16)int_val;

        int_val = pet_parse_field_int(ptr, '|', field_index + 2);
        g_pet_data_57[pet_index].field_3 = (u16)int_val;

        pet_parse_field(ptr, '|', field_index + 4, memo_buffer, sizeof(memo_buffer));
        pet_unescape_string(memo_buffer);
        strncpy(g_pet_data_57[pet_index].memo, memo_buffer, MAX_PET_MEMO_57 - 1);
        g_pet_data_57[pet_index].memo[MAX_PET_MEMO_57 - 1] = '\0';

        LOG_DEBUG("Parsed pet %d: name=%s, f1=%u, f2=%u, f3=%u",
                  pet_index, g_pet_data_57[pet_index].name,
                  g_pet_data_57[pet_index].field_1,
                  g_pet_data_57[pet_index].field_2,
                  g_pet_data_57[pet_index].field_3);

        field_index += 5;
        g_pet_data_57_count++;

        if (field_index > 0x23) break;
    }

    LOG_INFO("Parsed %d pets from text protocol (char %c)", g_pet_data_57_count, char_index);
}

/*
 * Pet skill load thread
 */
DWORD WINAPI pet_skill_load_thread(LPVOID param) {
    PetContext* ctx = (PetContext*)param;
    char params[64];
    int i;
    int j;

    for (i = 0; i < MAX_PET_SLOTS; i++) {
        if (ctx->slots[i].id == 0) continue;

        for (j = 0; j < MAX_PET_SKILLS_PER_PET; j++) {
            _snprintf(params, sizeof(params), "%d|%d", i, j);
            pet_send_packet(PET_CMD_SKILL_REQ, params);
            Sleep(100);
        }
    }

    ctx->skills_loaded = 1;
    ctx->skill_thread = NULL;
    return 0;
}

/*
 * Handle capture result - FUN_00464db0 pattern
 */
void pet_handle_capture_result(u32 flags, u16 result) {
    if (flags == 0) {
        g_pet.capture_result = 0;
        return;
    }

    g_pet.capture_result = result;

    if (flags & 0x01) {
        g_pet.capture_slot = result;
    }
    if (flags & 0x02) {
        g_pet.capture_level = result;
    }
    if (flags & 0x04) {
        g_pet.capture_hp = result;
    }
    if (flags & 0x08) {
        g_pet.capture_max_hp = result;
    }
}

/*
 * Give experience to pet
 */
void pet_gain_exp(int slot, u32 exp) {
    PetData* pet;

    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return;
    }

    pet = &g_pet.slots[slot];
    if (pet->id == 0) {
        return;
    }

    pet->experience += exp;

    while (pet->level < MAX_PET_LEVEL && pet->experience >= s_exp_table[pet->level]) {
        pet_level_up(slot);
    }
}

/*
 * Pet level up
 */
void pet_level_up(int slot) {
    PetData* pet;
    int i;

    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return;
    }

    pet = &g_pet.slots[slot];
    if (pet->id == 0) {
        return;
    }

    if (pet->level >= MAX_PET_LEVEL) {
        return;
    }

    pet->level++;
    pet->experience -= s_exp_table[pet->level - 1];

    pet->max_hp += pet->growth_rate_hp;
    pet->max_mp += pet->growth_rate_mp;
    pet->attack += pet->growth_rate_atk;
    pet->defense += pet->growth_rate_def;
    pet->speed += pet->growth_rate_spd;
    pet->magic += pet->growth_rate_mag;

    pet->hp = pet->max_hp;
    pet->mp = pet->max_mp;

    for (i = 0; i < MAX_PET_SKILLS_PER_PET; i++) {
        if (pet->skills[i].skill_id != 0) {
            pet->skills[i].max_exp = pet->skills[i].level * 100;
        }
    }

    LOG_INFO("Pet %s leveled up to %u!", pet->name, pet->level);
}

/*
 * Pet skill level up
 */
void pet_skill_level_up(int slot, int skill_slot) {
    PetData* pet;
    PetSkill* skill;

    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return;
    }
    if (skill_slot < 0 || skill_slot >= MAX_PET_SKILLS_PER_PET) {
        return;
    }

    pet = &g_pet.slots[slot];
    if (pet->id == 0) {
        return;
    }

    skill = &pet->skills[skill_slot];
    if (skill->skill_id == 0) {
        return;
    }

    if (skill->level >= MAX_SKILL_LEVEL) {
        return;
    }

    skill->level++;
    skill->current_exp = 0;
    skill->max_exp = skill->level * 100;

    LOG_INFO("Pet %s skill %u leveled up to %u!", pet->name, skill->skill_id, skill->level);
}

/*
 * Calculate pet stats
 */
void pet_calculate_stats(PetData* pet) {
    if (!pet || pet->id == 0) {
        return;
    }

    pet->attack = pet->base_attack + pet->growth_rate_atk * (pet->level - 1);
    pet->defense = pet->base_defense + pet->growth_rate_def * (pet->level - 1);
    pet->speed = pet->base_speed + pet->growth_rate_spd * (pet->level - 1);
    pet->magic = pet->base_magic + pet->growth_rate_mag * (pet->level - 1);
}

/*
 * Heal pet
 */
void pet_heal(int slot, int hp, int mp) {
    PetData* pet;

    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return;
    }

    pet = &g_pet.slots[slot];
    if (pet->id == 0) {
        return;
    }

    if (hp > 0) {
        pet->hp += hp;
        if (pet->hp > pet->max_hp) pet->hp = pet->max_hp;
    }

    if (mp > 0) {
        pet->mp += mp;
        if (pet->mp > pet->max_mp) pet->mp = pet->max_mp;
    }
}

/*
 * Pet takes damage
 */
int pet_take_damage(int slot, int damage) {
    PetData* pet;

    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return 0;
    }

    pet = &g_pet.slots[slot];
    if (pet->id == 0) {
        return 0;
    }

    pet->hp -= damage;
    if (pet->hp <= 0) {
        pet->hp = 0;
        pet->state = PET_STATE_DEAD;
        LOG_DEBUG("Pet %s died", pet->name);
        return 0;
    }

    LOG_DEBUG("Pet %s took %d damage, HP: %u/%u", pet->name, damage, pet->hp, pet->max_hp);
    return 1;
}

/*
 * Get pet count
 */
int pet_get_count(void) {
    return g_pet.count;
}

/*
 * Check if pet slot is empty
 */
int pet_slot_empty(int slot) {
    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return 1;
    }
    return g_pet.slots[slot].id == 0;
}
