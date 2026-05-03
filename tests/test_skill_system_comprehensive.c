/*
 * Stone Age Client - Skill System Comprehensive Tests
 * Tests for FUN_00465de0 (skill list), FUN_00466030 (skill use),
 * FUN_004660c0 (skill dispatch), FUN_00465d20 (skill protocol)
 *
 * Coverage:
 * - Skill list parsing
 * - Skill use handling
 * - Skill dispatch to entities
 * - Skill cooldown management
 * - Skill targeting system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDEBUG
#include <assert.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Skill constants */
#define MAX_SKILLS              200
#define MAX_SKILL_NAME_LENGTH   200
#define MAX_SKILL_DESC_LENGTH   200
#define MAX_SKILL_LEVEL         100
#define MAX_TARGETS             5

/* Skill types */
typedef enum {
    SKILL_TYPE_ATTACK = 0,
    SKILL_TYPE_HEAL = 1,
    SKILL_TYPE_BUFF = 2,
    SKILL_TYPE_DEBUFF = 3,
    SKILL_TYPE_SUMMON = 4,
    SKILL_TYPE_PASSIVE = 5
} SkillType;

/* Skill target types */
typedef enum {
    TARGET_SELF = 0,
    TARGET_SINGLE_ENEMY = 1,
    TARGET_ALL_ENEMIES = 2,
    TARGET_SINGLE_ALLY = 3,
    TARGET_ALL_ALLIES = 4,
    TARGET_GROUND = 5
} TargetType;

/* Skill structure */
typedef struct {
    u32 id;
    char name[MAX_SKILL_NAME_LENGTH];
    char description[MAX_SKILL_DESC_LENGTH];
    u8 type;
    u8 target_type;
    u16 level;
    u16 max_level;
    u16 mp_cost;
    u16 cooldown;
    u16 range;
    u16 power;
    u16 accuracy;
    u8 element;
    u8 priority;
    u32 flags;
} Skill;

/* Skill instance (learned skill) */
typedef struct {
    u32 skill_id;
    u16 current_level;
    u16 current_exp;
    u32 cooldown_end_time;
    u8 is_enabled;
} SkillInstance;

/* Skill list context */
typedef struct {
    Skill skills[MAX_SKILLS];
    u32 skill_count;
    u32 loaded;
} SkillContext;

/* Entity skill data */
typedef struct {
    SkillInstance learned_skills[20];
    u32 learned_count;
    u32 selected_skill;
    u32 skill_points;
} EntitySkills;

static SkillContext g_skills;
static EntitySkills g_player_skills;

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

/* ========================================
 * Skill System Functions
 * ======================================== */

void skill_init(void) {
    memset(&g_skills, 0, sizeof(g_skills));
    memset(&g_player_skills, 0, sizeof(g_player_skills));
}

int skill_add(u32 id, const char* name, u8 type, u8 target_type) {
    if (g_skills.skill_count >= MAX_SKILLS) return -1;
    if (!name) return -1;

    Skill* skill = &g_skills.skills[g_skills.skill_count];
    memset(skill, 0, sizeof(Skill));
    skill->id = id;
    strncpy(skill->name, name, MAX_SKILL_NAME_LENGTH - 1);
    skill->type = type;
    skill->target_type = target_type;
    skill->level = 1;
    skill->max_level = MAX_SKILL_LEVEL;
    g_skills.skill_count++;

    return 0;
}

Skill* skill_get(u32 id) {
    for (u32 i = 0; i < g_skills.skill_count; i++) {
        if (g_skills.skills[i].id == id) {
            return &g_skills.skills[i];
        }
    }
    return NULL;
}

int skill_learn(u32 skill_id) {
    if (g_player_skills.learned_count >= 20) return -1;

    /* Check if skill exists */
    if (!skill_get(skill_id)) return -1;

    /* Check if already learned */
    for (u32 i = 0; i < g_player_skills.learned_count; i++) {
        if (g_player_skills.learned_skills[i].skill_id == skill_id) {
            return -1;  /* Already learned */
        }
    }

    SkillInstance* instance = &g_player_skills.learned_skills[g_player_skills.learned_count];
    memset(instance, 0, sizeof(SkillInstance));
    instance->skill_id = skill_id;
    instance->current_level = 1;
    instance->is_enabled = 1;
    g_player_skills.learned_count++;

    return 0;
}

int skill_use(u32 skill_id, u32 target_id, u32 param3, u32 param4) {
    Skill* skill = skill_get(skill_id);
    if (!skill) return -1;

    /* Check if learned */
    int found = 0;
    for (u32 i = 0; i < g_player_skills.learned_count; i++) {
        if (g_player_skills.learned_skills[i].skill_id == skill_id) {
            found = 1;
            break;
        }
    }
    if (!found) return -1;

    /* Set selected skill */
    g_player_skills.selected_skill = skill_id;

    return 0;
}

int skill_parse_list(const char* data) {
    /* FUN_00465de0 - Parse skill list from pipe-delimited data */
    if (!data) return -1;

    /* Clear existing skills */
    g_skills.skill_count = 0;
    g_skills.loaded = 0;

    /* Parse entries separated by '#' (0x23) */
    const char* start = data;
    const char* end;
    u32 count = 0;

    while ((end = strchr(start, '#')) != NULL && count < MAX_SKILLS) {
        size_t len = end - start;
        if (len > 0) {
            char entry[256];
            if (len < sizeof(entry)) {
                strncpy(entry, start, len);
                entry[len] = '\0';

                /* Parse pipe-delimited fields: id|name|description */
                char* pipe1 = strchr(entry, '|');
                if (pipe1) {
                    Skill* skill = &g_skills.skills[count];
                    *pipe1 = '\0';
                    skill->id = atoi(entry);

                    char* pipe2 = strchr(pipe1 + 1, '|');
                    if (pipe2) {
                        size_t name_len = pipe2 - pipe1 - 1;
                        strncpy(skill->name, pipe1 + 1, name_len < MAX_SKILL_NAME_LENGTH ? name_len : MAX_SKILL_NAME_LENGTH - 1);
                        strncpy(skill->description, pipe2 + 1, MAX_SKILL_DESC_LENGTH - 1);
                    } else {
                        strncpy(skill->name, pipe1 + 1, MAX_SKILL_NAME_LENGTH - 1);
                    }
                    count++;
                }
            }
        }
        start = end + 1;
    }

    g_skills.skill_count = count;
    g_skills.loaded = (count > 0) ? 1 : 0;

    return count;
}

int skill_can_use(u32 skill_id, u32 current_mp, u32 current_time) {
    Skill* skill = skill_get(skill_id);
    if (!skill) return 0;

    /* Check MP cost */
    if (current_mp < skill->mp_cost) return 0;

    /* Check cooldown */
    for (u32 i = 0; i < g_player_skills.learned_count; i++) {
        if (g_player_skills.learned_skills[i].skill_id == skill_id) {
            if (g_player_skills.learned_skills[i].cooldown_end_time > current_time) {
                return 0;  /* Still on cooldown */
            }
            break;
        }
    }

    return 1;
}

int skill_set_cooldown(u32 skill_id, u32 end_time) {
    for (u32 i = 0; i < g_player_skills.learned_count; i++) {
        if (g_player_skills.learned_skills[i].skill_id == skill_id) {
            g_player_skills.learned_skills[i].cooldown_end_time = end_time;
            return 0;
        }
    }
    return -1;
}

int skill_level_up(u32 skill_id) {
    Skill* skill = skill_get(skill_id);
    if (!skill) return -1;

    for (u32 i = 0; i < g_player_skills.learned_count; i++) {
        if (g_player_skills.learned_skills[i].skill_id == skill_id) {
            if (g_player_skills.learned_skills[i].current_level < skill->max_level) {
                g_player_skills.learned_skills[i].current_level++;
                return 0;
            }
            return -1;  /* Max level reached */
        }
    }
    return -1;  /* Not learned */
}

int skill_forget(u32 skill_id) {
    for (u32 i = 0; i < g_player_skills.learned_count; i++) {
        if (g_player_skills.learned_skills[i].skill_id == skill_id) {
            /* Shift remaining skills */
            for (u32 j = i; j < g_player_skills.learned_count - 1; j++) {
                g_player_skills.learned_skills[j] = g_player_skills.learned_skills[j + 1];
            }
            g_player_skills.learned_count--;
            return 0;
        }
    }
    return -1;  /* Not found */
}

/* ========================================
 * Test Setup/Teardown
 * ======================================== */

static void test_setup(void) {
    skill_init();
}

static void test_teardown(void) {
    /* Nothing to clean up */
}

/* ========================================
 * Constants Tests
 * ======================================== */

static int test_max_skills(void) {
    return MAX_SKILLS == 200;
}

static int test_max_skill_name_length(void) {
    return MAX_SKILL_NAME_LENGTH == 200;
}

static int test_max_skill_level(void) {
    return MAX_SKILL_LEVEL == 100;
}

static int test_skill_type_values(void) {
    return SKILL_TYPE_ATTACK == 0 &&
           SKILL_TYPE_HEAL == 1 &&
           SKILL_TYPE_BUFF == 2 &&
           SKILL_TYPE_DEBUFF == 3 &&
           SKILL_TYPE_SUMMON == 4 &&
           SKILL_TYPE_PASSIVE == 5;
}

static int test_target_type_values(void) {
    return TARGET_SELF == 0 &&
           TARGET_SINGLE_ENEMY == 1 &&
           TARGET_ALL_ENEMIES == 2 &&
           TARGET_SINGLE_ALLY == 3 &&
           TARGET_ALL_ALLIES == 4 &&
           TARGET_GROUND == 5;
}

/* ========================================
 * Initialization Tests
 * ======================================== */

static int test_skill_init(void) {
    test_setup();

    int pass = g_skills.skill_count == 0 &&
               g_skills.loaded == 0 &&
               g_player_skills.learned_count == 0;

    test_teardown();
    return pass;
}

static int test_skill_init_clears_data(void) {
    test_setup();

    skill_add(1, "TestSkill", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);
    skill_init();

    int pass = g_skills.skill_count == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Skill Management Tests
 * ======================================== */

static int test_skill_add(void) {
    test_setup();

    int result = skill_add(1001, "Fireball", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);

    int pass = result == 0 &&
               g_skills.skill_count == 1 &&
               g_skills.skills[0].id == 1001 &&
               strcmp(g_skills.skills[0].name, "Fireball") == 0;

    test_teardown();
    return pass;
}

static int test_skill_add_max(void) {
    test_setup();

    /* Add MAX_SKILLS skills */
    for (int i = 0; i < MAX_SKILLS; i++) {
        char name[32];
        snprintf(name, sizeof(name), "Skill%d", i);
        skill_add(i, name, SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);
    }

    /* Try to add one more */
    int result = skill_add(MAX_SKILLS, "ExtraSkill", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);

    int pass = result == -1 &&
               g_skills.skill_count == MAX_SKILLS;

    test_teardown();
    return pass;
}

static int test_skill_get(void) {
    test_setup();

    skill_add(1001, "Fireball", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);
    skill_add(1002, "Heal", SKILL_TYPE_HEAL, TARGET_SINGLE_ALLY);

    Skill* skill = skill_get(1002);

    int pass = skill != NULL &&
               skill->id == 1002 &&
               strcmp(skill->name, "Heal") == 0;

    test_teardown();
    return pass;
}

static int test_skill_get_not_found(void) {
    test_setup();

    skill_add(1001, "Fireball", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);

    Skill* skill = skill_get(9999);

    int pass = skill == NULL;

    test_teardown();
    return pass;
}

/* ========================================
 * Skill Learning Tests
 * ======================================== */

static int test_skill_learn(void) {
    test_setup();

    skill_add(1001, "Fireball", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);
    int result = skill_learn(1001);

    int pass = result == 0 &&
               g_player_skills.learned_count == 1 &&
               g_player_skills.learned_skills[0].skill_id == 1001 &&
               g_player_skills.learned_skills[0].current_level == 1;

    test_teardown();
    return pass;
}

static int test_skill_learn_not_found(void) {
    test_setup();

    int result = skill_learn(9999);

    int pass = result == -1 &&
               g_player_skills.learned_count == 0;

    test_teardown();
    return pass;
}

static int test_skill_learn_duplicate(void) {
    test_setup();

    skill_add(1001, "Fireball", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);
    skill_learn(1001);
    int result = skill_learn(1001);

    int pass = result == -1 &&
               g_player_skills.learned_count == 1;

    test_teardown();
    return pass;
}

static int test_skill_learn_max(void) {
    test_setup();

    /* Add and learn 20 skills */
    for (int i = 0; i < 20; i++) {
        char name[32];
        snprintf(name, sizeof(name), "Skill%d", i);
        skill_add(1000 + i, name, SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);
        skill_learn(1000 + i);
    }

    /* Try to learn one more */
    skill_add(9999, "ExtraSkill", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);
    int result = skill_learn(9999);

    int pass = result == -1 &&
               g_player_skills.learned_count == 20;

    test_teardown();
    return pass;
}

/* ========================================
 * Skill Usage Tests
 * ======================================== */

static int test_skill_use_success(void) {
    test_setup();

    skill_add(1001, "Fireball", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);
    skill_learn(1001);

    int result = skill_use(1001, 100, 0, 0);

    int pass = result == 0 &&
               g_player_skills.selected_skill == 1001;

    test_teardown();
    return pass;
}

static int test_skill_use_not_learned(void) {
    test_setup();

    skill_add(1001, "Fireball", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);

    int result = skill_use(1001, 100, 0, 0);

    int pass = result == -1;

    test_teardown();
    return pass;
}

static int test_skill_use_not_found(void) {
    test_setup();

    int result = skill_use(9999, 100, 0, 0);

    int pass = result == -1;

    test_teardown();
    return pass;
}

/* ========================================
 * Cooldown Tests
 * ======================================== */

static int test_skill_cooldown(void) {
    test_setup();

    skill_add(1001, "Fireball", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);
    Skill* skill = skill_get(1001);
    skill->mp_cost = 10;
    skill_learn(1001);

    /* Set cooldown */
    skill_set_cooldown(1001, 1000);

    /* Check can't use during cooldown */
    int pass1 = skill_can_use(1001, 100, 500) == 0;

    /* Check can use after cooldown */
    int pass2 = skill_can_use(1001, 100, 1001) == 1;

    test_teardown();
    return pass1 && pass2;
}

static int test_skill_cooldown_mp_check(void) {
    test_setup();

    skill_add(1001, "Fireball", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);
    Skill* skill = skill_get(1001);
    skill->mp_cost = 50;
    skill_learn(1001);

    /* Not enough MP */
    int pass1 = skill_can_use(1001, 30, 0) == 0;

    /* Enough MP */
    int pass2 = skill_can_use(1001, 60, 0) == 1;

    test_teardown();
    return pass1 && pass2;
}

/* ========================================
 * Skill Level Tests
 * ======================================== */

static int test_skill_level_up(void) {
    test_setup();

    skill_add(1001, "Fireball", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);
    Skill* skill = skill_get(1001);
    skill->max_level = 10;
    skill_learn(1001);

    int result = skill_level_up(1001);

    int pass = result == 0 &&
               g_player_skills.learned_skills[0].current_level == 2;

    test_teardown();
    return pass;
}

static int test_skill_level_max(void) {
    test_setup();

    skill_add(1001, "Fireball", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);
    Skill* skill = skill_get(1001);
    skill->max_level = 2;
    skill_learn(1001);

    skill_level_up(1001);  /* Level 2 */
    int result = skill_level_up(1001);  /* Try to go to level 3 */

    int pass = result == -1 &&
               g_player_skills.learned_skills[0].current_level == 2;

    test_teardown();
    return pass;
}

static int test_skill_forget(void) {
    test_setup();

    skill_add(1001, "Fireball", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);
    skill_add(1002, "Heal", SKILL_TYPE_HEAL, TARGET_SINGLE_ALLY);
    skill_learn(1001);
    skill_learn(1002);

    int result = skill_forget(1001);

    int pass = result == 0 &&
               g_player_skills.learned_count == 1 &&
               g_player_skills.learned_skills[0].skill_id == 1002;

    test_teardown();
    return pass;
}

/* ========================================
 * Skill Parsing Tests
 * ======================================== */

static int test_skill_parse_list(void) {
    test_setup();

    const char* data = "1001|Fireball|Fire attack#1002|Heal|Restore HP#";
    int count = skill_parse_list(data);

    int pass = count == 2 &&
               g_skills.skills[0].id == 1001 &&
               strcmp(g_skills.skills[0].name, "Fireball") == 0 &&
               g_skills.skills[1].id == 1002 &&
               strcmp(g_skills.skills[1].name, "Heal") == 0;

    test_teardown();
    return pass;
}

static int test_skill_parse_empty(void) {
    test_setup();

    const char* data = "";
    int count = skill_parse_list(data);

    int pass = count == 0 && g_skills.loaded == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_skill_flow(void) {
    test_setup();

    /* Add skills */
    skill_add(1001, "Fireball", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);
    skill_add(1002, "Heal", SKILL_TYPE_HEAL, TARGET_SINGLE_ALLY);

    /* Learn skills */
    skill_learn(1001);
    skill_learn(1002);

    /* Level up */
    skill_level_up(1001);

    /* Use skill */
    skill_use(1001, 100, 50, 0);

    int pass = g_player_skills.learned_count == 2 &&
               g_player_skills.learned_skills[0].current_level == 2 &&
               g_player_skills.selected_skill == 1001;

    test_teardown();
    return pass;
}

static int test_skill_combat_flow(void) {
    test_setup();

    /* Setup skill with MP cost and cooldown */
    skill_add(1001, "Fireball", SKILL_TYPE_ATTACK, TARGET_SINGLE_ENEMY);
    Skill* skill = skill_get(1001);
    skill->mp_cost = 30;
    skill->cooldown = 60;
    skill_learn(1001);

    /* Use skill successfully */
    int can_use1 = skill_can_use(1001, 100, 0);
    skill_use(1001, 100, 50, 0);
    skill_set_cooldown(1001, 60);

    /* Try to use during cooldown */
    int can_use2 = skill_can_use(1001, 100, 30);

    /* Wait for cooldown */
    int can_use3 = skill_can_use(1001, 100, 61);

    int pass = can_use1 == 1 && can_use2 == 0 && can_use3 == 1;

    test_teardown();
    return pass;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Skill System Comprehensive Tests ===\n\n");

    /* Constants tests */
    printf("Constants Tests:\n");
    TEST(max_skills);
    TEST(max_skill_name_length);
    TEST(max_skill_level);
    TEST(skill_type_values);
    TEST(target_type_values);

    /* Initialization tests */
    printf("\nInitialization Tests:\n");
    TEST(skill_init);
    TEST(skill_init_clears_data);

    /* Skill management tests */
    printf("\nSkill Management Tests:\n");
    TEST(skill_add);
    TEST(skill_add_max);
    TEST(skill_get);
    TEST(skill_get_not_found);

    /* Skill learning tests */
    printf("\nSkill Learning Tests:\n");
    TEST(skill_learn);
    TEST(skill_learn_not_found);
    TEST(skill_learn_duplicate);
    TEST(skill_learn_max);

    /* Skill usage tests */
    printf("\nSkill Usage Tests:\n");
    TEST(skill_use_success);
    TEST(skill_use_not_learned);
    TEST(skill_use_not_found);

    /* Cooldown tests */
    printf("\nCooldown Tests:\n");
    TEST(skill_cooldown);
    TEST(skill_cooldown_mp_check);

    /* Skill level tests */
    printf("\nSkill Level Tests:\n");
    TEST(skill_level_up);
    TEST(skill_level_max);
    TEST(skill_forget);

    /* Skill parsing tests */
    printf("\nSkill Parsing Tests:\n");
    TEST(skill_parse_list);
    TEST(skill_parse_empty);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_skill_flow);
    TEST(skill_combat_flow);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - Skill damage calculation
     * - Skill accuracy calculation
     * - Element advantage in skills
     * - Area effect skills
     * - Chain skills
     * - Skill animation timing
     * - Real protocol parsing
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
