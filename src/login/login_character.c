/*
 * Stone Age Client - Login Character Module
 * Character management: selection, creation, deletion
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "login.h"
#include "network.h"
#include "logger.h"

/* External login context */
extern LoginContext g_login;

/*
 * Request character list
 */
void login_request_characters(void) {
    char packet[] = "CharList\n";
    network_send(packet, strlen(packet));
    LOG_DEBUG("Requesting character list");
}

/*
 * Handle character list response
 */
void login_handle_characters(void* data, u32 size) {
    u8* ptr = (u8*)data;
    int i;
    CharacterInfo* character;

    g_login.char_count = 0;
    memset(g_login.characters, 0, sizeof(g_login.characters));

    for (i = 0; i < MAX_CHARACTERS && ptr < (u8*)data + size; i++) {
        character = &g_login.characters[i];

        /* Parse character entry */
        character->id = *(u32*)ptr; ptr += 4;
        strncpy(character->name, (char*)ptr, 23); ptr += 24;
        character->level = *(u16*)ptr; ptr += 2;
        character->job = *(u16*)ptr; ptr += 2;
        character->face = *ptr++;
        character->body = *ptr++;
        character->map_id = *(u32*)ptr; ptr += 4;
        character->x = *(u16*)ptr; ptr += 2;
        character->y = *(u16*)ptr; ptr += 2;
        character->exists = 1;

        g_login.char_count++;
    }

    LOG_DEBUG("Received %d characters", g_login.char_count);

    /* Auto-select first character if only one */
    if (g_login.char_count == 1) {
        login_select_character(0);
    }
}

/*
 * Select character
 */
int login_select_character(int index) {
    CharacterInfo* character;

    if (index < 0 || index >= MAX_CHARACTERS) {
        return 0;
    }

    character = &g_login.characters[index];
    if (!character->exists) {
        return 0;
    }

    g_login.selected_char_index = index;
    g_login.state = LOGIN_STATE_CHAR_SELECT;

    LOG_INFO("Selected character: %s", character->name);
    return 1;
}

/*
 * Create new character
 */
int login_create_character(const char* name, u8 face, u8 body, u16 job) {
    char packet[128];

    if (!name || strlen(name) == 0) {
        return 0;
    }

    if (g_login.char_count >= MAX_CHARACTERS) {
        LOG_WARN("No free character slots");
        return 0;
    }

    _snprintf(packet, sizeof(packet), "CreateNewChar|%s|%d|%d|%d\n",
              name, face, body, job);
    network_send(packet, strlen(packet));

    LOG_DEBUG("Creating character: %s", name);
    return 1;
}

/*
 * Handle character create result
 */
void login_handle_create_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;

    result = *ptr++;

    if (result == 0) {
        login_request_characters();
        LOG_INFO("Character created successfully");
    } else {
        LOG_WARN("Failed to create character: result %d", result);
    }
}

/*
 * Delete character
 */
int login_delete_character(int index) {
    char packet[64];
    CharacterInfo* character;

    if (index < 0 || index >= MAX_CHARACTERS) {
        return 0;
    }

    character = &g_login.characters[index];
    if (!character->exists) {
        return 0;
    }

    _snprintf(packet, sizeof(packet), "CharDelete|%s\n", character->name);
    network_send(packet, strlen(packet));

    LOG_DEBUG("Deleting character: %s", character->name);
    return 1;
}

/*
 * Handle character delete result
 */
void login_handle_delete_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;
    u32 char_id;
    int i;

    result = *ptr++;
    char_id = *(u32*)ptr;

    if (result == 0) {
        for (i = 0; i < MAX_CHARACTERS; i++) {
            if (g_login.characters[i].id == char_id) {
                LOG_INFO("Deleted character: %s", g_login.characters[i].name);
                memset(&g_login.characters[i], 0, sizeof(CharacterInfo));
                g_login.char_count--;
                break;
            }
        }
    } else {
        LOG_WARN("Failed to delete character: result %d", result);
    }
}

/*
 * Enter game with selected character
 */
int login_enter_game(void) {
    char packet[64];
    CharacterInfo* character;

    if (g_login.selected_char_index < 0) {
        LOG_WARN("No character selected");
        return 0;
    }

    character = &g_login.characters[g_login.selected_char_index];
    if (!character->exists) {
        return 0;
    }

    _snprintf(packet, sizeof(packet), "CharLogin|%s\n", character->name);
    network_send(packet, strlen(packet));

    g_login.state = LOGIN_STATE_ENTERING_GAME;
    LOG_INFO("Entering game with: %s", character->name);

    return 1;
}

/*
 * Handle enter game response
 */
void login_handle_enter_game(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;

    result = *ptr++;

    if (result == 0) {
        g_login.state = LOGIN_STATE_IN_GAME;
        g_login.in_game = 1;
        LOG_INFO("Entered game successfully");
    } else {
        LOG_WARN("Failed to enter game: result %d", result);
        g_login.state = LOGIN_STATE_CHAR_SELECT;
    }
}

/*
 * Logout from game
 */
int login_logout(void) {
    char packet[] = "CharLogout\n";
    network_send(packet, strlen(packet));

    g_login.in_game = 0;
    g_login.state = LOGIN_STATE_CHAR_LIST;

    LOG_INFO("Logged out from game");
    return 1;
}

/*
 * Return to character selection
 */
int login_return_to_char_select(void) {
    g_login.in_game = 0;
    g_login.state = LOGIN_STATE_CHAR_SELECT;
    g_login.selected_char_index = -1;

    login_request_characters();

    LOG_DEBUG("Returned to character select");
    return 1;
}

/*
 * Get character by index
 */
CharacterInfo* login_get_character(int index) {
    if (index < 0 || index >= MAX_CHARACTERS) {
        return NULL;
    }
    return &g_login.characters[index];
}

/*
 * Get selected character
 */
CharacterInfo* login_get_selected_character(void) {
    if (g_login.selected_char_index < 0) {
        return NULL;
    }
    return &g_login.characters[g_login.selected_char_index];
}

/*
 * Get character count
 */
int login_get_character_count(void) {
    return g_login.char_count;
}

/*
 * Text protocol handlers
 */
void login_handle_text_result(const char* result) {
    if (strcmp(result, "OK") == 0) {
        g_login.result = LOGIN_SUCCESS;
        g_login.logged_in = 1;
        g_login.state = LOGIN_STATE_SERVER_LIST;
        LOG_INFO("Login successful");
    } else if (strcmp(result, "FAIL") == 0) {
        g_login.result = LOGIN_ERR_INVALID_USER;
        strcpy(g_login.error_message, "Login failed");
        g_login.state = LOGIN_STATE_ERROR;
    }
}

void login_handle_create_text_result(const char* result, const char* name) {
    if (strcmp(result, "OK") == 0) {
        LOG_INFO("Character %s created", name);
        login_request_characters();
    } else {
        LOG_WARN("Failed to create character %s", name);
    }
}

void login_handle_delete_text_result(const char* result, const char* name) {
    if (strcmp(result, "OK") == 0) {
        LOG_INFO("Character %s deleted", name);
        login_request_characters();
    } else {
        LOG_WARN("Failed to delete character %s", name);
    }
}

void login_handle_char_login_result(const char* result, const char* name) {
    if (strcmp(result, "OK") == 0) {
        g_login.state = LOGIN_STATE_IN_GAME;
        g_login.in_game = 1;
        LOG_INFO("Character %s logged in", name);
    } else {
        LOG_WARN("Failed to login character %s", name);
    }
}

void login_handle_char_list_text(const char* param1, const char* param2) {
    /* Parse character data from text format */
    LOG_DEBUG("Character list text: %s, %s", param1, param2);
}
