/*
 * Stone Age Client - Pet Operations
 * Split from pet.c for code organization
 *
 * Contains pet operations: rename, release, feed, capture, skills, items
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "pet.h"
#include "network.h"
#include "logger.h"

/* External reference to global pet context */
extern PetContext g_pet;

/* Pet protocol commands */
#define PET_CMD_SKILL       "PT"
#define PET_CMD_ACTION      "P"
#define PET_CMD_RENAME      "PETNAME"
#define PET_CMD_RELEASE     "PETRELEASE"
#define PET_CMD_CAPTURE     "PETCAPTURE"
#define PET_CMD_SKILL_REQ   "PETSKILL"

/* Helper to send packet */
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
 * Rename pet
 */
int pet_rename(int slot, const char* name) {
    PetData* pet;
    char params[64];

    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return 0;
    }

    pet = &g_pet.slots[slot];
    if (pet->id == 0) {
        return 0;
    }

    if (!name || strlen(name) == 0 || strlen(name) >= sizeof(pet->name)) {
        return 0;
    }

    _snprintf(params, sizeof(params), "%d|%s", slot, name);
    pet_send_packet(PET_CMD_RENAME, params);

    strncpy(pet->name, name, sizeof(pet->name) - 1);
    pet->name[sizeof(pet->name) - 1] = '\0';

    LOG_DEBUG("Renaming pet in slot %d to %s", slot, name);
    return 1;
}

/*
 * Release pet
 */
int pet_release(int slot) {
    PetData* pet;
    char params[32];

    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return 0;
    }

    pet = &g_pet.slots[slot];
    if (pet->id == 0) {
        return 0;
    }

    _snprintf(params, sizeof(params), "%d", slot);
    pet_send_packet(PET_CMD_RELEASE, params);

    LOG_INFO("Releasing pet %s from slot %d", pet->name, slot);

    memset(pet, 0, sizeof(PetData));
    g_pet.count--;

    if (g_pet.selected_index == slot) {
        g_pet.selected_index = -1;
    }
    if (g_pet.battle_pet_index == slot) {
        g_pet.battle_pet_index = -1;
    }

    return 1;
}

/*
 * Feed pet
 */
int pet_feed(int slot, u32 item_id) {
    PetData* pet;
    int hp_restore;
    int mp_restore;
    int loyalty_increase;

    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return 0;
    }

    pet = &g_pet.slots[slot];
    if (pet->id == 0) {
        return 0;
    }

    hp_restore = 0;
    mp_restore = 0;
    loyalty_increase = 0;

    switch (item_id) {
        case 1:
            hp_restore = 50;
            loyalty_increase = 1;
            break;
        case 2:
            hp_restore = 100;
            mp_restore = 50;
            loyalty_increase = 2;
            break;
        case 3:
            hp_restore = 200;
            mp_restore = 100;
            loyalty_increase = 5;
            break;
    }

    if (pet->hp < pet->max_hp) {
        pet->hp += hp_restore;
        if (pet->hp > pet->max_hp) pet->hp = pet->max_hp;
    }

    if (pet->mp < pet->max_mp) {
        pet->mp += mp_restore;
        if (pet->mp > pet->max_mp) pet->mp = pet->max_mp;
    }

    if (pet->loyalty < 100) {
        pet->loyalty += loyalty_increase;
        if (pet->loyalty > 100) pet->loyalty = 100;
    }

    LOG_DEBUG("Fed pet %s with item %u", pet->name, item_id);
    return 1;
}

/*
 * Pet capture attempt
 */
int pet_capture(u32 target_id) {
    char params[64];

    if (g_pet.count >= MAX_PET_SLOTS) {
        LOG_WARN("No pet slots available for capture");
        return 0;
    }

    _snprintf(params, sizeof(params), "%u", target_id);
    pet_send_packet(PET_CMD_CAPTURE, params);

    LOG_DEBUG("Attempting to capture enemy %u", target_id);
    return 1;
}

/*
 * Calculate capture success rate
 */
int pet_calculate_capture_rate(int enemy_hp_percent, int enemy_level, int capture_item_id) {
    int base_rate;
    int hp_factor;
    int level_factor;
    int item_factor;

    base_rate = 20;
    hp_factor = (100 - enemy_hp_percent) / 2;
    level_factor = (100 - enemy_level) / 4;
    if (level_factor < 0) level_factor = 0;

    item_factor = 0;
    switch (capture_item_id) {
        case 1: item_factor = 10; break;
        case 2: item_factor = 20; break;
        case 3: item_factor = 30; break;
    }

    return base_rate + hp_factor + level_factor + item_factor;
}

/*
 * Set pet skills
 */
int pet_set_skill(int slot, int skill_slot, u16 skill_id, u8 level) {
    PetData* pet;

    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return 0;
    }
    if (skill_slot < 0 || skill_slot >= MAX_PET_SKILLS_PER_PET) {
        return 0;
    }

    pet = &g_pet.slots[slot];
    if (pet->id == 0) {
        return 0;
    }

    pet->skills[skill_slot].skill_id = skill_id;
    pet->skills[skill_slot].level = level;
    pet->skills[skill_slot].current_exp = 0;
    pet->skills[skill_slot].max_exp = level * 100;

    LOG_DEBUG("Set skill %d to pet %s: skill_id=%u, level=%u",
              skill_slot, pet->name, skill_id, level);
    return 1;
}

/*
 * Use pet skill
 */
int pet_use_skill(int slot, int skill_slot, u32 target_id) {
    PetData* pet;
    PetSkill* skill;
    char params[64];

    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return 0;
    }
    if (skill_slot < 0 || skill_slot >= MAX_PET_SKILLS_PER_PET) {
        return 0;
    }

    pet = &g_pet.slots[slot];
    if (pet->id == 0) {
        return 0;
    }

    skill = &pet->skills[skill_slot];
    if (skill->skill_id == 0) {
        return 0;
    }

    if (pet->mp < skill->mp_cost) {
        LOG_DEBUG("Not enough MP for skill");
        return 0;
    }

    pet->mp -= skill->mp_cost;
    skill->use_count++;

    _snprintf(params, sizeof(params), "%u|%u", skill->skill_id, target_id);
    pet_send_packet(PET_CMD_SKILL, params);

    LOG_DEBUG("Pet %s uses skill %u on target %u", pet->name, skill->skill_id, target_id);
    return 1;
}

/*
 * Pet use item
 */
int pet_use_item(int slot, u32 item_id) {
    PetData* pet;
    char params[64];

    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return 0;
    }

    pet = &g_pet.slots[slot];
    if (pet->id == 0) {
        return 0;
    }

    _snprintf(params, sizeof(params), "%d|1|%u", slot, item_id);
    pet_send_packet(PET_CMD_ACTION, params);

    LOG_DEBUG("Pet %s using item %u", pet->name, item_id);
    return 1;
}

/*
 * Pet mount/dismount
 */
int pet_toggle_mount(int slot) {
    PetData* pet;

    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return 0;
    }

    pet = &g_pet.slots[slot];
    if (pet->id == 0) {
        return 0;
    }

    if (!pet->can_mount) {
        return 0;
    }

    if (g_pet.mounted_slot == slot) {
        g_pet.mounted_slot = -1;
        LOG_DEBUG("Dismounted from pet %s", pet->name);
    } else {
        g_pet.mounted_slot = slot;
        LOG_DEBUG("Mounted pet %s", pet->name);
    }

    return 1;
}

/*
 * Pet set AI mode
 */
void pet_set_ai_mode(int slot, PetAIMode mode) {
    PetData* pet;

    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return;
    }

    pet = &g_pet.slots[slot];
    if (pet->id == 0) {
        return;
    }

    pet->ai_mode = mode;
    LOG_DEBUG("Pet %s AI mode set to %d", pet->name, mode);
}

/*
 * Pet evolution
 */
int pet_evolve(int slot) {
    PetData* pet;

    if (slot < 0 || slot >= MAX_PET_SLOTS) {
        return 0;
    }

    pet = &g_pet.slots[slot];
    if (pet->id == 0) {
        return 0;
    }

    if (pet->level < pet->evolve_level) {
        LOG_DEBUG("Pet %s needs level %u to evolve", pet->name, pet->evolve_level);
        return 0;
    }

    pet->type_id = pet->evolve_type;
    pet->sprite_id = pet->evolve_sprite;

    pet_calculate_stats(pet);

    LOG_INFO("Pet %s evolved!", pet->name);
    return 1;
}

/*
 * Pet fusion
 */
int pet_fuse(int slot1, int slot2) {
    PetData* pet1;
    PetData* pet2;
    PetData new_pet;
    int i;

    if (slot1 < 0 || slot1 >= MAX_PET_SLOTS || slot2 < 0 || slot2 >= MAX_PET_SLOTS) {
        return 0;
    }
    if (slot1 == slot2) {
        return 0;
    }

    pet1 = &g_pet.slots[slot1];
    pet2 = &g_pet.slots[slot2];

    if (pet1->id == 0 || pet2->id == 0) {
        return 0;
    }

    memset(&new_pet, 0, sizeof(PetData));
    new_pet.id = 1;
    new_pet.type_id = pet1->type_id;
    new_pet.level = (pet1->level + pet2->level) / 2;

    new_pet.max_hp = (pet1->max_hp + pet2->max_hp) / 2;
    new_pet.max_mp = (pet1->max_mp + pet2->max_mp) / 2;
    new_pet.hp = new_pet.max_hp;
    new_pet.mp = new_pet.max_mp;

    new_pet.growth_rate_hp = (pet1->growth_rate_hp + pet2->growth_rate_hp) / 2;
    new_pet.growth_rate_mp = (pet1->growth_rate_mp + pet2->growth_rate_mp) / 2;
    new_pet.growth_rate_atk = (pet1->growth_rate_atk + pet2->growth_rate_atk) / 2;
    new_pet.growth_rate_def = (pet1->growth_rate_def + pet2->growth_rate_def) / 2;
    new_pet.growth_rate_spd = (pet1->growth_rate_spd + pet2->growth_rate_spd) / 2;
    new_pet.growth_rate_mag = (pet1->growth_rate_mag + pet2->growth_rate_mag) / 2;

    for (i = 0; i < MAX_PET_SKILLS_PER_PET; i++) {
        if (pet1->skills[i].level >= pet2->skills[i].level) {
            new_pet.skills[i] = pet1->skills[i];
        } else {
            new_pet.skills[i] = pet2->skills[i];
        }
    }

    memset(pet1, 0, sizeof(PetData));
    memset(pet2, 0, sizeof(PetData));

    memcpy(pet1, &new_pet, sizeof(PetData));

    LOG_INFO("Pet fusion complete!");
    return 1;
}
