/*
 * Stone Age Client - Skill System Implementation
 * Reverse engineered from sa_9061.exe
 *
 * Protocol Analysis:
 * FUN_00424b70 - Battle action dispatcher (main entry point)
 * FUN_00425380 - Attack/defend handler
 * FUN_004253d0 - Skill/item handler
 * FUN_00425bb0 - Pet summon handler
 * FUN_004262f0 - Capture/wait handler
 * FUN_0042e870 - Pet skill attack handler
 * FUN_0042f130 - Pet skill special handler
 * FUN_00425230 - Multi-field string parser
 *
 * Protocol Commands:
 * - PS|<skill_id>|<target_id>: Use player skill
 * - PE|<skill_id>|<target_id>: Use enemy skill
 * - PT|<skill_id>|<target_id>: Use pet skill
 * - J|<action>|<param1>|<param2>|<string>: Battle action
 *
 * Battle Action Types (J command, from FUN_00424b70):
 * - 0: Normal attack (FUN_00425380)
 * - 1: Defend (FUN_00425380)
 * - 2: Use skill/item (FUN_004253d0)
 * - 6: Escape (FUN_00425420)
 * - 7: Capture (FUN_004254e0)
 * - 8: Capture special (FUN_004254e0)
 * - 9: Summon pet (FUN_00425bb0)
 * - 10/11: Pet attack (FUN_00425b50)
 * - 12: Wait/Pass (FUN_004262f0)
 * - 13: Field action (FUN_00425dc0)
 * - 14: Pet skill (FUN_0042e870)
 * - 15: Pet skill special (FUN_0042f130)
 * - 42: Capture result (FUN_0042af40)
 * - 45: Combo attack (FUN_00425380)
 * - 101: Special skill action
 *
 * Module Split:
 * - skill.c - Core skill management (init, learn, forget, slots, cooldowns, casting)
 * - skill_protocol.c - Protocol parsing and network communication
 * - skill_damage.c - Damage calculation, effects, status, buffs
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
#include "character.h"
#include "battle.h"
#include "network.h"
#include "pet.h"
#include "pet_protocol.h"
#include "logger.h"

/* Global skill context */
SkillContext g_skill = {0};

/* Global skill list from server - FUN_0045ffb0 case 0x53 */
SkillListEntry g_skill_list[MAX_PLAYER_SKILLS] = {0};
int g_skill_list_count = 0;

/* Default cooldowns per skill type (ms) */
static const u32 s_default_cooldowns[] = {
    3000,   /* SKILL_TYPE_ATTACK */
    5000,   /* SKILL_TYPE_HEAL */
    8000,   /* SKILL_TYPE_BUFF */
    8000,   /* SKILL_TYPE_DEBUFF */
    0,      /* SKILL_TYPE_PASSIVE */
    10000,  /* SKILL_TYPE_SPECIAL */
    15000   /* SKILL_TYPE_SUMMON */
};

/* Exp required per level */
static const u16 s_exp_table[MAX_SKILL_LEVEL] = {
    100, 250, 500, 1000, 2000, 4000, 8000, 15000, 30000, 60000
};

/* Forward declarations for internal helpers */
static void skill_send_packet(const char* cmd, const char* params);
static void skill_send_formatted(const char* fmt, ...);

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
 * Initialize skill system
 */
int skill_init(void) {
    memset(&g_skill, 0, sizeof(SkillContext));

    if (!skill_load_database()) {
        LOG_WARN("Failed to load skill database");
    }

    LOG_INFO("Skill system initialized");
    return 1;
}

/*
 * Shutdown skill system
 */
void skill_shutdown(void) {
    if (g_skill.skill_db) {
        free(g_skill.skill_db);
        g_skill.skill_db = NULL;
    }

    memset(&g_skill, 0, sizeof(SkillContext));
    LOG_INFO("Skill system shutdown");
}

/*
 * Load skill database
 */
int skill_load_database(void) {
    FILE* fp;
    char line[256];
    int count = 0;

    /* Skill database is loaded from game data files */
    /* Format: data/skill.dat or similar - requires reverse engineering data format */
    /* For now, skills are populated from server protocol (case 0x53) */
    g_skill.skill_db_count = 0;

    LOG_DEBUG("Skill database initialized");
    return 1;
}

/*
 * Get skill data by ID
 */
SkillData* skill_get_data(u32 skill_id) {
    int i;

    /* Search in learned skills first */
    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            /* Found in learned skills */
            break;
        }
    }

    /* Search in database */
    if (g_skill.skill_db) {
        for (i = 0; i < g_skill.skill_db_count; i++) {
            if (g_skill.skill_db[i].id == skill_id) {
                return &g_skill.skill_db[i];
            }
        }
    }

    return NULL;
}

/*
 * Get skill data by name
 */
SkillData* skill_get_by_name(const char* name) {
    int i;

    if (!name || !g_skill.skill_db) {
        return NULL;
    }

    for (i = 0; i < g_skill.skill_db_count; i++) {
        if (strcmp(g_skill.skill_db[i].name, name) == 0) {
            return &g_skill.skill_db[i];
        }
    }

    return NULL;
}

/*
 * Learn new skill
 */
int skill_learn(u32 skill_id) {
    LearnedSkill* skill;
    SkillData* data;

    if (g_skill.skill_count >= MAX_PLAYER_SKILLS) {
        LOG_WARN("No free skill slots");
        return 0;
    }

    if (skill_has_skill(skill_id)) {
        LOG_WARN("Already have skill %u", skill_id);
        return 0;
    }

    data = skill_get_data(skill_id);
    if (!data) {
        LOG_WARN("Unknown skill %u", skill_id);
        return 0;
    }

    skill = &g_skill.skills[g_skill.skill_count];
    skill->skill_id = skill_id;
    skill->level = 1;
    skill->slot = 0xFF;
    skill->exp = 0;
    skill->exp_next = s_exp_table[0];

    g_skill.skill_count++;
    g_skill.dirty = 1;

    LOG_INFO("Learned skill: %s", data->name);
    return 1;
}

/*
 * Forget skill
 */
int skill_forget(u32 skill_id) {
    int i;

    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            /* Clear from slots */
            int slot;
            for (slot = 0; slot < 10; slot++) {
                if (g_skill.skill_slots[slot] == skill_id) {
                    g_skill.skill_slots[slot] = 0;
                }
            }

            /* Shift remaining skills */
            memmove(&g_skill.skills[i], &g_skill.skills[i + 1],
                    (g_skill.skill_count - i - 1) * sizeof(LearnedSkill));
            g_skill.skill_count--;
            g_skill.dirty = 1;

            LOG_INFO("Forgot skill %u", skill_id);
            return 1;
        }
    }

    return 0;
}

/*
 * Check if has skill
 */
int skill_has_skill(u32 skill_id) {
    int i;

    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            return 1;
        }
    }

    return 0;
}

/*
 * Check if can learn skill
 */
int skill_can_learn(u32 skill_id) {
    SkillData* data = skill_get_data(skill_id);

    if (!data) return 0;

    /* Check requirement skill */
    if (data->requirement_skill != 0) {
        if (!skill_has_skill(data->requirement_skill)) {
            return 0;
        }
        if (skill_get_level(data->requirement_skill) < data->requirement_level) {
            return 0;
        }
    }

    return 1;
}

/*
 * Get skill level
 */
int skill_get_level(u32 skill_id) {
    LearnedSkill* skill = skill_get_learned(skill_id);
    return skill ? skill->level : 0;
}

/*
 * Set skill level
 */
int skill_set_level(u32 skill_id, u8 level) {
    LearnedSkill* skill = skill_get_learned(skill_id);

    if (!skill) return 0;
    if (level < 1 || level > MAX_SKILL_LEVEL) return 0;

    skill->level = level;
    skill->exp = 0;
    if (level < MAX_SKILL_LEVEL) {
        skill->exp_next = s_exp_table[level - 1];
    }

    g_skill.dirty = 1;
    return 1;
}

/*
 * Level up skill
 */
int skill_level_up(u32 skill_id) {
    LearnedSkill* skill = skill_get_learned(skill_id);

    if (!skill) return 0;
    if (skill->level >= MAX_SKILL_LEVEL) return 0;
    if (g_skill.skill_points <= 0) return 0;

    skill->level++;
    g_skill.skill_points--;
    g_skill.total_points_used++;
    g_skill.dirty = 1;

    LOG_INFO("Skill %u leveled up to %d", skill_id, skill->level);
    return 1;
}

/*
 * Gain skill experience
 */
int skill_gain_exp(u32 skill_id, u16 exp) {
    LearnedSkill* skill = skill_get_learned(skill_id);

    if (!skill) return 0;
    if (skill->level >= MAX_SKILL_LEVEL) return 0;

    skill->exp += exp;

    /* Check for level up */
    while (skill->exp >= skill->exp_next && skill->level < MAX_SKILL_LEVEL) {
        skill->exp -= skill->exp_next;
        skill->level++;
        skill->exp_next = s_exp_table[skill->level - 1];
        LOG_INFO("Skill %u leveled up to %d", skill_id, skill->level);
    }

    g_skill.dirty = 1;
    return 1;
}

/*
 * Get exp required for level
 */
int skill_get_exp_for_level(u8 level) {
    if (level < 1 || level > MAX_SKILL_LEVEL) return 0;
    return s_exp_table[level - 1];
}

/*
 * Set skill points
 */
void skill_set_points(u16 points) {
    g_skill.skill_points = points;
}

/*
 * Get skill points
 */
u16 skill_get_points(void) {
    return g_skill.skill_points;
}

/*
 * Use skill point
 */
int skill_use_point(u32 skill_id) {
    if (g_skill.skill_points <= 0) return 0;
    return skill_level_up(skill_id);
}

/*
 * Set skill slot
 */
int skill_set_slot(int slot, u32 skill_id) {
    if (slot < 0 || slot >= 10) return 0;
    if (skill_id != 0 && !skill_has_skill(skill_id)) return 0;

    g_skill.skill_slots[slot] = skill_id;
    g_skill.dirty = 1;
    return 1;
}

/*
 * Get skill in slot
 */
u32 skill_get_slot(int slot) {
    if (slot < 0 || slot >= 10) return 0;
    return g_skill.skill_slots[slot];
}

/*
 * Clear skill slot
 */
int skill_clear_slot(int slot) {
    return skill_set_slot(slot, 0);
}

/*
 * Swap skill slots
 */
void skill_swap_slots(int slot1, int slot2) {
    u32 temp;

    if (slot1 < 0 || slot1 >= 10 || slot2 < 0 || slot2 >= 10) return;

    temp = g_skill.skill_slots[slot1];
    g_skill.skill_slots[slot1] = g_skill.skill_slots[slot2];
    g_skill.skill_slots[slot2] = temp;

    g_skill.dirty = 1;
}

/*
 * Start casting skill
 */
int skill_start_cast(u32 skill_id, u32 target_id) {
    SkillData* data;
    LearnedSkill* skill;

    if (g_skill.is_casting) {
        LOG_WARN("Already casting");
        return 0;
    }

    if (skill_is_on_cooldown(skill_id)) {
        LOG_WARN("Skill on cooldown");
        return 0;
    }

    data = skill_get_data(skill_id);
    skill = skill_get_learned(skill_id);

    if (!data || !skill) return 0;

    g_skill.casting_skill = skill_id;
    g_skill.casting_target = target_id;
    g_skill.cast_start_time = timeGetTime();
    g_skill.cast_time = 1000;  /* Default 1 second cast */
    g_skill.is_casting = 1;

    LOG_DEBUG("Casting skill: %s", data->name);
    return 1;
}

/*
 * Update casting
 */
void skill_update_cast(u32 current_time) {
    if (!g_skill.is_casting) return;

    if (current_time - g_skill.cast_start_time >= g_skill.cast_time) {
        skill_finish_cast();
    }
}

/*
 * Check if casting
 */
int skill_is_casting(void) {
    return g_skill.is_casting;
}

/*
 * Cancel cast
 */
void skill_cancel_cast(void) {
    g_skill.is_casting = 0;
    g_skill.casting_skill = 0;
    g_skill.casting_target = 0;
}

/*
 * Finish casting
 */
int skill_finish_cast(void) {
    SkillData* data;
    char params[64];

    if (!g_skill.is_casting) return 0;

    data = skill_get_data(g_skill.casting_skill);
    if (!data) {
        g_skill.is_casting = 0;
        return 0;
    }

    /* Send skill use packet based on skill type */
    /* Format: PS|<skill_id>|<target_index> for player skills */
    _snprintf(params, sizeof(params), "%u|%u", g_skill.casting_skill, g_skill.casting_target);

    switch (data->type) {
        case SKILL_TYPE_ATTACK:
        case SKILL_TYPE_HEAL:
        case SKILL_TYPE_BUFF:
        case SKILL_TYPE_DEBUFF:
        case SKILL_TYPE_SPECIAL:
            /* Player skill: PS|<skill_id>|<target> */
            skill_send_packet(SKILL_CMD_PLAYER, params);
            break;

        case SKILL_TYPE_SUMMON:
            /* Summon skill: J|9|<pet_slot>|<skill_id>|0 */
            skill_send_formatted("J|9|%u|%u|0", g_skill.casting_target, g_skill.casting_skill);
            break;

        default:
            skill_send_packet(SKILL_CMD_PLAYER, params);
            break;
    }

    skill_start_cooldown(g_skill.casting_skill);
    g_skill.is_casting = 0;

    LOG_DEBUG("Finished casting skill %u on target %u", g_skill.casting_skill, g_skill.casting_target);
    return 1;
}

/*
 * Get cast progress (0-100)
 */
int skill_get_cast_progress(void) {
    u32 elapsed;

    if (!g_skill.is_casting) return 0;

    elapsed = timeGetTime() - g_skill.cast_start_time;
    if (elapsed >= g_skill.cast_time) return 100;

    return (elapsed * 100) / g_skill.cast_time;
}

/*
 * Start cooldown
 */
void skill_start_cooldown(u32 skill_id) {
    int i;
    SkillData* data;

    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            data = skill_get_data(skill_id);
            if (data) {
                g_skill.cooldowns[i] = timeGetTime() +
                    s_default_cooldowns[data->type];
            }
            return;
        }
    }
}

/*
 * Check if skill is on cooldown
 */
int skill_is_on_cooldown(u32 skill_id) {
    int i;

    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            if (g_skill.cooldowns[i] > timeGetTime()) {
                return 1;
            }
            return 0;
        }
    }

    return 0;
}

/*
 * Get cooldown remaining
 */
u32 skill_get_cooldown_remaining(u32 skill_id) {
    int i;
    u32 current = timeGetTime();

    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            if (g_skill.cooldowns[i] > current) {
                return g_skill.cooldowns[i] - current;
            }
            return 0;
        }
    }

    return 0;
}

/*
 * Use skill directly
 */
int skill_use(u32 skill_id, u32 target_id) {
    SkillData* data;
    LearnedSkill* skill;
    char params[64];

    if (skill_is_on_cooldown(skill_id)) {
        LOG_WARN("Skill on cooldown");
        return 0;
    }

    data = skill_get_data(skill_id);
    skill = skill_get_learned(skill_id);

    if (!data || !skill) return 0;

    /* Instant cast */
    if (data->flags & SKILL_FLAG_INSTANT) {
        /* Send skill packet immediately */
        _snprintf(params, sizeof(params), "%u|%u", skill_id, target_id);

        if (data->type == SKILL_TYPE_SUMMON) {
            /* Summon is handled differently */
            skill_send_formatted("J|9|%u|%u|0", target_id, skill_id);
        } else {
            skill_send_packet(SKILL_CMD_PLAYER, params);
        }

        skill_start_cooldown(skill_id);
        return 1;
    }

    /* Start casting */
    return skill_start_cast(skill_id, target_id);
}

/*
 * Use skill from slot
 */
int skill_use_slot(int slot, u32 target_id) {
    u32 skill_id = skill_get_slot(slot);
    if (skill_id == 0) return 0;
    return skill_use(skill_id, target_id);
}

/*
 * Get learned skill by ID
 */
LearnedSkill* skill_get_learned(u32 skill_id) {
    int i;

    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            return &g_skill.skills[i];
        }
    }

    return NULL;
}

/*
 * Get learned skill by index
 */
LearnedSkill* skill_get_learned_by_index(int index) {
    if (index < 0 || index >= g_skill.skill_count) {
        return NULL;
    }
    return &g_skill.skills[index];
}

/*
 * Get skill count
 */
int skill_get_count(void) {
    return g_skill.skill_count;
}

/*
 * Get MP cost for skill
 */
int skill_get_mp_cost(u32 skill_id) {
    SkillData* data = skill_get_data(skill_id);
    LearnedSkill* skill = skill_get_learned(skill_id);

    if (!data || !skill) return 0;

    /* Cost increases with level */
    return data->mp_cost + (skill->level - 1) * 5;
}

/*
 * Get skill name
 */
const char* skill_get_name(u32 skill_id) {
    SkillData* data = skill_get_data(skill_id);
    return data ? data->name : "Unknown";
}

/*
 * Skill type to string
 */
const char* skill_type_to_string(SkillType type) {
    static const char* type_names[] = {
        "Attack", "Heal", "Buff", "Debuff",
        "Passive", "Special", "Summon"
    };

    if (type < sizeof(type_names) / sizeof(type_names[0])) {
        return type_names[type];
    }
    return "Unknown";
}

/*
 * Check if target is valid for skill
 */
int skill_is_valid_target(u32 skill_id, u32 caster_id, u32 target_id) {
    SkillData* data = skill_get_data(skill_id);

    if (!data) return 0;

    switch (data->target_type) {
        case SKILL_TARGET_SELF:
            return caster_id == target_id;

        case SKILL_TARGET_SINGLE:
        case SKILL_TARGET_RANDOM:
        case SKILL_TARGET_ROW:
            return 1;

        case SKILL_TARGET_ALL_ENEMY:
        case SKILL_TARGET_ALL_ALLY:
            return 1;  /* Auto-targeted */

        default:
            return 0;
    }
}

/*
 * Use pet skill - sends PT protocol
 * Format: PT|<skill_id>|<target_index>
 */
int pet_skill_use(u32 pet_slot, u32 skill_id, u32 target_id) {
    char params[64];

    /* Pet skill protocol: PT|<skill_id>|<target> */
    _snprintf(params, sizeof(params), "%u|%u", skill_id, target_id);
    skill_send_packet(SKILL_CMD_PET, params);

    LOG_DEBUG("Pet %u using skill %u on target %u", pet_slot, skill_id, target_id);
    return 1;
}

/*
 * Get pet skill count
 * Returns number of skills the pet has
 */
int pet_skill_get_count(u32 pet_slot) {
    /* Pet skills are typically stored in pet data */
    /* This would integrate with the pet system */
    return 0;
}

/*
 * Get pet skill by index
 */
u32 pet_skill_get(u32 pet_slot, int skill_index) {
    /* This would integrate with the pet system */
    return 0;
}
