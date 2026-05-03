/*
 * Stone Age Client - Character Selection Screen Implementation
 * Reverse engineered from sa_9061.exe
 *
 * Protocol Analysis:
 * FUN_0045fb80 - Character list handler
 * FUN_00463c20 - Create character result
 * FUN_00463d20 - Delete character result
 * FUN_0045fdc0 - Character login handler
 * FUN_00478a30 - Character slot parser
 *
 * Commands:
 * - CharList: Request character list
 * - CreateNewChar|name|face|body|job: Create new character
 * - CharDelete|name: Delete character
 * - CharLogin|name: Login with character
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "charselect.h"
#include "login.h"
#include "gamestate.h"
#include "render.h"
#include "logger.h"
#include "protocol.h"

/* Forward declarations */
void charselect_populate_slots(void);

/* Global context */
CharSelectContext g_charselect = {0};

/* Error messages - from DAT_004b8xxx strings in FUN_00421c00 */
static const char* s_error_messages[] = {
    "Unknown error",
    "Server error",
    "Connection failed",
    "Connection timeout",
    "Version mismatch",
    "Server is full",
    "Invalid character name",
    "Character name already exists",
    "Character creation failed",
    "Character deletion failed",
    "No empty character slots",
    "Character not found"
};

/*
 * Initialize character selection context
 */
int charselect_init(void) {
    memset(&g_charselect, 0, sizeof(CharSelectContext));
    g_charselect.state = CHAR_SELECT_STATE_INIT;
    g_charselect.selected_slot = -1;

    LOG_INFO("Character selection system initialized");
    return 1;
}

/*
 * Shutdown character selection
 */
void charselect_shutdown(void) {
    memset(&g_charselect, 0, sizeof(CharSelectContext));
    LOG_INFO("Character selection system shutdown");
}

/*
 * Update character selection state machine - FUN_00421110 pattern
 */
void charselect_update(void) {
    int result;
    char window_title[128];

    switch (g_charselect.state) {
        case CHAR_SELECT_STATE_INIT:
            /* State 0: Initialize - Set window title */
            _snprintf(window_title, sizeof(window_title), "StoneAge - %s", "Character Selection");
            /* SetWindowTextA(g_main_hwnd, window_title); */

            /* Check if we have server connection */
            if (login_is_connected()) {
                g_charselect.state = CHAR_SELECT_STATE_WAIT_LIST;
            } else {
                g_charselect.state = CHAR_SELECT_STATE_SERVER_WAIT;
            }
            break;

        case CHAR_SELECT_STATE_WAIT_LIST:
            /* State 1: Wait for character list from server */
            result = login_get_character_count();

            if (result >= 0) {
                /* Character list received */
                g_charselect.state = CHAR_SELECT_STATE_DISPLAY;
                charselect_populate_slots();
            } else if (result == -1) {
                /* Error receiving list */
                g_charselect.state = CHAR_SELECT_STATE_ERROR;
                strcpy(g_charselect.error_msg, "Failed to get character list");
            }
            break;

        case CHAR_SELECT_STATE_DISPLAY:
            /* State 2: Display character list - handled by render */
            break;

        case CHAR_SELECT_STATE_SELECTING:
            /* State 3: User is selecting a character */
            break;

        case CHAR_SELECT_STATE_ENTER_CREATE:
            /* State 4: Transition to character creation */
            gamestate_set_state(GAME_STATE_CHAR_CREATE, 0);
            break;

        case CHAR_SELECT_STATE_ERROR:
            /* State 100: Display error message */
            break;

        case CHAR_SELECT_STATE_ERROR_MSG:
            /* State 101: Wait for user to acknowledge error */
            break;

        case CHAR_SELECT_STATE_SERVER_WAIT:
            /* State 200: Waiting for server connection */
            if (login_is_connected()) {
                g_charselect.state = CHAR_SELECT_STATE_WAIT_LIST;
            }
            break;

        case CHAR_SELECT_STATE_SERVER_CONNECT:
            /* State 201: Connecting to server */
            break;

        default:
            break;
    }
}

/*
 * Populate slots from login system
 */
void charselect_populate_slots(void) {
    int i;
    CharacterInfo* login_char;
    CharSelectSlot* slot;

    g_charselect.slot_count = 0;

    for (i = 0; i < 7 && i < MAX_CHARACTERS; i++) {
        login_char = login_get_character(i);
        slot = &g_charselect.slots[i];

        if (login_char && login_char->exists) {
            memset(slot, 0, sizeof(CharSelectSlot));
            strncpy(slot->name, login_char->name, sizeof(slot->name) - 1);
            slot->level = login_char->level;
            slot->exists = 1;
            slot->map_id = login_char->map_id;
            g_charselect.slot_count++;
        } else {
            memset(slot, 0, sizeof(CharSelectSlot));
        }
    }
}

/*
 * Render character selection screen - FUN_00421110 rendering portion
 */
void charselect_render(void) {
    int i;
    int x, y;
    CharSelectSlot* slot;
    char text[64];

    /* Render background */
    render_sprite(0x7161, 320, 240);  /* Background sprite */

    /* Render title */
    render_text(0x23a, 0x1cc, "Select Character", COLOR_WHITE);

    /* Render character slots */
    x = 0x68;  /* Starting X position from FUN_00421c00 */
    y = 0x54;  /* Starting Y position */

    for (i = 0; i < 7; i++) {
        slot = &g_charselect.slots[i];

        if (slot->exists) {
            /* Render character info - matches FUN_00421c00 pattern */
            _snprintf(text, sizeof(text), "%s", slot->name);
            render_text(x, y + 0x2b, text, COLOR_WHITE);

            _snprintf(text, sizeof(text), "Lv.%d", slot->level);
            render_text(x, y + 0x44, text, COLOR_YELLOW);

            /* Highlight selected slot */
            if (i == g_charselect.selected_slot) {
                render_rect(x - 5, y - 5, 100, 150, COLOR_YELLOW);
            }
        } else {
            /* Empty slot */
            render_text(x, y + 0x2b, "Empty Slot", COLOR_GRAY);
        }

        x += 0x44;  /* Slot width */
    }

    /* Render buttons - Create, Delete, Enter Game */
    render_sprite(0x7169, 100, 350);  /* Create button */
    render_sprite(0x716a, 250, 350);  /* Delete button */
    render_sprite(0x716b, 400, 350);  /* Enter button */

    /* Render error message if in error state */
    if (g_charselect.state == CHAR_SELECT_STATE_ERROR ||
        g_charselect.state == CHAR_SELECT_STATE_ERROR_MSG) {
        render_text(200, 200, g_charselect.error_msg, COLOR_RED);
    }
}

/*
 * Handle mouse click - FUN_00421420 pattern
 */
int charselect_handle_click(int x, int y) {
    int slot_x, slot_y;
    int i;

    /* Check character slot clicks */
    slot_x = 0x68;
    slot_y = 0x54;

    for (i = 0; i < 7; i++) {
        if (x >= slot_x && x < slot_x + 0x40 &&
            y >= slot_y && y < slot_y + 0x100) {
            /* Clicked on character slot */
            if (g_charselect.slots[i].exists) {
                g_charselect.selected_slot = i;
                LOG_DEBUG("Selected character slot %d: %s", i, g_charselect.slots[i].name);
                return 1;
            }
        }
        slot_x += 0x44;
    }

    /* Check Create button */
    if (x >= 100 && x < 200 && y >= 350 && y < 390) {
        if (g_charselect.slot_count < 7) {
            charselect_start_create();
            return 2;
        }
    }

    /* Check Delete button */
    if (x >= 250 && x < 350 && y >= 350 && y < 390) {
        if (g_charselect.selected_slot >= 0) {
            g_charselect.is_deleting = 1;
            g_charselect.confirm_delete = 0;
            return 3;
        }
    }

    /* Check Enter button */
    if (x >= 400 && x < 500 && y >= 350 && y < 390) {
        if (g_charselect.selected_slot >= 0) {
            charselect_enter_game();
            return 4;
        }
    }

    return 0;
}

/*
 * Handle keyboard input
 */
int charselect_handle_key(u32 key) {
    switch (key) {
        case VK_ESCAPE:
            /* Return to login */
            gamestate_set_state(GAME_STATE_LOGIN, 0);
            return 1;

        case VK_RETURN:
            /* Enter game with selected character */
            if (g_charselect.selected_slot >= 0) {
                charselect_enter_game();
                return 1;
            }
            break;

        case VK_LEFT:
            /* Select previous slot */
            if (g_charselect.selected_slot > 0) {
                g_charselect.selected_slot--;
            }
            return 1;

        case VK_RIGHT:
            /* Select next slot */
            if (g_charselect.selected_slot < 6) {
                g_charselect.selected_slot++;
            }
            return 1;
    }

    return 0;
}

/*
 * Select a character slot
 */
int charselect_select_slot(int slot) {
    if (slot < 0 || slot >= 7) {
        return 0;
    }

    if (!g_charselect.slots[slot].exists) {
        return 0;
    }

    g_charselect.selected_slot = slot;
    return 1;
}

/*
 * Enter game with selected character - FUN_00421a30 pattern
 */
int charselect_enter_game(void) {
    CharacterInfo* character;
    char params[64];

    if (g_charselect.selected_slot < 0) {
        return 0;
    }

    character = login_get_character(g_charselect.selected_slot);
    if (!character || !character->exists) {
        return 0;
    }

    /* Send CharLogin command */
    _snprintf(params, sizeof(params), "%s", character->name);
    protocol_send_text_command(0, CHAR_CMD_LOGIN, params);

    LOG_INFO("Entering game with character: %s", character->name);
    return 1;
}

/*
 * Start character creation - FUN_00421a40 pattern
 */
int charselect_start_create(void) {
    if (g_charselect.slot_count >= 7) {
        strcpy(g_charselect.error_msg, "No empty character slots");
        g_charselect.state = CHAR_SELECT_STATE_ERROR;
        return 0;
    }

    g_charselect.is_creating = 1;
    g_charselect.create_name[0] = '\0';
    g_charselect.create_face = 0;
    g_charselect.create_body = 0;
    g_charselect.create_job = 0;

    gamestate_set_state(GAME_STATE_CHAR_CREATE, 0);
    return 1;
}

/*
 * Delete selected character
 */
int charselect_delete_character(void) {
    CharacterInfo* character;
    char params[64];

    if (g_charselect.selected_slot < 0) {
        return 0;
    }

    character = login_get_character(g_charselect.selected_slot);
    if (!character || !character->exists) {
        return 0;
    }

    /* Send CharDelete command */
    _snprintf(params, sizeof(params), "%s", character->name);
    protocol_send_text_command(0, CHAR_CMD_DELETE, params);

    LOG_INFO("Deleting character: %s", character->name);
    return 1;
}

/*
 * Update character creation state machine - FUN_00421c00 pattern
 */
void charcreate_update(void) {
    int result;

    switch (g_charselect.state) {
        case CHAR_CREATE_STATE_INIT:
            /* State 0: Initialize creation screen */
            g_charselect.state = CHAR_CREATE_STATE_WAIT_NETWORK;
            break;

        case CHAR_CREATE_STATE_WAIT_NETWORK:
            /* State 1: Wait for network - check connection */
            if (login_is_connected()) {
                g_charselect.state = CHAR_CREATE_STATE_DISPLAY;
            } else {
                g_charselect.state = CHAR_CREATE_STATE_ERROR;
                strcpy(g_charselect.error_msg, "Not connected to server");
            }
            break;

        case CHAR_CREATE_STATE_DISPLAY:
            /* State 10: Display creation UI */
            break;

        case CHAR_CREATE_STATE_CONFIRM_DELETE:
            /* State 11: Confirm character deletion */
            break;

        case CHAR_CREATE_STATE_SELECT_CHAR:
            /* State 20: Selecting character for deletion */
            break;

        case CHAR_CREATE_STATE_DELETE_WAIT:
            /* State 21: Waiting for delete confirmation */
            break;

        case CHAR_CREATE_STATE_DELETE_CONFIRM:
            /* State 22: Delete confirmed */
            break;

        case CHAR_CREATE_STATE_ERROR:
            /* State 100: Error */
            break;

        case CHAR_CREATE_STATE_DELETED:
            /* State 300: Character deleted */
            g_charselect.state = CHAR_CREATE_STATE_DISPLAY;
            break;

        default:
            break;
    }
}

/*
 * Render character creation screen
 */
void charcreate_render(void) {
    char text[64];

    /* Render background */
    render_sprite(0x7168, 320, 240);

    /* Render creation UI */
    render_text(160, 50, "Create New Character", COLOR_WHITE);

    /* Character name input */
    render_text(100, 100, "Name:", COLOR_WHITE);
    render_text(200, 100, g_charselect.create_name, COLOR_YELLOW);

    /* Face selection */
    render_text(100, 130, "Face:", COLOR_WHITE);
    _snprintf(text, sizeof(text), "%d", g_charselect.create_face);
    render_text(200, 130, text, COLOR_YELLOW);

    /* Body selection */
    render_text(100, 160, "Body:", COLOR_WHITE);
    _snprintf(text, sizeof(text), "%d", g_charselect.create_body);
    render_text(200, 160, text, COLOR_YELLOW);

    /* Job selection */
    render_text(100, 190, "Job:", COLOR_WHITE);
    _snprintf(text, sizeof(text), "%d", g_charselect.create_job);
    render_text(200, 190, text, COLOR_YELLOW);

    /* Preview sprite - render character based on face/body */
    {
        /* Character sprite ID formula: 0x20000 + face * 100 + body * 10 */
        u32 base_sprite = 0x20000;
        u32 face_sprite = g_charselect.create_face * 100;
        u32 body_sprite = g_charselect.create_body * 10;
        u32 preview_sprite = base_sprite + face_sprite + body_sprite;

        /* Render preview at center-right of screen */
        int preview_x = 400;
        int preview_y = 200;

        /* Draw preview background */
        render_fill_rect(NULL, preview_x - 40, preview_y - 60, 80, 120, 0x2104);

        /* Try to render the sprite */
        if (!render_sprite((u16)preview_sprite, preview_x - 32, preview_y - 32)) {
            /* Fallback: draw placeholder based on face/body */
            u16 placeholder_sprite;

            /* Use face/body to select from available sprites */
            if (g_charselect.create_face < 5) {
                placeholder_sprite = (u16)(0x7100 + g_charselect.create_face * 10 + g_charselect.create_body);
            } else {
                placeholder_sprite = (u16)(0x7150 + (g_charselect.create_face - 5) * 10 + g_charselect.create_body);
            }

            render_sprite(placeholder_sprite, preview_x - 32, preview_y - 32);
        }

        /* Draw selection info */
        _snprintf(text, sizeof(text), "Preview: F%d B%d",
                 g_charselect.create_face, g_charselect.create_body);
        render_text(preview_x - 40, preview_y + 50, text, COLOR_WHITE);
    }

    /* Buttons */
    render_sprite(0x7169, 200, 350);  /* Confirm button */
    render_sprite(0x716a, 350, 350);  /* Cancel button */

    /* Error message */
    if (g_charselect.state == CHAR_CREATE_STATE_ERROR) {
        render_text(160, 250, g_charselect.error_msg, COLOR_RED);
    }
}

/*
 * Handle click in character creation screen
 */
int charcreate_handle_click(int x, int y) {
    /* Check Confirm button */
    if (x >= 200 && x < 300 && y >= 350 && y < 390) {
        charcreate_confirm();
        return 1;
    }

    /* Check Cancel button */
    if (x >= 350 && x < 450 && y >= 350 && y < 390) {
        charcreate_cancel();
        return 2;
    }

    /* Check face arrows */
    if (x >= 280 && x < 310 && y >= 130 && y < 150) {
        g_charselect.create_face = (g_charselect.create_face + 1) % 10;
        return 3;
    }
    if (x >= 250 && x < 280 && y >= 130 && y < 150) {
        g_charselect.create_face = (g_charselect.create_face - 1 + 10) % 10;
        return 3;
    }

    /* Check body arrows */
    if (x >= 280 && x < 310 && y >= 160 && y < 180) {
        g_charselect.create_body = (g_charselect.create_body + 1) % 10;
        return 4;
    }
    if (x >= 250 && x < 280 && y >= 160 && y < 180) {
        g_charselect.create_body = (g_charselect.create_body - 1 + 10) % 10;
        return 4;
    }

    return 0;
}

/*
 * Handle keyboard input in creation screen
 */
int charcreate_handle_key(u32 key) {
    if (key == VK_ESCAPE) {
        charcreate_cancel();
        return 1;
    }

    if (key == VK_RETURN) {
        charcreate_confirm();
        return 1;
    }

    /* Handle text input for name */
    if (key >= 'A' && key <= 'Z') {
        size_t len = strlen(g_charselect.create_name);
        if (len < sizeof(g_charselect.create_name) - 1) {
            g_charselect.create_name[len] = (char)key;
            g_charselect.create_name[len + 1] = '\0';
        }
        return 1;
    }

    if (key >= 'a' && key <= 'z') {
        size_t len = strlen(g_charselect.create_name);
        if (len < sizeof(g_charselect.create_name) - 1) {
            g_charselect.create_name[len] = (char)(key - 'a' + 'A');
            g_charselect.create_name[len + 1] = '\0';
        }
        return 1;
    }

    if (key == VK_BACK) {
        size_t len = strlen(g_charselect.create_name);
        if (len > 0) {
            g_charselect.create_name[len - 1] = '\0';
        }
        return 1;
    }

    return 0;
}

/*
 * Confirm character creation
 */
int charcreate_confirm(void) {
    char params[128];

    if (strlen(g_charselect.create_name) == 0) {
        strcpy(g_charselect.error_msg, "Please enter a character name");
        g_charselect.state = CHAR_CREATE_STATE_ERROR;
        return 0;
    }

    /* Send CreateNewChar command: name|face|body|job */
    _snprintf(params, sizeof(params), "%s|%d|%d|%d",
              g_charselect.create_name,
              g_charselect.create_face,
              g_charselect.create_body,
              g_charselect.create_job);

    protocol_send_text_command(0, CHAR_CMD_CREATE, params);

    LOG_INFO("Creating character: %s", g_charselect.create_name);
    return 1;
}

/*
 * Cancel character creation
 */
int charcreate_cancel(void) {
    g_charselect.is_creating = 0;
    gamestate_set_state(GAME_STATE_CHAR_SELECT, 0);
    return 1;
}

/*
 * Handle character list response
 */
void charselect_handle_list(void* data, int size) {
    charselect_populate_slots();
    g_charselect.state = CHAR_SELECT_STATE_DISPLAY;
}

/*
 * Handle create result
 */
void charselect_handle_create_result(int result, const char* name) {
    if (result == 0) {
        LOG_INFO("Character created successfully: %s", name);
        g_charselect.state = CHAR_CREATE_STATE_DELETED;

        /* Refresh character list */
        login_request_characters();

        /* Return to selection screen */
        gamestate_set_state(GAME_STATE_CHAR_SELECT, 0);
    } else {
        g_charselect.state = CHAR_CREATE_STATE_ERROR;
        strcpy(g_charselect.error_msg, s_error_messages[-result]);
        LOG_WARN("Character creation failed: %d", result);
    }
}

/*
 * Handle delete result
 */
void charselect_handle_delete_result(int result, const char* name) {
    if (result == 0) {
        LOG_INFO("Character deleted: %s", name);

        /* Refresh character list */
        login_request_characters();

        g_charselect.selected_slot = -1;
        g_charselect.is_deleting = 0;
    } else {
        g_charselect.state = CHAR_SELECT_STATE_ERROR;
        strcpy(g_charselect.error_msg, s_error_messages[-result]);
        LOG_WARN("Character deletion failed: %d", result);
    }
}

/*
 * Handle enter game result
 */
void charselect_handle_enter_result(int result) {
    if (result == 0) {
        LOG_INFO("Entering game...");
        gamestate_set_state(GAME_STATE_PRELOAD, 0);
    } else {
        g_charselect.state = CHAR_SELECT_STATE_ERROR;
        strcpy(g_charselect.error_msg, "Failed to enter game");
        LOG_WARN("Failed to enter game: %d", result);
    }
}

/*
 * Get current state
 */
int charselect_get_state(void) {
    return g_charselect.state;
}

/*
 * Get selected slot
 */
int charselect_get_selected_slot(void) {
    return g_charselect.selected_slot;
}

/*
 * Get slot by index
 */
CharSelectSlot* charselect_get_slot(int index) {
    if (index < 0 || index >= 7) {
        return NULL;
    }
    return &g_charselect.slots[index];
}

/*
 * Check if any characters exist
 */
int charselect_has_characters(void) {
    return g_charselect.slot_count > 0;
}
