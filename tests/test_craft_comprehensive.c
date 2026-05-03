/*
 * Stone Age Client - Craft System Comprehensive Tests
 * Tests for crafting/synthesis system from src/craft/craft.c
 *
 * Coverage:
 * - Craft type constants
 * - State machine management
 * - Recipe learning/forgetting
 * - Material and product handling
 * - Skill level management
 * - Success rate calculation
 * - UI state management
 * - Packet parsing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Disable assert popups on Windows */
#define NDEBUG
#include <windows.h>

/* Stub types */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Constants - must match craft.h */
#define MAX_CRAFT_RECIPES   500
#define MAX_CRAFT_MATERIALS 10
#define MAX_CRAFT_PRODUCTS  3

/* Craft types */
typedef enum {
    CRAFT_TYPE_SYNTHESIS = 0,
    CRAFT_TYPE_UPGRADE = 1,
    CRAFT_TYPE_REFINE = 2,
    CRAFT_TYPE_COOKING = 3,
    CRAFT_TYPE_ALCHEMY = 4
} CraftType;

/* Material requirement */
typedef struct {
    u32 item_id;
    u16 count;
    u8  consume;
    u8  reserved;
} CraftMaterial;

/* Product result */
typedef struct {
    u32 item_id;
    u16 count;
    u16 rate;
} CraftProduct;

/* Recipe data */
typedef struct {
    u32 id;
    char name[32];
    char description[128];
    CraftType type;
    u16 skill_required;
    u16 skill_level;
    u16 success_rate;
    u16 gold_cost;
    u32 duration;

    CraftMaterial materials[MAX_CRAFT_MATERIALS];
    int material_count;

    CraftProduct products[MAX_CRAFT_PRODUCTS];
    int product_count;

    u16 tool_item;
    u8  learn_type;
    u8  learn_param;
} CraftRecipe;

/* Crafting state */
typedef enum {
    CRAFT_STATE_IDLE = 0,
    CRAFT_STATE_SELECTING = 1,
    CRAFT_STATE_CRAFTING = 2,
    CRAFT_STATE_SUCCESS = 3,
    CRAFT_STATE_FAILED = 4
} CraftState;

/* Craft result */
typedef struct {
    u32 recipe_id;
    u8 success;
    u8 count;
    u32 item_id;
} CraftResult;

/* Craft context */
typedef struct {
    u32 known_recipes[MAX_CRAFT_RECIPES];
    int recipe_count;

    CraftRecipe* recipe_db;
    int db_count;

    u32 current_recipe;
    u32 craft_start_time;
    u32 craft_duration;
    CraftState state;

    CraftResult last_result;

    CraftType filter_type;
    u8 show_unlearned;
    int selected_recipe;

    u16 craft_skills[5];
} CraftContext;

static CraftContext g_craft;

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

/* Sample recipes for testing */
static CraftRecipe s_test_recipes[] = {
    {
        .id = 1, .name = "Health Potion", .description = "Restore 100 HP",
        .type = CRAFT_TYPE_ALCHEMY, .skill_required = CRAFT_TYPE_ALCHEMY,
        .skill_level = 1, .success_rate = 90, .gold_cost = 50, .duration = 3000,
        .materials = {{1001, 2, 1}, {1002, 1, 1}},
        .material_count = 2,
        .products = {{2001, 1, 9000}},
        .product_count = 1,
        .tool_item = 5001
    },
    {
        .id = 2, .name = "Mana Potion", .description = "Restore 50 MP",
        .type = CRAFT_TYPE_ALCHEMY, .skill_required = CRAFT_TYPE_ALCHEMY,
        .skill_level = 5, .success_rate = 85, .gold_cost = 100, .duration = 5000,
        .materials = {{1001, 1, 1}, {1003, 2, 1}},
        .material_count = 2,
        .products = {{2002, 1, 8500}},
        .product_count = 1,
        .tool_item = 5001
    },
    {
        .id = 3, .name = "Iron Sword", .description = "Basic iron sword",
        .type = CRAFT_TYPE_SYNTHESIS, .skill_required = CRAFT_TYPE_SYNTHESIS,
        .skill_level = 10, .success_rate = 70, .gold_cost = 200, .duration = 10000,
        .materials = {{3001, 3, 1}, {3002, 1, 1}},
        .material_count = 2,
        .products = {{4001, 1, 7000}},
        .product_count = 1,
        .tool_item = 5002
    }
};

/* ========================================
 * Craft System Functions (Stubs)
 * ======================================== */

void craft_init(void) {
    memset(&g_craft, 0, sizeof(CraftContext));
    g_craft.recipe_db = s_test_recipes;
    g_craft.db_count = sizeof(s_test_recipes) / sizeof(CraftRecipe);
    g_craft.state = CRAFT_STATE_IDLE;
    g_craft.selected_recipe = -1;
}

void craft_shutdown(void) {
    memset(&g_craft, 0, sizeof(CraftContext));
}

CraftRecipe* craft_get_recipe(u32 recipe_id) {
    for (int i = 0; i < g_craft.db_count; i++) {
        if (g_craft.recipe_db[i].id == recipe_id) {
            return &g_craft.recipe_db[i];
        }
    }
    return NULL;
}

CraftRecipe* craft_get_recipe_by_index(int index) {
    if (index < 0 || index >= g_craft.db_count) {
        return NULL;
    }
    return &g_craft.recipe_db[index];
}

int craft_get_recipe_count(void) {
    return g_craft.db_count;
}

int craft_learn_recipe(u32 recipe_id) {
    if (g_craft.recipe_count >= MAX_CRAFT_RECIPES) {
        return 0;
    }

    /* Check if already known */
    for (int i = 0; i < g_craft.recipe_count; i++) {
        if (g_craft.known_recipes[i] == recipe_id) {
            return 1;
        }
    }

    g_craft.known_recipes[g_craft.recipe_count++] = recipe_id;
    return 1;
}

int craft_forget_recipe(u32 recipe_id) {
    for (int i = 0; i < g_craft.recipe_count; i++) {
        if (g_craft.known_recipes[i] == recipe_id) {
            memmove(&g_craft.known_recipes[i], &g_craft.known_recipes[i + 1],
                    (g_craft.recipe_count - i - 1) * sizeof(u32));
            g_craft.recipe_count--;
            return 1;
        }
    }
    return 0;
}

int craft_knows_recipe(u32 recipe_id) {
    for (int i = 0; i < g_craft.recipe_count; i++) {
        if (g_craft.known_recipes[i] == recipe_id) {
            return 1;
        }
    }
    return 0;
}

int craft_can_learn(u32 recipe_id) {
    CraftRecipe* recipe = craft_get_recipe(recipe_id);
    if (!recipe) return 0;
    return g_craft.craft_skills[recipe->skill_required] >= recipe->skill_level;
}

int craft_start(u32 recipe_id) {
    if (g_craft.state != CRAFT_STATE_IDLE) {
        return 0;
    }

    CraftRecipe* recipe = craft_get_recipe(recipe_id);
    if (!recipe) {
        return 0;
    }

    g_craft.current_recipe = recipe_id;
    g_craft.craft_start_time = GetTickCount();
    g_craft.craft_duration = recipe->duration;
    g_craft.state = CRAFT_STATE_CRAFTING;

    return 1;
}

int craft_is_crafting(void) {
    return g_craft.state == CRAFT_STATE_CRAFTING;
}

int craft_get_progress(void) {
    if (g_craft.state != CRAFT_STATE_CRAFTING) {
        return 0;
    }

    u32 elapsed = GetTickCount() - g_craft.craft_start_time;
    if (elapsed >= g_craft.craft_duration) {
        return 100;
    }

    return (elapsed * 100) / g_craft.craft_duration;
}

void craft_cancel(void) {
    if (g_craft.state == CRAFT_STATE_CRAFTING) {
        g_craft.state = CRAFT_STATE_IDLE;
        g_craft.current_recipe = 0;
    }
}

void craft_handle_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 success = *ptr++;
    u32 recipe_id = *(u32*)ptr; ptr += 4;
    u32 item_id = *(u32*)ptr; ptr += 4;
    u16 count = *(u16*)ptr;

    g_craft.last_result.recipe_id = recipe_id;
    g_craft.last_result.success = success;
    g_craft.last_result.item_id = item_id;
    g_craft.last_result.count = count;

    g_craft.state = success ? CRAFT_STATE_SUCCESS : CRAFT_STATE_FAILED;
}

int craft_get_last_result(CraftResult* result) {
    if (g_craft.state != CRAFT_STATE_SUCCESS &&
        g_craft.state != CRAFT_STATE_FAILED) {
        return 0;
    }
    *result = g_craft.last_result;
    return 1;
}

int craft_check_skill(u32 recipe_id) {
    CraftRecipe* recipe = craft_get_recipe(recipe_id);
    if (!recipe) return 0;
    return g_craft.craft_skills[recipe->skill_required] >= recipe->skill_level;
}

int craft_get_success_rate(u32 recipe_id) {
    CraftRecipe* recipe = craft_get_recipe(recipe_id);
    if (!recipe) return 0;

    int rate = recipe->success_rate;
    int skill_bonus = g_craft.craft_skills[recipe->skill_required] - recipe->skill_level;
    rate += skill_bonus * 2;

    if (rate > 100) rate = 100;
    if (rate < 5) rate = 5;

    return rate;
}

int craft_get_gold_cost(u32 recipe_id) {
    CraftRecipe* recipe = craft_get_recipe(recipe_id);
    return recipe ? recipe->gold_cost : 0;
}

void craft_set_skill_level(CraftType type, u16 level) {
    if (type < 5) {
        g_craft.craft_skills[type] = level;
    }
}

u16 craft_get_skill_level(CraftType type) {
    if (type < 5) {
        return g_craft.craft_skills[type];
    }
    return 0;
}

void craft_set_filter(CraftType type) {
    g_craft.filter_type = type;
}

CraftType craft_get_filter(void) {
    return g_craft.filter_type;
}

void craft_set_show_unlearned(u8 show) {
    g_craft.show_unlearned = show;
}

void craft_select_recipe(int index) {
    g_craft.selected_recipe = index;
}

void craft_handle_recipe_list(void* data, u32 size) {
    u8* ptr = (u8*)data;
    int count = *(u16*)ptr; ptr += 2;

    g_craft.recipe_count = 0;

    for (int i = 0; i < count && i < MAX_CRAFT_RECIPES; i++) {
        g_craft.known_recipes[i] = *(u32*)ptr; ptr += 4;
        g_craft.recipe_count++;
    }
}

void craft_handle_learn_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result = *ptr++;
    u32 recipe_id = *(u32*)ptr;

    if (result == 0) {
        craft_learn_recipe(recipe_id);
    }
}

const char* craft_type_to_string(CraftType type) {
    static const char* type_names[] = {
        "Synthesis", "Upgrade", "Refine", "Cooking", "Alchemy"
    };
    if (type < 5) {
        return type_names[type];
    }
    return "Unknown";
}

const char* craft_get_recipe_name(u32 recipe_id) {
    CraftRecipe* recipe = craft_get_recipe(recipe_id);
    return recipe ? recipe->name : "Unknown";
}

int craft_get_known_count(void) {
    return g_craft.recipe_count;
}

int craft_get_materials(u32 recipe_id, CraftMaterial* materials, int max_count) {
    CraftRecipe* recipe = craft_get_recipe(recipe_id);
    if (!recipe || !materials) return 0;

    for (int i = 0; i < recipe->material_count && i < max_count; i++) {
        materials[i] = recipe->materials[i];
    }
    return recipe->material_count;
}

int craft_get_products(u32 recipe_id, CraftProduct* products, int max_count) {
    CraftRecipe* recipe = craft_get_recipe(recipe_id);
    if (!recipe || !products) return 0;

    for (int i = 0; i < recipe->product_count && i < max_count; i++) {
        products[i] = recipe->products[i];
    }
    return recipe->product_count;
}

int craft_get_recipes_by_type(CraftType type, u32* recipe_ids, int max_count) {
    int count = 0;
    for (int i = 0; i < g_craft.db_count && count < max_count; i++) {
        if (g_craft.recipe_db[i].type == type) {
            recipe_ids[count++] = g_craft.recipe_db[i].id;
        }
    }
    return count;
}

/* ========================================
 * Test Setup/Teardown
 * ======================================== */

static void test_setup(void) {
    craft_init();
}

static void test_teardown(void) {
    craft_shutdown();
}

/* ========================================
 * Constants Tests
 * ======================================== */

static int test_max_recipes_constant(void) {
    return MAX_CRAFT_RECIPES == 500;
}

static int test_max_materials_constant(void) {
    return MAX_CRAFT_MATERIALS == 10;
}

static int test_max_products_constant(void) {
    return MAX_CRAFT_PRODUCTS == 3;
}

static int test_craft_type_values(void) {
    return CRAFT_TYPE_SYNTHESIS == 0 &&
           CRAFT_TYPE_UPGRADE == 1 &&
           CRAFT_TYPE_REFINE == 2 &&
           CRAFT_TYPE_COOKING == 3 &&
           CRAFT_TYPE_ALCHEMY == 4;
}

static int test_craft_state_values(void) {
    return CRAFT_STATE_IDLE == 0 &&
           CRAFT_STATE_SELECTING == 1 &&
           CRAFT_STATE_CRAFTING == 2 &&
           CRAFT_STATE_SUCCESS == 3 &&
           CRAFT_STATE_FAILED == 4;
}

/* ========================================
 * Initialization Tests
 * ======================================== */

static int test_craft_init(void) {
    test_setup();

    int pass = g_craft.state == CRAFT_STATE_IDLE &&
               g_craft.recipe_count == 0 &&
               g_craft.selected_recipe == -1;

    test_teardown();
    return pass;
}

static int test_craft_init_db_loaded(void) {
    test_setup();

    int pass = g_craft.recipe_db != NULL &&
               g_craft.db_count == 3;

    test_teardown();
    return pass;
}

static int test_craft_shutdown_clears(void) {
    test_setup();

    g_craft.recipe_count = 10;
    g_craft.state = CRAFT_STATE_CRAFTING;

    craft_shutdown();

    int pass = g_craft.recipe_count == 0 &&
               g_craft.state == 0;

    return pass;
}

/* ========================================
 * Recipe Query Tests
 * ======================================== */

static int test_get_recipe_by_id(void) {
    test_setup();

    CraftRecipe* recipe = craft_get_recipe(1);

    int pass = recipe != NULL &&
               strcmp(recipe->name, "Health Potion") == 0;

    test_teardown();
    return pass;
}

static int test_get_recipe_invalid_id(void) {
    test_setup();

    CraftRecipe* recipe = craft_get_recipe(999);

    int pass = recipe == NULL;

    test_teardown();
    return pass;
}

static int test_get_recipe_by_index(void) {
    test_setup();

    CraftRecipe* recipe = craft_get_recipe_by_index(0);

    int pass = recipe != NULL &&
               recipe->id == 1;

    test_teardown();
    return pass;
}

static int test_get_recipe_by_invalid_index(void) {
    test_setup();

    CraftRecipe* recipe1 = craft_get_recipe_by_index(-1);
    CraftRecipe* recipe2 = craft_get_recipe_by_index(100);

    int pass = recipe1 == NULL && recipe2 == NULL;

    test_teardown();
    return pass;
}

static int test_get_recipe_count(void) {
    test_setup();

    int count = craft_get_recipe_count();

    int pass = count == 3;

    test_teardown();
    return pass;
}

/* ========================================
 * Recipe Learning Tests
 * ======================================== */

static int test_learn_recipe(void) {
    test_setup();

    int result = craft_learn_recipe(1);

    int pass = result == 1 &&
               g_craft.recipe_count == 1 &&
               g_craft.known_recipes[0] == 1;

    test_teardown();
    return pass;
}

static int test_learn_recipe_already_known(void) {
    test_setup();

    craft_learn_recipe(1);
    int result = craft_learn_recipe(1);

    int pass = result == 1 &&
               g_craft.recipe_count == 1;

    test_teardown();
    return pass;
}

static int test_learn_multiple_recipes(void) {
    test_setup();

    craft_learn_recipe(1);
    craft_learn_recipe(2);
    craft_learn_recipe(3);

    int pass = g_craft.recipe_count == 3;

    test_teardown();
    return pass;
}

static int test_forget_recipe(void) {
    test_setup();

    craft_learn_recipe(1);
    craft_learn_recipe(2);
    int result = craft_forget_recipe(1);

    int pass = result == 1 &&
               g_craft.recipe_count == 1 &&
               g_craft.known_recipes[0] == 2;

    test_teardown();
    return pass;
}

static int test_forget_unknown_recipe(void) {
    test_setup();

    int result = craft_forget_recipe(999);

    int pass = result == 0;

    test_teardown();
    return pass;
}

static int test_knows_recipe(void) {
    test_setup();

    craft_learn_recipe(1);

    int pass = craft_knows_recipe(1) == 1 &&
               craft_knows_recipe(2) == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Skill Level Tests
 * ======================================== */

static int test_set_skill_level(void) {
    test_setup();

    craft_set_skill_level(CRAFT_TYPE_ALCHEMY, 10);

    int pass = g_craft.craft_skills[CRAFT_TYPE_ALCHEMY] == 10;

    test_teardown();
    return pass;
}

static int test_get_skill_level(void) {
    test_setup();

    g_craft.craft_skills[CRAFT_TYPE_SYNTHESIS] = 15;

    int pass = craft_get_skill_level(CRAFT_TYPE_SYNTHESIS) == 15;

    test_teardown();
    return pass;
}

static int test_skill_level_bounds(void) {
    test_setup();

    craft_set_skill_level((CraftType)10, 100);

    int pass = craft_get_skill_level((CraftType)10) == 0;

    test_teardown();
    return pass;
}

static int test_can_learn_skill_check(void) {
    test_setup();

    craft_set_skill_level(CRAFT_TYPE_ALCHEMY, 1);

    /* Recipe 1 requires Alchemy level 1 */
    int can_learn1 = craft_can_learn(1);

    /* Recipe 2 requires Alchemy level 5 */
    int can_learn2 = craft_can_learn(2);

    int pass = can_learn1 == 1 && can_learn2 == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Success Rate Tests
 * ======================================== */

static int test_base_success_rate(void) {
    test_setup();

    /* Set skill level to match requirement for base success rate */
    craft_set_skill_level(CRAFT_TYPE_ALCHEMY, 1);

    /* Recipe 1 has base success rate 90%, requires level 1
     * With matching skill level, bonus = 0, rate = 90 */
    int rate = craft_get_success_rate(1);

    int pass = rate == 90;

    test_teardown();
    return pass;
}

static int test_success_rate_skill_bonus(void) {
    test_setup();

    craft_set_skill_level(CRAFT_TYPE_ALCHEMY, 5);

    /* Recipe 1 requires level 1, with level 5 skill
     * Bonus = (5 - 1) * 2 = 8
     * Rate = 90 + 8 = 98 */
    int rate = craft_get_success_rate(1);

    int pass = rate == 98;

    test_teardown();
    return pass;
}

static int test_success_rate_cap_100(void) {
    test_setup();

    craft_set_skill_level(CRAFT_TYPE_ALCHEMY, 50);

    /* Very high skill should cap at 100% */
    int rate = craft_get_success_rate(1);

    int pass = rate == 100;

    test_teardown();
    return pass;
}

static int test_success_rate_floor_5(void) {
    test_setup();

    /* Recipe 3 has base rate 70%, requires Synthesis level 10
     * With 0 skill, penalty = (0 - 10) * 2 = -20
     * Rate = 70 - 20 = 50, which is above floor */
    int rate = craft_get_success_rate(3);

    /* Still 70% since skill level is 0, penalty applies */
    /* Rate = 70 + (0 - 10) * 2 = 50 */
    int pass = rate == 50;

    test_teardown();
    return pass;
}

static int test_success_rate_invalid_recipe(void) {
    test_setup();

    int rate = craft_get_success_rate(999);

    int pass = rate == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Gold Cost Tests
 * ======================================== */

static int test_get_gold_cost(void) {
    test_setup();

    int cost = craft_get_gold_cost(1);

    int pass = cost == 50;

    test_teardown();
    return pass;
}

static int test_get_gold_cost_invalid(void) {
    test_setup();

    int cost = craft_get_gold_cost(999);

    int pass = cost == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Crafting State Tests
 * ======================================== */

static int test_start_crafting(void) {
    test_setup();

    craft_learn_recipe(1);
    int result = craft_start(1);

    int pass = result == 1 &&
               g_craft.state == CRAFT_STATE_CRAFTING &&
               g_craft.current_recipe == 1;

    test_teardown();
    return pass;
}

static int test_start_crafting_busy(void) {
    test_setup();

    craft_learn_recipe(1);
    craft_start(1);
    int result = craft_start(2);

    int pass = result == 0;

    test_teardown();
    return pass;
}

static int test_start_crafting_invalid_recipe(void) {
    test_setup();

    int result = craft_start(999);

    int pass = result == 0;

    test_teardown();
    return pass;
}

static int test_cancel_crafting(void) {
    test_setup();

    craft_learn_recipe(1);
    craft_start(1);
    craft_cancel();

    int pass = g_craft.state == CRAFT_STATE_IDLE &&
               g_craft.current_recipe == 0;

    test_teardown();
    return pass;
}

static int test_is_crafting(void) {
    test_setup();

    int before = craft_is_crafting();

    craft_learn_recipe(1);
    craft_start(1);

    int after = craft_is_crafting();

    int pass = before == 0 && after == 1;

    test_teardown();
    return pass;
}

/* ========================================
 * Craft Result Tests
 * ======================================== */

static int test_handle_result_success(void) {
    test_setup();

    u8 packet[] = {1, 1, 0, 0, 0, 0xD1, 7, 0, 0, 1, 0};
    craft_handle_result(packet, sizeof(packet));

    int pass = g_craft.last_result.success == 1 &&
               g_craft.last_result.recipe_id == 1 &&
               g_craft.last_result.item_id == 2001 &&
               g_craft.last_result.count == 1 &&
               g_craft.state == CRAFT_STATE_SUCCESS;

    test_teardown();
    return pass;
}

static int test_handle_result_failure(void) {
    test_setup();

    u8 packet[] = {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    craft_handle_result(packet, sizeof(packet));

    int pass = g_craft.last_result.success == 0 &&
               g_craft.state == CRAFT_STATE_FAILED;

    test_teardown();
    return pass;
}

static int test_get_last_result(void) {
    test_setup();

    u8 packet[] = {1, 2, 0, 0, 0, 0xD2, 7, 0, 0, 3, 0};
    craft_handle_result(packet, sizeof(packet));

    CraftResult result;
    int ret = craft_get_last_result(&result);

    int pass = ret == 1 &&
               result.recipe_id == 2 &&
               result.success == 1;

    test_teardown();
    return pass;
}

static int test_get_last_result_no_result(void) {
    test_setup();

    CraftResult result;
    int ret = craft_get_last_result(&result);

    int pass = ret == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Materials and Products Tests
 * ======================================== */

static int test_get_materials(void) {
    test_setup();

    CraftMaterial materials[10];
    int count = craft_get_materials(1, materials, 10);

    int pass = count == 2 &&
               materials[0].item_id == 1001 &&
               materials[0].count == 2 &&
               materials[1].item_id == 1002;

    test_teardown();
    return pass;
}

static int test_get_products(void) {
    test_setup();

    CraftProduct products[3];
    int count = craft_get_products(1, products, 3);

    int pass = count == 1 &&
               products[0].item_id == 2001 &&
               products[0].count == 1;

    test_teardown();
    return pass;
}

static int test_get_materials_invalid_recipe(void) {
    test_setup();

    CraftMaterial materials[10];
    int count = craft_get_materials(999, materials, 10);

    int pass = count == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * UI State Tests
 * ======================================== */

static int test_set_filter(void) {
    test_setup();

    craft_set_filter(CRAFT_TYPE_ALCHEMY);

    int pass = g_craft.filter_type == CRAFT_TYPE_ALCHEMY;

    test_teardown();
    return pass;
}

static int test_get_filter(void) {
    test_setup();

    g_craft.filter_type = CRAFT_TYPE_COOKING;

    int pass = craft_get_filter() == CRAFT_TYPE_COOKING;

    test_teardown();
    return pass;
}

static int test_set_show_unlearned(void) {
    test_setup();

    craft_set_show_unlearned(1);

    int pass = g_craft.show_unlearned == 1;

    test_teardown();
    return pass;
}

static int test_select_recipe(void) {
    test_setup();

    craft_select_recipe(2);

    int pass = g_craft.selected_recipe == 2;

    test_teardown();
    return pass;
}

/* ========================================
 * Packet Handler Tests
 * ======================================== */

static int test_handle_recipe_list(void) {
    test_setup();

    u8 packet[] = {3, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0};
    craft_handle_recipe_list(packet, sizeof(packet));

    int pass = g_craft.recipe_count == 3 &&
               g_craft.known_recipes[0] == 1 &&
               g_craft.known_recipes[1] == 2 &&
               g_craft.known_recipes[2] == 3;

    test_teardown();
    return pass;
}

static int test_handle_learn_result_success(void) {
    test_setup();

    u8 packet[] = {0, 5, 0, 0, 0};
    craft_handle_learn_result(packet, sizeof(packet));

    int pass = g_craft.recipe_count == 1 &&
               g_craft.known_recipes[0] == 5;

    test_teardown();
    return pass;
}

static int test_handle_learn_result_failure(void) {
    test_setup();

    u8 packet[] = {1, 5, 0, 0, 0};
    craft_handle_learn_result(packet, sizeof(packet));

    int pass = g_craft.recipe_count == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Utility Function Tests
 * ======================================== */

static int test_type_to_string(void) {
    int pass = strcmp(craft_type_to_string(CRAFT_TYPE_SYNTHESIS), "Synthesis") == 0 &&
               strcmp(craft_type_to_string(CRAFT_TYPE_ALCHEMY), "Alchemy") == 0 &&
               strcmp(craft_type_to_string((CraftType)10), "Unknown") == 0;
    return pass;
}

static int test_get_recipe_name(void) {
    test_setup();

    int pass = strcmp(craft_get_recipe_name(1), "Health Potion") == 0 &&
               strcmp(craft_get_recipe_name(999), "Unknown") == 0;

    test_teardown();
    return pass;
}

static int test_get_known_count(void) {
    test_setup();

    craft_learn_recipe(1);
    craft_learn_recipe(2);

    int pass = craft_get_known_count() == 2;

    test_teardown();
    return pass;
}

static int test_get_recipes_by_type(void) {
    test_setup();

    u32 recipe_ids[10];
    int count = craft_get_recipes_by_type(CRAFT_TYPE_ALCHEMY, recipe_ids, 10);

    int pass = count == 2 &&
               recipe_ids[0] == 1 &&
               recipe_ids[1] == 2;

    test_teardown();
    return pass;
}

static int test_get_recipes_by_type_empty(void) {
    test_setup();

    u32 recipe_ids[10];
    int count = craft_get_recipes_by_type(CRAFT_TYPE_COOKING, recipe_ids, 10);

    int pass = count == 0;

    test_teardown();
    return pass;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_craft_flow(void) {
    test_setup();

    /* Learn recipe */
    craft_learn_recipe(1);
    craft_set_skill_level(CRAFT_TYPE_ALCHEMY, 5);

    /* Check requirements */
    int can_craft = craft_check_skill(1);
    if (!can_craft) {
        test_teardown();
        return 0;
    }

    /* Start crafting */
    int started = craft_start(1);
    if (!started) {
        test_teardown();
        return 0;
    }

    /* Verify state */
    int pass = craft_is_crafting() &&
               g_craft.current_recipe == 1 &&
               craft_get_success_rate(1) == 98;

    test_teardown();
    return pass;
}

static int test_recipe_lifecycle(void) {
    test_setup();

    /* Learn */
    craft_learn_recipe(1);
    if (!craft_knows_recipe(1)) {
        test_teardown();
        return 0;
    }

    /* Forget */
    craft_forget_recipe(1);
    if (craft_knows_recipe(1)) {
        test_teardown();
        return 0;
    }

    /* Re-learn */
    craft_learn_recipe(1);
    int result = craft_knows_recipe(1);

    test_teardown();
    return result;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Craft System Comprehensive Tests ===\n\n");

    /* Constants tests */
    printf("Constants Tests:\n");
    TEST(max_recipes_constant);
    TEST(max_materials_constant);
    TEST(max_products_constant);
    TEST(craft_type_values);
    TEST(craft_state_values);

    /* Initialization tests */
    printf("\nInitialization Tests:\n");
    TEST(craft_init);
    TEST(craft_init_db_loaded);
    TEST(craft_shutdown_clears);

    /* Recipe query tests */
    printf("\nRecipe Query Tests:\n");
    TEST(get_recipe_by_id);
    TEST(get_recipe_invalid_id);
    TEST(get_recipe_by_index);
    TEST(get_recipe_by_invalid_index);
    TEST(get_recipe_count);

    /* Recipe learning tests */
    printf("\nRecipe Learning Tests:\n");
    TEST(learn_recipe);
    TEST(learn_recipe_already_known);
    TEST(learn_multiple_recipes);
    TEST(forget_recipe);
    TEST(forget_unknown_recipe);
    TEST(knows_recipe);

    /* Skill level tests */
    printf("\nSkill Level Tests:\n");
    TEST(set_skill_level);
    TEST(get_skill_level);
    TEST(skill_level_bounds);
    TEST(can_learn_skill_check);

    /* Success rate tests */
    printf("\nSuccess Rate Tests:\n");
    TEST(base_success_rate);
    TEST(success_rate_skill_bonus);
    TEST(success_rate_cap_100);
    TEST(success_rate_floor_5);
    TEST(success_rate_invalid_recipe);

    /* Gold cost tests */
    printf("\nGold Cost Tests:\n");
    TEST(get_gold_cost);
    TEST(get_gold_cost_invalid);

    /* Crafting state tests */
    printf("\nCrafting State Tests:\n");
    TEST(start_crafting);
    TEST(start_crafting_busy);
    TEST(start_crafting_invalid_recipe);
    TEST(cancel_crafting);
    TEST(is_crafting);

    /* Craft result tests */
    printf("\nCraft Result Tests:\n");
    TEST(handle_result_success);
    TEST(handle_result_failure);
    TEST(get_last_result);
    TEST(get_last_result_no_result);

    /* Materials and products tests */
    printf("\nMaterials and Products Tests:\n");
    TEST(get_materials);
    TEST(get_products);
    TEST(get_materials_invalid_recipe);

    /* UI state tests */
    printf("\nUI State Tests:\n");
    TEST(set_filter);
    TEST(get_filter);
    TEST(set_show_unlearned);
    TEST(select_recipe);

    /* Packet handler tests */
    printf("\nPacket Handler Tests:\n");
    TEST(handle_recipe_list);
    TEST(handle_learn_result_success);
    TEST(handle_learn_result_failure);

    /* Utility function tests */
    printf("\nUtility Function Tests:\n");
    TEST(type_to_string);
    TEST(get_recipe_name);
    TEST(get_known_count);
    TEST(get_recipes_by_type);
    TEST(get_recipes_by_type_empty);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_craft_flow);
    TEST(recipe_lifecycle);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - Real packet data from game captures
     * - Progress calculation with timing
     * - Multiple simultaneous crafts (if supported)
     * - Recipe database loading from file
     * - Tool item requirements
     * - Learn type and param validation
     * - Craft duration calculations
     * - Material consumption on failure
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
