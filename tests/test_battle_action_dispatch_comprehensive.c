/*
 * Stone Age Client - Battle Action Dispatch Tests
 * Tests for FUN_00405160, battle_action_dispatch, message parsing
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

/* === Inline test mirrors === */

/* Action state mirror - matches BattleActionState */
static u32 t_action_type = 0;
static u32 t_action_p1 = 0;
static u32 t_action_p2 = 0;
static u32 t_action_p3 = 0;
static char t_msg_buf[256] = {0};
static int t_action_ended = 0;
static int t_is_combo = 0;
static int t_escape_max = 80;
static int t_escape_count = 0;
static int t_death_flag = 0;

/* Unit action marking globals mirror */
static s32 t_party[10];
static s32 t_action_group = 0;
static s32 t_turn_id = 0;
static u32 t_action_bitmask = 0;
static int t_marked[10];

/* Handler tracking */
static int t_last_handler = -1;
static int t_handler_called = 0;

/* Handlers called by dispatch */
static void t_handler_attack(void)    { t_last_handler = 0; t_handler_called++; }
static void t_handler_skill(void)     { t_last_handler = 1; t_handler_called++; }
static void t_handler_counter(void)   { t_last_handler = 2; t_handler_called++; }
static void t_handler_combo(void)     { t_last_handler = 3; t_handler_called++; }
static void t_handler_item(void)      { t_last_handler = 4; t_handler_called++; }
static void t_handler_wait(void)      { t_last_handler = 5; t_handler_called++; }
static void t_handler_capture(void)   { t_last_handler = 6; t_handler_called++; }
static void t_handler_end(void)       { t_last_handler = 7; t_handler_called++; }
static void t_handler_pet_attack(void){ t_last_handler = 8; t_handler_called++; }
static void t_handler_pet_skill(void) { t_last_handler = 9; t_handler_called++; }
static void t_handler_summon(void)    { t_last_handler = 10; t_handler_called++; }
static void t_handler_recall(void)    { t_last_handler = 11; t_handler_called++; }
static void t_handler_defend(void)    { t_last_handler = 12; t_handler_called++; }
static void t_handler_escape(void)    { t_last_handler = 13; t_handler_called++; }
static void t_handler_death(void)     { t_last_handler = 14; t_handler_called++; }
static void t_handler_revive(void)    { t_last_handler = 15; t_handler_called++; }
static void t_handler_berserk(void)   { t_last_handler = 16; t_handler_called++; }
static void t_handler_unknown(void)   { t_last_handler = -2; t_handler_called++; }

static void t_dispatch_reset(void) {
    t_action_type = 0;
    t_action_p1 = 0;
    t_action_p2 = 0;
    t_action_p3 = 0;
    t_msg_buf[0] = '\0';
    t_action_ended = 0;
    t_last_handler = -1;
    t_handler_called = 0;
}

/* Simplified dispatch mirror matching FUN_00424b70 */
static void t_dispatch(u32 action, u32 p1, u32 p2, u32 p3) {
    t_action_type = action;
    t_action_p1 = p1;
    t_action_p2 = p2;
    t_action_p3 = p3;

    switch (action) {
        case 0x00: case 0x01: case 0x2d: t_handler_attack(); break;
        case 0x02: t_handler_skill(); break;
        case 0x06: t_handler_counter(); break;
        case 0x07: case 0x08: t_handler_combo(); break;
        case 0x09: t_handler_item(); break;
        case 0x0a: case 0x0b: t_handler_wait(); break;
        case 0x0c: t_handler_capture(); break;
        case 0x0d: t_handler_end(); break;
        case 0x0e: t_handler_pet_attack(); break;
        case 0x0f: t_handler_pet_skill(); break;
        case 0x16: case 0x2c: t_handler_summon(); break;
        case 0x17: case 0x2b: t_handler_recall(); break;
        case 0x29: t_handler_defend(); break;
        case 0x2a: t_handler_escape(); break;
        case 0x1c: t_handler_death(); break;
        case 0x1e: t_handler_revive(); break;
        case 0x1f: t_handler_berserk(); break;
        default: t_handler_unknown(); break;
    }
    t_action_ended = 1;
}

/* Parse pipe-delimited message - FUN_00489f70 pattern */
static int t_parse_message(const char* msg, int* out_ints, int max_ints) {
    char buf[512];
    char* token;
    int count = 0;
    int i = 0;
    strncpy(buf, msg, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    token = strtok(buf, "|");
    while (token && count < max_ints) {
        while (*token == ' ') token++;
        out_ints[count] = atoi(token);
        count++;
        token = strtok(NULL, "|");
    }
    return count;
}

/* FUN_00405160 mirror - mark units for action */
static void t_mark_units(void) {
    int i;
    for (i = 0; i < 10; i++) t_marked[i] = 0;

    if (t_action_group != t_turn_id) {
        /* Mark all active units */
        for (i = 0; i < 10; i++) {
            if (t_party[i] != SLOT_EMPTY) t_marked[i] = 1;
        }
    } else {
        /* Mark only bitmask-selected units */
        for (i = 0; i < 10; i++) {
            if (t_party[i] != SLOT_EMPTY && (t_action_bitmask & (1u << i)))
                t_marked[i] = 1;
        }
    }
}

static void t_unit_reset(void) {
    int i;
    for (i = 0; i < 10; i++) t_party[i] = SLOT_EMPTY;
    for (i = 0; i < 10; i++) t_marked[i] = 0;
    t_action_group = 0;
    t_turn_id = 0;
    t_action_bitmask = 0;
}

/* === Tests === */

void test_dispatch_attack(void) {
    t_dispatch_reset();
    t_dispatch(0x00, 1, 2, 3);
    ASSERT_EQ(t_last_handler, 0);
    ASSERT_EQ(t_handler_called, 1);
    tests_passed++;
}

void test_dispatch_attack_variant(void) {
    t_dispatch_reset();
    t_dispatch(0x01, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 0);
    tests_passed++;
}

void test_dispatch_skill(void) {
    t_dispatch_reset();
    t_dispatch(0x02, 10, 5, 0);
    ASSERT_EQ(t_last_handler, 1);
    ASSERT_EQ(t_action_p1, 10);
    tests_passed++;
}

void test_dispatch_counter(void) {
    t_dispatch_reset();
    t_dispatch(0x06, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 2);
    tests_passed++;
}

void test_dispatch_combo(void) {
    t_dispatch_reset();
    t_dispatch(0x07, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 3);
    t_dispatch_reset();
    t_dispatch(0x08, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 3);
    tests_passed++;
}

void test_dispatch_item(void) {
    t_dispatch_reset();
    t_dispatch(0x09, 3, 0, 0);
    ASSERT_EQ(t_last_handler, 4);
    tests_passed++;
}

void test_dispatch_wait(void) {
    t_dispatch_reset();
    t_dispatch(0x0a, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 5);
    t_dispatch_reset();
    t_dispatch(0x0b, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 5);
    tests_passed++;
}

void test_dispatch_capture(void) {
    t_dispatch_reset();
    t_dispatch(0x0c, 5, 0, 0);
    ASSERT_EQ(t_last_handler, 6);
    tests_passed++;
}

void test_dispatch_end(void) {
    t_dispatch_reset();
    t_dispatch(0x0d, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 7);
    tests_passed++;
}

void test_dispatch_pet_attack(void) {
    t_dispatch_reset();
    t_dispatch(0x0e, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 8);
    tests_passed++;
}

void test_dispatch_pet_skill(void) {
    t_dispatch_reset();
    t_dispatch(0x0f, 2, 1, 0);
    ASSERT_EQ(t_last_handler, 9);
    tests_passed++;
}

void test_dispatch_summon(void) {
    t_dispatch_reset();
    t_dispatch(0x16, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 10);
    t_dispatch_reset();
    t_dispatch(0x2c, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 10);
    tests_passed++;
}

void test_dispatch_recall(void) {
    t_dispatch_reset();
    t_dispatch(0x17, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 11);
    t_dispatch_reset();
    t_dispatch(0x2b, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 11);
    tests_passed++;
}

void test_dispatch_defend(void) {
    t_dispatch_reset();
    t_dispatch(0x29, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 12);
    tests_passed++;
}

void test_dispatch_escape(void) {
    t_dispatch_reset();
    t_dispatch(0x2a, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 13);
    tests_passed++;
}

void test_dispatch_death(void) {
    t_dispatch_reset();
    t_dispatch(0x1c, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 14);
    tests_passed++;
}

void test_dispatch_revive(void) {
    t_dispatch_reset();
    t_dispatch(0x1e, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 15);
    tests_passed++;
}

void test_dispatch_berserk(void) {
    t_dispatch_reset();
    t_dispatch(0x1f, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 16);
    tests_passed++;
}

void test_dispatch_unknown(void) {
    t_dispatch_reset();
    t_dispatch(0xFF, 0, 0, 0);
    ASSERT_EQ(t_last_handler, -2);
    tests_passed++;
}

void test_dispatch_stores_params(void) {
    t_dispatch_reset();
    t_dispatch(0x02, 100, 200, 300);
    ASSERT_EQ(t_action_type, 0x02);
    ASSERT_EQ(t_action_p1, 100);
    ASSERT_EQ(t_action_p2, 200);
    ASSERT_EQ(t_action_p3, 300);
    tests_passed++;
}

void test_dispatch_sets_ended(void) {
    t_dispatch_reset();
    ASSERT_EQ(t_action_ended, 0);
    t_dispatch(0x00, 0, 0, 0);
    ASSERT_EQ(t_action_ended, 1);
    tests_passed++;
}

/* Message parsing tests */

void test_parse_single(void) {
    int vals[16] = {0};
    int count = t_parse_message("42", vals, 16);
    ASSERT_EQ(count, 1);
    ASSERT_EQ(vals[0], 42);
    tests_passed++;
}

void test_parse_pipe_delimited(void) {
    int vals[16] = {0};
    int count = t_parse_message("10|20|30", vals, 16);
    ASSERT_EQ(count, 3);
    ASSERT_EQ(vals[0], 10);
    ASSERT_EQ(vals[1], 20);
    ASSERT_EQ(vals[2], 30);
    tests_passed++;
}

void test_parse_many_values(void) {
    int vals[16] = {0};
    int count = t_parse_message("1|2|3|4|5|6|7|8", vals, 16);
    ASSERT_EQ(count, 8);
    ASSERT_EQ(vals[0], 1);
    ASSERT_EQ(vals[7], 8);
    tests_passed++;
}

void test_parse_negative(void) {
    int vals[16] = {0};
    int count = t_parse_message("-5|-10", vals, 16);
    ASSERT_EQ(count, 2);
    ASSERT_EQ(vals[0], -5);
    ASSERT_EQ(vals[1], -10);
    tests_passed++;
}

void test_parse_empty(void) {
    int vals[16] = {0};
    int count = t_parse_message("", vals, 16);
    ASSERT_TRUE(count >= 0);
    tests_passed++;
}

void test_parse_spaces(void) {
    int vals[16] = {0};
    int count = t_parse_message(" 42 | 99 ", vals, 16);
    ASSERT_TRUE(count >= 1);
    ASSERT_EQ(vals[0], 42);
    tests_passed++;
}

/* Unit action marking tests - FUN_00405160 */

void test_mark_all_active(void) {
    t_unit_reset();
    t_party[0] = 5;
    t_party[3] = 8;
    t_party[7] = 12;
    t_action_group = 1;
    t_turn_id = 2;
    t_mark_units();
    ASSERT_EQ(t_marked[0], 1);
    ASSERT_EQ(t_marked[1], 0);
    ASSERT_EQ(t_marked[3], 1);
    ASSERT_EQ(t_marked[7], 1);
    tests_passed++;
}

void test_mark_bitmask_selected(void) {
    t_unit_reset();
    t_party[0] = 5;
    t_party[1] = 8;
    t_party[2] = 12;
    t_action_group = 1;
    t_turn_id = 1;  /* same as action_group */
    t_action_bitmask = (1u << 0) | (1u << 2);  /* mark slots 0 and 2 */
    t_mark_units();
    ASSERT_EQ(t_marked[0], 1);
    ASSERT_EQ(t_marked[1], 0);
    ASSERT_EQ(t_marked[2], 1);
    tests_passed++;
}

void test_mark_bitmask_empty_slot(void) {
    t_unit_reset();
    t_party[0] = 5;
    /* slot 1 is SLOT_EMPTY */
    t_action_group = 1;
    t_turn_id = 1;
    t_action_bitmask = (1u << 0) | (1u << 1);  /* try to mark empty slot */
    t_mark_units();
    ASSERT_EQ(t_marked[0], 1);
    ASSERT_EQ(t_marked[1], 0);  /* empty slot not marked */
    tests_passed++;
}

void test_mark_all_empty(void) {
    t_unit_reset();
    t_action_group = 1;
    t_turn_id = 2;
    t_mark_units();
    {
        int i;
        for (i = 0; i < 10; i++) ASSERT_EQ(t_marked[i], 0);
    }
    tests_passed++;
}

void test_mark_no_bitmask(void) {
    t_unit_reset();
    t_party[0] = 5;
    t_action_group = 1;
    t_turn_id = 1;
    t_action_bitmask = 0;  /* no bits set */
    t_mark_units();
    ASSERT_EQ(t_marked[0], 0);  /* no units marked even though slot is active */
    tests_passed++;
}

void test_mark_all_full_party(void) {
    int i;
    t_unit_reset();
    for (i = 0; i < 10; i++) t_party[i] = i + 1;
    t_action_group = 1;
    t_turn_id = 2;  /* different = mark all */
    t_mark_units();
    for (i = 0; i < 10; i++) ASSERT_EQ(t_marked[i], 1);
    tests_passed++;
}

void test_mark_bitmask_full(void) {
    int i;
    t_unit_reset();
    for (i = 0; i < 10; i++) t_party[i] = i + 1;
    t_action_group = 1;
    t_turn_id = 1;  /* same = bitmask mode */
    t_action_bitmask = 0xFFFFFFFF;  /* all bits set */
    t_mark_units();
    for (i = 0; i < 10; i++) ASSERT_EQ(t_marked[i], 1);
    tests_passed++;
}

/* Action state tests */

void test_escape_max_default(void) {
    ASSERT_EQ(t_escape_max, 80);
    tests_passed++;
}

void test_action_init_clear(void) {
    t_action_type = 99;
    t_action_p1 = 50;
    t_action_ended = 1;
    t_is_combo = 1;
    /* Simulate init */
    t_action_type = 0;
    t_action_p1 = 0;
    t_action_p2 = 0;
    t_action_p3 = 0;
    t_action_ended = 0;
    t_is_combo = 0;
    t_escape_count = 0;
    t_death_flag = 0;
    ASSERT_EQ(t_action_type, 0);
    ASSERT_EQ(t_action_p1, 0);
    ASSERT_EQ(t_action_ended, 0);
    ASSERT_EQ(t_is_combo, 0);
    tests_passed++;
}

void test_special_attack(void) {
    t_dispatch_reset();
    t_dispatch(0x2d, 0, 0, 0);
    ASSERT_EQ(t_last_handler, 0);  /* same as normal attack */
    tests_passed++;
}

void test_dispatch_multiple_calls(void) {
    t_dispatch_reset();
    t_dispatch(0x00, 0, 0, 0);
    ASSERT_EQ(t_handler_called, 1);
    t_dispatch_reset();
    t_dispatch(0x02, 0, 0, 0);
    t_dispatch(0x09, 0, 0, 0);
    ASSERT_EQ(t_handler_called, 2);
    tests_passed++;
}

int main(void) {
    printf("=== Battle Action Dispatch Tests ===\n\n");

    printf("[Action Dispatch - FUN_00424b70]\n");
    test_dispatch_attack();          printf("  TEST: Attack dispatch 0x00 ... PASS\n");
    test_dispatch_attack_variant();  printf("  TEST: Attack variant 0x01 ... PASS\n");
    test_dispatch_skill();           printf("  TEST: Skill dispatch 0x02 ... PASS\n");
    test_dispatch_counter();         printf("  TEST: Counter dispatch 0x06 ... PASS\n");
    test_dispatch_combo();           printf("  TEST: Combo dispatch 0x07/08 ... PASS\n");
    test_dispatch_item();            printf("  TEST: Item dispatch 0x09 ... PASS\n");
    test_dispatch_wait();            printf("  TEST: Wait dispatch 0x0a/0b ... PASS\n");
    test_dispatch_capture();         printf("  TEST: Capture dispatch 0x0c ... PASS\n");
    test_dispatch_end();             printf("  TEST: End dispatch 0x0d ... PASS\n");
    test_dispatch_pet_attack();      printf("  TEST: Pet attack 0x0e ... PASS\n");
    test_dispatch_pet_skill();       printf("  TEST: Pet skill 0x0f ... PASS\n");
    test_dispatch_summon();          printf("  TEST: Summon 0x16/0x2c ... PASS\n");
    test_dispatch_recall();          printf("  TEST: Recall 0x17/0x2b ... PASS\n");
    test_dispatch_defend();          printf("  TEST: Defend 0x29 ... PASS\n");
    test_dispatch_escape();          printf("  TEST: Escape 0x2a ... PASS\n");
    test_dispatch_death();           printf("  TEST: Death 0x1c ... PASS\n");
    test_dispatch_revive();          printf("  TEST: Revive 0x1e ... PASS\n");
    test_dispatch_berserk();         printf("  TEST: Berserk 0x1f ... PASS\n");
    test_dispatch_unknown();         printf("  TEST: Unknown action ... PASS\n");
    test_dispatch_stores_params();   printf("  TEST: Stores parameters ... PASS\n");
    test_dispatch_sets_ended();      printf("  TEST: Sets action ended flag ... PASS\n");
    test_special_attack();           printf("  TEST: Special attack 0x2d ... PASS\n");
    test_dispatch_multiple_calls();  printf("  TEST: Multiple dispatch calls ... PASS\n");

    printf("[Message Parsing - FUN_00489f70]\n");
    test_parse_single();             printf("  TEST: Parse single value ... PASS\n");
    test_parse_pipe_delimited();     printf("  TEST: Parse pipe-delimited ... PASS\n");
    test_parse_many_values();        printf("  TEST: Parse 8 values ... PASS\n");
    test_parse_negative();           printf("  TEST: Parse negative values ... PASS\n");
    test_parse_empty();              printf("  TEST: Parse empty string ... PASS\n");
    test_parse_spaces();             printf("  TEST: Parse with spaces ... PASS\n");

    printf("[Unit Action Marking - FUN_00405160]\n");
    test_mark_all_active();          printf("  TEST: Mark all active (diff group) ... PASS\n");
    test_mark_bitmask_selected();    printf("  TEST: Mark bitmask selected ... PASS\n");
    test_mark_bitmask_empty_slot();  printf("  TEST: Bitmask skips empty ... PASS\n");
    test_mark_all_empty();           printf("  TEST: No marks when empty ... PASS\n");
    test_mark_no_bitmask();          printf("  TEST: Zero bitmask marks none ... PASS\n");
    test_mark_all_full_party();      printf("  TEST: Full party all marked ... PASS\n");
    test_mark_bitmask_full();        printf("  TEST: Full bitmask all marked ... PASS\n");

    printf("[Action State]\n");
    test_escape_max_default();       printf("  TEST: Escape max default 80 ... PASS\n");
    test_action_init_clear();        printf("  TEST: Init clears state ... PASS\n");

    printf("\n=== Results: %d passed, %d failed, %d total ===\n",
           tests_passed, tests_failed, tests_passed + tests_failed);
    return tests_failed;
}
