/*
 * Stone Age Client - Sound Core System
 * DirectSound initialization, WAV loading, basic playback
 * Split from sound.c
 */

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "types.h"
#include "sound.h"
#include "sound_internal.h"
#include "config.h"
#include "logger.h"

/* Global sound context */
SoundContext g_sound = {0};

/* DirectSound function pointers */
HRESULT (WINAPI* pDirectSoundCreate)(LPGUID, LPDIRECTSOUND*, LPUNKNOWN) = NULL;
HMODULE g_dsound_module = NULL;

/* Sound cache */
struct SoundCacheEntry s_sound_cache[SOUND_CACHE_SIZE];
int s_cache_count = 0;

/*
 * Initialize sound system - FUN_0048a120 pattern
 */
int sound_init(HWND hwnd) {
    HRESULT hr;
    DSBUFFERDESC dsbd;
    WAVEFORMATEX wfx;

    LOG_INFO("Initializing sound system...");

    /* Load DirectSound library */
    g_dsound_module = LoadLibraryA("dsound.dll");
    if (!g_dsound_module) {
        LOG_WARN("Failed to load dsound.dll");
        return 0;
    }

    pDirectSoundCreate = (HRESULT (WINAPI*)(LPGUID, LPDIRECTSOUND*, LPUNKNOWN))
        GetProcAddress(g_dsound_module, "DirectSoundCreate");

    if (!pDirectSoundCreate) {
        LOG_WARN("Failed to get DirectSoundCreate address");
        FreeLibrary(g_dsound_module);
        g_dsound_module = NULL;
        return 0;
    }

    /* Create DirectSound object */
    hr = pDirectSoundCreate(NULL, (LPDIRECTSOUND*)&g_sound.direct_sound, NULL);
    if (FAILED(hr)) {
        LOG_WARN("DirectSoundCreate failed: 0x%08X", hr);
        FreeLibrary(g_dsound_module);
        g_dsound_module = NULL;
        return 0;
    }

    /* Set cooperative level */
    hr = IDirectSound_SetCooperativeLevel((LPDIRECTSOUND)g_sound.direct_sound,
        hwnd, DSSCL_PRIORITY);
    if (FAILED(hr)) {
        LOG_WARN("SetCooperativeLevel failed: 0x%08X", hr);
        IDirectSound_Release((LPDIRECTSOUND)g_sound.direct_sound);
        FreeLibrary(g_dsound_module);
        g_dsound_module = NULL;
        return 0;
    }

    /* Create primary buffer */
    memset(&dsbd, 0, sizeof(dsbd));
    dsbd.dwSize = sizeof(dsbd);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
    dsbd.dwBufferBytes = 0;
    dsbd.lpwfxFormat = NULL;

    hr = IDirectSound_CreateSoundBuffer((LPDIRECTSOUND)g_sound.direct_sound,
        &dsbd, (LPDIRECTSOUNDBUFFER*)&g_sound.primary_buffer, NULL);
    if (FAILED(hr)) {
        LOG_WARN("CreateSoundBuffer (primary) failed: 0x%08X", hr);
    } else {
        /* Set primary buffer format */
        memset(&wfx, 0, sizeof(wfx));
        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nChannels = 2;
        wfx.nSamplesPerSec = 44100;
        wfx.wBitsPerSample = 16;
        wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
        wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
        wfx.cbSize = 0;

        IDirectSoundBuffer_SetFormat((LPDIRECTSOUNDBUFFER)g_sound.primary_buffer, &wfx);
    }

    /* Initialize sound array */
    memset(g_sound.sounds, 0, sizeof(g_sound.sounds));
    g_sound.sound_count = 0;
    g_sound.master_volume = 100;
    g_sound.sound_enabled = 1;
    g_sound.music_enabled = 1;

    LOG_INFO("Sound system initialized");

    /* Initialize BGM and SE subsystems */
    bgm_init();
    se_init();

    return 1;
}

/*
 * Shutdown sound system
 */
void sound_shutdown(void) {
    u32 i;

    LOG_INFO("Shutting down sound system...");

    /* Shutdown BGM and SE subsystems */
    bgm_shutdown();
    se_shutdown();

    /* Stop and release all sounds */
    sound_stop_all();
    sound_unload_all();

    /* Release primary buffer */
    if (g_sound.primary_buffer) {
        IDirectSoundBuffer_Release((LPDIRECTSOUNDBUFFER)g_sound.primary_buffer);
        g_sound.primary_buffer = NULL;
    }

    /* Release DirectSound */
    if (g_sound.direct_sound) {
        IDirectSound_Release((LPDIRECTSOUNDBUFFER)g_sound.direct_sound);
        g_sound.direct_sound = NULL;
    }

    /* Free library */
    if (g_dsound_module) {
        FreeLibrary(g_dsound_module);
        g_dsound_module = NULL;
        pDirectSoundCreate = NULL;
    }

    memset(&g_sound, 0, sizeof(SoundContext));
    LOG_INFO("Sound system shutdown");
}

/*
 * Load WAV file - uses mmio functions
 */
int wav_load_file(const char* path, void** buffer, u32* size, u32* frequency,
                  u32* channels, u32* bits) {
    HMMIO hmmio;
    MMCKINFO ckRiff, ckChunk;
    WAVEFORMATEX* wfx;
    LONG bytes_read;

    *buffer = NULL;
    *size = 0;
    *frequency = 22050;
    *channels = 1;
    *bits = 8;

    /* Open WAV file using mmio */
    hmmio = mmioOpenA((LPSTR)path, NULL, MMIO_READ);
    if (!hmmio) {
        LOG_DEBUG("mmioOpenA failed for: %s", path);
        return 0;
    }

    /* Descend into RIFF chunk */
    memset(&ckRiff, 0, sizeof(ckRiff));
    ckRiff.fccType = mmioFOURCC('W', 'A', 'V', 'E');

    if (mmioDescend(hmmio, &ckRiff, NULL, MMIO_FINDRIFF) != MMSYSERR_NOERROR) {
        LOG_DEBUG("mmioDescend (RIFF) failed for: %s", path);
        mmioClose(hmmio, 0);
        return 0;
    }

    /* Find format chunk */
    memset(&ckChunk, 0, sizeof(ckChunk));
    ckChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');

    if (mmioDescend(hmmio, &ckChunk, &ckRiff, MMIO_FINDCHUNK) != MMSYSERR_NOERROR) {
        LOG_DEBUG("mmioDescend (fmt) failed for: %s", path);
        mmioClose(hmmio, 0);
        return 0;
    }

    /* Read format data */
    wfx = (WAVEFORMATEX*)malloc(ckChunk.cksize);
    if (!wfx) {
        mmioClose(hmmio, 0);
        return 0;
    }

    bytes_read = mmioRead(hmmio, (HPSTR)wfx, ckChunk.cksize);
    if (bytes_read <= 0) {
        free(wfx);
        mmioClose(hmmio, 0);
        return 0;
    }

    *frequency = wfx->nSamplesPerSec;
    *channels = wfx->nChannels;
    *bits = wfx->wBitsPerSample;

    free(wfx);

    /* Ascend out of format chunk */
    mmioAscend(hmmio, &ckChunk, 0);

    /* Find data chunk */
    memset(&ckChunk, 0, sizeof(ckChunk));
    ckChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');

    if (mmioDescend(hmmio, &ckChunk, &ckRiff, MMIO_FINDCHUNK) != MMSYSERR_NOERROR) {
        LOG_DEBUG("mmioDescend (data) failed for: %s", path);
        mmioClose(hmmio, 0);
        return 0;
    }

    /* Allocate and read data */
    *buffer = malloc(ckChunk.cksize);
    if (!*buffer) {
        mmioClose(hmmio, 0);
        return 0;
    }

    bytes_read = mmioRead(hmmio, (HPSTR)*buffer, ckChunk.cksize);
    if (bytes_read <= 0) {
        free(*buffer);
        *buffer = NULL;
        mmioClose(hmmio, 0);
        return 0;
    }

    *size = bytes_read;

    mmioClose(hmmio, 0);
    return 1;
}

/*
 * Build sound path - FUN_0048a170 pattern (DBCS aware)
 */
void sound_build_path(char* dest, u32 size, const char* filename) {
    const char* data_path;
    const char* subdir;

    data_path = config_get_data_path();

    /* Determine subdirectory based on extension */
    if (strstr(filename, ".wav") || strstr(filename, ".WAV")) {
        subdir = "sound/";
    } else if (strstr(filename, ".mid") || strstr(filename, ".MID")) {
        subdir = "midi/";
    } else {
        subdir = "";
    }

    snprintf(dest, size, "%s%s%s", data_path, subdir, filename);
}

/*
 * Create sound buffer
 */
void* sound_create_buffer(u32 size, u32 frequency, u32 channels, u32 bits) {
    DSBUFFERDESC dsbd;
    WAVEFORMATEX wfx;
    LPDIRECTSOUNDBUFFER buffer;
    HRESULT hr;

    if (!g_sound.direct_sound) return NULL;

    memset(&wfx, 0, sizeof(wfx));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = (WORD)channels;
    wfx.nSamplesPerSec = frequency;
    wfx.wBitsPerSample = (WORD)bits;
    wfx.nBlockAlign = (WORD)(wfx.nChannels * wfx.wBitsPerSample / 8);
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    memset(&dsbd, 0, sizeof(dsbd));
    dsbd.dwSize = sizeof(dsbd);
    dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN |
                   DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC;
    dsbd.dwBufferBytes = size;
    dsbd.lpwfxFormat = &wfx;

    hr = IDirectSound_CreateSoundBuffer((LPDIRECTSOUND)g_sound.direct_sound,
        &dsbd, &buffer, NULL);

    if (FAILED(hr)) {
        LOG_WARN("CreateSoundBuffer failed: 0x%08X", hr);
        return NULL;
    }

    return buffer;
}

/*
 * Load sound
 */
int sound_load(u32 sound_id, const char* path, u32 flags) {
    void* data;
    u32 size, frequency, channels, bits;
    LPDIRECTSOUNDBUFFER buffer;
    HRESULT hr;
    LPVOID ptr1, ptr2;
    DWORD bytes1, bytes2;

    if (sound_id >= MAX_SOUND_BUFFERS) return 0;
    if (!g_sound.direct_sound) return 0;

    /* Load WAV file */
    if (!wav_load_file(path, &data, &size, &frequency, &channels, &bits)) {
        LOG_DEBUG("Failed to load WAV: %s", path);
        return 0;
    }

    /* Create buffer */
    buffer = (LPDIRECTSOUNDBUFFER)sound_create_buffer(size, frequency, channels, bits);
    if (!buffer) {
        free(data);
        return 0;
    }

    /* Copy data to buffer */
    hr = IDirectSoundBuffer_Lock(buffer, 0, size, &ptr1, &bytes1, &ptr2, &bytes2, 0);
    if (FAILED(hr)) {
        IDirectSoundBuffer_Release(buffer);
        free(data);
        return 0;
    }

    memcpy(ptr1, data, bytes1);
    if (ptr2 && bytes2 > 0) {
        memcpy(ptr2, (char*)data + bytes1, bytes2);
    }

    IDirectSoundBuffer_Unlock(buffer, ptr1, bytes1, ptr2, bytes2);

    /* Release old buffer if exists */
    if (g_sound.sounds[sound_id].buffer) {
        IDirectSoundBuffer_Release((LPDIRECTSOUNDBUFFER)g_sound.sounds[sound_id].buffer);
    }

    /* Store sound info */
    g_sound.sounds[sound_id].id = sound_id;
    strncpy(g_sound.sounds[sound_id].path, path, MAX_SOUND_PATH - 1);
    g_sound.sounds[sound_id].flags = flags;
    g_sound.sounds[sound_id].volume = 0;
    g_sound.sounds[sound_id].pan = 0;
    g_sound.sounds[sound_id].frequency = frequency;
    g_sound.sounds[sound_id].buffer = buffer;
    g_sound.sounds[sound_id].buffer_size = size;
    g_sound.sounds[sound_id].is_loaded = 1;
    g_sound.sounds[sound_id].is_playing = 0;

    free(data);

    if (sound_id >= g_sound.sound_count) {
        g_sound.sound_count = sound_id + 1;
    }

    LOG_DEBUG("Loaded sound %d: %s", sound_id, path);
    return 1;
}

/*
 * Load WAV file by path
 */
int sound_load_wav(u32 sound_id, const char* path) {
    char full_path[MAX_SOUND_PATH];
    sound_build_path(full_path, sizeof(full_path), path);
    return sound_load(sound_id, full_path, 0);
}

/*
 * Unload sound
 */
int sound_unload(u32 sound_id) {
    if (sound_id >= MAX_SOUND_BUFFERS) return 0;

    if (g_sound.sounds[sound_id].buffer) {
        IDirectSoundBuffer_Stop((LPDIRECTSOUNDBUFFER)g_sound.sounds[sound_id].buffer);
        IDirectSoundBuffer_Release((LPDIRECTSOUNDBUFFER)g_sound.sounds[sound_id].buffer);
        g_sound.sounds[sound_id].buffer = NULL;
    }

    memset(&g_sound.sounds[sound_id], 0, sizeof(Sound));
    return 1;
}

/*
 * Unload all sounds
 */
void sound_unload_all(void) {
    u32 i;
    for (i = 0; i < MAX_SOUND_BUFFERS; i++) {
        sound_unload(i);
    }
    g_sound.sound_count = 0;
}

/*
 * Play sound
 */
int sound_play(u32 sound_id) {
    LPDIRECTSOUNDBUFFER buffer;
    HRESULT hr;

    if (sound_id >= MAX_SOUND_BUFFERS) return 0;
    if (!g_sound.sound_enabled) return 0;

    buffer = (LPDIRECTSOUNDBUFFER)g_sound.sounds[sound_id].buffer;
    if (!buffer) return 0;

    /* Stop if already playing */
    IDirectSoundBuffer_Stop(buffer);
    IDirectSoundBuffer_SetCurrentPosition(buffer, 0);

    /* Play */
    hr = IDirectSoundBuffer_Play(buffer, 0, 0, 0);
    if (FAILED(hr)) {
        LOG_DEBUG("Play failed for sound %d: 0x%08X", sound_id, hr);
        return 0;
    }

    g_sound.sounds[sound_id].is_playing = 1;
    return 1;
}

/*
 * Play sound in loop
 */
int sound_play_loop(u32 sound_id) {
    LPDIRECTSOUNDBUFFER buffer;
    HRESULT hr;

    if (sound_id >= MAX_SOUND_BUFFERS) return 0;
    if (!g_sound.sound_enabled) return 0;

    buffer = (LPDIRECTSOUNDBUFFER)g_sound.sounds[sound_id].buffer;
    if (!buffer) return 0;

    /* Stop if already playing */
    IDirectSoundBuffer_Stop(buffer);
    IDirectSoundBuffer_SetCurrentPosition(buffer, 0);

    /* Play looping */
    hr = IDirectSoundBuffer_Play(buffer, 0, 0, DSBPLAY_LOOPING);
    if (FAILED(hr)) {
        return 0;
    }

    g_sound.sounds[sound_id].is_playing = 1;
    return 1;
}

/*
 * Stop sound
 */
int sound_stop(u32 sound_id) {
    LPDIRECTSOUNDBUFFER buffer;

    if (sound_id >= MAX_SOUND_BUFFERS) return 0;

    buffer = (LPDIRECTSOUNDBUFFER)g_sound.sounds[sound_id].buffer;
    if (!buffer) return 0;

    IDirectSoundBuffer_Stop(buffer);
    g_sound.sounds[sound_id].is_playing = 0;

    return 1;
}

/*
 * Stop all sounds
 */
int sound_stop_all(void) {
    u32 i;

    for (i = 0; i < MAX_SOUND_BUFFERS; i++) {
        if (g_sound.sounds[i].buffer) {
            IDirectSoundBuffer_Stop((LPDIRECTSOUNDBUFFER)g_sound.sounds[i].buffer);
            g_sound.sounds[i].is_playing = 0;
        }
    }

    return 1;
}

/*
 * Check if sound is playing
 */
int sound_is_playing(u32 sound_id) {
    LPDIRECTSOUNDBUFFER buffer;
    DWORD status;

    if (sound_id >= MAX_SOUND_BUFFERS) return 0;

    buffer = (LPDIRECTSOUNDBUFFER)g_sound.sounds[sound_id].buffer;
    if (!buffer) return 0;

    IDirectSoundBuffer_GetStatus(buffer, &status);
    return (status & DSBSTATUS_PLAYING) != 0;
}

/*
 * Set sound volume
 */
int sound_set_volume(u32 sound_id, s32 volume) {
    LPDIRECTSOUNDBUFFER buffer;
    LONG ds_volume;
    HRESULT hr;

    if (sound_id >= MAX_SOUND_BUFFERS) return 0;

    buffer = (LPDIRECTSOUNDBUFFER)g_sound.sounds[sound_id].buffer;
    if (!buffer) return 0;

    /* Convert volume (0-100) to DirectSound format (-10000 to 0) */
    if (volume <= 0) {
        ds_volume = DSBVOLUME_MIN;
    } else if (volume >= 100) {
        ds_volume = DSBVOLUME_MAX;
    } else {
        ds_volume = (LONG)(log10((double)volume / 100.0) * 2000.0);
    }

    hr = IDirectSoundBuffer_SetVolume(buffer, ds_volume);
    if (FAILED(hr)) {
        return 0;
    }

    g_sound.sounds[sound_id].volume = volume;
    return 1;
}

/*
 * Set sound pan
 */
int sound_set_pan(u32 sound_id, s32 pan) {
    LPDIRECTSOUNDBUFFER buffer;
    LONG ds_pan;
    HRESULT hr;

    if (sound_id >= MAX_SOUND_BUFFERS) return 0;

    buffer = (LPDIRECTSOUNDBUFFER)g_sound.sounds[sound_id].buffer;
    if (!buffer) return 0;

    /* Convert pan (-100 to 100) to DirectSound format (-10000 to 10000) */
    ds_pan = pan * 100;

    hr = IDirectSoundBuffer_SetPan(buffer, ds_pan);
    if (FAILED(hr)) {
        return 0;
    }

    g_sound.sounds[sound_id].pan = pan;
    return 1;
}

/*
 * Set sound frequency
 */
int sound_set_frequency(u32 sound_id, u32 frequency) {
    LPDIRECTSOUNDBUFFER buffer;
    HRESULT hr;

    if (sound_id >= MAX_SOUND_BUFFERS) return 0;

    buffer = (LPDIRECTSOUNDBUFFER)g_sound.sounds[sound_id].buffer;
    if (!buffer) return 0;

    hr = IDirectSoundBuffer_SetFrequency(buffer, frequency);
    if (FAILED(hr)) {
        return 0;
    }

    g_sound.sounds[sound_id].frequency = frequency;
    return 1;
}

/*
 * Set master volume
 */
void sound_set_master_volume(s32 volume) {
    g_sound.master_volume = volume;
    /* Apply to all sounds */
}

/*
 * Release sound buffer
 */
int sound_release_buffer(u32 sound_id) {
    return sound_unload(sound_id);
}

/*
 * DBCS-aware path escaping - FUN_0048a170 pattern
 * Handles double-byte character sets for Asian Windows
 */
void sound_escape_path(const char* src, char* dst, int max_len) {
    int src_idx = 0;
    int dst_idx = 0;
    int len = strlen(src);

    /* Escape table - characters that need backslash prefix */
    static const char escape_chars[] = ":.*?\"<>|";

    while (src_idx < len && dst_idx < max_len - 2) {
        if (IsDBCSLeadByte((BYTE)src[src_idx])) {
            /* Double-byte character - copy both bytes */
            if (src_idx + 1 < len) {
                dst[dst_idx++] = src[src_idx++];
                dst[dst_idx++] = src[src_idx++];
            } else {
                break;
            }
        } else if (src[src_idx] == '\\' && src_idx + 1 < len) {
            /* Check for escape sequence */
            char next = src[src_idx + 1];
            if (strchr(escape_chars, next) != NULL) {
                /* Already escaped - copy as-is */
                dst[dst_idx++] = src[src_idx++];
                dst[dst_idx++] = src[src_idx++];
            } else {
                dst[dst_idx++] = src[src_idx++];
            }
        } else {
            dst[dst_idx++] = src[src_idx++];
        }
    }

    dst[dst_idx] = '\0';
}

/*
 * DBCS-aware path unescaping - FUN_0048a200 pattern
 */
void sound_unescape_path(const char* src, char* dst, int max_len) {
    int src_idx = 0;
    int dst_idx = 0;
    int len = strlen(src);

    static const char escape_chars[] = ":.*?\"<>|";

    while (src_idx < len && dst_idx < max_len - 1) {
        if (IsDBCSLeadByte((BYTE)src[src_idx])) {
            /* Double-byte character */
            if (src_idx + 1 < len) {
                dst[dst_idx++] = src[src_idx++];
                dst[dst_idx++] = src[src_idx++];
            }
        } else if (src[src_idx] == '\\' && src_idx + 1 < len) {
            /* Check for escape sequence */
            char next = src[src_idx + 1];
            if (strchr(escape_chars, next) != NULL) {
                /* Remove backslash */
                src_idx++;
                dst[dst_idx++] = src[src_idx++];
            } else {
                dst[dst_idx++] = src[src_idx++];
            }
        } else {
            dst[dst_idx++] = src[src_idx++];
        }
    }

    dst[dst_idx] = '\0';
}

/*
 * Update sound cache statistics
 */
void sound_cache_touch(u32 sound_id) {
    int i;

    /* Find existing entry */
    for (i = 0; i < s_cache_count; i++) {
        if (s_sound_cache[i].sound_id == sound_id) {
            s_sound_cache[i].last_used = timeGetTime();
            s_sound_cache[i].use_count++;
            return;
        }
    }

    /* Add new entry */
    if (s_cache_count < SOUND_CACHE_SIZE) {
        s_sound_cache[s_cache_count].sound_id = sound_id;
        s_sound_cache[s_cache_count].last_used = timeGetTime();
        s_sound_cache[s_cache_count].use_count = 1;
        s_cache_count++;
    }
}

/*
 * Evict least recently used sound from cache
 */
u32 sound_cache_evict_lru(void) {
    u32 lru_id = 0xFFFFFFFF;
    u32 oldest = 0xFFFFFFFF;
    int i;

    for (i = 0; i < s_cache_count; i++) {
        if (s_sound_cache[i].last_used < oldest) {
            oldest = s_sound_cache[i].last_used;
            lru_id = s_sound_cache[i].sound_id;
        }
    }

    if (lru_id != 0xFFFFFFFF) {
        sound_unload(lru_id);

        /* Remove from cache */
        for (i = 0; i < s_cache_count; i++) {
            if (s_sound_cache[i].sound_id == lru_id) {
                s_sound_cache[i] = s_sound_cache[--s_cache_count];
                break;
            }
        }
    }

    return lru_id;
}

/*
 * Clear sound cache
 */
void sound_cache_clear(void) {
    s_cache_count = 0;
    memset(s_sound_cache, 0, sizeof(s_sound_cache));
}
