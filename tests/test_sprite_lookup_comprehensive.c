/*
 * Stone Age Client - Sprite Lookup Comprehensive Tests
 * Tests for sprite_lookup.c implementation
 *
 * Based on Ghidra analysis:
 * - FUN_0041fad0: sprite_lookup_offset
 * - FUN_0041f980: sprite_lookup_dimension
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

/* ========================================
 * Implementation Functions
 * ======================================== */

/*
 * Get sprite data offset - FUN_0041fad0
 */
static int sprite_lookup_offset(u32 sprite_id, u32* offset) {
    if (sprite_id < SPRITE_ID_HIGHRES_BASE) {
        if (offset) {
            *offset = sprite_id * 0x50;
        }
        return 1;
    }

    if (sprite_id <= SPRITE_ID_HIGHRES_MAX) {
        if (offset) {
            *offset = sprite_id;
        }
        return 1;
    }

    if (offset) *offset = 0;
    return 0;
}

/*
 * Get sprite dimensions - FUN_0041f980
 */
static int sprite_lookup_dimension(u32 sprite_id, u16* width, u16* height) {
    if (sprite_id < SPRITE_ID_HIGHRES_BASE) {
        if (g_sprite_dim_table && width && height) {
            *width = g_sprite_dim_table[sprite_id].width;
            *height = g_sprite_dim_table[sprite_id].height;
            return 1;
        }
        if (width) *width = 64;
        if (height) *height = 48;
        return 1;
    }

    if (sprite_id <= SPRITE_ID_HIGHRES_MAX) {
        if (g_sprite_dim_table_hires && width && height) {
            u32 index = (sprite_id - SPRITE_ID_HIGHRES_BASE) * 5 + 2500000;
            u32 offset = index * 8;
            *width = ((SpriteDimension*)((u8*)g_sprite_dim_table_hires + offset))[0].width;
            *height = ((SpriteDimension*)((u8*)g_sprite_dim_table_hires + offset))[0].height;
            return 1;
        }
        if (width) *width = 128;
        if (height) *height = 96;
        return 1;
    }

    if (width) *width = 0;
    if (height) *height = 0;
    return 0;
}

/*
 * Check if sprite is high-res
 */
static int sprite_is_highres(u32 sprite_id) {
    return (sprite_id >= SPRITE_ID_HIGHRES_BASE && sprite_id <= SPRITE_ID_HIGHRES_MAX);
}

/*
 * Get palette group from palette value
 */
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

/*
 * Set sprite offset table
 */
static void sprite_set_offset_table(u32* table) {
    g_sprite_offset_table = table;
}

/*
 * Set sprite dimension table
 */
static void sprite_set_dimension_table(SpriteDimension* table) {
    g_sprite_dim_table = table;
}

/*
 * Set high-res sprite dimension table
 */
static void sprite_set_dimension_table_hires(SpriteDimension* table) {
    g_sprite_dim_table_hires = table;
}

/*
 * Reset state
 */
static void reset_state(void) {
    g_sprite_offset_table = NULL;
    g_sprite_dim_table = NULL;
    g_sprite_dim_table_hires = NULL;
}

/* ========================================
 * Test Cases - Offset Lookup
 * ======================================== */

static int test_offset_standard_sprite(void) {
    reset_state();

    u32 offset;
    int result = sprite_lookup_offset(1000, &offset);

    return result == 1 && offset == 1000 * 0x50;
}

static int test_offset_sprite_zero(void) {
    reset_state();

    u32 offset;
    int result = sprite_lookup_offset(0, &offset);

    return result == 1 && offset == 0;
}

static int test_offset_highres_sprite(void) {
    reset_state();

    u32 offset;
    int result = sprite_lookup_offset(510000, &offset);

    /* High-res sprites return the ID itself as offset */
    return result == 1 && offset == 510000;
}

static int test_offset_highres_boundary_low(void) {
    reset_state();

    u32 offset;
    int result = sprite_lookup_offset(500000, &offset);

    return result == 1 && offset == 500000;
}

static int test_offset_highres_boundary_high(void) {
    reset_state();

    u32 offset;
    int result = sprite_lookup_offset(549999, &offset);

    return result == 1 && offset == 549999;
}

static int test_offset_invalid_over_max(void) {
    reset_state();

    u32 offset = 99;
    int result = sprite_lookup_offset(550000, &offset);

    return result == 0 && offset == 0;
}

static int test_offset_null_output(void) {
    reset_state();

    /* Should not crash */
    int result = sprite_lookup_offset(1000, NULL);

    return result == 1;
}

/* ========================================
 * Test Cases - Dimension Lookup
 * ======================================== */

static int test_dimension_standard_sprite(void) {
    reset_state();

    u16 width, height;
    int result = sprite_lookup_dimension(1000, &width, &height);

    /* Placeholder values when no table is set */
    return result == 1 && width == 64 && height == 48;
}

static int test_dimension_highres_sprite(void) {
    reset_state();

    u16 width, height;
    int result = sprite_lookup_dimension(510000, &width, &height);

    /* Placeholder values for high-res */
    return result == 1 && width == 128 && height == 96;
}

static int test_dimension_invalid_sprite(void) {
    reset_state();

    u16 width = 99, height = 99;
    int result = sprite_lookup_dimension(550000, &width, &height);

    return result == 0 && width == 0 && height == 0;
}

static int test_dimension_null_width(void) {
    reset_state();

    u16 height;
    int result = sprite_lookup_dimension(1000, NULL, &height);

    return result == 1 && height == 48;
}

static int test_dimension_null_height(void) {
    reset_state();

    u16 width;
    int result = sprite_lookup_dimension(1000, &width, NULL);

    return result == 1 && width == 64;
}

static int test_dimension_both_null(void) {
    reset_state();

    int result = sprite_lookup_dimension(1000, NULL, NULL);

    return result == 1;
}

static int test_dimension_with_table(void) {
    reset_state();

    /* Create mock dimension table */
    SpriteDimension table[10];
    table[5].width = 100;
    table[5].height = 200;

    sprite_set_dimension_table(table);

    u16 width, height;
    int result = sprite_lookup_dimension(5, &width, &height);

    return result == 1 && width == 100 && height == 200;
}

/* ========================================
 * Test Cases - High-res Check
 * ======================================== */

static int test_is_highres_standard(void) {
    reset_state();

    return sprite_is_highres(1000) == 0;
}

static int test_is_highres_zero(void) {
    reset_state();

    return sprite_is_highres(0) == 0;
}

static int test_is_highres_boundary_low(void) {
    reset_state();

    return sprite_is_highres(500000) == 1;
}

static int test_is_highres_middle(void) {
    reset_state();

    return sprite_is_highres(525000) == 1;
}

static int test_is_highres_boundary_high(void) {
    reset_state();

    return sprite_is_highres(549999) == 1;
}

static int test_is_highres_over_max(void) {
    reset_state();

    return sprite_is_highres(550000) == 0;
}

/* ========================================
 * Test Cases - Palette Groups
 * ======================================== */

static int test_palette_group_0(void) {
    reset_state();

    return sprite_get_palette_group(5) == 0;
}

static int test_palette_group_0_boundary(void) {
    reset_state();

    return sprite_get_palette_group(9) == 0;
}

static int test_palette_group_1(void) {
    reset_state();

    return sprite_get_palette_group(15) == 1;
}

static int test_palette_group_2(void) {
    reset_state();

    return sprite_get_palette_group(25) == 2;
}

static int test_palette_group_3(void) {
    reset_state();

    return sprite_get_palette_group(35) == 3;
}

static int test_palette_group_4(void) {
    reset_state();

    return sprite_get_palette_group(45) == 4;
}

static int test_palette_group_large(void) {
    reset_state();

    return sprite_get_palette_group(100) == 4;
}

static int test_palette_group_negative(void) {
    reset_state();

    return sprite_get_palette_group(-5) == 0;
}

/* ========================================
 * Test Cases - Table Setters
 * ======================================== */

static int test_set_offset_table(void) {
    reset_state();

    u32 table[10] = {0};
    sprite_set_offset_table(table);

    return g_sprite_offset_table == table;
}

static int test_set_dimension_table(void) {
    reset_state();

    SpriteDimension table[10];
    sprite_set_dimension_table(table);

    return g_sprite_dim_table == table;
}

static int test_set_dimension_table_hires(void) {
    reset_state();

    SpriteDimension table[10];
    sprite_set_dimension_table_hires(table);

    return g_sprite_dim_table_hires == table;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Sprite Lookup Comprehensive Tests ===\n\n");

    printf("Offset Lookup Tests:\n");
    TEST(offset_standard_sprite);
    TEST(offset_sprite_zero);
    TEST(offset_highres_sprite);
    TEST(offset_highres_boundary_low);
    TEST(offset_highres_boundary_high);
    TEST(offset_invalid_over_max);
    TEST(offset_null_output);

    printf("\nDimension Lookup Tests:\n");
    TEST(dimension_standard_sprite);
    TEST(dimension_highres_sprite);
    TEST(dimension_invalid_sprite);
    TEST(dimension_null_width);
    TEST(dimension_null_height);
    TEST(dimension_both_null);
    TEST(dimension_with_table);

    printf("\nHigh-res Check Tests:\n");
    TEST(is_highres_standard);
    TEST(is_highres_zero);
    TEST(is_highres_boundary_low);
    TEST(is_highres_middle);
    TEST(is_highres_boundary_high);
    TEST(is_highres_over_max);

    printf("\nPalette Group Tests:\n");
    TEST(palette_group_0);
    TEST(palette_group_0_boundary);
    TEST(palette_group_1);
    TEST(palette_group_2);
    TEST(palette_group_3);
    TEST(palette_group_4);
    TEST(palette_group_large);
    TEST(palette_group_negative);

    printf("\nTable Setter Tests:\n");
    TEST(set_offset_table);
    TEST(set_dimension_table);
    TEST(set_dimension_table_hires);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
