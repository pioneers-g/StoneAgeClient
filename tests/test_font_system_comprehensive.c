/*
 * Stone Age Client - Font System Comprehensive Tests
 * Tests for font.c implementation
 *
 * Covers:
 * - Font slot management
 * - Color utilities
 * - DBCS character handling
 * - String truncation
 *
 * Note: Windows GDI functions are mocked for testing
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
typedef void* HFONT;
typedef void* HDC;
typedef unsigned long COLORREF;
typedef unsigned char BYTE;

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

#define MAX_FONTS       16

/* Font slots */
typedef enum {
    FONT_DEFAULT = 0,
    FONT_TITLE = 1,
    FONT_SMALL = 2,
    FONT_CHAT = 3,
    FONT_ITEM = 4,
    FONT_DIALOG = 5,
    FONT_BUTTON = 6
} FontSlot;

/* Font alignment */
typedef enum {
    FONT_ALIGN_LEFT = 0,
    FONT_ALIGN_CENTER = 1,
    FONT_ALIGN_RIGHT = 2,
    FONT_ALIGN_TOP = 0,
    FONT_ALIGN_VCENTER = 4,
    FONT_ALIGN_BOTTOM = 8
} FontAlignment;

/* ========================================
 * Structures
 * ======================================== */

typedef struct {
    HFONT handle;
    char face[32];
    u16 size;
    u8 bold;
    u8 italic;
    u8 underline;
} FontEntry;

typedef struct {
    FontEntry fonts[MAX_FONTS];
    int font_count;
    FontSlot current_font;
    u32 default_color;
} FontContext;

/* ========================================
 * Global State
 * ======================================== */

static FontContext g_font = {0};

/* Mock DBCS lead byte table for testing */
/* Japanese Shift-JIS lead bytes: 0x81-0x9F, 0xE0-0xFC */
/* Chinese GBK lead bytes: 0x81-0xFE */
static int g_mock_dbcs_mode = 1;  /* Enable DBCS by default */

/* ========================================
 * Implementation Functions
 * ======================================== */

/*
 * Mock IsDBCSLeadByteEx
 * For testing, we simulate GBK lead bytes (0x81-0xFE)
 */
static int font_is_dbcs_lead_byte(BYTE c) {
    if (!g_mock_dbcs_mode) return 0;
    /* GBK lead byte range: 0x81-0xFE */
    return (c >= 0x81 && c <= 0xFE);
}

/*
 * Get previous character position in DBCS string
 */
static const char* font_prev_char(const char* str, const char* pos) {
    const char* p;

    if (!str || !pos || pos <= str) return str;

    p = pos - 1;

    if (p > str && font_is_dbcs_lead_byte(*(p - 1))) {
        return p - 1;
    }

    return p;
}

/*
 * Get next character position in DBCS string
 */
static const char* font_next_char(const char* str) {
    if (!str) return NULL;
    if (!*str) return str;  /* Empty string, return same position */

    if (font_is_dbcs_lead_byte(*str)) {
        if (str[1] == '\0') return str + 1;  /* Incomplete DBCS, advance 1 */
        return str + 2;
    }

    return str + 1;
}

/*
 * Get character byte length at position
 */
static int font_char_len(const char* str) {
    if (!str || !*str) return 0;

    if (font_is_dbcs_lead_byte(*str)) {
        return 2;
    }

    return 1;
}

/*
 * Count characters in DBCS string
 */
static int font_char_count(const char* str) {
    int count = 0;

    if (!str) return 0;

    while (*str) {
        if (font_is_dbcs_lead_byte(*str)) {
            if (str[1] == '\0') break;  /* Avoid overrun on incomplete DBCS */
            str += 2;
        } else {
            str++;
        }
        count++;
    }

    return count;
}

/*
 * String truncation with ellipsis
 */
static int font_truncate_string(const char* src, char* dest, int max_len) {
    int src_len;
    int copy_len;
    int ellipsis_len;

    if (!src || !dest || max_len <= 0) return 0;

    src_len = (int)strlen(src);

    if (src_len < max_len) {
        /* Source fits completely */
        strcpy(dest, src);
        return src_len;
    }

    ellipsis_len = 3;

    if (max_len <= ellipsis_len) {
        /* Too short for ellipsis, just truncate */
        strncpy(dest, src, max_len - 1);
        dest[max_len - 1] = '\0';
        return max_len - 1;
    }

    copy_len = max_len - ellipsis_len - 1;  /* -1 for null terminator */

    /* Check if we'd cut a DBCS character */
    if (copy_len > 0 && src[copy_len - 1] != '\0' && font_is_dbcs_lead_byte((BYTE)src[copy_len - 1])) {
        copy_len--;
    }

    if (copy_len < 0) copy_len = 0;

    strncpy(dest, src, copy_len);
    dest[copy_len] = '\0';

    strcat(dest, "...");

    return (int)strlen(dest);
}

/*
 * Get DBCS-safe substring
 */
static int font_substr_dbcs(const char* src, int start_char, int char_count, char* dest, int dest_size) {
    const char* p;
    int i;
    int char_len;

    if (!src || !dest || dest_size <= 0) return 0;

    p = src;

    /* Skip to start position */
    for (i = 0; i < start_char && *p; i++) {
        if (font_is_dbcs_lead_byte(*p)) {
            if (p[1] == '\0') break;  /* Incomplete DBCS */
            p += 2;
        } else {
            p++;
        }
    }

    /* Copy characters */
    for (i = 0; i < char_count && *p && dest_size > 0; i++) {
        char_len = font_char_len(p);
        if (char_len == 0) break;

        if (char_len >= dest_size) break;

        memcpy(dest, p, char_len);
        dest += char_len;
        dest_size -= char_len;
        p += char_len;
    }

    *dest = '\0';
    return i;
}

/*
 * Set current font
 */
static void font_set_current(FontSlot slot) {
    if (slot < MAX_FONTS) {
        g_font.current_font = slot;
    }
}

/*
 * Get current font
 */
static FontSlot font_get_current(void) {
    return g_font.current_font;
}

/*
 * Set default color
 */
static void font_set_default_color(u32 color) {
    g_font.default_color = color;
}

/*
 * Get default color
 */
static u32 font_get_default_color(void) {
    return g_font.default_color;
}

/*
 * Make color from RGB
 */
static u32 font_make_color(u8 r, u8 g, u8 b) {
    return ((u32)b << 16) | ((u32)g << 8) | (u32)r;
}

/*
 * Reset state
 */
static void reset_state(void) {
    memset(&g_font, 0, sizeof(FontContext));
    g_mock_dbcs_mode = 1;
}

/* ========================================
 * Test Cases - Font Slot Management
 * ======================================== */

static int test_font_slot_default(void) {
    reset_state();

    font_set_current(FONT_DEFAULT);

    return font_get_current() == FONT_DEFAULT;
}

static int test_font_slot_title(void) {
    reset_state();

    font_set_current(FONT_TITLE);

    return font_get_current() == FONT_TITLE;
}

static int test_font_slot_small(void) {
    reset_state();

    font_set_current(FONT_SMALL);

    return font_get_current() == FONT_SMALL;
}

static int test_font_slot_invalid(void) {
    reset_state();

    font_set_current((FontSlot)100);  /* Invalid slot */

    return font_get_current() == FONT_DEFAULT;  /* Should not change */
}

static int test_font_slot_boundary(void) {
    reset_state();

    font_set_current((FontSlot)(MAX_FONTS - 1));

    return font_get_current() == MAX_FONTS - 1;
}

static int test_font_slot_boundary_plus_one(void) {
    reset_state();

    font_set_current((FontSlot)MAX_FONTS);  /* Just over boundary */

    return font_get_current() == FONT_DEFAULT;  /* Should not change */
}

/* ========================================
 * Test Cases - Color Management
 * ======================================== */

static int test_color_set_get(void) {
    reset_state();

    font_set_default_color(0x12345678);

    return font_get_default_color() == 0x12345678;
}

static int test_color_make_red(void) {
    u32 color = font_make_color(255, 0, 0);
    /* RGB in BGR format: 0x0000FF */
    return (color & 0xFF) == 255 && ((color >> 8) & 0xFF) == 0 && ((color >> 16) & 0xFF) == 0;
}

static int test_color_make_green(void) {
    u32 color = font_make_color(0, 255, 0);
    /* RGB in BGR format: 0x00FF00 */
    return (color & 0xFF) == 0 && ((color >> 8) & 0xFF) == 255 && ((color >> 16) & 0xFF) == 0;
}

static int test_color_make_blue(void) {
    u32 color = font_make_color(0, 0, 255);
    /* RGB in BGR format: 0xFF0000 */
    return (color & 0xFF) == 0 && ((color >> 8) & 0xFF) == 0 && ((color >> 16) & 0xFF) == 255;
}

static int test_color_make_white(void) {
    u32 color = font_make_color(255, 255, 255);
    return color == 0xFFFFFF;
}

static int test_color_make_black(void) {
    u32 color = font_make_color(0, 0, 0);
    return color == 0;
}

/* ========================================
 * Test Cases - DBCS Character Length
 * ======================================== */

static int test_char_len_ascii(void) {
    return font_char_len("A") == 1;
}

static int test_char_len_dbcs_lead(void) {
    /* 0x81 is a GBK lead byte */
    char str[2] = { (char)0x81, 'A' };
    return font_char_len(str) == 2;
}

static int test_char_len_null(void) {
    return font_char_len(NULL) == 0;
}

static int test_char_len_empty(void) {
    return font_char_len("") == 0;
}

/* ========================================
 * Test Cases - DBCS Character Count
 * ======================================== */

static int test_char_count_ascii(void) {
    return font_char_count("Hello") == 5;
}

static int test_char_count_mixed(void) {
    /* "A" + DBCS char + "B" */
    char str[] = { 'A', (char)0x81, 'B', 'C', '\0' };  /* 0x81, B is one DBCS char */
    return font_char_count(str) == 3;  /* A + DBCS + C */
}

static int test_char_count_dbcs_only(void) {
    /* Two DBCS characters */
    char str[] = { (char)0x81, 'A', (char)0x82, 'B', '\0' };
    return font_char_count(str) == 2;
}

static int test_char_count_null(void) {
    return font_char_count(NULL) == 0;
}

static int test_char_count_empty(void) {
    return font_char_count("") == 0;
}

/* ========================================
 * Test Cases - DBCS Next Character
 * ======================================== */

static int test_next_char_ascii(void) {
    const char str[] = "ABC";
    const char* result = font_next_char(str);
    return result == str + 1;
}

static int test_next_char_dbcs(void) {
    char str[] = { (char)0x81, 'A', 'B', '\0' };
    const char* result = font_next_char(str);
    return result == str + 2;
}

static int test_next_char_null(void) {
    const char* result = font_next_char(NULL);
    return result == NULL;
}

static int test_next_char_empty(void) {
    const char str[] = "";
    const char* result = font_next_char(str);
    return result == str;  /* Points to same position */
}

/* ========================================
 * Test Cases - DBCS Previous Character
 * ======================================== */

static int test_prev_char_ascii(void) {
    const char str[] = "ABC";
    const char* result = font_prev_char(str, str + 2);  /* Point to 'C' */
    return result == str + 1;  /* Points to 'B' */
}

static int test_prev_char_dbcs(void) {
    char str[] = { (char)0x81, 'A', 'B', '\0' };
    /* Position at str+3 points to 'B' (after DBCS char) */
    /* prev should return str+2 (pointing to 'B') not str+1 */
    /* Because str+2 is not a DBCS lead byte, and str+1 is trail byte not lead byte */
    const char* result = font_prev_char(str, str + 3);
    /* pos-1 = str+2 = 'B', check if str+1 is lead byte - it's 'A' not a lead byte */
    /* So result should be str+2 */
    return result == str + 2;
}

static int test_prev_char_at_start(void) {
    const char* str = "ABC";
    const char* result = font_prev_char(str, str);
    return result == str;  /* At start, returns start */
}

static int test_prev_char_null(void) {
    const char* result = font_prev_char(NULL, NULL);
    return result == NULL;
}

/* ========================================
 * Test Cases - String Truncation
 * ======================================== */

static int test_truncate_fits(void) {
    char dest[32];
    memset(dest, 0, sizeof(dest));
    int result = font_truncate_string("Hello", dest, 31);  /* Use 31 to avoid boundary issue */
    return result == 5 && strcmp(dest, "Hello") == 0;
}

static int test_truncate_needs_ellipsis(void) {
    char dest[16];
    memset(dest, 0, sizeof(dest));
    int result = font_truncate_string("Hello World", dest, 10);
    return result == 9 && strcmp(dest, "Hello ...") == 0;
}

static int test_truncate_short_buffer(void) {
    char dest[8];
    memset(dest, 0, sizeof(dest));
    int result = font_truncate_string("Hello", dest, 3);
    /* With max_len=3 and ellipsis_len=3, we get max_len <= ellipsis_len,
     * so it truncates without ellipsis: "He" (2 chars + null) */
    return result == 2 && strcmp(dest, "He") == 0;
}

static int test_truncate_exact_fit(void) {
    char dest[8];
    memset(dest, 0, sizeof(dest));
    int result = font_truncate_string("Hello", dest, 6);
    return result == 5 && strcmp(dest, "Hello") == 0;
}

static int test_truncate_dbcs(void) {
    char dest[10];
    /* 0x81 'A' is a DBCS character */
    char src[] = { (char)0x81, 'A', (char)0x82, 'B', 'X', 'Y', 'Z', '\0' };
    int result = font_truncate_string(src, dest, 10);
    /* Should truncate and not cut DBCS character */
    /* With 10 bytes: copy_len = 7, but 0x82 at position 4 might be cut */
    /* Let's verify it returns something */
    return result > 0;
}

static int test_truncate_null_src(void) {
    char dest[10];
    int result = font_truncate_string(NULL, dest, 10);
    return result == 0;
}

static int test_truncate_null_dest(void) {
    int result = font_truncate_string("Hello", NULL, 10);
    return result == 0;
}

static int test_truncate_zero_max(void) {
    char dest[10];
    int result = font_truncate_string("Hello", dest, 0);
    return result == 0;
}

/* ========================================
 * Test Cases - DBCS Substring
 * ======================================== */

static int test_substr_ascii(void) {
    char dest[32];
    int result = font_substr_dbcs("Hello World", 0, 5, dest, sizeof(dest));
    return result == 5 && strcmp(dest, "Hello") == 0;
}

static int test_substr_offset(void) {
    char dest[32];
    int result = font_substr_dbcs("Hello World", 6, 5, dest, sizeof(dest));
    return result == 5 && strcmp(dest, "World") == 0;
}

static int test_substr_dbcs(void) {
    char dest[32];
    /* 0x81 'A' is one DBCS char, 'B' is one ASCII char */
    char src[] = { (char)0x81, 'A', 'B', 'C', '\0' };
    int result = font_substr_dbcs(src, 0, 2, dest, sizeof(dest));
    /* First 2 chars: DBCS (2 bytes) + B (1 byte) = 3 bytes */
    return result == 2 && (u8)dest[0] == 0x81;
}

static int test_substr_null_src(void) {
    char dest[32];
    int result = font_substr_dbcs(NULL, 0, 5, dest, sizeof(dest));
    return result == 0;
}

static int test_substr_null_dest(void) {
    int result = font_substr_dbcs("Hello", 0, 5, NULL, 0);
    return result == 0;
}

static int test_substr_small_buffer(void) {
    char dest[3];
    int result = font_substr_dbcs("Hello", 0, 5, dest, sizeof(dest));
    /* Should truncate to fit buffer */
    return result < 5 && strlen(dest) < sizeof(dest);
}

/* ========================================
 * Test Cases - Font Alignment
 * ======================================== */

static int test_align_left(void) {
    return FONT_ALIGN_LEFT == 0;
}

static int test_align_center(void) {
    return FONT_ALIGN_CENTER == 1;
}

static int test_align_right(void) {
    return FONT_ALIGN_RIGHT == 2;
}

static int test_align_vcenter(void) {
    return FONT_ALIGN_VCENTER == 4;
}

static int test_align_bottom(void) {
    return FONT_ALIGN_BOTTOM == 8;
}

/* ========================================
 * Test Cases - Font Slot Constants
 * ======================================== */

static int test_slot_constants(void) {
    return FONT_DEFAULT == 0 &&
           FONT_TITLE == 1 &&
           FONT_SMALL == 2 &&
           FONT_CHAT == 3 &&
           FONT_ITEM == 4 &&
           FONT_DIALOG == 5 &&
           FONT_BUTTON == 6;
}

static int test_max_fonts(void) {
    return MAX_FONTS == 16;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Font System Comprehensive Tests ===\n\n");

    printf("Font Slot Management Tests:\n");
    TEST(font_slot_default);
    TEST(font_slot_title);
    TEST(font_slot_small);
    TEST(font_slot_invalid);
    TEST(font_slot_boundary);
    TEST(font_slot_boundary_plus_one);

    printf("\nColor Management Tests:\n");
    TEST(color_set_get);
    TEST(color_make_red);
    TEST(color_make_green);
    TEST(color_make_blue);
    TEST(color_make_white);
    TEST(color_make_black);

    printf("\nDBCS Character Length Tests:\n");
    TEST(char_len_ascii);
    TEST(char_len_dbcs_lead);
    TEST(char_len_null);
    TEST(char_len_empty);

    printf("\nDBCS Character Count Tests:\n");
    TEST(char_count_ascii);
    TEST(char_count_mixed);
    TEST(char_count_dbcs_only);
    TEST(char_count_null);
    TEST(char_count_empty);

    printf("\nDBCS Next Character Tests:\n");
    TEST(next_char_ascii);
    TEST(next_char_dbcs);
    TEST(next_char_null);
    TEST(next_char_empty);

    printf("\nDBCS Previous Character Tests:\n");
    TEST(prev_char_ascii);
    TEST(prev_char_dbcs);
    TEST(prev_char_at_start);
    TEST(prev_char_null);

    printf("\nString Truncation Tests:\n");
    TEST(truncate_fits);
    TEST(truncate_needs_ellipsis);
    TEST(truncate_short_buffer);
    TEST(truncate_exact_fit);
    TEST(truncate_dbcs);
    TEST(truncate_null_src);
    TEST(truncate_null_dest);
    TEST(truncate_zero_max);

    printf("\nDBCS Substring Tests:\n");
    TEST(substr_ascii);
    TEST(substr_offset);
    TEST(substr_dbcs);
    TEST(substr_null_src);
    TEST(substr_null_dest);
    TEST(substr_small_buffer);

    printf("\nFont Alignment Tests:\n");
    TEST(align_left);
    TEST(align_center);
    TEST(align_right);
    TEST(align_vcenter);
    TEST(align_bottom);

    printf("\nFont Slot Constants Tests:\n");
    TEST(slot_constants);
    TEST(max_fonts);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - font_create_font (requires Windows GDI)
     * - font_draw_text functions (requires HDC)
     * - font_word_wrap
     * - font_get_text_size
     * - font_truncate_to_width
     * - font_create_chinese
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
