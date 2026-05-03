/*
 * Stone Age Client - Text Protocol Dispatcher Header
 * Reverse engineered from sa_9061.exe (FUN_0043bf90)
 */

#ifndef TEXT_PROTOCOL_H
#define TEXT_PROTOCOL_H

#include "types.h"

/* ========================================
 * Text Protocol Dispatcher
 * ======================================== */

/*
 * Main text protocol dispatcher - FUN_0043bf90
 * Parses and dispatches pipe-delimited text commands
 *
 * Parameters:
 *   line: Text line to parse (modified in place)
 *
 * Protocol format: COMMAND|param1|param2|...
 * Field separator: '|' (0x7c)
 * Integer encoding: Base-62 (see protocol_decode_int)
 */
void text_protocol_dispatch(char* line);

/* ========================================
 * Command Names (from FUN_0043bf90 analysis)
 * ======================================== */

/* Short commands (2-3 characters) */
#define CMD_XY      "XY"     /* Item use - FUN_00463e70 */
#define CMD_XYC     "XYC"    /* Item drop - FUN_00464e10 */
#define CMD_XYD     "XYD"    /* Char delete result - FUN_004643f0 */
#define CMD_XYE     "XYE"    /* Char logout result - FUN_00464610 */
#define CMD_XYF     "XYF"    /* Login complete - FUN_00464ef0 */
#define CMD_XYG     "XYG"    /* Map enter - FUN_00464670 */
#define CMD_XYH     "XYH"    /* Map data - FUN_00464650 */
#define CMD_XYJ     "XYJ"    /* Move result - FUN_00465170 */
#define CMD_XYK     "XYK"    /* Character positions - FUN_00464af0 */
#define CMD_XYL     "XYL"    /* Map change - FUN_00463f00 */
#define CMD_XYM     "XYM"    /* Teleport - FUN_00464190 */
#define CMD_XYN     "XYN"    /* Object spawn - FUN_00463790 */
#define CMD_XYO     "XYO"    /* Enter game - FUN_00462010 */
#define CMD_XYP     "XYP"    /* Action result - FUN_00462200 */
#define CMD_XYQ     "XYQ"    /* Talk result - FUN_00462590 */
#define CMD_XYR     "XYR"    /* NPC dialog - FUN_00462f60 */
#define CMD_XYS     "XYS"    /* NPC shop - FUN_00463380 */
#define CMD_XYT     "XYT"    /* Dialog close - FUN_00465460 */
#define CMD_XYU     "XYU"    /* Detailed stats - FUN_0045ffb0 */
#define CMD_XYV     "XYV"    /* Item pickup result - FUN_00465460 */
#define CMD_XYW     "XYW"    /* Inventory update - FUN_00463ee0 */
#define CMD_XYX     "XYX"    /* Item use result - FUN_00464ee0 */
#define CMD_XYY     "XYY"    /* Equip result - FUN_00463d80 */
#define CMD_XYZ     "XYZ"    /* Unequip result - FUN_00465060 */
#define CMD_XYA     "XYA"    /* Battle start - FUN_00465390 */
#define CMD_XYB     "XYB"    /* Battle turn - FUN_00465160 */
#define CMD_XYC2    "XYC2"   /* Battle action - FUN_00464ac0 */
#define CMD_XYD2    "XYD2"   /* Battle result - FUN_00464db0 */
#define CMD_XYE2    "XYE2"   /* Battle end - FUN_004653d0 */

/* Long commands */
#define CMD_CLIENTLOGIN    "ClientLogin"    /* FUN_0045fa40 */
#define CMD_CREATENEWCHAR  "CreateNewChar"  /* FUN_00463c20 */
#define CMD_CHARDELETE     "CharDelete"     /* FUN_00463d20 */
#define CMD_CHARLOGIN      "CharLogin"      /* FUN_0045fdc0 */
#define CMD_CHARLIST       "CharList"       /* FUN_0045fb80 */
#define CMD_CHARLOGOUT     "CharLogout"     /* FUN_0045ff50 */
#define CMD_PROCGET        "ProcGet"        /* FUN_00465460 */
#define CMD_PLAYERNUMGET   "PlayerNumGet"   /* FUN_00465460 */

/* Additional system commands */
#define CMD_XYE3    "XYE3"   /* System message - FUN_00465460 */
#define CMD_XYF2    "XYF2"   /* Party leave - FUN_00465440 */
#define CMD_XYG2    "XYG2"   /* Trade invite - FUN_0045a9a0 */
#define CMD_XYH2    "XYH2"   /* Trade request - FUN_00465470 */

/* ========================================
 * Command Category Macros
 * ======================================== */

/* Authentication commands */
#define IS_AUTH_CMD(cmd) \
    (strcmp(cmd, CMD_CLIENTLOGIN) == 0 || \
     strcmp(cmd, CMD_CREATENEWCHAR) == 0 || \
     strcmp(cmd, CMD_CHARDELETE) == 0 || \
     strcmp(cmd, CMD_CHARLOGIN) == 0 || \
     strcmp(cmd, CMD_CHARLIST) == 0 || \
     strcmp(cmd, CMD_CHARLOGOUT) == 0)

/* Map/movement commands */
#define IS_MAP_CMD(cmd) \
    (strcmp(cmd, CMD_XYG) == 0 || \
     strcmp(cmd, CMD_XYH) == 0 || \
     strcmp(cmd, CMD_XYJ) == 0 || \
     strcmp(cmd, CMD_XYL) == 0 || \
     strcmp(cmd, CMD_XYM) == 0 || \
     strcmp(cmd, CMD_XYN) == 0 || \
     strcmp(cmd, CMD_XYO) == 0 || \
     strcmp(cmd, CMD_XYK) == 0)

/* Item commands */
#define IS_ITEM_CMD(cmd) \
    (strcmp(cmd, CMD_XY) == 0 || \
     strcmp(cmd, CMD_XYC) == 0 || \
     strcmp(cmd, CMD_XYW) == 0 || \
     strcmp(cmd, CMD_XYX) == 0 || \
     strcmp(cmd, CMD_XYY) == 0 || \
     strcmp(cmd, CMD_XYZ) == 0 || \
     strcmp(cmd, CMD_XYV) == 0 || \
     strcmp(cmd, CMD_XYU) == 0)

/* Battle commands */
#define IS_BATTLE_CMD(cmd) \
    (strcmp(cmd, CMD_XYA) == 0 || \
     strcmp(cmd, CMD_XYB) == 0 || \
     strcmp(cmd, CMD_XYC2) == 0 || \
     strcmp(cmd, CMD_XYD2) == 0 || \
     strcmp(cmd, CMD_XYE2) == 0)

/* NPC commands */
#define IS_NPC_CMD(cmd) \
    (strcmp(cmd, CMD_XYR) == 0 || \
     strcmp(cmd, CMD_XYS) == 0 || \
     strcmp(cmd, CMD_XYT) == 0 || \
     strcmp(cmd, CMD_XYQ) == 0)

/* ========================================
 * Message Log Accessors (FUN_00464ef0)
 * ======================================== */

/*
 * Get chat log entry
 * Index 0 is most recent, 1-3 are older entries
 * Returns NULL if index out of range
 */
const char* text_protocol_get_chat_log(u32 index);

/*
 * Get message log entry
 * Index 0 is most recent, 1-3 are older entries
 * Returns NULL if index out of range
 */
const char* text_protocol_get_message_log(u32 index);

/*
 * Check if update flag is set (from XYF U command)
 */
int text_protocol_has_update(void);

/*
 * Clear update flag
 */
void text_protocol_clear_update(void);

/*
 * Get acknowledgment value (from XYF A command)
 */
u32 text_protocol_get_ack_value(void);

#endif /* TEXT_PROTOCOL_H */
