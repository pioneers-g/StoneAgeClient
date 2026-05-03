/*
 * Stone Age Client - Audio Manager Implementation
 * Reverse engineered from sa_9061.exe
 * Handles BGM (data/bgm/*.wav) and SE (data/se/*.wav)
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mmsystem.h>
#include "types.h"
#include "audiomanager.h"
#include "logger.h"

/* Global audio context */
AudioManagerContext g_audio = {0};

/*
 * Initialize audio manager
 */
int audiomanager_init(void) {
    memset(&g_audio, 0, sizeof(AudioManagerContext));

    g_audio.bgm_enabled = 1;
    g_audio.se_enabled = 1;
    g_audio.bgm_volume = 100;
    g_audio.se_volume = 100;
    g_audio.current_bgm = -1;

    /* Initialize sound channels */
    g_audio.channel_count = MAX_SOUND_CHANNELS;

    LOG_INFO("Audio manager initialized");
    return 1;
}

/*
 * Shutdown audio manager
 */
void audiomanager_shutdown(void) {
    /* Stop all sounds */
    audiomanager_stop_bgm();
    audiomanager_stop_all_se();

    memset(&g_audio, 0, sizeof(AudioManagerContext));
    LOG_INFO("Audio manager shutdown");
}

/*
 * Play background music
 */
int audiomanager_play_bgm(int bgm_id) {
    char path[256];

    if (!g_audio.bgm_enabled) return 0;
    if (bgm_id < 0 || bgm_id >= MAX_BGM) return 0;

    /* Stop current BGM */
    audiomanager_stop_bgm();

    /* Build path - based on reverse engineering: data\bgm\sabgm_*.wav */
    snprintf(path, sizeof(path), "data\\bgm\\sabgm_%s.wav", audiomanager_get_bgm_name(bgm_id));

    /* Play using DirectSound or Win32 multimedia */
    if (sndPlaySoundA(path, SND_ASYNC | SND_LOOP)) {
        g_audio.current_bgm = bgm_id;
        g_audio.bgm_playing = 1;
        strncpy(g_audio.current_bgm_path, path, sizeof(g_audio.current_bgm_path) - 1);
        LOG_DEBUG("Playing BGM: %s", path);
        return 1;
    }

    LOG_WARN("Failed to play BGM: %s", path);
    return 0;
}

/*
 * Stop background music
 */
void audiomanager_stop_bgm(void) {
    sndPlaySoundA(NULL, 0);
    g_audio.current_bgm = -1;
    g_audio.bgm_playing = 0;
}

/*
 * Pause background music
 */
void audiomanager_pause_bgm(void) {
    if (g_audio.bgm_playing) {
        sndPlaySoundA(NULL, 0);
        g_audio.bgm_paused = 1;
    }
}

/*
 * Resume background music
 */
void audiomanager_resume_bgm(void) {
    if (g_audio.bgm_paused && g_audio.current_bgm_path[0]) {
        sndPlaySoundA(g_audio.current_bgm_path, SND_ASYNC | SND_LOOP);
        g_audio.bgm_paused = 0;
    }
}

/*
 * Check if BGM playing
 */
int audiomanager_is_bgm_playing(void) {
    return g_audio.bgm_playing && !g_audio.bgm_paused;
}

/*
 * Play sound effect
 */
int audiomanager_play_se(int se_id) {
    char path[256];
    int channel;

    if (!g_audio.se_enabled) return 0;
    if (se_id < 0 || se_id >= MAX_SE) return 0;

    /* Find free channel */
    channel = audiomanager_find_free_channel();
    if (channel < 0) {
        /* All channels busy, use oldest */
        channel = 0;
        audiomanager_stop_channel(channel);
    }

    /* Build path */
    snprintf(path, sizeof(path), "data\\se\\%s.wav", audiomanager_get_se_name(se_id));

    /* Play sound effect */
    SoundChannel* ch = &g_audio.channels[channel];
    strncpy(ch->path, path, sizeof(ch->path) - 1);
    ch->playing = 1;
    ch->start_time = timeGetTime();
    ch->sound_id = se_id;

    /* Use PlaySound for simple playback */
    PlaySoundA(path, NULL, SND_ASYNC | SND_FILENAME);

    g_audio.se_play_count++;

    LOG_DEBUG("Playing SE: %s on channel %d", path, channel);
    return 1;
}

/*
 * Play 3D sound effect
 */
int audiomanager_play_se_3d(int se_id, int x, int y, int listener_x, int listener_y) {
    int distance;
    int volume;
    int pan;

    if (!g_audio.se_enabled) return 0;

    /* Calculate distance-based volume */
    distance = audiomanager_calculate_distance(x, y, listener_x, listener_y);
    volume = audiomanager_distance_to_volume(distance);
    pan = audiomanager_calculate_pan(x, listener_x);

    /* Apply volume and play */
    /* Note: Full 3D audio would require DirectSound 3D buffers */
    audiomanager_play_se(se_id);

    return 1;
}

/*
 * Stop all sound effects
 */
void audiomanager_stop_all_se(void) {
    int i;

    for (i = 0; i < g_audio.channel_count; i++) {
        g_audio.channels[i].playing = 0;
    }

    PlaySoundA(NULL, NULL, 0);
}

/*
 * Stop specific channel
 */
void audiomanager_stop_channel(int channel) {
    if (channel >= 0 && channel < MAX_SOUND_CHANNELS) {
        g_audio.channels[channel].playing = 0;
    }
}

/*
 * Set BGM volume (0-100)
 */
void audiomanager_set_bgm_volume(int volume) {
    g_audio.bgm_volume = volume > 100 ? 100 : volume;
    /* Would need DirectSound buffer for actual volume control */
}

/*
 * Set SE volume (0-100)
 */
void audiomanager_set_se_volume(int volume) {
    g_audio.se_volume = volume > 100 ? 100 : volume;
}

/*
 * Get BGM volume
 */
int audiomanager_get_bgm_volume(void) {
    return g_audio.bgm_volume;
}

/*
 * Get SE volume
 */
int audiomanager_get_se_volume(void) {
    return g_audio.se_volume;
}

/*
 * Enable/disable BGM
 */
void audiomanager_set_bgm_enabled(int enabled) {
    g_audio.bgm_enabled = enabled;
    if (!enabled) {
        audiomanager_stop_bgm();
    }
}

/*
 * Enable/disable SE
 */
void audiomanager_set_se_enabled(int enabled) {
    g_audio.se_enabled = enabled;
    if (!enabled) {
        audiomanager_stop_all_se();
    }
}

/*
 * Check if BGM enabled
 */
int audiomanager_is_bgm_enabled(void) {
    return g_audio.bgm_enabled;
}

/*
 * Check if SE enabled
 */
int audiomanager_is_se_enabled(void) {
    return g_audio.se_enabled;
}

/*
 * Update audio manager
 */
void audiomanager_update(void) {
    int i;
    u32 current_time;

    /* Update channel states */
    current_time = timeGetTime();

    for (i = 0; i < g_audio.channel_count; i++) {
        if (g_audio.channels[i].playing) {
            /* Check if sound finished (approximate) */
            /* In real implementation, would query DirectSound buffer status */
        }
    }
}

/*
 * Get current BGM ID
 */
int audiomanager_get_current_bgm(void) {
    return g_audio.current_bgm;
}

/*
 * Fade out BGM
 */
void audiomanager_fade_out_bgm(u32 duration_ms) {
    u32 start_time = timeGetTime();
    int start_volume = g_audio.bgm_volume;
    u32 elapsed;

    while ((elapsed = timeGetTime() - start_time) < duration_ms) {
        int volume = start_volume - (start_volume * elapsed / duration_ms);
        audiomanager_set_bgm_volume(volume);
        Sleep(10);
    }

    audiomanager_stop_bgm();
}

/*
 * Fade in BGM
 */
void audiomanager_fade_in_bgm(int bgm_id, u32 duration_ms) {
    u32 start_time = timeGetTime();
    int target_volume = g_audio.bgm_volume;
    u32 elapsed;

    g_audio.bgm_volume = 0;
    audiomanager_play_bgm(bgm_id);

    while ((elapsed = timeGetTime() - start_time) < duration_ms) {
        int volume = (target_volume * elapsed / duration_ms);
        audiomanager_set_bgm_volume(volume);
        Sleep(10);
    }

    audiomanager_set_bgm_volume(target_volume);
}

/*
 * Find free sound channel
 */
int audiomanager_find_free_channel(void) {
    int i;

    for (i = 0; i < g_audio.channel_count; i++) {
        if (!g_audio.channels[i].playing) {
            return i;
        }
    }

    return -1;
}

/*
 * Get BGM name by ID
 */
const char* audiomanager_get_bgm_name(int bgm_id) {
    /* BGM names from reverse engineering */
    static const char* bgm_names[] = {
        "s0", "s1",           /* Start */
        "d1", "d2",           /* Dungeon */
        "f2",                 /* Field */
        "b2",                 /* Battle */
        "t2", "t12"           /* Town */
    };

    if (bgm_id >= 0 && bgm_id < sizeof(bgm_names) / sizeof(bgm_names[0])) {
        return bgm_names[bgm_id];
    }

    return "s0";
}

/*
 * Get SE name by ID
 */
const char* audiomanager_get_se_name(int se_id) {
    /* SE names from reverse engineering - patterns: sa1_*, sa2_*, sa3_*, saam_*, sas_*, sax_* */
    static char buffer[32];

    if (se_id < 100) {
        snprintf(buffer, sizeof(buffer), "sa1_%02d", se_id);
    } else if (se_id < 200) {
        snprintf(buffer, sizeof(buffer), "sa2_%02d", se_id - 100);
    } else if (se_id < 300) {
        snprintf(buffer, sizeof(buffer), "sa3_%02d", se_id - 200);
    } else if (se_id < 400) {
        snprintf(buffer, sizeof(buffer), "saam_%02d", se_id - 300);
    } else if (se_id < 500) {
        snprintf(buffer, sizeof(buffer), "sas_%02d", se_id - 400);
    } else if (se_id < 600) {
        snprintf(buffer, sizeof(buffer), "sax_%02d", se_id - 500);
    } else {
        snprintf(buffer, sizeof(buffer), "sa1_%02d", se_id % 100);
    }

    return buffer;
}

/*
 * Calculate distance between two points
 */
int audiomanager_calculate_distance(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    return (int)sqrt((double)(dx * dx + dy * dy));
}

/*
 * Convert distance to volume
 */
int audiomanager_distance_to_volume(int distance) {
    /* Max audible distance is about 1000 units */
    if (distance >= 1000) return 0;
    return (1000 - distance) * 100 / 1000;
}

/*
 * Calculate stereo pan
 */
int audiomanager_calculate_pan(int source_x, int listener_x) {
    int diff = source_x - listener_x;
    /* Pan: -10000 (left) to 10000 (right) */
    if (diff < -500) return -10000;
    if (diff > 500) return 10000;
    return diff * 20;
}

/*
 * Preload sound effect
 */
int audiomanager_preload_se(int se_id) {
    char path[256];

    if (se_id < 0 || se_id >= MAX_SE) return 0;

    snprintf(path, sizeof(path), "data\\se\\%s.wav", audiomanager_get_se_name(se_id));

    /* Mark as preloaded (real implementation would load into memory) */
    g_audio.preloaded_se[se_id] = 1;

    return 1;
}

/*
 * Unload sound effect
 */
void audiomanager_unload_se(int se_id) {
    if (se_id >= 0 && se_id < MAX_SE) {
        g_audio.preloaded_se[se_id] = 0;
    }
}

/*
 * Play BGM by map ID
 */
int audiomanager_play_bgm_for_map(int map_id) {
    /* Map ID to BGM mapping from reverse engineering */
    int bgm_id;

    if (map_id < 100) {
        bgm_id = 0;  /* Starting area */
    } else if (map_id < 200) {
        bgm_id = 5;  /* Field */
    } else if (map_id < 300) {
        bgm_id = 3;  /* Dungeon */
    } else if (map_id < 400) {
        bgm_id = 6;  /* Town */
    } else {
        bgm_id = 0;
    }

    return audiomanager_play_bgm(bgm_id);
}

/*
 * Get SE play count
 */
u32 audiomanager_get_se_play_count(void) {
    return g_audio.se_play_count;
}

/*
 * Reset SE play count
 */
void audiomanager_reset_se_play_count(void) {
    g_audio.se_play_count = 0;
}
