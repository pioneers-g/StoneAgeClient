/*
 * Stone Age Client - Command Line Parsing
 * Reverse engineered from sa_9061.exe FUN_0043f4b0
 *
 * Parses command line parameters:
 * - userid: User identification string
 * - realbin: Path to real.bin
 * - adrnbin: Path to adrn.bin
 * - sprbin: Path to spr.bin
 * - spradrnbin: Path to spradrn.bin
 * - tokyohot: Path to tokyohot.dat
 * - realtrue: Path to realtrue.bin
 * - adrntrue: Path to adrntrue.bin
 * - usealpha: Enable alpha blending
 * - windowmode: Enable windowed mode
 * - nodelay: Disable frame delay
 */

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "types.h"
#include "commandline.h"
#include "logger.h"

/* Global command line context */
CommandLineContext g_cmdline = {0};

/* Static state for token iteration - matches FUN_00493170 pattern */
static const char* s_token_str = NULL;
static char s_token_delimiter = '\0';

/*
 * Copy string with length optimization - FUN_0043f1f0 pattern
 * Uses 32-bit word copying for efficiency
 */
static void copy_string_optimized(char* dest, const char* src) {
    size_t len;
    size_t i;
    const char* p;
    u32* dst32;
    const u32* src32;

    /* Get string length */
    len = 0;
    p = src;
    while (*p++) len++;

    if (len == 0) {
        dest[0] = '\0';
        return;
    }

    /* Copy in 32-bit chunks */
    dst32 = (u32*)dest;
    src32 = (const u32*)src;

    for (i = len >> 2; i != 0; i--) {
        *dst32++ = *src32++;
    }

    /* Copy remaining bytes */
    for (i = len & 3; i != 0; i--) {
        *((u8*)dst32)++ = *((const u8*)src32)++;
    }
}

/*
 * Find a parameter in command line - FUN_00492020 pattern
 * Returns pointer to value after '=' or NULL if not found
 */
const char* cmdline_find_param(const char* cmdline, const char* param) {
    char search_pattern[256];
    const char* pos;
    size_t param_len;

    if (!cmdline || !param) {
        return NULL;
    }

    /* Build search pattern: " param=" */
    _snprintf(search_pattern, sizeof(search_pattern), " %s=", param);
    param_len = strlen(search_pattern);

    /* Check at beginning of string */
    if (strncmp(cmdline, param, strlen(param)) == 0 && cmdline[strlen(param)] == '=') {
        return cmdline + strlen(param) + 1;
    }

    /* Search in rest of command line */
    pos = strstr(cmdline, search_pattern);
    if (pos) {
        return pos + param_len;
    }

    return NULL;
}

/*
 * Parse a boolean parameter (presence = true)
 */
int cmdline_parse_bool(const char* cmdline, const char* param) {
    char search_pattern[256];
    const char* pos;

    if (!cmdline || !param) {
        return 0;
    }

    /* Check at beginning of string */
    if (strncmp(cmdline, param, strlen(param)) == 0) {
        char next = cmdline[strlen(param)];
        if (next == ' ' || next == '\0' || next == '/') {
            return 1;
        }
    }

    /* Build search pattern with leading space */
    _snprintf(search_pattern, sizeof(search_pattern), " %s", param);

    /* Search for parameter */
    pos = strstr(cmdline, search_pattern);
    if (pos) {
        char next = pos[strlen(search_pattern)];
        if (next == ' ' || next == '\0' || next == '/') {
            return 1;
        }
    }

    return 0;
}

/*
 * Parse a string parameter value
 */
void cmdline_parse_string(const char* cmdline, const char* param, char* output, size_t max_len) {
    const char* value_start;
    const char* value_end;
    size_t len;

    if (!cmdline || !param || !output || max_len == 0) {
        return;
    }

    output[0] = '\0';

    value_start = cmdline_find_param(cmdline, param);
    if (!value_start) {
        return;
    }

    /* Find end of value (space or end of string) */
    value_end = strchr(value_start, ' ');
    if (!value_end) {
        value_end = value_start + strlen(value_start);
    }

    /* Copy value */
    len = (size_t)(value_end - value_start);
    if (len >= max_len) {
        len = max_len - 1;
    }
    strncpy(output, value_start, len);
    output[len] = '\0';
}

/*
 * Get first token from string - FUN_00493170 pattern
 * Used for parsing delimited values like "value1|value2|value3"
 */
const char* cmdline_get_first_token(const char* str, char delimiter, char* buffer, size_t buf_size) {
    const char* pos;
    size_t len;

    if (!str || !buffer || buf_size == 0) {
        return NULL;
    }

    s_token_str = str;
    s_token_delimiter = delimiter;

    /* Find delimiter or end */
    pos = strchr(str, delimiter);
    if (pos) {
        len = (size_t)(pos - str);
    } else {
        len = strlen(str);
    }

    if (len >= buf_size) {
        len = buf_size - 1;
    }

    strncpy(buffer, str, len);
    buffer[len] = '\0';

    /* Return next position or NULL */
    if (pos) {
        return pos + 1;
    }
    return NULL;
}

/*
 * Get next token - continues from previous position
 */
const char* cmdline_get_next_token(char delimiter, char* buffer, size_t buf_size) {
    const char* pos;
    size_t len;

    if (!s_token_str || !buffer || buf_size == 0) {
        return NULL;
    }

    /* Find delimiter or end */
    pos = strchr(s_token_str, delimiter);
    if (pos) {
        len = (size_t)(pos - s_token_str);
    } else {
        len = strlen(s_token_str);
    }

    if (len >= buf_size) {
        len = buf_size - 1;
    }

    strncpy(buffer, s_token_str, len);
    buffer[len] = '\0';

    /* Update position */
    if (pos) {
        s_token_str = pos + 1;
        return s_token_str;
    }

    s_token_str = NULL;
    return NULL;
}

/*
 * Initialize command line context - FUN_0043f4b0 pattern
 * Parses all known parameters from command line
 *
 * Key format from binary:
 * - userid=username|password|... (pipe-delimited)
 * - realbin:, adrnbin:, sprbin:, spradrnbin: etc. for data paths
 * - usealpha, windowmode, nodelay as boolean flags
 */
void cmdline_init(const char* cmdline) {
    const char* param_val;
    char token_buffer[256];
    const char* token_pos;
    int token_index;

    if (g_cmdline.initialized) {
        return;
    }

    memset(&g_cmdline, 0, sizeof(CommandLineContext));

    if (!cmdline) {
        LOG_WARN("Empty command line");
        g_cmdline.initialized = 1;
        return;
    }

    LOG_DEBUG("Parsing command line: %s", cmdline);

    /* Parse userid - FUN_0043f4b0 pattern
     * Format: userid=username|password|...
     * Uses FUN_00493170 (strtok-like) with '|' delimiter
     */
    param_val = cmdline_find_param(cmdline, "userid");
    if (param_val) {
        /* Copy the userid value for tokenization */
        char userid_value[512];
        cmdline_parse_string(cmdline, "userid", userid_value, sizeof(userid_value));
        LOG_DEBUG("User ID value: %s", userid_value);

        /* Tokenize with '|' delimiter - matches FUN_00493170 pattern */
        token_pos = userid_value;
        token_index = 0;

        while (*token_pos && token_index < 4) {
            /* Extract token until '|' or end */
            int i = 0;
            while (*token_pos && *token_pos != '|' && i < (int)sizeof(token_buffer) - 1) {
                token_buffer[i++] = *token_pos++;
            }
            token_buffer[i] = '\0';

            /* Skip delimiter if present */
            if (*token_pos == '|') {
                token_pos++;
            }

            /* Store token based on index - matches DAT_0454f278/DAT_0455aa58 pattern */
            switch (token_index) {
                case 0:  /* Username */
                    copy_string_optimized(g_cmdline.user_id, token_buffer);
                    LOG_DEBUG("Username: %s", g_cmdline.user_id);
                    break;
                case 1:  /* Password */
                    copy_string_optimized(g_cmdline.password, token_buffer);
                    LOG_DEBUG("Password: [hidden]");
                    break;
                case 2:  /* Additional field (possibly server or session) */
                    copy_string_optimized(g_cmdline.session_token, token_buffer);
                    break;
                case 3:  /* Additional field */
                    copy_string_optimized(g_cmdline.server_hint, token_buffer);
                    break;
            }
            token_index++;
        }
    }

    /* Parse realbin path */
    param_val = cmdline_find_param(cmdline, "realbin");
    if (param_val) {
        cmdline_parse_string(cmdline, "realbin", g_cmdline.real_bin_path, sizeof(g_cmdline.real_bin_path));
        LOG_DEBUG("real.bin path: %s", g_cmdline.real_bin_path);
    }

    /* Parse adrnbin path */
    param_val = cmdline_find_param(cmdline, "adrnbin");
    if (param_val) {
        cmdline_parse_string(cmdline, "adrnbin", g_cmdline.adrn_bin_path, sizeof(g_cmdline.adrn_bin_path));
        LOG_DEBUG("adrn.bin path: %s", g_cmdline.adrn_bin_path);
    }

    /* Parse sprbin path */
    param_val = cmdline_find_param(cmdline, "sprbin");
    if (param_val) {
        cmdline_parse_string(cmdline, "sprbin", g_cmdline.spr_bin_path, sizeof(g_cmdline.spr_bin_path));
        LOG_DEBUG("spr.bin path: %s", g_cmdline.spr_bin_path);
    }

    /* Parse spradrnbin path */
    param_val = cmdline_find_param(cmdline, "spradrnbin");
    if (param_val) {
        cmdline_parse_string(cmdline, "spradrnbin", g_cmdline.spradrn_bin_path, sizeof(g_cmdline.spradrn_bin_path));
        LOG_DEBUG("spradrn.bin path: %s", g_cmdline.spradrn_bin_path);
    }

    /* Parse tokyohot path */
    param_val = cmdline_find_param(cmdline, "tokyohot");
    if (param_val) {
        cmdline_parse_string(cmdline, "tokyohot", g_cmdline.tokyohot_path, sizeof(g_cmdline.tokyohot_path));
        LOG_DEBUG("tokyohot path: %s", g_cmdline.tokyohot_path);
    }

    /* Parse realtrue path */
    param_val = cmdline_find_param(cmdline, "realtrue");
    if (param_val) {
        cmdline_parse_string(cmdline, "realtrue", g_cmdline.realtrue_bin_path, sizeof(g_cmdline.realtrue_bin_path));
        LOG_DEBUG("realtrue.bin path: %s", g_cmdline.realtrue_bin_path);
    }

    /* Parse adrntrue path */
    param_val = cmdline_find_param(cmdline, "adrntrue");
    if (param_val) {
        cmdline_parse_string(cmdline, "adrntrue", g_cmdline.adrntrue_bin_path, sizeof(g_cmdline.adrntrue_bin_path));
        LOG_DEBUG("adrntrue.bin path: %s", g_cmdline.adrntrue_bin_path);
    }

    /* Parse boolean flags */
    g_cmdline.use_alpha = cmdline_parse_bool(cmdline, "usealpha");
    if (g_cmdline.use_alpha) {
        LOG_DEBUG("Alpha blending enabled");
    }

    g_cmdline.window_mode = cmdline_parse_bool(cmdline, "windowmode");
    if (g_cmdline.window_mode) {
        LOG_DEBUG("Window mode enabled");
    }

    g_cmdline.no_delay = cmdline_parse_bool(cmdline, "nodelay");
    if (g_cmdline.no_delay) {
        LOG_DEBUG("No delay mode enabled");
    }

    g_cmdline.initialized = 1;
    LOG_INFO("Command line parsed successfully");
}
