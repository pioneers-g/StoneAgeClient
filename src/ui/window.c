/*
 * Stone Age Client - Window Management
 * Reverse engineered from sa_9061.exe (FUN_0043f830, FUN_0043fae0)
 */

#include <windows.h>
#include "types.h"
#include "config.h"
#include "window.h"
#include "resource.h"
#include "logger.h"

/* Window class name from DAT_004b88c4 */
static const char* WINDOW_CLASS = "StoneAge";

/* Window dimensions from FUN_00440170 */
#define DEFAULT_FULLSCREEN_WIDTH    640
#define DEFAULT_FULLSCREEN_HEIGHT   480
#define DEFAULT_WINDOWED_WIDTH      320
#define DEFAULT_WINDOWED_HEIGHT     240

/* External global state */
extern GlobalState g_state;

/* Register window class */
int window_register_class(HINSTANCE hInstance) {
    WNDCLASSA wc;

    memset(&wc, 0, sizeof(WNDCLASSA));

    /* Style from binary: 0x1008 = CS_DBLCLKS */
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = window_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCE(0x70));
    wc.hCursor = LoadCursorA(hInstance, MAKEINTRESOURCE(0x68));
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WINDOW_CLASS;

    if (!RegisterClassA(&wc)) {
        LOG_ERROR("Failed to register window class, error: %lu", GetLastError());
        return 0;
    }

    LOG_INFO("Window class registered: %s", WINDOW_CLASS);
    return 1;
}

/* Create main window - FUN_0043f830 pattern */
HWND window_create(HINSTANCE hInstance, int window_mode) {
    HWND hWnd;
    DWORD dwStyle;
    RECT rect;
    int width, height;

    /* Calculate window style - from FUN_0043f830:
     * windowed: (0x70ca0000) + 0x90000000 = WS_VISIBLE | WS_OVERLAPPEDWINDOW
     * fullscreen: 0x90000000 = WS_VISIBLE | WS_POPUP
     */
    if (window_mode) {
        dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        width = DEFAULT_WINDOWED_WIDTH;
        height = DEFAULT_WINDOWED_HEIGHT;
    } else {
        dwStyle = WS_POPUP | WS_VISIBLE;
        width = DEFAULT_FULLSCREEN_WIDTH;
        height = DEFAULT_FULLSCREEN_HEIGHT;
    }

    /* Calculate window size - AdjustWindowRectEx from FUN_0043f830 */
    SetRect(&rect, 0, 0, width, height);
    AdjustWindowRectEx(&rect, dwStyle, 0, 0);

    /* Create window */
    hWnd = CreateWindowExA(
        0,
        WINDOW_CLASS,
        WINDOW_CLASS,
        dwStyle,
        0, 0,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hWnd == NULL) {
        LOG_ERROR("Failed to create window, error: %lu", GetLastError());
        return NULL;
    }

    /* Show and update - from FUN_0043f830 */
    ShowWindow(hWnd, g_state.nCmdShow);
    UpdateWindow(hWnd);

    LOG_INFO("Window created: %dx%d, mode: %s",
             width, height, window_mode ? "windowed" : "fullscreen");

    return hWnd;
}

/* Destroy window */
void window_destroy(void) {
    if (g_state.hWnd) {
        DestroyWindow(g_state.hWnd);
        g_state.hWnd = NULL;
    }
    UnregisterClassA(WINDOW_CLASS, g_state.hInstance);
}

/* Set window style */
void window_set_style(HWND hWnd, int window_mode) {
    DWORD dwStyle;
    RECT rect;
    int width, height;

    if (window_mode) {
        dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        width = DEFAULT_WINDOWED_WIDTH;
        height = DEFAULT_WINDOWED_HEIGHT;
    } else {
        dwStyle = WS_POPUP | WS_VISIBLE;
        width = DEFAULT_FULLSCREEN_WIDTH;
        height = DEFAULT_FULLSCREEN_HEIGHT;
    }

    SetRect(&rect, 0, 0, width, height);
    AdjustWindowRectEx(&rect, dwStyle, 0, 0);

    SetWindowLongA(hWnd, GWL_STYLE, dwStyle);
    SetWindowPos(hWnd, HWND_TOP, 0, 0,
                 rect.right - rect.left, rect.bottom - rect.top,
                 SWP_FRAMECHANGED);
}

/* Update window */
void window_update(void) {
    if (g_state.hWnd) {
        UpdateWindow(g_state.hWnd);
    }
}

/*
 * Window procedure - LAB_0043fae0
 * This is referenced in FUN_0043f1f0 at DAT_0455fdcc
 */
LRESULT CALLBACK window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            LOG_DEBUG("WM_CREATE");
            break;

        case WM_DESTROY:
            LOG_DEBUG("WM_DESTROY");
            PostQuitMessage(0);
            break;

        case WM_CLOSE:
            LOG_DEBUG("WM_CLOSE");
            DestroyWindow(hWnd);
            break;

        case WM_KEYDOWN:
            LOG_DEBUG("WM_KEYDOWN: %d", (int)wParam);
            /* Handle keyboard input */
            if (wParam == VK_ESCAPE && !g_config.window_mode) {
                PostQuitMessage(0);
            }
            break;

        case WM_SIZE:
            LOG_DEBUG("WM_SIZE: %dx%d", LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_ACTIVATE:
            LOG_DEBUG("WM_ACTIVATE: %d", LOWORD(wParam));
            break;

        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                /* Game rendering handled by DirectDraw */
                EndPaint(hWnd, &ps);
            }
            break;

        default:
            return DefWindowProcA(hWnd, message, wParam, lParam);
    }

    return 0;
}
