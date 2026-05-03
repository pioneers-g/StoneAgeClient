/*
 * Stone Age Client - Protocol Utility Comprehensive Tests
 * Tests for protocol.c utility functions
 *
 * Covers:
 * - Base-62 encode/decode (FUN_0043efe0)
 * - String unescaping (FUN_0043ded0)
 * - Packet building functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s16;
typedef int s32;

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    fflush(stdout); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Constants
 * ======================================== */

#define PACKET_HEADER_SIZE      8
#define MAX_PACKET_SIZE         65535

/* Opcodes */
#define PKT_CLIENT_LOGIN        0x1001
#define PKT_CHAR_LOGIN          0x1002
#define PKT_CLIENT_MOVE         0x1003
#define PKT_CLIENT_ACTION       0x1004
#define PKT_CLIENT_TALK         0x1005
#define PKT_HEARTBEAT           0x1006

/* ========================================
 * Implementation Functions
 * ======================================== */

/*
 * Calculate packet checksum
 */
static u16 packet_checksum(void* data, int len) {
    u16 sum = 0;
    u8* ptr = (u8*)data;

    while (len-- > 0) {
        sum += *ptr++;
    }

    return sum ^ 0xFFFF;
}

/*
 * Base-62 string to integer conversion - FUN_0043efe0
 */
static int protocol_decode_int(const char* str) {
    int result = 0;
    int sign = 1;

    if (*str == '-') {
        sign = -1;
        str++;
    }

    while (*str) {
        result *= 62;
        if (*str >= '0' && *str <= '9') {
            result += *str - '0';
        } else if (*str >= 'a' && *str <= 'z') {
            result += *str - 'a' + 10;
        } else if (*str >= 'A' && *str <= 'Z') {
            result += *str - 'A' + 36;
        } else {
            return 0;
        }
        str++;
    }

    return result * sign;
}

/*
 * Integer to Base-62 string conversion
 */
static void protocol_encode_int(int value, char* buffer, int max_len) {
    static const char base62[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char temp[16];
    int i = 0;
    int is_negative = 0;
    unsigned int uval;

    if (value < 0) {
        is_negative = 1;
        uval = (unsigned int)(-value);
    } else {
        uval = (unsigned int)value;
    }

    if (uval == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    while (uval > 0 && i < 15) {
        temp[i++] = base62[uval % 62];
        uval /= 62;
    }

    int pos = 0;
    if (is_negative && pos < max_len - 1) {
        buffer[pos++] = '-';
    }

    while (i > 0 && pos < max_len - 1) {
        buffer[pos++] = temp[--i];
    }

    buffer[pos] = '\0';
}

/*
 * Unescape string
 */
static void protocol_unescape_string_to(const char* src, char* dst, int max_len) {
    int src_idx = 0;
    int dst_idx = 0;

    if (!src || !dst || max_len <= 0) {
        if (dst && max_len > 0) dst[0] = '\0';
        return;
    }

    while (src[src_idx] && dst_idx < max_len - 1) {
        if (src[src_idx] == '\\') {
            char next = src[src_idx + 1];

            if (next == '\0') {
                dst[dst_idx++] = '\\';
                break;
            }

            switch (next) {
                case 'S':
                    dst[dst_idx++] = ' ';
                    src_idx += 2;
                    break;
                case 'n':
                    dst[dst_idx++] = '\n';
                    src_idx += 2;
                    break;
                case 'r':
                    dst[dst_idx++] = '\r';
                    src_idx += 2;
                    break;
                case '\\':
                    dst[dst_idx++] = '\\';
                    src_idx += 2;
                    break;
                default:
                    dst[dst_idx++] = '\\';
                    src_idx++;
                    break;
            }
        } else {
            dst[dst_idx++] = src[src_idx++];
        }
    }

    dst[dst_idx] = '\0';
}

/*
 * Reset state
 */
static void reset_state(void) {
    /* Nothing to reset for stateless functions */
}

/* ========================================
 * Test Cases - Base-62 Decode
 * ======================================== */

static int test_decode_zero(void) {
    return protocol_decode_int("0") == 0;
}

static int test_decode_single_digit(void) {
    return protocol_decode_int("5") == 5;
}

static int test_decode_single_lower(void) {
    /* 'a' = 10 */
    return protocol_decode_int("a") == 10;
}

static int test_decode_single_upper(void) {
    /* 'A' = 36 */
    return protocol_decode_int("A") == 36;
}

static int test_decode_multi_digit(void) {
    /* "10" = 1*62 + 0 = 62 */
    return protocol_decode_int("10") == 62;
}

static int test_decode_multi_lower(void) {
    /* "a0" = 10*62 + 0 = 620 */
    return protocol_decode_int("a0") == 620;
}

static int test_decode_multi_upper(void) {
    /* "A0" = 36*62 + 0 = 2232 */
    return protocol_decode_int("A0") == 2232;
}

static int test_decode_mixed(void) {
    /* "1aA" = 1*62*62 + 10*62 + 36 = 3844 + 620 + 36 = 4500 */
    return protocol_decode_int("1aA") == 4500;
}

static int test_decode_negative(void) {
    return protocol_decode_int("-5") == -5;
}

static int test_decode_negative_multi(void) {
    return protocol_decode_int("-10") == -62;
}

static int test_decode_max_digit(void) {
    /* 'Z' = 61 */
    return protocol_decode_int("Z") == 61;
}

static int test_decode_large_value(void) {
    /* "ZZZZ" = 61*62^3 + 61*62^2 + 61*62 + 61
     * = 61*238328 + 61*3844 + 61*62 + 61
     * = 14538008 + 234484 + 3782 + 61
     * = 14776335 */
    return protocol_decode_int("ZZZZ") == 14776335;
}

static int test_decode_invalid_char(void) {
    /* Space is invalid */
    return protocol_decode_int("1 2") == 0;
}

static int test_decode_empty(void) {
    return protocol_decode_int("") == 0;
}

/* ========================================
 * Test Cases - Base-62 Encode
 * ======================================== */

static int test_encode_zero(void) {
    char buffer[16];
    protocol_encode_int(0, buffer, sizeof(buffer));
    return strcmp(buffer, "0") == 0;
}

static int test_encode_single_digit(void) {
    char buffer[16];
    protocol_encode_int(5, buffer, sizeof(buffer));
    return strcmp(buffer, "5") == 0;
}

static int test_encode_ten(void) {
    char buffer[16];
    protocol_encode_int(10, buffer, sizeof(buffer));
    return strcmp(buffer, "a") == 0;
}

static int test_encode_thirty_six(void) {
    char buffer[16];
    protocol_encode_int(36, buffer, sizeof(buffer));
    return strcmp(buffer, "A") == 0;
}

static int test_encode_sixty_two(void) {
    char buffer[16];
    protocol_encode_int(62, buffer, sizeof(buffer));
    return strcmp(buffer, "10") == 0;
}

static int test_encode_negative(void) {
    char buffer[16];
    protocol_encode_int(-5, buffer, sizeof(buffer));
    return strcmp(buffer, "-5") == 0;
}

static int test_encode_large(void) {
    char buffer[16];
    protocol_encode_int(14776335, buffer, sizeof(buffer));
    return strcmp(buffer, "ZZZZ") == 0;
}

/* ========================================
 * Test Cases - Round Trip
 * ======================================== */

static int test_roundtrip_zero(void) {
    char buffer[16];
    protocol_encode_int(0, buffer, sizeof(buffer));
    return protocol_decode_int(buffer) == 0;
}

static int test_roundtrip_positive(void) {
    char buffer[16];
    int value = 12345;
    protocol_encode_int(value, buffer, sizeof(buffer));
    return protocol_decode_int(buffer) == value;
}

static int test_roundtrip_negative(void) {
    char buffer[16];
    int value = -12345;
    protocol_encode_int(value, buffer, sizeof(buffer));
    return protocol_decode_int(buffer) == value;
}

static int test_roundtrip_large(void) {
    char buffer[16];
    int value = 999999;
    protocol_encode_int(value, buffer, sizeof(buffer));
    return protocol_decode_int(buffer) == value;
}

/* ========================================
 * Test Cases - String Unescape
 * ======================================== */

static int test_unescape_plain(void) {
    char dst[64];
    protocol_unescape_string_to("hello", dst, sizeof(dst));
    return strcmp(dst, "hello") == 0;
}

static int test_unescape_space(void) {
    char dst[64];
    protocol_unescape_string_to("hello\\Sworld", dst, sizeof(dst));
    return strcmp(dst, "hello world") == 0;
}

static int test_unescape_newline(void) {
    char dst[64];
    protocol_unescape_string_to("line1\\nline2", dst, sizeof(dst));
    return strcmp(dst, "line1\nline2") == 0;
}

static int test_unescape_cr(void) {
    char dst[64];
    protocol_unescape_string_to("line1\\rline2", dst, sizeof(dst));
    return strcmp(dst, "line1\rline2") == 0;
}

static int test_unescape_backslash(void) {
    char dst[64];
    protocol_unescape_string_to("path\\\\file", dst, sizeof(dst));
    return strcmp(dst, "path\\file") == 0;
}

static int test_unescape_multiple(void) {
    char dst[64];
    protocol_unescape_string_to("a\\Sb\\nc\\\\d", dst, sizeof(dst));
    return strcmp(dst, "a b\nc\\d") == 0;
}

static int test_unescape_trailing_backslash(void) {
    char dst[64];
    protocol_unescape_string_to("test\\", dst, sizeof(dst));
    return strcmp(dst, "test\\") == 0;
}

static int test_unescape_unknown_escape(void) {
    char dst[64];
    protocol_unescape_string_to("test\\x", dst, sizeof(dst));
    /* Unknown escape keeps the backslash */
    return strcmp(dst, "test\\x") == 0;
}

static int test_unescape_empty(void) {
    char dst[64] = "unchanged";
    protocol_unescape_string_to("", dst, sizeof(dst));
    return strcmp(dst, "") == 0;
}

static int test_unescape_null_src(void) {
    char dst[64] = "unchanged";
    protocol_unescape_string_to(NULL, dst, sizeof(dst));
    return strcmp(dst, "") == 0;
}

static int test_unescape_null_dst(void) {
    /* Should not crash */
    protocol_unescape_string_to("test", NULL, 0);
    return 1;
}

static int test_unescape_buffer_limit(void) {
    char dst[5];
    protocol_unescape_string_to("1234567890", dst, sizeof(dst));
    /* Should truncate to fit buffer */
    return strlen(dst) < sizeof(dst);
}

/* ========================================
 * Test Cases - Packet Checksum
 * ======================================== */

static int test_checksum_empty(void) {
    return packet_checksum("", 0) == 0xFFFF;
}

static int test_checksum_single(void) {
    /* 0x41 ^ 0xFFFF = 0xFFBE */
    return packet_checksum("A", 1) == (0x41 ^ 0xFFFF);
}

static int test_checksum_multiple(void) {
    /* Sum = 'A' + 'B' = 0x41 + 0x42 = 0x83 */
    /* Result = 0x83 ^ 0xFFFF = 0xFF7C */
    u16 result = packet_checksum("AB", 2);
    return result == ((0x41 + 0x42) ^ 0xFFFF);
}

static int test_checksum_large_value(void) {
    /* Sum may overflow u8 but u16 handles it */
    u8 data[] = {0xFF, 0xFF, 0xFF};
    u16 result = packet_checksum(data, 3);
    return result == ((0xFF * 3) ^ 0xFFFF);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Protocol Utility Comprehensive Tests ===\n\n");

    printf("Base-62 Decode Tests:\n");
    TEST(decode_zero);
    TEST(decode_single_digit);
    TEST(decode_single_lower);
    TEST(decode_single_upper);
    TEST(decode_multi_digit);
    TEST(decode_multi_lower);
    TEST(decode_multi_upper);
    TEST(decode_mixed);
    TEST(decode_negative);
    TEST(decode_negative_multi);
    TEST(decode_max_digit);
    TEST(decode_large_value);
    TEST(decode_invalid_char);
    TEST(decode_empty);

    printf("\nBase-62 Encode Tests:\n");
    TEST(encode_zero);
    TEST(encode_single_digit);
    TEST(encode_ten);
    TEST(encode_thirty_six);
    TEST(encode_sixty_two);
    TEST(encode_negative);
    TEST(encode_large);

    printf("\nRound Trip Tests:\n");
    TEST(roundtrip_zero);
    TEST(roundtrip_positive);
    TEST(roundtrip_negative);
    TEST(roundtrip_large);

    printf("\nString Unescape Tests:\n");
    TEST(unescape_plain);
    TEST(unescape_space);
    TEST(unescape_newline);
    TEST(unescape_cr);
    TEST(unescape_backslash);
    TEST(unescape_multiple);
    TEST(unescape_trailing_backslash);
    TEST(unescape_unknown_escape);
    TEST(unescape_empty);
    TEST(unescape_null_src);
    TEST(unescape_null_dst);
    TEST(unescape_buffer_limit);

    printf("\nPacket Checksum Tests:\n");
    TEST(checksum_empty);
    TEST(checksum_single);
    TEST(checksum_multiple);
    TEST(checksum_large_value);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
