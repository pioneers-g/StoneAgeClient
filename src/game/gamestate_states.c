/*
 * Stone Age Client - Game State Update Functions
 * Split from gamestate.c for code organization
 *
 * Contains individual state update functions
 * Reverse engineered from FUN_00479c40, FUN_004799b0
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "gamestate.h"
#include "fade.h"
#include "battle.h"
#include "field_entity.h"
#include "logger.h"

/* Sub-state values from FUN_00479c40 binary analysis */
#define SUBSTATE_INIT           0
#define SUBSTATE_FADE_PREP      0x96      /* 150 - fade preparation */
#define SUBSTATE_UI_INIT        100       /* 0x64 - UI initialization */
#define SUBSTATE_WAIT_FLAG      0x65      /* 101 - wait for flag */
#define SUBSTATE_MAP_LOAD       0x66      /* 102 - map loading */
#define SUBSTATE_FADE_IN        0x67      /* 103 - fade in complete */
#define SUBSTATE_FIELD_ACTIVE   3
#define SUBSTATE_LOGOUT         0x14      /* 20 - logout */
#define SUBSTATE_MAP_CHANGE     200       /* 0xC8 - map change start */
#define SUBSTATE_MAP_FADE       0xC9      /* 201 - map fade */
#define SUBSTATE_COORD_CHECK    0xCA      /* 202 - coordinate check */
#define SUBSTATE_MAP_SETUP      0xCB      /* 203 - map setup */
#define SUBSTATE_MAP_COMPLETE   0xCC      /* 204 - map change complete */

/* External reference to global game state */
extern GameStateContext g_gamestate;

/* Static variables */
static int s_char_data_ready = 0;

/*
 * State 0: Init state update
 */
void update_state_init(void) {
    switch (g_gamestate.sub_state) {
        case 0:
            g_gamestate.sub_state = 1;
            break;

        case 1:
            g_gamestate.sub_state = 2;
            break;

        case 2:
            gamestate_set(GAME_STATE_LOGIN);
            break;

        default:
            break;
    }
}

/*
 * State 1: Login state update - FUN_00420590 pattern
 */
void update_state_login(void) {
    switch (g_gamestate.sub_state) {
        case 0:
            g_gamestate.sub_state = 1;
            break;

        case 1:
            g_gamestate.sub_state = 2;
            break;

        case 2:
            /* Waiting for server response */
            break;

        case 3:
            g_gamestate.sub_state = 4;
            break;

        case 100:
            g_gamestate.sub_state = 101;
            break;

        case 101:
            /* Retry dialog */
            break;

        default:
            break;
    }
}

/*
 * State 2: Character select update - FUN_00421110 pattern
 */
void update_state_char_select(void) {
    switch (g_gamestate.sub_state) {
        case 0:
            g_gamestate.sub_state = 1;
            break;

        case 1:
            g_gamestate.sub_state = 2;
            break;

        case 2:
            /* Character selection UI */
            break;

        case 3:
            /* Wait for character enter */
            break;

        case 4:
            g_gamestate.sub_state = 5;
            break;

        case 5:
            /* Enter game */
            break;

        case 100:
            /* Error state */
            break;

        case 200:
            gamestate_set(GAME_STATE_LOGIN);
            break;

        default:
            break;
    }
}

/*
 * State 3: Character create update - FUN_00421c00 pattern
 */
void update_state_char_create(void) {
    switch (g_gamestate.sub_state) {
        case 0:
            g_gamestate.sub_state = 1;
            break;

        case 1:
            /* Handle character creation input */
            break;

        default:
            break;
    }
}

/*
 * State 4: Menu update - FUN_00422aa0 pattern
 */
void update_state_menu(void) {
    switch (g_gamestate.sub_state) {
        case 0:
            g_gamestate.sub_state = 1;
            break;

        case 1:
            /* Process menu input */
            break;

        default:
            break;
    }
}

/*
 * State 5: Preload update - FUN_00440280 pattern
 */
void update_state_preload(void) {
    switch (g_gamestate.sub_state) {
        case 0:
            g_gamestate.sub_state = 1;
            break;

        case 1:
            g_gamestate.sub_state = 2;
            break;

        case 2:
            gamestate_set(GAME_STATE_FIELD_INIT);
            break;

        default:
            break;
    }
}

/*
 * State 6: Field init update - FUN_00424610 pattern
 */
void update_state_field_init(void) {
    switch (g_gamestate.sub_state) {
        case 0:
            g_gamestate.sub_state = 1;
            break;

        case 1:
            gamestate_set(GAME_STATE_PLAYING);
            break;

        default:
            break;
    }
}

/*
 * State 7: Playing update - FUN_00424880 pattern
 */
void update_state_playing(void) {
    switch (g_gamestate.sub_state) {
        case 0:
            g_gamestate.sub_state = 1;
            break;

        case 1:
            g_gamestate.sub_state = 2;
            break;

        case 2:
            /* Main game loop */
            break;

        case 100:
            g_gamestate.sub_state = 101;
            break;

        case 101:
            /* Wait for error dialog */
            break;

        default:
            break;
    }
}

/*
 * State 9: Battle init update - FUN_00479c40 pattern
 */
void update_state_battle_init(void) {
    int fade_result;

    switch (g_gamestate.sub_state) {
        case SUBSTATE_INIT:
            g_gamestate.sub_state = SUBSTATE_FADE_PREP;
            break;

        case SUBSTATE_FADE_PREP:
            g_gamestate.sub_state = SUBSTATE_UI_INIT;
            break;

        case SUBSTATE_UI_INIT:
            field_ui_init();
            battle_init_ui_state();
            g_gamestate.sub_state = SUBSTATE_WAIT_FLAG;
            break;

        case SUBSTATE_WAIT_FLAG:
            g_gamestate.render_mode = RENDER_MODE_NORMAL;
            g_gamestate.sub_state = SUBSTATE_MAP_LOAD;
            break;

        case SUBSTATE_MAP_LOAD:
            g_gamestate.render_mode = RENDER_MODE_LOADING;
            fade_result = fade_process(FADE_WIPE_RIGHT);

            if (fade_result) {
                s_char_data_ready = 1;
                g_gamestate.sub_state = SUBSTATE_FADE_IN;
            }
            break;

        case SUBSTATE_FADE_IN:
            g_gamestate.render_mode = RENDER_MODE_NORMAL;
            gamestate_set(GAME_STATE_PLAYING);
            break;

        case 1:
            if (s_char_data_ready) {
                g_gamestate.render_mode = RENDER_MODE_LOADING;
                fade_result = fade_process(FADE_WIPE_RIGHT);

                if (fade_result) {
                    g_gamestate.sub_state = 2;
                }
            } else {
                s_char_data_ready = 1;
                g_gamestate.sub_state = 3;
            }
            break;

        case 2:
            fade_result = fade_process(FADE_WIPE_RIGHT);
            if (fade_result) {
                g_gamestate.sub_state = 3;
            }
            break;

        case 3:
            g_gamestate.render_mode = RENDER_MODE_NORMAL;
            break;

        case 4:
            g_gamestate.render_mode = RENDER_MODE_LOADING;
            g_gamestate.sub_state = 5;
            break;

        case 5:
            fade_result = fade_process(FADE_WIPE_LEFT);
            if (fade_result) {
                gamestate_set(GAME_STATE_BATTLE);
            }
            break;

        case SUBSTATE_LOGOUT:
            g_gamestate.render_mode = RENDER_MODE_NORMAL;
            gamestate_set(GAME_STATE_LOGIN);
            break;

        case SUBSTATE_MAP_CHANGE:
            g_gamestate.render_mode = RENDER_MODE_LOADING;
            g_gamestate.sub_state = SUBSTATE_MAP_FADE;
            break;

        case SUBSTATE_MAP_FADE:
            g_gamestate.render_mode = RENDER_MODE_LOADING;
            fade_result = fade_process(FADE_BLIND);
            if (fade_result) {
                g_gamestate.sub_state = SUBSTATE_COORD_CHECK;
            }
            break;

        case SUBSTATE_COORD_CHECK:
            g_gamestate.sub_state = SUBSTATE_MAP_SETUP;
            break;

        case SUBSTATE_MAP_SETUP:
            g_gamestate.render_mode = RENDER_MODE_LOADING;
            g_gamestate.sub_state = SUBSTATE_MAP_COMPLETE;
            break;

        case SUBSTATE_MAP_COMPLETE:
            g_gamestate.render_mode = RENDER_MODE_LOADING;
            fade_result = fade_process(FADE_BLIND_REVERSE);
            if (fade_result) {
                gamestate_set(GAME_STATE_PLAYING);
            }
            break;

        default:
            break;
    }
}

/*
 * State 10: Battle active update - FUN_0040a1a0 pattern
 */
void update_state_battle(void) {
    switch (g_gamestate.sub_state) {
        case 0:
            g_gamestate.render_mode = RENDER_MODE_BATTLE;
            g_gamestate.sub_state = 1;
            break;

        case 1:
            g_gamestate.render_mode = RENDER_MODE_BATTLE;
            g_gamestate.sub_state = 2;
            break;

        case 2:
            /* Main battle update */
            break;

        case 3:
            g_gamestate.sub_state = 4;
            break;

        case 4:
            /* Action processing */
            break;

        case 5:
            /* Result processing */
            break;

        case 6:
            /* Text display */
            break;

        case 7:
            g_gamestate.render_mode = RENDER_MODE_LOADING;
            g_gamestate.sub_state = 8;
            break;

        case 8:
            gamestate_set(GAME_STATE_FIELD_INIT);
            break;

        default:
            break;
    }
}

/*
 * State 11: Map transition update
 */
void update_state_map_transition(void) {
    switch (g_gamestate.sub_state) {
        case 0:
            g_gamestate.render_mode = RENDER_MODE_LOADING;
            g_gamestate.sub_state = 1;
            break;

        case 1:
            g_gamestate.render_mode = RENDER_MODE_NORMAL;
            g_gamestate.sub_state = 2;
            break;

        case 2:
            if (g_gamestate.battle_result == -1) {
                gamestate_set(GAME_STATE_LOGIN);
            } else {
                gamestate_set(GAME_STATE_PLAYING);
            }
            break;

        default:
            break;
    }
}
