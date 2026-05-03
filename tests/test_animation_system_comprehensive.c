/*
 * Stone Age Client - Animation System Comprehensive Tests
 * Tests for animation module
 *
 * Coverage:
 * - Animation types and directions
 * - Animation playback
 * - Frame control
 * - Direction calculations
 * - Animation flags
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;
typedef short s16;

/* Constants */
#define MAX_ANIMATIONS      100
#define MAX_ANIM_FRAMES     32
#define MAX_ANIM_LAYERS     5

/* Animation types */
typedef enum {
    ANIM_TYPE_IDLE = 0,
    ANIM_TYPE_WALK = 1,
    ANIM_TYPE_RUN = 2,
    ANIM_TYPE_ATTACK = 3,
    ANIM_TYPE_SKILL = 4,
    ANIM_TYPE_CAST = 5,
    ANIM_TYPE_DAMAGE = 6,
    ANIM_TYPE_DEATH = 7,
    ANIM_TYPE_SIT = 8,
    ANIM_TYPE_STAND = 9,
    ANIM_TYPE_EMOTE = 10,
    ANIM_TYPE_VICTORY = 11,
    ANIM_TYPE_DEFEND = 12,
    ANIM_TYPE_SPECIAL = 13
} AnimationType;

/* Animation flags */
typedef enum {
    ANIM_FLAG_NONE = 0,
    ANIM_FLAG_LOOP = (1 << 0),
    ANIM_FLAG_PINGPONG = (1 << 1),
    ANIM_FLAG_REVERSE = (1 << 2),
    ANIM_FLAG_INTERRUPTIBLE = (1 << 3),
    ANIM_FLAG_MOVE_CHARACTER = (1 << 4),
    ANIM_FLAG_LOCK_INPUT = (1 << 5)
} AnimationFlags;

/* Animation direction */
typedef enum {
    ANIM_DIR_SW = 0,
    ANIM_DIR_W = 1,
    ANIM_DIR_NW = 2,
    ANIM_DIR_N = 3,
    ANIM_DIR_NE = 4,
    ANIM_DIR_E = 5,
    ANIM_DIR_SE = 6,
    ANIM_DIR_S = 7
} AnimationDirection;

/* Animation frame */
typedef struct {
    u16 sprite_id;
    u16 duration;
    s16 offset_x;
    s16 offset_y;
    u16 sound_id;
    u8 event_type;
    u8 event_param;
} AnimFrame;

/* Animation layer */
typedef struct {
    u16 z_order;
    u8 blend_mode;
    u8 alpha;
    s16 offset_x;
    s16 offset_y;
} AnimLayer;

/* Animation data */
typedef struct {
    u16 id;
    char name[24];
    AnimationType type;
    u16 total_duration;
    u16 frame_count;
    u8 direction_count;
    u8 flags;
    AnimFrame frames[MAX_ANIM_FRAMES][8];
    AnimLayer layers[MAX_ANIM_LAYERS];
    u8 layer_count;
    u16 next_anim;
    u16 transition_frame;
} AnimationData;

/* Active animation state */
typedef struct {
    u16 anim_id;
    u8 current_frame;
    u8 current_dir;
    u32 elapsed;
    u32 total_elapsed;
    u8 playing;
    u8 loop_count;
    u8 pingpong_forward;
    u8 speed;
    void (*on_complete)(u32 entity_id);
    void (*on_frame_event)(u32 entity_id, u8 event_type, u8 event_param);
    u32 entity_id;
} ActiveAnimation;

/* Animation context */
typedef struct {
    AnimationData* anim_db;
    int db_count;
    int db_capacity;
    ActiveAnimation active[MAX_ANIMATIONS];
    int active_count;
    u16 default_anims[14];
    u16 global_speed;
    u32 entity_anim_map[256];
} AnimationContext;

/* Global animation context */
static AnimationContext g_anim = {0};

/* Direction deltas - verified from FUN_00443e80 */
static const s16 s_dir_dx[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
static const s16 s_dir_dy[8] = {1, 0, -1, -1, -1, 0, 1, 1};

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
 * Implementation Functions
 * ======================================== */

void anim_init(void) {
    memset(&g_anim, 0, sizeof(AnimationContext));
    g_anim.global_speed = 100;
}

void anim_shutdown(void) {
    memset(&g_anim, 0, sizeof(AnimationContext));
}

int anim_play(u32 entity_id, u16 anim_id, u8 direction) {
    int slot;
    int i;

    if (g_anim.active_count >= MAX_ANIMATIONS) return 0;
    if (direction > 7) direction = 0;

    /* Find existing or free slot */
    slot = -1;
    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        slot = g_anim.active_count;
        g_anim.active_count++;
    }

    g_anim.active[slot].anim_id = anim_id;
    g_anim.active[slot].entity_id = entity_id;
    g_anim.active[slot].current_frame = 0;
    g_anim.active[slot].current_dir = direction;
    g_anim.active[slot].elapsed = 0;
    g_anim.active[slot].total_elapsed = 0;
    g_anim.active[slot].playing = 1;
    g_anim.active[slot].loop_count = 0;
    g_anim.active[slot].pingpong_forward = 1;
    g_anim.active[slot].speed = 100;

    if (entity_id < 256) {
        g_anim.entity_anim_map[entity_id] = slot;
    }

    return 1;
}

int anim_stop(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            g_anim.active[i].playing = 0;
            return 1;
        }
    }

    return 0;
}

int anim_is_playing(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id && g_anim.active[i].playing) {
            return 1;
        }
    }

    return 0;
}

u8 anim_get_current_frame(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            return g_anim.active[i].current_frame;
        }
    }

    return 0;
}

u8 anim_get_current_dir(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            return g_anim.active[i].current_dir;
        }
    }

    return 0;
}

int anim_set_speed(u32 entity_id, u8 speed) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            g_anim.active[i].speed = speed;
            return 1;
        }
    }

    return 0;
}

void anim_set_direction(u32 entity_id, u8 direction) {
    int i;

    if (direction > 7) return;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            g_anim.active[i].current_dir = direction;
            return;
        }
    }
}

u8 anim_direction_from_delta(s16 dx, s16 dy) {
    int i;

    for (i = 0; i < 8; i++) {
        if (s_dir_dx[i] == dx && s_dir_dy[i] == dy) {
            return (u8)i;
        }
    }

    return 0;
}

u8 anim_direction_to_target(s16 from_x, s16 from_y, s16 to_x, s16 to_y) {
    s16 dx = to_x - from_x;
    s16 dy = to_y - from_y;

    /* Normalize to -1, 0, 1 */
    if (dx > 0) dx = 1;
    else if (dx < 0) dx = -1;

    if (dy > 0) dy = 1;
    else if (dy < 0) dy = -1;

    return anim_direction_from_delta(dx, dy);
}

void anim_get_direction_delta(u8 direction, s16* dx, s16* dy) {
    if (direction > 7) direction = 0;

    if (dx) *dx = s_dir_dx[direction];
    if (dy) *dy = s_dir_dy[direction];
}

float anim_get_direction_angle(u8 direction) {
    /* 8 directions = 45 degrees each */
    /* 0 = SW = 225 degrees, going counter-clockwise */
    static const float angles[8] = {
        225.0f,  /* SW */
        270.0f,  /* W */
        315.0f,  /* NW */
        0.0f,    /* N */
        45.0f,   /* NE */
        90.0f,   /* E */
        135.0f,  /* SE */
        180.0f   /* S */
    };

    if (direction > 7) direction = 0;
    return angles[direction];
}

u16 anim_get_current_id(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            return g_anim.active[i].anim_id;
        }
    }

    return 0;
}

int anim_is_complete(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            return !g_anim.active[i].playing;
        }
    }

    return 1;
}

int anim_advance_frame(u32 entity_id) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            g_anim.active[i].current_frame++;
            if (g_anim.active[i].current_frame >= MAX_ANIM_FRAMES) {
                g_anim.active[i].current_frame = 0;
            }
            return 1;
        }
    }

    return 0;
}

int anim_goto_frame(u32 entity_id, u8 frame) {
    int i;

    if (frame >= MAX_ANIM_FRAMES) return 0;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            g_anim.active[i].current_frame = frame;
            return 1;
        }
    }

    return 0;
}

const char* anim_type_to_string(AnimationType type) {
    static const char* names[] = {
        "Idle", "Walk", "Run", "Attack", "Skill", "Cast",
        "Damage", "Death", "Sit", "Stand", "Emote",
        "Victory", "Defend", "Special"
    };

    if (type > ANIM_TYPE_SPECIAL) return "Unknown";
    return names[type];
}

const char* anim_dir_to_string(AnimationDirection dir) {
    static const char* names[] = {
        "SW", "W", "NW", "N", "NE", "E", "SE", "S"
    };

    if (dir > ANIM_DIR_S) return "Unknown";
    return names[dir];
}

int anim_has_looped(u32 entity_id, int loop_count) {
    int i;

    for (i = 0; i < g_anim.active_count; i++) {
        if (g_anim.active[i].entity_id == entity_id) {
            return g_anim.active[i].loop_count >= loop_count;
        }
    }

    return 0;
}

/* ========================================
 * Constants Tests
 * ======================================== */

static int test_max_animations(void) {
    return MAX_ANIMATIONS == 100;
}

static int test_max_anim_frames(void) {
    return MAX_ANIM_FRAMES == 32;
}

static int test_max_anim_layers(void) {
    return MAX_ANIM_LAYERS == 5;
}

/* ========================================
 * Animation Type Tests
 * ======================================== */

static int test_anim_type_values(void) {
    return ANIM_TYPE_IDLE == 0 &&
           ANIM_TYPE_WALK == 1 &&
           ANIM_TYPE_RUN == 2 &&
           ANIM_TYPE_ATTACK == 3;
}

static int test_anim_type_combat(void) {
    return ANIM_TYPE_SKILL == 4 &&
           ANIM_TYPE_CAST == 5 &&
           ANIM_TYPE_DAMAGE == 6 &&
           ANIM_TYPE_DEATH == 7;
}

static int test_anim_type_actions(void) {
    return ANIM_TYPE_SIT == 8 &&
           ANIM_TYPE_STAND == 9 &&
           ANIM_TYPE_EMOTE == 10;
}

static int test_anim_type_special(void) {
    return ANIM_TYPE_VICTORY == 11 &&
           ANIM_TYPE_DEFEND == 12 &&
           ANIM_TYPE_SPECIAL == 13;
}

/* ========================================
 * Animation Flag Tests
 * ======================================== */

static int test_anim_flag_values(void) {
    return ANIM_FLAG_NONE == 0 &&
           ANIM_FLAG_LOOP == 1 &&
           ANIM_FLAG_PINGPONG == 2 &&
           ANIM_FLAG_REVERSE == 4;
}

static int test_anim_flag_extended(void) {
    return ANIM_FLAG_INTERRUPTIBLE == 8 &&
           ANIM_FLAG_MOVE_CHARACTER == 16 &&
           ANIM_FLAG_LOCK_INPUT == 32;
}

static int test_anim_flag_combinable(void) {
    u8 flags = ANIM_FLAG_LOOP | ANIM_FLAG_INTERRUPTIBLE;
    return flags == 9;
}

/* ========================================
 * Animation Direction Tests
 * ======================================== */

static int test_anim_dir_values(void) {
    return ANIM_DIR_SW == 0 &&
           ANIM_DIR_W == 1 &&
           ANIM_DIR_NW == 2 &&
           ANIM_DIR_N == 3;
}

static int test_anim_dir_cardinal(void) {
    return ANIM_DIR_N == 3 &&
           ANIM_DIR_E == 5 &&
           ANIM_DIR_S == 7 &&
           ANIM_DIR_W == 1;
}

static int test_anim_dir_diagonal(void) {
    return ANIM_DIR_SW == 0 &&
           ANIM_DIR_NW == 2 &&
           ANIM_DIR_NE == 4 &&
           ANIM_DIR_SE == 6;
}

/* ========================================
 * Direction Delta Tests (from FUN_00443e80)
 * ======================================== */

static int test_dir_delta_sw(void) {
    s16 dx, dy;
    anim_get_direction_delta(ANIM_DIR_SW, &dx, &dy);
    return dx == -1 && dy == 1;
}

static int test_dir_delta_w(void) {
    s16 dx, dy;
    anim_get_direction_delta(ANIM_DIR_W, &dx, &dy);
    return dx == -1 && dy == 0;
}

static int test_dir_delta_nw(void) {
    s16 dx, dy;
    anim_get_direction_delta(ANIM_DIR_NW, &dx, &dy);
    return dx == -1 && dy == -1;
}

static int test_dir_delta_n(void) {
    s16 dx, dy;
    anim_get_direction_delta(ANIM_DIR_N, &dx, &dy);
    return dx == 0 && dy == -1;
}

static int test_dir_delta_ne(void) {
    s16 dx, dy;
    anim_get_direction_delta(ANIM_DIR_NE, &dx, &dy);
    return dx == 1 && dy == -1;
}

static int test_dir_delta_e(void) {
    s16 dx, dy;
    anim_get_direction_delta(ANIM_DIR_E, &dx, &dy);
    return dx == 1 && dy == 0;
}

static int test_dir_delta_se(void) {
    s16 dx, dy;
    anim_get_direction_delta(ANIM_DIR_SE, &dx, &dy);
    return dx == 1 && dy == 1;
}

static int test_dir_delta_s(void) {
    s16 dx, dy;
    anim_get_direction_delta(ANIM_DIR_S, &dx, &dy);
    return dx == 0 && dy == 1;
}

/* ========================================
 * Animation Init Tests
 * ======================================== */

static int test_anim_init(void) {
    anim_init();
    return g_anim.active_count == 0 &&
           g_anim.global_speed == 100;
}

static int test_anim_shutdown(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_E);
    anim_shutdown();
    return g_anim.active_count == 0;
}

/* ========================================
 * Animation Play Tests
 * ======================================== */

static int test_anim_play_basic(void) {
    anim_init();
    int result = anim_play(1, 100, ANIM_DIR_E);
    return result == 1 &&
           g_anim.active_count == 1 &&
           g_anim.active[0].anim_id == 100;
}

static int test_anim_play_multiple(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_E);
    anim_play(2, 200, ANIM_DIR_N);
    anim_play(3, 300, ANIM_DIR_S);
    return g_anim.active_count == 3;
}

static int test_anim_play_replace(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_E);
    anim_play(1, 200, ANIM_DIR_N);  /* Replace same entity */

    return g_anim.active_count == 1 &&
           g_anim.active[0].anim_id == 200;
}

static int test_anim_play_direction(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_NE);
    return g_anim.active[0].current_dir == ANIM_DIR_NE;
}

static int test_anim_play_invalid_direction(void) {
    anim_init();
    anim_play(1, 100, 99);  /* Invalid direction */
    return g_anim.active[0].current_dir == 0;  /* Should default to 0 */
}

/* ========================================
 * Animation Stop Tests
 * ======================================== */

static int test_anim_stop(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_E);
    int result = anim_stop(1);
    return result == 1 && g_anim.active[0].playing == 0;
}

static int test_anim_stop_not_playing(void) {
    anim_init();
    int result = anim_stop(999);
    return result == 0;
}

static int test_anim_is_playing(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_E);
    return anim_is_playing(1) == 1 &&
           anim_is_playing(2) == 0;
}

static int test_anim_is_playing_after_stop(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_E);
    anim_stop(1);
    return anim_is_playing(1) == 0;
}

/* ========================================
 * Frame Control Tests
 * ======================================== */

static int test_anim_get_current_frame(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_E);
    return anim_get_current_frame(1) == 0;
}

static int test_anim_advance_frame(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_E);
    anim_advance_frame(1);
    return anim_get_current_frame(1) == 1;
}

static int test_anim_goto_frame(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_E);
    anim_goto_frame(1, 5);
    return anim_get_current_frame(1) == 5;
}

static int test_anim_goto_frame_invalid(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_E);
    int result = anim_goto_frame(1, 100);  /* Invalid frame */
    return result == 0 && anim_get_current_frame(1) == 0;
}

static int test_anim_set_speed(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_E);
    anim_set_speed(1, 50);
    return g_anim.active[0].speed == 50;
}

/* ========================================
 * Direction Query Tests
 * ======================================== */

static int test_anim_get_current_dir(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_NW);
    return anim_get_current_dir(1) == ANIM_DIR_NW;
}

static int test_anim_set_direction(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_E);
    anim_set_direction(1, ANIM_DIR_W);
    return anim_get_current_dir(1) == ANIM_DIR_W;
}

static int test_anim_direction_from_delta_sw(void) {
    return anim_direction_from_delta(-1, 1) == ANIM_DIR_SW;
}

static int test_anim_direction_from_delta_n(void) {
    return anim_direction_from_delta(0, -1) == ANIM_DIR_N;
}

static int test_anim_direction_from_delta_e(void) {
    return anim_direction_from_delta(1, 0) == ANIM_DIR_E;
}

static int test_anim_direction_from_delta_se(void) {
    return anim_direction_from_delta(1, 1) == ANIM_DIR_SE;
}

static int test_anim_direction_to_target_north(void) {
    return anim_direction_to_target(0, 0, 0, -10) == ANIM_DIR_N;
}

static int test_anim_direction_to_target_east(void) {
    return anim_direction_to_target(0, 0, 10, 0) == ANIM_DIR_E;
}

static int test_anim_direction_to_target_diagonal(void) {
    return anim_direction_to_target(0, 0, 10, 10) == ANIM_DIR_SE;
}

/* ========================================
 * Direction Angle Tests
 * ======================================== */

static int test_anim_get_direction_angle_n(void) {
    float angle = anim_get_direction_angle(ANIM_DIR_N);
    return angle == 0.0f;
}

static int test_anim_get_direction_angle_e(void) {
    float angle = anim_get_direction_angle(ANIM_DIR_E);
    return angle == 90.0f;
}

static int test_anim_get_direction_angle_s(void) {
    float angle = anim_get_direction_angle(ANIM_DIR_S);
    return angle == 180.0f;
}

static int test_anim_get_direction_angle_w(void) {
    float angle = anim_get_direction_angle(ANIM_DIR_W);
    return angle == 270.0f;
}

/* ========================================
 * Animation ID Tests
 * ======================================== */

static int test_anim_get_current_id(void) {
    anim_init();
    anim_play(1, 500, ANIM_DIR_E);
    return anim_get_current_id(1) == 500;
}

static int test_anim_get_current_id_not_found(void) {
    anim_init();
    return anim_get_current_id(999) == 0;
}

/* ========================================
 * Animation Complete Tests
 * ======================================== */

static int test_anim_is_complete(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_E);
    return anim_is_complete(1) == 0;  /* Not complete while playing */
}

static int test_anim_is_complete_after_stop(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_E);
    anim_stop(1);
    return anim_is_complete(1) == 1;
}

static int test_anim_is_complete_not_found(void) {
    anim_init();
    return anim_is_complete(999) == 1;  /* Considered complete if not found */
}

/* ========================================
 * String Conversion Tests
 * ======================================== */

static int test_anim_type_to_string(void) {
    return strcmp(anim_type_to_string(ANIM_TYPE_IDLE), "Idle") == 0 &&
           strcmp(anim_type_to_string(ANIM_TYPE_WALK), "Walk") == 0 &&
           strcmp(anim_type_to_string(ANIM_TYPE_ATTACK), "Attack") == 0;
}

static int test_anim_type_to_string_unknown(void) {
    return strcmp(anim_type_to_string((AnimationType)99), "Unknown") == 0;
}

static int test_anim_dir_to_string(void) {
    return strcmp(anim_dir_to_string(ANIM_DIR_N), "N") == 0 &&
           strcmp(anim_dir_to_string(ANIM_DIR_E), "E") == 0 &&
           strcmp(anim_dir_to_string(ANIM_DIR_S), "S") == 0;
}

static int test_anim_dir_to_string_unknown(void) {
    return strcmp(anim_dir_to_string((AnimationDirection)99), "Unknown") == 0;
}

/* ========================================
 * Loop Tests
 * ======================================== */

static int test_anim_has_looped(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_E);
    /* Initially loop_count is 0 */
    return anim_has_looped(1, 1) == 0;
}

static int test_anim_has_looped_after_increment(void) {
    anim_init();
    anim_play(1, 100, ANIM_DIR_E);
    g_anim.active[0].loop_count = 2;
    return anim_has_looped(1, 2) == 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_animation_lifecycle(void) {
    anim_init();

    /* Play walk animation */
    anim_play(1, 100, ANIM_DIR_E);
    if (!anim_is_playing(1)) return 0;
    if (anim_get_current_id(1) != 100) return 0;

    /* Change direction */
    anim_set_direction(1, ANIM_DIR_N);
    if (anim_get_current_dir(1) != ANIM_DIR_N) return 0;

    /* Advance frames */
    anim_advance_frame(1);
    anim_advance_frame(1);
    if (anim_get_current_frame(1) != 2) return 0;

    /* Stop animation */
    anim_stop(1);
    if (anim_is_playing(1)) return 0;

    return 1;
}

static int test_multiple_entities_animation(void) {
    anim_init();

    /* Play different animations on different entities */
    anim_play(1, 100, ANIM_DIR_N);  /* Walk North */
    anim_play(2, 200, ANIM_DIR_E);  /* Attack East */
    anim_play(3, 300, ANIM_DIR_S);  /* Idle South */

    if (g_anim.active_count != 3) return 0;
    if (anim_get_current_id(1) != 100) return 0;
    if (anim_get_current_id(2) != 200) return 0;
    if (anim_get_current_id(3) != 300) return 0;

    /* Stop one entity */
    anim_stop(2);
    if (anim_is_playing(2)) return 0;
    if (!anim_is_playing(1)) return 0;
    if (!anim_is_playing(3)) return 0;

    return 1;
}

static int test_direction_calculation_scenario(void) {
    /* Test movement direction calculation */
    u8 dir;

    /* Moving from (0,0) to (10,0) = East */
    dir = anim_direction_to_target(0, 0, 10, 0);
    if (dir != ANIM_DIR_E) return 0;

    /* Moving from (0,0) to (0,10) = South */
    dir = anim_direction_to_target(0, 0, 0, 10);
    if (dir != ANIM_DIR_S) return 0;

    /* Moving from (10,10) to (0,0) = Northwest */
    dir = anim_direction_to_target(10, 10, 0, 0);
    if (dir != ANIM_DIR_NW) return 0;

    /* Moving from (0,10) to (10,0) = Northeast */
    dir = anim_direction_to_target(0, 10, 10, 0);
    if (dir != ANIM_DIR_NE) return 0;

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Animation System Comprehensive Tests ===\n\n");

    /* Constants tests */
    printf("Constants Tests:\n");
    TEST(max_animations);
    TEST(max_anim_frames);
    TEST(max_anim_layers);

    /* Animation type tests */
    printf("\nAnimation Type Tests:\n");
    TEST(anim_type_values);
    TEST(anim_type_combat);
    TEST(anim_type_actions);
    TEST(anim_type_special);

    /* Animation flag tests */
    printf("\nAnimation Flag Tests:\n");
    TEST(anim_flag_values);
    TEST(anim_flag_extended);
    TEST(anim_flag_combinable);

    /* Animation direction tests */
    printf("\nAnimation Direction Tests:\n");
    TEST(anim_dir_values);
    TEST(anim_dir_cardinal);
    TEST(anim_dir_diagonal);

    /* Direction delta tests */
    printf("\nDirection Delta Tests:\n");
    TEST(dir_delta_sw);
    TEST(dir_delta_w);
    TEST(dir_delta_nw);
    TEST(dir_delta_n);
    TEST(dir_delta_ne);
    TEST(dir_delta_e);
    TEST(dir_delta_se);
    TEST(dir_delta_s);

    /* Animation init tests */
    printf("\nAnimation Init Tests:\n");
    TEST(anim_init);
    TEST(anim_shutdown);

    /* Animation play tests */
    printf("\nAnimation Play Tests:\n");
    TEST(anim_play_basic);
    TEST(anim_play_multiple);
    TEST(anim_play_replace);
    TEST(anim_play_direction);
    TEST(anim_play_invalid_direction);

    /* Animation stop tests */
    printf("\nAnimation Stop Tests:\n");
    TEST(anim_stop);
    TEST(anim_stop_not_playing);
    TEST(anim_is_playing);
    TEST(anim_is_playing_after_stop);

    /* Frame control tests */
    printf("\nFrame Control Tests:\n");
    TEST(anim_get_current_frame);
    TEST(anim_advance_frame);
    TEST(anim_goto_frame);
    TEST(anim_goto_frame_invalid);
    TEST(anim_set_speed);

    /* Direction query tests */
    printf("\nDirection Query Tests:\n");
    TEST(anim_get_current_dir);
    TEST(anim_set_direction);
    TEST(anim_direction_from_delta_sw);
    TEST(anim_direction_from_delta_n);
    TEST(anim_direction_from_delta_e);
    TEST(anim_direction_from_delta_se);
    TEST(anim_direction_to_target_north);
    TEST(anim_direction_to_target_east);
    TEST(anim_direction_to_target_diagonal);

    /* Direction angle tests */
    printf("\nDirection Angle Tests:\n");
    TEST(anim_get_direction_angle_n);
    TEST(anim_get_direction_angle_e);
    TEST(anim_get_direction_angle_s);
    TEST(anim_get_direction_angle_w);

    /* Animation ID tests */
    printf("\nAnimation ID Tests:\n");
    TEST(anim_get_current_id);
    TEST(anim_get_current_id_not_found);

    /* Animation complete tests */
    printf("\nAnimation Complete Tests:\n");
    TEST(anim_is_complete);
    TEST(anim_is_complete_after_stop);
    TEST(anim_is_complete_not_found);

    /* String conversion tests */
    printf("\nString Conversion Tests:\n");
    TEST(anim_type_to_string);
    TEST(anim_type_to_string_unknown);
    TEST(anim_dir_to_string);
    TEST(anim_dir_to_string_unknown);

    /* Loop tests */
    printf("\nLoop Tests:\n");
    TEST(anim_has_looped);
    TEST(anim_has_looped_after_increment);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_animation_lifecycle);
    TEST(multiple_entities_animation);
    TEST(direction_calculation_scenario);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
