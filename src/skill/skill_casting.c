/*
 * Stone Age Client - Skill Casting System
 * Split from skill.c - Casting, cooldowns, usage, targeting
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "types.h"
#include "skill.h"
#include "skill_protocol.h"
#include "battle.h"
#include "network.h"
#include "logger.h"

/* External globals */
extern SkillContext g_skill;
extern SkillListEntry g_skill_list[];
extern int g_skill_list_count;

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

/*
 * Send skill protocol packet - helper function
 */
void skill_send_packet(const char* cmd, const char* params) {
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
 * Send formatted skill packet
 */
void skill_send_formatted(const char* fmt, ...) {
    char packet[512];
    va_list args;

    va_start(args, fmt);
    _vsnprintf(packet, sizeof(packet), fmt, args);
    va_end(args);

    network_send(packet, strlen(packet));
    network_send("\n", 1);
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
