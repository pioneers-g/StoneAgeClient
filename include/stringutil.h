/*
 * Stone Age Client - String Utility Functions Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <stddef.h>

/*
 * Truncate string with ellipsis - FUN_0044a940
 * If the source string is longer than max_len, truncates and adds "..."
 */
void string_truncate(const char* src, char* dest, size_t max_len);

/*
 * Truncate string for display - FUN_0044a940 variant
 * Optimized for UI display names
 */
void string_truncate_display(const char* src, char* dest, size_t max_len);

/*
 * Safe string copy with DBCS support
 * Handles double-byte character sets properly
 */
void string_copy_dbcs(const char* src, char* dest, size_t max_len);

/*
 * Safe string length with DBCS support
 */
size_t string_len_dbcs(const char* str);

/*
 * Check if string contains only valid characters
 * Used for player names, pet names, etc.
 */
int string_is_valid_name(const char* str);

/*
 * Convert string to lowercase (ASCII only, preserves DBCS)
 */
void string_to_lower(char* str);

/*
 * Trim leading and trailing whitespace
 */
void string_trim(char* str);

/*
 * Format number with commas
 * e.g., 1234567 -> "1,234,567"
 */
void string_format_number(char* dest, size_t max_len, int value);

/*
 * Compare strings with DBCS support
 */
int string_compare_dbcs(const char* s1, const char* s2);

#endif /* STRINGUTIL_H */
