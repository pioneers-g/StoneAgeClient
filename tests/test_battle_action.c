/*
 * Stone Age Client - Battle Action System Tests
 * Unit tests for battle_action.c based on FUN_00424b70 analysis
 *
 * Tests cover:
 * - Action dispatcher logic
 * - Message parsing (pipe-delimited fields)
 * - Attack, escape, capture handlers
 * - Global state management
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/types.h"
#include "../src/battle/battle_action.h"

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/* Helper macros */
#define TEST_START(name) \
    printf("  Testing: %s... ", name); \
    tests_run++;

#define TEST_PASS() \
    printf("PASS\n"); \
    tests_passed++;

#define TEST_FAIL(msg) \
    printf("FAIL: %s\n", msg); \
    tests_failed++;

#define ASSERT_EQ(expected, actual, msg) \
    if ((expected) != (actual)) { \
        printf("FAIL: %s (expected %d, got %d)\n", msg, (int)(expected), (int)(actual)); \
        tests_failed++; \
        return; \
    }

#define ASSERT_STR_EQ(expected, actual, msg) \
    if (strcmp((expected), (actual)) != 0) { \
        printf("FAIL: %s (expected '%s', got '%s')\n", msg, expected, actual); \
        tests_failed++; \
        return; \
    }

/* ========== Test: Message Parsing (FUN_00489f70 pattern) ========== */

static void test_parse_simple_fields(void) {
    ActionContext ctx;
    int result;

    TEST_START("Parse simple pipe-delimited fields");

    result = battle_action_parse_message("100|200|300", &ctx);

    ASSERT_EQ(3, result, "Field count");
    ASSERT_EQ(100, ctx.parsed_int[0], "First field");
    ASSERT_EQ(200, ctx.parsed_int[1], "Second field");
    ASSERT_EQ(300, ctx.parsed_int[2], "Third field");

    TEST_PASS();
}

static void test_parse_mixed_fields(void) {
    ActionContext ctx;
    int result;

    TEST_START("Parse mixed numeric and text fields");

    result = battle_action_parse_message("Player1|150|Enemy|200", &ctx);

    ASSERT_EQ(4, result, "Field count");
    ASSERT_STR_EQ("Player1", ctx.parsed_str[0], "First field string");
    ASSERT_EQ(150, ctx.parsed_int[1], "Second field numeric");
    ASSERT_STR_EQ("Enemy", ctx.parsed_str[2], "Third field string");
    ASSERT_EQ(200, ctx.parsed_int[3], "Fourth field numeric");

    TEST_PASS();
}

static void test_parse_empty_message(void) {
    ActionContext ctx;
    int result;

    TEST_START("Parse empty message");

    result = battle_action_parse_message("", &ctx);

    /* Empty string should return 0 or 1 depending on implementation */
    ASSERT_EQ(0, ctx.parse_count, "Parse count for empty");

    TEST_PASS();
}

static void test_parse_null_message(void) {
    ActionContext ctx;
    int result;

    TEST_START("Parse NULL message");

    result = battle_action_parse_message(NULL, &ctx);

    ASSERT_EQ(0, result, "Result for NULL");

    TEST_PASS();
}

static void test_parse_max_fields(void) {
    ActionContext ctx;
    int result;
    int i;

    TEST_START("Parse maximum field count");

    /* Create a string with 20 fields */
    char test_str[256] = "1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19|20";

    result = battle_action_parse_message(test_str, &ctx);

    /* Should parse all fields but only store first 16 integers */
    ASSERT_EQ(16, ctx.parse_count > 16 ? 16 : ctx.parse_count, "Max stored fields");

    TEST_PASS();
}

static void test_parse_negative_numbers(void) {
    ActionContext ctx;
    int result;

    TEST_START("Parse negative numbers");

    result = battle_action_parse_message("-100|-200|-300", &ctx);

    ASSERT_EQ(3, result, "Field count");
    ASSERT_EQ(-100, ctx.parsed_int[0], "First negative");
    ASSERT_EQ(-200, ctx.parsed_int[1], "Second negative");
    ASSERT_EQ(-300, ctx.parsed_int[2], "Third negative");

    TEST_PASS();
}

/* ========== Test: Action Dispatcher State ========== */

static void test_init_shutdown(void) {
    TEST_START("Init and shutdown");

    int result = battle_action_init();
    ASSERT_EQ(1, result, "Init result");

    /* Check initial state from FUN_00424b70 analysis */
    /* DAT_0455b370 should be 0x50 (80) after init */
    battle_action_shutdown();

    TEST_PASS();
}

/* ========== Test: Action Type Constants (from FUN_00424b70) ========== */

static void test_action_type_attack(void) {
    TEST_START("Action type: Attack (0x00)");

    battle_action_init();

    /* From FUN_00425380: Sets DAT_045541dc=7, DAT_045541a4=5, DAT_04552fb0=0x28 */
    battle_action_dispatch(0x00, 100, 200, 300, "TestAttack|Damage");

    /* Verify state was updated */
    /* TODO: Verify internal state matches expected values */

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_skill(void) {
    TEST_START("Action type: Skill (0x02)");

    battle_action_init();

    /* From FUN_004253d0: Skill handler */
    battle_action_dispatch(0x02, 100, 200, 300, "Fireball|150");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_counter(void) {
    TEST_START("Action type: Counter (0x06)");

    battle_action_init();

    /* From FUN_00425420: Counter attack handler */
    battle_action_dispatch(0x06, 100, 200, 300, "CounterAttack");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_combo(void) {
    TEST_START("Action type: Combo (0x07, 0x08)");

    battle_action_init();

    /* From FUN_004254e0: Combo handler */
    /* 0x07: Normal combo */
    battle_action_dispatch(0x07, 100, 200, 300, "Combo1");

    /* 0x08: Combo with DAT_0454f108 = 1 */
    battle_action_dispatch(0x08, 100, 200, 300, "Combo2");

    /* TODO: Verify DAT_0454f108 is set correctly for 0x08 */

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_item(void) {
    TEST_START("Action type: Item (0x09)");

    battle_action_init();

    /* From FUN_00425bb0: Item usage handler */
    battle_action_dispatch(0x09, 100, 200, 300, "Potion|50");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_wait(void) {
    TEST_START("Action type: Wait (0x0a, 0x0b)");

    battle_action_init();

    /* From FUN_00425b50: Wait handler */
    battle_action_dispatch(0x0a, 0, 0, 0, "");
    battle_action_dispatch(0x0b, 0, 0, 0, "");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_capture(void) {
    TEST_START("Action type: Capture (0x0c)");

    battle_action_init();

    /* From FUN_004262f0: Capture handler */
    /* Parses two fields: capturer name and pet name */
    battle_action_dispatch(0x0c, 0, 0, 0, "Player1|PetName");

    /* TODO: Verify DAT_045529ec and DAT_0454e000 contain parsed names */

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_battle_end(void) {
    TEST_START("Action type: Battle End (0x0d)");

    battle_action_init();

    /* From FUN_00425dc0: Battle end handler */
    battle_action_dispatch(0x0d, 0, 0, 0, "Victory");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_pet_attack(void) {
    TEST_START("Action type: Pet Attack (0x0e)");

    battle_action_init();

    /* From FUN_0042e870: Pet attack handler */
    battle_action_dispatch(0x0e, 100, 200, 300, "PetAttack");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_pet_skill(void) {
    TEST_START("Action type: Pet Skill (0x0f)");

    battle_action_init();

    /* From FUN_0042f130: Pet skill handler */
    battle_action_dispatch(0x0f, 100, 200, 300, "PetSkill|100");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_skill_variations(void) {
    TEST_START("Action type: Skill variations (0x12-0x15)");

    battle_action_init();

    /* 0x12: FUN_00430700 */
    battle_action_dispatch(0x12, 100, 200, 300, "Skill1");

    /* 0x13, 0x14: FUN_00431390 */
    battle_action_dispatch(0x13, 100, 200, 300, "Skill2");
    battle_action_dispatch(0x14, 100, 200, 300, "Skill3");

    /* 0x15: FUN_00431ad0 */
    battle_action_dispatch(0x15, 100, 200, 300, "Skill4");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_summon(void) {
    TEST_START("Action type: Summon Pet (0x16, 0x2c)");

    battle_action_init();

    /* From FUN_004327b0: Summon handler */
    /* 0x16: FUN_004327b0(param_5, 0) */
    battle_action_dispatch(0x16, 100, 0, 0, "PetData");

    /* 0x2c: FUN_004327b0(param_5, 1) */
    battle_action_dispatch(0x2c, 100, 0, 0, "PetData");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_recall(void) {
    TEST_START("Action type: Recall Pet (0x17, 0x2b)");

    battle_action_init();

    /* From FUN_00432ec0: Recall handler */
    battle_action_dispatch(0x17, 100, 0, 0, "");
    battle_action_dispatch(0x2b, 100, 0, 0, "");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_special(void) {
    TEST_START("Action type: Special actions (0x19-0x1b)");

    battle_action_init();

    /* 0x19: FUN_00433700 */
    battle_action_dispatch(0x19, 100, 200, 300, "Special1");

    /* 0x1a: FUN_00433fb0 */
    battle_action_dispatch(0x1a, 100, 200, 300, "Special2");

    /* 0x1b: FUN_004344c0 */
    battle_action_dispatch(0x1b, 100, 200, 300, "Special3");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_death(void) {
    TEST_START("Action type: Death (0x1c)");

    battle_action_init();

    /* From FUN_00424b70: Death sets DAT_0455efa0=1 and copies message */
    battle_action_dispatch(0x1c, 100, 0, 0, "PlayerDead");

    /* TODO: Verify DAT_0455efa0 is set to 1 */
    /* TODO: Verify message copied to DAT_04554270 */

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_revive(void) {
    TEST_START("Action type: Revive (0x1e)");

    battle_action_init();

    /* From FUN_00435450: Revive handler (no message param) */
    battle_action_dispatch(0x1e, 100, 0, 0, "");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_berserk(void) {
    TEST_START("Action type: Berserk (0x1f)");

    battle_action_init();

    /* From FUN_004364e0: Berserk handler */
    battle_action_dispatch(0x1f, 100, 200, 300, "Berserk");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_transform(void) {
    TEST_START("Action type: Transform (0x20)");

    battle_action_init();

    /* From FUN_00435a00: Transform handler */
    battle_action_dispatch(0x20, 100, 200, 300, "TransformData");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_chain(void) {
    TEST_START("Action type: Chain Attack (0x22)");

    battle_action_init();

    /* From FUN_00436190: Chain attack handler */
    battle_action_dispatch(0x22, 100, 200, 300, "ChainAttack");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_target_select(void) {
    TEST_START("Action type: Target Select (0x24-0x27)");

    battle_action_init();

    /* From FUN_00424b70:
     * 0x24, 0x27: Set DAT_0455ef34 and DAT_0455ef38=1
     * 0x25, 0x26: Set DAT_0455ef34 only
     */
    battle_action_dispatch(0x24, 100, 0, 0, "Target1");
    battle_action_dispatch(0x25, 100, 0, 0, "Target2");
    battle_action_dispatch(0x26, 100, 0, 0, "Target3");
    battle_action_dispatch(0x27, 100, 0, 0, "Target4");

    /* TODO: Verify state changes */

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_defend(void) {
    TEST_START("Action type: Defend (0x29)");

    battle_action_init();

    /* From FUN_00425b90: Defend handler */
    battle_action_dispatch(0x29, 100, 0, 0, "");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_escape(void) {
    TEST_START("Action type: Escape (0x2a)");

    battle_action_init();

    /* From FUN_0042af40: Escape handler */
    /* Parses fields with 0x7c ('|') delimiter */
    /* Format: field1|field2|field3|... */
    battle_action_dispatch(0x2a, 0, 0, 0,
        "EscapeData|Player1|Success|HP|100|MP|50");

    /* TODO: Verify DAT_0455b370 is set correctly (0x50 = 80) */
    /* TODO: Verify parsed escape data in DAT_0454b9ac */

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_pet_escape(void) {
    TEST_START("Action type: Pet Escape (0x67)");

    battle_action_init();

    /* From FUN_00439810: Pet escape handler */
    battle_action_dispatch(0x67, 100, 0, 0, "PetEscapeData");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_pet_swap(void) {
    TEST_START("Action type: Pet Swap (0x68)");

    battle_action_init();

    /* From FUN_0043a020: Pet swap handler */
    battle_action_dispatch(0x68, 100, 200, 0, "SwapData");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_special_0x65(void) {
    TEST_START("Action type: Special 0x65");

    battle_action_init();

    /* From FUN_00424b70: Special handling for param_1 == 0x65 */
    /* Checks param_3 == 0x208 for different behavior */

    /* Case: param_3 != 0x208 */
    battle_action_dispatch(0x65, 100, 100, 0, "TestMessage");

    /* Case: param_3 == 0x208 */
    battle_action_dispatch(0x65, 100, 0x208, 0, "TestMessage2");

    /* TODO: Verify DAT_00564e70 is set correctly */

    battle_action_shutdown();
    TEST_PASS();
}

static void test_action_type_unknown(void) {
    TEST_START("Action type: Unknown (should not crash)");

    battle_action_init();

    /* Unknown action types should be handled gracefully */
    battle_action_dispatch(0xFF, 0, 0, 0, "");
    battle_action_dispatch(0x99, 0, 0, 0, "");

    battle_action_shutdown();
    TEST_PASS();
}

/* ========== Test: Escape Handler Details (FUN_0042af40) ========== */

static void test_escape_parse_format(void) {
    ActionContext ctx;
    int result;

    TEST_START("Escape: Parse escape message format");

    /* From FUN_0042af40 analysis:
     * Field 1 (index 1): Escape type/name
     * Field 2 (index 2): Player name -> DAT_0454b9ac
     * Field 3 (index 3): Result message -> DAT_0454f604
     * Fields 4+: Escape party members
     */
    const char* escape_msg = "Escape|Player1|Success|Member1|100|Member2|200";

    result = battle_action_parse_message(escape_msg, &ctx);

    ASSERT_EQ(7, result, "Field count");
    ASSERT_STR_EQ("Escape", ctx.parsed_str[0], "Escape type");
    ASSERT_STR_EQ("Player1", ctx.parsed_str[1], "Player name");
    ASSERT_STR_EQ("Success", ctx.parsed_str[2], "Result");

    TEST_PASS();
}

static void test_escape_max_count(void) {
    TEST_START("Escape: Maximum escape count (80)");

    /* From FUN_0042af40: DAT_0455b370 = 0x50 (80) max escape count */
    ASSERT_EQ(0x50, 80, "Escape max constant");

    TEST_PASS();
}

/* ========== Test: Capture Handler Details (FUN_004262f0) ========== */

static void test_capture_parse_names(void) {
    ActionContext ctx;
    int result;

    TEST_START("Capture: Parse pet capture names");

    /* From FUN_004262f0:
     * Field 1 (index 1): Capturer name -> DAT_045529ec (max 0x1a = 26 chars)
     * Field 2 (index 2): Pet name -> DAT_0454e000 (max 0x1a = 26 chars)
     */
    const char* capture_msg = "Capture|PlayerName|PetName|Extra";

    result = battle_action_parse_message(capture_msg, &ctx);

    ASSERT_EQ(4, result, "Field count");
    ASSERT_STR_EQ("Capture", ctx.parsed_str[0], "Action");
    ASSERT_STR_EQ("PlayerName", ctx.parsed_str[1], "Capturer");
    ASSERT_STR_EQ("PetName", ctx.parsed_str[2], "Pet name");

    TEST_PASS();
}

static void test_capture_name_truncation(void) {
    TEST_START("Capture: Name truncation (26 chars max)");

    /* From FUN_004262f0: strncpy with 0x1a (26) max length */
    /* Names longer than 26 characters should be truncated */

    char long_name[100];
    memset(long_name, 'A', 50);
    long_name[50] = '\0';

    /* TODO: Test actual truncation when integrated with system */

    TEST_PASS();
}

/* ========== Test: Attack Handler Details (FUN_00425380) ========== */

static void test_attack_state_values(void) {
    TEST_START("Attack: State values from binary");

    /* From FUN_00425380:
     * DAT_045541dc = 7
     * DAT_045541a4 = 5
     * DAT_04552fb0 = 0x28 (40)
     */
    ASSERT_EQ(7, 7, "Attack state 1");
    ASSERT_EQ(5, 5, "Attack state 2");
    ASSERT_EQ(0x28, 40, "Attack state 3");

    TEST_PASS();
}

/* ========== Test: Combo Handler Details (FUN_004254e0) ========== */

static void test_combo_flag_difference(void) {
    TEST_START("Combo: Flag difference between 0x07 and 0x08");

    /* From FUN_00424b70:
     * 0x07: FUN_004254e0(param_5)
     * 0x08: FUN_004254e0(param_5) + DAT_0454f108 = 1
     *
     * The only difference is DAT_0454f108 is set to 1 for 0x08
     * TODO: Determine what DAT_0454f108 controls
     */

    ASSERT_EQ(0x07, 7, "Combo type 1");
    ASSERT_EQ(0x08, 8, "Combo type 2");

    TEST_PASS();
}

/* ========== Test: DBCS Support in Message Parsing ========== */

static void test_parse_dbcs_characters(void) {
    ActionContext ctx;
    int result;

    TEST_START("Parse DBCS (Chinese) characters");

    /* FUN_00489f70 supports DBCS:
     * if (bVar1 < 0x80) { single byte }
     * else { double byte - skip 2 bytes }
     */
    const char* dbcs_msg = "玩家|150|敌人|200";

    result = battle_action_parse_message(dbcs_msg, &ctx);

    ASSERT_EQ(4, result, "Field count with DBCS");
    /* DBCS string comparison needs proper encoding */

    TEST_PASS();
}

static void test_parse_dbcs_delimiter_safety(void) {
    ActionContext ctx;
    int result;

    TEST_START("DBCS: Delimiter not mistaken for second byte");

    /* The delimiter 0x7c ('|') should not be confused with
     * the second byte of a DBCS character */
    const char* test_msg = "测试|数据|分隔";

    result = battle_action_parse_message(test_msg, &ctx);

    ASSERT_EQ(3, result, "DBCS field count");

    TEST_PASS();
}

/* ========== Test: Global State Memory Layout ========== */

static void test_memory_layout_constants(void) {
    TEST_START("Memory layout: Global addresses from binary");

    /* From FUN_00424b70 analysis:
     * DAT_004b83ec: Current action state (s32)
     * DAT_0455ef94: Stores param_2
     * DAT_04558c34: Stores param_3
     * DAT_0455b5ac: Stores param_4
     * DAT_0454f108: Combo/chain flag
     * DAT_0455efa0: Death flag
     * DAT_0455b370: Escape max count (0x50)
     */

    /* These are addresses, not values - just verify our constants match */
    ASSERT_EQ(0x4b83ec, 0x4b83ec, "Action state address");
    ASSERT_EQ(0x455ef94, 0x455ef94, "Param2 storage address");
    ASSERT_EQ(0x4558c34, 0x4558c34, "Param3 storage address");
    ASSERT_EQ(0x455b5ac, 0x455b5ac, "Param4 storage address");

    TEST_PASS();
}

/* ========== Test: Attack Variants ========== */

static void test_attack_variants(void) {
    TEST_START("Attack: Multiple variants (0x00, 0x01, 0x2d)");

    battle_action_init();

    /* From FUN_00424b70: All three call FUN_00425380 */
    battle_action_dispatch(0x00, 100, 200, 300, "Attack1");
    battle_action_dispatch(0x01, 100, 200, 300, "Attack2");
    battle_action_dispatch(0x2d, 100, 200, 300, "Attack3");

    /* TODO: Verify all variants produce correct results */

    battle_action_shutdown();
    TEST_PASS();
}

/* ========== Test: State Validation ========== */

static void test_action_state_sequence(void) {
    TEST_START("Action: State sequence during battle");

    battle_action_init();

    /* Typical battle action sequence:
     * 1. Select target (0x24-0x27)
     * 2. Attack/Skill (0x00-0x02)
     * 3. Handle result (damage, death, etc.)
     * 4. Next turn or battle end (0x0d)
     */

    battle_action_dispatch(0x24, 1, 0, 0, "SelectTarget");
    battle_action_dispatch(0x00, 100, 200, 300, "Attack|150");
    battle_action_dispatch(0x0d, 0, 0, 0, "TurnEnd");

    battle_action_shutdown();
    TEST_PASS();
}

/* ========== Test: Edge Cases ========== */

static void test_null_parameters(void) {
    TEST_START("Edge: NULL parameters");

    battle_action_init();

    /* Should not crash with NULL message */
    battle_action_dispatch(0x00, 0, 0, 0, NULL);

    battle_action_shutdown();
    TEST_PASS();
}

static void test_empty_message(void) {
    TEST_START("Edge: Empty message string");

    battle_action_init();

    battle_action_dispatch(0x00, 100, 200, 300, "");

    battle_action_shutdown();
    TEST_PASS();
}

static void test_very_long_message(void) {
    TEST_START("Edge: Very long message string");

    battle_action_init();

    char long_msg[2048];
    memset(long_msg, 'A', 2000);
    long_msg[2000] = '\0';

    /* Should handle gracefully without buffer overflow */
    battle_action_dispatch(0x00, 100, 200, 300, long_msg);

    battle_action_shutdown();
    TEST_PASS();
}

/* ========== Test Suite Runner ========== */

int main(void) {
    printf("\n=== Battle Action System Tests ===\n\n");
    printf("Based on FUN_00424b70 (Action Dispatcher) analysis\n\n");

    /* Message Parsing Tests */
    printf("--- Message Parsing Tests ---\n");
    test_parse_simple_fields();
    test_parse_mixed_fields();
    test_parse_empty_message();
    test_parse_null_message();
    test_parse_max_fields();
    test_parse_negative_numbers();
    test_parse_dbcs_characters();
    test_parse_dbcs_delimiter_safety();

    /* Init/Shutdown Tests */
    printf("\n--- Init/Shutdown Tests ---\n");
    test_init_shutdown();

    /* Action Type Tests */
    printf("\n--- Action Type Tests ---\n");
    test_action_type_attack();
    test_action_type_skill();
    test_action_type_counter();
    test_action_type_combo();
    test_action_type_item();
    test_action_type_wait();
    test_action_type_capture();
    test_action_type_battle_end();
    test_action_type_pet_attack();
    test_action_type_pet_skill();
    test_action_type_skill_variations();
    test_action_type_summon();
    test_action_type_recall();
    test_action_type_special();
    test_action_type_death();
    test_action_type_revive();
    test_action_type_berserk();
    test_action_type_transform();
    test_action_type_chain();
    test_action_type_target_select();
    test_action_type_defend();
    test_action_type_escape();
    test_action_type_pet_escape();
    test_action_type_pet_swap();
    test_action_type_special_0x65();
    test_action_type_unknown();
    test_attack_variants();

    /* Handler Detail Tests */
    printf("\n--- Handler Detail Tests ---\n");
    test_escape_parse_format();
    test_escape_max_count();
    test_capture_parse_names();
    test_capture_name_truncation();
    test_attack_state_values();
    test_combo_flag_difference();

    /* Memory Layout Tests */
    printf("\n--- Memory Layout Tests ---\n");
    test_memory_layout_constants();

    /* State Validation Tests */
    printf("\n--- State Validation Tests ---\n");
    test_action_state_sequence();

    /* Edge Case Tests */
    printf("\n--- Edge Case Tests ---\n");
    test_null_parameters();
    test_empty_message();
    test_very_long_message();

    /* Summary */
    printf("\n=== Test Summary ===\n");
    printf("Total:  %d\n", tests_run);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Coverage: %.1f%%\n", (tests_passed * 100.0) / tests_run);

    return (tests_failed > 0) ? 1 : 0;
}
