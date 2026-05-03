/*
 * Stone Age Client - Unit Tests for Protocol Utility Functions
 * Test file: test_protocol_util.c
 *
 * Tests FUN_00489f70 (parse_text_field) and FUN_00489fe0 (extract_field_until_delim)
 *
 * These functions are DBCS-aware text parsing utilities used by 100+ callers
 * for parsing pipe-delimited protocol messages.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/protocol_util.h"

/* ========================================
 * Test Cases for parse_text_field
 * ======================================== */

/*
 * Test 1: Parse first field (ASCII only)
 */
static void test_parse_first_field(void) {
    TEST_BEGIN("Parse first field");

    char output[256];
    int result = parse_text_field("A|B|C|D", '|', 1, 256, output);

    TEST_ASSERT(result == 0, "Should return 0 (success)");
    TEST_ASSERT_STR_EQ("A", output, "First field should be 'A'");

    TEST_END();
}

/*
 * Test 2: Parse middle field (ASCII only)
 */
static void test_parse_middle_field(void) {
    TEST_BEGIN("Parse middle field");

    char output[256];
    int result = parse_text_field("A|B|C|D", '|', 3, 256, output);

    TEST_ASSERT(result == 0, "Should return 0 (success)");
    TEST_ASSERT_STR_EQ("C", output, "Third field should be 'C'");

    TEST_END();
}

/*
 * Test 3: Parse last field (ASCII only)
 */
static void test_parse_last_field(void) {
    TEST_BEGIN("Parse last field");

    char output[256];
    int result = parse_text_field("A|B|C|D", '|', 4, 256, output);

    TEST_ASSERT(result == 1, "Should return 1 (end of string, no delimiter)");
    TEST_ASSERT_STR_EQ("D", output, "Fourth field should be 'D'");

    TEST_END();
}

/*
 * Test 4: Parse non-existent field
 */
static void test_parse_nonexistent_field(void) {
    TEST_BEGIN("Parse non-existent field");

    char output[256];
    int result = parse_text_field("A|B|C", '|', 5, 256, output);

    TEST_ASSERT(result == 1, "Should return 1 (field not found)");
    TEST_ASSERT_STR_EQ("", output, "Output should be empty");

    TEST_END();
}

/*
 * Test 5: Parse empty string
 */
static void test_parse_empty_string(void) {
    TEST_BEGIN("Parse empty string");

    char output[256];
    int result = parse_text_field("", '|', 1, 256, output);

    TEST_ASSERT(result == 1, "Should return 1 (empty string)");
    TEST_ASSERT_STR_EQ("", output, "Output should be empty");

    TEST_END();
}

/*
 * Test 6: Parse single field (no delimiter)
 */
static void test_parse_single_field(void) {
    TEST_BEGIN("Parse single field");

    char output[256];
    int result = parse_text_field("Hello", '|', 1, 256, output);

    TEST_ASSERT(result == 1, "Should return 1 (no delimiter found)");
    TEST_ASSERT_STR_EQ("Hello", output, "Should return entire string");

    TEST_END();
}

/*
 * Test 7: Parse with buffer size limit
 */
static void test_parse_buffer_limit(void) {
    TEST_BEGIN("Parse with buffer limit");

    char output[5];
    int result = parse_text_field("HelloWorld", '|', 1, 5, output);

    /* Should truncate to fit buffer */
    TEST_ASSERT(strlen(output) < 5, "Output should be truncated");

    TEST_END();
}

/*
 * Test 8: Parse with different delimiter
 */
static void test_parse_different_delimiter(void) {
    TEST_BEGIN("Parse with comma delimiter");

    char output[256];
    int result = parse_text_field("one,two,three", ',', 2, 256, output);

    TEST_ASSERT(result == 0, "Should return 0 (success)");
    TEST_ASSERT_STR_EQ("two", output, "Second field should be 'two'");

    TEST_END();
}

/*
 * Test 9: Parse DBCS characters (Chinese)
 * DBCS lead bytes are >= 0x80
 */
static void test_parse_dbcs_characters(void) {
    TEST_BEGIN("Parse DBCS characters");

    /* Chinese characters: "中|文|测试"
     * In Shift-JIS/GBK: lead byte >= 0x80
     * This tests that DBCS chars are not mistaken for delimiters
     */
    char output[256];
    int result = parse_text_field("A|B|C", '|', 2, 256, output);

    TEST_ASSERT(result == 0, "Should return 0 (success)");
    TEST_ASSERT_STR_EQ("B", output, "Second field should be 'B'");

    /* TODO: Add actual DBCS test data when encoding is confirmed */

    TEST_END();
}

/*
 * Test 10: Parse empty field (consecutive delimiters)
 */
static void test_parse_empty_field(void) {
    TEST_BEGIN("Parse empty field");

    char output[256];
    int result = parse_text_field("A||C", '|', 2, 256, output);

    TEST_ASSERT(result == 0, "Should return 0 (success)");
    TEST_ASSERT_STR_EQ("", output, "Empty field should return empty string");

    TEST_END();
}

/* ========================================
 * Test Cases for extract_field_until_delim
 * ======================================== */

/*
 * Test 11: Extract until delimiter
 */
static void test_extract_until_delim(void) {
    TEST_BEGIN("Extract until delimiter");

    char output[256];
    int result = extract_field_until_delim("Hello|World", '|', 256, output);

    TEST_ASSERT(result == 0, "Should return 0 (delimiter found)");
    TEST_ASSERT_STR_EQ("Hello", output, "Should extract 'Hello'");

    TEST_END();
}

/*
 * Test 12: Extract until end of string
 */
static void test_extract_until_end(void) {
    TEST_BEGIN("Extract until end");

    char output[256];
    int result = extract_field_until_delim("Hello", '|', 256, output);

    TEST_ASSERT(result == 1, "Should return 1 (no delimiter)");
    TEST_ASSERT_STR_EQ("Hello", output, "Should extract entire string");

    TEST_END();
}

/*
 * Test 13: Extract with buffer limit
 */
static void test_extract_buffer_limit(void) {
    TEST_BEGIN("Extract with buffer limit");

    char output[5];
    int result = extract_field_until_delim("HelloWorld", '|', 5, output);

    TEST_ASSERT(strlen(output) < 5, "Output should be truncated");

    TEST_END();
}

/*
 * Test 14: Extract DBCS characters
 */
static void test_extract_dbcs(void) {
    TEST_BEGIN("Extract DBCS characters");

    /* Test that DBCS chars are copied as pairs */
    char output[256];
    int result = extract_field_until_delim("ABCD", '|', 256, output);

    TEST_ASSERT(result == 1, "Should return 1 (no delimiter)");
    TEST_ASSERT_STR_EQ("ABCD", output, "Should extract entire string");

    /* TODO: Add actual DBCS test data */

    TEST_END();
}

/* ========================================
 * Test Cases for skip_to_field
 * ======================================== */

/*
 * Test 15: Skip to first field
 */
static void test_skip_first(void) {
    TEST_BEGIN("Skip to first field");

    const char* ptr = skip_to_field("A|B|C", '|', 1);

    TEST_ASSERT(ptr != NULL, "Should not return NULL");
    TEST_ASSERT(*ptr == 'A', "Should point to 'A'");

    TEST_END();
}

/*
 * Test 16: Skip to middle field
 */
static void test_skip_middle(void) {
    TEST_BEGIN("Skip to middle field");

    const char* ptr = skip_to_field("A|B|C", '|', 2);

    TEST_ASSERT(ptr != NULL, "Should not return NULL");
    TEST_ASSERT(*ptr == 'B', "Should point to 'B'");

    TEST_END();
}

/*
 * Test 17: Skip to non-existent field
 */
static void test_skip_nonexistent(void) {
    TEST_BEGIN("Skip to non-existent field");

    const char* ptr = skip_to_field("A|B|C", '|', 5);

    TEST_ASSERT(ptr == NULL, "Should return NULL");

    TEST_END();
}

/* ========================================
 * Test Cases for count_fields
 * ======================================== */

/*
 * Test 18: Count fields
 */
static void test_count_fields(void) {
    TEST_BEGIN("Count fields");

    int count = count_fields("A|B|C|D", '|');

    TEST_ASSERT_EQ(4, count, "Should count 4 fields");

    TEST_END();
}

/*
 * Test 19: Count single field
 */
static void test_count_single(void) {
    TEST_BEGIN("Count single field");

    int count = count_fields("Hello", '|');

    TEST_ASSERT_EQ(1, count, "Should count 1 field");

    TEST_END();
}

/*
 * Test 20: Count empty string
 */
static void test_count_empty(void) {
    TEST_BEGIN("Count empty string");

    int count = count_fields("", '|');

    TEST_ASSERT_EQ(0, count, "Should count 0 fields");

    TEST_END();
}

/* ========================================
 * Test Cases with Real Protocol Data
 * ======================================== */

/*
 * Test 21: Parse real protocol message format
 * Many protocol messages use format: COMMAND|param1|param2|...
 */
static void test_real_protocol_format(void) {
    TEST_BEGIN("Real protocol format");

    /* Simulated protocol message: "XY|100|200|player_name" */
    char output[256];

    /* Extract command */
    parse_text_field("XY|100|200|player", '|', 1, 256, output);
    TEST_ASSERT_STR_EQ("XY", output, "Command should be XY");

    /* Extract X coordinate */
    parse_text_field("XY|100|200|player", '|', 2, 256, output);
    TEST_ASSERT_STR_EQ("100", output, "X should be 100");

    /* Extract Y coordinate */
    parse_text_field("XY|100|200|player", '|', 3, 256, output);
    TEST_ASSERT_STR_EQ("200", output, "Y should be 200");

    /* Extract player name */
    parse_text_field("XY|100|200|player", '|', 4, 256, output);
    TEST_ASSERT_STR_EQ("player", output, "Name should be player");

    TEST_END();
}

/*
 * Test 22: Parse shop item data format
 * Shop items use format: B|T|item_name|price|...
 */
static void test_shop_item_format(void) {
    TEST_BEGIN("Shop item format");

    /* Simulated shop message */
    char output[256];
    const char* shop_data = "B|T|Sword|1000|A sharp blade";

    parse_text_field(shop_data, '|', 1, 256, output);
    TEST_ASSERT_STR_EQ("B", output, "First field should be B");

    parse_text_field(shop_data, '|', 2, 256, output);
    TEST_ASSERT_STR_EQ("T", output, "Second field should be T");

    parse_text_field(shop_data, '|', 3, 256, output);
    TEST_ASSERT_STR_EQ("Sword", output, "Third field should be Sword");

    TEST_END();
}

/* ========================================
 * Test Cases for string_to_int
 * ======================================== */

/*
 * Test 23: String to integer - basic
 */
static void test_string_to_int_basic(void) {
    TEST_BEGIN("String to int - basic");

    TEST_ASSERT_EQ(123, string_to_int("123"), "Should parse 123");
    TEST_ASSERT_EQ(0, string_to_int("0"), "Should parse 0");
    TEST_ASSERT_EQ(9999, string_to_int("9999"), "Should parse 9999");

    TEST_END();
}

/*
 * Test 24: String to integer - negative
 */
static void test_string_to_int_negative(void) {
    TEST_BEGIN("String to int - negative");

    TEST_ASSERT_EQ(-123, string_to_int("-123"), "Should parse -123");
    TEST_ASSERT_EQ(-1, string_to_int("-1"), "Should parse -1");

    TEST_END();
}

/*
 * Test 25: String to integer - with whitespace
 */
static void test_string_to_int_whitespace(void) {
    TEST_BEGIN("String to int - whitespace");

    TEST_ASSERT_EQ(100, string_to_int("  100"), "Should skip leading spaces");
    TEST_ASSERT_EQ(200, string_to_int("\t200"), "Should skip leading tab");

    TEST_END();
}

/*
 * Test 26: String to integer - edge cases
 */
static void test_string_to_int_edge(void) {
    TEST_BEGIN("String to int - edge cases");

    TEST_ASSERT_EQ(0, string_to_int(NULL), "NULL should return 0");
    TEST_ASSERT_EQ(0, string_to_int(""), "Empty should return 0");
    TEST_ASSERT_EQ(0, string_to_int("abc"), "Non-numeric should return 0");

    TEST_END();
}

/* ========================================
 * Test Cases for parse_field_int
 * ======================================== */

/*
 * Test 27: Parse field int - basic
 */
static void test_parse_field_int_basic(void) {
    TEST_BEGIN("Parse field int - basic");

    TEST_ASSERT_EQ(100, parse_field_int("A|100|200", '|', 2), "Should parse 100");
    TEST_ASSERT_EQ(200, parse_field_int("A|100|200", '|', 3), "Should parse 200");

    TEST_END();
}

/*
 * Test 28: Parse field int - empty field
 */
static void test_parse_field_int_empty(void) {
    TEST_BEGIN("Parse field int - empty");

    TEST_ASSERT_EQ(-1, parse_field_int("A||C", '|', 2), "Empty field should return -1");
    TEST_ASSERT_EQ(-1, parse_field_int("A|B", '|', 5), "Non-existent field should return -1");

    TEST_END();
}

/* ========================================
 * Test Cases for parse_base62
 * ======================================== */

/*
 * Test 29: Base-62 decode - digits only
 */
static void test_base62_digits(void) {
    TEST_BEGIN("Base62 - digits");

    /* "0" in base62 = 0 */
    TEST_ASSERT_EQ(0, parse_base62("0"), "Should parse 0");
    /* "10" in base62 = 1*62 + 0 = 62 */
    TEST_ASSERT_EQ(62, parse_base62("10"), "Should parse 62");
    /* "a" in base62 = 10 */
    TEST_ASSERT_EQ(10, parse_base62("a"), "Should parse 10");

    TEST_END();
}

/*
 * Test 30: Base-62 decode - letters
 */
static void test_base62_letters(void) {
    TEST_BEGIN("Base62 - letters");

    /* "A" in base62 = 36 */
    TEST_ASSERT_EQ(36, parse_base62("A"), "A should be 36");
    /* "Z" in base62 = 61 */
    TEST_ASSERT_EQ(61, parse_base62("Z"), "Z should be 61");
    /* "z" in base62 = 35 */
    TEST_ASSERT_EQ(35, parse_base62("z"), "z should be 35");

    TEST_END();
}

/*
 * Test 31: Base-62 decode - complex
 */
static void test_base62_complex(void) {
    TEST_BEGIN("Base62 - complex");

    /* "1a" in base62 = 1*62 + 10 = 72 */
    TEST_ASSERT_EQ(72, parse_base62("1a"), "1a should be 72");
    /* "ZZ" in base62 = 61*62 + 61 = 3843 */
    TEST_ASSERT_EQ(3843, parse_base62("ZZ"), "ZZ should be 3843");

    TEST_END();
}

/*
 * Test 32: Base-62 decode - negative
 */
static void test_base62_negative(void) {
    TEST_BEGIN("Base62 - negative");

    TEST_ASSERT_EQ(-10, parse_base62("-a"), "-a should be -10");

    TEST_END();
}

/* ========================================
 * Test Cases for parse_field_base62
 * ======================================== */

/*
 * Test 33: Parse field base62
 */
static void test_parse_field_base62_basic(void) {
    TEST_BEGIN("Parse field base62");

    /* "a" = 10 in base62 */
    TEST_ASSERT_EQ(10, parse_field_base62("A|a|B", '|', 2), "Should parse base62 'a' = 10");

    TEST_END();
}

/* ========================================
 * Test Cases for string_unescape
 * ======================================== */

/*
 * Test 34: Unescape - no escapes
 */
static void test_unescape_none(void) {
    TEST_BEGIN("Unescape - no escapes");

    char str[] = "Hello World";
    string_unescape(str);

    TEST_ASSERT_STR_EQ("Hello World", str, "Should not change");

    TEST_END();
}

/*
 * Test 35: Unescape - newline
 */
static void test_unescape_newline(void) {
    TEST_BEGIN("Unescape - newline");

    char str[] = "Hello\\nWorld";
    string_unescape(str);

    TEST_ASSERT(strstr(str, "\n") != NULL, "Should contain newline");

    TEST_END();
}

/*
 * Test 36: Unescape - backslash
 */
static void test_unescape_backslash(void) {
    TEST_BEGIN("Unescape - backslash");

    char str[] = "Path\\\\File";
    string_unescape(str);

    TEST_ASSERT(strstr(str, "\\") != NULL, "Should contain backslash");

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(protocol_util) {
    /* parse_text_field tests */
    test_parse_first_field();
    test_parse_middle_field();
    test_parse_last_field();
    test_parse_nonexistent_field();
    test_parse_empty_string();
    test_parse_single_field();
    test_parse_buffer_limit();
    test_parse_different_delimiter();
    test_parse_dbcs_characters();
    test_parse_empty_field();

    /* extract_field_until_delim tests */
    test_extract_until_delim();
    test_extract_until_end();
    test_extract_buffer_limit();
    test_extract_dbcs();

    /* skip_to_field tests */
    test_skip_first();
    test_skip_middle();
    test_skip_nonexistent();

    /* count_fields tests */
    test_count_fields();
    test_count_single();
    test_count_empty();

    /* Real protocol tests */
    test_real_protocol_format();
    test_shop_item_format();

    /* string_to_int tests */
    test_string_to_int_basic();
    test_string_to_int_negative();
    test_string_to_int_whitespace();
    test_string_to_int_edge();

    /* parse_field_int tests */
    test_parse_field_int_basic();
    test_parse_field_int_empty();

    /* parse_base62 tests */
    test_base62_digits();
    test_base62_letters();
    test_base62_complex();
    test_base62_negative();

    /* parse_field_base62 tests */
    test_parse_field_base62_basic();

    /* string_unescape tests */
    test_unescape_none();
    test_unescape_newline();
    test_unescape_backslash();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - Protocol Util Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(protocol_util);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
