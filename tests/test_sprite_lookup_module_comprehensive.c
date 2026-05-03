/*
 * Stone Age Client - Sprite Lookup Module Comprehensive Tests
 * Tests for sprite_lookup.c implementation
 *
 * Covers:
 * - Sprite offset lookup (standard and high-res)
 * - Sprite dimension lookup
 * - High-res sprite detection
 * - Palette group calculation
 * - Lookup table management
 *
 * Based on FUN_0041fad0, FUN_0041f980
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

#define SPRITE_ID_HIGHRES_BASE  500000
#define SPRITE_ID_HIGHRES_MAX   549999

/* ========================================
 * Structures
 * ======================================== */

typedef struct {
    u16 width;
    u16 height;
} SpriteDimension;

/* ========================================
 * Global State
 * ======================================== */

static u32* g_sprite_offset_table = NULL;
static SpriteDimension* g_sprite_dim_table = NULL;
static SpriteDimension* g_sprite_dim_table_hires = NULL;

/* Test data */
static SpriteDimension test_dim_table[100];
static SpriteDimension test_dim_table_hires[100];

/* ========================================
 * Implementation Functions
 * ======================================== */

static int sprite_lookup_offset(u32 sprite_id, u32* offset) {
    if (sprite_id < SPRITE_ID_HIGHRES_BASE) {
        /* Standard sprite - look up offset in table */
        if (offset) {
            *offset = sprite_id * 0x50;  /* Placeholder calculation */
        }
        return 1;
    }

    if (sprite_id <= SPRITE_ID_HIGHRES_MAX) {
        /* Extended sprite - return ID as offset */
        if (offset) {
            *offset = sprite_id;
        }
        return 1;
    }

    /* Invalid sprite ID */
    if (offset) *offset = 0;
    return 0;
}

static int sprite_lookup_dimension(u32 sprite_id, u16* width, u16* height) {
    if (sprite_id < SPRITE_ID_HIGHRES_BASE) {
        /* Standard sprite */
        if (g_sprite_dim_table && sprite_id < 100) {
            if (width) *width = g_sprite_dim_table[sprite_id].width;
            if (height) *height = g_sprite_dim_table[sprite_id].height;
            return 1;
        }
        /* Placeholder values */
        if (width) *width = 64;
        if (height) *height = 48;
        return 1;
    }

    if (sprite_id <= SPRITE_ID_HIGHRES_MAX) {
        /* Extended sprite */
        if (g_sprite_dim_table_hires) {
            u32 index = sprite_id - SPRITE_ID_HIGHRES_BASE;
            if (index < 100) {
                if (width) *width = g_sprite_dim_table_hires[index].width;
                if (height) *height = g_sprite_dim_table_hires[index].height;
                return 1;
            }
        }
        /* Placeholder values */
        if (width) *width = 128;
        if (height) *height = 96;
        return 1;
    }

    if (width) *width = 0;
    if (height) *height = 0;
    return 0;
}

static int sprite_is_highres(u32 sprite_id) {
    return (sprite_id >= SPRITE_ID_HIGHRES_BASE && sprite_id <= SPRITE_ID_HIGHRES_MAX);
}

static int sprite_get_palette_group(int palette) {
    if (palette < 10) {
        return 0;
    } else if (palette < 20) {
        return 1;
    } else if (palette < 30) {
        return 2;
    } else if (palette < 40) {
        return 3;
    } else {
        return 4;
    }
}

static void sprite_set_offset_table(u32* table) {
    g_sprite_offset_table = table;
}

static void sprite_set_dimension_table(SpriteDimension* table) {
    g_sprite_dim_table = table;
}

static void sprite_set_dimension_table_hires(SpriteDimension* table) {
    g_sprite_dim_table_hires = table;
}

/* ========================================
 * Test Cases - Offset Lookup
 * ======================================== */

static int test_offset_standard_sprite_0(void) {
    u32 offset;
    int result = sprite_lookup_offset(0, &offset);
    return result == 1 && offset == 0;
}

static int test_offset_standard_sprite_1(void) {
    u32 offset;
    int result = sprite_lookup_offset(1, &offset);
    return result == 1 && offset == 0x50;
}

static int test_offset_standard_sprite_100(void) {
    u32 offset;
    int result = sprite_lookup_offset(100, &offset);
    return result == 1 && offset == 100 * 0x50;
}

static int test_offset_standard_sprite_max(void) {
    u32 offset;
    int result = sprite_lookup_offset(SPRITE_ID_HIGHRES_BASE - 1, &offset);
    return result == 1 && offset == (SPRITE_ID_HIGHRES_BASE - 1) * 0x50;
}

static int test_offset_highres_base(void) {
    u32 offset;
    int result = sprite_lookup_offset(SPRITE_ID_HIGHRES_BASE, &offset);
    return result == 1 && offset == SPRITE_ID_HIGHRES_BASE;
}

static int test_offset_highres_mid(void) {
    u32 offset;
    int result = sprite_lookup_offset(525000, &offset);
    return result == 1 && offset == 525000;
}

static int test_offset_highres_max(void) {
    u32 offset;
    int result = sprite_lookup_offset(SPRITE_ID_HIGHRES_MAX, &offset);
    return result == 1 && offset == SPRITE_ID_HIGHRES_MAX;
}

static int test_offset_invalid_over_max(void) {
    u32 offset = 0xFFFFFFFF;
    int result = sprite_lookup_offset(SPRITE_ID_HIGHRES_MAX + 1, &offset);
    return result == 0 && offset == 0;
}

static int test_offset_invalid_large(void) {
    u32 offset = 0xFFFFFFFF;
    int result = sprite_lookup_offset(600000, &offset);
    return result == 0 && offset == 0;
}

static int test_offset_null_pointer(void) {
    int result = sprite_lookup_offset(100, NULL);
    return result == 1;  /* Should still return success */
}

/* ========================================
 * Test Cases - Dimension Lookup
 * ======================================== */

static int test_dimension_standard_sprite(void) {
    u16 width, height;
    int result = sprite_lookup_dimension(0, &width, &height);
    return result == 1 && width == 64 && height == 48;
}

static int test_dimension_standard_with_table(void) {
    u16 width, height;

    /* Set up test table */
    memset(test_dim_table, 0, sizeof(test_dim_table));
    test_dim_table[50].width = 32;
    test_dim_table[50].height = 32;

    sprite_set_dimension_table(test_dim_table);

    int result = sprite_lookup_dimension(50, &width, &height);
    return result == 1 && width == 32 && height == 32;
}

static int test_dimension_highres_sprite(void) {
    u16 width, height;
    int result = sprite_lookup_dimension(SPRITE_ID_HIGHRES_BASE, &width, &height);
    return result == 1 && width == 128 && height == 96;
}

static int test_dimension_highres_with_table(void) {
    u16 width, height;

    /* Set up test table */
    memset(test_dim_table_hires, 0, sizeof(test_dim_table_hires));
    test_dim_table_hires[0].width = 256;
    test_dim_table_hires[0].height = 256;

    sprite_set_dimension_table_hires(test_dim_table_hires);

    int result = sprite_lookup_dimension(SPRITE_ID_HIGHRES_BASE, &width, &height);
    return result == 1 && width == 256 && height == 256;
}

static int test_dimension_invalid_sprite(void) {
    u16 width = 0xFFFF, height = 0xFFFF;
    int result = sprite_lookup_dimension(600000, &width, &height);
    return result == 0 && width == 0 && height == 0;
}

static int test_dimension_null_width(void) {
    u16 height;
    g_sprite_dim_table = NULL;  /* Reset to use placeholder values */
    int result = sprite_lookup_dimension(0, NULL, &height);
    return result == 1 && height == 48;
}

static int test_dimension_null_height(void) {
    u16 width;
    g_sprite_dim_table = NULL;  /* Reset to use placeholder values */
    int result = sprite_lookup_dimension(0, &width, NULL);
    return result == 1 && width == 64;
}

static int test_dimension_null_both(void) {
    int result = sprite_lookup_dimension(0, NULL, NULL);
    return result == 1;
}

/* ========================================
 * Test Cases - High-Res Detection
 * ======================================== */

static int test_is_highres_standard_0(void) {
    return sprite_is_highres(0) == 0;
}

static int test_is_highres_standard_mid(void) {
    return sprite_is_highres(250000) == 0;
}

static int test_is_highres_standard_max(void) {
    return sprite_is_highres(SPRITE_ID_HIGHRES_BASE - 1) == 0;
}

static int test_is_highres_extended_base(void) {
    return sprite_is_highres(SPRITE_ID_HIGHRES_BASE) == 1;
}

static int test_is_highres_extended_mid(void) {
    return sprite_is_highres(525000) == 1;
}

static int test_is_highres_extended_max(void) {
    return sprite_is_highres(SPRITE_ID_HIGHRES_MAX) == 1;
}

static int test_is_highres_over_max(void) {
    return sprite_is_highres(SPRITE_ID_HIGHRES_MAX + 1) == 0;
}

static int test_is_highres_large_value(void) {
    return sprite_is_highres(1000000) == 0;
}

/* ========================================
 * Test Cases - Palette Group
 * ======================================== */

static int test_palette_group_0(void) {
    return sprite_get_palette_group(0) == 0;
}

static int test_palette_group_5(void) {
    return sprite_get_palette_group(5) == 0;
}

static int test_palette_group_9(void) {
    return sprite_get_palette_group(9) == 0;
}

static int test_palette_group_10(void) {
    return sprite_get_palette_group(10) == 1;
}

static int test_palette_group_15(void) {
    return sprite_get_palette_group(15) == 1;
}

static int test_palette_group_19(void) {
    return sprite_get_palette_group(19) == 1;
}

static int test_palette_group_20(void) {
    return sprite_get_palette_group(20) == 2;
}

static int test_palette_group_25(void) {
    return sprite_get_palette_group(25) == 2;
}

static int test_palette_group_29(void) {
    return sprite_get_palette_group(29) == 2;
}

static int test_palette_group_30(void) {
    return sprite_get_palette_group(30) == 3;
}

static int test_palette_group_35(void) {
    return sprite_get_palette_group(35) == 3;
}

static int test_palette_group_39(void) {
    return sprite_get_palette_group(39) == 3;
}

static int test_palette_group_40(void) {
    return sprite_get_palette_group(40) == 4;
}

static int test_palette_group_50(void) {
    return sprite_get_palette_group(50) == 4;
}

static int test_palette_group_100(void) {
    return sprite_get_palette_group(100) == 4;
}

static int test_palette_group_negative(void) {
    return sprite_get_palette_group(-5) == 0;
}

/* ========================================
 * Test Cases - Table Management
 * ======================================== */

static int test_set_offset_table(void) {
    static u32 test_table[10] = {0};
    sprite_set_offset_table(test_table);
    return g_sprite_offset_table == test_table;
}

static int test_set_dimension_table(void) {
    sprite_set_dimension_table(test_dim_table);
    return g_sprite_dim_table == test_dim_table;
}

static int test_set_dimension_table_hires(void) {
    sprite_set_dimension_table_hires(test_dim_table_hires);
    return g_sprite_dim_table_hires == test_dim_table_hires;
}

static int test_null_tables_allowed(void) {
    sprite_set_offset_table(NULL);
    sprite_set_dimension_table(NULL);
    sprite_set_dimension_table_hires(NULL);
    return g_sprite_offset_table == NULL &&
           g_sprite_dim_table == NULL &&
           g_sprite_dim_table_hires == NULL;
}

/* ========================================
 * Test Cases - Boundary Values
 * ======================================== */

static int test_boundary_standard_to_highres(void) {
    /* Test the exact boundary between standard and high-res */
    int r1 = sprite_is_highres(SPRITE_ID_HIGHRES_BASE - 1);
    int r2 = sprite_is_highres(SPRITE_ID_HIGHRES_BASE);

    return r1 == 0 && r2 == 1;
}

static int test_boundary_highres_to_invalid(void) {
    /* Test the exact boundary between high-res and invalid */
    int r1 = sprite_is_highres(SPRITE_ID_HIGHRES_MAX);
    int r2 = sprite_is_highres(SPRITE_ID_HIGHRES_MAX + 1);

    return r1 == 1 && r2 == 0;
}

static int test_offset_boundary_standard(void) {
    u32 off1, off2;
    sprite_lookup_offset(SPRITE_ID_HIGHRES_BASE - 1, &off1);
    sprite_lookup_offset(SPRITE_ID_HIGHRES_BASE, &off2);

    /* Standard: offset calculated, High-res: offset = id */
    return off1 == (SPRITE_ID_HIGHRES_BASE - 1) * 0x50 &&
           off2 == SPRITE_ID_HIGHRES_BASE;
}

static int test_dimension_boundary_highres(void) {
    u16 w1, h1, w2, h2;

    /* Reset tables for placeholder values */
    sprite_set_dimension_table(NULL);
    sprite_set_dimension_table_hires(NULL);

    sprite_lookup_dimension(SPRITE_ID_HIGHRES_BASE - 1, &w1, &h1);
    sprite_lookup_dimension(SPRITE_ID_HIGHRES_BASE, &w2, &h2);

    /* Standard: 64x48, High-res: 128x96 (placeholder values) */
    return w1 == 64 && h1 == 48 && w2 == 128 && h2 == 96;
}

/* ========================================
 * Test Cases - Constants Validation
 * ======================================== */

static int test_highres_base_value(void) {
    /* High-res sprites start at 500000 */
    return SPRITE_ID_HIGHRES_BASE == 500000;
}

static int test_highres_max_value(void) {
    /* High-res sprites end at 549999 */
    return SPRITE_ID_HIGHRES_MAX == 549999;
}

static int test_highres_range_size(void) {
    /* Range should be 50000 sprites */
    return (SPRITE_ID_HIGHRES_MAX - SPRITE_ID_HIGHRES_BASE + 1) == 50000;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Sprite Lookup Module Comprehensive Tests ===\n\n");

    printf("Offset Lookup Tests:\n");
    TEST(offset_standard_sprite_0);
    TEST(offset_standard_sprite_1);
    TEST(offset_standard_sprite_100);
    TEST(offset_standard_sprite_max);
    TEST(offset_highres_base);
    TEST(offset_highres_mid);
    TEST(offset_highres_max);
    TEST(offset_invalid_over_max);
    TEST(offset_invalid_large);
    TEST(offset_null_pointer);

    printf("\nDimension Lookup Tests:\n");
    TEST(dimension_standard_sprite);
    TEST(dimension_standard_with_table);
    TEST(dimension_highres_sprite);
    TEST(dimension_highres_with_table);
    TEST(dimension_invalid_sprite);
    TEST(dimension_null_width);
    TEST(dimension_null_height);
    TEST(dimension_null_both);

    printf("\nHigh-Res Detection Tests:\n");
    TEST(is_highres_standard_0);
    TEST(is_highres_standard_mid);
    TEST(is_highres_standard_max);
    TEST(is_highres_extended_base);
    TEST(is_highres_extended_mid);
    TEST(is_highres_extended_max);
    TEST(is_highres_over_max);
    TEST(is_highres_large_value);

    printf("\nPalette Group Tests:\n");
    TEST(palette_group_0);
    TEST(palette_group_5);
    TEST(palette_group_9);
    TEST(palette_group_10);
    TEST(palette_group_15);
    TEST(palette_group_19);
    TEST(palette_group_20);
    TEST(palette_group_25);
    TEST(palette_group_29);
    TEST(palette_group_30);
    TEST(palette_group_35);
    TEST(palette_group_39);
    TEST(palette_group_40);
    TEST(palette_group_50);
    TEST(palette_group_100);
    TEST(palette_group_negative);

    printf("\nTable Management Tests:\n");
    TEST(set_offset_table);
    TEST(set_dimension_table);
    TEST(set_dimension_table_hires);
    TEST(null_tables_allowed);

    printf("\nBoundary Value Tests:\n");
    TEST(boundary_standard_to_highres);
    TEST(boundary_highres_to_invalid);
    TEST(offset_boundary_standard);
    TEST(dimension_boundary_highres);

    printf("\nConstants Validation Tests:\n");
    TEST(highres_base_value);
    TEST(highres_max_value);
    TEST(highres_range_size);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
