/*
 * Stone Age Client - Streaming BGM System
 * Streaming BGM with double-buffering, notification events, fade system
 * Split from sound.c
 */

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "sound.h"
#include "sound_internal.h"
#include "config.h"
#include "logger.h"

/*
 * Streaming thread function - from FUN_00487a90 analysis
 * This handles buffer notifications for continuous streaming
 */
static DWORD WINAPI bgm_streaming_thread(LPVOID param) {
    StreamingBGM* strm = (StreamingBGM*)param;
    DWORD result;
    HANDLE events[3];

    if (!strm || !strm->buffer) return 1;

    events[0] = strm->events[0];
    events[1] = strm->events[1];
    events[2] = strm->events[2];

    /* Signal initialization complete */
    g_sound.streaming_active = 1;
    strm->running = 1;

    LOG_DEBUG("Streaming thread started");

    while (strm->running) {
        /* Wait for notification */
        result = WaitForMultipleObjects(3, events, FALSE, 100);

        if (!strm->running) break;

        if (result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + 3) {
            /* Buffer position notification - would refill buffer here */
            /* In full implementation, would read more data from file */
        }
    }

    LOG_DEBUG("Streaming thread stopped");
    return 0;
}

/*
 * Load streaming BGM - from FUN_00487ba0, FUN_00487e40 analysis
 * Creates double-buffered streaming with notification events
 */
int bgm_load_streaming(u32 bgm_id) {
    char path[MAX_SOUND_PATH];
    HMMIO hmmio;
    MMCKINFO ckRiff, ckChunk;
    WAVEFORMATEX wfx;
    DSBUFFERDESC dsbd;
    DSBPOSITIONNOTIFY notify[3];
    LPDIRECTSOUNDNOTIFY dsNotify;
    HRESULT hr;
    LONG bytes_read;
    u32 data_size, bytes_per_sec;
    LPVOID ptr1, ptr2;
    DWORD bytes1, bytes2;
    int i;

    if (bgm_id >= BGM_ID_COUNT) {
        LOG_DEBUG("Invalid streaming BGM ID: %d", bgm_id);
        return 0;
    }

    /* Build full path */
    snprintf(path, sizeof(path), "%s%s", config_get_data_path(), s_bgm_paths[bgm_id]);

    LOG_DEBUG("Loading streaming BGM %d: %s", bgm_id, path);

    /* Open WAV file */
    hmmio = mmioOpenA((LPSTR)path, NULL, MMIO_READ);
    if (!hmmio) {
        LOG_DEBUG("Failed to open streaming BGM: %s", path);
        return 0;
    }

    /* Parse WAV header - from FUN_004872b0 */
    memset(&ckRiff, 0, sizeof(ckRiff));
    ckRiff.fccType = mmioFOURCC('W', 'A', 'V', 'E');

    if (mmioDescend(hmmio, &ckRiff, NULL, MMIO_FINDRIFF) != MMSYSERR_NOERROR) {
        mmioClose(hmmio, 0);
        return 0;
    }

    /* Read format chunk */
    memset(&ckChunk, 0, sizeof(ckChunk));
    ckChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');

    if (mmioDescend(hmmio, &ckChunk, &ckRiff, MMIO_FINDCHUNK) != MMSYSERR_NOERROR) {
        mmioClose(hmmio, 0);
        return 0;
    }

    memset(&wfx, 0, sizeof(wfx));
    bytes_read = mmioRead(hmmio, (HPSTR)&wfx, sizeof(WAVEFORMATEX));
    if (bytes_read < 18) {
        mmioClose(hmmio, 0);
        return 0;
    }

    mmioAscend(hmmio, &ckChunk, 0);

    /* Find data chunk */
    memset(&ckChunk, 0, sizeof(ckChunk));
    ckChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');

    if (mmioDescend(hmmio, &ckChunk, &ckRiff, MMIO_FINDCHUNK) != MMSYSERR_NOERROR) {
        mmioClose(hmmio, 0);
        return 0;
    }

    data_size = ckChunk.cksize;
    bytes_per_sec = wfx.nAvgBytesPerSec;

    /* Calculate buffer size (aligned to block align) */
    u32 buffer_size = ((data_size + 0xe) | 1) * wfx.nBlockAlign;

    /* Create streaming buffer */
    memset(&dsbd, 0, sizeof(dsbd));
    dsbd.dwSize = sizeof(dsbd);
    dsbd.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLVOLUME |
                   DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCHARDWARE;
    dsbd.dwBufferBytes = buffer_size;
    dsbd.lpwfxFormat = &wfx;

    hr = IDirectSound_CreateSoundBuffer((LPDIRECTSOUND)g_sound.direct_sound,
        &dsbd, (LPDIRECTSOUNDBUFFER*)&g_sound.streaming.buffer, NULL);

    if (FAILED(hr)) {
        /* Try software buffer */
        dsbd.dwFlags &= ~DSBCAPS_LOCHARDWARE;
        dsbd.dwFlags |= DSBCAPS_LOCSOFTWARE;
        hr = IDirectSound_CreateSoundBuffer((LPDIRECTSOUND)g_sound.direct_sound,
            &dsbd, (LPDIRECTSOUNDBUFFER*)&g_sound.streaming.buffer, NULL);

        if (FAILED(hr)) {
            mmioClose(hmmio, 0);
            LOG_DEBUG("Failed to create streaming buffer: 0x%08X", hr);
            return 0;
        }
    }

    /* Lock and read initial data */
    hr = IDirectSoundBuffer_Lock((LPDIRECTSOUNDBUFFER)g_sound.streaming.buffer,
        0, buffer_size, &ptr1, &bytes1, &ptr2, &bytes2, 0);

    if (SUCCEEDED(hr)) {
        bytes_read = mmioRead(hmmio, (HPSTR)ptr1, bytes1);
        if (bytes_read < (LONG)bytes1) {
            /* Fill remaining with silence */
            memset((char*)ptr1 + bytes_read, 0, bytes1 - bytes_read);
        }
        if (ptr2 && bytes2 > 0) {
            bytes_read = mmioRead(hmmio, (HPSTR)ptr2, bytes2);
            if (bytes_read < (LONG)bytes2) {
                memset((char*)ptr2 + bytes_read, 0, bytes2 - bytes_read);
            }
        }
        IDirectSoundBuffer_Unlock((LPDIRECTSOUNDBUFFER)g_sound.streaming.buffer,
            ptr1, bytes1, ptr2, bytes2);
    }

    mmioClose(hmmio, 0);

    /* Create notification events */
    for (i = 0; i < 3; i++) {
        g_sound.streaming.events[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!g_sound.streaming.events[i]) {
            /* Cleanup on failure */
            for (i--; i >= 0; i--) {
                CloseHandle(g_sound.streaming.events[i]);
            }
            IDirectSoundBuffer_Release((LPDIRECTSOUNDBUFFER)g_sound.streaming.buffer);
            g_sound.streaming.buffer = NULL;
            return 0;
        }
    }

    /* Set up notifications */
    hr = IDirectSoundBuffer_QueryInterface((LPDIRECTSOUNDBUFFER)g_sound.streaming.buffer,
        &IID_IDirectSoundNotify, (void**)&dsNotify);

    if (SUCCEEDED(hr)) {
        /* Notify at 0, 1/2, and end */
        notify[0].dwOffset = 0;
        notify[0].hEventNotify = g_sound.streaming.events[0];
        notify[1].dwOffset = buffer_size / 2;
        notify[1].hEventNotify = g_sound.streaming.events[1];
        notify[2].dwOffset = buffer_size - 1;
        notify[2].hEventNotify = g_sound.streaming.events[2];

        hr = IDirectSoundNotify_SetNotificationPositions(dsNotify, 3, notify);
        IDirectSoundNotify_Release(dsNotify);

        if (FAILED(hr)) {
            for (i = 0; i < 3; i++) {
                CloseHandle(g_sound.streaming.events[i]);
            }
            IDirectSoundBuffer_Release((LPDIRECTSOUNDBUFFER)g_sound.streaming.buffer);
            g_sound.streaming.buffer = NULL;
            return 0;
        }
    }

    /* Store buffer info */
    g_sound.streaming.buffer_size = buffer_size;
    g_sound.streaming.data_size = data_size;
    g_sound.streaming.bytes_per_sec = bytes_per_sec;
    g_sound.streaming.initialized = 1;

    /* Create streaming thread */
    g_sound.streaming.thread = CreateThread(NULL, 0, bgm_streaming_thread,
        &g_sound.streaming, 0, &g_sound.streaming.thread_id);

    if (!g_sound.streaming.thread) {
        for (i = 0; i < 3; i++) {
            CloseHandle(g_sound.streaming.events[i]);
        }
        IDirectSoundBuffer_Release((LPDIRECTSOUNDBUFFER)g_sound.streaming.buffer);
        g_sound.streaming.buffer = NULL;
        g_sound.streaming.initialized = 0;
        return 0;
    }

    /* Wait for thread to initialize */
    while (!g_sound.streaming_active) {
        Sleep(1);
    }

    g_bgm.current_bgm = bgm_id;
    LOG_DEBUG("Streaming BGM %d loaded", bgm_id);
    return 1;
}

/*
 * Stop streaming BGM - from FUN_00487470 analysis
 */
void bgm_stop_streaming(void) {
    int i;

    if (!g_sound.streaming.initialized) return;

    /* Signal thread to stop */
    g_sound.streaming.running = 0;
    g_sound.streaming_active = 0;

    /* Wait for thread to finish */
    if (g_sound.streaming.thread) {
        WaitForSingleObject(g_sound.streaming.thread, 1000);
        CloseHandle(g_sound.streaming.thread);
        g_sound.streaming.thread = NULL;
    }

    /* Stop buffer */
    if (g_sound.streaming.buffer) {
        IDirectSoundBuffer_Stop((LPDIRECTSOUNDBUFFER)g_sound.streaming.buffer);
        IDirectSoundBuffer_Release((LPDIRECTSOUNDBUFFER)g_sound.streaming.buffer);
        g_sound.streaming.buffer = NULL;
    }

    /* Close events */
    for (i = 0; i < 3; i++) {
        if (g_sound.streaming.events[i]) {
            CloseHandle(g_sound.streaming.events[i]);
            g_sound.streaming.events[i] = NULL;
        }
    }

    g_sound.streaming.initialized = 0;
    LOG_DEBUG("Streaming BGM stopped");
}

/*
 * Check if streaming BGM is active
 */
int bgm_is_streaming_active(void) {
    return g_sound.streaming_active;
}

/* ========================================
 * BGM Fade System
 * From FUN_00487520 analysis
 * ======================================== */

/*
 * Start BGM fade out
 */
void bgm_fade_out(u32 duration_ms) {
    g_bgm.is_fading_out = 1;
    g_bgm.is_fading_in = 0;
    g_bgm.fade_duration = duration_ms;
    g_bgm.fade_elapsed = 0;
    g_bgm.fade_volume = g_bgm.volume;
}

/*
 * Start BGM fade in
 */
void bgm_fade_in(u32 duration_ms) {
    g_bgm.is_fading_in = 1;
    g_bgm.is_fading_out = 0;
    g_bgm.fade_duration = duration_ms;
    g_bgm.fade_elapsed = 0;
    g_bgm.fade_volume = 0;
    bgm_set_volume(0);
}

/*
 * Update BGM fade
 */
void bgm_update_fade(u32 delta_time) {
    s32 new_volume;

    if (!g_bgm.is_fading_out && !g_bgm.is_fading_in) return;

    g_bgm.fade_elapsed += delta_time;

    if (g_bgm.fade_elapsed >= g_bgm.fade_duration) {
        if (g_bgm.is_fading_out) {
            bgm_stop();
            if (g_sound.streaming_active) {
                bgm_stop_streaming();
            }
            g_bgm.is_fading_out = 0;
        }
        if (g_bgm.is_fading_in) {
            bgm_set_volume(g_bgm.volume);
            g_bgm.is_fading_in = 0;
        }
        return;
    }

    /* Calculate current volume */
    if (g_bgm.is_fading_out) {
        new_volume = g_bgm.volume - (s32)((s64)g_bgm.volume * g_bgm.fade_elapsed / g_bgm.fade_duration);
    } else {
        new_volume = (s32)((s64)g_bgm.volume * g_bgm.fade_elapsed / g_bgm.fade_duration);
    }

    bgm_set_volume(new_volume);
}

/*
 * Get fade position (0-1000)
 */
int bgm_get_fade_position(void) {
    if (g_bgm.fade_duration == 0) return 1000;
    return (int)(g_bgm.fade_elapsed * 1000 / g_bgm.fade_duration);
}
