/*
 * Stone Age Client - Game State Manager Core
 * Reverse engineered from sa_9061.exe (FUN_00479bc0, FUN_00479c40, FUN_004799b0)
 *
 * State variables from original binary:
 * - DAT_04630dd8: Main game state
 * - DAT_04630df0: Sub-state (frame state within main state)
 * - DAT_04630de4: Additional state flag (char data ready)
 * - DAT_005ab6fc: Render mode (0=normal, 2=loading, 3=battle)
 *
 * Split modules:
 * - gamestate_states.c: State update functions
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

/* Global game state context - matches DAT_04630dd8 region */
GameStateContext g_gamestate = {0};

/* State timing variables - matches DAT_005ab7xx region */
static DWORD g_state_start_time = 0;
static DWORD g_last_update_time = 0;
static DWORD g_frame_counter = 0;

/* Render mode - matches DAT_005ab6fc */
static int s_render_mode = 0;

/* Forward declarations for state update functions (in gamestate_states.c) */
extern void update_state_init(void);
extern void update_state_login(void);
extern void update_state_char_select(void);
extern void update_state_char_create(void);
extern void update_state_menu(void);
extern void update_state_preload(void);
extern void update_state_field_init(void);
extern void update_state_playing(void);
extern void update_state_battle_init(void);
extern void update_state_battle(void);
extern void update_state_map_transition(void);

/*
 * Initialize game state manager - FUN_00479bc0 pattern
 */
int gamestate_init(void) {
    memset(&g_gamestate, 0, sizeof(GameStateContext));

    g_gamestate.current_state = GAME_STATE_INIT;
    g_gamestate.next_state = (GameState)-1;
    g_gamestate.sub_state = 0;
    g_gamestate.transitioning = 0;
    g_gamestate.state_start_time = timeGetTime();
    g_gamestate.render_mode = 0;

    g_state_start_time = timeGetTime();
    g_last_update_time = g_state_start_time;
    g_frame_counter = 0;
    s_render_mode = 0;

    LOG_INFO("Game state manager initialized");
    return 1;
}

/*
 * Shutdown game state manager
 */
void gamestate_shutdown(void) {
    if (g_gamestate.current_state != GAME_STATE_INIT) {
        gamestate_exit(g_gamestate.current_state);
    }

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    LOG_INFO("Game state manager shutdown");
}

/*
 * Set game state - FUN_00479bc0 exact pattern
 */
void gamestate_set(GameState state) {
    g_gamestate.sub_state = 0;
    g_gamestate.current_state = state;
    g_gamestate.state_start_time = timeGetTime();

    LOG_DEBUG("Game state set to: %d (%s)", state, gamestate_to_string(state));
}

/*
 * Set game state with parameter
 */
void gamestate_set_state(GameState state, int param) {
    g_gamestate.sub_state = param;
    g_gamestate.current_state = state;
    g_gamestate.state_start_time = timeGetTime();

    LOG_DEBUG("Game state set to: %d (%s) with param %d", state, gamestate_to_string(state), param);
}

/*
 * Change game state
 */
int gamestate_change(GameState new_state) {
    if (new_state == g_gamestate.current_state) {
        return 0;
    }

    LOG_INFO("Game state changing: %d -> %d", g_gamestate.current_state, new_state);

    gamestate_exit(g_gamestate.current_state);

    g_gamestate.current_state = new_state;
    g_gamestate.sub_state = 0;
    g_gamestate.state_start_time = timeGetTime();
    g_gamestate.transitioning = 0;

    gamestate_enter(new_state);

    return 1;
}

/*
 * Request state change (deferred)
 */
void gamestate_request_change(GameState new_state) {
    g_gamestate.next_state = new_state;
    g_gamestate.transitioning = 1;
}

/*
 * Process pending state change
 */
void gamestate_process_transition(void) {
    if (g_gamestate.transitioning && g_gamestate.next_state >= 0) {
        gamestate_change(g_gamestate.next_state);
        g_gamestate.next_state = (GameState)-1;
        g_gamestate.transitioning = 0;
    }
}

/*
 * Enter game state
 */
void gamestate_enter(GameState state) {
    g_gamestate.state_start_time = timeGetTime();

    switch (state) {
        case GAME_STATE_INIT:
            LOG_DEBUG("Entering INIT state");
            g_gamestate.render_mode = RENDER_MODE_NORMAL;
            break;

        case GAME_STATE_LOGIN:
            LOG_DEBUG("Entering LOGIN state");
            g_gamestate.render_mode = RENDER_MODE_NORMAL;
            break;

        case GAME_STATE_CHAR_SELECT:
            LOG_DEBUG("Entering CHARACTER SELECT state");
            g_gamestate.render_mode = RENDER_MODE_NORMAL;
            break;

        case GAME_STATE_CHAR_CREATE:
            LOG_DEBUG("Entering CHARACTER CREATE state");
            g_gamestate.render_mode = RENDER_MODE_NORMAL;
            break;

        case GAME_STATE_MENU:
            LOG_DEBUG("Entering MENU state");
            g_gamestate.render_mode = RENDER_MODE_NORMAL;
            break;

        case GAME_STATE_PRELOAD:
            LOG_DEBUG("Entering PRELOAD state");
            g_gamestate.render_mode = RENDER_MODE_NORMAL;
            break;

        case GAME_STATE_FIELD_INIT:
            LOG_DEBUG("Entering FIELD_INIT state");
            g_gamestate.render_mode = RENDER_MODE_NORMAL;
            break;

        case GAME_STATE_PLAYING:
            LOG_DEBUG("Entering PLAYING state");
            g_gamestate.render_mode = RENDER_MODE_NORMAL;
            break;

        case GAME_STATE_BATTLE_INIT:
            LOG_DEBUG("Entering BATTLE_INIT state");
            g_gamestate.render_mode = RENDER_MODE_BATTLE_INIT;
            break;

        case GAME_STATE_BATTLE:
            LOG_DEBUG("Entering BATTLE state");
            g_gamestate.render_mode = RENDER_MODE_BATTLE;
            break;

        case GAME_STATE_MAP_TRANSITION:
            LOG_DEBUG("Entering MAP_TRANSITION state");
            g_gamestate.render_mode = RENDER_MODE_LOADING;
            break;

        default:
            break;
    }
}

/*
 * Exit game state
 */
void gamestate_exit(GameState state) {
    switch (state) {
        case GAME_STATE_INIT:
            LOG_DEBUG("Exiting INIT state");
            break;

        case GAME_STATE_LOGIN:
            LOG_DEBUG("Exiting LOGIN state");
            break;

        case GAME_STATE_CHAR_SELECT:
            LOG_DEBUG("Exiting CHARACTER SELECT state");
            break;

        case GAME_STATE_CHAR_CREATE:
            LOG_DEBUG("Exiting CHARACTER CREATE state");
            break;

        case GAME_STATE_MENU:
            LOG_DEBUG("Exiting MENU state");
            break;

        case GAME_STATE_PRELOAD:
            LOG_DEBUG("Exiting PRELOAD state");
            break;

        case GAME_STATE_FIELD_INIT:
            LOG_DEBUG("Exiting FIELD_INIT state");
            break;

        case GAME_STATE_PLAYING:
            LOG_DEBUG("Exiting PLAYING state");
            break;

        case GAME_STATE_BATTLE_INIT:
            LOG_DEBUG("Exiting BATTLE_INIT state");
            break;

        case GAME_STATE_BATTLE:
            LOG_DEBUG("Exiting BATTLE state");
            break;

        case GAME_STATE_MAP_TRANSITION:
            LOG_DEBUG("Exiting MAP_TRANSITION state");
            break;

        default:
            break;
    }
}

/*
 * Dispatch game state - FUN_004799b0 exact pattern
 */
void gamestate_dispatch(void) {
    if (g_gamestate.next_state != (GameState)-1) {
        g_gamestate.current_state = g_gamestate.next_state;
        g_gamestate.next_state = (GameState)-1;
        g_gamestate.sub_state = 0;
    }

    g_frame_counter++;

    switch (g_gamestate.current_state) {
        case GAME_STATE_INIT:
            update_state_init();
            break;

        case GAME_STATE_LOGIN:
            s_render_mode = 0;
            update_state_login();
            break;

        case GAME_STATE_CHAR_SELECT:
            s_render_mode = 0;
            update_state_char_select();
            break;

        case GAME_STATE_CHAR_CREATE:
            s_render_mode = 0;
            update_state_char_create();
            break;

        case GAME_STATE_MENU:
            s_render_mode = 0;
            update_state_menu();
            break;

        case GAME_STATE_PRELOAD:
            s_render_mode = 0;
            update_state_preload();
            break;

        case GAME_STATE_FIELD_INIT:
            s_render_mode = 0;
            update_state_field_init();
            break;

        case GAME_STATE_PLAYING:
            update_state_playing();
            break;

        case GAME_STATE_BATTLE_INIT:
            update_state_battle_init();
            break;

        case GAME_STATE_BATTLE:
            update_state_battle();
            break;

        case GAME_STATE_MAP_TRANSITION:
            update_state_map_transition();
            break;

        default:
            break;
    }

    g_last_update_time = timeGetTime();
}

/*
 * Update current game state
 */
void gamestate_update(u32 delta_time) {
    (void)delta_time;
    gamestate_dispatch();
}

/*
 * Render current game state
 */
void gamestate_render(void) {
    switch (g_gamestate.current_state) {
        case GAME_STATE_INIT:
        case GAME_STATE_LOGIN:
        case GAME_STATE_CHAR_SELECT:
        case GAME_STATE_CHAR_CREATE:
        case GAME_STATE_MENU:
        case GAME_STATE_PRELOAD:
        case GAME_STATE_FIELD_INIT:
        case GAME_STATE_PLAYING:
        case GAME_STATE_BATTLE_INIT:
        case GAME_STATE_BATTLE:
        case GAME_STATE_MAP_TRANSITION:
            /* Render handled by specific modules */
            break;

        default:
            break;
    }
}

/*
 * Handle input for current state
 */
void gamestate_handle_input(u32 msg, u32 wparam, u32 lparam) {
    switch (g_gamestate.current_state) {
        case GAME_STATE_LOGIN:
        case GAME_STATE_CHAR_SELECT:
        case GAME_STATE_CHAR_CREATE:
        case GAME_STATE_MENU:
        case GAME_STATE_PLAYING:
        case GAME_STATE_BATTLE:
        case GAME_STATE_MAP_TRANSITION:
            /* Input handled by specific modules */
            break;

        default:
            break;
    }
}

/*
 * Get current game state
 */
GameState gamestate_get_current(void) {
    return g_gamestate.current_state;
}

/*
 * Get sub-state (frame state)
 */
int gamestate_get_sub_state(void) {
    return g_gamestate.sub_state;
}

/*
 * Set sub-state
 */
void gamestate_set_sub_state(int sub_state) {
    g_gamestate.sub_state = sub_state;
}

/*
 * Check if in specific state
 */
int gamestate_is_state(GameState state) {
    return g_gamestate.current_state == state;
}

/*
 * Check if in battle
 */
int gamestate_is_battle(void) {
    return g_gamestate.current_state == GAME_STATE_BATTLE ||
           g_gamestate.current_state == GAME_STATE_BATTLE_INIT;
}

/*
 * Check if in playing state
 */
int gamestate_is_playing(void) {
    return g_gamestate.current_state == GAME_STATE_PLAYING;
}

/*
 * Check if transitioning
 */
int gamestate_is_transitioning(void) {
    return g_gamestate.transitioning;
}

/*
 * Get time in current state
 */
u32 gamestate_get_state_time(void) {
    return timeGetTime() - g_gamestate.state_start_time;
}

/*
 * Get render mode - matches DAT_005ab6fc
 */
int gamestate_get_render_mode(void) {
    return g_gamestate.render_mode;
}

/*
 * Set render mode
 */
void gamestate_set_render_mode(int mode) {
    g_gamestate.render_mode = mode;
}

/*
 * Push game state (for overlays)
 */
void gamestate_push(GameState state) {
    if (g_gamestate.stack_depth >= MAX_STATE_STACK) {
        LOG_WARN("State stack overflow");
        return;
    }

    g_gamestate.state_stack[g_gamestate.stack_depth] = g_gamestate.current_state;
    g_gamestate.stack_depth++;

    gamestate_change(state);
}

/*
 * Pop game state
 */
void gamestate_pop(void) {
    if (g_gamestate.stack_depth <= 0) {
        LOG_WARN("State stack underflow");
        return;
    }

    g_gamestate.stack_depth--;
    GameState prev_state = g_gamestate.state_stack[g_gamestate.stack_depth];

    gamestate_change(prev_state);
}

/*
 * Get stack depth
 */
int gamestate_get_stack_depth(void) {
    return g_gamestate.stack_depth;
}

/*
 * Pause game state
 */
void gamestate_pause(void) {
    g_gamestate.paused = 1;
    g_gamestate.pause_time = timeGetTime();
}

/*
 * Resume game state
 */
void gamestate_resume(void) {
    g_gamestate.paused = 0;
}

/*
 * Check if paused
 */
int gamestate_is_paused(void) {
    return g_gamestate.paused;
}

/*
 * Set state data
 */
void gamestate_set_data(void* data) {
    g_gamestate.state_data = data;
}

/*
 * Get state data
 */
void* gamestate_get_data(void) {
    return g_gamestate.state_data;
}

/*
 * Game state to string
 */
const char* gamestate_to_string(GameState state) {
    static const char* state_names[] = {
        "Init",              /* State 0 */
        "Login",             /* State 1 */
        "Character Select",  /* State 2 */
        "Character Create",  /* State 3 */
        "Menu",              /* State 4 */
        "Preload",           /* State 5 */
        "Field Init",        /* State 6 */
        "Playing",           /* State 7 */
        "Unused 8",          /* State 8 */
        "Battle Init",       /* State 9 */
        "Battle",            /* State 10 */
        "Map Transition",    /* State 11 */
        "Shutdown"           /* State 12 */
    };

    if (state >= 0 && state < (int)(sizeof(state_names) / sizeof(state_names[0]))) {
        return state_names[state];
    }

    return "Unknown";
}

/*
 * Can exit current state
 */
int gamestate_can_exit(void) {
    switch (g_gamestate.current_state) {
        case GAME_STATE_BATTLE:
        case GAME_STATE_BATTLE_INIT:
            return 0;

        default:
            return 1;
    }
}

/*
 * Request exit to menu
 */
void gamestate_request_exit_to_menu(void) {
    if (gamestate_can_exit()) {
        gamestate_request_change(GAME_STATE_MENU);
    }
}

/*
 * Request logout
 */
void gamestate_request_logout(void) {
    if (gamestate_can_exit()) {
        gamestate_request_change(GAME_STATE_LOGIN);
    }
}

/*
 * Set loading progress
 */
void gamestate_set_loading_progress(int progress) {
    g_gamestate.loading_progress = progress;
}

/*
 * Get loading progress
 */
int gamestate_get_loading_progress(void) {
    return g_gamestate.loading_progress;
}

/*
 * Set battle result
 */
void gamestate_set_battle_result(int result) {
    g_gamestate.battle_result = result;
}

/*
 * Get battle result
 */
int gamestate_get_battle_result(void) {
    return g_gamestate.battle_result;
}

/*
 * Set server time from server
 */
void game_set_server_time(u16 year, u16 month, u16 day, u16 hour) {
    g_gamestate.server_year = year;
    g_gamestate.server_month = (u8)month;
    g_gamestate.server_day = (u8)day;
    g_gamestate.server_hour = (u8)hour;

    LOG_DEBUG("Server time: %u/%u/%u %u:00", year, month, day, hour);
}
