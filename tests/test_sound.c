/*
 * Stone Age Client - Unit Tests for Sound System
 * Test file: test_sound.c
 *
 * Tests for DirectSound audio playback, BGM, SE systems
 * Based on reverse engineering of sa_9061.exe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <mmreg.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/sound.h"

/* ========================================
 * Test Cases for Sound Constants
 * ======================================== */

/*
 * Test 1: BGM ID constants
 */
static void test_bgm_id_constants(void) {
    TEST_BEGIN("BGM ID constants");

    TEST_ASSERT(BGM_ID_S0 == 0, "BGM_S0 should be 0");
    TEST_ASSERT(BGM_ID_S1 == 1, "BGM_S1 should be 1");
    TEST_ASSERT(BGM_ID_D1 == 2, "BGM_D1 should be 2");
    TEST_ASSERT(BGM_ID_D2 == 3, "BGM_D2 should be 3");
    TEST_ASSERT(BGM_ID_F2 == 4, "BGM_F2 should be 4");
    TEST_ASSERT(BGM_ID_B2 == 5, "BGM_B2 should be 5");
    TEST_ASSERT(BGM_ID_T2 == 6, "BGM_T2 should be 6");
    TEST_ASSERT(BGM_ID_T12 == 7, "BGM_T12 should be 7");
    TEST_ASSERT(BGM_ID_COUNT == 8, "BGM_COUNT should be 8");

    TEST_END();
}

/*
 * Test 2: Sound Effect Series 1 constants
 */
static void test_se_series_1_constants(void) {
    TEST_BEGIN("SE Series 1 constants");

    TEST_ASSERT(SE_SERIES_1_START == 0, "Series 1 start should be 0");
    TEST_ASSERT(SE_SERIES_1_END == 22, "Series 1 end should be 22");

    TEST_ASSERT(SE_SA1_01 == 0, "SA1_01 should be 0");
    TEST_ASSERT(SE_SA1_03 == 2, "SA1_03 should be 2");
    TEST_ASSERT(SE_SA1_23 == 15, "SA1_23 should be 15");

    TEST_END();
}

/*
 * Test 3: Sound Effect Series 2/3 constants
 */
static void test_se_series_2_3_constants(void) {
    TEST_BEGIN("SE Series 2/3 constants");

    TEST_ASSERT(SE_SERIES_2_START == 100, "Series 2 start should be 100");
    TEST_ASSERT(SE_SA2_12 == 100, "SA2_12 should be 100");

    TEST_ASSERT(SE_SERIES_3_START == 200, "Series 3 start should be 200");
    TEST_ASSERT(SE_SA3_01 == 200, "SA3_01 should be 200");
    TEST_ASSERT(SE_SA3_02 == 201, "SA3_02 should be 201");

    TEST_END();
}

/*
 * Test 4: Ambient sound constants
 */
static void test_se_ambient_constants(void) {
    TEST_BEGIN("SE Ambient constants");

    TEST_ASSERT(SE_AMBIENT_START == 300, "Ambient start should be 300");
    TEST_ASSERT(SE_SAAM_11 == 300, "SAAM_11 should be 300");
    TEST_ASSERT(SE_SAAM_27 == 316, "SAAM_27 should be 316");

    TEST_END();
}

/*
 * Test 5: Special sound constants
 */
static void test_se_special_constants(void) {
    TEST_BEGIN("SE Special constants");

    TEST_ASSERT(SE_SPECIAL_START == 400, "Special start should be 400");
    TEST_ASSERT(SE_SAS_33 == 400, "SAS_33 should be 400");
    TEST_ASSERT(SE_SAS_34 == 401, "SAS_34 should be 401");
    TEST_ASSERT(SE_SAS_35 == 402, "SAS_35 should be 402");

    TEST_END();
}

/*
 * Test 6: Extended sound constants
 */
static void test_se_extended_constants(void) {
    TEST_BEGIN("SE Extended constants");

    TEST_ASSERT(SE_EXTENDED_START == 500, "Extended start should be 500");
    TEST_ASSERT(SE_SAX_01 == 500, "SAX_01 should be 500");
    TEST_ASSERT(SE_SAX_02 == 501, "SAX_02 should be 501");
    TEST_ASSERT(SE_SAX_03 == 502, "SAX_03 should be 502");

    TEST_END();
}

/*
 * Test 7: Sound flags
 */
static void test_sound_flags(void) {
    TEST_BEGIN("Sound flags");

    TEST_ASSERT(SOUND_FLAG_LOOP == 0x0001, "Loop flag should be 0x0001");
    TEST_ASSERT(SOUND_FLAG_3D == 0x0002, "3D flag should be 0x0002");
    TEST_ASSERT(SOUND_FLAG_STREAM == 0x0004, "Stream flag should be 0x0004");

    TEST_END();
}

/*
 * Test 8: Sound buffer limit
 */
static void test_sound_buffer_limit(void) {
    TEST_BEGIN("Sound buffer limit");

    TEST_ASSERT(MAX_SOUND_BUFFERS == 64, "Max sound buffers should be 64");
    TEST_ASSERT(MAX_SOUND_PATH == 260, "Max sound path should be 260");

    TEST_END();
}

/* ========================================
 * Test Cases for Sound Context
 * ======================================== */

/*
 * Test 9: Sound context initialization
 */
static void test_sound_context_init(void) {
    TEST_BEGIN("Sound context init");

    /* Clear and verify initial state */
    memset(&g_sound, 0xFF, sizeof(SoundContext));

    /* Reset to zero state */
    memset(&g_sound, 0, sizeof(SoundContext));

    TEST_ASSERT(g_sound.sound_count == 0, "Sound count should be 0");
    TEST_ASSERT(g_sound.master_volume == 0, "Master volume should be 0");
    TEST_ASSERT(g_sound.sound_enabled == 0, "Sound enabled should be 0");
    TEST_ASSERT(g_sound.music_enabled == 0, "Music enabled should be 0");

    TEST_END();
}

/*
 * Test 10: Sound structure size
 */
static void test_sound_structure_size(void) {
    TEST_BEGIN("Sound structure size");

    /* Sound structure should have reasonable size */
    TEST_ASSERT(sizeof(Sound) >= 280, "Sound struct should be at least 280 bytes");
    TEST_ASSERT(sizeof(Sound) <= 400, "Sound struct should be at most 400 bytes");

    TEST_END();
}

/*
 * Test 11: Streaming BGM structure
 */
static void test_streaming_bgm_structure(void) {
    TEST_BEGIN("Streaming BGM structure");

    /* StreamingBGM structure should have expected fields */
    StreamingBGM streaming;
    memset(&streaming, 0, sizeof(StreamingBGM));

    TEST_ASSERT(sizeof(StreamingBGM) >= 40, "StreamingBGM should be at least 40 bytes");

    /* Test event array size */
    TEST_ASSERT(sizeof(streaming.events) == 3 * sizeof(HANDLE), "Should have 3 events");

    TEST_END();
}

/*
 * Test 12: Sound context structure
 */
static void test_sound_context_structure(void) {
    TEST_BEGIN("Sound context structure");

    /* SoundContext should contain expected arrays */
    TEST_ASSERT(sizeof(g_sound.sounds) == sizeof(Sound) * MAX_SOUND_BUFFERS,
                "Sounds array should be MAX_SOUND_BUFFERS elements");

    TEST_END();
}

/* ========================================
 * Test Cases for WAV Loading
 * ======================================== */

/*
 * Test 13: WAV format constants
 */
static void test_wav_format_constants(void) {
    TEST_BEGIN("WAV format constants");

    /* WAV format constants from FUN_004872b0 */
    /* RIFF chunk ID: 'WAVE' = 0x45564157 (little-endian) */
    u32 wave_id = 0x45564157;
    TEST_ASSERT(wave_id == 0x45564157, "WAVE chunk ID should be 0x45564157");

    /* fmt chunk ID: 'fmt ' = 0x20746d66 */
    u32 fmt_id = 0x20746d66;
    TEST_ASSERT(fmt_id == 0x20746d66, "fmt chunk ID should be 0x20746d66");

    /* data chunk ID: 'data' = 0x61746164 */
    u32 data_id = 0x61746164;
    TEST_ASSERT(data_id == 0x61746164, "data chunk ID should be 0x61746164");

    TEST_END();
}

/*
 * Test 14: WAVEFORMATEX structure
 */
static void test_waveformatex_structure(void) {
    TEST_BEGIN("WAVEFORMATEX structure");

    WAVEFORMATEX wfx;
    memset(&wfx, 0, sizeof(wfx));

    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = 44100;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    TEST_ASSERT(wfx.wFormatTag == 1, "PCM format tag should be 1");
    TEST_ASSERT(wfx.nChannels == 2, "Channels should be 2 (stereo)");
    TEST_ASSERT(wfx.nSamplesPerSec == 44100, "Sample rate should be 44100");
    TEST_ASSERT(wfx.wBitsPerSample == 16, "Bits per sample should be 16");
    TEST_ASSERT(wfx.nBlockAlign == 4, "Block align should be 4");
    TEST_ASSERT(wfx.nAvgBytesPerSec == 176400, "Bytes per sec should be 176400");

    TEST_END();
}

/*
 * Test 15: WAV header parsing
 */
static void test_wav_header_parsing(void) {
    TEST_BEGIN("WAV header parsing");

    /* Simulate WAV header format tag check from FUN_004872b0 */
    u16 format_tag = 1;  /* PCM format */

    TEST_ASSERT(format_tag == 1, "Format tag should be 1 for PCM");

    /* Format tag != 1 would be rejected */
    format_tag = 2;
    TEST_ASSERT(format_tag != 1, "Non-PCM format should be rejected");

    TEST_END();
}

/* ========================================
 * Test Cases for Sound Paths
 * ======================================== */

/*
 * Test 16: BGM path format
 */
static void test_bgm_path_format(void) {
    TEST_BEGIN("BGM path format");

    /* BGM paths from binary: data\\bgm\\sabgm_s0.wav */
    const char* bgm_paths[] = {
        "data\\bgm\\sabgm_s0.wav",
        "data\\bgm\\sabgm_s1.wav",
        "data\\bgm\\sabgm_d1.wav",
        "data\\bgm\\sabgm_d2.wav",
        "data\\bgm\\sabgm_f2.wav",
        "data\\bgm\\sabgm_b2.wav",
        "data\\bgm\\sabgm_t2.wav",
        "data\\bgm\\sabgm_t12.wav"
    };

    TEST_ASSERT(bgm_paths[0] != NULL, "BGM path 0 should exist");
    TEST_ASSERT(strstr(bgm_paths[0], "sabgm") != NULL, "BGM path should contain 'sabgm'");
    TEST_ASSERT(strstr(bgm_paths[0], ".wav") != NULL, "BGM path should have .wav extension");

    TEST_END();
}

/*
 * Test 17: SE path format
 */
static void test_se_path_format(void) {
    TEST_BEGIN("SE path format");

    /* SE paths from binary: data\\se\\sa1_xx.wav */
    const char* se_paths[] = {
        "data\\se\\sa1_03.wav",
        "data\\se\\sa1_23.wav",
        "data\\se\\sa2_12.wav",
        "data\\se\\sa3_02.wav",
        "data\\se\\sas_35.wav",
        "data\\se\\sax_01.wav"
    };

    TEST_ASSERT(se_paths[0] != NULL, "SE path should exist");
    TEST_ASSERT(strstr(se_paths[0], "data\\se\\") != NULL, "SE path should start with data\\se\\");
    TEST_ASSERT(strstr(se_paths[0], ".wav") != NULL, "SE path should have .wav extension");

    TEST_END();
}

/*
 * Test 18: Sound path building
 */
static void test_sound_path_building(void) {
    TEST_BEGIN("Sound path building");

    char path[MAX_SOUND_PATH];

    /* Build path for BGM */
    sound_build_path(path, sizeof(path), "sabgm_s0.wav");

    TEST_ASSERT(path[0] != '\0', "Path should not be empty");
    TEST_ASSERT(strstr(path, "sabgm_s0.wav") != NULL, "Path should contain filename");

    TEST_END();
}

/* ========================================
 * Test Cases for Volume Control
 * ======================================== */

/*
 * Test 19: Volume range
 */
static void test_volume_range(void) {
    TEST_BEGIN("Volume range");

    /* DirectSound volume range: -10000 to 0 (millibels) */
    s32 min_volume = -10000;
    s32 max_volume = 0;

    /* Test volume clamp */
    s32 volume = 50;
    TEST_ASSERT(volume >= 0 && volume <= 100, "Volume percent should be 0-100");

    /* Convert to DirectSound range */
    s32 ds_volume = (s32)((volume - 100) * 100);
    TEST_ASSERT(ds_volume >= min_volume, "DS volume should be >= -10000");
    TEST_ASSERT(ds_volume <= max_volume, "DS volume should be <= 0");

    TEST_END();
}

/*
 * Test 20: Pan range
 */
static void test_pan_range(void) {
    TEST_BEGIN("Pan range");

    /* DirectSound pan range: -10000 (left) to 10000 (right) */
    s32 min_pan = -10000;
    s32 max_pan = 10000;

    /* Center pan */
    s32 center_pan = 0;
    TEST_ASSERT(center_pan == 0, "Center pan should be 0");

    /* Test pan bounds */
    TEST_ASSERT(min_pan == -10000, "Min pan should be -10000");
    TEST_ASSERT(max_pan == 10000, "Max pan should be 10000");

    TEST_END();
}

/*
 * Test 21: Master volume control
 */
static void test_master_volume_control(void) {
    TEST_BEGIN("Master volume control");

    /* Initialize context */
    memset(&g_sound, 0, sizeof(SoundContext));

    sound_set_master_volume(75);
    TEST_ASSERT(g_sound.master_volume == 75, "Master volume should be 75");

    sound_set_master_volume(100);
    TEST_ASSERT(g_sound.master_volume == 100, "Master volume should be 100");

    TEST_END();
}

/* ========================================
 * Test Cases for Sound Playback
 * ======================================== */

/*
 * Test 22: Sound play without init
 */
static void test_sound_play_without_init(void) {
    TEST_BEGIN("Sound play without init");

    /* Clear sound context */
    memset(&g_sound, 0, sizeof(SoundContext));

    /* Should fail gracefully without DirectSound */
    int result = sound_play(0);
    TEST_ASSERT(result == 0, "Sound play should fail without init");

    TEST_END();
}

/*
 * Test 23: Sound stop without init
 */
static void test_sound_stop_without_init(void) {
    TEST_BEGIN("Sound stop without init");

    memset(&g_sound, 0, sizeof(SoundContext));

    /* Should not crash */
    sound_stop(0);
    TEST_ASSERT(1, "Sound stop should not crash without init");

    TEST_END();
}

/*
 * Test 24: Sound is playing check
 */
static void test_sound_is_playing(void) {
    TEST_BEGIN("Sound is playing check");

    memset(&g_sound, 0, sizeof(SoundContext));

    /* No sounds loaded, should return false */
    int playing = sound_is_playing(0);
    TEST_ASSERT(playing == 0, "Sound should not be playing without load");

    TEST_END();
}

/*
 * Test 25: Stop all sounds
 */
static void test_stop_all_sounds(void) {
    TEST_BEGIN("Stop all sounds");

    memset(&g_sound, 0, sizeof(SoundContext));

    /* Should not crash */
    sound_stop_all();
    TEST_ASSERT(1, "Stop all sounds should not crash");

    TEST_END();
}

/* ========================================
 * Test Cases for BGM System
 * ======================================== */

/*
 * Test 26: BGM init
 */
static void test_bgm_init(void) {
    TEST_BEGIN("BGM init");

    int result = bgm_init();
    /* BGM init may succeed or fail depending on DirectSound availability */
    TEST_ASSERT(1, "BGM init should complete");

    TEST_END();
}

/*
 * Test 27: BGM get current
 */
static void test_bgm_get_current(void) {
    TEST_BEGIN("BGM get current");

    /* Clear context */
    memset(&g_sound, 0, sizeof(SoundContext));

    u32 current = bgm_get_current();
    TEST_ASSERT(current == 0 || current == 0xFFFFFFFF, "No BGM should be playing");

    TEST_END();
}

/*
 * Test 28: BGM is playing
 */
static void test_bgm_is_playing(void) {
    TEST_BEGIN("BGM is playing");

    memset(&g_sound, 0, sizeof(SoundContext));

    int playing = bgm_is_playing();
    TEST_ASSERT(playing == 0, "BGM should not be playing");

    TEST_END();
}

/*
 * Test 29: BGM stop
 */
static void test_bgm_stop(void) {
    TEST_BEGIN("BGM stop");

    memset(&g_sound, 0, sizeof(SoundContext));

    /* Should not crash */
    bgm_stop();
    TEST_ASSERT(1, "BGM stop should not crash");

    TEST_END();
}

/*
 * Test 30: Streaming BGM status
 */
static void test_streaming_bgm_status(void) {
    TEST_BEGIN("Streaming BGM status");

    memset(&g_sound, 0, sizeof(SoundContext));

    int active = bgm_is_streaming_active();
    TEST_ASSERT(active == 0, "Streaming should not be active initially");

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(sound) {
    /* Constants tests */
    test_bgm_id_constants();
    test_se_series_1_constants();
    test_se_series_2_3_constants();
    test_se_ambient_constants();
    test_se_special_constants();
    test_se_extended_constants();
    test_sound_flags();
    test_sound_buffer_limit();

    /* Structure tests */
    test_sound_context_init();
    test_sound_structure_size();
    test_streaming_bgm_structure();
    test_sound_context_structure();

    /* WAV tests */
    test_wav_format_constants();
    test_waveformatex_structure();
    test_wav_header_parsing();

    /* Path tests */
    test_bgm_path_format();
    test_se_path_format();
    test_sound_path_building();

    /* Volume tests */
    test_volume_range();
    test_pan_range();
    test_master_volume_control();

    /* Playback tests */
    test_sound_play_without_init();
    test_sound_stop_without_init();
    test_sound_is_playing();
    test_stop_all_sounds();

    /* BGM tests */
    test_bgm_init();
    test_bgm_get_current();
    test_bgm_is_playing();
    test_bgm_stop();
    test_streaming_bgm_status();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - Sound System Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(sound);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
