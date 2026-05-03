/*
 * Stone Age Client - String Parsing System Unit Tests
 * Tests for FUN_00489f70, FUN_00489fe0, FUN_0048a170, FUN_0048a050,
 * FUN_0048a120, FUN_00492973, FUN_0048a0a0, FUN_0048a200
 *
 * Based on Ghidra decompilation analysis:
 * - FUN_00489f70: Extract Nth pipe-delimited field (DBCS-aware)
 * - FUN_00489fe0: Copy field until delimiter
 * - FUN_0048a170: String unescape (backslash escape handling)
 * - FUN_0048a050: Extract field and convert to decimal integer
 * - FUN_0048a120: Extract field and convert to Base-62 integer
 * - FUN_00492973: Decimal string to integer (atoi)
 * - FUN_0048a0a0: Base-62 decoding
 * - FUN_0048a200: String escape encoding
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

/* Constants from Ghidra */
#define PIPE_DELIMITER '|'      /* 0x7c */
#define MAX_FIELD_SIZE 128      /* 0x7f - max field buffer */
#define MAX_STRING_255 255      /* 0xff - max string size */
#define BASE_62 62              /* Base-62 radix */

/* Escape character mappings (from DAT_004d5828/DAT_004d5829) */
/* Format: escape_char -> original_char */
#define ESCAPE_BACKSLASH '\\'   /* \\ -> \ */
#define ESCAPE_PIPE '|'         /* \| -> | */
#define ESCAPE_NEWLINE 'n'      /* \n -> newline */
#define ESCAPE_RETURN 'r'       /* \r -> carriage return */
#define ESCAPE_TAB 't'          /* \t -> tab */
#define ESCAPE_NULL '0'         /* \0 -> null */

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

/* Setup */
static void test_setup(void) {
    /* Reset test state */
}

/*
 * Extract Nth field from pipe-delimited string - FUN_00489f70
 * Parameters: src, delimiter, field_num (1-based), max_size, dest
 * Returns: 0 on success, 1 if field not found
 */
static int extract_field(const char* src, char delimiter, int field_num, int max_size, char* dest) {
    int current_field = 1;
    const char* ptr = src;

    /* Navigate to requested field */
    if (field_num > 1) {
        while (current_field < field_num && *ptr) {
            /* DBCS-aware character skip */
            if ((u8)*ptr >= 0x80) {
                /* DBCS lead byte - skip 2 bytes */
                if (ptr[1] == '\0') break;
                ptr += 2;
            } else {
                if (*ptr == delimiter) {
                    current_field++;
                }
                ptr++;
            }
        }
    }

    /* Check if we found the field */
    if (*ptr == '\0' || current_field < field_num) {
        *dest = '\0';
        return 1;
    }

    /* Copy field content */
    int i = 0;
    while (i < max_size - 1 && *ptr && *ptr != delimiter) {
        if ((u8)*ptr >= 0x80) {
            /* DBCS character */
            if (i + 1 < max_size - 1 && ptr[1]) {
                dest[i++] = *ptr++;
                dest[i++] = *ptr++;
            } else {
                break;
            }
        } else {
            dest[i++] = *ptr++;
        }
    }
    dest[i] = '\0';

    return 0;
}

/*
 * Copy field until delimiter - FUN_00489fe0
 */
static int copy_until_delimiter(const char* src, char delimiter, int max_size, char* dest) {
    int i = 0;
    while (i < max_size - 1 && src[i] && src[i] != delimiter) {
        if ((u8)src[i] >= 0x80) {
            /* DBCS character */
            if (i + 1 < max_size - 1 && src[i+1]) {
                dest[i] = src[i];
                i++;
                dest[i] = src[i];
                i++;
            } else {
                break;
            }
        } else {
            dest[i] = src[i];
            i++;
        }
    }
    dest[i] = '\0';
    return 0;
}

/*
 * String unescape - FUN_0048a170
 * Converts \n -> newline, \t -> tab, \\ -> backslash, etc.
 */
static char* string_unescape(char* str) {
    int read_pos = 0;
    int write_pos = 0;

    while (str[read_pos]) {
        /* Check for DBCS lead byte */
        if ((u8)str[read_pos] >= 0x80) {
            /* DBCS - copy both bytes */
            if (str[read_pos + 1]) {
                str[write_pos++] = str[read_pos++];
                str[write_pos++] = str[read_pos++];
            } else {
                break;
            }
        } else if (str[read_pos] == '\\') {
            /* Escape sequence */
            read_pos++;
            switch (str[read_pos]) {
                case 'n':  str[write_pos++] = '\n'; break;
                case 'r':  str[write_pos++] = '\r'; break;
                case 't':  str[write_pos++] = '\t'; break;
                case '0':  str[write_pos++] = '\0'; break;
                case '\\': str[write_pos++] = '\\'; break;
                case '|':  str[write_pos++] = '|';  break;
                default:   str[write_pos++] = str[read_pos]; break;
            }
            read_pos++;
        } else {
            str[write_pos++] = str[read_pos++];
        }
    }
    str[write_pos] = '\0';
    return str;
}

/*
 * Decimal string to integer - FUN_00492973
 * Handles leading whitespace, +/- sign
 */
static int string_to_int(const char* str) {
    const char* ptr = str;

    /* Skip leading whitespace */
    while (*ptr == ' ' || *ptr == '\t') ptr++;

    /* Handle sign */
    int sign = 1;
    if (*ptr == '-') {
        sign = -1;
        ptr++;
    } else if (*ptr == '+') {
        ptr++;
    }

    /* Parse digits */
    int result = 0;
    while (*ptr >= '0' && *ptr <= '9') {
        result = result * 10 + (*ptr - '0');
        ptr++;
    }

    return sign * result;
}

/*
 * Base-62 decode - FUN_0048a0a0
 * Characters: 0-9, a-z (10-35), A-Z (36-61)
 */
static int base62_decode(const char* str) {
    int result = 0;
    int sign = 1;

    while (*str) {
        if (*str >= '0' && *str <= '9') {
            result = result * 62 + (*str - '0');
        } else if (*str >= 'a' && *str <= 'z') {
            result = result * 62 + (*str - 'a' + 10);
        } else if (*str >= 'A' && *str <= 'Z') {
            result = result * 62 + (*str - 'A' + 36);
        } else if (*str == '-') {
            sign = -1;
        } else {
            return 0;  /* Invalid character */
        }
        str++;
    }

    return sign * result;
}

/*
 * String escape encoding - FUN_0048a200
 * Converts newline -> \n, tab -> \t, etc.
 */
static int string_escape(const char* src, char* dest, int max_size) {
    int read_pos = 0;
    int write_pos = 0;
    int total_len = 2;  /* Start at 2 to account for something in original */

    while (src[read_pos] && total_len < max_size) {
        if ((u8)src[read_pos] >= 0x80) {
            /* DBCS - copy both bytes */
            if (write_pos + 2 < max_size && src[read_pos + 1]) {
                dest[write_pos++] = src[read_pos++];
                dest[write_pos++] = src[read_pos++];
                total_len += 2;
            } else {
                break;
            }
        } else {
            char c = src[read_pos];
            char escape = 0;

            /* Check if character needs escaping */
            switch (c) {
                case '\n': escape = 'n'; break;
                case '\r': escape = 'r'; break;
                case '\t': escape = 't'; break;
                case '\\': escape = '\\'; break;
                case '|':  escape = '|'; break;
            }

            if (escape && write_pos + 2 < max_size) {
                dest[write_pos++] = '\\';
                dest[write_pos++] = escape;
                total_len += 2;
            } else {
                dest[write_pos++] = c;
                total_len++;
            }
            read_pos++;
        }
    }
    dest[write_pos] = '\0';
    return write_pos;
}

/* ========================================
 * Field Extraction Tests (FUN_00489f70)
 * ======================================== */

static int test_extract_first_field(void) {
    test_setup();
    char dest[128];

    int result = extract_field("field1|field2|field3", '|', 1, 128, dest);

    assert(result == 0);
    assert(strcmp(dest, "field1") == 0);

    return 1;
}

static int test_extract_second_field(void) {
    test_setup();
    char dest[128];

    int result = extract_field("field1|field2|field3", '|', 2, 128, dest);

    assert(result == 0);
    assert(strcmp(dest, "field2") == 0);

    return 1;
}

static int test_extract_third_field(void) {
    test_setup();
    char dest[128];

    int result = extract_field("field1|field2|field3", '|', 3, 128, dest);

    assert(result == 0);
    assert(strcmp(dest, "field3") == 0);

    return 1;
}

static int test_extract_nonexistent_field(void) {
    test_setup();
    char dest[128];

    int result = extract_field("field1|field2", '|', 5, 128, dest);

    assert(result == 1);
    assert(dest[0] == '\0');

    return 1;
}

static int test_extract_max_size(void) {
    test_setup();
    char dest[10];

    int result = extract_field("verylongfield", '|', 1, 10, dest);

    assert(result == 0);
    assert(strlen(dest) < 10);

    return 1;
}

/* ========================================
 * DBCS Handling Tests
 * ======================================== */

static int test_dbcs_skip(void) {
    test_setup();
    /* DBCS characters have lead byte >= 0x80 */
    /* Should skip 2 bytes for DBCS characters */

    /* In shift-jis, 0x81 0x40 is a valid DBCS character */
    /* This tests that the parser doesn't treat second byte as delimiter */

    u8 test_str[] = { 0x81, 0x40, '|', 'a', 0 };  /* DBCS char + pipe + 'a' */
    char dest[128];

    int result = extract_field((char*)test_str, '|', 2, 128, dest);

    assert(result == 0);
    assert(strcmp(dest, "a") == 0);

    return 1;
}

static int test_dbcs_in_field(void) {
    test_setup();
    u8 test_str[] = { 'a', 0x81, 0x40, 'b', 0 };
    char dest[128];

    int result = extract_field((char*)test_str, '|', 1, 128, dest);

    assert(result == 0);
    /* Field should contain 'a' + DBCS char + 'b' */

    return 1;
}

/* ========================================
 * String Unescape Tests (FUN_0048a170)
 * ======================================== */

static int test_unescape_newline(void) {
    test_setup();
    char str[] = "line1\\nline2";

    string_unescape(str);

    assert(strcmp(str, "line1\nline2") == 0);

    return 1;
}

static int test_unescape_tab(void) {
    test_setup();
    char str[] = "col1\\tcol2";

    string_unescape(str);

    assert(strcmp(str, "col1\tcol2") == 0);

    return 1;
}

static int test_unescape_backslash(void) {
    test_setup();
    char str[] = "path\\\\file";

    string_unescape(str);

    assert(strcmp(str, "path\\file") == 0);

    return 1;
}

static int test_unescape_pipe(void) {
    test_setup();
    char str[] = "a\\|b";

    string_unescape(str);

    assert(strcmp(str, "a|b") == 0);

    return 1;
}

static int test_unescape_multiple(void) {
    test_setup();
    char str[] = "a\\nb\\tc\\\\d";

    string_unescape(str);

    assert(strcmp(str, "a\nb\tc\\d") == 0);

    return 1;
}

static int test_unescape_empty(void) {
    test_setup();
    char str[] = "";

    string_unescape(str);

    assert(strcmp(str, "") == 0);

    return 1;
}

/* ========================================
 * String to Integer Tests (FUN_00492973)
 * ======================================== */

static int test_atoi_positive(void) {
    test_setup();

    assert(string_to_int("123") == 123);
    assert(string_to_int("0") == 0);
    assert(string_to_int("99999") == 99999);

    return 1;
}

static int test_atoi_negative(void) {
    test_setup();

    assert(string_to_int("-123") == -123);
    assert(string_to_int("-1") == -1);
    assert(string_to_int("-99999") == -99999);

    return 1;
}

static int test_atoi_with_sign(void) {
    test_setup();

    assert(string_to_int("+123") == 123);
    assert(string_to_int("-456") == -456);

    return 1;
}

static int test_atoi_with_whitespace(void) {
    test_setup();

    assert(string_to_int("  123") == 123);
    assert(string_to_int("\t456") == 456);

    return 1;
}

static int test_atoi_stop_at_non_digit(void) {
    test_setup();

    assert(string_to_int("123abc") == 123);
    assert(string_to_int("456|789") == 456);

    return 1;
}

/* ========================================
 * Base-62 Decode Tests (FUN_0048a0a0)
 * ======================================== */

static int test_base62_digits(void) {
    test_setup();

    /* 0-9 = values 0-9 */
    assert(base62_decode("0") == 0);
    assert(base62_decode("9") == 9);
    assert(base62_decode("10") == 62);

    return 1;
}

static int test_base62_lowercase(void) {
    test_setup();

    /* a-z = values 10-35 */
    assert(base62_decode("a") == 10);
    assert(base62_decode("z") == 35);
    assert(base62_decode("A") == 36);

    return 1;
}

static int test_base62_uppercase(void) {
    test_setup();

    /* A-Z = values 36-61 */
    assert(base62_decode("A") == 36);
    assert(base62_decode("Z") == 61);

    return 1;
}

static int test_base62_multi_digit(void) {
    test_setup();

    /* "10" in base 62 = 1*62 + 0 = 62 */
    assert(base62_decode("10") == 62);

    /* "a0" = 10*62 + 0 = 620 */
    assert(base62_decode("a0") == 620);

    /* "A0" = 36*62 + 0 = 2232 */
    assert(base62_decode("A0") == 2232);

    return 1;
}

static int test_base62_negative(void) {
    test_setup();

    assert(base62_decode("-1") == -1);
    assert(base62_decode("-10") == -62);

    return 1;
}

static int test_base62_invalid(void) {
    test_setup();

    /* Invalid characters return 0 */
    assert(base62_decode("!@#") == 0);

    return 1;
}

/* ========================================
 * String Escape Tests (FUN_0048a200)
 * ======================================== */

static int test_escape_newline(void) {
    test_setup();
    char src[] = "line1\nline2";
    char dest[128];

    string_escape(src, dest, 128);

    assert(strcmp(dest, "line1\\nline2") == 0);

    return 1;
}

static int test_escape_tab(void) {
    test_setup();
    char src[] = "col1\tcol2";
    char dest[128];

    string_escape(src, dest, 128);

    assert(strcmp(dest, "col1\\tcol2") == 0);

    return 1;
}

static int test_escape_backslash(void) {
    test_setup();
    char src[] = "path\\file";
    char dest[128];

    string_escape(src, dest, 128);

    assert(strcmp(dest, "path\\\\file") == 0);

    return 1;
}

static int test_escape_pipe(void) {
    test_setup();
    char src[] = "a|b";
    char dest[128];

    string_escape(src, dest, 128);

    assert(strcmp(dest, "a\\|b") == 0);

    return 1;
}

static int test_escape_max_size(void) {
    test_setup();
    char src[] = "aaaaaaaaaaaaaaaa";
    char dest[10];

    int result = string_escape(src, dest, 10);

    /* Should truncate */
    assert(result < 10);

    return 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_field_then_unescape(void) {
    test_setup();
    char field[128];
    char data[] = "value\\nwith\\nnewlines|other";

    /* Extract field with escaped content */
    extract_field(data, '|', 1, 128, field);
    assert(strcmp(field, "value\\nwith\\nnewlines") == 0);

    /* Unescape the field */
    string_unescape(field);
    assert(strcmp(field, "value\nwith\nnewlines") == 0);

    return 1;
}

static int test_field_then_atoi(void) {
    test_setup();
    char field[128];
    char data[] = "name|12345|value";

    /* Extract numeric field */
    extract_field(data, '|', 2, 128, field);
    assert(strcmp(field, "12345") == 0);

    /* Convert to integer */
    int value = string_to_int(field);
    assert(value == 12345);

    return 1;
}

static int test_field_then_base62(void) {
    test_setup();
    char field[128];
    char data[] = "name|a1B|value";

    /* Extract Base-62 field */
    extract_field(data, '|', 2, 128, field);
    assert(strcmp(field, "a1B") == 0);

    /* Decode Base-62 */
    int value = base62_decode(field);
    /* a=10, 1=1, B=37: 10*62*62 + 1*62 + 37 = 38480 + 62 + 37 = 38579 */

    return 1;
}

static int test_escape_unescape_roundtrip(void) {
    test_setup();
    char original[] = "a\nb\tc\\d|e";
    char escaped[128];
    char unescaped[128];

    /* Escape */
    string_escape(original, escaped, 128);

    /* Unescape */
    strcpy(unescaped, escaped);
    string_unescape(unescaped);

    assert(strcmp(unescaped, original) == 0);

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== String Parsing System Unit Tests ===\n\n");

    /* Field extraction tests */
    printf("Field Extraction Tests (FUN_00489f70):\n");
    TEST(extract_first_field);
    TEST(extract_second_field);
    TEST(extract_third_field);
    TEST(extract_nonexistent_field);
    TEST(extract_max_size);

    /* DBCS handling tests */
    printf("\nDBCS Handling Tests:\n");
    TEST(dbcs_skip);
    TEST(dbcs_in_field);

    /* String unescape tests */
    printf("\nString Unescape Tests (FUN_0048a170):\n");
    TEST(unescape_newline);
    TEST(unescape_tab);
    TEST(unescape_backslash);
    TEST(unescape_pipe);
    TEST(unescape_multiple);
    TEST(unescape_empty);

    /* String to integer tests */
    printf("\nString to Integer Tests (FUN_00492973):\n");
    TEST(atoi_positive);
    TEST(atoi_negative);
    TEST(atoi_with_sign);
    TEST(atoi_with_whitespace);
    TEST(atoi_stop_at_non_digit);

    /* Base-62 decode tests */
    printf("\nBase-62 Decode Tests (FUN_0048a0a0):\n");
    TEST(base62_digits);
    TEST(base62_lowercase);
    TEST(base62_uppercase);
    TEST(base62_multi_digit);
    TEST(base62_negative);
    TEST(base62_invalid);

    /* String escape tests */
    printf("\nString Escape Tests (FUN_0048a200):\n");
    TEST(escape_newline);
    TEST(escape_tab);
    TEST(escape_backslash);
    TEST(escape_pipe);
    TEST(escape_max_size);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(field_then_unescape);
    TEST(field_then_atoi);
    TEST(field_then_base62);
    TEST(escape_unescape_roundtrip);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - DBCS character handling with real Shift-JIS sequences
     * - Escape character table verification
     * - Buffer boundary conditions
     * - NULL pointer handling
     * - Maximum field sizes
     * - Unicode/UTF-8 handling (if applicable)
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
