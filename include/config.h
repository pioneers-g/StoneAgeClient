#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"
#include "gameconfig.h"

/* Default paths */
#define DEFAULT_REAL_BIN        "data/real.bin"
#define DEFAULT_ADRN_BIN        "data/adrn.bin"
#define DEFAULT_SPR_BIN         "data/spr.bin"
#define DEFAULT_SPRADRN_BIN     "data/spradrn.bin"
#define DEFAULT_TOKYOHOT_DAT    "tokyohot.dat"
#define DEFAULT_REALTRUE_BIN    "data/realtrue.bin"
#define DEFAULT_ADRNTRUE_BIN    "data/adrntrue.bin"

/* Window class name - matches original binary */
#define WINDOW_CLASS_NAME       "StoneAgeClient"

/* Window style constants */
#define WINDOW_STYLE_FULLSCREEN (WS_POPUP | WS_VISIBLE)
#define WINDOW_STYLE_WINDOWED   (WS_OVERLAPPEDWINDOW | WS_VISIBLE)

/* Version info */
#define GAME_VERSION_MAJOR      9
#define GAME_VERSION_MINOR      0
#define GAME_VERSION_PATCH      6
#define GAME_VERSION_BUILD      1

/* Configuration functions - declared in gameconfig.h */
/* void config_init(void); */

/* Path access */
const char* config_get_data_path(void);
const char* config_get_real_bin_path(void);
const char* config_get_adrn_bin_path(void);
const char* config_get_spr_bin_path(void);

#endif /* CONFIG_H */
