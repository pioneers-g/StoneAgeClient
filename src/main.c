/*
 * Stone Age Client - Main Entry Point
 * Reverse engineered from sa_9061.exe (FUN_0043f1f0, FUN_004936c3, FUN_00440170)
 *
 * WinMain flow:
 * 1. Check for "updated" command line parameter (anti-tamper)
 * 2. Get process info for parent detection
 * 3. Register window class with WNDPROC at LAB_0043fae0
 * 4. Initialize data file paths from binary strings
 * 5. Create main window (FUN_0043f830)
 * 6. Initialize DirectX (FUN_00411a00)
 * 7. Run game main loop (FUN_0041db40)
 */

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "config.h"
#include "window.h"
#include "commandline.h"
#include "logger.h"
#include "game.h"
#include "directx.h"

/* Global state - matches DAT_0455xxxx region */
GlobalState g_state = {0};
GameConfig g_config = {0};

/* Data file paths - from s_data_real_bin etc. at 0x004ba4ac */
char g_path_real_bin[MAX_PATH] = "data\\real.bin";           /* DAT_0455fdf0 */
char g_path_adrn_bin[MAX_PATH] = "data\\adrn.bin";           /* DAT_0455fef0 */
char g_path_spr_bin[MAX_PATH] = "data\\spr.bin";             /* DAT_0455f918 */
char g_path_spradrn_bin[MAX_PATH] = "data\\spradrn.bin";     /* DAT_04560110 */
char g_path_tokyohot_dat[MAX_PATH] = "tokyohot.dat";         /* DAT_0455fb44 */
char g_path_realtrue_bin[MAX_PATH] = "data\\realtrue.bin";   /* DAT_0455fff0 */
char g_path_adrntrue_bin[MAX_PATH] = "data\\adrntrue.bin";   /* DAT_0455fc44 */

/* Graphics context - allocated by FUN_00440170 via HeapAlloc(0x90) */
/* Stored at DAT_0054a90c, dimensions at offset 0x88 and 0x8c */
static HGLOBAL g_graphics_context = NULL;

/* Forward declarations */
static int check_updated_param(const char* cmdline);
static void copy_string_safe(char* dest, const char* src, size_t max_len);
static void setup_data_paths(const char* cmdline);

/*
 * Copy string safely with 32-bit optimization - FUN_0043f1f0 pattern
 * Used for copying data file paths from command line
 */
static void copy_string_safe(char* dest, const char* src, size_t max_len) {
    size_t len = 0;
    size_t i;
    const char* p;
    u32* dst32;
    const u32* src32;

    /* Get string length - matches binary pattern */
    p = src;
    while (*p++) len++;

    if (len == 0 || len >= max_len) {
        dest[0] = '\0';
        return;
    }

    /* Copy in 32-bit chunks for efficiency */
    dst32 = (u32*)dest;
    src32 = (const u32*)src;

    for (i = len >> 2; i != 0; i--) {
        *dst32++ = *src32++;
    }

    /* Copy remaining bytes */
    for (i = len & 3; i != 0; i--) {
        *((u8*)dst32)++ = *((const u8*)src32)++;
    }

    dest[len] = '\0';
}

/*
 * Check for "updated" command line parameter - FUN_00492020 pattern
 * Returns 1 if found, 0 if not found
 */
static int check_updated_param(const char* cmdline) {
    const char* param = "updated";
    const char* p;
    const char* q;

    if (!cmdline) return 0;

    /* Search for "updated" in command line */
    p = cmdline;
    while (*p) {
        /* Skip to next parameter start */
        while (*p && *p != ' ' && *p != '-' && *p != '/') p++;
        if (!*p) break;

        /* Skip the delimiter */
        if (*p == ' ') p++;
        if (!*p) break;

        /* Check if this is the parameter */
        q = param;
        while (*p && *q && (*p == *q || (*p | 0x20) == (*q | 0x20))) {
            p++;
            q++;
        }

        if (*q == '\0') {
            /* Found parameter - check end condition */
            if (*p == '\0' || *p == ' ' || *p == '-' || *p == '/') {
                return 1;
            }
        }

        /* Skip to next space */
        while (*p && *p != ' ') p++;
    }

    return 0;
}

/*
 * Setup data file paths from command line - FUN_0043f4b0 pattern
 * Parses command line parameters for custom data paths
 */
static void setup_data_paths(const char* cmdline) {
    char buffer[MAX_PATH];
    const char* val;

    /* Initialize command line parsing */
    cmdline_init(cmdline);

    /* Override default paths if specified in command line */

    /* real.bin path */
    if (g_cmdline.real_bin_path[0]) {
        copy_string_safe(g_path_real_bin, g_cmdline.real_bin_path, MAX_PATH);
    }

    /* adrn.bin path */
    if (g_cmdline.adrn_bin_path[0]) {
        copy_string_safe(g_path_adrn_bin, g_cmdline.adrn_bin_path, MAX_PATH);
    }

    /* spr.bin path */
    if (g_cmdline.spr_bin_path[0]) {
        copy_string_safe(g_path_spr_bin, g_cmdline.spr_bin_path, MAX_PATH);
    }

    /* spradrn.bin path */
    if (g_cmdline.spradrn_bin_path[0]) {
        copy_string_safe(g_path_spradrn_bin, g_cmdline.spradrn_bin_path, MAX_PATH);
    }

    /* tokyohot.dat path */
    if (g_cmdline.tokyohot_path[0]) {
        copy_string_safe(g_path_tokyohot_dat, g_cmdline.tokyohot_path, MAX_PATH);
    }

    /* realtrue.bin path */
    if (g_cmdline.realtrue_bin_path[0]) {
        copy_string_safe(g_path_realtrue_bin, g_cmdline.realtrue_bin_path, MAX_PATH);
    }

    /* adrntrue.bin path */
    if (g_cmdline.adrntrue_bin_path[0]) {
        copy_string_safe(g_path_adrntrue_bin, g_cmdline.adrntrue_bin_path, MAX_PATH);
    }

    /* Set window mode from command line */
    if (g_cmdline.window_mode) {
        g_config.window_mode = 1;
    }

    /* Set alpha blending flag */
    if (g_cmdline.use_alpha) {
        g_config.use_alpha = 1;
    }

    /* Set no delay flag */
    if (g_cmdline.no_delay) {
        g_config.no_delay = 1;
    }

    LOG_DEBUG("Data paths initialized:");
    LOG_DEBUG("  real.bin: %s", g_path_real_bin);
    LOG_DEBUG("  adrn.bin: %s", g_path_adrn_bin);
    LOG_DEBUG("  spr.bin: %s", g_path_spr_bin);
    LOG_DEBUG("  spradrn.bin: %s", g_path_spradrn_bin);
}

/*
 * Entry point - FUN_0043f1f0 pattern
 * Called from WinMain or CRT entry point
 *
 * Flow matches decompiled FUN_0043f1f0:
 * 1. Check for "updated" parameter via FUN_00492020
 * 2. GetCurrentProcessId -> DAT_0455f8f8
 * 3. CreateToolhelp32Snapshot -> DAT_0456021c
 * 4. Process32First/Next to find parent PID -> DAT_0455fa30
 * 5. Register window class if hPrevInstance == NULL
 * 6. Copy default data paths
 * 7. FUN_0043f4b0 - parse command line
 * 8. FUN_00440170 - allocate graphics context
 * 9. FUN_0043f830 - create window
 * 10. FUN_00488ca0 - cursor init
 * 11. FUN_0041db40 - main game loop
 */
int main_entry(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    DWORD version;
    int result = 0;
    HANDLE process_snapshot;
    PROCESSENTRY32 pe32;
    DWORD current_pid;

    /* Store command show and instance - DAT_045600f0, DAT_045600f8 */
    g_state.nCmdShow = nCmdShow;
    g_state.hInstance = hInstance;
    g_state.cmdline = lpCmdLine;

    /* Initialize logger first */
    logger_init();
    LOG_INFO("Stone Age Client v%d.%d.%d.%d starting...",
             GAME_VERSION_MAJOR, GAME_VERSION_MINOR,
             GAME_VERSION_PATCH, GAME_VERSION_BUILD);

    /* Check for "updated" parameter - FUN_00492020 at 0x004ba538 */
    if (!check_updated_param(lpCmdLine)) {
        MessageBoxA(NULL, "Please run the updater first.", "Stone Age", MB_OK | MB_ICONERROR);
        logger_shutdown();
        return 0;
    }

    /* Get current process ID - DAT_0455f8f8 */
    current_pid = GetCurrentProcessId();
    g_state.process_id = current_pid;

    /* Create process snapshot for parent detection - DAT_0456021c */
    process_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    g_state.process_snapshot = process_snapshot;

    if (process_snapshot != INVALID_HANDLE_VALUE) {
        /* Find our process entry to get parent info - FUN_0043f1f0 pattern */
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(process_snapshot, &pe32)) {
            do {
                if (pe32.th32ProcessID == current_pid) {
                    /* Store parent process ID - DAT_0455fa30 */
                    g_state.parent_process_id = pe32.th32ParentProcessID;
                    memcpy(g_state.parent_name, pe32.szExeFile, sizeof(pe32.szExeFile));
                    break;
                }
            } while (Process32Next(process_snapshot, &pe32));
        }

        /* Register window class if not already done - DAT_0455fdc8 region */
        if (hPrevInstance == NULL) {
            WNDCLASSA wc = {0};

            /* Window class style - 0x1008 (CS_DBLCLKS) from binary */
            wc.style = CS_DBLCLKS;
            wc.lpfnWndProc = window_proc;       /* LAB_0043fae0 */
            wc.cbClsExtra = 0;
            wc.cbWndExtra = 0;
            wc.hInstance = hInstance;
            wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCE(0x70));
            wc.hCursor = LoadCursorA(hInstance, MAKEINTRESOURCE(0x68));
            wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
            wc.lpszClassName = "StoneAge";      /* DAT_004b88c4 */

            if (!RegisterClassA(&wc)) {
                MessageBoxA(NULL, "Failed to register window class.", "Stone Age", MB_OK | MB_ICONERROR);
                CloseHandle(process_snapshot);
                logger_shutdown();
                return 0;
            }
        }

        /* Setup data file paths - default strings copied at FUN_0043f1f0 */
        setup_data_paths(lpCmdLine);

        /* Initialize configuration */
        config_init();

        /* Store paths in config for assets module */
        g_config.real_bin_path_ptr = g_path_real_bin;
        g_config.adrn_bin_path_ptr = g_path_adrn_bin;
        g_config.spr_bin_path_ptr = g_path_spr_bin;
        g_config.spradrn_bin_path_ptr = g_path_spradrn_bin;

        /* Allocate graphics context - FUN_00440170 pattern
         * Uses HeapAlloc with size 0x90, stored at DAT_0054a90c
         * Sets dimensions based on screen mode:
         *   Mode 0,2: 640x480 (0x280 x 0x1e0), sprite 64x48
         *   Mode 1:   320x240 (0x140 x 0xf0), sprite 32x24
         */
        if (!graphics_allocate_context(g_config.window_mode)) {
            MessageBoxA(NULL, "Failed to allocate graphics context.", "Stone Age", MB_OK | MB_ICONERROR);
            CloseHandle(process_snapshot);
            logger_shutdown();
            return 0;
        }

        /* Create window - FUN_0043f830 */
        g_state.hWnd = window_create(hInstance, g_config.window_mode);
        if (g_state.hWnd == NULL) {
            MessageBoxA(NULL, "Failed to create window.", "Stone Age", MB_OK | MB_ICONERROR);
            window_destroy();
            CloseHandle(process_snapshot);
            logger_shutdown();
            return 0;
        }

        /* Initialize DirectX - FUN_00411a00 */
        if (!directx_init(g_state.hWnd, g_config.window_mode)) {
            MessageBoxA(NULL, "Failed to initialize DirectX.", "Stone Age", MB_OK | MB_ICONERROR);
            window_destroy();
            CloseHandle(process_snapshot);
            logger_shutdown();
            return 0;
        }

        /* Initialize cursor - FUN_00488ca0 */
        cursor_init(g_state.hWnd, hInstance);

        /* Initialize game - FUN_0041e260 */
        if (!game_init()) {
            MessageBoxA(NULL, "Failed to initialize game.", "Stone Age", MB_OK | MB_ICONERROR);
            window_destroy();
            CloseHandle(process_snapshot);
            logger_shutdown();
            return 0;
        }

        /* Run main game loop - FUN_0041db40 */
        game_run();

        /* Cleanup */
        game_shutdown();
        window_destroy();
        CloseHandle(process_snapshot);

        LOG_INFO("Stone Age Client shutting down...");
    }

    logger_shutdown();
    return result;
}

/*
 * WinMain - actual entry point called by Windows
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    return main_entry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
