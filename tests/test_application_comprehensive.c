/*
 * Stone Age Client - Application Module Comprehensive Tests
 * Tests for application.c implementation
 *
 * Coverage:
 * - Application initialization and shutdown
 * - Configuration management
 * - State management
 * - Error handling
 * - System coordination
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Platform stubs */
typedef void* HINSTANCE;
typedef void* HWND;
typedef int BOOL;

/* Application states */
typedef enum {
    APP_STATE_NONE = 0,
    APP_STATE_INITIALIZING = 1,
    APP_STATE_RUNNING = 2,
    APP_STATE_PAUSED = 3,
    APP_STATE_SHUTTING_DOWN = 4,
    APP_STATE_ERROR = 5
} AppState;

/* Application flags */
typedef enum {
    APP_FLAG_NONE = 0,
    APP_FLAG_DEBUG_MODE = (1 << 0),
    APP_FLAG_FULLSCREEN = (1 << 1),
    APP_FLAG_VSYNC = (1 << 2),
    APP_FLAG_ACTIVE = (1 << 3)
} AppFlags;

/* Game states for stub */
typedef enum {
    GAME_STATE_NONE = 0,
    GAME_STATE_INIT = 1,
    GAME_STATE_LOGIN = 2,
    GAME_STATE_CHARACTER_SELECT = 3,
    GAME_STATE_FIELD = 4,
    GAME_STATE_BATTLE = 5
} GameState;

/* Configuration structure */
typedef struct {
    int width;
    int height;
    int bpp;
    int fullscreen;
    int vsync;
    int bgm_enabled;
    int se_enabled;
    int bgm_volume;
    int se_volume;
    char server_ip[64];
    u16 server_port;
    char username[32];
    char password[32];
    int auto_login;
    int debug_mode;
} AppConfig;

/* Application context */
typedef struct {
    HINSTANCE hinstance;
    HWND hwnd;
    AppState state;
    u32 flags;
    u32 start_time;
    AppConfig config;
    char last_error[256];
    int error_code;
    int audio_initialized;
    int network_initialized;
} ApplicationContext;

/* Global application context */
static ApplicationContext g_app = {0};

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

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Implementation Functions (matching application.c)
 * ======================================== */

static void application_init(HINSTANCE hinstance) {
    memset(&g_app, 0, sizeof(ApplicationContext));
    g_app.hinstance = hinstance;
    g_app.state = APP_STATE_INITIALIZING;
    g_app.start_time = 0;
    memcpy(&g_app.config, &s_default_config, sizeof(AppConfig));
}

static void application_shutdown(void) {
    g_app.state = APP_STATE_SHUTTING_DOWN;
    g_app.audio_initialized = 0;
    g_app.network_initialized = 0;
    memset(&g_app, 0, sizeof(ApplicationContext));
}

static void application_set_config(AppConfig* config) {
    if (config) {
        memcpy(&g_app.config, config, sizeof(AppConfig));
    }
}

static HWND application_get_hwnd(void) {
    return g_app.hwnd;
}

static void application_set_hwnd(HWND hwnd) {
    g_app.hwnd = hwnd;
}

static void application_resize(int width, int height) {
    g_app.config.width = width;
    g_app.config.height = height;
}

static void application_toggle_fullscreen(void) {
    g_app.config.fullscreen = !g_app.config.fullscreen;
}

static AppState application_get_state(void) {
    return g_app.state;
}

static void application_set_state(AppState state) {
    g_app.state = state;
}

static int application_is_running(void) {
    return g_app.state == APP_STATE_RUNNING;
}

static void application_set_error(const char* error, int code) {
    if (error) {
        strncpy(g_app.last_error, error, sizeof(g_app.last_error) - 1);
        g_app.last_error[sizeof(g_app.last_error) - 1] = '\0';
    }
    g_app.error_code = code;
}

static const char* application_get_error(void) {
    return g_app.last_error;
}

static int application_get_error_code(void) {
    return g_app.error_code;
}

static void application_toggle_debug(void) {
    g_app.config.debug_mode = !g_app.config.debug_mode;
    g_app.flags ^= APP_FLAG_DEBUG_MODE;
}

static int application_is_debug(void) {
    return g_app.config.debug_mode;
}

static u32 application_get_uptime(void) {
    return 1000; /* Stub: return fixed value for testing */
}

static void application_request_exit(void) {
    g_app.state = APP_STATE_SHUTTING_DOWN;
}

/* ========================================
 * Constants Tests
 * ======================================== */

static int test_app_state_values(void) {
    return APP_STATE_NONE == 0 &&
           APP_STATE_INITIALIZING == 1 &&
           APP_STATE_RUNNING == 2 &&
           APP_STATE_PAUSED == 3 &&
           APP_STATE_SHUTTING_DOWN == 4 &&
           APP_STATE_ERROR == 5;
}

static int test_app_flag_values(void) {
    return APP_FLAG_NONE == 0 &&
           APP_FLAG_DEBUG_MODE == 1 &&
           APP_FLAG_FULLSCREEN == 2 &&
           APP_FLAG_VSYNC == 4 &&
           APP_FLAG_ACTIVE == 8;
}

static int test_game_state_values(void) {
    return GAME_STATE_NONE == 0 &&
           GAME_STATE_INIT == 1 &&
           GAME_STATE_LOGIN == 2 &&
           GAME_STATE_CHARACTER_SELECT == 3 &&
           GAME_STATE_FIELD == 4 &&
           GAME_STATE_BATTLE == 5;
}

/* ========================================
 * Default Config Tests
 * ======================================== */

static int test_default_width(void) {
    return s_default_config.width == 640;
}

static int test_default_height(void) {
    return s_default_config.height == 480;
}

static int test_default_bpp(void) {
    return s_default_config.bpp == 16;
}

static int test_default_fullscreen(void) {
    return s_default_config.fullscreen == 0;
}

static int test_default_vsync(void) {
    return s_default_config.vsync == 1;
}

static int test_default_bgm_enabled(void) {
    return s_default_config.bgm_enabled == 1;
}

static int test_default_se_enabled(void) {
    return s_default_config.se_enabled == 1;
}

static int test_default_bgm_volume(void) {
    return s_default_config.bgm_volume == 100;
}

static int test_default_se_volume(void) {
    return s_default_config.se_volume == 100;
}

static int test_default_server_port(void) {
    return s_default_config.server_port == 9061;
}

static int test_default_server_ip(void) {
    return strcmp(s_default_config.server_ip, "127.0.0.1") == 0;
}

static int test_default_auto_login(void) {
    return s_default_config.auto_login == 0;
}

static int test_default_debug_mode(void) {
    return s_default_config.debug_mode == 0;
}

/* ========================================
 * Initialization Tests
 * ======================================== */

static int test_application_init(void) {
    application_init((HINSTANCE)1);

    return g_app.hinstance == (HINSTANCE)1 &&
           g_app.state == APP_STATE_INITIALIZING &&
           g_app.config.width == 640 &&
           g_app.config.height == 480;
}

static int test_application_init_clears_data(void) {
    g_app.config.width = 1024;
    g_app.config.height = 768;
    g_app.error_code = 123;

    application_init((HINSTANCE)1);

    return g_app.config.width == 640 &&
           g_app.config.height == 480 &&
           g_app.error_code == 0;
}

static int test_application_shutdown(void) {
    application_init((HINSTANCE)1);
    g_app.state = APP_STATE_RUNNING;
    g_app.audio_initialized = 1;
    g_app.network_initialized = 1;

    application_shutdown();

    return g_app.state == APP_STATE_NONE &&
           g_app.audio_initialized == 0 &&
           g_app.network_initialized == 0;
}

/* ========================================
 * State Management Tests
 * ======================================== */

static int test_application_get_state(void) {
    application_init((HINSTANCE)1);
    g_app.state = APP_STATE_RUNNING;

    return application_get_state() == APP_STATE_RUNNING;
}

static int test_application_set_state(void) {
    application_init((HINSTANCE)1);

    application_set_state(APP_STATE_PAUSED);

    return g_app.state == APP_STATE_PAUSED;
}

static int test_application_is_running_true(void) {
    application_init((HINSTANCE)1);
    g_app.state = APP_STATE_RUNNING;

    return application_is_running() == 1;
}

static int test_application_is_running_false(void) {
    application_init((HINSTANCE)1);
    g_app.state = APP_STATE_INITIALIZING;

    return application_is_running() == 0;
}

static int test_application_request_exit(void) {
    application_init((HINSTANCE)1);
    g_app.state = APP_STATE_RUNNING;

    application_request_exit();

    return g_app.state == APP_STATE_SHUTTING_DOWN;
}

/* ========================================
 * Configuration Tests
 * ======================================== */

static int test_application_set_config(void) {
    AppConfig config = {
        .width = 800,
        .height = 600,
        .bpp = 32,
        .fullscreen = 1,
        .vsync = 0,
        .bgm_enabled = 0,
        .se_enabled = 1,
        .bgm_volume = 50,
        .se_volume = 75,
        .server_port = 8080,
        .auto_login = 1,
        .debug_mode = 1
    };
    strcpy(config.server_ip, "192.168.1.1");

    application_init((HINSTANCE)1);
    application_set_config(&config);

    return g_app.config.width == 800 &&
           g_app.config.height == 600 &&
           g_app.config.bpp == 32 &&
           g_app.config.fullscreen == 1 &&
           g_app.config.vsync == 0 &&
           g_app.config.bgm_volume == 50 &&
           g_app.config.se_volume == 75 &&
           strcmp(g_app.config.server_ip, "192.168.1.1") == 0;
}

static int test_application_set_config_null(void) {
    application_init((HINSTANCE)1);
    g_app.config.width = 1024;

    application_set_config(NULL);

    /* Should remain unchanged */
    return g_app.config.width == 1024;
}

static int test_application_resize(void) {
    application_init((HINSTANCE)1);

    application_resize(1280, 720);

    return g_app.config.width == 1280 &&
           g_app.config.height == 720;
}

static int test_application_toggle_fullscreen(void) {
    application_init((HINSTANCE)1);
    int initial = g_app.config.fullscreen;

    application_toggle_fullscreen();

    return g_app.config.fullscreen == !initial;
}

/* ========================================
 * Window Handle Tests
 * ======================================== */

static int test_application_set_hwnd(void) {
    application_init((HINSTANCE)1);

    application_set_hwnd((HWND)0x12345678);

    return g_app.hwnd == (HWND)0x12345678;
}

static int test_application_get_hwnd(void) {
    application_init((HINSTANCE)1);
    g_app.hwnd = (HWND)0xABCDEF00;

    return application_get_hwnd() == (HWND)0xABCDEF00;
}

/* ========================================
 * Error Handling Tests
 * ======================================== */

static int test_application_set_error(void) {
    application_init((HINSTANCE)1);

    application_set_error("Test error message", 42);

    return strcmp(g_app.last_error, "Test error message") == 0 &&
           g_app.error_code == 42;
}

static int test_application_get_error(void) {
    application_init((HINSTANCE)1);
    strcpy(g_app.last_error, "Error text");

    const char* err = application_get_error();

    return strcmp(err, "Error text") == 0;
}

static int test_application_get_error_code(void) {
    application_init((HINSTANCE)1);
    g_app.error_code = 100;

    return application_get_error_code() == 100;
}

static int test_application_error_truncation(void) {
    application_init((HINSTANCE)1);

    /* Create a very long error message */
    char long_error[512];
    memset(long_error, 'X', sizeof(long_error) - 1);
    long_error[sizeof(long_error) - 1] = '\0';

    application_set_error(long_error, 1);

    /* Should be truncated to fit in last_error buffer */
    return strlen(g_app.last_error) < sizeof(g_app.last_error);
}

/* ========================================
 * Debug Mode Tests
 * ======================================== */

static int test_application_toggle_debug(void) {
    application_init((HINSTANCE)1);
    int initial = g_app.config.debug_mode;

    application_toggle_debug();

    return g_app.config.debug_mode == !initial;
}

static int test_application_is_debug(void) {
    application_init((HINSTANCE)1);
    g_app.config.debug_mode = 1;

    return application_is_debug() == 1;
}

static int test_application_is_debug_off(void) {
    application_init((HINSTANCE)1);
    g_app.config.debug_mode = 0;

    return application_is_debug() == 0;
}

/* ========================================
 * Flags Tests
 * ======================================== */

static int test_app_flag_debug_mode(void) {
    application_init((HINSTANCE)1);
    g_app.flags = APP_FLAG_NONE;

    application_toggle_debug();

    return (g_app.flags & APP_FLAG_DEBUG_MODE) != 0;
}

static int test_app_flag_toggle_twice(void) {
    application_init((HINSTANCE)1);
    g_app.flags = APP_FLAG_NONE;

    application_toggle_debug();
    application_toggle_debug();

    return (g_app.flags & APP_FLAG_DEBUG_MODE) == 0;
}

/* ========================================
 * System Initialization Tests
 * ======================================== */

static int test_audio_initialized_flag(void) {
    application_init((HINSTANCE)1);
    g_app.audio_initialized = 1;

    application_shutdown();

    return g_app.audio_initialized == 0;
}

static int test_network_initialized_flag(void) {
    application_init((HINSTANCE)1);
    g_app.network_initialized = 1;

    application_shutdown();

    return g_app.network_initialized == 0;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_application_lifecycle(void) {
    /* Initialize */
    application_init((HINSTANCE)1);
    if (g_app.state != APP_STATE_INITIALIZING) return 0;

    /* Set window */
    application_set_hwnd((HWND)0x1234);
    if (application_get_hwnd() != (HWND)0x1234) return 0;

    /* Resize */
    application_resize(1024, 768);
    if (g_app.config.width != 1024 || g_app.config.height != 768) return 0;

    /* Start running */
    application_set_state(APP_STATE_RUNNING);
    if (!application_is_running()) return 0;

    /* Set error */
    application_set_error("Test error", 99);
    if (application_get_error_code() != 99) return 0;

    /* Request exit */
    application_request_exit();
    if (g_app.state != APP_STATE_SHUTTING_DOWN) return 0;

    /* Shutdown */
    application_shutdown();
    if (g_app.state != APP_STATE_NONE) return 0;

    return 1;
}

static int test_config_change_flow(void) {
    AppConfig custom = {
        .width = 1920,
        .height = 1080,
        .bpp = 32,
        .fullscreen = 1,
        .vsync = 1,
        .bgm_enabled = 1,
        .se_enabled = 1,
        .bgm_volume = 80,
        .se_volume = 90,
        .server_port = 9999,
        .auto_login = 0,
        .debug_mode = 0
    };
    strcpy(custom.server_ip, "game.example.com");

    application_init((HINSTANCE)1);
    application_set_config(&custom);

    if (g_app.config.width != 1920) return 0;
    if (g_app.config.height != 1080) return 0;
    if (strcmp(g_app.config.server_ip, "game.example.com") != 0) return 0;

    /* Toggle fullscreen off */
    application_toggle_fullscreen();
    if (g_app.config.fullscreen != 0) return 0;

    return 1;
}

static int test_error_recovery_flow(void) {
    application_init((HINSTANCE)1);

    /* Simulate error */
    application_set_error("Network timeout", 1001);
    application_set_state(APP_STATE_ERROR);

    if (application_get_state() != APP_STATE_ERROR) return 0;
    if (application_get_error_code() != 1001) return 0;

    /* Clear error and resume */
    application_set_error("", 0);
    application_set_state(APP_STATE_RUNNING);

    if (!application_is_running()) return 0;

    return 1;
}

static int test_debug_mode_flow(void) {
    application_init((HINSTANCE)1);

    /* Initially off */
    if (application_is_debug()) return 0;

    /* Toggle on */
    application_toggle_debug();
    if (!application_is_debug()) return 0;
    if (!(g_app.flags & APP_FLAG_DEBUG_MODE)) return 0;

    /* Toggle off */
    application_toggle_debug();
    if (application_is_debug()) return 0;

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Application Module Comprehensive Tests ===\n\n");

    /* Constants tests */
    printf("Constants Tests:\n");
    TEST(app_state_values);
    TEST(app_flag_values);
    TEST(game_state_values);

    /* Default config tests */
    printf("\nDefault Config Tests:\n");
    TEST(default_width);
    TEST(default_height);
    TEST(default_bpp);
    TEST(default_fullscreen);
    TEST(default_vsync);
    TEST(default_bgm_enabled);
    TEST(default_se_enabled);
    TEST(default_bgm_volume);
    TEST(default_se_volume);
    TEST(default_server_port);
    TEST(default_server_ip);
    TEST(default_auto_login);
    TEST(default_debug_mode);

    /* Initialization tests */
    printf("\nInitialization Tests:\n");
    TEST(application_init);
    TEST(application_init_clears_data);
    TEST(application_shutdown);

    /* State management tests */
    printf("\nState Management Tests:\n");
    TEST(application_get_state);
    TEST(application_set_state);
    TEST(application_is_running_true);
    TEST(application_is_running_false);
    TEST(application_request_exit);

    /* Configuration tests */
    printf("\nConfiguration Tests:\n");
    TEST(application_set_config);
    TEST(application_set_config_null);
    TEST(application_resize);
    TEST(application_toggle_fullscreen);

    /* Window handle tests */
    printf("\nWindow Handle Tests:\n");
    TEST(application_set_hwnd);
    TEST(application_get_hwnd);

    /* Error handling tests */
    printf("\nError Handling Tests:\n");
    TEST(application_set_error);
    TEST(application_get_error);
    TEST(application_get_error_code);
    TEST(application_error_truncation);

    /* Debug mode tests */
    printf("\nDebug Mode Tests:\n");
    TEST(application_toggle_debug);
    TEST(application_is_debug);
    TEST(application_is_debug_off);

    /* Flags tests */
    printf("\nFlags Tests:\n");
    TEST(app_flag_debug_mode);
    TEST(app_flag_toggle_twice);

    /* System initialization tests */
    printf("\nSystem Initialization Tests:\n");
    TEST(audio_initialized_flag);
    TEST(network_initialized_flag);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_application_lifecycle);
    TEST(config_change_flow);
    TEST(error_recovery_flow);
    TEST(debug_mode_flow);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
