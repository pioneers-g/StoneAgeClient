/*
 * Stone Age Client - Game Loop Manager Header
 * Reverse engineered from sa_9061.exe FUN_0041db40
 * Main game loop that coordinates all systems
 */

#ifndef GAMELOOP_H
#define GAMELOOP_H

#include "types.h"

/* Frame timing constants */
#define TARGET_FPS          60
#define FRAME_TIME_MS       (1000 / TARGET_FPS)
#define MAX_FRAME_SKIP      5

/* Anti-tamper interval - 2 minutes */
#define ANTITAMPER_INTERVAL_MS  120000

/* Game loop modes */
typedef enum {
    GAMELOOP_MODE_NORMAL = 0,
    GAMELOOP_MODE_BATTLE = 1,
    GAMELOOP_MODE_DIALOG = 2,
    GAMELOOP_MODE_MENU = 3,
    GAMELOOP_MODE_LOADING = 4
} GameLoopMode;

/* Game loop statistics */
typedef struct {
    u32 fps;
    u32 frame_time_ms;
    u32 update_time_ms;
    u32 render_time_ms;
    u32 frames_skipped;
    u32 total_frames;
    u64 total_time_ms;
    float cpu_usage;
} GameLoopStats;

/* Game loop context - matches data from FUN_0041db40 */
typedef struct {
    /* Running state */
    int running;
    int paused;
    int minimized;

    /* Timing */
    u32 frame_start;
    u32 frame_end;
    u32 last_frame_time;
    u32 accumulator;
    u32 timing_seed;           /* XOR'd with 0xffffbcde for frame timing */

    /* Mode */
    GameLoopMode mode;

    /* State flags - from FUN_0041db40 DAT_005ab724 */
    u32 state_flags;
    int fullscreen_toggle;

    /* Statistics */
    GameLoopStats stats;

    /* Callbacks */
    void (*on_update)(u32 delta_time);
    void (*on_render)(void);
    void (*on_input)(u32 msg, u32 wparam, u32 lparam);

} GameLoopContext;

/* Global game loop */
extern GameLoopContext g_gameloop;

/* Initialization */
int gameloop_init(void);
void gameloop_shutdown(void);

/* Main loop */
int gameloop_run(void);
void gameloop_stop(void);

/* Control */
void gameloop_pause(void);
void gameloop_resume(void);
void gameloop_set_mode(GameLoopMode mode);

/* Callbacks */
void gameloop_set_update_callback(void (*callback)(u32));
void gameloop_set_render_callback(void (*callback)(void));
void gameloop_set_input_callback(void (*callback)(u32, u32, u32));

/* Frame processing */
void gameloop_limit_fps(u32 frame_start);

/* Timing */
u32 gameloop_get_frame_time(void);
u32 gameloop_get_fps(void);
u32 gameloop_get_timing_seed(void);

/* Statistics */
void gameloop_get_stats(GameLoopStats* stats);
void gameloop_reset_stats(void);

/* Utilities */
int gameloop_is_running(void);
int gameloop_is_paused(void);

#endif /* GAMELOOP_H */
