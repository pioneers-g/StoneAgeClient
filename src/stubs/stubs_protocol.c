/*
 * Stone Age Client - Protocol Stub Functions
 * Split from stubs.c to reduce file size
 */

#include <windows.h>
#include <string.h>
#include "types.h"

/* Forward declarations */
int string_to_int(const char* str);  /* implemented in protocol_util.c */
void FUN_0043dc40(char* dest, const char* src, int max_size);  /* implemented below */

/*
 * FUN_0043bf90 - Text Protocol Dispatcher
 *
 * Binary analysis:
 * - Main text-based protocol command dispatcher
 * - Commands are newline-delimited strings parsed at DAT_004ba014
 * - Uses FUN_0043dbe0 for initial parsing into local_400 buffer
 * - Compares first token against command strings at DAT_004b9xxx
 * - Routes to handler functions with parsed parameters
 *
 * Command categories (50+ commands total):
 * - Battle commands: FUN_00465400, FUN_00463e70, FUN_00464e10
 * - Party commands: FUN_004643f0, FUN_00464610, FUN_00464ef0, FUN_00464670
 * - Guild commands: FUN_00464650, FUN_00465170, FUN_00464af0, FUN_00463f00
 * - Trade commands: FUN_00464190, FUN_00463790, FUN_00462010, FUN_00462200
 * - Chat commands: FUN_00462590, FUN_00462f60, FUN_00463380, FUN_00465460
 * - NPC commands: FUN_0045ffb0, FUN_00463ee0, FUN_00464ee0, FUN_00463d80
 * - Login commands: FUN_0045fa40, FUN_00463c20, FUN_00463d20, FUN_0045fdc0
 * - Character commands: FUN_0045fb80, FUN_0045ff50, FUN_00465440
 * - Mail commands: FUN_0045a9a0, FUN_00465470
 *
 * Uses FUN_0043dd50 to parse integer parameters
 * Uses FUN_0043dd70 to parse string parameters
 * Uses FUN_0043e1b0 for string unescaping/processing
 */
void FUN_0043bf90(const char* param_1) {
    (void)param_1;
    /* TODO: Full implementation with command dispatch */
}

/*
 * FUN_0048d3e0 - Binary Protocol Dispatcher
 *
 * Binary analysis:
 * - Main binary protocol command dispatcher
 * - Dispatches to handler functions based on opcode
 * - Uses anti-tamper checksum validation
 *
 * Process:
 * 1. Initialize via FUN_00492d80(), FUN_0048c720(), FUN_0048c5a0()
 * 2. Get packet count via FUN_0048c7f0()
 * 3. For each packet, read opcode from stack (in_stack_00000024)
 * 4. Validate checksum: sum of parameters must equal return address
 * 5. Dispatch to appropriate handler
 *
 * Opcodes (partial list):
 * - 0x02: Battle action (FUN_00465400)
 * - 0x04: Party update (FUN_00463e70)
 * - 0x07: Pet update (FUN_00464e10)
 * - 0x0C: Party member join (FUN_004643f0)
 * - 0x0D: Party member leave (FUN_00464610)
 * - 0x0F: Friend update (FUN_00465460, FUN_00464ef0)
 * - 0x18: Guild member update (FUN_00464650)
 * - 0x1A: Guild info (FUN_00465170)
 * - 0x1C: Guild member list (FUN_00464af0)
 * - 0x2A: NPC dialog (FUN_00462f60)
 * - 0x48: Login server response (FUN_0045fa40)
 * - 0x50: Character list response (FUN_0045fb80)
 * - 0x5C: Mail receive (FUN_0045a9a0)
 * - 0x75: Shop protocol (FUN_00465d20)
 * - 0x78: Skill protocol (FUN_00465de0)
 * - 0x85: Guild member data (FUN_004665b0)
 * - 0x86: Guild info (FUN_00466550)
 */
int FUN_0048d3e0(int param_1) {
    (void)param_1;
    /* TODO: Full binary protocol dispatcher implementation */
    return 0;
}

/*
 * FUN_0043dbe0 - Parse Integer Parameter with Default
 *
 * Binary analysis:
 * - Parses integer from protocol buffer, returns default if not present
 * - param_1: output integer pointer
 * - param_2: destination buffer for string copy
 * - param_3: buffer size
 * - param_4: source data (2 dwords: [0]=value or 0, [1]=string or 0)
 *
 * Process:
 * - If param_4[0] and param_4[1] are both non-zero:
 *   - Convert param_4[0] to integer via FUN_004930a0 (base 10)
 *   - Copy string via FUN_0043dc40
 * - Otherwise: set output to 0 and copy default string from DAT_004e220c
 */
void FUN_0043dbe0(int* output, char* dest, int dest_size, int* source) {
    static const char* default_string = "";  /* DAT_004e220c */

    if (output == NULL || dest == NULL || dest_size <= 0) {
        return;
    }

    if (source && source[0] != 0 && source[1] != 0) {
        const char* src_str = (const char*)(intptr_t)source[1];
        /* Parse integer from source string */
        *output = string_to_int(src_str);
        /* Copy string to destination */
        FUN_0043dc40(dest, src_str, dest_size);
    } else {
        /* Use default values */
        *output = 0;
        FUN_0043dc40(dest, default_string, dest_size);
    }
}

/*
 * FUN_0043dc40 - String Copy with Length Limit
 *
 * Binary analysis:
 * - Copies string with length limit (null-terminated)
 * - param_1: destination buffer
 * - param_2: source string
 * - param_3: maximum size including null terminator
 * - Copies until null byte or max_size-1 characters
 * - Always null-terminates the result
 */
void FUN_0043dc40(char* dest, const char* src, int max_size) {
    int i = 0;

    if (max_size <= 1) {
        if (max_size == 1) {
            dest[0] = '\0';
        }
        return;
    }

    /* Copy characters until null or max_size-1 */
    while (i < max_size - 1 && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }

    /* Always null-terminate */
    dest[i] = '\0';
}

/*
 * FUN_0043dc90 - Safe String Append with Buffer Limit
 *
 * Binary analysis:
 * - Appends source string to destination with buffer size limit
 * - param_1: destination buffer
 * - param_2: source string
 * - param_3: buffer size (including null terminator)
 * - Finds end of destination string first
 * - Appends source until buffer limit reached or source exhausted
 * - Always null-terminates the result
 */
void FUN_0043dc90(char* dest, const char* src, int max_size) {
    int dest_len = 0;
    int i;

    if (max_size <= 1) return;

    max_size--;

    /* Find end of destination */
    while (dest[dest_len] != '\0') {
        dest_len++;
        if (dest_len >= max_size) return;
    }

    /* Append source */
    for (i = 0; i < max_size - dest_len && src[i] != '\0'; i++) {
        dest[dest_len + i] = src[i];
    }
    dest[dest_len + i] = '\0';
}

/*
 * FUN_0043ded0 - String Extract with Escape Handling
 *
 * Binary analysis:
 * - Extracts string with escape sequence handling
 * - param_1: source string
 * - Returns pointer to static buffer DAT_004b9ff8
 * - Handles DBCS characters and escape sequences
 *
 * Escape sequences:
 * - \S -> space (0x20)
 * - \n -> newline (0x0A)
 * - \r -> carriage return (0x0D)
 * - \\ -> backslash (0x5C)
 */
char* FUN_0043ded0(const char* source) {
    static char result_buffer[4096];  /* DAT_004b9ff8 - static result buffer */
    int src_idx = 0;
    int dst_idx = 0;
    char c;

    result_buffer[0] = '\0';

    if (!source || !*source) {
        return result_buffer;
    }

    while ((c = source[src_idx]) != '\0') {
        /* Check for DBCS lead byte */
        if (IsDBCSLeadByte((unsigned char)c)) {
            /* Copy DBCS character as-is */
            result_buffer[dst_idx] = c;
            if (source[src_idx + 1] == '\0') {
                result_buffer[dst_idx] = '\0';
                return result_buffer;
            }
            result_buffer[dst_idx + 1] = source[src_idx + 1];
            src_idx += 2;
            dst_idx += 2;
        } else if (c == '\\') {
            /* Escape sequence */
            char next = source[src_idx + 1];
            if (next == '\0') {
                result_buffer[dst_idx] = '\\';
                result_buffer[dst_idx + 1] = '\0';
                return result_buffer;
            }
            switch (next) {
                case 'S': result_buffer[dst_idx] = ' '; break;   /* Space */
                case 'n': result_buffer[dst_idx] = '\n'; break;  /* Newline */
                case 'r': result_buffer[dst_idx] = '\r'; break;  /* Carriage return */
                case '\\': result_buffer[dst_idx] = '\\'; break; /* Backslash */
                default: result_buffer[dst_idx] = '\\'; break;   /* Unknown escape */
            }
            src_idx += 2;
            dst_idx++;
        } else {
            /* Normal character */
            result_buffer[dst_idx] = c;
            src_idx++;
            dst_idx++;
        }

        if (source[src_idx] == '\0') {
            result_buffer[dst_idx] = '\0';
            return result_buffer;
        }
    }

    result_buffer[dst_idx] = '\0';
    return result_buffer;
}

/*
 * FUN_0043dd50 - Parse String Field
 *
 * Binary analysis:
 * - Parses string field from protocol buffer
 * - param_1: source pointer (or 0 for empty)
 * - Returns pointer to parsed string
 * - If param_1 == 0: copies default from DAT_004e220c to DAT_004b9ff8
 * - Otherwise: calls FUN_0043ded0 to extract field
 */
const char* FUN_0043dd50(const char* source) {
    static char default_buffer[256];  /* DAT_004b9ff8 */

    if (source == NULL || *source == '\0') {
        /* Return empty string */
        default_buffer[0] = '\0';
        return default_buffer;
    }

    return FUN_0043ded0(source);
}

/*
 * FUN_0043dd70 - Get String Field with Default
 *
 * Binary analysis:
 * - Similar to FUN_0043dd50 but with default handling
 * - param_1: source pointer (or 0 for default)
 * - Returns pointer to result string
 * - If param_1 == 0: copies default string
 * - Otherwise: extracts via FUN_0043ded0
 */
const char* FUN_0043dd70(const char* source) {
    static char default_buffer[256];

    if (source == NULL || *source == '\0') {
        default_buffer[0] = '\0';
        return default_buffer;
    }

    return FUN_0043ded0(source);
}

/*
 * FUN_0043e170 - String Copy
 *
 * Binary analysis:
 * - Copies formatted string to destination
 * - Uses FUN_0043e0f0 to process format string
 * - Uses FUN_004923a7 for string formatting
 */
void FUN_0043e170(char* dest, const char* src) {
    (void)dest; (void)src;
    /* TODO: Full implementation */
}

/*
 * FUN_0043e1b0 - String Field Copy
 *
 * Binary analysis:
 * - Simple wrapper for FUN_0043dc40
 * - param_1: destination buffer
 * - param_2: buffer size
 * - param_3: source string
 * - Returns param_1
 */
char* FUN_0043e1b0(char* dest, int size, const char* src) {
    if (dest && src && size > 0) {
        FUN_0043dc40(dest, src, size);
    }
    return dest;
}

/*
 * FUN_0048a0a0 - Base-62 String to Integer
 *
 * Binary analysis:
 * - Converts Base-62 encoded string to integer
 * - param_1: null-terminated string
 * - Returns: decoded integer value
 *
 * Base-62 character set:
 * - '0'-'9': values 0-9
 * - 'a'-'z': values 10-35
 * - 'A'-'Z': values 36-61
 * - '-': negative sign (only at start)
 *
 * Formula: result = result * 62 + digit_value
 * Used for bitmask encoding in protocol messages
 */
int FUN_0048a0a0(const char* str) {
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
 * FUN_0048a170 - String Unescape Processing
 *
 * Binary analysis:
 * - Processes escape sequences in protocol strings
 * - param_1: string to process (in-place)
 * - Returns pointer to processed string
 *
 * Escape sequences (backslash-prefixed):
 * - DAT_004d5829 contains escape characters to find
 * - DAT_004d5828 contains replacement characters
 * - Table has 4 entries (8 bytes total)
 *
 * Process:
 * 1. Calculate string length
 * 2. Iterate through characters
 * 3. For DBCS lead bytes: copy both bytes as-is
 * 4. For backslash: look up next char in escape table
 *    - If found: replace with corresponding character from DAT_004d5828
 *    - If not found: copy the character as-is
 * 5. Null-terminate the result
 *
 * Common escape sequences:
 * - \n -> newline
 * - \t -> tab
 * - \\ -> backslash
 * - \| -> pipe (field delimiter)
 */
char* FUN_0048a170(char* str) {
    /* Escape table from DAT_004d5829 (chars to find) and DAT_004d5828 (replacement) */
    static const char escape_find[] = { 'n', 'r', 't', '\\', '"', '\'', '|', '\0' };
    static const char escape_repl[] = { '\n', '\r', '\t', '\\', '"', '\'', '|', '\0' };

    char* src;
    char* dst;
    char c;
    int i;
    int len;

    if (!str || !*str) {
        *str = '\0';
        return str;
    }

    /* Calculate string length first (binary does this) */
    len = 0;
    while (str[len] != '\0') {
        len++;
    }

    if (len < 1) {
        *str = '\0';
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
            /* Escape sequence - look up in table */
            char next = *src++;

            for (i = 0; escape_find[i] != '\0'; i++) {
                if (escape_find[i] == next) {
                    *dst++ = escape_repl[i];
                    break;
                }
            }

            if (escape_find[i] == '\0') {
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

/* Protocol send stub */
void protocol_send_text_command(const char* cmd) { (void)cmd; }

/*
 * FUN_0048a050 - Get Integer Field from Delimited String
 *
 * Binary analysis:
 * - Extracts a field and converts to integer
 * - param_1: source string
 * - param_2: delimiter character
 * - param_3: field index (1-based)
 * - Returns: integer value, or -1 if field empty/not found
 * - Uses FUN_00489f70 for extraction, FUN_004929fe for conversion
 */
int FUN_0048a050(const char* str, char delimiter, int field_index) {
    char buffer[128];
    const char* ptr;
    int current_field;
    unsigned char c;
    int i;

    if (!str || field_index < 1) {
        return -1;
    }

    /* Skip to the desired field (field_index is 1-based) */
    ptr = str;
    current_field = 1;

    while (current_field < field_index && *ptr) {
        c = (unsigned char)*ptr;

        if (c < 0x80) {
            /* Single-byte character */
            if (c == (unsigned char)delimiter) {
                current_field++;
            }
            ptr++;
        } else {
            /* Double-byte character */
            if (ptr[1] == '\0') {
                break;
            }
            ptr += 2;
        }
    }

    if (!*ptr) {
        return -1;  /* Field not found */
    }

    /* Extract field content until delimiter */
    i = 0;
    while (i < 127 && *ptr) {
        c = (unsigned char)*ptr;

        if (c < 0x80) {
            if (c == (unsigned char)delimiter) {
                break;
            }
            buffer[i++] = *ptr++;
        } else {
            /* DBCS character */
            if (i + 1 >= 127 || ptr[1] == '\0') {
                break;
            }
            buffer[i++] = *ptr++;
            buffer[i++] = *ptr++;
        }
    }

    buffer[i] = '\0';

    if (buffer[0] == '\0') {
        return -1;  /* Empty field */
    }

    /* Convert to integer */
    return string_to_int(buffer);
}

/*
 * FUN_00489fe0 - Copy String with DBCS Support Until Delimiter
 *
 * Binary analysis:
 * - Copies string handling DBCS (double-byte character set)
 * - param_1: source string
 * - param_2: delimiter character (stops copying if found)
 * - param_3: maximum length
 * - param_4: destination buffer
 * - Returns 0 if delimiter found, 1 if reached end/max length
 * - DBCS characters (lead byte >= 0x80) are copied as pairs
 * - Null-terminates the result
 */
int FUN_00489fe0(const char* src, char delimiter, int max_len, char* dst) {
    (void)src; (void)delimiter; (void)max_len; (void)dst;
    return 1;
}
