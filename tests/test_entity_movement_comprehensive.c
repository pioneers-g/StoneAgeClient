/*
 * Stone Age Client - Entity Movement System Comprehensive Tests
 * Tests for FUN_0040b6e0, FUN_0040b740 and related functions
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

/* Test framework macros */
#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    test_##name(); \
    printf("PASS\n"); \
    tests_run++; \
} while(0)

#define ASSERT_EQ(a, b) assert((a) == (b))
#define ASSERT_NE(a, b) assert((a) != (b))
#define ASSERT_TRUE(x) assert(x)
#define ASSERT_FALSE(x) assert(!(x))

/* Types */
typedef unsigned int u32;
typedef unsigned short u16;
typedef int s32;

/* Entity structure offsets from binary analysis */
#define ENTITY_OFFSET_X_QUEUE      0xc0
#define ENTITY_OFFSET_Y_QUEUE      0xe8
#define ENTITY_OFFSET_QUEUE_COUNT  0x110
#define ENTITY_OFFSET_CURRENT_X    0xb8
#define ENTITY_OFFSET_CURRENT_Y    0xbc
#define MAX_QUEUE_SIZE             10

/*
 * FUN_0040b6e0 - Entity Movement Queue Add
 */
static void FUN_0040b6e0(void* entity, int target_x, int target_y) {
    unsigned short* queue_count;
    int* x_queue;
    int* y_queue;

    if (entity == NULL) return;

    queue_count = (unsigned short*)((char*)entity + ENTITY_OFFSET_QUEUE_COUNT);

    if (*queue_count < 10) {
        x_queue = (int*)((char*)entity + ENTITY_OFFSET_X_QUEUE);
        y_queue = (int*)((char*)entity + ENTITY_OFFSET_Y_QUEUE);

        x_queue[*queue_count] = target_x;
        y_queue[*queue_count] = target_y;
        (*queue_count)++;
    } else {
        *queue_count = 0;
    }
}

/*
 * FUN_0040b740 - Entity Movement with Interpolation
 */
static void FUN_0040b740(void* entity, int target_x, int target_y) {
    short* queue_count;
    int* x_queue;
    int* y_queue;
    int current_x, current_y;
    int delta_x, delta_y;
    int abs_dx, abs_dy;
    int waypoints[3][2];
    int num_waypoints = 1;
    int i;

    if (entity == NULL) return;

    queue_count = (short*)((char*)entity + ENTITY_OFFSET_QUEUE_COUNT);
    x_queue = (int*)((char*)entity + ENTITY_OFFSET_X_QUEUE);
    y_queue = (int*)((char*)entity + ENTITY_OFFSET_Y_QUEUE);

    if (*queue_count < 1) {
        current_x = *(int*)((char*)entity + ENTITY_OFFSET_CURRENT_X);
        current_y = *(int*)((char*)entity + ENTITY_OFFSET_CURRENT_Y);
    } else {
        current_x = x_queue[*queue_count - 1];
        current_y = y_queue[*queue_count - 1];
    }

    delta_x = target_x - current_x;
    delta_y = target_y - current_y;
    abs_dx = (delta_x < 0) ? -delta_x : delta_x;
    abs_dy = (delta_y < 0) ? -delta_y : delta_y;

    if (abs_dx == 2 && abs_dy == 2) {
        waypoints[0][0] = current_y + delta_y / 2;
        waypoints[0][1] = current_x + delta_x / 2;
        num_waypoints = 2;
    } else if (abs_dx == 2) {
        waypoints[0][0] = target_y;
        waypoints[0][1] = current_x + delta_x / 2;
        num_waypoints = 2;
    } else if (abs_dy == 2) {
        waypoints[0][0] = current_y + delta_y / 2;
        waypoints[0][1] = target_x;
        num_waypoints = 2;
    }

    waypoints[num_waypoints - 1][0] = target_y;
    waypoints[num_waypoints - 1][1] = target_x;

    if ((unsigned int)(*queue_count + num_waypoints) < 11) {
        for (i = 0; i < num_waypoints; i++) {
            x_queue[*queue_count] = waypoints[i][1];
            y_queue[*queue_count] = waypoints[i][0];
            (*queue_count)++;
        }
    } else {
        *queue_count = 0;
    }
}

/* Test entity buffer */
#define ENTITY_BUFFER_SIZE 0x120
typedef struct {
    char buffer[ENTITY_BUFFER_SIZE];
} TestEntity;

/* Helper functions */
static s32* get_current_x(TestEntity* e) { return (s32*)&e->buffer[ENTITY_OFFSET_CURRENT_X]; }
static s32* get_current_y(TestEntity* e) { return (s32*)&e->buffer[ENTITY_OFFSET_CURRENT_Y]; }
static s32* get_x_queue(TestEntity* e, int idx) { return (s32*)&e->buffer[ENTITY_OFFSET_X_QUEUE + idx * 4]; }
static s32* get_y_queue(TestEntity* e, int idx) { return (s32*)&e->buffer[ENTITY_OFFSET_Y_QUEUE + idx * 4]; }
static u16* get_queue_count(TestEntity* e) { return (u16*)&e->buffer[ENTITY_OFFSET_QUEUE_COUNT]; }

static void init_test_entity(TestEntity* entity) {
    memset(entity, 0, sizeof(TestEntity));
    *get_current_x(entity) = 100;
    *get_current_y(entity) = 100;
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(entity_movement_constants) {
    ASSERT_EQ(ENTITY_OFFSET_X_QUEUE, 0xc0);
    ASSERT_EQ(ENTITY_OFFSET_Y_QUEUE, 0xe8);
    ASSERT_EQ(ENTITY_OFFSET_QUEUE_COUNT, 0x110);
    ASSERT_EQ(MAX_QUEUE_SIZE, 10);
}

TEST(entity_movement_queue_init) {
    TestEntity entity;
    init_test_entity(&entity);

    ASSERT_EQ(*get_queue_count(&entity), 0);
    ASSERT_EQ(*get_current_x(&entity), 100);
    ASSERT_EQ(*get_current_y(&entity), 100);
}

TEST(entity_movement_queue_add_single) {
    TestEntity entity;
    init_test_entity(&entity);

    FUN_0040b6e0(&entity, 110, 120);

    ASSERT_EQ(*get_queue_count(&entity), 1);
    ASSERT_EQ(*get_x_queue(&entity, 0), 110);
    ASSERT_EQ(*get_y_queue(&entity, 0), 120);
}

TEST(entity_movement_queue_add_multiple) {
    TestEntity entity;
    init_test_entity(&entity);

    FUN_0040b6e0(&entity, 110, 100);
    FUN_0040b6e0(&entity, 120, 100);
    FUN_0040b6e0(&entity, 130, 110);

    ASSERT_EQ(*get_queue_count(&entity), 3);
    ASSERT_EQ(*get_x_queue(&entity, 0), 110);
    ASSERT_EQ(*get_x_queue(&entity, 1), 120);
    ASSERT_EQ(*get_x_queue(&entity, 2), 130);
}

TEST(entity_movement_queue_fill) {
    TestEntity entity;
    int i;

    init_test_entity(&entity);

    for (i = 0; i < 10; i++) {
        FUN_0040b6e0(&entity, 100 + i, 100 + i);
    }

    ASSERT_EQ(*get_queue_count(&entity), 10);
}

TEST(entity_movement_queue_overflow) {
    TestEntity entity;
    int i;

    init_test_entity(&entity);

    for (i = 0; i < 12; i++) {
        FUN_0040b6e0(&entity, 100 + i, 100 + i);
    }

    ASSERT_TRUE(*get_queue_count(&entity) <= 10);
}

TEST(entity_movement_null_entity) {
    FUN_0040b6e0(NULL, 100, 100);
    FUN_0040b740(NULL, 100, 100);
    ASSERT_TRUE(1);
}

TEST(entity_movement_interpolation_diagonal) {
    TestEntity entity;
    init_test_entity(&entity);

    *get_current_x(&entity) = 100;
    *get_current_y(&entity) = 100;

    FUN_0040b740(&entity, 102, 102);

    ASSERT_TRUE(*get_queue_count(&entity) >= 1);
}

TEST(entity_movement_interpolation_x_only) {
    TestEntity entity;
    init_test_entity(&entity);

    *get_current_x(&entity) = 100;
    *get_current_y(&entity) = 100;

    FUN_0040b740(&entity, 102, 100);

    ASSERT_TRUE(*get_queue_count(&entity) >= 1);
}

TEST(entity_movement_interpolation_y_only) {
    TestEntity entity;
    init_test_entity(&entity);

    *get_current_x(&entity) = 100;
    *get_current_y(&entity) = 100;

    FUN_0040b740(&entity, 100, 102);

    ASSERT_TRUE(*get_queue_count(&entity) >= 1);
}

TEST(entity_movement_queue_order) {
    TestEntity entity;
    init_test_entity(&entity);

    FUN_0040b6e0(&entity, 110, 100);
    FUN_0040b6e0(&entity, 120, 100);
    FUN_0040b6e0(&entity, 130, 100);

    ASSERT_EQ(*get_x_queue(&entity, 0), 110);
    ASSERT_EQ(*get_x_queue(&entity, 1), 120);
    ASSERT_EQ(*get_x_queue(&entity, 2), 130);
}

TEST(entity_movement_negative_coords) {
    TestEntity entity;
    init_test_entity(&entity);

    FUN_0040b6e0(&entity, -10, -20);

    ASSERT_EQ(*get_queue_count(&entity), 1);
    ASSERT_EQ(*get_x_queue(&entity, 0), -10);
    ASSERT_EQ(*get_y_queue(&entity, 0), -20);
}

TEST(entity_movement_large_coords) {
    TestEntity entity;
    init_test_entity(&entity);

    FUN_0040b6e0(&entity, 10000, 20000);

    ASSERT_EQ(*get_queue_count(&entity), 1);
    ASSERT_EQ(*get_x_queue(&entity, 0), 10000);
    ASSERT_EQ(*get_y_queue(&entity, 0), 20000);
}

TEST(entity_movement_interpolation_with_existing_queue) {
    TestEntity entity;
    init_test_entity(&entity);

    FUN_0040b6e0(&entity, 110, 100);
    FUN_0040b6e0(&entity, 120, 100);

    FUN_0040b740(&entity, 122, 102);

    ASSERT_TRUE(*get_queue_count(&entity) >= 2);
}

TEST(entity_movement_boundary_coords) {
    TestEntity entity;
    init_test_entity(&entity);

    FUN_0040b6e0(&entity, 0, 0);
    FUN_0040b6e0(&entity, 65535, 65535);
    FUN_0040b6e0(&entity, -32768, -32768);

    ASSERT_EQ(*get_queue_count(&entity), 3);
    ASSERT_EQ(*get_x_queue(&entity, 0), 0);
    ASSERT_EQ(*get_x_queue(&entity, 1), 65535);
    ASSERT_EQ(*get_x_queue(&entity, 2), -32768);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    int tests_run = 0;

    printf("\n=== Entity Movement System Comprehensive Tests ===\n\n");

    printf("Structure Tests:\n");
    RUN_TEST(entity_movement_constants);
    RUN_TEST(entity_movement_queue_init);

    printf("\nQueue Operation Tests:\n");
    RUN_TEST(entity_movement_queue_add_single);
    RUN_TEST(entity_movement_queue_add_multiple);
    RUN_TEST(entity_movement_queue_fill);
    RUN_TEST(entity_movement_queue_overflow);
    RUN_TEST(entity_movement_queue_order);

    printf("\nInterpolation Tests:\n");
    RUN_TEST(entity_movement_interpolation_diagonal);
    RUN_TEST(entity_movement_interpolation_x_only);
    RUN_TEST(entity_movement_interpolation_y_only);
    RUN_TEST(entity_movement_interpolation_with_existing_queue);

    printf("\nEdge Case Tests:\n");
    RUN_TEST(entity_movement_null_entity);
    RUN_TEST(entity_movement_negative_coords);
    RUN_TEST(entity_movement_large_coords);
    RUN_TEST(entity_movement_boundary_coords);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_run);
    printf("Tests failed: 0\n");

    return 0;
}
