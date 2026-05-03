/*
 * Stone Age Client - Protocol Utility Functions Header
 * Reverse engineered from sa_9061.exe
 *
 * FUN_00489f70 - Parse delimited text field (DBCS-aware)
 * FUN_00489fe0 - Extract field content until delimiter
 *
 * These functions handle DBCS (Double-Byte Character Set) strings,
 * supporting both single-byte ASCII and double-byte characters (e.g., Chinese).
 */

#ifndef PROTOCOL_UTIL_H
#define PROTOCOL_UTIL_H

#include "types.h"

/*
 * Parse delimited text field - FUN_00489f70
 * Extracts the Nth field from a delimited string with DBCS support.
 *
 * Parameters:
 *   str       - Source string to parse
 *   delimiter - Field delimiter character (e.g., '|')
 *   field_num - Field number to extract (1-indexed)
 *   max_size  - Maximum bytes to copy to output buffer
 *   output    - Output buffer for extracted field
 *
 * Returns:
 *   0 on success (field found and extracted)
 *   1 if field not found or string is empty
 *
 * Example:
 *   parse_text_field("A|B|C", '|', 2, 256, buffer) -> "B"
 */
int parse_text_field(const char* str, char delimiter, int field_num, int max_size, char* output);

/*
 * Extract field until delimiter - FUN_00489fe0
 * Copies characters from source to destination until delimiter or end of string.
 * Handles DBCS characters by treating them as single units.
 *
 * Parameters:
 *   src       - Source string position
 *   delimiter - Delimiter character
 *   max_size  - Maximum bytes to copy
 *   dest      - Output buffer
 *
 * Returns:
 *   0 if delimiter was found (field extracted successfully)
 *   1 if end of string reached (no delimiter found)
 */
int extract_field_until_delim(const char* src, char delimiter, int max_size, char* dest);

/*
 * Skip to Nth field in delimited string - DBCS aware
 * Returns pointer to the start of the Nth field, or NULL if not found.
 *
 * Parameters:
 *   str       - Source string
 *   delimiter - Field delimiter
 *   field_num - Field number to find (1-indexed)
 *
 * Returns:
 *   Pointer to start of field, or NULL if field doesn't exist
 */
const char* skip_to_field(const char* str, char delimiter, int field_num);

/*
 * Count fields in delimited string - DBCS aware
 *
 * Parameters:
 *   str       - Source string
 *   delimiter - Field delimiter
 *
 * Returns:
 *   Number of fields in the string
 */
int count_fields(const char* str, char delimiter);

/*
 * Check if byte is DBCS lead byte
 * Lead bytes are >= 0x80 in Shift-JIS and similar encodings
 */
#define IS_DBCS_LEAD(b) ((unsigned char)(b) >= 0x80)

/*
 * Parse integer from string - FUN_00492973
 * Locale-aware string to integer conversion.
 *
 * Parameters:
 *   str - String to parse
 *
 * Returns:
 *   Parsed integer value
 *
 * Notes:
 *   - Skips leading whitespace
 *   - Handles + and - signs
 *   - Stops at first non-digit character
 */
int string_to_int(const char* str);

/*
 * Parse integer field from delimited string - FUN_0048a050
 * Combines parse_text_field and string_to_int.
 *
 * Parameters:
 *   str       - Source string to parse
 *   delimiter - Field delimiter character
 *   field_num - Field number to extract (1-indexed)
 *
 * Returns:
 *   Parsed integer value, or -1 if field is empty or not found
 */
int parse_field_int(const char* str, char delimiter, int field_num);

/*
 * Parse Base-62 encoded integer - FUN_0048a0a0
 * Stone Age protocol uses Base-62 encoding for bitmask values.
 *
 * Parameters:
 *   str - Base-62 encoded string
 *
 * Returns:
 *   Decoded integer value
 *
 * Base-62 uses characters: 0-9, a-z, A-Z (62 characters)
 */
int parse_base62(const char* str);

/*
 * Parse Base-62 encoded field - FUN_0048a120
 * Combines parse_text_field and parse_base62.
 *
 * Parameters:
 *   str       - Source string to parse
 *   delimiter - Field delimiter character
 *   field_num - Field number to extract (1-indexed)
 *
 * Returns:
 *   Decoded integer value, or -1 if field is empty
 */
int parse_field_base62(const char* str, char delimiter, int field_num);

/*
 * Unescape string - FUN_0048a170
 * Processes escape sequences in protocol strings.
 *
 * Handles:
 *   \\ - backslash
 *   \n - newline (based on DAT_004d5828 lookup table)
 *   \t - tab
 *   etc.
 *
 * Parameters:
 *   str - String to unescape (modified in place)
 *
 * Returns:
 *   Pointer to the string
 */
char* string_unescape(char* str);

#endif /* PROTOCOL_UTIL_H */
