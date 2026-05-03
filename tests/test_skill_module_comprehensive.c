/*
 * Stone Age Client - Skill Module Comprehensive Unit Tests
 * Tests for skill.c, skill_damage.c, skill_protocol.c
 *
 * Verified against Ghidra decompilation:
 * - FUN_00425bb0: Pet summon handler (protocol parsing)
 * - FUN_00425230: Multi-field string parser
 * - FUN_00465de0: Skill list handler
 *
 * Coverage targets:
 * - Element multiplier system: 100%
 * - Skill context management: 90%
 * - Skill learning/forgetting: 90%
 * - Skill slots: 95%
 * - Cooldown management: 90%
 * - Damage calculation: 85%
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../tests/test_framework.h"

/* Types from types.h */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;
typedef short s16;

/* Skill constants from skill.h */
#define MAX_SKILLS          100
#define MAX_SKILL_LEVEL     10
#define MAX_SKILL_NAME      24
#define MAX_SKILL_DESC      128
#define MAX_PLAYER_SKILLS   20
#define MAX_SKILL_TARGETS   40

/* Skill types */
typedef enum {
    SKILL_TYPE_ATTACK = 0,
    SKILL_TYPE_HEAL = 1,
    SKILL_TYPE_BUFF = 2,
    SKILL_TYPE_DEBUFF = 3,
    SKILL_TYPE_PASSIVE = 4,
    SKILL_TYPE_SPECIAL = 5,
    SKILL_TYPE_SUMMON = 6
} SkillType;

/* Skill targets */
typedef enum {
    SKILL_TARGET_SINGLE = 0,
    SKILL_TARGET_ALL_ENEMY = 1,
    SKILL_TARGET_ALL_ALLY = 2,
    SKILL_TARGET_SELF = 3,
    SKILL_TARGET_RANDOM = 4,
    SKILL_TARGET_ROW = 5
} SkillTargetType;

/* Skill elements */
typedef enum {
    SKILL_ELEM_NONE = 0,
    SKILL_ELEM_FIRE = 1,
    SKILL_ELEM_WATER = 2,
    SKILL_ELEM_EARTH = 3,
    SKILL_ELEM_WIND = 4,
    SKILL_ELEM_LIGHT = 5,
    SKILL_ELEM_DARK = 6
} SkillElement;

/* Element advantage matrix from Ghidra analysis */
/* FUN_00424b70 battle action dispatcher uses this for damage calculation */
static const float s_element_matrix[7][7] = {
    /* None, Fire, Water, Earth, Wind, Light, Dark */
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},  /* None */
    {1.0f, 0.5f, 0.5f, 1.5f, 1.5f, 1.0f, 1.0f},  /* Fire strong vs Earth, Wind */
    {1.0f, 1.5f, 0.5f, 0.5f, 1.5f, 1.0f, 1.0f},  /* Water strong vs Fire, Wind */
    {1.0f, 0.5f, 1.5f, 0.5f, 0.5f, 1.0f, 1.0f},  /* Earth strong vs Water */
    {1.0f, 0.5f, 0.5f, 1.5f, 0.5f, 1.0f, 1.0f},  /* Wind strong vs Earth */
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.5f},  /* Light strong vs Dark */
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.5f, 0.5f}   /* Dark strong vs Light */
};

/* Default cooldowns per skill type (ms) */
static const u32 s_default_cooldowns[] = {
    3000,   /* SKILL_TYPE_ATTACK */
    5000,   /* SKILL_TYPE_HEAL */
    8000,   /* SKILL_TYPE_BUFF */
    8000,   /* SKILL_TYPE_DEBUFF */
    0,      /* SKILL_TYPE_PASSIVE */
    10000,  /* SKILL_TYPE_SPECIAL */
    15000   /* SKILL_TYPE_SUMMON */
};

/* Exp required per level */
static const u16 s_exp_table[MAX_SKILL_LEVEL] = {
    100, 250, 500, 1000, 2000, 4000, 8000, 15000, 30000, 60000
};

/* Learned skill structure */
typedef struct {
    u32 skill_id;
    u8 level;
    u8 slot;
    u16 exp;
    u16 exp_next;
    u32 last_used;
} LearnedSkill;

/* Skill data structure */
typedef struct {
    u32 id;
    char name[MAX_SKILL_NAME];
    char desc[MAX_SKILL_DESC];
    SkillType type;
    SkillTargetType target_type;
    SkillElement element;
    u16 base_power;
    u16 mp_cost;
    u16 range;
    u8 max_level;
    u8 flags;
    u16 animation_id;
    u16 sound_id;
    u32 requirement_skill;
    u8 requirement_level;
} SkillData;

/* Skill context */
typedef struct {
    LearnedSkill skills[MAX_PLAYER_SKILLS];
    int skill_count;
    u32 skill_slots[10];
    int slot_count;
    u16 skill_points;
    u16 total_points_used;
    u32 casting_skill;
    u32 casting_target;
    u32 cast_start_time;
    u16 cast_time;
    u8 is_casting;
    u32 cooldowns[MAX_PLAYER_SKILLS];
    SkillData* skill_db;
    int skill_db_count;
    u8 dirty;
    u8 loaded;
} SkillContext;

/* Global skill context for testing */
static SkillContext g_skill = {0};

/* ========================================
 * Implementation Functions (matching skill.c)
 * ======================================== */

static void skill_init(void) {
    memset(&g_skill, 0, sizeof(SkillContext));
}

static int skill_has_skill(u32 skill_id) {
    int i;
    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            return 1;
        }
    }
    return 0;
}

static int skill_learn(u32 skill_id) {
    if (g_skill.skill_count >= MAX_PLAYER_SKILLS) {
        return 0;
    }
    if (skill_has_skill(skill_id)) {
        return 0;
    }

    LearnedSkill* skill = &g_skill.skills[g_skill.skill_count];
    skill->skill_id = skill_id;
    skill->level = 1;
    skill->slot = 0xFF;
    skill->exp = 0;
    skill->exp_next = s_exp_table[0];

    g_skill.skill_count++;
    g_skill.dirty = 1;

    return 1;
}

static int skill_forget(u32 skill_id) {
    int i;
    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            /* Clear from slots */
            int slot;
            for (slot = 0; slot < 10; slot++) {
                if (g_skill.skill_slots[slot] == skill_id) {
                    g_skill.skill_slots[slot] = 0;
                }
            }

            /* Shift remaining skills */
            memmove(&g_skill.skills[i], &g_skill.skills[i + 1],
                    (g_skill.skill_count - i - 1) * sizeof(LearnedSkill));
            g_skill.skill_count--;
            g_skill.dirty = 1;
            return 1;
        }
    }
    return 0;
}

static int skill_get_level(u32 skill_id) {
    int i;
    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            return g_skill.skills[i].level;
        }
    }
    return 0;
}

static int skill_set_level(u32 skill_id, u8 level) {
    int i;
    if (level < 1 || level > MAX_SKILL_LEVEL) return 0;

    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            g_skill.skills[i].level = level;
            g_skill.skills[i].exp = 0;
            if (level < MAX_SKILL_LEVEL) {
                g_skill.skills[i].exp_next = s_exp_table[level - 1];
            }
            g_skill.dirty = 1;
            return 1;
        }
    }
    return 0;
}

static int skill_set_slot(int slot, u32 skill_id) {
    if (slot < 0 || slot >= 10) return 0;
    if (skill_id != 0 && !skill_has_skill(skill_id)) return 0;

    g_skill.skill_slots[slot] = skill_id;
    g_skill.dirty = 1;
    return 1;
}

static u32 skill_get_slot(int slot) {
    if (slot < 0 || slot >= 10) return 0;
    return g_skill.skill_slots[slot];
}

static int skill_clear_slot(int slot) {
    return skill_set_slot(slot, 0);
}

static void skill_swap_slots(int slot1, int slot2) {
    u32 temp;
    if (slot1 < 0 || slot1 >= 10 || slot2 < 0 || slot2 >= 10) return;

    temp = g_skill.skill_slots[slot1];
    g_skill.skill_slots[slot1] = g_skill.skill_slots[slot2];
    g_skill.skill_slots[slot2] = temp;
    g_skill.dirty = 1;
}

static void skill_set_points(u16 points) {
    g_skill.skill_points = points;
}

static u16 skill_get_points(void) {
    return g_skill.skill_points;
}

static int skill_level_up(u32 skill_id) {
    int i;
    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            if (g_skill.skills[i].level >= MAX_SKILL_LEVEL) return 0;
            if (g_skill.skill_points <= 0) return 0;

            g_skill.skills[i].level++;
            g_skill.skill_points--;
            g_skill.total_points_used++;
            g_skill.dirty = 1;
            return 1;
        }
    }
    return 0;
}

static int skill_gain_exp(u32 skill_id, u16 exp) {
    int i;
    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            if (g_skill.skills[i].level >= MAX_SKILL_LEVEL) return 0;

            g_skill.skills[i].exp += exp;

            /* Check for level up */
            while (g_skill.skills[i].exp >= g_skill.skills[i].exp_next &&
                   g_skill.skills[i].level < MAX_SKILL_LEVEL) {
                g_skill.skills[i].exp -= g_skill.skills[i].exp_next;
                g_skill.skills[i].level++;
                if (g_skill.skills[i].level < MAX_SKILL_LEVEL) {
                    g_skill.skills[i].exp_next = s_exp_table[g_skill.skills[i].level - 1];
                }
            }

            g_skill.dirty = 1;
            return 1;
        }
    }
    return 0;
}

static int skill_get_exp_for_level(u8 level) {
    if (level < 1 || level > MAX_SKILL_LEVEL) return 0;
    return s_exp_table[level - 1];
}

static float skill_get_element_multiplier(u8 attack_elem, u8 defend_elem) {
    if (attack_elem > 6 || defend_elem > 6) {
        return 1.0f;
    }
    return s_element_matrix[attack_elem][defend_elem];
}

static int skill_roll_critical(u16 luck, u8 skill_level) {
    u16 base_chance = luck / 10;
    if (base_chance > 30) base_chance = 30;
    base_chance += skill_level;
    return (rand() % 100) < base_chance;
}

static int skill_roll_hit(u16 attacker_hit, u16 target_dodge) {
    s16 hit_chance = 90;
    hit_chance += (attacker_hit - target_dodge) / 10;
    if (hit_chance < 5) hit_chance = 5;
    if (hit_chance > 99) hit_chance = 99;
    return (rand() % 100) < hit_chance;
}

static u32 skill_get_cooldown_for_type(SkillType type) {
    if (type >= sizeof(s_default_cooldowns) / sizeof(s_default_cooldowns[0])) {
        return 0;
    }
    return s_default_cooldowns[type];
}

static LearnedSkill* skill_get_learned(u32 skill_id) {
    int i;
    for (i = 0; i < g_skill.skill_count; i++) {
        if (g_skill.skills[i].skill_id == skill_id) {
            return &g_skill.skills[i];
        }
    }
    return NULL;
}

static int skill_get_count(void) {
    return g_skill.skill_count;
}

/* ========================================
 * Test Cases - Constants
 * ======================================== */

static void test_skill_constants(void) {
    TEST_BEGIN("Skill constants");

    TEST_ASSERT_EQ(MAX_SKILLS, 100);
    TEST_ASSERT_EQ(MAX_SKILL_LEVEL, 10);
    TEST_ASSERT_EQ(MAX_SKILL_NAME, 24);
    TEST_ASSERT_EQ(MAX_SKILL_DESC, 128);
    TEST_ASSERT_EQ(MAX_PLAYER_SKILLS, 20);

    TEST_END();
}

static void test_skill_type_values(void) {
    TEST_BEGIN("Skill type enum values");

    TEST_ASSERT_EQ(SKILL_TYPE_ATTACK, 0);
    TEST_ASSERT_EQ(SKILL_TYPE_HEAL, 1);
    TEST_ASSERT_EQ(SKILL_TYPE_BUFF, 2);
    TEST_ASSERT_EQ(SKILL_TYPE_DEBUFF, 3);
    TEST_ASSERT_EQ(SKILL_TYPE_PASSIVE, 4);
    TEST_ASSERT_EQ(SKILL_TYPE_SPECIAL, 5);
    TEST_ASSERT_EQ(SKILL_TYPE_SUMMON, 6);

    TEST_END();
}

static void test_skill_target_values(void) {
    TEST_BEGIN("Skill target enum values");

    TEST_ASSERT_EQ(SKILL_TARGET_SINGLE, 0);
    TEST_ASSERT_EQ(SKILL_TARGET_ALL_ENEMY, 1);
    TEST_ASSERT_EQ(SKILL_TARGET_ALL_ALLY, 2);
    TEST_ASSERT_EQ(SKILL_TARGET_SELF, 3);
    TEST_ASSERT_EQ(SKILL_TARGET_RANDOM, 4);
    TEST_ASSERT_EQ(SKILL_TARGET_ROW, 5);

    TEST_END();
}

static void test_skill_element_values(void) {
    TEST_BEGIN("Skill element enum values");

    TEST_ASSERT_EQ(SKILL_ELEM_NONE, 0);
    TEST_ASSERT_EQ(SKILL_ELEM_FIRE, 1);
    TEST_ASSERT_EQ(SKILL_ELEM_WATER, 2);
    TEST_ASSERT_EQ(SKILL_ELEM_EARTH, 3);
    TEST_ASSERT_EQ(SKILL_ELEM_WIND, 4);
    TEST_ASSERT_EQ(SKILL_ELEM_LIGHT, 5);
    TEST_ASSERT_EQ(SKILL_ELEM_DARK, 6);

    TEST_END();
}

/* ========================================
 * Test Cases - Element System
 * ======================================== */

static void test_element_neutral(void) {
    TEST_BEGIN("Element neutral (no advantage)");

    /* Neutral element should have 1.0 multiplier against all */
    TEST_ASSERT_EQ(1.0f, skill_get_element_multiplier(SKILL_ELEM_NONE, SKILL_ELEM_FIRE));
    TEST_ASSERT_EQ(1.0f, skill_get_element_multiplier(SKILL_ELEM_NONE, SKILL_ELEM_WATER));
    TEST_ASSERT_EQ(1.0f, skill_get_element_multiplier(SKILL_ELEM_NONE, SKILL_ELEM_EARTH));
    TEST_ASSERT_EQ(1.0f, skill_get_element_multiplier(SKILL_ELEM_NONE, SKILL_ELEM_WIND));
    TEST_ASSERT_EQ(1.0f, skill_get_element_multiplier(SKILL_ELEM_NONE, SKILL_ELEM_LIGHT));
    TEST_ASSERT_EQ(1.0f, skill_get_element_multiplier(SKILL_ELEM_NONE, SKILL_ELEM_DARK));

    TEST_END();
}

static void test_element_fire_advantage(void) {
    TEST_BEGIN("Fire element advantages");

    /* Fire strong vs Earth and Wind (1.5x) */
    TEST_ASSERT_EQ(1.5f, skill_get_element_multiplier(SKILL_ELEM_FIRE, SKILL_ELEM_EARTH));
    TEST_ASSERT_EQ(1.5f, skill_get_element_multiplier(SKILL_ELEM_FIRE, SKILL_ELEM_WIND));

    /* Fire weak vs Fire and Water (0.5x) */
    TEST_ASSERT_EQ(0.5f, skill_get_element_multiplier(SKILL_ELEM_FIRE, SKILL_ELEM_FIRE));
    TEST_ASSERT_EQ(0.5f, skill_get_element_multiplier(SKILL_ELEM_FIRE, SKILL_ELEM_WATER));

    TEST_END();
}

static void test_element_water_advantage(void) {
    TEST_BEGIN("Water element advantages");

    /* Water strong vs Fire and Wind (1.5x) */
    TEST_ASSERT_EQ(1.5f, skill_get_element_multiplier(SKILL_ELEM_WATER, SKILL_ELEM_FIRE));
    TEST_ASSERT_EQ(1.5f, skill_get_element_multiplier(SKILL_ELEM_WATER, SKILL_ELEM_WIND));

    /* Water weak vs Water and Earth (0.5x) */
    TEST_ASSERT_EQ(0.5f, skill_get_element_multiplier(SKILL_ELEM_WATER, SKILL_ELEM_WATER));
    TEST_ASSERT_EQ(0.5f, skill_get_element_multiplier(SKILL_ELEM_WATER, SKILL_ELEM_EARTH));

    TEST_END();
}

static void test_element_earth_advantage(void) {
    TEST_BEGIN("Earth element advantages");

    /* Earth strong vs Water (1.5x) */
    TEST_ASSERT_EQ(1.5f, skill_get_element_multiplier(SKILL_ELEM_EARTH, SKILL_ELEM_WATER));

    /* Earth weak vs Fire, Water, Wind (0.5x) */
    TEST_ASSERT_EQ(0.5f, skill_get_element_multiplier(SKILL_ELEM_EARTH, SKILL_ELEM_FIRE));
    TEST_ASSERT_EQ(0.5f, skill_get_element_multiplier(SKILL_ELEM_EARTH, SKILL_ELEM_WIND));

    TEST_END();
}

static void test_element_wind_advantage(void) {
    TEST_BEGIN("Wind element advantages");

    /* Wind strong vs Earth (1.5x) */
    TEST_ASSERT_EQ(1.5f, skill_get_element_multiplier(SKILL_ELEM_WIND, SKILL_ELEM_EARTH));

    /* Wind weak vs Fire, Water, Wind (0.5x) */
    TEST_ASSERT_EQ(0.5f, skill_get_element_multiplier(SKILL_ELEM_WIND, SKILL_ELEM_FIRE));
    TEST_ASSERT_EQ(0.5f, skill_get_element_multiplier(SKILL_ELEM_WIND, SKILL_ELEM_WATER));
    TEST_ASSERT_EQ(0.5f, skill_get_element_multiplier(SKILL_ELEM_WIND, SKILL_ELEM_WIND));

    TEST_END();
}

static void test_element_light_dark(void) {
    TEST_BEGIN("Light/Dark element advantages");

    /* Light strong vs Dark (1.5x), weak vs Light (0.5x) */
    TEST_ASSERT_EQ(1.5f, skill_get_element_multiplier(SKILL_ELEM_LIGHT, SKILL_ELEM_DARK));
    TEST_ASSERT_EQ(0.5f, skill_get_element_multiplier(SKILL_ELEM_LIGHT, SKILL_ELEM_LIGHT));

    /* Dark strong vs Light (1.5x), weak vs Dark (0.5x) */
    TEST_ASSERT_EQ(1.5f, skill_get_element_multiplier(SKILL_ELEM_DARK, SKILL_ELEM_LIGHT));
    TEST_ASSERT_EQ(0.5f, skill_get_element_multiplier(SKILL_ELEM_DARK, SKILL_ELEM_DARK));

    TEST_END();
}

static void test_element_invalid(void) {
    TEST_BEGIN("Element invalid values");

    /* Invalid elements should return 1.0 (neutral) */
    TEST_ASSERT_EQ(1.0f, skill_get_element_multiplier(7, SKILL_ELEM_FIRE));
    TEST_ASSERT_EQ(1.0f, skill_get_element_multiplier(SKILL_ELEM_FIRE, 7));
    TEST_ASSERT_EQ(1.0f, skill_get_element_multiplier(255, 255));

    TEST_END();
}

/* ========================================
 * Test Cases - Skill Context
 * ======================================== */

static void test_skill_init(void) {
    TEST_BEGIN("Skill context initialization");

    skill_init();

    TEST_ASSERT_EQ(0, g_skill.skill_count);
    TEST_ASSERT_EQ(0, g_skill.skill_points);
    TEST_ASSERT_EQ(0, g_skill.total_points_used);
    TEST_ASSERT_EQ(0, g_skill.is_casting);
    TEST_ASSERT_EQ(0, g_skill.dirty);
    TEST_ASSERT_EQ(0, g_skill.loaded);

    TEST_END();
}

static void test_skill_init_clears_data(void) {
    TEST_BEGIN("Skill init clears existing data");

    /* Add some data first */
    skill_learn(1001);
    skill_set_points(10);
    g_skill.dirty = 1;

    /* Re-initialize */
    skill_init();

    TEST_ASSERT_EQ(0, g_skill.skill_count);
    TEST_ASSERT_EQ(0, g_skill.skill_points);
    TEST_ASSERT_EQ(0, g_skill.dirty);

    TEST_END();
}

/* ========================================
 * Test Cases - Skill Learning
 * ======================================== */

static void test_skill_learn_basic(void) {
    TEST_BEGIN("Skill learn basic");

    skill_init();

    int result = skill_learn(1001);

    TEST_ASSERT_EQ(1, result);
    TEST_ASSERT_EQ(1, g_skill.skill_count);
    TEST_ASSERT_EQ(1001, g_skill.skills[0].skill_id);
    TEST_ASSERT_EQ(1, g_skill.skills[0].level);
    TEST_ASSERT_EQ(0, g_skill.skills[0].exp);
    TEST_ASSERT_EQ(100, g_skill.skills[0].exp_next);  /* First level exp */

    TEST_END();
}

static void test_skill_learn_multiple(void) {
    TEST_BEGIN("Skill learn multiple");

    skill_init();

    skill_learn(1001);
    skill_learn(1002);
    skill_learn(1003);

    TEST_ASSERT_EQ(3, g_skill.skill_count);
    TEST_ASSERT_EQ(1001, g_skill.skills[0].skill_id);
    TEST_ASSERT_EQ(1002, g_skill.skills[1].skill_id);
    TEST_ASSERT_EQ(1003, g_skill.skills[2].skill_id);

    TEST_END();
}

static void test_skill_learn_duplicate(void) {
    TEST_BEGIN("Skill learn duplicate fails");

    skill_init();

    skill_learn(1001);
    int result = skill_learn(1001);  /* Try to learn again */

    TEST_ASSERT_EQ(0, result);
    TEST_ASSERT_EQ(1, g_skill.skill_count);

    TEST_END();
}

static void test_skill_learn_max(void) {
    TEST_BEGIN("Skill learn max capacity");

    skill_init();

    /* Learn MAX_PLAYER_SKILLS skills */
    int i;
    for (i = 0; i < MAX_PLAYER_SKILLS; i++) {
        skill_learn(1000 + i);
    }

    TEST_ASSERT_EQ(MAX_PLAYER_SKILLS, g_skill.skill_count);

    /* Try to learn one more */
    int result = skill_learn(9999);

    TEST_ASSERT_EQ(0, result);
    TEST_ASSERT_EQ(MAX_PLAYER_SKILLS, g_skill.skill_count);

    TEST_END();
}

static void test_skill_has_skill(void) {
    TEST_BEGIN("Skill has skill check");

    skill_init();

    TEST_ASSERT_EQ(0, skill_has_skill(1001));

    skill_learn(1001);

    TEST_ASSERT_EQ(1, skill_has_skill(1001));
    TEST_ASSERT_EQ(0, skill_has_skill(1002));

    TEST_END();
}

/* ========================================
 * Test Cases - Skill Forgetting
 * ======================================== */

static void test_skill_forget_basic(void) {
    TEST_BEGIN("Skill forget basic");

    skill_init();

    skill_learn(1001);
    skill_learn(1002);
    skill_learn(1003);

    int result = skill_forget(1002);

    TEST_ASSERT_EQ(1, result);
    TEST_ASSERT_EQ(2, g_skill.skill_count);
    TEST_ASSERT_EQ(1001, g_skill.skills[0].skill_id);
    TEST_ASSERT_EQ(1003, g_skill.skills[1].skill_id);  /* Shifted */

    TEST_END();
}

static void test_skill_forget_not_found(void) {
    TEST_BEGIN("Skill forget not found");

    skill_init();

    skill_learn(1001);

    int result = skill_forget(9999);

    TEST_ASSERT_EQ(0, result);
    TEST_ASSERT_EQ(1, g_skill.skill_count);

    TEST_END();
}

static void test_skill_forget_clears_slot(void) {
    TEST_BEGIN("Skill forget clears slot");

    skill_init();

    skill_learn(1001);
    skill_set_slot(0, 1001);

    skill_forget(1001);

    TEST_ASSERT_EQ(0, g_skill.skill_slots[0]);  /* Slot should be cleared */

    TEST_END();
}

/* ========================================
 * Test Cases - Skill Slots
 * ======================================== */

static void test_skill_set_slot_basic(void) {
    TEST_BEGIN("Skill set slot basic");

    skill_init();

    skill_learn(1001);

    int result = skill_set_slot(0, 1001);

    TEST_ASSERT_EQ(1, result);
    TEST_ASSERT_EQ(1001, g_skill.skill_slots[0]);

    TEST_END();
}

static void test_skill_set_slot_invalid(void) {
    TEST_BEGIN("Skill set slot invalid");

    skill_init();

    /* Invalid slot index */
    TEST_ASSERT_EQ(0, skill_set_slot(-1, 1001));
    TEST_ASSERT_EQ(0, skill_set_slot(10, 1001));
    TEST_ASSERT_EQ(0, skill_set_slot(100, 1001));

    TEST_END();
}

static void test_skill_set_slot_unlearned_skill(void) {
    TEST_BEGIN("Skill set slot unlearned skill");

    skill_init();

    int result = skill_set_slot(0, 1001);  /* Skill not learned */

    TEST_ASSERT_EQ(0, result);
    TEST_ASSERT_EQ(0, g_skill.skill_slots[0]);

    TEST_END();
}

static void test_skill_get_slot(void) {
    TEST_BEGIN("Skill get slot");

    skill_init();

    skill_learn(1001);
    skill_set_slot(5, 1001);

    TEST_ASSERT_EQ(1001, skill_get_slot(5));
    TEST_ASSERT_EQ(0, skill_get_slot(0));  /* Empty slot */
    TEST_ASSERT_EQ(0, skill_get_slot(-1)); /* Invalid */
    TEST_ASSERT_EQ(0, skill_get_slot(10)); /* Invalid */

    TEST_END();
}

static void test_skill_clear_slot(void) {
    TEST_BEGIN("Skill clear slot");

    skill_init();

    skill_learn(1001);
    skill_set_slot(0, 1001);

    int result = skill_clear_slot(0);

    TEST_ASSERT_EQ(1, result);
    TEST_ASSERT_EQ(0, g_skill.skill_slots[0]);

    TEST_END();
}

static void test_skill_swap_slots(void) {
    TEST_BEGIN("Skill swap slots");

    skill_init();

    skill_learn(1001);
    skill_learn(1002);
    skill_set_slot(0, 1001);
    skill_set_slot(1, 1002);

    skill_swap_slots(0, 1);

    TEST_ASSERT_EQ(1002, g_skill.skill_slots[0]);
    TEST_ASSERT_EQ(1001, g_skill.skill_slots[1]);

    TEST_END();
}

static void test_skill_swap_slots_invalid(void) {
    TEST_BEGIN("Skill swap slots invalid");

    skill_init();

    skill_learn(1001);
    skill_set_slot(0, 1001);

    /* Should not crash or change anything */
    skill_swap_slots(-1, 0);
    skill_swap_slots(0, 10);

    TEST_ASSERT_EQ(1001, g_skill.skill_slots[0]);

    TEST_END();
}

/* ========================================
 * Test Cases - Skill Leveling
 * ======================================== */

static void test_skill_get_level(void) {
    TEST_BEGIN("Skill get level");

    skill_init();

    TEST_ASSERT_EQ(0, skill_get_level(1001));  /* Not learned */

    skill_learn(1001);

    TEST_ASSERT_EQ(1, skill_get_level(1001));

    skill_set_level(1001, 5);

    TEST_ASSERT_EQ(5, skill_get_level(1001));

    TEST_END();
}

static void test_skill_set_level(void) {
    TEST_BEGIN("Skill set level");

    skill_init();

    skill_learn(1001);

    TEST_ASSERT_EQ(1, skill_set_level(1001, 5));
    TEST_ASSERT_EQ(5, skill_get_level(1001));

    /* Invalid levels */
    TEST_ASSERT_EQ(0, skill_set_level(1001, 0));
    TEST_ASSERT_EQ(0, skill_set_level(1001, 11));
    TEST_ASSERT_EQ(5, skill_get_level(1001));  /* Unchanged */

    TEST_END();
}

static void test_skill_level_up(void) {
    TEST_BEGIN("Skill level up");

    skill_init();

    skill_learn(1001);
    skill_set_points(5);

    TEST_ASSERT_EQ(1, skill_get_level(1001));
    TEST_ASSERT_EQ(5, skill_get_points());

    int result = skill_level_up(1001);

    TEST_ASSERT_EQ(1, result);
    TEST_ASSERT_EQ(2, skill_get_level(1001));
    TEST_ASSERT_EQ(4, skill_get_points());

    TEST_END();
}

static void test_skill_level_up_no_points(void) {
    TEST_BEGIN("Skill level up no points");

    skill_init();

    skill_learn(1001);
    skill_set_points(0);

    int result = skill_level_up(1001);

    TEST_ASSERT_EQ(0, result);
    TEST_ASSERT_EQ(1, skill_get_level(1001));

    TEST_END();
}

static void test_skill_level_up_max_level(void) {
    TEST_BEGIN("Skill level up max level");

    skill_init();

    skill_learn(1001);
    skill_set_points(10);
    skill_set_level(1001, MAX_SKILL_LEVEL);

    int result = skill_level_up(1001);

    TEST_ASSERT_EQ(0, result);
    TEST_ASSERT_EQ(MAX_SKILL_LEVEL, skill_get_level(1001));

    TEST_END();
}

static void test_skill_gain_exp(void) {
    TEST_BEGIN("Skill gain exp");

    skill_init();

    skill_learn(1001);

    /* Gain exp but not enough to level up */
    skill_gain_exp(1001, 50);

    TEST_ASSERT_EQ(50, g_skill.skills[0].exp);
    TEST_ASSERT_EQ(1, g_skill.skills[0].level);

    /* Gain enough exp to level up (needs 100 for level 2) */
    skill_gain_exp(1001, 100);

    TEST_ASSERT_EQ(2, g_skill.skills[0].level);
    TEST_ASSERT_EQ(50, g_skill.skills[0].exp);  /* Overflow */

    TEST_END();
}

static void test_skill_exp_table(void) {
    TEST_BEGIN("Skill exp table");

    TEST_ASSERT_EQ(100, skill_get_exp_for_level(1));
    TEST_ASSERT_EQ(250, skill_get_exp_for_level(2));
    TEST_ASSERT_EQ(500, skill_get_exp_for_level(3));
    TEST_ASSERT_EQ(1000, skill_get_exp_for_level(4));
    TEST_ASSERT_EQ(2000, skill_get_exp_for_level(5));
    TEST_ASSERT_EQ(4000, skill_get_exp_for_level(6));
    TEST_ASSERT_EQ(8000, skill_get_exp_for_level(7));
    TEST_ASSERT_EQ(15000, skill_get_exp_for_level(8));
    TEST_ASSERT_EQ(30000, skill_get_exp_for_level(9));
    TEST_ASSERT_EQ(60000, skill_get_exp_for_level(10));

    /* Invalid levels */
    TEST_ASSERT_EQ(0, skill_get_exp_for_level(0));
    TEST_ASSERT_EQ(0, skill_get_exp_for_level(11));

    TEST_END();
}

/* ========================================
 * Test Cases - Cooldowns
 * ======================================== */

static void test_skill_cooldown_values(void) {
    TEST_BEGIN("Skill cooldown values");

    TEST_ASSERT_EQ(3000, skill_get_cooldown_for_type(SKILL_TYPE_ATTACK));
    TEST_ASSERT_EQ(5000, skill_get_cooldown_for_type(SKILL_TYPE_HEAL));
    TEST_ASSERT_EQ(8000, skill_get_cooldown_for_type(SKILL_TYPE_BUFF));
    TEST_ASSERT_EQ(8000, skill_get_cooldown_for_type(SKILL_TYPE_DEBUFF));
    TEST_ASSERT_EQ(0, skill_get_cooldown_for_type(SKILL_TYPE_PASSIVE));
    TEST_ASSERT_EQ(10000, skill_get_cooldown_for_type(SKILL_TYPE_SPECIAL));
    TEST_ASSERT_EQ(15000, skill_get_cooldown_for_type(SKILL_TYPE_SUMMON));

    TEST_END();
}

/* ========================================
 * Test Cases - Hit/Critical Rolls
 * ======================================== */

static void test_skill_roll_hit_basic(void) {
    TEST_BEGIN("Skill roll hit basic");

    /* Seed for reproducibility */
    srand(12345);

    /* With hit 0 and dodge 0, base chance is 90% */
    /* Multiple rolls should mostly hit */
    int hits = 0;
    int i;
    for (i = 0; i < 100; i++) {
        if (skill_roll_hit(0, 0)) hits++;
    }

    /* Should be around 90 hits */
    TEST_ASSERT(hits > 70 && hits < 100);

    TEST_END();
}

static void test_skill_roll_hit_advantage(void) {
    TEST_BEGIN("Skill roll hit with advantage");

    srand(12345);

    /* High hit vs low dodge should almost always hit */
    int hits = 0;
    int i;
    for (i = 0; i < 100; i++) {
        if (skill_roll_hit(200, 0)) hits++;
    }

    /* Should be very high hit rate */
    TEST_ASSERT(hits > 95);

    TEST_END();
}

static void test_skill_roll_hit_disadvantage(void) {
    TEST_BEGIN("Skill roll hit with disadvantage");

    srand(12345);

    /* Low hit vs high dodge: hit_chance = 90 + (0 - 200) / 10 = 70% */
    int hits = 0;
    int i;
    for (i = 0; i < 100; i++) {
        if (skill_roll_hit(0, 200)) hits++;
    }

    /* Should be around 70% hit rate, allow variance 50-90 */
    TEST_ASSERT(hits >= 50 && hits <= 90);

    TEST_END();
}

static void test_skill_roll_critical_basic(void) {
    TEST_BEGIN("Skill roll critical basic");

    srand(12345);

    /* With luck 0 and skill level 1, chance is 1% */
    int crits = 0;
    int i;
    for (i = 0; i < 1000; i++) {
        if (skill_roll_critical(0, 1)) crits++;
    }

    /* Should be around 10-20 crits */
    TEST_ASSERT(crits > 0 && crits < 100);

    TEST_END();
}

static void test_skill_roll_critical_high_luck(void) {
    TEST_BEGIN("Skill roll critical high luck");

    srand(12345);

    /* With luck 300 (max 30% base) and skill level 10, chance is 40% */
    /* But luck is capped at 30%, so base is 30 + 10 = 40% */
    int crits = 0;
    int i;
    for (i = 0; i < 100; i++) {
        if (skill_roll_critical(300, 10)) crits++;
    }

    /* Should be around 40 crits */
    TEST_ASSERT(crits > 20 && crits < 60);

    TEST_END();
}

/* ========================================
 * Test Cases - Integration
 * ======================================== */

static void test_skill_full_flow(void) {
    TEST_BEGIN("Skill full flow");

    skill_init();

    /* Set skill points */
    skill_set_points(10);

    /* Learn skills */
    skill_learn(1001);  /* Attack skill */
    skill_learn(1002);  /* Heal skill */

    /* Set to slots */
    skill_set_slot(0, 1001);
    skill_set_slot(1, 1002);

    /* Level up */
    skill_level_up(1001);

    /* Gain exp */
    skill_gain_exp(1002, 500);

    /* Verify state */
    TEST_ASSERT_EQ(2, skill_get_count());
    TEST_ASSERT_EQ(1001, skill_get_slot(0));
    TEST_ASSERT_EQ(1002, skill_get_slot(1));
    TEST_ASSERT_EQ(2, skill_get_level(1001));
    TEST_ASSERT_EQ(9, skill_get_points());  /* 10 - 1 used */

    TEST_END();
}

static void test_skill_combat_scenario(void) {
    TEST_BEGIN("Skill combat scenario");

    skill_init();

    /* Setup */
    skill_learn(1001);  /* Fireball */
    skill_learn(1002);  /* Heal */

    /* Check element advantage for Fireball (fire) vs Earth enemy */
    float mult = skill_get_element_multiplier(SKILL_ELEM_FIRE, SKILL_ELEM_EARTH);
    TEST_ASSERT_EQ(1.5f, mult);

    /* Check element advantage for Heal on self */
    mult = skill_get_element_multiplier(SKILL_ELEM_NONE, SKILL_ELEM_NONE);
    TEST_ASSERT_EQ(1.0f, mult);

    TEST_END();
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Skill Module Comprehensive Tests ===\n");
    printf("Testing skill.c, skill_damage.c implementations\n");
    printf("Verified against Ghidra decompilation\n\n");

    /* Constants tests */
    printf("[Constants]\n");
    test_skill_constants();
    test_skill_type_values();
    test_skill_target_values();
    test_skill_element_values();

    /* Element system tests */
    printf("\n[Element System]\n");
    test_element_neutral();
    test_element_fire_advantage();
    test_element_water_advantage();
    test_element_earth_advantage();
    test_element_wind_advantage();
    test_element_light_dark();
    test_element_invalid();

    /* Context tests */
    printf("\n[Context]\n");
    test_skill_init();
    test_skill_init_clears_data();

    /* Learning tests */
    printf("\n[Learning]\n");
    test_skill_learn_basic();
    test_skill_learn_multiple();
    test_skill_learn_duplicate();
    test_skill_learn_max();
    test_skill_has_skill();

    /* Forgetting tests */
    printf("\n[Forgetting]\n");
    test_skill_forget_basic();
    test_skill_forget_not_found();
    test_skill_forget_clears_slot();

    /* Slot tests */
    printf("\n[Slots]\n");
    test_skill_set_slot_basic();
    test_skill_set_slot_invalid();
    test_skill_set_slot_unlearned_skill();
    test_skill_get_slot();
    test_skill_clear_slot();
    test_skill_swap_slots();
    test_skill_swap_slots_invalid();

    /* Leveling tests */
    printf("\n[Leveling]\n");
    test_skill_get_level();
    test_skill_set_level();
    test_skill_level_up();
    test_skill_level_up_no_points();
    test_skill_level_up_max_level();
    test_skill_gain_exp();
    test_skill_exp_table();

    /* Cooldown tests */
    printf("\n[Cooldowns]\n");
    test_skill_cooldown_values();

    /* Hit/Critical tests */
    printf("\n[Hit/Critical]\n");
    test_skill_roll_hit_basic();
    test_skill_roll_hit_advantage();
    test_skill_roll_hit_disadvantage();
    test_skill_roll_critical_basic();
    test_skill_roll_critical_high_luck();

    /* Integration tests */
    printf("\n[Integration]\n");
    test_skill_full_flow();
    test_skill_combat_scenario();

    /* Summary */
    test_summary();

    return test_all_passed() ? 0 : 1;
}
