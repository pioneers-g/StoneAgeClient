/*
 * Stone Age Client - Game Configuration Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include "types.h"

/* Constants */
#define MAX_CONFIG_LINE     256
#define MAX_CONFIG_VALUE    128
#define CONFIG_FILE         "setup.ini"
#define ACCOUNT_FILE        "account.dat"

/* Graphics settings */
typedef struct {
    u16 screen_width;
    u16 screen_height;
    u8  color_depth;
    u8  fullscreen;
    u8  vsync;
    u8  gamma;
    u8  detail_level;
    u8  shadow_quality;
    u8  weather_effects;
    u8  high_res_mode;      /* DAT_0054c83c - 0=standard sprites (0x6xxx), 1=high-res sprites (0x7axxx) */
    u8  reserved;
} GraphicsConfig;

/* Sound settings */
typedef struct {
    u8  master_volume;
    u8  music_volume;
    u8  sfx_volume;
    u8  ambient_volume;
    u8  voice_volume;
    u8  mute_audio;
    u8  mute_music;
    u8  reserved;
} SoundConfig;

/* Network settings */
typedef struct {
    char server_host[64];
    u16 server_port;
    u16 backup_port;
    u8  connection_timeout;
    u8  retry_count;
    u8  use_proxy;
    char proxy_host[64];
    u16 proxy_port;
} NetworkConfig;

/* Game settings */
typedef struct {
    u8  language;
    u8  auto_attack;
    u8  auto_pickup;
    u8  show_damage;
    u8  show_hp_bar;
    u8  show_names;
    u8  show_minimap;
    u8  camera_mode;
    u8  chat_filter;
    u8  profanity_filter;
    u8  auto_save;
    u8  save_interval;
    u16 fps_limit;
    u32 reserved;
} GameSettings;

/* Key bindings */
typedef struct {
    u32 move_up;
    u32 move_down;
    u32 move_left;
    u32 move_right;
    u32 attack;
    u32 skill1;
    u32 skill2;
    u32 skill3;
    u32 inventory;
    u32 character;
    u32 skill_window;
    u32 map;
    u32 party;
    u32 guild;
    u32 chat;
    u32 system;
    u32 quick_slots[10];
} KeyBindings;

/* Account info */
typedef struct {
    char username[24];
    char encrypted_pass[48];
    u8  remember_pass;
    u8  auto_login;
    u32 last_server;
    u8  last_character;
    char last_char_name[24];
} AccountInfo;

/* Full configuration - use GameConfig as the main config type */
typedef struct GameConfig {
    /* Data file paths */
    char userid[256];
    char real_bin_path[260];
    char adrn_bin_path[260];
    char spr_bin_path[260];
    char spradrn_bin_path[260];
    char tokyohot_path[260];
    char realtrue_bin_path[260];
    char adrntrue_bin_path[260];

    /* Path pointers for runtime assignment */
    const char* real_bin_path_ptr;
    const char* adrn_bin_path_ptr;
    const char* spr_bin_path_ptr;
    const char* spradrn_bin_path_ptr;

    /* Flags */
    int use_alpha;
    int window_mode;
    int no_delay;
    int updated;

    /* Nested configs */
    GraphicsConfig graphics;
    SoundConfig sound;
    NetworkConfig network;
    GameSettings game;
    KeyBindings keys;
    AccountInfo account;

    /* State */
    u8 loaded;
    u8 dirty;
    char config_path[64];

} GameConfig;

/* Global config */
extern GameConfig g_config;

/* Initialization */
int config_init(void);
void config_shutdown(void);

/* Load/Save */
int config_load(void);
int config_save(void);
int config_load_default(void);

/* Get/Set values */
int config_get_int(const char* section, const char* key, int default_val);
const char* config_get_string(const char* section, const char* key, const char* default_val);
int config_set_int(const char* section, const char* key, int value);
int config_set_string(const char* section, const char* key, const char* value);

/* Graphics */
void config_set_resolution(u16 width, u16 height);
void config_get_resolution(u16* width, u16* height);
void config_set_fullscreen(u8 fullscreen);
u8 config_is_fullscreen(void);

/* Sound */
void config_set_master_volume(u8 volume);
u8 config_get_master_volume(void);
void config_set_music_volume(u8 volume);
u8 config_get_music_volume(void);
void config_set_sfx_volume(u8 volume);
u8 config_get_sfx_volume(void);

/* Network */
void config_set_server(const char* host, u16 port);
const char* config_get_server_host(void);
u16 config_get_server_port(void);

/* Key bindings */
int config_set_key_binding(const char* action, u32 key);
u32 config_get_key_binding(const char* action);
int config_reset_key_bindings(void);

/* Account */
int config_save_account(const char* username, const char* password, int remember);
int config_load_account(char* username, char* password, int max_len);
void config_clear_account(void);
int config_has_saved_account(void);

/* Game settings */
void config_set_language(u8 lang);
u8 config_get_language(void);
void config_set_auto_attack(u8 enable);
u8 config_get_auto_attack(void);
void config_set_fps_limit(u16 fps);
u16 config_get_fps_limit(void);

/* Utility */
void config_reset_defaults(void);
void config_apply(void);

#endif /* GAMECONFIG_H */
