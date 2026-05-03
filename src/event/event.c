/*
 * Stone Age Client - Event System Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "types.h"
#include "event.h"
#include "network_protocol.h"
#include "render.h"
#include "logger.h"

/* Global event context */
EventContext g_event = {0};

/*
 * Initialize event system
 */
int event_init(void) {
    memset(&g_event, 0, sizeof(EventContext));

    g_event.notice_interval = 300000;  /* 5 minutes */
    g_event.show_event_notice = 1;

    LOG_INFO("Event system initialized");
    return 1;
}

/*
 * Shutdown event system
 */
void event_shutdown(void) {
    memset(&g_event, 0, sizeof(EventContext));
    LOG_INFO("Event system shutdown");
}

/*
 * Update event system
 */
void event_update(u32 current_time) {
    int i;
    EventData* event;

    /* Check event schedules */
    for (i = 0; i < g_event.event_count; i++) {
        event = &g_event.events[i];

        if (event->state == EVENT_STATE_ACTIVE) {
            /* Check if event ended */
            if (current_time >= event->schedule.end_time) {
                event->state = EVENT_STATE_INACTIVE;
                LOG_INFO("Event ended: %s", event->name);
                event_recalculate_bonuses();

                /* Show notice */
                if (event->show_notice) {
                    event_show_notice(event->id);
                }
            }
        } else if (event->state == EVENT_STATE_UPCOMING) {
            /* Check if event started */
            if (current_time >= event->schedule.start_time) {
                event->state = EVENT_STATE_ACTIVE;
                LOG_INFO("Event started: %s", event->name);
                event_recalculate_bonuses();

                /* Show notice */
                if (event->show_notice) {
                    event_show_notice(event->id);
                }
            }
        }
    }
}

/*
 * Check all event schedules
 */
void event_check_schedules(void) {
    u32 current_time = (u32)time(NULL);
    event_update(current_time);
}

/*
 * Get event by ID
 */
EventData* event_get(u32 event_id) {
    int i;

    for (i = 0; i < g_event.event_count; i++) {
        if (g_event.events[i].id == event_id) {
            return &g_event.events[i];
        }
    }

    return NULL;
}

/*
 * Get event by index
 */
EventData* event_get_by_index(int index) {
    if (index < 0 || index >= g_event.event_count) {
        return NULL;
    }
    return &g_event.events[index];
}

/*
 * Get active event
 */
EventData* event_get_active(u32 event_id) {
    EventData* event = event_get(event_id);
    return (event && event->state == EVENT_STATE_ACTIVE) ? event : NULL;
}

/*
 * Get event count
 */
int event_get_count(void) {
    return g_event.event_count;
}

/*
 * Get active event count
 */
int event_get_active_count(void) {
    int i, count = 0;

    for (i = 0; i < g_event.event_count; i++) {
        if (g_event.events[i].state == EVENT_STATE_ACTIVE) {
            count++;
        }
    }

    return count;
}

/*
 * Check if event is active
 */
int event_is_active(u32 event_id) {
    EventData* event = event_get(event_id);
    return event ? event->state == EVENT_STATE_ACTIVE : 0;
}

/*
 * Get event state
 */
EventState event_get_state(u32 event_id) {
    EventData* event = event_get(event_id);
    return event ? event->state : EVENT_STATE_INACTIVE;
}

/*
 * Get EXP bonus
 */
u16 event_get_exp_bonus(void) {
    return g_event.exp_bonus;
}

/*
 * Get gold bonus
 */
u16 event_get_gold_bonus(void) {
    return g_event.gold_bonus;
}

/*
 * Get drop bonus
 */
u16 event_get_drop_bonus(void) {
    return g_event.drop_bonus;
}

/*
 * Get skill EXP bonus
 */
u16 event_get_skill_exp_bonus(void) {
    return g_event.skill_exp_bonus;
}

/*
 * Get pet EXP bonus
 */
u16 event_get_pet_exp_bonus(void) {
    return g_event.pet_exp_bonus;
}

/*
 * Get capture bonus
 */
u16 event_get_capture_bonus(void) {
    return g_event.capture_bonus;
}

/*
 * Get bonus by type
 */
u16 event_get_bonus(BonusType type) {
    switch (type) {
        case BONUS_EXP: return g_event.exp_bonus;
        case BONUS_GOLD: return g_event.gold_bonus;
        case BONUS_DROP: return g_event.drop_bonus;
        case BONUS_SKILL_EXP: return g_event.skill_exp_bonus;
        case BONUS_PET_EXP: return g_event.pet_exp_bonus;
        case BONUS_CAPTURE: return g_event.capture_bonus;
        default: return 100;
    }
}

/*
 * Recalculate all bonuses
 */
void event_recalculate_bonuses(void) {
    int i, j;
    EventData* event;

    /* Reset to base (100%) */
    g_event.exp_bonus = 100;
    g_event.gold_bonus = 100;
    g_event.drop_bonus = 100;
    g_event.skill_exp_bonus = 100;
    g_event.pet_exp_bonus = 100;
    g_event.capture_bonus = 100;

    /* Add active event bonuses */
    for (i = 0; i < g_event.event_count; i++) {
        event = &g_event.events[i];

        if (event->state != EVENT_STATE_ACTIVE) {
            continue;
        }

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

    /* Ensure minimum 100% */
    if (g_event.exp_bonus < 100) g_event.exp_bonus = 100;
    if (g_event.gold_bonus < 100) g_event.gold_bonus = 100;
    if (g_event.drop_bonus < 100) g_event.drop_bonus = 100;
}

/*
 * Handle event list packet
 */
void event_handle_list(void* data, u32 size) {
    u8* ptr = (u8*)data;
    int i;
    EventData* event;

    g_event.event_count = 0;

    u16 count = *(u16*)ptr; ptr += 2;

    for (i = 0; i < count && i < MAX_EVENTS && ptr < (u8*)data + size; i++) {
        event = &g_event.events[i];

        event->id = *(u32*)ptr; ptr += 4;
        if (event->id == 0) continue;

        strncpy(event->name, (char*)ptr, MAX_EVENT_NAME - 1);
        ptr += strlen(event->name) + 1;

        event->type = *(u8*)ptr; ptr += 1;
        event->state = *(u8*)ptr; ptr += 1;
        event->priority = *(u8*)ptr; ptr += 1;

        event->schedule.start_time = *(u32*)ptr; ptr += 4;
        event->schedule.end_time = *(u32*)ptr; ptr += 4;

        event->bonus_count = *(u8*)ptr; ptr += 1;

        int j;
        for (j = 0; j < event->bonus_count; j++) {
            event->bonuses[j].type = *(u8*)ptr; ptr += 1;
            event->bonuses[j].value = *(u16*)ptr; ptr += 2;
        }

        event->icon_id = *(u16*)ptr; ptr += 2;
        event->show_notice = *(u8*)ptr; ptr += 1;

        g_event.event_count++;
    }

    event_recalculate_bonuses();
    LOG_INFO("Loaded %d events", g_event.event_count);
}

/*
 * Handle event start packet
 */
void event_handle_start(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 event_id;
    EventData* event;

    event_id = *(u32*)ptr;
    event = event_get(event_id);

    if (event) {
        event->state = EVENT_STATE_ACTIVE;
        event_recalculate_bonuses();
        LOG_INFO("Event started: %s", event->name);

        if (event->show_notice) {
            event_show_notice(event_id);
        }
    }
}

/*
 * Handle event end packet
 */
void event_handle_end(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 event_id;
    EventData* event;

    event_id = *(u32*)ptr;
    event = event_get(event_id);

    if (event) {
        event->state = EVENT_STATE_INACTIVE;
        event_recalculate_bonuses();
        LOG_INFO("Event ended: %s", event->name);
    }
}

/*
 * Handle bonus update packet
 */
void event_handle_bonus(void* data, u32 size) {
    u8* ptr = (u8*)data;

    g_event.exp_bonus = *(u16*)ptr; ptr += 2;
    g_event.gold_bonus = *(u16*)ptr; ptr += 2;
    g_event.drop_bonus = *(u16*)ptr; ptr += 2;
    g_event.skill_exp_bonus = *(u16*)ptr; ptr += 2;
    g_event.pet_exp_bonus = *(u16*)ptr; ptr += 2;
    g_event.capture_bonus = *(u16*)ptr;

    LOG_DEBUG("Event bonuses updated");
}

/*
 * Show event notice
 */
void event_show_notice(u32 event_id) {
    EventData* event = event_get(event_id);

    if (event) {
        g_event.last_notice_time = timeGetTime();

        /* Store notice message for UI rendering */
        if (event->state == EVENT_STATE_ACTIVE) {
            _snprintf(g_event.notice_message, sizeof(g_event.notice_message),
                     "Event Active: %s", event->name);
        } else if (event->state == EVENT_STATE_UPCOMING) {
            u32 time_until = event_get_time_until_start(event_id);
            if (time_until < 3600) {
                _snprintf(g_event.notice_message, sizeof(g_event.notice_message),
                         "Event Starting Soon: %s", event->name);
            } else {
                _snprintf(g_event.notice_message, sizeof(g_event.notice_message),
                         "Upcoming Event: %s", event->name);
            }
        }

        g_event.notice_timer = 300;  /* 5 seconds at 60fps */

        LOG_INFO("Event Notice: %s", event->name);
    }
}

/*
 * Check if has pending notice
 */
int event_has_notice(void) {
    return g_event.show_event_notice &&
           (timeGetTime() - g_event.last_notice_time >= g_event.notice_interval);
}

/*
 * Clear notice flag
 */
void event_clear_notice(void) {
    g_event.last_notice_time = timeGetTime();
}

/*
 * Set calendar month
 */
void event_set_calendar_month(u32 year, u32 month) {
    g_event.calendar_year = year;
    g_event.calendar_month = month;
}

/*
 * Get events for calendar day
 */
int event_get_calendar_events(u8 day, u32* event_ids, int max_count) {
    int i, count = 0;
    u32 current_time;
    struct tm* tm_info;
    time_t t;

    if (day == 0 || day > 31 || !event_ids || max_count <= 0) {
        return 0;
    }

    /* Get current time */
    time(&t);
    tm_info = localtime(&t);

    /* Set to the specified day in the calendar month */
    tm_info->tm_year = (int)g_event.calendar_year - 1900;
    tm_info->tm_mon = (int)g_event.calendar_month - 1;
    tm_info->tm_mday = (int)day;
    tm_info->tm_hour = 12;  /* Noon to avoid DST issues */
    current_time = (u32)mktime(tm_info);

    /* Check each event */
    for (i = 0; i < g_event.event_count && count < max_count; i++) {
        EventData* event = &g_event.events[i];
        u32 start = event->schedule.start_time;
        u32 end = event->schedule.end_time;

        /* Check if event spans this day */
        if (start <= current_time && current_time <= end) {
            event_ids[count++] = event->id;
            continue;
        }

        /* Check repeat schedules */
        if (event->schedule.repeat_type > 0) {
            /* For repeating events, check if the day matches the pattern */
            struct tm start_tm;
            time_t start_time_t = (time_t)start;
            struct tm* start_info = localtime(&start_time_t);

            switch (event->schedule.repeat_type) {
                case 1:  /* Daily */
                    event_ids[count++] = event->id;
                    break;

                case 2:  /* Weekly */
                    if (tm_info->tm_wday == event->schedule.repeat_param) {
                        event_ids[count++] = event->id;
                    }
                    break;

                case 3:  /* Monthly */
                    if (day == event->schedule.repeat_param ||
                        (event->schedule.repeat_param == 0 && day == tm_info->tm_mday)) {
                        event_ids[count++] = event->id;
                    }
                    break;
            }
        }
    }

    return count;
}

/*
 * Join event
 */
int event_join(u32 event_id) {
    EventData* event = event_get(event_id);
    char packet[64];

    if (!event) return 0;
    if (event->state != EVENT_STATE_ACTIVE) return 0;

    /* Check if already participating */
    if (event_is_participating(event_id)) {
        LOG_DEBUG("Already participating in event: %s", event->name);
        return 1;
    }

    /* Send join packet: EJ|<event_id> */
    _snprintf(packet, sizeof(packet), "EJ|%u", event_id);
    send_queue_add(packet, 0, 0);

    /* Add to participating list (optimistic) */
    if (g_event.participating_count < MAX_EVENTS) {
        g_event.participating_events[g_event.participating_count++] = event_id;
    }

    LOG_DEBUG("Joining event: %s", event->name);
    return 1;
}

/*
 * Leave event
 */
int event_leave(u32 event_id) {
    char packet[64];
    int i, found = 0;

    /* Send leave packet: EL|<event_id> */
    _snprintf(packet, sizeof(packet), "EL|%u", event_id);
    send_queue_add(packet, 0, 0);

    /* Remove from participating list */
    for (i = 0; i < g_event.participating_count; i++) {
        if (g_event.participating_events[i] == event_id) {
            found = 1;
        }
        if (found && i < g_event.participating_count - 1) {
            g_event.participating_events[i] = g_event.participating_events[i + 1];
        }
    }

    if (found) {
        g_event.participating_count--;
        LOG_DEBUG("Left event: %u", event_id);
    }

    return found;
}

/*
 * Check if participating in event
 */
int event_is_participating(u32 event_id) {
    int i;

    for (i = 0; i < g_event.participating_count; i++) {
        if (g_event.participating_events[i] == event_id) {
            return 1;
        }
    }

    return 0;
}

/*
 * Get events by type
 */
int event_get_by_type(EventType type, EventData** events, int max_count) {
    int i, count = 0;

    for (i = 0; i < g_event.event_count && count < max_count; i++) {
        if (g_event.events[i].type == type) {
            events[count++] = &g_event.events[i];
        }
    }

    return count;
}

/*
 * Get active events by type
 */
int event_get_active_by_type(EventType type, EventData** events, int max_count) {
    int i, count = 0;

    for (i = 0; i < g_event.event_count && count < max_count; i++) {
        if (g_event.events[i].type == type &&
            g_event.events[i].state == EVENT_STATE_ACTIVE) {
            events[count++] = &g_event.events[i];
        }
    }

    return count;
}

/*
 * Get time remaining for event
 */
u32 event_get_time_remaining(u32 event_id) {
    EventData* event = event_get(event_id);

    if (!event || event->state != EVENT_STATE_ACTIVE) {
        return 0;
    }

    u32 current = (u32)time(NULL);
    if (current >= event->schedule.end_time) {
        return 0;
    }

    return event->schedule.end_time - current;
}

/*
 * Get time until event starts
 */
u32 event_get_time_until_start(u32 event_id) {
    EventData* event = event_get(event_id);

    if (!event || event->state != EVENT_STATE_UPCOMING) {
        return 0;
    }

    u32 current = (u32)time(NULL);
    if (current >= event->schedule.start_time) {
        return 0;
    }

    return event->schedule.start_time - current;
}

/*
 * Select event
 */
void event_select(int index) {
    if (index >= 0 && index < g_event.event_count) {
        g_event.selected_index = index;
    }
}

/*
 * Get selected event
 */
EventData* event_get_selected(void) {
    return event_get_by_index(g_event.selected_index);
}

/*
 * Event type to string
 */
const char* event_type_to_string(EventType type) {
    static const char* type_names[] = {
        "None", "Boss", "Double Exp", "Double Drop",
        "Festival", "Anniversary", "Weekend", "Daily",
        "Special", "Guild War", "PvP Tournament"
    };

    if (type < sizeof(type_names) / sizeof(type_names[0])) {
        return type_names[type];
    }
    return "Unknown";
}

/*
 * Event state to string
 */
const char* event_state_to_string(EventState state) {
    static const char* state_names[] = {
        "Inactive", "Upcoming", "Active", "Ending"
    };

    if (state < sizeof(state_names) / sizeof(state_names[0])) {
        return state_names[state];
    }
    return "Unknown";
}
