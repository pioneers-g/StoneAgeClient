/*
 * Stone Age Client - Text Queue Functions Unit Tests
 * Tests for text rendering queue - self-contained
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions for self-contained test */
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef int s32;

/* ========================================
 * Test Data and Mock Implementations
 * ======================================== */

#define TEXT_QUEUE_MAX 1024

/* Text queue count */
static u32 test_text_queue_count = 0;

/* Text queue entries - simplified structure */
typedef struct {
    u16 x;
    u16 y;
    u8 type;
    u8 style;
    char text[256];
    u32 sprite;
    u32 flags;
} TextQueueEntry;

static TextQueueEntry test_text_queue[TEXT_QUEUE_MAX];

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
 * Inline Implementations (from stubs_misc.c)
 * ======================================== */

static void reset_queue(void) {
    test_text_queue_count = 0;
    memset(test_text_queue, 0, sizeof(test_text_queue));
}

static int FUN_0041d7c0(u16 x, u16 y, u8 type, u8 style, const char* text, u32 sprite, u32 flags) {
    int idx;

    if (test_text_queue_count >= TEXT_QUEUE_MAX) {
        return -2;  /* Queue full */
    }

    idx = test_text_queue_count;
    test_text_queue[idx].x = x;
    test_text_queue[idx].y = y;
    test_text_queue[idx].type = type;
    test_text_queue[idx].style = style;
    test_text_queue[idx].sprite = sprite;
    test_text_queue[idx].flags = flags;

    if (text != NULL) {
        strncpy(test_text_queue[idx].text, text, 255);
        test_text_queue[idx].text[255] = '\0';
    }

    test_text_queue_count++;
    return idx;
}

static void FUN_0041d860(u16 x, u16 y, u8 type, u8 style, const char* text, u32 sprite) {
    FUN_0041d7c0(x, y, type, style, text, sprite, 0);
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(add_single_entry) {
    reset_queue();

    int idx = FUN_0041d7c0(100, 200, 1, 2, "Hello", 0x1234, 0);
    ASSERT(idx == 0);
    ASSERT(test_text_queue_count == 1);
    ASSERT(test_text_queue[0].x == 100);
    ASSERT(test_text_queue[0].y == 200);
    ASSERT(test_text_queue[0].type == 1);
    ASSERT(test_text_queue[0].style == 2);
    ASSERT(strcmp(test_text_queue[0].text, "Hello") == 0);
    ASSERT(test_text_queue[0].sprite == 0x1234);
    ASSERT(test_text_queue[0].flags == 0);
}

TEST(add_multiple_entries) {
    reset_queue();

    int idx1 = FUN_0041d7c0(10, 20, 1, 0, "First", 100, 1);
    int idx2 = FUN_0041d7c0(30, 40, 2, 0, "Second", 200, 2);
    int idx3 = FUN_0041d7c0(50, 60, 3, 0, "Third", 300, 3);

    ASSERT(idx1 == 0);
    ASSERT(idx2 == 1);
    ASSERT(idx3 == 2);
    ASSERT(test_text_queue_count == 3);

    ASSERT(test_text_queue[0].x == 10);
    ASSERT(test_text_queue[1].x == 30);
    ASSERT(test_text_queue[2].x == 50);
}

TEST(simplified_wrapper) {
    reset_queue();

    FUN_0041d860(100, 200, 1, 2, "Test", 0x5678);

    ASSERT(test_text_queue_count == 1);
    ASSERT(test_text_queue[0].x == 100);
    ASSERT(test_text_queue[0].y == 200);
    ASSERT(test_text_queue[0].flags == 0);  /* Wrapper sets flags to 0 */
}

TEST(queue_full_returns_error) {
    reset_queue();

    /* Fill the queue */
    for (int i = 0; i < TEXT_QUEUE_MAX; i++) {
        int idx = FUN_0041d7c0(i, i*2, 1, 0, "X", 0, 0);
        ASSERT(idx == i);
    }

    ASSERT(test_text_queue_count == TEXT_QUEUE_MAX);

    /* Try to add one more - should fail */
    int idx = FUN_0041d7c0(999, 999, 1, 0, "Overflow", 0, 0);
    ASSERT(idx == -2);  /* Queue full error */
    ASSERT(test_text_queue_count == TEXT_QUEUE_MAX);  /* Count unchanged */
}

TEST(null_text_handling) {
    reset_queue();

    int idx = FUN_0041d7c0(100, 100, 1, 1, NULL, 0, 0);
    ASSERT(idx == 0);
    ASSERT(test_text_queue[0].text[0] == '\0');
}

TEST(empty_text_handling) {
    reset_queue();

    int idx = FUN_0041d7c0(100, 100, 1, 1, "", 0, 0);
    ASSERT(idx == 0);
    ASSERT(test_text_queue[0].text[0] == '\0');
}

TEST(long_text_truncation) {
    reset_queue();

    char long_text[300];
    memset(long_text, 'A', 299);
    long_text[299] = '\0';

    int idx = FUN_0041d7c0(100, 100, 1, 1, long_text, 0, 0);
    ASSERT(idx == 0);
    ASSERT(strlen(test_text_queue[0].text) == 255);  /* Truncated to 255 chars */
}

TEST(position_values) {
    reset_queue();

    /* Test boundary positions */
    FUN_0041d7c0(0, 0, 1, 0, "Origin", 0, 0);
    FUN_0041d7c0(65535, 65535, 1, 0, "Max", 0, 0);

    ASSERT(test_text_queue[0].x == 0);
    ASSERT(test_text_queue[0].y == 0);
    ASSERT(test_text_queue[1].x == 65535);
    ASSERT(test_text_queue[1].y == 65535);
}

TEST(type_style_values) {
    reset_queue();

    FUN_0041d7c0(0, 0, 0, 0, "Zero", 0, 0);
    FUN_0041d7c0(0, 0, 255, 255, "Max", 0, 0);

    ASSERT(test_text_queue[0].type == 0);
    ASSERT(test_text_queue[0].style == 0);
    ASSERT(test_text_queue[1].type == 255);
    ASSERT(test_text_queue[1].style == 255);
}

TEST(sprite_and_flags_values) {
    reset_queue();

    FUN_0041d7c0(0, 0, 1, 1, "Test", 0xDEADBEEF, 0xCAFEBABE);

    ASSERT(test_text_queue[0].sprite == 0xDEADBEEF);
    ASSERT(test_text_queue[0].flags == 0xCAFEBABE);
}

TEST(queue_order_preserved) {
    reset_queue();

    /* Add entries with different texts */
    FUN_0041d7c0(0, 0, 1, 0, "First", 0, 0);
    FUN_0041d7c0(0, 0, 1, 0, "Second", 0, 0);
    FUN_0041d7c0(0, 0, 1, 0, "Third", 0, 0);

    /* Verify order is preserved */
    ASSERT(strcmp(test_text_queue[0].text, "First") == 0);
    ASSERT(strcmp(test_text_queue[1].text, "Second") == 0);
    ASSERT(strcmp(test_text_queue[2].text, "Third") == 0);
}

TEST(independent_entries) {
    reset_queue();

    /* Each entry should be independent */
    FUN_0041d7c0(10, 20, 1, 2, "Entry1", 100, 200);
    FUN_0041d7c0(30, 40, 3, 4, "Entry2", 300, 400);

    /* First entry should not be affected by second */
    ASSERT(test_text_queue[0].x == 10);
    ASSERT(test_text_queue[0].y == 20);
    ASSERT(test_text_queue[0].type == 1);
    ASSERT(test_text_queue[0].style == 2);
    ASSERT(strcmp(test_text_queue[0].text, "Entry1") == 0);
    ASSERT(test_text_queue[0].sprite == 100);
    ASSERT(test_text_queue[0].flags == 200);

    /* Second entry should have correct values */
    ASSERT(test_text_queue[1].x == 30);
    ASSERT(test_text_queue[1].y == 40);
    ASSERT(test_text_queue[1].type == 3);
    ASSERT(test_text_queue[1].style == 4);
    ASSERT(strcmp(test_text_queue[1].text, "Entry2") == 0);
    ASSERT(test_text_queue[1].sprite == 300);
    ASSERT(test_text_queue[1].flags == 400);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Text Queue Functions Unit Tests ===\n\n");

    printf("Basic Queue Operations:\n");
    RUN_TEST(add_single_entry);
    RUN_TEST(add_multiple_entries);
    RUN_TEST(simplified_wrapper);
    RUN_TEST(queue_full_returns_error);

    printf("\nText Handling:\n");
    RUN_TEST(null_text_handling);
    RUN_TEST(empty_text_handling);
    RUN_TEST(long_text_truncation);

    printf("\nValue Preservation:\n");
    RUN_TEST(position_values);
    RUN_TEST(type_style_values);
    RUN_TEST(sprite_and_flags_values);
    RUN_TEST(queue_order_preserved);

    printf("\nEntry Independence:\n");
    RUN_TEST(independent_entries);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
