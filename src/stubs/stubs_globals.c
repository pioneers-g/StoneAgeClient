/*
 * Stone Age Client - Global Variables Stub Implementation
 * Split from stubs.c to reduce file size
 */

#include <windows.h>
#include "types.h"

/* ========================================
 * Pixel Format Variables
 * ======================================== */

u32 s_pixel_format = 0;
u32 g_pixel_format = 0;
int g_resolution_mode = 0;

/* ========================================
 * Mouse State Globals
 * ======================================== */

s32 g_mouse_state = 0;
u32 g_mouse_state_flags = 0;
void* g_mouse_hover_target = NULL;
u32 g_hover_sprite = 0;
s32 g_mouse_x = 0;
s32 g_mouse_y = 0;
int g_mouse_pressed = 0;
u32 g_last_click_sprite = 0;

/* ========================================
 * Game State Globals
 * ======================================== */

int g_skip_enter_game = 0;
int g_dialog_skip_flag = 0;
u32 g_player_char_id = 0;
u32 g_current_npc_id = 0;
void* g_current_npc_context = NULL;
int g_protocol_mode = 0;
int g_in_battle = 0;
int g_game_sub_state = 0;
int g_game_state_flag = 0;
int g_state_changed = 0;

/* ========================================
 * Map Position Variables
 * ======================================== */

s32 s_map_pos_x = 0;
s32 s_map_pos_y = 0;
u32 s_last_map_id = 0;
u32 s_map_param = 0;
s32 s_view_min_x = 0;
s32 s_view_min_y = 0;
s32 s_view_max_x = 640;
s32 s_view_max_y = 480;
s32 s_view_width = 640;
s32 s_view_height = 480;
u32 s_entity_count = 0;
u32 s_npc_count = 0;
u32 s_render_queue_count = 0;
int s_fade_active = 0;
s32 s_iso_camera_x = 0;
s32 s_iso_camera_y = 0;

/* ========================================
 * Movement Variables
 * ======================================== */

s32 s_move_target = 0;
int s_move_timer_active = 0;
u32 s_current_field_id = 0;
u32 s_field_sub_id = 0;
u32 s_last_field_id = 0;
u32 s_battle_field1 = 0;
u32 s_battle_field2 = 0;
u32 s_battle_field3 = 0;
u32 s_update_value = 0;
int s_map_entry_flag = 0;
int s_game_state = 0;
int s_game_substate = 0;
int s_battle_flag = 0;
int s_battle_substate = 0;
int s_battle_active_flag = 0;
int s_player_clear_flag = 0;
u32 s_map_id = 0;
u32 s_map_width = 0;
u32 s_map_height = 0;

/* ========================================
 * Battle Unit Globals
 * ======================================== */

void* s_battle_units = NULL;
u32 s_battle_unit_count = 0;
u32 s_selected_unit = 0;
u32 s_target_unit = 0;
u32 s_battle_unit_flags = 0;
void* s_battle_pet = NULL;

/* ========================================
 * Pet Data Globals
 * ======================================== */

void* s_pet_data = NULL;
void* s_pet_skills = NULL;
u32 s_pet_skill_count = 0;
u32 s_selected_pet = 0;

/* ========================================
 * Party Data Globals
 * ======================================== */

void* s_party_entries = NULL;
u32 s_party_count = 0;
u32 s_party_flag = 0;
int s_in_party = 0;

/* ========================================
 * Player Data Globals
 * ======================================== */

char s_player_name[64] = {0};
u32 s_player_field1 = 0;
u32 s_player_field2 = 0;
u32 s_player_field3 = 0;
u32 s_player_field4 = 0;
u32 s_last_move_time = 0;

/* ========================================
 * Network Socket
 * ======================================== */

void* gSocket = NULL;

/* ========================================
 * Pet AI
 * ======================================== */

void* g_petai = NULL;

/* ========================================
 * Data Placeholders (Original Addresses)
 * ======================================== */

u32 DAT_04581d3c = 0;
u32 DAT_04581d40 = 0;
u32 DAT_04558c34 = 0;
u32 DAT_0455b5ac = 0;
u32 DAT_04ebe490 = 0;
u32 DAT_005676a0 = 0;
u32 DAT_00564e62 = 0;
u32 DAT_0461b420 = 0;
u32 DAT_0455ef9c = 0;
u32 DAT_0461b658 = 0;
u32 DAT_046308b8 = 0;
u32 DAT_045829d8 = 0;
u32 DAT_045829dc = 0;
u32 DAT_004b83ec = 0;

/* State check globals for FUN_0044b030 */
s32 DAT_045e19b0 = 0;    /* Battle active flag */
s32 DAT_004e21dc = 0;    /* Dialog active flag */
s32 DAT_045e8ce0 = 0;    /* Menu active flag */
char DAT_045f1a3b = 0;   /* State override 1 */
char DAT_045f1a3a = 0;   /* State override 2 */

/* Search globals for FUN_00421080 */
u32 DAT_045f1b90 = 0;    /* Search target value */
u32 DAT_045f1bc4 = 0;    /* Search enabled flag */

/* VIP level for gold limit */
u32 DAT_0462e3b4 = 0;    /* VIP level */
u32 DAT_0462e3b0 = 0;    /* Player render color */

/* Entity array for pet/summon checking - 20 entries, stride 0x184 */
u32 DAT_0462bf50[20 * 0x62] = {0};    /* Entity array with sprite IDs at start of each entry */

/* Player slot array - used for party/member tracking */
u32 DAT_0463091c[20 * 12] = {0};      /* Player slot entries, stride 0x30 */

/* Player entity globals */
void* DAT_0462e3ac = NULL;  /* Player entity pointer */
u32 DAT_0462bf2c = 0;       /* Player flags (bit 8 = riding) */
char DAT_0462bef8[18] = {0}; /* Player name buffer */
char DAT_0462bf09[34] = {0}; /* Player title buffer */
int DAT_0462be98 = 0;       /* Player HP */
int DAT_0462be9c = 0;       /* Player MP */
int DAT_0462bec0 = 0;       /* Player level */
u32 DAT_0462be90 = 0;       /* Player direction global 1 */
u32 DAT_0462be94 = 0;       /* Player direction global 2 */

/* Pet/special entity globals */
void* DAT_004e2b18 = NULL;  /* Pet/special entity pointer */
char DAT_00544d74 = 0;      /* Pet spawn flag 1 */
char DAT_00544d75 = 0;      /* Pet spawn flag 2 */

/* Render queue count for FUN_0047e210 */
u32 DAT_0464f488 = 0;    /* Render queue count (max 4096) */

/* Network buffer globals for FUN_0045ec80/0045ee40 */
u32 DAT_0461b408 = 0;    /* Receive buffer position */
void* DAT_0461b41c = NULL; /* Receive buffer pointer (16KB) */
u32 DAT_0461b3fc = 0;    /* Buffer initialized flag */

/* Render mode global */
u32 DAT_005ab6fc = 0;

/* Sprite data arrays */
u32 DAT_04633488[0x7001] = {0};  /* X positions */
u32 DAT_0463348c[0x7001] = {0};  /* Y positions */
u32 DAT_04633490[0x7001] = {0};  /* Sprite IDs */
u32 DAT_04633494[0x7001] = {0};  /* Entity pointers */
char DAT_04633498[0x7001] = {0}; /* Render modes */
char DAT_0463349c[0x7001] = {0}; /* Blend modes */

/* Sprite lookup tables - external references */
extern u32 DAT_00a04c64[];
extern u16 DAT_00e8f234[];
extern u16 DAT_00e8f238[];
extern u16 DAT_0081c7f4[];

/* More state globals */
u32 DAT_004bb414 = 0;
u32 DAT_004bb418 = 0;
u32 _DAT_004bb41c = 0;
u32 _DAT_004bb420 = 0;
u32 DAT_045602b0 = 0;
u32 DAT_04633308 = 0;
u32 DAT_04ebe270 = 0;
u32 DAT_04ebe278 = 0;
u32 DAT_04630df0 = 0;
u32 DAT_04630dd8 = 0;
s32 DAT_04630df8 = -1;  /* Pending game state (-1 = no pending transition) */
u32 DAT_04630de8 = 0;   /* Pending sub-state for transition */
u32 DAT_04630de4 = 0;
u32 DAT_04630df4 = 0;
u32 DAT_0049c3e0 = 0;
u32 DAT_0054a90c = 0;
u32 DAT_0465d7c4 = 0;
u32 DAT_0465d7c0 = 0;
u32 DAT_0054c83c = 0;
u32 DAT_04560214 = 0;
u32 DAT_04581190 = 0;
u32 DAT_045e19bc = 0;
u32 DAT_04581180 = 0;
u32 DAT_045829ae = 0;
u32 DAT_045829ac = 0;
u32 DAT_04560258 = 0;
u32 DAT_04581198 = 0;
u32 DAT_046309f8 = 0;
u32 DAT_0462ac0c = 0;
u32 DAT_045967b8 = 0;
u32 DAT_004d803c = 0;
u32 DAT_004e1118 = 0;
u32 DAT_004e2144 = 0;
u32 DAT_004d351c = 0;
u32 DAT_005ab6f8 = 0;
u32 DAT_005ab708 = 0;
u32 DAT_004cf830 = 0;
u32 DAT_00567690 = 0;
u32 DAT_005676b4 = 0;
u32 DAT_004e2644 = 0;
u32 DAT_04582a22 = 0;
u32 DAT_04582a24 = 0;
u32 DAT_04582a28 = 0;
u32 DAT_04560e44 = 0;
u32 DAT_04560e3c = 0;
u32 DAT_04560e40 = 0;
u32 DAT_04560e38 = 0;
u32 DAT_04560e48 = 0;
u32 DAT_045602ac = 0;
u32 DAT_04569b5c = 0;
u32 DAT_0456a62c = 0;
u32 DAT_045602c0 = 0;
u32 DAT_04569b64 = 0;
u32 DAT_045829bc = 0;
u32 DAT_045829c0 = 0;
u32 DAT_045602cc = 0;
u32 DAT_04569b68 = 0;
u32 DAT_04569b70 = 0;    /* Map/field coordinate */
u32 DAT_0458119c = 0;    /* Map width */
u32 DAT_04581d48 = 0;
u32 DAT_0456a64c = 0;
u32 DAT_04581280 = 0;
u32 DAT_04569b74 = 0;
u32 DAT_0456a630 = 0;
u32 DAT_0456a634 = 0;
u32 DAT_0456a628 = 0;
u32 DAT_0455f4ec = 0;
u32 DAT_045829d0 = 0;
u32 DAT_045829d4 = 0;
u32 DAT_0464f64c = 0;
u32 DAT_0464f7b0 = 0;
u32 DAT_0464f48c = 0;
u32 DAT_0464b488 = 0;
u32 DAT_046333f0 = 0;
u32 DAT_046333f4 = 0;    /* Previous field sprite ID */
u32 DAT_04633404 = 0;    /* Field type flag */
u32 DAT_04633408 = 0;
u32 DAT_046333e8 = 0;
u32 DAT_046333ec = 0;
u32 DAT_046333b4 = 0;    /* Field width */
u32 DAT_046333d4 = 0;    /* Field height */
u32 DAT_046333bc = 0;    /* Field scale X */
u32 DAT_04633398 = 0;    /* Field scale Y */
u32 DAT_046333c0 = 0;    /* View offset X calc */
u32 DAT_046333c8 = 0;    /* View offset X */
u32 DAT_046333c4 = 0;    /* View offset Y calc */
u32 DAT_046333cc = 0;    /* View offset Y */
u32 DAT_004bb424 = 0;
u32 DAT_004bb428 = 0;
s32 DAT_004bb758 = -1;    /* Current direction (0-7) for offset calculation */
u32 DAT_04560e54 = 0;
/* DAT_04560214 already defined above */
u32 DAT_004d2628 = 0;
u32 DAT_0054ad18 = 0;
float _DAT_0049c31c = 1.0f;
float _DAT_04582998 = 0.0f;
float _DAT_04582994 = 0.0f;
float _DAT_0049c334 = 1.0f;
float _DAT_0049c3e8 = 0.5f;
float _DAT_0049c3e4 = 0.25f;

/* World position globals for FUN_00440df0 */
float DAT_0456a644 = 0.0f;
float DAT_0456a648 = 0.0f;
u32 DAT_0458118c = 0;
u32 DAT_04581184 = 0;
float _DAT_045827fc = 0.0f;
float _DAT_04582800 = 0.0f;

/* Entity movement speed constants */
float _DAT_0049c32c = 1.0f;    /* Default speed */
float _DAT_0049c308 = 1.5f;    /* Speed for queue < 6 */
float _DAT_0049c330 = 2.0f;    /* Speed for queue < 4 */
float _DAT_0049c30c = 1.2f;    /* Speed for queue 2-3 */
float _DAT_0049c318 = 0.001f;  /* Minimum distance threshold */
float _DAT_0049c328 = 1.0f;    /* Speed multiplier */
float _DAT_0049c324 = 0.0f;    /* Direction offset */

/* Angle normalization constants */
#define PI 3.14159265358979323846f
#define TWO_PI (2.0f * PI)
float _DAT_0049c3f0 = TWO_PI;     /* 2*PI for angle normalization */
float _DAT_0049c3f8 = TWO_PI;     /* Upper bound for angle */
float _DAT_0049c3d8 = 0.0f;       /* Lower bound for angle */
float _DAT_0049c3d0 = 0.0f;       /* Direction offset */

/* Atan2 quadrant constants */
float _DAT_0049c408 = PI / 2.0f;      /* PI/2 = 1.5708... */
float _DAT_0049c400 = 3.0f * PI / 2.0f; /* 3*PI/2 = 4.7124... */
float _DAT_0049c404 = 5.0f * PI / 4.0f; /* 5*PI/4 = 3.927... */

/* Additional globals needed by render */
u32 DAT_045829b8 = 0;
u32 DAT_045829b4 = 0;

/* Sprite lookup tables - pointers to be initialized from data files */
/* These are large tables that should be loaded from game data */
/* For now, use small placeholder arrays */
u32 DAT_00a04c64[1] = {0};  /* Sprite ID to image index lookup */
u16 DAT_00e8f234[1] = {0};  /* Sprite width lookup */
u16 DAT_00e8f238[1] = {0};  /* Sprite height lookup */
u16 DAT_0081c7f4[1] = {0};  /* High-res sprite lookup */

/* Sprite data table - placeholder (actual game uses 500000 * 10 dwords) */
/* TODO: Implement dynamic allocation for full table */
#define SPRITE_TABLE_SIZE 10000
u32 DAT_0081c7e0[SPRITE_TABLE_SIZE * 10] = {0};

/* File handles for sprite data */
HANDLE DAT_005ab7d8 = INVALID_HANDLE_VALUE;
HANDLE DAT_00a04c60 = INVALID_HANDLE_VALUE;

/* Game data globals */
void* g_inventory = NULL;
void* g_map = NULL;

/* ========================================
 * Fade/Transition Effect Variables
 * ======================================== */

/* Scroll transition globals (FUN_0047a730) */
float _DAT_046332dc = 0.0f;   /* Scroll X position */
float _DAT_046332e0 = 0.0f;   /* Scroll Y position (unused in some modes) */
float _DAT_046332e4 = 0.0f;   /* Scroll Y velocity */
float _DAT_046332e8 = 0.0f;   /* Scroll Y position 2 (diagonal) */
float _DAT_046332ec = 0.0f;   /* Scroll velocity/acceleration */

/* Accelerated scroll transition globals (FUN_0047aac0) */
float _DAT_046332f0 = 0.0f;   /* Position X */
float _DAT_046332f4 = 0.0f;   /* Position X2 (diagonal) */
float _DAT_046332f8 = 0.0f;   /* Velocity */
float _DAT_046332fc = 0.0f;   /* Position Y2 (diagonal) */
float _DAT_04630e04 = 0.0f;   /* Acceleration */

/* Box wipe transition globals (FUN_0047aea0) */
s32 DAT_046311b0 = 0;         /* Box rect left */
s32 DAT_046311b4 = 0;         /* Box rect right */
s32 DAT_046311b8 = 0;         /* Box rect top */
s32 DAT_046311bc = 0;         /* Box rect bottom */
u16 DAT_046332cc = 0;         /* Box rect left (duplicate) */
u16 DAT_046332d0 = 0;         /* Box rect top (duplicate) */
u16 DAT_046332d4 = 0;         /* Box rect bottom (duplicate) */
u16 DAT_046332d8 = 0;         /* Box rect right (duplicate) */

/* Fade state flags */
s32 DAT_004cf834 = -1;        /* Accelerated scroll init flag */
s32 DAT_004cf838 = -1;        /* Box wipe init flag */
u32 DAT_005ab710 = 0;         /* Fade state machine: 2=fade out, 3=fade in, 4=complete */

/* Float constants for fade calculations */
/* _DAT_0049c318 defined above with entity movement constants */
float _DAT_0049c440 = 0.0f;   /* Scroll boundary left */
float _DAT_0049c444 = 0.0f;   /* Scroll boundary top */
float _DAT_0049c448 = 1.0f;   /* X acceleration */
float _DAT_0049c44c = 480.0f; /* Scroll boundary bottom */
float _DAT_0049c450 = 0.0f;   /* Scroll boundary top */
float _DAT_0049c454 = 1.0f;   /* Y acceleration */
float _DAT_0049c458 = 0.0f;   /* Initial X position */
float _DAT_0049c45c = 480.0f; /* Screen height */
float _DAT_0049c460 = 640.0f; /* Screen width */
float _DAT_0049c464 = 0.0f;   /* Initial Y position */

/* ========================================
 * Block Dissolve Transition Variables (FUN_0047b180)
 * ======================================== */

u32 DAT_04630e0c[64] = {0};    /* 8x8 tile states: 0=hidden, 1=visible, 2=transitioning */
u32 DAT_046331cc[64] = {0};    /* Progress counter per tile */
s32 DAT_0463108c[64] = {0};    /* Target offset per tile */
s32 DAT_046330c8[64] = {0};    /* Current offset per tile */
s32 DAT_04632fc0[64] = {0};    /* X position per tile */
s32 DAT_04630f0c[64] = {0};    /* Y position per tile */
u32 DAT_0463100c = 0;          /* Completed tile counter */
u32 DAT_04631088 = 0;          /* Active tile counter */
s32 DAT_004cf83c = -1;         /* Block dissolve init flag */

/* ========================================
 * Pixel Dissolve Variables (FUN_0047b7e0)
 * ======================================== */

s32 DAT_046311c0[464] = {0};   /* X positions for pixel dissolve grid */
u32 DAT_04630e00 = 0;          /* Progress counter (0-64 scale) */
u32 DAT_04633300 = 0;          /* Pixel dissolve state flag */

/* ========================================
 * Sprite Blit Variables (FUN_004142f0)
 * ======================================== */

u32 DAT_0466b7d4 = 0;          /* Current sprite width */
u32 DAT_0466b7d0 = 0;          /* Current sprite height */

/* ========================================
 * IME (Input Method Editor) Variables
 * ======================================== */

HIMC DAT_04ec08e4 = NULL;      /* IME context handle */
char* DAT_04ec08cc = NULL;     /* IME composition string buffer */
char* DAT_04ec08d0 = NULL;     /* IME candidate string buffer */
DWORD DAT_04ec0900 = 0;        /* IME status flags */
void* DAT_04ec08c4 = NULL;     /* IME context data */

/* ========================================
 * Entity Flag Array
 * ======================================== */

/* Entity flags array - stride 0x43 (67 dwords) per entity, max 1500 entities */
u32 DAT_004e2b4c[1500 * 67] = {0};

/* Player slot data array - 2 slots, stride 0x44 (68 bytes) for name and stats */
char DAT_04630a00[2 * 0x44] = {0};

/* Entity movement constant */
float _DAT_0456a640 = 1.0f;    /* Movement scale factor */

/* Random shuffle table for FUN_00447310 */
int DAT_04582a40[100] = {0};
int DAT_04582bd0 = 0;  /* Current index in shuffle table */

/* LCG random state for FUN_00492403 */
u32 DAT_004d7160 = 0;
