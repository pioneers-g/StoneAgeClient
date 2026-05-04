/*
 * Stone Age Client - String Swap Function Unit Tests
 * Tests for FUN_004777e0 (swap two strings)
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
 * FUN_004777e0 - Swap Two Strings
 */
static void FUN_004777e0(char* str1, char* str2) {
    char temp[256];
    size_t len1, len2;

    /* Get lengths */
    len1 = strlen(str1);
    len2 = strlen(str2);

    /* Only swap if both strings fit in buffer */
    if (len1 >= 256 || len2 >= 256) {
        return;
    }

    /* Copy str1 to temp */
    memcpy(temp, str1, len1 + 1);

    /* Copy str2 to str1 */
    memcpy(str1, str2, len2 + 1);

    /* Copy temp to str2 */
    memcpy(str2, temp, len1 + 1);
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(swap_simple_strings) {
    char str1[256] = "hello";
    char str2[256] = "world";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "world") == 0);
    ASSERT(strcmp(str2, "hello") == 0);
}

TEST(swap_empty_strings) {
    char str1[256] = "";
    char str2[256] = "test";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "test") == 0);
    ASSERT(strcmp(str2, "") == 0);
}

TEST(swap_both_empty) {
    char str1[256] = "";
    char str2[256] = "";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "") == 0);
    ASSERT(strcmp(str2, "") == 0);
}

TEST(swap_same_content) {
    char str1[256] = "same";
    char str2[256] = "same";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "same") == 0);
    ASSERT(strcmp(str2, "same") == 0);
}

TEST(swap_different_lengths) {
    char str1[256] = "short";
    char str2[256] = "this is a much longer string";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "this is a much longer string") == 0);
    ASSERT(strcmp(str2, "short") == 0);
}

TEST(swap_single_char) {
    char str1[256] = "a";
    char str2[256] = "b";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "b") == 0);
    ASSERT(strcmp(str2, "a") == 0);
}

TEST(swap_with_numbers) {
    char str1[256] = "12345";
    char str2[256] = "67890";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "67890") == 0);
    ASSERT(strcmp(str2, "12345") == 0);
}

TEST(swap_with_special_chars) {
    char str1[256] = "hello!";
    char str2[256] = "world?";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "world?") == 0);
    ASSERT(strcmp(str2, "hello!") == 0);
}

TEST(swap_max_length) {
    char str1[256];
    char str2[256];

    /* Fill with 'a' and 'b' */
    memset(str1, 'a', 255);
    str1[255] = '\0';
    memset(str2, 'b', 255);
    str2[255] = '\0';

    FUN_004777e0(str1, str2);

    /* Verify swapped */
    ASSERT(str1[0] == 'b');
    ASSERT(str2[0] == 'a');
    ASSERT(str1[254] == 'b');
    ASSERT(str2[254] == 'a');
}

TEST(swap_multiple_times) {
    char str1[256] = "first";
    char str2[256] = "second";

    FUN_004777e0(str1, str2);
    ASSERT(strcmp(str1, "second") == 0);
    ASSERT(strcmp(str2, "first") == 0);

    FUN_004777e0(str1, str2);
    ASSERT(strcmp(str1, "first") == 0);
    ASSERT(strcmp(str2, "second") == 0);

    FUN_004777e0(str1, str2);
    ASSERT(strcmp(str1, "second") == 0);
    ASSERT(strcmp(str2, "first") == 0);
}

TEST(swap_with_spaces) {
    char str1[256] = "hello world";
    char str2[256] = "goodbye world";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "goodbye world") == 0);
    ASSERT(strcmp(str2, "hello world") == 0);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== String Swap Function Unit Tests ===\n\n");

    printf("Basic Tests:\n");
    RUN_TEST(swap_simple_strings);
    RUN_TEST(swap_empty_strings);
    RUN_TEST(swap_both_empty);
    RUN_TEST(swap_same_content);

    printf("\nLength Tests:\n");
    RUN_TEST(swap_different_lengths);
    RUN_TEST(swap_single_char);
    RUN_TEST(swap_max_length);

    printf("\nContent Tests:\n");
    RUN_TEST(swap_with_numbers);
    RUN_TEST(swap_with_special_chars);
    RUN_TEST(swap_with_spaces);

    printf("\nMultiple Swap Tests:\n");
    RUN_TEST(swap_multiple_times);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
