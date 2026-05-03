/*
 * Stone Age Client - Application Context Header
 * Reverse engineered from sa_9061.exe
 * Central coordinator for all game systems
 */

#ifndef APPLICATION_H
#define APPLICATION_H

#include "types.h"

/* Application flags */
#define APP_FLAG_FULLSCREEN     (1 << 0)
#define APP_FLAG_WINDOWED       (1 << 1)
#define APP_FLAG_VSYNC          (1 << 2)
#define APP_FLAG_SHOW_FPS       (1 << 3)
#define APP_FLAG_DEBUG_MODE     (1 << 4)

/* Application state */
typedef enum {
    APP_STATE_NONE = 0,
    APP_STATE_INITIALIZING,
    APP_STATE_RUNNING,
    APP_STATE_PAUSED,
    APP_STATE_SHUTTING_DOWN
} AppState;

/* Application configuration */
typedef struct {
    /* Window settings */
    int width;
    int height;
    int bpp;
    int fullscreen;
    int vsync;

    /* Audio settings */
    int bgm_enabled;
    int se_enabled;
    int bgm_volume;
    int se_volume;

    /* Network settings */
    char server_ip[64];
    u16 server_port;

    /* Game settings */
    char username[32];
    char password[32];
    int auto_login;
    int debug_mode;

} AppConfig;

/* Application context */
typedef struct {
    /* State */
    AppState state;
    u32 flags;
    u32 start_time;

    /* Configuration */
    AppConfig config;

    /* Window handle */
    HWND hwnd;
    HINSTANCE hinstance;

    /* System initialization flags */
    int graphics_initialized;
    int audio_initialized;
    int network_initialized;
    int input_initialized;

    /* Error information */
    char last_error[256];
    int error_code;

} ApplicationContext;

/* Global application context */
extern ApplicationContext g_app;

/* Initialization */
int application_init(HINSTANCE hinstance, int cmd_show);
int application_init_systems(void);
void application_shutdown(void);

/* Main entry point */
int application_run(void);

/* Configuration */
void application_load_config(void);
void application_save_config(void);
void application_set_config(AppConfig* config);

/* Window management */
HWND application_get_hwnd(void);
void application_set_hwnd(HWND hwnd);
void application_resize(int width, int height);
void application_toggle_fullscreen(void);

/* State management */
AppState application_get_state(void);
void application_set_state(AppState state);
int application_is_running(void);

/* Error handling */
void application_set_error(const char* error, int code);
const char* application_get_error(void);
int application_get_error_code(void);

/* Debug */
void application_toggle_debug(void);
int application_is_debug(void);

/* Utilities */
u32 application_get_uptime(void);
void application_request_exit(void);

#endif /* APPLICATION_H */
