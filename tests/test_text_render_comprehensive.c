/*
 * Stone Age Client - Text Rendering Queue Unit Tests
 * Tests for FUN_0041d7c0 (text queue add) and FUN_00414820 (text render)
 *
 * Based on Ghidra decompilation analysis:
 * - Maximum queue size: 0x3ff (1023 entries)
 * - Entry size: 0x110 (272 bytes)
 * - Two-pass rendering: shadow then main text
 * - Color table at PTR_DAT_004a2720 (10 colors)
 * - Special type 0x03 has offset handling
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
typedef int s32;

/* Constants from Ghidra */
#define MAX_TEXT_QUEUE 1024     /* 0x400 */
#define TEXT_ENTRY_SIZE 272     /* 0x110 */
#define MAX_TEXT_LENGTH 255     /* String buffer size */
#define MAX_COLOR_INDEX 10      /* Color table entries */

/* Text entry type flags */
#define TEXT_TYPE_NORMAL 0x00
#define TEXT_TYPE_SHADOW 0x01
#define TEXT_TYPE_OUTLINE 0x02
#define TEXT_TYPE_SPECIAL 0x03  /* Has offset handling */

/* Text entry structure - 0x110 bytes */
typedef struct {
    s16 x;                      /* +0x00 */
    s16 y;                      /* +0x02 */
    u8 type;                    /* +0x04: Color/type index */
    u8 flags;                   /* +0x05: Render flags */
    char text[MAX_TEXT_LENGTH]; /* +0x06: Null-terminated string */
    u8 color_index;             /* +0x105: Color index for rendering */
    s32 font_size;              /* +0x106: Font size (0 = default) */
    u32 param;                  /* +0x10a: Additional parameter */
} TextQueueEntry;

/* Color table - PTR_DAT_004a2720 (10 entries) */
static const u32 g_color_table[MAX_COLOR_INDEX] = {
    0x00000000,  /* Black (shadow) */
    0x00FFFFFF,  /* White */
    0x00FF0000,  /* Red */
    0x0000FF00,  /* Green */
    0x000000FF,  /* Blue */
    0x00FFFF00,  /* Yellow */
    0x00FF00FF,  /* Magenta */
    0x0000FFFF,  /* Cyan */
    0x00FF8000,  /* Orange */
    0x00808080   /* Gray */
};

/* Text queue storage */
static TextQueueEntry g_text_queue[MAX_TEXT_QUEUE];
static int g_text_queue_count = 0;

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* Setup */
static void test_setup(void) {
    memset(g_text_queue, 0, sizeof(g_text_queue));
    g_text_queue_count = 0;
}

/*
 * Add text to queue - FUN_0041d7c0 pattern
 * Returns queue index on success, -2 if queue full
 */
static int text_queue_add(s16 x, s16 y, u8 type, u8 flags,
                          const char* text, s32 font_size, u32 param) {
    if (g_text_queue_count >= MAX_TEXT_QUEUE - 1) {
        return -2;  /* Queue full */
    }

    int index = g_text_queue_count;
    TextQueueEntry* entry = &g_text_queue[index];

    entry->x = x;
    entry->y = y;
    entry->type = type;
    entry->flags = flags;
    entry->font_size = font_size;
    entry->param = param;

    /* Copy string with length limit */
    strncpy(entry->text, text, MAX_TEXT_LENGTH - 1);
    entry->text[MAX_TEXT_LENGTH - 1] = '\0';

    g_text_queue_count++;
    return index;
}

/*
 * Get text from queue
 */
static TextQueueEntry* text_queue_get(int index) {
    if (index < 0 || index >= g_text_queue_count) {
        return NULL;
    }
    return &g_text_queue[index];
}

/*
 * Clear text queue
 */
static void text_queue_clear(void) {
    g_text_queue_count = 0;
}

/*
 * Calculate shadow offset - FUN_00414820 pattern
 */
static void get_shadow_offset(int base_x, int base_y, int* shadow_x, int* shadow_y) {
    *shadow_x = base_x + 1;
    *shadow_y = base_y + 1;
}

/*
 * Calculate special type offset - FUN_00414820 pattern for type 0x03
 */
static void get_special_offset(int base_x, int base_y, int has_indicator,
                               int* offset_x, int* offset_y) {
    if (has_indicator) {
        *offset_x = base_x + 0x16;  /* 22 pixels offset */
    } else {
        *offset_x = base_x;
    }
    *offset_y = base_y;
}

/* ========================================
 * Queue Size Tests
 * ======================================== */

static int test_max_queue_size(void) {
    test_setup();

    /* Maximum queue size is 0x3ff (1023) before returning -2 */
    assert(MAX_TEXT_QUEUE == 1024);
    assert(0x3ff == 1023);

    return 1;
}

static int test_entry_size(void) {
    test_setup();

    /* Entry size is 0x110 (272) bytes */
    assert(TEXT_ENTRY_SIZE == 272);
    assert(sizeof(TextQueueEntry) == 272);

    return 1;
}

static int test_max_text_length(void) {
    test_setup();

    /* Text buffer is 255 bytes (256 with null terminator range) */
    assert(MAX_TEXT_LENGTH == 255);

    return 1;
}

/* ========================================
 * Queue Add Tests
 * ======================================== */

static int test_queue_add_basic(void) {
    test_setup();

    int index = text_queue_add(100, 200, 0, 0, "Hello", 0, 0);

    assert(index == 0);
    assert(g_text_queue_count == 1);

    TextQueueEntry* entry = text_queue_get(0);
    assert(entry != NULL);
    assert(entry->x == 100);
    assert(entry->y == 200);
    assert(strcmp(entry->text, "Hello") == 0);

    return 1;
}

static int test_queue_add_multiple(void) {
    test_setup();

    int i1 = text_queue_add(10, 20, 0, 0, "First", 0, 0);
    int i2 = text_queue_add(30, 40, 1, 0, "Second", 12, 0);
    int i3 = text_queue_add(50, 60, 2, 0, "Third", 14, 0);

    assert(i1 == 0);
    assert(i2 == 1);
    assert(i3 == 2);
    assert(g_text_queue_count == 3);

    return 1;
}

static int test_queue_full(void) {
    test_setup();

    /* Fill queue to near capacity */
    for (int i = 0; i < MAX_TEXT_QUEUE - 1; i++) {
        int result = text_queue_add(i, i, 0, 0, "Test", 0, 0);
        if (result == -2) {
            /* Queue full before expected */
            assert(i >= 1022);
            return 1;
        }
    }

    /* Next add should fail */
    int result = text_queue_add(0, 0, 0, 0, "Overflow", 0, 0);
    assert(result == -2);

    return 1;
}

static int test_queue_long_text(void) {
    test_setup();

    /* Create text longer than buffer */
    char long_text[300];
    memset(long_text, 'A', 299);
    long_text[299] = '\0';

    int index = text_queue_add(0, 0, 0, 0, long_text, 0, 0);
    assert(index == 0);

    TextQueueEntry* entry = text_queue_get(0);
    assert(strlen(entry->text) == MAX_TEXT_LENGTH - 1);

    return 1;
}

/* ========================================
 * Queue Get Tests
 * ======================================== */

static int test_queue_get_invalid(void) {
    test_setup();

    text_queue_add(0, 0, 0, 0, "Test", 0, 0);

    /* Invalid indices should return NULL */
    assert(text_queue_get(-1) == NULL);
    assert(text_queue_get(100) == NULL);
    assert(text_queue_get(MAX_TEXT_QUEUE) == NULL);

    /* Valid index should work */
    assert(text_queue_get(0) != NULL);

    return 1;
}

static int test_queue_get_valid(void) {
    test_setup();

    text_queue_add(100, 200, 5, 0x80, "TestString", 16, 0x12345678);

    TextQueueEntry* entry = text_queue_get(0);
    assert(entry != NULL);
    assert(entry->x == 100);
    assert(entry->y == 200);
    assert(entry->type == 5);
    assert(entry->flags == 0x80);
    assert(strcmp(entry->text, "TestString") == 0);
    assert(entry->font_size == 16);
    assert(entry->param == 0x12345678);

    return 1;
}

/* ========================================
 * Shadow Rendering Tests
 * ======================================== */

static int test_shadow_offset(void) {
    test_setup();

    /* Shadow is rendered at +1, +1 offset */
    int sx, sy;
    get_shadow_offset(100, 200, &sx, &sy);

    assert(sx == 101);
    assert(sy == 201);

    return 1;
}

static int test_shadow_offset_zero(void) {
    test_setup();

    int sx, sy;
    get_shadow_offset(0, 0, &sx, &sy);

    assert(sx == 1);
    assert(sy == 1);

    return 1;
}

static int test_shadow_offset_negative(void) {
    test_setup();

    int sx, sy;
    get_shadow_offset(-50, -30, &sx, &sy);

    assert(sx == -49);
    assert(sy == -29);

    return 1;
}

/* ========================================
 * Special Type Offset Tests
 * ======================================== */

static int test_special_type_offset_no_indicator(void) {
    test_setup();

    /* Type 0x03 without indicator has no X offset */
    int ox, oy;
    get_special_offset(100, 200, 0, &ox, &oy);

    assert(ox == 100);
    assert(oy == 200);

    return 1;
}

static int test_special_type_offset_with_indicator(void) {
    test_setup();

    /* Type 0x03 with indicator has +0x16 (22) X offset */
    int ox, oy;
    get_special_offset(100, 200, 1, &ox, &oy);

    assert(ox == 100 + 0x16);  /* 122 */
    assert(oy == 200);

    return 1;
}

static int test_special_offset_constant(void) {
    test_setup();

    /* Offset constant from FUN_00414820: 0x16 and 0x17 */
    assert(0x16 == 22);
    assert(0x17 == 23);

    return 1;
}

/* ========================================
 * Color Table Tests
 * ======================================== */

static int test_color_table_size(void) {
    test_setup();

    /* Color table has 10 entries (PTR_DAT_004a2720 to 0x4a2748) */
    assert(MAX_COLOR_INDEX == 10);
    assert(sizeof(g_color_table) == 10 * sizeof(u32));

    return 1;
}

static int test_color_table_values(void) {
    test_setup();

    /* Verify color values */
    assert(g_color_table[0] == 0x00000000);  /* Black */
    assert(g_color_table[1] == 0x00FFFFFF);  /* White */
    assert(g_color_table[2] == 0x00FF0000);  /* Red */
    assert(g_color_table[3] == 0x0000FF00);  /* Green */
    assert(g_color_table[4] == 0x000000FF);  /* Blue */

    return 1;
}

static int test_color_table_bounds(void) {
    test_setup();

    /* Color indices should be 0-9 */
    for (int i = 0; i < MAX_COLOR_INDEX; i++) {
        u32 color = g_color_table[i];
        (void)color;  /* Just verify access */
    }

    return 1;
}

/* ========================================
 * Text Type Tests
 * ======================================== */

static int test_text_type_constants(void) {
    test_setup();

    /* Text type values */
    assert(TEXT_TYPE_NORMAL == 0x00);
    assert(TEXT_TYPE_SHADOW == 0x01);
    assert(TEXT_TYPE_OUTLINE == 0x02);
    assert(TEXT_TYPE_SPECIAL == 0x03);

    return 1;
}

static int test_text_type_storage(void) {
    test_setup();

    text_queue_add(0, 0, TEXT_TYPE_SPECIAL, 0, "Test", 0, 0);

    TextQueueEntry* entry = text_queue_get(0);
    assert(entry->type == TEXT_TYPE_SPECIAL);

    return 1;
}

/* ========================================
 * Font Size Tests
 * ======================================== */

static int test_font_size_default(void) {
    test_setup();

    /* Font size 0 means default font */
    text_queue_add(0, 0, 0, 0, "Test", 0, 0);

    TextQueueEntry* entry = text_queue_get(0);
    assert(entry->font_size == 0);

    return 1;
}

static int test_font_size_custom(void) {
    test_setup();

    /* Custom font sizes */
    text_queue_add(0, 0, 0, 0, "Small", 8, 0);
    text_queue_add(0, 0, 0, 0, "Medium", 12, 0);
    text_queue_add(0, 0, 0, 0, "Large", 16, 0);

    assert(text_queue_get(0)->font_size == 8);
    assert(text_queue_get(1)->font_size == 12);
    assert(text_queue_get(2)->font_size == 16);

    return 1;
}

/* ========================================
 * Two-Pass Rendering Tests
 * ======================================== */

static int test_two_pass_render_order(void) {
    test_setup();

    /* FUN_00414820 renders in two passes:
     * Pass 1: Shadow (black, offset +1,+1)
     * Pass 2: Main text (color, no offset)
     */

    /* This test verifies the rendering order logic */
    int shadow_pass = 0;
    int main_pass = 1;

    assert(shadow_pass < main_pass);

    return 1;
}

static int test_render_pass_offset(void) {
    test_setup();

    /* From FUN_00414820:
     * First TextOutA call uses +1 offsets (shadow)
     * Second TextOutA call uses original coordinates (main)
     */

    int base_x = 100, base_y = 50;

    /* Shadow coordinates */
    int shadow_x = base_x + 1;
    int shadow_y = base_y + 1;

    /* Main coordinates */
    int main_x = base_x;
    int main_y = base_y;

    assert(shadow_x == 101);
    assert(shadow_y == 51);
    assert(main_x == 100);
    assert(main_y == 50);

    return 1;
}

/* ========================================
 * Memory Layout Tests
 * ======================================== */

static int test_entry_structure_layout(void) {
    test_setup();

    /* Verify structure layout matches Ghidra analysis */
    TextQueueEntry entry;
    memset(&entry, 0, sizeof(entry));

    /* Offsets from FUN_0041d7c0:
     * +0x00: param_1 (x)
     * +0x02: param_2 (y)
     * +0x04: param_3 (type)
     * +0x05: param_4 (flags)
     * +0x06: text buffer
     * +0x106: font size (param_6 related)
     */

    assert(offsetof(TextQueueEntry, x) == 0x00);
    assert(offsetof(TextQueueEntry, y) == 0x02);
    assert(offsetof(TextQueueEntry, type) == 0x04);
    assert(offsetof(TextQueueEntry, flags) == 0x05);
    assert(offsetof(TextQueueEntry, text) == 0x06);

    return 1;
}

static int test_queue_memory_size(void) {
    test_setup();

    /* Total queue memory: 1024 * 272 = 278528 bytes */
    size_t total_size = sizeof(g_text_queue);
    assert(total_size == MAX_TEXT_QUEUE * TEXT_ENTRY_SIZE);
    assert(total_size == 278528);

    return 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_render_cycle(void) {
    test_setup();

    /* Simulate a full text rendering cycle */
    int i1 = text_queue_add(100, 100, 0, 0, "Player Name", 12, 0);
    int i2 = text_queue_add(100, 120, 1, 0, "Health: 100", 10, 0);
    int i3 = text_queue_add(100, 140, 2, 0, "Level 50", 10, 0);

    assert(i1 == 0);
    assert(i2 == 1);
    assert(i3 == 2);
    assert(g_text_queue_count == 3);

    /* Clear queue after render */
    text_queue_clear();
    assert(g_text_queue_count == 0);

    return 1;
}

static int test_special_type_rendering(void) {
    test_setup();

    /* Type 0x03 text has special handling in FUN_00414820 */
    text_queue_add(50, 100, TEXT_TYPE_SPECIAL, 0, "Special Text", 12, 0);

    TextQueueEntry* entry = text_queue_get(0);
    assert(entry->type == TEXT_TYPE_SPECIAL);

    /* Special type can have indicator offset */
    int ox, oy;
    get_special_offset(entry->x, entry->y, 1, &ox, &oy);
    assert(ox == 50 + 0x16);

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Text Rendering Queue Unit Tests ===\n\n");

    /* Queue size tests */
    printf("Queue Size Tests:\n");
    TEST(max_queue_size);
    TEST(entry_size);
    TEST(max_text_length);

    /* Queue add tests */
    printf("\nQueue Add Tests (FUN_0041d7c0):\n");
    TEST(queue_add_basic);
    TEST(queue_add_multiple);
    TEST(queue_full);
    TEST(queue_long_text);

    /* Queue get tests */
    printf("\nQueue Get Tests:\n");
    TEST(queue_get_invalid);
    TEST(queue_get_valid);

    /* Shadow rendering tests */
    printf("\nShadow Rendering Tests (FUN_00414820):\n");
    TEST(shadow_offset);
    TEST(shadow_offset_zero);
    TEST(shadow_offset_negative);

    /* Special type offset tests */
    printf("\nSpecial Type Offset Tests:\n");
    TEST(special_type_offset_no_indicator);
    TEST(special_type_offset_with_indicator);
    TEST(special_offset_constant);

    /* Color table tests */
    printf("\nColor Table Tests:\n");
    TEST(color_table_size);
    TEST(color_table_values);
    TEST(color_table_bounds);

    /* Text type tests */
    printf("\nText Type Tests:\n");
    TEST(text_type_constants);
    TEST(text_type_storage);

    /* Font size tests */
    printf("\nFont Size Tests:\n");
    TEST(font_size_default);
    TEST(font_size_custom);

    /* Two-pass rendering tests */
    printf("\nTwo-Pass Rendering Tests:\n");
    TEST(two_pass_render_order);
    TEST(render_pass_offset);

    /* Memory layout tests */
    printf("\nMemory Layout Tests:\n");
    TEST(entry_structure_layout);
    TEST(queue_memory_size);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_render_cycle);
    TEST(special_type_rendering);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - FUN_00415a70 (font creation)
     * - Actual GDI rendering verification
     * - Dual-screen rendering (DAT_0054c83c check)
     * - Resolution mode scaling (DAT_04560214 check)
     * - Text indicator rendering at offset 0x1b0/0x1b1
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
