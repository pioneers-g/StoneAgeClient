/*
 * Stone Age Client - Field Update Handler
 * Reverse engineered from sa_9061.exe (FUN_00418370)
 *
 * Main field gameplay update function handling:
 * - Connection keepalive
 * - Player movement and actions
 * - Menu states and transitions
 * - Battle encounters
 * - Map-specific logic
 */

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "field_update.h"
#include "character.h"
#include "map.h"
#include "input.h"
#include "network.h"
#include "battle.h"
#include "fade.h"
#include "ui.h"
#include "logger.h"

/* Use action codes from input.h */
#ifndef FIELD_ACTION_NONE
#define FIELD_ACTION_NONE       0
#define FIELD_ACTION_WALK       1
#define FIELD_ACTION_RUN        2
#define FIELD_ACTION_BATTLE     3
#define FIELD_ACTION_MENU       4
#define FIELD_ACTION_SKILL      5
#define FIELD_ACTION_WAIT       6
#define FIELD_ACTION_ATTACK     7
#define FIELD_ACTION_BATTLE_MENU 8
#define FIELD_ACTION_ITEM       9
#define FIELD_ACTION_ESCAPE     10
#define FIELD_ACTION_SPECIAL    11
#define FIELD_ACTION_DEFEND     12
#define FIELD_ACTION_PET        13
#endif

/* Key lookup array size - from FUN_00420ff0 */
#define KEY_ARRAY_SIZE  14

/* Key state flags - DAT_045f1bc4 */
#define KEY_FLAG_PRESSED     0x01
#define KEY_FLAG_VALID       0x02

/* ========================================
 * Global State Variables
 * Matches DAT_005xxxxx region from binary
 * ======================================== */

/* Connection state - DAT_005676xx region */
static DWORD g_last_keepalive_time = 0;      /* DAT_005676b0 */
static int g_keepalive_pending = 0;           /* DAT_005676a4 */
static int g_connection_state = 0;            /* DAT_005676a0 */
static int g_encounter_state = 0;             /* DAT_00564e62 */

/* Movement state - DAT_0054cdxx region */
static int g_move_state = 0;                  /* DAT_0054cd00 */
static int g_move_direction = 0;              /* DAT_0054cd68 */
static int g_move_step = 0;                   /* DAT_0054cd0c */

/* Menu states - DAT_00564exx region */
static int g_menu_state_3c = 0;               /* DAT_00564e3c */
static int g_menu_state_3e = 0;               /* DAT_00564e3e */
static int g_menu_state_38 = 0;               /* DAT_00564e38 */
static int g_battle_menu_state = 0;          /* DAT_00567690 */
static int g_action_menu_state = 0;          /* DAT_005676a0 */

/* Player state flags */
static int g_player_action = 0;              /* Action being performed */
static int g_wait_counter = 0;               /* Wait timer for actions */

/* ========================================
 * Helper Functions
 * ======================================== */

/*
 * Check if current map has special handling
 * Returns true for maps with restricted features
 */
static int is_special_map(u16 map_id) {
    /* From FUN_00418370: Maps with special handling */
    if (map_id == 0x7bda || map_id == 0x27dc) return 1;  /* Special maps */
    if (map_id >= 0x2969 && map_id <= 0x296e) return 1;  /* Map range */
    if (map_id == 0x2aa7 || map_id == 0x2aa8) return 1;  /* Instance maps */
    if (map_id == 0x50e7 || map_id == 0x50e8) return 1;  /* Dungeon maps */
    if (map_id == 0x3f0 || map_id == 0x3fd) return 1;    /* Special areas */
    if (map_id == 0xbc0 || map_id == 0xbcd) return 1;    /* Arena maps */
    if (map_id >= 0x2008 && map_id <= 0x2015) return 1;  /* Event maps */
    if (map_id >= 0x7541 && map_id <= 0x7545) return 1;  /* PvP maps */
    if (map_id == 0x1b71 || map_id == 0x4269) return 1;  /* Guild maps */
    if (map_id == 0x426b || map_id == 0x426d) return 1;
    if (map_id == 0x1b73) return 1;
    return 0;
}

/*
 * Check if map allows running
 * Returns 0 for restricted maps, 1 otherwise
 */
static int map_allows_run(u16 map_id) {
    /* From FUN_00418370 running logic */
    if (map_id == 0x3f0 || map_id == 0x3fd) return 0;
    if (map_id == 0xbc0 || map_id == 0xbcd) return 0;
    if (map_id == 0x2aa7 || map_id == 0x2aa8) return 0;
    if (map_id == 0x50e7 || map_id == 0x50e8) return 0;
    if (map_id == 0x27dc) return 0;
    return 1;
}

/*
 * Check if map has random encounters
 */
static int map_has_encounters(u16 map_id) {
    /* Maps ending in 7 (xxx7) have encounters */
    if (map_id < 10000 && (map_id / 1000) > 0 && (map_id % 1000) == 7) {
        return 1;
    }
    if (map_id == 0x82) return 1;
    return 0;
}

/*
 * Send keepalive packet to server
 */
static void send_keepalive(void) {
    DWORD current_time = timeGetTime();

    if (g_last_keepalive_time + 500 < current_time) {
        /* FUN_0043b4e0 or FUN_0048f950 depending on mode */
        network_send_keepalive();
        g_last_keepalive_time = current_time;
        g_keepalive_pending = 0;
    }
}

/*
 * Check for random encounter trigger
 */
static int check_encounter(void) {
    /* Encounter check from FUN_00418370 */
    if (g_encounter_state == 0 && !is_special_map(map_get_current_id())) {
        /* Random encounter logic would go here */
        return 0;
    }
    return 0;
}

/*
 * Key lookup in array - FUN_00420ff0
 * Searches for current key in key array
 * Returns index if found, -1 otherwise
 */
static int key_lookup(int* key_array, int count) {
    int i;
    int key_flags = input_get_key_flags();  /* DAT_045f1bc4 */
    int current_key = input_get_current_key();  /* DAT_045f1bf0 */

    /* Check if valid key state */
    if ((key_flags & KEY_FLAG_PRESSED) == 0) {
        return -1;
    }

    /* Search for key in array */
    if (count > 0) {
        for (i = 0; i < count; i++) {
            if (key_array[i] == current_key) {
                return i;
            }
        }
    }

    return -1;
}

/*
 * Get main action from key state - FUN_00421050
 */
static int get_main_action(int* key_array, int count) {
    return key_lookup(key_array, count);
}

/*
 * Get sub action from key state - FUN_004210b0
 */
static int get_sub_action(int* key_array, int count) {
    return key_lookup(key_array, count);
}

/*
 * Get movement direction from input
 */
static int get_move_direction(void) {
    u32 key_state = input_get_key_state();

    /* Check arrow keys - from FUN_00418370 pattern
     * Direction values from FUN_00443e80:
     * 0=Up, 1=Up-Right, 2=Right, 3=Down-Right
     * 4=Down, 5=Down-Left, 6=Left, 7=Up-Left
     */
    if (key_state & KEY_STATE_UP) {
        if (key_state & KEY_STATE_RIGHT) return 1;  /* Up-Right */
        if (key_state & KEY_STATE_LEFT) return 7;   /* Up-Left */
        return 0;  /* Up */
    }
    if (key_state & KEY_STATE_DOWN) {
        if (key_state & KEY_STATE_RIGHT) return 3;  /* Down-Right */
        if (key_state & KEY_STATE_LEFT) return 5;   /* Down-Left */
        return 4;  /* Down */
    }
    if (key_state & KEY_STATE_RIGHT) return 2;  /* Right */
    if (key_state & KEY_STATE_LEFT) return 6;   /* Left */

    return -1;  /* No movement */
}

/*
 * Direction to movement offset - FUN_00443e80
 * Converts direction (0-7) to dx/dy offsets
 */
static void direction_to_offset(int direction, int* dx, int* dy) {
    switch (direction) {
        case 0: *dx = -1; *dy =  1; break;  /* Up */
        case 1: *dx = -1; *dy =  0; break;  /* Up-Right */
        case 2: *dx = -1; *dy = -1; break;  /* Right */
        case 3: *dx =  0; *dy = -1; break;  /* Down-Right */
        case 4: *dx =  1; *dy = -1; break;  /* Down */
        case 5: *dx =  1; *dy =  0; break;  /* Down-Left */
        case 6: *dx =  1; *dy =  1; break;  /* Left */
        case 7: *dx =  0; *dy =  1; break;  /* Up-Left */
        default: *dx = 0; *dy = 0; break;
    }
}

/* ========================================
 * Menu State Handlers
 * ======================================== */

/*
 * Handle menu state transitions
 */
static void handle_menu_states(void) {
    /* Menu state 3c - main menu */
    if (g_menu_state_3c == 1 || g_menu_state_3c == 3) {
        g_action_menu_state = 1;
        fade_out();

        /* Clear effects on menu open */
        if (g_menu_state_38) {
            /* FUN_004011c0 - cleanup */
            g_menu_state_38 = 0;
        }

        g_menu_state_3c = 2;
    }
    else if (g_menu_state_3c == 2) {
        /* Wait for fade complete */
        if (fade_is_complete()) {
            g_menu_state_3c = 0;
            g_action_menu_state = 0;
        }
    }

    /* Menu state 3e - shop/trade menu */
    if (g_menu_state_3e == 1) {
        g_action_menu_state = 1;
        fade_out();
        g_menu_state_3e = 2;
    }
    else if (g_menu_state_3e == 2) {
        /* Wait for fade complete */
        if (fade_is_complete()) {
            g_menu_state_3e = 0;
            g_action_menu_state = 0;
        }
    }
}

/*
 * Handle battle menu states
 */
static void handle_battle_menu(void) {
    if (g_battle_menu_state == 0) {
        return;
    }

    /* Battle menu from FUN_00418370 cases 8 and 0xC */
    switch (g_battle_menu_state) {
        case 1:
            /* Init battle menu */
            fade_out();
            g_battle_menu_state = 2;
            break;

        case 2:
            /* Wait for battle start */
            if (fade_is_complete()) {
                g_battle_menu_state = 0;
                g_action_menu_state = 0;
            }
            break;
    }
}

/* ========================================
 * Action Handlers
 * ======================================== */

/*
 * Handle player movement - walk/run
 * From FUN_00418370 movement handling
 */
static void handle_movement(int is_running) {
    int direction;
    u16 player_x, player_y;
    int dx, dy;
    int move_result;
    DWORD current_time;

    direction = get_move_direction();
    if (direction < 0) {
        return;  /* No movement input */
    }

    character_get_position(&player_x, &player_y);

    /* Get movement offset - FUN_00443e80 */
    direction_to_offset(direction, &dx, &dy);

    /* Check collision - FUN_0040e8c0 (8-way) or FUN_0040e9b0 (with range) */
    move_result = map_check_collision(player_x + dx, player_y + dy, 8);

    if (move_result == 1) {
        /* Valid move - check timing */
        current_time = timeGetTime();
        if (g_last_keepalive_time + 500 < current_time) {
            /* Send move packet - FUN_0043b210 or FUN_0048f700 */
            network_send_move(player_x + dx, player_y + dy, is_running);
            g_last_keepalive_time = current_time;

            /* Set movement flags for animation */
            if (is_running) {
                g_player_action = FIELD_ACTION_RUN;
            } else {
                g_player_action = FIELD_ACTION_WALK;
            }
        }
    }
}

/*
 * Handle encounter trigger
 */
static void handle_encounter(void) {
    if (g_encounter_state == 1) {
        fade_out();
        g_encounter_state = 2;
    }
    else if (g_encounter_state == 2) {
        if (fade_is_complete()) {
            /* Transition to battle */
            g_encounter_state = 0;
            /* Set game state to battle */
        }
    }
}

/*
 * Handle action based on input
 * Complete action dispatcher from FUN_00418370
 */
static void process_action(int action, int sub_action) {
    u16 map_id = map_get_current_id();
    int dx, dy;
    int collision_result;
    DWORD current_time;

    switch (action) {
        case FIELD_ACTION_WALK:
            /* Action 1: Walk movement */
            handle_movement(0);
            break;

        case FIELD_ACTION_RUN:
            /* Action 2: Run (if map allows) */
            if (map_allows_run(map_id)) {
                handle_movement(1);
            }
            break;

        case FIELD_ACTION_BATTLE:
            /* Action 3: Battle encounter/menu */
            handle_encounter();
            break;

        case FIELD_ACTION_MENU:
            /* Action 4: Main menu */
            if (map_id == 0x7bda || is_special_map(map_id)) {
                /* Menu disabled on special maps */
                break;
            }
            g_menu_state_3c = 1;
            g_action_menu_state = 1;
            break;

        case FIELD_ACTION_SKILL:
            /* Action 5: Skill menu */
            g_menu_state_3e = 1;
            break;

        case FIELD_ACTION_WAIT:
            /* Action 6: Wait/stand */
            g_wait_counter++;
            if (g_wait_counter > 30) {
                g_wait_counter = 0;
            }
            break;

        case FIELD_ACTION_ATTACK:
            /* Action 7: Attack in direction */
            if (sub_action == FIELD_ACTION_ATTACK) {
                int direction = get_move_direction();
                if (direction >= 0) {
                    direction_to_offset(direction, &dx, &dy);
                    /* Send attack packet - FUN_0043b580 */
                    /* network_send_attack(player_x + dx, player_y + dy, 0); */
                }
            }
            break;

        case FIELD_ACTION_BATTLE_MENU:
            /* Action 8: Battle menu */
            if (g_battle_menu_state == 0) {
                fade_out();
                g_battle_menu_state = 1;
                g_action_menu_state = 1;
            } else {
                /* Already in menu, set delay */
                /* DAT_005ab7b4 = 1000 */
            }
            break;

        case FIELD_ACTION_ITEM:
            /* Action 9: Item menu */
            if (sub_action == FIELD_ACTION_ITEM) {
                int direction = get_move_direction();
                if (direction >= 0) {
                    direction_to_offset(direction, &dx, &dy);
                    /* FUN_0040e9b0 - collision check with range */
                    /* Send item use packet */
                }
            }
            break;

        case FIELD_ACTION_ESCAPE:
            /* Action 10: Escape/flee */
            if (sub_action == FIELD_ACTION_ESCAPE) {
                int direction = get_move_direction();
                if (direction >= 0) {
                    direction_to_offset(direction, &dx, &dy);
                    /* FUN_0040e8c0 - collision check 8-way */
                    current_time = timeGetTime();
                    if (g_last_keepalive_time + 500 < current_time) {
                        /* Send escape packet - FUN_0043bef0 */
                        g_last_keepalive_time = current_time;
                    }
                }
            }
            break;

        case FIELD_ACTION_SPECIAL:
            /* Action 11: Special action */
            if (sub_action == FIELD_ACTION_SPECIAL) {
                current_time = timeGetTime();
                if (g_last_keepalive_time + 500 < current_time) {
                    fade_process(FADE_WIPE_RIGHT);
                    /* Send special action packet - FUN_00490600 */
                    g_last_keepalive_time = current_time;
                }
            }
            break;

        case FIELD_ACTION_DEFEND:
            /* Action 12: Defend */
            break;

        case FIELD_ACTION_PET:
            /* Action 13: Pet action */
            if (sub_action == FIELD_ACTION_PET) {
                current_time = timeGetTime();
                if (g_last_keepalive_time + 500 < current_time) {
                    fade_process(FADE_WIPE_RIGHT);
                    /* Send pet action packet */
                    g_last_keepalive_time = current_time;
                }
            }
            break;

        default:
            break;
    }
}

/* ========================================
 * Main Update Function - FUN_00418370
 * ======================================== */

/*
 * Main field update - FUN_00418370
 * Called every frame during gameplay
 */
void field_update(void) {
    u16 map_id;
    int main_action, sub_action;
    int fade_result;
    DWORD current_time;

    /* Clear some state flags - FUN_00447340, FUN_0041ba30 */
    /* Update timers */

    /* Keepalive handling */
    if (g_keepalive_pending && !is_special_map(map_get_current_id())) {
        send_keepalive();
    }

    /* Check encounter state */
    if (check_encounter()) {
        g_encounter_state = 1;
    }

    /* Get map ID for special handling */
    map_id = map_get_current_id();

    /* Update movement state machine */
    g_move_state = (g_move_state + 0x340) & 0x3ff;
    if (g_move_state < 0) {
        g_move_state = (g_move_state - 1 | 0xfffffc00) + 1;
    }
    g_move_direction = (g_move_state + (g_move_state >> 7)) >> 8;
    g_move_step = (g_move_direction + 1) & 3;
    if (g_move_step < 0) {
        g_move_step = (g_move_step - 1 | 0xfffffffc) + 1;
    }

    /* Check for map-specific settings */
    if (is_special_map(map_id)) {
        /* Disable certain features on special maps */
    }

    /* Check for encounter-enabled maps */
    if (map_has_encounters(map_id)) {
        /* Enable random encounters */
    }

    /* Get current action from input state */
    main_action = input_get_main_action();
    sub_action = input_get_sub_action();

    /* Handle menu states */
    handle_menu_states();
    handle_battle_menu();

    /* Check for fade/transition state */
    if (g_action_menu_state) {
        /* In menu/transition mode */
        fade_process(FADE_WIPE_RIGHT);
        return;
    }

    /* Process main action */
    if (main_action >= 0) {
        process_action(main_action, sub_action);
    }

    /* Handle waiting states */
    if (g_battle_menu_state) {
        /* Battle menu active */
        if (g_battle_menu_state == 0) {
            g_wait_counter = 0;
        } else {
            current_time = timeGetTime();
            if (g_wait_counter == 0) {
                g_wait_counter = 1;
                g_last_keepalive_time = current_time;
            } else if (g_last_keepalive_time + 1000 < current_time) {
                g_wait_counter = (g_wait_counter - 1) & 1;
                g_last_keepalive_time = current_time;
            }
        }
    }

    /* Handle special action 0xD */
    if (main_action == FIELD_ACTION_PET && sub_action == FIELD_ACTION_PET) {
        current_time = timeGetTime();
        if (g_last_keepalive_time + 500 < current_time) {
            fade_process(FADE_WIPE_RIGHT);
            network_send_pet_action();
            g_last_keepalive_time = current_time;
        }
    }

    /* Update frame state */
    g_player_action = main_action;
}

/*
 * Initialize field update state
 */
void field_update_init(void) {
    g_last_keepalive_time = timeGetTime();
    g_keepalive_pending = 0;
    g_connection_state = 0;
    g_encounter_state = 0;
    g_move_state = 0;
    g_move_direction = 0;
    g_move_step = 0;
    g_menu_state_3c = 0;
    g_menu_state_3e = 0;
    g_menu_state_38 = 0;
    g_battle_menu_state = 0;
    g_action_menu_state = 0;
    g_player_action = 0;
    g_wait_counter = 0;

    LOG_INFO("Field update initialized");
}

/*
 * Shutdown field update
 */
void field_update_shutdown(void) {
    LOG_INFO("Field update shutdown");
}

/*
 * Get current action menu state
 */
int field_get_menu_state(void) {
    return g_action_menu_state;
}

/*
 * Set encounter state (for testing or scripted encounters)
 */
void field_set_encounter(int state) {
    g_encounter_state = state;
}
