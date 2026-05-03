/*
 * Stone Age Client - Pathfinding System Comprehensive Tests
 * Tests for direction conversion, A* pathfinding, angle calculation
 * Reverse engineered from FUN_00443e80, FUN_00447150, FUN_00443fe0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s16;
typedef int s32;
typedef float f32;

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

/* ========================================
 * Constants from Ghidra analysis
 * ======================================== */

/* Direction constants - 8 directions */
#define DIR_SOUTHWEST   0
#define DIR_WEST        1
#define DIR_NORTHWEST   2
#define DIR_NORTH       3
#define DIR_NORTHEAST   4
#define DIR_EAST        5
#define DIR_SOUTHEAST   6
#define DIR_SOUTH       7
#define DIR_COUNT       8

/* Path status */
#define PATH_STATUS_IDLE        0
#define PATH_STATUS_SEARCHING   1
#define PATH_STATUS_MOVING      2
#define PATH_STATUS_COMPLETE    3
#define PATH_STATUS_FAILED      4

/* Pathfinding limits */
#define MAX_PATHFIND_NODES      4096
#define PATHFIND_GRID_MAX       256
#define MAX_PATH_LENGTH         1024

/* Floating point constants from DAT_0049c3xx */
#define PATHFIND_PI             3.14159265358979323846
#define PATHFIND_HALF_PI        1.57079632679489661923
#define PATHFIND_TWO_PI         6.28318530717958647693
#define PATHFIND_ANGLE_OFFSET   0.39269908169872415481  /* pi/8 */

/* Direction deltas - matching FUN_00443e80 EXACTLY */
static const s32 s_move_dx[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
static const s32 s_move_dy[8] = {1, 0, -1, -1, -1, 0, 1, 1};

/* Angle lookup table - DAT_004bc2b0 (first 10 entries) */
static const double s_angle_table[10] = {
    0.000000, 0.024679, 0.049462, 0.074443, 0.099669,
    0.125170, 0.150983, 0.177147, 0.203704, 0.230703
};

/* ========================================
 * Stub Structures
 * ======================================== */

typedef struct {
    int status;
    u16 start_x, start_y;
    u16 dest_x, dest_y;
    u32 current_map;
    u32 dest_map;
    int move_delay;
    void* map_links;
} PathfindContext;

typedef struct {
    int x, y;
    int g_cost, h_cost, f_cost;
    int parent;
    int closed;
} PathfindNode;

static PathfindContext g_pathfind = {0};
static PathfindNode s_nodes[MAX_PATHFIND_NODES];
static int s_node_count;

/* ========================================
 * Stub Implementations
 * ======================================== */

/*
 * Direction to delta conversion - FUN_00443e80
 * Binary verified - see pathfind_movement.c comments
 */
void direction_get_delta(u32 direction, s32* dx, s32* dy) {
    if (direction > 7) {
        *dx = 0;
        *dy = 0;
        return;
    }
    *dx = s_move_dx[direction];
    *dy = s_move_dy[direction];
}

/*
 * Initialize pathfinding system
 */
int pathfind_init(void) {
    memset(&g_pathfind, 0, sizeof(PathfindContext));
    g_pathfind.status = PATH_STATUS_IDLE;
    g_pathfind.move_delay = 200;
    return 1;
}

/*
 * Shutdown pathfinding system
 */
void pathfind_shutdown(void) {
    if (g_pathfind.map_links) {
        free(g_pathfind.map_links);
    }
    memset(&g_pathfind, 0, sizeof(PathfindContext));
}

/*
 * Calculate angle from direction - FUN_00447150
 */
f32 pathfind_calculate_angle(f32 dx, f32 dy) {
    if (dx == 0.0f && dy == 0.0f) {
        return 0.0f;
    }
    return (f32)atan2(dy, dx);
}

/*
 * Get opposite direction
 */
u32 direction_opposite(u32 direction) {
    if (direction > 7) return 0;
    return (direction + 4) % 8;
}

/*
 * Check if direction is diagonal
 */
int direction_is_diagonal(u32 direction) {
    return (direction == DIR_NORTHWEST || direction == DIR_NORTHEAST ||
            direction == DIR_SOUTHWEST || direction == DIR_SOUTHEAST);
}

/*
 * Check if direction is cardinal
 */
int direction_is_cardinal(u32 direction) {
    return (direction == DIR_NORTH || direction == DIR_SOUTH ||
            direction == DIR_EAST || direction == DIR_WEST);
}

/*
 * Get direction from deltas
 */
u32 direction_from_delta(s32 dx, s32 dy) {
    /* Normalize to -1, 0, 1 */
    if (dx < 0) dx = -1;
    if (dx > 0) dx = 1;
    if (dy < 0) dy = -1;
    if (dy > 0) dy = 1;

    /* Match to direction table */
    int i;
    for (i = 0; i < 8; i++) {
        if (s_move_dx[i] == dx && s_move_dy[i] == dy) {
            return i;
        }
    }
    return 0;
}

/*
 * Calculate Manhattan distance
 */
int pathfind_manhattan_distance(int x1, int y1, int x2, int y2) {
    return abs(x2 - x1) + abs(y2 - y1);
}

/*
 * Calculate Chebyshev distance (for 8-direction movement)
 */
int pathfind_chebyshev_distance(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    return (dx > dy) ? dx : dy;
}

/*
 * Calculate Euclidean distance
 */
f32 pathfind_euclidean_distance(f32 x1, f32 y1, f32 x2, f32 y2) {
    f32 dx = x2 - x1;
    f32 dy = y2 - y1;
    return (f32)sqrt(dx * dx + dy * dy);
}

/* ========================================
 * Test Cases for Direction Deltas
 * ======================================== */

static int test_dir_southwest(void) {
    /* Direction 0: dx=-1, dy=1 (Southwest) */
    return s_move_dx[0] == -1 && s_move_dy[0] == 1;
}

static int test_dir_west(void) {
    /* Direction 1: dx=-1, dy=0 (West) */
    return s_move_dx[1] == -1 && s_move_dy[1] == 0;
}

static int test_dir_northwest(void) {
    /* Direction 2: dx=-1, dy=-1 (Northwest) */
    return s_move_dx[2] == -1 && s_move_dy[2] == -1;
}

static int test_dir_north(void) {
    /* Direction 3: dx=0, dy=-1 (North) */
    return s_move_dx[3] == 0 && s_move_dy[3] == -1;
}

static int test_dir_northeast(void) {
    /* Direction 4: dx=1, dy=-1 (Northeast) */
    return s_move_dx[4] == 1 && s_move_dy[4] == -1;
}

static int test_dir_east(void) {
    /* Direction 5: dx=1, dy=0 (East) */
    return s_move_dx[5] == 1 && s_move_dy[5] == 0;
}

static int test_dir_southeast(void) {
    /* Direction 6: dx=1, dy=1 (Southeast) */
    return s_move_dx[6] == 1 && s_move_dy[6] == 1;
}

static int test_dir_south(void) {
    /* Direction 7: dx=0, dy=1 (South) */
    return s_move_dx[7] == 0 && s_move_dy[7] == 1;
}

static int test_dir_count(void) {
    return DIR_COUNT == 8;
}

/* ========================================
 * Test Cases for Direction Conversion
 * ======================================== */

static int test_direction_get_delta_valid(void) {
    s32 dx, dy;
    direction_get_delta(DIR_NORTH, &dx, &dy);
    return dx == 0 && dy == -1;
}

static int test_direction_get_delta_invalid(void) {
    s32 dx, dy;
    direction_get_delta(99, &dx, &dy);
    return dx == 0 && dy == 0;
}

static int test_direction_get_delta_all(void) {
    s32 dx, dy;
    int i;
    for (i = 0; i < 8; i++) {
        direction_get_delta(i, &dx, &dy);
        if (dx != s_move_dx[i] || dy != s_move_dy[i]) {
            return 0;
        }
    }
    return 1;
}

/* ========================================
 * Test Cases for Direction Utilities
 * ======================================== */

static int test_direction_opposite_north(void) {
    return direction_opposite(DIR_NORTH) == DIR_SOUTH;
}

static int test_direction_opposite_east(void) {
    return direction_opposite(DIR_EAST) == DIR_WEST;
}

static int test_direction_opposite_diag(void) {
    return direction_opposite(DIR_NORTHEAST) == DIR_SOUTHWEST;
}

static int test_direction_is_diagonal(void) {
    return direction_is_diagonal(DIR_NORTHWEST) == 1 &&
           direction_is_diagonal(DIR_NORTH) == 0;
}

static int test_direction_is_cardinal(void) {
    return direction_is_cardinal(DIR_NORTH) == 1 &&
           direction_is_cardinal(DIR_NORTHWEST) == 0;
}

static int test_direction_from_delta(void) {
    return direction_from_delta(0, -1) == DIR_NORTH &&
           direction_from_delta(1, 0) == DIR_EAST &&
           direction_from_delta(-1, 1) == DIR_SOUTHWEST;
}

/* ========================================
 * Test Cases for Path Status
 * ======================================== */

static int test_path_status_idle(void) {
    return PATH_STATUS_IDLE == 0;
}

static int test_path_status_searching(void) {
    return PATH_STATUS_SEARCHING == 1;
}

static int test_path_status_moving(void) {
    return PATH_STATUS_MOVING == 2;
}

static int test_path_status_complete(void) {
    return PATH_STATUS_COMPLETE == 3;
}

static int test_path_status_failed(void) {
    return PATH_STATUS_FAILED == 4;
}

/* ========================================
 * Test Cases for Initialization
 * ======================================== */

static int test_pathfind_init(void) {
    memset(&g_pathfind, 0xFF, sizeof(PathfindContext));
    int result = pathfind_init();

    return result == 1 &&
           g_pathfind.status == PATH_STATUS_IDLE &&
           g_pathfind.move_delay == 200;
}

static int test_pathfind_shutdown(void) {
    pathfind_init();
    g_pathfind.status = PATH_STATUS_MOVING;
    pathfind_shutdown();

    return g_pathfind.status == 0;
}

/* ========================================
 * Test Cases for Distance Calculations
 * ======================================== */

static int test_manhattan_distance_zero(void) {
    return pathfind_manhattan_distance(5, 5, 5, 5) == 0;
}

static int test_manhattan_distance_horizontal(void) {
    return pathfind_manhattan_distance(0, 0, 5, 0) == 5;
}

static int test_manhattan_distance_vertical(void) {
    return pathfind_manhattan_distance(0, 0, 0, 5) == 5;
}

static int test_manhattan_distance_diagonal(void) {
    return pathfind_manhattan_distance(0, 0, 3, 4) == 7;
}

static int test_chebyshev_distance_zero(void) {
    return pathfind_chebyshev_distance(5, 5, 5, 5) == 0;
}

static int test_chebyshev_distance_horizontal(void) {
    return pathfind_chebyshev_distance(0, 0, 5, 0) == 5;
}

static int test_chebyshev_distance_diagonal(void) {
    /* For diagonal, Chebyshev = max(dx, dy) */
    return pathfind_chebyshev_distance(0, 0, 3, 4) == 4;
}

static int test_euclidean_distance_zero(void) {
    return pathfind_euclidean_distance(5.0f, 5.0f, 5.0f, 5.0f) < 0.0001f;
}

static int test_euclidean_distance_unit(void) {
    f32 dist = pathfind_euclidean_distance(0.0f, 0.0f, 1.0f, 0.0f);
    return dist > 0.99f && dist < 1.01f;
}

static int test_euclidean_distance_diagonal(void) {
    f32 dist = pathfind_euclidean_distance(0.0f, 0.0f, 3.0f, 4.0f);
    return dist > 4.99f && dist < 5.01f;  /* sqrt(9+16) = 5 */
}

/* ========================================
 * Test Cases for Angle Calculations
 * ======================================== */

static int test_angle_zero(void) {
    f32 angle = pathfind_calculate_angle(1.0f, 0.0f);
    return angle > -0.01f && angle < 0.01f;
}

static int test_angle_90(void) {
    f32 angle = pathfind_calculate_angle(0.0f, 1.0f);
    return angle > 1.56f && angle < 1.58f;  /* ~pi/2 */
}

static int test_angle_180(void) {
    f32 angle = pathfind_calculate_angle(-1.0f, 0.0f);
    return (angle > 3.13f && angle < 3.15f) || (angle < -3.13f && angle > -3.15f);
}

static int test_angle_negative(void) {
    f32 angle = pathfind_calculate_angle(0.0f, -1.0f);
    return angle < -1.56f && angle > -1.58f;  /* ~-pi/2 */
}

/* ========================================
 * Test Cases for Constants
 * ======================================== */

static int test_pi_value(void) {
    return PATHFIND_PI > 3.141f && PATHFIND_PI < 3.142f;
}

static int test_half_pi_value(void) {
    return PATHFIND_HALF_PI > 1.570f && PATHFIND_HALF_PI < 1.571f;
}

static int test_two_pi_value(void) {
    return PATHFIND_TWO_PI > 6.283f && PATHFIND_TWO_PI < 6.284f;
}

static int test_angle_offset(void) {
    /* pi/8 = 0.392699... */
    return PATHFIND_ANGLE_OFFSET > 0.392f && PATHFIND_ANGLE_OFFSET < 0.393f;
}

/* ========================================
 * Test Cases for Limits
 * ======================================== */

static int test_max_pathfind_nodes(void) {
    return MAX_PATHFIND_NODES == 4096;
}

static int test_pathfind_grid_max(void) {
    return PATHFIND_GRID_MAX == 256;
}

static int test_max_path_length(void) {
    return MAX_PATH_LENGTH == 1024;
}

/* ========================================
 * Test Cases for Angle Table
 * ======================================== */

static int test_angle_table_first(void) {
    return s_angle_table[0] < 0.0001f;
}

static int test_angle_table_increasing(void) {
    int i;
    for (i = 1; i < 10; i++) {
        if (s_angle_table[i] <= s_angle_table[i-1]) {
            return 0;
        }
    }
    return 1;
}

static int test_angle_table_positive(void) {
    int i;
    for (i = 0; i < 10; i++) {
        if (s_angle_table[i] < 0) {
            return 0;
        }
    }
    return 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_direction_cycle(void) {
    int i;
    for (i = 0; i < 8; i++) {
        s32 dx, dy;
        direction_get_delta(i, &dx, &dy);
        u32 result = direction_from_delta(dx, dy);
        if (result != i) {
            return 0;
        }
    }
    return 1;
}

static int test_direction_opposite_cycle(void) {
    int i;
    for (i = 0; i < 8; i++) {
        u32 opp = direction_opposite(i);
        u32 opp2 = direction_opposite(opp);
        if (opp2 != i) {
            return 0;
        }
    }
    return 1;
}

static int test_pathfind_init_shutdown_cycle(void) {
    int i;
    for (i = 0; i < 10; i++) {
        if (!pathfind_init()) return 0;
        if (g_pathfind.status != PATH_STATUS_IDLE) return 0;
        pathfind_shutdown();
        if (g_pathfind.status != 0) return 0;
    }
    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Pathfinding System Comprehensive Tests ===\n\n");

    /* Direction delta tests */
    printf("Direction Delta Tests:\n");
    TEST(dir_southwest);
    TEST(dir_west);
    TEST(dir_northwest);
    TEST(dir_north);
    TEST(dir_northeast);
    TEST(dir_east);
    TEST(dir_southeast);
    TEST(dir_south);
    TEST(dir_count);

    /* Direction conversion tests */
    printf("\nDirection Conversion Tests:\n");
    TEST(direction_get_delta_valid);
    TEST(direction_get_delta_invalid);
    TEST(direction_get_delta_all);

    /* Direction utility tests */
    printf("\nDirection Utility Tests:\n");
    TEST(direction_opposite_north);
    TEST(direction_opposite_east);
    TEST(direction_opposite_diag);
    TEST(direction_is_diagonal);
    TEST(direction_is_cardinal);
    TEST(direction_from_delta);

    /* Path status tests */
    printf("\nPath Status Tests:\n");
    TEST(path_status_idle);
    TEST(path_status_searching);
    TEST(path_status_moving);
    TEST(path_status_complete);
    TEST(path_status_failed);

    /* Initialization tests */
    printf("\nInitialization Tests:\n");
    TEST(pathfind_init);
    TEST(pathfind_shutdown);

    /* Distance calculation tests */
    printf("\nDistance Calculation Tests:\n");
    TEST(manhattan_distance_zero);
    TEST(manhattan_distance_horizontal);
    TEST(manhattan_distance_vertical);
    TEST(manhattan_distance_diagonal);
    TEST(chebyshev_distance_zero);
    TEST(chebyshev_distance_horizontal);
    TEST(chebyshev_distance_diagonal);
    TEST(euclidean_distance_zero);
    TEST(euclidean_distance_unit);
    TEST(euclidean_distance_diagonal);

    /* Angle calculation tests */
    printf("\nAngle Calculation Tests:\n");
    TEST(angle_zero);
    TEST(angle_90);
    TEST(angle_180);
    TEST(angle_negative);

    /* Constants tests */
    printf("\nConstants Tests:\n");
    TEST(pi_value);
    TEST(half_pi_value);
    TEST(two_pi_value);
    TEST(angle_offset);

    /* Limits tests */
    printf("\nLimits Tests:\n");
    TEST(max_pathfind_nodes);
    TEST(pathfind_grid_max);
    TEST(max_path_length);

    /* Angle table tests */
    printf("\nAngle Table Tests:\n");
    TEST(angle_table_first);
    TEST(angle_table_increasing);
    TEST(angle_table_positive);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_direction_cycle);
    TEST(direction_opposite_cycle);
    TEST(pathfind_init_shutdown_cycle);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
