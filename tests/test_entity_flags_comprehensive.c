/*
 * Stone Age Client - Entity Flag Functions Unit Tests
 * Tests for entity flag manipulation functions - self-contained
 *
 * NOTE: Original code is 32-bit, uses pointer-sized accesses at offset 0xc.
 * On 64-bit, pointers are 8 bytes, so we simulate 32-bit behavior.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ========================================
 * Test Data and Mock Implementations
 * ======================================== */

/* Entity flag array - stride 0x43 per entity */
#define MAX_ENTITIES 100
#define FLAG_STRIDE 0x43
static unsigned int test_flag_array[MAX_ENTITIES * FLAG_STRIDE] = {0};

/* Test entity structure - matches 32-bit layout
 * In 32-bit: offset 0xc is a pointer to extra data
 * Extra data first int is entity_id
 */
typedef struct {
    char pad[12];
    int* extra;  /* At offset 0xc in 32-bit, but 0xc in 64-bit too */
} TestEntity;

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
 * Inline Implementations (from stubs_entity_flags.c)
 * Simplified for 64-bit test environment
 * ======================================== */

static void FUN_0040c020(void* entity) {
    TestEntity* ent = (TestEntity*)entity;
    int entity_id;

    if (entity == NULL) return;
    if (ent->extra == NULL) return;

    entity_id = ent->extra[0];
    test_flag_array[entity_id * FLAG_STRIDE] |= 0x100;
}

static void FUN_0040c050(void* entity) {
    TestEntity* ent = (TestEntity*)entity;
    int entity_id;

    if (entity == NULL) return;
    if (ent->extra == NULL) return;

    entity_id = ent->extra[0];
    test_flag_array[entity_id * FLAG_STRIDE] &= 0xfffffeff;
}

static void FUN_0040c080(void* entity) {
    TestEntity* ent = (TestEntity*)entity;
    int entity_id;

    if (entity == NULL) return;
    if (ent->extra == NULL) return;

    entity_id = ent->extra[0];
    test_flag_array[entity_id * FLAG_STRIDE] |= 0x200;
}

static void FUN_0040c0b0(void* entity) {
    TestEntity* ent = (TestEntity*)entity;
    int entity_id;

    if (entity == NULL) return;
    if (ent->extra == NULL) return;

    entity_id = ent->extra[0];
    test_flag_array[entity_id * FLAG_STRIDE] &= 0xfffffdff;
}

static void FUN_0040c0e0(void* entity) {
    TestEntity* ent = (TestEntity*)entity;
    int entity_id;

    if (entity == NULL) return;
    if (ent->extra == NULL) return;

    entity_id = ent->extra[0];
    test_flag_array[entity_id * FLAG_STRIDE] |= 0x4000;
}

static void FUN_0040c110(void* entity) {
    TestEntity* ent = (TestEntity*)entity;
    int entity_id;

    if (entity == NULL) return;
    if (ent->extra == NULL) return;

    entity_id = ent->extra[0];
    test_flag_array[entity_id * FLAG_STRIDE] &= 0xffffbfff;
}

static void FUN_0040c240(void* entity) {
    TestEntity* ent = (TestEntity*)entity;
    int entity_id;

    if (entity == NULL) return;
    if (ent->extra == NULL) return;

    entity_id = ent->extra[0];
    test_flag_array[entity_id * FLAG_STRIDE] |= 0x8000;
}

static void FUN_0040c270(void* entity) {
    TestEntity* ent = (TestEntity*)entity;
    int entity_id;

    if (entity == NULL) return;
    if (ent->extra == NULL) return;

    entity_id = ent->extra[0];
    test_flag_array[entity_id * FLAG_STRIDE] &= 0xffff7fff;
}

static void FUN_0040c2a0(void* entity) {
    TestEntity* ent = (TestEntity*)entity;
    int entity_id;

    if (entity == NULL) return;
    if (ent->extra == NULL) return;

    entity_id = ent->extra[0];
    test_flag_array[entity_id * FLAG_STRIDE] |= 0x10000;
}

static void FUN_0040c2d0(void* entity) {
    TestEntity* ent = (TestEntity*)entity;
    int entity_id;

    if (entity == NULL) return;
    if (ent->extra == NULL) return;

    entity_id = ent->extra[0];
    test_flag_array[entity_id * FLAG_STRIDE] &= 0xfffeffff;
}

static void FUN_0040c300(void* entity) {
    TestEntity* ent = (TestEntity*)entity;
    int entity_id;

    if (entity == NULL) return;
    if (ent->extra == NULL) return;

    entity_id = ent->extra[0];
    test_flag_array[entity_id * FLAG_STRIDE] &= 0xfffff7ff;
}

static void FUN_0040c210(void* entity) {
    TestEntity* ent = (TestEntity*)entity;
    int entity_id;

    if (entity == NULL) return;
    if (ent->extra == NULL) return;

    entity_id = ent->extra[0];
    test_flag_array[entity_id * FLAG_STRIDE] &= 0xffffebff;
}

static void FUN_0040c1a0(void* entity, int dialog_type, unsigned short param1, short param2) {
    TestEntity* ent = (TestEntity*)entity;
    int entity_id;

    if (entity == NULL) return;
    if (ent->extra == NULL) return;

    entity_id = ent->extra[0];

    test_flag_array[entity_id * FLAG_STRIDE] |= 0x400;

    if (param2 != 0) {
        test_flag_array[entity_id * FLAG_STRIDE] |= 0x1000;
    } else {
        test_flag_array[entity_id * FLAG_STRIDE] &= 0xffffefff;
    }

    (void)dialog_type;
    (void)param1;
}

/* ========================================
 * Test Helper Functions
 * ======================================== */

static int test_entity_id = 5;
static TestEntity test_entity;
static int test_extra[16];

static void setup_test_entity(void) {
    memset(&test_entity, 0, sizeof(test_entity));
    memset(test_extra, 0, sizeof(test_extra));
    test_extra[0] = test_entity_id;
    test_entity.extra = test_extra;

    /* Clear flag for this entity */
    test_flag_array[test_entity_id * FLAG_STRIDE] = 0;
}

/* ========================================
 * Hidden Flag Tests (0x100)
 * ======================================== */

TEST(set_hidden_flag) {
    setup_test_entity();
    unsigned int flags = test_flag_array[test_entity_id * FLAG_STRIDE];
    ASSERT((flags & 0x100) == 0);

    FUN_0040c020(&test_entity);
    flags = test_flag_array[test_entity_id * FLAG_STRIDE];
    ASSERT((flags & 0x100) != 0);
}

TEST(clear_hidden_flag) {
    setup_test_entity();
    test_flag_array[test_entity_id * FLAG_STRIDE] = 0x100;

    FUN_0040c050(&test_entity);
    unsigned int flags = test_flag_array[test_entity_id * FLAG_STRIDE];
    ASSERT((flags & 0x100) == 0);
}

TEST(hidden_flag_toggle) {
    setup_test_entity();

    FUN_0040c020(&test_entity);
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x100) != 0);

    FUN_0040c050(&test_entity);
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x100) == 0);

    FUN_0040c020(&test_entity);
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x100) != 0);
}

TEST(null_entity_hidden) {
    /* Should not crash with null entity */
    FUN_0040c020(NULL);
    FUN_0040c050(NULL);
}

/* ========================================
 * Highlight Flag Tests (0x200)
 * ======================================== */

TEST(set_highlight_flag) {
    setup_test_entity();
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x200) == 0);

    FUN_0040c080(&test_entity);
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x200) != 0);
}

TEST(clear_highlight_flag) {
    setup_test_entity();
    test_flag_array[test_entity_id * FLAG_STRIDE] = 0x200;

    FUN_0040c0b0(&test_entity);
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x200) == 0);
}

TEST(highlight_flag_preserves_others) {
    setup_test_entity();
    test_flag_array[test_entity_id * FLAG_STRIDE] = 0x100 | 0x400;

    FUN_0040c080(&test_entity);
    unsigned int flags = test_flag_array[test_entity_id * FLAG_STRIDE];
    ASSERT((flags & 0x200) != 0);
    ASSERT((flags & 0x100) != 0);
    ASSERT((flags & 0x400) != 0);
}

/* ========================================
 * Special Render Flag Tests (0x4000)
 * ======================================== */

TEST(set_special_render_flag) {
    setup_test_entity();
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x4000) == 0);

    FUN_0040c0e0(&test_entity);
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x4000) != 0);
}

TEST(clear_special_render_flag) {
    setup_test_entity();
    test_flag_array[test_entity_id * FLAG_STRIDE] = 0x4000;

    FUN_0040c110(&test_entity);
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x4000) == 0);
}

/* ========================================
 * Ride State Flag Tests (0x8000)
 * ======================================== */

TEST(set_ride_flag) {
    setup_test_entity();
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x8000) == 0);

    FUN_0040c240(&test_entity);
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x8000) != 0);
}

TEST(clear_ride_flag) {
    setup_test_entity();
    test_flag_array[test_entity_id * FLAG_STRIDE] = 0x8000;

    FUN_0040c270(&test_entity);
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x8000) == 0);
}

/* ========================================
 * Combat State Flag Tests (0x10000)
 * ======================================== */

TEST(set_combat_flag) {
    setup_test_entity();
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x10000) == 0);

    FUN_0040c2a0(&test_entity);
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x10000) != 0);
}

TEST(clear_combat_flag) {
    setup_test_entity();
    test_flag_array[test_entity_id * FLAG_STRIDE] = 0x10000;

    FUN_0040c2d0(&test_entity);
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x10000) == 0);
}

/* ========================================
 * Animation State Flag Tests (0x800)
 * ======================================== */

TEST(clear_animation_flag) {
    setup_test_entity();
    test_flag_array[test_entity_id * FLAG_STRIDE] = 0x800;

    FUN_0040c300(&test_entity);
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x800) == 0);
}

TEST(clear_animation_preserves_others) {
    setup_test_entity();
    test_flag_array[test_entity_id * FLAG_STRIDE] = 0x800 | 0x100 | 0x200;

    FUN_0040c300(&test_entity);
    unsigned int flags = test_flag_array[test_entity_id * FLAG_STRIDE];
    ASSERT((flags & 0x800) == 0);
    ASSERT((flags & 0x100) != 0);
    ASSERT((flags & 0x200) != 0);
}

/* ========================================
 * Target Indicator Flag Tests (0x400)
 * ======================================== */

TEST(clear_target_flag) {
    setup_test_entity();
    test_flag_array[test_entity_id * FLAG_STRIDE] = 0x400;

    FUN_0040c210(&test_entity);
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x400) == 0);
}

/* ========================================
 * Dialog State Tests
 * ======================================== */

TEST(set_dialog_state) {
    setup_test_entity();
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x400) == 0);

    FUN_0040c1a0(&test_entity, 1, 100, 0);
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x400) != 0);
}

TEST(dialog_state_with_param2) {
    setup_test_entity();
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x1000) == 0);

    FUN_0040c1a0(&test_entity, 1, 100, 5);
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x1000) != 0);
}

TEST(dialog_state_without_param2) {
    setup_test_entity();
    test_flag_array[test_entity_id * FLAG_STRIDE] = 0x1000;

    FUN_0040c1a0(&test_entity, 1, 100, 0);
    ASSERT((test_flag_array[test_entity_id * FLAG_STRIDE] & 0x1000) == 0);
}

/* ========================================
 * Multiple Flag Tests
 * ======================================== */

TEST(multiple_flags_independent) {
    setup_test_entity();

    FUN_0040c020(&test_entity);  /* Hidden */
    FUN_0040c080(&test_entity);  /* Highlight */
    FUN_0040c0e0(&test_entity);  /* Special render */

    unsigned int flags = test_flag_array[test_entity_id * FLAG_STRIDE];
    ASSERT((flags & 0x100) != 0);
    ASSERT((flags & 0x200) != 0);
    ASSERT((flags & 0x4000) != 0);
}

TEST(clear_one_preserves_others) {
    setup_test_entity();

    FUN_0040c020(&test_entity);  /* Hidden */
    FUN_0040c080(&test_entity);  /* Highlight */
    FUN_0040c240(&test_entity);  /* Ride */

    FUN_0040c050(&test_entity);  /* Clear hidden */

    unsigned int flags = test_flag_array[test_entity_id * FLAG_STRIDE];
    ASSERT((flags & 0x100) == 0);
    ASSERT((flags & 0x200) != 0);
    ASSERT((flags & 0x8000) != 0);
}

TEST(all_flags_combination) {
    setup_test_entity();

    FUN_0040c020(&test_entity);   /* Hidden 0x100 */
    FUN_0040c080(&test_entity);   /* Highlight 0x200 */
    FUN_0040c240(&test_entity);   /* Ride 0x8000 */
    FUN_0040c2a0(&test_entity);   /* Combat 0x10000 */
    FUN_0040c0e0(&test_entity);   /* Special render 0x4000 */

    unsigned int flags = test_flag_array[test_entity_id * FLAG_STRIDE];
    ASSERT(flags == (0x100 | 0x200 | 0x8000 | 0x10000 | 0x4000));
}

/* ========================================
 * Entity ID Stride Tests
 * ======================================== */

TEST(different_entity_ids) {
    int id1 = 10;
    int id2 = 20;

    int extra1[16] = {id1};
    int extra2[16] = {id2};
    TestEntity entity1 = {.extra = extra1};
    TestEntity entity2 = {.extra = extra2};

    test_flag_array[id1 * FLAG_STRIDE] = 0;
    test_flag_array[id2 * FLAG_STRIDE] = 0;

    FUN_0040c020(&entity1);
    FUN_0040c080(&entity2);

    ASSERT((test_flag_array[id1 * FLAG_STRIDE] & 0x100) != 0);
    ASSERT((test_flag_array[id1 * FLAG_STRIDE] & 0x200) == 0);
    ASSERT((test_flag_array[id2 * FLAG_STRIDE] & 0x100) == 0);
    ASSERT((test_flag_array[id2 * FLAG_STRIDE] & 0x200) != 0);
}

TEST(flag_stride_calculation) {
    /* Verify that stride is calculated correctly */
    int id = 50;
    int expected_index = id * FLAG_STRIDE;

    int extra[16] = {id};
    TestEntity entity = {.extra = extra};

    test_flag_array[expected_index] = 0;
    FUN_0040c020(&entity);

    ASSERT((test_flag_array[expected_index] & 0x100) != 0);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Entity Flag Functions Unit Tests ===\n\n");

    printf("Hidden Flag Tests (0x100):\n");
    RUN_TEST(set_hidden_flag);
    RUN_TEST(clear_hidden_flag);
    RUN_TEST(hidden_flag_toggle);
    RUN_TEST(null_entity_hidden);

    printf("\nHighlight Flag Tests (0x200):\n");
    RUN_TEST(set_highlight_flag);
    RUN_TEST(clear_highlight_flag);
    RUN_TEST(highlight_flag_preserves_others);

    printf("\nSpecial Render Flag Tests (0x4000):\n");
    RUN_TEST(set_special_render_flag);
    RUN_TEST(clear_special_render_flag);

    printf("\nRide State Flag Tests (0x8000):\n");
    RUN_TEST(set_ride_flag);
    RUN_TEST(clear_ride_flag);

    printf("\nCombat State Flag Tests (0x10000):\n");
    RUN_TEST(set_combat_flag);
    RUN_TEST(clear_combat_flag);

    printf("\nAnimation State Flag Tests (0x800):\n");
    RUN_TEST(clear_animation_flag);
    RUN_TEST(clear_animation_preserves_others);

    printf("\nTarget Indicator Flag Tests (0x400):\n");
    RUN_TEST(clear_target_flag);

    printf("\nDialog State Tests:\n");
    RUN_TEST(set_dialog_state);
    RUN_TEST(dialog_state_with_param2);
    RUN_TEST(dialog_state_without_param2);

    printf("\nMultiple Flag Tests:\n");
    RUN_TEST(multiple_flags_independent);
    RUN_TEST(clear_one_preserves_others);
    RUN_TEST(all_flags_combination);

    printf("\nEntity ID Stride Tests:\n");
    RUN_TEST(different_entity_ids);
    RUN_TEST(flag_stride_calculation);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
