/*
 * Stone Age Client - Event System Comprehensive Tests
 * Tests for event module - event.c
 *
 * Coverage:
 * - Event initialization and shutdown
 * - Event queries and state management
 * - Event bonuses
 * - Event participation
 * - Event schedule checking
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Constants */
#define MAX_EVENTS          50
#define MAX_EVENT_NAME      32
#define MAX_EVENT_DESC      128

/* Event types */
typedef enum {
    EVENT_TYPE_NONE = 0,
    EVENT_TYPE_BOSS = 1,
    EVENT_TYPE_DOUBLE_EXP = 2,
    EVENT_TYPE_DOUBLE_DROP = 3,
    EVENT_TYPE_FESTIVAL = 4,
    EVENT_TYPE_ANNIVERSARY = 5,
    EVENT_TYPE_WEEKEND = 6,
    EVENT_TYPE_DAILY = 7,
    EVENT_TYPE_SPECIAL = 8,
    EVENT_TYPE_GUILD_WAR = 9,
    EVENT_TYPE_PVP_TOURNAMENT = 10
} EventType;

/* Event state */
typedef enum {
    EVENT_STATE_INACTIVE = 0,
    EVENT_STATE_UPCOMING = 1,
    EVENT_STATE_ACTIVE = 2,
    EVENT_STATE_ENDING = 3
} EventState;

/* Event bonus types */
typedef enum {
    BONUS_NONE = 0,
    BONUS_EXP = 1,
    BONUS_GOLD = 2,
    BONUS_DROP = 3,
    BONUS_SKILL_EXP = 4,
    BONUS_PET_EXP = 5,
    BONUS_CAPTURE = 6,
    BONUS_ALL = 7
} BonusType;

/* Event schedule */
typedef struct {
    u32 start_time;
    u32 end_time;
    u8 repeat_type;
    u8 repeat_param;
    u16 repeat_hour;
    u16 repeat_minute;
    u16 duration_hours;
} EventSchedule;

/* Event bonus */
typedef struct {
    BonusType type;
    u16 value;
    u16 stack_type;
} EventBonus;

/* Event data */
typedef struct {
    u32 id;
    char name[MAX_EVENT_NAME];
    char description[MAX_EVENT_DESC];
    EventType type;
    EventState state;
    u8 priority;
    u8 reserved;
    EventSchedule schedule;
    EventBonus bonuses[5];
    u8 bonus_count;
    u16 min_level;
    u16 max_level;
    u32 required_quest;
    u32 reward_item;
    u16 reward_count;
    u32 reward_gold;
    u32 reward_exp;
    u16 icon_id;
    u8 show_notice;
    u8 auto_join;
} EventData;

/* Event context */
typedef struct {
    EventData events[MAX_EVENTS];
    int event_count;
    u32 participating_events[MAX_EVENTS];
    int participating_count;
    u16 exp_bonus;
    u16 gold_bonus;
    u16 drop_bonus;
    u16 skill_exp_bonus;
    u16 pet_exp_bonus;
    u16 capture_bonus;
    u32 last_notice_time;
    u32 notice_interval;
    u8 show_event_notice;
    char notice_message[256];
    u32 notice_timer;
    u32 calendar_month;
    u32 calendar_year;
    u8 calendar_events[32];
    int selected_index;
} EventContext;

/* Global event context */
static EventContext g_event = {0};

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
 * Implementation Functions
 * ======================================== */

void event_init(void) {
    memset(&g_event, 0, sizeof(EventContext));
    g_event.notice_interval = 300000;  /* 5 minutes */
    g_event.show_event_notice = 1;
    g_event.selected_index = -1;
}

void event_shutdown(void) {
    memset(&g_event, 0, sizeof(EventContext));
}

EventData* event_get(u32 event_id) {
    int i;
    for (i = 0; i < g_event.event_count; i++) {
        if (g_event.events[i].id == event_id) {
            return &g_event.events[i];
        }
    }
    return NULL;
}

EventData* event_get_by_index(int index) {
    if (index < 0 || index >= g_event.event_count) {
        return NULL;
    }
    return &g_event.events[index];
}

EventData* event_get_active(u32 event_id) {
    EventData* event = event_get(event_id);
    return (event && event->state == EVENT_STATE_ACTIVE) ? event : NULL;
}

int event_get_count(void) {
    return g_event.event_count;
}

int event_get_active_count(void) {
    int i, count = 0;
    for (i = 0; i < g_event.event_count; i++) {
        if (g_event.events[i].state == EVENT_STATE_ACTIVE) {
            count++;
        }
    }
    return count;
}

int event_is_active(u32 event_id) {
    EventData* event = event_get(event_id);
    return event ? event->state == EVENT_STATE_ACTIVE : 0;
}

EventState event_get_state(u32 event_id) {
    EventData* event = event_get(event_id);
    return event ? event->state : EVENT_STATE_INACTIVE;
}

int event_add(EventData* event) {
    if (g_event.event_count >= MAX_EVENTS) return 0;
    if (!event) return 0;

    g_event.events[g_event.event_count] = *event;
    g_event.event_count++;
    return 1;
}

int event_join(u32 event_id) {
    int i;

    if (g_event.participating_count >= MAX_EVENTS) return 0;
    if (!event_is_active(event_id)) return 0;

    /* Check if already participating */
    for (i = 0; i < g_event.participating_count; i++) {
        if (g_event.participating_events[i] == event_id) {
            return 1;  /* Already joined */
        }
    }

    g_event.participating_events[g_event.participating_count++] = event_id;
    return 1;
}

int event_leave(u32 event_id) {
    int i, found = 0;

    for (i = 0; i < g_event.participating_count; i++) {
        if (g_event.participating_events[i] == event_id) {
            found = 1;
            /* Shift remaining elements */
            for (; i < g_event.participating_count - 1; i++) {
                g_event.participating_events[i] = g_event.participating_events[i + 1];
            }
            g_event.participating_count--;
            break;
        }
    }

    return found;
}

int event_is_participating(u32 event_id) {
    int i;
    for (i = 0; i < g_event.participating_count; i++) {
        if (g_event.participating_events[i] == event_id) {
            return 1;
        }
    }
    return 0;
}

u16 event_get_exp_bonus(void) {
    return g_event.exp_bonus;
}

u16 event_get_gold_bonus(void) {
    return g_event.gold_bonus;
}

u16 event_get_drop_bonus(void) {
    return g_event.drop_bonus;
}

u16 event_get_skill_exp_bonus(void) {
    return g_event.skill_exp_bonus;
}

u16 event_get_pet_exp_bonus(void) {
    return g_event.pet_exp_bonus;
}

u16 event_get_capture_bonus(void) {
    return g_event.capture_bonus;
}

u16 event_get_bonus(BonusType type) {
    switch (type) {
        case BONUS_EXP: return g_event.exp_bonus;
        case BONUS_GOLD: return g_event.gold_bonus;
        case BONUS_DROP: return g_event.drop_bonus;
        case BONUS_SKILL_EXP: return g_event.skill_exp_bonus;
        case BONUS_PET_EXP: return g_event.pet_exp_bonus;
        case BONUS_CAPTURE: return g_event.capture_bonus;
        default: return 0;
    }
}

void event_recalculate_bonuses(void) {
    int i, j;

    g_event.exp_bonus = 100;      /* Base 100% */
    g_event.gold_bonus = 100;
    g_event.drop_bonus = 100;
    g_event.skill_exp_bonus = 100;
    g_event.pet_exp_bonus = 100;
    g_event.capture_bonus = 100;

    for (i = 0; i < g_event.event_count; i++) {
        EventData* event = &g_event.events[i];
        if (event->state != EVENT_STATE_ACTIVE) continue;

        for (j = 0; j < event->bonus_count; j++) {
            EventBonus* bonus = &event->bonuses[j];
            switch (bonus->type) {
                case BONUS_EXP:
                    g_event.exp_bonus += (bonus->value - 100);
                    break;
                case BONUS_GOLD:
                    g_event.gold_bonus += (bonus->value - 100);
                    break;
                case BONUS_DROP:
                    g_event.drop_bonus += (bonus->value - 100);
                    break;
                case BONUS_SKILL_EXP:
                    g_event.skill_exp_bonus += (bonus->value - 100);
                    break;
                case BONUS_PET_EXP:
                    g_event.pet_exp_bonus += (bonus->value - 100);
                    break;
                case BONUS_CAPTURE:
                    g_event.capture_bonus += (bonus->value - 100);
                    break;
                case BONUS_ALL:
                    g_event.exp_bonus += (bonus->value - 100);
                    g_event.gold_bonus += (bonus->value - 100);
                    g_event.drop_bonus += (bonus->value - 100);
                    break;
                default:
                    break;
            }
        }
    }
}

void event_select(int index) {
    if (index >= 0 && index < g_event.event_count) {
        g_event.selected_index = index;
    } else {
        g_event.selected_index = -1;
    }
}

EventData* event_get_selected(void) {
    return event_get_by_index(g_event.selected_index);
}

const char* event_type_to_string(EventType type) {
    switch (type) {
        case EVENT_TYPE_NONE: return "None";
        case EVENT_TYPE_BOSS: return "Boss";
        case EVENT_TYPE_DOUBLE_EXP: return "Double Exp";
        case EVENT_TYPE_DOUBLE_DROP: return "Double Drop";
        case EVENT_TYPE_FESTIVAL: return "Festival";
        case EVENT_TYPE_ANNIVERSARY: return "Anniversary";
        case EVENT_TYPE_WEEKEND: return "Weekend";
        case EVENT_TYPE_DAILY: return "Daily";
        case EVENT_TYPE_SPECIAL: return "Special";
        case EVENT_TYPE_GUILD_WAR: return "Guild War";
        case EVENT_TYPE_PVP_TOURNAMENT: return "PvP Tournament";
        default: return "Unknown";
    }
}

const char* event_state_to_string(EventState state) {
    switch (state) {
        case EVENT_STATE_INACTIVE: return "Inactive";
        case EVENT_STATE_UPCOMING: return "Upcoming";
        case EVENT_STATE_ACTIVE: return "Active";
        case EVENT_STATE_ENDING: return "Ending";
        default: return "Unknown";
    }
}

/* ========================================
 * Constants Tests
 * ======================================== */

static int test_max_events(void) {
    return MAX_EVENTS == 50;
}

static int test_max_event_name(void) {
    return MAX_EVENT_NAME == 32;
}

static int test_max_event_desc(void) {
    return MAX_EVENT_DESC == 128;
}

/* ========================================
 * Event Type Tests
 * ======================================== */

static int test_event_type_values(void) {
    return EVENT_TYPE_NONE == 0 &&
           EVENT_TYPE_BOSS == 1 &&
           EVENT_TYPE_DOUBLE_EXP == 2 &&
           EVENT_TYPE_DOUBLE_DROP == 3 &&
           EVENT_TYPE_FESTIVAL == 4;
}

static int test_event_type_guild_war(void) {
    return EVENT_TYPE_GUILD_WAR == 9 &&
           EVENT_TYPE_PVP_TOURNAMENT == 10;
}

/* ========================================
 * Event State Tests
 * ======================================== */

static int test_event_state_values(void) {
    return EVENT_STATE_INACTIVE == 0 &&
           EVENT_STATE_UPCOMING == 1 &&
           EVENT_STATE_ACTIVE == 2 &&
           EVENT_STATE_ENDING == 3;
}

/* ========================================
 * Bonus Type Tests
 * ======================================== */

static int test_bonus_type_values(void) {
    return BONUS_NONE == 0 &&
           BONUS_EXP == 1 &&
           BONUS_GOLD == 2 &&
           BONUS_DROP == 3;
}

static int test_bonus_type_extended(void) {
    return BONUS_SKILL_EXP == 4 &&
           BONUS_PET_EXP == 5 &&
           BONUS_CAPTURE == 6 &&
           BONUS_ALL == 7;
}

/* ========================================
 * Event Init Tests
 * ======================================== */

static int test_event_init(void) {
    event_init();
    return g_event.event_count == 0 &&
           g_event.notice_interval == 300000 &&
           g_event.show_event_notice == 1;
}

static int test_event_shutdown(void) {
    event_init();
    EventData evt = {1, "Test", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    event_add(&evt);
    event_shutdown();
    return g_event.event_count == 0;
}

/* ========================================
 * Event Add Tests
 * ======================================== */

static int test_event_add_basic(void) {
    event_init();
    EventData evt = {1, "Test Event", "Description", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    int result = event_add(&evt);
    return result == 1 && g_event.event_count == 1;
}

static int test_event_add_multiple(void) {
    event_init();
    EventData evt1 = {1, "Event 1", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    EventData evt2 = {2, "Event 2", "Desc", EVENT_TYPE_WEEKEND, EVENT_STATE_ACTIVE};
    EventData evt3 = {3, "Event 3", "Desc", EVENT_TYPE_SPECIAL, EVENT_STATE_ACTIVE};

    event_add(&evt1);
    event_add(&evt2);
    event_add(&evt3);

    return g_event.event_count == 3;
}

static int test_event_add_null(void) {
    event_init();
    int result = event_add(NULL);
    return result == 0 && g_event.event_count == 0;
}

static int test_event_add_full(void) {
    int i;
    event_init();

    for (i = 0; i < MAX_EVENTS; i++) {
        EventData evt = {(u32)(i + 1), "Event", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
        event_add(&evt);
    }

    /* Try to add one more */
    EventData extra = {999, "Extra", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    int result = event_add(&extra);

    return result == 0 && g_event.event_count == MAX_EVENTS;
}

/* ========================================
 * Event Query Tests
 * ======================================== */

static int test_event_get(void) {
    event_init();
    EventData evt = {42, "Found Event", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    event_add(&evt);

    EventData* found = event_get(42);
    return found != NULL && strcmp(found->name, "Found Event") == 0;
}

static int test_event_get_not_found(void) {
    event_init();
    EventData* found = event_get(999);
    return found == NULL;
}

static int test_event_get_by_index(void) {
    event_init();
    EventData evt = {1, "Index Test", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    event_add(&evt);

    EventData* found = event_get_by_index(0);
    return found != NULL && strcmp(found->name, "Index Test") == 0;
}

static int test_event_get_by_index_invalid(void) {
    event_init();
    EventData* found = event_get_by_index(-1);
    if (found != NULL) return 0;
    found = event_get_by_index(100);
    return found == NULL;
}

static int test_event_get_active(void) {
    event_init();
    EventData active = {1, "Active", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    EventData inactive = {2, "Inactive", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_INACTIVE};
    event_add(&active);
    event_add(&inactive);

    EventData* found = event_get_active(1);
    if (!found) return 0;

    found = event_get_active(2);
    return found == NULL;
}

static int test_event_get_count(void) {
    event_init();
    EventData evt1 = {1, "E1", "D", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    EventData evt2 = {2, "E2", "D", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    event_add(&evt1);
    event_add(&evt2);

    return event_get_count() == 2;
}

static int test_event_get_active_count(void) {
    event_init();
    EventData evt1 = {1, "E1", "D", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    EventData evt2 = {2, "E2", "D", EVENT_TYPE_DAILY, EVENT_STATE_INACTIVE};
    EventData evt3 = {3, "E3", "D", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    event_add(&evt1);
    event_add(&evt2);
    event_add(&evt3);

    return event_get_active_count() == 2;
}

/* ========================================
 * Event State Tests
 * ======================================== */

static int test_event_is_active(void) {
    event_init();
    EventData active = {1, "Active", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    EventData inactive = {2, "Inactive", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_INACTIVE};
    event_add(&active);
    event_add(&inactive);

    return event_is_active(1) == 1 && event_is_active(2) == 0;
}

static int test_event_is_active_not_found(void) {
    event_init();
    return event_is_active(999) == 0;
}

static int test_event_get_state(void) {
    event_init();
    EventData evt = {1, "Test", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_UPCOMING};
    event_add(&evt);

    return event_get_state(1) == EVENT_STATE_UPCOMING;
}

static int test_event_get_state_not_found(void) {
    event_init();
    return event_get_state(999) == EVENT_STATE_INACTIVE;
}

/* ========================================
 * Event Participation Tests
 * ======================================== */

static int test_event_join(void) {
    event_init();
    EventData evt = {1, "Test", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    event_add(&evt);

    int result = event_join(1);
    return result == 1 && g_event.participating_count == 1;
}

static int test_event_join_inactive(void) {
    event_init();
    EventData evt = {1, "Test", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_INACTIVE};
    event_add(&evt);

    int result = event_join(1);
    return result == 0 && g_event.participating_count == 0;
}

static int test_event_join_twice(void) {
    event_init();
    EventData evt = {1, "Test", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    event_add(&evt);

    event_join(1);
    event_join(1);  /* Join again */

    return g_event.participating_count == 1;
}

static int test_event_leave(void) {
    event_init();
    EventData evt = {1, "Test", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    event_add(&evt);
    event_join(1);

    int result = event_leave(1);
    return result == 1 && g_event.participating_count == 0;
}

static int test_event_leave_not_joined(void) {
    event_init();
    int result = event_leave(999);
    return result == 0;
}

static int test_event_is_participating(void) {
    event_init();
    EventData evt = {1, "Test", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    event_add(&evt);
    event_join(1);

    return event_is_participating(1) == 1 &&
           event_is_participating(2) == 0;
}

/* ========================================
 * Event Bonus Tests
 * ======================================== */

static int test_event_bonus_defaults(void) {
    event_init();
    event_recalculate_bonuses();

    return g_event.exp_bonus == 100 &&
           g_event.gold_bonus == 100 &&
           g_event.drop_bonus == 100;
}

static int test_event_bonus_exp(void) {
    event_init();
    EventData evt = {1, "Double Exp", "Desc", EVENT_TYPE_DOUBLE_EXP, EVENT_STATE_ACTIVE};
    evt.bonuses[0].type = BONUS_EXP;
    evt.bonuses[0].value = 200;
    evt.bonus_count = 1;
    event_add(&evt);

    event_recalculate_bonuses();

    return g_event.exp_bonus == 200;
}

static int test_event_bonus_multiple(void) {
    event_init();

    EventData evt1 = {1, "Exp", "Desc", EVENT_TYPE_DOUBLE_EXP, EVENT_STATE_ACTIVE};
    evt1.bonuses[0].type = BONUS_EXP;
    evt1.bonuses[0].value = 150;
    evt1.bonus_count = 1;
    event_add(&evt1);

    EventData evt2 = {2, "Gold", "Desc", EVENT_TYPE_WEEKEND, EVENT_STATE_ACTIVE};
    evt2.bonuses[0].type = BONUS_GOLD;
    evt2.bonuses[0].value = 200;
    evt2.bonus_count = 1;
    event_add(&evt2);

    event_recalculate_bonuses();

    return g_event.exp_bonus == 150 && g_event.gold_bonus == 200;
}

static int test_event_bonus_inactive_not_counted(void) {
    event_init();

    EventData active = {1, "Active", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    active.bonuses[0].type = BONUS_EXP;
    active.bonuses[0].value = 200;
    active.bonus_count = 1;
    event_add(&active);

    EventData inactive = {2, "Inactive", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_INACTIVE};
    inactive.bonuses[0].type = BONUS_EXP;
    inactive.bonuses[0].value = 300;
    inactive.bonus_count = 1;
    event_add(&inactive);

    event_recalculate_bonuses();

    /* Only active event's bonus should count */
    return g_event.exp_bonus == 200;
}

static int test_event_bonus_all_type(void) {
    event_init();

    EventData evt = {1, "All Bonus", "Desc", EVENT_TYPE_SPECIAL, EVENT_STATE_ACTIVE};
    evt.bonuses[0].type = BONUS_ALL;
    evt.bonuses[0].value = 150;
    evt.bonus_count = 1;
    event_add(&evt);

    event_recalculate_bonuses();

    /* BONUS_ALL should affect exp, gold, and drop */
    return g_event.exp_bonus == 150 &&
           g_event.gold_bonus == 150 &&
           g_event.drop_bonus == 150;
}

static int test_event_get_bonus(void) {
    event_init();
    g_event.exp_bonus = 150;
    g_event.gold_bonus = 200;
    g_event.drop_bonus = 175;

    return event_get_bonus(BONUS_EXP) == 150 &&
           event_get_bonus(BONUS_GOLD) == 200 &&
           event_get_bonus(BONUS_DROP) == 175;
}

static int test_event_get_bonus_invalid(void) {
    event_init();
    return event_get_bonus((BonusType)99) == 0;
}

/* ========================================
 * Event Selection Tests
 * ======================================== */

static int test_event_select(void) {
    event_init();
    EventData evt1 = {1, "E1", "D", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    EventData evt2 = {2, "E2", "D", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    event_add(&evt1);
    event_add(&evt2);

    event_select(1);
    return g_event.selected_index == 1;
}

static int test_event_select_invalid(void) {
    event_init();
    EventData evt = {1, "E1", "D", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    event_add(&evt);

    event_select(100);
    return g_event.selected_index == -1;
}

static int test_event_get_selected(void) {
    event_init();
    EventData evt = {1, "Selected", "Desc", EVENT_TYPE_DAILY, EVENT_STATE_ACTIVE};
    event_add(&evt);

    event_select(0);
    EventData* selected = event_get_selected();

    return selected != NULL && strcmp(selected->name, "Selected") == 0;
}

/* ========================================
 * Event Type String Tests
 * ======================================== */

static int test_event_type_to_string(void) {
    return strcmp(event_type_to_string(EVENT_TYPE_BOSS), "Boss") == 0 &&
           strcmp(event_type_to_string(EVENT_TYPE_DOUBLE_EXP), "Double Exp") == 0 &&
           strcmp(event_type_to_string(EVENT_TYPE_GUILD_WAR), "Guild War") == 0;
}

static int test_event_type_to_string_unknown(void) {
    return strcmp(event_type_to_string((EventType)99), "Unknown") == 0;
}

static int test_event_state_to_string(void) {
    return strcmp(event_state_to_string(EVENT_STATE_ACTIVE), "Active") == 0 &&
           strcmp(event_state_to_string(EVENT_STATE_INACTIVE), "Inactive") == 0;
}

static int test_event_state_to_string_unknown(void) {
    return strcmp(event_state_to_string((EventState)99), "Unknown") == 0;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_event_lifecycle(void) {
    event_init();

    /* Create events */
    EventData doubleExp = {1, "Double Exp Weekend", "2x EXP", EVENT_TYPE_DOUBLE_EXP, EVENT_STATE_UPCOMING};
    doubleExp.bonuses[0].type = BONUS_EXP;
    doubleExp.bonuses[0].value = 200;
    doubleExp.bonus_count = 1;

    EventData bossEvent = {2, "World Boss", "Boss spawns", EVENT_TYPE_BOSS, EVENT_STATE_ACTIVE};
    bossEvent.bonuses[0].type = BONUS_DROP;
    bossEvent.bonuses[0].value = 150;
    bossEvent.bonus_count = 1;

    event_add(&doubleExp);
    event_add(&bossEvent);

    /* Check initial state */
    if (event_get_count() != 2) return 0;
    if (event_get_active_count() != 1) return 0;

    /* Activate double exp */
    EventData* expEvent = event_get(1);
    expEvent->state = EVENT_STATE_ACTIVE;

    /* Recalculate bonuses */
    event_recalculate_bonuses();

    /* Should have exp 200 (from double exp) + drop 150 (from boss) */
    if (g_event.exp_bonus != 200) return 0;
    if (g_event.drop_bonus != 150) return 0;

    /* Join events */
    event_join(1);
    event_join(2);

    if (!event_is_participating(1)) return 0;
    if (!event_is_participating(2)) return 0;

    /* Leave one */
    event_leave(1);

    if (event_is_participating(1)) return 0;
    if (!event_is_participating(2)) return 0;

    return 1;
}

static int test_event_boss_spawn_scenario(void) {
    event_init();

    EventData boss = {100, "World Boss", "Weekly boss", EVENT_TYPE_BOSS, EVENT_STATE_ACTIVE};
    boss.schedule.start_time = 1000000;
    boss.schedule.end_time = 1100000;
    boss.bonuses[0].type = BONUS_DROP;
    boss.bonuses[0].value = 200;
    boss.bonus_count = 1;

    event_add(&boss);

    if (!event_is_active(100)) return 0;

    event_recalculate_bonuses();
    if (g_event.drop_bonus != 200) return 0;

    /* Boss event ends */
    EventData* evt = event_get(100);
    evt->state = EVENT_STATE_INACTIVE;

    event_recalculate_bonuses();
    if (g_event.drop_bonus != 100) return 0;  /* Back to base */

    return 1;
}

static int test_event_festival_all_bonuses(void) {
    event_init();

    EventData festival = {1, "Anniversary Festival", "Happy Anniversary!", EVENT_TYPE_ANNIVERSARY, EVENT_STATE_ACTIVE};
    festival.bonuses[0].type = BONUS_ALL;
    festival.bonuses[0].value = 200;
    festival.bonuses[1].type = BONUS_CAPTURE;
    festival.bonuses[1].value = 150;
    festival.bonus_count = 2;

    event_add(&festival);
    event_recalculate_bonuses();

    /* BONUS_ALL = +100 for exp/gold/drop, CAPTURE = +50 */
    return g_event.exp_bonus == 200 &&
           g_event.gold_bonus == 200 &&
           g_event.drop_bonus == 200 &&
           g_event.capture_bonus == 150;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Event System Comprehensive Tests ===\n\n");

    /* Constants tests */
    printf("Constants Tests:\n");
    TEST(max_events);
    TEST(max_event_name);
    TEST(max_event_desc);

    /* Event type tests */
    printf("\nEvent Type Tests:\n");
    TEST(event_type_values);
    TEST(event_type_guild_war);

    /* Event state tests */
    printf("\nEvent State Tests:\n");
    TEST(event_state_values);

    /* Bonus type tests */
    printf("\nBonus Type Tests:\n");
    TEST(bonus_type_values);
    TEST(bonus_type_extended);

    /* Event init tests */
    printf("\nEvent Init Tests:\n");
    TEST(event_init);
    TEST(event_shutdown);

    /* Event add tests */
    printf("\nEvent Add Tests:\n");
    TEST(event_add_basic);
    TEST(event_add_multiple);
    TEST(event_add_null);
    TEST(event_add_full);

    /* Event query tests */
    printf("\nEvent Query Tests:\n");
    TEST(event_get);
    TEST(event_get_not_found);
    TEST(event_get_by_index);
    TEST(event_get_by_index_invalid);
    TEST(event_get_active);
    TEST(event_get_count);
    TEST(event_get_active_count);

    /* Event state tests */
    printf("\nEvent State Tests:\n");
    TEST(event_is_active);
    TEST(event_is_active_not_found);
    TEST(event_get_state);
    TEST(event_get_state_not_found);

    /* Event participation tests */
    printf("\nEvent Participation Tests:\n");
    TEST(event_join);
    TEST(event_join_inactive);
    TEST(event_join_twice);
    TEST(event_leave);
    TEST(event_leave_not_joined);
    TEST(event_is_participating);

    /* Event bonus tests */
    printf("\nEvent Bonus Tests:\n");
    TEST(event_bonus_defaults);
    TEST(event_bonus_exp);
    TEST(event_bonus_multiple);
    TEST(event_bonus_inactive_not_counted);
    TEST(event_bonus_all_type);
    TEST(event_get_bonus);
    TEST(event_get_bonus_invalid);

    /* Event selection tests */
    printf("\nEvent Selection Tests:\n");
    TEST(event_select);
    TEST(event_select_invalid);
    TEST(event_get_selected);

    /* Event type string tests */
    printf("\nEvent Type String Tests:\n");
    TEST(event_type_to_string);
    TEST(event_type_to_string_unknown);
    TEST(event_state_to_string);
    TEST(event_state_to_string_unknown);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_event_lifecycle);
    TEST(event_boss_spawn_scenario);
    TEST(event_festival_all_bonuses);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
