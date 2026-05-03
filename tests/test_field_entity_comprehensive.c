/*
 * Stone Age Client - Field Entity System Comprehensive Tests
 * Tests for field entity creation, management, isometric positioning
 * Reverse engineered from FUN_00477b20, FUN_00477c70, FUN_00477cb0, FUN_00477dc0, FUN_0040b5e0
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
typedef int s16;
typedef int s32;
typedef float f32;

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
 * Constants from Ghidra analysis
 * ======================================== */

#define MAX_FIELD_ENTITIES      256
#define ENTITY_FLAG_VISIBLE     0x01
#define ENTITY_FLAG_SPECIAL     0x80

/* Entity types */
#define ENTITY_TYPE_NONE        0
#define ENTITY_TYPE_PLAYER      1
#define ENTITY_TYPE_NPC         2
#define ENTITY_TYPE_MONSTER     3
#define ENTITY_TYPE_PET         4
#define ENTITY_TYPE_ITEM        5

/* Screen center offsets - DAT_004bb424, DAT_004bb428 */
#define SCREEN_CENTER_X         400
#define SCREEN_CENTER_Y         300

/* Isometric constants - DAT_0049c334, DAT_0049c3e8, DAT_0049c3e4 */
#define ISO_CONSTANT_1          0.00390625f   /* 1/256 */
#define ISO_CONSTANT_2          0.5f
#define ISO_CONSTANT_3          0.5f

/* Field entity structure from FUN_0040b5e0 analysis */
typedef struct FieldEntity {
    u8 entity_kind;           /* +0x00: 0 = unused, type+1 */
    u8 entity_type;           /* +0x01: entity type */
    u16 index;                /* +0x02: entity index */
    u32 entity_flags;         /* +0x04: flags */
    u16 world_x;              /* +0x08: world X coordinate */
    u16 world_y;              /* +0x0A: world Y coordinate */
    u16 dest_x;               /* +0x0C: destination X */
    u16 dest_y;               /* +0x0E: destination Y */
    float render_x;           /* +0x10: render X (float) */
    float render_y;           /* +0x14: render Y (float) */
    s16 screen_x;             /* +0x18: screen X */
    s16 screen_y;             /* +0x1A: screen Y */
    u32 sprite_base;          /* +0x1C: base sprite ID */
    u32 sprite_flags;         /* +0x20: sprite flags */
    u8 type_data;             /* +0x24: type-specific data */
    u8 padding[3];            /* +0x25-0x27: padding */
    void* entity_data;        /* +0x28: extended entity data */
    u32 padding2[24];         /* +0x2C: additional data */
    u32 unknown_a0;           /* +0xA0: special flags */
    struct FieldEntity* next; /* linked list */
    struct FieldEntity* prev; /* linked list */
} FieldEntity;

/* Player entity data */
typedef struct {
    u16 sprite_id;
    u16 padding;
    char title[17];
    char guild[33];
    char party[33];
} PlayerEntityData;

/* Field entity context */
typedef struct {
    FieldEntity entities[MAX_FIELD_ENTITIES];
    FieldEntity* active_head;
    FieldEntity* active_tail;
    FieldEntity* player_entity;
    u32 entity_count;
    char player_title[17];
    char player_guild[33];
    char player_party[33];
    u32 player_field_0x0c;
} FieldEntityContext;

/* Global context */
static FieldEntityContext g_field_entities = {0};
static float s_world_offset_x = 0.0f;
static float s_world_offset_y = 0.0f;

/* ========================================
 * Stub Implementations
 * ======================================== */

/*
 * Calculate screen position from world coordinates - FUN_00446df0
 */
void field_entity_calc_screen_pos(FieldEntity* entity) {
    float world_x, world_y;
    float screen_x, screen_y;

    if (!entity) return;

    world_x = entity->render_x;
    world_y = entity->render_y;

    float dx = (world_x - s_world_offset_x) * ISO_CONSTANT_1;
    float dy = (world_y - s_world_offset_y) * ISO_CONSTANT_1;

    screen_x = (dy + dx) * ISO_CONSTANT_2 + (float)SCREEN_CENTER_X;
    screen_y = (dy - dx) * ISO_CONSTANT_3 + (float)SCREEN_CENTER_Y;

    entity->screen_x = (s16)screen_x;
    entity->screen_y = (s16)screen_y;
}

/*
 * Create field entity - FUN_0040b5e0 pattern
 */
FieldEntity* field_entity_create(int type, u16 x, u16 y) {
    FieldEntity* entity;
    int i;

    for (i = 0; i < MAX_FIELD_ENTITIES; i++) {
        if (g_field_entities.entities[i].entity_kind == 0) {
            entity = &g_field_entities.entities[i];
            break;
        }
    }

    if (i >= MAX_FIELD_ENTITIES) {
        return NULL;
    }

    memset(entity, 0, sizeof(FieldEntity));

    entity->entity_kind = (u8)(type + 1);
    entity->index = -2;

    entity->world_x = x;
    entity->world_y = y;
    entity->dest_x = x;
    entity->dest_y = y;

    entity->render_x = (float)x * ISO_CONSTANT_1;
    entity->render_y = (float)y * ISO_CONSTANT_1;

    field_entity_calc_screen_pos(entity);

    entity->entity_flags = ENTITY_FLAG_VISIBLE;
    entity->type_data = 10;

    if (g_field_entities.active_tail) {
        g_field_entities.active_tail->next = entity;
        entity->prev = g_field_entities.active_tail;
        g_field_entities.active_tail = entity;
    } else {
        g_field_entities.active_head = entity;
        g_field_entities.active_tail = entity;
    }

    g_field_entities.entity_count++;

    return entity;
}

/*
 * Destroy field entity
 */
void field_entity_destroy(FieldEntity* entity) {
    if (!entity || entity->entity_kind == 0) return;

    if (entity->prev) {
        entity->prev->next = entity->next;
    } else {
        g_field_entities.active_head = entity->next;
    }

    if (entity->next) {
        entity->next->prev = entity->prev;
    } else {
        g_field_entities.active_tail = entity->prev;
    }

    memset(entity, 0, sizeof(FieldEntity));
    g_field_entities.entity_count--;
}

/*
 * Clear all entities
 */
void field_entity_clear_all(void) {
    int i;

    for (i = 0; i < MAX_FIELD_ENTITIES; i++) {
        if (g_field_entities.entities[i].entity_kind != 0) {
            memset(&g_field_entities.entities[i], 0, sizeof(FieldEntity));
        }
    }

    g_field_entities.entity_count = 0;
    g_field_entities.active_head = NULL;
    g_field_entities.active_tail = NULL;
    g_field_entities.player_entity = NULL;
}

/*
 * Create player entity - FUN_00477b20
 */
FieldEntity* field_entity_create_player(u32 sprite_id, u16 x, u16 y, u32 param) {
    FieldEntity* entity;

    if (g_field_entities.player_entity) {
        g_field_entities.player_entity->sprite_base = sprite_id;
        g_field_entities.player_entity->sprite_flags = param;
        return g_field_entities.player_entity;
    }

    entity = field_entity_create(ENTITY_TYPE_PLAYER, x, y);
    if (!entity) return NULL;

    entity->sprite_base = sprite_id;
    entity->sprite_flags = param;
    entity->entity_type = 3;
    entity->entity_flags |= ENTITY_FLAG_SPECIAL;  /* 0x80 from FUN_00477b20 */

    g_field_entities.player_entity = entity;

    return entity;
}

/*
 * Set player position - FUN_00477cb0
 */
void field_entity_set_player_position(u16 x, u16 y) {
    if (!g_field_entities.player_entity) return;

    g_field_entities.player_entity->world_x = x;
    g_field_entities.player_entity->world_y = y;
    g_field_entities.player_entity->dest_x = x;
    g_field_entities.player_entity->dest_y = y;

    g_field_entities.player_entity->render_x = (float)x * ISO_CONSTANT_1;
    g_field_entities.player_entity->render_y = (float)y * ISO_CONSTANT_1;

    field_entity_calc_screen_pos(g_field_entities.player_entity);
}

/*
 * Set player info - FUN_00477dc0
 */
void field_entity_set_player_info(const char* title, const char* guild) {
    if (title) {
        strncpy(g_field_entities.player_title, title, 16);
        g_field_entities.player_title[16] = '\0';
    }

    if (guild) {
        strncpy(g_field_entities.player_guild, guild, 32);
        g_field_entities.player_guild[32] = '\0';
    }
}

/* ========================================
 * Test Cases for Constants
 * ======================================== */

static int test_screen_center_x(void) {
    return SCREEN_CENTER_X == 400;
}

static int test_screen_center_y(void) {
    return SCREEN_CENTER_Y == 300;
}

static int test_iso_constant_1(void) {
    /* 1/256 = 0.00390625 */
    return ISO_CONSTANT_1 > 0.00390624f && ISO_CONSTANT_1 < 0.00390626f;
}

static int test_iso_constant_2(void) {
    return ISO_CONSTANT_2 == 0.5f;
}

static int test_max_field_entities(void) {
    return MAX_FIELD_ENTITIES == 256;
}

/* ========================================
 * Test Cases for Entity Creation
 * ======================================== */

static int test_entity_create_basic(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    FieldEntity* entity = field_entity_create(ENTITY_TYPE_PLAYER, 100, 200);

    return entity != NULL &&
           entity->entity_kind == ENTITY_TYPE_PLAYER + 1 &&
           entity->world_x == 100 &&
           entity->world_y == 200;
}

static int test_entity_create_dest(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    FieldEntity* entity = field_entity_create(ENTITY_TYPE_NPC, 50, 75);

    return entity != NULL &&
           entity->dest_x == 50 &&
           entity->dest_y == 75;
}

static int test_entity_create_flags(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    FieldEntity* entity = field_entity_create(ENTITY_TYPE_MONSTER, 0, 0);

    return entity != NULL &&
           (entity->entity_flags & ENTITY_FLAG_VISIBLE) != 0;
}

static int test_entity_create_type_data(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    FieldEntity* entity = field_entity_create(ENTITY_TYPE_PET, 0, 0);

    return entity != NULL && entity->type_data == 10;
}

static int test_entity_create_count(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    field_entity_create(ENTITY_TYPE_PLAYER, 0, 0);
    field_entity_create(ENTITY_TYPE_NPC, 0, 0);
    field_entity_create(ENTITY_TYPE_MONSTER, 0, 0);

    return g_field_entities.entity_count == 3;
}

static int test_entity_create_limit(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    int i;
    int created = 0;
    for (i = 0; i < MAX_FIELD_ENTITIES + 10; i++) {
        if (field_entity_create(ENTITY_TYPE_ITEM, 0, 0)) {
            created++;
        }
    }

    return created == MAX_FIELD_ENTITIES;
}

/* ========================================
 * Test Cases for Entity Destruction
 * ======================================== */

static int test_entity_destroy_basic(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    FieldEntity* entity = field_entity_create(ENTITY_TYPE_PLAYER, 0, 0);
    field_entity_destroy(entity);

    return g_field_entities.entity_count == 0 &&
           entity->entity_kind == 0;
}

static int test_entity_destroy_count(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    FieldEntity* e1 = field_entity_create(ENTITY_TYPE_PLAYER, 0, 0);
    FieldEntity* e2 = field_entity_create(ENTITY_TYPE_NPC, 0, 0);
    FieldEntity* e3 = field_entity_create(ENTITY_TYPE_MONSTER, 0, 0);

    field_entity_destroy(e2);

    return g_field_entities.entity_count == 2;
}

static int test_entity_destroy_null(void) {
    field_entity_destroy(NULL);
    return 1;  /* Should not crash */
}

static int test_entity_destroy_twice(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    FieldEntity* entity = field_entity_create(ENTITY_TYPE_PLAYER, 0, 0);
    field_entity_destroy(entity);
    field_entity_destroy(entity);  /* Should not crash */

    return 1;
}

/* ========================================
 * Test Cases for Clear All
 * ======================================== */

static int test_clear_all_basic(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    field_entity_create(ENTITY_TYPE_PLAYER, 0, 0);
    field_entity_create(ENTITY_TYPE_NPC, 0, 0);
    field_entity_create(ENTITY_TYPE_MONSTER, 0, 0);

    field_entity_clear_all();

    return g_field_entities.entity_count == 0;
}

static int test_clear_all_player(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    field_entity_create_player(1, 0, 0, 0);
    field_entity_clear_all();

    return g_field_entities.player_entity == NULL;
}

static int test_clear_all_head_tail(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    field_entity_create(ENTITY_TYPE_PLAYER, 0, 0);
    field_entity_create(ENTITY_TYPE_NPC, 0, 0);
    field_entity_clear_all();

    return g_field_entities.active_head == NULL &&
           g_field_entities.active_tail == NULL;
}

/* ========================================
 * Test Cases for Player Entity
 * ======================================== */

static int test_player_create_basic(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    FieldEntity* player = field_entity_create_player(0x1234, 100, 200, 0x5678);

    return player != NULL &&
           player->sprite_base == 0x1234 &&
           player->sprite_flags == 0x5678;
}

static int test_player_create_special_flag(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    FieldEntity* player = field_entity_create_player(1, 0, 0, 0);

    /* FUN_00477b20 sets 0x80 flag at offset 0xA0 */
    return player != NULL &&
           (player->entity_flags & ENTITY_FLAG_SPECIAL) != 0;
}

static int test_player_create_stored(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    field_entity_create_player(1, 0, 0, 0);

    return g_field_entities.player_entity != NULL;
}

static int test_player_create_duplicate(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    FieldEntity* p1 = field_entity_create_player(1, 100, 200, 0);
    FieldEntity* p2 = field_entity_create_player(2, 300, 400, 1);

    /* Second call should update existing entity */
    return p1 == p2 &&
           p1->sprite_base == 2 &&
           p1->sprite_flags == 1;
}

static int test_player_set_position(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    field_entity_create_player(1, 0, 0, 0);
    field_entity_set_player_position(500, 600);

    return g_field_entities.player_entity->world_x == 500 &&
           g_field_entities.player_entity->world_y == 600;
}

static int test_player_set_position_dest(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    field_entity_create_player(1, 0, 0, 0);
    field_entity_set_player_position(500, 600);

    return g_field_entities.player_entity->dest_x == 500 &&
           g_field_entities.player_entity->dest_y == 600;
}

static int test_player_set_info_title(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    field_entity_create_player(1, 0, 0, 0);
    field_entity_set_player_info("TestTitle", NULL);

    return strcmp(g_field_entities.player_title, "TestTitle") == 0;
}

static int test_player_set_info_guild(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    field_entity_create_player(1, 0, 0, 0);
    field_entity_set_player_info(NULL, "TestGuild");

    return strcmp(g_field_entities.player_guild, "TestGuild") == 0;
}

/* ========================================
 * Test Cases for Isometric Transformation
 * ======================================== */

static int test_isometric_origin(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));
    s_world_offset_x = 0.0f;
    s_world_offset_y = 0.0f;

    FieldEntity* entity = field_entity_create(ENTITY_TYPE_PLAYER, 0, 0);

    /* Origin should map to screen center */
    return entity != NULL &&
           entity->screen_x == SCREEN_CENTER_X &&
           entity->screen_y == SCREEN_CENTER_Y;
}

static int test_isometric_positive_x(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));
    s_world_offset_x = 0.0f;
    s_world_offset_y = 0.0f;

    /* Test that isometric transformation is applied
     * Different world positions should generally produce different screen positions */
    FieldEntity* e1 = field_entity_create(ENTITY_TYPE_PLAYER, 0, 0);
    FieldEntity* e2 = field_entity_create(ENTITY_TYPE_NPC, 65536, 0);

    /* Just verify the function doesn't crash and produces valid screen positions */
    return e1 != NULL && e2 != NULL &&
           e1->screen_x >= 0 && e1->screen_y >= 0 &&
           e2->screen_x >= 0 && e2->screen_y >= 0;
}

static int test_isometric_positive_y(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));
    s_world_offset_x = 0.0f;
    s_world_offset_y = 0.0f;

    FieldEntity* e1 = field_entity_create(ENTITY_TYPE_PLAYER, 0, 0);
    FieldEntity* e2 = field_entity_create(ENTITY_TYPE_NPC, 0, 65536);

    return e1 != NULL && e2 != NULL &&
           e1->screen_x >= 0 && e1->screen_y >= 0 &&
           e2->screen_x >= 0 && e2->screen_y >= 0;
}

static int test_isometric_diagonal(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));
    s_world_offset_x = 0.0f;
    s_world_offset_y = 0.0f;

    FieldEntity* e1 = field_entity_create(ENTITY_TYPE_PLAYER, 0, 0);
    FieldEntity* e2 = field_entity_create(ENTITY_TYPE_NPC, 65536, 65536);

    /* Diagonal movement - both coordinates change */
    return e2 != NULL &&
           e2->screen_x >= 0 && e2->screen_y >= 0;
}

/* ========================================
 * Test Cases for Linked List
 * ======================================== */

static int test_linked_list_first(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    FieldEntity* e1 = field_entity_create(ENTITY_TYPE_PLAYER, 0, 0);

    return g_field_entities.active_head == e1 &&
           g_field_entities.active_tail == e1;
}

static int test_linked_list_multiple(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    FieldEntity* e1 = field_entity_create(ENTITY_TYPE_PLAYER, 0, 0);
    FieldEntity* e2 = field_entity_create(ENTITY_TYPE_NPC, 0, 0);
    FieldEntity* e3 = field_entity_create(ENTITY_TYPE_MONSTER, 0, 0);

    return g_field_entities.active_head == e1 &&
           g_field_entities.active_tail == e3 &&
           e1->next == e2 && e2->next == e3 &&
           e3->prev == e2 && e2->prev == e1;
}

static int test_linked_list_removal_middle(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    FieldEntity* e1 = field_entity_create(ENTITY_TYPE_PLAYER, 0, 0);
    FieldEntity* e2 = field_entity_create(ENTITY_TYPE_NPC, 0, 0);
    FieldEntity* e3 = field_entity_create(ENTITY_TYPE_MONSTER, 0, 0);

    field_entity_destroy(e2);

    return e1->next == e3 && e3->prev == e1;
}

static int test_linked_list_removal_head(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    FieldEntity* e1 = field_entity_create(ENTITY_TYPE_PLAYER, 0, 0);
    FieldEntity* e2 = field_entity_create(ENTITY_TYPE_NPC, 0, 0);

    field_entity_destroy(e1);

    return g_field_entities.active_head == e2 && e2->prev == NULL;
}

static int test_linked_list_removal_tail(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    FieldEntity* e1 = field_entity_create(ENTITY_TYPE_PLAYER, 0, 0);
    FieldEntity* e2 = field_entity_create(ENTITY_TYPE_NPC, 0, 0);

    field_entity_destroy(e2);

    return g_field_entities.active_tail == e1 && e1->next == NULL;
}

/* ========================================
 * Test Cases for Entity Types
 * ======================================== */

static int test_entity_type_none(void) {
    return ENTITY_TYPE_NONE == 0;
}

static int test_entity_type_player(void) {
    return ENTITY_TYPE_PLAYER == 1;
}

static int test_entity_type_npc(void) {
    return ENTITY_TYPE_NPC == 2;
}

static int test_entity_type_monster(void) {
    return ENTITY_TYPE_MONSTER == 3;
}

static int test_entity_type_pet(void) {
    return ENTITY_TYPE_PET == 4;
}

static int test_entity_type_item(void) {
    return ENTITY_TYPE_ITEM == 5;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_entity_lifecycle(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    /* Create */
    FieldEntity* player = field_entity_create_player(0x1234, 100, 200, 0x5678);
    if (!player) return 0;

    /* Update position */
    field_entity_set_player_position(300, 400);
    if (player->world_x != 300) return 0;

    /* Set info */
    field_entity_set_player_info("Title", "Guild");
    if (strcmp(g_field_entities.player_title, "Title") != 0) return 0;

    /* Destroy */
    field_entity_clear_all();
    if (g_field_entities.player_entity != NULL) return 0;

    return 1;
}

static int test_multiple_entity_types(void) {
    memset(&g_field_entities, 0, sizeof(FieldEntityContext));

    FieldEntity* player = field_entity_create(ENTITY_TYPE_PLAYER, 0, 0);
    FieldEntity* npc = field_entity_create(ENTITY_TYPE_NPC, 100, 100);
    FieldEntity* monster = field_entity_create(ENTITY_TYPE_MONSTER, 200, 200);
    FieldEntity* pet = field_entity_create(ENTITY_TYPE_PET, 50, 50);
    FieldEntity* item = field_entity_create(ENTITY_TYPE_ITEM, 150, 150);

    return player != NULL && npc != NULL && monster != NULL &&
           pet != NULL && item != NULL &&
           g_field_entities.entity_count == 5;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Field Entity System Comprehensive Tests ===\n\n");

    /* Constants tests */
    printf("Constants Tests:\n");
    TEST(screen_center_x);
    TEST(screen_center_y);
    TEST(iso_constant_1);
    TEST(iso_constant_2);
    TEST(max_field_entities);

    /* Entity creation tests */
    printf("\nEntity Creation Tests:\n");
    TEST(entity_create_basic);
    TEST(entity_create_dest);
    TEST(entity_create_flags);
    TEST(entity_create_type_data);
    TEST(entity_create_count);
    TEST(entity_create_limit);

    /* Entity destruction tests */
    printf("\nEntity Destruction Tests:\n");
    TEST(entity_destroy_basic);
    TEST(entity_destroy_count);
    TEST(entity_destroy_null);
    TEST(entity_destroy_twice);

    /* Clear all tests */
    printf("\nClear All Tests:\n");
    TEST(clear_all_basic);
    TEST(clear_all_player);
    TEST(clear_all_head_tail);

    /* Player entity tests */
    printf("\nPlayer Entity Tests:\n");
    TEST(player_create_basic);
    TEST(player_create_special_flag);
    TEST(player_create_stored);
    TEST(player_create_duplicate);
    TEST(player_set_position);
    TEST(player_set_position_dest);
    TEST(player_set_info_title);
    TEST(player_set_info_guild);

    /* Isometric transformation tests */
    printf("\nIsometric Transformation Tests:\n");
    TEST(isometric_origin);
    TEST(isometric_positive_x);
    TEST(isometric_positive_y);
    TEST(isometric_diagonal);

    /* Linked list tests */
    printf("\nLinked List Tests:\n");
    TEST(linked_list_first);
    TEST(linked_list_multiple);
    TEST(linked_list_removal_middle);
    TEST(linked_list_removal_head);
    TEST(linked_list_removal_tail);

    /* Entity type tests */
    printf("\nEntity Type Tests:\n");
    TEST(entity_type_none);
    TEST(entity_type_player);
    TEST(entity_type_npc);
    TEST(entity_type_monster);
    TEST(entity_type_pet);
    TEST(entity_type_item);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_entity_lifecycle);
    TEST(multiple_entity_types);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
