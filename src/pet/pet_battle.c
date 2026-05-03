/*
 * Stone Age Client - Pet Battle Handlers
 * Reverse engineered from sa_9061.exe
 * Split from pet.c for code organization
 *
 * Battle handlers from Ghidra analysis:
 * - FUN_004327b0: Summon pet to battle
 * - FUN_00432ec0: Recall pet from battle
 * - FUN_0042e870: Pet attack
 * - FUN_0042f130: Pet skill
 * - FUN_0042af40: Pet escape
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "pet.h"
#include "battle.h"
#include "network.h"
#include "logger.h"

/* External pet context */
extern PetContext g_pet;

/* Forward declarations for helpers */
static void pet_parse_skill_data(const char* skill_data, int slot);

/*
 * Summon pet to battle - FUN_004327b0 pattern
 * param_2: 0 = normal summon, 1 = field skill
 * Parses pet data and activates pet for battle
 */
void pet_summon_battle(const char* pet_data, int is_field_skill) {
    char buffer[256];
    char name_buf[36];
    char skill_buf[36];
    char data_buf[20];
    int i;
    int pet_index = 0;
    const char* ptr;
    int found_count = 0;

    if (!pet_data) return;

    /* Unescape input */
    strncpy(buffer, pet_data, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    pet_unescape_string(buffer);

    /* Clear pet summon state - DAT_04558d3c, DAT_0454f5b0, DAT_0455eed0 */
    g_pet.summon_pending = 0;
    g_pet.selected_skill = 0;
    g_pet.active_slot = -1;

    /* Parse target data - FUN_00425230 pattern */
    /* Process pet entries in loop */
    ptr = buffer;
    for (i = 0; i < MAX_PET_SLOTS && pet_index < MAX_PET_SLOTS; i++) {
        PetData* pet = &g_pet.slots[pet_index];

        /* Check if this is end of data */
        pet_parse_field(ptr, '|', 0, name_buf, sizeof(name_buf));
        if (name_buf[0] == '\0') {
            break;
        }

        found_count++;

        if (is_field_skill == 0) {
            /* Normal summon - parse full pet data */
            /* Field 2: Pet type/ID (7 bytes max) */
            pet_parse_field(ptr, '|', 2, data_buf, 7);

            /* Field 3: Pet name (0x23 = 35 bytes) */
            pet_parse_field(ptr, '|', 3, name_buf, 0x23);
            pet_unescape_string(name_buf);
            strncpy(pet->name, name_buf, sizeof(pet->name) - 1);

            /* Field 4: Pet skill name */
            pet_parse_field(ptr, '|', 4, skill_buf, 0x23);
            pet_unescape_string(skill_buf);

            /* Field 5: Level (0x13 = 19 bytes max) */
            pet_parse_field(ptr, '|', 5, data_buf, 0x13);

            /* Field 6: Additional data */
            pet_parse_field(ptr, '|', 6, data_buf, 0x13);

            /* Field 8: Compare string for activation */
            pet_parse_field(ptr, '|', 8, data_buf, 3);

            /* Check if this matches activation string - DAT_004b97f8 */
            if (strcmp(data_buf, "\xE8\xAE") == 0) {
                g_pet.active_slot = pet_index;
            }

            LOG_DEBUG("Summoned pet %d: %s", pet_index, pet->name);
        } else {
            /* Field skill mode - different parsing */
            pet_parse_field(ptr, '|', 2, data_buf, 7);
            pet_parse_field(ptr, '|', 3, name_buf, 0x23);
            pet_unescape_string(name_buf);
            pet_parse_field(ptr, '|', 4, skill_buf, 0x23);
            pet_unescape_string(skill_buf);
            pet_parse_field(ptr, '|', 5, data_buf, 0x13);
            pet_parse_field(ptr, '|', 6, data_buf, 0x13);

            LOG_DEBUG("Field skill pet %d: %s", pet_index, pet->name);
        }

        pet_index++;
        ptr = strchr(ptr, '|');
        if (ptr) ptr++;
    }

    /* Update summon state */
    g_pet.summon_pending = found_count;
}

/*
 * Recall pet from battle - FUN_00432ec0 pattern
 * Parses pet data and deactivates pet
 */
void pet_recall_battle(const char* pet_data) {
    char buffer[256];
    char name_buf[36];
    char skill_buf[36];
    char data_buf[8];
    int i;
    int pet_index = 0;
    const char* ptr;

    if (!pet_data) return;

    /* Unescape input */
    strncpy(buffer, pet_data, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    pet_unescape_string(buffer);

    /* Clear pet summon state */
    g_pet.summon_pending = 0;
    g_pet.selected_skill = 0;

    /* Parse pet entries */
    ptr = buffer;
    for (i = 0; i < MAX_PET_SLOTS && pet_index < MAX_PET_SLOTS; i++) {
        PetData* pet = &g_pet.slots[pet_index];

        pet_parse_field(ptr, '|', 0, name_buf, sizeof(name_buf));
        if (name_buf[0] == '\0') {
            break;
        }

        /* Parse recall data */
        pet_parse_field(ptr, '|', 2, data_buf, 7);
        pet_parse_field(ptr, '|', 3, name_buf, 0x23);
        pet_unescape_string(name_buf);
        pet_parse_field(ptr, '|', 4, skill_buf, 0x23);
        pet_unescape_string(skill_buf);
        pet_parse_field(ptr, '|', 5, data_buf, 0x13);
        pet_parse_field(ptr, '|', 6, data_buf, 3);

        /* Check activation string */
        if (strcmp(data_buf, "\xE8\xAE") == 0) {
            g_pet.active_slot = -1;
        }

        LOG_DEBUG("Recalled pet %d: %s", pet_index, pet->name);

        pet_index++;
        ptr = strchr(ptr, '|');
        if (ptr) ptr++;
    }
}

/*
 * Pet attack handler - FUN_0042e870 pattern
 */
void pet_attack_handler(const char* target_data) {
    char buffer[256];

    if (!target_data) return;

    /* Unescape and parse target */
    strncpy(buffer, target_data, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    pet_unescape_string(buffer);

    /* Clear pet attack context - DAT_0454de9c */
    g_pet.attack_target = 0;
    g_pet.attack_type = 2;  /* DAT_004b848c = 2 */

    /* Parse targets - FUN_00425230 pattern */
    /* battle_process_targets equivalent */

    LOG_DEBUG("Pet attack on: %s", buffer);
}

/*
 * Pet skill handler - FUN_0042f130 pattern
 * Parses skill data and activates pet skill
 */
void pet_skill_handler(const char* skill_data) {
    char buffer[512];
    char skill_buf[512];
    char field_buf[64];
    int skill_count;
    int first_skill_id;
    int data_count;
    int i;
    char* ptr;
    int skill_id, power, mp_cost, element, skill_type, target_type;

    if (!skill_data) return;

    /* Mark pet skill active - DAT_0455efc8 */
    g_pet.skill_active = 1;

    /* Check action type - DAT_004b83ec */
    if (g_pet.current_action != 0x0f) {
        g_pet.current_action = 0x0f;
        /* Clear context - DAT_0455ef98 */
        g_pet.skill_context = NULL;
    }

    /* Clear skill slots - DAT_0454ffe0 region */
    /* Each slot is 0x418 (1048) bytes */
    for (i = 0; i < MAX_PET_SKILLS_PER_PET; i++) {
        g_pet.battle_skills[i].id = 0;
        g_pet.battle_skills[i].name[0] = '\0';
        g_pet.battle_skills[i].description[0] = '\0';
        g_pet.battle_skills[i].power = 0;
        g_pet.battle_skills[i].mp_cost = 0;
        g_pet.battle_skills[i].element = 0;
        g_pet.battle_skills[i].type = 0;
        g_pet.battle_skills[i].target_type = 0;
    }

    /* Parse skill count - field 3 */
    pet_parse_field(skill_data, '|', 3, field_buf, 0x0f);
    skill_count = atoi(field_buf);
    g_pet.battle_skill_count = skill_count;

    /* Parse first skill ID - field 4 */
    pet_parse_field(skill_data, '|', 4, field_buf, 0x0f);
    first_skill_id = atoi(field_buf);

    /* Parse data count - field 5 */
    pet_parse_field(skill_data, '|', 5, field_buf, 0x0f);
    data_count = atoi(field_buf);

    /* Parse each skill entry - starting at field 6 */
    for (i = 0; i < data_count && i < MAX_PET_SKILLS_PER_PET; i++) {
        pet_parse_field(skill_data, '|', 6 + i, skill_buf, 0x7f);

        /* Parse space-delimited subfields */
        /* Format: ID Name Description Power MP Element Type TargetType */

        pet_parse_field(skill_buf, ' ', 1, field_buf, 0x0f);
        skill_id = atoi(field_buf);
        g_pet.battle_skills[i].id = (u16)skill_id;

        pet_parse_field(skill_buf, ' ', 2, g_pet.battle_skills[i].name, 0x1ff);

        pet_parse_field(skill_buf, ' ', 3, g_pet.battle_skills[i].description, 0x1ff);

        pet_parse_field(skill_buf, ' ', 5, field_buf, 0x0f);
        power = atoi(field_buf);
        g_pet.battle_skills[i].power = (u16)power;

        pet_parse_field(skill_buf, ' ', 4, field_buf, 0x0f);
        mp_cost = atoi(field_buf);
        g_pet.battle_skills[i].mp_cost = (u16)mp_cost;

        pet_parse_field(skill_buf, ' ', 6, field_buf, 0x0f);
        element = atoi(field_buf);
        g_pet.battle_skills[i].element = (u16)element;

        pet_parse_field(skill_buf, ' ', 7, field_buf, 0x0f);
        skill_type = atoi(field_buf);
        g_pet.battle_skills[i].type = (u16)skill_type;

        pet_parse_field(skill_buf, ' ', 8, field_buf, 0x0f);
        target_type = atoi(field_buf);
        g_pet.battle_skills[i].target_type = (u16)target_type;

        LOG_DEBUG("Pet skill %d: ID=%d, Name=%s, Power=%d, MP=%d",
                  i, skill_id, g_pet.battle_skills[i].name, power, mp_cost);
    }

    g_pet.battle_skill_count = data_count;
}

/*
 * Pet escape handler - FUN_0042af40 pattern
 * Handles pets that run away during battle
 */
void pet_escape_handler(const char* escape_data) {
    char buffer[256];
    char name_buf[64];
    int count;
    int i;

    if (!escape_data) return;

    /* Unescape input */
    strncpy(buffer, escape_data, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    pet_unescape_string(buffer);

    /* Clear flags - DAT_0455b0f0, DAT_0454f400 */
    g_pet.escape_flag = 0;
    g_pet.escape_result = 0;

    /* Parse count - field 1 */
    count = pet_parse_field_int(buffer, '|', 1);
    if (count == 0) {
        return;
    }

    /* Parse player name - field 2 */
    pet_parse_field(buffer, '|', 2, name_buf, 0x1a);
    pet_unescape_string(name_buf);

    /* Store in g_pet.escape_pet_name */
    strncpy(g_pet.escape_pet_name, name_buf, sizeof(g_pet.escape_pet_name) - 1);

    /* Parse target name - field 3 */
    pet_parse_field(buffer, '|', 3, name_buf, 0x1a);
    pet_unescape_string(name_buf);

    /* Set escape count - DAT_0455b370 = 0x50 (80) max */
    g_pet.escape_pet_count = count;

    /* Parse each escape pet entry (4 fields each, starting at field 5) */
    for (i = 0; i < count && i < MAX_ESCAPE_PET_SLOTS; i++) {
        int field_offset = 5 + i * 4;

        /* Parse pet name */
        pet_parse_field(buffer, '|', field_offset - 1, name_buf, 0x3f);
        pet_unescape_string(name_buf);

        LOG_DEBUG("Escape pet %d: %s", i, name_buf);

        /* Parse additional fields */
        /* Field 0: Some data */
        /* Field 1: Skill data */
        /* Field 2: More data */
    }

    /* Calculate final count: (count + 7) / 8 */
    g_pet.escape_pet_count = (count + 7) / 8;

    LOG_DEBUG("Pet escape: %d pets escaped", g_pet.escape_pet_count);
}

/*
 * Handle capture text result
 */
void pet_handle_capture_text(const char* pet_data) {
    char buffer[256];
    char name_buf[64];

    if (!pet_data) return;

    strncpy(buffer, pet_data, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    pet_unescape_string(buffer);

    /* Parse capture result */
    LOG_DEBUG("Pet capture text: %s", buffer);

    /* Update pet slots based on captured pet info */
}

/*
 * Helper: Parse skill data for a pet slot
 */
static void pet_parse_skill_data(const char* skill_data, int slot) {
    char buffer[256];
    int i;

    if (!skill_data || slot < 0 || slot >= MAX_PET_SLOTS) return;

    /* Parse skill data into pet slot */
    strncpy(buffer, skill_data, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    pet_unescape_string(buffer);
}
