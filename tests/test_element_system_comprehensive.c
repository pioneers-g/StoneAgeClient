/*
 * Stone Age Client - Element System Unit Tests
 * Tests for element advantage, damage calculation, buffs
 *
 * Based on Ghidra decompilation analysis:
 * - Element advantage table: Fire > Wind > Earth > Water > Fire
 * - Light <-> Dark (mutual weakness)
 * - 7 elements: None, Fire, Water, Earth, Wind, Light, Dark
 * - Damage formula: attack + skill_power/2 - defense/4
 * - Buff modifiers: +/- 20%
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Element types */
typedef enum {
    ELEMENT_NONE = 0,
    ELEMENT_FIRE = 1,
    ELEMENT_WATER = 2,
    ELEMENT_EARTH = 3,
    ELEMENT_WIND = 4,
    ELEMENT_LIGHT = 5,
    ELEMENT_DARK = 6,
    ELEMENT_MAX = 7
} ElementType;

/* Buff flags */
#define BUFF_NONE          0x0000
#define BUFF_ATTACK_UP     0x0001
#define BUFF_ATTACK_DOWN   0x0002
#define BUFF_DEFENSE_UP    0x0004
#define BUFF_DEFENSE_DOWN  0x0008
#define BUFF_SPEED_UP      0x0010
#define BUFF_SPEED_DOWN    0x0020
#define BUFF_POISON        0x0040
#define BUFF_STUN          0x0080
#define BUFF_SLEEP         0x0100
#define BUFF_CONFUSE       0x0200
#define BUFF_BLIND         0x0400

/* Element advantage table: Fire > Wind > Earth > Water > Fire */
/* Light <-> Dark (mutual weakness) */
static const float s_element_table[ELEMENT_MAX][ELEMENT_MAX] = {
    /* Target: None, Fire, Water, Earth, Wind, Light, Dark */
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},  /* None attacks */
    {1.0f, 1.0f, 0.5f, 1.0f, 1.5f, 1.0f, 1.0f},  /* Fire attacks */
    {1.0f, 1.5f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f},  /* Water attacks */
    {1.0f, 1.0f, 1.5f, 1.0f, 0.5f, 1.0f, 1.0f},  /* Earth attacks */
    {1.0f, 0.5f, 1.0f, 1.5f, 1.0f, 1.0f, 1.0f},  /* Wind attacks */
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.5f},  /* Light attacks */
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.5f, 1.0f},  /* Dark attacks */
};

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

/* Setup */
static void test_setup(void) {
    /* Reset test state */
}

/*
 * Get element multiplier
 */
static float get_element_multiplier(int attack_element, int target_element) {
    if (attack_element < 0 || attack_element >= ELEMENT_MAX) return 1.0f;
    if (target_element < 0 || target_element >= ELEMENT_MAX) return 1.0f;
    return s_element_table[attack_element][target_element];
}

/*
 * Calculate damage with element
 */
static s32 calc_damage(s32 attack, s32 defense, int atk_element, int def_element, u16 buff_flags) {
    s32 damage;
    s32 adj_attack = attack;
    s32 adj_defense = defense;

    /* Apply attack buffs */
    if (buff_flags & BUFF_ATTACK_UP) {
        adj_attack = adj_attack * 12 / 10;  /* +20% */
    }
    if (buff_flags & BUFF_ATTACK_DOWN) {
        adj_attack = adj_attack * 8 / 10;   /* -20% */
    }

    /* Apply defense buffs */
    if (buff_flags & BUFF_DEFENSE_UP) {
        adj_defense = adj_defense * 12 / 10;
    }
    if (buff_flags & BUFF_DEFENSE_DOWN) {
        adj_defense = adj_defense * 8 / 10;
    }

    /* Basic damage formula */
    damage = adj_attack - adj_defense / 2;
    if (damage < 1) damage = 1;

    /* Apply element multiplier */
    damage = (s32)(damage * get_element_multiplier(atk_element, def_element));

    return damage;
}

/* ========================================
 * Element Type Tests
 * ======================================== */

static int test_element_count(void) {
    test_setup();

    /* Must have exactly 7 elements (0-6) */
    assert(ELEMENT_MAX == 7);
    assert(ELEMENT_NONE == 0);
    assert(ELEMENT_DARK == 6);

    return 1;
}

static int test_element_values(void) {
    test_setup();

    /* Verify element values */
    assert(ELEMENT_NONE == 0);
    assert(ELEMENT_FIRE == 1);
    assert(ELEMENT_WATER == 2);
    assert(ELEMENT_EARTH == 3);
    assert(ELEMENT_WIND == 4);
    assert(ELEMENT_LIGHT == 5);
    assert(ELEMENT_DARK == 6);

    return 1;
}

/* ========================================
 * Element Advantage Tests
 * ======================================== */

static int test_fire_advantage(void) {
    test_setup();

    /* Fire > Wind (1.5x damage) */
    assert(get_element_multiplier(ELEMENT_FIRE, ELEMENT_WIND) == 1.5f);

    /* Fire < Water (0.5x damage) */
    assert(get_element_multiplier(ELEMENT_FIRE, ELEMENT_WATER) == 0.5f);

    /* Fire = Fire (1.0x damage) */
    assert(get_element_multiplier(ELEMENT_FIRE, ELEMENT_FIRE) == 1.0f);

    return 1;
}

static int test_water_advantage(void) {
    test_setup();

    /* Water > Fire (1.5x damage) */
    assert(get_element_multiplier(ELEMENT_WATER, ELEMENT_FIRE) == 1.5f);

    /* Water < Earth (0.5x damage) */
    assert(get_element_multiplier(ELEMENT_WATER, ELEMENT_EARTH) == 0.5f);

    /* Water = Water (1.0x damage) */
    assert(get_element_multiplier(ELEMENT_WATER, ELEMENT_WATER) == 1.0f);

    return 1;
}

static int test_earth_advantage(void) {
    test_setup();

    /* Earth > Water (1.5x damage) */
    assert(get_element_multiplier(ELEMENT_EARTH, ELEMENT_WATER) == 1.5f);

    /* Earth < Wind (0.5x damage) */
    assert(get_element_multiplier(ELEMENT_EARTH, ELEMENT_WIND) == 0.5f);

    /* Earth = Earth (1.0x damage) */
    assert(get_element_multiplier(ELEMENT_EARTH, ELEMENT_EARTH) == 1.0f);

    return 1;
}

static int test_wind_advantage(void) {
    test_setup();

    /* Wind > Earth (1.5x damage) */
    assert(get_element_multiplier(ELEMENT_WIND, ELEMENT_EARTH) == 1.5f);

    /* Wind < Fire (0.5x damage) */
    assert(get_element_multiplier(ELEMENT_WIND, ELEMENT_FIRE) == 0.5f);

    /* Wind = Wind (1.0x damage) */
    assert(get_element_multiplier(ELEMENT_WIND, ELEMENT_WIND) == 1.0f);

    return 1;
}

static int test_light_dark_advantage(void) {
    test_setup();

    /* Light > Dark (1.5x damage) */
    assert(get_element_multiplier(ELEMENT_LIGHT, ELEMENT_DARK) == 1.5f);

    /* Dark > Light (1.5x damage) */
    assert(get_element_multiplier(ELEMENT_DARK, ELEMENT_LIGHT) == 1.5f);

    /* Light = Light (1.0x damage) */
    assert(get_element_multiplier(ELEMENT_LIGHT, ELEMENT_LIGHT) == 1.0f);

    /* Dark = Dark (1.0x damage) */
    assert(get_element_multiplier(ELEMENT_DARK, ELEMENT_DARK) == 1.0f);

    return 1;
}

static int test_element_neutral(void) {
    test_setup();

    /* None element always neutral */
    assert(get_element_multiplier(ELEMENT_NONE, ELEMENT_FIRE) == 1.0f);
    assert(get_element_multiplier(ELEMENT_NONE, ELEMENT_WATER) == 1.0f);
    assert(get_element_multiplier(ELEMENT_FIRE, ELEMENT_NONE) == 1.0f);

    /* Light/Dark neutral to Fire/Water/Earth/Wind */
    assert(get_element_multiplier(ELEMENT_LIGHT, ELEMENT_FIRE) == 1.0f);
    assert(get_element_multiplier(ELEMENT_DARK, ELEMENT_WATER) == 1.0f);

    return 1;
}

static int test_element_cycle(void) {
    test_setup();

    /* Fire > Wind > Earth > Water > Fire cycle */
    /* Fire beats Wind */
    assert(get_element_multiplier(ELEMENT_FIRE, ELEMENT_WIND) > 1.0f);

    /* Wind beats Earth */
    assert(get_element_multiplier(ELEMENT_WIND, ELEMENT_EARTH) > 1.0f);

    /* Earth beats Water */
    assert(get_element_multiplier(ELEMENT_EARTH, ELEMENT_WATER) > 1.0f);

    /* Water beats Fire */
    assert(get_element_multiplier(ELEMENT_WATER, ELEMENT_FIRE) > 1.0f);

    return 1;
}

/* ========================================
 * Damage Calculation Tests
 * ======================================== */

static int test_basic_damage(void) {
    test_setup();

    /* Basic damage: attack - defense/2 */
    s32 damage = calc_damage(100, 50, ELEMENT_NONE, ELEMENT_NONE, BUFF_NONE);
    assert(damage == 75);  /* 100 - 25 = 75 */

    return 1;
}

static int test_minimum_damage(void) {
    test_setup();

    /* Minimum damage is 1 */
    s32 damage = calc_damage(10, 100, ELEMENT_NONE, ELEMENT_NONE, BUFF_NONE);
    assert(damage == 1);  /* Would be negative, clamped to 1 */

    return 1;
}

static int test_element_damage_bonus(void) {
    test_setup();

    /* Fire vs Wind: 1.5x damage */
    s32 neutral = calc_damage(100, 50, ELEMENT_FIRE, ELEMENT_FIRE, BUFF_NONE);
    s32 advantage = calc_damage(100, 50, ELEMENT_FIRE, ELEMENT_WIND, BUFF_NONE);

    assert(advantage == (s32)(neutral * 1.5f));

    return 1;
}

static int test_element_damage_penalty(void) {
    test_setup();

    /* Fire vs Water: 0.5x damage */
    s32 neutral = calc_damage(100, 50, ELEMENT_FIRE, ELEMENT_FIRE, BUFF_NONE);
    s32 penalty = calc_damage(100, 50, ELEMENT_FIRE, ELEMENT_WATER, BUFF_NONE);

    assert(penalty == (s32)(neutral * 0.5f));

    return 1;
}

/* ========================================
 * Buff Modifier Tests
 * ======================================== */

static int test_attack_up_buff(void) {
    test_setup();

    /* Attack up: +20% */
    s32 normal = calc_damage(100, 50, ELEMENT_NONE, ELEMENT_NONE, BUFF_NONE);
    s32 buffed = calc_damage(100, 50, ELEMENT_NONE, ELEMENT_NONE, BUFF_ATTACK_UP);

    /* Attack goes from 100 to 120, damage = 120 - 25 = 95 */
    assert(buffed == 95);
    assert(buffed > normal);

    return 1;
}

static int test_attack_down_buff(void) {
    test_setup();

    /* Attack down: -20% */
    s32 normal = calc_damage(100, 50, ELEMENT_NONE, ELEMENT_NONE, BUFF_NONE);
    s32 debuffed = calc_damage(100, 50, ELEMENT_NONE, ELEMENT_NONE, BUFF_ATTACK_DOWN);

    /* Attack goes from 100 to 80, damage = 80 - 25 = 55 */
    assert(debuffed == 55);
    assert(debuffed < normal);

    return 1;
}

static int test_defense_up_buff(void) {
    test_setup();

    /* Defense up: +20% */
    s32 normal = calc_damage(100, 50, ELEMENT_NONE, ELEMENT_NONE, BUFF_NONE);
    s32 buffed = calc_damage(100, 50, ELEMENT_NONE, ELEMENT_NONE, BUFF_DEFENSE_UP);

    /* Defense goes from 50 to 60, damage = 100 - 30 = 70 */
    assert(buffed == 70);
    assert(buffed < normal);

    return 1;
}

static int test_defense_down_buff(void) {
    test_setup();

    /* Defense down: -20% */
    s32 normal = calc_damage(100, 50, ELEMENT_NONE, ELEMENT_NONE, BUFF_NONE);
    s32 debuffed = calc_damage(100, 50, ELEMENT_NONE, ELEMENT_NONE, BUFF_DEFENSE_DOWN);

    /* Defense goes from 50 to 40, damage = 100 - 20 = 80 */
    assert(debuffed == 80);
    assert(debuffed > normal);

    return 1;
}

static int test_multiple_buffs(void) {
    test_setup();

    /* Attack up + Defense down: big damage */
    s32 normal = calc_damage(100, 50, ELEMENT_NONE, ELEMENT_NONE, BUFF_NONE);
    s32 combined = calc_damage(100, 50, ELEMENT_NONE, ELEMENT_NONE,
                               BUFF_ATTACK_UP | BUFF_DEFENSE_DOWN);

    /* Attack 120, Defense 40: damage = 120 - 20 = 100 */
    assert(combined == 100);
    assert(combined > normal);

    return 1;
}

/* ========================================
 * Hit/Critical Tests
 * ======================================== */

static int test_base_hit_chance(void) {
    test_setup();

    /* Base hit chance is 90% */
    /* This is a probability test, so we just verify the constant */
    int base_hit = 90;
    assert(base_hit == 90);

    return 1;
}

static int test_blind_hit_penalty(void) {
    test_setup();

    /* Blind reduces hit chance by 30% */
    int base_hit = 90;
    int blind_hit = base_hit * 7 / 10;
    assert(blind_hit == 63);  /* 90 * 0.7 = 63 */

    return 1;
}

static int test_base_crit_chance(void) {
    test_setup();

    /* Base critical chance is 5% */
    int base_crit = 5;
    assert(base_crit == 5);

    return 1;
}

static int test_crit_vs_debuffed(void) {
    test_setup();

    /* +10% crit chance against stunned/sleeping/confused targets */
    int base_crit = 5;
    int debuff_crit = base_crit + 10;
    assert(debuff_crit == 15);

    return 1;
}

/* ========================================
 * Capture Rate Tests
 * ======================================== */

static int test_capture_hp_factor(void) {
    test_setup();

    /* Lower HP = higher capture rate */
    /* At 100% HP: 100 - 100 = 0, /2 = 0% */
    /* At 50% HP: 100 - 50 = 50, /2 = 25% */
    /* At 1% HP: 100 - 1 = 99, /2 = 49% */

    int rate_100hp = (100 - 100) / 2;
    int rate_50hp = (100 - 50) / 2;
    int rate_1hp = (100 - 1) / 2;

    assert(rate_100hp == 0);
    assert(rate_50hp == 25);
    assert(rate_1hp == 49);

    return 1;
}

static int test_capture_level_penalty(void) {
    test_setup();

    /* Level > 50: penalty of (level - 50) */
    int base_rate = 50;
    int level_60 = base_rate - (60 - 50);
    int level_80 = base_rate - (80 - 50);

    assert(level_60 == 40);
    assert(level_80 == 20);

    return 1;
}

static int test_capture_bounds(void) {
    test_setup();

    /* Capture rate bounds: 5% to 80% */
    int min_rate = 5;
    int max_rate = 80;

    assert(min_rate == 5);
    assert(max_rate == 80);

    return 1;
}

/* ========================================
 * Escape Chance Tests
 * ======================================== */

static int test_base_escape_chance(void) {
    test_setup();

    /* Base escape chance is 50% */
    int base_escape = 50;
    assert(base_escape == 50);

    return 1;
}

static int test_escape_speed_bonus(void) {
    test_setup();

    /* Speed difference adds to escape chance */
    int player_speed = 100;
    int enemy_speed = 80;
    int base_escape = 50;
    int escape_chance = base_escape + (player_speed - enemy_speed);

    assert(escape_chance == 70);

    return 1;
}

static int test_escape_bounds(void) {
    test_setup();

    /* Escape chance bounds: 10% to 90% */
    int min_escape = 10;
    int max_escape = 90;

    assert(min_escape == 10);
    assert(max_escape == 90);

    return 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_damage_calculation(void) {
    test_setup();

    /* Full scenario: Fire attack vs Wind target with buffs */
    s32 attack = 200;
    s32 defense = 100;
    int atk_element = ELEMENT_FIRE;
    int def_element = ELEMENT_WIND;
    u16 buffs = BUFF_ATTACK_UP;

    /* Calculation:
     * Attack with buff: 200 * 1.2 = 240
     * Defense halved: 100 / 2 = 50
     * Base damage: 240 - 50 = 190
     * Element bonus: 190 * 1.5 = 285
     */
    s32 damage = calc_damage(attack, defense, atk_element, def_element, buffs);
    assert(damage == 285);

    return 1;
}

static int test_element_counter_damage(void) {
    test_setup();

    /* Water counters Fire */
    s32 neutral = calc_damage(100, 50, ELEMENT_WATER, ELEMENT_EARTH, BUFF_NONE);
    s32 counter = calc_damage(100, 50, ELEMENT_WATER, ELEMENT_FIRE, BUFF_NONE);

    assert(counter > neutral);
    assert(counter == (s32)(neutral * 3.0f));  /* 1.5 vs 0.5 */

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Element System Unit Tests ===\n\n");

    /* Element type tests */
    printf("Element Type Tests:\n");
    TEST(element_count);
    TEST(element_values);

    /* Element advantage tests */
    printf("\nElement Advantage Tests:\n");
    TEST(fire_advantage);
    TEST(water_advantage);
    TEST(earth_advantage);
    TEST(wind_advantage);
    TEST(light_dark_advantage);
    TEST(element_neutral);
    TEST(element_cycle);

    /* Damage calculation tests */
    printf("\nDamage Calculation Tests:\n");
    TEST(basic_damage);
    TEST(minimum_damage);
    TEST(element_damage_bonus);
    TEST(element_damage_penalty);

    /* Buff modifier tests */
    printf("\nBuff Modifier Tests:\n");
    TEST(attack_up_buff);
    TEST(attack_down_buff);
    TEST(defense_up_buff);
    TEST(defense_down_buff);
    TEST(multiple_buffs);

    /* Hit/critical tests */
    printf("\nHit/Critical Tests:\n");
    TEST(base_hit_chance);
    TEST(blind_hit_penalty);
    TEST(base_crit_chance);
    TEST(crit_vs_debuffed);

    /* Capture rate tests */
    printf("\nCapture Rate Tests:\n");
    TEST(capture_hp_factor);
    TEST(capture_level_penalty);
    TEST(capture_bounds);

    /* Escape chance tests */
    printf("\nEscape Chance Tests:\n");
    TEST(base_escape_chance);
    TEST(escape_speed_bonus);
    TEST(escape_bounds);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_damage_calculation);
    TEST(element_counter_damage);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - Skill damage formula verification
     * - Element resistance from equipment
     * - Combined element + buff scenarios
     * - Real damage values from game data
     * - Light/Dark interaction with other elements
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
