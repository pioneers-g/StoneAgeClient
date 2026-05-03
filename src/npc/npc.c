/*
 * Stone Age Client - NPC Dialogue Script System
 * Reverse engineered from sa_9061.exe (FUN_00462f60, FUN_00463380, FUN_004781f0, FUN_00488ea0)
 *
 * Core NPC system: initialization, talk window, dialogue management
 * Split modules:
 *   - npc_shop.c: Shop functions
 *   - npc_dialog.c: Dialog action functions
 *   - npc_packet.c: Packet handlers
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "npc.h"
#include "npc_shop.h"
#include "npc_dialog.h"
#include "config.h"
#include "logger.h"

/* Global NPC context */
NPCContext g_npc = {0};

/* Talk window state */
static TalkWindow g_talk_window = {0};

/* Dialog menu mode - DAT_046308b0 pattern */
static u32 s_dialog_menu_mode = 0;

/* External send function from network module */
extern void send_queue_add(const char* data, int len, int flags);

/* External network function for binary mode */
extern void network_send_raw(void* data, int size);

/* Dialogue entry pool - FUN_00488ca0 pattern */
#define MAX_DIALOGUE_ENTRIES    40
#define DIALOGUE_ENTRY_SIZE     0x118

/*
 * Initialize NPC system - FUN_00488ca0 pattern
 */
int npc_init(void) {
    int i;
    DialogueEntry* entry;
    DialogueEntry* prev;

    memset(&g_npc, 0, sizeof(NPCContext));
    memset(&g_talk_window, 0, sizeof(TalkWindow));

    /* Allocate dialogue entry pool */
    g_talk_window.entries = (DialogueEntry*)malloc(sizeof(DialogueEntry));
    if (!g_talk_window.entries) {
        LOG_ERROR("Failed to allocate dialogue entry pool");
        return 0;
    }

    /* Initialize linked list of dialogue entries */
    prev = g_talk_window.entries;
    memset(prev, 0, sizeof(DialogueEntry));

    for (i = 0; i < MAX_DIALOGUE_ENTRIES; i++) {
        entry = (DialogueEntry*)malloc(sizeof(DialogueEntry));
        if (!entry) {
            LOG_ERROR("Failed to allocate dialogue entry %d", i);
            npc_shutdown();
            return 0;
        }

        memset(entry, 0, sizeof(DialogueEntry));
        prev->next = entry;
        entry->prev = prev;
        prev = entry;
    }

    /* Close the circular list */
    prev->next = NULL;
    g_talk_window.tail = prev;
    g_talk_window.current = g_talk_window.entries;
    g_talk_window.head = g_talk_window.entries;
    g_talk_window.initialized = 1;

    /* Load default skin */
    npc_load_skin("data\\skin\\default");

    LOG_INFO("NPC system initialized with %d dialogue entries", MAX_DIALOGUE_ENTRIES);
    return 1;
}

/*
 * Shutdown NPC system - FUN_00488dc0 pattern
 */
void npc_shutdown(void) {
    DialogueEntry* entry;
    DialogueEntry* next;
    int i;

    /* Free device contexts */
    for (i = 0; i < 5; i++) {
        if (g_talk_window.hdc[i]) {
            SelectObject(g_talk_window.hdc[i], g_talk_window.hOldBmp[i]);
            DeleteObject(g_talk_window.hBmp[i]);
            DeleteDC(g_talk_window.hdc[i]);
        }
    }

    if (g_talk_window.hdcBuffer) {
        SelectObject(g_talk_window.hdcBuffer, g_talk_window.hOldBufferBmp);
        DeleteObject(g_talk_window.hBufferBmp);
        DeleteDC(g_talk_window.hdcBuffer);
    }

    /* Kill timer */
    if (g_talk_window.hWnd) {
        KillTimer(g_talk_window.hWnd, 1);
    }

    /* Free dialogue entries */
    entry = g_talk_window.entries;
    for (i = 0; i < MAX_DIALOGUE_ENTRIES && entry; i++) {
        next = entry->next;
        free(entry);
        entry = next;
    }

    /* Free shop items */
    if (g_npc.shop_items) {
        free(g_npc.shop_items);
        g_npc.shop_items = NULL;
    }

    memset(&g_npc, 0, sizeof(NPCContext));
    memset(&g_talk_window, 0, sizeof(TalkWindow));

    LOG_INFO("NPC system shutdown");
}

/*
 * Load talk window skin - FUN_00489070 pattern
 */
int npc_load_skin(const char* path) {
    char filename[128];
    int i;
    HANDLE hBitmap;

    for (i = 0; i < 5; i++) {
        /* Build skin filename */
        _snprintf(filename, sizeof(filename), "%s\\talk%d.bmp", path, i + 1);

        /* Load bitmap */
        hBitmap = LoadImageA(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        g_talk_window.hBmp[i] = hBitmap;

        if (!hBitmap) {
            LOG_WARN("Failed to load skin: %s", filename);
            continue;
        }

        /* Create device context */
        g_talk_window.hdc[i] = CreateCompatibleDC(NULL);
        g_talk_window.hOldBmp[i] = SelectObject(g_talk_window.hdc[i], hBitmap);
    }

    /* Create buffer DC */
    g_talk_window.hdcBuffer = CreateCompatibleDC(g_talk_window.hdc[0]);
    g_talk_window.hBufferBmp = CreateCompatibleBitmap(g_talk_window.hdc[0], 645, 155);
    g_talk_window.hOldBufferBmp = SelectObject(g_talk_window.hdcBuffer, g_talk_window.hBufferBmp);

    LOG_INFO("Loaded NPC skin from %s", path);
    return 1;
}

/*
 * Show talk window - FUN_00488ea0 pattern
 */
int npc_show_talk_window(HINSTANCE hInstance) {
    WNDCLASSA wc;
    RECT rect;
    HWND hWnd;
    HRGN hRgn;
    DWORD style;

    if (!g_talk_window.initialized) {
        return 0;
    }

    /* Check if window already exists */
    if (g_talk_window.hWnd) {
        ShowWindow(g_talk_window.hWnd, SW_SHOW);
        SetFocus(g_talk_window.hMainWnd);
        return 1;
    }

    /* Register window class */
    wc.hInstance = hInstance;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = npc_window_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursorA(hInstance, (LPCSTR)0x68);
    wc.hbrBackground = GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "TalkWindowClass";

    if (!RegisterClassA(&wc)) {
        LOG_ERROR("Failed to register talk window class");
        return 0;
    }

    /* Get main window position */
    GetWindowRect(g_talk_window.hMainWnd, &rect);

    /* Create talk window */
    hWnd = CreateWindowExA(0, "TalkWindowClass", "TalkWIndow",
                           0, rect.left, rect.bottom + rect.top,
                           645, 155, g_talk_window.hMainWnd, NULL, hInstance, NULL);

    g_talk_window.hWnd = hWnd;

    if (!hWnd) {
        LOG_ERROR("Failed to create talk window");
        return 0;
    }

    /* Show and update window */
    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    /* Create region for window shaping */
    int captionHeight = GetSystemMetrics(SM_CYCAPTION);
    int borderWidth = GetSystemMetrics(SM_CXBORDER);
    hRgn = CreateRectRgn(0, 0, 645, captionHeight + 155 + borderWidth);
    SetWindowRgn(hWnd, hRgn, TRUE);
    DeleteObject(hRgn);

    /* Remove window decorations */
    style = GetWindowLongA(hWnd, GWL_STYLE);
    SetWindowLongA(hWnd, GWL_STYLE, style & ~(WS_CAPTION | WS_THICKFRAME | WS_SYSMENU));

    /* Invalidate and resize */
    InvalidateRect(hWnd, NULL, TRUE);
    SetWindowPos(hWnd, NULL, 0, 0, 645, captionHeight + 155, SWP_NOMOVE | SWP_NOZORDER);

    /* Set timer for animation */
    SetTimer(hWnd, 1, 500, NULL);

    /* Initialize IME */
    npc_init_ime();

    SetFocus(g_talk_window.hMainWnd);

    LOG_INFO("Talk window shown");
    return 1;
}

/*
 * Hide talk window - FUN_00489810 pattern
 */
void npc_hide_talk_window(void) {
    if (g_talk_window.hWnd) {
        ShowWindow(g_talk_window.hWnd, SW_HIDE);
        npc_clear_dialogue();
    }
}

/*
 * Initialize IME for input - FUN_00489050 pattern
 */
void npc_init_ime(void) {
    /* IME initialization if needed */
}

/*
 * Add dialogue entry
 */
int npc_add_dialogue(u32 npc_id, const char* text, u16 portrait_id, u8 flags) {
    DialogueEntry* entry;

    if (!g_talk_window.initialized) {
        return 0;
    }

    /* Get next available entry */
    entry = g_talk_window.current;
    if (!entry) {
        return 0;
    }

    /* Set entry data */
    entry->npc_id = npc_id;
    entry->portrait_id = portrait_id;
    entry->flags = flags;
    entry->timestamp = timeGetTime();

    /* Copy text */
    strncpy(entry->text, text, sizeof(entry->text) - 1);
    entry->text[sizeof(entry->text) - 1] = '\0';

    /* Move to next entry */
    g_talk_window.current = entry->next;
    g_talk_window.entry_count++;

    LOG_DEBUG("Added dialogue: NPC=%u, Text=%s", npc_id, text);
    return 1;
}

/*
 * Clear all dialogue entries - FUN_004897d0 pattern
 */
void npc_clear_dialogue(void) {
    DialogueEntry* entry;

    if (!g_talk_window.initialized) {
        return;
    }

    /* Clear all entries in the pool */
    for (entry = g_talk_window.entries; entry != NULL; entry = entry->next) {
        memset(entry, 0, sizeof(DialogueEntry));
    }

    /* Reset pointers */
    g_talk_window.current = g_talk_window.entries;
    g_talk_window.head = g_talk_window.entries;
    g_talk_window.entry_count = 0;

    LOG_DEBUG("Dialogue cleared");
}

/*
 * Set current dialogue - FUN_00489740 pattern
 */
void npc_set_dialogue(u32 npc_id, const char* text) {
    if (!g_talk_window.initialized) {
        return;
    }

    g_talk_window.active_npc = npc_id;

    if (text && text[0]) {
        npc_add_dialogue(npc_id, text, 0, 0);
    }
}

/*
 * Send NPC response - FUN_00478190 pattern
 */
void npc_send_response(u32 npc_id) {
    char packet[64];

    /* Build NPC response packet: T|npc_id */
    snprintf(packet, sizeof(packet), "T|%u", npc_id);

    /* Send via text protocol */
    send_queue_add(packet, 0, 0);

    LOG_DEBUG("Sent NPC response for %u", npc_id);
}

/*
 * Find NPC by ID
 */
NPCData* npc_find_by_id(u32 npc_id) {
    u32 i;

    for (i = 0; i < g_npc.npc_count; i++) {
        if (g_npc.npcs[i].id == npc_id) {
            return &g_npc.npcs[i];
        }
    }

    return NULL;
}

/*
 * Find shop item
 */
int npc_find_shop_item(u32 item_id) {
    int i;

    for (i = 0; i < g_npc.shop_count; i++) {
        if (g_npc.shop_items[i].id == item_id) {
            return i;
        }
    }

    return -1;
}

/*
 * Update shop display
 */
void npc_update_shop(void) {
    /* Update shop UI */
    LOG_DEBUG("Updating shop display");
}

/*
 * Render dialogue
 */
void npc_render_dialog(void) {
    /* Render dialogue to talk window */
    LOG_DEBUG("Rendering dialogue");
}

/*
 * Window procedure for talk window
 */
LRESULT CALLBACK npc_window_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT:
            npc_render_talk_window(hWnd);
            return 0;

        case WM_TIMER:
            /* Animation update */
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;

        case WM_LBUTTONDOWN:
            /* Handle click */
            npc_handle_click(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_KEYDOWN:
            /* Handle key input */
            if (wParam == VK_ESCAPE) {
                npc_hide_talk_window();
            }
            return 0;

        case WM_CLOSE:
            ShowWindow(hWnd, SW_HIDE);
            return 0;

        default:
            return DefWindowProcA(hWnd, msg, wParam, lParam);
    }
}

/*
 * Render talk window content
 */
void npc_render_talk_window(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hdc;
    DialogueEntry* entry;
    int y;
    RECT rect;

    hdc = BeginPaint(hWnd, &ps);

    if (!g_talk_window.hdcBuffer) {
        EndPaint(hWnd, &ps);
        return;
    }

    /* Draw background skins */
    BitBlt(g_talk_window.hdcBuffer, 0, 0, 645, 155,
           g_talk_window.hdc[0], 0, 0, SRCCOPY);

    /* Draw dialogue entries */
    y = 10;
    entry = g_talk_window.head;

    while (entry && y < 140) {
        if (entry->text[0]) {
            /* Set text rect */
            SetRect(&rect, 20, y, 625, y + 20);

            /* Draw text */
            SetTextColor(g_talk_window.hdcBuffer, RGB(255, 255, 255));
            SetBkMode(g_talk_window.hdcBuffer, TRANSPARENT);
            DrawTextA(g_talk_window.hdcBuffer, entry->text, -1, &rect,
                      DT_LEFT | DT_WORDBREAK);

            y += 25;
        }
        entry = entry->next;
    }

    /* Copy to window */
    BitBlt(hdc, 0, 0, 645, 155, g_talk_window.hdcBuffer, 0, 0, SRCCOPY);

    EndPaint(hWnd, &ps);
}

/*
 * Handle mouse click
 */
void npc_handle_click(int x, int y) {
    /* Check if click is in dialogue area */
    if (y > 100) {
        /* Click on options area */
        npc_select_option((x - 20) / 150);
    } else {
        /* Click on dialogue - advance */
        npc_advance_dialogue();
    }
}

/*
 * Select dialogue option
 */
void npc_select_option(int option) {
    if (option >= 0 && option < 4) {
        LOG_DEBUG("Selected option %d", option);
        /* Send selection to server */
        npc_send_option(option);
    }
}

/*
 * Send option to server - dialog choice
 */
void npc_send_option(int option) {
    char packet[64];

    /* Build option packet: D|option */
    snprintf(packet, sizeof(packet), "D|%d", option);

    /* Send via text protocol */
    send_queue_add(packet, 0, 0);

    LOG_DEBUG("Sent option %d to server", option);
}

/*
 * Advance dialogue
 */
void npc_advance_dialogue(void) {
    if (g_talk_window.current && g_talk_window.current->next) {
        g_talk_window.current = g_talk_window.current->next;
        if (g_talk_window.hWnd) {
            InvalidateRect(g_talk_window.hWnd, NULL, TRUE);
        }
    } else {
        /* End of dialogue */
        npc_hide_talk_window();
    }
}

/*
 * Handle NPC interaction - send interaction packet
 */
void npc_interact(u32 npc_id) {
    char packet[64];

    g_npc.current_npc = npc_id;

    /* Build interaction packet: T|npc_id */
    snprintf(packet, sizeof(packet), "T|%u", npc_id);

    /* Send via text protocol */
    send_queue_add(packet, 0, 0);

    LOG_DEBUG("Interacting with NPC %u", npc_id);
}

/*
 * Quest NPC handling
 */
void npc_handle_quest(u32 npc_id, u32 quest_id) {
    g_npc.quest_npc = npc_id;
    g_npc.current_quest = quest_id;
    g_npc.in_quest = 1;

    LOG_DEBUG("Quest NPC %u, quest %u", npc_id, quest_id);
}

/*
 * End quest interaction
 */
void npc_end_quest(void) {
    g_npc.quest_npc = 0;
    g_npc.current_quest = 0;
    g_npc.in_quest = 0;
}

/*
 * Set player data reference
 */
void npc_set_player_data(void* player_data) {
    g_npc.player_data = player_data;
}

/*
 * Get talk window handle
 */
HWND npc_get_talk_window(void) {
    return g_talk_window.hWnd;
}

/*
 * Check if talk window is visible
 */
int npc_is_talk_visible(void) {
    return g_talk_window.hWnd && IsWindowVisible(g_talk_window.hWnd);
}

/*
 * Close NPC dialog
 */
void npc_close_dialog(void) {
    g_talk_window.npc_id = 0;
    if (g_talk_window.hWnd) {
        ShowWindow(g_talk_window.hWnd, SW_HIDE);
    }
    g_npc.in_dialog = 0;
}

/*
 * Parse field from delimited string - FUN_00489f70 pattern
 */
int npc_parse_field(const char* str, char delimiter, int field_index, char* output, int max_len) {
    const char* ptr = str;
    const char* start;
    int current_field = 0;
    int len;

    if (!str || !output || max_len <= 0) {
        return -1;
    }

    output[0] = '\0';

    while (*ptr) {
        if (current_field == field_index) {
            start = ptr;
            while (*ptr && *ptr != delimiter) {
                ptr++;
            }
            len = (int)(ptr - start);
            if (len >= max_len) {
                len = max_len - 1;
            }
            strncpy(output, start, len);
            output[len] = '\0';
            return len;
        }
        if (*ptr == delimiter) {
            current_field++;
        }
        ptr++;
    }

    return -1;
}

/*
 * Parse integer field from delimited string - FUN_0048a050 pattern
 */
int npc_parse_field_int(const char* str, char delimiter, int field_index) {
    char buffer[32];
    if (npc_parse_field(str, delimiter, field_index, buffer, sizeof(buffer)) > 0) {
        return atoi(buffer);
    }
    return 0;
}

/*
 * Set dialog window position - used by npc_dialog.c
 */
void npc_set_dialog_position(int x, int y) {
    if (g_talk_window.hWnd) {
        SetWindowPos(g_talk_window.hWnd, NULL, x, y, 0, 0,
                     SWP_NOSIZE | SWP_NOZORDER);
    }
}
