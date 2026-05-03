/*
 * Stone Age Client - Audio Manager Header
 * Reverse engineered from sa_9061.exe
 * Handles BGM (data/bgm/*.wav) and SE (data/se/*.wav)
 */

#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include "types.h"

/* Constants */
#define MAX_BGM             20
#define MAX_SE              1000
#define MAX_SOUND_CHANNELS  16

/* Sound channel */
typedef struct {
    char path[256];
    int sound_id;
    u32 start_time;
    u32 duration;
    int playing;
    int volume;
    int pan;

} SoundChannel;

/* Audio manager context */
typedef struct {
    /* BGM state */
    int current_bgm;
    char current_bgm_path[256];
    int bgm_playing;
    int bgm_paused;
    int bgm_enabled;
    int bgm_volume;

    /* SE state */
    int se_enabled;
    int se_volume;
    int preloaded_se[MAX_SE];
    u32 se_play_count;

    /* Sound channels */
    SoundChannel channels[MAX_SOUND_CHANNELS];
    int channel_count;

} AudioManagerContext;

/* Global audio context */
extern AudioManagerContext g_audio;

/* Initialization */
int audiomanager_init(void);
void audiomanager_shutdown(void);

/* BGM */
int audiomanager_play_bgm(int bgm_id);
void audiomanager_stop_bgm(void);
void audiomanager_pause_bgm(void);
void audiomanager_resume_bgm(void);
int audiomanager_is_bgm_playing(void);
int audiomanager_get_current_bgm(void);
int audiomanager_play_bgm_for_map(int map_id);

/* BGM fading */
void audiomanager_fade_out_bgm(u32 duration_ms);
void audiomanager_fade_in_bgm(int bgm_id, u32 duration_ms);

/* Sound effects */
int audiomanager_play_se(int se_id);
int audiomanager_play_se_3d(int se_id, int x, int y, int listener_x, int listener_y);
void audiomanager_stop_all_se(void);
void audiomanager_stop_channel(int channel);

/* Volume */
void audiomanager_set_bgm_volume(int volume);
void audiomanager_set_se_volume(int volume);
int audiomanager_get_bgm_volume(void);
int audiomanager_get_se_volume(void);

/* Enable/disable */
void audiomanager_set_bgm_enabled(int enabled);
void audiomanager_set_se_enabled(int enabled);
int audiomanager_is_bgm_enabled(void);
int audiomanager_is_se_enabled(void);

/* Update */
void audiomanager_update(void);

/* Channels */
int audiomanager_find_free_channel(void);

/* Names */
const char* audiomanager_get_bgm_name(int bgm_id);
const char* audiomanager_get_se_name(int se_id);

/* 3D audio helpers */
int audiomanager_calculate_distance(int x1, int y1, int x2, int y2);
int audiomanager_distance_to_volume(int distance);
int audiomanager_calculate_pan(int source_x, int listener_x);

/* Preloading */
int audiomanager_preload_se(int se_id);
void audiomanager_unload_se(int se_id);

/* Statistics */
u32 audiomanager_get_se_play_count(void);
void audiomanager_reset_se_play_count(void);

#endif /* AUDIOMANAGER_H */
