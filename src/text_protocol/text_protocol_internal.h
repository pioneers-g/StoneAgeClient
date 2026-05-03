/*
 * Stone Age Client - Text Protocol Internal Header
 * Internal definitions for text protocol module
 */

#ifndef TEXT_PROTOCOL_INTERNAL_H
#define TEXT_PROTOCOL_INTERNAL_H

#include "types.h"

/* Maximum fields per protocol line */
#define MAX_TEXT_FIELDS    32

/* Message log constants - from FUN_00464ef0 analysis */
#define MSG_LOG_SIZE       0x1000   /* 4096 bytes per entry */
#define MSG_LOG_COUNT      4        /* 4 entry circular buffer */

/* Protocol parse context - DAT_004ba014 */
typedef struct {
    const char* fields[MAX_TEXT_FIELDS];
    u32 field_count;
    char* line_copy;       /* Mutated line with null terminators */
} TextProtocolContext;

/* Global parse context - shared between modules */
extern TextProtocolContext s_proto_ctx;

/* Message log buffers - DAT_004dd074, DAT_004d906c */
extern char s_chat_log[MSG_LOG_COUNT][MSG_LOG_SIZE];
extern char s_message_log[MSG_LOG_COUNT][MSG_LOG_SIZE];
extern u32 s_chat_log_index;      /* DAT_004e2138 */
extern u32 s_message_log_index;   /* DAT_004e2118 */
extern u32 s_update_flag;         /* DAT_004e21d0 */
extern u32 s_ack_flag;            /* DAT_004e2144 */
extern u32 s_ack_value;           /* DAT_004e1110 */

/* Core functions - text_protocol_core.c */
void parse_protocol_line(char* line);
int get_int_param(int field_index);
const char* get_string_param(int field_index);
const char* get_unescaped_param(int field_index);
int dbcs_strcmp(const char* s1, const char* s2);

/* Handler functions - text_protocol_handlers.c */
void handle_L(void);
void handle_XY(void);
void handle_XYC(void);
void handle_XYD(void);
void handle_XYE(void);
void handle_XYF(void);
void handle_XYG(void);
void handle_XYH(void);
void handle_XYJ(void);
void handle_XYK(void);
void handle_XYL(void);
void handle_XYM(void);
void handle_XYN(void);
void handle_XYO(void);
void handle_XYP(void);
void handle_XYQ(void);
void handle_XYR(void);
void handle_XYS(void);
void handle_XYT(void);
void handle_XYU(void);
void handle_XYV(void);
void handle_XYW(void);
void handle_XYX(void);
void handle_XYY(void);
void handle_XYZ(void);
void handle_XYA(void);
void handle_XYB(void);
void handle_XYC2(void);
void handle_XYD2(void);
void handle_XYE2(void);
void handle_ClientLogin(void);
void handle_CreateNewChar(void);
void handle_CharDelete(void);
void handle_CharLogin(void);
void handle_CharList(void);
void handle_CharLogout(void);
void handle_ProcGet(void);
void handle_PlayerNumGet(void);
void handle_XYE3(void);
void handle_XYF2(void);
void handle_XYG2(void);
void handle_XYH2(void);

/* Message log functions - text_protocol_message.c */
void message_log_init(void);
void message_log_chat(const char* message);
void message_log_system(const char* message);

#endif /* TEXT_PROTOCOL_INTERNAL_H */
