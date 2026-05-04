/*
 * Stone Age Client - Sprite Lookup Functions Unit Tests
 * Tests for FUN_0041fad0, FUN_0041f900, FUN_0041f980 - Sprite data lookup functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;
typedef unsigned short ushort;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Simulated sprite ID tables */
#define SPRITE_TABLE_SIZE 500000
#define SPRITE_EXT_START 500000
#define SPRITE_EXT_END 550000

static u32 DAT_00a04c64[100] = {0};  /* Simulated lookup table (partial) */
static ushort DAT_00e8f234_data[100] = {0};  /* X offsets */
static ushort DAT_00e8f238_data[100] = {0};  /* Y offsets */
static ushort DAT_00e8f23c_data[100] = {0};  /* Width */
static ushort DAT_00e8f240_data[100] = {0};  /* Height */

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

/* Reset test state */
static void reset_test_state(void) {
    memset(DAT_00a04c64, 0, sizeof(DAT_00a04c64));
    memset(DAT_00e8f234_data, 0, sizeof(DAT_00e8f234_data));
    memset(DAT_00e8f238_data, 0, sizeof(DAT_00e8f238_data));
    memset(DAT_00e8f23c_data, 0, sizeof(DAT_00e8f23c_data));
    memset(DAT_00e8f240_data, 0, sizeof(DAT_00e8f240_data));
}

/* Initialize test data */
static void init_test_data(void) {
    reset_test_state();
    for (int i = 0; i < 100; i++) {
        DAT_00a04c64[i] = 0x1000 + i;
        DAT_00e8f234_data[i] = (ushort)(i * 10);
        DAT_00e8f238_data[i] = (ushort)(i * 20);
        DAT_00e8f23c_data[i] = (ushort)(32 + i);
        DAT_00e8f240_data[i] = (ushort)(32 + i * 2);
    }
}

/*
 * FUN_0041fad0 - Get sprite data by ID
 * param_1: sprite ID
 * param_2: output value pointer
 * Returns: 1 on success, 0 on failure
 */
static u32 FUN_0041fad0(u32 param_1, u32 *param_2) {
    if (param_1 < 500000) {
        if (param_1 < 100) {
            *param_2 = DAT_00a04c64[param_1];
        } else {
            *param_2 = param_1;  /* Simulated */
        }
        return 1;
    }
    if (param_1 > 549999) {
        *param_2 = 0;
        return 0;
    }
    *param_2 = param_1;
    return 1;
}

/*
 * FUN_0041f900 - Get sprite offset by ID
 * param_1: sprite ID
 * param_2: output X offset
 * param_3: output Y offset
 * Returns: 1 on success, 0 on failure
 */
static u32 FUN_0041f900(u32 param_1, ushort *param_2, ushort *param_3) {
    if (param_1 < 500000) {
        if (param_1 < 100) {
            *param_2 = DAT_00e8f234_data[param_1];
            *param_3 = DAT_00e8f238_data[param_1];
        } else {
            *param_2 = (ushort)param_1;
            *param_3 = (ushort)(param_1 >> 16);
        }
        return 1;
    }
    if (param_1 > 550000) {
        *param_2 = 0;
        *param_3 = 0;
        return 0;
    }
    /* Extended range calculation */
    *param_2 = (ushort)(param_1 - 500000);
    *param_3 = (ushort)((param_1 - 500000) >> 8);
    return 1;
}

/*
 * FUN_0041f980 - Get sprite dimensions by ID
 * param_1: sprite ID
 * param_2: output width
 * param_3: output height
 * Returns: 1 on success, 0 on failure
 */
static u32 FUN_0041f980(u32 param_1, ushort *param_2, ushort *param_3) {
    if (param_1 < 500000) {
        if (param_1 < 100) {
            *param_2 = DAT_00e8f23c_data[param_1];
            *param_3 = DAT_00e8f240_data[param_1];
        } else {
            *param_2 = 32;
            *param_3 = 32;
        }
        return 1;
    }
    if (param_1 > 550000) {
        *param_2 = 0;
        *param_3 = 0;
        return 0;
    }
    /* Extended range */
    *param_2 = (ushort)(64 + (param_1 - 500000) % 64);
    *param_3 = (ushort)(64 + (param_1 - 500000) % 32);
    return 1;
}

/* ========================================
 * Test Cases
 * ======================================== */

/* FUN_0041fad0 tests */
TEST(get_sprite_id_zero) {
    init_test_data();
    u32 result = 0;

    u32 ret = FUN_0041fad0(0, &result);

    ASSERT(ret == 1);
    ASSERT(result == 0x1000);
}

TEST(get_sprite_id_valid) {
    init_test_data();
    u32 result = 0;

    u32 ret = FUN_0041fad0(50, &result);

    ASSERT(ret == 1);
    ASSERT(result == 0x1000 + 50);
}

TEST(get_sprite_id_boundary) {
    init_test_data();
    u32 result = 0;

    u32 ret = FUN_0041fad0(499999, &result);

    ASSERT(ret == 1);
}

TEST(get_sprite_id_extended) {
    init_test_data();
    u32 result = 0;

    u32 ret = FUN_0041fad0(520000, &result);

    ASSERT(ret == 1);
    ASSERT(result == 520000);
}

TEST(get_sprite_id_invalid_high) {
    init_test_data();
    u32 result = 0;

    u32 ret = FUN_0041fad0(550000, &result);

    ASSERT(ret == 0);
    ASSERT(result == 0);
}

TEST(get_sprite_id_invalid_very_high) {
    init_test_data();
    u32 result = 0;

    u32 ret = FUN_0041fad0(1000000, &result);

    ASSERT(ret == 0);
    ASSERT(result == 0);
}

/* FUN_0041f900 tests */
TEST(get_offset_zero) {
    init_test_data();
    ushort x = 0, y = 0;

    u32 ret = FUN_0041f900(0, &x, &y);

    ASSERT(ret == 1);
    ASSERT(x == 0);
    ASSERT(y == 0);
}

TEST(get_offset_valid) {
    init_test_data();
    ushort x = 0, y = 0;

    u32 ret = FUN_0041f900(10, &x, &y);

    ASSERT(ret == 1);
    ASSERT(x == 100);  /* 10 * 10 */
    ASSERT(y == 200);  /* 10 * 20 */
}

TEST(get_offset_extended) {
    init_test_data();
    ushort x = 0, y = 0;

    u32 ret = FUN_0041f900(510000, &x, &y);

    ASSERT(ret == 1);
}

TEST(get_offset_invalid) {
    init_test_data();
    ushort x = 0, y = 0;

    u32 ret = FUN_0041f900(560000, &x, &y);

    ASSERT(ret == 0);
    ASSERT(x == 0);
    ASSERT(y == 0);
}

/* FUN_0041f980 tests */
TEST(get_dimension_zero) {
    init_test_data();
    ushort w = 0, h = 0;

    u32 ret = FUN_0041f980(0, &w, &h);

    ASSERT(ret == 1);
    ASSERT(w == 32);  /* 32 + 0 */
    ASSERT(h == 32);  /* 32 + 0 */
}

TEST(get_dimension_valid) {
    init_test_data();
    ushort w = 0, h = 0;

    u32 ret = FUN_0041f980(5, &w, &h);

    ASSERT(ret == 1);
    ASSERT(w == 37);  /* 32 + 5 */
    ASSERT(h == 42);  /* 32 + 5*2 */
}

TEST(get_dimension_extended) {
    init_test_data();
    ushort w = 0, h = 0;

    u32 ret = FUN_0041f980(510000, &w, &h);

    ASSERT(ret == 1);
}

TEST(get_dimension_invalid) {
    init_test_data();
    ushort w = 0, h = 0;

    u32 ret = FUN_0041f980(560000, &w, &h);

    ASSERT(ret == 0);
    ASSERT(w == 0);
    ASSERT(h == 0);
}

/* Boundary tests */
TEST(boundary_499999) {
    init_test_data();
    u32 result = 0;
    ushort x = 0, y = 0;

    u32 ret1 = FUN_0041fad0(499999, &result);
    u32 ret2 = FUN_0041f900(499999, &x, &y);

    ASSERT(ret1 == 1);
    ASSERT(ret2 == 1);
}

TEST(boundary_500000) {
    init_test_data();
    u32 result = 0;
    ushort x = 0, y = 0;

    u32 ret1 = FUN_0041fad0(500000, &result);
    u32 ret2 = FUN_0041f900(500000, &x, &y);

    ASSERT(ret1 == 1);
    ASSERT(ret2 == 1);
}

TEST(boundary_549999) {
    init_test_data();
    u32 result = 0;

    u32 ret = FUN_0041fad0(549999, &result);

    ASSERT(ret == 1);
}

TEST(boundary_550000) {
    init_test_data();
    u32 result = 0;

    u32 ret = FUN_0041fad0(550000, &result);

    ASSERT(ret == 0);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Sprite Lookup Functions Unit Tests ===\n\n");

    printf("FUN_0041fad0 (Get Sprite ID) Tests:\n");
    RUN_TEST(get_sprite_id_zero);
    RUN_TEST(get_sprite_id_valid);
    RUN_TEST(get_sprite_id_boundary);
    RUN_TEST(get_sprite_id_extended);
    RUN_TEST(get_sprite_id_invalid_high);
    RUN_TEST(get_sprite_id_invalid_very_high);

    printf("\nFUN_0041f900 (Get Sprite Offset) Tests:\n");
    RUN_TEST(get_offset_zero);
    RUN_TEST(get_offset_valid);
    RUN_TEST(get_offset_extended);
    RUN_TEST(get_offset_invalid);

    printf("\nFUN_0041f980 (Get Sprite Dimensions) Tests:\n");
    RUN_TEST(get_dimension_zero);
    RUN_TEST(get_dimension_valid);
    RUN_TEST(get_dimension_extended);
    RUN_TEST(get_dimension_invalid);

    printf("\nBoundary Tests:\n");
    RUN_TEST(boundary_499999);
    RUN_TEST(boundary_500000);
    RUN_TEST(boundary_549999);
    RUN_TEST(boundary_550000);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
