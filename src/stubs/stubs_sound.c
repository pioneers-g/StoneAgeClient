/*
 * Stone Age Client - Sound Stub Functions
 * Split from stubs.c to reduce file size
 */

#include <windows.h>
#include "types.h"

/*
 * FUN_00488190 - Play Sound/BGM
 *
 * Binary analysis:
 * - Plays sound effect or background music
 * - param_1: sound/bgm ID or type
 * - param_2: volume or parameter
 * - param_3: additional parameter
 */
void FUN_00488190(int param_1, int param_2, int param_3) {
    (void)param_1; (void)param_2; (void)param_3;
}

/*
 * FUN_004872b0 - WAV File Header Parser
 *
 * Binary analysis:
 * - Parses WAV file format header using Windows multimedia I/O
 * - param_1: HMMIO file handle (from mmioOpen)
 * - param_2: output WAVEFORMATEX structure (18 bytes)
 * - param_3: output data chunk size
 * - Returns 1 on success, 0 on failure
 *
 * Process:
 * 1. Descend into 'WAVE' chunk (fccType = 0x45564157 = "WAVE")
 * 2. Descend into 'fmt ' chunk (ckid = 0x20746d66 = "fmt ")
 * 3. Read 18 bytes of format data (WAVEFORMATEX)
 * 4. Validate format tag is 1 (PCM)
 * 5. Ascend from fmt chunk
 * 6. Descend into 'data' chunk (ckid = 0x61746164 = "data")
 * 7. Return data chunk size
 *
 * WAVEFORMATEX structure (param_2):
 * - offset 0: wFormatTag (1 = PCM)
 * - offset 2: nChannels
 * - offset 4: nSamplesPerSec
 * - offset 8: nAvgBytesPerSec
 * - offset 12: nBlockAlign
 * - offset 14: wBitsPerSample
 * - offset 16: cbSize
 */
int FUN_004872b0(void* hmmio, void* format, u32* data_size) {
    (void)hmmio; (void)format; (void)data_size;
    /* TODO: Full WAV parsing implementation with mmioDescend/mmioRead */
    return 0;
}

/* Sound function stubs */
void sound_play_bgm(int bgm_id) { (void)bgm_id; }
void sound_play_effect(int effect_id) { (void)effect_id; }
