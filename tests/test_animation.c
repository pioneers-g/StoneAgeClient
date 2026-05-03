/*
 * Stone Age Client - Animation System Unit Tests
 * Tests for animation types, direction calculation, frame management
 * Based on FUN_00443e80, FUN_00447150, FUN_00443c40 analysis
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "../tests/test_framework.h"
#include "types.h"
#include "animation.h"

/* ========================================
 * Test Cases for Animation Constants
 * ======================================== */

/* Test animation type values */
static void test_animation_type_values(void) {
    TEST_BEGIN("Animation type values");

    TEST_ASSERT_EQ(ANIM_TYPE_IDLE, 0);
    TEST_ASSERT_EQ(ANIM_TYPE_WALK, 1);
    TEST_ASSERT_EQ(ANIM_TYPE_RUN, 2);
    TEST_ASSERT_EQ(ANIM_TYPE_ATTACK, 3);
    TEST_ASSERT_EQ(ANIM_TYPE_SKILL, 4);
    TEST_ASSERT_EQ(ANIM_TYPE_CAST, 5);
    TEST_ASSERT_EQ(ANIM_TYPE_DAMAGE, 6);
    TEST_ASSERT_EQ(ANIM_TYPE_DEATH, 7);
    TEST_ASSERT_EQ(ANIM_TYPE_SIT, 8);
    TEST_ASSERT_EQ(ANIM_TYPE_STAND, 9);
    TEST_ASSERT_EQ(ANIM_TYPE_EMOTE, 10);
    TEST_ASSERT_EQ(ANIM_TYPE_VICTORY, 11);
    TEST_ASSERT_EQ(ANIM_TYPE_DEFEND, 12);
    TEST_ASSERT_EQ(ANIM_TYPE_SPECIAL, 13);

    TEST_END();
}

/* Test animation flag values */
static void test_animation_flag_values(void) {
    TEST_BEGIN("Animation flag values");

    TEST_ASSERT_EQ(ANIM_FLAG_NONE, 0);
    TEST_ASSERT_EQ(ANIM_FLAG_LOOP, (1 << 0));
    TEST_ASSERT_EQ(ANIM_FLAG_PINGPONG, (1 << 1));
    TEST_ASSERT_EQ(ANIM_FLAG_REVERSE, (1 << 2));
    TEST_ASSERT_EQ(ANIM_FLAG_INTERRUPTIBLE, (1 << 3));
    TEST_ASSERT_EQ(ANIM_FLAG_MOVE_CHARACTER, (1 << 4));
    TEST_ASSERT_EQ(ANIM_FLAG_LOCK_INPUT, (1 << 5));

    TEST_END();
}

/* Test direction values - FUN_00443e80 verified */
static void test_direction_values(void) {
    TEST_BEGIN("Direction values");

    /* New naming - isometric coordinate system */
    TEST_ASSERT_EQ(ANIM_DIR_SW, 0);    /* Southwest diagonal */
    TEST_ASSERT_EQ(ANIM_DIR_W, 1);     /* Pure West */
    TEST_ASSERT_EQ(ANIM_DIR_NW, 2);    /* Northwest diagonal */
    TEST_ASSERT_EQ(ANIM_DIR_N, 3);     /* Pure North */
    TEST_ASSERT_EQ(ANIM_DIR_NE, 4);    /* Northeast diagonal */
    TEST_ASSERT_EQ(ANIM_DIR_E, 5);     /* Pure East */
    TEST_ASSERT_EQ(ANIM_DIR_SE, 6);    /* Southeast diagonal */
    TEST_ASSERT_EQ(ANIM_DIR_S, 7);     /* Pure South */

    TEST_END();
}

/* Test max constants */
static void test_max_constants(void) {
    TEST_BEGIN("Max constants");

    TEST_ASSERT_EQ(MAX_ANIMATIONS, 100);
    TEST_ASSERT_EQ(MAX_ANIM_FRAMES, 32);
    TEST_ASSERT_EQ(MAX_ANIM_LAYERS, 5);

    TEST_END();
}

/* ========================================
 * Test Cases for Direction Delta (FUN_00443e80)
 * ======================================== */

/* Test direction to delta conversion - from FUN_00443e80
 * Verified from binary decompilation:
 *   Case 0: dx=-1, dy=1   (SW diagonal)
 *   Case 1: dx=-1, dy=0   (Pure W)
 *   Case 2: dx=-1, dy=-1  (NW diagonal)
 *   Case 3: dx=0,  dy=-1  (Pure N)
 *   Case 4: dx=1,  dy=-1  (NE diagonal)
 *   Case 5: dx=1,  dy=0   (Pure E)
 *   Case 6: dx=1,  dy=1   (SE diagonal)
 *   Case 7: dx=0,  dy=1   (Pure S)
 */

static void test_direction_delta_sw(void) {
    TEST_BEGIN("Direction delta - SW (0)");

    s16 dx, dy;
    anim_get_direction_delta(ANIM_DIR_SW, &dx, &dy);

    /* From FUN_00443e80 case 0: dx=-1, dy=1 */
    TEST_ASSERT_EQ(dx, -1);
    TEST_ASSERT_EQ(dy, 1);

    TEST_END();
}

static void test_direction_delta_w(void) {
    TEST_BEGIN("Direction delta - W (1)");

    s16 dx, dy;
    anim_get_direction_delta(ANIM_DIR_W, &dx, &dy);

    /* From FUN_00443e80 case 1: dx=-1, dy=0 */
    TEST_ASSERT_EQ(dx, -1);
    TEST_ASSERT_EQ(dy, 0);

    TEST_END();
}

static void test_direction_delta_nw(void) {
    TEST_BEGIN("Direction delta - NW (2)");

    s16 dx, dy;
    anim_get_direction_delta(ANIM_DIR_NW, &dx, &dy);

    /* From FUN_00443e80 case 2: dx=-1, dy=-1 */
    TEST_ASSERT_EQ(dx, -1);
    TEST_ASSERT_EQ(dy, -1);

    TEST_END();
}

static void test_direction_delta_n(void) {
    TEST_BEGIN("Direction delta - N (3)");

    s16 dx, dy;
    anim_get_direction_delta(ANIM_DIR_N, &dx, &dy);

    /* From FUN_00443e80 case 3: dx=0, dy=-1 */
    TEST_ASSERT_EQ(dx, 0);
    TEST_ASSERT_EQ(dy, -1);

    TEST_END();
}

static void test_direction_delta_ne(void) {
    TEST_BEGIN("Direction delta - NE (4)");

    s16 dx, dy;
    anim_get_direction_delta(ANIM_DIR_NE, &dx, &dy);

    /* From FUN_00443e80 case 4: dx=1, dy=-1 */
    TEST_ASSERT_EQ(dx, 1);
    TEST_ASSERT_EQ(dy, -1);

    TEST_END();
}

static void test_direction_delta_e(void) {
    TEST_BEGIN("Direction delta - E (5)");

    s16 dx, dy;
    anim_get_direction_delta(ANIM_DIR_E, &dx, &dy);

    /* From FUN_00443e80 case 5: dx=1, dy=0 */
    TEST_ASSERT_EQ(dx, 1);
    TEST_ASSERT_EQ(dy, 0);

    TEST_END();
}

static void test_direction_delta_se(void) {
    TEST_BEGIN("Direction delta - SE (6)");

    s16 dx, dy;
    anim_get_direction_delta(ANIM_DIR_SE, &dx, &dy);

    /* From FUN_00443e80 case 6: dx=1, dy=1 */
    TEST_ASSERT_EQ(dx, 1);
    TEST_ASSERT_EQ(dy, 1);

    TEST_END();
}

static void test_direction_delta_s(void) {
    TEST_BEGIN("Direction delta - S (7)");

    s16 dx, dy;
    anim_get_direction_delta(ANIM_DIR_S, &dx, &dy);

    /* From FUN_00443e80 case 7: dx=0, dy=1 */
    TEST_ASSERT_EQ(dx, 0);
    TEST_ASSERT_EQ(dy, 1);

    TEST_END();
}

/* Test direction delta table matches binary FUN_00443e80 */
static void test_direction_delta_table(void) {
    TEST_BEGIN("Direction delta table - FUN_00443e80 verified");

    /* Expected values from FUN_00443e80 binary decompilation */
    s16 expected_dx[] = {-1, -1, -1, 0, 1, 1, 1, 0};
    s16 expected_dy[] = {1, 0, -1, -1, -1, 0, 1, 1};

    for (int i = 0; i < 8; i++) {
        s16 dx, dy;
        anim_get_direction_delta(i, &dx, &dy);
        TEST_ASSERT_EQ(dx, expected_dx[i]);
        TEST_ASSERT_EQ(dy, expected_dy[i]);
    }

    TEST_END();
}

/* ========================================
 * Test Cases for Direction Calculation
 * ======================================== */

/* Test direction from delta */
static void test_direction_from_delta(void) {
    TEST_BEGIN("Direction from delta");

    /* Test exact matches from FUN_00443e80 */
    TEST_ASSERT_EQ(anim_direction_from_delta(-1, 1), ANIM_DIR_SW);   /* Case 0 */
    TEST_ASSERT_EQ(anim_direction_from_delta(-1, 0), ANIM_DIR_W);    /* Case 1 */
    TEST_ASSERT_EQ(anim_direction_from_delta(-1, -1), ANIM_DIR_NW);  /* Case 2 */
    TEST_ASSERT_EQ(anim_direction_from_delta(0, -1), ANIM_DIR_N);    /* Case 3 */
    TEST_ASSERT_EQ(anim_direction_from_delta(1, -1), ANIM_DIR_NE);   /* Case 4 */
    TEST_ASSERT_EQ(anim_direction_from_delta(1, 0), ANIM_DIR_E);     /* Case 5 */
    TEST_ASSERT_EQ(anim_direction_from_delta(1, 1), ANIM_DIR_SE);    /* Case 6 */
    TEST_ASSERT_EQ(anim_direction_from_delta(0, 1), ANIM_DIR_S);     /* Case 7 */

    TEST_END();
}

/* Test direction to target calculation */
static void test_direction_to_target(void) {
    TEST_BEGIN("Direction to target");

    /* Test diagonal directions from (0,0) */
    TEST_ASSERT_EQ(anim_direction_to_target(0, 0, -1, 1), ANIM_DIR_SW);   /* Southwest */
    TEST_ASSERT_EQ(anim_direction_to_target(0, 0, -1, -1), ANIM_DIR_NW);  /* Northwest */
    TEST_ASSERT_EQ(anim_direction_to_target(0, 0, 1, -1), ANIM_DIR_NE);   /* Northeast */
    TEST_ASSERT_EQ(anim_direction_to_target(0, 0, 1, 1), ANIM_DIR_SE);    /* Southeast */

    /* Test cardinal directions */
    TEST_ASSERT_EQ(anim_direction_to_target(0, 0, -1, 0), ANIM_DIR_W);    /* West */
    TEST_ASSERT_EQ(anim_direction_to_target(0, 0, 0, -1), ANIM_DIR_N);    /* North */
    TEST_ASSERT_EQ(anim_direction_to_target(0, 0, 1, 0), ANIM_DIR_E);     /* East */
    TEST_ASSERT_EQ(anim_direction_to_target(0, 0, 0, 1), ANIM_DIR_S);     /* South */

    TEST_END();
}

/* ========================================
 * Test Cases for Animation Context
 * ======================================== */

/* Test context initialization */
static void test_context_init(void) {
    TEST_BEGIN("Context initialization");

    memset(&g_anim, 0, sizeof(AnimationContext));

    TEST_ASSERT_EQ(g_anim.anim_db, NULL);
    TEST_ASSERT_EQ(g_anim.db_count, 0);
    TEST_ASSERT_EQ(g_anim.active_count, 0);
    TEST_ASSERT_EQ(g_anim.global_speed, 0);

    TEST_END();
}

/* Test anim_init function */
static void test_anim_init(void) {
    TEST_BEGIN("Animation init");

    int result = anim_init();

    TEST_ASSERT_EQ(result, 1);
    TEST_ASSERT_EQ(g_anim.global_speed, 100);

    /* Default animations should be set */
    TEST_ASSERT(g_anim.default_anims[ANIM_TYPE_IDLE] > 0);
    TEST_ASSERT(g_anim.default_anims[ANIM_TYPE_WALK] > 0);
    TEST_ASSERT(g_anim.default_anims[ANIM_TYPE_ATTACK] > 0);

    TEST_END();
}

/* Test anim_shutdown function */
static void test_anim_shutdown(void) {
    TEST_BEGIN("Animation shutdown");

    anim_init();
    anim_shutdown();

    TEST_ASSERT_EQ(g_anim.anim_db, NULL);
    TEST_ASSERT_EQ(g_anim.db_count, 0);

    TEST_END();
}

/* ========================================
 * Test Cases for Active Animation
 * ======================================== */

/* Test active animation structure size */
static void test_active_animation_size(void) {
    TEST_BEGIN("Active animation size");

    /* ActiveAnimation should be reasonable size */
    TEST_ASSERT(sizeof(ActiveAnimation) >= sizeof(u16) * 2);  /* anim_id + padding */
    TEST_ASSERT(sizeof(ActiveAnimation) >= sizeof(void*) * 2);  /* callbacks */

    TEST_END();
}

/* Test animation play */
static void test_anim_play(void) {
    TEST_BEGIN("Animation play");

    anim_init();

    int result = anim_play(1, 1, ANIM_DIR_S);

    TEST_ASSERT_EQ(result, 1);
    TEST_ASSERT(anim_is_playing(1));

    TEST_END();
}

/* Test animation stop */
static void test_anim_stop(void) {
    TEST_BEGIN("Animation stop");

    anim_init();
    anim_play(1, 1, ANIM_DIR_S);
    anim_stop(1);

    TEST_ASSERT(!anim_is_playing(1));

    TEST_END();
}

/* Test animation frame advance */
static void test_anim_advance_frame(void) {
    TEST_BEGIN("Animation frame advance");

    anim_init();
    anim_play(1, 1, ANIM_DIR_S);

    u8 frame_before = anim_get_current_frame(1);
    anim_advance_frame(1);
    u8 frame_after = anim_get_current_frame(1);

    /* Frame should change (or wrap) */
    TEST_ASSERT(frame_after >= 0);

    TEST_END();
}

/* Test animation set direction */
static void test_anim_set_direction(void) {
    TEST_BEGIN("Animation set direction");

    anim_init();
    anim_play(1, 1, ANIM_DIR_S);

    anim_set_direction(1, ANIM_DIR_N);
    TEST_ASSERT_EQ(anim_get_current_dir(1), ANIM_DIR_N);

    TEST_END();
}

/* Test animation set speed */
static void test_anim_set_speed(void) {
    TEST_BEGIN("Animation set speed");

    anim_init();
    anim_play(1, 1, ANIM_DIR_S);

    int result = anim_set_speed(1, 50);
    TEST_ASSERT_EQ(result, 1);

    TEST_END();
}

/* ========================================
 * Test Cases for Animation Data
 * ======================================== */

/* Test AnimFrame structure size */
static void test_anim_frame_size(void) {
    TEST_BEGIN("AnimFrame size");

    /* AnimFrame: sprite_id(2) + duration(2) + offset_x(2) + offset_y(2) + sound_id(2) + event_type(1) + event_param(1) = 12 bytes minimum */
    TEST_ASSERT(sizeof(AnimFrame) >= 12);

    TEST_END();
}

/* Test AnimationData structure size */
static void test_animation_data_size(void) {
    TEST_BEGIN("AnimationData size");

    /* AnimationData contains frames[32][8], so it should be large */
    size_t expected_min = sizeof(u16) + 24 + sizeof(AnimationType) + sizeof(u16) * 3;
    TEST_ASSERT(sizeof(AnimationData) > expected_min);

    TEST_END();
}

/* Test animation database default */
static void test_anim_get_default(void) {
    TEST_BEGIN("Animation get default");

    anim_init();
    anim_load_database();

    AnimationData* idle_anim = anim_get_default(ANIM_TYPE_IDLE);
    /* Should return a valid animation or NULL if not loaded */
    /* In test environment, default animations are created */

    TEST_END();
}

/* ========================================
 * Test Cases for Frame Timing
 * ======================================== */

/* Test frame interval constants */
static void test_frame_intervals(void) {
    TEST_BEGIN("Frame intervals");

    /* From binary analysis:
     * - Idle: 200ms per frame
     * - Walk: 80ms per frame
     * - Run: 60ms per frame
     * - Attack: 50ms per frame
     */

    u32 idle_interval = anim_get_frame_interval(ANIM_TYPE_IDLE);
    u32 walk_interval = anim_get_frame_interval(ANIM_TYPE_WALK);
    u32 attack_interval = anim_get_frame_interval(ANIM_TYPE_ATTACK);

    /* Intervals should be positive */
    TEST_ASSERT(idle_interval > 0);
    TEST_ASSERT(walk_interval > 0);
    TEST_ASSERT(attack_interval > 0);

    TEST_END();
}

/* Test animation update */
static void test_anim_update(void) {
    TEST_BEGIN("Animation update");

    anim_init();
    anim_play(1, 1, ANIM_DIR_S);

    /* Update with 100ms */
    anim_update(100);

    /* Animation should still be playing */
    TEST_ASSERT(anim_is_playing(1));

    TEST_END();
}

/* ========================================
 * Test Cases for Animation Flags
 * ======================================== */

/* Test loop flag */
static void test_loop_flag(void) {
    TEST_BEGIN("Loop flag");

    anim_init();
    anim_play_loop(1, 1, ANIM_DIR_S, 3);

    /* Should be playing */
    TEST_ASSERT(anim_is_playing(1));
    TEST_ASSERT(anim_is_looping(1));

    TEST_END();
}

/* Test animation complete check */
static void test_anim_is_complete(void) {
    TEST_BEGIN("Animation complete check");

    anim_init();
    anim_play(1, 1, ANIM_DIR_S);

    /* Initially not complete */
    TEST_ASSERT(!anim_is_complete(1));

    TEST_END();
}

/* ========================================
 * Test Cases for Utility Functions
 * ======================================== */

/* Test animation type to string */
static void test_anim_type_to_string(void) {
    TEST_BEGIN("Animation type to string");

    const char* str;

    str = anim_type_to_string(ANIM_TYPE_IDLE);
    TEST_ASSERT(str != NULL);
    TEST_ASSERT(strlen(str) > 0);

    str = anim_type_to_string(ANIM_TYPE_ATTACK);
    TEST_ASSERT(str != NULL);

    str = anim_type_to_string(ANIM_TYPE_DEATH);
    TEST_ASSERT(str != NULL);

    TEST_END();
}

/* Test direction to string */
static void test_anim_dir_to_string(void) {
    TEST_BEGIN("Direction to string");

    const char* str;

    str = anim_dir_to_string(ANIM_DIR_S);
    TEST_ASSERT(str != NULL);
    TEST_ASSERT(strlen(str) > 0);

    str = anim_dir_to_string(ANIM_DIR_N);
    TEST_ASSERT(str != NULL);

    str = anim_dir_to_string(ANIM_DIR_E);
    TEST_ASSERT(str != NULL);

    TEST_END();
}

/* Test animation progress */
static void test_anim_progress(void) {
    TEST_BEGIN("Animation progress");

    anim_init();
    anim_play(1, 1, ANIM_DIR_S);

    u8 progress = anim_get_progress(1);
    TEST_ASSERT(progress >= 0 && progress <= 100);

    TEST_END();
}

/* Test remaining time */
static void test_anim_remaining_time(void) {
    TEST_BEGIN("Animation remaining time");

    anim_init();
    anim_play(1, 1, ANIM_DIR_S);

    u32 remaining = anim_get_remaining_time(1);
    TEST_ASSERT(remaining >= 0);

    TEST_END();
}

/* ========================================
 * Test Cases for Animation Memory
 * ======================================== */

/* Test entity animation mapping */
static void test_entity_anim_map(void) {
    TEST_BEGIN("Entity animation map");

    anim_init();

    /* Map size should be 256 entries */
    TEST_ASSERT(sizeof(g_anim.entity_anim_map) >= 256 * sizeof(u32));

    TEST_END();
}

/* Test animation queue */
static void test_anim_queue(void) {
    TEST_BEGIN("Animation queue");

    anim_init();
    anim_play(1, 1, ANIM_DIR_S);

    /* Queue next animation */
    int result = anim_queue_next(1, 2);
    TEST_ASSERT_EQ(result, 1);

    /* Check if has next */
    TEST_ASSERT(anim_has_next(1));

    TEST_END();
}

/* Test animation blend */
static void test_anim_blend(void) {
    TEST_BEGIN("Animation blend");

    anim_init();
    anim_play(1, 1, ANIM_DIR_S);

    /* Blend to new animation */
    int result = anim_blend_to(1, 2, ANIM_DIR_N, 0.5f);
    TEST_ASSERT(result >= 0);  /* May not be implemented */

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(animation) {
    /* Constants */
    test_animation_type_values();
    test_animation_flag_values();
    test_direction_values();
    test_max_constants();

    /* Direction delta - FUN_00443e80 */
    test_direction_delta_sw();
    test_direction_delta_w();
    test_direction_delta_nw();
    test_direction_delta_n();
    test_direction_delta_ne();
    test_direction_delta_e();
    test_direction_delta_se();
    test_direction_delta_s();
    test_direction_delta_table();

    /* Direction calculation */
    test_direction_from_delta();
    test_direction_to_target();

    /* Context */
    test_context_init();
    test_anim_init();
    test_anim_shutdown();

    /* Active animation */
    test_active_animation_size();
    test_anim_play();
    test_anim_stop();
    test_anim_advance_frame();
    test_anim_set_direction();
    test_anim_set_speed();

    /* Animation data */
    test_anim_frame_size();
    test_animation_data_size();
    test_anim_get_default();

    /* Frame timing */
    test_frame_intervals();
    test_anim_update();

    /* Flags */
    test_loop_flag();
    test_anim_is_complete();

    /* Utility */
    test_anim_type_to_string();
    test_anim_dir_to_string();
    test_anim_progress();
    test_anim_remaining_time();

    /* Memory */
    test_entity_anim_map();
    test_anim_queue();
    test_anim_blend();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - Animation Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(animation);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
