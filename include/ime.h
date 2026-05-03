/*
 * Stone Age Client - IME (Input Method Editor) Support
 * Reverse engineered from sa_9061.exe FUN_00491780, FUN_00491950, FUN_00491c40
 *
 * Handles Chinese text input through Windows IME
 */

#ifndef IME_H
#define IME_H

#include "types.h"

/* IME buffer sizes - from original binary */
#define IME_COMP_BUFFER_SIZE    0x1000  /* 4096 - composition buffer */
#define IME_CAND_BUFFER_SIZE    0x400   /* 1024 - candidate list buffer */
#define IME_RESULT_BUFFER_SIZE  0x100   /* 256 - result string buffer */
#define IME_DESC_BUFFER_SIZE    100     /* IME description */

/* IME context structure - matches DAT_04ec08xx region */
typedef struct {
    /* IME handles */
    HIMC    imc;                /* DAT_04ec08e4 - Input Method Context */
    HIMC    old_imc;            /* DAT_04ec08dc - Previous context */
    HWND    hwnd;               /* DAT_04ec08e0 - Associated window */
    HANDLE  heap;               /* DAT_04ec08e8 - IME heap */

    /* Buffers */
    char*   comp_buffer;        /* DAT_04ec08ec - Composition buffer (0x1000) */
    char*   cand_buffer;        /* DAT_04ec08d0 - Candidate buffer (0x400) */
    char*   result_buffer;      /* DAT_04ec08cc - Result buffer (0x100) */
    char*   status_buffer;      /* DAT_04ec08c8 - Status buffer (0x100) */
    char*   desc_buffer;        /* DAT_04ec08c4 - Description buffer (100) */
    char*   extra_buffer;       /* DAT_04ec08c0 - Extra buffer (0x100) */

    /* State */
    HKL     keyboard_layout;    /* DAT_04ec08f0 - Keyboard layout */
    int     ime_enabled;        /* DAT_04ec0900 - IME enabled flag */
    int     property_flag;      /* DAT_04ec0904 - IME property flag */

} IMEContext;

/* Global IME context */
extern IMEContext g_ime;

/* ========================================
 * IME Initialization - FUN_00491780
 * ======================================== */

/* Initialize IME system for a window */
int ime_init(HWND hwnd);

/* Shutdown IME system - FUN_00491950 */
void ime_shutdown(void);

/* ========================================
 * IME Status - FUN_00491c40
 * ======================================== */

/* Update IME status (description, conversion mode) */
void ime_update_status(void);

/* Check if IME is available */
int ime_is_available(void);

/* Check if IME is currently active */
int ime_is_active(void);

/* Get IME description string */
const char* ime_get_description(void);

/* Get IME status string */
const char* ime_get_status_string(void);

/* ========================================
 * IME Control
 * ======================================== */

/* Enable/disable IME */
void ime_set_enabled(int enabled);

/* Toggle IME on/off */
void ime_toggle(void);

/* Set conversion mode */
void ime_set_conversion_mode(DWORD mode);

/* Get conversion mode */
DWORD ime_get_conversion_mode(void);

/* ========================================
 * Composition String Handling
 * ======================================== */

/* Get composition string */
int ime_get_composition_string(char* buffer, int size);

/* Get composition string length */
int ime_get_composition_length(void);

/* Get candidate list */
int ime_get_candidate_list(char* buffer, int size);

/* Clear composition buffer */
void ime_clear_composition(void);

/* ========================================
 * IME Window Position
 * ======================================== */

/* Set composition window position */
void ime_set_composition_window(int x, int y);

/* Set candidate window position */
void ime_set_candidate_window(int x, int y);

#endif /* IME_H */
