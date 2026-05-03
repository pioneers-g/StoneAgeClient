/*
 * Stone Age Client - Crafting/Synthesis System Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef CRAFT_H
#define CRAFT_H

#include "types.h"

/* Constants */
#define MAX_CRAFT_RECIPES   500
#define MAX_CRAFT_MATERIALS 10
#define MAX_CRAFT_PRODUCTS  3

/* Craft types */
typedef enum {
    CRAFT_TYPE_SYNTHESIS = 0,   /* Combine items */
    CRAFT_TYPE_UPGRADE = 1,     /* Enhance equipment */
    CRAFT_TYPE_REFINE = 2,      /* Refine materials */
    CRAFT_TYPE_COOKING = 3,     /* Create food */
    CRAFT_TYPE_ALCHEMY = 4      /* Create potions */
} CraftType;

/* Material requirement */
typedef struct {
    u32 item_id;
    u16 count;
    u8  consume;      /* Whether consumed on failure */
    u8  reserved;
} CraftMaterial;

/* Product result */
typedef struct {
    u32 item_id;
    u16 count;
    u16 rate;         /* Success rate (0-10000) */
} CraftProduct;

/* Recipe data */
typedef struct {
    u32 id;
    char name[32];
    char description[128];
    CraftType type;
    u16 skill_required;
    u16 skill_level;
    u16 success_rate;    /* Base success rate (0-100) */
    u16 gold_cost;
    u32 duration;        /* Craft time in ms */

    /* Materials */
    CraftMaterial materials[MAX_CRAFT_MATERIALS];
    int material_count;

    /* Products */
    CraftProduct products[MAX_CRAFT_PRODUCTS];
    int product_count;

    /* Requirements */
    u16 tool_item;       /* Required tool (e.g., furnace) */
    u8  learn_type;      /* How recipe is learned */
    u8  learn_param;     /* Quest ID or NPC ID */

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
    /* Known recipes */
    u32 known_recipes[MAX_CRAFT_RECIPES];
    int recipe_count;

    /* Recipe database */
    CraftRecipe* recipe_db;
    int db_count;

    /* Current crafting */
    u32 current_recipe;
    u32 craft_start_time;
    u32 craft_duration;
    CraftState state;

    /* Result */
    CraftResult last_result;

    /* UI state */
    CraftType filter_type;
    u8 show_unlearned;
    int selected_recipe;

    /* Skills */
    u16 craft_skills[5];  /* Synthesis, Upgrade, Refine, Cooking, Alchemy */

} CraftContext;

/* Global craft context */
extern CraftContext g_craft;

/* Initialization */
int craft_init(void);
void craft_shutdown(void);

/* Recipe database */
int craft_load_recipes(void);
CraftRecipe* craft_get_recipe(u32 recipe_id);
CraftRecipe* craft_get_recipe_by_index(int index);
int craft_get_recipe_count(void);

/* Learning */
int craft_learn_recipe(u32 recipe_id);
int craft_forget_recipe(u32 recipe_id);
int craft_knows_recipe(u32 recipe_id);
int craft_can_learn(u32 recipe_id);

/* Crafting */
int craft_start(u32 recipe_id);
void craft_update(void);
int craft_is_crafting(void);
int craft_get_progress(void);
void craft_cancel(void);

/* Result */
void craft_handle_result(void* data, u32 size);
int craft_get_last_result(CraftResult* result);

/* Check requirements */
int craft_check_materials(u32 recipe_id);
int craft_check_skill(u32 recipe_id);
int craft_check_gold(u32 recipe_id);
int craft_can_craft(u32 recipe_id);

/* Get materials */
int craft_get_materials(u32 recipe_id, CraftMaterial* materials, int max_count);
int craft_get_products(u32 recipe_id, CraftProduct* products, int max_count);

/* Query */
int craft_get_recipes_by_type(CraftType type, u32* recipe_ids, int max_count);
int craft_get_known_count(void);
int craft_get_success_rate(u32 recipe_id);
int craft_get_gold_cost(u32 recipe_id);

/* Skills */
void craft_set_skill_level(CraftType type, u16 level);
u16 craft_get_skill_level(CraftType type);

/* UI */
void craft_set_filter(CraftType type);
CraftType craft_get_filter(void);
void craft_set_show_unlearned(u8 show);
void craft_select_recipe(int index);

/* Packet handlers */
void craft_handle_recipe_list(void* data, u32 size);
void craft_handle_learn_result(void* data, u32 size);

/* Utility */
const char* craft_type_to_string(CraftType type);
const char* craft_get_recipe_name(u32 recipe_id);

#endif /* CRAFT_H */
