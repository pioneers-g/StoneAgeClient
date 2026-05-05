/*
 * Stone Age Client - Sound Stub Functions
 * Split from stubs.c to reduce file size
 */

#include <windows.h>
#include "types.h"

/* Fade/render area globals for FUN_00488190 */
static u32 s_fade_mode = 0;
static u32 s_fade_width = 0;
static u32 s_fade_height = 0;

/*
 * FUN_00488190 - Render Area / Fade Palette Setup
 * Sets render target dimensions for battle fade effects.
 * param_1: mode/flags (0xca = default battle fade)
 * param_2: width (0x140=320 low-res, 0x280=640 high-res)
 * param_3: height (0xf0=240 low-res, 0x1e0=480 high-res)
 */
void FUN_00488190(int param_1, int param_2, int param_3) {
    s_fade_mode = (u32)param_1;
    s_fade_width = (u32)param_2;
    s_fade_height = (u32)param_3;
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
