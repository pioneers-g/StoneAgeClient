/*
 * Stone Age Client - Text Protocol Core
 * Main dispatcher, line parser, DBCS string comparison
 * Split from text_protocol.c - FUN_0043bf90, FUN_0043dbe0
 */

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "text_protocol.h"
#include "text_protocol_internal.h"
#include "logger.h"

/* Global parse context - DAT_004ba014 */
TextProtocolContext s_proto_ctx;

/* Message log buffers */
char s_chat_log[MSG_LOG_COUNT][MSG_LOG_SIZE];
char s_message_log[MSG_LOG_COUNT][MSG_LOG_SIZE];
u32 s_chat_log_index = 0;
u32 s_message_log_index = 0;
u32 s_update_flag = 0;
u32 s_ack_flag = 0;
u32 s_ack_value = 0;

/* ========================================
 * DBCS-Aware String Comparison - FUN_0043c010 pattern
 * ======================================== */

/*
 * Compare strings with DBCS awareness
 * Returns: 0 if equal, <0 if s1 < s2, >0 if s1 > s2
 */
int dbcs_strcmp(const char* s1, const char* s2) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;

    while (*p1 && *p2) {
        /* Check for DBCS lead byte */
        if (IsDBCSLeadByte(*p1)) {
            /* Compare DBCS characters */
            if (!IsDBCSLeadByte(*p2)) {
                return *p1 - *p2;
            }
            /* Compare both bytes */
            if (*p1 != *p2) {
                return *p1 - *p2;
            }
            p1++; p2++;
            if (*p1 != *p2) {
                return *p1 - *p2;
            }
            p1++; p2++;
        }
        else {
            /* ASCII comparison */
            if (*p1 != *p2) {
                return *p1 - *p2;
            }
            p1++; p2++;
        }
    }

    return *p1 - *p2;
}

/* ========================================
 * Protocol Line Parser - FUN_0043dbe0
 * ======================================== */

/*
 * Parse pipe-delimited line into fields
 * Modifies line in-place by replacing pipes with null terminators
 */
void parse_protocol_line(char* line) {
    char* ptr = line;
    int field_idx = 0;

    memset(&s_proto_ctx, 0, sizeof(s_proto_ctx));
    s_proto_ctx.line_copy = line;

    /* Skip leading whitespace */
    while (*ptr == ' ' || *ptr == '\t') {
        ptr++;
    }

    /* First field starts at beginning */
    if (*ptr && *ptr != '|' && *ptr != '\n' && *ptr != '\r') {
        s_proto_ctx.fields[field_idx++] = ptr;
    }

    /* Find field boundaries */
    while (*ptr && field_idx < MAX_TEXT_FIELDS - 1) {
        unsigned char c = (unsigned char)*ptr;

        if (c < 0x80) {
            /* ASCII character */
            if (c == '|') {
                *ptr = '\0';  /* Replace separator with null */
                ptr++;
                /* Next field starts after separator */
                if (*ptr && *ptr != '|' && *ptr != '\n' && *ptr != '\r') {
                    s_proto_ctx.fields[field_idx++] = ptr;
                }
            }
            else if (c == '\n' || c == '\r') {
                *ptr = '\0';
                break;
            }
            else {
                ptr++;
            }
        }
        else {
            /* DBCS character - skip 2 bytes */
            ptr++;
            if (*ptr) {
                ptr++;
            }
        }
    }

    /* Null-terminate last field */
    while (*ptr && *ptr != '\n' && *ptr != '\r') {
        ptr++;
    }
    *ptr = '\0';

    s_proto_ctx.field_count = field_idx;
}

/*
 * Get integer parameter - FUN_0043dd50 pattern
 * Uses protocol_decode_int for base-62 decoding
 */
int get_int_param(int field_index) {
    if (field_index >= (int)s_proto_ctx.field_count) {
        return 0;
    }
    return protocol_decode_int(s_proto_ctx.fields[field_index]);
}

/*
 * Get string parameter - FUN_0043dd70 pattern
 * Returns the raw string field
 */
const char* get_string_param(int field_index) {
    if (field_index >= (int)s_proto_ctx.field_count) {
        return "";
    }
    return s_proto_ctx.fields[field_index];
}

/*
 * Get unescaped string parameter - FUN_0043e1b0 pattern
 */
const char* get_unescaped_param(int field_index) {
    if (field_index >= (int)s_proto_ctx.field_count) {
        return "";
    }
    return protocol_unescape_string(s_proto_ctx.fields[field_index]);
}

/* ========================================
 * Command Table
 * ======================================== */

typedef struct {
    const char* command;
    void (*handler)(void);
} TextCommandEntry;

/* Command table from FUN_0043bf90 analysis */
static const TextCommandEntry s_text_commands[] = {
    /* Short commands (2-3 chars) */
    { "L",     handle_L },       /* DAT_004b9fe4 */
    { "XY",    handle_XY },      /* DAT_004b9eb8 */
    { "XYC",   handle_XYC },     /* DAT_004b9ebc */
    { "XYD",   handle_XYD },     /* DAT_004b9fe4 */
    { "XYE",   handle_XYE },     /* DAT_004b9fe0 */
    { "XYF",   handle_XYF },     /* DAT_004b9fdc */
    { "XYG",   handle_XYG },     /* DAT_004b9ecc */
    { "XYH",   handle_XYH },     /* DAT_004b9fd8 */
    { "XYJ",   handle_XYJ },     /* DAT_004b9fd4 */
    { "XYK",   handle_XYK },     /* DAT_004b9ee4 */
    { "XYL",   handle_XYL },     /* DAT_004b9ef0 */
    { "XYM",   handle_XYM },     /* DAT_004b9fcc */
    { "XYN",   handle_XYN },     /* DAT_004b9f00 */
    { "XYO",   handle_XYO },     /* DAT_004b9fc8 */
    { "XYP",   handle_XYP },     /* DAT_004b9f04 */
    { "XYQ",   handle_XYQ },     /* DAT_004a2624 */
    { "XYR",   handle_XYR },     /* DAT_004b9fc4 */
    { "XYS",   handle_XYS },     /* DAT_004b9fc0 */
    { "XYT",   handle_XYT },     /* DAT_004b9fbc */
    { "XYU",   handle_XYU },     /* DAT_004a262c */
    { "XYV",   handle_XYV },     /* DAT_004a2628 */
    { "XYW",   handle_XYW },     /* DAT_004b9f08 */
    { "XYX",   handle_XYX },     /* DAT_004b9f0c */
    { "XYY",   handle_XYY },     /* DAT_004b9f10 */
    { "XYZ",   handle_XYZ },     /* DAT_004b9f14 */
    { "XYA",   handle_XYA },     /* DAT_004b9f20 */
    { "XYB",   handle_XYB },     /* DAT_004b9f28 */
    { "XYC2",  handle_XYC2 },    /* DAT_004b9f34 */
    { "XYD2",  handle_XYD2 },    /* DAT_004b9fb8 */
    { "XYE2",  handle_XYE2 },    /* DAT_004b9fb4 */

    /* Long commands */
    { "ClientLogin",   handle_ClientLogin },    /* DAT_004b9f3c */
    { "CreateNewChar", handle_CreateNewChar },  /* DAT_004b9f48 */
    { "CharDelete",    handle_CharDelete },     /* DAT_004b9f58 */
    { "CharLogin",     handle_CharLogin },      /* DAT_004b9f64 */
    { "CharList",      handle_CharList },       /* DAT_004b9f70 */
    { "CharLogout",    handle_CharLogout },     /* DAT_004b9f7c */
    { "ProcGet",       handle_ProcGet },        /* DAT_004b9f88 */
    { "PlayerNumGet",  handle_PlayerNumGet },   /* DAT_004b9f90 */

    /* Additional commands */
    { "XYE3",  handle_XYE3 },    /* DAT_004b9fa0 */
    { "XYF2",  handle_XYF2 },    /* DAT_004b9fb0 */
    { "XYG2",  handle_XYG2 },    /* DAT_004b9fa8 */
    { "XYH2",  handle_XYH2 },    /* DAT_004b9fac */

    { NULL, NULL }  /* Terminator */
};

/* ========================================
 * Main Text Protocol Dispatcher - FUN_0043bf90
 * ======================================== */

/*
 * Dispatch text protocol command
 * This is the main entry point for text protocol handling
 */
void text_protocol_dispatch(char* line) {
    const TextCommandEntry* entry;

    if (!line || !line[0]) {
        return;
    }

    /* Parse line into fields */
    parse_protocol_line(line);

    if (s_proto_ctx.field_count == 0) {
        return;
    }

    /* Find matching command */
    for (entry = s_text_commands; entry->command; entry++) {
        if (dbcs_strcmp(s_proto_ctx.fields[0], entry->command) == 0) {
            LOG_DEBUG("Text protocol: %s (%d fields)", entry->command, s_proto_ctx.field_count);
            entry->handler();
            return;
        }
    }

    /* Unknown command */
    LOG_DEBUG("Unknown text protocol command: %s", s_proto_ctx.fields[0]);
}

/* ========================================
 * Message Log Accessor Functions
 * ======================================== */

/*
 * Get chat log entry
 * Index 0 is most recent, 1-3 are older entries
 */
const char* text_protocol_get_chat_log(u32 index) {
    if (index >= MSG_LOG_COUNT) return NULL;

    /* Calculate actual index (most recent is at current_index - 1) */
    u32 actual_idx = (s_chat_log_index + MSG_LOG_COUNT - 1 - index) & 3;
    return s_chat_log[actual_idx];
}

/*
 * Get message log entry
 * Index 0 is most recent, 1-3 are older entries
 */
const char* text_protocol_get_message_log(u32 index) {
    if (index >= MSG_LOG_COUNT) return NULL;

    u32 actual_idx = (s_message_log_index + MSG_LOG_COUNT - 1 - index) & 3;
    return s_message_log[actual_idx];
}

/*
 * Check if update flag is set (from XYF U command)
 */
int text_protocol_has_update(void) {
    return s_update_flag;
}

/*
 * Clear update flag
 */
void text_protocol_clear_update(void) {
    s_update_flag = 0;
}

/*
 * Get acknowledgment value (from XYF A command)
 */
u32 text_protocol_get_ack_value(void) {
    return s_ack_value;
}
