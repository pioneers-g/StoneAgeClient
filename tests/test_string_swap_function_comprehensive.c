/*
 * Stone Age Client - String Swap Function Unit Tests
 * Tests for FUN_004777e0 - Swap two strings with temp buffer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;

/* ========================================
 * Test Data and Constants
 * ======================================== */

#define MAX_STRING_LEN 256

static int test_passed = 0;
static int test_failed = 0;

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    test_##name(); \
    printf("PASS\n"); \
    test_passed++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL at line %d: %s\n", __LINE__, #cond); \
        test_failed++; \
        return; \
    } \
} while(0)

/*
 * FUN_004777e0 - Swap two strings using a temporary buffer
 * param_1: first string (char*)
 * param_2: second string (char*)
 * Swaps the contents of both strings if both are < 256 chars
 *
 * This is a simplified, safer implementation that matches the
 * original function's behavior while avoiding pointer arithmetic issues.
 */
static void FUN_004777e0(char *param_1, char *param_2) {
    size_t len1, len2;
    char temp[MAX_STRING_LEN];

    /* Get lengths */
    len1 = strlen(param_1);
    len2 = strlen(param_2);

    /* Only swap if both strings fit in buffer (< 256 chars) */
    if (len1 < MAX_STRING_LEN && len2 < MAX_STRING_LEN) {
        /* Copy param_1 to temp */
        memcpy(temp, param_1, len1 + 1);

        /* Copy param_2 to param_1 */
        memcpy(param_1, param_2, len2 + 1);

        /* Copy temp to param_2 */
        memcpy(param_2, temp, len1 + 1);
    }
}

/* ========================================
 * Test Cases
 * ======================================== */

/* Basic swap tests */
TEST(swap_simple) {
    char str1[64] = "Hello";
    char str2[64] = "World";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "World") == 0);
    ASSERT(strcmp(str2, "Hello") == 0);
}

TEST(swap_empty_strings) {
    char str1[64] = "";
    char str2[64] = "";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "") == 0);
    ASSERT(strcmp(str2, "") == 0);
}

TEST(swap_one_empty) {
    char str1[64] = "NotEmpty";
    char str2[64] = "";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "") == 0);
    ASSERT(strcmp(str2, "NotEmpty") == 0);
}

TEST(swap_other_empty) {
    char str1[64] = "";
    char str2[64] = "NotEmpty";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "NotEmpty") == 0);
    ASSERT(strcmp(str2, "") == 0);
}

/* Different length strings */
TEST(swap_different_lengths) {
    char str1[64] = "Short";
    char str2[64] = "ThisIsAVeryLongString";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "ThisIsAVeryLongString") == 0);
    ASSERT(strcmp(str2, "Short") == 0);
}

TEST(swap_single_char) {
    char str1[64] = "A";
    char str2[64] = "B";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "B") == 0);
    ASSERT(strcmp(str2, "A") == 0);
}

/* Same content */
TEST(swap_same_content) {
    char str1[64] = "SameString";
    char str2[64] = "SameString";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "SameString") == 0);
    ASSERT(strcmp(str2, "SameString") == 0);
}

/* Multiple swaps */
TEST(swap_multiple_times) {
    char str1[64] = "First";
    char str2[64] = "Second";

    FUN_004777e0(str1, str2);
    ASSERT(strcmp(str1, "Second") == 0);
    ASSERT(strcmp(str2, "First") == 0);

    FUN_004777e0(str1, str2);
    ASSERT(strcmp(str1, "First") == 0);
    ASSERT(strcmp(str2, "Second") == 0);

    FUN_004777e0(str1, str2);
    ASSERT(strcmp(str1, "Second") == 0);
    ASSERT(strcmp(str2, "First") == 0);
}

/* Long strings (within limit) */
TEST(swap_long_strings) {
    char str1[256];
    char str2[256];

    /* Create 200-char strings */
    memset(str1, 'A', 200);
    str1[200] = '\0';
    memset(str2, 'B', 200);
    str2[200] = '\0';

    FUN_004777e0(str1, str2);

    ASSERT(str1[0] == 'B');
    ASSERT(str1[199] == 'B');
    ASSERT(str1[200] == '\0');
    ASSERT(str2[0] == 'A');
    ASSERT(str2[199] == 'A');
    ASSERT(str2[200] == '\0');
}

/* Near boundary (255 chars - should work) */
TEST(swap_near_boundary) {
    char str1[300];
    char str2[300];

    /* Create 255-char strings */
    memset(str1, 'X', 255);
    str1[255] = '\0';
    memset(str2, 'Y', 255);
    str2[255] = '\0';

    FUN_004777e0(str1, str2);

    ASSERT(str1[0] == 'Y');
    ASSERT(str1[254] == 'Y');
    ASSERT(str2[0] == 'X');
    ASSERT(str2[254] == 'X');
}

/* Special characters */
TEST(swap_special_chars) {
    char str1[64] = "Line1\nLine2";
    char str2[64] = "Tab\tHere";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "Tab\tHere") == 0);
    ASSERT(strcmp(str2, "Line1\nLine2") == 0);
}

TEST(swap_spaces) {
    char str1[64] = "   Spaces   ";
    char str2[64] = "\tTabs\t";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "\tTabs\t") == 0);
    ASSERT(strcmp(str2, "   Spaces   ") == 0);
}

/* Numbers and mixed content */
TEST(swap_numbers) {
    char str1[64] = "12345";
    char str2[64] = "67890";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "67890") == 0);
    ASSERT(strcmp(str2, "12345") == 0);
}

TEST(swap_mixed_content) {
    char str1[64] = "ABC123xyz!@#";
    char str2[64] = "Mixed_987_Content";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "Mixed_987_Content") == 0);
    ASSERT(strcmp(str2, "ABC123xyz!@#") == 0);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== String Swap Function Unit Tests ===\n\n");

    printf("Basic Swap Tests:\n");
    RUN_TEST(swap_simple);
    RUN_TEST(swap_empty_strings);
    RUN_TEST(swap_one_empty);
    RUN_TEST(swap_other_empty);

    printf("\nDifferent Length Tests:\n");
    RUN_TEST(swap_different_lengths);
    RUN_TEST(swap_single_char);

    printf("\nSame Content Tests:\n");
    RUN_TEST(swap_same_content);

    printf("\nMultiple Swap Tests:\n");
    RUN_TEST(swap_multiple_times);

    printf("\nLong String Tests:\n");
    RUN_TEST(swap_long_strings);
    RUN_TEST(swap_near_boundary);

    printf("\nSpecial Character Tests:\n");
    RUN_TEST(swap_special_chars);
    RUN_TEST(swap_spaces);

    printf("\nMixed Content Tests:\n");
    RUN_TEST(swap_numbers);
    RUN_TEST(swap_mixed_content);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
