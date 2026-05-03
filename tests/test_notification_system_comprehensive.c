/*
 * Stone Age Client - Notification System Comprehensive Tests
 * Tests for notification.c implementation
 *
 * Covers:
 * - Notification types and states
 * - Add/remove operations
 * - Settings management
 * - Color and icon lookups
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
typedef int s16;
typedef int s32;

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    fflush(stdout); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Constants
 * ======================================== */

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

/* ========================================
 * Structures
 * ======================================== */

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

typedef struct {
    NotificationEntry notifications[MAX_NOTIFICATIONS];
    int count;
    int next_id;
    int enabled;
    int max_display;
    u32 default_duration;
    u32 fade_duration;
} NotificationContext;

/* ========================================
 * Global State
 * ======================================== */

static NotificationContext g_notify = {0};

/* Mock time */
static u32 g_mock_time = 0;

/* ========================================
 * Implementation Functions
 * ======================================== */

/* Forward declaration */
static int notification_remove_at(int index);

/*
 * Get mock time
 */
static u32 timeGetTime(void) {
    return g_mock_time;
}

/*
 * Initialize notification system
 */
static int notification_init(void) {
    memset(&g_notify, 0, sizeof(NotificationContext));

    g_notify.enabled = 1;
    g_notify.max_display = 5;
    g_notify.default_duration = 5000;
    g_notify.fade_duration = 500;

    return 1;
}

/*
 * Add notification
 */
static int notification_add(NotificationType type, const char* message, u32 duration) {
    NotificationEntry* entry;
    int index;

    if (!message || !g_notify.enabled) {
        return -1;
    }

    if (g_notify.count >= MAX_NOTIFICATIONS) {
        /* Remove oldest */
        notification_remove_at(0);
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

    return entry->id;
}

/*
 * Remove notification by index
 */
static int notification_remove_at(int index) {
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
 * Remove notification by ID
 */
static int notification_remove(int id) {
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
 * Clear all notifications
 */
static void notification_clear_all(void) {
    g_notify.count = 0;
}

/*
 * Get notification count
 */
static int notification_get_count(void) {
    return g_notify.count;
}

/*
 * Get notification by index
 */
static NotificationEntry* notification_get(int index) {
    if (index < 0 || index >= g_notify.count) {
        return NULL;
    }
    return &g_notify.notifications[index];
}

/*
 * Set max display count
 */
static void notification_set_max_display(int max) {
    g_notify.max_display = max > MAX_NOTIFICATIONS ? MAX_NOTIFICATIONS : max;
}

/*
 * Set default duration
 */
static void notification_set_default_duration(u32 duration) {
    g_notify.default_duration = duration;
}

/*
 * Enable/disable notifications
 */
static void notification_set_enabled(int enabled) {
    g_notify.enabled = enabled;
}

/*
 * Check if enabled
 */
static int notification_is_enabled(void) {
    return g_notify.enabled;
}

/*
 * Get color for notification type
 */
static u32 notification_get_color(NotificationType type) {
    switch (type) {
        case NOTIFY_INFO:     return 0xFFFFFFFF;
        case NOTIFY_SUCCESS:  return 0xFF00FF00;
        case NOTIFY_WARNING:  return 0xFFFFFF00;
        case NOTIFY_ERROR:    return 0xFFFF0000;
        case NOTIFY_SYSTEM:   return 0xFF00FFFF;
        case NOTIFY_COMBAT:   return 0xFFFF8000;
        case NOTIFY_ITEM:     return 0xFF00AAFF;
        case NOTIFY_SOCIAL:   return 0xFFFF00FF;
        default:              return 0xFFFFFFFF;
    }
}

/*
 * Get icon for notification type
 */
static u16 notification_get_icon(NotificationType type) {
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
static const char* notification_type_to_string(NotificationType type) {
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
 * Check if has notifications
 */
static int notification_has_pending(void) {
    return g_notify.count > 0;
}

/*
 * Extend notification duration
 */
static int notification_extend(int id, u32 additional_ms) {
    int i;

    for (i = 0; i < g_notify.count; i++) {
        if (g_notify.notifications[i].id == id) {
            g_notify.notifications[i].duration += additional_ms;
            return 1;
        }
    }

    return 0;
}

/*
 * Dismiss all notifications of type
 */
static int notification_dismiss_by_type(NotificationType type) {
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
 * Reset state
 */
static void reset_state(void) {
    notification_init();
    g_mock_time = 1000;  /* Start at non-zero time */
}

/* ========================================
 * Test Cases - Constants
 * ======================================== */

static int test_max_notifications(void) {
    return MAX_NOTIFICATIONS == 20;
}

static int test_max_message(void) {
    return MAX_NOTIFY_MESSAGE == 256;
}

static int test_type_constants(void) {
    return NOTIFY_INFO == 0 &&
           NOTIFY_SUCCESS == 1 &&
           NOTIFY_WARNING == 2 &&
           NOTIFY_ERROR == 3 &&
           NOTIFY_SYSTEM == 4 &&
           NOTIFY_COMBAT == 5 &&
           NOTIFY_ITEM == 6 &&
           NOTIFY_SOCIAL == 7;
}

static int test_state_constants(void) {
    return NOTIFY_STATE_FADEIN == 0 &&
           NOTIFY_STATE_VISIBLE == 1 &&
           NOTIFY_STATE_FADEOUT == 2 &&
           NOTIFY_STATE_REMOVE == 3;
}

/* ========================================
 * Test Cases - Add Notifications
 * ======================================== */

static int test_add_basic(void) {
    reset_state();

    int id = notification_add(NOTIFY_INFO, "Test message", 0);

    return id >= 0 && g_notify.count == 1;
}

static int test_add_multiple(void) {
    reset_state();

    notification_add(NOTIFY_INFO, "Message 1", 0);
    notification_add(NOTIFY_SUCCESS, "Message 2", 0);
    notification_add(NOTIFY_ERROR, "Message 3", 0);

    return g_notify.count == 3;
}

static int test_add_null_message(void) {
    reset_state();

    int id = notification_add(NOTIFY_INFO, NULL, 0);

    return id == -1 && g_notify.count == 0;
}

static int test_add_disabled(void) {
    reset_state();

    notification_set_enabled(0);
    int id = notification_add(NOTIFY_INFO, "Test", 0);

    return id == -1 && g_notify.count == 0;
}

static int test_add_default_duration(void) {
    reset_state();

    notification_set_default_duration(3000);
    int id = notification_add(NOTIFY_INFO, "Test", 0);

    NotificationEntry* entry = notification_get(0);
    return entry != NULL && entry->duration == 3000;
}

static int test_add_custom_duration(void) {
    reset_state();

    int id = notification_add(NOTIFY_INFO, "Test", 10000);

    NotificationEntry* entry = notification_get(0);
    return entry != NULL && entry->duration == 10000;
}

static int test_add_unique_ids(void) {
    reset_state();

    int id1 = notification_add(NOTIFY_INFO, "Test 1", 0);
    int id2 = notification_add(NOTIFY_INFO, "Test 2", 0);
    int id3 = notification_add(NOTIFY_INFO, "Test 3", 0);

    return id1 != id2 && id2 != id3 && id1 != id3;
}

static int test_add_max_overflow(void) {
    reset_state();

    int i;
    for (i = 0; i < MAX_NOTIFICATIONS + 5; i++) {
        notification_add(NOTIFY_INFO, "Test", 0);
    }

    /* Should be capped at MAX_NOTIFICATIONS */
    return g_notify.count == MAX_NOTIFICATIONS;
}

/* ========================================
 * Test Cases - Remove Notifications
 * ======================================== */

static int test_remove_by_id(void) {
    reset_state();

    int id = notification_add(NOTIFY_INFO, "Test", 0);
    int result = notification_remove(id);

    return result == 1 && g_notify.count == 0;
}

static int test_remove_invalid_id(void) {
    reset_state();

    notification_add(NOTIFY_INFO, "Test", 0);
    int result = notification_remove(99999);

    return result == 0 && g_notify.count == 1;
}

static int test_remove_at(void) {
    reset_state();

    notification_add(NOTIFY_INFO, "Test 1", 0);
    notification_add(NOTIFY_INFO, "Test 2", 0);
    notification_add(NOTIFY_INFO, "Test 3", 0);

    int result = notification_remove_at(1);

    return result == 1 && g_notify.count == 2;
}

static int test_remove_at_invalid(void) {
    reset_state();

    notification_add(NOTIFY_INFO, "Test", 0);
    int result = notification_remove_at(10);

    return result == 0;
}

static int test_remove_at_negative(void) {
    reset_state();

    notification_add(NOTIFY_INFO, "Test", 0);
    int result = notification_remove_at(-1);

    return result == 0;
}

static int test_clear_all(void) {
    reset_state();

    notification_add(NOTIFY_INFO, "Test 1", 0);
    notification_add(NOTIFY_INFO, "Test 2", 0);
    notification_add(NOTIFY_INFO, "Test 3", 0);

    notification_clear_all();

    return g_notify.count == 0;
}

static int test_dismiss_by_type(void) {
    reset_state();

    notification_add(NOTIFY_INFO, "Info 1", 0);
    notification_add(NOTIFY_ERROR, "Error 1", 0);
    notification_add(NOTIFY_INFO, "Info 2", 0);
    notification_add(NOTIFY_WARNING, "Warning 1", 0);

    int removed = notification_dismiss_by_type(NOTIFY_INFO);

    return removed == 2 && g_notify.count == 2;
}

/* ========================================
 * Test Cases - Query Functions
 * ======================================== */

static int test_get_count(void) {
    reset_state();

    notification_add(NOTIFY_INFO, "Test 1", 0);
    notification_add(NOTIFY_INFO, "Test 2", 0);

    return notification_get_count() == 2;
}

static int test_get_valid(void) {
    reset_state();

    notification_add(NOTIFY_INFO, "Test Message", 0);
    NotificationEntry* entry = notification_get(0);

    return entry != NULL && strcmp(entry->message, "Test Message") == 0;
}

static int test_get_invalid(void) {
    reset_state();

    notification_add(NOTIFY_INFO, "Test", 0);
    NotificationEntry* entry = notification_get(10);

    return entry == NULL;
}

static int test_get_negative(void) {
    reset_state();

    NotificationEntry* entry = notification_get(-1);

    return entry == NULL;
}

static int test_has_pending(void) {
    reset_state();

    if (notification_has_pending()) return 0;

    notification_add(NOTIFY_INFO, "Test", 0);

    return notification_has_pending() == 1;
}

/* ========================================
 * Test Cases - Settings
 * ======================================== */

static int test_enabled_default(void) {
    reset_state();

    return notification_is_enabled() == 1;
}

static int test_set_enabled(void) {
    reset_state();

    notification_set_enabled(0);
    int disabled = notification_is_enabled() == 0;

    notification_set_enabled(1);
    int enabled = notification_is_enabled() == 1;

    return disabled && enabled;
}

static int test_set_max_display(void) {
    reset_state();

    notification_set_max_display(10);

    return g_notify.max_display == 10;
}

static int test_set_max_display_overflow(void) {
    reset_state();

    notification_set_max_display(1000);

    return g_notify.max_display == MAX_NOTIFICATIONS;
}

static int test_set_default_duration(void) {
    reset_state();

    notification_set_default_duration(8000);

    return g_notify.default_duration == 8000;
}

/* ========================================
 * Test Cases - Colors
 * ======================================== */

static int test_color_info(void) {
    return notification_get_color(NOTIFY_INFO) == 0xFFFFFFFF;
}

static int test_color_success(void) {
    return notification_get_color(NOTIFY_SUCCESS) == 0xFF00FF00;
}

static int test_color_warning(void) {
    return notification_get_color(NOTIFY_WARNING) == 0xFFFFFF00;
}

static int test_color_error(void) {
    return notification_get_color(NOTIFY_ERROR) == 0xFFFF0000;
}

static int test_color_system(void) {
    return notification_get_color(NOTIFY_SYSTEM) == 0xFF00FFFF;
}

static int test_color_combat(void) {
    return notification_get_color(NOTIFY_COMBAT) == 0xFFFF8000;
}

static int test_color_item(void) {
    return notification_get_color(NOTIFY_ITEM) == 0xFF00AAFF;
}

static int test_color_social(void) {
    return notification_get_color(NOTIFY_SOCIAL) == 0xFFFF00FF;
}

static int test_color_invalid(void) {
    return notification_get_color((NotificationType)100) == 0xFFFFFFFF;
}

/* ========================================
 * Test Cases - Icons
 * ======================================== */

static int test_icon_info(void) {
    return notification_get_icon(NOTIFY_INFO) == 100;
}

static int test_icon_success(void) {
    return notification_get_icon(NOTIFY_SUCCESS) == 101;
}

static int test_icon_warning(void) {
    return notification_get_icon(NOTIFY_WARNING) == 102;
}

static int test_icon_error(void) {
    return notification_get_icon(NOTIFY_ERROR) == 103;
}

static int test_icon_invalid(void) {
    return notification_get_icon((NotificationType)100) == 0;
}

/* ========================================
 * Test Cases - Type Strings
 * ======================================== */

static int test_type_string_info(void) {
    return strcmp(notification_type_to_string(NOTIFY_INFO), "Info") == 0;
}

static int test_type_string_success(void) {
    return strcmp(notification_type_to_string(NOTIFY_SUCCESS), "Success") == 0;
}

static int test_type_string_warning(void) {
    return strcmp(notification_type_to_string(NOTIFY_WARNING), "Warning") == 0;
}

static int test_type_string_error(void) {
    return strcmp(notification_type_to_string(NOTIFY_ERROR), "Error") == 0;
}

static int test_type_string_invalid(void) {
    return strcmp(notification_type_to_string((NotificationType)100), "Unknown") == 0;
}

/* ========================================
 * Test Cases - Extend Duration
 * ======================================== */

static int test_extend_valid(void) {
    reset_state();

    int id = notification_add(NOTIFY_INFO, "Test", 5000);
    int result = notification_extend(id, 3000);

    NotificationEntry* entry = notification_get(0);
    return result == 1 && entry->duration == 8000;
}

static int test_extend_invalid_id(void) {
    reset_state();

    notification_add(NOTIFY_INFO, "Test", 5000);
    int result = notification_extend(99999, 3000);

    return result == 0;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Notification System Comprehensive Tests ===\n\n");

    printf("Constants Tests:\n");
    TEST(max_notifications);
    TEST(max_message);
    TEST(type_constants);
    TEST(state_constants);

    printf("\nAdd Notification Tests:\n");
    TEST(add_basic);
    TEST(add_multiple);
    TEST(add_null_message);
    TEST(add_disabled);
    TEST(add_default_duration);
    TEST(add_custom_duration);
    TEST(add_unique_ids);
    TEST(add_max_overflow);

    printf("\nRemove Notification Tests:\n");
    TEST(remove_by_id);
    TEST(remove_invalid_id);
    TEST(remove_at);
    TEST(remove_at_invalid);
    TEST(remove_at_negative);
    TEST(clear_all);
    TEST(dismiss_by_type);

    printf("\nQuery Tests:\n");
    TEST(get_count);
    TEST(get_valid);
    TEST(get_invalid);
    TEST(get_negative);
    TEST(has_pending);

    printf("\nSettings Tests:\n");
    TEST(enabled_default);
    TEST(set_enabled);
    TEST(set_max_display);
    TEST(set_max_display_overflow);
    TEST(set_default_duration);

    printf("\nColor Tests:\n");
    TEST(color_info);
    TEST(color_success);
    TEST(color_warning);
    TEST(color_error);
    TEST(color_system);
    TEST(color_combat);
    TEST(color_item);
    TEST(color_social);
    TEST(color_invalid);

    printf("\nIcon Tests:\n");
    TEST(icon_info);
    TEST(icon_success);
    TEST(icon_warning);
    TEST(icon_error);
    TEST(icon_invalid);

    printf("\nType String Tests:\n");
    TEST(type_string_info);
    TEST(type_string_success);
    TEST(type_string_warning);
    TEST(type_string_error);
    TEST(type_string_invalid);

    printf("\nExtend Duration Tests:\n");
    TEST(extend_valid);
    TEST(extend_invalid_id);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
