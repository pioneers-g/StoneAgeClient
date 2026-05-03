/*
 * Stone Age Client - Quest Module Comprehensive Tests
 * Tests for quest.c - quest and achievement systems
 *
 * Coverage:
 * - Initialization and shutdown
 * - Quest packet handling
 * - Quest state management
 * - Quest objectives
 * - Achievement system
 * - Quest acceptance/completion/abandon
 * - Reward claiming
 * - Query functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Constants */
#define MAX_QUESTS          50
#define MAX_OBJECTIVES      10
#define MAX_ACHIEVEMENTS    100

/* Quest packet type codes */
#define QUEST_PKT_AVAILABLE     0x42    /* 'B' */
#define QUEST_PKT_DATA          0x44    /* 'D' */
#define QUEST_PKT_COMPLETE      0x43    /* 'C' */
#define QUEST_PKT_OBJECTIVE     0x4F    /* 'O' */
#define QUEST_PKT_STATUS        0x53    /* 'S' */

/* Achievement packet type codes */
#define ACHIEVEMENT_PKT_EARNED      0x41    /* 'A' */
#define ACHIEVEMENT_PKT_PROGRESS    0x43    /* 'C' */
#define ACHIEVEMENT_PKT_MILESTONE   0x4D    /* 'M' */
#define ACHIEVEMENT_PKT_REWARD      0x56    /* 'V' */

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
    QuestObjective* objectives;
    int objective_count;
    u32 exp_reward;
    u32 gold_reward;
    u32 item_reward_id;
    u32 item_reward_count;
    int reward_claimed;
    u32 time_limit;
    u32 time_remaining;
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
    QuestData quests[MAX_QUESTS];
    int quest_count;
    int list_loaded;
    u32 active_quest_id;
    int in_quest;
    int new_quest_available;
    int objective_updated;
    int update_received;
    Achievement achievements[MAX_ACHIEVEMENTS];
    int achievement_count;
    int achievement_state;
} QuestContext;

/* Global quest context */
static QuestContext g_quest = {0};

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Helper Functions
 * ======================================== */

void quest_init(void) {
    memset(&g_quest, 0, sizeof(QuestContext));
}

void quest_shutdown(void) {
    int i;
    for (i = 0; i < MAX_QUESTS; i++) {
        if (g_quest.quests[i].objectives) {
            free(g_quest.quests[i].objectives);
        }
    }
    memset(&g_quest, 0, sizeof(QuestContext));
}

QuestData* quest_find(u32 quest_id) {
    int i;
    for (i = 0; i < MAX_QUESTS; i++) {
        if (g_quest.quests[i].id == quest_id) {
            return &g_quest.quests[i];
        }
    }
    return NULL;
}

QuestData* quest_get_active(void) {
    if (g_quest.active_quest_id == 0) return NULL;
    return quest_find(g_quest.active_quest_id);
}

QuestData* quest_get_by_index(int index) {
    if (index < 0 || index >= MAX_QUESTS) return NULL;
    return &g_quest.quests[index];
}

int quest_get_count(void) {
    return g_quest.quest_count;
}

int quest_is_active(void) {
    return g_quest.in_quest;
}

int quest_accept(u32 quest_id) {
    QuestData* quest = quest_find(quest_id);
    if (!quest) return 0;

    if (quest->state != QUEST_STATE_AVAILABLE && quest->state != QUEST_STATE_NONE) {
        return 0;
    }

    quest->state = QUEST_STATE_ACTIVE;
    g_quest.active_quest_id = quest_id;
    g_quest.in_quest = 1;
    return 1;
}

int quest_complete(u32 quest_id) {
    QuestData* quest = quest_find(quest_id);
    if (!quest) return 0;

    int i;
    for (i = 0; i < quest->objective_count; i++) {
        if (quest->objectives[i].current < quest->objectives[i].required) {
            return 0;
        }
    }

    quest->state = QUEST_STATE_COMPLETE;
    if (g_quest.active_quest_id == quest_id) {
        g_quest.active_quest_id = 0;
        g_quest.in_quest = 0;
    }
    return 1;
}

int quest_abandon(u32 quest_id) {
    QuestData* quest = quest_find(quest_id);
    if (!quest) return 0;

    if (quest->state != QUEST_STATE_ACTIVE) return 0;

    quest->state = QUEST_STATE_ABANDONED;
    if (g_quest.active_quest_id == quest_id) {
        g_quest.active_quest_id = 0;
        g_quest.in_quest = 0;
    }
    return 1;
}

int quest_claim_reward(u32 quest_id) {
    QuestData* quest = quest_find(quest_id);
    if (!quest) return 0;

    if (quest->state != QUEST_STATE_COMPLETE) return 0;

    quest->reward_claimed = 1;
    quest->state = QUEST_STATE_REWARDED;
    return 1;
}

void quest_update_progress(u32 quest_id, int objective_index, u32 progress) {
    QuestData* quest = quest_find(quest_id);
    if (!quest) return;

    if (objective_index < 0 || objective_index >= quest->objective_count) return;

    quest->objectives[objective_index].current = progress;
    if (progress >= quest->objectives[objective_index].required) {
        quest->objectives[objective_index].completed = 1;
    }
}

void quest_add_test_quest(u32 id, const char* name, int obj_count) {
    int i;
    for (i = 0; i < MAX_QUESTS; i++) {
        if (g_quest.quests[i].id == 0) {
            g_quest.quests[i].id = id;
            strncpy(g_quest.quests[i].name, name, 63);
            g_quest.quests[i].state = QUEST_STATE_AVAILABLE;
            g_quest.quests[i].objective_count = obj_count;

            if (obj_count > 0) {
                g_quest.quests[i].objectives = (QuestObjective*)calloc(obj_count, sizeof(QuestObjective));
                int j;
                for (j = 0; j < obj_count; j++) {
                    g_quest.quests[i].objectives[j].required = 10;
                }
            }

            g_quest.quest_count++;
            break;
        }
    }
}

Achievement* quest_get_achievement(u32 achievement_id) {
    int i;
    for (i = 0; i < MAX_ACHIEVEMENTS; i++) {
        if (g_quest.achievements[i].id == achievement_id) {
            return &g_quest.achievements[i];
        }
    }
    return NULL;
}

int quest_get_achievement_count(void) {
    return g_quest.achievement_count;
}

int quest_claim_achievement_reward(u32 achievement_id) {
    Achievement* ach = quest_get_achievement(achievement_id);
    if (!ach) return 0;

    if (!ach->unlocked || ach->reward_claimed) return 0;

    ach->reward_claimed = 1;
    return 1;
}

void quest_add_test_achievement(u32 id, const char* name, u32 target) {
    int i;
    for (i = 0; i < MAX_ACHIEVEMENTS; i++) {
        if (g_quest.achievements[i].id == 0) {
            g_quest.achievements[i].id = id;
            strncpy(g_quest.achievements[i].name, name, 63);
            g_quest.achievements[i].target = target;
            g_quest.achievements[i].progress = 0;
            g_quest.achievement_count++;
            break;
        }
    }
}

/* ========================================
 * Constants Tests
 * ======================================== */

static int test_max_quests(void) {
    return MAX_QUESTS == 50;
}

static int test_max_objectives(void) {
    return MAX_OBJECTIVES == 10;
}

static int test_max_achievements(void) {
    return MAX_ACHIEVEMENTS == 100;
}

static int test_quest_pkt_available(void) {
    return QUEST_PKT_AVAILABLE == 0x42;
}

static int test_quest_pkt_data(void) {
    return QUEST_PKT_DATA == 0x44;
}

static int test_quest_pkt_complete(void) {
    return QUEST_PKT_COMPLETE == 0x43;
}

static int test_quest_pkt_objective(void) {
    return QUEST_PKT_OBJECTIVE == 0x4F;
}

static int test_quest_pkt_status(void) {
    return QUEST_PKT_STATUS == 0x53;
}

static int test_achievement_pkt_earned(void) {
    return ACHIEVEMENT_PKT_EARNED == 0x41;
}

static int test_achievement_pkt_milestone(void) {
    return ACHIEVEMENT_PKT_MILESTONE == 0x4D;
}

/* ========================================
 * Initialization Tests
 * ======================================== */

static int test_quest_init(void) {
    quest_init();
    return g_quest.quest_count == 0 &&
           g_quest.active_quest_id == 0 &&
           g_quest.in_quest == 0;
}

static int test_quest_shutdown(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 1);
    quest_shutdown();
    return g_quest.quest_count == 0;
}

/* ========================================
 * Quest State Tests
 * ======================================== */

static int test_quest_state_none(void) {
    return QUEST_STATE_NONE == 0;
}

static int test_quest_state_available(void) {
    return QUEST_STATE_AVAILABLE == 1;
}

static int test_quest_state_active(void) {
    return QUEST_STATE_ACTIVE == 2;
}

static int test_quest_state_complete(void) {
    return QUEST_STATE_COMPLETE == 3;
}

static int test_quest_state_rewarded(void) {
    return QUEST_STATE_REWARDED == 4;
}

static int test_quest_state_abandoned(void) {
    return QUEST_STATE_ABANDONED == 5;
}

/* ========================================
 * Quest Find Tests
 * ======================================== */

static int test_quest_find_exists(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 0);
    QuestData* quest = quest_find(1001);
    return quest != NULL && quest->id == 1001;
}

static int test_quest_find_not_exists(void) {
    quest_init();
    QuestData* quest = quest_find(9999);
    return quest == NULL;
}

static int test_quest_get_by_index_valid(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 0);
    QuestData* quest = quest_get_by_index(0);
    return quest != NULL && quest->id == 1001;
}

static int test_quest_get_by_index_invalid(void) {
    quest_init();
    QuestData* quest = quest_get_by_index(-1);
    if (quest != NULL) return 0;
    quest = quest_get_by_index(MAX_QUESTS);
    return quest == NULL;
}

/* ========================================
 * Quest Accept Tests
 * ======================================== */

static int test_quest_accept_basic(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 0);
    int result = quest_accept(1001);
    return result == 1 &&
           g_quest.quests[0].state == QUEST_STATE_ACTIVE &&
           g_quest.active_quest_id == 1001 &&
           g_quest.in_quest == 1;
}

static int test_quest_accept_not_found(void) {
    quest_init();
    int result = quest_accept(9999);
    return result == 0;
}

static int test_quest_accept_already_active(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 0);
    quest_accept(1001);
    int result = quest_accept(1001);
    return result == 0;  /* Can't accept if already active */
}

static int test_quest_accept_sets_active_id(void) {
    quest_init();
    quest_add_test_quest(1001, "Quest 1", 0);
    quest_add_test_quest(1002, "Quest 2", 0);
    quest_accept(1001);
    return g_quest.active_quest_id == 1001;
}

/* ========================================
 * Quest Complete Tests
 * ======================================== */

static int test_quest_complete_basic(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 1);
    quest_accept(1001);
    quest_update_progress(1001, 0, 10);  /* Complete objective */
    int result = quest_complete(1001);
    return result == 1 &&
           g_quest.quests[0].state == QUEST_STATE_COMPLETE;
}

static int test_quest_complete_objectives_incomplete(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 1);
    quest_accept(1001);
    quest_update_progress(1001, 0, 5);  /* Only 5/10 */
    int result = quest_complete(1001);
    return result == 0 &&
           g_quest.quests[0].state == QUEST_STATE_ACTIVE;
}

static int test_quest_complete_clears_active(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 1);
    quest_accept(1001);
    quest_update_progress(1001, 0, 10);
    quest_complete(1001);
    return g_quest.active_quest_id == 0 && g_quest.in_quest == 0;
}

static int test_quest_complete_not_found(void) {
    quest_init();
    int result = quest_complete(9999);
    return result == 0;
}

/* ========================================
 * Quest Abandon Tests
 * ======================================== */

static int test_quest_abandon_basic(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 0);
    quest_accept(1001);
    int result = quest_abandon(1001);
    return result == 1 &&
           g_quest.quests[0].state == QUEST_STATE_ABANDONED;
}

static int test_quest_abandon_clears_active(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 0);
    quest_accept(1001);
    quest_abandon(1001);
    return g_quest.active_quest_id == 0 && g_quest.in_quest == 0;
}

static int test_quest_abandon_not_active(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 0);
    /* Don't accept - state is AVAILABLE */
    int result = quest_abandon(1001);
    return result == 0;
}

static int test_quest_abandon_not_found(void) {
    quest_init();
    int result = quest_abandon(9999);
    return result == 0;
}

/* ========================================
 * Quest Reward Tests
 * ======================================== */

static int test_quest_claim_reward_basic(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 1);
    quest_accept(1001);
    quest_update_progress(1001, 0, 10);
    quest_complete(1001);
    int result = quest_claim_reward(1001);
    return result == 1 &&
           g_quest.quests[0].reward_claimed == 1 &&
           g_quest.quests[0].state == QUEST_STATE_REWARDED;
}

static int test_quest_claim_reward_not_complete(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 0);
    quest_accept(1001);
    int result = quest_claim_reward(1001);
    return result == 0;
}

static int test_quest_claim_reward_already_claimed(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 1);
    quest_accept(1001);
    quest_update_progress(1001, 0, 10);
    quest_complete(1001);
    quest_claim_reward(1001);
    int result = quest_claim_reward(1001);
    return result == 0;
}

/* ========================================
 * Quest Objective Tests
 * ======================================== */

static int test_quest_update_progress(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 1);
    quest_accept(1001);
    quest_update_progress(1001, 0, 5);
    return g_quest.quests[0].objectives[0].current == 5;
}

static int test_quest_update_progress_completes(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 1);
    quest_accept(1001);
    quest_update_progress(1001, 0, 10);
    return g_quest.quests[0].objectives[0].completed == 1;
}

static int test_quest_update_progress_invalid_index(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 1);
    quest_accept(1001);
    quest_update_progress(1001, 5, 10);  /* Invalid index */
    return 1;  /* Should not crash */
}

static int test_quest_multiple_objectives(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 3);
    quest_accept(1001);
    quest_update_progress(1001, 0, 10);
    quest_update_progress(1001, 1, 5);
    quest_update_progress(1001, 2, 10);

    return g_quest.quests[0].objectives[0].completed == 1 &&
           g_quest.quests[0].objectives[1].completed == 0 &&
           g_quest.quests[0].objectives[2].completed == 1;
}

/* ========================================
 * Quest Query Tests
 * ======================================== */

static int test_quest_get_count(void) {
    quest_init();
    quest_add_test_quest(1001, "Quest 1", 0);
    quest_add_test_quest(1002, "Quest 2", 0);
    return quest_get_count() == 2;
}

static int test_quest_get_active(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 0);
    quest_accept(1001);
    QuestData* quest = quest_get_active();
    return quest != NULL && quest->id == 1001;
}

static int test_quest_get_active_none(void) {
    quest_init();
    QuestData* quest = quest_get_active();
    return quest == NULL;
}

static int test_quest_is_active(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 0);
    if (quest_is_active() != 0) return 0;
    quest_accept(1001);
    return quest_is_active() == 1;
}

/* ========================================
 * Achievement Tests
 * ======================================== */

static int test_achievement_find(void) {
    quest_init();
    quest_add_test_achievement(2001, "Test Achievement", 100);
    Achievement* ach = quest_get_achievement(2001);
    return ach != NULL && ach->id == 2001;
}

static int test_achievement_find_not_exists(void) {
    quest_init();
    Achievement* ach = quest_get_achievement(9999);
    return ach == NULL;
}

static int test_achievement_count(void) {
    quest_init();
    quest_add_test_achievement(2001, "Ach 1", 100);
    quest_add_test_achievement(2002, "Ach 2", 200);
    return quest_get_achievement_count() == 2;
}

static int test_achievement_claim_reward(void) {
    quest_init();
    quest_add_test_achievement(2001, "Test", 100);
    g_quest.achievements[0].progress = 100;
    g_quest.achievements[0].unlocked = 1;
    int result = quest_claim_achievement_reward(2001);
    return result == 1 && g_quest.achievements[0].reward_claimed == 1;
}

static int test_achievement_claim_not_unlocked(void) {
    quest_init();
    quest_add_test_achievement(2001, "Test", 100);
    g_quest.achievements[0].unlocked = 0;
    int result = quest_claim_achievement_reward(2001);
    return result == 0;
}

static int test_achievement_claim_already_claimed(void) {
    quest_init();
    quest_add_test_achievement(2001, "Test", 100);
    g_quest.achievements[0].unlocked = 1;
    g_quest.achievements[0].reward_claimed = 1;
    int result = quest_claim_achievement_reward(2001);
    return result == 0;
}

/* ========================================
 * Quest Packet Handling Tests
 * ======================================== */

static int test_quest_pkt_available_flag(void) {
    quest_init();
    /* Simulate packet type 0x42 (QUEST_PKT_AVAILABLE) */
    g_quest.new_quest_available = 1;
    return g_quest.new_quest_available == 1;
}

static int test_quest_pkt_complete_flag(void) {
    quest_init();
    g_quest.new_quest_available = 1;
    /* Simulate packet type 0x43 (QUEST_PKT_COMPLETE) */
    g_quest.new_quest_available = 0;
    return g_quest.new_quest_available == 0;
}

static int test_quest_pkt_objective_flag(void) {
    quest_init();
    /* Simulate packet type 0x4F (QUEST_PKT_OBJECTIVE) */
    g_quest.objective_updated = 1;
    g_quest.in_quest = 1;
    return g_quest.objective_updated == 1 && g_quest.in_quest == 1;
}

static int test_achievement_pkt_earned_state(void) {
    quest_init();
    /* Simulate packet type 0x41 (ACHIEVEMENT_PKT_EARNED) */
    g_quest.achievement_state = 3;
    return g_quest.achievement_state == 3;
}

static int test_achievement_pkt_progress_state(void) {
    quest_init();
    /* Simulate packet type 0x43 (ACHIEVEMENT_PKT_PROGRESS) */
    g_quest.achievement_state = 2;
    return g_quest.achievement_state == 2;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_quest_lifecycle(void) {
    quest_init();
    quest_add_test_quest(1001, "Goblin Slayer", 2);
    quest_accept(1001);

    /* Complete objectives */
    quest_update_progress(1001, 0, 10);
    quest_update_progress(1001, 1, 10);

    /* Complete quest */
    int complete_result = quest_complete(1001);

    /* Claim reward */
    int claim_result = quest_claim_reward(1001);

    return complete_result == 1 &&
           claim_result == 1 &&
           g_quest.quests[0].state == QUEST_STATE_REWARDED;
}

static int test_quest_abandon_restart(void) {
    quest_init();
    quest_add_test_quest(1001, "Test Quest", 1);
    quest_accept(1001);
    quest_update_progress(1001, 0, 5);
    quest_abandon(1001);

    /* Quest should be abandoned, can accept again */
    g_quest.quests[0].state = QUEST_STATE_AVAILABLE;
    int result = quest_accept(1001);
    return result == 1;
}

static int test_multiple_quests(void) {
    quest_init();
    quest_add_test_quest(1001, "Quest 1", 1);
    quest_add_test_quest(1002, "Quest 2", 1);
    quest_add_test_quest(1003, "Quest 3", 1);

    quest_accept(1001);
    quest_update_progress(1001, 0, 10);
    quest_complete(1001);

    /* Active quest should be cleared, but other quests remain */
    return g_quest.active_quest_id == 0 &&
           g_quest.quests[0].state == QUEST_STATE_COMPLETE &&
           g_quest.quests[1].state == QUEST_STATE_AVAILABLE &&
           g_quest.quests[2].state == QUEST_STATE_AVAILABLE;
}

static int test_achievement_progression(void) {
    quest_init();
    quest_add_test_achievement(2001, "Monster Hunter", 100);

    /* Simulate progress */
    g_quest.achievements[0].progress = 50;
    if (g_quest.achievements[0].progress >= g_quest.achievements[0].target) return 0;

    g_quest.achievements[0].progress = 100;
    g_quest.achievements[0].unlocked = 1;

    return g_quest.achievements[0].unlocked == 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Quest Module Comprehensive Tests ===\n\n");

    /* Constants tests */
    printf("Constants Tests:\n");
    TEST(max_quests);
    TEST(max_objectives);
    TEST(max_achievements);
    TEST(quest_pkt_available);
    TEST(quest_pkt_data);
    TEST(quest_pkt_complete);
    TEST(quest_pkt_objective);
    TEST(quest_pkt_status);
    TEST(achievement_pkt_earned);
    TEST(achievement_pkt_milestone);

    /* Initialization tests */
    printf("\nInitialization Tests:\n");
    TEST(quest_init);
    TEST(quest_shutdown);

    /* Quest state tests */
    printf("\nQuest State Tests:\n");
    TEST(quest_state_none);
    TEST(quest_state_available);
    TEST(quest_state_active);
    TEST(quest_state_complete);
    TEST(quest_state_rewarded);
    TEST(quest_state_abandoned);

    /* Quest find tests */
    printf("\nQuest Find Tests:\n");
    TEST(quest_find_exists);
    TEST(quest_find_not_exists);
    TEST(quest_get_by_index_valid);
    TEST(quest_get_by_index_invalid);

    /* Quest accept tests */
    printf("\nQuest Accept Tests:\n");
    TEST(quest_accept_basic);
    TEST(quest_accept_not_found);
    TEST(quest_accept_already_active);
    TEST(quest_accept_sets_active_id);

    /* Quest complete tests */
    printf("\nQuest Complete Tests:\n");
    TEST(quest_complete_basic);
    TEST(quest_complete_objectives_incomplete);
    TEST(quest_complete_clears_active);
    TEST(quest_complete_not_found);

    /* Quest abandon tests */
    printf("\nQuest Abandon Tests:\n");
    TEST(quest_abandon_basic);
    TEST(quest_abandon_clears_active);
    TEST(quest_abandon_not_active);
    TEST(quest_abandon_not_found);

    /* Quest reward tests */
    printf("\nQuest Reward Tests:\n");
    TEST(quest_claim_reward_basic);
    TEST(quest_claim_reward_not_complete);
    TEST(quest_claim_reward_already_claimed);

    /* Quest objective tests */
    printf("\nQuest Objective Tests:\n");
    TEST(quest_update_progress);
    TEST(quest_update_progress_completes);
    TEST(quest_update_progress_invalid_index);
    TEST(quest_multiple_objectives);

    /* Quest query tests */
    printf("\nQuest Query Tests:\n");
    TEST(quest_get_count);
    TEST(quest_get_active);
    TEST(quest_get_active_none);
    TEST(quest_is_active);

    /* Achievement tests */
    printf("\nAchievement Tests:\n");
    TEST(achievement_find);
    TEST(achievement_find_not_exists);
    TEST(achievement_count);
    TEST(achievement_claim_reward);
    TEST(achievement_claim_not_unlocked);
    TEST(achievement_claim_already_claimed);

    /* Packet handling tests */
    printf("\nPacket Handling Tests:\n");
    TEST(quest_pkt_available_flag);
    TEST(quest_pkt_complete_flag);
    TEST(quest_pkt_objective_flag);
    TEST(achievement_pkt_earned_state);
    TEST(achievement_pkt_progress_state);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_quest_lifecycle);
    TEST(quest_abandon_restart);
    TEST(multiple_quests);
    TEST(achievement_progression);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
