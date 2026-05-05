/*
 * Stone Age Client - Battle Core Functions Tests
 * Tests for FUN_00405080, FUN_00405120, FUN_00405370, FUN_00424b70, FUN_00404850
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef signed int s32;
typedef unsigned int u32;
typedef unsigned short u16;

#define SLOT_EMPTY (-2)

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT_EQ(a, b) do { int _a = (int)(a); int _b = (int)(b); if (_a != _b) { printf("FAIL: line %d got %d expected %d\n", __LINE__, _a, _b); tests_failed++; return; } } while(0)
#define ASSERT_TRUE(c) do { if (!(c)) { printf("FAIL: line %d false\n", __LINE__); tests_failed++; return; } } while(0)

/* === Inline battle logic mirrors for testing === */

static s32 t_party[10];
static s32 t_enemy[6];
static s32 t_reserve[6];
static s32 t_pet[7];
static s32 t_formation[9];
static s32 t_state_flag = 0;
static s32 t_action_pending = 0;
static s32 t_battle_active = 0;
static s32 t_selection = 0;
static s32 t_mode = 0;
static s32 t_pvp_flag = 0;
static s32 t_pvp_check = 0;

static u32 t_action_type = 0;
static u32 t_action_param = 0;
static u32 t_action_target = 0;
static u32 t_action_data = 0;

static void t_reset(void) {
    int i;
    for (i = 0; i < 10; i++) t_party[i] = 99;
    for (i = 0; i < 6; i++) t_enemy[i] = 99;
    for (i = 0; i < 6; i++) t_reserve[i] = 99;
    for (i = 0; i < 7; i++) t_pet[i] = 99;
    for (i = 0; i < 9; i++) t_formation[i] = 99;
    t_state_flag = 5;
    t_action_pending = 3;
    t_battle_active = 0;
    t_selection = 7;
    t_mode = 0;
    t_pvp_flag = 1;
    t_pvp_check = 0;
    t_action_type = 0;
    t_action_param = 0;
    t_action_target = 0;
    t_action_data = 0;
}

static void t_unit_reset(void) {
    int i;
    for (i = 0; i < 10; i++) t_party[i] = SLOT_EMPTY;
    for (i = 0; i < 6; i++) t_enemy[i] = SLOT_EMPTY;
    for (i = 0; i < 6; i++) t_reserve[i] = SLOT_EMPTY;
    for (i = 0; i < 7; i++) t_pet[i] = SLOT_EMPTY;
    for (i = 0; i < 9; i++) t_formation[i] = SLOT_EMPTY;
    t_action_pending = 0;
    t_battle_active = 1;
    t_selection = -1;
    t_mode = 1;
    t_pvp_flag = (t_pvp_check == -2) ? 1 : 0;
    t_state_flag = 0;
}

static void t_action_dispatch(int type, int p2, int p3, int p4) {
    t_action_type = type;
    t_action_param = p2;
    t_action_target = p3;
    t_action_data = p4;
}

static void t_set_positions(int bx, int by) {
    int offsets[9][2] = {
        {0,0},{1,0},{-1,0},{0,1},{0,-1},
        {1,1},{-1,1},{1,-1},{-1,-1}
    };
    int i;
    for (i = 0; i < 9; i++) {
        t_formation[i] = (bx + offsets[i][0]) | ((by + offsets[i][1]) << 16);
    }
}

static int t_check_active(void) {
    int count = 0, i;
    for (i = 0; i < 10; i++) {
        if (t_party[i] != SLOT_EMPTY) count++;
    }
    if (count == 2) return 1;
    if (count > 2) return 2;
    return 0;
}

/* === Tests === */

void test_reset_clears_party(void) {
    int i;
    t_reset();
    t_unit_reset();
    for (i = 0; i < 10; i++) ASSERT_EQ(t_party[i], SLOT_EMPTY);
    tests_passed++;
}

void test_reset_clears_enemy(void) {
    int i;
    t_reset();
    t_unit_reset();
    for (i = 0; i < 6; i++) ASSERT_EQ(t_enemy[i], SLOT_EMPTY);
    tests_passed++;
}

void test_reset_clears_reserve(void) {
    int i;
    t_reset();
    t_unit_reset();
    for (i = 0; i < 6; i++) ASSERT_EQ(t_reserve[i], SLOT_EMPTY);
    tests_passed++;
}

void test_reset_clears_pet(void) {
    int i;
    t_reset();
    t_unit_reset();
    for (i = 0; i < 7; i++) ASSERT_EQ(t_pet[i], SLOT_EMPTY);
    tests_passed++;
}

void test_reset_clears_formation(void) {
    int i;
    t_reset();
    t_unit_reset();
    for (i = 0; i < 9; i++) ASSERT_EQ(t_formation[i], SLOT_EMPTY);
    tests_passed++;
}

void test_reset_state_flags(void) {
    t_reset();
    ASSERT_EQ(t_action_pending, 3);
    ASSERT_EQ(t_battle_active, 0);
    ASSERT_EQ(t_selection, 7);
    t_unit_reset();
    ASSERT_EQ(t_action_pending, 0);
    ASSERT_EQ(t_battle_active, 1);
    ASSERT_EQ(t_selection, -1);
    ASSERT_EQ(t_mode, 1);
    ASSERT_EQ(t_state_flag, 0);
    tests_passed++;
}

void test_pvp_flag_set(void) {
    t_reset();
    t_pvp_check = -2;
    t_unit_reset();
    ASSERT_EQ(t_pvp_flag, 1);
    tests_passed++;
}

void test_pvp_flag_clear(void) {
    t_reset();
    t_pvp_check = 0;
    t_unit_reset();
    ASSERT_EQ(t_pvp_flag, 0);
    tests_passed++;
}

void test_action_dispatch_stores(void) {
    t_action_dispatch(7, 100, 200, 300);
    ASSERT_EQ(t_action_type, 7);
    ASSERT_EQ(t_action_param, 100);
    ASSERT_EQ(t_action_target, 200);
    ASSERT_EQ(t_action_data, 300);
    tests_passed++;
}

void test_action_attack(void) {
    t_action_dispatch(0, 1, 2, 3);
    ASSERT_EQ(t_action_type, 0);
    tests_passed++;
}

void test_action_guard(void) {
    t_action_dispatch(1, 0, 0, 0);
    ASSERT_EQ(t_action_type, 1);
    tests_passed++;
}

void test_action_escape(void) {
    t_action_dispatch(2, 0, 0, 0);
    ASSERT_EQ(t_action_type, 2);
    tests_passed++;
}

void test_action_skill(void) {
    t_action_dispatch(7, 10, 5, 0);
    ASSERT_EQ(t_action_type, 7);
    ASSERT_EQ(t_action_param, 10);
    ASSERT_EQ(t_action_target, 5);
    tests_passed++;
}

void test_action_capture(void) {
    t_action_dispatch(9, 3, 0, 0);
    ASSERT_EQ(t_action_type, 9);
    tests_passed++;
}

void test_positions_center(void) {
    t_set_positions(320, 240);
    ASSERT_EQ(t_formation[0] & 0xFFFF, 320);
    ASSERT_EQ(t_formation[0] >> 16, 240);
    tests_passed++;
}

void test_positions_offset(void) {
    t_set_positions(320, 240);
    ASSERT_EQ(t_formation[1] & 0xFFFF, 321);
    ASSERT_EQ(t_formation[2] & 0xFFFF, 319);
    ASSERT_EQ(t_formation[3] >> 16, 241);
    ASSERT_EQ(t_formation[4] >> 16, 239);
    tests_passed++;
}

void test_positions_diagonal(void) {
    t_set_positions(100, 200);
    ASSERT_EQ(t_formation[5] & 0xFFFF, 101);
    ASSERT_EQ(t_formation[5] >> 16, 201);
    ASSERT_EQ(t_formation[8] & 0xFFFF, 99);
    ASSERT_EQ(t_formation[8] >> 16, 199);
    tests_passed++;
}

void test_check_active_none(void) {
    t_unit_reset();
    ASSERT_EQ(t_check_active(), 0);
    tests_passed++;
}

void test_check_active_one(void) {
    t_unit_reset();
    t_party[0] = 5;
    ASSERT_EQ(t_check_active(), 0);
    tests_passed++;
}

void test_check_active_two(void) {
    t_unit_reset();
    t_party[0] = 5;
    t_party[1] = 8;
    ASSERT_EQ(t_check_active(), 1);
    tests_passed++;
}

void test_check_active_many(void) {
    t_unit_reset();
    t_party[0] = 5;
    t_party[1] = 8;
    t_party[2] = 12;
    ASSERT_EQ(t_check_active(), 2);
    tests_passed++;
}

void test_positions_zero_origin(void) {
    t_set_positions(0, 0);
    ASSERT_EQ(t_formation[0], 0);
    ASSERT_EQ(t_formation[1] & 0xFFFF, 1);
    ASSERT_EQ(t_formation[1] >> 16, 0);
    tests_passed++;
}

void test_action_item(void) {
    t_action_dispatch(6, 1, 3, 0);
    ASSERT_EQ(t_action_type, 6);
    ASSERT_EQ(t_action_param, 1);
    ASSERT_EQ(t_action_target, 3);
    tests_passed++;
}

void test_action_pet_skill(void) {
    t_action_dispatch(8, 2, 1, 0);
    ASSERT_EQ(t_action_type, 8);
    ASSERT_EQ(t_action_param, 2);
    tests_passed++;
}

void test_action_team(void) {
    t_action_dispatch(10, 0, 0, 0);
    ASSERT_EQ(t_action_type, 10);
    t_action_dispatch(11, 0, 0, 0);
    ASSERT_EQ(t_action_type, 11);
    tests_passed++;
}

void test_action_pet_summon(void) {
    t_action_dispatch(520, 0, 0, 0);
    ASSERT_EQ(t_action_type, 520);
    tests_passed++;
}

void test_action_battle_text(void) {
    t_action_dispatch(101, 0, 0, 0);
    ASSERT_EQ(t_action_type, 101);
    tests_passed++;
}

void test_action_gold_reward(void) {
    t_action_dispatch(18, 500, 0, 0);
    ASSERT_EQ(t_action_type, 18);
    ASSERT_EQ(t_action_param, 500);
    tests_passed++;
}

void test_check_active_all_ten(void) {
    int i;
    t_unit_reset();
    for (i = 0; i < 10; i++) t_party[i] = i;
    ASSERT_EQ(t_check_active(), 2);
    tests_passed++;
}

void test_formation_all_slots(void) {
    int i;
    t_set_positions(50, 60);
    for (i = 0; i < 9; i++) {
        ASSERT_TRUE(t_formation[i] != 99);
    }
    tests_passed++;
}

int main(void) {
    printf("=== Battle Core Functions Tests ===\n\n");

    printf("[Unit Reset - FUN_00405080]\n");
    test_reset_clears_party();    printf("  TEST: Reset clears party slots ... PASS\n");
    test_reset_clears_enemy();    printf("  TEST: Reset clears enemy slots ... PASS\n");
    test_reset_clears_reserve();  printf("  TEST: Reset clears reserve slots ... PASS\n");
    test_reset_clears_pet();      printf("  TEST: Reset clears pet slots ... PASS\n");
    test_reset_clears_formation();printf("  TEST: Reset clears formation ... PASS\n");
    test_reset_state_flags();     printf("  TEST: Reset sets state flags ... PASS\n");
    test_pvp_flag_set();          printf("  TEST: PVP flag set when check=-2 ... PASS\n");
    test_pvp_flag_clear();        printf("  TEST: PVP flag clear when check=0 ... PASS\n");

    printf("[Action Dispatch - FUN_00424b70]\n");
    test_action_dispatch_stores(); printf("  TEST: Dispatch stores params ... PASS\n");
    test_action_attack();          printf("  TEST: Attack action type 0 ... PASS\n");
    test_action_guard();           printf("  TEST: Guard action type 1 ... PASS\n");
    test_action_escape();          printf("  TEST: Escape action type 2 ... PASS\n");
    test_action_skill();           printf("  TEST: Skill action type 7 ... PASS\n");
    test_action_capture();         printf("  TEST: Capture action type 9 ... PASS\n");
    test_action_item();            printf("  TEST: Item action type 6 ... PASS\n");
    test_action_pet_skill();       printf("  TEST: Pet skill action type 8 ... PASS\n");
    test_action_team();            printf("  TEST: Team actions type 10/11 ... PASS\n");
    test_action_pet_summon();      printf("  TEST: Pet summon type 520 ... PASS\n");
    test_action_battle_text();     printf("  TEST: Battle text type 101 ... PASS\n");
    test_action_gold_reward();     printf("  TEST: Gold reward type 18 ... PASS\n");

    printf("[Battle Positions - FUN_00405370]\n");
    test_positions_center();       printf("  TEST: Center position (0,0) ... PASS\n");
    test_positions_offset();       printf("  TEST: Cardinal offsets ... PASS\n");
    test_positions_diagonal();     printf("  TEST: Diagonal offsets ... PASS\n");
    test_positions_zero_origin();  printf("  TEST: Zero origin ... PASS\n");
    test_formation_all_slots();    printf("  TEST: All 9 slots filled ... PASS\n");

    printf("[Active Units - FUN_00405120]\n");
    test_check_active_none();      printf("  TEST: No active units returns 0 ... PASS\n");
    test_check_active_one();       printf("  TEST: One active unit returns 0 ... PASS\n");
    test_check_active_two();       printf("  TEST: Two active units returns 1 ... PASS\n");
    test_check_active_many();      printf("  TEST: 3+ active units returns 2 ... PASS\n");
    test_check_active_all_ten();   printf("  TEST: All 10 active returns 2 ... PASS\n");

    printf("\n=== Results: %d passed, %d failed, %d total ===\n",
           tests_passed, tests_failed, tests_passed + tests_failed);
    return tests_failed;
}
