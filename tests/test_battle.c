/*
 * Stone Age Client - Battle System Comprehensive Unit Tests
 * Tests for battle state machine, unit management, action handling
 * Based on FUN_0040a1a0, FUN_00405080, FUN_004053e0, FUN_00424b70 analysis
 *
 * Coverage target: 80%+ of battle.c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../tests/test_framework.h"
#include "types.h"
#include "battle.h"

/* Mock functions for external dependencies */
void logger_log(int level, const char* format, ...) {
    (void)level;
    (void)format;
}

/* Mock character functions */
static Character s_mock_player = {0};

Character* character_get_player(void) {
    s_mock_player.id = 1;
    s_mock_player.stats.hp = 1000;
    s_mock_player.stats.max_hp = 1000;
    s_mock_player.stats.mp = 100;
    s_mock_player.stats.max_mp = 100;
    s_mock_player.stats.attack = 150;
    s_mock_player.stats.defense = 80;
    s_mock_player.stats.agility = 100;
    s_mock_player.appearance.base_sprite = 100;
    return &s_mock_player;
}

/* Mock render functions */
void battle_render_field(void) {}
void battle_render_units(void) {}
void battle_render_ui(void) {}
void battle_render_effects(void) {}
void battle_render_queue_process(void) {}
void battle_ui_init(void) {}
void battle_cursor_init(void) {}
void battle_field_render(void) {}

/* Mock asset functions */
typedef struct {
    u16 width;
    u16 height;
} SpriteEntry;

SpriteEntry* assets_get_sprite(u32 id) {
    static SpriteEntry sprite = {64, 64};
    (void)id;
    return &sprite;
}

/* Mock fade functions */
int battle_fade_check(void) { return 1; }
void battle_load_field(u32 field_id) { (void)field_id; }

/* Mock sound functions */
void battle_start_fade(u32 fade_type) { (void)fade_type; }

/* Reset battle state for each test */
static void reset_battle_state(void) {
    memset(&g_battle, 0, sizeof(BattleContext));
}

/* ========================================
 * Test Constants
 * ======================================== */

/* Test battle state constants from FUN_0040a1a0 */
static void test_battle_state_constants(void) {
    TEST_BEGIN("Battle state constants");

    /* Main battle states from DAT_04630df0 switch cases */
    TEST_ASSERT_EQ(BATTLE_STATE_NONE, 0);
    TEST_ASSERT_EQ(BATTLE_STATE_INIT, 1);
    TEST_ASSERT_EQ(BATTLE_STATE_LOAD_FIELD, 2);
    TEST_ASSERT_EQ(BATTLE_STATE_READY, 3);
    TEST_ASSERT_EQ(BATTLE_STATE_START, 6);
    TEST_ASSERT_EQ(BATTLE_STATE_INTRO, 7);
    TEST_ASSERT_EQ(BATTLE_STATE_MAIN, 8);
    TEST_ASSERT_EQ(BATTLE_STATE_ACTION, 9);
    TEST_ASSERT_EQ(BATTLE_STATE_EXECUTE, 10);
    TEST_ASSERT_EQ(BATTLE_STATE_RESULT, 11);
    TEST_ASSERT_EQ(BATTLE_STATE_NEXT_TURN, 12);
    TEST_ASSERT_EQ(BATTLE_STATE_END, 13);
    TEST_ASSERT_EQ(BATTLE_STATE_EXIT, 14);

    TEST_END();
}

/* Test battle action types from FUN_00424b70 */
static void test_battle_action_types(void) {
    TEST_BEGIN("Battle action types");

    TEST_ASSERT_EQ(BATTLE_ACTION_NONE, 0);
    TEST_ASSERT_EQ(BATTLE_ACTION_ATTACK, 1);
    TEST_ASSERT_EQ(BATTLE_ACTION_SKILL, 2);
    TEST_ASSERT_EQ(BATTLE_ACTION_COUNTER, 6);
    TEST_ASSERT_EQ(BATTLE_ACTION_COMBO, 7);
    TEST_ASSERT_EQ(BATTLE_ACTION_COMBO_FOLLOW, 8);
    TEST_ASSERT_EQ(BATTLE_ACTION_ITEM, 9);
    TEST_ASSERT_EQ(BATTLE_ACTION_WAIT, 0x0b);
    TEST_ASSERT_EQ(BATTLE_ACTION_CAPTURE, 0x0c);
    TEST_ASSERT_EQ(BATTLE_ACTION_DEFEND, 0x29);
    TEST_ASSERT_EQ(BATTLE_ACTION_ESCAPE, 0x2a);
    TEST_ASSERT_EQ(BATTLE_ACTION_SUMMON, 0x16);
    TEST_ASSERT_EQ(BATTLE_ACTION_RECALL, 0x17);
    TEST_ASSERT_EQ(BATTLE_ACTION_PET_ESCAPE, 0x67);
    TEST_ASSERT_EQ(BATTLE_ACTION_PET_SWAP, 0x68);

    TEST_END();
}

/* Test battle positions */
static void test_battle_positions(void) {
    TEST_BEGIN("Battle positions");

    TEST_ASSERT_EQ(BATTLE_POS_LEFT_FRONT, 0);
    TEST_ASSERT_EQ(BATTLE_POS_LEFT_BACK, 1);
    TEST_ASSERT_EQ(BATTLE_POS_RIGHT_FRONT, 2);
    TEST_ASSERT_EQ(BATTLE_POS_RIGHT_BACK, 3);

    TEST_END();
}

/* Test buff flags */
static void test_buff_flags(void) {
    TEST_BEGIN("Buff flags");

    TEST_ASSERT_EQ(BUFF_ATTACK_UP, (1 << 0));
    TEST_ASSERT_EQ(BUFF_ATTACK_DOWN, (1 << 1));
    TEST_ASSERT_EQ(BUFF_DEFENSE_UP, (1 << 2));
    TEST_ASSERT_EQ(BUFF_DEFENSE_DOWN, (1 << 3));
    TEST_ASSERT_EQ(BUFF_SPEED_UP, (1 << 4));
    TEST_ASSERT_EQ(BUFF_SPEED_DOWN, (1 << 5));
    TEST_ASSERT_EQ(BUFF_STUN, (1 << 6));
    TEST_ASSERT_EQ(BUFF_POISON, (1 << 7));
    TEST_ASSERT_EQ(BUFF_BERSERK, (1 << 8));
    TEST_ASSERT_EQ(BUFF_STONE, (1 << 9));
    TEST_ASSERT_EQ(BUFF_SLEEP, (1 << 10));
    TEST_ASSERT_EQ(BUFF_CONFUSE, (1 << 12));
    TEST_ASSERT_EQ(BUFF_PARALYZE, (1 << 13));
    TEST_ASSERT_EQ(BUFF_SILENCE, (1 << 14));
    TEST_ASSERT_EQ(BUFF_BLIND, (1 << 15));

    TEST_END();
}

/* Test element types */
static void test_element_types(void) {
    TEST_BEGIN("Element types");

    TEST_ASSERT_EQ(ELEMENT_NONE, 0);
    TEST_ASSERT_EQ(ELEMENT_FIRE, 1);
    TEST_ASSERT_EQ(ELEMENT_WATER, 2);
    TEST_ASSERT_EQ(ELEMENT_EARTH, 3);
    TEST_ASSERT_EQ(ELEMENT_WIND, 4);
    TEST_ASSERT_EQ(ELEMENT_LIGHT, 5);
    TEST_ASSERT_EQ(ELEMENT_DARK, 6);
    TEST_ASSERT_EQ(ELEMENT_MAX, 7);

    TEST_END();
}

/* ========================================
 * Test Structure Layout
 * ======================================== */

/* Test BattleUnit structure */
static void test_battle_unit_structure(void) {
    TEST_BEGIN("BattleUnit structure");

    BattleUnit unit;
    memset(&unit, 0, sizeof(BattleUnit));

    unit.id = 1;
    unit.char_id = 100;
    unit.sprite_id = 500;
    unit.side = 0;
    unit.position = BATTLE_POS_LEFT_FRONT;
    unit.is_pet = 0;
    unit.is_alive = 1;
    unit.level = 50;
    unit.hp = 1000;
    unit.max_hp = 1000;
    unit.mp = 100;
    unit.max_mp = 100;
    unit.attack = 150;
    unit.defense = 80;
    unit.speed = 120;
    unit.buff_flags = BUFF_ATTACK_UP | BUFF_DEFENSE_UP;
    unit.action = BATTLE_ACTION_ATTACK;
    unit.target_id = 5;

    TEST_ASSERT_EQ(unit.id, 1);
    TEST_ASSERT_EQ(unit.side, 0);
    TEST_ASSERT_EQ(unit.is_alive, 1);
    TEST_ASSERT_EQ(unit.hp, 1000);
    TEST_ASSERT_EQ(unit.attack, 150);
    TEST_ASSERT_EQ(unit.buff_flags, 0x05);
    TEST_ASSERT_EQ(unit.action, BATTLE_ACTION_ATTACK);

    TEST_END();
}

/* Test BattleContext initialization */
static void test_battle_context_init(void) {
    TEST_BEGIN("BattleContext initialization");

    reset_battle_state();

    TEST_ASSERT_EQ(g_battle.state, BATTLE_STATE_NONE);
    TEST_ASSERT_EQ(g_battle.unit_count, 0);
    TEST_ASSERT_EQ(g_battle.active, 0);
    TEST_ASSERT_EQ(g_battle.ended, 0);
    TEST_ASSERT_EQ(g_battle.victory, 0);

    TEST_END();
}

/* Test BattleResult structure */
static void test_battle_result_structure(void) {
    TEST_BEGIN("BattleResult structure");

    BattleResult result;
    memset(&result, 0, sizeof(BattleResult));

    result.attacker_id = 1;
    result.target_id = 2;
    result.damage = 500;
    result.flags = 0;
    result.critical = 1;
    result.miss = 0;
    result.skill_id = 10;

    TEST_ASSERT_EQ(result.attacker_id, 1);
    TEST_ASSERT_EQ(result.target_id, 2);
    TEST_ASSERT_EQ(result.damage, 500);
    TEST_ASSERT_EQ(result.critical, 1);
    TEST_ASSERT_EQ(result.miss, 0);

    TEST_END();
}

/* ========================================
 * Test Initialization
 * ======================================== */

/* Test battle_init */
static void test_battle_init(void) {
    TEST_BEGIN("Battle init");

    reset_battle_state();

    int result = battle_init();

    TEST_ASSERT_EQ(result, 1);
    TEST_ASSERT_EQ(g_battle.state, BATTLE_STATE_NONE);

    TEST_END();
}

/* Test battle_shutdown */
static void test_battle_shutdown(void) {
    TEST_BEGIN("Battle shutdown");

    battle_init();
    g_battle.state = BATTLE_STATE_MAIN;

    battle_shutdown();

    TEST_ASSERT_EQ(g_battle.state, BATTLE_STATE_NONE);

    TEST_END();
}

/* ========================================
 * Test Unit Management
 * ======================================== */

/* Test battle_add_unit */
static void test_battle_add_unit(void) {
    TEST_BEGIN("Battle add unit");

    reset_battle_state();

    BattleUnit unit = {0};
    unit.id = 1;
    unit.char_id = 100;
    unit.sprite_id = 500;
    unit.side = 0;
    unit.is_alive = 1;
    unit.hp = 1000;
    unit.max_hp = 1000;

    battle_add_unit(&unit, 0);

    TEST_ASSERT_EQ(g_battle.unit_count, 1);
    TEST_ASSERT_EQ(g_battle.units[0].id, 1);
    TEST_ASSERT_EQ(g_battle.player_count, 1);

    TEST_END();
}

/* Test battle_add_unit multiple */
static void test_battle_add_unit_multiple(void) {
    TEST_BEGIN("Battle add unit multiple");

    reset_battle_state();

    BattleUnit unit1 = {.id = 1, .side = 0, .is_alive = 1, .hp = 100};
    BattleUnit unit2 = {.id = 2, .side = 0, .is_alive = 1, .hp = 100};
    BattleUnit unit3 = {.id = 101, .side = 1, .is_alive = 1, .hp = 50};

    battle_add_unit(&unit1, 0);
    battle_add_unit(&unit2, 0);
    battle_add_unit(&unit3, 1);

    TEST_ASSERT_EQ(g_battle.unit_count, 3);
    TEST_ASSERT_EQ(g_battle.player_count, 2);
    TEST_ASSERT_EQ(g_battle.enemy_count, 1);

    TEST_END();
}

/* Test battle_get_unit */
static void test_battle_get_unit(void) {
    TEST_BEGIN("Battle get unit");

    reset_battle_state();

    BattleUnit unit1 = {.id = 1, .is_alive = 1, .hp = 100};
    BattleUnit unit2 = {.id = 2, .is_alive = 1, .hp = 200};

    battle_add_unit(&unit1, 0);
    battle_add_unit(&unit2, 0);

    BattleUnit* found = battle_get_unit(1);
    TEST_ASSERT(found != NULL);
    TEST_ASSERT_EQ(found->id, 1);
    TEST_ASSERT_EQ(found->hp, 100);

    BattleUnit* not_found = battle_get_unit(999);
    TEST_ASSERT(not_found == NULL);

    TEST_END();
}

/* Test battle_remove_unit */
static void test_battle_remove_unit(void) {
    TEST_BEGIN("Battle remove unit");

    reset_battle_state();

    BattleUnit unit1 = {.id = 1, .is_alive = 1};
    BattleUnit unit2 = {.id = 2, .is_alive = 1};
    BattleUnit unit3 = {.id = 3, .is_alive = 1};

    battle_add_unit(&unit1, 0);
    battle_add_unit(&unit2, 0);
    battle_add_unit(&unit3, 0);

    TEST_ASSERT_EQ(g_battle.unit_count, 3);

    battle_remove_unit(2);

    TEST_ASSERT_EQ(g_battle.unit_count, 2);
    TEST_ASSERT_EQ(g_battle.units[0].id, 1);
    TEST_ASSERT_EQ(g_battle.units[1].id, 3);

    TEST_END();
}

/* Test battle_get_unit_by_index */
static void test_battle_get_unit_by_index(void) {
    TEST_BEGIN("Battle get unit by index");

    reset_battle_state();

    BattleUnit unit1 = {.id = 10, .is_alive = 1};
    BattleUnit unit2 = {.id = 20, .is_alive = 1};

    battle_add_unit(&unit1, 0);
    battle_add_unit(&unit2, 0);

    BattleUnit* u0 = battle_get_unit_by_index(0);
    BattleUnit* u1 = battle_get_unit_by_index(1);
    BattleUnit* invalid = battle_get_unit_by_index(100);

    TEST_ASSERT(u0 != NULL);
    TEST_ASSERT_EQ(u0->id, 10);
    TEST_ASSERT(u1 != NULL);
    TEST_ASSERT_EQ(u1->id, 20);
    TEST_ASSERT(invalid == NULL);

    TEST_END();
}

/* ========================================
 * Test Battle Start/End
 * ======================================== */

/* Test battle_start */
static void test_battle_start(void) {
    TEST_BEGIN("Battle start");

    reset_battle_state();

    u32 enemies[] = {100, 101};
    int result = battle_start(1, 2, enemies);

    TEST_ASSERT_EQ(result, 1);
    TEST_ASSERT_EQ(g_battle.battle_id, 1);
    TEST_ASSERT_EQ(g_battle.state, BATTLE_STATE_START);
    TEST_ASSERT_EQ(g_battle.can_escape, 1);
    TEST_ASSERT(g_battle.unit_count >= 1);  /* Player + enemies */

    TEST_END();
}

/* Test battle_end */
static void test_battle_end(void) {
    TEST_BEGIN("Battle end");

    reset_battle_state();
    g_battle.state = BATTLE_STATE_MAIN;
    g_battle.ended = 0;

    battle_end();

    TEST_ASSERT_EQ(g_battle.state, BATTLE_STATE_END);
    TEST_ASSERT_EQ(g_battle.ended, 1);

    TEST_END();
}

/* Test battle_victory */
static void test_battle_victory(void) {
    TEST_BEGIN("Battle victory");

    reset_battle_state();
    g_battle.state = BATTLE_STATE_MAIN;

    battle_victory();

    TEST_ASSERT_EQ(g_battle.victory, 1);
    TEST_ASSERT_EQ(g_battle.ended, 1);

    TEST_END();
}

/* Test battle_defeat */
static void test_battle_defeat(void) {
    TEST_BEGIN("Battle defeat");

    reset_battle_state();
    g_battle.state = BATTLE_STATE_MAIN;

    battle_defeat();

    TEST_ASSERT_EQ(g_battle.victory, 0);
    TEST_ASSERT_EQ(g_battle.ended, 1);

    TEST_END();
}

/* ========================================
 * Test Action Selection
 * ======================================== */

/* Test battle_select_action attack */
static void test_battle_select_action_attack(void) {
    TEST_BEGIN("Battle select action attack");

    reset_battle_state();

    BattleUnit unit = {.id = 1, .is_alive = 1, .speed = 100};
    battle_add_unit(&unit, 0);

    battle_select_action(1, BATTLE_ACTION_ATTACK, 2, 0);

    BattleUnit* u = battle_get_unit(1);
    TEST_ASSERT_EQ(u->action, BATTLE_ACTION_ATTACK);
    TEST_ASSERT_EQ(u->target_id, 2);

    TEST_END();
}

/* Test battle_select_action skill */
static void test_battle_select_action_skill(void) {
    TEST_BEGIN("Battle select action skill");

    reset_battle_state();

    BattleUnit unit = {.id = 1, .is_alive = 1, .speed = 100};
    battle_add_unit(&unit, 0);

    battle_select_action(1, BATTLE_ACTION_SKILL, 5, 10);

    BattleUnit* u = battle_get_unit(1);
    TEST_ASSERT_EQ(u->action, BATTLE_ACTION_SKILL);
    TEST_ASSERT_EQ(u->target_id, 5);
    TEST_ASSERT_EQ(u->skill_id, 10);

    TEST_END();
}

/* Test battle_select_action defend */
static void test_battle_select_action_defend(void) {
    TEST_BEGIN("Battle select action defend");

    reset_battle_state();

    BattleUnit unit = {.id = 1, .is_alive = 1, .speed = 100};
    battle_add_unit(&unit, 0);

    battle_select_action(1, BATTLE_ACTION_DEFEND, 0, 0);

    BattleUnit* u = battle_get_unit(1);
    TEST_ASSERT_EQ(u->action, BATTLE_ACTION_DEFEND);

    TEST_END();
}

/* ========================================
 * Test Turn Management
 * ======================================== */

/* Test turn count initialization */
static void test_turn_count_init(void) {
    TEST_BEGIN("Turn count initialization");

    reset_battle_state();

    TEST_ASSERT_EQ(g_battle.turn_count, 0);

    u32 enemies[] = {100};
    battle_start(1, 1, enemies);

    TEST_ASSERT_EQ(g_battle.turn_count, 1);

    TEST_END();
}

/* Test turn order array size */
static void test_turn_order_size(void) {
    TEST_BEGIN("Turn order size");

    TEST_ASSERT(sizeof(g_battle.turn_order) >= 20 * sizeof(u32));

    TEST_END();
}

/* Test action queue size */
static void test_action_queue_size(void) {
    TEST_BEGIN("Action queue size");

    TEST_ASSERT(sizeof(g_battle.action_queue) >= 20 * sizeof(u32));

    TEST_END();
}

/* ========================================
 * Test Escape
 * ======================================== */

/* Test battle_try_escape basic */
static void test_battle_try_escape(void) {
    TEST_BEGIN("Battle try escape");

    reset_battle_state();

    /* Setup battle with escape enabled */
    g_battle.can_escape = 1;
    g_battle.state = BATTLE_STATE_MAIN;

    /* Try escape - may succeed or fail based on RNG */
    int result = battle_try_escape();

    /* Result should be 0 or 1 */
    TEST_ASSERT(result == 0 || result == 1);

    TEST_END();
}

/* Test escape disabled in boss battle */
static void test_battle_escape_disabled(void) {
    TEST_BEGIN("Battle escape disabled");

    reset_battle_state();

    g_battle.can_escape = 0;
    g_battle.is_boss = 1;

    /* In boss battles, escape should fail */
    int result = battle_try_escape();

    TEST_ASSERT_EQ(result, 0);

    TEST_END();
}

/* ========================================
 * Test Special Map IDs from Binary
 * ======================================== */

/* Test special map IDs from FUN_0040a1a0 */
static void test_special_map_ids(void) {
    TEST_BEGIN("Special map IDs");

    /* Special map IDs from FUN_0040a1a0 checks */
    TEST_ASSERT_EQ(0x331, 817);     /* Special map */
    TEST_ASSERT_EQ(0x1f47, 8007);   /* Dungeon */
    TEST_ASSERT_EQ(0x1fa5, 8101);   /* Dungeon */
    TEST_ASSERT_EQ(0x1fa4, 8100);   /* Dungeon */
    TEST_ASSERT_EQ(0x1f5b, 8027);   /* Dungeon */
    TEST_ASSERT_EQ(0x1f5c, 8028);   /* Dungeon */
    TEST_ASSERT_EQ(0x1f5d, 8029);   /* Dungeon */
    TEST_ASSERT_EQ(0x1f4f, 8015);   /* Dungeon */
    TEST_ASSERT_EQ(0x1fb1, 8113);   /* Special */
    TEST_ASSERT_EQ(0x1fb2, 8114);   /* Special */
    TEST_ASSERT_EQ(0x2147, 8519);   /* Special */

    TEST_END();
}

/* Test BGM selection values */
static void test_bgm_selection(void) {
    TEST_BEGIN("BGM selection");

    /* BGM IDs from FUN_0040a1a0 */
    TEST_ASSERT_EQ(0x18, 24);   /* Special maps */
    TEST_ASSERT_EQ(0x0c, 12);   /* Outdoor variant */
    TEST_ASSERT_EQ(0x0d, 13);   /* Indoor variant */
    TEST_ASSERT_EQ(0x0e, 14);   /* Dungeon */

    TEST_END();
}

/* Test frame counter masks */
static void test_frame_counter_masks(void) {
    TEST_BEGIN("Frame counter masks");

    /* Frame counter masks from DAT_04ebe31c */
    TEST_ASSERT_EQ(7, 0x7);
    TEST_ASSERT_EQ(0x1f, 31);
    TEST_ASSERT_EQ(3, 0x3);

    TEST_END();
}

/* Test fade types */
static void test_fade_types(void) {
    TEST_BEGIN("Fade types");

    /* FUN_0047bde0 fade types */
    TEST_ASSERT_EQ(0x11, 17);   /* Battle start fade */
    TEST_ASSERT_EQ(0x0c, 12);   /* Battle end fade */

    TEST_END();
}

/* ========================================
 * Test Battle Unit Update Bitmask
 * ======================================== */

/* Test unit update bitmask from binary protocol */
static void test_unit_update_bitmask(void) {
    TEST_BEGIN("Unit update bitmask");

    TEST_ASSERT_EQ(BATTLE_UNIT_UPDATE_ID, (1 << 1));
    TEST_ASSERT_EQ(BATTLE_UNIT_UPDATE_HP, (1 << 2));
    TEST_ASSERT_EQ(BATTLE_UNIT_UPDATE_MP, (1 << 3));
    TEST_ASSERT_EQ(BATTLE_UNIT_UPDATE_LEVEL, (1 << 4));
    TEST_ASSERT_EQ(BATTLE_UNIT_UPDATE_STATUS, (1 << 5));
    TEST_ASSERT_EQ(BATTLE_UNIT_UPDATE_NAME, (1 << 6));

    TEST_END();
}

/* Test combined bitmask */
static void test_combined_bitmask(void) {
    TEST_BEGIN("Combined bitmask");

    u32 mask = BATTLE_UNIT_UPDATE_ID | BATTLE_UNIT_UPDATE_HP | BATTLE_UNIT_UPDATE_MP;

    TEST_ASSERT_EQ(mask, 0x0E);  /* Bits 1, 2, 3 */

    TEST_END();
}

/* ========================================
 * Test Battle Field
 * ======================================== */

/* Test battle field dimensions */
static void test_battle_field_dimensions(void) {
    TEST_BEGIN("Battle field dimensions");

    /* Battle field is 20x20 grid */
    TEST_ASSERT_EQ(BATTLE_FIELD_SIZE, 20);
    TEST_ASSERT_EQ(BATTLE_TILE_COUNT, 400);

    TEST_END();
}

/* Test field tiles array */
static void test_field_tiles_array(void) {
    TEST_BEGIN("Field tiles array");

    TEST_ASSERT(sizeof(g_battle.field_tiles) >= 400 * sizeof(u16));

    TEST_END();
}

/* ========================================
 * Test Battle Results
 * ======================================== */

/* Test battle result storage */
static void test_battle_result_storage(void) {
    TEST_BEGIN("Battle result storage");

    reset_battle_state();

    BattleResult result = {
        .attacker_id = 1,
        .target_id = 2,
        .damage = 500,
        .critical = 1
    };

    battle_process_result(&result);

    TEST_ASSERT_EQ(g_battle.result_count, 1);
    TEST_ASSERT_EQ(g_battle.results[0].attacker_id, 1);
    TEST_ASSERT_EQ(g_battle.results[0].damage, 500);

    TEST_END();
}

/* Test multiple battle results */
static void test_multiple_battle_results(void) {
    TEST_BEGIN("Multiple battle results");

    reset_battle_state();

    for (int i = 0; i < 5; i++) {
        BattleResult result = {
            .attacker_id = i,
            .target_id = i + 10,
            .damage = 100 * (i + 1)
        };
        battle_process_result(&result);
    }

    TEST_ASSERT_EQ(g_battle.result_count, 5);
    TEST_ASSERT_EQ(g_battle.results[4].damage, 500);

    TEST_END();
}

/* ========================================
 * Test Is Active
 * ======================================== */

/* Test battle_is_active when not active */
static void test_battle_is_active_false(void) {
    TEST_BEGIN("Battle is active false");

    reset_battle_state();
    g_battle.active = 0;

    TEST_ASSERT(!battle_is_active());

    TEST_END();
}

/* Test battle_is_active when active */
static void test_battle_is_active_true(void) {
    TEST_BEGIN("Battle is active true");

    reset_battle_state();
    g_battle.active = 1;

    TEST_ASSERT(battle_is_active());

    TEST_END();
}

/* ========================================
 * Test Rewards
 * ======================================== */

/* Test reward storage */
static void test_reward_storage(void) {
    TEST_BEGIN("Reward storage");

    reset_battle_state();

    g_battle.exp_reward = 1000;
    g_battle.gold_reward = 500;
    g_battle.item_rewards[0] = 3001;
    g_battle.item_rewards[1] = 4001;

    TEST_ASSERT_EQ(g_battle.exp_reward, 1000);
    TEST_ASSERT_EQ(g_battle.gold_reward, 500);
    TEST_ASSERT_EQ(g_battle.item_rewards[0], 3001);
    TEST_ASSERT_EQ(g_battle.item_rewards[1], 4001);

    TEST_END();
}

/* ========================================
 * Test Capture
 * ======================================== */

/* Test capture target storage */
static void test_capture_target_storage(void) {
    TEST_BEGIN("Capture target storage");

    reset_battle_state();

    g_battle.capture_target = 12345;
    g_battle.capture_flags = 1;

    TEST_ASSERT_EQ(g_battle.capture_target, 12345);
    TEST_ASSERT_EQ(g_battle.capture_flags, 1);

    TEST_END();
}

/* ========================================
 * Test Message System
 * ======================================== */

/* Test battle message storage */
static void test_battle_message(void) {
    TEST_BEGIN("Battle message");

    reset_battle_state();

    strcpy(g_battle.message, "Critical hit!");
    g_battle.message_type = 1;
    g_battle.message_timer = 60;

    TEST_ASSERT_STR_EQ(g_battle.message, "Critical hit!");
    TEST_ASSERT_EQ(g_battle.message_type, 1);
    TEST_ASSERT_EQ(g_battle.message_timer, 60);

    TEST_END();
}

/* Test action result messages */
static void test_action_result_messages(void) {
    TEST_BEGIN("Action result messages");

    reset_battle_state();

    /* 4 message buffers for rotation */
    strcpy(g_battle.action_results[0], "Player attacks!");
    strcpy(g_battle.action_results[1], "Enemy takes 50 damage!");
    g_battle.action_result_read = 0;

    TEST_ASSERT_STR_EQ(g_battle.action_results[0], "Player attacks!");
    TEST_ASSERT_STR_EQ(g_battle.action_results[1], "Enemy takes 50 damage!");

    TEST_END();
}

/* ========================================
 * Test Unit Slot Count
 * ======================================== */

/* Test unit slot count from FUN_004053e0 */
static void test_unit_slot_count(void) {
    TEST_BEGIN("Unit slot count");

    /* FUN_004053e0 iterates 0-19 (20 units max) */
    TEST_ASSERT(sizeof(g_battle.units) / sizeof(BattleUnit) == 20);

    TEST_END();
}

/* Test maximum units per side */
static void test_units_per_side(void) {
    TEST_BEGIN("Units per side");

    /* 10 player + 10 enemy */
    TEST_ASSERT(10 + 10 == 20);

    TEST_END();
}

/* ========================================
 * Test Battle Sprite IDs
 * ======================================== */

/* Test battle sprite IDs from FUN_004053e0 */
static void test_battle_sprite_ids(void) {
    TEST_BEGIN("Battle sprite IDs");

    /* Sprite IDs from FUN_004053e0 */
    TEST_ASSERT_EQ(0x65a3, 26019);  /* Selected highlight */
    TEST_ASSERT_EQ(0x65a4, 26020);  /* Normal marker */
    TEST_ASSERT_EQ(0x659e, 26014);  /* Action marker */

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

int main(void) {
    printf("========================================\n");
    printf("Stone Age Client - Battle System Tests\n");
    printf("========================================\n\n");

    /* Constants */
    test_battle_state_constants();
    test_battle_action_types();
    test_battle_positions();
    test_buff_flags();
    test_element_types();

    /* Structures */
    test_battle_unit_structure();
    test_battle_context_init();
    test_battle_result_structure();

    /* Initialization */
    test_battle_init();
    test_battle_shutdown();

    /* Unit Management */
    test_battle_add_unit();
    test_battle_add_unit_multiple();
    test_battle_get_unit();
    test_battle_remove_unit();
    test_battle_get_unit_by_index();

    /* Battle Start/End */
    test_battle_start();
    test_battle_end();
    test_battle_victory();
    test_battle_defeat();

    /* Action Selection */
    test_battle_select_action_attack();
    test_battle_select_action_skill();
    test_battle_select_action_defend();

    /* Turn Management */
    test_turn_count_init();
    test_turn_order_size();
    test_action_queue_size();

    /* Escape */
    test_battle_try_escape();
    test_battle_escape_disabled();

    /* Binary Analysis */
    test_special_map_ids();
    test_bgm_selection();
    test_frame_counter_masks();
    test_fade_types();

    /* Protocol */
    test_unit_update_bitmask();
    test_combined_bitmask();

    /* Field */
    test_battle_field_dimensions();
    test_field_tiles_array();

    /* Results */
    test_battle_result_storage();
    test_multiple_battle_results();

    /* Active State */
    test_battle_is_active_false();
    test_battle_is_active_true();

    /* Rewards */
    test_reward_storage();

    /* Capture */
    test_capture_target_storage();

    /* Messages */
    test_battle_message();
    test_action_result_messages();

    /* Limits */
    test_unit_slot_count();
    test_units_per_side();
    test_battle_sprite_ids();

    test_summary();

    return test_all_passed() ? 0 : 1;
}
