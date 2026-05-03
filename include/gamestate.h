/*
 * Stone Age Client - Game State Manager Header
 * Reverse engineered from sa_9061.exe (FUN_00479bc0, FUN_00479c40)
 *
 * State variables from original binary:
 * - DAT_04630dd8: Main game state
 * - DAT_04630df0: Sub-state (frame state)
 * - DAT_005ab6fc: Render mode
 */

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "types.h"

/* Game states - matches DAT_04630dd8 values from FUN_004799b0 */
typedef enum {
    GAME_STATE_NONE = -1,        /* No state (pending) */
    GAME_STATE_INIT = 0,         /* State 0 - Initialization */
    GAME_STATE_LOGIN = 1,        /* State 1 - Login screen */
    GAME_STATE_CHAR_SELECT = 2,  /* State 2 - Character selection */
    GAME_STATE_CHAR_WAIT = 3,    /* State 3 - Waiting for game entry */
    GAME_STATE_CHAR_CREATE = 4,  /* State 4 - Character creation */
    GAME_STATE_FIELD = 5,        /* State 5 - Main game field */
    GAME_STATE_BATTLE_INIT = 6,  /* State 6 - Battle initialization */
    GAME_STATE_BATTLE_READY = 7, /* State 7 - Battle ready check */
    GAME_STATE_MENU = 8,         /* State 8 - Menu/password screen */
    GAME_STATE_PRELOAD = 9,      /* State 9 - Pre-field setup */
    GAME_STATE_FIELD_INIT = 10,  /* State 10 - Field initialization */
    GAME_STATE_PLAYING = 11,     /* State 11 - Field gameplay */
    GAME_STATE_BATTLE = 12,      /* State 12 - Battle active */
    GAME_STATE_MAP_TRANSITION = 13, /* State 13 - Map transition/loading */
    GAME_STATE_SHUTDOWN = 14,    /* Exit state */
    GAME_STATE_DISCONNECTED = 15,  /* Connection lost state */

    /* Additional states from FUN_00479c40 - specific numeric values */
    GAME_STATE_FADE_IN = 0x96,      /* State 150: Fade in transition */
    GAME_STATE_LOGIN_INIT = 100,    /* State 100 (0x64): Login screen init */
    GAME_STATE_LOGIN_WAIT = 0x66,   /* State 102: Login processing */
    GAME_STATE_LOGIN_DONE = 0x67,   /* State 103: Login complete */
    GAME_STATE_LOGOUT = 0x14,       /* State 20: Logout */
    GAME_STATE_MAP_CHANGE = 200,    /* State 200 (0xC8): Map change */
    GAME_STATE_MAP_LOAD = 0xC9,     /* State 201: Map loading */
    GAME_STATE_MAP_WAIT = 0xCA,     /* State 202: Wait for data */
    GAME_STATE_MAP_ENTER = 0xCB,    /* State 203: Enter map */
    GAME_STATE_MAP_DONE = 0xCC,     /* State 204: Map enter done */

    /* Special value */
    GAME_STATE_EXIT = 0xFF
} GameState;

/* Render modes - matches DAT_005ab6fc */
typedef enum {
    RENDER_MODE_NORMAL = 0,
    RENDER_MODE_LOADING = 2,
    RENDER_MODE_BATTLE = 3,
    RENDER_MODE_BATTLE_INIT = 4
} RenderMode;

/* Constants */
#define MAX_STATE_STACK 8

/* State timing */
#define STATE_TRANSITION_DELAY 500

/* Game state context - matches DAT_04630dd8 region structure */
typedef struct {
    /* Current state - DAT_04630dd8 */
    GameState current_state;
    GameState next_state;

    /* Sub-state - DAT_04630df0 */
    int sub_state;

    /* State flags - DAT_04630de4 */
    int state_flags;

    /* Transition state */
    int transitioning;
    u32 state_start_time;

    /* Render mode - DAT_005ab6fc */
    int render_mode;

    /* State stack (for overlays) */
    GameState state_stack[MAX_STATE_STACK];
    int stack_depth;

    /* Pause state */
    int paused;
    u32 pause_time;

    /* State-specific data */
    void* state_data;

    /* Loading progress */
    int loading_progress;

    /* Battle result */
    int battle_result;

    /* Server time - set by PKT_SV_SERVER_TIME (0x86) */
    u16 server_year;
    u8 server_month;
    u8 server_day;
    u8 server_hour;
    u8 server_minute;

    /* Reserved for future use */
    int reserved[4];

} GameStateContext;

/* Global game state */
extern GameStateContext g_gamestate;

/* Initialization */
int gamestate_init(void);
void gamestate_shutdown(void);

/* State changes - FUN_00479bc0 */
void gamestate_set(GameState state);
void gamestate_set_state(GameState state, int param);
int gamestate_change(GameState new_state);
void gamestate_request_change(GameState new_state);
void gamestate_process_transition(void);

/* Enter/exit handlers */
void gamestate_enter(GameState state);
void gamestate_exit(GameState state);

/* Main loop - FUN_004799b0 */
void gamestate_dispatch(void);
void gamestate_update(u32 delta_time);
void gamestate_render(void);
void gamestate_handle_input(u32 msg, u32 wparam, u32 lparam);

/* State queries */
GameState gamestate_get_current(void);
int gamestate_get_sub_state(void);
void gamestate_set_sub_state(int sub_state);
int gamestate_is_state(GameState state);
int gamestate_is_battle(void);
int gamestate_is_playing(void);
int gamestate_is_transitioning(void);
int gamestate_is_paused(void);

/* Render mode */
int gamestate_get_render_mode(void);
void gamestate_set_render_mode(int mode);

/* State stack */
void gamestate_push(GameState state);
void gamestate_pop(void);
int gamestate_get_stack_depth(void);

/* Pause/resume */
void gamestate_pause(void);
void gamestate_resume(void);

/* State data */
void gamestate_set_data(void* data);
void* gamestate_get_data(void);

/* Loading */
void gamestate_set_loading_progress(int progress);
int gamestate_get_loading_progress(void);

/* Battle */
void gamestate_set_battle_result(int result);
int gamestate_get_battle_result(void);

/* Utilities */
u32 gamestate_get_state_time(void);
const char* gamestate_to_string(GameState state);

/* Exit handling */
int gamestate_can_exit(void);
void gamestate_request_exit_to_menu(void);
void gamestate_request_logout(void);

#endif /* GAMESTATE_H */
