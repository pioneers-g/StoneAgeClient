/*
 * Stone Age Client - Notification System Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "notification.h"
#include "logger.h"

/* Global notification context */
NotificationContext g_notify = {0};

/*
 * Initialize notification system
 */
int notification_init(void) {
    memset(&g_notify, 0, sizeof(NotificationContext));

    g_notify.enabled = 1;
    g_notify.max_display = 5;
    g_notify.default_duration = 5000;
    g_notify.fade_duration = 500;

    LOG_INFO("Notification system initialized");
    return 1;
}

/*
 * Shutdown notification system
 */
void notification_shutdown(void) {
    notification_clear_all();
    memset(&g_notify, 0, sizeof(NotificationContext));
    LOG_INFO("Notification system shutdown");
}

/*
 * Add notification
 */
int notification_add(NotificationType type, const char* message, u32 duration) {
    NotificationEntry* entry;
    int index;

    if (!message || !g_notify.enabled) {
        return -1;
    }

    /* Find free slot */
    if (g_notify.count >= MAX_NOTIFICATIONS) {
        /* Remove oldest */
        notification_remove(0);
    }

    index = g_notify.count;
    entry = &g_notify.notifications[index];

    entry->id = g_notify.next_id++;
    entry->type = type;
    strncpy(entry->message, message, MAX_NOTIFY_MESSAGE - 1);
    entry->message[MAX_NOTIFY_MESSAGE - 1] = '\0';
    entry->start_time = timeGetTime();
    entry->duration = duration > 0 ? duration : g_notify.default_duration;
    entry->alpha = 255;
    entry->state = NOTIFY_STATE_FADEIN;
    entry->fade_start = entry->start_time;

    g_notify.count++;

    LOG_DEBUG("Notification added: [%d] %s", type, message);
    return entry->id;
}

/*
 * Add simple notification
 */
int notification_show(const char* message) {
    return notification_add(NOTIFY_INFO, message, 0);
}

/*
 * Add info notification
 */
int notification_info(const char* message) {
    return notification_add(NOTIFY_INFO, message, 0);
}

/*
 * Add success notification
 */
int notification_success(const char* message) {
    return notification_add(NOTIFY_SUCCESS, message, 0);
}

/*
 * Add warning notification
 */
int notification_warning(const char* message) {
    return notification_add(NOTIFY_WARNING, message, 0);
}

/*
 * Add error notification
 */
int notification_error(const char* message) {
    return notification_add(NOTIFY_ERROR, message, 0);
}

/*
 * Add system notification
 */
int notification_system(const char* message) {
    return notification_add(NOTIFY_SYSTEM, message, 0);
}

/*
 * Add combat notification
 */
int notification_combat(const char* message) {
    return notification_add(NOTIFY_COMBAT, message, 3000);
}

/*
 * Update notifications
 */
void notification_update(void) {
    u32 current_time;
    int i, j;

    if (!g_notify.enabled) return;

    current_time = timeGetTime();

    for (i = 0; i < g_notify.count; i++) {
        NotificationEntry* entry = &g_notify.notifications[i];
        u32 elapsed = current_time - entry->start_time;

        /* Update fade state */
        switch (entry->state) {
            case NOTIFY_STATE_FADEIN:
                {
                    u32 fade_elapsed = current_time - entry->fade_start;
                    if (fade_elapsed >= g_notify.fade_duration) {
                        entry->alpha = 255;
                        entry->state = NOTIFY_STATE_VISIBLE;
                    } else {
                        entry->alpha = (u8)((fade_elapsed * 255) / g_notify.fade_duration);
                    }
                }
                break;

            case NOTIFY_STATE_VISIBLE:
                if (elapsed >= entry->duration - g_notify.fade_duration) {
                    entry->state = NOTIFY_STATE_FADEOUT;
                    entry->fade_start = current_time;
                }
                break;

            case NOTIFY_STATE_FADEOUT:
                {
                    u32 fade_elapsed = current_time - entry->fade_start;
                    if (fade_elapsed >= g_notify.fade_duration) {
                        entry->alpha = 0;
                        entry->state = NOTIFY_STATE_REMOVE;
                    } else {
                        entry->alpha = (u8)(255 - (fade_elapsed * 255) / g_notify.fade_duration);
                    }
                }
                break;

            case NOTIFY_STATE_REMOVE:
                break;
        }

        /* Check if expired */
        if (elapsed >= entry->duration) {
            entry->state = NOTIFY_STATE_REMOVE;
        }
    }

    /* Remove expired notifications */
    i = 0;
    while (i < g_notify.count) {
        if (g_notify.notifications[i].state == NOTIFY_STATE_REMOVE) {
            /* Shift remaining */
            for (j = i; j < g_notify.count - 1; j++) {
                g_notify.notifications[j] = g_notify.notifications[j + 1];
            }
            g_notify.count--;
        } else {
            i++;
        }
    }
}

/*
 * Remove notification by ID
 */
int notification_remove(int id) {
    int i, j;

    for (i = 0; i < g_notify.count; i++) {
        if (g_notify.notifications[i].id == id) {
            for (j = i; j < g_notify.count - 1; j++) {
                g_notify.notifications[j] = g_notify.notifications[j + 1];
            }
            g_notify.count--;
            return 1;
        }
    }

    return 0;
}

/*
 * Remove notification by index
 */
int notification_remove_at(int index) {
    int i;

    if (index < 0 || index >= g_notify.count) {
        return 0;
    }

    for (i = index; i < g_notify.count - 1; i++) {
        g_notify.notifications[i] = g_notify.notifications[i + 1];
    }

    g_notify.count--;
    return 1;
}

/*
 * Clear all notifications
 */
void notification_clear_all(void) {
    g_notify.count = 0;
}

/*
 * Get notification count
 */
int notification_get_count(void) {
    return g_notify.count;
}

/*
 * Get notification by index
 */
NotificationEntry* notification_get(int index) {
    if (index < 0 || index >= g_notify.count) {
        return NULL;
    }
    return &g_notify.notifications[index];
}

/*
 * Set max display count
 */
void notification_set_max_display(int max) {
    g_notify.max_display = max > MAX_NOTIFICATIONS ? MAX_NOTIFICATIONS : max;
}

/*
 * Set default duration
 */
void notification_set_default_duration(u32 duration) {
    g_notify.default_duration = duration;
}

/*
 * Enable/disable notifications
 */
void notification_set_enabled(int enabled) {
    g_notify.enabled = enabled;
}

/*
 * Check if enabled
 */
int notification_is_enabled(void) {
    return g_notify.enabled;
}

/*
 * Get color for notification type
 */
u32 notification_get_color(NotificationType type) {
    switch (type) {
        case NOTIFY_INFO:     return 0xFFFFFFFF;  /* White */
        case NOTIFY_SUCCESS:  return 0xFF00FF00;  /* Green */
        case NOTIFY_WARNING:  return 0xFFFFFF00;  /* Yellow */
        case NOTIFY_ERROR:    return 0xFFFF0000;  /* Red */
        case NOTIFY_SYSTEM:   return 0xFF00FFFF;  /* Cyan */
        case NOTIFY_COMBAT:   return 0xFFFF8000;  /* Orange */
        case NOTIFY_ITEM:     return 0xFF00AAFF;  /* Gold */
        case NOTIFY_SOCIAL:   return 0xFFFF00FF;  /* Magenta */
        default:              return 0xFFFFFFFF;
    }
}

/*
 * Get icon for notification type
 */
u16 notification_get_icon(NotificationType type) {
    switch (type) {
        case NOTIFY_INFO:     return 100;
        case NOTIFY_SUCCESS:  return 101;
        case NOTIFY_WARNING:  return 102;
        case NOTIFY_ERROR:    return 103;
        case NOTIFY_SYSTEM:   return 104;
        case NOTIFY_COMBAT:   return 105;
        case NOTIFY_ITEM:     return 106;
        case NOTIFY_SOCIAL:   return 107;
        default:              return 0;
    }
}

/*
 * Notification type to string
 */
const char* notification_type_to_string(NotificationType type) {
    static const char* type_names[] = {
        "Info", "Success", "Warning", "Error",
        "System", "Combat", "Item", "Social"
    };

    if (type < sizeof(type_names) / sizeof(type_names[0])) {
        return type_names[type];
    }
    return "Unknown";
}

/*
 * Dismiss all notifications of type
 */
int notification_dismiss_by_type(NotificationType type) {
    int i;
    int count = 0;

    i = 0;
    while (i < g_notify.count) {
        if (g_notify.notifications[i].type == type) {
            notification_remove_at(i);
            count++;
        } else {
            i++;
        }
    }

    return count;
}

/*
 * Check if has notifications
 */
int notification_has_pending(void) {
    return g_notify.count > 0;
}

/*
 * Get total notification display time
 */
u32 notification_get_total_duration(int index) {
    if (index < 0 || index >= g_notify.count) {
        return 0;
    }
    return g_notify.notifications[index].duration;
}

/*
 * Extend notification duration
 */
int notification_extend(int id, u32 additional_ms) {
    int i;

    for (i = 0; i < g_notify.count; i++) {
        if (g_notify.notifications[i].id == id) {
            g_notify.notifications[i].duration += additional_ms;
            return 1;
        }
    }

    return 0;
}
