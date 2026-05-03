/*
 * Stone Age Client - Quest System Header
 * Reverse engineered from sa_9061.exe
 *
 * Protocol Analysis:
 * FUN_00465d20 - Quest packet router (delimiter: '|')
 * FUN_00465de0 - Quest update handler (delimiter: '#')
 * FUN_00465f70 - Achievement handler (delimiter: '|')
 *
 * Packet Types:
 * - 'B' (0x42): New quest available
 * - 'D' (0x44): Quest data
 * - 'C' (0x43): Quest complete
 * - 'O' (0x4F): Objective update
 * - 'S' (0x53): Quest status
 *
 * Achievement Types:
 * - 'A' (0x41): Achievement earned
 * - 'C' (0x43): Progress update
 * - 'M' (0x4D): Milestone
 * - 'V' (0x56): Reward claim
 */

#ifndef QUEST_H
#define QUEST_H

#include "types.h"

/* Constants */
#define MAX_QUESTS          50
#define MAX_OBJECTIVES      10
#define MAX_ACHIEVEMENTS    100

/* Quest packet type codes */
#define QUEST_PKT_AVAILABLE     0x42    /* 'B' - New quest available */
#define QUEST_PKT_DATA          0x44    /* 'D' - Quest data */
#define QUEST_PKT_COMPLETE      0x43    /* 'C' - Quest complete */
#define QUEST_PKT_OBJECTIVE     0x4F    /* 'O' - Objective update */
#define QUEST_PKT_STATUS        0x53    /* 'S' - Quest status */

/* Achievement packet type codes */
#define ACHIEVEMENT_PKT_EARNED      0x41    /* 'A' - Achievement earned */
#define ACHIEVEMENT_PKT_PROGRESS    0x43    /* 'C' - Progress update */
#define ACHIEVEMENT_PKT_MILESTONE   0x4D    /* 'M' - Milestone */
#define ACHIEVEMENT_PKT_REWARD      0x56    /* 'V' - Reward claim */

/* Protocol commands */
#define QUEST_CMD_ACCEPT    "QA"
#define QUEST_CMD_COMPLETE  "QC"
#define QUEST_CMD_ABANDON   "QX"
#define QUEST_CMD_CLAIM     "QR"
#define QUEST_CMD_LIST      "QL"

/* Quest states */
typedef enum {
    QUEST_STATE_NONE = 0,
    QUEST_STATE_AVAILABLE = 1,
    QUEST_STATE_ACTIVE = 2,
    QUEST_STATE_COMPLETE = 3,
    QUEST_STATE_REWARDED = 4,
    QUEST_STATE_ABANDONED = 5
} QuestState;

/* Quest objective */
typedef struct {
    u32 type;
    u32 target_id;
    u32 required;
    u32 current;
    char description[64];
    int completed;
} QuestObjective;

/* Quest data */
typedef struct {
    u32 id;
    char name[64];
    char description[256];
    QuestState state;
    u16 level_required;
    u16 level_recommended;

    /* Objectives */
    QuestObjective* objectives;
    int objective_count;

    /* Rewards */
    u32 exp_reward;
    u32 gold_reward;
    u32 item_reward_id;
    u32 item_reward_count;
    int reward_claimed;

    /* Time limit (0 = no limit) */
    u32 time_limit;
    u32 time_remaining;

    /* NPC */
    u32 start_npc;
    u32 end_npc;

} QuestData;

/* Achievement */
typedef struct {
    u32 id;
    char name[64];
    char description[128];
    u32 category;
    u32 progress;
    u32 target;
    u32 exp_reward;
    u32 item_reward;
    int unlocked;
    int reward_claimed;
} Achievement;

/* Quest context */
typedef struct {
    /* Quests */
    QuestData quests[MAX_QUESTS];
    int quest_count;
    int list_loaded;

    /* Active quest */
    u32 active_quest_id;
    int in_quest;

    /* Flags */
    int new_quest_available;
    int objective_updated;
    int update_received;

    /* Achievements */
    Achievement achievements[MAX_ACHIEVEMENTS];
    int achievement_count;
    int achievement_state;

} QuestContext;

/* Global quest context */
extern QuestContext g_quest;

/* Initialization */
int quest_init(void);
void quest_shutdown(void);

/* Packet handlers */
void quest_handle_list_packet(void* data, u32 size);
void quest_parse_list(void* data, u32 size);
void quest_parse_objective(void* data, u32 size);
void quest_handle_update(void* data, u32 size);
void quest_handle_achievement(void* data, u32 size);
void quest_parse_achievement(void* data, u32 size, int is_reward);

/* Quest management */
int quest_accept(u32 quest_id);
int quest_complete(u32 quest_id);
int quest_abandon(u32 quest_id);
int quest_claim_reward(u32 quest_id);
void quest_update_progress(u32 quest_id, int objective_index, u32 progress);

/* Query */
QuestData* quest_find(u32 quest_id);
QuestData* quest_get_active(void);
QuestData* quest_get_by_index(int index);
int quest_get_count(void);
int quest_is_active(void);

/* Request */
void quest_request_list(void);

/* Achievements */
Achievement* quest_get_achievement(u32 achievement_id);
int quest_get_achievement_count(void);
int quest_claim_achievement_reward(u32 achievement_id);

#endif /* QUEST_H */
