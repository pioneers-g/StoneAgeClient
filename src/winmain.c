/*
 * Stone Age Client - Windows Entry Point
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include "types.h"

/* Main entry function implemented in main.c */
extern int main_entry(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

/*
 * WinMain - Windows entry point
 * This matches the original binary's entry structure at 0x0043f1f0
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    return main_entry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

/*
 * CRT Entry Point - Entry function at 0x004936c3
 * This handles CRT initialization before calling WinMain
 */
#ifdef USE_CRT_ENTRY
void entry(void) {
    /* This would be the raw entry point if needed */
    /* The original binary has CRT startup code here */
}
#endif
