/*
 * Stone Age Client - String Utility Functions
 * Reverse engineered from sa_9061.exe
 * FUN_0044a940 - String truncation with ellipsis
 */

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "stringutil.h"

/*
 * Truncate string with ellipsis - FUN_0044a940
 * If the source string is longer than max_len, truncates and adds "..."
 *
 * Parameters:
 *   src     - Source string
 *   dest    - Destination buffer
 *   max_len - Maximum characters to copy (excluding null terminator)
 *
 * The function handles two truncation modes:
 * - If there's room for "...%s..." format, uses that
 * - Otherwise uses a simpler truncation format
 */
void string_truncate(const char* src, char* dest, size_t max_len) {
    size_t src_len;
    int room_for_ellipsis;

    if (!src || !dest || max_len == 0) {
        if (dest && max_len > 0) {
            dest[0] = '\0';
        }
        return;
    }

    src_len = strlen(src);

    /* If source fits, copy directly */
    if (src_len <= max_len) {
        strncpy(dest, src, max_len);
        dest[max_len] = '\0';
        return;
    }

    /* Calculate room for ellipsis handling */
    room_for_ellipsis = (int)(max_len - (src_len - max_len)) / 2;

    if (room_for_ellipsis > 0) {
        /* Use "...%s..." format - show start and end with ellipsis in middle */
        size_t prefix_len = max_len / 2;
        size_t suffix_len = max_len - prefix_len;

        /* Copy prefix */
        strncpy(dest, src, prefix_len);
        dest[prefix_len] = '\0';

        /* Add ellipsis */
        strcat(dest, "...");

        /* Add suffix from end of source */
        if (suffix_len > 0) {
            strncat(dest, src + src_len - suffix_len, suffix_len);
        }
    } else {
        /* Simple truncation - just copy what fits */
        strncpy(dest, src, max_len - 3);
        dest[max_len - 3] = '\0';
        strcat(dest, "...");
    }
}

/*
 * Truncate string for display - FUN_0044a940 variant
 * Similar to string_truncate but optimized for UI display
 *
 * This matches the original binary behavior where:
 * - If the string fits, copy as-is
 * - If truncation needed, check if we can show middle ellipsis
 * - Otherwise show beginning with trailing ellipsis
 */
void string_truncate_display(const char* src, char* dest, size_t max_len) {
    size_t src_len;
    size_t copy_len;

    if (!src || !dest || max_len == 0) {
        if (dest && max_len > 0) {
            dest[0] = '\0';
        }
        return;
    }

    src_len = strlen(src);

    /* If source fits within limit, copy directly */
    if (src_len <= max_len) {
        strncpy(dest, src, max_len);
        dest[max_len] = '\0';
        return;
    }

    /* Check if we have room for ellipsis in the middle */
    if (max_len >= 6) {
        /* Show first half and last half with "..." in middle */
        size_t half = (max_len - 3) / 2;

        /* Copy first half */
        strncpy(dest, src, half);
        dest[half] = '\0';

        /* Add ellipsis */
        strcat(dest, "...");

        /* Add last part */
        strcat(dest, src + src_len - (max_len - half - 3));
    } else {
        /* Very short max_len - just truncate and add ellipsis */
        copy_len = max_len > 3 ? max_len - 3 : 0;
        strncpy(dest, src, copy_len);
        dest[copy_len] = '\0';
        if (max_len > 3) {
            strcat(dest, "...");
        }
    }
}

/*
 * Safe string copy with DBCS support
 * Handles double-byte character sets properly
 */
void string_copy_dbcs(const char* src, char* dest, size_t max_len) {
    size_t i = 0;

    if (!src || !dest || max_len == 0) {
        if (dest && max_len > 0) {
            dest[0] = '\0';
        }
        return;
    }

    while (src[i] && i < max_len - 1) {
        if (IsDBCSLeadByte((BYTE)src[i])) {
            /* Double-byte character - need 2 bytes */
            if (i + 1 >= max_len - 1) {
                /* Not enough room for full character */
                break;
            }
            dest[i] = src[i];
            dest[i + 1] = src[i + 1];
            i += 2;
        } else {
            dest[i] = src[i];
            i++;
        }
    }

    dest[i] = '\0';
}

/*
 * Safe string length with DBCS support
 */
size_t string_len_dbcs(const char* str) {
    size_t len = 0;

    if (!str) return 0;

    while (*str) {
        if (IsDBCSLeadByte((BYTE)*str)) {
            str += 2;
        } else {
            str++;
        }
        len++;
    }

    return len;
}

/*
 * Check if string contains only valid characters
 * Used for player names, pet names, etc.
 */
int string_is_valid_name(const char* str) {
    if (!str || !*str) return 0;

    while (*str) {
        if (IsDBCSLeadByte((BYTE)*str)) {
            /* Double-byte character - accept */
            str += 2;
        } else if (*str >= 0x20 && *str < 0x7f) {
            /* Printable ASCII */
            str++;
        } else {
            return 0;
        }
    }

    return 1;
}

/*
 * Convert string to lowercase (ASCII only, preserves DBCS)
 */
void string_to_lower(char* str) {
    if (!str) return;

    while (*str) {
        if (IsDBCSLeadByte((BYTE)*str)) {
            str += 2;
        } else if (*str >= 'A' && *str <= 'Z') {
            *str = *str - 'A' + 'a';
            str++;
        } else {
            str++;
        }
    }
}

/*
 * Trim leading and trailing whitespace
 */
void string_trim(char* str) {
    char* start;
    char* end;

    if (!str || !*str) return;

    /* Find first non-space */
    start = str;
    while (*start == ' ' || *start == '\t') {
        start++;
    }

    /* Find last non-space */
    end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t')) {
        end--;
    }

    /* Move trimmed string to beginning */
    if (start != str) {
        memmove(str, start, end - start + 1);
    }

    /* Null terminate */
    str[end - start + 1] = '\0';
}

/*
 * Format number with commas
 * e.g., 1234567 -> "1,234,567"
 */
void string_format_number(char* dest, size_t max_len, int value) {
    char buffer[32];
    int i, j, len, comma_count;

    if (!dest || max_len == 0) return;

    /* Convert to string */
    snprintf(buffer, sizeof(buffer), "%d", value < 0 ? -value : value);
    len = strlen(buffer);

    /* Calculate number of commas needed */
    comma_count = (len - 1) / 3;

    /* Check if it fits */
    if (len + comma_count >= (int)max_len) {
        strncpy(dest, buffer, max_len - 1);
        dest[max_len - 1] = '\0';
        return;
    }

    /* Add commas */
    i = len - 1;
    j = len + comma_count - 1;
    dest[len + comma_count] = '\0';

    if (value < 0) {
        *dest++ = '-';
    }

    while (i >= 0) {
        dest[j--] = buffer[i--];
        if (i >= 0 && (len - 1 - i) % 3 == 2) {
            dest[j--] = ',';
        }
    }
}

/*
 * Compare strings with DBCS support
 */
int string_compare_dbcs(const char* s1, const char* s2) {
    if (!s1 || !s2) return (s1 == s2) ? 0 : (s1 ? 1 : -1);

    while (*s1 && *s2) {
        if (IsDBCSLeadByte((BYTE)*s1) && IsDBCSLeadByte((BYTE)*s2)) {
            /* Compare both bytes of DBCS character */
            if (*s1 != *s2 || *(s1 + 1) != *(s2 + 1)) {
                return (unsigned char)*s1 - (unsigned char)*s2;
            }
            s1 += 2;
            s2 += 2;
        } else if (IsDBCSLeadByte((BYTE)*s1) || IsDBCSLeadByte((BYTE)*s2)) {
            /* One is DBCS, other is not */
            return (unsigned char)*s1 - (unsigned char)*s2;
        } else {
            /* Both are single-byte */
            if (*s1 != *s2) {
                return (unsigned char)*s1 - (unsigned char)*s2;
            }
            s1++;
            s2++;
        }
    }

    return (unsigned char)*s1 - (unsigned char)*s2;
}
