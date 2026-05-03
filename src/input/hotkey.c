/*
 * Stone Age Client - Hotkey System Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "hotkey.h"
#include "logger.h"

/* Global hotkey context */
HotkeyContext g_hotkey = {0};

/* Default hotkey bindings */
static const HotkeyBinding s_default_bindings[] = {
    { HOTKEY_ATTACK,       'A',      0,           "Attack" },
    { HOTKEY_SKILL,        'S',      0,           "Skill Menu" },
    { HOTKEY_ITEM,         'I',      0,           "Item Menu" },
    { HOTKEY_MAP,          'M',      0,           "Map" },
    { HOTKEY_PARTY,        'P',      0,           "Party" },
    { HOTKEY_GUILD,        'G',      0,           "Guild" },
    { HOTKEY_FRIEND,       'F',      0,           "Friend List" },
    { HOTKEY_CHAT,         VK_RETURN, 0,          "Chat" },
    { HOTKEY_INVENTORY,    'E',      0,           "Inventory" },
    { HOTKEY_CHARACTER,    'C',      0,           "Character" },
    { HOTKEY_QUEST,        'Q',      0,           "Quest" },
    { HOTKEY_MAIL,         'L',      0,           "Mail" },
    { HOTKEY_MINIMAP,      VK_TAB,   0,           "Toggle Minimap" },
    { HOTKEY_SCREENSHOT,   VK_F12,   0,           "Screenshot" },
    { HOTKEY_OPTIONS,      VK_ESCAPE, 0,          "Options" },
    { HOTKEY_SKILL_1,      '1',      0,           "Skill Slot 1" },
    { HOTKEY_SKILL_2,      '2',      0,           "Skill Slot 2" },
    { HOTKEY_SKILL_3,      '3',      0,           "Skill Slot 3" },
    { HOTKEY_SKILL_4,      '4',      0,           "Skill Slot 4" },
    { HOTKEY_SKILL_5,      '5',      0,           "Skill Slot 5" },
    { HOTKEY_SKILL_6,      '6',      0,           "Skill Slot 6" },
    { HOTKEY_SKILL_7,      '7',      0,           "Skill Slot 7" },
    { HOTKEY_SKILL_8,      '8',      0,           "Skill Slot 8" },
    { HOTKEY_ITEM_1,       '1',      HOTKEY_CTRL, "Item Slot 1" },
    { HOTKEY_ITEM_2,       '2',      HOTKEY_CTRL, "Item Slot 2" },
    { HOTKEY_ITEM_3,       '3',      HOTKEY_CTRL, "Item Slot 3" },
    { HOTKEY_ITEM_4,       '4',      HOTKEY_CTRL, "Item Slot 4" },
    { HOTKEY_ITEM_5,       '5',      HOTKEY_CTRL, "Item Slot 5" },
    { HOTKEY_PET_1,        '1',      HOTKEY_ALT,  "Pet Slot 1" },
    { HOTKEY_PET_2,        '2',      HOTKEY_ALT,  "Pet Slot 2" },
    { HOTKEY_PET_3,        '3',      HOTKEY_ALT,  "Pet Slot 3" },
    { HOTKEY_PET_4,        '4',      HOTKEY_ALT,  "Pet Slot 4" },
    { HOTKEY_PET_5,        '5',      HOTKEY_ALT,  "Pet Slot 5" },
    { HOTKEY_AUTO_WALK,    'W',      0,           "Auto Walk" },
    { HOTKEY_SIT,          VK_INSERT, 0,          "Sit/Stand" },
    { HOTKEY_PICKUP,       VK_SPACE, 0,           "Pickup Item" },
    { HOTKEY_TARGET_NEXT,  VK_TAB,   HOTKEY_CTRL, "Next Target" },
    { HOTKEY_REPLY,        'R',      0,           "Reply Whisper" }
};

#define DEFAULT_BINDING_COUNT (sizeof(s_default_bindings) / sizeof(s_default_bindings[0]))

/*
 * Initialize hotkey system
 */
int hotkey_init(void) {
    memset(&g_hotkey, 0, sizeof(HotkeyContext));

    /* Load default bindings */
    hotkey_load_defaults();

    /* Try to load from file */
    hotkey_load_config("data\\hotkey.dat");

    LOG_INFO("Hotkey system initialized");
    return 1;
}

/*
 * Shutdown hotkey system
 */
void hotkey_shutdown(void) {
    hotkey_save_config("data\\hotkey.dat");
    memset(&g_hotkey, 0, sizeof(HotkeyContext));
    LOG_INFO("Hotkey system shutdown");
}

/*
 * Load default bindings
 */
void hotkey_load_defaults(void) {
    u32 i;

    for (i = 0; i < DEFAULT_BINDING_COUNT && i < MAX_HOTKEYS; i++) {
        g_hotkey.bindings[i].id = s_default_bindings[i].id;
        g_hotkey.bindings[i].key = s_default_bindings[i].key;
        g_hotkey.bindings[i].modifiers = s_default_bindings[i].modifiers;
        strncpy(g_hotkey.bindings[i].name, s_default_bindings[i].name, 31);
        g_hotkey.bindings[i].name[31] = '\0';
        g_hotkey.bindings[i].enabled = 1;
    }

    g_hotkey.binding_count = DEFAULT_BINDING_COUNT;
}

/*
 * Load hotkey config from file
 */
int hotkey_load_config(const char* filename) {
    FILE* fp;
    char line[128];
    int id, key, modifiers;

    fp = fopen(filename, "r");
    if (!fp) {
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n') continue;

        if (sscanf(line, "%d,%d,%d", &id, &key, &modifiers) == 3) {
            hotkey_set_binding((HotkeyID)id, (u8)key, (u8)modifiers);
        }
    }

    fclose(fp);
    return 1;
}

/*
 * Save hotkey config to file
 */
int hotkey_save_config(const char* filename) {
    FILE* fp;
    u32 i;

    fp = fopen(filename, "w");
    if (!fp) {
        LOG_ERROR("Failed to save hotkey config: %s", filename);
        return 0;
    }

    fprintf(fp, "# Stone Age Hotkey Config\n");
    fprintf(fp, "# Format: id,key,modifiers\n");

    for (i = 0; i < g_hotkey.binding_count; i++) {
        fprintf(fp, "%d,%d,%d\n",
                g_hotkey.bindings[i].id,
                g_hotkey.bindings[i].key,
                g_hotkey.bindings[i].modifiers);
    }

    fclose(fp);
    return 1;
}

/*
 * Set hotkey binding
 */
int hotkey_set_binding(HotkeyID id, u8 key, u8 modifiers) {
    u32 i;

    /* Find existing binding */
    for (i = 0; i < g_hotkey.binding_count; i++) {
        if (g_hotkey.bindings[i].id == id) {
            g_hotkey.bindings[i].key = key;
            g_hotkey.bindings[i].modifiers = modifiers;
            return 1;
        }
    }

    /* Add new binding */
    if (g_hotkey.binding_count < MAX_HOTKEYS) {
        g_hotkey.bindings[g_hotkey.binding_count].id = id;
        g_hotkey.bindings[g_hotkey.binding_count].key = key;
        g_hotkey.bindings[g_hotkey.binding_count].modifiers = modifiers;
        g_hotkey.bindings[g_hotkey.binding_count].enabled = 1;
        g_hotkey.binding_count++;
        return 1;
    }

    return 0;
}

/*
 * Get hotkey binding
 */
HotkeyBinding* hotkey_get_binding(HotkeyID id) {
    u32 i;

    for (i = 0; i < g_hotkey.binding_count; i++) {
        if (g_hotkey.bindings[i].id == id) {
            return &g_hotkey.bindings[i];
        }
    }

    return NULL;
}

/*
 * Find hotkey by key and modifiers
 */
HotkeyID hotkey_find(u8 key, u8 modifiers) {
    u32 i;

    for (i = 0; i < g_hotkey.binding_count; i++) {
        if (g_hotkey.bindings[i].key == key &&
            g_hotkey.bindings[i].modifiers == modifiers &&
            g_hotkey.bindings[i].enabled) {
            return g_hotkey.bindings[i].id;
        }
    }

    return HOTKEY_NONE;
}

/*
 * Check if hotkey is triggered
 */
int hotkey_check(HotkeyID id) {
    HotkeyBinding* binding = hotkey_get_binding(id);
    if (!binding || !binding->enabled) return 0;

    /* Check key state */
    if (GetAsyncKeyState(binding->key) & 0x8000) {
        /* Check modifiers */
        if (binding->modifiers & HOTKEY_CTRL) {
            if (!(GetAsyncKeyState(VK_CONTROL) & 0x8000)) return 0;
        }
        if (binding->modifiers & HOTKEY_ALT) {
            if (!(GetAsyncKeyState(VK_MENU) & 0x8000)) return 0;
        }
        if (binding->modifiers & HOTKEY_SHIFT) {
            if (!(GetAsyncKeyState(VK_SHIFT) & 0x8000)) return 0;
        }

        return 1;
    }

    return 0;
}

/*
 * Process keyboard input
 */
HotkeyID hotkey_process_input(u8 key, u8 modifiers) {
    HotkeyID id = hotkey_find(key, modifiers);

    if (id != HOTKEY_NONE) {
        g_hotkey.last_hotkey = id;
        g_hotkey.trigger_time = timeGetTime();
    }

    return id;
}

/*
 * Enable/disable hotkey
 */
void hotkey_set_enabled(HotkeyID id, int enabled) {
    HotkeyBinding* binding = hotkey_get_binding(id);
    if (binding) {
        binding->enabled = enabled;
    }
}

/*
 * Check if hotkey enabled
 */
int hotkey_is_enabled(HotkeyID id) {
    HotkeyBinding* binding = hotkey_get_binding(id);
    return binding ? binding->enabled : 0;
}

/*
 * Enable/disable all hotkeys
 */
void hotkey_set_all_enabled(int enabled) {
    u32 i;
    for (i = 0; i < g_hotkey.binding_count; i++) {
        g_hotkey.bindings[i].enabled = enabled;
    }
    g_hotkey.enabled = enabled;
}

/*
 * Check if hotkeys enabled
 */
int hotkey_is_system_enabled(void) {
    return g_hotkey.enabled;
}

/*
 * Get key name string
 */
const char* hotkey_get_key_name(u8 key) {
    static char buffer[16];

    if (key >= 'A' && key <= 'Z') {
        buffer[0] = key;
        buffer[1] = '\0';
        return buffer;
    }

    if (key >= '0' && key <= '9') {
        buffer[0] = key;
        buffer[1] = '\0';
        return buffer;
    }

    switch (key) {
        case VK_F1: return "F1";
        case VK_F2: return "F2";
        case VK_F3: return "F3";
        case VK_F4: return "F4";
        case VK_F5: return "F5";
        case VK_F6: return "F6";
        case VK_F7: return "F7";
        case VK_F8: return "F8";
        case VK_F9: return "F9";
        case VK_F10: return "F10";
        case VK_F11: return "F11";
        case VK_F12: return "F12";
        case VK_RETURN: return "Enter";
        case VK_ESCAPE: return "Esc";
        case VK_SPACE: return "Space";
        case VK_TAB: return "Tab";
        case VK_INSERT: return "Insert";
        case VK_DELETE: return "Delete";
        case VK_HOME: return "Home";
        case VK_END: return "End";
        case VK_PRIOR: return "PgUp";
        case VK_NEXT: return "PgDn";
        case VK_UP: return "Up";
        case VK_DOWN: return "Down";
        case VK_LEFT: return "Left";
        case VK_RIGHT: return "Right";
        default:
            snprintf(buffer, sizeof(buffer), "Key%d", key);
            return buffer;
    }
}

/*
 * Get modifier string
 */
const char* hotkey_get_modifier_string(u8 modifiers) {
    static char buffer[32];

    buffer[0] = '\0';

    if (modifiers & HOTKEY_CTRL) {
        strcat(buffer, "Ctrl+");
    }
    if (modifiers & HOTKEY_ALT) {
        strcat(buffer, "Alt+");
    }
    if (modifiers & HOTKEY_SHIFT) {
        strcat(buffer, "Shift+");
    }

    return buffer;
}

/*
 * Get full hotkey string
 */
const char* hotkey_get_full_string(HotkeyID id) {
    static char buffer[64];
    HotkeyBinding* binding;

    binding = hotkey_get_binding(id);
    if (!binding) return "None";

    snprintf(buffer, sizeof(buffer), "%s%s",
             hotkey_get_modifier_string(binding->modifiers),
             hotkey_get_key_name(binding->key));

    return buffer;
}

/*
 * Reset binding to default
 */
int hotkey_reset_binding(HotkeyID id) {
    u32 i;

    for (i = 0; i < DEFAULT_BINDING_COUNT; i++) {
        if (s_default_bindings[i].id == id) {
            return hotkey_set_binding(id, s_default_bindings[i].key,
                                       s_default_bindings[i].modifiers);
        }
    }

    return 0;
}

/*
 * Reset all bindings to default
 */
void hotkey_reset_all(void) {
    hotkey_load_defaults();
}

/*
 * Get binding count
 */
int hotkey_get_binding_count(void) {
    return g_hotkey.binding_count;
}

/*
 * Get binding by index
 */
HotkeyBinding* hotkey_get_binding_by_index(int index) {
    if (index < 0 || index >= g_hotkey.binding_count) {
        return NULL;
    }
    return &g_hotkey.bindings[index];
}

/*
 * Get last triggered hotkey
 */
HotkeyID hotkey_get_last_triggered(void) {
    return g_hotkey.last_hotkey;
}

/*
 * Clear hotkey repeat state
 */
void hotkey_clear_repeat(void) {
    g_hotkey.repeat_count = 0;
}

/*
 * Check for hotkey repeat
 */
int hotkey_is_repeat(HotkeyID id, u32 delay_ms) {
    if (g_hotkey.last_hotkey == id) {
        if (timeGetTime() - g_hotkey.trigger_time < delay_ms) {
            g_hotkey.repeat_count++;
            return 1;
        }
    }
    return 0;
}
