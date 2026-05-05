/*
 * Stone Age Client - Round 3 Stub Functions Tests
 * Tests for text queue (FUN_0041d7c0/0041d860), sprintf (FUN_004923a7)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int32_t s32;

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) printf("  TEST: %s ... ", name)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)

static void check_eq_impl(int a, int b, const char* file, int line) {
    if (a != b) {
        printf("FAIL: %s:%d got %d expected %d\n", file, line, a, b);
        tests_failed++;
    }
}

static void check_streq_impl(const char* a, const char* b, const char* file, int line) {
    if (strcmp(a, b) != 0) {
        printf("FAIL: %s:%d \"%s\" != \"%s\"\n", file, line, a, b);
        tests_failed++;
    }
}

#define ASSERT_EQ(a, b) do { int _a = (int)(a); int _b = (int)(b); if (_a != _b) { printf("FAIL: line %d got %d expected %d\n", __LINE__, _a, _b); tests_failed++; return; } } while(0)
#define ASSERT_STREQ(a, b) do { const char* _sa = (a); const char* _sb = (b); if (strcmp(_sa, _sb) != 0) { printf("FAIL: line %d strings differ\n", __LINE__); tests_failed++; return; } } while(0)

/* ===== FUN_004923a7 - sprintf wrapper ===== */
int FUN_004923a7(char* buffer, const char* format, ...) {
    va_list args;
    int result;
    va_start(args, format);
    result = vsprintf(buffer, format, args);
    va_end(args);
    return result;
}

void test_sprintf_basic(void) {
    TEST("FUN_004923a7 basic string");
    char buf[256];
    int len = FUN_004923a7(buf, "Hello %s", "World");
    ASSERT_EQ(len, 11);
    ASSERT_STREQ(buf, "Hello World");
    PASS();
}

void test_sprintf_int(void) {
    TEST("FUN_004923a7 integer formatting");
    char buf[256];
    int len = FUN_004923a7(buf, "HP: %d/%d", 100, 200);
    ASSERT_EQ(len, 11);
    ASSERT_STREQ(buf, "HP: 100/200");
    PASS();
}

void test_sprintf_hex(void) {
    TEST("FUN_004923a7 hex formatting");
    char buf[256];
    FUN_004923a7(buf, "0x%04X", 0x1234);
    ASSERT_STREQ(buf, "0x1234");
    PASS();
}

void test_sprintf_empty(void) {
    TEST("FUN_004923a7 empty format");
    char buf[256];
    int len = FUN_004923a7(buf, "");
    ASSERT_EQ(len, 0);
    ASSERT_STREQ(buf, "");
    PASS();
}

void test_sprintf_multiple(void) {
    TEST("FUN_004923a7 mixed types");
    char buf[256];
    FUN_004923a7(buf, "%s has %d gold (%u items)", "Player", 500, 10);
    ASSERT_STREQ(buf, "Player has 500 gold (10 items)");
    PASS();
}

/* ===== Text Queue (FUN_0041d7c0) ===== */

#define TEXT_QUEUE_MAX 1024

typedef struct {
    u16 x;
    u16 y;
    u8 type;
    u8 style;
    char text[256];
    u32 sprite;
    u32 flags;
    int active;
} TextQueueEntry;

static TextQueueEntry s_text_queue[TEXT_QUEUE_MAX];
static u32 s_text_queue_count = 0;

int FUN_0041d7c0(u16 x, u16 y, u8 type, u8 style, const char* text, u32 sprite, u32 flags) {
    u32 index;
    if (s_text_queue_count >= TEXT_QUEUE_MAX) return -2;
    index = s_text_queue_count;
    s_text_queue[index].x = x;
    s_text_queue[index].y = y;
    s_text_queue[index].type = type;
    s_text_queue[index].style = style;
    s_text_queue[index].sprite = sprite;
    s_text_queue[index].flags = flags;
    s_text_queue[index].active = 1;
    if (text) {
        strncpy(s_text_queue[index].text, text, 255);
        s_text_queue[index].text[255] = '\0';
    } else {
        s_text_queue[index].text[0] = '\0';
    }
    s_text_queue_count++;
    return (int)index;
}

void FUN_0041d860(u16 x, u16 y, u8 type, u8 style, const char* text, u32 sprite) {
    FUN_0041d7c0(x, y, type, style, text, sprite, 0);
}

void test_text_queue_add(void) {
    TEST("FUN_0041d7c0 add single entry");
    s_text_queue_count = 0;
    memset(s_text_queue, 0, sizeof(s_text_queue));
    int idx = FUN_0041d7c0(100, 200, 1, 0, "Test", 42, 0);
    ASSERT_EQ(idx, 0);
    ASSERT_EQ(s_text_queue[0].x, 100);
    ASSERT_EQ(s_text_queue[0].y, 200);
    ASSERT_EQ(s_text_queue[0].type, 1);
    ASSERT_EQ((int)s_text_queue[0].sprite, 42);
    ASSERT_EQ(s_text_queue[0].active, 1);
    ASSERT_STREQ(s_text_queue[0].text, "Test");
    ASSERT_EQ((int)s_text_queue_count, 1);
    PASS();
}

void test_text_queue_multiple(void) {
    TEST("FUN_0041d7c0 multiple entries");
    s_text_queue_count = 0;
    memset(s_text_queue, 0, sizeof(s_text_queue));

    FUN_0041d7c0(10, 20, 0, 0, "First", 1, 0);
    FUN_0041d7c0(30, 40, 1, 0, "Second", 2, 0);
    int idx = FUN_0041d7c0(50, 60, 2, 0, "Third", 3, 0);

    ASSERT_EQ(idx, 2);
    ASSERT_EQ((int)s_text_queue_count, 3);
    ASSERT_STREQ(s_text_queue[0].text, "First");
    ASSERT_STREQ(s_text_queue[1].text, "Second");
    ASSERT_STREQ(s_text_queue[2].text, "Third");
    ASSERT_EQ(s_text_queue[2].x, 50);
    ASSERT_EQ((int)s_text_queue[2].sprite, 3);
    PASS();
}

void test_text_queue_full(void) {
    TEST("FUN_0041d7c0 queue full returns -2");
    s_text_queue_count = TEXT_QUEUE_MAX;
    int idx = FUN_0041d7c0(0, 0, 0, 0, "overflow", 0, 0);
    ASSERT_EQ(idx, -2);
    PASS();
}

void test_text_queue_null_text(void) {
    TEST("FUN_0041d7c0 NULL text handling");
    s_text_queue_count = 0;
    memset(s_text_queue, 0, sizeof(s_text_queue));
    int idx = FUN_0041d7c0(0, 0, 0, 0, NULL, 0, 0);
    ASSERT_EQ(idx, 0);
    ASSERT_EQ(s_text_queue[0].text[0], '\0');
    PASS();
}

void test_text_queue_wrapper(void) {
    TEST("FUN_0041d860 wrapper delegates correctly");
    s_text_queue_count = 0;
    memset(s_text_queue, 0, sizeof(s_text_queue));
    FUN_0041d860(150, 250, 3, 2, "Wrapper", 99);
    ASSERT_EQ(s_text_queue[0].x, 150);
    ASSERT_EQ(s_text_queue[0].y, 250);
    ASSERT_EQ(s_text_queue[0].type, 3);
    ASSERT_EQ((int)s_text_queue[0].flags, 0);
    ASSERT_STREQ(s_text_queue[0].text, "Wrapper");
    PASS();
}

void test_text_queue_long_text(void) {
    TEST("FUN_0041d7c0 long text truncated");
    s_text_queue_count = 0;
    memset(s_text_queue, 0, sizeof(s_text_queue));
    char longtext[512];
    memset(longtext, 'A', 511);
    longtext[511] = '\0';
    FUN_0041d7c0(0, 0, 0, 0, longtext, 0, 0);
    ASSERT_EQ((int)strlen(s_text_queue[0].text), 255);
    PASS();
}

int main(void) {
    printf("\n=== Round 3 Stub Functions Tests ===\n\n");

    printf("[FUN_004923a7 - sprintf wrapper]\n");
    test_sprintf_basic();
    test_sprintf_int();
    test_sprintf_hex();
    test_sprintf_empty();
    test_sprintf_multiple();

    printf("\n[FUN_0041d7c0/0041d860 - Text Render Queue]\n");
    test_text_queue_add();
    test_text_queue_multiple();
    test_text_queue_full();
    test_text_queue_null_text();
    test_text_queue_wrapper();
    test_text_queue_long_text();

    printf("\n=== Results: %d passed, %d failed, %d total ===\n",
           tests_passed, tests_failed, tests_passed + tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
