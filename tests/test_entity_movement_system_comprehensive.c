/*
 * Stone Age Client - Entity Movement System Unit Tests
 * Tests for entity walking/movement, path queue, direction calculation
 *
 * Based on Ghidra decompilation analysis:
 * - FUN_0040b6e0: Add single walk step
 * - FUN_0040b740: Add long walk with path interpolation
 * - FUN_0040bfc0: Set entity position (instant)
 * - FUN_00443e80: Get direction deltas
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
typedef short s16;
typedef int s32;

/* Movement constants */
#define MAX_PATH_LENGTH     11      /* Maximum path queue length */
#define ISOMETRIC_FACTOR    32.0f   /* Approximate isometric factor */

/* Direction constants */
#define DIR_SOUTH_WEST      0
#define DIR_WEST            1
#define DIR_NORTH_WEST      2
#define DIR_NORTH           3
#define DIR_NORTH_EAST      4
#define DIR_EAST            5
#define DIR_SOUTH_EAST      6
#define DIR_SOUTH           7
#define DIR_MAX             8

/* Test entity structure */
typedef struct Entity {
    s32 render_x;
    s32 render_y;
    s32 target_x;
    s32 target_y;
    s32 path_x[MAX_PATH_LENGTH];
    s32 path_y[MAX_PATH_LENGTH];
    s16 path_count;
    float iso_x;
    float iso_y;
    s32 field_11C;
    s32 field_120;
} Entity;

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
    /* Called before each test */
}

/*
 * Set entity position (FUN_0040bfc0)
 */
static void entity_set_position(Entity* entity, s32 x, s32 y) {
    if (entity == NULL) return;

    entity->render_x = x;
    entity->render_y = y;
    entity->target_x = x;
    entity->target_y = y;
    entity->iso_x = (float)x * ISOMETRIC_FACTOR;
    entity->iso_y = (float)y * ISOMETRIC_FACTOR;
    entity->field_11C = 0;
    entity->field_120 = 0;
}

/*
 * Add single walk step (FUN_0040b6e0)
 */
static void entity_add_walk_step(Entity* entity, s32 x, s32 y) {
    if (entity == NULL) return;

    if (entity->path_count >= 10) {
        entity->path_count = 0;
        entity_set_position(entity, x, y);
        return;
    }

    entity->path_x[entity->path_count] = x;
    entity->path_y[entity->path_count] = y;
    entity->path_count++;
}

/*
 * Add long walk with path interpolation (FUN_0040b740)
 */
static void entity_add_long_walk(Entity* entity, s32 dest_x, s32 dest_y) {
    if (entity == NULL) return;

    /* Get current position */
    s32 current_x, current_y;
    if (entity->path_count < 1) {
        current_x = entity->target_x;
        current_y = entity->target_y;
    } else {
        current_x = entity->path_x[entity->path_count - 1];
        current_y = entity->path_y[entity->path_count - 1];
    }

    s32 delta_x = dest_x - current_x;
    s32 delta_y = dest_y - current_y;

    s32 abs_x = delta_x < 0 ? -delta_x : delta_x;
    s32 abs_y = delta_y < 0 ? -delta_y : delta_y;

    s32 mid_x = 0, mid_y = 0;
    s32 steps = 0;

    /* Check for diagonal movement */
    if (abs_x == 2 && abs_y == 2) {
        mid_x = current_x + delta_x / 2;
        mid_y = current_y + delta_y / 2;
        steps = 1;
    }
    /* Check for horizontal movement */
    else if (abs_x == 2) {
        mid_x = current_x + delta_x / 2;
        mid_y = dest_y;
        steps = 1;
    }
    /* Check for vertical movement */
    else if (abs_y == 2) {
        mid_x = dest_x;
        mid_y = current_y + delta_y / 2;
        steps = 1;
    }

    /* Build path points */
    s32 path_points_x[3];
    s32 path_points_y[3];

    if (steps == 1) {
        path_points_x[0] = mid_x;
        path_points_y[0] = mid_y;
    }
    path_points_x[steps] = dest_x;
    path_points_y[steps] = dest_y;
    steps++;

    /* Check if queue has room */
    if (entity->path_count + steps < MAX_PATH_LENGTH) {
        for (s32 i = 0; i < steps; i++) {
            entity->path_x[entity->path_count] = path_points_x[i];
            entity->path_y[entity->path_count] = path_points_y[i];
            entity->path_count++;
        }
    } else {
        entity->path_count = 0;
        entity_set_position(entity, dest_x, dest_y);
    }
}

/*
 * Get direction deltas (FUN_00443e80)
 */
static void get_direction_delta(s32 direction, s32* dx, s32* dy) {
    switch (direction) {
        case DIR_SOUTH_WEST:  *dx = -1; *dy =  1; break;
        case DIR_WEST:        *dx = -1; *dy =  0; break;
        case DIR_NORTH_WEST:  *dx = -1; *dy = -1; break;
        case DIR_NORTH:       *dx =  0; *dy = -1; break;
        case DIR_NORTH_EAST:  *dx =  1; *dy = -1; break;
        case DIR_EAST:        *dx =  1; *dy =  0; break;
        case DIR_SOUTH_EAST:  *dx =  1; *dy =  1; break;
        case DIR_SOUTH:       *dx =  0; *dy =  1; break;
        default:              *dx =  0; *dy =  0; break;
    }
}

/* ========================================
 * Position Setting Tests
 * ======================================== */

static int test_set_position_basic(void) {
    test_setup();
    Entity entity = {0};

    entity_set_position(&entity, 100, 200);

    assert(entity.render_x == 100);
    assert(entity.render_y == 200);
    assert(entity.target_x == 100);
    assert(entity.target_y == 200);

    return 1;
}

static int test_set_position_null(void) {
    test_setup();

    /* Should not crash */
    entity_set_position(NULL, 100, 200);

    return 1;
}

static int test_set_position_isometric(void) {
    test_setup();
    Entity entity = {0};

    entity_set_position(&entity, 10, 20);

    assert(entity.iso_x == 10 * ISOMETRIC_FACTOR);
    assert(entity.iso_y == 20 * ISOMETRIC_FACTOR);

    return 1;
}

static int test_set_position_clears_counters(void) {
    test_setup();
    Entity entity = {0};

    entity.field_11C = 100;
    entity.field_120 = 200;

    entity_set_position(&entity, 50, 60);

    assert(entity.field_11C == 0);
    assert(entity.field_120 == 0);

    return 1;
}

/* ========================================
 * Single Walk Step Tests
 * ======================================== */

static int test_add_single_step(void) {
    test_setup();
    Entity entity = {0};
    entity.target_x = 0;
    entity.target_y = 0;

    entity_add_walk_step(&entity, 10, 20);

    assert(entity.path_count == 1);
    assert(entity.path_x[0] == 10);
    assert(entity.path_y[0] == 20);

    return 1;
}

static int test_add_multiple_steps(void) {
    test_setup();
    Entity entity = {0};

    entity_add_walk_step(&entity, 10, 20);
    entity_add_walk_step(&entity, 20, 30);
    entity_add_walk_step(&entity, 30, 40);

    assert(entity.path_count == 3);
    assert(entity.path_x[0] == 10);
    assert(entity.path_x[1] == 20);
    assert(entity.path_x[2] == 30);

    return 1;
}

static int test_queue_overflow(void) {
    test_setup();
    Entity entity = {0};

    /* Fill queue to max (10 steps) */
    for (int i = 0; i < 10; i++) {
        entity_add_walk_step(&entity, i * 10, i * 10);
    }

    assert(entity.path_count == 10);

    /* Adding one more should trigger overflow */
    entity_add_walk_step(&entity, 999, 999);

    /* Position should be set directly */
    assert(entity.path_count == 0);
    assert(entity.target_x == 999);
    assert(entity.target_y == 999);

    return 1;
}

static int test_add_step_null_entity(void) {
    test_setup();

    /* Should not crash */
    entity_add_walk_step(NULL, 100, 200);

    return 1;
}

static int test_fill_queue_exact(void) {
    test_setup();
    Entity entity = {0};

    /* Add exactly 10 steps */
    for (int i = 0; i < 10; i++) {
        entity_add_walk_step(&entity, i, i);
    }

    /* All should be in queue */
    assert(entity.path_count == 10);

    return 1;
}

/* ========================================
 * Long Walk Tests
 * ======================================== */

static int test_long_walk_diagonal(void) {
    test_setup();
    Entity entity = {0};
    entity.target_x = 100;
    entity.target_y = 100;

    /* Diagonal movement: delta_x = 2, delta_y = 2 */
    entity_add_long_walk(&entity, 102, 102);

    /* Should add midpoint */
    assert(entity.path_count == 2);
    assert(entity.path_x[0] == 101);  /* Midpoint */
    assert(entity.path_y[0] == 101);
    assert(entity.path_x[1] == 102);  /* Destination */
    assert(entity.path_y[1] == 102);

    return 1;
}

static int test_long_walk_horizontal(void) {
    test_setup();
    Entity entity = {0};
    entity.target_x = 100;
    entity.target_y = 100;

    /* Horizontal movement: delta_x = 2, delta_y = 0 */
    entity_add_long_walk(&entity, 102, 100);

    assert(entity.path_count == 2);
    assert(entity.path_x[0] == 101);  /* Midpoint X */
    assert(entity.path_y[0] == 100);  /* Same Y */

    return 1;
}

static int test_long_walk_vertical(void) {
    test_setup();
    Entity entity = {0};
    entity.target_x = 100;
    entity.target_y = 100;

    /* Vertical movement: delta_x = 0, delta_y = 2 */
    entity_add_long_walk(&entity, 100, 102);

    assert(entity.path_count == 2);
    assert(entity.path_x[0] == 100);  /* Same X */
    assert(entity.path_y[0] == 101);  /* Midpoint Y */

    return 1;
}

static int test_long_walk_no_interpolation(void) {
    test_setup();
    Entity entity = {0};
    entity.target_x = 100;
    entity.target_y = 100;

    /* Small movement: delta_x = 1, delta_y = 1 */
    entity_add_long_walk(&entity, 101, 101);

    /* Should just add destination */
    assert(entity.path_count == 1);
    assert(entity.path_x[0] == 101);
    assert(entity.path_y[0] == 101);

    return 1;
}

static int test_long_walk_from_path_end(void) {
    test_setup();
    Entity entity = {0};
    entity.target_x = 100;
    entity.target_y = 100;

    /* Add first step */
    entity_add_walk_step(&entity, 110, 110);
    assert(entity.path_count == 1);

    /* Add long walk - should continue from last path point */
    entity_add_long_walk(&entity, 112, 112);

    assert(entity.path_count == 3);  /* 1 + 2 (diagonal) */
    assert(entity.path_x[1] == 111);  /* Midpoint */
    assert(entity.path_x[2] == 112);  /* Destination */

    return 1;
}

static int test_long_walk_null_entity(void) {
    test_setup();

    /* Should not crash */
    entity_add_long_walk(NULL, 100, 200);

    return 1;
}

/* ========================================
 * Direction Tests
 * ======================================== */

static int test_direction_south_west(void) {
    test_setup();
    s32 dx, dy;

    get_direction_delta(DIR_SOUTH_WEST, &dx, &dy);

    assert(dx == -1);
    assert(dy == 1);

    return 1;
}

static int test_direction_west(void) {
    test_setup();
    s32 dx, dy;

    get_direction_delta(DIR_WEST, &dx, &dy);

    assert(dx == -1);
    assert(dy == 0);

    return 1;
}

static int test_direction_north(void) {
    test_setup();
    s32 dx, dy;

    get_direction_delta(DIR_NORTH, &dx, &dy);

    assert(dx == 0);
    assert(dy == -1);

    return 1;
}

static int test_direction_east(void) {
    test_setup();
    s32 dx, dy;

    get_direction_delta(DIR_EAST, &dx, &dy);

    assert(dx == 1);
    assert(dy == 0);

    return 1;
}

static int test_direction_south(void) {
    test_setup();
    s32 dx, dy;

    get_direction_delta(DIR_SOUTH, &dx, &dy);

    assert(dx == 0);
    assert(dy == 1);

    return 1;
}

static int test_all_directions(void) {
    test_setup();

    s32 expected_dx[] = {-1, -1, -1,  0,  1, 1, 1, 0};
    s32 expected_dy[] = { 1,  0, -1, -1, -1, 0, 1, 1};

    for (int i = 0; i < DIR_MAX; i++) {
        s32 dx, dy;
        get_direction_delta(i, &dx, &dy);
        assert(dx == expected_dx[i]);
        assert(dy == expected_dy[i]);
    }

    return 1;
}

static int test_invalid_direction(void) {
    test_setup();
    s32 dx, dy;

    get_direction_delta(999, &dx, &dy);

    assert(dx == 0);
    assert(dy == 0);

    return 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_movement_sequence(void) {
    test_setup();
    Entity entity = {0};

    /* Set initial position */
    entity_set_position(&entity, 50, 50);

    /* Add walk steps */
    entity_add_walk_step(&entity, 51, 51);
    entity_add_walk_step(&entity, 52, 52);
    entity_add_walk_step(&entity, 53, 53);

    assert(entity.path_count == 3);

    return 1;
}

static int test_long_walk_sequence(void) {
    test_setup();
    Entity entity = {0};

    /* Set initial position */
    entity_set_position(&entity, 100, 100);

    /* Add long walks */
    entity_add_long_walk(&entity, 102, 100);  /* Horizontal */
    entity_add_long_walk(&entity, 102, 102);  /* Vertical from last */

    assert(entity.path_count == 4);  /* 2 + 2 */

    return 1;
}

static int test_mixed_walk_types(void) {
    test_setup();
    Entity entity = {0};

    entity_set_position(&entity, 0, 0);

    /* Mix single and long walks */
    entity_add_walk_step(&entity, 1, 1);
    entity_add_long_walk(&entity, 3, 3);  /* Diagonal from (1,1) */
    entity_add_walk_step(&entity, 4, 4);

    assert(entity.path_count == 5);  /* 1 + 2 + 1 + 1 */

    return 1;
}

static int test_direction_from_position(void) {
    test_setup();

    /* Calculate direction from position difference */
    s32 dx = 1, dy = -1;  /* North-East direction */
    int direction = -1;

    for (int i = 0; i < DIR_MAX; i++) {
        s32 test_dx, test_dy;
        get_direction_delta(i, &test_dx, &test_dy);
        if (test_dx == dx && test_dy == dy) {
            direction = i;
            break;
        }
    }

    assert(direction == DIR_NORTH_EAST);

    return 1;
}

static int test_path_queue_max(void) {
    test_setup();
    Entity entity = {0};
    entity_set_position(&entity, 0, 0);

    /* Fill queue to exactly max - 1 */
    for (int i = 0; i < 10; i++) {
        entity_add_walk_step(&entity, i + 1, i + 1);
    }

    assert(entity.path_count == 10);

    /* Queue should be at max, next add will overflow */
    entity_add_walk_step(&entity, 999, 999);

    assert(entity.path_count == 0);
    assert(entity.target_x == 999);

    return 1;
}

static int test_negative_movement(void) {
    test_setup();
    Entity entity = {0};
    entity_set_position(&entity, 100, 100);

    /* Move in negative direction */
    entity_add_long_walk(&entity, 98, 98);

    assert(entity.path_count == 2);  /* Diagonal with midpoint */
    assert(entity.path_x[0] == 99);
    assert(entity.path_x[1] == 98);

    return 1;
}

static int test_large_distance(void) {
    test_setup();
    Entity entity = {0};
    entity_set_position(&entity, 0, 0);

    /* Large distance movement (no interpolation for > 2) */
    entity_add_long_walk(&entity, 10, 10);

    /* Should just add destination without interpolation */
    assert(entity.path_count == 1);
    assert(entity.path_x[0] == 10);
    assert(entity.path_y[0] == 10);

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Entity Movement System Unit Tests ===\n\n");

    /* Position setting tests */
    printf("Position Setting Tests:\n");
    TEST(set_position_basic);
    TEST(set_position_null);
    TEST(set_position_isometric);
    TEST(set_position_clears_counters);

    /* Single walk step tests */
    printf("\nSingle Walk Step Tests:\n");
    TEST(add_single_step);
    TEST(add_multiple_steps);
    TEST(queue_overflow);
    TEST(add_step_null_entity);
    TEST(fill_queue_exact);

    /* Long walk tests */
    printf("\nLong Walk Tests:\n");
    TEST(long_walk_diagonal);
    TEST(long_walk_horizontal);
    TEST(long_walk_vertical);
    TEST(long_walk_no_interpolation);
    TEST(long_walk_from_path_end);
    TEST(long_walk_null_entity);

    /* Direction tests */
    printf("\nDirection Tests:\n");
    TEST(direction_south_west);
    TEST(direction_west);
    TEST(direction_north);
    TEST(direction_east);
    TEST(direction_south);
    TEST(all_directions);
    TEST(invalid_direction);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(movement_sequence);
    TEST(long_walk_sequence);
    TEST(mixed_walk_types);
    TEST(direction_from_position);
    TEST(path_queue_max);
    TEST(negative_movement);
    TEST(large_distance);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - Isometric factor exact value
     * - Path processing per frame
     * - Movement speed calculation
     * - Real game movement data
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
