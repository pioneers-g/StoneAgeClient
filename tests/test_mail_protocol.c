/*
 * Stone Age Client - Mail Protocol Unit Tests
 * Tests for FUN_0045a9a0, FUN_004495c0, FUN_0045bb00
 *
 * Based on reverse engineering analysis of sa_9061.exe
 * Mail protocol: send/close/delete/confirm/cancel commands
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/types.h"
#include "../include/mail.h"

/* Test infrastructure */
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
 * Mail Protocol Format from FUN_0045a9a0:
 *
 * Commands:
 *   "send" (DAT_004a2624) - Send mail
 *   "close" (DAT_004c228c) - Close mail window
 *   "delete" (DAT_004b9fd8) - Delete mail
 *   "confirm" (DAT_004a262c) - Confirm action
 *   "cancel" (DAT_004b9eb4) - Cancel action
 *
 * Packet Format:
 *   Field 1: Command type
 *   Field 2: Sender name (0x7f max)
 *   Field 3: Recipient name (0x7f max)
 *   Field 4: Subject/type indicator
 *   Fields 5+: Additional data
 */

/*
 * Test 1-5: Mail command parsing
 */
TEST(parse_send_command) {
    const char* packet = "send|Player1|Player2|Hello|Body text|";
    char cmd[16];
    char sender[32], recipient[32];
    int pos = 0;

    ASSERT_EQ(sscanf(packet, "%[^|]|%[^|]|%[^|]|%n", cmd, sender, recipient, &pos), 3);
    ASSERT_STR_EQ(cmd, "send");
    ASSERT_STR_EQ(sender, "Player1");
    ASSERT_STR_EQ(recipient, "Player2");
}

TEST(parse_close_command) {
    const char* packet = "close|";
    char cmd[16];

    ASSERT_EQ(sscanf(packet, "%[^|]|", cmd), 1);
    ASSERT_STR_EQ(cmd, "close");
}

TEST(parse_delete_command) {
    const char* packet = "delete|12345|";
    char cmd[16];
    int mail_id;

    ASSERT_EQ(sscanf(packet, "%[^|]|%d|", cmd, &mail_id), 2);
    ASSERT_STR_EQ(cmd, "delete");
    ASSERT_EQ(mail_id, 12345);
}

TEST(parse_confirm_command) {
    const char* packet = "confirm|PlayerName|";
    char cmd[16];
    char name[32];

    ASSERT_EQ(sscanf(packet, "%[^|]|%[^|]|", cmd, name), 2);
    ASSERT_STR_EQ(cmd, "confirm");
    ASSERT_STR_EQ(name, "PlayerName");
}

TEST(parse_cancel_command) {
    const char* packet = "cancel|";
    char cmd[16];

    ASSERT_EQ(sscanf(packet, "%[^|]|", cmd), 1);
    ASSERT_STR_EQ(cmd, "cancel");
}

/*
 * Test 6-10: Mail data structure sizes
 * From FUN_004495c0 and FUN_0045a9a0 analysis
 */
TEST(struct_size_mail_entry) {
    /* Mail entry: 0xef0 (3824) bytes from FUN_004495c0 */
    ASSERT_EQ(sizeof(MailData), 0xef0);
}

TEST(struct_size_mail_header) {
    /* Mail header for quick list: 0x3bc (956) bytes */
    ASSERT_EQ(sizeof(MailHeader), 0x3bc);
}

TEST(struct_offset_sender) {
    /* Sender name at offset in MailData */
    MailData mail = {0};
    mail.sender[0] = 'T';
    ASSERT_EQ(mail.sender[0], 'T');
}

TEST(struct_offset_subject) {
    /* Subject field */
    MailData mail = {0};
    strncpy(mail.subject, "Test Subject", sizeof(mail.subject) - 1);
    ASSERT_STR_EQ(mail.subject, "Test Subject");
}

TEST(struct_max_sizes) {
    /* Max sender/recipient: 0x7f (127) chars */
    ASSERT_EQ(MAX_MAIL_SENDER_LEN, 0x7f);
    /* Max subject: 0xff (255) chars */
    ASSERT_EQ(MAX_MAIL_SUBJECT_LEN, 0xff);
    /* Max body: 0x3ff (1023) chars */
    ASSERT_EQ(MAX_MAIL_BODY_LEN, 0x3ff);
}

/*
 * Test 11-15: Mail file format
 * From FUN_004495c0 analysis
 */
TEST(file_format_entry_size) {
    /* Each mail entry: 0xef0 (3824) bytes */
    ASSERT_EQ(MAIL_ENTRY_SIZE, 0xef0);
}

TEST(file_format_header_offset) {
    /* Header offset: DAT_04630a88 * 0x4ab00 */
    /* Entry offset: index * 0xef0 */
    int index = 5;
    u32 offset = 0x10 + index * MAIL_ENTRY_SIZE;  /* 16 byte header + entries */
    ASSERT_EQ(offset, 0x10 + 5 * 0xef0);
}

TEST(file_format_path) {
    /* Mail file: data\mail.dat */
    const char* expected = "data\\mail.dat";
    ASSERT_STR_EQ(MAIL_FILE_PATH, expected);
}

TEST(file_format_max_mails) {
    /* Maximum mails: MAX_MAILS constant */
    ASSERT_EQ(MAX_MAILS, 100);
}

TEST(file_format_max_items) {
    /* Maximum attachments: MAX_MAIL_ITEMS */
    ASSERT_EQ(MAX_MAIL_ITEMS, 5);
}

/*
 * Test 16-20: Mail state machine
 * From DAT_04630878 analysis
 */
TEST(state_closed) {
    ASSERT_EQ(MAIL_STATE_CLOSED, 0);
}

TEST(state_opening) {
    ASSERT_EQ(MAIL_STATE_OPENING, 1);
}

TEST(state_read_mode) {
    ASSERT_EQ(MAIL_STATE_READ, 2);
}

TEST(state_compose_mode) {
    ASSERT_EQ(MAIL_STATE_COMPOSE, 3);
}

TEST(state_reply_mode) {
    ASSERT_EQ(MAIL_STATE_REPLY, 4);
}

/*
 * Test 21-25: Mail fee calculation
 * From FUN_0045a9a0 protocol
 */
TEST(fee_base) {
    ASSERT_EQ(MAIL_FEE_BASE, 100);  /* Base postage fee */
}

TEST(fee_with_gold) {
    u32 fee = MAIL_FEE_BASE + MAIL_FEE_GOLD;
    ASSERT_EQ(fee, 150);  /* Base + gold attachment fee */
}

TEST(fee_with_items) {
    u32 fee = MAIL_FEE_BASE + 2 * MAIL_FEE_PER_ITEM;
    ASSERT_EQ(fee, 200);  /* Base + 2 items */
}

TEST(fee_calculate) {
    /* Test fee calculation function */
    u32 fee = mail_calculate_fee(0, 0);  /* Text only */
    ASSERT_EQ(fee, MAIL_FEE_BASE);

    fee = mail_calculate_fee(1000, 0);  /* With gold */
    ASSERT_EQ(fee, MAIL_FEE_BASE + MAIL_FEE_GOLD);

    fee = mail_calculate_fee(0, 3);  /* With 3 items */
    ASSERT_EQ(fee, MAIL_FEE_BASE + 3 * MAIL_FEE_PER_ITEM);
}

TEST(fee_max) {
    /* Maximum fee with all attachments */
    u32 fee = MAIL_FEE_BASE + MAIL_FEE_GOLD +
              MAX_MAIL_ITEMS * MAIL_FEE_PER_ITEM;
    ASSERT(fee > 0);
}

/*
 * Test 26-30: Mail send result codes
 * From FUN_0045d6f0 pattern
 */
TEST(result_success) {
    ASSERT_EQ(MAIL_SEND_SUCCESS, 0);
}

TEST(result_delivered) {
    ASSERT_EQ(MAIL_SEND_DELIVERED, 1);
}

TEST(result_mailbox_full) {
    ASSERT_EQ(MAIL_SEND_MAILBOX_FULL, 2);
}

TEST(result_no_postage) {
    ASSERT_EQ(MAIL_SEND_NO_POSTAGE, 3);
}

TEST(result_recipient_not_found) {
    ASSERT_EQ(MAIL_SEND_RECIPIENT_NOT_FOUND, 4);
}

/*
 * Test 31-35: Mail protocol packet building
 */
TEST(packet_build_send) {
    char packet[256];
    snprintf(packet, sizeof(packet), "M|%s|%d|%u", "recipient", MAIL_TYPE_TEXT, 0);
    ASSERT_STR_EQ(packet, "M|recipient|0|0");
}

TEST(packet_build_with_gold) {
    char packet[256];
    snprintf(packet, sizeof(packet), "M|%s|%d|%u", "recipient", MAIL_TYPE_GOLD, 500);
    ASSERT_STR_EQ(packet, "M|recipient|1|500");
}

TEST(packet_build_with_items) {
    char packet[256];
    snprintf(packet, sizeof(packet), "M|%s|%d|%u", "recipient", MAIL_TYPE_ITEMS, 3);
    ASSERT_STR_EQ(packet, "M|recipient|2|3");
}

TEST(packet_build_delete) {
    char packet[64];
    snprintf(packet, sizeof(packet), "MD|%u", 12345);
    ASSERT_STR_EQ(packet, "MD|12345");
}

TEST(packet_build_claim) {
    char packet[64];
    snprintf(packet, sizeof(packet), "T|%d", 5);  /* Mail index */
    ASSERT_STR_EQ(packet, "T|5");
}

int main(void) {
    printf("=== Mail Protocol Unit Tests ===\n\n");

    /* Command parsing tests */
    RUN_TEST(parse_send_command);
    RUN_TEST(parse_close_command);
    RUN_TEST(parse_delete_command);
    RUN_TEST(parse_confirm_command);
    RUN_TEST(parse_cancel_command);

    /* Structure size tests */
    RUN_TEST(struct_size_mail_entry);
    RUN_TEST(struct_size_mail_header);
    RUN_TEST(struct_offset_sender);
    RUN_TEST(struct_offset_subject);
    RUN_TEST(struct_max_sizes);

    /* File format tests */
    RUN_TEST(file_format_entry_size);
    RUN_TEST(file_format_header_offset);
    RUN_TEST(file_format_path);
    RUN_TEST(file_format_max_mails);
    RUN_TEST(file_format_max_items);

    /* State machine tests */
    RUN_TEST(state_closed);
    RUN_TEST(state_opening);
    RUN_TEST(state_read_mode);
    RUN_TEST(state_compose_mode);
    RUN_TEST(state_reply_mode);

    /* Fee calculation tests */
    RUN_TEST(fee_base);
    RUN_TEST(fee_with_gold);
    RUN_TEST(fee_with_items);
    RUN_TEST(fee_calculate);
    RUN_TEST(fee_max);

    /* Result code tests */
    RUN_TEST(result_success);
    RUN_TEST(result_delivered);
    RUN_TEST(result_mailbox_full);
    RUN_TEST(result_no_postage);
    RUN_TEST(result_recipient_not_found);

    /* Packet building tests */
    RUN_TEST(packet_build_send);
    RUN_TEST(packet_build_with_gold);
    RUN_TEST(packet_build_with_items);
    RUN_TEST(packet_build_delete);
    RUN_TEST(packet_build_claim);

    printf("\n=== Test Results ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
