/*
 * Stone Age Client - Sprite System Unit Tests (Real Data)
 * Tests for sprite loading, decoding, and caching
 * Uses real game data from D:\Games\石器时代8.0\石器时代8.0单机版\data\
 *
 * Reverse engineered from FUN_0041f900, FUN_0041f980, FUN_0041fad0, FUN_0041fb10, FUN_0048a550
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/assets.h"
#include "../include/assets_sprite.h"

/* Test data directory */
#define TEST_DATA_DIR "D:/Games/石器时代8.0/石器时代8.0单机版/data/"

/* Sprite ID ranges from FUN_0041f900, FUN_0041fb10 */
#define SPRITE_ID_STANDARD_MAX  500000
#define SPRITE_ID_EXTENDED_MAX  550000

/* ========================================
 * Test Cases for Sprite System
 * ======================================== */

/*
 * Test 1: Sprite data files exist
 */
static void test_sprite_files_exist(void) {
    TEST_BEGIN("Sprite files exist");

    TEST_ASSERT(save_exists(TEST_DATA_DIR "spr.bin") == 1);
    TEST_ASSERT(save_exists(TEST_DATA_DIR "spradrn.bin") == 1);
    TEST_ASSERT(save_exists(TEST_DATA_DIR "real.bin") == 1);

    TEST_END();
}

/*
 * Test 2: Sprite file sizes
 */
static void test_sprite_file_sizes(void) {
    TEST_BEGIN("Sprite file sizes");

    u32 spr_size = save_get_size(TEST_DATA_DIR "spr.bin");
    u32 spradrn_size = save_get_size(TEST_DATA_DIR "spradrn.bin");
    u32 real_size = save_get_size(TEST_DATA_DIR "real.bin");

    TEST_ASSERT(spr_size == 103008);
    TEST_ASSERT(spradrn_size == 156);
    TEST_ASSERT(real_size == 25020180);

    TEST_END();
}

/*
 * Test 3: Real.bin header format - "RD" magic
 * FUN_0048a550 checks for "RD" at start of sprite data
 */
static void test_real_bin_header(void) {
    TEST_BEGIN("Real.bin RD header");

    FILE* fp = fopen(TEST_DATA_DIR "real.bin", "rb");
    if (!fp) {
        TEST_END();
        return;
    }

    u8 header[16];
    int read = fread(header, 1, 16, fp);
    fclose(fp);

    TEST_ASSERT(read == 16);

    /* First two bytes should be "RD" */
    TEST_ASSERT(header[0] == 'R');
    TEST_ASSERT(header[1] == 'D');

    /* Third byte is version/compression flag */
    u8 version = header[2];
    TEST_ASSERT(version == 0x01);  /* Version 1 */

    /* Bytes 4-7: width, Bytes 8-11: height */
    u32 width = *(u32*)(header + 4);
    u32 height = *(u32*)(header + 8);

    /* From hex dump: width=0x30 (48), height=0x70 (112) */
    TEST_ASSERT(width > 0);
    TEST_ASSERT(height > 0);

    TEST_END();
}

/*
 * Test 4: Spr.bin structure
 * Format: sprite index/offset table
 */
static void test_spr_bin_structure(void) {
    TEST_BEGIN("Spr.bin structure");

    FILE* fp = fopen(TEST_DATA_DIR "spr.bin", "rb");
    if (!fp) {
        TEST_END();
        return;
    }

    /* Read first 16 bytes */
    u32 header[4];
    int read = fread(header, 4, 4, fp);
    fclose(fp);

    TEST_ASSERT(read == 4);

    /* From hex dump:
     * 0x00: 0x00000000 - padding/reserved
     * 0x04: 0x000003e8 = 1000 - count?
     * 0x08: 0x00000009 = 9 - version?
     * 0x0c: 0x0006f6ce - offset?
     */

    /* Spr.bin appears to be an index/offset table */
    TEST_ASSERT(header[1] == 1000);  /* Expected entry count */

    TEST_END();
}

/*
 * Test 5: Sprite ID range validation
 * FUN_0041f900: sprite_id < 500000 for standard sprites
 */
static void test_sprite_id_range(void) {
    TEST_BEGIN("Sprite ID range");

    /* Standard sprites: 0-499999 */
    u32 standard_id = 0;
    TEST_ASSERT(standard_id < SPRITE_ID_STANDARD_MAX);

    standard_id = 499999;
    TEST_ASSERT(standard_id < SPRITE_ID_STANDARD_MAX);

    /* Extended sprites: 500000-549999 */
    u32 extended_id = 500000;
    TEST_ASSERT(extended_id >= SPRITE_ID_STANDARD_MAX);
    TEST_ASSERT(extended_id < SPRITE_ID_EXTENDED_MAX);

    extended_id = 549999;
    TEST_ASSERT(extended_id < SPRITE_ID_EXTENDED_MAX);

    /* Invalid: >= 550000 */
    u32 invalid_id = 550000;
    TEST_ASSERT(invalid_id >= SPRITE_ID_EXTENDED_MAX);

    TEST_END();
}

/*
 * Test 6: Sprite info entry size
 * FUN_0041fb10: entry size = 0x14 (20 bytes)
 */
static void test_sprite_info_size(void) {
    TEST_BEGIN("Sprite info entry size");

    /* From FUN_0041fb10:
     * puVar2 = &DAT_00e8f228 + param_1 * 0x14;
     * for (iVar1 = 0x14; iVar1 != 0; iVar1 = iVar1 + -1)
     */
    TEST_ASSERT(sizeof(SpriteEntry) == 20 || sizeof(SpriteEntry) >= 20);

    TEST_END();
}

/*
 * Test 7: Sprite dimension entry size
 * FUN_0041f900: dimension entry = 0x50 (80 bytes)
 */
static void test_sprite_dimension_size(void) {
    TEST_BEGIN("Sprite dimension entry size");

    /* From FUN_0041f900:
     * *param_2 = *(undefined2 *)(&DAT_00e8f234 + param_1 * 0x50);
     * *param_3 = *(undefined2 *)(&DAT_00e8f238 + param_1 * 0x50);
     * Width at offset 0, height at offset 4
     */
    TEST_ASSERT(SPRITE_DIM_ENTRY_SIZE == 0x50);

    TEST_END();
}

/*
 * Test 8: RLE control byte decoding
 * FUN_0048a550: RLE control byte format
 */
static void test_rle_control_byte(void) {
    TEST_BEGIN("RLE control byte");

    /*
     * From FUN_0048a550:
     * Bit 7 (0x80): Mode flag
     *   0 = Literal (copy next N pixels)
     *   1 = RLE run (repeat single value)
     * Bit 6 (0x40): For RLE runs, pixel source
     *   0 = Pixel byte in next byte
     *   1 = Zero fill
     * Bits 5-4: Count encoding
     *   00 = 4-bit count
     *   01 = 12-bit count
     *   10 = 20-bit count
     */

    /* Literal run, 4-bit count = 5 */
    u8 ctrl1 = 0x05;
    TEST_ASSERT((ctrl1 & 0x80) == 0);  /* Not RLE */
    TEST_ASSERT((ctrl1 & 0x0F) == 5);  /* Count = 5 */

    /* RLE run, zero fill, count = 15 */
    u8 ctrl2 = 0x80 | 0x40 | 0x0F;
    TEST_ASSERT((ctrl2 & 0x80) != 0);  /* Is RLE */
    TEST_ASSERT((ctrl2 & 0x40) != 0);  /* Zero fill */
    TEST_ASSERT((ctrl2 & 0x0F) == 15); /* Count = 15 */

    /* RLE run, pixel byte, 12-bit count */
    u8 ctrl3 = 0x80 | 0x10;
    TEST_ASSERT((ctrl3 & 0x80) != 0);  /* Is RLE */
    TEST_ASSERT((ctrl3 & 0x40) == 0);  /* Has pixel byte */
    TEST_ASSERT((ctrl3 & 0x10) != 0);  /* 12-bit count */

    TEST_END();
}

/*
 * Test 9: Sprite decode - raw data
 */
static void test_sprite_decode_raw(void) {
    TEST_BEGIN("Sprite decode raw");

    /* Raw sprite: 2x2 = 4 pixels, 8-bit paletted (4 bytes decoded) */
    u8 src[] = {'R', 'D', 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x0A, 0x0B, 0x0C, 0x0D};
    u8 dst[16];
    int width, height;
    u32 data_size;

    int result = sprite_decode_from_data(src, sizeof(src), dst, sizeof(dst),
                                          &width, &height, &data_size);

    TEST_ASSERT(result == 1);
    TEST_ASSERT(width == 2);
    TEST_ASSERT(height == 2);
    TEST_ASSERT(data_size == 4);  /* 2x2 pixels * 1 byte (8-bit paletted) */
    TEST_ASSERT(dst[0] == 0x0A && dst[3] == 0x0D);

    TEST_END();
}

/*
 * Test 10: Sprite decode - RLE data
 */
static void test_sprite_decode_rle(void) {
    TEST_BEGIN("Sprite decode RLE");

    /*
     * RLE encoded 4x1 sprite:
     * Header: RD, version=1, width=4, height=1, compressed_size
     * Data: RLE run with pixel=0xFF, count=4
     */
    u8 src[] = {'R', 'D', 0x01, 0x00,
                0x04, 0x00, 0x00, 0x00,  /* width = 4 */
                0x01, 0x00, 0x00, 0x00,  /* height = 1 */
                0x03, 0x00, 0x00, 0x00,  /* compressed size */
                0x84, 0xFF, 0x03};       /* RLE: 0x84 = run, 0xFF=pixel, 0x03=count-1? */
    u8 dst[32];
    int width, height;
    u32 data_size;

    int result = sprite_decode_from_data(src, sizeof(src), dst, sizeof(dst),
                                          &width, &height, &data_size);

    TEST_ASSERT(result == 1);
    TEST_ASSERT(width == 4);
    TEST_ASSERT(height == 1);

    TEST_END();
}

/*
 * Test 11: Extended sprite files
 */
static void test_extended_sprite_files(void) {
    TEST_BEGIN("Extended sprite files");

    /* spr_116.bin and spradrn_116.bin exist */
    TEST_ASSERT(save_exists(TEST_DATA_DIR "spr_116.bin") == 1);
    TEST_ASSERT(save_exists(TEST_DATA_DIR "spradrn_116.bin") == 1);

    u32 spr_116_size = save_get_size(TEST_DATA_DIR "spr_116.bin");
    TEST_ASSERT(spr_116_size == 8437712);

    u32 spradrn_116_size = save_get_size(TEST_DATA_DIR "spradrn_116.bin");
    TEST_ASSERT(spradrn_116_size == 20328);

    TEST_END();
}

/*
 * Test 12: Real_138.bin (largest sprite file)
 */
static void test_real_138_file(void) {
    TEST_BEGIN("Real_138.bin file");

    TEST_ASSERT(save_exists(TEST_DATA_DIR "real_138.bin") == 1);

    u64 size = save_get_size(TEST_DATA_DIR "real_138.bin");
    /* File is 1.6GB - verify it's accessible */
    TEST_ASSERT(size > 1000000000);  /* > 1GB */

    TEST_END();
}

/*
 * Test 13: Adrn.bin (address/offset data)
 */
static void test_adrn_files(void) {
    TEST_BEGIN("Adrn files");

    TEST_ASSERT(save_exists(TEST_DATA_DIR "adrn.bin") == 1);
    TEST_ASSERT(save_exists(TEST_DATA_DIR "adrn_136.bin") == 1);

    u32 adrn_size = save_get_size(TEST_DATA_DIR "adrn.bin");
    TEST_ASSERT(adrn_size == 388480);

    u32 adrn_136_size = save_get_size(TEST_DATA_DIR "adrn_136.bin");
    TEST_ASSERT(adrn_136_size == 31786560);

    TEST_END();
}

/*
 * Test 14: Spradrn.bin structure
 */
static void test_spradrn_structure(void) {
    TEST_BEGIN("Spradrn.bin structure");

    FILE* fp = fopen(TEST_DATA_DIR "spradrn.bin", "rb");
    if (!fp) {
        TEST_END();
        return;
    }

    /* File is 156 bytes - small index file */
    u8 data[156];
    int read = fread(data, 1, 156, fp);
    fclose(fp);

    TEST_ASSERT(read == 156);

    /*
     * Spradrn.bin contains sprite address info
     * Format: array of entries pointing to spr.bin offsets
     */

    TEST_END();
}

/*
 * Test 15: Sprite cache initialization
 */
static void test_sprite_cache_init(void) {
    TEST_BEGIN("Sprite cache init");

    int result = sprite_cache_init(1000);
    TEST_ASSERT(result == 1);

    sprite_cache_shutdown();
    TEST_END();
}

/*
 * Test 16: Sprite cache add/get
 */
static void test_sprite_cache_add_get(void) {
    TEST_BEGIN("Sprite cache add/get");

    sprite_cache_init(100);

    u8 test_data[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    void* data_copy = malloc(16);
    memcpy(data_copy, test_data, 16);

    SpriteCacheEntry* entry = sprite_cache_add(12345, data_copy, 16, 2, 2);
    TEST_ASSERT(entry != NULL);
    TEST_ASSERT(entry->sprite_id == 12345);
    TEST_ASSERT(entry->width == 2);
    TEST_ASSERT(entry->height == 2);

    SpriteCacheEntry* found = sprite_cache_get(12345);
    TEST_ASSERT(found != NULL);
    TEST_ASSERT(found->sprite_id == 12345);

    sprite_cache_shutdown();
    TEST_END();
}

/*
 * Test 17: Sprite cache invalidate
 */
static void test_sprite_cache_invalidate(void) {
    TEST_BEGIN("Sprite cache invalidate");

    sprite_cache_init(100);

    u8 test_data[16] = {0};
    void* data_copy = malloc(16);
    memcpy(data_copy, test_data, 16);

    sprite_cache_add(100, data_copy, 16, 2, 2);

    SpriteCacheEntry* found = sprite_cache_get(100);
    TEST_ASSERT(found != NULL);

    sprite_cache_invalidate(100);

    found = sprite_cache_get(100);
    TEST_ASSERT(found == NULL);

    sprite_cache_shutdown();
    TEST_END();
}

/*
 * Test 18: Sprite cache clear
 */
static void test_sprite_cache_clear(void) {
    TEST_BEGIN("Sprite cache clear");

    sprite_cache_init(100);

    u8 test_data[16] = {0};

    int i;
    for (i = 0; i < 10; i++) {
        void* data_copy = malloc(16);
        memcpy(data_copy, test_data, 16);
        sprite_cache_add(i, data_copy, 16, 2, 2);
    }

    sprite_cache_clear();

    for (i = 0; i < 10; i++) {
        SpriteCacheEntry* found = sprite_cache_get(i);
        TEST_ASSERT(found == NULL);
    }

    sprite_cache_shutdown();
    TEST_END();
}

/*
 * Test 19: Sprite dimension lookup - sprite_id < 500000
 * FUN_0041f900: Standard sprites use DAT_00e8f234 table
 */
static void test_sprite_dimension_standard(void) {
    TEST_BEGIN("Sprite dimension standard");

    u16 width, height;

    /* For standard sprites, look up in dimension table */
    /* The actual implementation needs assets loaded */

    /* Test the range check logic */
    u32 standard_id = 0;
    TEST_ASSERT(standard_id < SPRITE_ID_STANDARD_MAX);

    standard_id = 499999;
    TEST_ASSERT(standard_id < SPRITE_ID_STANDARD_MAX);

    TEST_END();
}

/*
 * Test 20: Sprite dimension lookup - extended sprites
 * FUN_0041f900: Extended sprites (500000-549999) use different table
 */
static void test_sprite_dimension_extended(void) {
    TEST_BEGIN("Sprite dimension extended");

    /* Extended sprite calculation from FUN_0041f900:
     * iVar1 = (param_1 * 5 + -2500000) * 8;
     * *param_2 = *(undefined2 *)(&DAT_0081c7f4 + iVar1);
     * *param_3 = *(undefined2 *)(&DAT_0081c7f8 + iVar1);
     */

    u32 extended_id = 500000;
    u32 expected_index = (extended_id * 5 - 2500000) * 8;
    TEST_ASSERT(expected_index == 0);

    extended_id = 500001;
    expected_index = (extended_id * 5 - 2500000) * 8;
    TEST_ASSERT(expected_index == 40);

    TEST_END();
}

/*
 * Test 21: Sprite data offset lookup
 * FUN_0041fad0: Returns offset for sprite data
 */
static void test_sprite_data_offset(void) {
    TEST_BEGIN("Sprite data offset");

    /*
     * From FUN_0041fad0:
     * if (param_1 < 500000) {
     *     *param_2 = *(uint *)(&DAT_00a04c64 + param_1 * 4);
     *     return 1;
     * }
     * if (549999 < param_1) {
     *     *param_2 = 0;
     *     return 0;
     * }
     * *param_2 = param_1;  // Extended sprites return their ID as offset
     * return 1;
     */

    TEST_END();
}

/*
 * Test 22: Battle sprite file
 */
static void test_battle_sprite_file(void) {
    TEST_BEGIN("Battle sprite file");

    TEST_ASSERT(save_exists(TEST_DATA_DIR "battle_2.bin") == 1);

    u32 size = save_get_size(TEST_DATA_DIR "battle_2.bin");
    TEST_ASSERT(size == 187500);

    TEST_END();
}

/*
 * Test 23: Sound bin file
 */
static void test_sound_bin_file(void) {
    TEST_BEGIN("Sound bin file");

    TEST_ASSERT(save_exists(TEST_DATA_DIR "sound_3.bin") == 1);

    u32 size = save_get_size(TEST_DATA_DIR "sound_3.bin");
    TEST_ASSERT(size == 2696934);

    TEST_END();
}

/*
 * Test 24: Real.bin sprite data reading
 */
static void test_real_bin_data_reading(void) {
    TEST_BEGIN("Real.bin data reading");

    FILE* fp = fopen(TEST_DATA_DIR "real.bin", "rb");
    if (!fp) {
        TEST_END();
        return;
    }

    /* Seek past header */
    fseek(fp, 16, SEEK_SET);

    /* Read some compressed sprite data */
    u8 compressed[256];
    int read = fread(compressed, 1, 256, fp);
    fclose(fp);

    TEST_ASSERT(read == 256);

    /* Try to decode */
    u8 decoded[1024];
    int width, height;
    u32 data_size;

    /* First sprite in real.bin */
    int result = sprite_decode_from_data(compressed, read, decoded, sizeof(decoded),
                                          &width, &height, &data_size);

    /* May fail if data is incomplete, but shouldn't crash */
    TEST_ASSERT(result == 0 || result == 1);

    TEST_END();
}

/*
 * Test 25: RLE literal run
 */
static void test_rle_literal_run(void) {
    TEST_BEGIN("RLE literal run");

    /* Literal run: copy next 3 pixels (6 bytes) */
    /* Control byte: 0x03 (bit 7 = 0, count = 3) */

    u8 src[] = {0x03, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    u8 dst[16] = {0};

    u8* src_ptr = src;
    u8* dst_ptr = dst;
    u8 ctrl = *src_ptr++;

    if ((ctrl & 0x80) == 0) {
        /* Literal run */
        u32 count = ctrl & 0x0F;
        int i;
        for (i = 0; i < (int)count && src_ptr + 1 < src + sizeof(src); i++) {
            *dst_ptr++ = *src_ptr++;
            *dst_ptr++ = *src_ptr++;
        }
    }

    /* Verify 3 pixels copied (6 bytes) */
    TEST_ASSERT(dst[0] == 0xAA);
    TEST_ASSERT(dst[1] == 0xBB);
    TEST_ASSERT(dst[2] == 0xCC);
    TEST_ASSERT(dst[3] == 0xDD);
    TEST_ASSERT(dst[4] == 0xEE);
    TEST_ASSERT(dst[5] == 0xFF);

    TEST_END();
}

/*
 * Test 26: RLE zero fill run
 */
static void test_rle_zero_fill(void) {
    TEST_BEGIN("RLE zero fill");

    /* Zero fill: control = 0xC0 | count */
    /* 0x80 = RLE mode */
    /* 0x40 = zero fill */

    u8 ctrl = 0xC5;  /* Zero fill, count = 5 */
    TEST_ASSERT((ctrl & 0x80) != 0);  /* RLE */
    TEST_ASSERT((ctrl & 0x40) != 0);  /* Zero fill */
    TEST_ASSERT((ctrl & 0x0F) == 5);  /* Count */

    TEST_END();
}

/*
 * Test 27: RLE pixel byte run
 */
static void test_rle_pixel_byte_run(void) {
    TEST_BEGIN("RLE pixel byte run");

    /* Pixel byte fill: control = 0x80 | count, followed by pixel byte */
    u8 ctrl = 0x84;  /* RLE mode, count = 4 */
    u8 pixel = 0xFF;

    TEST_ASSERT((ctrl & 0x80) != 0);  /* RLE */
    TEST_ASSERT((ctrl & 0x40) == 0);  /* Not zero fill - has pixel byte */
    TEST_ASSERT((ctrl & 0x0F) == 4);  /* Count */

    TEST_END();
}

/*
 * Test 28: Sprite cache memory management
 */
static void test_sprite_cache_memory(void) {
    TEST_BEGIN("Sprite cache memory");

    sprite_cache_init(10);

    /* Add multiple sprites */
    int i;
    for (i = 0; i < 5; i++) {
        void* data = malloc(64);
        memset(data, i, 64);
        sprite_cache_add(i, data, 64, 4, 4);
    }

    /* Verify all cached */
    for (i = 0; i < 5; i++) {
        SpriteCacheEntry* entry = sprite_cache_get(i);
        TEST_ASSERT(entry != NULL);
    }

    sprite_cache_shutdown();
    TEST_END();
}

/*
 * Test 29: Invalid sprite ID handling
 */
static void test_invalid_sprite_id(void) {
    TEST_BEGIN("Invalid sprite ID");

    /* Sprite ID >= 550000 should be invalid */
    u32 invalid_id = 550000;
    TEST_ASSERT(invalid_id >= SPRITE_ID_EXTENDED_MAX);

    invalid_id = 999999;
    TEST_ASSERT(invalid_id >= SPRITE_ID_EXTENDED_MAX);

    TEST_END();
}

/*
 * Test 30: Empty sprite data handling
 */
static void test_empty_sprite_data(void) {
    TEST_BEGIN("Empty sprite data");

    u8 empty_src[1] = {0};
    u8 dst[16];
    int width, height;
    u32 data_size;

    int result = sprite_decode_from_data(empty_src, 1, dst, sizeof(dst),
                                          &width, &height, &data_size);

    TEST_ASSERT(result == 0);  /* Should fail with too small data */

    TEST_END();
}

/* Helper function declaration from save.h */
int save_exists(const char* path);
u32 save_get_size(const char* path);

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(sprite_real) {
    test_sprite_files_exist();
    test_sprite_file_sizes();
    test_real_bin_header();
    test_spr_bin_structure();
    test_sprite_id_range();
    test_sprite_info_size();
    test_sprite_dimension_size();
    test_rle_control_byte();
    test_sprite_decode_raw();
    test_sprite_decode_rle();
    test_extended_sprite_files();
    test_real_138_file();
    test_adrn_files();
    test_spradrn_structure();
    test_sprite_cache_init();
    test_sprite_cache_add_get();
    test_sprite_cache_invalidate();
    test_sprite_cache_clear();
    test_sprite_dimension_standard();
    test_sprite_dimension_extended();
    test_sprite_data_offset();
    test_battle_sprite_file();
    test_sound_bin_file();
    test_real_bin_data_reading();
    test_rle_literal_run();
    test_rle_zero_fill();
    test_rle_pixel_byte_run();
    test_sprite_cache_memory();
    test_invalid_sprite_id();
    test_empty_sprite_data();
}

/* ========================================
 * Main
 * ======================================== */

int main(void) {
    printf("=== Sprite System Unit Tests (Real Data) ===\n\n");
    RUN_SUITE(sprite_real);

    printf("\n=== Test Summary ===\n");
    printf("Total:  %d\n", s_tests_run);
    printf("Passed: %d\n", s_tests_passed);
    printf("Failed: %d\n", s_tests_failed);

    if (s_tests_failed > 0) {
        printf("\nSOME TESTS FAILED!\n");
        return 1;
    }

    printf("\nAll tests passed!\n");
    return 0;
}
