/*
 * Stone Age Client - Game Module Implementation
 * Reverse engineered from sa_9061.exe (FUN_0041db40, FUN_0041e260)
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "config.h"
#include "game.h"
#include "login.h"
#include "network.h"
#include "directx.h"
#include "assets.h"
#include "input.h"
#include "render.h"
#include "map.h"
#include "character.h"
#include "ui.h"
#include "battle.h"
#include "petai.h"
#include "logger.h"

/* Global game context */
GameContext g_game = {0};

/* External global window handle */
extern GlobalState g_state;

/* State frame counter - DAT_04630dfc from FUN_004799b0 */
static DWORD g_state_counter = 0;

/*
 * Save screenshot to file - FUN_00414e30 pattern
 */
static void game_save_screenshot(void) {
    char filename[MAX_PATH];
    FILE* fp;
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
    u16* pixels;
    int width, height;
    int pitch;
    u8* row_buffer;
    int y;

    /* Generate filename with timestamp */
    SYSTEMTIME st;
    GetLocalTime(&st);
    _snprintf(filename, sizeof(filename), "screenshot_%04d%02d%02d_%02d%02d%02d.bmp",
              st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    /* Get screen dimensions */
    width = g_config.graphics.screen_width;
    height = g_config.graphics.screen_height;

    /* Get surface pixels */
    pixels = (u16*)graphics_get_surface_pixels(g_graphics.offscreen_surface);
    if (!pixels) {
        LOG_WARN("Failed to get surface pixels for screenshot");
        return;
    }
    pitch = graphics_get_surface_pitch(g_graphics.offscreen_surface) / 2;  /* In u16 units */

    /* Open file */
    fp = fopen(filename, "wb");
    if (!fp) {
        LOG_WARN("Failed to create screenshot file: %s", filename);
        return;
    }

    /* Setup BMP headers for 24-bit BMP */
    memset(&bfh, 0, sizeof(bfh));
    bfh.bfType = 0x4D42;  /* "BM" */
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfSize = bfh.bfOffBits + width * height * 3;

    memset(&bih, 0, sizeof(bih));
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = height;  /* Bottom-up */
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = BI_RGB;

    /* Write headers */
    fwrite(&bfh, sizeof(bfh), 1, fp);
    fwrite(&bih, sizeof(bih), 1, fp);

    /* Allocate row buffer for 24-bit data */
    row_buffer = (u8*)malloc(width * 3);

    /* Write pixel data (flip vertically for BMP) */
    for (y = height - 1; y >= 0; y--) {
        int x;
        u8* ptr = row_buffer;
        for (x = 0; x < width; x++) {
            u16 pixel = pixels[y * pitch + x];
            /* Convert 565 to 888 */
            u8 r = (u8)((pixel >> 11) & 0x1F);
            u8 g = (u8)((pixel >> 5) & 0x3F);
            u8 b = (u8)(pixel & 0x1F);
            *ptr++ = (b << 3) | (b >> 2);  /* Blue */
            *ptr++ = (g << 2) | (g >> 4);  /* Green */
            *ptr++ = (r << 3) | (r >> 2);  /* Red */
        }
        fwrite(row_buffer, width * 3, 1, fp);
    }

    free(row_buffer);
    fclose(fp);

    LOG_INFO("Screenshot saved: %s", filename);
}

/* Forward declarations for state handlers */
static void game_state_field(void);

/* Fade check - FUN_0047bde0 pattern */
int game_fade_check(int fade_id);

/* Game flags - matches DAT_04ebe494 region */
static DWORD g_game_flags = 0;
#define GAME_FLAG_FULLSCREEN    0x100
#define GAME_FLAG_VSYNC         0x400
#define GAME_FLAG_DEBUG         0x800

/* Frame timing */
static DWORD g_frame_start_time = 0;
static DWORD g_last_heartbeat_time = 0;

/* Anti-tamper timer - matches DAT_005ab718 pattern */
static DWORD g_antitamper_timer = 0;
static DWORD g_antitamper_flag = 0;

/*
 * Initialize game - FUN_0041e260 pattern
 */
int game_init(void) {
    DWORD current_time;
    char path[MAX_PATH];

    LOG_INFO("Initializing game...");

    memset(&g_game, 0, sizeof(GameContext));
    g_game.state = GAME_STATE_INIT;
    g_game.is_running = 1;
    g_game.fps = 60;

    /* Initialize CPU info - FUN_004813f0, cpuid */
    LOG_DEBUG("Checking CPU features...");

    /* DirectX already initialized by main_entry before game_init.
     * Original binary: FUN_0043f1f0 calls FUN_00411a00 (directx_init)
     * before FUN_0041db40 (game_run) which calls FUN_0041e260 (game_init). */

    /* Initialize render system (sets g_render.target to offscreen surface) */
    render_init();

    /* Load game assets */
    if (!assets_init()) {
        LOG_WARN("Failed to load game assets - continuing without data files");
    }

    /* Initialize UI system */
    if (!ui_init()) {
        LOG_ERROR("Failed to initialize UI system");
        return 0;
    }

    /* Initialize network */
    if (!network_init()) {
        LOG_WARN("Failed to initialize network - offline mode");
    }

    /* Initialize pet AI system */
    if (!petai_init()) {
        LOG_WARN("Failed to initialize pet AI system");
    }

    /* Get current time for random seed */
    current_time = timeGetTime();

    /* Initialize game timing */
    g_frame_start_time = current_time;
    g_last_heartbeat_time = current_time;
    g_game.last_frame_time = current_time;
    g_game.fps_time = current_time;

    /* Set game flags */
    g_game_flags = 0;
    if (!g_config.window_mode) {
        g_game_flags |= GAME_FLAG_FULLSCREEN;
    }

    /* Hide cursor in fullscreen mode */
    if (!g_config.window_mode) {
        ShowCursor(FALSE);
    }

    /* Create chat directory */
    CreateDirectoryA("chat", NULL);

    /* Set initial game state */
    g_game.state = GAME_STATE_LOGIN;

    LOG_INFO("Game initialized successfully");
    return 1;
}

/*
 * Process Windows messages - FUN_0043fa70 pattern
 */
static int game_process_messages(void) {
    MSG msg;

    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return 0;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 1;
}

/*
 * Main game loop - FUN_0041db40 pattern
 */
void game_main_loop(void) {
    DWORD current_time;
    DWORD frame_time;
    const DWORD target_frame_time = 16; /* ~60 FPS */
    const DWORD heartbeat_interval = 30000; /* 30 seconds */
    const DWORD antitamper_interval = 120000; /* 2 minutes */

    LOG_INFO("Entering main game loop");

    /* Initialize timing */
    g_frame_start_time = timeGetTime();
    g_last_heartbeat_time = g_frame_start_time;
    g_antitamper_timer = g_frame_start_time + antitamper_interval;

    while (g_game.is_running) {
        g_frame_start_time = timeGetTime();

        /* Process Windows messages */
        if (!game_process_messages()) {
            LOG_INFO("Received WM_QUIT");
            break;
        }

        /* Update input state */
        input_update();

        /* Handle keyboard shortcuts */
        if (input_key_pressed(KEY_ESCAPE) && !g_config.window_mode) {
            g_game.is_running = 0;
            break;
        }

        /* Screenshot on F12 - FUN_00414e30 */
        if ((g_game_flags & GAME_FLAG_DEBUG) && input_key_pressed(KEY_F12)) {
            game_save_screenshot();
            LOG_DEBUG("Screenshot requested");
        }

        /* Toggle vsync */
        if (input_key_pressed(KEY_F11)) {
            g_game_flags ^= GAME_FLAG_VSYNC;
        }

        /* Process network I/O - FUN_0045e880 */
        network_process();

        /* Update game systems */
        /* FUN_004799b0 - Key processing */
        /* FUN_0045db10 - Some update */
        /* FUN_0047a640 - Some update */
        /* FUN_0047d8c0 - Some update */
        /* FUN_0045dd10 - Some update */
        /* FUN_00412740 - Some update */

        /* Update game state */
        game_update();

        /* Render frame */
        game_render();

        /* Anti-tamper check - every 2 minutes */
        current_time = timeGetTime();
        if (g_antitamper_timer < current_time) {
            g_antitamper_timer = current_time + antitamper_interval;
            g_antitamper_flag = (g_antitamper_flag + 1) & 1;
            /* Original does VirtualAlloc/VirtualProtect here */
        }

        /* Frame rate limiting */
        current_time = timeGetTime();
        frame_time = current_time - g_frame_start_time;

        if (frame_time < target_frame_time) {
            /* Wait for remaining time */
            while ((timeGetTime() - g_frame_start_time) < target_frame_time) {
                Sleep(1);
            }
        }

        /* FPS calculation */
        g_game.frame_count++;
        if (current_time - g_game.fps_time >= 1000) {
            g_game.fps = g_game.frame_count;
            g_game.frame_count = 0;
            g_game.fps_time = current_time;
        }

        g_game.last_frame_time = current_time;
        g_game.frame_time = frame_time;
    }

    LOG_INFO("Exited main game loop (frames: %d, fps: %d)", g_game.frame_count, g_game.fps);
}

/*
 * Set game state - FUN_00479bc0
 */
void game_set_state(GameState state) {
    g_game.state_frame = 0;
    g_game.state = state;
}

/*
 * Get current game state
 */
GameState game_get_state(void) {
    return g_game.state;
}

/*
 * Process game state machine - FUN_00479c40
 * Accurate implementation from binary analysis
 */
void game_process_state(void) {
    int result;

    /* Check for pending state change - DAT_04630df8 != -1 */
    if (g_game.next_state != (GameState)-1) {
        g_game.state_frame = 0;
        g_game.state = g_game.next_state;
        g_game.next_state = (GameState)-1;
    }

    /* Process current state - matches FUN_00479c40 switch */
    switch (g_game.state) {
        case GAME_STATE_INIT:
            /* State 0: Initialize with fade */
            g_game.state_frame++;
            if (g_game.state_frame >= 0x96) {
                /* After fade, go to login init */
                g_game.state = GAME_STATE_LOGIN_INIT;
            }
            break;

        case GAME_STATE_FADE_IN:
            /* State 0x96: Fade in transition */
            g_game.state = GAME_STATE_LOGIN_INIT;
            break;

        case GAME_STATE_LOGIN_INIT:
            /* State 100: Login screen initialization */
            ui_update();
            g_game.state = GAME_STATE_LOGIN;
            break;

        case GAME_STATE_LOGIN:
            /* State 0x65: Login screen - handle input and update */
            {
                int login_result = login_screen_handle_input();
                if (login_result == 1 && g_login.username[0] && g_login.password[0]) {
                    /* Login pressed - attempt authentication */
                    login_auth(g_login.username, g_login.password);
                    g_game.state = GAME_STATE_LOGIN_WAIT;
                } else if (login_result == 2) {
                    /* Exit pressed */
                    g_game.is_running = 0;
                }
            }
            ui_update();
            if (login_get_state() >= LOGIN_STATE_SERVER_LIST) {
                g_game.render_mode = RENDER_MODE_NORMAL;
                g_game.state = GAME_STATE_LOGIN_WAIT;
            }
            break;

        case GAME_STATE_LOGIN_WAIT:
            /* State 0x66: Processing login */
            ui_update();
            if (login_get_state() == LOGIN_STATE_CHAR_LIST) {
                g_game.state = GAME_STATE_LOGIN_DONE;
            }
            break;

        case GAME_STATE_LOGIN_DONE:
            /* State 0x67: Login complete */
            /* FUN_0047a5e0 - Field init */
            /* FUN_004445c0 - Clear something */
            /* FUN_00401170 - Sound update */
            /* FUN_0047e440 - Render queue */
            /* FUN_004874d0 - Input update */
            /* FUN_00440e90 - UI update */
            /* FUN_0044b0b0 - Clear queue */
            /* FUN_00410850 - Update */
            /* FUN_004117e0 - Update */
            /* FUN_0041f1e0 - UI update */
            /* FUN_0047d8c0 - Update */
            /* FUN_00412a40 - Update */
            /* FUN_0047dc60 - Render */
            g_game.render_mode = RENDER_MODE_NORMAL;
            g_game.state = GAME_STATE_CHAR_SELECT;
            break;

        case GAME_STATE_CHAR_SELECT:
            /* State 1: Character selection */
            /* FUN_004779d0, FUN_00418330, FUN_0040f7d0, FUN_00424b50 */
            ui_update();
            if (login_get_state() == LOGIN_STATE_ENTERING_GAME) {
                /* Character selected, entering game */
                g_game.state = GAME_STATE_CHAR_WAIT;
            }
            break;

        case GAME_STATE_CHAR_WAIT:
            /* State 2: Waiting for game entry */
            result = game_fade_check(0x0D);
            if (result) {
                /* Transition to field */
                g_game.state = GAME_STATE_FIELD;
            }
            break;

        case GAME_STATE_FIELD:
            /* State 3: Main game field */
            game_state_field();
            break;

        case GAME_STATE_BATTLE_INIT:
            /* State 4: Battle initialization */
            /* FUN_0047a6e0 - Battle init */
            g_game.render_mode = RENDER_MODE_BATTLE_INIT;
            g_game.state = GAME_STATE_BATTLE_READY;
            break;

        case GAME_STATE_BATTLE_READY:
            /* State 5: Battle ready check */
            result = game_fade_check(0x10);
            if (result) {
                /* FUN_00444e60(0) - Hide cursor */
                game_set_state(GAME_STATE_LOGIN_INIT);
            }
            /* FUN_0044b0b0, FUN_0041f1e0 */
            ui_update();
            break;

        case GAME_STATE_LOGOUT:
            /* State 0x14: Logout */
            g_game.render_mode = RENDER_MODE_NORMAL;
            /* FUN_0047a490 - Cleanup */
            g_game.state = GAME_STATE_CHAR_SELECT;
            break;

        case GAME_STATE_MAP_CHANGE:
            /* State 200: Map change start */
            /* DAT_004cf830 = 1 */
            g_game.state_frame = 0;
            g_game.state = GAME_STATE_MAP_LOAD;
            break;

        case GAME_STATE_MAP_LOAD:
            /* State 0xC9: Map loading */
            g_game.render_mode = RENDER_MODE_BATTLE_INIT;
            result = game_fade_check(0x1C);
            if (result) {
                g_game.state = GAME_STATE_MAP_WAIT;
            }
            /* FUN_00419af0, FUN_0044b0b0, FUN_0041f1e0 */
            ui_update();
            break;

        case GAME_STATE_MAP_WAIT:
            /* State 0xCA: Wait for map data */
            /* Check if map data received */
            if (g_game.state_frame > 0) {
                g_game.state_frame = 0;
                g_game.state = GAME_STATE_MAP_ENTER;
            }
            g_game.state_frame++;
            ui_update();
            break;

        case GAME_STATE_MAP_ENTER:
            /* State 0xCB: Enter map */
            /* FUN_0047a5e0 - Field init */
            /* FUN_004445c0, FUN_00401170, FUN_0047e440 */
            /* FUN_004419a0, FUN_00440e90 */
            /* FUN_00410850, FUN_004117e0 */
            /* FUN_0047d8c0, FUN_00412a40 */
            /* FUN_0047dc60 - Render */
            g_game.render_mode = RENDER_MODE_NORMAL;
            g_game.state = GAME_STATE_MAP_DONE;
            break;

        case GAME_STATE_MAP_DONE:
            /* State 0xCC: Map enter done */
            g_game.render_mode = RENDER_MODE_BATTLE_INIT;
            result = game_fade_check(0x1D);
            if (result) {
                /* Return to field state */
                g_game.state = GAME_STATE_FIELD;
            }
            ui_update();
            break;

        case GAME_STATE_SHUTDOWN:
            g_game.is_running = 0;
            break;

        default:
            break;
    }
}

/*
 * Fade effect check - FUN_0047bde0 pattern
 * Returns 1 when fade is complete
 */
int game_fade_check(int fade_id) {
    /* In original, this checks fade animation state */
    /* For now, return based on frame counter */
    g_game.state_frame++;
    return (g_game.state_frame > 30);
}

/*
 * State 3: Main game field - FUN_00424880
 */
static void game_state_field(void) {
    /* FUN_0047a5e0 - Field update */
    /* FUN_0040fa10 - Player update */
    /* FUN_00442420 - Movement */
    /* FUN_00418370 - Animation */
    /* FUN_00442670 - NPC update */

    /* Update player movement */
    if (input_key_down(KEY_UP) || input_key_down(KEY_W)) {
        character_move(DIRECTION_UP);
    }
    else if (input_key_down(KEY_DOWN) || input_key_down(KEY_S)) {
        character_move(DIRECTION_DOWN);
    }
    else if (input_key_down(KEY_LEFT) || input_key_down(KEY_A)) {
        character_move(DIRECTION_LEFT);
    }
    else if (input_key_down(KEY_RIGHT) || input_key_down(KEY_D)) {
        character_move(DIRECTION_RIGHT);
    }

    /* Handle mouse clicks */
    if (input_mouse_pressed(MOUSE_BUTTON_LEFT)) {
        s32 mx = input_get_mouse_x();
        s32 my = input_get_mouse_y();
        ui_handle_click(mx, my);
    }

    /* FUN_004445c0 - Clear */
    /* FUN_00424f50 - Battle check */
    /* FUN_0041b870 - Update */
    /* FUN_00401170 - Sound */
    /* FUN_0047e440 - Render */
    /* FUN_00445070 - UI */
    /* FUN_004412e0 - Minimap */

    character_update_all();
    map_update();
    ui_update();

    /* Check for battle trigger - DAT_004e2644 */
    if (g_game.battle_encounter) {
        g_game.battle_encounter = 0;
        g_game.state = GAME_STATE_BATTLE_INIT;
    }
}

/*
 * Game update logic (deprecated - use game_process_state)
 */
void game_update(void) {
    game_process_state();
}

/*
 * Game rendering
 */
void game_render(void) {
    static int first_frame = 1;
    if (first_frame) {
        LOG_INFO("First frame rendering, state=%d", g_game.state);
        first_frame = 0;
    }

    /* Clear back buffer */
    graphics_clear(0);

    switch (g_game.state) {
        case GAME_STATE_INIT:
        case GAME_STATE_FADE_IN:
        case GAME_STATE_LOGIN_INIT:
        case GAME_STATE_LOGIN:
        case GAME_STATE_LOGIN_WAIT:
        case GAME_STATE_LOGIN_DONE:
            ui_render_login_screen();
            break;

        case GAME_STATE_CHAR_SELECT:
        case GAME_STATE_CHAR_WAIT:
            ui_render_character_select_screen();
            break;

        case GAME_STATE_FIELD:
            /* Render map tiles */
            map_render();
            /* Render characters */
            character_render_all();
            break;

        case GAME_STATE_BATTLE_INIT:
        case GAME_STATE_BATTLE_READY:
            /* Render battle background */
            battle_render_background();
            /* Render battle UI */
            battle_render_ui();
            /* Render combatants */
            battle_render_combatants();
            break;

        case GAME_STATE_MAP_CHANGE:
        case GAME_STATE_MAP_LOAD:
        case GAME_STATE_MAP_WAIT:
        case GAME_STATE_MAP_ENTER:
        case GAME_STATE_MAP_DONE:
            /* Fade/transition effect */
            map_render();
            character_render_all();
            break;

        default:
            break;
    }

    /* Render UI on top */
    ui_render();

    /* Flip surfaces */
    graphics_flip();
}

/*
 * Handle input
 */
void game_handle_input(void) {
    /* Already handled in input_update() */
}

/*
 * Run game - entry point
 */
void game_run(void) {
    game_main_loop();
}

/*
 * Shutdown game
 */
void game_shutdown(void) {
    LOG_INFO("Shutting down game...");

    g_game.state = GAME_STATE_SHUTDOWN;
    g_game.is_running = 0;

    /* Show cursor */
    ShowCursor(TRUE);

    /* Shutdown systems in reverse order */
    battle_shutdown();
    petai_shutdown();
    ui_shutdown();
    network_shutdown();
    assets_shutdown();
    sound_shutdown();
    input_shutdown();
    directx_shutdown();

    LOG_INFO("Game shutdown complete");
}
