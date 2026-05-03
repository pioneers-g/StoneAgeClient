/*
 * Stone Age Client - Notification System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "types.h"

/* Constants */
#define MAX_NOTIFICATIONS      20
#define MAX_NOTIFY_MESSAGE     256

/* Notification types */
typedef enum {
    NOTIFY_INFO = 0,
    NOTIFY_SUCCESS = 1,
    NOTIFY_WARNING = 2,
    NOTIFY_ERROR = 3,
    NOTIFY_SYSTEM = 4,
    NOTIFY_COMBAT = 5,
    NOTIFY_ITEM = 6,
    NOTIFY_SOCIAL = 7
} NotificationType;

/* Notification states */
typedef enum {
    NOTIFY_STATE_FADEIN = 0,
    NOTIFY_STATE_VISIBLE = 1,
    NOTIFY_STATE_FADEOUT = 2,
    NOTIFY_STATE_REMOVE = 3
} NotificationState;

/* Notification entry */
typedef struct {
    int id;
    NotificationType type;
    char message[MAX_NOTIFY_MESSAGE];
    u32 start_time;
    u32 duration;
    u32 fade_start;
    u8 alpha;
    u8 state;
    u16 icon_id;

} NotificationEntry;

/* Notification context */
typedef struct {
    /* Notifications */
    NotificationEntry notifications[MAX_NOTIFICATIONS];
    int count;
    int next_id;

    /* Settings */
    int enabled;
    int max_display;
    u32 default_duration;
    u32 fade_duration;

} NotificationContext;

/* Global notification context */
extern NotificationContext g_notify;

/* Initialization */
int notification_init(void);
void notification_shutdown(void);

/* Add notifications */
int notification_add(NotificationType type, const char* message, u32 duration);
int notification_show(const char* message);
int notification_info(const char* message);
int notification_success(const char* message);
int notification_warning(const char* message);
int notification_error(const char* message);
int notification_system(const char* message);
int notification_combat(const char* message);

/* Update */
void notification_update(void);

/* Remove */
int notification_remove(int id);
int notification_remove_at(int index);
void notification_clear_all(void);
int notification_dismiss_by_type(NotificationType type);

/* Query */
int notification_get_count(void);
NotificationEntry* notification_get(int index);
int notification_has_pending(void);
u32 notification_get_total_duration(int index);

/* Settings */
void notification_set_max_display(int max);
void notification_set_default_duration(u32 duration);
void notification_set_enabled(int enabled);
int notification_is_enabled(void);

/* Utility */
u32 notification_get_color(NotificationType type);
u16 notification_get_icon(NotificationType type);
const char* notification_type_to_string(NotificationType type);

/* Modify */
int notification_extend(int id, u32 additional_ms);

#endif /* NOTIFICATION_H */
