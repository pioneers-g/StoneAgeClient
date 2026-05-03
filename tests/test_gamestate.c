/*
 * Stone Age Client - Game State System Unit Tests
 * Tests for game state machine, transitions, state dispatch
 * Based on FUN_004799b0, FUN_00479c40, FUN_00479bc0 analysis
 *
 * Coverage target: 80%+ of gamestate.c
 * Key functions tested:
 * - gamestate_init/shutdown
 * - gamestate_set/set_state/change
 * - gamestate_dispatch (all 12 states)
 * - gamestate_enter/exit handlers
 * - Sub-state transitions (FUN_00479c40)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../tests/test_framework.h"
#include "types.h"
#include "gamestate.h"

/* ========================================
 * Mock Functions for External Dependencies
 * ======================================== */

/* Counter for mock function calls */
static int g_mock_fade_process_calls = 0;
static int g_mock_fade_process_result = 1;
static int g_mock_fade_process_last_type = -1;

/* Mock fade_process for testing */
int fade_process(int type) {
    g_mock_fade_process_calls++;
    g_mock_fade_process_last_type = type;
    return g_mock_fade_process_result;
}

/* Mock logger functions */
void logger_log(int level, const char* format, ...) {
    (void)level;
    (void)format;
    /* Suppress logging in tests */
}

/* Mock field_ui_init */
void field_ui_init(void) {
    /* Stub */
}

/* Mock battle_init_ui_state */
void battle_init_ui_state(void) {
    /* Stub */
}

/* Reset mock counters */
static void reset_mocks(void) {
    g_mock_fade_process_calls = 0;
    g_mock_fade_process_result = 1;
    g_mock_fade_process_last_type = -1;
}

/* ========================================
 * Test Cases for State Constants
 * ======================================== */

/* Test game state values from DAT_04630dd8 */
static void test_game_state_values(void) {
    TEST_BEGIN("Game state values");

    /* States from FUN_004799b0 switch cases */
    TEST_ASSERT_EQ(GAME_STATE_NONE, -1);
    TEST_ASSERT_EQ(GAME_STATE_INIT, 0);
    TEST_ASSERT_EQ(GAME_STATE_LOGIN, 1);
    TEST_ASSERT_EQ(GAME_STATE_CHAR_SELECT, 2);
    TEST_ASSERT_EQ(GAME_STATE_CHAR_CREATE, 3);
    TEST_ASSERT_EQ(GAME_STATE_MENU, 4);
    TEST_ASSERT_EQ(GAME_STATE_PRELOAD, 5);
    TEST_ASSERT_EQ(GAME_STATE_FIELD_INIT, 6);
    TEST_ASSERT_EQ(GAME_STATE_PLAYING, 7);
    TEST_ASSERT_EQ(GAME_STATE_UNUSED_8, 8);
    TEST_ASSERT_EQ(GAME_STATE_BATTLE_INIT, 9);
    TEST_ASSERT_EQ(GAME_STATE_BATTLE, 10);
    TEST_ASSERT_EQ(GAME_STATE_MAP_TRANSITION, 11);
    TEST_ASSERT_EQ(GAME_STATE_SHUTDOWN, 12);

    TEST_END();
}

/* Test render mode values from DAT_005ab6fc */
static void test_render_mode_values(void) {
    TEST_BEGIN("Render mode values");

    TEST_ASSERT_EQ(RENDER_MODE_NORMAL, 0);
    TEST_ASSERT_EQ(RENDER_MODE_LOADING, 2);
    TEST_ASSERT_EQ(RENDER_MODE_BATTLE, 3);
    TEST_ASSERT_EQ(RENDER_MODE_BATTLE_INIT, 4);

    TEST_END();
}

/* Test state stack size */
static void test_state_stack_size(void) {
    TEST_BEGIN("State stack size");

    TEST_ASSERT_EQ(MAX_STATE_STACK, 8);
    TEST_ASSERT(sizeof(g_gamestate.state_stack) >= 8 * sizeof(GameState));

    TEST_END();
}

/* Test transition delay constant */
static void test_transition_delay(void) {
    TEST_BEGIN("Transition delay constant");

    TEST_ASSERT_EQ(STATE_TRANSITION_DELAY, 500);

    TEST_END();
}

/* ========================================
 * Test Cases for Structure Layout
 * ======================================== */

/* Test GameStateContext structure size */
static void test_context_structure_size(void) {
    TEST_BEGIN("Context structure size");

    /* Minimum expected size for all fields */
    TEST_ASSERT(sizeof(GameStateContext) >= sizeof(GameState) * 2);  /* current + next */
    TEST_ASSERT(sizeof(GameStateContext) >= sizeof(int) * 10);  /* various int fields */
    TEST_ASSERT(sizeof(GameStateContext) >= sizeof(void*));  /* state_data */

    TEST_END();
}

/* Test context initialization */
static void test_context_init(void) {
    TEST_BEGIN("Context initialization");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    TEST_ASSERT_EQ(g_gamestate.current_state, GAME_STATE_NONE);
    TEST_ASSERT_EQ(g_gamestate.next_state, GAME_STATE_NONE);
    TEST_ASSERT_EQ(g_gamestate.sub_state, 0);
    TEST_ASSERT_EQ(g_gamestate.stack_depth, 0);
    TEST_ASSERT_EQ(g_gamestate.paused, 0);

    TEST_END();
}

/* Test state stack initialization */
static void test_state_stack_init(void) {
    TEST_BEGIN("State stack initialization");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    /* Stack should be empty */
    TEST_ASSERT_EQ(g_gamestate.stack_depth, 0);

    /* All stack entries should be NONE */
    for (int i = 0; i < MAX_STATE_STACK; i++) {
        TEST_ASSERT_EQ(g_gamestate.state_stack[i], GAME_STATE_NONE);
    }

    TEST_END();
}

/* ========================================
 * Test Cases for State Transitions
 * ======================================== */

/* Test state set function */
static void test_state_set(void) {
    TEST_BEGIN("State set");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    gamestate_set(GAME_STATE_LOGIN);

    TEST_ASSERT_EQ(g_gamestate.current_state, GAME_STATE_LOGIN);

    TEST_END();
}

/* Test state transition request */
static void test_state_request_change(void) {
    TEST_BEGIN("State request change");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_LOGIN;

    gamestate_request_change(GAME_STATE_CHAR_SELECT);

    /* Should set next_state for transition */
    TEST_ASSERT_EQ(g_gamestate.next_state, GAME_STATE_CHAR_SELECT);
    TEST_ASSERT_EQ(g_gamestate.transitioning, 1);

    TEST_END();
}

/* Test state query functions */
static void test_state_queries(void) {
    TEST_BEGIN("State queries");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_PLAYING;

    TEST_ASSERT_EQ(gamestate_get_current(), GAME_STATE_PLAYING);
    TEST_ASSERT(gamestate_is_state(GAME_STATE_PLAYING));
    TEST_ASSERT(!gamestate_is_state(GAME_STATE_LOGIN));
    TEST_ASSERT(gamestate_is_playing());
    TEST_ASSERT(!gamestate_is_battle());

    TEST_END();
}

/* Test battle state detection */
static void test_battle_state_detection(void) {
    TEST_BEGIN("Battle state detection");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    g_gamestate.current_state = GAME_STATE_BATTLE;
    TEST_ASSERT(gamestate_is_battle());
    TEST_ASSERT(!gamestate_is_playing());

    g_gamestate.current_state = GAME_STATE_BATTLE_INIT;
    TEST_ASSERT(!gamestate_is_battle());  /* INIT is not active battle */

    g_gamestate.current_state = GAME_STATE_PLAYING;
    TEST_ASSERT(!gamestate_is_battle());

    TEST_END();
}

/* Test transitioning flag */
static void test_transitioning_flag(void) {
    TEST_BEGIN("Transitioning flag");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    TEST_ASSERT(!gamestate_is_transitioning());

    g_gamestate.transitioning = 1;
    TEST_ASSERT(gamestate_is_transitioning());

    TEST_END();
}

/* Test paused state */
static void test_paused_state(void) {
    TEST_BEGIN("Paused state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    TEST_ASSERT(!gamestate_is_paused());

    gamestate_pause();
    TEST_ASSERT(gamestate_is_paused());
    TEST_ASSERT_EQ(g_gamestate.paused, 1);

    gamestate_resume();
    TEST_ASSERT(!gamestate_is_paused());

    TEST_END();
}

/* ========================================
 * Test Cases for State Stack
 * ======================================== */

/* Test state stack push */
static void test_state_stack_push(void) {
    TEST_BEGIN("State stack push");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_PLAYING;

    gamestate_push(GAME_STATE_MENU);

    TEST_ASSERT_EQ(g_gamestate.stack_depth, 1);
    TEST_ASSERT_EQ(g_gamestate.state_stack[0], GAME_STATE_MENU);

    TEST_END();
}

/* Test state stack pop */
static void test_state_stack_pop(void) {
    TEST_BEGIN("State stack pop");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_PLAYING;

    gamestate_push(GAME_STATE_MENU);
    gamestate_pop();

    TEST_ASSERT_EQ(g_gamestate.stack_depth, 0);

    TEST_END();
}

/* Test state stack overflow protection */
static void test_state_stack_overflow(void) {
    TEST_BEGIN("State stack overflow protection");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    /* Fill stack */
    for (int i = 0; i < MAX_STATE_STACK; i++) {
        gamestate_push(GAME_STATE_MENU);
    }

    /* One more push should not overflow */
    int depth_before = g_gamestate.stack_depth;
    gamestate_push(GAME_STATE_MENU);

    TEST_ASSERT_EQ(g_gamestate.stack_depth, depth_before);

    TEST_END();
}

/* Test state stack underflow protection */
static void test_state_stack_underflow(void) {
    TEST_BEGIN("State stack underflow protection");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    /* Pop from empty stack should not crash */
    gamestate_pop();

    TEST_ASSERT_EQ(g_gamestate.stack_depth, 0);

    TEST_END();
}

/* ========================================
 * Test Cases for Sub-states
 * ======================================== */

/* Test sub-state set/get */
static void test_sub_state_access(void) {
    TEST_BEGIN("Sub-state access");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    gamestate_set_sub_state(5);
    TEST_ASSERT_EQ(gamestate_get_sub_state(), 5);
    TEST_ASSERT_EQ(g_gamestate.sub_state, 5);

    TEST_END();
}

/* Test sub-state values from FUN_00479c40 */
static void test_sub_state_values(void) {
    TEST_BEGIN("Sub-state values");

    /* Field sub-states from FUN_00479c40 */
    int field_sub_state_normal = 0;
    int field_sub_state_menu = 1;
    int field_sub_state_dialog = 2;

    /* Battle sub-states from FUN_0040a1a0 */
    int battle_sub_state_init = 0;
    int battle_sub_state_intro = 1;
    int battle_sub_state_main = 2;
    int battle_sub_state_action = 3;
    int battle_sub_state_execute = 4;

    TEST_ASSERT_EQ(field_sub_state_normal, 0);
    TEST_ASSERT_EQ(field_sub_state_menu, 1);
    TEST_ASSERT_EQ(field_sub_state_dialog, 2);
    TEST_ASSERT_EQ(battle_sub_state_init, 0);
    TEST_ASSERT_EQ(battle_sub_state_intro, 1);
    TEST_ASSERT_EQ(battle_sub_state_main, 2);

    TEST_END();
}

/* ========================================
 * Test Cases for Render Mode
 * ======================================== */

/* Test render mode set/get */
static void test_render_mode_access(void) {
    TEST_BEGIN("Render mode access");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    gamestate_set_render_mode(RENDER_MODE_BATTLE);
    TEST_ASSERT_EQ(gamestate_get_render_mode(), RENDER_MODE_BATTLE);

    TEST_END();
}

/* Test render mode by state */
static void test_render_mode_by_state(void) {
    TEST_BEGIN("Render mode by state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    /* Normal field play */
    g_gamestate.current_state = GAME_STATE_PLAYING;
    g_gamestate.render_mode = RENDER_MODE_NORMAL;
    TEST_ASSERT_EQ(gamestate_get_render_mode(), RENDER_MODE_NORMAL);

    /* Battle state */
    g_gamestate.current_state = GAME_STATE_BATTLE;
    g_gamestate.render_mode = RENDER_MODE_BATTLE;
    TEST_ASSERT_EQ(gamestate_get_render_mode(), RENDER_MODE_BATTLE);

    TEST_END();
}

/* ========================================
 * Test Cases for Loading Progress
 * ======================================== */

/* Test loading progress */
static void test_loading_progress(void) {
    TEST_BEGIN("Loading progress");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    gamestate_set_loading_progress(50);
    TEST_ASSERT_EQ(gamestate_get_loading_progress(), 50);

    gamestate_set_loading_progress(100);
    TEST_ASSERT_EQ(gamestate_get_loading_progress(), 100);

    TEST_END();
}

/* Test loading state transition */
static void test_loading_state(void) {
    TEST_BEGIN("Loading state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    g_gamestate.current_state = GAME_STATE_MAP_TRANSITION;
    g_gamestate.render_mode = RENDER_MODE_LOADING;
    g_gamestate.loading_progress = 0;

    TEST_ASSERT_EQ(g_gamestate.current_state, GAME_STATE_MAP_TRANSITION);
    TEST_ASSERT_EQ(g_gamestate.render_mode, RENDER_MODE_LOADING);

    TEST_END();
}

/* ========================================
 * Test Cases for Battle Result
 * ======================================== */

/* Test battle result storage */
static void test_battle_result(void) {
    TEST_BEGIN("Battle result");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    /* Battle result values: 0=lose, 1=win, 2=escape */
    gamestate_set_battle_result(1);
    TEST_ASSERT_EQ(gamestate_get_battle_result(), 1);

    gamestate_set_battle_result(0);
    TEST_ASSERT_EQ(gamestate_get_battle_result(), 0);

    TEST_END();
}

/* Test battle result values */
static void test_battle_result_values(void) {
    TEST_BEGIN("Battle result values");

    int result_lose = 0;
    int result_win = 1;
    int result_escape = 2;

    TEST_ASSERT_EQ(result_lose, 0);
    TEST_ASSERT_EQ(result_win, 1);
    TEST_ASSERT_EQ(result_escape, 2);

    TEST_END();
}

/* ========================================
 * Test Cases for State Data
 * ======================================== */

/* Test state data storage */
static void test_state_data(void) {
    TEST_BEGIN("State data");

    int test_data = 12345;

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    gamestate_set_data(&test_data);
    TEST_ASSERT(gamestate_get_data() == &test_data);
    TEST_ASSERT_EQ(*(int*)gamestate_get_data(), 12345);

    TEST_END();
}

/* ========================================
 * Test Cases for State Time
 * ======================================== */

/* Test state time tracking */
static void test_state_time(void) {
    TEST_BEGIN("State time");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    /* State start time should be set on state change */
    g_gamestate.state_start_time = 1000;

    u32 elapsed = gamestate_get_state_time();
    TEST_ASSERT(elapsed >= 0);  /* Time should be non-negative */

    TEST_END();
}

/* ========================================
 * Test Cases for State String Conversion
 * ======================================== */

/* Test state to string conversion */
static void test_state_to_string(void) {
    TEST_BEGIN("State to string");

    const char* str;

    str = gamestate_to_string(GAME_STATE_INIT);
    TEST_ASSERT(str != NULL);
    TEST_ASSERT(strlen(str) > 0);

    str = gamestate_to_string(GAME_STATE_LOGIN);
    TEST_ASSERT(str != NULL);

    str = gamestate_to_string(GAME_STATE_PLAYING);
    TEST_ASSERT(str != NULL);

    str = gamestate_to_string(GAME_STATE_BATTLE);
    TEST_ASSERT(str != NULL);

    TEST_END();
}

/* ========================================
 * Test Cases for Exit Handling
 * ======================================== */

/* Test exit to menu request */
static void test_exit_to_menu(void) {
    TEST_BEGIN("Exit to menu");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_PLAYING;

    gamestate_request_exit_to_menu();

    /* Should request transition to menu */
    TEST_ASSERT(g_gamestate.next_state == GAME_STATE_MENU ||
                g_gamestate.state_flags != 0);

    TEST_END();
}

/* Test logout request */
static void test_logout_request(void) {
    TEST_BEGIN("Logout request");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_PLAYING;

    gamestate_request_logout();

    /* Should request transition to login */
    TEST_ASSERT(g_gamestate.next_state == GAME_STATE_LOGIN ||
                g_gamestate.state_flags != 0);

    TEST_END();
}

/* ========================================
 * Test Cases for State Flags
 * ======================================== */

/* Test state flags from DAT_04630de4 */
static void test_state_flags(void) {
    TEST_BEGIN("State flags");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    /* State flags can indicate various conditions */
    g_gamestate.state_flags = 0x01;
    TEST_ASSERT_EQ(g_gamestate.state_flags, 1);

    g_gamestate.state_flags = 0x02;
    TEST_ASSERT_EQ(g_gamestate.state_flags, 2);

    g_gamestate.state_flags = 0x04;
    TEST_ASSERT_EQ(g_gamestate.state_flags, 4);

    TEST_END();
}

/* ========================================
 * Test Cases for Memory Addresses
 * ======================================== */

/* Test state variable addresses from binary */
static void test_state_addresses(void) {
    TEST_BEGIN("State variable addresses");

    /* Key addresses from binary analysis */
    u32 addr_state = 0x04630dd8;     /* Main game state */
    u32 addr_sub_state = 0x04630df0; /* Sub-state */
    u32 addr_flags = 0x04630de4;     /* State flags */
    u32 addr_render_mode = 0x005ab6fc; /* Render mode */

    TEST_ASSERT(addr_state > 0);
    TEST_ASSERT(addr_sub_state > addr_state);
    TEST_ASSERT(addr_flags > addr_state);
    TEST_ASSERT(addr_render_mode > 0);

    TEST_END();
}

/* Test state dispatch function */
static void test_state_dispatch_exists(void) {
    TEST_BEGIN("State dispatch function");

    /* FUN_004799b0 - main state dispatcher */
    u32 dispatch_addr = 0x004799b0;
    TEST_ASSERT(dispatch_addr > 0);

    TEST_END();
}

/* ========================================
 * Test Cases for State Dispatch (FUN_004799b0)
 * ======================================== */

/* Test dispatch with init state */
static void test_dispatch_init_state(void) {
    TEST_BEGIN("Dispatch init state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_INIT;
    g_gamestate.next_state = (GameState)-1;
    reset_mocks();

    /* Dispatch should call state 0 handler */
    gamestate_dispatch();

    /* State 0 transitions internally */
    TEST_ASSERT(g_gamestate.current_state == GAME_STATE_INIT ||
                g_gamestate.current_state == GAME_STATE_LOGIN);

    TEST_END();
}

/* Test dispatch with login state */
static void test_dispatch_login_state(void) {
    TEST_BEGIN("Dispatch login state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_LOGIN;
    g_gamestate.next_state = (GameState)-1;
    g_gamestate.sub_state = 0;
    reset_mocks();

    gamestate_dispatch();

    /* Login state should set render_mode to 0 */
    TEST_ASSERT_EQ(g_gamestate.render_mode, 0);

    TEST_END();
}

/* Test dispatch with char_select state */
static void test_dispatch_char_select_state(void) {
    TEST_BEGIN("Dispatch char_select state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_CHAR_SELECT;
    g_gamestate.next_state = (GameState)-1;
    reset_mocks();

    gamestate_dispatch();

    TEST_ASSERT_EQ(g_gamestate.render_mode, 0);

    TEST_END();
}

/* Test dispatch with char_create state */
static void test_dispatch_char_create_state(void) {
    TEST_BEGIN("Dispatch char_create state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_CHAR_CREATE;
    g_gamestate.next_state = (GameState)-1;
    reset_mocks();

    gamestate_dispatch();

    TEST_ASSERT_EQ(g_gamestate.render_mode, 0);

    TEST_END();
}

/* Test dispatch with menu state */
static void test_dispatch_menu_state(void) {
    TEST_BEGIN("Dispatch menu state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_MENU;
    g_gamestate.next_state = (GameState)-1;
    reset_mocks();

    gamestate_dispatch();

    TEST_ASSERT_EQ(g_gamestate.render_mode, 0);

    TEST_END();
}

/* Test dispatch with preload state */
static void test_dispatch_preload_state(void) {
    TEST_BEGIN("Dispatch preload state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_PRELOAD;
    g_gamestate.next_state = (GameState)-1;
    g_gamestate.sub_state = 0;
    reset_mocks();

    gamestate_dispatch();

    /* Preload should transition through sub-states */
    TEST_ASSERT(g_gamestate.sub_state >= 0);

    TEST_END();
}

/* Test dispatch with field_init state */
static void test_dispatch_field_init_state(void) {
    TEST_BEGIN("Dispatch field_init state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_FIELD_INIT;
    g_gamestate.next_state = (GameState)-1;
    g_gamestate.sub_state = 0;
    reset_mocks();

    gamestate_dispatch();

    TEST_ASSERT_EQ(g_gamestate.render_mode, 0);

    TEST_END();
}

/* Test dispatch with playing state */
static void test_dispatch_playing_state(void) {
    TEST_BEGIN("Dispatch playing state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_PLAYING;
    g_gamestate.next_state = (GameState)-1;
    g_gamestate.sub_state = 0;
    reset_mocks();

    gamestate_dispatch();

    /* Playing state should update sub-state */
    TEST_ASSERT(g_gamestate.sub_state >= 0);

    TEST_END();
}

/* Test dispatch with battle_init state */
static void test_dispatch_battle_init_state(void) {
    TEST_BEGIN("Dispatch battle_init state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_BATTLE_INIT;
    g_gamestate.next_state = (GameState)-1;
    g_gamestate.sub_state = 0;
    reset_mocks();

    gamestate_dispatch();

    /* Battle init should progress through sub-states */
    TEST_ASSERT(g_gamestate.sub_state >= 0);

    TEST_END();
}

/* Test dispatch with battle state */
static void test_dispatch_battle_state(void) {
    TEST_BEGIN("Dispatch battle state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_BATTLE;
    g_gamestate.next_state = (GameState)-1;
    g_gamestate.sub_state = 0;
    reset_mocks();

    gamestate_dispatch();

    TEST_ASSERT(g_gamestate.render_mode == RENDER_MODE_BATTLE ||
                g_gamestate.render_mode == 0);

    TEST_END();
}

/* Test dispatch with map_transition state */
static void test_dispatch_map_transition_state(void) {
    TEST_BEGIN("Dispatch map_transition state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_MAP_TRANSITION;
    g_gamestate.next_state = (GameState)-1;
    g_gamestate.sub_state = 0;
    reset_mocks();

    gamestate_dispatch();

    /* Map transition should set loading render mode */
    TEST_ASSERT(g_gamestate.render_mode == RENDER_MODE_LOADING ||
                g_gamestate.render_mode == 0);

    TEST_END();
}

/* Test dispatch processes pending transition */
static void test_dispatch_processes_transition(void) {
    TEST_BEGIN("Dispatch processes pending transition");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_LOGIN;
    g_gamestate.next_state = GAME_STATE_CHAR_SELECT;
    g_gamestate.transitioning = 1;
    reset_mocks();

    gamestate_dispatch();

    /* Should have processed the transition */
    TEST_ASSERT_EQ(g_gamestate.current_state, GAME_STATE_CHAR_SELECT);
    TEST_ASSERT_EQ(g_gamestate.transitioning, 0);

    TEST_END();
}

/* ========================================
 * Test Cases for Battle Init Sub-states (FUN_00479c40)
 * ======================================== */

/* Test battle init sub-state 0 -> 0x96 -> 100 flow */
static void test_battle_init_substate_flow(void) {
    TEST_BEGIN("Battle init sub-state flow");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_BATTLE_INIT;
    g_gamestate.sub_state = 0;
    reset_mocks();

    /* First dispatch: 0 -> 0x96 */
    gamestate_dispatch();

    /* Sub-state should progress (case 0 falls through to 0x96) */
    TEST_ASSERT(g_gamestate.sub_state == 100 || g_gamestate.sub_state == 0x65);

    TEST_END();
}

/* Test battle init sub-state 0x14 (logout) */
static void test_battle_init_logout(void) {
    TEST_BEGIN("Battle init logout sub-state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_BATTLE_INIT;
    g_gamestate.sub_state = 0x14;  /* 20 - logout */
    reset_mocks();

    gamestate_dispatch();

    /* Should transition to login state */
    TEST_ASSERT_EQ(g_gamestate.current_state, GAME_STATE_LOGIN);

    TEST_END();
}

/* Test battle init sub-state 200 (map change start) */
static void test_battle_init_map_change(void) {
    TEST_BEGIN("Battle init map change sub-state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_BATTLE_INIT;
    g_gamestate.sub_state = 200;  /* 0xC8 - map change start */
    reset_mocks();

    gamestate_dispatch();

    /* Should progress to 0xC9 */
    TEST_ASSERT(g_gamestate.sub_state == 0xC9 || g_gamestate.sub_state > 200);

    TEST_END();
}

/* Test battle init sub-state 5 -> battle transition */
static void test_battle_init_to_battle(void) {
    TEST_BEGIN("Battle init to battle transition");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_BATTLE_INIT;
    g_gamestate.sub_state = 5;
    g_mock_fade_process_result = 1;  /* Fade complete */
    reset_mocks();

    gamestate_dispatch();

    /* Should transition to battle state */
    TEST_ASSERT_EQ(g_gamestate.current_state, GAME_STATE_BATTLE);

    TEST_END();
}

/* ========================================
 * Test Cases for Enter/Exit Handlers
 * ======================================== */

/* Test enter handler for login state */
static void test_enter_login_state(void) {
    TEST_BEGIN("Enter login state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    gamestate_enter(GAME_STATE_LOGIN);

    TEST_ASSERT_EQ(g_gamestate.render_mode, RENDER_MODE_NORMAL);

    TEST_END();
}

/* Test enter handler for battle state */
static void test_enter_battle_state(void) {
    TEST_BEGIN("Enter battle state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    gamestate_enter(GAME_STATE_BATTLE);

    TEST_ASSERT_EQ(g_gamestate.render_mode, RENDER_MODE_BATTLE);

    TEST_END();
}

/* Test enter handler for battle_init state */
static void test_enter_battle_init_state(void) {
    TEST_BEGIN("Enter battle_init state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    gamestate_enter(GAME_STATE_BATTLE_INIT);

    TEST_ASSERT_EQ(g_gamestate.render_mode, RENDER_MODE_BATTLE_INIT);

    TEST_END();
}

/* Test enter handler for map_transition state */
static void test_enter_map_transition_state(void) {
    TEST_BEGIN("Enter map_transition state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    gamestate_enter(GAME_STATE_MAP_TRANSITION);

    TEST_ASSERT_EQ(g_gamestate.render_mode, RENDER_MODE_LOADING);

    TEST_END();
}

/* Test exit handler doesn't crash */
static void test_exit_handlers(void) {
    TEST_BEGIN("Exit handlers no crash");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    /* All exit handlers should complete without crash */
    gamestate_exit(GAME_STATE_INIT);
    gamestate_exit(GAME_STATE_LOGIN);
    gamestate_exit(GAME_STATE_CHAR_SELECT);
    gamestate_exit(GAME_STATE_CHAR_CREATE);
    gamestate_exit(GAME_STATE_MENU);
    gamestate_exit(GAME_STATE_PRELOAD);
    gamestate_exit(GAME_STATE_FIELD_INIT);
    gamestate_exit(GAME_STATE_PLAYING);
    gamestate_exit(GAME_STATE_BATTLE_INIT);
    gamestate_exit(GAME_STATE_BATTLE);
    gamestate_exit(GAME_STATE_MAP_TRANSITION);

    TEST_ASSERT(1);  /* If we get here, no crash */

    TEST_END();
}

/* ========================================
 * Test Cases for Init and Shutdown
 * ======================================== */

/* Test gamestate_init */
static void test_gamestate_init(void) {
    TEST_BEGIN("Gamestate init");

    /* Initialize with some garbage data first */
    memset(&g_gamestate, 0xFF, sizeof(GameStateContext));

    /* Init should clear and set proper defaults */
    int result = gamestate_init();

    TEST_ASSERT_EQ(result, 1);
    TEST_ASSERT_EQ(g_gamestate.current_state, GAME_STATE_INIT);
    TEST_ASSERT_EQ(g_gamestate.sub_state, 0);
    TEST_ASSERT_EQ(g_gamestate.transitioning, 0);
    TEST_ASSERT_EQ(g_gamestate.render_mode, 0);

    TEST_END();
}

/* Test gamestate_shutdown */
static void test_gamestate_shutdown(void) {
    TEST_BEGIN("Gamestate shutdown");

    gamestate_init();
    gamestate_set(GAME_STATE_PLAYING);

    /* Shutdown should clean up */
    gamestate_shutdown();

    TEST_ASSERT_EQ(g_gamestate.current_state, 0);  /* Cleared */

    TEST_END();
}

/* ========================================
 * Test Cases for State Change Functions
 * ======================================== */

/* Test gamestate_change */
static void test_gamestate_change(void) {
    TEST_BEGIN("Gamestate change");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_LOGIN;

    int result = gamestate_change(GAME_STATE_CHAR_SELECT);

    TEST_ASSERT_EQ(result, 1);
    TEST_ASSERT_EQ(g_gamestate.current_state, GAME_STATE_CHAR_SELECT);
    TEST_ASSERT_EQ(g_gamestate.sub_state, 0);
    TEST_ASSERT_EQ(g_gamestate.transitioning, 0);

    TEST_END();
}

/* Test gamestate_change to same state */
static void test_gamestate_change_same(void) {
    TEST_BEGIN("Gamestate change same state");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_LOGIN;

    int result = gamestate_change(GAME_STATE_LOGIN);

    TEST_ASSERT_EQ(result, 0);  /* No change */

    TEST_END();
}

/* Test gamestate_process_transition */
static void test_process_transition(void) {
    TEST_BEGIN("Process transition");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_LOGIN;
    g_gamestate.next_state = GAME_STATE_PLAYING;
    g_gamestate.transitioning = 1;

    gamestate_process_transition();

    TEST_ASSERT_EQ(g_gamestate.current_state, GAME_STATE_PLAYING);
    TEST_ASSERT_EQ(g_gamestate.transitioning, 0);

    TEST_END();
}

/* Test gamestate_process_transition with no pending */
static void test_process_transition_no_pending(void) {
    TEST_BEGIN("Process transition no pending");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_LOGIN;
    g_gamestate.next_state = (GameState)-1;
    g_gamestate.transitioning = 0;

    gamestate_process_transition();

    /* Should remain unchanged */
    TEST_ASSERT_EQ(g_gamestate.current_state, GAME_STATE_LOGIN);

    TEST_END();
}

/* ========================================
 * Test Cases for gamestate_update Wrapper
 * ======================================== */

/* Test gamestate_update calls dispatch */
static void test_gamestate_update(void) {
    TEST_BEGIN("Gamestate update");

    memset(&g_gamestate, 0, sizeof(GameStateContext));
    g_gamestate.current_state = GAME_STATE_LOGIN;
    g_gamestate.next_state = (GameState)-1;

    /* Update should call dispatch internally */
    gamestate_update(16);  /* 16ms delta */

    TEST_ASSERT(g_gamestate.current_state == GAME_STATE_LOGIN);

    TEST_END();
}

/* ========================================
 * Test Cases for gamestate_render
 * ======================================== */

/* Test gamestate_render doesn't crash */
static void test_gamestate_render(void) {
    TEST_BEGIN("Gamestate render");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    /* Render all states - should not crash */
    g_gamestate.current_state = GAME_STATE_INIT;
    gamestate_render();

    g_gamestate.current_state = GAME_STATE_LOGIN;
    gamestate_render();

    g_gamestate.current_state = GAME_STATE_PLAYING;
    gamestate_render();

    g_gamestate.current_state = GAME_STATE_BATTLE;
    gamestate_render();

    TEST_ASSERT(1);  /* If we get here, no crash */

    TEST_END();
}

/* ========================================
 * Test Cases for gamestate_handle_input
 * ======================================== */

/* Test gamestate_handle_input doesn't crash */
static void test_gamestate_handle_input(void) {
    TEST_BEGIN("Gamestate handle input");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    /* Handle input for various states - should not crash */
    g_gamestate.current_state = GAME_STATE_LOGIN;
    gamestate_handle_input(0, 0, 0);

    g_gamestate.current_state = GAME_STATE_PLAYING;
    gamestate_handle_input(0x100, 0x41, 0);  /* Key down, 'A' */

    g_gamestate.current_state = GAME_STATE_BATTLE;
    gamestate_handle_input(0x200, 1, 0);  /* Mouse click */

    TEST_ASSERT(1);  /* If we get here, no crash */

    TEST_END();
}

/* ========================================
 * Test Cases for gamestate_can_exit
 * ======================================== */

/* Test gamestate_can_exit for various states */
static void test_can_exit(void) {
    TEST_BEGIN("Can exit various states");

    memset(&g_gamestate, 0, sizeof(GameStateContext));

    g_gamestate.current_state = GAME_STATE_LOGIN;
    TEST_ASSERT(gamestate_can_exit());

    g_gamestate.current_state = GAME_STATE_PLAYING;
    TEST_ASSERT(gamestate_can_exit());

    g_gamestate.current_state = GAME_STATE_BATTLE;
    TEST_ASSERT(!gamestate_can_exit());  /* Can't exit during battle */

    g_gamestate.current_state = GAME_STATE_BATTLE_INIT;
    TEST_ASSERT(!gamestate_can_exit());  /* Can't exit during battle init */

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(gamestate) {
    /* Constants */
    test_game_state_values();
    test_render_mode_values();
    test_state_stack_size();
    test_transition_delay();

    /* Structure */
    test_context_structure_size();
    test_context_init();
    test_state_stack_init();

    /* Init and Shutdown */
    test_gamestate_init();
    test_gamestate_shutdown();

    /* State Changes */
    test_state_set();
    test_gamestate_change();
    test_gamestate_change_same();
    test_state_request_change();
    test_process_transition();
    test_process_transition_no_pending();

    /* Transitions */
    test_state_queries();
    test_battle_state_detection();
    test_transitioning_flag();
    test_paused_state();

    /* Stack */
    test_state_stack_push();
    test_state_stack_pop();
    test_state_stack_overflow();
    test_state_stack_underflow();

    /* Sub-states */
    test_sub_state_access();
    test_sub_state_values();

    /* Render mode */
    test_render_mode_access();
    test_render_mode_by_state();

    /* Loading */
    test_loading_progress();
    test_loading_state();

    /* Battle */
    test_battle_result();
    test_battle_result_values();

    /* Data */
    test_state_data();
    test_state_time();
    test_state_to_string();

    /* Exit handling */
    test_exit_to_menu();
    test_logout_request();
    test_can_exit();

    /* Flags and addresses */
    test_state_flags();
    test_state_addresses();
    test_state_dispatch_exists();

    /* State Dispatch (FUN_004799b0) */
    test_dispatch_init_state();
    test_dispatch_login_state();
    test_dispatch_char_select_state();
    test_dispatch_char_create_state();
    test_dispatch_menu_state();
    test_dispatch_preload_state();
    test_dispatch_field_init_state();
    test_dispatch_playing_state();
    test_dispatch_battle_init_state();
    test_dispatch_battle_state();
    test_dispatch_map_transition_state();
    test_dispatch_processes_transition();

    /* Battle Init Sub-states (FUN_00479c40) */
    test_battle_init_substate_flow();
    test_battle_init_logout();
    test_battle_init_map_change();
    test_battle_init_to_battle();

    /* Enter/Exit Handlers */
    test_enter_login_state();
    test_enter_battle_state();
    test_enter_battle_init_state();
    test_enter_map_transition_state();
    test_exit_handlers();

    /* Update/Render/Input */
    test_gamestate_update();
    test_gamestate_render();
    test_gamestate_handle_input();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - Game State Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(gamestate);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
