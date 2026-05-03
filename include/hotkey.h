/*
 * Stone Age Client - Hotkey System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef HOTKEY_H
#define HOTKEY_H

#include "types.h"

/* Constants */
#define MAX_HOTKEYS        64

/* Modifier flags */
#define HOTKEY_CTRL        0x01
#define HOTKEY_ALT         0x02
#define HOTKEY_SHIFT       0x04

/* Hotkey IDs */
typedef enum {
    HOTKEY_NONE = 0,
    HOTKEY_ATTACK = 1,
    HOTKEY_SKILL = 2,
    HOTKEY_ITEM = 3,
    HOTKEY_MAP = 4,
    HOTKEY_PARTY = 5,
    HOTKEY_GUILD = 6,
    HOTKEY_FRIEND = 7,
    HOTKEY_CHAT = 8,
    HOTKEY_INVENTORY = 9,
    HOTKEY_CHARACTER = 10,
    HOTKEY_QUEST = 11,
    HOTKEY_MAIL = 12,
    HOTKEY_MINIMAP = 13,
    HOTKEY_SCREENSHOT = 14,
    HOTKEY_OPTIONS = 15,
    HOTKEY_SKILL_1 = 20,
    HOTKEY_SKILL_2 = 21,
    HOTKEY_SKILL_3 = 22,
    HOTKEY_SKILL_4 = 23,
    HOTKEY_SKILL_5 = 24,
    HOTKEY_SKILL_6 = 25,
    HOTKEY_SKILL_7 = 26,
    HOTKEY_SKILL_8 = 27,
    HOTKEY_ITEM_1 = 30,
    HOTKEY_ITEM_2 = 31,
    HOTKEY_ITEM_3 = 32,
    HOTKEY_ITEM_4 = 33,
    HOTKEY_ITEM_5 = 34,
    HOTKEY_PET_1 = 40,
    HOTKEY_PET_2 = 41,
    HOTKEY_PET_3 = 42,
    HOTKEY_PET_4 = 43,
    HOTKEY_PET_5 = 44,
    HOTKEY_AUTO_WALK = 50,
    HOTKEY_SIT = 51,
    HOTKEY_PICKUP = 52,
    HOTKEY_TARGET_NEXT = 53,
    HOTKEY_REPLY = 54,
    HOTKEY_CUSTOM_START = 100
} HotkeyID;

/* Hotkey binding */
typedef struct {
    HotkeyID id;
    u8 key;
    u8 modifiers;
    char name[32];
    int enabled;

} HotkeyBinding;

/* Hotkey context */
typedef struct {
    /* Bindings */
    HotkeyBinding bindings[MAX_HOTKEYS];
    int binding_count;

    /* State */
    HotkeyID last_hotkey;
    u32 trigger_time;
    int repeat_count;

    /* System state */
    int enabled;

} HotkeyContext;

/* Global hotkey context */
extern HotkeyContext g_hotkey;

/* Initialization */
int hotkey_init(void);
void hotkey_shutdown(void);

/* Default bindings */
void hotkey_load_defaults(void);

/* Config */
int hotkey_load_config(const char* filename);
int hotkey_save_config(const char* filename);

/* Binding management */
int hotkey_set_binding(HotkeyID id, u8 key, u8 modifiers);
HotkeyBinding* hotkey_get_binding(HotkeyID id);
HotkeyID hotkey_find(u8 key, u8 modifiers);

/* Check */
int hotkey_check(HotkeyID id);
HotkeyID hotkey_process_input(u8 key, u8 modifiers);

/* Enable/disable */
void hotkey_set_enabled(HotkeyID id, int enabled);
int hotkey_is_enabled(HotkeyID id);
void hotkey_set_all_enabled(int enabled);
int hotkey_is_system_enabled(void);

/* Strings */
const char* hotkey_get_key_name(u8 key);
const char* hotkey_get_modifier_string(u8 modifiers);
const char* hotkey_get_full_string(HotkeyID id);

/* Reset */
int hotkey_reset_binding(HotkeyID id);
void hotkey_reset_all(void);

/* Query */
int hotkey_get_binding_count(void);
HotkeyBinding* hotkey_get_binding_by_index(int index);
HotkeyID hotkey_get_last_triggered(void);

/* Repeat */
void hotkey_clear_repeat(void);
int hotkey_is_repeat(HotkeyID id, u32 delay_ms);

#endif /* HOTKEY_H */
