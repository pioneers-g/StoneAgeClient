/*
 * Stone Age Client - Protocol Utility Functions Implementation
 * Reverse engineered from sa_9061.exe
 *
 * FUN_00489f70 - Parse delimited text field (DBCS-aware)
 * FUN_00489fe0 - Extract field content until delimiter
 *
 * These functions handle DBCS (Double-Byte Character Set) strings,
 * supporting both single-byte ASCII and double-byte characters.
 */

#include <windows.h>
#include <string.h>
#include "types.h"
#include "protocol_util.h"

/*
 * Parse delimited text field - FUN_00489f70
 *
 * Binary analysis:
 * - Loops through string looking for delimiter
 * - Counts delimiters to find the target field
 * - Handles DBCS by checking if byte >= 0x80 (lead byte)
 * - If lead byte found, skips 2 bytes instead of 1
 * - Calls FUN_00489fe0 to extract field content
 */
int parse_text_field(const char* str, char delimiter, int field_num, int max_size, char* output) {
    const unsigned char* ptr;
    int current_field;
    unsigned char c;

    if (!str || !output || field_num < 1) {
        if (output) {
            output[0] = '\0';
        }
        return 1;
    }

    ptr = (const unsigned char*)str;
    current_field = 1;

    /* Skip to the desired field */
    if (field_num > 1) {
        do {
            if (ptr == NULL || *ptr == '\0') {
                /* Reached end of string before finding field */
                output[0] = '\0';
                return 1;
            }

            /* Process character */
            c = *ptr;

            if (c < 0x80) {
                /* Single-byte character */
                if (c == (unsigned char)delimiter) {
                    /* Found delimiter, advance to next field */
                    ptr++;
                    current_field++;
                } else {
                    ptr++;
                }
            } else {
                /* Double-byte character - need 2 bytes */
                if (ptr[1] == '\0') {
                    /* Incomplete DBCS character */
                    output[0] = '\0';
                    return 1;
                }
                ptr += 2;
            }
        } while (current_field < field_num);
    }

    /* Now at the start of the desired field */
    if (ptr == NULL || *ptr == '\0') {
        output[0] = '\0';
        return 1;
    }

    /* Extract field content */
    return extract_field_until_delim((const char*)ptr, delimiter, max_size, output);
}

/*
 * Extract field until delimiter - FUN_00489fe0
 *
 * Binary analysis:
 * - Copies characters until delimiter or end of string
 * - Handles DBCS by treating lead byte + trail byte as unit
 * - Returns 0 if delimiter found, 1 if end of string
 */
int extract_field_until_delim(const char* src, char delimiter, int max_size, char* dest) {
    const unsigned char* ptr;
    unsigned char c;
    int i;

    if (!src || !dest || max_size <= 0) {
        if (dest && max_size > 0) {
            dest[0] = '\0';
        }
        return 1;
    }

    ptr = (const unsigned char*)src;
    i = 0;

    while (i < max_size - 1) {
        c = *ptr;

        if (c < 0x80) {
            /* Single-byte character */
            if (c == (unsigned char)delimiter) {
                /* Found delimiter - stop and null-terminate */
                dest[i] = '\0';
                return 0;  /* Success - delimiter found */
            }

            dest[i] = c;

            if (c == '\0') {
                /* End of string */
                return 1;
            }

            ptr++;
            i++;
        } else {
            /* Double-byte character */
            if (i + 1 >= max_size - 1) {
                /* Not enough room for full DBCS character */
                break;
            }

            /* Copy both bytes */
            dest[i] = c;
            dest[i + 1] = ptr[1];

            if (ptr[1] == '\0') {
                /* Incomplete DBCS at end */
                dest[i] = '\0';
                return 1;
            }

            ptr += 2;
            i += 2;
        }
    }

    /* Buffer full or max_size reached */
    dest[i] = '\0';
    return 1;
}

/*
 * Skip to Nth field in delimited string - DBCS aware
 */
const char* skip_to_field(const char* str, char delimiter, int field_num) {
    const unsigned char* ptr;
    int current_field;
    unsigned char c;

    if (!str || field_num < 1) {
        return NULL;
    }

    ptr = (const unsigned char*)str;
    current_field = 1;

    /* Already at first field */
    if (field_num == 1) {
        return (const char*)ptr;
    }

    /* Skip fields until we reach the target */
    while (*ptr && current_field < field_num) {
        c = *ptr;

        if (c < 0x80) {
            if (c == (unsigned char)delimiter) {
                current_field++;
            }
            ptr++;
        } else {
            /* DBCS character */
            if (ptr[1] == '\0') {
                break;
            }
            ptr += 2;
        }
    }

    if (current_field == field_num) {
        return (const char*)ptr;
    }

    return NULL;
}

/*
 * Count fields in delimited string - DBCS aware
 */
int count_fields(const char* str, char delimiter) {
    const unsigned char* ptr;
    int count;
    unsigned char c;

    if (!str || !*str) {
        return 0;
    }

    ptr = (const unsigned char*)str;
    count = 1;  /* At least one field if string is non-empty */

    while (*ptr) {
        c = *ptr;

        if (c < 0x80) {
            if (c == (unsigned char)delimiter) {
                count++;
            }
            ptr++;
        } else {
            /* DBCS character */
            if (ptr[1] == '\0') {
                break;
            }
            ptr += 2;
        }
    }

    return count;
}

/*
 * String to integer - FUN_00492973
 *
 * Binary analysis:
 * - Uses locale-aware character classification via FUN_00496796
 * - Skips leading whitespace (character type & 8)
 * - Handles + and - signs
 * - Parses decimal digits (character type & 4)
 * - Returns parsed integer
 */
int string_to_int(const char* str) {
    const unsigned char* ptr;
    int result;
    int sign;
    unsigned char c;

    if (!str) {
        return 0;
    }

    ptr = (const unsigned char*)str;

    /* Skip leading whitespace (space, tab, etc.) */
    /* Binary checks character type via lookup table or iswctype */
    while (*ptr) {
        c = *ptr;
        /* Check if whitespace: space (0x20), tab (0x09), etc. */
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            ptr++;
        } else {
            break;
        }
    }

    /* Handle sign */
    sign = 1;
    if (*ptr == '-') {
        sign = -1;
        ptr++;
    } else if (*ptr == '+') {
        ptr++;
    }

    /* Parse digits */
    result = 0;
    while (*ptr >= '0' && *ptr <= '9') {
        result = result * 10 + (*ptr - '0');
        ptr++;
    }

    return result * sign;
}

/*
 * Parse integer field - FUN_0048a050
 *
 * Binary analysis:
 * - Calls FUN_00489f70 to extract field (max 127 bytes)
 * - Returns -1 if field is empty
 * - Otherwise calls FUN_00492973 to convert to integer
 *
 * Used extensively in protocol parsing (100+ call sites)
 */
int parse_field_int(const char* str, char delimiter, int field_num) {
    char buffer[128];
    int result;

    /* Extract field with max 127 bytes */
    result = parse_text_field(str, delimiter, field_num, 127, buffer);

    if (result != 0 || buffer[0] == '\0') {
        /* Field not found or empty - return -1 */
        return -1;
    }

    /* Convert to integer */
    return string_to_int(buffer);
}

/*
 * Parse Base-62 encoded integer - FUN_0048a0a0
 *
 * Binary analysis:
 * - Base-62 encoding uses 0-9, a-z, A-Z (62 characters)
 * - Multiplies accumulator by 0x3e (62) each iteration
 * - Handles negative sign
 * - Used for bitmask values in protocol
 *
 * Character mapping:
 *   0-9 -> 0-9
 *   a-z -> 10-35 (binary: c - 'a' + 0xa, using -0x57 + c where 0x57='a')
 *   A-Z -> 36-61 (binary: c - 'A' + 36, using -0x1d + c where -0x1d=-29+65=36)
 */
int parse_base62(const char* str) {
    const char* ptr;
    int result;
    int sign;
    char c;

    if (!str || !*str) {
        return 0;
    }

    ptr = str;
    result = 0;
    sign = 1;

    while ((c = *ptr) != '\0') {
        result = result * 62;  /* 0x3e = 62 */

        if (c >= '0' && c <= '9') {
            result += c - '0';      /* 0-9 map to 0-9 */
        } else if (c >= 'a' && c <= 'z') {
            result += c - 'a' + 10; /* a-z map to 10-35 */
        } else if (c >= 'A' && c <= 'Z') {
            result += c - 'A' + 36; /* A-Z map to 36-61 */
        } else if (c == '-') {
            sign = -1;
        } else {
            return 0;  /* Invalid character */
        }

        ptr++;
    }

    return result * sign;
}

/*
 * Parse Base-62 encoded field - FUN_0048a120
 *
 * Binary analysis:
 * - Calls FUN_00489f70 to extract field
 * - Returns -1 if field is empty
 * - Otherwise calls FUN_0048a0a0 for Base-62 decoding
 */
int parse_field_base62(const char* str, char delimiter, int field_num) {
    char buffer[128];
    int result;

    /* Extract field with max 127 bytes */
    result = parse_text_field(str, delimiter, field_num, 127, buffer);

    if (result != 0 || buffer[0] == '\0') {
        /* Field not found or empty - return -1 */
        return -1;
    }

    /* Convert from Base-62 */
    return parse_base62(buffer);
}

/*
 * Unescape string - FUN_0048a170
 *
 * Binary analysis:
 * - Uses IsDBCSLeadByte for DBCS support
 * - Handles escape sequences starting with backslash
 * - Uses lookup table at DAT_004d5828 for escape character mapping
 *
 * Escape sequences from DAT_004d5829:
 *   Characters like 'n', 't', 'r', etc. map to control characters
 */
char* string_unescape(char* str) {
    static const char escape_chars[] = "nrt\\\"\'";  /* From DAT_004d5829 */
    static const char escape_values[] = "\n\r\t\\\"\'";  /* From DAT_004d5828 */

    char* src;
    char* dst;
    char c;
    int i;

    if (!str || !*str) {
        return str;
    }

    src = str;
    dst = str;

    while (*src) {
        /* Check for DBCS lead byte */
        if (IsDBCSLeadByte((unsigned char)*src)) {
            /* Copy DBCS character as-is */
            *dst++ = *src++;
            if (*src) {
                *dst++ = *src++;
            }
            continue;
        }

        c = *src++;

        if (c == '\\' && *src) {
            /* Escape sequence */
            char next = *src++;

            /* Look up escape character */
            for (i = 0; escape_chars[i]; i++) {
                if (escape_chars[i] == next) {
                    *dst++ = escape_values[i];
                    break;
                }
            }

            if (!escape_chars[i]) {
                /* Unknown escape - copy as-is */
                *dst++ = next;
            }
        } else {
            *dst++ = c;
        }
    }

    *dst = '\0';
    return str;
}
