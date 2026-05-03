/*
 * Stone Age Client - Sound System
 * Reverse engineered from sa_9061.exe
 * DirectSound audio playback with WAV loading
 */

#ifndef SOUND_H
#define SOUND_H

#include "types.h"

/* Sound buffer limits */
#define MAX_SOUND_BUFFERS 64
#define MAX_SOUND_PATH 260

/* Sound flags */
#define SOUND_FLAG_LOOP    0x0001
#define SOUND_FLAG_3D      0x0002
#define SOUND_FLAG_STREAM  0x0004

/* BGM IDs - from sa_9061.exe string analysis */
#define BGM_ID_S0          0    /* sabgm_s0.wav - Special 0 */
#define BGM_ID_S1          1    /* sabgm_s1.wav - Special 1 */
#define BGM_ID_D1          2    /* sabgm_d1.wav - Dungeon 1 */
#define BGM_ID_D2          3    /* sabgm_d2.wav - Dungeon 2 */
#define BGM_ID_F2          4    /* sabgm_f2.wav - Field 2 */
#define BGM_ID_B2          5    /* sabgm_b2.wav - Battle 2 */
#define BGM_ID_T2          6    /* sabgm_t2.wav - Title 2 */
#define BGM_ID_T12         7    /* sabgm_t12.wav - Title 12 */
#define BGM_ID_COUNT       8

/* Sound Effect Series - from sa_9061.exe string analysis */
/* Series 1: sa1_xx.wav (01-23) */
#define SE_SERIES_1_START    0
#define SE_SERIES_1_END      22
#define SE_SA1_01            0
#define SE_SA1_02            1
#define SE_SA1_03            2
#define SE_SA1_06            3
#define SE_SA1_10            4
#define SE_SA1_11            5
#define SE_SA1_12            6
#define SE_SA1_13            7
#define SE_SA1_14            8
#define SE_SA1_15            9
#define SE_SA1_16            10
#define SE_SA1_17            11
#define SE_SA1_20            12
#define SE_SA1_21            13
#define SE_SA1_22            14
#define SE_SA1_23            15

/* Series 2: sa2_xx.wav */
#define SE_SERIES_2_START    100
#define SE_SA2_12            100

/* Series 3: sa3_xx.wav (01-02) */
#define SE_SERIES_3_START    200
#define SE_SA3_01            200
#define SE_SA3_02            201

/* Ambient: saam_xx.wav (11-27) */
#define SE_AMBIENT_START     300
#define SE_SAAM_11           300
#define SE_SAAM_12           301
#define SE_SAAM_13           302
#define SE_SAAM_14           303
#define SE_SAAM_15           304
#define SE_SAAM_16           305
#define SE_SAAM_17           306
#define SE_SAAM_18           307
#define SE_SAAM_19           308
#define SE_SAAM_20           309
#define SE_SAAM_21           310
#define SE_SAAM_22           311
#define SE_SAAM_23           312
#define SE_SAAM_24           313
#define SE_SAAM_25           314
#define SE_SAAM_26           315
#define SE_SAAM_27           316

/* Special: sas_xx.wav (33-35) */
#define SE_SPECIAL_START     400
#define SE_SAS_33            400
#define SE_SAS_34            401
#define SE_SAS_35            402

/* Extended: sax_xx.wav (01-03) */
#define SE_EXTENDED_START    500
#define SE_SAX_01            500
#define SE_SAX_02            501
#define SE_SAX_03            502

/* Sound structure */
typedef struct {
    u32 id;
    char path[MAX_SOUND_PATH];
    u32 flags;
    s32 volume;
    s32 pan;
    u32 frequency;
    void* buffer;
    u32 buffer_size;
    u8 is_playing;
    u8 is_loaded;
    u8 is_streaming;
    u8 reserved[1];
} Sound;

/* Streaming BGM state - from FUN_00487ba0 analysis */
typedef struct {
    void* buffer;           /* Secondary buffer for streaming */
    void* notify;           /* IDirectSoundNotify */
    HANDLE events[3];       /* Notification events */
    HANDLE thread;          /* Streaming thread handle */
    u32 thread_id;          /* Thread ID */
    u32 buffer_size;        /* Size of each buffer half */
    u32 data_size;          /* Total data size */
    u32 bytes_per_sec;      /* Bytes per second */
    volatile u8 running;    /* Thread running flag */
    u8 initialized;         /* Initialization flag */
    u8 fade_active;         /* Fade in progress */
    u8 reserved[1];
} StreamingBGM;

/* Sound context */
typedef struct {
    void* direct_sound;
    void* primary_buffer;
    Sound sounds[MAX_SOUND_BUFFERS];
    u32 sound_count;
    s32 master_volume;
    u8 sound_enabled;
    u8 music_enabled;
    u8 streaming_active;    /* Streaming BGM active flag */
    u8 reserved[1];
    StreamingBGM streaming; /* Streaming BGM state */
} SoundContext;

/* Global sound context */
extern SoundContext g_sound;

/* Sound system functions */
int sound_init(HWND hwnd);
void sound_shutdown(void);

/* Sound loading */
int sound_load(u32 sound_id, const char* path, u32 flags);
int sound_load_wav(u32 sound_id, const char* path);
int sound_unload(u32 sound_id);
void sound_unload_all(void);

/* Sound playback */
int sound_play(u32 sound_id);
int sound_play_loop(u32 sound_id);
int sound_stop(u32 sound_id);
int sound_stop_all(void);
int sound_is_playing(u32 sound_id);

/* Sound properties */
int sound_set_volume(u32 sound_id, s32 volume);
int sound_set_pan(u32 sound_id, s32 pan);
int sound_set_frequency(u32 sound_id, u32 frequency);
void sound_set_master_volume(s32 volume);

/* Sound buffers */
void* sound_create_buffer(u32 size, u32 frequency, u32 channels, u32 bits);
int sound_release_buffer(u32 sound_id);

/* WAV file loading */
int wav_load_file(const char* path, void** buffer, u32* size, u32* frequency,
                  u32* channels, u32* bits);

/* Sound paths */
void sound_build_path(char* dest, u32 size, const char* filename);

/* BGM Management - FUN_0048a120 pattern */
int bgm_init(void);
void bgm_shutdown(void);
int bgm_load(u32 bgm_id);
int bgm_play(u32 bgm_id);
int bgm_stop(void);
int bgm_is_playing(void);
int bgm_set_volume(s32 volume);
u32 bgm_get_current(void);

/* Streaming BGM - from FUN_00487ba0, FUN_00487e40 analysis */
int bgm_load_streaming(u32 bgm_id);
void bgm_stop_streaming(void);
int bgm_is_streaming_active(void);

/* BGM fade control - from FUN_00487520 analysis */
void bgm_fade_out(u32 duration_ms);
void bgm_fade_in(u32 duration_ms);
void bgm_update_fade(u32 delta_time);
int bgm_get_fade_position(void);

/* BGM type check - type 2 = streaming */
int bgm_is_type_streaming(u32 bgm_id);

/* Sound Effects Management */
int se_init(void);
void se_shutdown(void);
int se_load_series(int series, int start, int end);
int se_play(u32 se_id);
int se_play_with_volume(u32 se_id, s32 volume);
int se_stop(u32 se_id);
int se_stop_all(void);
int se_preload_common(void);

/* DBCS-aware path handling - FUN_0048a170, FUN_0048a200 patterns */
void sound_escape_path(const char* src, char* dst, int max_len);
void sound_unescape_path(const char* src, char* dst, int max_len);

#endif /* SOUND_H */
