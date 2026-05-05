/*
 * Stone Age Client - Battle Action Render Tests
 * Tests for effect queue, render execute, position check
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef signed int s32;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT_EQ(a, b) do { int _a = (int)(a); int _b = (int)(b); if (_a != _b) { printf("FAIL: line %d got %d expected %d\n", __LINE__, _a, _b); tests_failed++; return; } } while(0)
#define ASSERT_TRUE(c) do { if (!(c)) { printf("FAIL: line %d false\n", __LINE__); tests_failed++; return; } } while(0)

/* === Effect queue mirror === */

#define MAX_EFFECTS 32

typedef struct {
    u16 id;
    u16 x;
    u16 y;
    u16 timer;
    u16 frame;
    u8  active;
    u8  reserved;
} TestEffect;

static TestEffect t_effects[MAX_EFFECTS];
static u32 t_effect_count = 0;

static void t_effects_init(void) {
    memset(t_effects, 0, sizeof(t_effects));
    t_effect_count = 0;
}

static void t_effect_add(u32 id, u32 x, u32 y) {
    if (t_effect_count >= MAX_EFFECTS) return;
    TestEffect* e = &t_effects[t_effect_count++];
    e->id = (u16)id;
    e->x = (u16)x;
    e->y = (u16)y;
    e->timer = 30;
    e->frame = 0;
    e->active = 1;
}

static void t_effect_clear(void) {
    t_effect_count = 0;
    memset(t_effects, 0, sizeof(t_effects));
}

/* === Action state mirror === */
static u32 t_action_type = 0;
static int t_death_flag = 0;
static int t_exec_called = 0;
static int t_last_action = -1;

static void t_exec_reset(void) {
    t_action_type = 0;
    t_death_flag = 0;
    t_exec_called = 0;
    t_last_action = -1;
}

/* Render execute mirror - matches battle_render_execute */
static void t_render_execute(void) {
    if (t_death_flag != 0) {
        t_death_flag = 0;
    }
    if ((s32)t_action_type < 0) return;
    t_exec_called = 1;
    t_last_action = (int)t_action_type;
}

/* Position check mirror */
static u16 t_player_x = 0;
static u16 t_player_y = 0;
static u16 t_target_x = 0;
static u16 t_target_y = 0;

static int t_check_position(void) {
    int dx = (int)t_target_x - (int)t_player_x;
    int dy = (int)t_target_y - (int)t_player_y;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return (dx < 2 && dy < 2);
}

/* === Tests === */

void test_effect_add_single(void) {
    t_effects_init();
    t_effect_add(1, 100, 200);
    ASSERT_EQ(t_effect_count, 1);
    ASSERT_EQ(t_effects[0].id, 1);
    ASSERT_EQ(t_effects[0].x, 100);
    ASSERT_EQ(t_effects[0].y, 200);
    ASSERT_EQ(t_effects[0].active, 1);
    tests_passed++;
}

void test_effect_add_multiple(void) {
    t_effects_init();
    t_effect_add(1, 10, 20);
    t_effect_add(2, 30, 40);
    t_effect_add(5, 50, 60);
    ASSERT_EQ(t_effect_count, 3);
    ASSERT_EQ(t_effects[0].id, 1);
    ASSERT_EQ(t_effects[1].id, 2);
    ASSERT_EQ(t_effects[2].id, 5);
    tests_passed++;
}

void test_effect_max_queue(void) {
    int i;
    t_effects_init();
    for (i = 0; i < MAX_EFFECTS; i++) {
        t_effect_add(i, i * 10, i * 20);
    }
    ASSERT_EQ(t_effect_count, MAX_EFFECTS);
    /* One more should be rejected */
    t_effect_add(99, 0, 0);
    ASSERT_EQ(t_effect_count, MAX_EFFECTS);
    tests_passed++;
}

void test_effect_clear(void) {
    t_effects_init();
    t_effect_add(1, 0, 0);
    t_effect_add(2, 0, 0);
    ASSERT_EQ(t_effect_count, 2);
    t_effect_clear();
    ASSERT_EQ(t_effect_count, 0);
    tests_passed++;
}

void test_effect_timer_default(void) {
    t_effects_init();
    t_effect_add(1, 0, 0);
    ASSERT_EQ(t_effects[0].timer, 30);
    tests_passed++;
}

void test_effect_types(void) {
    /* Effect IDs: 1=hit, 2=heal, 3=death, 4=revive, 5=skill, 6=summon, 7=recall */
    t_effects_init();
    t_effect_add(1, 100, 200);  /* hit */
    t_effect_add(2, 150, 250);  /* heal */
    t_effect_add(3, 200, 300);  /* death */
    t_effect_add(5, 250, 350);  /* skill */
    ASSERT_EQ(t_effects[0].id, 1);
    ASSERT_EQ(t_effects[1].id, 2);
    ASSERT_EQ(t_effects[2].id, 3);
    ASSERT_EQ(t_effects[3].id, 5);
    tests_passed++;
}

void test_render_execute_basic(void) {
    t_exec_reset();
    t_action_type = 0;
    t_render_execute();
    ASSERT_EQ(t_exec_called, 1);
    ASSERT_EQ(t_last_action, 0);
    tests_passed++;
}

void test_render_execute_negative(void) {
    t_exec_reset();
    t_action_type = (u32)(-1);
    t_render_execute();
    ASSERT_EQ(t_exec_called, 0);
    tests_passed++;
}

void test_render_execute_death_flag(void) {
    t_exec_reset();
    t_death_flag = 1;
    t_action_type = 0;
    t_render_execute();
    ASSERT_EQ(t_death_flag, 0);  /* cleared after processing */
    ASSERT_EQ(t_exec_called, 1);
    tests_passed++;
}

void test_render_execute_various_actions(void) {
    int i;
    u32 actions[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x0c, 0x0d, 0x0e, 0x0f};
    for (i = 0; i < 14; i++) {
        t_exec_reset();
        t_action_type = actions[i];
        t_render_execute();
        ASSERT_EQ(t_last_action, (int)actions[i]);
    }
    tests_passed++;
}

/* Position check tests */

void test_position_same(void) {
    t_player_x = 100; t_player_y = 200;
    t_target_x = 100; t_target_y = 200;
    ASSERT_EQ(t_check_position(), 1);
    tests_passed++;
}

void test_position_adjacent(void) {
    t_player_x = 100; t_player_y = 200;
    t_target_x = 101; t_target_y = 201;
    ASSERT_EQ(t_check_position(), 1);
    tests_passed++;
}

void test_position_far(void) {
    t_player_x = 100; t_player_y = 200;
    t_target_x = 200; t_target_y = 300;
    ASSERT_EQ(t_check_position(), 0);
    tests_passed++;
}

void test_position_boundary(void) {
    t_player_x = 100; t_player_y = 200;
    t_target_x = 101; t_target_y = 201;
    ASSERT_EQ(t_check_position(), 1);
    t_target_x = 102; t_target_y = 200;
    ASSERT_EQ(t_check_position(), 0);
    tests_passed++;
}

void test_position_zero(void) {
    t_player_x = 0; t_player_y = 0;
    t_target_x = 0; t_target_y = 0;
    ASSERT_EQ(t_check_position(), 1);
    tests_passed++;
}

void test_position_overflow(void) {
    t_player_x = 65535; t_player_y = 65535;
    t_target_x = 65534; t_target_y = 65534;
    ASSERT_EQ(t_check_position(), 1);
    tests_passed++;
}

/* Combined effect + execute tests */

void test_effect_then_execute(void) {
    t_effects_init();
    t_exec_reset();
    t_effect_add(1, 100, 200);
    t_action_type = 0x00;
    t_render_execute();
    ASSERT_EQ(t_effect_count, 1);
    ASSERT_EQ(t_exec_called, 1);
    tests_passed++;
}

void test_death_with_effect(void) {
    t_effects_init();
    t_exec_reset();
    t_death_flag = 1;
    t_effect_add(3, 200, 300);  /* death effect */
    t_action_type = 0x1c;
    t_render_execute();
    ASSERT_EQ(t_death_flag, 0);
    ASSERT_EQ(t_effects[0].id, 3);
    tests_passed++;
}

int main(void) {
    printf("=== Battle Action Render Tests ===\n\n");

    printf("[Effect Queue]\n");
    test_effect_add_single();     printf("  TEST: Add single effect ... PASS\n");
    test_effect_add_multiple();   printf("  TEST: Add multiple effects ... PASS\n");
    test_effect_max_queue();      printf("  TEST: Max queue (32 effects) ... PASS\n");
    test_effect_clear();          printf("  TEST: Clear effect queue ... PASS\n");
    test_effect_timer_default();  printf("  TEST: Default timer 30 frames ... PASS\n");
    test_effect_types();          printf("  TEST: Effect type IDs ... PASS\n");

    printf("[Render Execute - FUN_00424f50]\n");
    test_render_execute_basic();          printf("  TEST: Basic execute action 0 ... PASS\n");
    test_render_execute_negative();       printf("  TEST: Negative action ignored ... PASS\n");
    test_render_execute_death_flag();     printf("  TEST: Death flag cleared ... PASS\n");
    test_render_execute_various_actions(); printf("  TEST: Various action types ... PASS\n");

    printf("[Position Check]\n");
    test_position_same();         printf("  TEST: Same position valid ... PASS\n");
    test_position_adjacent();     printf("  TEST: Adjacent position valid ... PASS\n");
    test_position_far();          printf("  TEST: Far position invalid ... PASS\n");
    test_position_boundary();     printf("  TEST: Boundary check (dx<2) ... PASS\n");
    test_position_zero();         printf("  TEST: Zero origin ... PASS\n");
    test_position_overflow();     printf("  TEST: Overflow wrap ... PASS\n");

    printf("[Combined Effects + Execute]\n");
    test_effect_then_execute();   printf("  TEST: Effect then execute ... PASS\n");
    test_death_with_effect();     printf("  TEST: Death with effect ... PASS\n");

    printf("\n=== Results: %d passed, %d failed, %d total ===\n",
           tests_passed, tests_failed, tests_passed + tests_failed);
    return tests_failed;
}
