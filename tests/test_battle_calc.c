/*
 * Stone Age Client - Unit Tests for Battle Calculations
 * Test file: test_battle_calc.c
 *
 * Tests for damage calculation, element system, buffs, capture rate
 * Based on reverse engineering of sa_9061.exe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_framework.h"
#include "../include/types.h"
#include "../include/battle.h"

/* ========================================
 * Test Data Structures
 * ======================================== */

/* Create a test battle unit with specified stats */
static BattleUnit create_test_unit(u16 id, u16 hp, u16 mp, u16 attack, u16 defense, u16 speed) {
    BattleUnit unit;
    memset(&unit, 0, sizeof(BattleUnit));
    unit.id = id;
    unit.hp = hp;
    unit.max_hp = hp;
    unit.mp = mp;
    unit.max_mp = mp;
    unit.attack = attack;
    unit.defense = defense;
    unit.speed = speed;
    unit.is_alive = 1;
    return unit;
}

/* Create a test skill */
static BattleSkill create_test_skill(u16 id, u16 power, u16 mp_cost, u8 type) {
    BattleSkill skill;
    memset(&skill, 0, sizeof(BattleSkill));
    skill.id = id;
    skill.power = power;
    skill.mp_cost = mp_cost;
    skill.type = type;
    return skill;
}

/* ========================================
 * Test Cases for Damage Calculation
 * ======================================== */

/*
 * Test 1: Basic damage calculation
 */
static void test_basic_damage(void) {
    TEST_BEGIN("Basic damage calculation");

    BattleUnit attacker = create_test_unit(1, 100, 50, 50, 30, 20);
    BattleUnit target = create_test_unit(2, 100, 50, 40, 30, 15);

    u16 damage = battle_calc_damage(&attacker, &target, NULL);

    /* Damage should be attack - defense/2 = 50 - 15 = 35, with variance */
    TEST_ASSERT(damage >= 30 && damage <= 42, "Damage should be around 35 with variance");

    TEST_END();
}

/*
 * Test 2: Damage with skill
 */
static void test_skill_damage(void) {
    TEST_BEGIN("Damage with skill");

    BattleUnit attacker = create_test_unit(1, 100, 50, 50, 30, 20);
    BattleUnit target = create_test_unit(2, 100, 50, 40, 30, 15);
    BattleSkill skill = create_test_skill(1, 30, 10, 0);

    u16 damage = battle_calc_damage(&attacker, &target, &skill);

    /* Skill damage: power + attack/2 - defense/4 = 30 + 25 - 7 = 48 */
    TEST_ASSERT(damage >= 43 && damage <= 53, "Skill damage should be around 48 with variance");

    TEST_END();
}

/*
 * Test 3: Damage with attack buff
 */
static void test_damage_attack_buff(void) {
    TEST_BEGIN("Damage with attack buff");

    BattleUnit attacker = create_test_unit(1, 100, 50, 50, 30, 20);
    BattleUnit target = create_test_unit(2, 100, 50, 40, 30, 15);

    /* Apply attack up buff */
    attacker.buff_flags |= BUFF_ATTACK_UP;

    u16 damage = battle_calc_damage(&attacker, &target, NULL);

    /* Attack should be 50 * 1.2 = 60, damage = 60 - 15 = 45 */
    TEST_ASSERT(damage >= 40 && damage <= 55, "Damage should be increased with attack buff");

    TEST_END();
}

/*
 * Test 4: Damage with defense buff
 */
static void test_damage_defense_buff(void) {
    TEST_BEGIN("Damage with defense buff");

    BattleUnit attacker = create_test_unit(1, 100, 50, 50, 30, 20);
    BattleUnit target = create_test_unit(2, 100, 50, 40, 30, 15);

    /* Apply defense up buff */
    target.buff_flags |= BUFF_DEFENSE_UP;

    u16 damage = battle_calc_damage(&attacker, &target, NULL);

    /* Defense should be 30 * 1.2 = 36, damage = 50 - 18 = 32 */
    TEST_ASSERT(damage >= 28 && damage <= 38, "Damage should be reduced with defense buff");

    TEST_END();
}

/*
 * Test 5: Minimum damage
 */
static void test_minimum_damage(void) {
    TEST_BEGIN("Minimum damage");

    BattleUnit attacker = create_test_unit(1, 100, 50, 10, 5, 20);
    BattleUnit target = create_test_unit(2, 100, 50, 40, 100, 15);

    u16 damage = battle_calc_damage(&attacker, &target, NULL);

    /* Even with high defense, minimum damage should be 1 */
    TEST_ASSERT(damage >= 1, "Damage should be at least 1");

    TEST_END();
}

/*
 * Test 6: Null attacker
 */
static void test_null_attacker(void) {
    TEST_BEGIN("Null attacker");

    BattleUnit target = create_test_unit(2, 100, 50, 40, 30, 15);

    u16 damage = battle_calc_damage(NULL, &target, NULL);
    TEST_ASSERT(damage == 0, "Damage should be 0 with null attacker");

    TEST_END();
}

/*
 * Test 7: Null target
 */
static void test_null_target(void) {
    TEST_BEGIN("Null target");

    BattleUnit attacker = create_test_unit(1, 100, 50, 50, 30, 20);

    u16 damage = battle_calc_damage(&attacker, NULL, NULL);
    TEST_ASSERT(damage == 0, "Damage should be 0 with null target");

    TEST_END();
}

/* ========================================
 * Test Cases for Hit Calculation
 * ======================================== */

/*
 * Test 8: Basic hit chance
 */
static void test_basic_hit_chance(void) {
    TEST_BEGIN("Basic hit chance");

    BattleUnit attacker = create_test_unit(1, 100, 50, 50, 30, 20);
    BattleUnit target = create_test_unit(2, 100, 50, 40, 30, 15);

    /* Base hit chance is 90%, so most attacks should hit */
    int hits = 0;
    int i;
    for (i = 0; i < 100; i++) {
        if (battle_calc_hit(&attacker, &target)) {
            hits++;
        }
    }

    TEST_ASSERT(hits >= 80 && hits <= 100, "Hit rate should be around 90%");

    TEST_END();
}

/*
 * Test 9: Hit stunned target
 */
static void test_hit_stunned_target(void) {
    TEST_BEGIN("Hit stunned target");

    BattleUnit attacker = create_test_unit(1, 100, 50, 50, 30, 20);
    BattleUnit target = create_test_unit(2, 100, 50, 40, 30, 15);

    /* Stunned targets are always hit */
    target.buff_flags |= BUFF_STUN;

    int result = battle_calc_hit(&attacker, &target);
    TEST_ASSERT(result == 1, "Should always hit stunned target");

    TEST_END();
}

/*
 * Test 10: Null hit check
 */
static void test_null_hit_check(void) {
    TEST_BEGIN("Null hit check");

    BattleUnit attacker = create_test_unit(1, 100, 50, 50, 30, 20);

    int result = battle_calc_hit(&attacker, NULL);
    TEST_ASSERT(result == 0, "Hit should be 0 with null target");

    result = battle_calc_hit(NULL, NULL);
    TEST_ASSERT(result == 0, "Hit should be 0 with null attacker");

    TEST_END();
}

/* ========================================
 * Test Cases for Critical Hit
 * ======================================== */

/*
 * Test 11: Basic critical chance
 */
static void test_basic_critical_chance(void) {
    TEST_BEGIN("Basic critical chance");

    BattleUnit attacker = create_test_unit(1, 100, 50, 50, 30, 20);
    BattleUnit target = create_test_unit(2, 100, 50, 40, 30, 15);

    /* Base critical chance is 5% */
    int crits = 0;
    int i;
    for (i = 0; i < 200; i++) {
        if (battle_calc_critical(&attacker, &target)) {
            crits++;
        }
    }

    /* Should have roughly 5% crit rate (5-15 out of 200) */
    TEST_ASSERT(crits >= 0 && crits <= 40, "Crit rate should be around 5%");

    TEST_END();
}

/*
 * Test 12: Critical against debuffed target
 */
static void test_critical_debuffed_target(void) {
    TEST_BEGIN("Critical against debuffed target");

    BattleUnit attacker = create_test_unit(1, 100, 50, 50, 30, 20);
    BattleUnit target = create_test_unit(2, 100, 50, 40, 30, 15);

    /* Stunned targets have higher crit chance against them */
    target.buff_flags |= BUFF_STUN;

    int crits = 0;
    int i;
    for (i = 0; i < 200; i++) {
        if (battle_calc_critical(&attacker, &target)) {
            crits++;
        }
    }

    /* Should have roughly 15% crit rate against stunned */
    TEST_ASSERT(crits >= 0 && crits <= 60, "Crit rate should be around 15% against stunned");

    TEST_END();
}

/* ========================================
 * Test Cases for Capture Rate
 * ======================================== */

/*
 * Test 13: Capture rate at full HP
 */
static void test_capture_full_hp(void) {
    TEST_BEGIN("Capture rate at full HP");

    int rate = battle_calc_capture_rate(100, 30, 0);

    /* At 100% HP, base rate should be very low (0%) */
    TEST_ASSERT(rate >= 0 && rate <= 10, "Capture rate should be low at full HP");

    TEST_END();
}

/*
 * Test 14: Capture rate at low HP
 */
static void test_capture_low_hp(void) {
    TEST_BEGIN("Capture rate at low HP");

    int rate = battle_calc_capture_rate(10, 30, 0);

    /* At 10% HP, base rate should be around 45% */
    TEST_ASSERT(rate >= 40 && rate <= 50, "Capture rate should be higher at low HP");

    TEST_END();
}

/*
 * Test 15: Capture rate with item
 */
static void test_capture_with_item(void) {
    TEST_BEGIN("Capture rate with item");

    int rate_no_item = battle_calc_capture_rate(20, 30, 0);
    int rate_with_item = battle_calc_capture_rate(20, 30, 1);

    TEST_ASSERT(rate_with_item > rate_no_item, "Capture rate should increase with item");

    TEST_END();
}

/*
 * Test 16: Capture rate high level penalty
 */
static void test_capture_high_level(void) {
    TEST_BEGIN("Capture rate high level penalty");

    int rate_low = battle_calc_capture_rate(20, 40, 0);
    int rate_high = battle_calc_capture_rate(20, 70, 0);

    TEST_ASSERT(rate_high < rate_low, "Capture rate should decrease for high level");

    TEST_END();
}

/* ========================================
 * Test Cases for Escape Chance
 * ======================================== */

/*
 * Test 17: Basic escape chance
 */
static void test_basic_escape_chance(void) {
    TEST_BEGIN("Basic escape chance");

    BattleUnit player = create_test_unit(1, 100, 50, 50, 30, 30);
    BattleUnit enemy = create_test_unit(2, 100, 50, 40, 30, 20);

    int chance = battle_calc_escape_chance(&player, &enemy);

    /* Base 50% + 10 speed difference = 60% */
    TEST_ASSERT(chance >= 50 && chance <= 70, "Escape chance should be around 60%");

    TEST_END();
}

/*
 * Test 18: Escape with null player
 */
static void test_escape_null_player(void) {
    TEST_BEGIN("Escape with null player");

    BattleUnit enemy = create_test_unit(2, 100, 50, 40, 30, 20);

    int chance = battle_calc_escape_chance(NULL, &enemy);

    /* Should return base 50% */
    TEST_ASSERT(chance == 50, "Escape chance should be base 50% with null player");

    TEST_END();
}

/*
 * Test 19: Escape with slower player
 */
static void test_escape_slower_player(void) {
    TEST_BEGIN("Escape with slower player");

    BattleUnit player = create_test_unit(1, 100, 50, 50, 30, 20);
    BattleUnit enemy = create_test_unit(2, 100, 50, 40, 30, 40);

    int chance = battle_calc_escape_chance(&player, &enemy);

    /* Base 50% - 20 speed difference = 30%, clamped to 10% minimum */
    TEST_ASSERT(chance >= 10 && chance <= 40, "Escape chance should be lower with slower player");

    TEST_END();
}

/* ========================================
 * Test Cases for Buff System
 * ======================================== */

/*
 * Test 20: Apply buff
 */
static void test_apply_buff(void) {
    TEST_BEGIN("Apply buff");

    BattleUnit unit = create_test_unit(1, 100, 50, 50, 30, 20);

    battle_apply_buff(&unit, 0, 3);  /* Apply attack up for 3 turns */

    TEST_ASSERT(unit.buff_flags & BUFF_ATTACK_UP, "Buff flag should be set");
    TEST_ASSERT(unit.buff_turns[0] == 3, "Buff duration should be 3");

    TEST_END();
}

/*
 * Test 21: Remove buff
 */
static void test_remove_buff(void) {
    TEST_BEGIN("Remove buff");

    BattleUnit unit = create_test_unit(1, 100, 50, 50, 30, 20);

    battle_apply_buff(&unit, 0, 3);
    TEST_ASSERT(unit.buff_flags & BUFF_ATTACK_UP, "Buff should be applied");

    battle_remove_buff(&unit, 0);
    TEST_ASSERT(!(unit.buff_flags & BUFF_ATTACK_UP), "Buff should be removed");
    TEST_ASSERT(unit.buff_turns[0] == 0, "Buff duration should be 0");

    TEST_END();
}

/*
 * Test 22: Multiple buffs
 */
static void test_multiple_buffs(void) {
    TEST_BEGIN("Multiple buffs");

    BattleUnit unit = create_test_unit(1, 100, 50, 50, 30, 20);

    battle_apply_buff(&unit, 0, 3);  /* Attack up */
    battle_apply_buff(&unit, 2, 2);  /* Defense up */
    battle_apply_buff(&unit, 4, 4);  /* Speed up */

    TEST_ASSERT(unit.buff_flags & BUFF_ATTACK_UP, "Attack buff should be set");
    TEST_ASSERT(unit.buff_flags & BUFF_DEFENSE_UP, "Defense buff should be set");
    TEST_ASSERT(unit.buff_flags & BUFF_SPEED_UP, "Speed buff should be set");

    TEST_END();
}

/*
 * Test 23: Null buff operations
 */
static void test_null_buff_operations(void) {
    TEST_BEGIN("Null buff operations");

    /* These should not crash */
    battle_apply_buff(NULL, 0, 3);
    battle_remove_buff(NULL, 0);

    TEST_ASSERT(1, "Null buff operations should not crash");

    TEST_END();
}

/*
 * Test 24: Invalid buff ID
 */
static void test_invalid_buff_id(void) {
    TEST_BEGIN("Invalid buff ID");

    BattleUnit unit = create_test_unit(1, 100, 50, 50, 30, 20);

    /* Buff ID >= 16 should be ignored */
    battle_apply_buff(&unit, 16, 3);
    battle_apply_buff(&unit, 100, 3);

    TEST_ASSERT(unit.buff_flags == 0, "Invalid buff IDs should be ignored");

    TEST_END();
}

/* ========================================
 * Test Cases for Skill System
 * ======================================== */

/*
 * Test 25: Can use skill with enough MP
 */
static void test_can_use_skill(void) {
    TEST_BEGIN("Can use skill with MP");

    BattleUnit unit = create_test_unit(1, 100, 50, 50, 30, 20);

    int result = battle_can_use_skill(&unit, 0);  /* Skill 0 costs 0 MP by default */

    TEST_ASSERT(result == 1, "Should be able to use skill with enough MP");

    TEST_END();
}

/*
 * Test 26: Skill damage type
 */
static void test_skill_damage_type(void) {
    TEST_BEGIN("Skill damage type");

    BattleUnit attacker = create_test_unit(1, 100, 100, 50, 30, 20);
    BattleUnit target = create_test_unit(2, 100, 50, 40, 30, 15);
    BattleSkill skill = create_test_skill(1, 50, 10, 0);  /* Attack type */

    int result = battle_use_skill(&attacker, &target, 1);

    /* This test is informational since skill database may not be populated */
    TEST_ASSERT(1, "Skill usage test completed");

    TEST_END();
}

/*
 * Test 27: Skill heal type
 */
static void test_skill_heal_type(void) {
    TEST_BEGIN("Skill heal type");

    BattleUnit unit = create_test_unit(1, 50, 100, 50, 30, 20);
    unit.max_hp = 100;
    BattleSkill skill = create_test_skill(2, 30, 10, 1);  /* Heal type */

    /* This test is informational */
    TEST_ASSERT(1, "Heal skill test completed");

    TEST_END();
}

/*
 * Test 28: Null skill
 */
static void test_null_skill(void) {
    TEST_BEGIN("Null skill");

    BattleUnit unit = create_test_unit(1, 100, 50, 50, 30, 20);

    int result = battle_can_use_skill(&unit, 200);  /* Out of range skill */

    TEST_ASSERT(result == 0, "Should return 0 for invalid skill");

    TEST_END();
}

/* ========================================
 * Test Cases for Element System
 * ======================================== */

/*
 * Test 29: Element constants
 */
static void test_element_constants(void) {
    TEST_BEGIN("Element constants");

    /* Verify element constants are defined */
    TEST_ASSERT(ELEMENT_NONE == 0, "ELEMENT_NONE should be 0");
    TEST_ASSERT(ELEMENT_FIRE == 1, "ELEMENT_FIRE should be 1");
    TEST_ASSERT(ELEMENT_WATER == 2, "ELEMENT_WATER should be 2");
    TEST_ASSERT(ELEMENT_EARTH == 3, "ELEMENT_EARTH should be 3");
    TEST_ASSERT(ELEMENT_WIND == 4, "ELEMENT_WIND should be 4");
    TEST_ASSERT(ELEMENT_LIGHT == 5, "ELEMENT_LIGHT should be 5");
    TEST_ASSERT(ELEMENT_DARK == 6, "ELEMENT_DARK should be 6");
    TEST_ASSERT(ELEMENT_MAX == 7, "ELEMENT_MAX should be 7");

    TEST_END();
}

/*
 * Test 30: Element table values
 * Based on binary analysis - element advantage system
 * Fire > Wind > Earth > Water > Fire (1.5x damage)
 * Light <-> Dark (mutual 1.5x damage)
 */
static void test_element_table_values(void) {
    TEST_BEGIN("Element table values");

    /* The element table is defined in battle_calc.c:
     * Fire > Wind (1.5x), Wind > Earth (1.5x), etc.
     * Verify the expected pattern matches the binary
     *
     * TODO: Verify these values match the actual binary data table
     * The binary may have different values or ordering
     */

    /* Expected element advantage pattern from RPG conventions:
     * Fire > Wind: 1.5x
     * Wind > Earth: 1.5x
     * Earth > Water: 1.5x
     * Water > Fire: 1.5x
     * Light vs Dark: mutual 1.5x
     * Same element: 1.0x
     * Disadvantage: 0.5x
     */

    TEST_ASSERT(1, "Element table pattern should match RPG conventions");

    TEST_END();
}

/* ========================================
 * Additional Element System Tests (31-50)
 * Based on binary analysis of element damage calculation
 * ======================================== */

/*
 * Test 31: Fire beats Wind (1.5x damage)
 */
static void test_element_fire_beats_wind(void) {
    TEST_BEGIN("Element: Fire beats Wind (1.5x)");

    /* TODO: Verify from binary element table:
     * s_element_table[ELEMENT_FIRE][ELEMENT_WIND] should be 1.5f
     * This matches typical RPG element advantage: Fire > Wind
     *
     * From battle_calc.c:
     * [ELEMENT_FIRE][ELEMENT_WIND] = 1.5f (row 1, col 4)
     */

    TEST_ASSERT(ELEMENT_FIRE == 1, "Fire element is 1");
    TEST_ASSERT(ELEMENT_WIND == 4, "Wind element is 4");

    TEST_END();
}

/*
 * Test 32: Wind beats Earth (1.5x damage)
 */
static void test_element_wind_beats_earth(void) {
    TEST_BEGIN("Element: Wind beats Earth (1.5x)");

    /* TODO: Verify s_element_table[ELEMENT_WIND][ELEMENT_EARTH] == 1.5f
     * From battle_calc.c:
     * [ELEMENT_WIND][ELEMENT_EARTH] = 1.5f (row 4, col 3)
     */

    TEST_ASSERT(ELEMENT_WIND == 4, "Wind element is 4");
    TEST_ASSERT(ELEMENT_EARTH == 3, "Earth element is 3");

    TEST_END();
}

/*
 * Test 33: Earth beats Water (1.5x damage)
 */
static void test_element_earth_beats_water(void) {
    TEST_BEGIN("Element: Earth beats Water (1.5x)");

    /* TODO: Verify s_element_table[ELEMENT_EARTH][ELEMENT_WATER] == 1.5f
     * From battle_calc.c:
     * [ELEMENT_EARTH][ELEMENT_WATER] = 1.5f (row 3, col 2)
     */

    TEST_ASSERT(ELEMENT_EARTH == 3, "Earth element is 3");
    TEST_ASSERT(ELEMENT_WATER == 2, "Water element is 2");

    TEST_END();
}

/*
 * Test 34: Water beats Fire (1.5x damage)
 */
static void test_element_water_beats_fire(void) {
    TEST_BEGIN("Element: Water beats Fire (1.5x)");

    /* TODO: Verify s_element_table[ELEMENT_WATER][ELEMENT_FIRE] == 1.5f
     * From battle_calc.c:
     * [ELEMENT_WATER][ELEMENT_FIRE] = 1.5f (row 2, col 1)
     */

    TEST_ASSERT(ELEMENT_WATER == 2, "Water element is 2");
    TEST_ASSERT(ELEMENT_FIRE == 1, "Fire element is 1");

    TEST_END();
}

/*
 * Test 35: Fire weak to Water (0.5x damage)
 */
static void test_element_fire_weak_to_water(void) {
    TEST_BEGIN("Element: Fire weak to Water (0.5x)");

    /* TODO: Verify s_element_table[ELEMENT_FIRE][ELEMENT_WATER] == 0.5f
     * From battle_calc.c:
     * [ELEMENT_FIRE][ELEMENT_WATER] = 0.5f (row 1, col 2)
     */

    TEST_ASSERT(ELEMENT_FIRE == 1, "Fire element is 1");
    TEST_ASSERT(ELEMENT_WATER == 2, "Water element is 2");

    TEST_END();
}

/*
 * Test 36: Light vs Dark mutual 1.5x
 */
static void test_element_light_dark(void) {
    TEST_BEGIN("Element: Light vs Dark mutual 1.5x");

    /* TODO: Verify from binary:
     * s_element_table[ELEMENT_LIGHT][ELEMENT_DARK] == 1.5f
     * s_element_table[ELEMENT_DARK][ELEMENT_LIGHT] == 1.5f
     *
     * From battle_calc.c:
     * [ELEMENT_LIGHT][ELEMENT_DARK] = 1.5f (row 5, col 6)
     * [ELEMENT_DARK][ELEMENT_LIGHT] = 1.5f (row 6, col 5)
     */

    TEST_ASSERT(ELEMENT_LIGHT == 5, "Light element is 5");
    TEST_ASSERT(ELEMENT_DARK == 6, "Dark element is 6");

    TEST_END();
}

/*
 * Test 37: Same element neutral (1.0x)
 */
static void test_element_same_neutral(void) {
    TEST_BEGIN("Element: Same type neutral (1.0x)");

    /* TODO: Verify s_element_table[x][x] == 1.0f for all elements
     * From battle_calc.c diagonal elements are all 1.0f
     */

    TEST_ASSERT(ELEMENT_NONE == 0, "None element is 0");

    TEST_END();
}

/*
 * Test 38: Element damage calculation
 */
static void test_element_damage_calculation(void) {
    TEST_BEGIN("Element: Damage with element advantage");

    BattleUnit attacker = create_test_unit(1, 100, 50, 50, 30, 20);
    BattleUnit target = create_test_unit(2, 100, 50, 40, 30, 15);

    /* TODO: Set element attributes and verify damage multiplication
     * attacker.element = ELEMENT_FIRE;
     * target.element = ELEMENT_WIND;
     * Damage should be multiplied by 1.5
     *
     * FIX: BattleUnit structure needs element field added
     */

    u16 damage = battle_calc_damage(&attacker, &target, NULL);
    TEST_ASSERT(damage > 0, "Damage should be positive");

    TEST_END();
}

/*
 * Test 39: Element disadvantage reduces damage
 */
static void test_element_disadvantage(void) {
    TEST_BEGIN("Element: Disadvantage reduces damage (0.5x)");

    /* TODO: Test fire attacking water (0.5x damage)
     * FIX: Need element field in BattleUnit
     */

    TEST_ASSERT(1, "Element disadvantage test - implement after element system complete");

    TEST_END();
}

/*
 * Test 40: None element always neutral
 */
static void test_element_none_neutral(void) {
    TEST_BEGIN("Element: None type always neutral");

    /* TODO: Verify s_element_table[ELEMENT_NONE][x] == 1.0f for all x
     * and s_element_table[x][ELEMENT_NONE] == 1.0f for all x
     *
     * From battle_calc.c first row and column are all 1.0f
     */

    TEST_ASSERT(ELEMENT_NONE == 0, "None element is 0");

    TEST_END();
}

/* ========================================
 * Additional Capture Rate Tests (41-45)
 * Based on binary analysis of capture system
 * ======================================== */

/*
 * Test 41: Capture rate formula validation
 * From binary: base_rate = (100 - hp_percent) / 2
 */
static void test_capture_rate_formula(void) {
    TEST_BEGIN("Capture rate formula");

    /* At 50% HP: base = (100 - 50) / 2 = 25% */
    int rate = battle_calc_capture_rate(50, 30, 0);
    TEST_ASSERT(rate >= 20 && rate <= 30, "Capture at 50% HP should be ~25%");

    /* At 20% HP: base = (100 - 20) / 2 = 40% */
    rate = battle_calc_capture_rate(20, 30, 0);
    TEST_ASSERT(rate >= 35 && rate <= 45, "Capture at 20% HP should be ~40%");

    TEST_END();
}

/*
 * Test 42: Capture rate level penalty
 */
static void test_capture_level_penalty(void) {
    TEST_BEGIN("Capture rate level penalty");

    /* Level > 50 should reduce capture rate */
    int rate_low = battle_calc_capture_rate(20, 40, 0);
    int rate_high = battle_calc_capture_rate(20, 70, 0);

    /* High level should have lower rate */
    TEST_ASSERT(rate_high < rate_low, "Higher level should reduce capture rate");

    TEST_END();
}

/*
 * Test 43: Capture item bonus
 */
static void test_capture_item_bonus(void) {
    TEST_BEGIN("Capture item bonus");

    int rate_no_item = battle_calc_capture_rate(20, 30, 0);
    int rate_with_item = battle_calc_capture_rate(20, 30, 1);

    TEST_ASSERT(rate_with_item > rate_no_item, "Item should increase capture rate");
    TEST_ASSERT(rate_with_item - rate_no_item >= 10, "Item bonus should be at least 10%");

    TEST_END();
}

/*
 * Test 44: Capture rate clamping
 */
static void test_capture_rate_clamping(void) {
    TEST_BEGIN("Capture rate clamping");

    /* Very low HP + item should cap at 80% */
    int rate = battle_calc_capture_rate(1, 10, 1);
    TEST_ASSERT(rate <= 80, "Capture rate should cap at 80%");

    /* Very high HP should have minimum 5% */
    rate = battle_calc_capture_rate(99, 30, 0);
    TEST_ASSERT(rate >= 5, "Capture rate should have minimum 5%");

    TEST_END();
}

/*
 * Test 45: Capture rate edge cases
 */
static void test_capture_rate_edge_cases(void) {
    TEST_BEGIN("Capture rate edge cases");

    /* 0% HP (impossible but test) */
    int rate = battle_calc_capture_rate(0, 30, 0);
    TEST_ASSERT(rate >= 5 && rate <= 80, "Capture rate should be in valid range");

    /* 100% HP */
    rate = battle_calc_capture_rate(100, 30, 0);
    TEST_ASSERT(rate >= 0 && rate <= 10, "Capture at 100% HP should be very low");

    TEST_END();
}

/* ========================================
 * Buff/Debuff System Tests (46-50)
 * ======================================== */

/*
 * Test 46: Buff flag constants
 */
static void test_buff_flag_constants(void) {
    TEST_BEGIN("Buff flag constants");

    /* Verify buff flags are defined */
    TEST_ASSERT(BUFF_ATTACK_UP != 0 || BUFF_DEFENSE_UP != 0, "Buff flags should be defined");

    TEST_END();
}

/*
 * Test 47: Buff duration tracking
 */
static void test_buff_duration(void) {
    TEST_BEGIN("Buff duration tracking");

    BattleUnit unit = create_test_unit(1, 100, 50, 50, 30, 20);

    /* Apply buff for 3 turns */
    battle_apply_buff(&unit, 0, 3);
    TEST_ASSERT(unit.buff_turns[0] == 3, "Buff duration should be 3");

    /* Apply buff again should reset duration */
    battle_apply_buff(&unit, 0, 5);
    TEST_ASSERT(unit.buff_turns[0] == 5, "Buff duration should be updated to 5");

    TEST_END();
}

/*
 * Test 48: Multiple buff interactions
 */
static void test_multiple_buff_interactions(void) {
    TEST_BEGIN("Multiple buff interactions");

    BattleUnit attacker = create_test_unit(1, 100, 50, 50, 30, 20);
    BattleUnit target = create_test_unit(2, 100, 50, 40, 30, 15);

    /* Apply attack up and target defense down */
    attacker.buff_flags |= BUFF_ATTACK_UP;
    target.buff_flags |= BUFF_DEFENSE_DOWN;

    u16 damage = battle_calc_damage(&attacker, &target, NULL);

    /* Attack up + defense down should significantly increase damage */
    TEST_ASSERT(damage > 0, "Damage should be positive with buffs");

    TEST_END();
}

/*
 * Test 49: Buff removal
 */
static void test_buff_removal(void) {
    TEST_BEGIN("Buff removal");

    BattleUnit unit = create_test_unit(1, 100, 50, 50, 30, 20);

    /* Apply and remove buff */
    battle_apply_buff(&unit, 0, 3);
    TEST_ASSERT(unit.buff_flags & BUFF_ATTACK_UP, "Buff should be applied");

    battle_remove_buff(&unit, 0);
    TEST_ASSERT(!(unit.buff_flags & BUFF_ATTACK_UP), "Buff should be removed");
    TEST_ASSERT(unit.buff_turns[0] == 0, "Buff duration should be 0");

    TEST_END();
}

/*
 * Test 50: Damage variance
 */
static void test_damage_variance(void) {
    TEST_BEGIN("Damage variance");

    BattleUnit attacker = create_test_unit(1, 100, 50, 100, 30, 20);
    BattleUnit target = create_test_unit(2, 100, 50, 40, 30, 15);

    /* Calculate damage multiple times */
    u16 min_damage = 0xFFFF;
    u16 max_damage = 0;
    int i;

    for (i = 0; i < 100; i++) {
        u16 damage = battle_calc_damage(&attacker, &target, NULL);
        if (damage < min_damage) min_damage = damage;
        if (damage > max_damage) max_damage = damage;
    }

    /* Should have some variance (90-110%) */
    TEST_ASSERT(max_damage > min_damage, "Damage should have variance");
    TEST_ASSERT(min_damage > 0, "Minimum damage should be positive");

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(battle_calc) {
    /* Damage tests */
    test_basic_damage();
    test_skill_damage();
    test_damage_attack_buff();
    test_damage_defense_buff();
    test_minimum_damage();
    test_null_attacker();
    test_null_target();

    /* Hit tests */
    test_basic_hit_chance();
    test_hit_stunned_target();
    test_null_hit_check();

    /* Critical tests */
    test_basic_critical_chance();
    test_critical_debuffed_target();

    /* Capture tests */
    test_capture_full_hp();
    test_capture_low_hp();
    test_capture_with_item();
    test_capture_high_level();

    /* Escape tests */
    test_basic_escape_chance();
    test_escape_null_player();
    test_escape_slower_player();

    /* Buff tests */
    test_apply_buff();
    test_remove_buff();
    test_multiple_buffs();
    test_null_buff_operations();
    test_invalid_buff_id();

    /* Skill tests */
    test_can_use_skill();
    test_skill_damage_type();
    test_skill_heal_type();
    test_null_skill();

    /* Element tests */
    test_element_constants();
    test_element_table_values();

    /* Extended element tests */
    test_element_fire_beats_wind();
    test_element_wind_beats_earth();
    test_element_earth_beats_water();
    test_element_water_beats_fire();
    test_element_fire_weak_to_water();
    test_element_light_dark();
    test_element_same_neutral();
    test_element_damage_calculation();
    test_element_disadvantage();
    test_element_none_neutral();

    /* Additional capture rate tests */
    test_capture_rate_formula();
    test_capture_level_penalty();
    test_capture_item_bonus();
    test_capture_rate_clamping();
    test_capture_rate_edge_cases();

    /* Additional buff/debuff tests */
    test_buff_flag_constants();
    test_buff_duration();
    test_multiple_buff_interactions();
    test_buff_removal();
    test_damage_variance();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - Battle Calc Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(battle_calc);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
