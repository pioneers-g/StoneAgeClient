/*
 * Stone Age Client - BGM (Background Music) System
 * BGM management, loading, playback
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

/* BGM file paths - indexed by BGM_ID */
const char* s_bgm_paths[BGM_ID_COUNT] = {
    "data\\bgm\\sabgm_s0.wav",   /* BGM_ID_S0 */
    "data\\bgm\\sabgm_s1.wav",   /* BGM_ID_S1 */
    "data\\bgm\\sabgm_d1.wav",   /* BGM_ID_D1 */
    "data\\bgm\\sabgm_d2.wav",   /* BGM_ID_D2 */
    "data\\bgm\\sabgm_f2.wav",   /* BGM_ID_F2 */
    "data\\bgm\\sabgm_b2.wav",   /* BGM_ID_B2 */
    "data\\bgm\\sabgm_t2.wav",   /* BGM_ID_T2 */
    "data\\bgm\\sabgm_t12.wav"   /* BGM_ID_T12 */
};

/* BGM types - type 2 = streaming, 0 = static, from FUN_00487ba0 analysis */
const u8 s_bgm_types[BGM_ID_COUNT] = {
    0,  /* BGM_ID_S0 - static */
    0,  /* BGM_ID_S1 - static */
    0,  /* BGM_ID_D1 - static */
    2,  /* BGM_ID_D2 - streaming */
    0,  /* BGM_ID_F2 - static */
    0,  /* BGM_ID_B2 - static */
    2,  /* BGM_ID_T2 - streaming */
    2   /* BGM_ID_T12 - streaming */
};

/* BGM state - defined as extern in sound_internal.h */
BgmState g_bgm = {0};

/*
 * Initialize BGM system
 */
int bgm_init(void) {
    memset(&g_bgm, 0, sizeof(g_bgm));
    g_bgm.bgm_buffer_id = 0;  /* Use buffer 0 for BGM */
    g_bgm.volume = 100;
    LOG_DEBUG("BGM system initialized");
    return 1;
}

/*
 * Shutdown BGM system
 */
void bgm_shutdown(void) {
    bgm_stop();
    bgm_stop_streaming();
    sound_unload(g_bgm.bgm_buffer_id);
    memset(&g_bgm, 0, sizeof(g_bgm));
}

/*
 * Load BGM by ID
 */
int bgm_load(u32 bgm_id) {
    char path[MAX_SOUND_PATH];
    const char* bgm_file;

    if (bgm_id >= BGM_ID_COUNT) {
        LOG_DEBUG("Invalid BGM ID: %d", bgm_id);
        return 0;
    }

    bgm_file = s_bgm_paths[bgm_id];

    /* Build full path */
    snprintf(path, sizeof(path), "%s%s", config_get_data_path(), bgm_file);

    LOG_DEBUG("Loading BGM %d: %s", bgm_id, path);

    if (!sound_load(g_bgm.bgm_buffer_id, path, SOUND_FLAG_LOOP)) {
        LOG_DEBUG("Failed to load BGM %d", bgm_id);
        return 0;
    }

    g_bgm.current_bgm = bgm_id;
    return 1;
}

/*
 * Play BGM by ID
 */
int bgm_play(u32 bgm_id) {
    /* Stop current BGM */
    if (g_bgm.is_playing) {
        bgm_stop();
    }

    /* Stop streaming if active */
    if (g_sound.streaming_active) {
        bgm_stop_streaming();
    }

    /* Check if music is enabled */
    if (!g_sound.music_enabled) {
        return 0;
    }

    /* Check BGM type - streaming vs static */
    if (bgm_is_type_streaming(bgm_id)) {
        /* Type 2 = streaming BGM - from FUN_00487ba0 */
        if (!bgm_load_streaming(bgm_id)) {
            LOG_DEBUG("Failed to load streaming BGM %d", bgm_id);
            return 0;
        }

        /* Play streaming buffer */
        if (g_sound.streaming.buffer) {
            HRESULT hr = IDirectSoundBuffer_Play(
                (LPDIRECTSOUNDBUFFER)g_sound.streaming.buffer, 0, 0, DSBPLAY_LOOPING);
            if (FAILED(hr)) {
                LOG_DEBUG("Failed to play streaming BGM %d", bgm_id);
                return 0;
            }
        }
    } else {
        /* Type 0 = static BGM */
        if (g_bgm.current_bgm != bgm_id) {
            if (!bgm_load(bgm_id)) {
                return 0;
            }
        }

        /* Play looping */
        if (!sound_play_loop(g_bgm.bgm_buffer_id)) {
            LOG_DEBUG("Failed to play BGM %d", bgm_id);
            return 0;
        }
    }

    g_bgm.is_playing = 1;
    g_bgm.current_bgm = bgm_id;
    bgm_set_volume(g_bgm.volume);

    LOG_DEBUG("Playing BGM %d", bgm_id);
    return 1;
}

/*
 * Stop BGM
 */
int bgm_stop(void) {
    if (g_bgm.is_playing) {
        /* Check for streaming BGM */
        if (g_sound.streaming_active) {
            bgm_stop_streaming();
        } else {
            sound_stop(g_bgm.bgm_buffer_id);
        }
        g_bgm.is_playing = 0;
    }
    return 1;
}

/*
 * Check if BGM is playing
 */
int bgm_is_playing(void) {
    return g_bgm.is_playing && sound_is_playing(g_bgm.bgm_buffer_id);
}

/*
 * Set BGM volume
 */
int bgm_set_volume(s32 volume) {
    LONG ds_volume;
    HRESULT hr;

    g_bgm.volume = volume;

    if (!g_bgm.is_playing) return 1;

    /* Convert volume (0-100) to DirectSound format (-10000 to 0) */
    if (volume <= 0) {
        ds_volume = DSBVOLUME_MIN;
    } else if (volume >= 100) {
        ds_volume = DSBVOLUME_MAX;
    } else {
        ds_volume = (LONG)(log10((double)volume / 100.0) * 2000.0);
    }

    /* Set volume on appropriate buffer */
    if (g_sound.streaming_active && g_sound.streaming.buffer) {
        hr = IDirectSoundBuffer_SetVolume(
            (LPDIRECTSOUNDBUFFER)g_sound.streaming.buffer, ds_volume);
        return SUCCEEDED(hr);
    } else {
        return sound_set_volume(g_bgm.bgm_buffer_id, volume);
    }
}

/*
 * Get current BGM ID
 */
u32 bgm_get_current(void) {
    return g_bgm.current_bgm;
}

/*
 * Check if BGM type is streaming
 */
int bgm_is_type_streaming(u32 bgm_id) {
    if (bgm_id >= BGM_ID_COUNT) return 0;
    return s_bgm_types[bgm_id] == 2;
}

/* ========================================
 * Field BGM Lookup - FUN_0047cfe0
 * ======================================== */

/* External globals for field rendering */
extern u32 DAT_046333f0;    /* Current field sprite ID */
extern u32 DAT_046333f4;    /* Previous field sprite ID */
extern u32 DAT_04633404;    /* Field type flag */

/*
 * FUN_0047cfe0 - Field BGM Lookup
 *
 * Binary analysis:
 * - Maps field/map ID to BGM sprite ID
 * - Sets DAT_046333f0 to the appropriate sprite ID for the field
 * - Special field IDs have hardcoded sprite IDs
 * - Calls FUN_0047cd80 to update field rendering if sprite changed
 *
 * Field ID to Sprite ID mapping:
 * - 0x68 (104): 0x29e3f (171583)
 * - 0x15cd (5581): 0x19e3f (106303)
 * - 0x203f (8255): 0x19e40 (106304)
 * - 0x2040 (8256): 0x29e40 (171584)
 * - 0x2041 (8257): 0x39e40 (236864)
 * - 0x77e1 (30689): 0x69e3e (433726)
 * - 0x77e3-0x77e7: Various sprite IDs for special maps
 * - 0x1d1a-0x1d1b (7450-7451): 0x10000 (65536)
 */
void field_set_bgm_sprite(int field_id) {
    extern u32 DAT_04581d3c, DAT_04581d40;
    extern u32 DAT_04569b70;
    extern u32 DAT_046333b4, DAT_046333bc, DAT_046333c0, DAT_046333c8;
    extern u32 DAT_046333c4, DAT_046333cc, DAT_046333d4, DAT_04633398;
    extern void FUN_0047cd80(void);

    u32 prev_sprite = DAT_046333f0;

    /* Save previous sprite and reset */
    DAT_046333f4 = DAT_046333f0;
    DAT_046333f0 = 0;
    DAT_04633404 = 0;

    if (field_id < 0x77e2) {
        if (field_id == 0x77e1) {
            DAT_046333f0 = 0x69e3e;
        } else {
            if (field_id < 0x2040) {
                if (field_id == 0x203f) {
                    DAT_046333f0 = 0x19e40;
                } else {
                    if (field_id < 0x15ce) {
                        if (field_id == 0x15cd) {
                            DAT_046333f0 = 0x19e3f;
                        } else if (field_id == 0x68) {
                            DAT_046333f0 = 0x29e3f;
                        }
                    } else if (field_id > 0x1d19 && field_id < 0x1d1c) {
                        DAT_046333f0 = 0x10000;
                    }
                    goto check_update;
                }
            } else if (field_id == 0x2040) {
                DAT_046333f0 = 0x29e40;
            } else if (field_id == 0x2041) {
                DAT_046333f0 = 0x39e40;
            } else {
                goto check_update;
            }
            DAT_04633404 = 1;
        }
    } else {
        switch (field_id) {
        case 0x77e3:
            DAT_046333f0 = 0x19e3e;
            break;
        case 0x77e4:
            DAT_046333f0 = 0x29e3e;
            break;
        case 0x77e5:
            DAT_046333f0 = 0x39e3e;
            break;
        case 0x77e6:
            DAT_046333f0 = 0x49e3e;
            break;
        case 0x77e7:
            DAT_046333f0 = 0x59e3e;
            break;
        }
    }

check_update:
    /* If sprite changed, update field rendering */
    if (prev_sprite != DAT_046333f0) {
        FUN_0047cd80();

        /* Calculate view offsets */
        DAT_046333c0 = ((DAT_04581d40 + DAT_04581d3c) *
                       (DAT_046333b4 - 0x280) * 0x20) / DAT_046333bc;
        DAT_046333c8 = DAT_046333c0 + 0x280;

        DAT_046333c4 = (((DAT_04569b70 - DAT_04581d3c) + DAT_04581d40) *
                       (DAT_046333d4 - 0x1e0) * 0x18) / DAT_04633398;
        DAT_046333cc = DAT_046333c4 + 0x1e0;
    }
}
