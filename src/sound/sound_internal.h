/*
 * Stone Age Client - Sound System Internal Header
 * Internal definitions for sound module split
 */

#ifndef SOUND_INTERNAL_H
#define SOUND_INTERNAL_H

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include "types.h"
#include "sound.h"

/* IID_IDirectSoundNotify for MinGW compatibility */
#ifndef IID_IDirectSoundNotify
static const GUID IID_IDirectSoundNotify_local =
    { 0xb0210783, 0x89cd, 0x11d0, { 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0x25, 0xcd, 0x16 } };
#define IID_IDirectSoundNotify IID_IDirectSoundNotify_local
#endif

/* External global sound context */
extern SoundContext g_sound;

/* DirectSound function pointers (shared) */
extern HRESULT (WINAPI* pDirectSoundCreate)(LPGUID, LPDIRECTSOUND*, LPUNKNOWN);
extern HMODULE g_dsound_module;

/* Sound cache management */
#define SOUND_CACHE_SIZE 64

struct SoundCacheEntry {
    u32 sound_id;
    u32 last_used;
    u32 use_count;
};

extern struct SoundCacheEntry s_sound_cache[SOUND_CACHE_SIZE];
extern int s_cache_count;

/* BGM paths and types - shared between bgm and streaming */
extern const char* s_bgm_paths[];
extern const u8 s_bgm_types[];

/* BGM state - shared between bgm and streaming */
typedef struct BgmState {
    u32 current_bgm;
    u32 bgm_buffer_id;
    s32 volume;
    s32 fade_volume;
    u32 fade_duration;
    u32 fade_elapsed;
    u8 is_playing;
    u8 is_fading_out;
    u8 is_fading_in;
    u8 reserved;
} BgmState;

extern BgmState g_bgm;

/* Core functions - sound_core.c */
int wav_load_file(const char* path, void** buffer, u32* size, u32* frequency,
                  u32* channels, u32* bits);
void sound_build_path(char* dest, u32 size, const char* filename);
void* sound_create_buffer(u32 size, u32 frequency, u32 channels, u32 bits);
void sound_escape_path(const char* src, char* dst, int max_len);
void sound_unescape_path(const char* src, char* dst, int max_len);
void sound_cache_touch(u32 sound_id);
u32 sound_cache_evict_lru(void);
void sound_cache_clear(void);

/* BGM functions - sound_bgm.c */
int bgm_init(void);
void bgm_shutdown(void);
int bgm_load(u32 bgm_id);
int bgm_play(u32 bgm_id);
int bgm_stop(void);
int bgm_is_playing(void);
int bgm_set_volume(s32 volume);
u32 bgm_get_current(void);
int bgm_is_type_streaming(u32 bgm_id);

/* Streaming BGM functions - sound_bgm_streaming.c */
int bgm_load_streaming(u32 bgm_id);
void bgm_stop_streaming(void);
int bgm_is_streaming_active(void);
void bgm_fade_out(u32 duration_ms);
void bgm_fade_in(u32 duration_ms);
void bgm_update_fade(u32 delta_time);
int bgm_get_fade_position(void);

/* SE functions - sound_se.c */
int se_init(void);
void se_shutdown(void);
int se_load_series(int series, int start, int end);
int se_play(u32 se_id);
int se_play_with_volume(u32 se_id, s32 volume);
int se_stop(u32 se_id);
int se_stop_all(void);
int se_preload_common(void);

#endif /* SOUND_INTERNAL_H */
