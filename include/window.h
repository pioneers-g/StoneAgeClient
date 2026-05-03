#ifndef WINDOW_H
#define WINDOW_H

#include "types.h"

/* Window procedure - LAB_0043fae0 */
LRESULT CALLBACK window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/* Window creation and management */
int window_register_class(HINSTANCE hInstance);
HWND window_create(HINSTANCE hInstance, int window_mode);
void window_destroy(void);

/* Window utilities */
void window_set_style(HWND hWnd, int window_mode);
void window_update(void);

#endif /* WINDOW_H */
