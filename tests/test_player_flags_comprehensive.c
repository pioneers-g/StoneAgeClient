/*
 * Stone Age Client - Player Flag Functions Unit Tests
 * Tests for player flag manipulation functions - self-contained
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions for self-contained test */
typedef unsigned int u32;
typedef unsigned short u16;
typedef int s32;

/* ========================================
 * Test Data and Mock Implementations
 * ======================================== */

/* Player flags global */
static u32 test_player_flags = 0;

/* Player entity mock */
static void* test_player_entity = NULL;

/* Entity data for testing */
#define ENTITY_STATE_OFFSET 0x148
#define ENTITY_DIRECTION_OFFSET 0x150
#define ENTITY_MOVE_FLAG_OFFSET 0x112
static char test_entity_data[0x200] = {0};

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
 * Inline Implementations (from stubs_player.c)
 * ======================================== */

static void FUN_00478090(void) {
    test_player_flags |= 0x100;
}

static void FUN_004780a0(void) {
    test_player_flags &= 0xfffffeff;
}

static void FUN_004780b0(void) {
    test_player_flags |= 0x200;
}

static void FUN_004780c0(void) {
    test_player_flags &= 0xfffffdff;
}

static void FUN_004780d0(void) {
    test_player_flags |= 0x4000;
}

static void FUN_00478150(void) {
    test_player_flags &= 0xfffff7ff;
}

static void FUN_00477d70(u32 direction) {
    if (test_player_entity != NULL) {
        *(u32*)(test_entity_data + ENTITY_DIRECTION_OFFSET) = direction;
    }
}

static void FUN_00477d90(int state) {
    if (test_player_entity != NULL) {
        *(int*)(test_entity_data + ENTITY_STATE_OFFSET) = state;
        if (state != 4) {
            *(int*)(test_entity_data + 0x14c) = -1;
        }
    }
}

static void FUN_00478110(void) {
    if (test_player_entity != NULL) {
        *(unsigned short*)(test_entity_data + ENTITY_MOVE_FLAG_OFFSET) = 0;
    }
}

/* Clear special render flag - FUN_004780e0 */
static void clear_special_render_flag_inline(void) {
    test_player_flags &= 0xffffbfff;
}

/* Set movement flag to 1 - FUN_004780f0 */
static void set_movement_flag_one_inline(void) {
    if (test_player_entity != NULL) {
        *(unsigned short*)(test_entity_data + ENTITY_MOVE_FLAG_OFFSET) = 1;
    }
}

/* Player slot array for testing */
#define SLOT_ARRAY_COUNT 20
static u32 test_slot_array[SLOT_ARRAY_COUNT * 12] = {0};

/* Clear player slot array - FUN_00478980 */
static void clear_player_slot_array_inline(void) {
    u32* entry = test_slot_array;
    int max_count = SLOT_ARRAY_COUNT * 12;
    do {
        *entry = 0;
        entry += 12;
    } while ((entry - test_slot_array) < max_count);
}

/* ========================================
 * Test Helper Functions
 * ======================================== */

static void setup_test(void) {
    test_player_flags = 0;
    test_player_entity = test_entity_data;
    memset(test_entity_data, 0, sizeof(test_entity_data));
}

/* ========================================
 * Ride Flag Tests (0x100)
 * ======================================== */

TEST(set_ride_flag) {
    setup_test();
    ASSERT((test_player_flags & 0x100) == 0);

    FUN_00478090();
    ASSERT((test_player_flags & 0x100) != 0);
}

TEST(clear_ride_flag) {
    setup_test();
    test_player_flags = 0x100;

    FUN_004780a0();
    ASSERT((test_player_flags & 0x100) == 0);
}

TEST(ride_flag_toggle) {
    setup_test();

    FUN_00478090();
    ASSERT((test_player_flags & 0x100) != 0);

    FUN_004780a0();
    ASSERT((test_player_flags & 0x100) == 0);

    FUN_00478090();
    ASSERT((test_player_flags & 0x100) != 0);
}

/* ========================================
 * Highlight Flag Tests (0x200)
 * ======================================== */

TEST(set_highlight_flag) {
    setup_test();
    ASSERT((test_player_flags & 0x200) == 0);

    FUN_004780b0();
    ASSERT((test_player_flags & 0x200) != 0);
}

TEST(clear_highlight_flag) {
    setup_test();
    test_player_flags = 0x200;

    FUN_004780c0();
    ASSERT((test_player_flags & 0x200) == 0);
}

TEST(highlight_flag_toggle) {
    setup_test();

    FUN_004780b0();
    ASSERT((test_player_flags & 0x200) != 0);

    FUN_004780c0();
    ASSERT((test_player_flags & 0x200) == 0);
}

/* ========================================
 * Special Render Flag Tests (0x4000)
 * ======================================== */

TEST(set_special_render_flag) {
    setup_test();
    ASSERT((test_player_flags & 0x4000) == 0);

    FUN_004780d0();
    ASSERT((test_player_flags & 0x4000) != 0);
}

/* ========================================
 * Animation Flag Tests (0x800)
 * ======================================== */

TEST(clear_animation_flag) {
    setup_test();
    test_player_flags = 0x800;

    FUN_00478150();
    ASSERT((test_player_flags & 0x800) == 0);
}

TEST(animation_preserves_other_flags) {
    setup_test();
    test_player_flags = 0x800 | 0x100 | 0x200;

    FUN_00478150();
    ASSERT((test_player_flags & 0x800) == 0);
    ASSERT((test_player_flags & 0x100) != 0);
    ASSERT((test_player_flags & 0x200) != 0);
}

/* ========================================
 * Player State Tests
 * ======================================== */

TEST(set_player_state_idle) {
    setup_test();

    FUN_00477d90(3);  /* Idle state */
    ASSERT(*(int*)(test_entity_data + ENTITY_STATE_OFFSET) == 3);
    ASSERT(*(int*)(test_entity_data + 0x14c) == -1);
}

TEST(set_player_state_moving) {
    setup_test();

    FUN_00477d90(4);  /* Moving state */
    ASSERT(*(int*)(test_entity_data + ENTITY_STATE_OFFSET) == 4);
    /* 0x14c should NOT be set to -1 for moving state */
}

TEST(set_player_state_null_entity) {
    setup_test();
    test_player_entity = NULL;

    /* Should not crash */
    FUN_00477d90(3);
}

/* ========================================
 * Player Direction Tests
 * ======================================== */

TEST(set_player_direction) {
    setup_test();

    FUN_00477d70(5);
    ASSERT(*(u32*)(test_entity_data + ENTITY_DIRECTION_OFFSET) == 5);
}

TEST(set_player_direction_null_entity) {
    setup_test();
    test_player_entity = NULL;

    /* Should not crash */
    FUN_00477d70(10);
}

/* ========================================
 * Movement Flag Tests
 * ======================================== */

TEST(clear_movement_flag) {
    setup_test();
    *(unsigned short*)(test_entity_data + ENTITY_MOVE_FLAG_OFFSET) = 1;

    FUN_00478110();
    ASSERT(*(unsigned short*)(test_entity_data + ENTITY_MOVE_FLAG_OFFSET) == 0);
}

TEST(clear_movement_flag_null_entity) {
    setup_test();
    test_player_entity = NULL;

    /* Should not crash */
    FUN_00478110();
}

/* ========================================
 * Multiple Flag Tests
 * ======================================== */

TEST(multiple_flags_independent) {
    setup_test();

    FUN_00478090();  /* Ride */
    FUN_004780b0();  /* Highlight */
    FUN_004780d0();  /* Special render */

    ASSERT((test_player_flags & 0x100) != 0);
    ASSERT((test_player_flags & 0x200) != 0);
    ASSERT((test_player_flags & 0x4000) != 0);
}

TEST(clear_one_preserves_others) {
    setup_test();

    FUN_00478090();  /* Ride */
    FUN_004780b0();  /* Highlight */

    FUN_004780a0();  /* Clear ride */

    ASSERT((test_player_flags & 0x100) == 0);
    ASSERT((test_player_flags & 0x200) != 0);
}

TEST(all_flags_combination) {
    setup_test();

    FUN_00478090();   /* Ride 0x100 */
    FUN_004780b0();   /* Highlight 0x200 */
    FUN_004780d0();   /* Special render 0x4000 */

    ASSERT(test_player_flags == (0x100 | 0x200 | 0x4000));
}

/* ========================================
 * Clear Special Render Flag Tests (0x4000)
 * ======================================== */

TEST(clear_special_render_flag) {
    setup_test();
    FUN_004780d0();  /* Set first */

    clear_special_render_flag_inline();  /* FUN_004780e0 */
    ASSERT((test_player_flags & 0x4000) == 0);
}

TEST(clear_special_render_preserves_others) {
    setup_test();
    FUN_00478090();  /* Ride 0x100 */
    FUN_004780d0();  /* Special render 0x4000 */

    clear_special_render_flag_inline();
    ASSERT((test_player_flags & 0x100) != 0);
    ASSERT((test_player_flags & 0x4000) == 0);
}

/* ========================================
 * Set Movement Flag Tests
 * ======================================== */

TEST(set_movement_flag_to_one) {
    setup_test();
    *(unsigned short*)(test_entity_data + ENTITY_MOVE_FLAG_OFFSET) = 0;

    set_movement_flag_one_inline();  /* FUN_004780f0 */
    ASSERT(*(unsigned short*)(test_entity_data + ENTITY_MOVE_FLAG_OFFSET) == 1);
}

TEST(set_movement_flag_null_entity) {
    setup_test();
    test_player_entity = NULL;

    /* Should not crash */
    set_movement_flag_one_inline();
}

/* ========================================
 * Player Slot Array Clear Tests
 * ======================================== */

TEST(clear_player_slot_array) {
    /* Set up some test data at indices that get cleared (0, 12, 24, etc) */
    test_slot_array[0] = 0x12345678;
    test_slot_array[12] = 0xCAFEBABE;
    test_slot_array[24] = 0xDEADBEEF;

    clear_player_slot_array_inline();  /* FUN_00478980 */

    /* The function clears every 12th element (indices 0, 12, 24, ...) */
    ASSERT(test_slot_array[0] == 0);
    ASSERT(test_slot_array[12] == 0);
    ASSERT(test_slot_array[24] == 0);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Player Flag Functions Unit Tests ===\n\n");

    printf("Ride Flag Tests (0x100):\n");
    RUN_TEST(set_ride_flag);
    RUN_TEST(clear_ride_flag);
    RUN_TEST(ride_flag_toggle);

    printf("\nHighlight Flag Tests (0x200):\n");
    RUN_TEST(set_highlight_flag);
    RUN_TEST(clear_highlight_flag);
    RUN_TEST(highlight_flag_toggle);

    printf("\nSpecial Render Flag Tests (0x4000):\n");
    RUN_TEST(set_special_render_flag);
    RUN_TEST(clear_special_render_flag);
    RUN_TEST(clear_special_render_preserves_others);

    printf("\nAnimation Flag Tests (0x800):\n");
    RUN_TEST(clear_animation_flag);
    RUN_TEST(animation_preserves_other_flags);

    printf("\nPlayer State Tests:\n");
    RUN_TEST(set_player_state_idle);
    RUN_TEST(set_player_state_moving);
    RUN_TEST(set_player_state_null_entity);

    printf("\nPlayer Direction Tests:\n");
    RUN_TEST(set_player_direction);
    RUN_TEST(set_player_direction_null_entity);

    printf("\nMovement Flag Tests:\n");
    RUN_TEST(clear_movement_flag);
    RUN_TEST(clear_movement_flag_null_entity);
    RUN_TEST(set_movement_flag_to_one);
    RUN_TEST(set_movement_flag_null_entity);

    printf("\nMultiple Flag Tests:\n");
    RUN_TEST(multiple_flags_independent);
    RUN_TEST(clear_one_preserves_others);
    RUN_TEST(all_flags_combination);

    printf("\nPlayer Slot Array Tests:\n");
    RUN_TEST(clear_player_slot_array);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
