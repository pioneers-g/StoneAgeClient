/*
 * Stone Age Client - Event System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef EVENT_H
#define EVENT_H

#include "types.h"

/* Constants */
#define MAX_EVENTS          50
#define MAX_EVENT_NAME      32
#define MAX_EVENT_DESC      128

/* Event types */
typedef enum {
    EVENT_TYPE_NONE = 0,
    EVENT_TYPE_BOSS = 1,        /* Boss spawn event */
    EVENT_TYPE_DOUBLE_EXP = 2,  /* Double experience */
    EVENT_TYPE_DOUBLE_DROP = 3, /* Double drop rate */
    EVENT_TYPE_FESTIVAL = 4,    /* Festival event */
    EVENT_TYPE_ANNIVERSARY = 5, /* Anniversary */
    EVENT_TYPE_WEEKEND = 6,     /* Weekend bonus */
    EVENT_TYPE_DAILY = 7,       /* Daily event */
    EVENT_TYPE_SPECIAL = 8,     /* Special limited event */
    EVENT_TYPE_GUILD_WAR = 9,   /* Guild war */
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
    u32 start_time;     /* Unix timestamp */
    u32 end_time;
    u8 repeat_type;     /* 0=once, 1=daily, 2=weekly, 3=monthly */
    u8 repeat_param;    /* Day of week for weekly, day of month for monthly */
    u16 repeat_hour;    /* Hour to start */
    u16 repeat_minute;
    u16 duration_hours;
} EventSchedule;

/* Event bonus */
typedef struct {
    BonusType type;
    u16 value;          /* Percentage bonus (200 = 2x) */
    u16 stack_type;     /* How bonuses stack */
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

    /* Schedule */
    EventSchedule schedule;

    /* Bonuses */
    EventBonus bonuses[5];
    u8 bonus_count;

    /* Requirements */
    u16 min_level;
    u16 max_level;
    u32 required_quest;

    /* Rewards */
    u32 reward_item;
    u16 reward_count;
    u32 reward_gold;
    u32 reward_exp;

    /* UI */
    u16 icon_id;
    u8 show_notice;
    u8 auto_join;

} EventData;

/* Event context */
typedef struct {
    /* Active events */
    EventData events[MAX_EVENTS];
    int event_count;

    /* Participating events */
    u32 participating_events[MAX_EVENTS];
    int participating_count;

    /* Current bonuses (cached) */
    u16 exp_bonus;
    u16 gold_bonus;
    u16 drop_bonus;
    u16 skill_exp_bonus;
    u16 pet_exp_bonus;
    u16 capture_bonus;

    /* Notice state */
    u32 last_notice_time;
    u32 notice_interval;
    u8 show_event_notice;

    /* Notice message */
    char notice_message[256];
    u32 notice_timer;

    /* Calendar */
    u32 calendar_month;
    u32 calendar_year;
    u8 calendar_events[32];  /* Events for each day */

    /* Selected event */
    int selected_index;

} EventContext;

/* Global event context */
extern EventContext g_event;

/* Initialization */
int event_init(void);
void event_shutdown(void);

/* Update */
void event_update(u32 current_time);
void event_check_schedules(void);

/* Event queries */
EventData* event_get(u32 event_id);
EventData* event_get_by_index(int index);
EventData* event_get_active(u32 event_id);
int event_get_count(void);
int event_get_active_count(void);
int event_is_active(u32 event_id);
EventState event_get_state(u32 event_id);

/* Bonuses */
u16 event_get_exp_bonus(void);
u16 event_get_gold_bonus(void);
u16 event_get_drop_bonus(void);
u16 event_get_skill_exp_bonus(void);
u16 event_get_pet_exp_bonus(void);
u16 event_get_capture_bonus(void);
u16 event_get_bonus(BonusType type);
void event_recalculate_bonuses(void);

/* Packet handlers */
void event_handle_list(void* data, u32 size);
void event_handle_start(void* data, u32 size);
void event_handle_end(void* data, u32 size);
void event_handle_bonus(void* data, u32 size);

/* Notice */
void event_show_notice(u32 event_id);
int event_has_notice(void);
void event_clear_notice(void);

/* Calendar */
void event_set_calendar_month(u32 year, u32 month);
int event_get_calendar_events(u8 day, u32* event_ids, int max_count);

/* Participation */
int event_join(u32 event_id);
int event_leave(u32 event_id);
int event_is_participating(u32 event_id);

/* Query by type */
int event_get_by_type(EventType type, EventData** events, int max_count);
int event_get_active_by_type(EventType type, EventData** events, int max_count);

/* Time check */
u32 event_get_time_remaining(u32 event_id);
u32 event_get_time_until_start(u32 event_id);

/* UI */
void event_select(int index);
EventData* event_get_selected(void);

/* Utility */
const char* event_type_to_string(EventType type);
const char* event_state_to_string(EventState state);

#endif /* EVENT_H */
