/*
 * Stone Age Client - String Swap Function Unit Tests
 * Tests for FUN_004777e0 - Swap two strings
 * This function swaps two strings using a temporary buffer
 * Only swaps if both strings are < 256 characters
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
 * FUN_004777e0 - Swap two strings
 * Swaps the contents of two null-terminated strings
 * Only works if both strings are less than 256 characters
 */
static void FUN_004777e0(char *param_1, char *param_2) {
    char cVar1;
    u32 uVar2;
    u32 uVar3;
    char *pcVar4;
    char *pcVar5;
    char local_100[256];

    /* Get length of first string */
    uVar2 = 0xffffffff;
    pcVar4 = param_1;
    do {
        if (uVar2 == 0) break;
        uVar2 = uVar2 - 1;
        cVar1 = *pcVar4;
        pcVar4 = pcVar4 + 1;
    } while (cVar1 != '\0');

    /* Check if first string < 256 chars */
    if (~uVar2 - 1 >= 0x100) {
        return;
    }

    /* Get length of second string */
    uVar2 = 0xffffffff;
    pcVar4 = param_2;
    do {
        if (uVar2 == 0) break;
        uVar2 = uVar2 - 1;
        cVar1 = *pcVar4;
        pcVar4 = pcVar4 + 1;
    } while (cVar1 != '\0');

    /* Check if second string < 256 chars */
    if (~uVar2 - 1 >= 0x100) {
        return;
    }

    /* Copy first string to temp buffer */
    uVar2 = 0xffffffff;
    pcVar4 = param_1;
    do {
        pcVar5 = pcVar4;
        if (uVar2 == 0) break;
        uVar2 = uVar2 - 1;
        pcVar5 = pcVar4 + 1;
        cVar1 = *pcVar4;
        pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar2 = ~uVar2;
    pcVar4 = pcVar5 + (-uVar2);
    pcVar5 = local_100;
    for (uVar3 = uVar2 >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
        *(u32 *)pcVar5 = *(u32 *)pcVar4;
        pcVar4 = pcVar4 + 4;
        pcVar5 = pcVar5 + 4;
    }
    for (uVar2 = uVar2 & 3; uVar2 != 0; uVar2 = uVar2 - 1) {
        *pcVar5 = *pcVar4;
        pcVar4 = pcVar4 + 1;
        pcVar5 = pcVar5 + 1;
    }

    /* Copy second string to first */
    uVar2 = 0xffffffff;
    pcVar4 = param_2;
    do {
        pcVar5 = pcVar4;
        if (uVar2 == 0) break;
        uVar2 = uVar2 - 1;
        pcVar5 = pcVar4 + 1;
        cVar1 = *pcVar4;
        pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar2 = ~uVar2;
    pcVar4 = pcVar5 + (-uVar2);
    for (uVar3 = uVar2 >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
        *(u32 *)param_1 = *(u32 *)pcVar4;
        pcVar4 = pcVar4 + 4;
        param_1 = param_1 + 4;
    }
    for (uVar2 = uVar2 & 3; uVar2 != 0; uVar2 = uVar2 - 1) {
        *param_1 = *pcVar4;
        pcVar4 = pcVar4 + 1;
        param_1 = param_1 + 1;
    }

    /* Copy temp buffer to second */
    uVar2 = 0xffffffff;
    pcVar4 = local_100;
    do {
        pcVar5 = pcVar4;
        if (uVar2 == 0) break;
        uVar2 = uVar2 - 1;
        pcVar5 = pcVar4 + 1;
        cVar1 = *pcVar4;
        pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar2 = ~uVar2;
    pcVar4 = pcVar5 + (-uVar2);
    for (uVar3 = uVar2 >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
        *(u32 *)param_2 = *(u32 *)pcVar4;
        pcVar4 = pcVar4 + 4;
        param_2 = param_2 + 4;
    }
    for (uVar2 = uVar2 & 3; uVar2 != 0; uVar2 = uVar2 - 1) {
        *param_2 = *pcVar4;
        pcVar4 = pcVar4 + 1;
        param_2 = param_2 + 1;
    }
}

/* ========================================
 * Test Cases for FUN_004777e0
 * ======================================== */

TEST(swap_simple_strings) {
    char str1[256] = "Hello";
    char str2[256] = "World";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "World") == 0);
    ASSERT(strcmp(str2, "Hello") == 0);
}

TEST(swap_empty_strings) {
    char str1[256] = "";
    char str2[256] = "";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "") == 0);
    ASSERT(strcmp(str2, "") == 0);
}

TEST(swap_one_empty) {
    char str1[256] = "Test";
    char str2[256] = "";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "") == 0);
    ASSERT(strcmp(str2, "Test") == 0);
}

TEST(swap_single_char) {
    char str1[256] = "A";
    char str2[256] = "B";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "B") == 0);
    ASSERT(strcmp(str2, "A") == 0);
}

TEST(swap_long_strings) {
    char str1[256];
    char str2[256];

    memset(str1, 'X', 200);
    str1[200] = '\0';
    memset(str2, 'Y', 200);
    str2[200] = '\0';

    FUN_004777e0(str1, str2);

    ASSERT(str1[0] == 'Y');
    ASSERT(str1[199] == 'Y');
    ASSERT(str1[200] == '\0');
    ASSERT(str2[0] == 'X');
    ASSERT(str2[199] == 'X');
    ASSERT(str2[200] == '\0');
}

TEST(swap_different_lengths) {
    char str1[256] = "Short";
    char str2[256] = "ThisIsALongerString";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "ThisIsALongerString") == 0);
    ASSERT(strcmp(str2, "Short") == 0);
}

TEST(swap_with_spaces) {
    char str1[256] = "Hello World";
    char str2[256] = "Goodbye World";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "Goodbye World") == 0);
    ASSERT(strcmp(str2, "Hello World") == 0);
}

TEST(swap_preserves_content) {
    char str1[256] = "Content1";
    char str2[256] = "Content2";

    FUN_004777e0(str1, str2);
    FUN_004777e0(str1, str2);  /* Swap back */

    ASSERT(strcmp(str1, "Content1") == 0);
    ASSERT(strcmp(str2, "Content2") == 0);
}

TEST(swap_numeric_strings) {
    char str1[256] = "12345";
    char str2[256] = "67890";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "67890") == 0);
    ASSERT(strcmp(str2, "12345") == 0);
}

TEST(swap_special_chars) {
    char str1[256] = "A|B|C";
    char str2[256] = "X-Y-Z";

    FUN_004777e0(str1, str2);

    ASSERT(strcmp(str1, "X-Y-Z") == 0);
    ASSERT(strcmp(str2, "A|B|C") == 0);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== String Swap Function Unit Tests ===\n\n");

    printf("FUN_004777e0 (String Swap) Tests:\n");
    RUN_TEST(swap_simple_strings);
    RUN_TEST(swap_empty_strings);
    RUN_TEST(swap_one_empty);
    RUN_TEST(swap_single_char);
    RUN_TEST(swap_long_strings);
    RUN_TEST(swap_different_lengths);
    RUN_TEST(swap_with_spaces);
    RUN_TEST(swap_preserves_content);
    RUN_TEST(swap_numeric_strings);
    RUN_TEST(swap_special_chars);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
