/*
 * Stone Age Client - Quest System Implementation
 * Reverse engineered from sa_9061.exe
 *
 * Protocol Analysis:
 * FUN_00465d20 - Quest packet router (delimiter: '|')
 *   - 'B' (0x42): New quest available
 *   - 'D' (0x44): Quest data
 *   - 'C' (0x43): Quest complete
 *   - 'O' (0x4F): Objective update
 *   - 'S' (0x53): Quest status
 *
 * FUN_00465de0 - Quest update handler (delimiter: '#')
 *   - Field 1: Quest ID (u32)
 *   - Field 2: Quest name (string)
 *   - Field 3: Quest description (string)
 *   - Quest struct size: 0xD8 (216 bytes)
 *
 * FUN_00465f70 - Achievement handler (delimiter: '|')
 *   - 'A' (0x41): Achievement earned
 *   - 'C' (0x43): Achievement progress
 *   - 'M' (0x4D): Achievement milestone
 *   - 'V' (0x56): Achievement reward
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "quest.h"
#include "network.h"
#include "logger.h"

/* Quest packet type codes */
#define QUEST_PKT_AVAILABLE     'B'     /* 0x42 - New quest available */
#define QUEST_PKT_DATA          'D'     /* 0x44 - Quest data */
#define QUEST_PKT_COMPLETE      'C'     /* 0x43 - Quest complete */
#define QUEST_PKT_OBJECTIVE     'O'     /* 0x4F - Objective update */
#define QUEST_PKT_STATUS        'S'     /* 0x53 - Quest status */

/* Achievement packet type codes */
#define ACHIEVEMENT_PKT_EARNED      'A'     /* 0x41 - Achievement earned */
#define ACHIEVEMENT_PKT_PROGRESS    'C'     /* 0x43 - Progress update */
#define ACHIEVEMENT_PKT_MILESTONE   'M'     /* 0x4D - Milestone */
#define ACHIEVEMENT_PKT_REWARD      'V'     /* 0x56 - Reward claim */

/* Protocol commands */
#define QUEST_CMD_ACCEPT    "QA"
#define QUEST_CMD_COMPLETE  "QC"
#define QUEST_CMD_ABANDON   "QX"
#define QUEST_CMD_CLAIM     "QR"

/* Global quest context */
QuestContext g_quest = {0};

/* Internal helper: Send quest protocol command */
static int quest_send_command(const char* cmd, const char* params) {
    char packet[256];
    int len;

    if (!cmd) return 0;

    if (params && params[0]) {
        len = _snprintf(packet, sizeof(packet), "%s|%s", cmd, params);
    } else {
        len = _snprintf(packet, sizeof(packet), "%s", cmd);
    }

    if (len < 0 || len >= (int)sizeof(packet)) {
        LOG_ERROR("Quest packet too long");
        return 0;
    }

    network_send(packet, strlen(packet));
    network_send("\n", 1);
    LOG_DEBUG("Quest send: %s", packet);
    return 1;
}

/*
 * Initialize quest system
 */
int quest_init(void) {
    memset(&g_quest, 0, sizeof(QuestContext));
    LOG_INFO("Quest system initialized");
    return 1;
}

/*
 * Shutdown quest system
 */
void quest_shutdown(void) {
    int i;

    for (i = 0; i < MAX_QUESTS; i++) {
        if (g_quest.quests[i].objectives) {
            free(g_quest.quests[i].objectives);
        }
    }

    memset(&g_quest, 0, sizeof(QuestContext));
    LOG_INFO("Quest system shutdown");
}

/*
 * Handle quest list packet - FUN_00465d20 pattern
 * Delimiter: '|' (0x7C)
 */
void quest_handle_list_packet(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 type;

    if (!data || size < 1) return;

    type = *ptr++;

    switch (type) {
        case QUEST_PKT_AVAILABLE:   /* 'B' (0x42) - New quest available */
            g_quest.new_quest_available = 1;
            LOG_DEBUG("New quest available flag set");
            /* Fall through to parse quest data */
            quest_parse_list(data, size);
            break;

        case QUEST_PKT_DATA:        /* 'D' (0x44) - Quest data */
            quest_parse_list(data, size);
            break;

        case QUEST_PKT_COMPLETE:    /* 'C' (0x43) - Quest complete */
            g_quest.new_quest_available = 0;
            LOG_DEBUG("Quest complete flag cleared");
            break;

        case QUEST_PKT_OBJECTIVE:   /* 'O' (0x4F) - Objective update */
            g_quest.objective_updated = 1;
            g_quest.in_quest = 1;
            LOG_DEBUG("Quest objective updated");
            break;

        case QUEST_PKT_STATUS:      /* 'S' (0x53) - Quest status */
            g_quest.objective_updated = 3;
            quest_parse_objective(data, size);
            break;

        default:
            LOG_DEBUG("Unknown quest packet type: 0x%02X", type);
            break;
    }
}

/*
 * Parse quest list - FUN_00465de0 pattern
 * Uses '#' (0x23) as main delimiter, '|' (0x7C) as sub-delimiter
 * Each quest: ID|Name|Description
 */
void quest_parse_list(void* data, u32 size) {
    char* ptr = (char*)data;
    char* end = (char*)data + size;
    char field_buffer[256];
    char quest_buffer[512];
    int quest_index = 0;
    int field_index;
    QuestData* quest;

    /* Clear existing quests */
    int i;
    for (i = 0; i < MAX_QUESTS; i++) {
        if (g_quest.quests[i].objectives) {
            free(g_quest.quests[i].objectives);
            g_quest.quests[i].objectives = NULL;
        }
    }
    memset(g_quest.quests, 0, sizeof(g_quest.quests));

    /* Skip first field (packet type) */
    ptr = strchr(ptr, '|');
    if (!ptr) return;
    ptr++;

    /* Parse quests separated by '#' */
    g_quest.quest_count = 0;

    while (ptr < end && quest_index < MAX_QUESTS) {
        quest = &g_quest.quests[quest_index];
        field_index = 1;

        /* Parse each field separated by '|' */
        while (ptr < end && field_index <= 3) {
            char* next_field = strchr(ptr, '|');
            char* next_quest = strchr(ptr, '#');

            /* Determine field end */
            char* field_end;
            if (next_field && next_quest) {
                field_end = (next_field < next_quest) ? next_field : next_quest;
            } else if (next_field) {
                field_end = next_field;
            } else if (next_quest) {
                field_end = next_quest;
            } else {
                field_end = end;
            }

            /* Extract field */
            int field_len = field_end - ptr;
            if (field_len > 0 && field_len < (int)sizeof(field_buffer)) {
                strncpy(field_buffer, ptr, field_len);
                field_buffer[field_len] = '\0';

                switch (field_index) {
                    case 1:  /* Quest ID */
                        quest->id = (u32)atoi(field_buffer);
                        break;
                    case 2:  /* Quest name */
                        strncpy(quest->name, field_buffer, sizeof(quest->name) - 1);
                        break;
                    case 3:  /* Quest description */
                        strncpy(quest->description, field_buffer, sizeof(quest->description) - 1);
                        break;
                }
            }

            ptr = field_end + 1;
            field_index++;

            /* Check if we hit a quest separator */
            if (next_quest && next_quest < next_field) {
                break;
            }
        }

        /* Validate quest */
        if (quest->id != 0) {
            quest->state = QUEST_STATE_ACTIVE;
            g_quest.quest_count++;
            quest_index++;
        }

        /* Skip to next quest after '#' */
        char* next = strchr(ptr - 1, '#');
        if (next) {
            ptr = next + 1;
        } else {
            break;
        }
    }

    g_quest.list_loaded = 1;
    g_quest.update_received = (quest_index > 1) ? 1 : 0;

    LOG_INFO("Parsed %d quests from packet", g_quest.quest_count);
}

/*
 * Parse quest objective
 */
void quest_parse_objective(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 quest_id;
    int obj_index;
    QuestData* quest;

    quest_id = *(u32*)ptr; ptr += 4;
    obj_index = *(int*)ptr; ptr += 4;

    quest = quest_find(quest_id);
    if (!quest) return;

    if (obj_index >= 0 && obj_index < quest->objective_count) {
        quest->objectives[obj_index].current = *(u32*)ptr;
    }
}

/*
 * Handle quest update - FUN_00465de0 pattern
 */
void quest_handle_update(void* data, u32 size) {
    u8* ptr = (u8*)data;
    int quest_index;
    QuestData* quest;
    int i;

    /* Clear quest update flags */
    g_quest.update_received = 0;

    /* Parse quest index */
    quest_index = 0;
    while (ptr < (u8*)data + size) {
        quest = &g_quest.quests[quest_index];
        if (quest_index >= MAX_QUESTS) break;

        /* Parse quest ID */
        quest->id = *(u32*)ptr; ptr += 4;

        /* Parse quest name */
        strncpy(quest->name, (char*)ptr, sizeof(quest->name) - 1);
        ptr += strlen(quest->name) + 1;

        /* Parse description */
        strncpy(quest->description, (char*)ptr, sizeof(quest->description) - 1);
        ptr += strlen(quest->description) + 1;

        quest->state = QUEST_STATE_ACTIVE;
        quest_index++;
    }

    if (quest_index > 1) {
        g_quest.update_received = 1;
    }

    LOG_DEBUG("Quest update received, %d quests", quest_index);
}

/*
 * Handle achievement packet - FUN_00465f70 pattern
 * Delimiter: '|' (0x7C)
 */
void quest_handle_achievement(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 type;

    if (!data || size < 1) return;

    type = *ptr++;

    switch (type) {
        case ACHIEVEMENT_PKT_EARNED:    /* 'A' (0x41) - Achievement earned */
            g_quest.achievement_state = 3;
            LOG_DEBUG("Achievement earned");
            quest_parse_achievement(data, size, 1);
            break;

        case ACHIEVEMENT_PKT_PROGRESS:  /* 'C' (0x43) - Achievement progress */
            g_quest.achievement_state = 2;
            LOG_DEBUG("Achievement progress update");
            quest_parse_achievement(data, size, 0);
            break;

        case ACHIEVEMENT_PKT_MILESTONE: /* 'M' (0x4D) - Achievement milestone */
            g_quest.achievement_state = 1;
            LOG_DEBUG("Achievement milestone reached");
            break;

        case ACHIEVEMENT_PKT_REWARD:    /* 'V' (0x56) - Achievement reward */
            g_quest.achievement_state = 4;
            LOG_DEBUG("Achievement reward available");
            quest_parse_achievement(data, size, 2);
            break;

        default:
            LOG_DEBUG("Unknown achievement packet type: 0x%02X", type);
            break;
    }
}

/*
 * Parse achievement data
 */
void quest_parse_achievement(void* data, u32 size, int is_reward) {
    u8* ptr = (u8*)data;
    Achievement* ach;
    int i;

    for (i = 0; i < MAX_ACHIEVEMENTS && ptr < (u8*)data + size; i++) {
        ach = &g_quest.achievements[i];

        ach->id = *(u32*)ptr; ptr += 4;
        strncpy(ach->name, (char*)ptr, sizeof(ach->name) - 1);
        ptr += strlen(ach->name) + 1;

        ach->progress = *(u32*)ptr; ptr += 4;
        ach->target = *(u32*)ptr; ptr += 4;

        if (is_reward) {
            ach->reward_claimed = 1;
        }

        ach->unlocked = (ach->progress >= ach->target);
    }

    g_quest.achievement_count = i;
}

/*
 * Accept quest
 */
int quest_accept(u32 quest_id) {
    QuestData* quest;
    char params[32];

    quest = quest_find(quest_id);
    if (!quest) {
        return 0;
    }

    if (quest->state != QUEST_STATE_AVAILABLE && quest->state != QUEST_STATE_NONE) {
        LOG_WARN("Quest %u not available for acceptance", quest_id);
        return 0;
    }

    /* Send accept packet: QA|quest_id */
    _snprintf(params, sizeof(params), "%u", quest_id);
    if (!quest_send_command(QUEST_CMD_ACCEPT, params)) {
        return 0;
    }

    quest->state = QUEST_STATE_ACTIVE;
    g_quest.active_quest_id = quest_id;
    g_quest.in_quest = 1;

    LOG_INFO("Accepted quest: %s (ID: %u)", quest->name, quest_id);
    return 1;
}

/*
 * Complete quest
 */
int quest_complete(u32 quest_id) {
    QuestData* quest;
    char params[32];
    int i;

    quest = quest_find(quest_id);
    if (!quest) {
        return 0;
    }

    /* Check all objectives complete */
    for (i = 0; i < quest->objective_count; i++) {
        if (quest->objectives[i].current < quest->objectives[i].required) {
            LOG_WARN("Quest objectives not complete: %s", quest->name);
            return 0;
        }
    }

    /* Send complete packet: QC|quest_id */
    _snprintf(params, sizeof(params), "%u", quest_id);
    if (!quest_send_command(QUEST_CMD_COMPLETE, params)) {
        return 0;
    }

    quest->state = QUEST_STATE_COMPLETE;

    if (g_quest.active_quest_id == quest_id) {
        g_quest.active_quest_id = 0;
        g_quest.in_quest = 0;
    }

    LOG_INFO("Completed quest: %s (ID: %u)", quest->name, quest_id);
    return 1;
}

/*
 * Abandon quest
 */
int quest_abandon(u32 quest_id) {
    QuestData* quest;
    char params[32];

    quest = quest_find(quest_id);
    if (!quest) {
        return 0;
    }

    if (quest->state != QUEST_STATE_ACTIVE) {
        LOG_WARN("Quest %u not active", quest_id);
        return 0;
    }

    /* Send abandon packet: QX|quest_id */
    _snprintf(params, sizeof(params), "%u", quest_id);
    if (!quest_send_command(QUEST_CMD_ABANDON, params)) {
        return 0;
    }

    quest->state = QUEST_STATE_ABANDONED;

    if (g_quest.active_quest_id == quest_id) {
        g_quest.active_quest_id = 0;
        g_quest.in_quest = 0;
    }

    LOG_INFO("Abandoned quest: %s (ID: %u)", quest->name, quest_id);
    return 1;
}

/*
 * Claim quest reward
 */
int quest_claim_reward(u32 quest_id) {
    QuestData* quest;
    char params[32];

    quest = quest_find(quest_id);
    if (!quest) {
        return 0;
    }

    if (quest->state != QUEST_STATE_COMPLETE) {
        LOG_WARN("Quest %u not complete", quest_id);
        return 0;
    }

    /* Send claim packet: QR|quest_id */
    _snprintf(params, sizeof(params), "%u", quest_id);
    if (!quest_send_command(QUEST_CMD_CLAIM, params)) {
        return 0;
    }

    quest->reward_claimed = 1;
    quest->state = QUEST_STATE_REWARDED;

    LOG_INFO("Claimed reward for quest: %s (ID: %u)", quest->name, quest_id);
    return 1;
}

/*
 * Update quest progress
 */
void quest_update_progress(u32 quest_id, int objective_index, u32 progress) {
    QuestData* quest;

    quest = quest_find(quest_id);
    if (!quest) return;

    if (objective_index < 0 || objective_index >= quest->objective_count) {
        return;
    }

    quest->objectives[objective_index].current = progress;

    /* Check if objective complete */
    if (progress >= quest->objectives[objective_index].required) {
        quest->objectives[objective_index].completed = 1;
        LOG_DEBUG("Quest objective %d complete", objective_index);
    }
}

/*
 * Find quest by ID
 */
QuestData* quest_find(u32 quest_id) {
    int i;

    for (i = 0; i < MAX_QUESTS; i++) {
        if (g_quest.quests[i].id == quest_id) {
            return &g_quest.quests[i];
        }
    }

    return NULL;
}

/*
 * Get active quest
 */
QuestData* quest_get_active(void) {
    if (g_quest.active_quest_id == 0) {
        return NULL;
    }
    return quest_find(g_quest.active_quest_id);
}

/*
 * Get quest by index
 */
QuestData* quest_get_by_index(int index) {
    if (index < 0 || index >= MAX_QUESTS) {
        return NULL;
    }
    return &g_quest.quests[index];
}

/*
 * Get quest count
 */
int quest_get_count(void) {
    return g_quest.quest_count;
}

/*
 * Check if in quest
 */
int quest_is_active(void) {
    return g_quest.in_quest;
}

/*
 * Request quest list from server
 */
void quest_request_list(void) {
    /* Send quest list request: QL */
    quest_send_command("QL", NULL);
    LOG_DEBUG("Requested quest list from server");
}

/*
 * Get achievement
 */
Achievement* quest_get_achievement(u32 achievement_id) {
    int i;

    for (i = 0; i < MAX_ACHIEVEMENTS; i++) {
        if (g_quest.achievements[i].id == achievement_id) {
            return &g_quest.achievements[i];
        }
    }

    return NULL;
}

/*
 * Get achievement count
 */
int quest_get_achievement_count(void) {
    return g_quest.achievement_count;
}

/*
 * Claim achievement reward
 */
int quest_claim_achievement_reward(u32 achievement_id) {
    Achievement* ach;
    char params[32];

    ach = quest_get_achievement(achievement_id);
    if (!ach) {
        LOG_WARN("Achievement %u not found", achievement_id);
        return 0;
    }

    if (!ach->unlocked || ach->reward_claimed) {
        LOG_WARN("Achievement %u not available for claim", achievement_id);
        return 0;
    }

    /* Send claim packet: AR|achievement_id */
    _snprintf(params, sizeof(params), "%u", achievement_id);
    network_send("AR|", 3);
    network_send(params, strlen(params));
    network_send("\n", 1);

    ach->reward_claimed = 1;
    LOG_INFO("Claimed achievement reward: %s (ID: %u)", ach->name, achievement_id);
    return 1;
}

/*
 * Handle quest list - wrapper for quest_handle_list_packet
 */
void quest_handle_list(void* data, u32 size) {
    quest_handle_list_packet(data, size);
}

/*
 * Handle achievement - wrapper for quest_handle_achievement
 */
void achievement_handle(void* data, u32 size) {
    quest_handle_achievement(data, size);
}
