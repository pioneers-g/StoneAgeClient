/*
 * Stone Age Client - Text Protocol Dispatcher Header
 * Reverse engineered from sa_9061.exe (FUN_0043bf90)
 */

#ifndef PROTOCOL_TEXT_H
#define PROTOCOL_TEXT_H

#include "types.h"
#include <winsock2.h>

/* ========================================
 * Command String Constants - from DAT_004b9xxx
 * ======================================== */

/* Two-character commands */
#define CMD_XYD  "XYD"   /* DAT_004b9fe4 - FUN_00465400 */
#define CMD_EV   "EV"    /* DAT_004b9eb8 - Encounter/battle state */
#define CMD_EN   "EN"    /* DAT_004b9ebc - Entity/notification */
#define CMD_DU   "DU"    /* DAT_004b9fe0 - Dialog/update */
#define CMD_EO   "EO"    /* DAT_004b9fdc - Entity/option */
#define CMD_EB   "EB"    /* DAT_004b9ecc - Chat/message buffer */
#define CMD_EC   "EC"    /* DAT_004b9fd8 - Entity/config */
#define CMD_ED   "ED"    /* DAT_004b9fd4 - Entity/data */
#define CMD_EE   "EE"    /* DAT_004b9ee4 - Entity/effect */
#define CMD_EF   "EF"    /* DAT_004b9fd0 - Enter field */
#define CMD_EG   "EG"    /* DAT_004b9ef0 - Entity/group */
#define CMD_EH   "EH"    /* DAT_004b9fcc - Entity handler */
#define CMD_EI   "EI"    /* DAT_004b9f00 - Entity/item */
#define CMD_EJ   "EJ"    /* DAT_004b9fc8 - Extended job */
#define CMD_EK   "EK"    /* DAT_004b9f04 - Entity/kill */
#define CMD_EL   "EL"    /* DAT_004a2624 - Entity/location */
#define CMD_EM   "EM"    /* DAT_004b9fc4 - Entity/menu */
#define CMD_EN2  "EN2"   /* DAT_004b9fc0 - FUN_00463380 */
#define CMD_EO2  "EO2"   /* DAT_004b9fbc - FUN_00465460 */
#define CMD_EP   "EP"    /* DAT_004a262c - FUN_0045ffb0 */
#define CMD_EQ   "EQ"    /* DAT_004a2628 - FUN_00465460 */
#define CMD_ER   "ER"    /* DAT_004b9f08 - Entity/result */
#define CMD_ES   "ES"    /* DAT_004b9f0c - Entity/status */
#define CMD_ET   "ET"    /* DAT_004b9f10 - Entity/target */
#define CMD_EU   "EU"    /* DAT_004b9f14 - Entity/update */
#define CMD_EV2  "EV2"   /* DAT_004b9f20 - FUN_00465390 */
#define CMD_EW   "EW"    /* DAT_004b9f28 - Entity/weapon (battle turn) */
#define CMD_EX   "EX"    /* DAT_004b9f34 - Entity/exit (battle action) */
#define CMD_EY   "EY"    /* DAT_004b9fb8 - Entity/yield (battle result) */
#define CMD_EZ   "EZ"    /* DAT_004b9fb4 - Entity/zone (battle end) */
#define CMD_TD   "TD"    /* DAT_004b9fa8 - Trade/dialog */
#define CMD_FM   "FM"    /* DAT_004b9fac - Friend/message */
#define CMD_NU   "NU"    /* DAT_004b9fb0 - Number/user */
#define CMD_SE   "SE"    /* DAT_004b9fb4 - Skill/effect */
#define CMD_EF2  "EF2"   /* DAT_004b9fb8 - FUN_00464db0 */
#define CMD_R    "R"     /* DAT_004b9fbc - Result */

/* Long commands */
#define CMD_ClientLogin   "ClientLogin"   /* DAT_004b9f3c */
#define CMD_CreateNewChar "CreateNewChar" /* DAT_004b9f48 */
#define CMD_CharDelete    "CharDelete"    /* DAT_004b9f58 */
#define CMD_CharLogin     "CharLogin"     /* DAT_004b9f64 */
#define CMD_CharList      "CharList"      /* DAT_004b9f70 */
#define CMD_CharLogout    "CharLogout"    /* DAT_004b9f7c */
#define CMD_ProcGet       "ProcGet"       /* DAT_004b9f88 */
#define CMD_PlayerNumGet  "PlayerNumGet"  /* DAT_004b9f90 */
#define CMD_Echo          "Echo"          /* DAT_004b9fa0 */

/* ========================================
 * Function Declarations
 * ======================================== */

/**
 * Initialize text protocol module
 */
void protocol_text_init(void);

/**
 * Shutdown text protocol module
 */
void protocol_text_shutdown(void);

/**
 * Text protocol dispatcher - FUN_0043bf90
 * Main entry point for all text-based server commands
 *
 * @param sock Socket (unused in original)
 * @param line Text line from server (pipe-delimited)
 */
void packet_dispatch_text(SOCKET sock, const char* line);

#endif /* PROTOCOL_TEXT_H */
