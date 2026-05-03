/*
 * Stone Age Client - Game Loop Manager Implementation
 * Reverse engineered from sa_9061.exe FUN_0041db40
 * Main game loop that coordinates all systems
 *
 * Key mechanisms:
 * - Timing seed stored as string (obfuscated frame timing)
 * - Anti-tamper buffer relocation every 2 minutes
 * - String-based seed for frame rate limiting
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tlhelp32.h>
#include "types.h"
#include "gameloop.h"
#include "gamestate.h"
#include "network.h"
#include "render.h"
#include "map.h"
#include "character.h"
#include "input.h"
#include "fade.h"
#include "logger.h"

/* Global game loop context */
GameLoopContext g_gameloop = {0};

/* Anti-tamper buffers - from DAT_0461b41c and gBuffer (DAT_0461b418) */
LPVOID g_antitamper_buf1 = NULL;  /* DAT_0461b41c */
LPVOID g_antitamper_buf2 = NULL;  /* gBuffer */
size_t g_antitamper_size1 = 0;
size_t g_antitamper_size2 = 0;

/* Anti-tamper toggle flag - DAT_004ab7d4 */
static int g_antitamper_toggle = 0;

/* Timing seed from binary */
#define TIMING_SEED 0xffffbcde

/* State flags - matches DAT_005ab724 bits */
#define STATE_FLAG_TIMING_INIT    0x01
#define STATE_FLAG_TIMING_SET     0x02
#define STATE_FLAG_KEY1_INIT      0x04
#define STATE_FLAG_KEY2_INIT      0x08

/* Key scan codes */
#define SCAN_CODE_1    0x02
#define SCAN_CODE_2    0x03

/* String buffer for timing seed (from local_20 in FUN_0041db40) */
static char s_timing_seed_str[32] = {0};

/*
 * Convert integer to string (base 10) - FUN_0049b135 pattern
 * Used for timing seed string storage
 */
static void timing_seed_to_string(int value, char* buffer, int base) {
    char* ptr = buffer;
    char* ptr1 = buffer;
    char tmp_char;
    int is_negative = 0;
    unsigned int uvalue;

    if (base == 10 && value < 0) {
        is_negative = 1;
        uvalue = (unsigned int)(-value);
    } else {
        uvalue = (unsigned int)value;
    }

    /* Convert to string (reversed) */
    do {
        int digit = uvalue % base;
        *ptr++ = (char)(digit < 10 ? digit + '0' : digit + 'W');
        uvalue /= base;
    } while (uvalue != 0);

    /* Add negative sign if needed */
    if (is_negative) {
        *ptr++ = '-';
    }

    *ptr = '\0';

    /* Reverse the string */
    ptr--;
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

/*
 * Convert string to integer - FUN_00492973 pattern
 * Used for timing seed parsing
 */
static int timing_string_to_int(const char* str) {
    const char* ptr = str;
    int result = 0;
    int sign = 1;

    /* Skip whitespace */
    while (*ptr == ' ' || *ptr == '\t') {
        ptr++;
    }

    /* Handle sign */
    if (*ptr == '-') {
        sign = -1;
        ptr++;
    } else if (*ptr == '+') {
        ptr++;
    }

    /* Parse digits */
    while (*ptr >= '0' && *ptr <= '9') {
        result = result * 10 + (*ptr - '0');
        ptr++;
    }

    return sign * result;
}

/*
 * Initialize timing seed string - FUN_0049b108 pattern
 */
static void init_timing_seed_string(u32 seed) {
    timing_seed_to_string((int)seed, s_timing_seed_str, 10);
}

/*
 * Get timing value from seed string - FUN_004929fe pattern
 */
static u32 get_timing_value(void) {
    return (u32)timing_string_to_int(s_timing_seed_str);
}

/*
 * Initialize game loop
 */
int gameloop_init(void) {
    DWORD current_time;
    DWORD seed;

    memset(&g_gameloop, 0, sizeof(GameLoopContext));

    g_gameloop.running = 0;
    g_gameloop.paused = 0;
    g_gameloop.mode = GAMELOOP_MODE_NORMAL;

    /* Initialize timing seed - from FUN_0041db40
     * The seed is stored as a string for obfuscation */
    current_time = timeGetTime();
    seed = current_time ^ TIMING_SEED;
    g_gameloop.timing_seed = seed;
    init_timing_seed_string(seed);

    /* Allocate anti-tamper buffers - DAT_0461b41c and gBuffer
     * First buffer: PAGE_READWRITE (used for code)
     * Second buffer: PAGE_EXECUTE_READWRITE */
    g_antitamper_buf1 = VirtualAlloc(NULL, 0x4000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (g_antitamper_buf1) {
        g_antitamper_size1 = 0x4000;
    }

    g_antitamper_buf2 = VirtualAlloc(NULL, 0x4000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (g_antitamper_buf2) {
        g_antitamper_size2 = 0x4000;
    }

    /* Create chat directory - from FUN_0041e260 */
    CreateDirectoryA("chat", NULL);

    LOG_INFO("Game loop initialized (anti-tamper: buf1=%p, buf2=%p)",
             g_antitamper_buf1, g_antitamper_buf2);
    return 1;
}

/*
 * Shutdown game loop
 */
void gameloop_shutdown(void) {
    g_gameloop.running = 0;

    if (g_antitamper_buf1) {
        VirtualFree(g_antitamper_buf1, 0, MEM_RELEASE);
        g_antitamper_buf1 = NULL;
    }
    if (g_antitamper_buf2) {
        VirtualFree(g_antitamper_buf2, 0, MEM_RELEASE);
        g_antitamper_buf2 = NULL;
    }

    memset(&g_gameloop, 0, sizeof(GameLoopContext));
    LOG_INFO("Game loop shutdown");
}

/*
 * Process window messages - FUN_0043fa70 pattern
 * Returns 0 if WM_QUIT received, 1 otherwise
 */
static int gameloop_process_messages(void) {
    MSG msg;

    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return 0;
        }

        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 1;
}

/*
 * Anti-tamper code relocation - from FUN_0041db40
 * Called every 2 minutes to relocate code buffers
 *
 * The binary toggles between DAT_0461b41c and gBuffer:
 * - Toggle == 1: Relocate DAT_0461b41c (buf1)
 * - Toggle == 0: Relocate gBuffer (buf2)
 */
static void gameloop_antitamper_relocate(void) {
    DWORD old_protect;
    LPVOID new_buf;
    u32* src32;
    u32* dst32;
    u8* src8;
    u8* dst8;
    size_t size;
    size_t i;

    /* Toggle flag - DAT_004ab7d4 = (DAT_004ab7d4 - 1) & 1 */
    g_antitamper_toggle = (g_antitamper_toggle - 1) & 1;

    if (g_antitamper_toggle == 1) {
        /* Relocate buffer 1 (DAT_0461b41c) */
        if (g_antitamper_buf1 && g_antitamper_size1 > 0) {
            new_buf = VirtualAlloc(NULL, 0x4000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (new_buf) {
                /* Change protection to readwrite for copying */
                VirtualProtect(g_antitamper_buf1, 0x4000, PAGE_READWRITE, &old_protect);

                /* Copy in 32-bit chunks first (faster) */
                size = g_antitamper_size1;
                src32 = (u32*)g_antitamper_buf1;
                dst32 = (u32*)new_buf;
                for (i = size >> 2; i != 0; i--) {
                    *dst32++ = *src32++;
                }

                /* Copy remaining bytes */
                src8 = (u8*)src32;
                dst8 = (u8*)dst32;
                for (i = size & 3; i != 0; i--) {
                    *dst8++ = *src8++;
                }

                VirtualFree(g_antitamper_buf1, 0, MEM_RELEASE);
                g_antitamper_buf1 = new_buf;

                /* Set execute permission */
                VirtualProtect(new_buf, 0x4000, PAGE_EXECUTE_READ, &old_protect);
            }
        }
    } else {
        /* Relocate buffer 2 (gBuffer) */
        if (g_antitamper_buf2 && g_antitamper_size2 > 0) {
            new_buf = VirtualAlloc(NULL, 0x4000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (new_buf) {
                /* Change protection to readwrite for copying */
                VirtualProtect(g_antitamper_buf2, 0x4000, PAGE_READWRITE, &old_protect);

                /* Copy in 32-bit chunks first (faster) */
                size = g_antitamper_size2;
                src32 = (u32*)g_antitamper_buf2;
                dst32 = (u32*)new_buf;
                for (i = size >> 2; i != 0; i--) {
                    *dst32++ = *src32++;
                }

                /* Copy remaining bytes */
                src8 = (u8*)src32;
                dst8 = (u8*)dst32;
                for (i = size & 3; i != 0; i--) {
                    *dst8++ = *src8++;
                }

                VirtualFree(g_antitamper_buf2, 0, MEM_RELEASE);
                g_antitamper_buf2 = new_buf;

                /* Set execute permission */
                VirtualProtect(new_buf, 0x4000, PAGE_EXECUTE_READ, &old_protect);
            }
        }
    }
}

/*
 * Check for explorer.exe process - from FUN_0041db40
 * Used for anti-tamper detection
 */
static int gameloop_check_explorer_process(DWORD process_id) {
    HANDLE snapshot;
    PROCESSENTRY32 pe32;
    int found;

    if (process_id == 0) return 0;

    pe32.dwSize = sizeof(PROCESSENTRY32);
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    found = 0;
    if (Process32First(snapshot, &pe32)) {
        do {
            if (pe32.th32ProcessID == process_id) {
                /* Check if not explorer.exe */
                if (strcmp(pe32.szExeFile, "explorer.exe") != 0) {
                    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
                    if (hProcess) {
                        TerminateProcess(hProcess, 0);
                        CloseHandle(hProcess);
                    }
                }
                found = 1;
                break;
            }
        } while (Process32Next(snapshot, &pe32));
    }

    CloseHandle(snapshot);
    return found;
}

/*
 * Main game loop - FUN_0041db40 pattern
 *
 * Key behaviors from binary:
 * 1. Timing seed stored as string, parsed each frame
 * 2. Anti-tamper buffer relocation every 2 minutes
 * 3. Frame rate limiting using XOR'd timing value
 * 4. Resolution switch handling with F11/F12
 */
int gameloop_run(void) {
    u32 current_time;
    u32 frame_start;
    u32 anti_tamper_timer;
    u32 key1_timer = 0;
    u32 key2_timer = 0;
    u32 periodic_timer = 0;
    u32 timing_value;
    u32 target_time;
    u32 validation_key = 0;
    int frame_count = 0;

    LOG_INFO("Starting game loop");

    g_gameloop.running = 1;
    current_time = timeGetTime();

    /* Set initial anti-tamper timer - DAT_005ab718 */
    anti_tamper_timer = current_time + ANTITAMPER_INTERVAL_MS;

    /* Initialize timing seed string */
    g_gameloop.timing_seed = current_time ^ TIMING_SEED;
    init_timing_seed_string(g_gameloop.timing_seed);

    /* Mark timing as initialized (DAT_005ab724 |= 1) */
    g_gameloop.state_flags |= STATE_FLAG_TIMING_INIT;

    while (g_gameloop.running) {
        /* Process window messages - FUN_0043fa70 */
        if (!gameloop_process_messages()) {
            LOG_INFO("WM_QUIT received, exiting game loop");
            break;
        }

        /* Frame timing */
        frame_start = timeGetTime();
        g_gameloop.frame_start = frame_start;

        /* Reset counters - DAT_0464f488 = 0, DAT_005ab6f8 = 0 */
        g_gameloop.stats.frames_skipped = 0;

        /* Network I/O - FUN_0045e880 */
        network_process();

        /* Input update - FUN_00488c70, FUN_00480bd0 */
        input_update();

        /* Periodic 500ms update for screenshots */
        if ((g_input.key_ext_press_edge & 0x800) &&
            (frame_start - periodic_timer >= 500)) {
            /* Screenshot capture - FUN_00414e30 */
            periodic_timer = frame_start;
        }

        /* Toggle fullscreen on F-key - DAT_004a2620 toggle */
        if (g_input.key_ext_press_edge & 0x400) {
            g_gameloop.fullscreen_toggle = !g_gameloop.fullscreen_toggle;
        }

        /* Game state dispatch - FUN_004799b0 */
        gamestate_dispatch();

        /* Additional per-frame processing */
        /* FUN_0045db10 - send queue processing */
        /* FUN_0047a640 - fade effect update */
        /* FUN_0047d8c0 - render update */
        /* FUN_0045dd10 - network heartbeat */
        /* FUN_00412740 - sprite update */

        /* Anti-tamper check every 2 minutes */
        current_time = timeGetTime();
        if (current_time >= anti_tamper_timer) {
            anti_tamper_timer = current_time + ANTITAMPER_INTERVAL_MS;
            gameloop_antitamper_relocate();
        }

        /* Update timing seed string for frame limiting */
        timing_value = get_timing_value();
        target_time = timing_value ^ TIMING_SEED;

        /* Initialize validation key if needed (DAT_005ab728) */
        if (!(g_gameloop.state_flags & STATE_FLAG_TIMING_SET)) {
            g_gameloop.state_flags |= STATE_FLAG_TIMING_SET;
            validation_key = timing_value ^ 0x7aa04981;
        }

        /* Frame rate limiting - from FUN_0041db40
         * Wait until current_time > target_time */
        while (current_time <= target_time) {
            Sleep(1);
            current_time = timeGetTime();
        }

        /* Update timing seed for next frame */
        g_gameloop.timing_seed = current_time ^ TIMING_SEED;
        init_timing_seed_string(g_gameloop.timing_seed);

        /* Update statistics */
        g_gameloop.stats.total_frames++;
        g_gameloop.last_frame_time = timeGetTime() - frame_start;
        frame_count++;

        /* FPS calculation */
        {
            static u32 fps_counter = 0;
            static u32 fps_timer = 0;

            fps_counter++;
            fps_timer += g_gameloop.last_frame_time;

            if (fps_timer >= 1000) {
                g_gameloop.stats.fps = fps_counter;
                fps_counter = 0;
                fps_timer = 0;
            }
        }
    }

    g_gameloop.running = 0;
    LOG_INFO("Game loop ended (total frames: %u)", frame_count);

    return 0;
}

/*
 * Stop game loop
 */
void gameloop_stop(void) {
    g_gameloop.running = 0;
}

/*
 * Pause game loop
 */
void gameloop_pause(void) {
    g_gameloop.paused = 1;
    LOG_DEBUG("Game loop paused");
}

/*
 * Resume game loop
 */
void gameloop_resume(void) {
    g_gameloop.paused = 0;
    LOG_DEBUG("Game loop resumed");
}

/*
 * Set game loop mode
 */
void gameloop_set_mode(GameLoopMode mode) {
    g_gameloop.mode = mode;
}

/*
 * Set update callback
 */
void gameloop_set_update_callback(void (*callback)(u32)) {
    g_gameloop.on_update = callback;
}

/*
 * Set render callback
 */
void gameloop_set_render_callback(void (*callback)(void)) {
    g_gameloop.on_render = callback;
}

/*
 * Set input callback
 */
void gameloop_set_input_callback(void (*callback)(u32, u32, u32)) {
    g_gameloop.on_input = callback;
}

/*
 * Limit frame rate to target FPS - from FUN_0041db40 pattern
 *
 * The binary uses string-based timing for obfuscation:
 * 1. Parse timing seed string to get target frame time
 * 2. XOR with 0xffffbcde to get actual target
 * 3. Wait until current time exceeds target
 */
void gameloop_limit_fps(u32 frame_start) {
    u32 current_time;
    u32 target_time;
    u32 timing_value;

    /* Get timing value from string and decode */
    timing_value = get_timing_value();
    target_time = timing_value ^ TIMING_SEED;

    current_time = timeGetTime();

    /* Wait until we've passed the target time */
    while (current_time <= target_time) {
        Sleep(1);
        current_time = timeGetTime();
    }

    /* Update timing seed for next frame */
    g_gameloop.timing_seed = current_time ^ TIMING_SEED;
    init_timing_seed_string(g_gameloop.timing_seed);
}

/*
 * Get frame time
 */
u32 gameloop_get_frame_time(void) {
    return g_gameloop.last_frame_time;
}

/*
 * Get FPS
 */
u32 gameloop_get_fps(void) {
    return g_gameloop.stats.fps;
}

/*
 * Get game loop statistics
 */
void gameloop_get_stats(GameLoopStats* stats) {
    if (stats) {
        *stats = g_gameloop.stats;
    }
}

/*
 * Reset statistics
 */
void gameloop_reset_stats(void) {
    memset(&g_gameloop.stats, 0, sizeof(GameLoopStats));
}

/*
 * Check if running
 */
int gameloop_is_running(void) {
    return g_gameloop.running;
}

/*
 * Check if paused
 */
int gameloop_is_paused(void) {
    return g_gameloop.paused;
}

/*
 * Get timing seed
 */
u32 gameloop_get_timing_seed(void) {
    return g_gameloop.timing_seed;
}
