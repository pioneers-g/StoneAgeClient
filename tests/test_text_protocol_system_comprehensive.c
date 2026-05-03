/*
 * Stone Age Client - Text Protocol System Comprehensive Tests
 * Tests for FUN_0043bf90 (text protocol dispatcher) and related functions
 *
 * Coverage:
 * - Command parsing
 * - Parameter extraction
 * - String processing
 * - Handler routing
 * - Command categories
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Maximum fields in parsed command */
#define MAX_FIELDS 16
#define MAX_FIELD_LEN 256
#define MAX_COMMAND_LEN 1024

/* Parsed command structure */
typedef struct {
    char raw[MAX_COMMAND_LEN];
    int field_count;
    char fields[MAX_FIELDS][MAX_FIELD_LEN];
} ParsedCommand;

/* Command handler type */
typedef void (*CommandHandler)(void* ctx, ParsedCommand* cmd);

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Command Parsing Functions
 * ======================================== */

/*
 * Parse command string into fields
 * Simplified version of FUN_0043dbe0
 */
static int parse_command(const char* input, ParsedCommand* cmd) {
    if (!input || !cmd) return -1;

    memset(cmd, 0, sizeof(ParsedCommand));
    strncpy(cmd->raw, input, MAX_COMMAND_LEN - 1);

    const char* p = input;
    int field_idx = 0;
    int char_idx = 0;
    int in_field = 0;

    while (*p && field_idx < MAX_FIELDS) {
        if (*p == ' ' || *p == '\t') {
            if (in_field) {
                cmd->fields[field_idx][char_idx] = '\0';
                field_idx++;
                char_idx = 0;
                in_field = 0;
            }
        } else if (*p == '\r' || *p == '\n') {
            /* End of command */
            if (in_field) {
                cmd->fields[field_idx][char_idx] = '\0';
                field_idx++;
                in_field = 0;  /* FIX: Clear in_field to prevent post-loop increment */
            }
            break;
        } else {
            if (char_idx < MAX_FIELD_LEN - 1) {
                cmd->fields[field_idx][char_idx++] = *p;
            }
            in_field = 1;
        }
        p++;
    }

    if (in_field && field_idx < MAX_FIELDS) {
        cmd->fields[field_idx][char_idx] = '\0';
        field_idx++;
    }

    cmd->field_count = field_idx;
    return field_idx;
}

/*
 * Extract integer parameter - FUN_0043dd50
 */
static int extract_int(const char* field) {
    if (!field || !*field) return 0;
    return atoi(field);
}

/*
 * Extract string parameter - FUN_0043dd70
 */
static const char* extract_string(const char* field) {
    return field ? field : "";
}

/*
 * Process string (DBCS-aware copy) - FUN_0043e1b0
 */
static int process_string(char* dest, size_t dest_size, const char* src) {
    if (!dest || !src || dest_size == 0) return -1;

    size_t i = 0;
    while (*src && i < dest_size - 1) {
        /* Check for DBCS lead byte (simplified) */
        if ((u8)*src >= 0x81 && (u8)*src <= 0xFE) {
            /* DBCS character - copy both bytes */
            if (i + 1 < dest_size - 1 && *(src + 1)) {
                dest[i++] = *src++;
                dest[i++] = *src++;
            } else {
                break;
            }
        } else {
            dest[i++] = *src++;
        }
    }
    dest[i] = '\0';
    return (int)i;
}

/* ========================================
 * Command Matching Functions
 * ======================================== */

/*
 * Compare command name - simplified from binary
 */
static int match_command(const char* input, const char* command) {
    while (*input && *command) {
        if (*input != *command) {
            return (int)(u8)*input - (int)(u8)*command;
        }
        input++;
        command++;
    }
    if (*command) return -1;
    if (*input && *input != ' ') return (int)(u8)*input;
    return 0;
}

/*
 * Get command category
 */
static int get_command_category(const char* cmd) {
    if (!cmd || !*cmd) return -1;

    /* Login commands */
    if (strcmp(cmd, "ClientLogin") == 0) return 1;
    if (strcmp(cmd, "CreateNewChar") == 0) return 2;
    if (strcmp(cmd, "CharDelete") == 0) return 3;
    if (strcmp(cmd, "CharLogin") == 0) return 4;
    if (strcmp(cmd, "CharList") == 0) return 5;
    if (strcmp(cmd, "CharLogout") == 0) return 6;

    /* Party commands */
    if (strcmp(cmd, "XYD") == 0) return 10;
    if (strcmp(cmd, "EV") == 0) return 11;
    if (strcmp(cmd, "EN") == 0) return 12;
    if (strcmp(cmd, "PM") == 0) return 13;
    if (strcmp(cmd, "PS") == 0) return 14;

    /* Communication commands */
    if (strcmp(cmd, "MC") == 0) return 20;
    if (strcmp(cmd, "CA") == 0) return 21;
    if (strcmp(cmd, "DU") == 0) return 22;
    if (strcmp(cmd, "EO") == 0) return 23;

    /* NPC/Map commands */
    if (strcmp(cmd, "N") == 0) return 30;
    if (strcmp(cmd, "B") == 0) return 31;
    if (strcmp(cmd, "PT") == 0) return 32;
    if (strcmp(cmd, "PD") == 0) return 33;

    /* Battle commands */
    if (strcmp(cmd, "CP") == 0) return 40;
    if (strcmp(cmd, "P") == 0) return 41;
    if (strcmp(cmd, "R") == 0) return 42;

    /* System commands */
    if (strcmp(cmd, "ProcGet") == 0) return 50;
    if (strcmp(cmd, "PlayerNumGet") == 0) return 51;
    if (cmd[0] == 'K') return 60;  /* KA-KK key storage */
    if (cmd[0] == 'S') return 70;  /* SK-SN skill/mail */

    return -1;
}

/* ========================================
 * Command Parsing Tests
 * ======================================== */

static int test_parse_simple(void) {
    ParsedCommand cmd;
    int count = parse_command("ClientLogin test", &cmd);
    return count == 2 &&
           strcmp(cmd.fields[0], "ClientLogin") == 0 &&
           strcmp(cmd.fields[1], "test") == 0;
}

static int test_parse_multiple_params(void) {
    ParsedCommand cmd;
    int count = parse_command("XYD 100 200 300", &cmd);
    return count == 4 &&
           strcmp(cmd.fields[0], "XYD") == 0 &&
           extract_int(cmd.fields[1]) == 100 &&
           extract_int(cmd.fields[2]) == 200 &&
           extract_int(cmd.fields[3]) == 300;
}

static int test_parse_empty(void) {
    ParsedCommand cmd;
    int count = parse_command("", &cmd);
    return count == 0;
}

static int test_parse_whitespace(void) {
    ParsedCommand cmd;
    int count = parse_command("   MC   hello   world   ", &cmd);
    return count == 3 &&
           strcmp(cmd.fields[0], "MC") == 0 &&
           strcmp(cmd.fields[1], "hello") == 0 &&
           strcmp(cmd.fields[2], "world") == 0;
}

static int test_parse_newline(void) {
    ParsedCommand cmd;
    int count = parse_command("ClientLogin user\r\n", &cmd);
    return count == 2 &&
           strcmp(cmd.fields[0], "ClientLogin") == 0 &&
           strcmp(cmd.fields[1], "user") == 0;
}

static int test_parse_long_field(void) {
    ParsedCommand cmd;
    char long_field[300];
    memset(long_field, 'A', 299);
    long_field[299] = '\0';

    char input[512];
    snprintf(input, sizeof(input), "MC %s", long_field);

    int count = parse_command(input, &cmd);
    return count == 2 && strlen(cmd.fields[1]) == MAX_FIELD_LEN - 1;
}

/* ========================================
 * Parameter Extraction Tests
 * ======================================== */

static int test_extract_int_positive(void) {
    return extract_int("12345") == 12345;
}

static int test_extract_int_negative(void) {
    return extract_int("-6789") == -6789;
}

static int test_extract_int_zero(void) {
    return extract_int("0") == 0;
}

static int test_extract_int_invalid(void) {
    return extract_int("abc") == 0;
}

static int test_extract_int_empty(void) {
    return extract_int("") == 0;
}

static int test_extract_string_basic(void) {
    const char* str = extract_string("hello");
    return strcmp(str, "hello") == 0;
}

static int test_extract_string_empty(void) {
    const char* str = extract_string("");
    return strcmp(str, "") == 0;
}

static int test_extract_string_null(void) {
    const char* str = extract_string(NULL);
    return strcmp(str, "") == 0;
}

/* ========================================
 * String Processing Tests
 * ======================================== */

static int test_process_string_ascii(void) {
    char dest[64];
    int len = process_string(dest, sizeof(dest), "Hello World");
    return len == 11 && strcmp(dest, "Hello World") == 0;
}

static int test_process_string_empty(void) {
    char dest[64];
    int len = process_string(dest, sizeof(dest), "");
    return len == 0 && dest[0] == '\0';
}

static int test_process_string_dbcs(void) {
    char dest[64];
    /* Simplified DBCS test - lead byte 0x82 (Japanese Shift-JIS range) */
    const char* dbcs_str = "\x82\xb1\x82\xf1\x82\xc9\x82\xbf\x82\xcd"; /* Japanese */
    int len = process_string(dest, sizeof(dest), dbcs_str);
    return len > 0 && len % 2 == 0;  /* Should be even for DBCS pairs */
}

static int test_process_string_truncate(void) {
    char dest[8];
    int len = process_string(dest, sizeof(dest), "This is a long string");
    return len == 7 && dest[7] == '\0';
}

/* ========================================
 * Command Matching Tests
 * ======================================== */

static int test_match_exact(void) {
    return match_command("ClientLogin test", "ClientLogin") == 0;
}

static int test_match_partial(void) {
    return match_command("ClientL", "ClientLogin") != 0;
}

static int test_match_longer_input(void) {
    return match_command("ClientLoginExtra", "ClientLogin") != 0;
}

static int test_match_short_command(void) {
    return match_command("N dialog data", "N") == 0;
}

static int test_match_two_char(void) {
    return match_command("MC hello", "MC") == 0;
}

/* ========================================
 * Command Category Tests
 * ======================================== */

static int test_category_login(void) {
    return get_command_category("ClientLogin") == 1;
}

static int test_category_create_char(void) {
    return get_command_category("CreateNewChar") == 2;
}

static int test_category_char_list(void) {
    return get_command_category("CharList") == 5;
}

static int test_category_party(void) {
    return get_command_category("XYD") == 10;
}

static int test_category_chat(void) {
    return get_command_category("MC") == 20;
}

static int test_category_npc(void) {
    return get_command_category("N") == 30;
}

static int test_category_battle(void) {
    return get_command_category("P") == 41;
}

static int test_category_key_storage(void) {
    return get_command_category("KA") == 60 &&
           get_command_category("KB") == 60 &&
           get_command_category("KK") == 60;
}

static int test_category_skill(void) {
    return get_command_category("SK") == 70 &&
           get_command_category("SM") == 70;
}

static int test_category_unknown(void) {
    return get_command_category("UNKNOWN") == -1;
}

/* ========================================
 * Protocol Handler Tests
 * ======================================== */

/* Simulated handler call counts */
static int s_login_handler_calls = 0;
static int s_chat_handler_calls = 0;
static int s_npc_handler_calls = 0;

static void reset_handlers(void) {
    s_login_handler_calls = 0;
    s_chat_handler_calls = 0;
    s_npc_handler_calls = 0;
}

/*
 * Simulated dispatcher - simplified FUN_0043bf90
 */
static int dispatch_command(void* ctx, ParsedCommand* cmd) {
    if (cmd->field_count == 0) return -1;

    const char* command = cmd->fields[0];
    int category = get_command_category(command);

    switch (category) {
        case 1:  /* ClientLogin */
            s_login_handler_calls++;
            return 0;
        case 20: /* MC chat */
            s_chat_handler_calls++;
            return 0;
        case 30: /* N NPC dialog */
            s_npc_handler_calls++;
            return 0;
        case -1:
            return -1;  /* Unknown command */
        default:
            return 0;
    }
}

static int test_dispatch_login(void) {
    reset_handlers();
    ParsedCommand cmd;
    parse_command("ClientLogin user pass", &cmd);
    int result = dispatch_command(NULL, &cmd);
    return result == 0 && s_login_handler_calls == 1;
}

static int test_dispatch_chat(void) {
    reset_handlers();
    ParsedCommand cmd;
    parse_command("MC Hello World", &cmd);
    int result = dispatch_command(NULL, &cmd);
    return result == 0 && s_chat_handler_calls == 1;
}

static int test_dispatch_npc(void) {
    reset_handlers();
    ParsedCommand cmd;
    parse_command("N Welcome to the village", &cmd);
    int result = dispatch_command(NULL, &cmd);
    return result == 0 && s_npc_handler_calls == 1;
}

static int test_dispatch_unknown(void) {
    reset_handlers();
    ParsedCommand cmd;
    parse_command("UNKNOWN command here", &cmd);
    int result = dispatch_command(NULL, &cmd);
    return result == -1;
}

static int test_dispatch_empty(void) {
    reset_handlers();
    ParsedCommand cmd;
    parse_command("", &cmd);
    int result = dispatch_command(NULL, &cmd);
    return result == -1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_login_flow(void) {
    ParsedCommand cmd;

    /* Parse login command */
    int count = parse_command("ClientLogin myuser mypass", &cmd);
    if (count != 3) return 0;

    /* Verify command structure */
    if (get_command_category(cmd.fields[0]) != 1) return 0;
    if (strcmp(cmd.fields[1], "myuser") != 0) return 0;
    if (strcmp(cmd.fields[2], "mypass") != 0) return 0;

    /* Dispatch */
    reset_handlers();
    int result = dispatch_command(NULL, &cmd);
    return result == 0 && s_login_handler_calls == 1;
}

static int test_full_chat_flow(void) {
    ParsedCommand cmd;

    /* Parse chat command with spaces in message */
    int count = parse_command("MC Hello everyone in the game!", &cmd);
    if (count < 2) return 0;

    /* Verify */
    if (get_command_category(cmd.fields[0]) != 20) return 0;

    /* Dispatch */
    reset_handlers();
    int result = dispatch_command(NULL, &cmd);
    return result == 0 && s_chat_handler_calls == 1;
}

static int test_multi_command_sequence(void) {
    ParsedCommand cmd;

    reset_handlers();

    /* Sequence: login -> chat -> npc */
    parse_command("ClientLogin user pass", &cmd);
    dispatch_command(NULL, &cmd);

    parse_command("MC Hello!", &cmd);
    dispatch_command(NULL, &cmd);

    parse_command("N Welcome!", &cmd);
    dispatch_command(NULL, &cmd);

    return s_login_handler_calls == 1 &&
           s_chat_handler_calls == 1 &&
           s_npc_handler_calls == 1;
}

static int test_protocol_with_int_params(void) {
    ParsedCommand cmd;

    /* XYD command with 3 integer parameters */
    int count = parse_command("XYD 100 200 300", &cmd);
    if (count != 4) return 0;

    int p1 = extract_int(cmd.fields[1]);
    int p2 = extract_int(cmd.fields[2]);
    int p3 = extract_int(cmd.fields[3]);

    return p1 == 100 && p2 == 200 && p3 == 300;
}

static int test_protocol_mixed_params(void) {
    ParsedCommand cmd;

    /* CP command with mixed int and string params */
    int count = parse_command("CP 1 2 PlayerName", &cmd);
    if (count != 4) return 0;

    int p1 = extract_int(cmd.fields[1]);
    int p2 = extract_int(cmd.fields[2]);
    const char* p3 = extract_string(cmd.fields[3]);

    return p1 == 1 && p2 == 2 && strcmp(p3, "PlayerName") == 0;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Text Protocol System Comprehensive Tests ===\n\n");

    /* Command parsing tests */
    printf("Command Parsing Tests:\n");
    TEST(parse_simple);
    TEST(parse_multiple_params);
    TEST(parse_empty);
    TEST(parse_whitespace);
    TEST(parse_newline);
    TEST(parse_long_field);

    /* Parameter extraction tests */
    printf("\nParameter Extraction Tests:\n");
    TEST(extract_int_positive);
    TEST(extract_int_negative);
    TEST(extract_int_zero);
    TEST(extract_int_invalid);
    TEST(extract_int_empty);
    TEST(extract_string_basic);
    TEST(extract_string_empty);
    TEST(extract_string_null);

    /* String processing tests */
    printf("\nString Processing Tests:\n");
    TEST(process_string_ascii);
    TEST(process_string_empty);
    TEST(process_string_dbcs);
    TEST(process_string_truncate);

    /* Command matching tests */
    printf("\nCommand Matching Tests:\n");
    TEST(match_exact);
    TEST(match_partial);
    TEST(match_longer_input);
    TEST(match_short_command);
    TEST(match_two_char);

    /* Command category tests */
    printf("\nCommand Category Tests:\n");
    TEST(category_login);
    TEST(category_create_char);
    TEST(category_char_list);
    TEST(category_party);
    TEST(category_chat);
    TEST(category_npc);
    TEST(category_battle);
    TEST(category_key_storage);
    TEST(category_skill);
    TEST(category_unknown);

    /* Protocol handler tests */
    printf("\nProtocol Handler Tests:\n");
    TEST(dispatch_login);
    TEST(dispatch_chat);
    TEST(dispatch_npc);
    TEST(dispatch_unknown);
    TEST(dispatch_empty);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_login_flow);
    TEST(full_chat_flow);
    TEST(multi_command_sequence);
    TEST(protocol_with_int_params);
    TEST(protocol_mixed_params);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - Binary protocol gateway (P command)
     * - Escape sequences in strings
     * - Maximum field count
     * - DBCS parameter handling
     * - Error recovery
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
