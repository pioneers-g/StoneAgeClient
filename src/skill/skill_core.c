/*
 * Stone Age Client - Skill System Core Functions
 * Split from skill.c - Core skill management (init, learn, levels, slots)
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "skill.h"
#include "character.h"
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

/* Forward declarations */
extern void skill_send_packet(const char* cmd, const char* params);
extern void skill_send_formatted(const char* fmt, ...);
extern int skill_is_on_cooldown(u32 skill_id);
extern void skill_start_cooldown(u32 skill_id);

/*
 * Initialize skill system
 */
int skill_init(void) {
    memset(&g_skill, 0, sizeof(SkillContext));
    memset(g_skill_list, 0, sizeof(g_skill_list));
    g_skill_list_count = 0;

    LOG_INFO("Skill system initialized");
    return 1;
}

/*
 * Shutdown skill system
 */
void skill_shutdown(void) {
    memset(&g_skill, 0, sizeof(SkillContext));
    memset(g_skill_list, 0, sizeof(g_skill_list));
    g_skill_list_count = 0;

    LOG_INFO("Skill system shutdown");
}

/*
 * Load skill database
 * Returns number of skills loaded
 */
int skill_load_database(void) {
    /* Skill data would be loaded from game resources */
    /* For now, return 0 - actual loading in assets module */
    return 0;
}

/*
 * Get skill data by ID
 * Returns pointer to skill definition
 */
SkillData* skill_get_data(u32 skill_id) {
    int i;

    for (i = 0; i < g_skill_list_count; i++) {
        if (g_skill_list[i].skill_id == skill_id) {
            return (SkillData*)&g_skill_list[i];
        }
    }

    return NULL;
}

/*
 * Learn new skill
 * Returns 1 on success, 0 on failure
 */
int skill_learn(u32 skill_id) {
    SkillData* data;
    LearnedSkill* skill;

    /* Check if already learned */
    if (skill_has_skill(skill_id)) {
        LOG_WARN("Already have skill %u", skill_id);
        return 0;
    }

    /* Check if can learn */
    if (!skill_can_learn(skill_id)) {
        LOG_WARN("Cannot learn skill %u", skill_id);
        return 0;
    }

    /* Get skill data */
    data = skill_get_data(skill_id);
    if (!data) {
        LOG_WARN("Unknown skill %u", skill_id);
        return 0;
    }

    /* Add to learned skills */
    if (g_skill.skill_count >= MAX_PLAYER_SKILLS) {
        LOG_WARN("Skill limit reached");
        return 0;
    }

    skill = &g_skill.skills[g_skill.skill_count];
    skill->skill_id = skill_id;
    skill->level = 1;
    skill->exp = 0;

    g_skill.skill_count++;
    g_skill.dirty = 1;

    LOG_INFO("Learned skill: %s (ID: %u)", data->name, skill_id);
    return 1;
}

/*
 * Forget skill
 * Returns 1 on success, 0 on failure
 */
int skill_forget(u32 skill_id) {
    int i, found = -1;

    /* Find skill */
    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            found = i;
            break;
        }
    }

    if (found < 0) {
        LOG_WARN("Don't have skill %u", skill_id);
        return 0;
    }

    /* Remove from slots if equipped */
    for (i = 0; i < 10; i++) {
        if (g_skill.skill_slots[i] == skill_id) {
            g_skill.skill_slots[i] = 0;
        }
    }

    /* Shift remaining skills */
    for (i = found; i < g_skill.skill_count - 1; i++) {
        g_skill.skills[i] = g_skill.skills[i + 1];
        g_skill.cooldowns[i] = g_skill.cooldowns[i + 1];
    }

    g_skill.skill_count--;
    g_skill.dirty = 1;

    LOG_INFO("Forgot skill %u", skill_id);
    return 1;
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

    /* Check level requirement */
    /* Check class requirement */
    /* Check prerequisite skills */

    return 1;
}

/*
 * Get skill level
 */
int skill_get_level(u32 skill_id) {
    int i;

    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            return g_skill.skills[i].level;
        }
    }

    return 0;
}

/*
 * Set skill level
 */
int skill_set_level(u32 skill_id, u8 level) {
    int i;

    if (level < 1 || level > MAX_SKILL_LEVEL) return 0;

    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            g_skill.skills[i].level = level;
            g_skill.dirty = 1;
            return 1;
        }
    }

    return 0;
}

/*
 * Level up skill
 */
int skill_level_up(u32 skill_id) {
    int level = skill_get_level(skill_id);

    if (level <= 0 || level >= MAX_SKILL_LEVEL) return 0;

    return skill_set_level(skill_id, level + 1);
}

/*
 * Gain skill experience
 */
int skill_gain_exp(u32 skill_id, u16 exp) {
    int i;
    u32 new_exp;

    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            new_exp = g_skill.skills[i].exp + exp;

            /* Check for level up */
            while (g_skill.skills[i].level < MAX_SKILL_LEVEL &&
                   new_exp >= skill_get_exp_for_level(g_skill.skills[i].level + 1)) {
                g_skill.skills[i].level++;
                LOG_INFO("Skill %u leveled up to %u", skill_id, g_skill.skills[i].level);
            }

            g_skill.skills[i].exp = (u16)new_exp;
            g_skill.dirty = 1;
            return 1;
        }
    }

    return 0;
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
    g_skill.dirty = 1;
}

/*
 * Get skill points
 */
u16 skill_get_points(void) {
    return g_skill.skill_points;
}

/*
 * Use skill point on skill
 */
int skill_use_point(u32 skill_id) {
    if (g_skill.skill_points <= 0) return 0;

    if (!skill_has_skill(skill_id)) return 0;

    g_skill.skill_points--;
    skill_level_up(skill_id);
    return 1;
}

/*
 * Set skill slot
 */
int skill_set_slot(int slot, u32 skill_id) {
    if (slot < 0 || slot >= 10) return 0;

    /* skill_id 0 clears the slot */
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
 * Check if skill system is dirty (needs save)
 */
int skill_is_dirty(void) {
    return g_skill.dirty;
}

/*
 * Clear dirty flag
 */
void skill_clear_dirty(void) {
    g_skill.dirty = 0;
}
