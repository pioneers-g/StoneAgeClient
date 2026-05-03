#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include "types.h"

/* Command line context structure - matches FUN_0043f4b0 parsing */
typedef struct {
    char user_id[256];          /* Parsed username from userid param */
    char password[256];         /* Parsed password from userid param */
    char session_token[256];    /* Optional session token */
    char server_hint[256];      /* Optional server hint */
    char real_bin_path[MAX_PATH];
    char adrn_bin_path[MAX_PATH];
    char spr_bin_path[MAX_PATH];
    char spradrn_bin_path[MAX_PATH];
    char tokyohot_path[MAX_PATH];
    char realtrue_bin_path[MAX_PATH];
    char adrntrue_bin_path[MAX_PATH];
    int use_alpha;              /* Alpha blending flag - DAT_0054c83c */
    int window_mode;            /* Window mode flag - DAT_04560210 */
    int no_delay;               /* No delay flag - DAT_004ba438 */
    int initialized;            /* Parsing done flag - DAT_04630dd8 */
} CommandLineContext;

/* Global command line context */
extern CommandLineContext g_cmdline;

/* Initialize and parse command line - FUN_0043f4b0 */
void cmdline_init(const char* cmdline);

/* Find a parameter in command line, returns pointer to value or NULL */
const char* cmdline_find_param(const char* cmdline, const char* param);

/* Parse functions */
int cmdline_parse_bool(const char* cmdline, const char* param);
void cmdline_parse_string(const char* cmdline, const char* param, char* output, size_t max_len);

/* Token iteration - FUN_00493170 pattern */
const char* cmdline_get_first_token(const char* str, char delimiter, char* buffer, size_t buf_size);
const char* cmdline_get_next_token(char delimiter, char* buffer, size_t buf_size);

#endif /* COMMANDLINE_H */
