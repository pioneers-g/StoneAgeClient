/*
 * Stone Age Client - Sound Effects (SE) System
 * Sound effect loading, playback, cache management
 * Split from sound.c
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "sound.h"
#include "sound_internal.h"
#include "config.h"
#include "logger.h"

/* SE buffer range */
#define SE_BUFFER_START    10
#define SE_BUFFER_END      63
#define SE_BUFFER_COUNT    (SE_BUFFER_END - SE_BUFFER_START + 1)

/* SE state */
static struct {
    u32 loaded_se[SE_BUFFER_COUNT];
    u32 next_buffer;
    u8 initialized;
} g_se = {0};

/*
 * Initialize SE system
 */
int se_init(void) {
    memset(&g_se, 0, sizeof(g_se));
    g_se.next_buffer = SE_BUFFER_START;
    g_se.initialized = 1;
    LOG_DEBUG("SE system initialized");
    return 1;
}

/*
 * Shutdown SE system
 */
void se_shutdown(void) {
    u32 i;
    for (i = SE_BUFFER_START; i <= SE_BUFFER_END; i++) {
        sound_unload(i);
    }
    memset(&g_se, 0, sizeof(g_se));
}

/*
 * Load SE series
 */
int se_load_series(int series, int start, int end) {
    char path[MAX_SOUND_PATH];
    int i, count;
    const char* prefix;
    u32 buffer_id;

    /* Determine series prefix */
    switch (series) {
        case 1:  prefix = "sa1_"; break;
        case 2:  prefix = "sa2_"; break;
        case 3:  prefix = "sa3_"; break;
        case 4:  prefix = "saam_"; break;  /* Ambient */
        case 5:  prefix = "sas_"; break;   /* Special */
        case 6:  prefix = "sax_"; break;   /* Extended */
        default: return 0;
    }

    count = 0;
    for (i = start; i <= end; i++) {
        if (g_se.next_buffer > SE_BUFFER_END) {
            LOG_DEBUG("SE buffer full");
            break;
        }

        buffer_id = g_se.next_buffer;
        snprintf(path, sizeof(path), "%sdata\\se\\%s%02d.wav",
                 config_get_data_path(), prefix, i);

        if (sound_load(buffer_id, path, 0)) {
            g_se.loaded_se[buffer_id - SE_BUFFER_START] = (series << 16) | i;
            g_se.next_buffer++;
            count++;
        }
    }

    LOG_DEBUG("Loaded %d SE from series %d", count, series);
    return count;
}

/*
 * Get SE buffer ID from SE ID
 */
static int se_get_buffer_id(u32 se_id) {
    u32 i;
    u32 target = se_id;

    for (i = 0; i < SE_BUFFER_COUNT; i++) {
        if (g_se.loaded_se[i] == target) {
            return SE_BUFFER_START + i;
        }
    }
    return -1;
}

/*
 * Play sound effect by ID
 */
int se_play(u32 se_id) {
    int buffer_id;

    if (!g_sound.sound_enabled) return 0;

    buffer_id = se_get_buffer_id(se_id);
    if (buffer_id < 0) {
        /* Try to load dynamically */
        char path[MAX_SOUND_PATH];
        u32 series = (se_id >> 16) & 0xFF;
        u32 num = se_id & 0xFF;
        const char* prefix;

        switch (series) {
            case 1:  prefix = "sa1_"; break;
            case 2:  prefix = "sa2_"; break;
            case 3:  prefix = "sa3_"; break;
            case 4:  prefix = "saam_"; break;
            case 5:  prefix = "sas_"; break;
            case 6:  prefix = "sax_"; break;
            default: return 0;
        }

        if (g_se.next_buffer > SE_BUFFER_END) {
            /* Evict LRU */
            sound_cache_evict_lru();
        }

        snprintf(path, sizeof(path), "%sdata\\se\\%s%02d.wav",
                 config_get_data_path(), prefix, num);

        if (!sound_load(g_se.next_buffer, path, 0)) {
            return 0;
        }

        g_se.loaded_se[g_se.next_buffer - SE_BUFFER_START] = se_id;
        buffer_id = g_se.next_buffer;
        g_se.next_buffer++;
    }

    sound_stop((u32)buffer_id);
    return sound_play((u32)buffer_id);
}

/*
 * Play sound effect with volume
 */
int se_play_with_volume(u32 se_id, s32 volume) {
    int buffer_id = se_get_buffer_id(se_id);
    if (buffer_id < 0) {
        if (!se_play(se_id)) return 0;
        buffer_id = se_get_buffer_id(se_id);
    }
    if (buffer_id >= 0) {
        sound_set_volume((u32)buffer_id, volume);
        return sound_play((u32)buffer_id);
    }
    return 0;
}

/*
 * Stop sound effect
 */
int se_stop(u32 se_id) {
    int buffer_id = se_get_buffer_id(se_id);
    if (buffer_id >= 0) {
        return sound_stop((u32)buffer_id);
    }
    return 0;
}

/*
 * Stop all sound effects
 */
int se_stop_all(void) {
    u32 i;
    for (i = SE_BUFFER_START; i <= SE_BUFFER_END; i++) {
        sound_stop(i);
    }
    return 1;
}

/*
 * Preload common sound effects
 */
int se_preload_common(void) {
    int count = 0;

    /* Load series 1 common sounds */
    count += se_load_series(1, 1, 17);  /* sa1_01 to sa1_17 */

    /* Load ambient sounds */
    count += se_load_series(4, 11, 27); /* saam_11 to saam_27 */

    LOG_DEBUG("Preloaded %d common sound effects", count);
    return count;
}
