/*
 * Stone Age Client - Mail File Operations
 * Split from mail.c
 *
 * Contains file load/save operations:
 * - Mail data file loading
 * - Mail data file saving
 * - Page-based mail storage
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "mail.h"
#include "logger.h"

/* External mail context */
extern MailContext g_mail;

/*
 * Load mail from file - matches FUN_004495c0 pattern
 * param_1 is mail index (0-79)
 */
int mail_load_from_file(int page_index, int mail_index) {
    FILE* fp;
    u32 offset;
    char path[] = MAIL_DATA_FILE;

    if (mail_index < 0 || mail_index >= MAX_MAILS) {
        return 0;
    }

    fp = fopen(path, "rb");
    if (!fp) {
        LOG_WARN("Mail data file not found: %s", path);
        return 0;
    }

    /* Calculate offset: page * 0x4ab00 + mail_index * 0xef0 */
    offset = (u32)page_index * MAIL_PAGE_SIZE + (u32)mail_index * MAIL_ENTRY_SIZE;

    if (fseek(fp, offset, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }

    /* Read mail entry - 0xef0 bytes */
    MailData* mail = &g_mail.mails[mail_index];

    u8 buffer[MAIL_ENTRY_SIZE];
    if (fread(buffer, 1, MAIL_ENTRY_SIZE, fp) != MAIL_ENTRY_SIZE) {
        fclose(fp);
        return 0;
    }

    fclose(fp);

    /* Parse mail data from buffer */
    mail->id = 1;  /* Placeholder */

    LOG_DEBUG("Loaded mail %d from page %d", mail_index, page_index);
    return 1;
}

/*
 * Save mail to file - matches FUN_004495c0 write pattern
 */
int mail_save_to_file(int mail_index) {
    FILE* fp;
    u32 offset;
    char path[] = MAIL_DATA_FILE;

    if (mail_index < 0 || mail_index >= MAX_MAILS) {
        return 0;
    }

    fp = fopen(path, "r+b");
    if (!fp) {
        fp = fopen(path, "wb");
        if (!fp) {
            LOG_ERROR("Cannot create mail data file: %s", path);
            return 0;
        }
    }

    /* Calculate offset: page 0 + mail_index * 0xef0 */
    offset = (u32)mail_index * MAIL_ENTRY_SIZE;

    if (fseek(fp, offset, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }

    u8 buffer[MAIL_ENTRY_SIZE];
    memset(buffer, 0, MAIL_ENTRY_SIZE);

    MailData* mail = &g_mail.mails[mail_index];

    /* Serialize mail data to buffer */
    memcpy(buffer, &mail->id, sizeof(mail->id));

    if (fwrite(buffer, 1, MAIL_ENTRY_SIZE, fp) != MAIL_ENTRY_SIZE) {
        fclose(fp);
        return 0;
    }

    fclose(fp);

    LOG_DEBUG("Saved mail %d to file", mail_index);
    return 1;
}

/*
 * Load mail page - matches FUN_00449660 pattern
 * Reads 0x50 (80) mail entries from specified page
 */
int mail_load_page(int page_index) {
    FILE* fp;
    u32 offset;
    u32 read_count;
    int i;
    char path[] = MAIL_DATA_FILE;

    fp = fopen(path, "rb");
    if (!fp) {
        LOG_WARN("Mail data file not found: %s", path);
        return 0;
    }

    /* Calculate page offset */
    offset = (u32)page_index * MAIL_PAGE_SIZE;

    if (fseek(fp, offset, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }

    /* Read 80 mail entries */
    read_count = 0;
    for (i = 0; i < MAX_MAILS; i++) {
        u8 buffer[MAIL_ENTRY_SIZE];

        if (fread(buffer, 1, MAIL_ENTRY_SIZE, fp) != MAIL_ENTRY_SIZE) {
            break;
        }

        /* Parse mail data */
        MailData* mail = &g_mail.mails[i];
        memcpy(&mail->id, buffer, sizeof(mail->id));

        if (mail->id != 0) {
            read_count++;
        }
    }

    fclose(fp);

    g_mail.mail_count = read_count;
    g_mail.list_loaded = 1;

    LOG_INFO("Loaded %u mails from page %d", read_count, page_index);
    return read_count > 0;
}
