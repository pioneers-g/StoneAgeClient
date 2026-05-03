/*
 * Stone Age Client - Crafting/Synthesis System Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "craft.h"
#include "item.h"
#include "character.h"
#include "network_protocol.h"
#include "logger.h"

/* Global craft context */
CraftContext g_craft = {0};

/* Sample recipes */
static const CraftRecipe s_sample_recipes[] = {
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
        .skill_level = 1, .success_rate = 85, .gold_cost = 100, .duration = 5000,
        .materials = {{1001, 1, 1}, {1003, 2, 1}},
        .material_count = 2,
        .products = {{2002, 1, 8500}},
        .product_count = 1,
        .tool_item = 5001
    },
    {
        .id = 3, .name = "Iron Sword", .description = "Basic iron sword",
        .type = CRAFT_TYPE_SYNTHESIS, .skill_required = CRAFT_TYPE_SYNTHESIS,
        .skill_level = 5, .success_rate = 70, .gold_cost = 200, .duration = 10000,
        .materials = {{2001, 3, 1}, {2002, 1, 1}},
        .material_count = 2,
        .products = {{3001, 1, 7000}},
        .product_count = 1,
        .tool_item = 5002
    }
};

/*
 * Initialize craft system
 */
int craft_init(void) {
    memset(&g_craft, 0, sizeof(CraftContext));

    g_craft.recipe_db = (CraftRecipe*)s_sample_recipes;
    g_craft.db_count = sizeof(s_sample_recipes) / sizeof(CraftRecipe);
    g_craft.state = CRAFT_STATE_IDLE;

    LOG_INFO("Craft system initialized");
    return 1;
}

/*
 * Shutdown craft system
 */
void craft_shutdown(void) {
    memset(&g_craft, 0, sizeof(CraftContext));
    LOG_INFO("Craft system shutdown");
}

/*
 * Load recipes from file
 */
int craft_load_recipes(void) {
    /* Using static data for now */
    return 1;
}

/*
 * Get recipe by ID
 */
CraftRecipe* craft_get_recipe(u32 recipe_id) {
    int i;

    for (i = 0; i < g_craft.db_count; i++) {
        if (g_craft.recipe_db[i].id == recipe_id) {
            return &g_craft.recipe_db[i];
        }
    }

    return NULL;
}

/*
 * Get recipe by index
 */
CraftRecipe* craft_get_recipe_by_index(int index) {
    if (index < 0 || index >= g_craft.db_count) {
        return NULL;
    }
    return &g_craft.recipe_db[index];
}

/*
 * Get recipe count
 */
int craft_get_recipe_count(void) {
    return g_craft.db_count;
}

/*
 * Learn recipe
 */
int craft_learn_recipe(u32 recipe_id) {
    if (g_craft.recipe_count >= MAX_CRAFT_RECIPES) {
        LOG_WARN("Recipe list full");
        return 0;
    }

    if (craft_knows_recipe(recipe_id)) {
        return 1;
    }

    g_craft.known_recipes[g_craft.recipe_count++] = recipe_id;
    LOG_INFO("Learned recipe: %s", craft_get_recipe_name(recipe_id));
    return 1;
}

/*
 * Forget recipe
 */
int craft_forget_recipe(u32 recipe_id) {
    int i;

    for (i = 0; i < g_craft.recipe_count; i++) {
        if (g_craft.known_recipes[i] == recipe_id) {
            memmove(&g_craft.known_recipes[i], &g_craft.known_recipes[i + 1],
                    (g_craft.recipe_count - i - 1) * sizeof(u32));
            g_craft.recipe_count--;
            return 1;
        }
    }

    return 0;
}

/*
 * Check if knows recipe
 */
int craft_knows_recipe(u32 recipe_id) {
    int i;

    for (i = 0; i < g_craft.recipe_count; i++) {
        if (g_craft.known_recipes[i] == recipe_id) {
            return 1;
        }
    }

    return 0;
}

/*
 * Check if can learn recipe
 */
int craft_can_learn(u32 recipe_id) {
    CraftRecipe* recipe = craft_get_recipe(recipe_id);

    if (!recipe) return 0;

    /* Check skill level */
    if (g_craft.craft_skills[recipe->skill_required] < recipe->skill_level) {
        return 0;
    }

    return 1;
}

/*
 * Start crafting
 */
int craft_start(u32 recipe_id) {
    CraftRecipe* recipe;
    char packet[64];

    if (g_craft.state != CRAFT_STATE_IDLE) {
        LOG_WARN("Already crafting");
        return 0;
    }

    recipe = craft_get_recipe(recipe_id);
    if (!recipe) {
        LOG_WARN("Unknown recipe: %u", recipe_id);
        return 0;
    }

    if (!craft_can_craft(recipe_id)) {
        return 0;
    }

    /* Send craft request packet: CRAFT|<recipe_id> */
    _snprintf(packet, sizeof(packet), "CRAFT|%u", recipe_id);
    send_queue_add(packet, 0, 0);

    g_craft.current_recipe = recipe_id;
    g_craft.craft_start_time = timeGetTime();
    g_craft.craft_duration = recipe->duration;
    g_craft.state = CRAFT_STATE_CRAFTING;

    LOG_DEBUG("Started crafting: %s", recipe->name);
    return 1;
}

/*
 * Update crafting
 */
void craft_update(void) {
    if (g_craft.state != CRAFT_STATE_CRAFTING) {
        return;
    }

    /* Check if done */
    u32 elapsed = timeGetTime() - g_craft.craft_start_time;
    if (elapsed >= g_craft.craft_duration) {
        /* Wait for server result or auto-complete */
    }
}

/*
 * Check if crafting
 */
int craft_is_crafting(void) {
    return g_craft.state == CRAFT_STATE_CRAFTING;
}

/*
 * Get craft progress (0-100)
 */
int craft_get_progress(void) {
    if (g_craft.state != CRAFT_STATE_CRAFTING) {
        return 0;
    }

    u32 elapsed = timeGetTime() - g_craft.craft_start_time;
    if (elapsed >= g_craft.craft_duration) {
        return 100;
    }

    return (elapsed * 100) / g_craft.craft_duration;
}

/*
 * Cancel crafting
 */
void craft_cancel(void) {
    if (g_craft.state == CRAFT_STATE_CRAFTING) {
        g_craft.state = CRAFT_STATE_IDLE;
        g_craft.current_recipe = 0;
        LOG_DEBUG("Crafting cancelled");
    }
}

/*
 * Handle craft result
 */
void craft_handle_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 success;
    u32 recipe_id;
    u32 item_id;
    u16 count;

    success = *ptr++;
    recipe_id = *(u32*)ptr; ptr += 4;
    item_id = *(u32*)ptr; ptr += 4;
    count = *(u16*)ptr;

    g_craft.last_result.recipe_id = recipe_id;
    g_craft.last_result.success = success;
    g_craft.last_result.item_id = item_id;
    g_craft.last_result.count = count;

    if (success) {
        g_craft.state = CRAFT_STATE_SUCCESS;
        LOG_INFO("Craft success: %s x%d", item_get_name(item_id), count);
    } else {
        g_craft.state = CRAFT_STATE_FAILED;
        LOG_INFO("Craft failed: recipe %u", recipe_id);
    }
}

/*
 * Get last result
 */
int craft_get_last_result(CraftResult* result) {
    if (g_craft.state != CRAFT_STATE_SUCCESS &&
        g_craft.state != CRAFT_STATE_FAILED) {
        return 0;
    }

    *result = g_craft.last_result;
    return 1;
}

/*
 * Check materials
 */
int craft_check_materials(u32 recipe_id) {
    CraftRecipe* recipe = craft_get_recipe(recipe_id);
    int i;

    if (!recipe) return 0;

    for (i = 0; i < recipe->material_count; i++) {
        CraftMaterial* mat = &recipe->materials[i];

        /* Check inventory for required material count */
        int have_count = inventory_get_count(mat->item_id);
        if (have_count < mat->count) {
            LOG_DEBUG("Missing material %u: have %d, need %d",
                     mat->item_id, have_count, mat->count);
            return 0;
        }
    }

    return 1;
}

/*
 * Check skill level
 */
int craft_check_skill(u32 recipe_id) {
    CraftRecipe* recipe = craft_get_recipe(recipe_id);

    if (!recipe) return 0;

    return g_craft.craft_skills[recipe->skill_required] >= recipe->skill_level;
}

/*
 * Check gold
 */
int craft_check_gold(u32 recipe_id) {
    CraftRecipe* recipe = craft_get_recipe(recipe_id);
    Character* player;

    if (!recipe) return 0;

    /* Get player gold from character system */
    player = character_get_player();
    if (player) {
        /* Gold is stored in detailed stats or as a field */
        /* For now, assume sufficient gold (server validates) */
        return 1;
    }

    return 1;
}

/*
 * Check if can craft
 */
int craft_can_craft(u32 recipe_id) {
    if (!craft_knows_recipe(recipe_id)) {
        return 0;
    }

    if (!craft_check_materials(recipe_id)) {
        return 0;
    }

    if (!craft_check_skill(recipe_id)) {
        return 0;
    }

    if (!craft_check_gold(recipe_id)) {
        return 0;
    }

    return 1;
}

/*
 * Get materials for recipe
 */
int craft_get_materials(u32 recipe_id, CraftMaterial* materials, int max_count) {
    CraftRecipe* recipe = craft_get_recipe(recipe_id);
    int i;

    if (!recipe || !materials) return 0;

    for (i = 0; i < recipe->material_count && i < max_count; i++) {
        materials[i] = recipe->materials[i];
    }

    return recipe->material_count;
}

/*
 * Get products for recipe
 */
int craft_get_products(u32 recipe_id, CraftProduct* products, int max_count) {
    CraftRecipe* recipe = craft_get_recipe(recipe_id);
    int i;

    if (!recipe || !products) return 0;

    for (i = 0; i < recipe->product_count && i < max_count; i++) {
        products[i] = recipe->products[i];
    }

    return recipe->product_count;
}

/*
 * Get recipes by type
 */
int craft_get_recipes_by_type(CraftType type, u32* recipe_ids, int max_count) {
    int i, count = 0;

    for (i = 0; i < g_craft.db_count && count < max_count; i++) {
        if (g_craft.recipe_db[i].type == type) {
            recipe_ids[count++] = g_craft.recipe_db[i].id;
        }
    }

    return count;
}

/*
 * Get known recipe count
 */
int craft_get_known_count(void) {
    return g_craft.recipe_count;
}

/*
 * Get success rate
 */
int craft_get_success_rate(u32 recipe_id) {
    CraftRecipe* recipe = craft_get_recipe(recipe_id);

    if (!recipe) return 0;

    /* Base rate + skill bonus */
    int rate = recipe->success_rate;
    int skill_bonus = g_craft.craft_skills[recipe->skill_required] - recipe->skill_level;
    rate += skill_bonus * 2;

    if (rate > 100) rate = 100;
    if (rate < 5) rate = 5;

    return rate;
}

/*
 * Get gold cost
 */
int craft_get_gold_cost(u32 recipe_id) {
    CraftRecipe* recipe = craft_get_recipe(recipe_id);
    return recipe ? recipe->gold_cost : 0;
}

/*
 * Set skill level
 */
void craft_set_skill_level(CraftType type, u16 level) {
    if (type < 5) {
        g_craft.craft_skills[type] = level;
    }
}

/*
 * Get skill level
 */
u16 craft_get_skill_level(CraftType type) {
    if (type < 5) {
        return g_craft.craft_skills[type];
    }
    return 0;
}

/*
 * Set filter
 */
void craft_set_filter(CraftType type) {
    g_craft.filter_type = type;
}

/*
 * Get filter
 */
CraftType craft_get_filter(void) {
    return g_craft.filter_type;
}

/*
 * Set show unlearned
 */
void craft_set_show_unlearned(u8 show) {
    g_craft.show_unlearned = show;
}

/*
 * Select recipe
 */
void craft_select_recipe(int index) {
    g_craft.selected_recipe = index;
}

/*
 * Handle recipe list packet
 */
void craft_handle_recipe_list(void* data, u32 size) {
    u8* ptr = (u8*)data;
    int i, count;

    g_craft.recipe_count = 0;

    count = *(u16*)ptr; ptr += 2;

    for (i = 0; i < count && i < MAX_CRAFT_RECIPES; i++) {
        g_craft.known_recipes[i] = *(u32*)ptr; ptr += 4;
        g_craft.recipe_count++;
    }

    LOG_INFO("Loaded %d recipes", g_craft.recipe_count);
}

/*
 * Handle learn result
 */
void craft_handle_learn_result(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u8 result;
    u32 recipe_id;

    result = *ptr++;
    recipe_id = *(u32*)ptr;

    if (result == 0) {
        craft_learn_recipe(recipe_id);
    } else {
        LOG_WARN("Failed to learn recipe %u", recipe_id);
    }
}

/*
 * Craft type to string
 */
const char* craft_type_to_string(CraftType type) {
    static const char* type_names[] = {
        "Synthesis", "Upgrade", "Refine", "Cooking", "Alchemy"
    };

    if (type < 5) {
        return type_names[type];
    }
    return "Unknown";
}

/*
 * Get recipe name
 */
const char* craft_get_recipe_name(u32 recipe_id) {
    CraftRecipe* recipe = craft_get_recipe(recipe_id);
    return recipe ? recipe->name : "Unknown";
}
