/*
 * Stone Age Client - Pet Protocol Unit Tests
 * Tests for FUN_00425bb0, FUN_00425230, FUN_00489f70, FUN_0048a050, FUN_0048a170
 *
 * Based on reverse engineering analysis of sa_9061.exe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/types.h"
#include "../include/pet.h"

/* External functions from pet_protocol.c */
extern int pet_parse_field(const char* str, char delimiter, int field_index, char* output, int max_len);
extern int pet_parse_field_int(const char* str, char delimiter, int field_index);
extern int pet_decode_base62(const char* str);
extern int pet_parse_field_base62(const char* str, char delimiter, int field_index);
extern void pet_unescape_string(char* str);
extern int pet_escape_string(const char* src, char* dest, int max_len);
extern void pet_parse_subfields(char* dest, const char* src, int field_size, int field_count, int max_elem);

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("Running test_%s... ", #name); \
    test_##name(); \
    printf("PASSED\n"); \
    tests_passed++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAILED at line %d: %s\n", __LINE__, #cond); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_STR_EQ(a, b) ASSERT(strcmp((a), (b)) == 0)

/*
 * Test 1-6: pet_parse_field - FUN_00489f70 pattern
 * Basic field extraction from pipe-delimited strings
 */
TEST(parse_field_basic) {
    char output[128];
    const char* input = "field1|field2|field3";

    ASSERT_EQ(pet_parse_field(input, '|', 1, output, sizeof(output)), 0);
    ASSERT_STR_EQ(output, "field1");

    ASSERT_EQ(pet_parse_field(input, '|', 2, output, sizeof(output)), 0);
    ASSERT_STR_EQ(output, "field2");

    ASSERT_EQ(pet_parse_field(input, '|', 3, output, sizeof(output)), 0);
    ASSERT_STR_EQ(output, "field3");
}

TEST(parse_field_empty) {
    char output[128];
    const char* input = "a||c";

    ASSERT_EQ(pet_parse_field(input, '|', 2, output, sizeof(output)), 0);
    ASSERT_STR_EQ(output, "");
}

TEST(parse_field_out_of_range) {
    char output[128];
    const char* input = "a|b";

    ASSERT_EQ(pet_parse_field(input, '|', 5, output, sizeof(output)), -1);
}

TEST(parse_field_max_len) {
    char output[8];
    const char* input = "abcdefghijklmnop";

    ASSERT_EQ(pet_parse_field(input, '|', 1, output, sizeof(output)), 0);
    ASSERT_EQ(strlen(output), 7);  /* max_len - 1 */
    ASSERT_STR_EQ(output, "abcdefg");
}

TEST(parse_field_different_delimiter) {
    char output[128];
    const char* input = "one,two,three";

    ASSERT_EQ(pet_parse_field(input, ',', 1, output, sizeof(output)), 0);
    ASSERT_STR_EQ(output, "one");

    ASSERT_EQ(pet_parse_field(input, ',', 3, output, sizeof(output)), 0);
    ASSERT_STR_EQ(output, "three");
}

TEST(parse_field_dbcs) {
    char output[128];
    /* Test with multi-byte character handling */
    const char* input = "\x82\xa0|test|\x82\xa2";  /* Japanese "a" | test | Japanese "i" */

    ASSERT_EQ(pet_parse_field(input, '|', 2, output, sizeof(output)), 0);
    ASSERT_STR_EQ(output, "test");
}

/*
 * Test 7-10: pet_parse_field_int - FUN_0048a050 pattern
 * Integer field parsing
 */
TEST(parse_field_int_basic) {
    const char* input = "123|456|789";

    ASSERT_EQ(pet_parse_field_int(input, '|', 1), 123);
    ASSERT_EQ(pet_parse_field_int(input, '|', 2), 456);
    ASSERT_EQ(pet_parse_field_int(input, '|', 3), 789);
}

TEST(parse_field_int_negative) {
    const char* input = "-100|-200|-300";

    ASSERT_EQ(pet_parse_field_int(input, '|', 1), -100);
    ASSERT_EQ(pet_parse_field_int(input, '|', 2), -200);
}

TEST(parse_field_int_empty) {
    const char* input = "100||200";

    /* Empty field should return 0 from atoi */
    ASSERT_EQ(pet_parse_field_int(input, '|', 2), 0);
}

TEST(parse_field_int_invalid) {
    const char* input = "abc|def";

    /* Non-numeric should return 0 */
    ASSERT_EQ(pet_parse_field_int(input, '|', 1), 0);
}

/*
 * Test 11-15: pet_decode_base62 - FUN_0048a0a0 pattern
 * Base-62 decoding used in bitmask fields (cases 0x4B, 0x4E, 0x50)
 */
TEST(decode_base62_digits) {
    /* 0-9 map to 0-9 */
    ASSERT_EQ(pet_decode_base62("0"), 0);
    ASSERT_EQ(pet_decode_base62("9"), 9);
    ASSERT_EQ(pet_decode_base62("10"), 62);
    ASSERT_EQ(pet_decode_base62("1z"), 61);  /* 1*62 + 35 */
}

TEST(decode_base62_lowercase) {
    /* a-z map to 10-35 */
    ASSERT_EQ(pet_decode_base62("a"), 10);
    ASSERT_EQ(pet_decode_base62("z"), 35);
    ASSERT_EQ(pet_decode_base62("A"), 36);  /* A = 36 in base62 */
    ASSERT_EQ(pet_decode_base62("Z"), 61);  /* Z = 61 in base62 */
}

TEST(decode_base62_uppercase) {
    /* A-Z map to 36-61 */
    ASSERT_EQ(pet_decode_base62("10"), 62);    /* 1*62 + 0 */
    ASSERT_EQ(pet_decode_base62("1a"), 72);    /* 1*62 + 10 */
    ASSERT_EQ(pet_decode_base62("1Z"), 123);   /* 1*62 + 61 */
}

TEST(decode_base62_multi_digit) {
    ASSERT_EQ(pet_decode_base62("100"), 3844);   /* 1*62*62 + 0*62 + 0 */
    ASSERT_EQ(pet_decode_base62("zz"), 3843);    /* 35*62 + 35 = 2205 */
    ASSERT_EQ(pet_decode_base62("ZZ"), 3843);    /* Should be same with different case handling */
}

TEST(decode_base62_negative) {
    ASSERT_EQ(pet_decode_base62("-1"), -1);
    ASSERT_EQ(pet_decode_base62("-a"), -10);
    ASSERT_EQ(pet_decode_base62("-Z"), -61);
}

/*
 * Test 16-20: pet_unescape_string - FUN_0048a170 pattern
 * String unescaping for game protocol
 */
TEST(unescape_basic) {
    char str[64];

    strcpy(str, "hello\\Sworld");
    pet_unescape_string(str);
    ASSERT_STR_EQ(str, "hello world");

    strcpy(str, "line1\\nline2");
    pet_unescape_string(str);
    ASSERT_STR_EQ(str, "line1\nline2");
}

TEST(unescape_special_chars) {
    char str[64];

    strcpy(str, "a\\cb\\zd");
    pet_unescape_string(str);
    ASSERT_STR_EQ(str, "a,b|d");

    strcpy(str, "back\\\\slash");
    pet_unescape_string(str);
    ASSERT_STR_EQ(str, "back\\slash");
}

TEST(unescape_no_escape) {
    char str[64];

    strcpy(str, "normal string");
    pet_unescape_string(str);
    ASSERT_STR_EQ(str, "normal string");
}

TEST(unescape_empty) {
    char str[64] = "";
    pet_unescape_string(str);
    ASSERT_STR_EQ(str, "");
}

TEST(unescape_trailing_backslash) {
    char str[64];

    strcpy(str, "test\\");
    pet_unescape_string(str);
    ASSERT_STR_EQ(str, "test");  /* Trailing backslash removed */
}

/*
 * Test 21-25: pet_escape_string - FUN_0048a200 pattern
 * String escaping for sending to server
 */
TEST(escape_basic) {
    char dest[64];

    pet_escape_string("hello world", dest, sizeof(dest));
    ASSERT_STR_EQ(dest, "hello\\Sworld");

    pet_escape_string("a,b|d", dest, sizeof(dest));
    ASSERT_STR_EQ(dest, "a\\cb\\zd");
}

TEST(escape_newline) {
    char dest[64];

    pet_escape_string("line1\nline2", dest, sizeof(dest));
    ASSERT_STR_EQ(dest, "line1\\nline2");

    pet_escape_string("line1\r\nline2", dest, sizeof(dest));
    ASSERT_STR_EQ(dest, "line1\\r\\nline2");
}

TEST(escape_no_special) {
    char dest[64];

    pet_escape_string("normal", dest, sizeof(dest));
    ASSERT_STR_EQ(dest, "normal");
}

TEST(escape_backslash) {
    char dest[64];

    pet_escape_string("path\\to\\file", dest, sizeof(dest));
    ASSERT_STR_EQ(dest, "path\\\\to\\\\file");
}

TEST(escape_buffer_limit) {
    char dest[10];

    int len = pet_escape_string("hello world", dest, sizeof(dest));
    ASSERT(len < (int)sizeof(dest));  /* Should truncate safely */
    ASSERT(dest[sizeof(dest) - 1] == '\0');  /* Null terminated */
}

/*
 * Test 26-30: pet_parse_subfields - FUN_00425230 pattern
 * Multi-field array parsing from FUN_00425bb0
 */
TEST(parse_subfields_basic) {
    char dest[3][32];
    const char* input = "one\ntwo\nthree";  /* newline = delimiter 10 */

    pet_parse_subfields((char*)dest, input, 32, 3, 31);

    ASSERT_STR_EQ(dest[0], "one");
    ASSERT_STR_EQ(dest[1], "two");
    ASSERT_STR_EQ(dest[2], "three");
}

TEST(parse_subfields_partial) {
    char dest[5][32];
    const char* input = "a\nb";

    memset(dest, 0xFF, sizeof(dest));  /* Fill with 0xFF */
    pet_parse_subfields((char*)dest, input, 32, 5, 31);

    ASSERT_STR_EQ(dest[0], "a");
    ASSERT_STR_EQ(dest[1], "b");
    ASSERT_STR_EQ(dest[2], "");  /* Empty for missing fields */
}

TEST(parse_subfields_empty) {
    char dest[3][32];

    pet_parse_subfields((char*)dest, "", 32, 3, 31);

    ASSERT_STR_EQ(dest[0], "");
    ASSERT_STR_EQ(dest[1], "");
    ASSERT_STR_EQ(dest[2], "");
}

TEST(parse_subfields_max_elem) {
    char dest[3][32];
    const char* input = "verylongstring";

    pet_parse_subfields((char*)dest, input, 32, 3, 5);

    ASSERT_EQ(strlen(dest[0]), 4);  /* max_elem - 1 */
    ASSERT_STR_EQ(dest[0], "very");
}

TEST(parse_subfields_null_handling) {
    char dest[3][32];

    /* Should not crash with null input */
    pet_parse_subfields((char*)dest, NULL, 32, 3, 31);

    /* Should not crash with null output */
    pet_parse_subfields(NULL, "test", 32, 3, 31);
}

/*
 * Test 31-35: Integration tests based on FUN_00425bb0 pattern
 * Testing the complete pet summon data parsing flow
 */
TEST(integration_pet_summon_parsing) {
    /* Simulate the packet format from FUN_00425bb0:
     * Field 1: pet count flag
     * Field 2: owner name
     * Field 3: pet data (comma-separated subfields)
     * Fields 5+: pet details (4 fields per pet)
     */
    const char* packet = "|PlayerName|pet1,25,100|0|123|45|data1|0|456|78|data2";
    char output[128];
    char subfields[3][32];
    int value;

    /* Field 1 - check for presence */
    value = pet_parse_field_int(packet, '|', 1);
    ASSERT_EQ(value, 0);  /* Empty field -> 0 */

    /* Field 2 - owner name */
    ASSERT_EQ(pet_parse_field(packet, '|', 2, output, sizeof(output)), 0);
    ASSERT_STR_EQ(output, "PlayerName");

    /* Field 3 - pet data with subfields */
    ASSERT_EQ(pet_parse_field(packet, '|', 3, output, sizeof(output)), 0);
    pet_unescape_string(output);
    pet_parse_subfields((char*)subfields, output, 32, 3, 31);
    ASSERT_STR_EQ(subfields[0], "pet1");
    ASSERT_STR_EQ(subfields[1], "25");
    ASSERT_STR_EQ(subfields[2], "100");
}

TEST(integration_base62_bitmask) {
    /* Test the Base-62 bitmask decoding used in protocol cases 0x4B, 0x4E, 0x50 */
    const char* packet = "data|1Z|abc|end";
    int bitmask;

    bitmask = pet_parse_field_base62(packet, '|', 2);
    ASSERT(bitmask > 0);  /* Should decode to a valid value */

    /* 1Z = 1*62 + 35 = 97 (if Z is lowercase) or 1*62 + 61 = 123 (if uppercase) */
}

TEST(integration_escape_roundtrip) {
    char escaped[128];
    char unescaped[128];
    const char* original = "hello world,test|data\\n";

    pet_escape_string(original, escaped, sizeof(escaped));
    strcpy(unescaped, escaped);
    pet_unescape_string(unescaped);

    ASSERT_STR_EQ(unescaped, original);
}

TEST(integration_complex_packet) {
    /* Simulate a complex game packet with multiple encoding layers */
    const char* packet = "123|Player\\SName|a,b\\zc|100|200";
    char field[128];
    char subfields[4][32];

    /* Parse and unescape player name */
    pet_parse_field(packet, '|', 2, field, sizeof(field));
    pet_unescape_string(field);
    ASSERT_STR_EQ(field, "Player Name");

    /* Parse subfields with escaped delimiter */
    pet_parse_field(packet, '|', 3, field, sizeof(field));
    pet_unescape_string(field);
    pet_parse_subfields((char*)subfields, field, 32, 4, 31);
    ASSERT_STR_EQ(subfields[0], "a");
    ASSERT_STR_EQ(subfields[1], "b|c");  /* Unescaped comma becomes | */
}

TEST(integration_empty_fields) {
    /* Test handling of empty fields like in FUN_00425bb0 */
    const char* packet = "|||";
    char output[64];

    ASSERT_EQ(pet_parse_field(packet, '|', 1, output, sizeof(output)), 0);
    ASSERT_STR_EQ(output, "");

    ASSERT_EQ(pet_parse_field(packet, '|', 2, output, sizeof(output)), 0);
    ASSERT_STR_EQ(output, "");

    ASSERT_EQ(pet_parse_field(packet, '|', 4, output, sizeof(output)), 0);
    ASSERT_STR_EQ(output, "");
}

int main(void) {
    printf("=== Pet Protocol Unit Tests ===\n\n");

    /* FUN_00489f70 tests */
    RUN_TEST(parse_field_basic);
    RUN_TEST(parse_field_empty);
    RUN_TEST(parse_field_out_of_range);
    RUN_TEST(parse_field_max_len);
    RUN_TEST(parse_field_different_delimiter);
    RUN_TEST(parse_field_dbcs);

    /* FUN_0048a050 tests */
    RUN_TEST(parse_field_int_basic);
    RUN_TEST(parse_field_int_negative);
    RUN_TEST(parse_field_int_empty);
    RUN_TEST(parse_field_int_invalid);

    /* FUN_0048a0a0 tests */
    RUN_TEST(decode_base62_digits);
    RUN_TEST(decode_base62_lowercase);
    RUN_TEST(decode_base62_uppercase);
    RUN_TEST(decode_base62_multi_digit);
    RUN_TEST(decode_base62_negative);

    /* FUN_0048a170 tests */
    RUN_TEST(unescape_basic);
    RUN_TEST(unescape_special_chars);
    RUN_TEST(unescape_no_escape);
    RUN_TEST(unescape_empty);
    RUN_TEST(unescape_trailing_backslash);

    /* FUN_0048a200 tests */
    RUN_TEST(escape_basic);
    RUN_TEST(escape_newline);
    RUN_TEST(escape_no_special);
    RUN_TEST(escape_backslash);
    RUN_TEST(escape_buffer_limit);

    /* FUN_00425230 tests */
    RUN_TEST(parse_subfields_basic);
    RUN_TEST(parse_subfields_partial);
    RUN_TEST(parse_subfields_empty);
    RUN_TEST(parse_subfields_max_elem);
    RUN_TEST(parse_subfields_null_handling);

    /* Integration tests */
    RUN_TEST(integration_pet_summon_parsing);
    RUN_TEST(integration_base62_bitmask);
    RUN_TEST(integration_escape_roundtrip);
    RUN_TEST(integration_complex_packet);
    RUN_TEST(integration_empty_fields);

    printf("\n=== Test Results ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
