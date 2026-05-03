/*
 * Stone Age Client - Character Selection Screen
 * Reverse engineered from sa_9061.exe
 *
 * Protocol Commands:
 * - CharList: Request character list
 * - CreateNewChar|name|face|body|job: Create new character
 * - CharDelete|name: Delete character
 * - CharLogin|name: Login with character
 * - CharLogout: Logout from game
 * - PlayerNumGet: Get online player count
 *
 * Protocol Analysis:
 * FUN_0045fb80 - Character list handler
 * FUN_00463c20 - Create character result
 * FUN_00463d20 - Delete character result
 * FUN_0045fdc0 - Character login handler
 * FUN_00478a30 - Character slot parser
 */

#ifndef CHARSELECT_H
#define CHARSELECT_H

#include "types.h"

/* Protocol commands */
#define CHAR_CMD_LIST           "CharList"      /* Request character list */
#define CHAR_CMD_CREATE         "CreateNewChar" /* Create new character */
#define CHAR_CMD_DELETE         "CharDelete"    /* Delete character */
#define CHAR_CMD_LOGIN          "CharLogin"     /* Login with character */
#define CHAR_CMD_LOGOUT         "CharLogout"    /* Logout from game */
#define CHAR_CMD_PLAYER_COUNT   "PlayerNumGet"  /* Get online player count */

/* Server response strings */
#define CHAR_RESP_SUCCESS       "successful"
#define CHAR_RESP_OUT_OF_SERVICE "OUTOFSERVICE"
#define CHAR_RESP_NOT_FOUND     "char not found"

/* Error codes */
#define CHAR_ERROR_NOT_FOUND    -502    /* Character not found */

/* Character selection sub-states - DAT_04630df0 values */
typedef enum {
    CHAR_SELECT_STATE_INIT = 0,
    CHAR_SELECT_STATE_WAIT_LIST = 1,
    CHAR_SELECT_STATE_DISPLAY = 2,
    CHAR_SELECT_STATE_SELECTING = 3,
    CHAR_SELECT_STATE_ENTER_CREATE = 4,
    CHAR_SELECT_STATE_CREATING = 5,
    CHAR_SELECT_STATE_ERROR = 100,
    CHAR_SELECT_STATE_ERROR_MSG = 101,
    CHAR_SELECT_STATE_SERVER_WAIT = 200,
    CHAR_SELECT_STATE_SERVER_CONNECT = 201
} CharSelectState;

/* Character creation sub-states */
typedef enum {
    CHAR_CREATE_STATE_INIT = 0,
    CHAR_CREATE_STATE_WAIT_NETWORK = 1,
    CHAR_CREATE_STATE_DISPLAY = 10,
    CHAR_CREATE_STATE_CONFIRM_DELETE = 11,
    CHAR_CREATE_STATE_SELECT_CHAR = 20,
    CHAR_CREATE_STATE_DELETE_WAIT = 21,
    CHAR_CREATE_STATE_DELETE_CONFIRM = 22,
    CHAR_CREATE_STATE_ERROR = 100,
    CHAR_CREATE_STATE_DELETED = 300
} CharCreateState;

/* Character slot data - matches DAT_04630a00 structure */
typedef struct {
    char name[24];          /* 0x00 - Character name */
    u16 unknown_18;         /* 0x18 */
    u16 level;              /* 0x1a - Level */
    u16 hp;                 /* 0x1c - HP */
    u16 max_hp;             /* 0x1e - Max HP */
    u16 mp;                 /* 0x20 - MP */
    u16 max_mp;             /* 0x22 - Max MP */
    u32 exp;                /* 0x24 - Experience */
    u16 str;                /* 0x28 - Strength */
    u16 dex;                /* 0x2a - Dexterity */
    u16 vit;                /* 0x2c - Vitality */
    u16 agi;                /* 0x2e - Agility */
    u16 intel;              /* 0x30 - Intelligence */
    u16 luck;               /* 0x32 - Luck */
    u16 charm;              /* 0x34 - Charm */
    u16 unknown_36;         /* 0x36 */
    u32 unknown_38;         /* 0x38 */
    u32 unknown_3c;         /* 0x3c */
    u32 map_id;             /* 0x40 - Map ID */
    u8  exists;             /* 0x44 - Slot has character */
    u8  padding[3];
} CharSelectSlot;          /* Size: 0x44 = 68 bytes */

/* Error codes from FUN_00421c00 */
typedef enum {
    CHAR_SELECT_ERR_NONE = 0,
    CHAR_SELECT_ERR_SERVER = -1,
    CHAR_SELECT_ERR_CONNECTION = -2,
    CHAR_SELECT_ERR_TIMEOUT = -3,
    CHAR_SELECT_ERR_VERSION = -4,
    CHAR_SELECT_ERR_FULL = -5,
    CHAR_SELECT_ERR_INVALID_NAME = -6,
    CHAR_SELECT_ERR_NAME_EXISTS = -7,
    CHAR_SELECT_ERR_CREATE_FAILED = -8,
    CHAR_SELECT_ERR_DELETE_FAILED = -9,
    CHAR_SELECT_ERR_NO_SLOTS = -10,
    CHAR_SELECT_ERR_NOT_FOUND = -11,
    CHAR_SELECT_ERR_101 = -101,
    CHAR_SELECT_ERR_102 = -102,
    CHAR_SELECT_ERR_103 = -103,
    CHAR_SELECT_ERR_200 = -200,
    CHAR_SELECT_ERR_201 = -202,
    CHAR_SELECT_ERR_203 = -203,
    CHAR_SELECT_ERR_204 = -205,
    CHAR_SELECT_ERR_300 = -301,
    CHAR_SELECT_ERR_301 = -302,
    CHAR_SELECT_ERR_303 = -303,
    CHAR_SELECT_ERR_400 = -401,
    CHAR_SELECT_ERR_402 = -402
} CharSelectError;

/* Context for character selection screen */
typedef struct {
    /* Current state - DAT_04630df0 */
    int state;
    int sub_state;

    /* Character slots - DAT_04630a00, max 7 characters */
    CharSelectSlot slots[7];
    int slot_count;
    int selected_slot;

    /* Error message */
    char error_msg[128];
    int error_code;

    /* UI state */
    int scroll_offset;
    int create_face;
    int create_body;
    int create_job;
    char create_name[24];

    /* Animation */
    u32 animation_timer;
    int animation_frame;

    /* Flags */
    int is_creating;
    int is_deleting;
    int confirm_delete;

} CharSelectContext;

/* Global context */
extern CharSelectContext g_charselect;

/* Initialization */
int charselect_init(void);
void charselect_shutdown(void);

/* State machine - FUN_00421110 */
void charselect_update(void);
void charselect_render(void);

/* Character creation state machine - FUN_00421c00 */
void charcreate_update(void);
void charcreate_render(void);

/* Input handling */
int charselect_handle_click(int x, int y);
int charselect_handle_key(u32 key);
int charcreate_handle_click(int x, int y);
int charcreate_handle_key(u32 key);

/* Character operations */
int charselect_select_slot(int slot);
int charselect_enter_game(void);
int charselect_start_create(void);
int charselect_delete_character(void);
int charcreate_confirm(void);
int charcreate_cancel(void);

/* Server response handlers */
void charselect_handle_list(void* data, int size);
void charselect_handle_create_result(int result, const char* name);
void charselect_handle_delete_result(int result, const char* name);
void charselect_handle_enter_result(int result);

/* State queries */
int charselect_get_state(void);
int charselect_get_selected_slot(void);
CharSelectSlot* charselect_get_slot(int index);
int charselect_has_characters(void);

#endif /* CHARSELECT_H */
