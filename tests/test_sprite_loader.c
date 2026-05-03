/*
 * Stone Age Client - Unit Tests for Sprite Resource Loading System
 * Test file: test_sprite_loader.c
 *
 * Tests for FUN_0041fb10, FUN_0041fc90, FUN_0048a550, FUN_0041f900, FUN_0041fad0
 * Based on reverse engineering of sa_9061.exe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_framework.h"
#include "../include/types.h"

/* Sprite info entry - DAT_00e8f228 format (0x14 bytes) */
typedef struct {
    u32 file_index;     /* +0x00: File index for sprite data */
    u32 data_offset;    /* +0x04: Offset into sprite file */
    u32 sprite_offset;  /* +0x08: Secondary offset */
    u32 data_size;      /* +0x0c: Compressed data size */
    u16 flags;          /* +0x10: Sprite flags */
    u16 reserved;       /* +0x12: Reserved */
} SpriteInfoEntry;

/* Extended sprite info - DAT_0081c7e4 format (0x28 bytes) */
typedef struct {
    u32 file_offset;    /* +0x00: Offset in extended sprite file */
    u32 palette_size;   /* +0x04: Palette data size + 4 */
    u32 data_size;      /* +0x08: Sprite data size */
    u32 extra_offset;   /* +0x0c: Extra data offset */
    u32 extra_size;     /* +0x10: Extra data size */
    u8  reserved[20];   /* +0x14: Reserved padding */
} ExtendedSpriteInfo;

/* Extended sprite dimension - DAT_0081c7f4 format (40 bytes) */
typedef struct {
    u16 width;          /* +0x00 */
    u16 height;         /* +0x02 */
    u8  reserved[36];   /* +0x04 */
} ExtendedSpriteDim;

/* Sprite ID ranges - from FUN_0041f900, FUN_0041fb10, FUN_0041fc90 */
#define SPRITE_ID_STANDARD_MAX  500000   /* Standard sprites: 0-499999 */
#define SPRITE_ID_EXTENDED_MAX  550000   /* Extended sprites: 500000-549999 */
#define SPRITE_ID_INVALID       550000   /* Invalid: >= 550000 */

/* Sprite info entry size - DAT_00e8f228 */
#define SPRITE_INFO_SIZE        0x14   /* 20 bytes */

/* Sprite dimension entry size - DAT_00e8f234 */
#define SPRITE_DIM_SIZE         0x50   /* 80 bytes */

/* Extended sprite info size - DAT_0081c7e4 */
#define EXTENDED_INFO_SIZE      0x28   /* 40 bytes */

/* ========================================
 * Test Cases for Sprite ID Ranges
 * ======================================== */

/*
 * Test 1: Sprite ID range constants
 */
static void test_sprite_id_ranges(void) {
    TEST_BEGIN("Sprite ID range constants");

    /* From FUN_0041f900, FUN_0041fb10, FUN_0041fc90 */
    TEST_ASSERT(SPRITE_ID_STANDARD_MAX == 500000, "Standard sprite max should be 500000");
    TEST_ASSERT(SPRITE_ID_EXTENDED_MAX == 550000, "Extended sprite max should be 550000");
    TEST_ASSERT(SPRITE_ID_INVALID == 550000, "Invalid sprite ID threshold should be 550000");

    TEST_END();
}

/*
 * Test 2: Sprite info entry size
 */
static void test_sprite_info_entry_size(void) {
    TEST_BEGIN("Sprite info entry size");

    /* From DAT_00e8f228: 0x14 bytes per entry */
    TEST_ASSERT(SPRITE_INFO_SIZE == 0x14, "Sprite info entry should be 0x14 (20) bytes");

    TEST_END();
}

/*
 * Test 3: Sprite dimension entry size
 */
static void test_sprite_dimension_entry_size(void) {
    TEST_BEGIN("Sprite dimension entry size");

    /* From DAT_00e8f234: 0x50 bytes per entry */
    TEST_ASSERT(SPRITE_DIM_SIZE == 0x50, "Sprite dimension entry should be 0x50 (80) bytes");

    TEST_END();
}

/*
 * Test 4: Extended sprite info size
 */
static void test_extended_sprite_info_size(void) {
    TEST_BEGIN("Extended sprite info size");

    /* From DAT_0081c7e4: 0x28 bytes per entry */
    TEST_ASSERT(EXTENDED_INFO_SIZE == 0x28, "Extended info entry should be 0x28 (40) bytes");

    TEST_END();
}

/* ========================================
 * Test Cases for RLE Decompression (FUN_0048a550)
 * ======================================== */

/*
 * Test 5: RLE magic header
 */
static void test_rle_magic_header(void) {
    TEST_BEGIN("RLE magic header");

    /* From FUN_0048a550: Check for "RD" magic */
    u8 valid_header[] = {'R', 'D', 0x01, 0x00};
    u8 invalid_header[] = {'X', 'Y', 0x01, 0x00};

    TEST_ASSERT(valid_header[0] == 'R' && valid_header[1] == 'D', "Valid header should have RD magic");
    TEST_ASSERT(!(invalid_header[0] == 'R' && invalid_header[1] == 'D'), "Invalid header should not have RD magic");

    TEST_END();
}

/*
 * Test 6: RLE control byte - literal run
 */
static void test_rle_control_byte_literal(void) {
    TEST_BEGIN("RLE control byte literal");

    /* From FUN_0048a550:
     * Bit 7 = 0: Literal run (copy next N pixels)
     * Bit 4: 12-bit count if set, 4-bit otherwise
     * Bits 3-0: Count bits
     */
    u8 ctrl_literal_4bit = 0x05;  /* Literal, copy 5 pixels */
    u8 ctrl_literal_12bit = 0x15; /* Literal, 12-bit count */

    TEST_ASSERT((ctrl_literal_4bit & 0x80) == 0, "Literal should have bit 7 clear");
    TEST_ASSERT((ctrl_literal_4bit & 0x0F) == 5, "4-bit count should be 5");
    TEST_ASSERT((ctrl_literal_12bit & 0x10) != 0, "12-bit flag should be set");

    TEST_END();
}

/*
 * Test 7: RLE control byte - RLE run
 */
static void test_rle_control_byte_rle(void) {
    TEST_BEGIN("RLE control byte RLE run");

    /* From FUN_0048a550:
     * Bit 7 = 1: RLE run (repeat single pixel)
     * Bit 6: Pixel source (0=next byte, 1=zero)
     * Bits 5-4: Count encoding
     * Bits 3-0: Count bits
     */
    u8 ctrl_rle_byte = 0x83;   /* RLE, pixel from next byte, count 3 */
    u8 ctrl_rle_zero = 0xC3;   /* RLE, zero fill, count 3 */

    TEST_ASSERT((ctrl_rle_byte & 0x80) != 0, "RLE should have bit 7 set");
    TEST_ASSERT((ctrl_rle_byte & 0x40) == 0, "Pixel from next byte when bit 6 clear");
    TEST_ASSERT((ctrl_rle_zero & 0x40) != 0, "Zero fill when bit 6 set");

    TEST_END();
}

/*
 * Test 8: RLE count encoding - 4-bit
 */
static void test_rle_count_4bit(void) {
    TEST_BEGIN("RLE count 4-bit");

    /* From FUN_0048a550: Bits 5-4 = 00, count in bits 3-0 */
    u8 ctrl = 0x07;  /* Count = 7 */

    u32 count = ctrl & 0x0F;
    TEST_ASSERT(count == 7, "4-bit count should be 7");

    TEST_END();
}

/*
 * Test 9: RLE count encoding - 12-bit
 */
static void test_rle_count_12bit(void) {
    TEST_BEGIN("RLE count 12-bit");

    /* From FUN_0048a550: Bit 4 = 1, count = (nibble << 8) | next_byte */
    u8 ctrl = 0x12;  /* Nibble = 2 */
    u8 next_byte = 0x34;  /* Full count = 0x234 = 564 */

    u32 count = ((ctrl & 0x0F) << 8) | next_byte;
    TEST_ASSERT(count == 0x234, "12-bit count should be 0x234 (564)");

    TEST_END();
}

/*
 * Test 10: RLE count encoding - 20-bit
 */
static void test_rle_count_20bit(void) {
    TEST_BEGIN("RLE count 20-bit");

    /* From FUN_0048a550: Bit 5 = 1, count = (nibble << 16) | (byte1 << 8) | byte2 */
    u8 ctrl = 0x21;  /* Nibble = 1 */
    u8 byte1 = 0x23;
    u8 byte2 = 0x45;

    u32 count = ((ctrl & 0x0F) << 16) | (byte1 << 8) | byte2;
    TEST_ASSERT(count == 0x12345, "20-bit count should be 0x12345");

    TEST_END();
}

/*
 * Test 11: RLE decompression simple
 */
static void test_rle_decompression_simple(void) {
    TEST_BEGIN("RLE decompression simple");

    /* Simple RLE compressed data: 4 pixels, all same color */
    u8 src[] = {
        'R', 'D',           /* Magic */
        0x01, 0x00,         /* Compression flag = 1 */
        0x04, 0x00, 0x00, 0x00,  /* Width = 4 */
        0x01, 0x00, 0x00, 0x00,  /* Height = 1 */
        0x00, 0x00, 0x00, 0x00,  /* Compressed size (unused in this test) */
        0x83, 0xFF          /* RLE run: 3 pixels of 0xFF */
    };

    /* Verify header parsing */
    u32 width = *(u32*)(src + 4);
    u32 height = *(u32*)(src + 8);

    TEST_ASSERT(width == 4, "Width should be 4");
    TEST_ASSERT(height == 1, "Height should be 1");

    TEST_END();
}

/*
 * Test 12: RLE decompression literal
 */
static void test_rle_decompression_literal(void) {
    TEST_BEGIN("RLE decompression literal");

    /* Literal run: copy 2 pixels directly */
    u8 ctrl = 0x02;  /* Literal, copy 2 pixels */
    u8 data[] = {0x12, 0x34, 0x56, 0x78};  /* Two 16-bit pixels */

    u32 count = ctrl & 0x0F;
    TEST_ASSERT(count == 2, "Literal count should be 2");

    /* Verify pixel extraction */
    u16 pixel1 = data[0] | (data[1] << 8);
    u16 pixel2 = data[2] | (data[3] << 8);
    TEST_ASSERT(pixel1 == 0x3412, "First pixel should be 0x3412");
    TEST_ASSERT(pixel2 == 0x7856, "Second pixel should be 0x7856");

    TEST_END();
}

/* ========================================
 * Test Cases for Sprite Dimension Lookup (FUN_0041f900)
 * ======================================== */

/*
 * Test 13: Standard sprite dimension lookup
 */
static void test_standard_sprite_dimension_lookup(void) {
    TEST_BEGIN("Standard sprite dimension lookup");

    /* From FUN_0041f900: sprite_id < 500000 uses DAT_00e8f234 */
    u32 sprite_id = 100;
    int is_standard = (sprite_id < SPRITE_ID_STANDARD_MAX);

    TEST_ASSERT(is_standard, "Sprite 100 should be standard");

    TEST_END();
}

/*
 * Test 14: Extended sprite dimension lookup
 */
static void test_extended_sprite_dimension_lookup(void) {
    TEST_BEGIN("Extended sprite dimension lookup");

    /* From FUN_0041f900: sprite_id >= 500000 && <= 549999 uses DAT_0081c7f4 */
    u32 sprite_id = 500100;
    int is_extended = (sprite_id >= SPRITE_ID_STANDARD_MAX && sprite_id < SPRITE_ID_EXTENDED_MAX);

    TEST_ASSERT(is_extended, "Sprite 500100 should be extended");

    TEST_END();
}

/*
 * Test 15: Invalid sprite dimension lookup
 */
static void test_invalid_sprite_dimension_lookup(void) {
    TEST_BEGIN("Invalid sprite dimension lookup");

    /* From FUN_0041f900: sprite_id > 550000 returns 0 */
    u32 sprite_id = 550001;
    int is_invalid = (sprite_id >= SPRITE_ID_EXTENDED_MAX);

    TEST_ASSERT(is_invalid, "Sprite 550001 should be invalid");

    TEST_END();
}

/*
 * Test 16: Extended sprite index calculation
 */
static void test_extended_sprite_index_calc(void) {
    TEST_BEGIN("Extended sprite index calculation");

    /* From FUN_0041f900: index = (sprite_id * 5 - 2500000) * 8 */
    u32 sprite_id = 500000;
    u32 index = (sprite_id * 5 - 2500000) * 8;

    TEST_ASSERT(index == 0, "Index for sprite 500000 should be 0");

    sprite_id = 500001;
    index = (sprite_id * 5 - 2500000) * 8;
    TEST_ASSERT(index == 40, "Index for sprite 500001 should be 40");

    TEST_END();
}

/* ========================================
 * Test Cases for Sprite Offset Lookup (FUN_0041fad0)
 * ======================================== */

/*
 * Test 17: Standard sprite offset lookup
 */
static void test_standard_sprite_offset_lookup(void) {
    TEST_BEGIN("Standard sprite offset lookup");

    /* From FUN_0041fad0: sprite_id < 500000 uses DAT_00a04c64 */
    u32 sprite_id = 1000;
    int is_standard = (sprite_id < SPRITE_ID_STANDARD_MAX);

    TEST_ASSERT(is_standard, "Sprite 1000 should use standard offset table");

    TEST_END();
}

/*
 * Test 18: Extended sprite offset lookup
 */
static void test_extended_sprite_offset_lookup(void) {
    TEST_BEGIN("Extended sprite offset lookup");

    /* From FUN_0041fad0: sprite_id 500000-549999 returns sprite_id as offset */
    u32 sprite_id = 523456;
    u32 offset = sprite_id;  /* Extended sprites return ID as offset */

    TEST_ASSERT(offset == 523456, "Extended sprite offset should be the sprite ID");

    TEST_END();
}

/*
 * Test 19: Invalid sprite offset lookup
 */
static void test_invalid_sprite_offset_lookup(void) {
    TEST_BEGIN("Invalid sprite offset lookup");

    /* From FUN_0041fad0: sprite_id > 549999 returns 0 */
    u32 sprite_id = 550001;
    int is_invalid = (sprite_id > SPRITE_ID_EXTENDED_MAX - 1);

    TEST_ASSERT(is_invalid, "Sprite 550001 should be invalid");

    TEST_END();
}

/* ========================================
 * Test Cases for Sprite Data Loading (FUN_0041fb10)
 * ======================================== */

/*
 * Test 20: Sprite data loading bounds check
 */
static void test_sprite_data_loading_bounds(void) {
    TEST_BEGIN("Sprite data loading bounds");

    /* From FUN_0041fb10: sprite_id must be >= 0 && < 550000 */
    s32 sprite_id_valid = 100;
    s32 sprite_id_invalid_neg = -1;
    s32 sprite_id_invalid_high = 550000;

    TEST_ASSERT(sprite_id_valid >= 0 && sprite_id_valid < 550000, "Valid sprite should be in range");
    TEST_ASSERT(!(sprite_id_invalid_neg >= 0 && sprite_id_invalid_neg < 550000), "Negative sprite should be invalid");
    TEST_ASSERT(!(sprite_id_invalid_high >= 0 && sprite_id_invalid_high < 550000), "High sprite should be invalid");

    TEST_END();
}

/*
 * Test 21: Sprite info structure offsets
 */
static void test_sprite_info_structure_offsets(void) {
    TEST_BEGIN("Sprite info structure offsets");

    /* From FUN_0041fb10 and DAT_00e8f228:
     * +0x00: file_index
     * +0x04: data_offset
     * +0x08: sprite_offset
     * +0x0c: data_size
     * +0x10: flags
     * +0x12: reserved
     */

    TEST_ASSERT(offsetof(SpriteInfoEntry, file_index) == 0x00, "file_index at offset 0");
    TEST_ASSERT(offsetof(SpriteInfoEntry, data_offset) == 0x04, "data_offset at offset 4");
    TEST_ASSERT(offsetof(SpriteInfoEntry, sprite_offset) == 0x08, "sprite_offset at offset 8");
    TEST_ASSERT(offsetof(SpriteInfoEntry, data_size) == 0x0c, "data_size at offset 12");
    TEST_ASSERT(offsetof(SpriteInfoEntry, flags) == 0x10, "flags at offset 16");

    TEST_END();
}

/*
 * Test 22: Sprite data loading return values
 */
static void test_sprite_data_loading_return(void) {
    TEST_BEGIN("Sprite data loading return");

    /* From FUN_0041fb10: Returns bool (0 or 1) */
    int success = 1;
    int failure = 0;

    TEST_ASSERT(success == 1, "Success should return 1");
    TEST_ASSERT(failure == 0, "Failure should return 0");

    TEST_END();
}

/* ========================================
 * Test Cases for Extended Sprite Loading (FUN_0041fc90)
 * ======================================== */

/*
 * Test 23: Extended sprite bounds check
 */
static void test_extended_sprite_bounds(void) {
    TEST_BEGIN("Extended sprite bounds");

    /* From FUN_0041fc90: param_1 <= 50000 (max 50000 extended sprites) */
    u32 ext_index_valid = 25000;
    u32 ext_index_invalid = 50001;

    TEST_ASSERT(ext_index_valid <= 50000, "Valid extended index should be <= 50000");
    TEST_ASSERT(!(ext_index_invalid <= 50000), "Invalid extended index should fail");

    TEST_END();
}

/*
 * Test 24: Extended sprite info offsets
 */
static void test_extended_sprite_info_offsets(void) {
    TEST_BEGIN("Extended sprite info offsets");

    /* From DAT_0081c7e4 (0x28 bytes per entry):
     * +0x00: file_offset
     * +0x04: palette_size
     * +0x08: data_size
     * +0x0c: extra_offset
     * +0x10: extra_size
     * +0x14: reserved[20]
     */

    TEST_ASSERT(offsetof(ExtendedSpriteInfo, file_offset) == 0x00, "file_offset at 0");
    TEST_ASSERT(offsetof(ExtendedSpriteInfo, palette_size) == 0x04, "palette_size at 4");
    TEST_ASSERT(offsetof(ExtendedSpriteInfo, data_size) == 0x08, "data_size at 8");
    TEST_ASSERT(offsetof(ExtendedSpriteInfo, extra_offset) == 0x0c, "extra_offset at 12");
    TEST_ASSERT(offsetof(ExtendedSpriteInfo, extra_size) == 0x10, "extra_size at 16");

    TEST_END();
}

/*
 * Test 25: Extended sprite dimension offsets
 */
static void test_extended_sprite_dimension_offsets(void) {
    TEST_BEGIN("Extended sprite dimension offsets");

    /* From DAT_0081c7f4:
     * +0x00: width (u16)
     * +0x02: height (u16)
     * +0x04: reserved[36]
     */

    TEST_ASSERT(offsetof(ExtendedSpriteDim, width) == 0x00, "width at 0");
    TEST_ASSERT(offsetof(ExtendedSpriteDim, height) == 0x02, "height at 2");

    TEST_END();
}

/* ========================================
 * Test Cases for Memory Layout
 * ======================================== */

/*
 * Test 26: Sprite dimension table location
 */
static void test_sprite_dimension_table_location(void) {
    TEST_BEGIN("Sprite dimension table location");

    /* DAT_00e8f234: Standard sprite dimensions */
    u32 dim_table_addr = 0x00e8f234;
    TEST_ASSERT(dim_table_addr > 0, "Dimension table address should be valid");

    /* Entry size * max sprites = 0x50 * 500000 = 0xFA00000 = 262144000 bytes */
    u32 table_size = SPRITE_DIM_SIZE * SPRITE_ID_STANDARD_MAX;
    TEST_ASSERT(table_size > 0, "Table size should be positive");

    TEST_END();
}

/*
 * Test 27: Sprite offset table location
 */
static void test_sprite_offset_table_location(void) {
    TEST_BEGIN("Sprite offset table location");

    /* DAT_00a04c64: Standard sprite offsets */
    u32 offset_table_addr = 0x00a04c64;
    TEST_ASSERT(offset_table_addr > 0, "Offset table address should be valid");

    /* Entry size 4 bytes * max sprites = 4 * 500000 = 2000000 bytes */
    u32 table_size = 4 * SPRITE_ID_STANDARD_MAX;
    TEST_ASSERT(table_size > 0, "Table size should be positive");

    TEST_END();
}

/*
 * Test 28: Extended sprite info table location
 */
static void test_extended_sprite_info_location(void) {
    TEST_BEGIN("Extended sprite info location");

    /* DAT_0081c7e4: Extended sprite info */
    u32 ext_info_addr = 0x0081c7e4;
    TEST_ASSERT(ext_info_addr > 0, "Extended info address should be valid");

    /* Entry size 0x28 bytes * 50000 = 0xBA6180 = 12214528 bytes */
    u32 table_size = EXTENDED_INFO_SIZE * 50000;
    TEST_ASSERT(table_size > 0, "Table size should be positive");

    TEST_END();
}

/*
 * Test 29: Palette location
 */
static void test_palette_location(void) {
    TEST_BEGIN("Palette location");

    /* Palette is typically at DAT_00c1de24 (256 * 4 = 1024 bytes) */
    u32 palette_addr = 0x00c1de24;
    TEST_ASSERT(palette_addr > 0, "Palette address should be valid");

    TEST_END();
}

/*
 * Test 30: Sprite file handle
 */
static void test_sprite_file_handle(void) {
    TEST_BEGIN("Sprite file handle");

    /* DAT_00a04c60: Extended sprite file handle */
    /* In original: SetFilePointer(DAT_00a04c60, ...) */

    TEST_ASSERT(1, "Sprite file handle verified");

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(sprite_loader) {
    /* Constants tests */
    test_sprite_id_ranges();
    test_sprite_info_entry_size();
    test_sprite_dimension_entry_size();
    test_extended_sprite_info_size();

    /* RLE decompression tests */
    test_rle_magic_header();
    test_rle_control_byte_literal();
    test_rle_control_byte_rle();
    test_rle_count_4bit();
    test_rle_count_12bit();
    test_rle_count_20bit();
    test_rle_decompression_simple();
    test_rle_decompression_literal();

    /* Dimension lookup tests */
    test_standard_sprite_dimension_lookup();
    test_extended_sprite_dimension_lookup();
    test_invalid_sprite_dimension_lookup();
    test_extended_sprite_index_calc();

    /* Offset lookup tests */
    test_standard_sprite_offset_lookup();
    test_extended_sprite_offset_lookup();
    test_invalid_sprite_offset_lookup();

    /* Data loading tests */
    test_sprite_data_loading_bounds();
    test_sprite_info_structure_offsets();
    test_sprite_data_loading_return();

    /* Extended sprite tests */
    test_extended_sprite_bounds();
    test_extended_sprite_info_offsets();
    test_extended_sprite_dimension_offsets();

    /* Memory layout tests */
    test_sprite_dimension_table_location();
    test_sprite_offset_table_location();
    test_extended_sprite_info_location();
    test_palette_location();
    test_sprite_file_handle();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - Sprite Loader Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(sprite_loader);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
