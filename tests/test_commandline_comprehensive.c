/*
 * Stone Age Client - Command Line Parsing Comprehensive Tests
 * Tests for commandline.c implementation
 *
 * Covers:
 * - Parameter finding (cmdline_find_param)
 * - Boolean parameter parsing
 * - String parameter parsing
 * - Token iteration (first/next token)
 * - Full command line initialization
 * - Path and user ID parsing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned int u32;
typedef size_t SIZE_T;

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    fflush(stdout); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Constants
 * ======================================== */

#define MAX_PATH_LEN 260
#define MAX_USER_LEN 128
#define MAX_PASS_LEN 128
#define MAX_TOKEN_LEN 256

/* ========================================
 * Structures
 * ======================================== */

typedef struct {
    int initialized;
    char user_id[MAX_USER_LEN];
    char password[MAX_PASS_LEN];
    char session_token[MAX_TOKEN_LEN];
    char server_hint[MAX_TOKEN_LEN];
    char real_bin_path[MAX_PATH_LEN];
    char adrn_bin_path[MAX_PATH_LEN];
    char spr_bin_path[MAX_PATH_LEN];
    char spradrn_bin_path[MAX_PATH_LEN];
    char tokyohot_path[MAX_PATH_LEN];
    char realtrue_bin_path[MAX_PATH_LEN];
    char adrntrue_bin_path[MAX_PATH_LEN];
    int use_alpha;
    int window_mode;
    int no_delay;
} CommandLineContext;

/* ========================================
 * Global State
 * ======================================== */

static CommandLineContext g_cmdline = {0};
static const char* s_token_str = NULL;
static char s_token_delimiter = '\0';

/* ========================================
 * Implementation Functions
 * ======================================== */

const char* cmdline_find_param(const char* cmdline, const char* param) {
    char search_pattern[256];
    const char* pos;
    size_t param_len;

    if (!cmdline || !param) {
        return NULL;
    }

    /* Build search pattern: " param=" */
    sprintf(search_pattern, " %s=", param);
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
    sprintf(search_pattern, " %s", param);

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

void cmdline_parse_string(const char* cmdline, const char* param, char* output, size_t max_len) {
    const char* value_start;
    const char* value_end;
    size_t len;

    if (!output || max_len == 0) {
        return;
    }

    output[0] = '\0';

    if (!cmdline || !param) {
        return;
    }

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

const char* cmdline_get_first_token(const char* str, char delimiter, char* buffer, size_t buf_size) {
    const char* pos;
    size_t len;

    if (!str || !buffer || buf_size == 0) {
        return NULL;
    }

    s_token_delimiter = delimiter;

    /* Find delimiter or end */
    pos = strchr(str, delimiter);
    if (pos) {
        len = (size_t)(pos - str);
        s_token_str = pos + 1;  /* Set position for next token */
    } else {
        len = strlen(str);
        s_token_str = str + len;  /* Point to end */
    }

    if (len >= buf_size) {
        len = buf_size - 1;
    }

    strncpy(buffer, str, len);
    buffer[len] = '\0';

    /* Return next position or NULL */
    return (pos != NULL) ? s_token_str : NULL;
}

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
        g_cmdline.initialized = 1;
        return;
    }

    /* Parse userid - Format: userid=username|password|... */
    param_val = cmdline_find_param(cmdline, "userid");
    if (param_val) {
        char userid_value[512];
        cmdline_parse_string(cmdline, "userid", userid_value, sizeof(userid_value));

        /* Tokenize with '|' delimiter */
        token_pos = userid_value;
        token_index = 0;

        while (*token_pos && token_index < 4) {
            int i = 0;
            while (*token_pos && *token_pos != '|' && i < (int)sizeof(token_buffer) - 1) {
                token_buffer[i++] = *token_pos++;
            }
            token_buffer[i] = '\0';

            if (*token_pos == '|') {
                token_pos++;
            }

            switch (token_index) {
                case 0:
                    strncpy(g_cmdline.user_id, token_buffer, MAX_USER_LEN - 1);
                    break;
                case 1:
                    strncpy(g_cmdline.password, token_buffer, MAX_PASS_LEN - 1);
                    break;
                case 2:
                    strncpy(g_cmdline.session_token, token_buffer, MAX_TOKEN_LEN - 1);
                    break;
                case 3:
                    strncpy(g_cmdline.server_hint, token_buffer, MAX_TOKEN_LEN - 1);
                    break;
            }
            token_index++;
        }
    }

    /* Parse paths */
    param_val = cmdline_find_param(cmdline, "realbin");
    if (param_val) {
        cmdline_parse_string(cmdline, "realbin", g_cmdline.real_bin_path, sizeof(g_cmdline.real_bin_path));
    }

    param_val = cmdline_find_param(cmdline, "adrnbin");
    if (param_val) {
        cmdline_parse_string(cmdline, "adrnbin", g_cmdline.adrn_bin_path, sizeof(g_cmdline.adrn_bin_path));
    }

    param_val = cmdline_find_param(cmdline, "sprbin");
    if (param_val) {
        cmdline_parse_string(cmdline, "sprbin", g_cmdline.spr_bin_path, sizeof(g_cmdline.spr_bin_path));
    }

    param_val = cmdline_find_param(cmdline, "spradrnbin");
    if (param_val) {
        cmdline_parse_string(cmdline, "spradrnbin", g_cmdline.spradrn_bin_path, sizeof(g_cmdline.spradrn_bin_path));
    }

    param_val = cmdline_find_param(cmdline, "tokyohot");
    if (param_val) {
        cmdline_parse_string(cmdline, "tokyohot", g_cmdline.tokyohot_path, sizeof(g_cmdline.tokyohot_path));
    }

    param_val = cmdline_find_param(cmdline, "realtrue");
    if (param_val) {
        cmdline_parse_string(cmdline, "realtrue", g_cmdline.realtrue_bin_path, sizeof(g_cmdline.realtrue_bin_path));
    }

    param_val = cmdline_find_param(cmdline, "adrntrue");
    if (param_val) {
        cmdline_parse_string(cmdline, "adrntrue", g_cmdline.adrntrue_bin_path, sizeof(g_cmdline.adrntrue_bin_path));
    }

    /* Parse boolean flags */
    g_cmdline.use_alpha = cmdline_parse_bool(cmdline, "usealpha");
    g_cmdline.window_mode = cmdline_parse_bool(cmdline, "windowmode");
    g_cmdline.no_delay = cmdline_parse_bool(cmdline, "nodelay");

    g_cmdline.initialized = 1;
}

static void cmdline_reset(void) {
    memset(&g_cmdline, 0, sizeof(CommandLineContext));
}

/* ========================================
 * Test Cases - Find Parameter
 * ======================================== */

static int test_find_param_at_start(void) {
    const char* cmdline = "userid=testuser";
    const char* value = cmdline_find_param(cmdline, "userid");
    return value != NULL && strcmp(value, "testuser") == 0;
}

static int test_find_param_in_middle(void) {
    const char* cmdline = "usealpha userid=testuser windowmode";
    const char* value = cmdline_find_param(cmdline, "userid");
    /* Value ends at space, so we get "testuser" without the trailing space */
    return value != NULL && strncmp(value, "testuser", 8) == 0;
}

static int test_find_param_not_found(void) {
    const char* cmdline = "usealpha windowmode";
    return cmdline_find_param(cmdline, "userid") == NULL;
}

static int test_find_param_null_cmdline(void) {
    return cmdline_find_param(NULL, "userid") == NULL;
}

static int test_find_param_null_param(void) {
    return cmdline_find_param("userid=test", NULL) == NULL;
}

static int test_find_param_empty_cmdline(void) {
    return cmdline_find_param("", "userid") == NULL;
}

static int test_find_param_with_spaces(void) {
    const char* cmdline = "realbin=C:\\path\\with spaces\\file.bin";
    const char* value = cmdline_find_param(cmdline, "realbin");
    /* Value ends at space or end of string */
    return value != NULL && strncmp(value, "C:\\path\\with", 12) == 0;
}

/* ========================================
 * Test Cases - Boolean Parsing
 * ======================================== */

static int test_parse_bool_present(void) {
    const char* cmdline = "usealpha windowmode";
    return cmdline_parse_bool(cmdline, "usealpha") == 1;
}

static int test_parse_bool_at_end(void) {
    const char* cmdline = "userid=test windowmode";
    return cmdline_parse_bool(cmdline, "windowmode") == 1;
}

static int test_parse_bool_not_present(void) {
    const char* cmdline = "userid=test";
    return cmdline_parse_bool(cmdline, "usealpha") == 0;
}

static int test_parse_bool_null_cmdline(void) {
    return cmdline_parse_bool(NULL, "usealpha") == 0;
}

static int test_parse_bool_null_param(void) {
    return cmdline_parse_bool("usealpha", NULL) == 0;
}

static int test_parse_bool_prefix_match(void) {
    const char* cmdline = "usealphatest value";
    /* "usealpha" is a prefix of "usealphatest", but shouldn't match */
    return cmdline_parse_bool(cmdline, "usealpha") == 0;
}

static int test_parse_bool_multiple_flags(void) {
    const char* cmdline = "usealpha windowmode nodelay";
    return cmdline_parse_bool(cmdline, "usealpha") == 1 &&
           cmdline_parse_bool(cmdline, "windowmode") == 1 &&
           cmdline_parse_bool(cmdline, "nodelay") == 1;
}

/* ========================================
 * Test Cases - String Parsing
 * ======================================== */

static int test_parse_string_basic(void) {
    char output[256];
    const char* cmdline = "userid=testuser";
    cmdline_parse_string(cmdline, "userid", output, sizeof(output));
    return strcmp(output, "testuser") == 0;
}

static int test_parse_string_with_space(void) {
    char output[256];
    const char* cmdline = "userid=testuser other=value";
    cmdline_parse_string(cmdline, "userid", output, sizeof(output));
    return strcmp(output, "testuser") == 0;
}

static int test_parse_string_not_found(void) {
    char output[256] = "unchanged";
    const char* cmdline = "other=value";
    cmdline_parse_string(cmdline, "userid", output, sizeof(output));
    return output[0] == '\0';
}

static int test_parse_string_null_cmdline(void) {
    char output[256] = "unchanged";
    cmdline_parse_string(NULL, "userid", output, sizeof(output));
    /* Output should be cleared to empty string when cmdline is NULL */
    return output[0] == '\0';
}

static int test_parse_string_null_output(void) {
    /* Should not crash */
    cmdline_parse_string("userid=test", "userid", NULL, 256);
    return 1;
}

static int test_parse_string_zero_size(void) {
    char output[256] = "unchanged";
    cmdline_parse_string("userid=test", "userid", output, 0);
    /* Output should be unchanged */
    return strcmp(output, "unchanged") == 0;
}

static int test_parse_string_truncation(void) {
    char output[8];
    const char* cmdline = "userid=verylongusername";
    cmdline_parse_string(cmdline, "userid", output, sizeof(output));
    return strlen(output) == 7 && strncmp(output, "verylon", 7) == 0;
}

/* ========================================
 * Test Cases - Token Iteration
 * ======================================== */

static int test_first_token_basic(void) {
    char buffer[256];
    const char* next = cmdline_get_first_token("value1|value2|value3", '|', buffer, sizeof(buffer));
    return strcmp(buffer, "value1") == 0 && next != NULL;
}

static int test_first_token_single(void) {
    char buffer[256];
    const char* next = cmdline_get_first_token("single", '|', buffer, sizeof(buffer));
    return strcmp(buffer, "single") == 0 && next == NULL;
}

static int test_first_token_null_str(void) {
    char buffer[256];
    return cmdline_get_first_token(NULL, '|', buffer, sizeof(buffer)) == NULL;
}

static int test_first_token_null_buffer(void) {
    return cmdline_get_first_token("test", '|', NULL, 256) == NULL;
}

static int test_next_token_basic(void) {
    char buffer[256];
    const char* next;

    cmdline_get_first_token("value1|value2|value3", '|', buffer, sizeof(buffer));
    next = cmdline_get_next_token('|', buffer, sizeof(buffer));

    return strcmp(buffer, "value2") == 0 && next != NULL;
}

static int test_next_token_last(void) {
    char buffer[256];
    const char* next;

    cmdline_get_first_token("value1|value2", '|', buffer, sizeof(buffer));
    cmdline_get_next_token('|', buffer, sizeof(buffer));
    next = cmdline_get_next_token('|', buffer, sizeof(buffer));

    return strcmp(buffer, "value2") == 0 && next == NULL;
}

static int test_token_iteration_all(void) {
    char buffer[256];
    int count = 0;
    const char* next;

    next = cmdline_get_first_token("a|b|c|d", '|', buffer, sizeof(buffer));
    count++;  /* First token "a" */

    while (next) {
        next = cmdline_get_next_token('|', buffer, sizeof(buffer));
        if (buffer[0] != '\0') {
            count++;
        }
    }

    return count == 4;
}

static int test_token_different_delimiter(void) {
    char buffer[256];
    cmdline_get_first_token("a,b,c", ',', buffer, sizeof(buffer));
    return strcmp(buffer, "a") == 0;
}

static int test_token_truncation(void) {
    char buffer[8];
    cmdline_get_first_token("verylongtoken", '|', buffer, sizeof(buffer));
    return strlen(buffer) == 7;
}

/* ========================================
 * Test Cases - Full Initialization
 * ======================================== */

static int test_init_basic(void) {
    cmdline_reset();
    cmdline_init("userid=user1|pass2 windowmode");
    return g_cmdline.initialized == 1;
}

static int test_init_null_cmdline(void) {
    cmdline_reset();
    cmdline_init(NULL);
    return g_cmdline.initialized == 1;
}

static int test_init_twice(void) {
    cmdline_reset();
    cmdline_init("userid=test");
    cmdline_init("userid=other");  /* Should be ignored */
    return strcmp(g_cmdline.user_id, "test") == 0;
}

static int test_init_userid_parsing(void) {
    cmdline_reset();
    cmdline_init("userid=user1|pass2|token3|server4");
    return strcmp(g_cmdline.user_id, "user1") == 0 &&
           strcmp(g_cmdline.password, "pass2") == 0 &&
           strcmp(g_cmdline.session_token, "token3") == 0 &&
           strcmp(g_cmdline.server_hint, "server4") == 0;
}

static int test_init_userid_partial(void) {
    cmdline_reset();
    cmdline_init("userid=user1|pass2");
    return strcmp(g_cmdline.user_id, "user1") == 0 &&
           strcmp(g_cmdline.password, "pass2") == 0 &&
           g_cmdline.session_token[0] == '\0';
}

static int test_init_realbin_path(void) {
    cmdline_reset();
    cmdline_init("realbin=C:\\game\\real.bin");
    return strcmp(g_cmdline.real_bin_path, "C:\\game\\real.bin") == 0;
}

static int test_init_adrnbin_path(void) {
    cmdline_reset();
    cmdline_init("adrnbin=C:\\game\\adrn.bin");
    return strcmp(g_cmdline.adrn_bin_path, "C:\\game\\adrn.bin") == 0;
}

static int test_init_sprbin_path(void) {
    cmdline_reset();
    cmdline_init("sprbin=C:\\game\\spr.bin");
    return strcmp(g_cmdline.spr_bin_path, "C:\\game\\spr.bin") == 0;
}

static int test_init_spradrnbin_path(void) {
    cmdline_reset();
    cmdline_init("spradrnbin=C:\\game\\spradrn.bin");
    return strcmp(g_cmdline.spradrn_bin_path, "C:\\game\\spradrn.bin") == 0;
}

static int test_init_tokyohot_path(void) {
    cmdline_reset();
    cmdline_init("tokyohot=C:\\game\\tokyohot.dat");
    return strcmp(g_cmdline.tokyohot_path, "C:\\game\\tokyohot.dat") == 0;
}

static int test_init_realtrue_path(void) {
    cmdline_reset();
    cmdline_init("realtrue=C:\\game\\realtrue.bin");
    return strcmp(g_cmdline.realtrue_bin_path, "C:\\game\\realtrue.bin") == 0;
}

static int test_init_adrntrue_path(void) {
    cmdline_reset();
    cmdline_init("adrntrue=C:\\game\\adrntrue.bin");
    return strcmp(g_cmdline.adrntrue_bin_path, "C:\\game\\adrntrue.bin") == 0;
}

static int test_init_use_alpha(void) {
    cmdline_reset();
    cmdline_init("usealpha");
    return g_cmdline.use_alpha == 1;
}

static int test_init_window_mode(void) {
    cmdline_reset();
    cmdline_init("windowmode");
    return g_cmdline.window_mode == 1;
}

static int test_init_no_delay(void) {
    cmdline_reset();
    cmdline_init("nodelay");
    return g_cmdline.no_delay == 1;
}

static int test_init_all_flags(void) {
    cmdline_reset();
    cmdline_init("usealpha windowmode nodelay");
    return g_cmdline.use_alpha == 1 &&
           g_cmdline.window_mode == 1 &&
           g_cmdline.no_delay == 1;
}

static int test_init_full_cmdline(void) {
    cmdline_reset();
    cmdline_init("userid=testuser|testpass realbin=C:\\real.bin adrnbin=C:\\adrn.bin "
                 "sprbin=C:\\spr.bin usealpha windowmode");

    return strcmp(g_cmdline.user_id, "testuser") == 0 &&
           strcmp(g_cmdline.password, "testpass") == 0 &&
           strcmp(g_cmdline.real_bin_path, "C:\\real.bin") == 0 &&
           strcmp(g_cmdline.adrn_bin_path, "C:\\adrn.bin") == 0 &&
           strcmp(g_cmdline.spr_bin_path, "C:\\spr.bin") == 0 &&
           g_cmdline.use_alpha == 1 &&
           g_cmdline.window_mode == 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Command Line Parsing Comprehensive Tests ===\n\n");

    printf("Find Parameter Tests:\n");
    TEST(find_param_at_start);
    TEST(find_param_in_middle);
    TEST(find_param_not_found);
    TEST(find_param_null_cmdline);
    TEST(find_param_null_param);
    TEST(find_param_empty_cmdline);
    TEST(find_param_with_spaces);

    printf("\nBoolean Parsing Tests:\n");
    TEST(parse_bool_present);
    TEST(parse_bool_at_end);
    TEST(parse_bool_not_present);
    TEST(parse_bool_null_cmdline);
    TEST(parse_bool_null_param);
    TEST(parse_bool_prefix_match);
    TEST(parse_bool_multiple_flags);

    printf("\nString Parsing Tests:\n");
    TEST(parse_string_basic);
    TEST(parse_string_with_space);
    TEST(parse_string_not_found);
    TEST(parse_string_null_cmdline);
    TEST(parse_string_null_output);
    TEST(parse_string_zero_size);
    TEST(parse_string_truncation);

    printf("\nToken Iteration Tests:\n");
    TEST(first_token_basic);
    TEST(first_token_single);
    TEST(first_token_null_str);
    TEST(first_token_null_buffer);
    TEST(next_token_basic);
    TEST(next_token_last);
    TEST(token_iteration_all);
    TEST(token_different_delimiter);
    TEST(token_truncation);

    printf("\nFull Initialization Tests:\n");
    TEST(init_basic);
    TEST(init_null_cmdline);
    TEST(init_twice);
    TEST(init_userid_parsing);
    TEST(init_userid_partial);
    TEST(init_realbin_path);
    TEST(init_adrnbin_path);
    TEST(init_sprbin_path);
    TEST(init_spradrnbin_path);
    TEST(init_tokyohot_path);
    TEST(init_realtrue_path);
    TEST(init_adrntrue_path);
    TEST(init_use_alpha);
    TEST(init_window_mode);
    TEST(init_no_delay);
    TEST(init_all_flags);
    TEST(init_full_cmdline);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
