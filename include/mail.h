/*
 * Stone Age Client - Mail System Header
 * Reverse engineered from sa_9061.exe
 *
 * Protocol Analysis:
 * FUN_0045a900 - Mail list request handler
 * FUN_0045a980 - Mail read handler
 * FUN_0045d6f0 - Mail send result router
 * FUN_0045c530 - Mail success handler
 * FUN_0045ca00 - Recipient error handler
 * FUN_00430b50 - Mail send UI handler
 *
 * Protocol Commands:
 * - ML: Request mail list
 * - MR|<mail_id>: Read mail
 * - M|<recipient>|<type>|<param>: Send mail (0x004b9658)
 * - MD|<mail_id>: Delete mail
 * - T|<mail_index>: Take/claim attachments (0x004b9650)
 *
 * Response Codes:
 * - 'B' (0x42): Success
 * - 'D' (0x44): Delivered
 * - 'J' (0x4A): Mailbox full
 * - 'K' (0x4B): Insufficient postage
 * - 'R' (0x52): Recipient not found
 * - 'T' (0x54): Timeout/error
 */

#ifndef MAIL_H
#define MAIL_H

#include "types.h"

/* Constants from sa_9061.exe FUN_00449660 */
#define MAX_MAILS           80      /* 0x50 entries per page */
#define MAX_MAIL_ITEMS      5
#define MAIL_ENTRY_SIZE     0xef0   /* 3824 bytes per mail entry */
#define MAIL_PAGE_SIZE      0x4ab00 /* 306176 bytes per page (80 * 3824) */
#define MAIL_DATA_FILE      "data\\mail.dat"
#define MAIL_FILE_PATH      "data\\mail.dat"

/* String field sizes from FUN_0045a9a0 */
#define MAX_MAIL_SENDER_LEN     0x7f    /* 127 chars max */
#define MAX_MAIL_RECIPIENT_LEN  0x7f    /* 127 chars max */
#define MAX_MAIL_SUBJECT_LEN    0xff    /* 255 chars max */
#define MAX_MAIL_BODY_LEN       0x3ff   /* 1023 chars max */

/* Mail state machine - DAT_04630878 */
#define MAIL_STATE_CLOSED       0
#define MAIL_STATE_OPENING      1
#define MAIL_STATE_READ         2
#define MAIL_STATE_COMPOSE      3
#define MAIL_STATE_REPLY        4

/* Mail header for quick list - 0x3bc (956) bytes */
typedef struct {
    u32 id;
    char sender[24];
    char subject[64];
    u32 timestamp;
    u8  read;
    u8  has_items;
    u8  has_gold;
    u8  reserved;
} MailHeader;

/* Mail send results */
typedef enum {
    MAIL_SEND_SUCCESS = 0,
    MAIL_SEND_DELIVERED = 1,
    MAIL_SEND_MAILBOX_FULL = 2,
    MAIL_SEND_NO_POSTAGE = 3,
    MAIL_SEND_RECIPIENT_NOT_FOUND = 4,
    MAIL_SEND_ERROR = 5
} MailSendResult;

/* Mail type codes for sending */
typedef enum {
    MAIL_TYPE_TEXT = 0,         /* Text only */
    MAIL_TYPE_GOLD = 1,         /* Gold attachment */
    MAIL_TYPE_ITEMS = 2         /* Items attachment */
} MailType;

/* Mail packet response codes */
#define MAIL_PKT_SUCCESS        'B'     /* 0x42 - Success */
#define MAIL_PKT_DELIVERED      'D'     /* 0x44 - Delivered */
#define MAIL_PKT_MAILBOX_FULL   'J'     /* 0x4A - Mailbox full */
#define MAIL_PKT_NO_POSTAGE     'K'     /* 0x4B - Insufficient postage */
#define MAIL_PKT_RECIPIENT_ERR  'R'     /* 0x52 - Recipient not found */
#define MAIL_PKT_TIMEOUT        'T'     /* 0x54 - Timeout/error */

/* Protocol commands */
#define MAIL_CMD_LIST       "ML"    /* Request mail list */
#define MAIL_CMD_READ       "MR"    /* Read mail */
#define MAIL_CMD_SEND       "M"     /* Send mail */
#define MAIL_CMD_DELETE     "MD"    /* Delete mail */
#define MAIL_CMD_TAKE       "T"     /* Take attachments */

/* Fee constants */
#define MAIL_FEE_BASE       100     /* Base mail fee */
#define MAIL_FEE_GOLD       100     /* Gold attachment fee */
#define MAIL_FEE_PER_ITEM   500     /* Fee per item attachment */

/* Mail item attachment */
typedef struct {
    u32 item_id;
    u32 count;
} MailItem;

/* Mail data */
typedef struct {
    u32 id;
    char sender[24];
    char subject[64];
    char body[512];
    u32 timestamp;
    u8  read;
    u8  has_items;
    u8  has_gold;
    u8  reserved;

    /* Attachments */
    MailItem* items;
    u32 item_count;
    u32 gold;

} MailData;

/* Mail context */
typedef struct {
    /* Mails */
    MailData mails[MAX_MAILS];
    int mail_count;
    int list_loaded;

    /* Current mail being viewed */
    u32 current_mail_id;

    /* Flags */
    int new_mail;
    int busy;
    int has_attachment;

    /* Send result */
    MailSendResult send_result;
    u32 last_send_time;
    char last_recipient[24];
    char error_message[128];

    /* Player gold for fee calculation */
    u32 player_gold;

} MailContext;

/* Global mail context */
extern MailContext g_mail;

/* Initialization */
int mail_init(void);
void mail_shutdown(void);

/* Request/refresh */
void mail_request_list(void);
void mail_refresh_list(void);

/* Packet handlers */
void mail_handle_list(void* data, u32 size);
void mail_handle_read(void* data, u32 size);
void mail_handle_send_result(void* data, u32 size);
void mail_handle_success(void* data, u32 size);
void mail_handle_recipient_error(void* data, u32 size);

/* Send mail */
int mail_send(const char* recipient, const char* subject, const char* body,
              u32 gold, u32 item_count, MailItem* items);

/* Delete mail */
int mail_delete(u32 mail_id);

/* Claim attachments */
int mail_claim_items(u32 mail_id);
int mail_claim_gold(u32 mail_id);

/* Query */
MailData* mail_find(u32 mail_id);
MailData* mail_get_by_index(int index);
MailData* mail_get_current(void);
int mail_get_unread_count(void);
int mail_has_new(void);
int mail_get_count(void);
int mail_is_full(void);

/* Navigation */
void mail_set_current(u32 mail_id);

/* Player gold */
void mail_set_player_gold(u32 gold);

/* Fee calculation */
u32 mail_calculate_fee(u32 gold, u32 item_count);

/* File persistence - from FUN_004495c0 and FUN_00449660 */
int mail_load_from_file(int page_index, int mail_index);
int mail_save_to_file(int mail_index);
int mail_load_page(int page_index);

#endif /* MAIL_H */
