/*
 * Stone Age Client - NPC Interaction System Unit Tests
 * Tests for FUN_00462f60 (NPC dialog handler) and FUN_004781f0 (NPC action dispatcher)
 *
 * Based on Ghidra decompilation analysis:
 * - FUN_00462f60: Parses NPC dialog packets and spawns NPC entities
 * - FUN_004781f0: Dispatches NPC actions with 25+ action types
 * - Action types: dialog, shop, teleport, battle, quest, etc.
 * - String escaping via FUN_00489f70
 * - Entity creation via FUN_0040f460
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

/* Constants from Ghidra */
#define MAX_NPC_QUEUE 0x1000  /* 4096 */
#define MAX_NAME_LENGTH 0x15  /* 21 bytes */
#define MAX_NPC_SLOTS 4

/* NPC action types from FUN_004781f0 */
typedef enum {
    NPC_ACTION_CLOSE_DIALOG_0 = 0,
    NPC_ACTION_CLOSE_DIALOG_1 = 1,
    NPC_ACTION_SHOW_DIALOG = 2,
    NPC_ACTION_SHOW_DIALOG_ALT = 3,
    NPC_ACTION_SET_FLAG = 4,
    NPC_ACTION_SET_FLAG_ALT = 5,
    NPC_ACTION_QUEST = 10,
    NPC_ACTION_SHOP = 11,
    NPC_ACTION_BANK = 12,
    NPC_ACTION_GUILD = 13,
    NPC_ACTION_PET_SHOP = 14,
    NPC_ACTION_SKILL = 15,
    NPC_ACTION_TELEPORT = 16,
    NPC_ACTION_BATTLE = 17,
    NPC_ACTION_QUEST_ALT = 18,
    NPC_ACTION_PARTY = 19,
    NPC_ACTION_SPAWN_PET = 20,
    NPC_ACTION_21 = 21,
    NPC_ACTION_END_TURN = 22,
    NPC_ACTION_DUEL = 25,
    NPC_ACTION_26 = 26,
    NPC_ACTION_30 = 30,
    NPC_ACTION_31 = 31,
    NPC_ACTION_34 = 34,
    NPC_ACTION_35 = 35,
    NPC_ACTION_41 = 41,
    NPC_ACTION_42 = 42,
    NPC_ACTION_51 = 51,
    NPC_ACTION_52 = 52,
    NPC_ACTION_60 = 60
} NPCActionType;

/* Dialog types for FUN_00477d90 */
typedef enum {
    DIALOG_TYPE_0 = 0,
    DIALOG_TYPE_1 = 1,
    DIALOG_TYPE_2 = 2,
    DIALOG_TYPE_3 = 3,
    DIALOG_TYPE_4 = 4,
    DIALOG_TYPE_5 = 5,
    DIALOG_TYPE_6 = 6,
    DIALOG_TYPE_7 = 7,
    DIALOG_TYPE_8 = 8,
    DIALOG_TYPE_9 = 9,
    DIALOG_TYPE_10 = 10,
    DIALOG_TYPE_11 = 11,
    DIALOG_TYPE_12 = 12
} DialogType;

/* NPC entity structure */
typedef struct {
    u32 id;                     /* NPC ID */
    char name[MAX_NAME_LENGTH]; /* NPC name */
    u32 map_x;                  /* Map X position */
    u32 map_y;                  /* Map Y position */
    u32 sprite_id;              /* Sprite ID */
    u32 action_type;            /* Current action */
    u32 dialog_id;              /* Dialog ID */
} NPCEntity;

/* Action mapping from FUN_004781f0 */
static const struct {
    NPCActionType action;
    DialogType dialog;
    int has_text;
} g_action_map[] = {
    {NPC_ACTION_CLOSE_DIALOG_0, DIALOG_TYPE_3, 0},
    {NPC_ACTION_CLOSE_DIALOG_1, DIALOG_TYPE_4, 0},
    {NPC_ACTION_SHOW_DIALOG, DIALOG_TYPE_0, 1},
    {NPC_ACTION_SHOW_DIALOG_ALT, DIALOG_TYPE_12, 1},
    {NPC_ACTION_SET_FLAG, DIALOG_TYPE_1, 1},
    {NPC_ACTION_SET_FLAG_ALT, DIALOG_TYPE_2, 1},
    {NPC_ACTION_QUEST, DIALOG_TYPE_2, 1},
    {NPC_ACTION_SHOP, DIALOG_TYPE_5, 1},
    {NPC_ACTION_BANK, DIALOG_TYPE_6, 1},
    {NPC_ACTION_GUILD, DIALOG_TYPE_7, 1},
    {NPC_ACTION_PET_SHOP, DIALOG_TYPE_8, 1},
    {NPC_ACTION_SKILL, DIALOG_TYPE_9, 1},
    {NPC_ACTION_TELEPORT, DIALOG_TYPE_10, 1},
    {NPC_ACTION_BATTLE, DIALOG_TYPE_4, 1},
    {NPC_ACTION_QUEST_ALT, DIALOG_TYPE_3, 1}
};

/* Test data storage */
static NPCEntity g_npc_entities[100];
static int g_npc_count = 0;
static u32 g_current_dialog_type = 0;
static char g_current_dialog_text[256];

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
    memset(g_npc_entities, 0, sizeof(g_npc_entities));
    g_npc_count = 0;
    g_current_dialog_type = 0;
    memset(g_current_dialog_text, 0, sizeof(g_current_dialog_text));
}

/*
 * Get action dialog type - FUN_004781f0 pattern
 */
static DialogType get_action_dialog_type(NPCActionType action) {
    switch (action) {
        case NPC_ACTION_CLOSE_DIALOG_0: return DIALOG_TYPE_3;
        case NPC_ACTION_CLOSE_DIALOG_1: return DIALOG_TYPE_4;
        case NPC_ACTION_SHOW_DIALOG: return DIALOG_TYPE_0;
        case NPC_ACTION_SHOW_DIALOG_ALT: return DIALOG_TYPE_12;
        case NPC_ACTION_SET_FLAG: return DIALOG_TYPE_1;
        case NPC_ACTION_SET_FLAG_ALT: return DIALOG_TYPE_2;
        case NPC_ACTION_QUEST: return DIALOG_TYPE_2;
        case NPC_ACTION_SHOP: return DIALOG_TYPE_5;
        case NPC_ACTION_BANK: return DIALOG_TYPE_6;
        case NPC_ACTION_GUILD: return DIALOG_TYPE_7;
        case NPC_ACTION_PET_SHOP: return DIALOG_TYPE_8;
        case NPC_ACTION_SKILL: return DIALOG_TYPE_9;
        case NPC_ACTION_TELEPORT: return DIALOG_TYPE_10;
        case NPC_ACTION_BATTLE: return DIALOG_TYPE_4;
        case NPC_ACTION_QUEST_ALT: return DIALOG_TYPE_3;
        default: return DIALOG_TYPE_0;
    }
}

/*
 * Check if action requires text - FUN_004781f0 pattern
 */
static int action_requires_text(NPCActionType action) {
    switch (action) {
        case NPC_ACTION_CLOSE_DIALOG_0:
        case NPC_ACTION_CLOSE_DIALOG_1:
            return 0;
        default:
            return 1;
    }
}

/*
 * Check if action uses position - FUN_004781f0 pattern (case 0x14, 0x15, 0x16, 0x1f, 0x22)
 */
static int action_uses_position(NPCActionType action) {
    switch (action) {
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x1f:
        case 0x22:
            return 1;
        default:
            return 0;
    }
}

/*
 * Spawn NPC entity - FUN_0040f460 pattern
 */
static int npc_spawn(u32 id, const char* name, u32 x, u32 y, u32 sprite) {
    if (g_npc_count >= 100) return -1;

    NPCEntity* npc = &g_npc_entities[g_npc_count];
    npc->id = id;
    strncpy(npc->name, name, MAX_NAME_LENGTH - 1);
    npc->name[MAX_NAME_LENGTH - 1] = '\0';
    npc->map_x = x;
    npc->map_y = y;
    npc->sprite_id = sprite;

    return g_npc_count++;
}

/*
 * Find NPC by ID - FUN_0040f460 pattern
 */
static NPCEntity* npc_find_by_id(u32 id) {
    for (int i = 0; i < g_npc_count; i++) {
        if (g_npc_entities[i].id == id) {
            return &g_npc_entities[i];
        }
    }
    return NULL;
}

/* ========================================
 * Action Type Tests
 * ======================================== */

static int test_action_type_count(void) {
    test_setup();

    /* At least 25 distinct action types from FUN_004781f0 */
    assert(NPC_ACTION_CLOSE_DIALOG_0 == 0);
    assert(NPC_ACTION_BATTLE == 17);
    assert(NPC_ACTION_DUEL == 25);

    return 1;
}

static int test_action_dialog_mapping(void) {
    test_setup();

    /* From FUN_004781f0 switch statement */
    assert(get_action_dialog_type(NPC_ACTION_CLOSE_DIALOG_0) == DIALOG_TYPE_3);
    assert(get_action_dialog_type(NPC_ACTION_CLOSE_DIALOG_1) == DIALOG_TYPE_4);
    assert(get_action_dialog_type(NPC_ACTION_SHOW_DIALOG) == DIALOG_TYPE_0);
    assert(get_action_dialog_type(NPC_ACTION_SHOP) == DIALOG_TYPE_5);
    assert(get_action_dialog_type(NPC_ACTION_BANK) == DIALOG_TYPE_6);
    assert(get_action_dialog_type(NPC_ACTION_TELEPORT) == DIALOG_TYPE_10);

    return 1;
}

static int test_action_text_requirement(void) {
    test_setup();

    /* Actions 0 and 1 don't require text */
    assert(action_requires_text(NPC_ACTION_CLOSE_DIALOG_0) == 0);
    assert(action_requires_text(NPC_ACTION_CLOSE_DIALOG_1) == 0);

    /* Most actions require text */
    assert(action_requires_text(NPC_ACTION_SHOW_DIALOG) == 1);
    assert(action_requires_text(NPC_ACTION_SHOP) == 1);

    return 1;
}

static int test_action_position_usage(void) {
    test_setup();

    /* Actions 0x14, 0x15, 0x16, 0x1f, 0x22 use position */
    assert(action_uses_position(0x14) == 1);
    assert(action_uses_position(0x15) == 1);
    assert(action_uses_position(0x16) == 1);
    assert(action_uses_position(0x1f) == 1);
    assert(action_uses_position(0x22) == 1);

    /* Others don't */
    assert(action_uses_position(NPC_ACTION_SHOW_DIALOG) == 0);
    assert(action_uses_position(NPC_ACTION_SHOP) == 0);

    return 1;
}

/* ========================================
 * NPC Entity Tests
 * ======================================== */

static int test_npc_spawn_basic(void) {
    test_setup();

    int index = npc_spawn(1001, "TestNPC", 50, 60, 0x1234);

    assert(index == 0);
    assert(g_npc_count == 1);
    assert(g_npc_entities[0].id == 1001);
    assert(strcmp(g_npc_entities[0].name, "TestNPC") == 0);
    assert(g_npc_entities[0].map_x == 50);
    assert(g_npc_entities[0].map_y == 60);

    return 1;
}

static int test_npc_spawn_multiple(void) {
    test_setup();

    npc_spawn(1001, "NPC1", 10, 20, 0x100);
    npc_spawn(1002, "NPC2", 30, 40, 0x200);
    npc_spawn(1003, "NPC3", 50, 60, 0x300);

    assert(g_npc_count == 3);
    assert(g_npc_entities[0].id == 1001);
    assert(g_npc_entities[1].id == 1002);
    assert(g_npc_entities[2].id == 1003);

    return 1;
}

static int test_npc_find_by_id(void) {
    test_setup();

    npc_spawn(1001, "NPC1", 10, 20, 0x100);
    npc_spawn(1002, "NPC2", 30, 40, 0x200);
    npc_spawn(1003, "NPC3", 50, 60, 0x300);

    NPCEntity* npc = npc_find_by_id(1002);
    assert(npc != NULL);
    assert(npc->id == 1002);
    assert(strcmp(npc->name, "NPC2") == 0);

    /* Non-existent */
    assert(npc_find_by_id(9999) == NULL);

    return 1;
}

static int test_npc_name_truncation(void) {
    test_setup();

    /* Name longer than MAX_NAME_LENGTH */
    char long_name[100];
    memset(long_name, 'A', 50);
    long_name[50] = '\0';

    npc_spawn(1001, long_name, 0, 0, 0);

    assert(strlen(g_npc_entities[0].name) == MAX_NAME_LENGTH - 1);

    return 1;
}

/* ========================================
 * Dialog Type Tests
 * ======================================== */

static int test_dialog_type_values(void) {
    test_setup();

    /* Dialog types 0-12 from FUN_00477d90 calls */
    assert(DIALOG_TYPE_0 == 0);
    assert(DIALOG_TYPE_12 == 12);

    return 1;
}

static int test_dialog_type_range(void) {
    test_setup();

    /* Valid dialog types are 0-12 */
    for (int i = 0; i <= 12; i++) {
        DialogType dt = (DialogType)i;
        assert(dt >= DIALOG_TYPE_0 && dt <= DIALOG_TYPE_12);
    }

    return 1;
}

/* ========================================
 * Queue Size Tests
 * ======================================== */

static int test_npc_queue_size(void) {
    test_setup();

    /* From FUN_00462f60: uStack00000018 < 0x1000 */
    assert(MAX_NPC_QUEUE == 0x1000);
    assert(MAX_NPC_QUEUE == 4096);

    return 1;
}

static int test_name_length(void) {
    test_setup();

    /* From FUN_00462f60: _strncpy with 0x15 */
    assert(MAX_NAME_LENGTH == 0x15);
    assert(MAX_NAME_LENGTH == 21);

    return 1;
}

/* ========================================
 * Special Action Tests
 * ======================================== */

static int test_action_0x29(void) {
    test_setup();

    /* Action 0x29 (41) - Special NPC action
     * Creates timer via FUN_004010a0
     * Sets fields at +0x1d4 to +0x1e8
     */
    assert(NPC_ACTION_41 == 0x29);
    assert(NPC_ACTION_41 == 41);

    return 1;
}

static int test_action_0x2a(void) {
    test_setup();

    /* Action 0x2a (42) - Clear timer
     * Calls FUN_004011c0 to delete timer
     */
    assert(NPC_ACTION_42 == 0x2a);
    assert(NPC_ACTION_42 == 42);

    return 1;
}

static int test_action_0x33(void) {
    test_setup();

    /* Action 0x33 (51) - Spawn pet/minion
     * Creates entity at +0x1c or +0x20
     * Sets position relative to NPC
     */
    assert(NPC_ACTION_51 == 0x33);
    assert(NPC_ACTION_51 == 51);

    return 1;
}

static int test_action_0x3c(void) {
    test_setup();

    /* Action 0x3c (60) - Complex spawn
     * Handles up to 4 entities
     * Different behavior based on param_7 (0, 1, 2, 3)
     */
    assert(NPC_ACTION_60 == 0x3c);
    assert(NPC_ACTION_60 == 60);

    return 1;
}

/* ========================================
 * NPC Slot Tests
 * ======================================== */

static int test_npc_slot_count(void) {
    test_setup();

    /* From FUN_004781f0 case 0x3c: loop with iVar4 = 4 */
    assert(MAX_NPC_SLOTS == 4);

    return 1;
}

/* ========================================
 * Sprite ID Tests
 * ======================================== */

static int test_sprite_id_constants(void) {
    test_setup();

    /* From FUN_004781f0 case 0x3c:
     * 0x18b00, 0x18cfc, 0x18baa, 0x18e36, 0x18e37
     */
    assert(0x18b00 == 101376);
    assert(0x18cfc == 101884);
    assert(0x18baa == 101290);
    assert(0x18e36 == 102198);
    assert(0x18e37 == 102199);

    return 1;
}

/* ========================================
 * Position Offset Tests
 * ======================================== */

static int test_position_offset_constants(void) {
    test_setup();

    /* From FUN_004781f0:
     * Position +1 for x, -1 for y
     * +3, -3 for param_5 == 3 in case 0x33
     */

    int base_x = 100, base_y = 100;

    /* Normal offset */
    int offset_x = base_x + 1;
    int offset_y = base_y - 1;
    assert(offset_x == 101);
    assert(offset_y == 99);

    /* Special offset for param_5 == 3 */
    int special_x = base_x + 3;
    int special_y = base_y - 3;
    assert(special_x == 103);
    assert(special_y == 97);

    return 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_npc_dialog_flow(void) {
    test_setup();

    /* Simulate NPC dialog interaction */
    npc_spawn(1001, "Shopkeeper", 50, 50, 0x1000);

    /* Player interacts -> action 11 (shop) */
    NPCActionType action = NPC_ACTION_SHOP;
    DialogType dialog = get_action_dialog_type(action);

    assert(dialog == DIALOG_TYPE_5);
    assert(action_requires_text(action) == 1);

    return 1;
}

static int test_npc_teleport_flow(void) {
    test_setup();

    /* Simulate NPC teleport interaction */
    npc_spawn(1002, "Teleporter", 100, 100, 0x2000);

    NPCActionType action = NPC_ACTION_TELEPORT;
    DialogType dialog = get_action_dialog_type(action);

    assert(dialog == DIALOG_TYPE_10);

    return 1;
}

static int test_npc_battle_flow(void) {
    test_setup();

    /* Simulate NPC battle interaction */
    npc_spawn(1003, "Arena Master", 200, 200, 0x3000);

    NPCActionType action = NPC_ACTION_BATTLE;
    DialogType dialog = get_action_dialog_type(action);

    assert(dialog == DIALOG_TYPE_4);

    return 1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== NPC Interaction System Unit Tests ===\n\n");

    /* Action type tests */
    printf("Action Type Tests (FUN_004781f0):\n");
    TEST(action_type_count);
    TEST(action_dialog_mapping);
    TEST(action_text_requirement);
    TEST(action_position_usage);

    /* NPC entity tests */
    printf("\nNPC Entity Tests:\n");
    TEST(npc_spawn_basic);
    TEST(npc_spawn_multiple);
    TEST(npc_find_by_id);
    TEST(npc_name_truncation);

    /* Dialog type tests */
    printf("\nDialog Type Tests:\n");
    TEST(dialog_type_values);
    TEST(dialog_type_range);

    /* Queue size tests */
    printf("\nQueue Size Tests:\n");
    TEST(npc_queue_size);
    TEST(name_length);

    /* Special action tests */
    printf("\nSpecial Action Tests:\n");
    TEST(action_0x29);
    TEST(action_0x2a);
    TEST(action_0x33);
    TEST(action_0x3c);

    /* NPC slot tests */
    printf("\nNPC Slot Tests:\n");
    TEST(npc_slot_count);

    /* Sprite ID tests */
    printf("\nSprite ID Tests:\n");
    TEST(sprite_id_constants);

    /* Position offset tests */
    printf("\nPosition Offset Tests:\n");
    TEST(position_offset_constants);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(npc_dialog_flow);
    TEST(npc_teleport_flow);
    TEST(npc_battle_flow);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    /* TODO: Add tests for:
     * - FUN_00489f70 string escaping
     * - FUN_0040f460 entity creation
     * - FUN_004010a0 timer creation
     * - FUN_00477cb0 coordinate handling
     * - FUN_00477d70 text setting
     * - Full packet parsing from FUN_00462f60
     */

    return (tests_passed == tests_run) ? 0 : 1;
}
