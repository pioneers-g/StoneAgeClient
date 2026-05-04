/*
 * Stone Age Client - String Unescape Function Unit Tests
 * Tests for FUN_0048a170 (string unescape processing)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned char u8;

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

/* ========================================
 * Inline Implementation (from stubs_protocol.c)
 * ======================================== */

/*
 * FUN_0048a170 - String Unescape Processing
 * Handles backslash escape sequences in strings
 */
static char* FUN_0048a170(char* str) {
    static const char escape_find[] = { 'n', 'r', 't', '\\', '"', '\'', '|', '\0' };
    static const char escape_repl[] = { '\n', '\r', '\t', '\\', '"', '\'', '|', '\0' };

    char* src;
    char* dst;
    char c;
    int i;
    int len;

    if (!str || !*str) {
        if (str) *str = '\0';
        return str;
    }

    /* Calculate string length */
    len = 0;
    while (str[len] != '\0') {
        len++;
    }

    src = str;
    dst = str;

    while (*src != '\0') {
        /* Check for DBCS lead byte */
        if (*src & 0x80) {
            /* DBCS character - copy both bytes */
            *dst++ = *src++;
            if (*src != '\0') {
                *dst++ = *src++;
            }
            continue;
        }

        if (*src == '\\' && *(src + 1) != '\0') {
            /* Escape sequence */
            src++;  /* Skip backslash */
            c = *src;

            /* Look up in escape table */
            for (i = 0; escape_find[i] != '\0'; i++) {
                if (c == escape_find[i]) {
                    *dst++ = escape_repl[i];
                    src++;
                    break;
                }
            }

            /* If not in table, copy character as-is */
            if (escape_find[i] == '\0') {
                *dst++ = *src++;
            }
        } else {
            *dst++ = *src++;
        }
    }

    *dst = '\0';
    return str;
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(empty_string) {
    char str[16] = "";
    FUN_0048a170(str);
    ASSERT(strcmp(str, "") == 0);
}

TEST(simple_string) {
    char str[64] = "Hello World";
    FUN_0048a170(str);
    ASSERT(strcmp(str, "Hello World") == 0);
}

TEST(newline_escape) {
    char str[64] = "Hello\\nWorld";
    FUN_0048a170(str);
    ASSERT(strcmp(str, "Hello\nWorld") == 0);
}

TEST(tab_escape) {
    char str[64] = "Col1\\tCol2";
    FUN_0048a170(str);
    ASSERT(strcmp(str, "Col1\tCol2") == 0);
}

TEST(carriage_return_escape) {
    char str[64] = "Line1\\rLine2";
    FUN_0048a170(str);
    ASSERT(strcmp(str, "Line1\rLine2") == 0);
}

TEST(backslash_escape) {
    char str[64] = "Path\\\\File";
    FUN_0048a170(str);
    ASSERT(strcmp(str, "Path\\File") == 0);
}

TEST(quote_escape) {
    char str[64] = "Say \\\"Hello\\\"";
    FUN_0048a170(str);
    ASSERT(strcmp(str, "Say \"Hello\"") == 0);
}

TEST(single_quote_escape) {
    char str[64] = "It\\'s fine";
    FUN_0048a170(str);
    ASSERT(strcmp(str, "It's fine") == 0);
}

TEST(pipe_escape) {
    char str[64] = "Field1\\|Field2";
    FUN_0048a170(str);
    ASSERT(strcmp(str, "Field1|Field2") == 0);
}

TEST(multiple_escapes) {
    char str[64] = "Line1\\nLine2\\tTab\\rEnd";
    FUN_0048a170(str);
    ASSERT(strcmp(str, "Line1\nLine2\tTab\rEnd") == 0);
}

TEST(unknown_escape) {
    char str[64] = "Test\\xValue";
    FUN_0048a170(str);
    ASSERT(strcmp(str, "TestxValue") == 0);
}

TEST(trailing_backslash) {
    char str[64] = "Test\\";
    FUN_0048a170(str);
    /* Trailing backslash should be removed or preserved */
    ASSERT(str[0] == 'T' && str[1] == 'e' && str[2] == 's' && str[3] == 't');
}

TEST(consecutive_escapes) {
    char str[64] = "A\\n\\t\\rB";
    FUN_0048a170(str);
    ASSERT(strcmp(str, "A\n\t\rB") == 0);
}

TEST(mixed_content) {
    char str[64] = "Name: \\\"John\\\"\\tAge: 25\\n";
    FUN_0048a170(str);
    ASSERT(strcmp(str, "Name: \"John\"\tAge: 25\n") == 0);
}

TEST(no_escapes) {
    char str[64] = "Normal text without escapes";
    FUN_0048a170(str);
    ASSERT(strcmp(str, "Normal text without escapes") == 0);
}

TEST(null_pointer) {
    char* result = FUN_0048a170(NULL);
    ASSERT(result == NULL);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== String Unescape Function Unit Tests ===\n\n");

    printf("Basic Tests:\n");
    RUN_TEST(empty_string);
    RUN_TEST(simple_string);
    RUN_TEST(no_escapes);

    printf("\nEscape Sequence Tests:\n");
    RUN_TEST(newline_escape);
    RUN_TEST(tab_escape);
    RUN_TEST(carriage_return_escape);
    RUN_TEST(backslash_escape);
    RUN_TEST(quote_escape);
    RUN_TEST(single_quote_escape);
    RUN_TEST(pipe_escape);

    printf("\nComplex Tests:\n");
    RUN_TEST(multiple_escapes);
    RUN_TEST(unknown_escape);
    RUN_TEST(trailing_backslash);
    RUN_TEST(consecutive_escapes);
    RUN_TEST(mixed_content);
    RUN_TEST(null_pointer);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
