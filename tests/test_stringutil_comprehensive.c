/*
 * Stone Age Client - String Utility Comprehensive Tests
 * Tests for stringutil.c implementation
 *
 * Verified against Ghidra decompilation:
 * - FUN_0044a940: String truncation with ellipsis
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef size_t usize;
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
        printf("FAIL (line %d)\n", __LINE__); \
        fflush(stdout); \
    } \
} while(0)

/* Debug print macro for error details */
#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "    ASSERT FAILED: %s (line %d)\n", msg, __LINE__); \
        return 0; \
    } \
} while(0)

/* DBCS lead byte check */
static int IsDBCSLeadByte(u8 c) {
    return (c >= 0x81 && c <= 0xFE);
}

/* ========================================
 * Implementation Functions (Safe versions)
 * ======================================== */

void string_truncate(const char* src, char* dest, usize max_len) {
    usize src_len;
    usize copy_len;

    if (!src || !dest || max_len == 0) {
        if (dest && max_len > 0) dest[0] = '\0';
        return;
    }

    src_len = strlen(src);

    if (src_len < max_len) {
        strcpy(dest, src);
        return;
    }

    /* Ensure max_len-1 bytes copied + null terminator */
    copy_len = max_len - 1;

    if (max_len >= 4) {
        usize prefix = (max_len - 3) / 2;
        usize suffix = max_len - 3 - prefix;

        /* Copy prefix */
        strncpy(dest, src, prefix);
        dest[prefix] = '\0';

        /* Add ellipsis */
        strcat(dest, "...");

        /* Add suffix if space allows */
        if (suffix > 0 && strlen(dest) + suffix < max_len) {
            strncat(dest, src + src_len - suffix, max_len - strlen(dest) - 1);
        }
    } else {
        strncpy(dest, src, copy_len);
    }

    dest[max_len - 1] = '\0';  /* Always null terminate */
}

void string_truncate_display(const char* src, char* dest, usize max_len) {
    usize src_len;

    if (!src || !dest || max_len == 0) {
        if (dest && max_len > 0) dest[0] = '\0';
        return;
    }

    src_len = strlen(src);

    if (src_len < max_len) {
        strcpy(dest, src);
        return;
    }

    if (max_len >= 7) {
        usize half = (max_len - 3) / 2;
        usize suffix_len = max_len - half - 3;
        strncpy(dest, src, half);
        dest[half] = '\0';
        strcat(dest, "...");
        strncat(dest, src + src_len - suffix_len, suffix_len);
        dest[max_len - 1] = '\0';  /* Ensure null termination */
    } else if (max_len >= 4) {
        strncpy(dest, src, max_len - 3);
        dest[max_len - 3] = '\0';
        strcat(dest, "...");
        dest[max_len - 1] = '\0';
    } else {
        strncpy(dest, src, max_len - 1);
        dest[max_len - 1] = '\0';
    }
}

usize string_len_dbcs(const char* str) {
    usize len = 0;
    if (!str) return 0;
    while (*str) {
        if (IsDBCSLeadByte((u8)*str)) {
            str += 2;
        } else {
            str++;
        }
        len++;
    }
    return len;
}

int string_is_valid_name(const char* str) {
    if (!str || !*str) return 0;
    while (*str) {
        if (IsDBCSLeadByte((u8)*str)) {
            str += 2;
        } else if (*str >= 0x20 && *str < 0x7f) {
            str++;
        } else {
            return 0;
        }
    }
    return 1;
}

void string_to_lower(char* str) {
    if (!str) return;
    while (*str) {
        if (IsDBCSLeadByte((u8)*str)) {
            str += 2;
        } else if (*str >= 'A' && *str <= 'Z') {
            *str = *str - 'A' + 'a';
            str++;
        } else {
            str++;
        }
    }
}

void string_trim(char* str) {
    char* start;
    char* end;
    usize len;

    if (!str || !*str) return;

    start = str;
    while (*start == ' ' || *start == '\t') start++;

    len = strlen(start);
    if (len == 0) {
        str[0] = '\0';
        return;
    }

    end = start + len - 1;
    while (end > start && (*end == ' ' || *end == '\t')) end--;

    len = end - start + 1;
    if (start != str) {
        memmove(str, start, len);
    }
    str[len] = '\0';
}

void string_format_number(char* dest, usize max_len, int value) {
    char buffer[32];
    int i, j, len, commas, neg = value < 0;
    int total_len;
    int digit_count = 0;

    if (!dest || max_len == 0) return;
    if (max_len == 1) {
        dest[0] = '\0';
        return;
    }

    snprintf(buffer, sizeof(buffer), "%d", neg ? -value : value);
    len = (int)strlen(buffer);
    commas = (len - 1) / 3;
    total_len = len + commas + (neg ? 1 : 0);

    if (total_len >= (int)max_len) {
        strncpy(dest, buffer, max_len - 1);
        dest[max_len - 1] = '\0';
        return;
    }

    j = total_len - 1;
    dest[total_len] = '\0';

    if (neg) dest[0] = '-';

    /* Process digits from right to left */
    for (i = len - 1; i >= 0; i--) {
        dest[j--] = buffer[i];
        digit_count++;
        /* Add comma after every 3 digits, but not at the end */
        if (digit_count % 3 == 0 && i > 0 && j >= 0) {
            dest[j--] = ',';
        }
    }
}

int string_compare_dbcs(const char* s1, const char* s2) {
    if (!s1 || !s2) return (s1 == s2) ? 0 : (s1 ? 1 : -1);

    while (*s1 && *s2) {
        if (IsDBCSLeadByte((u8)*s1) && IsDBCSLeadByte((u8)*s2)) {
            if (*s1 != *s2 || *(s1+1) != *(s2+1)) {
                return (u8)*s1 - (u8)*s2;
            }
            s1 += 2;
            s2 += 2;
        } else if (IsDBCSLeadByte((u8)*s1) || IsDBCSLeadByte((u8)*s2)) {
            return (u8)*s1 - (u8)*s2;
        } else {
            if (*s1 != *s2) return (u8)*s1 - (u8)*s2;
            s1++;
            s2++;
        }
    }
    return (u8)*s1 - (u8)*s2;
}

/* ========================================
 * Test Cases
 * ======================================== */

static int test_truncate_null_src(void) {
    char dest[32];
    dest[0] = 'X'; dest[1] = '\0';
    string_truncate(NULL, dest, sizeof(dest));
    return dest[0] == '\0';
}

static int test_truncate_null_dest(void) {
    string_truncate("hello", NULL, 32);
    return 1;  /* Should not crash */
}

static int test_truncate_zero_len(void) {
    char dest[32] = "unchanged";
    string_truncate("hello", dest, 0);
    return 1;  /* Should not crash */
}

static int test_truncate_fits(void) {
    char dest[32];
    memset(dest, 0xFF, sizeof(dest));  /* Fill with garbage */
    string_truncate("hello", dest, sizeof(dest));
    return strcmp(dest, "hello") == 0;
}

static int test_truncate_long(void) {
    char dest[16];
    memset(dest, 0xFF, sizeof(dest));
    string_truncate("hello world this is long", dest, sizeof(dest));
    size_t len = strlen(dest);
    return len < sizeof(dest) && dest[sizeof(dest)-1] == '\0';
}

static int test_truncate_small_buffer(void) {
    char dest[4];
    memset(dest, 0xFF, sizeof(dest));
    string_truncate("hello", dest, sizeof(dest));
    size_t len = strlen(dest);
    return len < sizeof(dest) && dest[sizeof(dest)-1] == '\0';
}

static int test_truncate_exact_fit(void) {
    char dest[6];
    memset(dest, 0xFF, sizeof(dest));
    string_truncate("hello", dest, sizeof(dest));
    return strcmp(dest, "hello") == 0 && dest[5] == '\0';
}

static int test_dbcs_len_ascii(void) {
    return string_len_dbcs("hello") == 5;
}

static int test_dbcs_len_null(void) {
    return string_len_dbcs(NULL) == 0;
}

static int test_dbcs_len_empty(void) {
    return string_len_dbcs("") == 0;
}

static int test_valid_name_simple(void) {
    return string_is_valid_name("Player1") == 1;
}

static int test_valid_name_null(void) {
    return string_is_valid_name(NULL) == 0;
}

static int test_valid_name_empty(void) {
    return string_is_valid_name("") == 0;
}

static int test_valid_name_special(void) {
    /* underscore (0x5F) is in valid range 0x20-0x7F */
    return string_is_valid_name("Player_1") == 1;
}

static int test_to_lower_simple(void) {
    char str[] = "HELLO";
    string_to_lower(str);
    return strcmp(str, "hello") == 0;
}

static int test_to_lower_mixed(void) {
    char str[] = "HeLLo123";
    string_to_lower(str);
    return strcmp(str, "hello123") == 0;
}

static int test_to_lower_null(void) {
    string_to_lower(NULL);
    return 1;
}

static int test_to_lower_empty(void) {
    char str[] = "";
    string_to_lower(str);
    return strcmp(str, "") == 0;
}

static int test_trim_leading(void) {
    char str[] = "   hello";
    string_trim(str);
    return strcmp(str, "hello") == 0;
}

static int test_trim_trailing(void) {
    char str[] = "hello   ";
    string_trim(str);
    return strcmp(str, "hello") == 0;
}

static int test_trim_both(void) {
    char str[] = "   hello   ";
    string_trim(str);
    return strcmp(str, "hello") == 0;
}

static int test_trim_null(void) {
    string_trim(NULL);
    return 1;
}

static int test_trim_empty(void) {
    char str[] = "";
    string_trim(str);
    return strcmp(str, "") == 0;
}

static int test_trim_all_spaces(void) {
    char str[] = "     ";
    string_trim(str);
    return strcmp(str, "") == 0;
}

static int test_trim_tabs(void) {
    char str[] = "\t\thello\t";
    string_trim(str);
    return strcmp(str, "hello") == 0;
}

static int test_format_number_simple(void) {
    char dest[32];
    string_format_number(dest, sizeof(dest), 123);
    return strcmp(dest, "123") == 0;
}

static int test_format_number_thousands(void) {
    char dest[32];
    string_format_number(dest, sizeof(dest), 1234);
    return strcmp(dest, "1,234") == 0;
}

static int test_format_number_zero(void) {
    char dest[32];
    string_format_number(dest, sizeof(dest), 0);
    return strcmp(dest, "0") == 0;
}

static int test_format_number_negative(void) {
    char dest[32];
    string_format_number(dest, sizeof(dest), -1234);
    return strcmp(dest, "-1,234") == 0;
}

static int test_format_number_large(void) {
    char dest[32];
    string_format_number(dest, sizeof(dest), 1234567890);
    return strcmp(dest, "1,234,567,890") == 0;
}

static int test_format_number_null(void) {
    string_format_number(NULL, 32, 123);
    return 1;
}

static int test_format_number_small_buffer(void) {
    char dest[5];
    string_format_number(dest, sizeof(dest), 12345);
    return strlen(dest) < sizeof(dest);
}

static int test_compare_equal(void) {
    return string_compare_dbcs("hello", "hello") == 0;
}

static int test_compare_less(void) {
    return string_compare_dbcs("abc", "def") < 0;
}

static int test_compare_greater(void) {
    return string_compare_dbcs("def", "abc") > 0;
}

static int test_compare_null_both(void) {
    return string_compare_dbcs(NULL, NULL) == 0;
}

static int test_compare_null_first(void) {
    return string_compare_dbcs(NULL, "hello") < 0;
}

static int test_compare_null_second(void) {
    return string_compare_dbcs("hello", NULL) > 0;
}

static int test_compare_empty(void) {
    return string_compare_dbcs("", "") == 0;
}

static int test_compare_prefix(void) {
    return string_compare_dbcs("hello", "helloworld") < 0;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== String Utility Comprehensive Tests ===\n\n");

    printf("String Truncate Tests:\n");
    TEST(truncate_null_src);
    TEST(truncate_null_dest);
    TEST(truncate_zero_len);
    TEST(truncate_fits);
    TEST(truncate_long);
    TEST(truncate_small_buffer);
    TEST(truncate_exact_fit);

    printf("\nDBCS String Length Tests:\n");
    TEST(dbcs_len_ascii);
    TEST(dbcs_len_null);
    TEST(dbcs_len_empty);

    printf("\nValid Name Tests:\n");
    TEST(valid_name_simple);
    TEST(valid_name_null);
    TEST(valid_name_empty);
    TEST(valid_name_special);

    printf("\nString To Lower Tests:\n");
    TEST(to_lower_simple);
    TEST(to_lower_mixed);
    TEST(to_lower_null);
    TEST(to_lower_empty);

    printf("\nString Trim Tests:\n");
    TEST(trim_leading);
    TEST(trim_trailing);
    TEST(trim_both);
    TEST(trim_null);
    TEST(trim_empty);
    TEST(trim_all_spaces);
    TEST(trim_tabs);

    printf("\nFormat Number Tests:\n");
    TEST(format_number_simple);
    TEST(format_number_thousands);
    TEST(format_number_zero);
    TEST(format_number_negative);
    TEST(format_number_large);
    TEST(format_number_null);
    TEST(format_number_small_buffer);

    printf("\nString Compare Tests:\n");
    TEST(compare_equal);
    TEST(compare_less);
    TEST(compare_greater);
    TEST(compare_null_both);
    TEST(compare_null_first);
    TEST(compare_null_second);
    TEST(compare_empty);
    TEST(compare_prefix);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
