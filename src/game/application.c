/*
 * Stone Age Client - Application Context Implementation
 * Reverse engineered from sa_9061.exe
 * Central coordinator for all game systems
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "application.h"
#include "gameloop.h"
#include "gamestate.h"
#include "audiomanager.h"
#include "resource.h"
#include "configfile.h"
#include "logger.h"
#include "window.h"
#include "directx.h"
#include "network.h"
#include "input.h"

/* Global application context */
ApplicationContext g_app = {0};

/* Default configuration */
static AppConfig s_default_config = {
    .width = 640,
    .height = 480,
    .bpp = 16,
    .fullscreen = 0,
    .vsync = 1,
    .bgm_enabled = 1,
    .se_enabled = 1,
    .bgm_volume = 100,
    .se_volume = 100,
    .server_ip = "127.0.0.1",
    .server_port = 9061,
    .username = "",
    .password = "",
    .auto_login = 0,
    .debug_mode = 0
};

/*
 * Initialize application
 */
int application_init(HINSTANCE hinstance, int cmd_show) {
    memset(&g_app, 0, sizeof(ApplicationContext));

    g_app.hinstance = hinstance;
    g_app.state = APP_STATE_INITIALIZING;
    g_app.start_time = timeGetTime();

    /* Load configuration */
    application_load_config();

    LOG_INFO("Application initialized");
    return 1;
}

/*
 * Initialize all game systems
 */
int application_init_systems(void) {
    LOG_INFO("Initializing game systems...");

    /* Initialize resource cache */
    if (!resource_init()) {
        application_set_error("Failed to initialize resource system", 1);
        return 0;
    }

    /* Initialize game state manager */
    if (!gamestate_init()) {
        application_set_error("Failed to initialize game state manager", 2);
        return 0;
    }

    /* Initialize audio manager */
    if (!audiomanager_init()) {
        LOG_WARN("Audio system initialization failed, continuing without audio");
        /* Non-fatal error */
    } else {
        g_app.audio_initialized = 1;
    }

    /* Initialize game loop */
    if (!gameloop_init()) {
        application_set_error("Failed to initialize game loop", 3);
        return 0;
    }

    /* Initialize network */
    if (!network_init()) {
        LOG_WARN("Network initialization failed");
        /* Non-fatal - can retry connection later */
    } else {
        g_app.network_initialized = 1;
    }

    g_app.state = APP_STATE_RUNNING;
    LOG_INFO("All game systems initialized successfully");

    return 1;
}

/*
 * Shutdown application
 */
void application_shutdown(void) {
    LOG_INFO("Shutting down application...");

    g_app.state = APP_STATE_SHUTTING_DOWN;

    /* Shutdown systems in reverse order */
    gameloop_shutdown();

    if (g_app.audio_initialized) {
        audiomanager_shutdown();
        g_app.audio_initialized = 0;
    }

    gamestate_shutdown();
    resource_shutdown();

    if (g_app.network_initialized) {
        network_shutdown();
        g_app.network_initialized = 0;
    }

    LOG_INFO("Application shutdown complete");
}

/*
 * Main application loop
 */
int application_run(void) {
    int result;

    /* Initialize systems */
    if (!application_init_systems()) {
        LOG_ERROR("Failed to initialize game systems");
        return -1;
    }

    /* Set initial game state */
    gamestate_change(GAME_STATE_INIT);

    /* Run main game loop */
    result = gameloop_run();

    /* Shutdown */
    application_shutdown();

    return result;
}

/*
 * Load configuration from file
 */
void application_load_config(void) {
    ConfigFile config;

    /* Start with defaults */
    memcpy(&g_app.config, &s_default_config, sizeof(AppConfig));

    /* Try to load from file */
    configfile_init(&config);
    if (configfile_load(&config, "data\\config.ini")) {
        /* Window settings */
        g_app.config.width = configfile_get_int(&config, "Graphics.Width", 640);
        g_app.config.height = configfile_get_int(&config, "Graphics.Height", 480);
        g_app.config.bpp = configfile_get_int(&config, "Graphics.BPP", 16);
        g_app.config.fullscreen = configfile_get_int(&config, "Graphics.Fullscreen", 0);
        g_app.config.vsync = configfile_get_int(&config, "Graphics.VSync", 1);

        /* Audio settings */
        g_app.config.bgm_enabled = configfile_get_int(&config, "Audio.BGM", 1);
        g_app.config.se_enabled = configfile_get_int(&config, "Audio.SE", 1);
        g_app.config.bgm_volume = configfile_get_int(&config, "Audio.BGMVolume", 100);
        g_app.config.se_volume = configfile_get_int(&config, "Audio.SEVolume", 100);

        /* Network settings */
        const char* server = configfile_get(&config, "Network.Server", "127.0.0.1");
        strncpy(g_app.config.server_ip, server, sizeof(g_app.config.server_ip) - 1);
        g_app.config.server_port = (u16)configfile_get_int(&config, "Network.Port", 9061);

        LOG_INFO("Configuration loaded from data\\config.ini");
    }

    configfile_shutdown(&config);
}

/*
 * Save configuration to file
 */
void application_save_config(void) {
    ConfigFile config;

    configfile_init(&config);

    /* Window settings */
    configfile_set_int(&config, "Graphics.Width", g_app.config.width);
    configfile_set_int(&config, "Graphics.Height", g_app.config.height);
    configfile_set_int(&config, "Graphics.BPP", g_app.config.bpp);
    configfile_set_int(&config, "Graphics.Fullscreen", g_app.config.fullscreen);
    configfile_set_int(&config, "Graphics.VSync", g_app.config.vsync);

    /* Audio settings */
    configfile_set_int(&config, "Audio.BGM", g_app.config.bgm_enabled);
    configfile_set_int(&config, "Audio.SE", g_app.config.se_enabled);
    configfile_set_int(&config, "Audio.BGMVolume", g_app.config.bgm_volume);
    configfile_set_int(&config, "Audio.SEVolume", g_app.config.se_volume);

    /* Network settings */
    configfile_set(&config, "Network.Server", g_app.config.server_ip);
    configfile_set_int(&config, "Network.Port", g_app.config.server_port);

    configfile_save(&config, "data\\config.ini");
    configfile_shutdown(&config);

    LOG_INFO("Configuration saved");
}

/*
 * Set configuration
 */
void application_set_config(AppConfig* config) {
    if (config) {
        memcpy(&g_app.config, config, sizeof(AppConfig));
    }
}

/*
 * Get window handle
 */
HWND application_get_hwnd(void) {
    return g_app.hwnd;
}

/*
 * Set window handle
 */
void application_set_hwnd(HWND hwnd) {
    g_app.hwnd = hwnd;
}

/*
 * Resize application window
 */
void application_resize(int width, int height) {
    g_app.config.width = width;
    g_app.config.height = height;

    /* Notify graphics system of resize */
    /* graphics_resize(width, height); */
}

/*
 * Toggle fullscreen mode
 */
void application_toggle_fullscreen(void) {
    g_app.config.fullscreen = !g_app.config.fullscreen;

    /* Notify graphics system */
    /* graphics_set_fullscreen(g_app.config.fullscreen); */
}

/*
 * Get application state
 */
AppState application_get_state(void) {
    return g_app.state;
}

/*
 * Set application state
 */
void application_set_state(AppState state) {
    g_app.state = state;
}

/*
 * Check if application is running
 */
int application_is_running(void) {
    return g_app.state == APP_STATE_RUNNING;
}

/*
 * Set error information
 */
void application_set_error(const char* error, int code) {
    strncpy(g_app.last_error, error, sizeof(g_app.last_error) - 1);
    g_app.error_code = code;
    LOG_ERROR("Application error: %s (code: %d)", error, code);
}

/*
 * Get last error string
 */
const char* application_get_error(void) {
    return g_app.last_error;
}

/*
 * Get last error code
 */
int application_get_error_code(void) {
    return g_app.error_code;
}

/*
 * Toggle debug mode
 */
void application_toggle_debug(void) {
    g_app.config.debug_mode = !g_app.config.debug_mode;
    g_app.flags ^= APP_FLAG_DEBUG_MODE;
}

/*
 * Check if debug mode enabled
 */
int application_is_debug(void) {
    return g_app.config.debug_mode;
}

/*
 * Get application uptime in milliseconds
 */
u32 application_get_uptime(void) {
    return timeGetTime() - g_app.start_time;
}

/*
 * Request application exit
 */
void application_request_exit(void) {
    g_app.state = APP_STATE_SHUTTING_DOWN;
    gameloop_stop();
}
