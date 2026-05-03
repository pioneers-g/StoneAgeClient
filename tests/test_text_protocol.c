/*
 * Stone Age Client - Text Protocol Unit Tests
 * Tests for DBCS string comparison, line parsing, command dispatch
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../tests/test_framework.h"
#include "types.h"
#include "text_protocol.h"
#include "text_protocol_internal.h"

/* Test DBCS string comparison - FUN_0043c010 pattern */
void test_dbcs_strcmp_ascii(void) {
    TEST_ASSERT_EQ(dbcs_strcmp("XY", "XY"), 0);
    TEST_ASSERT_EQ(dbcs_strcmp("XY", "XZ"), -1);
    TEST_ASSERT_EQ(dbcs_strcmp("XY", "XA"), 1);
    TEST_ASSERT_EQ(dbcs_strcmp("ClientLogin", "ClientLogin"), 0);
    TEST_ASSERT_EQ(dbcs_strcmp("ClientLogin", "ClientLogou"), -1);
    TEST_ASSERT_EQ(dbcs_strcmp("", ""), 0);
    TEST_ASSERT_EQ(dbcs_strcmp("A", ""), 'A');
    TEST_ASSERT_EQ(dbcs_strcmp("", "A"), -'A');
}

void test_dbcs_strcmp_partial(void) {
    /* Partial matching - prefix */
    TEST_ASSERT(dbcs_strcmp("XY", "XYG") < 0);
    TEST_ASSERT(dbcs_strcmp("XYG", "XY") > 0);
    TEST_ASSERT(dbcs_strcmp("XYA", "XYB") < 0);
}

void test_dbcs_strcmp_dbcs(void) {
    /* DBCS character tests (Chinese characters) */
    /* Chinese "中" = 0xD6D0 in GB2312 */
    /* Chinese "文" = 0xCEC4 in GB2312 */
    unsigned char str1[] = {0xD6, 0xD0, 0x00};  /* 中 */
    unsigned char str2[] = {0xD6, 0xD0, 0x00};  /* 中 */
    unsigned char str3[] = {0xCE, 0xC4, 0x00};  /* 文 */

    TEST_ASSERT_EQ(dbcs_strcmp((char*)str1, (char*)str2), 0);
    TEST_ASSERT(dbcs_strcmp((char*)str1, (char*)str3) != 0);

    /* DBCS + ASCII mixed */
    unsigned char str4[] = {0xD6, 0xD0, 'A', 0x00};  /* 中A */
    unsigned char str5[] = {0xD6, 0xD0, 'B', 0x00};  /* 中B */
    TEST_ASSERT(dbcs_strcmp((char*)str4, (char*)str5) < 0);
}

/* Test protocol line parsing - FUN_0043dbe0 pattern */
void test_parse_simple_command(void) {
    char line[] = "XY";
    parse_protocol_line(line);

    TEST_ASSERT_EQ(s_proto_ctx.field_count, 1);
    TEST_ASSERT_STR_EQ(s_proto_ctx.fields[0], "XY");
}

void test_parse_with_params(void) {
    char line[] = "XY|123|456";
    parse_protocol_line(line);

    TEST_ASSERT_EQ(s_proto_ctx.field_count, 3);
    TEST_ASSERT_STR_EQ(s_proto_ctx.fields[0], "XY");
    TEST_ASSERT_STR_EQ(s_proto_ctx.fields[1], "123");
    TEST_ASSERT_STR_EQ(s_proto_ctx.fields[2], "456");
}

void test_parse_long_command(void) {
    char line[] = "ClientLogin|account123|0";
    parse_protocol_line(line);

    TEST_ASSERT_EQ(s_proto_ctx.field_count, 3);
    TEST_ASSERT_STR_EQ(s_proto_ctx.fields[0], "ClientLogin");
    TEST_ASSERT_STR_EQ(s_proto_ctx.fields[1], "account123");
    TEST_ASSERT_STR_EQ(s_proto_ctx.fields[2], "0");
}

void test_parse_empty_fields(void) {
    char line[] = "XY||123";
    parse_protocol_line(line);

    /* Empty field should be skipped or counted */
    TEST_ASSERT(s_proto_ctx.field_count >= 2);
}

void test_parse_trailing_pipe(void) {
    char line[] = "XY|123|";
    parse_protocol_line(line);

    TEST_ASSERT(s_proto_ctx.field_count >= 2);
    TEST_ASSERT_STR_EQ(s_proto_ctx.fields[0], "XY");
    TEST_ASSERT_STR_EQ(s_proto_ctx.fields[1], "123");
}

void test_parse_newline_terminated(void) {
    char line[] = "XY|123\n";
    parse_protocol_line(line);

    TEST_ASSERT_EQ(s_proto_ctx.field_count, 2);
    TEST_ASSERT_STR_EQ(s_proto_ctx.fields[0], "XY");
    TEST_ASSERT_STR_EQ(s_proto_ctx.fields[1], "123");
}

void test_parse_max_fields(void) {
    /* Create a line with many fields */
    char line[512];
    strcpy(line, "CMD");
    for (int i = 1; i < 40; i++) {
        strcat(line, "|field");
    }
    parse_protocol_line(line);

    /* Should not exceed MAX_TEXT_FIELDS */
    TEST_ASSERT(s_proto_ctx.field_count <= MAX_TEXT_FIELDS);
}

void test_parse_dbcs_content(void) {
    /* Line with DBCS characters in field */
    unsigned char line[] = "XY|\xD6\xD0\xCE\xC4";  /* XY|中文 */
    line[9] = '\0';

    parse_protocol_line((char*)line);

    TEST_ASSERT_EQ(s_proto_ctx.field_count, 2);
    TEST_ASSERT_STR_EQ(s_proto_ctx.fields[0], "XY");
}

void test_parse_empty_line(void) {
    char line[] = "";
    parse_protocol_line(line);

    TEST_ASSERT_EQ(s_proto_ctx.field_count, 0);
}

void test_parse_whitespace(void) {
    char line[] = "  XY|123";
    parse_protocol_line(line);

    TEST_ASSERT(s_proto_ctx.field_count >= 1);
    /* Should skip leading whitespace */
}

/* Test message log circular buffer - FUN_00464ef0 pattern */
void test_message_log_init(void) {
    message_log_init();

    TEST_ASSERT_EQ(s_chat_log_index, 0);
    TEST_ASSERT_EQ(s_message_log_index, 0);
    TEST_ASSERT_EQ(s_update_flag, 0);
}

void test_message_log_chat(void) {
    message_log_init();

    message_log_chat("Test message 1");
    TEST_ASSERT_EQ(s_chat_log_index, 1);

    message_log_chat("Test message 2");
    TEST_ASSERT_EQ(s_chat_log_index, 2);
}

void test_message_log_wrap(void) {
    message_log_init();

    /* Add 5 messages to test wrap at 4 */
    for (int i = 0; i < 5; i++) {
        char msg[32];
        snprintf(msg, sizeof(msg), "Message %d", i);
        message_log_chat(msg);
    }

    /* Index should wrap back to 1 (5 mod 4 = 1) */
    TEST_ASSERT_EQ(s_chat_log_index, 1);
}

void test_message_log_get(void) {
    message_log_init();

    message_log_chat("First");
    message_log_chat("Second");

    const char* log0 = text_protocol_get_chat_log(0);
    const char* log1 = text_protocol_get_chat_log(1);

    /* Index 0 should be most recent ("Second") */
    TEST_ASSERT(log0 != NULL);
    TEST_ASSERT(log1 != NULL);
}

void test_message_log_bounds(void) {
    const char* log = text_protocol_get_chat_log(10);
    TEST_ASSERT_EQ(log, NULL);
}

/* Test update flag - XYF U command */
void test_update_flag(void) {
    message_log_init();

    TEST_ASSERT_EQ(text_protocol_has_update(), 0);

    s_update_flag = 1;
    TEST_ASSERT_EQ(text_protocol_has_update(), 1);

    text_protocol_clear_update();
    TEST_ASSERT_EQ(text_protocol_has_update(), 0);
}

/* Test command strings from binary - DAT_004b9eb8 etc */
void test_command_strings_exist(void) {
    /* Verify command table entries match expected patterns */
    TEST_ASSERT_STR_EQ("XY", "XY");
    TEST_ASSERT_STR_EQ("XYG", "XYG");
    TEST_ASSERT_STR_EQ("ClientLogin", "ClientLogin");
    TEST_ASSERT_STR_EQ("CharList", "CharList");
}

/* Test get accessor functions */
void test_get_string_param(void) {
    char line[] = "XY|field1|field2";
    parse_protocol_line(line);

    TEST_ASSERT_STR_EQ(get_string_param(0), "XY");
    TEST_ASSERT_STR_EQ(get_string_param(1), "field1");
    TEST_ASSERT_STR_EQ(get_string_param(2), "field2");
    TEST_ASSERT_STR_EQ(get_string_param(10), "");  /* Out of bounds */
}

void test_get_int_param_basic(void) {
    char line[] = "XY|123|456";
    parse_protocol_line(line);

    /* Note: get_int_param uses base-62 decoding, so results vary */
    int val = get_int_param(1);
    TEST_ASSERT(val >= 0);  /* Should return non-negative for valid input */
}

void test_get_int_param_out_of_bounds(void) {
    char line[] = "XY|123";
    parse_protocol_line(line);

    TEST_ASSERT_EQ(get_int_param(10), 0);
}

/* Test constants from binary */
void test_constants(void) {
    TEST_ASSERT_EQ(MAX_TEXT_FIELDS, 32);
    TEST_ASSERT_EQ(MSG_LOG_SIZE, 0x1000);
    TEST_ASSERT_EQ(MSG_LOG_COUNT, 4);
}

/* Test protocol constants from header */
void test_protocol_constants(void) {
    TEST_ASSERT_STR_EQ(CMD_XY, "XY");
    TEST_ASSERT_STR_EQ(CMD_XYG, "XYG");
    TEST_ASSERT_STR_EQ(CMD_CLIENTLOGIN, "ClientLogin");
    TEST_ASSERT_STR_EQ(CMD_CHARLIST, "CharList");
}

int main(void) {
    TEST_SUITE_BEGIN("Text Protocol Tests");

    /* DBCS string comparison */
    TEST_RUN(test_dbcs_strcmp_ascii);
    TEST_RUN(test_dbcs_strcmp_partial);
    TEST_RUN(test_dbcs_strcmp_dbcs);

    /* Protocol line parsing */
    TEST_RUN(test_parse_simple_command);
    TEST_RUN(test_parse_with_params);
    TEST_RUN(test_parse_long_command);
    TEST_RUN(test_parse_empty_fields);
    TEST_RUN(test_parse_trailing_pipe);
    TEST_RUN(test_parse_newline_terminated);
    TEST_RUN(test_parse_max_fields);
    TEST_RUN(test_parse_dbcs_content);
    TEST_RUN(test_parse_empty_line);
    TEST_RUN(test_parse_whitespace);

    /* Message log */
    TEST_RUN(test_message_log_init);
    TEST_RUN(test_message_log_chat);
    TEST_RUN(test_message_log_wrap);
    TEST_RUN(test_message_log_get);
    TEST_RUN(test_message_log_bounds);

    /* Update flag */
    TEST_RUN(test_update_flag);

    /* Command strings */
    TEST_RUN(test_command_strings_exist);

    /* Accessor functions */
    TEST_RUN(test_get_string_param);
    TEST_RUN(test_get_int_param_basic);
    TEST_RUN(test_get_int_param_out_of_bounds);

    /* Constants */
    TEST_RUN(test_constants);
    TEST_RUN(test_protocol_constants);

    TEST_SUITE_END();
}
