/*
 * Stone Age Client - UI Input Handling
 * Mouse and keyboard input for UI elements
 */

#ifndef UI_INPUT_H
#define UI_INPUT_H

#include "types.h"

/* ========================================
 * Mouse Input Handling
 * ======================================== */

int ui_handle_mouse_move(int x, int y);
int ui_handle_mouse_down(int x, int y, int button);
int ui_handle_mouse_up(int x, int y, int button);
void ui_handle_click(int x, int y);

/* ========================================
 * Keyboard Input Handling
 * ======================================== */

int ui_handle_key_down(int key);
int ui_handle_char(int ch);

/* ========================================
 * Main Input Processing
 * ======================================== */

void ui_handle_input(void);

#endif /* UI_INPUT_H */
