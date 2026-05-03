/*
 * Stone Age Client - Mail System Core
 * Reverse engineered from sa_9061.exe
 *
 * Core mail system functionality:
 * - Initialization and shutdown
 * - Mail data management
 * - Utility functions
 *
 * Protocol handlers: mail_protocol.c
 * File operations: mail_file.c
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "mail.h"
#include "logger.h"

/* Global mail context */
MailContext g_mail = {0};

/*
 * Initialize mail system
 */
int mail_init(void) {
    memset(&g_mail, 0, sizeof(MailContext));
    g_mail.send_result = MAIL_SEND_SUCCESS;
    LOG_INFO("Mail system initialized");
    return 1;
}

/*
 * Shutdown mail system
 */
void mail_shutdown(void) {
    int i;

    for (i = 0; i < MAX_MAILS; i++) {
        if (g_mail.mails[i].items) {
            free(g_mail.mails[i].items);
            g_mail.mails[i].items = NULL;
        }
    }

    memset(&g_mail, 0, sizeof(MailContext));
    LOG_INFO("Mail system shutdown");
}

/*
 * Find mail by ID
 */
MailData* mail_find(u32 mail_id) {
    int i;

    for (i = 0; i < MAX_MAILS; i++) {
        if (g_mail.mails[i].id == mail_id) {
            return &g_mail.mails[i];
        }
    }

    return NULL;
}

/*
 * Get mail by index
 */
MailData* mail_get_by_index(int index) {
    if (index < 0 || index >= MAX_MAILS) {
        return NULL;
    }
    return &g_mail.mails[index];
}

/*
 * Get unread count
 */
int mail_get_unread_count(void) {
    int i;
    int count = 0;

    for (i = 0; i < MAX_MAILS; i++) {
        if (g_mail.mails[i].id != 0 && !g_mail.mails[i].read) {
            count++;
        }
    }

    return count;
}

/*
 * Check if new mail
 */
int mail_has_new(void) {
    return g_mail.new_mail;
}

/*
 * Refresh mail list
 */
void mail_refresh_list(void) {
    mail_request_list();
}

/*
 * Get current mail
 */
MailData* mail_get_current(void) {
    if (g_mail.current_mail_id == 0) {
        return NULL;
    }
    return mail_find(g_mail.current_mail_id);
}

/*
 * Set current mail
 */
void mail_set_current(u32 mail_id) {
    g_mail.current_mail_id = mail_id;
}

/*
 * Get mail count
 */
int mail_get_count(void) {
    return g_mail.mail_count;
}

/*
 * Check if mailbox full
 */
int mail_is_full(void) {
    return g_mail.mail_count >= MAX_MAILS;
}

/*
 * Set player gold (for fee calculation)
 */
void mail_set_player_gold(u32 gold) {
    g_mail.player_gold = gold;
}

/*
 * Calculate mail fee - from FUN_0045a9a0 pattern
 * Base fee + gold fee + item fees
 */
u32 mail_calculate_fee(u32 gold, u32 item_count) {
    u32 fee = MAIL_FEE_BASE;

    if (gold > 0) {
        fee += MAIL_FEE_GOLD;
    }

    if (item_count > 0) {
        fee += item_count * MAIL_FEE_PER_ITEM;
    }

    return fee;
}
