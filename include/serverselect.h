/*
 * Stone Age Client - Server Selection System Header
 * Reverse engineered from sa_9061.exe:
 *   FUN_004794d0 - Main server selection screen handler
 *   FUN_004792f0 - Initialize text input fields
 *   FUN_004793b0 - Detect server list item click
 *   FUN_00420ff0 - Find clicked button index
 *   FUN_0044aba0 - Rectangle click detection
 *
 * Server selection screen appears after login,
 * allows user to select server, enter username/password
 */

#ifndef SERVERSELECT_H
#define SERVERSELECT_H

#include "types.h"

/* Maximum lengths */
#define MAX_SERVER_NAME_LEN     64
#define MAX_USERNAME_LEN        31
#define MAX_PASSWORD_LEN        31
#define MAX_SERVER_LIST         10

/* Button indices - from FUN_004793b0 */
#define SERVER_BTN_OK           0
#define SERVER_BTN_CANCEL       1
#define SERVER_BTN_NEW_ACCOUNT  2
#define SERVER_BTN_CHANGE_PWD   3
#define SERVER_BTN_BACK         4

/* Button states - from DAT_04630c78 region */
#define BUTTON_STATE_NORMAL     -1
#define BUTTON_STATE_CLICKED    1

/* Server selection states */
typedef enum {
    SERVER_SELECT_INIT = 0,
    SERVER_SELECT_ACTIVE,
    SERVER_SELECT_CONNECTING,
    SERVER_SELECT_ERROR,
    SERVER_SELECT_DONE
} ServerSelectState;

/* Server entry structure */
typedef struct {
    char name[MAX_SERVER_NAME_LEN];
    char host[64];
    u16 port;
    u8 status;          /* 0 = offline, 1 = online, 2 = maintenance */
    u8 player_count;
    u16 ping;
    u8 selected;
} ServerEntry;

/* Text input field structure */
typedef struct {
    char buffer[64];
    u8 cursor_pos;
    u8 length;
    u8 max_length;
    u8 is_password;     /* If true, display as *** */
    u8 active;
    u8 changed;
    u16 x, y;
    u32 color;
} TextInputField;

/* Server selection context - matches DAT_04630b50 region */
typedef struct {
    /* Window handle */
    void* window;               /* DAT_04630dd4 - from FUN_00448610 */

    /* Button states */
    s32 btn_ok_state;           /* DAT_04630c78 */
    s32 btn_cancel_state;       /* DAT_04630c7c */
    s32 btn_newacct_state;      /* DAT_04630c80 */

    /* Click tracking */
    u32 click_sprites[3];       /* DAT_04630dbc, DAT_04630dc0, DAT_04630dc4 */
    s32 clicked_index;          /* Index of clicked item */

    /* Text input fields */
    TextInputField username;    /* DAT_04630b50 region */
    TextInputField password;    /* DAT_04630c90 region */

    /* Timing */
    u32 click_time;             /* DAT_04630db8 - from timeGetTime() */
    u32 init_count;             /* DAT_04630dc8 - initialization counter */

    /* Server list */
    ServerEntry servers[MAX_SERVER_LIST];
    u8 server_count;
    u8 selected_server;         /* DAT_04630dcc */
    u8 padding[2];

    /* UI positions from DAT_004cea58 region */
    s16 window_x;
    s16 window_y;
    s16 username_x;
    s16 username_y;
    s16 password_x;
    s16 password_y;

    /* State */
    ServerSelectState state;
    u8 initialized;

} ServerSelectContext;

/* Global context */
extern ServerSelectContext g_server_select;

/* Initialization - FUN_004794d0 */
int server_select_init(void);
void server_select_shutdown(void);

/* Main handler - FUN_004794d0 */
int server_select_update(void);

/* Text field initialization - FUN_004792f0 */
void server_select_init_fields(void);

/* Click detection - FUN_004793b0 */
int server_select_detect_click(int x, int y);

/* Button click check - FUN_00420ff0 */
int server_select_check_button_click(s32* button_ids, int count);

/* Rectangle click test - FUN_0044aba0 */
int server_select_rect_test(int x1, int y1, int x2, int y2, int sprite_type);

/* Render */
void server_select_render(void);

/* Input handling */
void server_select_handle_click(int x, int y);
void server_select_handle_key(u32 key, u32 flags);

/* Server list management */
int server_select_add_server(const char* name, const char* host, u16 port);
void server_select_clear_servers(void);
void server_select_set_server(int index);

/* Actions */
void server_select_connect(void);
void server_select_cancel(void);
void server_select_new_account(void);

/* Window close - FUN_004011c0 */
void server_select_close_window(void* window);

#endif /* SERVERSELECT_H */
