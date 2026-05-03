/*
 * Stone Age Client - Configuration
 * Reverse engineered from sa_9061.exe (FUN_0043f4b0)
 */

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "types.h"
#include "config.h"
#include "commandline.h"
#include "logger.h"

/* String constants from original binary */
static const char* STR_USERID       = "userid";
static const char* STR_REALBIN      = "realbin";
static const char* STR_ADRNBIN      = "adrnbin";
static const char* STR_SPRBIN       = "sprbin";
static const char* STR_SPRADBNBIN   = "spradrnbin";
static const char* STR_TOKYOHOT     = "tokyohot";
static const char* STR_REALTRUE     = "realtrue";
static const char* STR_ADRNTRUE     = "adrntrue";
static const char* STR_USEALPHA     = "usealpha";
static const char* STR_WINDOWMODE   = "windowmode";
static const char* STR_NODELAY      = "nodelay";

/* Default data path */
static const char* g_data_path = "data/";

/* Initialize configuration paths with defaults */
void config_init_paths(GameConfig* config) {
    memset(config, 0, sizeof(GameConfig));

    /* Set default paths */
    strncpy(config->real_bin_path, DEFAULT_REAL_BIN, sizeof(config->real_bin_path) - 1);
    strncpy(config->adrn_bin_path, DEFAULT_ADRN_BIN, sizeof(config->adrn_bin_path) - 1);
    strncpy(config->spr_bin_path, DEFAULT_SPR_BIN, sizeof(config->spr_bin_path) - 1);
    strncpy(config->spradrn_bin_path, DEFAULT_SPRADRN_BIN, sizeof(config->spradrn_bin_path) - 1);
    strncpy(config->tokyohot_path, DEFAULT_TOKYOHOT_DAT, sizeof(config->tokyohot_path) - 1);
    strncpy(config->realtrue_bin_path, DEFAULT_REALTRUE_BIN, sizeof(config->realtrue_bin_path) - 1);
    strncpy(config->adrntrue_bin_path, DEFAULT_ADRNTRUE_BIN, sizeof(config->adrntrue_bin_path) - 1);

    /* Default settings */
    config->use_alpha = 0;
    config->window_mode = 0;  /* Fullscreen by default */
    config->no_delay = 0;
    config->updated = 0;
}

/* Parse command line parameters */
void config_parse_commandline(GameConfig* config, const char* cmdline) {
    const char* param;

    /* Parse userid */
    cmdline_parse_string(cmdline, STR_USERID, config->userid, sizeof(config->userid));
    if (config->userid[0] != '\0') {
        LOG_INFO("User ID: %s", config->userid);
    }

    /* Parse realbin path */
    param = cmdline_find_param(cmdline, STR_REALBIN);
    if (param) {
        snprintf(config->real_bin_path, sizeof(config->real_bin_path),
                 "data/real_%s.bin", param);
        LOG_INFO("Real bin path: %s", config->real_bin_path);
    }

    /* Parse adrnbin path */
    param = cmdline_find_param(cmdline, STR_ADRNBIN);
    if (param) {
        snprintf(config->adrn_bin_path, sizeof(config->adrn_bin_path),
                 "data/adrn_%s.bin", param);
        LOG_INFO("Adrn bin path: %s", config->adrn_bin_path);
    }

    /* Parse sprbin path */
    param = cmdline_find_param(cmdline, STR_SPRBIN);
    if (param) {
        snprintf(config->spr_bin_path, sizeof(config->spr_bin_path),
                 "data/spr_%s.bin", param);
        LOG_INFO("Spr bin path: %s", config->spr_bin_path);
    }

    /* Parse spradrnbin path */
    param = cmdline_find_param(cmdline, STR_SPRADBNBIN);
    if (param) {
        snprintf(config->spradrn_bin_path, sizeof(config->spradrn_bin_path),
                 "data/spradrn_%s.bin", param);
        LOG_INFO("Spradrn bin path: %s", config->spradrn_bin_path);
    }

    /* Parse tokyohot path */
    param = cmdline_find_param(cmdline, STR_TOKYOHOT);
    if (param) {
        snprintf(config->tokyohot_path, sizeof(config->tokyohot_path),
                 "%s.dll", param);
        LOG_INFO("Tokyohot path: %s", config->tokyohot_path);
    }

    /* Parse realtrue path */
    param = cmdline_find_param(cmdline, STR_REALTRUE);
    if (param) {
        snprintf(config->realtrue_bin_path, sizeof(config->realtrue_bin_path),
                 "data/realtrue_%s.bin", param);
        LOG_INFO("Realtrue bin path: %s", config->realtrue_bin_path);
    }

    /* Parse adrntrue path */
    param = cmdline_find_param(cmdline, STR_ADRNTRUE);
    if (param) {
        snprintf(config->adrntrue_bin_path, sizeof(config->adrntrue_bin_path),
                 "data/adrntrue_%s.bin", param);
        LOG_INFO("Adrntrue bin path: %s", config->adrntrue_bin_path);
    }

    /* Parse boolean flags */
    config->use_alpha = cmdline_parse_bool(cmdline, STR_USEALPHA);
    config->window_mode = cmdline_parse_bool(cmdline, STR_WINDOWMODE);
    config->no_delay = cmdline_parse_bool(cmdline, STR_NODELAY);

    LOG_INFO("Alpha: %d, Window mode: %d, No delay: %d",
             config->use_alpha, config->window_mode, config->no_delay);
}

/* Get data path */
const char* config_get_data_path(void) {
    return g_data_path;
}

/* Get real bin path */
const char* config_get_real_bin_path(void) {
    return DEFAULT_REAL_BIN;
}

/* Get adrn bin path */
const char* config_get_adrn_bin_path(void) {
    return DEFAULT_ADRN_BIN;
}

/* Get spr bin path */
const char* config_get_spr_bin_path(void) {
    return DEFAULT_SPR_BIN;
}
