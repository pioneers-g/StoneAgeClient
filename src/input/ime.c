/*
 * Stone Age Client - IME (Input Method Editor) Support Implementation
 * Reverse engineered from sa_9061.exe FUN_00491780, FUN_00491950, FUN_00491c40
 *
 * Handles Chinese text input through Windows IME
 */

#include <windows.h>
#include <imm.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "ime.h"
#include "logger.h"

/* Global IME context */
IMEContext g_ime = {0};

/* Status strings - from DAT_004d7148, DAT_004d7150, DAT_004d7158, DAT_004d715c */
static const char* s_ime_off = "IME Off";
static const char* s_ime_on = "IME On";
static const char* s_ime_english_caps = "English";
static const char* s_ime_english_nocaps = "english";

/* ========================================
 * IME Initialization - FUN_00491780
 * ======================================== */

/*
 * Initialize IME system - FUN_00491780
 * Creates IME context and allocates buffers
 */
int ime_init(HWND hwnd) {
    SYSTEM_INFO sys_info;

    /* Create IME context */
    g_ime.imc = ImmCreateContext();
    if (!g_ime.imc) {
        LOG_ERROR("Failed to create IME context");
        return 0;
    }

    /* Create heap for IME buffers */
    GetSystemInfo(&sys_info);
    g_ime.heap = HeapCreate(HEAP_NO_SERIALIZE, sys_info.dwPageSize, 0x4000);
    if (!g_ime.heap) {
        ImmDestroyContext(g_ime.imc);
        g_ime.imc = NULL;
        LOG_ERROR("Failed to create IME heap");
        return 0;
    }

    /* Allocate buffers */
    g_ime.comp_buffer = (char*)HeapAlloc(g_ime.heap, HEAP_ZERO_MEMORY, IME_COMP_BUFFER_SIZE);
    if (!g_ime.comp_buffer) goto cleanup;

    g_ime.cand_buffer = (char*)HeapAlloc(g_ime.heap, HEAP_ZERO_MEMORY, IME_CAND_BUFFER_SIZE);
    if (!g_ime.cand_buffer) goto cleanup;

    g_ime.result_buffer = (char*)HeapAlloc(g_ime.heap, HEAP_ZERO_MEMORY, IME_RESULT_BUFFER_SIZE);
    if (!g_ime.result_buffer) goto cleanup;

    g_ime.status_buffer = (char*)HeapAlloc(g_ime.heap, HEAP_ZERO_MEMORY, IME_RESULT_BUFFER_SIZE);
    if (!g_ime.status_buffer) goto cleanup;

    g_ime.desc_buffer = (char*)HeapAlloc(g_ime.heap, HEAP_ZERO_MEMORY, IME_DESC_BUFFER_SIZE);
    if (!g_ime.desc_buffer) goto cleanup;

    g_ime.extra_buffer = (char*)HeapAlloc(g_ime.heap, HEAP_ZERO_MEMORY, IME_RESULT_BUFFER_SIZE);
    if (!g_ime.extra_buffer) goto cleanup;

    /* Store window handle */
    g_ime.hwnd = hwnd;

    /* Associate IME context with window */
    g_ime.old_imc = ImmAssociateContext(hwnd, g_ime.imc);

    /* Update IME status */
    ime_update_status();

    g_ime.ime_enabled = 1;

    LOG_INFO("IME system initialized");
    return 1;

cleanup:
    if (g_ime.comp_buffer) HeapFree(g_ime.heap, 0, g_ime.comp_buffer);
    if (g_ime.cand_buffer) HeapFree(g_ime.heap, 0, g_ime.cand_buffer);
    if (g_ime.result_buffer) HeapFree(g_ime.heap, 0, g_ime.result_buffer);
    if (g_ime.status_buffer) HeapFree(g_ime.heap, 0, g_ime.status_buffer);
    if (g_ime.desc_buffer) HeapFree(g_ime.heap, 0, g_ime.desc_buffer);
    if (g_ime.extra_buffer) HeapFree(g_ime.heap, 0, g_ime.extra_buffer);

    HeapDestroy(g_ime.heap);
    ImmDestroyContext(g_ime.imc);

    g_ime.heap = NULL;
    g_ime.imc = NULL;

    LOG_ERROR("Failed to allocate IME buffers");
    return 0;
}

/*
 * Shutdown IME system - FUN_00491950
 */
void ime_shutdown(void) {
    if (g_ime.imc) {
        ImmDestroyContext(g_ime.imc);
        ImmAssociateContext(g_ime.hwnd, g_ime.old_imc);
        g_ime.ime_enabled = 1;
        g_ime.imc = NULL;
    }

    if (g_ime.heap) {
        if (g_ime.comp_buffer) {
            HeapFree(g_ime.heap, 0, g_ime.comp_buffer);
            g_ime.comp_buffer = NULL;
        }
        if (g_ime.cand_buffer) {
            HeapFree(g_ime.heap, 0, g_ime.cand_buffer);
            g_ime.cand_buffer = NULL;
        }
        if (g_ime.result_buffer) {
            HeapFree(g_ime.heap, 0, g_ime.result_buffer);
            g_ime.result_buffer = NULL;
        }
        if (g_ime.status_buffer) {
            HeapFree(g_ime.heap, 0, g_ime.status_buffer);
            g_ime.status_buffer = NULL;
        }
        if (g_ime.desc_buffer) {
            HeapFree(g_ime.heap, 0, g_ime.desc_buffer);
            g_ime.desc_buffer = NULL;
        }
        if (g_ime.extra_buffer) {
            HeapFree(g_ime.heap, 0, g_ime.extra_buffer);
            g_ime.extra_buffer = NULL;
        }

        HeapDestroy(g_ime.heap);
        g_ime.heap = NULL;
    }

    LOG_INFO("IME system shutdown");
}

/* ========================================
 * IME Status - FUN_00491c40
 * ======================================== */

/*
 * Update IME status - FUN_00491c40
 * Gets keyboard layout, description, and conversion status
 */
void ime_update_status(void) {
    DWORD conversion = 0;
    DWORD sentence = 0;
    UINT desc_len;
    BOOL is_ime;

    /* Get current keyboard layout */
    g_ime.keyboard_layout = GetKeyboardLayout(0);

    /* Clear result buffer */
    if (g_ime.result_buffer) {
        g_ime.result_buffer[0] = '\0';
    }

    /* Check if current layout is an IME */
    is_ime = ImmIsIME(g_ime.keyboard_layout);

    if (!is_ime) {
        /* Not an IME - show English mode based on CapsLock */
        SHORT caps_state = GetKeyState(VK_CAPITAL);
        const char* status_str;

        if (caps_state & 1) {
            status_str = s_ime_english_caps;
        } else {
            status_str = s_ime_english_nocaps;
        }

        if (g_ime.desc_buffer) {
            strcpy(g_ime.desc_buffer, status_str);
        }
        return;
    }

    /* Get IME property */
    DWORD property = ImmGetProperty(g_ime.keyboard_layout, IGP_PROPERTY);
    g_ime.property_flag = (property >> 18) & 1;

    /* Get IME description */
    desc_len = ImmGetDescriptionA(g_ime.keyboard_layout, g_ime.desc_buffer, IME_DESC_BUFFER_SIZE);

    if (desc_len > 0 && g_ime.status_buffer) {
        /* Append to status buffer */
        strcat(g_ime.status_buffer, g_ime.desc_buffer);
    }

    /* Get conversion status */
    if (ImmGetConversionStatus(g_ime.imc, &conversion, &sentence)) {
        const char* mode_str;

        if (conversion & IME_CMODE_NATIVE) {
            mode_str = s_ime_on;
        } else {
            mode_str = s_ime_off;
        }

        if (g_ime.status_buffer) {
            strcat(g_ime.status_buffer, mode_str);
        }
    }
}

/*
 * Check if IME is available
 */
int ime_is_available(void) {
    return g_ime.imc != NULL;
}

/*
 * Check if IME is currently active
 */
int ime_is_active(void) {
    if (!g_ime.imc) return 0;

    DWORD conversion = 0;
    DWORD sentence = 0;

    if (ImmGetConversionStatus(g_ime.imc, &conversion, &sentence)) {
        return (conversion & IME_CMODE_NATIVE) != 0;
    }

    return 0;
}

/*
 * Get IME description string
 */
const char* ime_get_description(void) {
    return g_ime.desc_buffer ? g_ime.desc_buffer : "";
}

/*
 * Get IME status string
 */
const char* ime_get_status_string(void) {
    return g_ime.status_buffer ? g_ime.status_buffer : "";
}

/* ========================================
 * IME Control
 * ======================================== */

/*
 * Enable/disable IME
 */
void ime_set_enabled(int enabled) {
    if (g_ime.imc) {
        ImmSetOpenStatus(g_ime.imc, enabled ? TRUE : FALSE);
        g_ime.ime_enabled = enabled;
    }
}

/*
 * Toggle IME on/off
 */
void ime_toggle(void) {
    if (g_ime.imc) {
        BOOL current = ImmGetOpenStatus(g_ime.imc);
        ImmSetOpenStatus(g_ime.imc, !current);
        g_ime.ime_enabled = !current;
    }
}

/*
 * Set conversion mode
 */
void ime_set_conversion_mode(DWORD mode) {
    if (g_ime.imc) {
        DWORD conversion = 0;
        DWORD sentence = 0;

        if (ImmGetConversionStatus(g_ime.imc, &conversion, &sentence)) {
            ImmSetConversionStatus(g_ime.imc, mode, sentence);
        }
    }
}

/*
 * Get conversion mode
 */
DWORD ime_get_conversion_mode(void) {
    if (!g_ime.imc) return 0;

    DWORD conversion = 0;
    DWORD sentence = 0;

    ImmGetConversionStatus(g_ime.imc, &conversion, &sentence);
    return conversion;
}

/* ========================================
 * Composition String Handling
 * ======================================== */

/*
 * Get composition string
 */
int ime_get_composition_string(char* buffer, int size) {
    LONG len;

    if (!g_ime.imc || !buffer || size <= 0) return 0;

    len = ImmGetCompositionStringA(g_ime.imc, GCS_COMPSTR, NULL, 0);
    if (len <= 0) return 0;

    if (len >= size) len = size - 1;

    ImmGetCompositionStringA(g_ime.imc, GCS_COMPSTR, buffer, len);
    buffer[len] = '\0';

    return (int)len;
}

/*
 * Get composition string length
 */
int ime_get_composition_length(void) {
    if (!g_ime.imc) return 0;

    return (int)ImmGetCompositionStringA(g_ime.imc, GCS_COMPSTR, NULL, 0);
}

/*
 * Get candidate list
 */
int ime_get_candidate_list(char* buffer, int size) {
    DWORD len;

    if (!g_ime.imc || !buffer || size <= 0) return 0;

    len = ImmGetCandidateListA(g_ime.imc, 0, NULL, 0);
    if (len <= 0) return 0;

    if (len >= (DWORD)size) len = size - 1;

    ImmGetCandidateListA(g_ime.imc, 0, (LPCANDIDATELIST)buffer, len);

    return (int)len;
}

/*
 * Clear composition buffer
 */
void ime_clear_composition(void) {
    if (g_ime.comp_buffer) {
        memset(g_ime.comp_buffer, 0, IME_COMP_BUFFER_SIZE);
    }
}

/* ========================================
 * IME Window Position
 * ======================================== */

/*
 * Set composition window position
 */
void ime_set_composition_window(int x, int y) {
    COMPOSITIONFORM cf;

    if (!g_ime.imc) return;

    cf.dwStyle = CFS_POINT;
    cf.ptCurrentPos.x = x;
    cf.ptCurrentPos.y = y;
    cf.rcArea.left = 0;
    cf.rcArea.top = 0;
    cf.rcArea.right = 0;
    cf.rcArea.bottom = 0;

    ImmSetCompositionWindow(g_ime.imc, &cf);
}

/*
 * Set candidate window position
 */
void ime_set_candidate_window(int x, int y) {
    CANDIDATEFORM cf;

    if (!g_ime.imc) return;

    cf.dwIndex = 0;
    cf.dwStyle = CFS_POINT;
    cf.ptCurrentPos.x = x;
    cf.ptCurrentPos.y = y;
    cf.rcArea.left = 0;
    cf.rcArea.top = 0;
    cf.rcArea.right = 0;
    cf.rcArea.bottom = 0;

    ImmSetCandidateWindow(g_ime.imc, &cf);
}
