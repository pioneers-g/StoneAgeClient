/*
 * Stone Age Client - Game Module Header
 * Reverse engineered from sa_9061.exe (FUN_0041db40)
 */

#ifndef GAME_H
#define GAME_H

#include "types.h"
#include "gamestate.h"

/* Direction constants */
#define DIRECTION_UP        0
#define DIRECTION_DOWN      1
#define DIRECTION_LEFT      2
#define DIRECTION_RIGHT     3

/* Game context - matches DAT_04630dd8 region */
typedef struct {
    /* State machine - DAT_04630dd8, DAT_04630df8, DAT_04630df0 */
    GameState state;              /* Current state */
    GameState next_state;         /* Pending state (set to -1 when no change) */
    int state_frame;              /* Frame counter within state */
    int render_mode;              /* DAT_005ab6fc: 0=normal, 2=battle */

    /* Running flag */
    int is_running;

    /* Timing */
    DWORD frame_time;
    DWORD last_frame_time;
    DWORD frame_start_time;

    /* FPS tracking */
    int fps;
    DWORD fps_time;
    int frame_count;

    /* Game data */
    u32 current_map_id;
    u32 player_id;

    /* Battle state */
    int battle_encounter;
    int battle_ended;

    /* Reserved */
    u32 reserved[8];
} GameContext;

/* Global game context */
extern GameContext g_game;

/* Game functions - FUN_0041e260, FUN_0041db40 */
int game_init(void);
void game_run(void);
void game_shutdown(void);

/* Main game loop */
void game_main_loop(void);

/* Per-frame updates */
void game_update(void);
void game_render(void);
void game_handle_input(void);

/* State machine - FUN_00479bc0, FUN_00479c40 */
void game_set_state(GameState state);
GameState game_get_state(void);
void game_process_state(void);  /* FUN_00479c40 - main state processor */

/* Fade check - FUN_0047bde0 pattern */
int game_fade_check(int fade_id);

#endif /* GAME_H */
