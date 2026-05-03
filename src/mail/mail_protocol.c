/*
 * Stone Age Client - Mail Protocol Handlers
 * Split from mail.c
 *
 * Contains protocol communication handlers:
 * - Mail list request/response
 * - Mail read handler
 * - Mail send operations
 * - Send result handling
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "types.h"
#include "mail.h"
#include "network.h"
#include "logger.h"

/* External mail context */
extern MailContext g_mail;

/* Internal helper: Send mail protocol command */
static int mail_send_command(const char* cmd, const char* params) {
    char packet[512];
    int len;

    if (!cmd) return 0;

    if (params && params[0]) {
        len = _snprintf(packet, sizeof(packet), "%s|%s", cmd, params);
    } else {
        strncpy(packet, cmd, sizeof(packet) - 1);
        packet[sizeof(packet) - 1] = '\0';
        len = (int)strlen(packet);
    }

    if (len < 0 || len >= (int)sizeof(packet)) {
        LOG_ERROR("Mail packet too long");
        return 0;
    }

    network_send(packet, strlen(packet));
    network_send("\n", 1);
    LOG_DEBUG("Mail send: %s", packet);
    return 1;
}

/* Internal helper: Send formatted packet */
static void mail_send_formatted(const char* fmt, ...) {
    char packet[1024];
    va_list args;

    va_start(args, fmt);
    _vsnprintf(packet, sizeof(packet), fmt, args);
    va_end(args);

    network_send(packet, strlen(packet));
    network_send("\n", 1);
}

/*
 * Request mail list - FUN_0045a900 pattern
 */
void mail_request_list(void) {
    mail_send_command(MAIL_CMD_LIST, NULL);

    g_mail.busy = 1;
    g_mail.list_loaded = 0;

    LOG_DEBUG("Requested mail list from server");
}

/*
 * Handle mail list response - FUN_0045a900 pattern
 */
void mail_handle_list(void* data, u32 size) {
    char* ptr = (char*)data;
    char* end = (char*)data + size;
    char field_buffer[128];
    int mail_index = 0;
    int field_index;
    MailData* mail;
    int i;

    if (!data || size < 2) return;

    /* Clear existing mails */
    for (i = 0; i < MAX_MAILS; i++) {
        if (g_mail.mails[i].items) {
            free(g_mail.mails[i].items);
            g_mail.mails[i].items = NULL;
        }
    }
    memset(g_mail.mails, 0, sizeof(g_mail.mails));

    g_mail.mail_count = 0;

    while (ptr < end && mail_index < MAX_MAILS) {
        mail = &g_mail.mails[mail_index];
        field_index = 0;

        while (ptr < end && field_index < 7) {
            char* next = strchr(ptr, '|');
            if (!next) next = end;

            int field_len = next - ptr;
            if (field_len > 0 && field_len < (int)sizeof(field_buffer)) {
                strncpy(field_buffer, ptr, field_len);
                field_buffer[field_len] = '\0';

                switch (field_index) {
                    case 0:  mail->id = (u32)atoi(field_buffer); break;
                    case 1:  strncpy(mail->sender, field_buffer, sizeof(mail->sender) - 1); break;
                    case 2:  strncpy(mail->subject, field_buffer, sizeof(mail->subject) - 1); break;
                    case 3:  mail->timestamp = (u32)atoi(field_buffer); break;
                    case 4:  mail->read = (u8)atoi(field_buffer); break;
                    case 5:  mail->has_items = (u8)atoi(field_buffer); break;
                    case 6:  mail->has_gold = (u8)atoi(field_buffer); break;
                }
            }

            ptr = next + 1;
            field_index++;
        }

        if (mail->id != 0) {
            g_mail.mail_count++;
            mail_index++;
        } else {
            break;
        }
    }

    g_mail.list_loaded = 1;
    g_mail.new_mail = 0;
    g_mail.busy = 0;

    LOG_INFO("Received %d mails from server", g_mail.mail_count);
}

/*
 * Handle mail read response - FUN_0045a980 pattern
 */
void mail_handle_read(void* data, u32 size) {
    char* ptr = (char*)data;
    char* end = (char*)data + size;
    char field_buffer[64];
    u32 mail_id;
    MailData* mail;
    int i;

    if (!data || size < 4) return;

    char* next = strchr(ptr, '|');
    if (!next) return;

    int id_len = next - ptr;
    if (id_len > 0 && id_len < (int)sizeof(field_buffer)) {
        strncpy(field_buffer, ptr, id_len);
        field_buffer[id_len] = '\0';
        mail_id = (u32)atoi(field_buffer);
    } else {
        return;
    }

    ptr = next + 1;

    mail = mail_find(mail_id);
    if (!mail) {
        LOG_WARN("Mail ID %u not found", mail_id);
        return;
    }

    mail->read = 1;

    next = strchr(ptr, '|');
    if (next) {
        int body_len = next - ptr;
        if (body_len > 0 && body_len < (int)sizeof(mail->body)) {
            strncpy(mail->body, ptr, body_len);
            mail->body[body_len] = '\0';
        }
        ptr = next + 1;
    }

    if (mail->has_items && ptr < end) {
        next = strchr(ptr, '|');
        if (next) {
            int count_len = next - ptr;
            if (count_len > 0 && count_len < (int)sizeof(field_buffer)) {
                strncpy(field_buffer, ptr, count_len);
                field_buffer[count_len] = '\0';
                mail->item_count = (u32)atoi(field_buffer);
            }
            ptr = next + 1;
        }

        if (mail->item_count > 0 && mail->item_count <= MAX_MAIL_ITEMS) {
            mail->items = (MailItem*)malloc(mail->item_count * sizeof(MailItem));
            if (mail->items) {
                memset(mail->items, 0, mail->item_count * sizeof(MailItem));

                for (i = 0; i < (int)mail->item_count && ptr < end; i++) {
                    next = strchr(ptr, '|');
                    if (next) {
                        strncpy(field_buffer, ptr, next - ptr);
                        field_buffer[next - ptr] = '\0';
                        mail->items[i].item_id = (u32)atoi(field_buffer);
                        ptr = next + 1;

                        next = strchr(ptr, '|');
                        if (next) {
                            strncpy(field_buffer, ptr, next - ptr);
                            field_buffer[next - ptr] = '\0';
                            mail->items[i].count = (u32)atoi(field_buffer);
                            ptr = next + 1;
                        }
                    }
                }
            }
        }
    }

    if (mail->has_gold && ptr < end) {
        next = strchr(ptr, '|');
        int gold_len = next ? (next - ptr) : (end - ptr);
        if (gold_len > 0 && gold_len < (int)sizeof(field_buffer)) {
            strncpy(field_buffer, ptr, gold_len);
            field_buffer[gold_len] = '\0';
            mail->gold = (u32)atoi(field_buffer);
        }
    }

    g_mail.current_mail_id = mail_id;

    LOG_DEBUG("Read mail from %s: %s", mail->sender, mail->subject);
}

/*
 * Send mail - uses "M|%s|%d|%d" protocol format
 */
int mail_send(const char* recipient, const char* subject, const char* body,
              u32 gold, u32 item_count, MailItem* items) {
    char packet[1024];
    char escaped_body[512];
    MailType type;
    u32 fee;
    u32 param;
    int i;
    int offset;

    if (!recipient || !subject || !body) {
        LOG_ERROR("Invalid mail parameters");
        return 0;
    }

    if (g_mail.busy) {
        LOG_WARN("Mail system busy");
        return 0;
    }

    fee = MAIL_FEE_BASE;
    if (gold > 0) fee += MAIL_FEE_GOLD;
    if (item_count > 0) fee += item_count * MAIL_FEE_PER_ITEM;

    if (g_mail.player_gold < fee) {
        LOG_WARN("Not enough gold for mail fee: have %u, need %u", g_mail.player_gold, fee);
        g_mail.send_result = MAIL_SEND_NO_POSTAGE;
        strcpy(g_mail.error_message, "Insufficient postage");
        return 0;
    }

    if (item_count > 0) {
        type = MAIL_TYPE_ITEMS;
        param = item_count;
    } else if (gold > 0) {
        type = MAIL_TYPE_GOLD;
        param = gold;
    } else {
        type = MAIL_TYPE_TEXT;
        param = 0;
    }

    offset = 0;
    for (i = 0; body[i] && offset < (int)sizeof(escaped_body) - 2; i++) {
        if (body[i] == '|') {
            escaped_body[offset++] = '|';
            escaped_body[offset++] = '|';
        } else {
            escaped_body[offset++] = body[i];
        }
    }
    escaped_body[offset] = '\0';

    mail_send_formatted("M|%s|%d|%u", recipient, type, param);

    _snprintf(packet, sizeof(packet), "MS|%s|%s", subject, escaped_body);
    network_send(packet, strlen(packet));
    network_send("\n", 1);

    if (item_count > 0 && items) {
        for (i = 0; i < (int)item_count && i < MAX_MAIL_ITEMS; i++) {
            _snprintf(packet, sizeof(packet), "MI|%u|%u", items[i].item_id, items[i].count);
            network_send(packet, strlen(packet));
            network_send("\n", 1);
        }
    }

    g_mail.busy = 1;
    g_mail.has_attachment = (type > MAIL_TYPE_TEXT) ? 1 : 0;
    strncpy(g_mail.last_recipient, recipient, sizeof(g_mail.last_recipient) - 1);
    g_mail.last_send_time = timeGetTime();
    g_mail.send_result = MAIL_SEND_SUCCESS;

    LOG_INFO("Sent mail to %s (type=%d, param=%u, fee=%u)", recipient, type, param, fee);
    return 1;
}

/*
 * Handle send result - FUN_0045d6f0 pattern
 */
void mail_handle_send_result(void* data, u32 size) {
    char* ptr = (char*)data;
    char result;
    char field_buffer[64];

    if (!data || size < 1) return;

    char* next = strchr(ptr, '|');
    if (next) {
        strncpy(field_buffer, ptr, next - ptr);
        field_buffer[next - ptr] = '\0';
        result = field_buffer[0];
        ptr = next + 1;
    } else {
        result = *ptr;
        ptr++;
    }

    switch (result) {
        case MAIL_PKT_SUCCESS:
            g_mail.send_result = MAIL_SEND_SUCCESS;
            mail_handle_success(ptr, size - (ptr - (char*)data));
            break;

        case MAIL_PKT_DELIVERED:
            g_mail.send_result = MAIL_SEND_DELIVERED;
            g_mail.busy = 0;
            if (*ptr) {
                strncpy(g_mail.last_recipient, ptr, sizeof(g_mail.last_recipient) - 1);
            }
            LOG_INFO("Mail delivered to: %s", g_mail.last_recipient);
            break;

        case MAIL_PKT_MAILBOX_FULL:
            g_mail.send_result = MAIL_SEND_MAILBOX_FULL;
            strcpy(g_mail.error_message, "Recipient mailbox is full");
            g_mail.busy = 0;
            LOG_WARN("Mailbox full for recipient: %s", g_mail.last_recipient);
            break;

        case MAIL_PKT_NO_POSTAGE:
            g_mail.send_result = MAIL_SEND_NO_POSTAGE;
            g_mail.busy = 0;
            if (*ptr) {
                strncpy(g_mail.error_message, ptr, sizeof(g_mail.error_message) - 1);
            } else {
                strcpy(g_mail.error_message, "Insufficient postage");
            }
            LOG_WARN("Mail postage error: %s", g_mail.error_message);
            break;

        case MAIL_PKT_RECIPIENT_ERR:
            g_mail.send_result = MAIL_SEND_RECIPIENT_NOT_FOUND;
            mail_handle_recipient_error(ptr, size - (ptr - (char*)data));
            break;

        case MAIL_PKT_TIMEOUT:
            g_mail.send_result = MAIL_SEND_ERROR;
            g_mail.busy = 0;
            strcpy(g_mail.error_message, "Mail delivery timeout");
            LOG_WARN("Mail delivery timeout");
            break;

        default:
            LOG_WARN("Unknown mail result code: 0x%02X", (u8)result);
            g_mail.send_result = MAIL_SEND_ERROR;
            g_mail.busy = 0;
            break;
    }

    g_mail.last_send_time = timeGetTime();
}

/*
 * Handle mail success - FUN_0045c530 pattern
 */
void mail_handle_success(void* data, u32 size) {
    char* ptr = (char*)data;
    char field_buffer[64];
    u32 mail_id = 0;

    if (!data) return;

    char* next = strchr(ptr, '|');
    if (next) {
        strncpy(field_buffer, ptr, next - ptr);
        field_buffer[next - ptr] = '\0';
        mail_id = (u32)atoi(field_buffer);
    }

    LOG_DEBUG("Mail sent successfully, ID: %u", mail_id);

    g_mail.has_attachment = 0;
    g_mail.busy = 0;
}

/*
 * Handle recipient error - FUN_0045ca00 pattern
 */
void mail_handle_recipient_error(void* data, u32 size) {
    char* ptr = (char*)data;
    char recipient[24];

    if (!data) return;

    char* next = strchr(ptr, '|');
    if (next) {
        int len = next - ptr;
        if (len > 0 && len < (int)sizeof(recipient)) {
            strncpy(recipient, ptr, len);
            recipient[len] = '\0';
        }
    } else if (*ptr) {
        strncpy(recipient, ptr, sizeof(recipient) - 1);
        recipient[sizeof(recipient) - 1] = '\0';
    } else {
        strcpy(recipient, "unknown");
    }

    LOG_WARN("Recipient not found: %s", recipient);

    _snprintf(g_mail.error_message, sizeof(g_mail.error_message),
              "Recipient '%s' not found", recipient);

    g_mail.has_attachment = 0;
    g_mail.busy = 0;
}

/*
 * Delete mail - sends MD command
 */
int mail_delete(u32 mail_id) {
    MailData* mail;
    char params[32];

    mail = mail_find(mail_id);
    if (!mail) {
        LOG_WARN("Mail ID %u not found", mail_id);
        return 0;
    }

    if (mail->has_items && mail->item_count > 0) {
        LOG_WARN("Cannot delete mail with unclaimed items");
        return 0;
    }

    if (mail->has_gold && mail->gold > 0) {
        LOG_WARN("Cannot delete mail with unclaimed gold");
        return 0;
    }

    _snprintf(params, sizeof(params), "%u", mail_id);
    mail_send_command(MAIL_CMD_DELETE, params);

    if (mail->items) {
        free(mail->items);
        mail->items = NULL;
    }
    memset(mail, 0, sizeof(MailData));

    g_mail.mail_count--;

    LOG_DEBUG("Deleted mail ID: %u", mail_id);
    return 1;
}

/*
 * Claim mail items - uses "T|%d" protocol
 */
int mail_claim_items(u32 mail_id) {
    MailData* mail;
    char params[32];
    int i;

    mail = mail_find(mail_id);
    if (!mail) {
        LOG_WARN("Mail ID %u not found", mail_id);
        return 0;
    }

    if (!mail->has_items && !mail->has_gold) {
        LOG_WARN("Mail has no attachments");
        return 0;
    }

    for (i = 0; i < MAX_MAILS; i++) {
        if (g_mail.mails[i].id == mail_id) {
            break;
        }
    }

    if (i >= MAX_MAILS) {
        LOG_WARN("Mail index not found for ID: %u", mail_id);
        return 0;
    }

    _snprintf(params, sizeof(params), "%d", i);
    mail_send_command(MAIL_CMD_TAKE, params);

    if (mail->items) {
        free(mail->items);
        mail->items = NULL;
    }
    mail->item_count = 0;
    mail->has_items = 0;
    mail->gold = 0;
    mail->has_gold = 0;

    LOG_INFO("Claimed items from mail ID: %u (index %d)", mail_id, i);
    return 1;
}

/*
 * Claim mail gold
 */
int mail_claim_gold(u32 mail_id) {
    return mail_claim_items(mail_id);
}
