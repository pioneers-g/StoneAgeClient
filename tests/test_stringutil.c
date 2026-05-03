/*
 * Stone Age Client - Unit Tests for String Utility Functions
 * Test file: test_stringutil.c
 *
 * Tests FUN_0044a940 (string_truncate) and related functions
 *
 * String truncation with ellipsis is used extensively in UI rendering
 * to fit long strings into fixed-width display areas.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "test_framework.h"
#include "../include/types.h"

/* External function from stringutil.c */
extern void string_truncate(const char* src, char* dest, size_t max_len);
extern void string_truncate_display(const char* src, char* dest, size_t max_len);
extern void string_copy_dbcs(const char* src, char* dest, size_t max_len);
extern size_t string_len_dbcs(const char* str);
extern int string_is_valid_name(const char* str);
extern void string_to_lower(char* str);
extern void string_trim(char* str);
extern void string_format_number(char* dest, size_t max_len, int value);
extern int string_compare_dbcs(const char* s1, const char* s2);

/* ========================================
 * Test Cases for string_truncate
 * ======================================== */

/*
 * Test 1: String fits within limit
 */
static void test_truncate_fits(void) {
    TEST_BEGIN("Truncate - string fits");

    char output[32];
    string_truncate("Hello", output, 10);

    TEST_ASSERT_STR_EQ("Hello", output, "Should return original string");

    TEST_END();
}

/*
 * Test 2: String needs truncation
 */
static void test_truncate_needs_truncation(void) {
    TEST_BEGIN("Truncate - needs truncation");

    char output[10];
    string_truncate("HelloWorld12345", output, 9);

    /* Should truncate and add ellipsis */
    TEST_ASSERT(strlen(output) <= 9, "Output should fit in buffer");

    TEST_END();
}

/*
 * Test 3: Empty string
 */
static void test_truncate_empty(void) {
    TEST_BEGIN("Truncate - empty string");

    char output[32];
    string_truncate("", output, 10);

    TEST_ASSERT_STR_EQ("", output, "Empty string should remain empty");

    TEST_END();
}

/*
 * Test 4: NULL inputs
 */
static void test_truncate_null(void) {
    TEST_BEGIN("Truncate - NULL inputs");

    char output[32];

    /* Should handle gracefully */
    string_truncate(NULL, output, 10);
    TEST_ASSERT_STR_EQ("", output, "NULL src should give empty output");

    string_truncate("Hello", NULL, 10);  /* Should not crash */

    TEST_END();
}

/*
 * Test 5: Very short buffer
 */
static void test_truncate_short_buffer(void) {
    TEST_BEGIN("Truncate - short buffer");

    char output[4];
    string_truncate("HelloWorld", output, 3);

    TEST_ASSERT(strlen(output) <= 3, "Should fit in 3 chars");

    TEST_END();
}

/* ========================================
 * Test Cases for string_copy_dbcs
 * ======================================== */

/*
 * Test 6: Copy ASCII string
 */
static void test_copy_dbcs_ascii(void) {
    TEST_BEGIN("Copy DBCS - ASCII");

    char output[32];
    string_copy_dbcs("Hello", output, 32);

    TEST_ASSERT_STR_EQ("Hello", output, "Should copy ASCII string");

    TEST_END();
}

/*
 * Test 7: Copy with buffer limit
 */
static void test_copy_dbcs_limit(void) {
    TEST_BEGIN("Copy DBCS - buffer limit");

    char output[5];
    string_copy_dbcs("HelloWorld", output, 5);

    TEST_ASSERT(strlen(output) < 5, "Should truncate to fit buffer");

    TEST_END();
}

/*
 * Test 8: Copy empty string
 */
static void test_copy_dbcs_empty(void) {
    TEST_BEGIN("Copy DBCS - empty");

    char output[32];
    string_copy_dbcs("", output, 32);

    TEST_ASSERT_STR_EQ("", output, "Empty should remain empty");

    TEST_END();
}

/* ========================================
 * Test Cases for string_len_dbcs
 * ======================================== */

/*
 * Test 9: Length of ASCII string
 */
static void test_len_dbcs_ascii(void) {
    TEST_BEGIN("Len DBCS - ASCII");

    size_t len = string_len_dbcs("Hello");

    TEST_ASSERT_EQ(5, len, "ASCII length should be 5");

    TEST_END();
}

/*
 * Test 10: Length of empty string
 */
static void test_len_dbcs_empty(void) {
    TEST_BEGIN("Len DBCS - empty");

    size_t len = string_len_dbcs("");

    TEST_ASSERT_EQ(0, len, "Empty string length should be 0");

    TEST_END();
}

/*
 * Test 11: Length of NULL
 */
static void test_len_dbcs_null(void) {
    TEST_BEGIN("Len DBCS - NULL");

    size_t len = string_len_dbcs(NULL);

    TEST_ASSERT_EQ(0, len, "NULL length should be 0");

    TEST_END();
}

/* ========================================
 * Test Cases for string_is_valid_name
 * ======================================== */

/*
 * Test 12: Valid ASCII name
 */
static void test_valid_name_ascii(void) {
    TEST_BEGIN("Valid name - ASCII");

    TEST_ASSERT(string_is_valid_name("Player1") == 1, "Should be valid");
    TEST_ASSERT(string_is_valid_name("Test_Name") == 1, "Should be valid");

    TEST_END();
}

/*
 * Test 13: Invalid characters
 */
static void test_valid_name_invalid(void) {
    TEST_BEGIN("Valid name - invalid chars");

    TEST_ASSERT(string_is_valid_name("") == 0, "Empty should be invalid");
    TEST_ASSERT(string_is_valid_name(NULL) == 0, "NULL should be invalid");

    TEST_END();
}

/* ========================================
 * Test Cases for string_to_lower
 * ======================================== */

/*
 * Test 14: Convert to lowercase
 */
static void test_to_lower(void) {
    TEST_BEGIN("To lower");

    char str[] = "HELLO World";
    string_to_lower(str);

    TEST_ASSERT_STR_EQ("hello world", str, "Should be lowercase");

    TEST_END();
}

/*
 * Test 15: Lowercase with NULL
 */
static void test_to_lower_null(void) {
    TEST_BEGIN("To lower - NULL");

    /* Should not crash */
    string_to_lower(NULL);

    TEST_END();
}

/* ========================================
 * Test Cases for string_trim
 * ======================================== */

/*
 * Test 16: Trim leading spaces
 */
static void test_trim_leading(void) {
    TEST_BEGIN("Trim - leading");

    char str[] = "   Hello";
    string_trim(str);

    TEST_ASSERT_STR_EQ("Hello", str, "Should trim leading spaces");

    TEST_END();
}

/*
 * Test 17: Trim trailing spaces
 */
static void test_trim_trailing(void) {
    TEST_BEGIN("Trim - trailing");

    char str[] = "Hello   ";
    string_trim(str);

    TEST_ASSERT_STR_EQ("Hello", str, "Should trim trailing spaces");

    TEST_END();
}

/*
 * Test 18: Trim both
 */
static void test_trim_both(void) {
    TEST_BEGIN("Trim - both");

    char str[] = "   Hello   ";
    string_trim(str);

    TEST_ASSERT_STR_EQ("Hello", str, "Should trim both sides");

    TEST_END();
}

/*
 * Test 19: Trim all spaces
 */
static void test_trim_all(void) {
    TEST_BEGIN("Trim - all spaces");

    char str[] = "     ";
    string_trim(str);

    TEST_ASSERT_STR_EQ("", str, "All spaces should become empty");

    TEST_END();
}

/* ========================================
 * Test Cases for string_format_number
 * ======================================== */

/*
 * Test 20: Format small number
 */
static void test_format_number_small(void) {
    TEST_BEGIN("Format number - small");

    char output[32];
    string_format_number(output, 32, 123);

    TEST_ASSERT_STR_EQ("123", output, "Should format 123");

    TEST_END();
}

/*
 * Test 21: Format with commas
 */
static void test_format_number_commas(void) {
    TEST_BEGIN("Format number - commas");

    char output[32];
    string_format_number(output, 32, 1234567);

    TEST_ASSERT_STR_EQ("1,234,567", output, "Should add commas");

    TEST_END();
}

/*
 * Test 22: Format negative number
 */
static void test_format_number_negative(void) {
    TEST_BEGIN("Format number - negative");

    char output[32];
    string_format_number(output, 32, -1234);

    /* Should handle negative */
    TEST_ASSERT(strstr(output, "-") != NULL || strstr(output, "1234") != NULL,
                "Should handle negative");

    TEST_END();
}

/*
 * Test 23: Format zero
 */
static void test_format_number_zero(void) {
    TEST_BEGIN("Format number - zero");

    char output[32];
    string_format_number(output, 32, 0);

    TEST_ASSERT_STR_EQ("0", output, "Should format 0");

    TEST_END();
}

/* ========================================
 * Test Cases for string_compare_dbcs
 * ======================================== */

/*
 * Test 24: Compare equal strings
 */
static void test_compare_equal(void) {
    TEST_BEGIN("Compare - equal");

    TEST_ASSERT(string_compare_dbcs("Hello", "Hello") == 0, "Should be equal");

    TEST_END();
}

/*
 * Test 25: Compare different strings
 */
static void test_compare_different(void) {
    TEST_BEGIN("Compare - different");

    TEST_ASSERT(string_compare_dbcs("Hello", "World") != 0, "Should be different");
    TEST_ASSERT(string_compare_dbcs("AAA", "AAB") < 0, "AAA < AAB");

    TEST_END();
}

/*
 * Test 26: Compare with NULL
 */
static void test_compare_null(void) {
    TEST_BEGIN("Compare - NULL");

    TEST_ASSERT(string_compare_dbcs(NULL, NULL) == 0, "NULL == NULL");
    TEST_ASSERT(string_compare_dbcs("Hello", NULL) > 0, "String > NULL");
    TEST_ASSERT(string_compare_dbcs(NULL, "Hello") < 0, "NULL < String");

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(stringutil) {
    /* string_truncate tests */
    test_truncate_fits();
    test_truncate_needs_truncation();
    test_truncate_empty();
    test_truncate_null();
    test_truncate_short_buffer();

    /* string_copy_dbcs tests */
    test_copy_dbcs_ascii();
    test_copy_dbcs_limit();
    test_copy_dbcs_empty();

    /* string_len_dbcs tests */
    test_len_dbcs_ascii();
    test_len_dbcs_empty();
    test_len_dbcs_null();

    /* string_is_valid_name tests */
    test_valid_name_ascii();
    test_valid_name_invalid();

    /* string_to_lower tests */
    test_to_lower();
    test_to_lower_null();

    /* string_trim tests */
    test_trim_leading();
    test_trim_trailing();
    test_trim_both();
    test_trim_all();

    /* string_format_number tests */
    test_format_number_small();
    test_format_number_commas();
    test_format_number_negative();
    test_format_number_zero();

    /* string_compare_dbcs tests */
    test_compare_equal();
    test_compare_different();
    test_compare_null();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - String Util Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(stringutil);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
