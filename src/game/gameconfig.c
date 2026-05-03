/*
 * Stone Age Client - Game Configuration Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "gameconfig.h"
#include "graphics.h"
#include "audiomanager.h"
#include "minimap.h"
#include "logger.h"

/* XOR encryption key for password storage */
static const u8 s_encrypt_key[] = {
    0x5A, 0x3C, 0x7B, 0x1E, 0x9F, 0x2D, 0x4A, 0x8C,
    0x6E, 0x5F, 0x0B, 0x3D, 0x7A, 0x1C, 0x9E, 0x2F
};
#define ENCRYPT_KEY_LEN (sizeof(s_encrypt_key) / sizeof(s_encrypt_key[0]))

/* Global config */
GameConfig g_config = {0};

/* Default key bindings */
static const KeyBindings s_default_keys = {
    'W', 'S', 'A', 'D',           /* Movement */
    VK_SPACE,                      /* Attack */
    '1', '2', '3',                 /* Skills */
    'I', 'C', 'K', 'M',           /* Windows */
    'P', 'G', VK_RETURN, VK_ESCAPE, /* Party, Guild, Chat, System */
    { 'Q', 'E', 'R', 'F', 'Z', 'X', 'V', 'B', 'N', 'T' }  /* Quick slots */
};

/*
 * Initialize configuration
 */
int config_init(void) {
    memset(&g_config, 0, sizeof(GameConfig));

    config_load_default();

    if (!config_load()) {
        LOG_WARN("Using default configuration");
    }

    LOG_INFO("Configuration initialized");
    return 1;
}

/*
 * Shutdown configuration
 */
void config_shutdown(void) {
    if (g_config.dirty) {
        config_save();
    }

    memset(&g_config, 0, sizeof(GameConfig));
    LOG_INFO("Configuration shutdown");
}

/*
 * Load default configuration
 */
int config_load_default(void) {
    /* Graphics defaults */
    g_config.graphics.screen_width = 640;
    g_config.graphics.screen_height = 480;
    g_config.graphics.color_depth = 16;
    g_config.graphics.fullscreen = 0;
    g_config.graphics.vsync = 1;
    g_config.graphics.gamma = 50;
    g_config.graphics.detail_level = 2;
    g_config.graphics.shadow_quality = 1;
    g_config.graphics.weather_effects = 1;

    /* Sound defaults */
    g_config.sound.master_volume = 100;
    g_config.sound.music_volume = 80;
    g_config.sound.sfx_volume = 100;
    g_config.sound.ambient_volume = 70;
    g_config.sound.voice_volume = 100;
    g_config.sound.mute_audio = 0;
    g_config.sound.mute_music = 0;

    /* Network defaults */
    strcpy(g_config.network.server_host, "localhost");
    g_config.network.server_port = 4567;
    g_config.network.backup_port = 4568;
    g_config.network.connection_timeout = 30;
    g_config.network.retry_count = 3;
    g_config.network.use_proxy = 0;

    /* Game defaults */
    g_config.game.language = 0;
    g_config.game.auto_attack = 1;
    g_config.game.auto_pickup = 0;
    g_config.game.show_damage = 1;
    g_config.game.show_hp_bar = 1;
    g_config.game.show_names = 1;
    g_config.game.show_minimap = 1;
    g_config.game.camera_mode = 0;
    g_config.game.chat_filter = 0xFF;
    g_config.game.profanity_filter = 1;
    g_config.game.auto_save = 1;
    g_config.game.save_interval = 5;
    g_config.game.fps_limit = 60;

    /* Key bindings */
    memcpy(&g_config.keys, &s_default_keys, sizeof(KeyBindings));

    strcpy(g_config.config_path, CONFIG_FILE);

    return 1;
}

/*
 * Load configuration from file
 */
int config_load(void) {
    FILE* fp;
    char line[MAX_CONFIG_LINE];
    char section[64] = "";
    char key[64], value[MAX_CONFIG_VALUE];

    fp = fopen(g_config.config_path, "r");
    if (!fp) {
        LOG_DEBUG("Config file not found: %s", g_config.config_path);
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        char* ptr = line;

        /* Skip whitespace */
        while (*ptr == ' ' || *ptr == '\t') ptr++;

        /* Skip empty lines and comments */
        if (*ptr == '\n' || *ptr == '\r' || *ptr == '#' || *ptr == ';') {
            continue;
        }

        /* Section header */
        if (*ptr == '[') {
            char* end = strchr(ptr, ']');
            if (end) {
                *end = '\0';
                strncpy(section, ptr + 1, sizeof(section) - 1);
            }
            continue;
        }

        /* Key = Value */
        char* eq = strchr(ptr, '=');
        if (eq) {
            *eq = '\0';
            strncpy(key, ptr, sizeof(key) - 1);

            /* Trim key */
            char* k = key + strlen(key) - 1;
            while (k > key && (*k == ' ' || *k == '\t')) *k-- = '\0';

            /* Get value */
            ptr = eq + 1;
            while (*ptr == ' ' || *ptr == '\t') ptr++;

            /* Remove trailing whitespace/newline */
            char* v = ptr + strlen(ptr) - 1;
            while (v > ptr && (*v == '\n' || *v == '\r' || *v == ' ' || *v == '\t')) {
                *v-- = '\0';
            }

            strncpy(value, ptr, sizeof(value) - 1);

            /* Apply setting */
            config_set_string(section, key, value);
        }
    }

    fclose(fp);
    g_config.loaded = 1;

    LOG_INFO("Configuration loaded from %s", g_config.config_path);
    return 1;
}

/*
 * Save configuration to file
 */
int config_save(void) {
    FILE* fp;
    char path[128];

    snprintf(path, sizeof(path), "%s", g_config.config_path);

    fp = fopen(path, "w");
    if (!fp) {
        LOG_ERROR("Failed to save config: %s", path);
        return 0;
    }

    /* [Graphics] */
    fprintf(fp, "[Graphics]\n");
    fprintf(fp, "Width=%d\n", g_config.graphics.screen_width);
    fprintf(fp, "Height=%d\n", g_config.graphics.screen_height);
    fprintf(fp, "ColorDepth=%d\n", g_config.graphics.color_depth);
    fprintf(fp, "Fullscreen=%d\n", g_config.graphics.fullscreen);
    fprintf(fp, "VSync=%d\n", g_config.graphics.vsync);
    fprintf(fp, "Gamma=%d\n", g_config.graphics.gamma);
    fprintf(fp, "DetailLevel=%d\n", g_config.graphics.detail_level);
    fprintf(fp, "ShadowQuality=%d\n", g_config.graphics.shadow_quality);
    fprintf(fp, "WeatherEffects=%d\n", g_config.graphics.weather_effects);
    fprintf(fp, "\n");

    /* [Sound] */
    fprintf(fp, "[Sound]\n");
    fprintf(fp, "MasterVolume=%d\n", g_config.sound.master_volume);
    fprintf(fp, "MusicVolume=%d\n", g_config.sound.music_volume);
    fprintf(fp, "SFXVolume=%d\n", g_config.sound.sfx_volume);
    fprintf(fp, "AmbientVolume=%d\n", g_config.sound.ambient_volume);
    fprintf(fp, "VoiceVolume=%d\n", g_config.sound.voice_volume);
    fprintf(fp, "MuteAudio=%d\n", g_config.sound.mute_audio);
    fprintf(fp, "MuteMusic=%d\n", g_config.sound.mute_music);
    fprintf(fp, "\n");

    /* [Network] */
    fprintf(fp, "[Network]\n");
    fprintf(fp, "ServerHost=%s\n", g_config.network.server_host);
    fprintf(fp, "ServerPort=%d\n", g_config.network.server_port);
    fprintf(fp, "ConnectionTimeout=%d\n", g_config.network.connection_timeout);
    fprintf(fp, "\n");

    /* [Game] */
    fprintf(fp, "[Game]\n");
    fprintf(fp, "Language=%d\n", g_config.game.language);
    fprintf(fp, "AutoAttack=%d\n", g_config.game.auto_attack);
    fprintf(fp, "AutoPickup=%d\n", g_config.game.auto_pickup);
    fprintf(fp, "ShowDamage=%d\n", g_config.game.show_damage);
    fprintf(fp, "ShowHPBar=%d\n", g_config.game.show_hp_bar);
    fprintf(fp, "ShowNames=%d\n", g_config.game.show_names);
    fprintf(fp, "ShowMinimap=%d\n", g_config.game.show_minimap);
    fprintf(fp, "FPSLimit=%d\n", g_config.game.fps_limit);
    fprintf(fp, "\n");

    /* [Keys] */
    fprintf(fp, "[Keys]\n");
    fprintf(fp, "MoveUp=%d\n", g_config.keys.move_up);
    fprintf(fp, "MoveDown=%d\n", g_config.keys.move_down);
    fprintf(fp, "MoveLeft=%d\n", g_config.keys.move_left);
    fprintf(fp, "MoveRight=%d\n", g_config.keys.move_right);
    fprintf(fp, "Attack=%d\n", g_config.keys.attack);
    fprintf(fp, "Skill1=%d\n", g_config.keys.skill1);
    fprintf(fp, "Skill2=%d\n", g_config.keys.skill2);
    fprintf(fp, "Skill3=%d\n", g_config.keys.skill3);
    fprintf(fp, "\n");

    fclose(fp);
    g_config.dirty = 0;

    LOG_INFO("Configuration saved to %s", path);
    return 1;
}

/*
 * Get integer config value
 */
int config_get_int(const char* section, const char* key, int default_val) {
    /* Simple implementation - just return defaults for now */
    if (strcmp(section, "Graphics") == 0) {
        if (strcmp(key, "Width") == 0) return g_config.graphics.screen_width;
        if (strcmp(key, "Height") == 0) return g_config.graphics.screen_height;
        if (strcmp(key, "ColorDepth") == 0) return g_config.graphics.color_depth;
        if (strcmp(key, "Fullscreen") == 0) return g_config.graphics.fullscreen;
        if (strcmp(key, "VSync") == 0) return g_config.graphics.vsync;
        if (strcmp(key, "Gamma") == 0) return g_config.graphics.gamma;
    }
    else if (strcmp(section, "Sound") == 0) {
        if (strcmp(key, "MasterVolume") == 0) return g_config.sound.master_volume;
        if (strcmp(key, "MusicVolume") == 0) return g_config.sound.music_volume;
        if (strcmp(key, "SFXVolume") == 0) return g_config.sound.sfx_volume;
    }
    else if (strcmp(section, "Network") == 0) {
        if (strcmp(key, "ServerPort") == 0) return g_config.network.server_port;
    }
    else if (strcmp(section, "Game") == 0) {
        if (strcmp(key, "Language") == 0) return g_config.game.language;
        if (strcmp(key, "AutoAttack") == 0) return g_config.game.auto_attack;
        if (strcmp(key, "FPSLimit") == 0) return g_config.game.fps_limit;
    }

    return default_val;
}

/*
 * Get string config value
 */
const char* config_get_string(const char* section, const char* key, const char* default_val) {
    if (strcmp(section, "Network") == 0) {
        if (strcmp(key, "ServerHost") == 0) return g_config.network.server_host;
    }

    return default_val;
}

/*
 * Set integer config value
 */
int config_set_int(const char* section, const char* key, int value) {
    g_config.dirty = 1;

    if (strcmp(section, "Graphics") == 0) {
        if (strcmp(key, "Width") == 0) { g_config.graphics.screen_width = value; return 1; }
        if (strcmp(key, "Height") == 0) { g_config.graphics.screen_height = value; return 1; }
        if (strcmp(key, "ColorDepth") == 0) { g_config.graphics.color_depth = value; return 1; }
        if (strcmp(key, "Fullscreen") == 0) { g_config.graphics.fullscreen = value; return 1; }
        if (strcmp(key, "VSync") == 0) { g_config.graphics.vsync = value; return 1; }
        if (strcmp(key, "Gamma") == 0) { g_config.graphics.gamma = value; return 1; }
    }
    else if (strcmp(section, "Sound") == 0) {
        if (strcmp(key, "MasterVolume") == 0) { g_config.sound.master_volume = value; return 1; }
        if (strcmp(key, "MusicVolume") == 0) { g_config.sound.music_volume = value; return 1; }
        if (strcmp(key, "SFXVolume") == 0) { g_config.sound.sfx_volume = value; return 1; }
    }
    else if (strcmp(section, "Game") == 0) {
        if (strcmp(key, "Language") == 0) { g_config.game.language = value; return 1; }
        if (strcmp(key, "FPSLimit") == 0) { g_config.game.fps_limit = value; return 1; }
    }

    return 0;
}

/*
 * Set string config value
 */
int config_set_string(const char* section, const char* key, const char* value) {
    g_config.dirty = 1;

    if (strcmp(section, "Network") == 0) {
        if (strcmp(key, "ServerHost") == 0) {
            strncpy(g_config.network.server_host, value, sizeof(g_config.network.server_host) - 1);
            return 1;
        }
    }

    return 0;
}

/*
 * Set resolution
 */
void config_set_resolution(u16 width, u16 height) {
    g_config.graphics.screen_width = width;
    g_config.graphics.screen_height = height;
    g_config.dirty = 1;
}

/*
 * Get resolution
 */
void config_get_resolution(u16* width, u16* height) {
    *width = g_config.graphics.screen_width;
    *height = g_config.graphics.screen_height;
}

/*
 * Set fullscreen
 */
void config_set_fullscreen(u8 fullscreen) {
    g_config.graphics.fullscreen = fullscreen;
    g_config.dirty = 1;
}

/*
 * Is fullscreen
 */
u8 config_is_fullscreen(void) {
    return g_config.graphics.fullscreen;
}

/*
 * Set master volume
 */
void config_set_master_volume(u8 volume) {
    g_config.sound.master_volume = volume > 100 ? 100 : volume;
    g_config.dirty = 1;
}

/*
 * Get master volume
 */
u8 config_get_master_volume(void) {
    return g_config.sound.master_volume;
}

/*
 * Set music volume
 */
void config_set_music_volume(u8 volume) {
    g_config.sound.music_volume = volume > 100 ? 100 : volume;
    g_config.dirty = 1;
}

/*
 * Get music volume
 */
u8 config_get_music_volume(void) {
    return g_config.sound.music_volume;
}

/*
 * Set SFX volume
 */
void config_set_sfx_volume(u8 volume) {
    g_config.sound.sfx_volume = volume > 100 ? 100 : volume;
    g_config.dirty = 1;
}

/*
 * Get SFX volume
 */
u8 config_get_sfx_volume(void) {
    return g_config.sound.sfx_volume;
}

/*
 * Set server
 */
void config_set_server(const char* host, u16 port) {
    strncpy(g_config.network.server_host, host, sizeof(g_config.network.server_host) - 1);
    g_config.network.server_port = port;
    g_config.dirty = 1;
}

/*
 * Get server host
 */
const char* config_get_server_host(void) {
    return g_config.network.server_host;
}

/*
 * Get server port
 */
u16 config_get_server_port(void) {
    return g_config.network.server_port;
}

/*
 * Set key binding
 */
int config_set_key_binding(const char* action, u32 key) {
    g_config.dirty = 1;

    if (strcmp(action, "MoveUp") == 0) { g_config.keys.move_up = key; return 1; }
    if (strcmp(action, "MoveDown") == 0) { g_config.keys.move_down = key; return 1; }
    if (strcmp(action, "MoveLeft") == 0) { g_config.keys.move_left = key; return 1; }
    if (strcmp(action, "MoveRight") == 0) { g_config.keys.move_right = key; return 1; }
    if (strcmp(action, "Attack") == 0) { g_config.keys.attack = key; return 1; }
    if (strcmp(action, "Skill1") == 0) { g_config.keys.skill1 = key; return 1; }
    if (strcmp(action, "Skill2") == 0) { g_config.keys.skill2 = key; return 1; }
    if (strcmp(action, "Skill3") == 0) { g_config.keys.skill3 = key; return 1; }

    return 0;
}

/*
 * Get key binding
 */
u32 config_get_key_binding(const char* action) {
    if (strcmp(action, "MoveUp") == 0) return g_config.keys.move_up;
    if (strcmp(action, "MoveDown") == 0) return g_config.keys.move_down;
    if (strcmp(action, "MoveLeft") == 0) return g_config.keys.move_left;
    if (strcmp(action, "MoveRight") == 0) return g_config.keys.move_right;
    if (strcmp(action, "Attack") == 0) return g_config.keys.attack;
    if (strcmp(action, "Skill1") == 0) return g_config.keys.skill1;
    if (strcmp(action, "Skill2") == 0) return g_config.keys.skill2;
    if (strcmp(action, "Skill3") == 0) return g_config.keys.skill3;

    return 0;
}

/*
 * Reset key bindings
 */
int config_reset_key_bindings(void) {
    memcpy(&g_config.keys, &s_default_keys, sizeof(KeyBindings));
    g_config.dirty = 1;
    return 1;
}

/*
 * Simple XOR password encryption
 */
static void config_encrypt_password(const char* src, char* dst, int max_len) {
    int i;
    size_t src_len = strlen(src);

    for (i = 0; i < (int)src_len && i < max_len - 1; i++) {
        dst[i] = src[i] ^ s_encrypt_key[i % ENCRYPT_KEY_LEN];
    }
    dst[i] = '\0';
}

/*
 * Simple XOR password decryption
 */
static void config_decrypt_password(const char* src, char* dst, int max_len) {
    int i;
    size_t src_len = strlen(src);

    for (i = 0; i < (int)src_len && i < max_len - 1; i++) {
        dst[i] = src[i] ^ s_encrypt_key[i % ENCRYPT_KEY_LEN];
    }
    dst[i] = '\0';
}

/*
 * Save account info
 */
int config_save_account(const char* username, const char* password, int remember) {
    FILE* fp;

    if (!remember) {
        config_clear_account();
        return 1;
    }

    fp = fopen(ACCOUNT_FILE, "wb");
    if (!fp) return 0;

    strncpy(g_config.account.username, username, sizeof(g_config.account.username) - 1);

    /* Encrypt password before storing */
    config_encrypt_password(password, g_config.account.encrypted_pass,
                           sizeof(g_config.account.encrypted_pass));
    g_config.account.remember_pass = 1;

    fwrite(&g_config.account, sizeof(AccountInfo), 1, fp);
    fclose(fp);

    LOG_DEBUG("Account info saved");
    return 1;
}

/*
 * Load account info
 */
int config_load_account(char* username, char* password, int max_len) {
    FILE* fp;
    char decrypted[48];

    if (!g_config.account.remember_pass) {
        return 0;
    }

    fp = fopen(ACCOUNT_FILE, "rb");
    if (!fp) return 0;

    fread(&g_config.account, sizeof(AccountInfo), 1, fp);
    fclose(fp);

    strncpy(username, g_config.account.username, max_len - 1);

    /* Decrypt password after loading */
    config_decrypt_password(g_config.account.encrypted_pass, decrypted, sizeof(decrypted));
    strncpy(password, decrypted, max_len - 1);

    return 1;
}

/*
 * Clear saved account
 */
void config_clear_account(void) {
    memset(&g_config.account, 0, sizeof(AccountInfo));
    remove(ACCOUNT_FILE);
}

/*
 * Check if has saved account
 */
int config_has_saved_account(void) {
    return g_config.account.remember_pass && g_config.account.username[0] != '\0';
}

/*
 * Set language
 */
void config_set_language(u8 lang) {
    g_config.game.language = lang;
    g_config.dirty = 1;
}

/*
 * Get language
 */
u8 config_get_language(void) {
    return g_config.game.language;
}

/*
 * Set auto attack
 */
void config_set_auto_attack(u8 enable) {
    g_config.game.auto_attack = enable;
    g_config.dirty = 1;
}

/*
 * Get auto attack
 */
u8 config_get_auto_attack(void) {
    return g_config.game.auto_attack;
}

/*
 * Set FPS limit
 */
void config_set_fps_limit(u16 fps) {
    g_config.game.fps_limit = fps > 0 ? fps : 60;
    g_config.dirty = 1;
}

/*
 * Get FPS limit
 */
u16 config_get_fps_limit(void) {
    return g_config.game.fps_limit;
}

/*
 * Reset all to defaults
 */
void config_reset_defaults(void) {
    config_load_default();
    g_config.dirty = 1;
}

/*
 * Apply configuration to all running systems
 */
void config_apply(void) {
    /* Apply graphics settings */
    graphics_set_resolution(g_config.graphics.screen_width,
                            g_config.graphics.screen_height);
    graphics_set_fullscreen(g_config.graphics.fullscreen);
    graphics_set_gamma(g_config.graphics.gamma);
    graphics_set_vsync(g_config.graphics.vsync);

    /* Apply sound settings */
    audio_set_master_volume(g_config.sound.master_volume);
    audio_set_music_volume(g_config.sound.music_volume);
    audio_set_sfx_volume(g_config.sound.sfx_volume);
    audio_set_ambient_volume(g_config.sound.ambient_volume);
    audio_set_mute(g_config.sound.mute_audio);

    /* Apply minimap settings */
    if (g_config.game.show_minimap) {
        minimap_show();
    } else {
        minimap_hide();
    }

    /* Apply game settings */
    /* Auto-attack, auto-pickup, etc. would be applied to game state */

    LOG_DEBUG("Configuration applied");
}
