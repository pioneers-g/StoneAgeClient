/*
 * Stone Age Client - NPC System Unit Tests
 * Tests for dialog actions, shop, entity spawning, packet parsing
 * Based on FUN_00462f60, FUN_00463380, FUN_004781f0 analysis
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../tests/test_framework.h"
#include "types.h"
#include "npc.h"

/* ========================================
 * Test Cases for Dialog Action Types
 * ======================================== */

/* Test dialog action type values */
static void test_dialog_action_values(void) {
    TEST_BEGIN("Dialog action type values");

    /* From FUN_004781f0 switch cases */
    TEST_ASSERT_EQ(DIALOG_ACTION_CLOSE, 0);
    TEST_ASSERT_EQ(DIALOG_ACTION_OPEN, 1);
    TEST_ASSERT_EQ(DIALOG_ACTION_SET_TEXT, 2);
    TEST_ASSERT_EQ(DIALOG_ACTION_SET_MENU, 3);
    TEST_ASSERT_EQ(DIALOG_ACTION_SET_OPTIONS, 4);
    TEST_ASSERT_EQ(DIALOG_ACTION_SET_VAR, 5);

    /* Shop/Menu actions */
    TEST_ASSERT_EQ(DIALOG_ACTION_SHOW_ITEM, 10);
    TEST_ASSERT_EQ(DIALOG_ACTION_MENU_BUY, 11);
    TEST_ASSERT_EQ(DIALOG_ACTION_MENU_SELL, 12);
    TEST_ASSERT_EQ(DIALOG_ACTION_MENU_TRADE, 13);
    TEST_ASSERT_EQ(DIALOG_ACTION_MENU_STORAGE, 14);
    TEST_ASSERT_EQ(DIALOG_ACTION_MENU_HEAL, 15);
    TEST_ASSERT_EQ(DIALOG_ACTION_MENU_TELEPORT, 16);
    TEST_ASSERT_EQ(DIALOG_ACTION_MENU_QUEST, 17);

    /* Status/Quest actions */
    TEST_ASSERT_EQ(DIALOG_ACTION_SHOW_STATUS, 18);
    TEST_ASSERT_EQ(DIALOG_ACTION_SHOW_QUEST, 19);

    /* Position/Confirm actions */
    TEST_ASSERT_EQ(DIALOG_ACTION_SET_POSITION, 20);
    TEST_ASSERT_EQ(DIALOG_ACTION_CONFIRM, 21);
    TEST_ASSERT_EQ(DIALOG_ACTION_SHOW_GUILD, 22);
    TEST_ASSERT_EQ(DIALOG_ACTION_SPECIAL_PARAM, 23);

    /* Display actions */
    TEST_ASSERT_EQ(DIALOG_ACTION_HIDE, 30);
    TEST_ASSERT_EQ(DIALOG_ACTION_SHOW_PARTY, 31);
    TEST_ASSERT_EQ(DIALOG_ACTION_QUEST_COMPLETE, 34);

    /* Entity actions */
    TEST_ASSERT_EQ(DIALOG_ACTION_ENTITY_EFFECT, 41);
    TEST_ASSERT_EQ(DIALOG_ACTION_CLEAR_ENTITY, 42);
    TEST_ASSERT_EQ(DIALOG_ACTION_CREATE_ENTITY, 43);
    TEST_ASSERT_EQ(DIALOG_ACTION_CLEAR_EFFECT, 51);
    TEST_ASSERT_EQ(DIALOG_ACTION_MULTI_SPAWN, 60);

    TEST_END();
}

/* Test NPC flags */
static void test_npc_flags(void) {
    TEST_BEGIN("NPC flags");

    TEST_ASSERT_EQ(NPC_FLAG_SHOP, 0x01);
    TEST_ASSERT_EQ(NPC_FLAG_QUEST, 0x02);
    TEST_ASSERT_EQ(NPC_FLAG_HEALER, 0x04);
    TEST_ASSERT_EQ(NPC_FLAG_STORAGE, 0x08);
    TEST_ASSERT_EQ(NPC_FLAG_TELEPORT, 0x10);

    TEST_END();
}

/* Test max constants */
static void test_npc_max_constants(void) {
    TEST_BEGIN("NPC max constants");

    TEST_ASSERT_EQ(MAX_DIALOGUE_ENTRIES, 40);
    TEST_ASSERT_EQ(MAX_SHOP_ITEMS, 100);
    TEST_ASSERT_EQ(MAX_SEEN_NPCS, 4096);
    TEST_ASSERT_EQ(MAX_DIALOGUE_TEXT, 256);
    TEST_ASSERT_EQ(MAX_SHOP_NAME, 32);
    TEST_ASSERT_EQ(MAX_NPC_NAME, 21);  /* 0x15 from FUN_00462f60 */
    TEST_ASSERT_EQ(MAX_DIALOG_OPTIONS, 4);
    TEST_ASSERT_EQ(MAX_NPCS, 256);

    TEST_END();
}

/* ========================================
 * Test Cases for Structure Sizes
 * ======================================== */

/* Test ShopItem structure size */
static void test_shop_item_size(void) {
    TEST_BEGIN("ShopItem size");

    /* ShopItem: id(4) + price(4) + stock(4) + flags(4) + name(32) = 48 bytes minimum */
    TEST_ASSERT(sizeof(ShopItem) >= 48);

    TEST_END();
}

/* Test DialogOption structure size */
static void test_dialog_option_size(void) {
    TEST_BEGIN("DialogOption size");

    /* DialogOption: text(64) + action(2) + param(2) + enabled(1) + selected(1) = 70 bytes minimum */
    TEST_ASSERT(sizeof(DialogOption) >= 70);

    TEST_END();
}

/* Test DialogEntity structure size */
static void test_dialog_entity_size(void) {
    TEST_BEGIN("DialogEntity size");

    /* DialogEntity: sprite_id(4) + x(4) + y(4) + action_type(2) + anim_frame(2) + visible(1) + reserved(3) = 20 bytes */
    TEST_ASSERT(sizeof(DialogEntity) >= 20);

    TEST_END();
}

/* Test DialogContext structure size */
static void test_dialog_context_size(void) {
    TEST_BEGIN("DialogContext size");

    /* DialogContext should have room for all fields */
    TEST_ASSERT(sizeof(DialogContext) >= sizeof(u32) * 6);  /* npc_id + params */
    TEST_ASSERT(sizeof(DialogContext) >= MAX_NPC_NAME);
    TEST_ASSERT(sizeof(DialogContext) >= MAX_DIALOGUE_TEXT);

    TEST_END();
}

/* Test NPCData structure size */
static void test_npc_data_size(void) {
    TEST_BEGIN("NPCData size");

    /* NPCData: id(4) + type_id(4) + sprite_id(2) + portrait_id(2) + x(2) + y(2) + direction(1) + flags(1) + name(21) = 39 bytes minimum */
    TEST_ASSERT(sizeof(NPCData) >= 39);

    TEST_END();
}

/* Test NPCContext structure size */
static void test_npc_context_size(void) {
    TEST_BEGIN("NPCContext size");

    /* NPCContext contains seen_npcs[4096] = 16384 bytes minimum */
    TEST_ASSERT(sizeof(NPCContext) >= MAX_SEEN_NPCS * sizeof(u32));

    TEST_END();
}

/* ========================================
 * Test Cases for NPC Context
 * ======================================== */

/* Test context initialization */
static void test_context_init(void) {
    TEST_BEGIN("Context initialization");

    memset(&g_npc, 0, sizeof(NPCContext));

    TEST_ASSERT_EQ(g_npc.current_npc, 0);
    TEST_ASSERT_EQ(g_npc.dialog_open, 0);
    TEST_ASSERT_EQ(g_npc.shop_open, 0);
    TEST_ASSERT_EQ(g_npc.in_quest, 0);
    TEST_ASSERT_EQ(g_npc.in_event, 0);
    TEST_ASSERT_EQ(g_npc.in_dialog, 0);
    TEST_ASSERT_EQ(g_npc.debug_mode, 0);
    TEST_ASSERT_EQ(g_npc.shop_count, 0);
    TEST_ASSERT_EQ(g_npc.seen_count, 0);
    TEST_ASSERT_EQ(g_npc.npc_count, 0);

    TEST_END();
}

/* Test npc_init function */
static void test_npc_init(void) {
    TEST_BEGIN("NPC init");

    int result = npc_init();

    TEST_ASSERT_EQ(result, 1);
    TEST_ASSERT_EQ(g_npc.debug_mode, 0);
    TEST_ASSERT_EQ(g_npc.shop_items, NULL);

    npc_shutdown();

    TEST_END();
}

/* Test npc_shutdown function */
static void test_npc_shutdown(void) {
    TEST_BEGIN("NPC shutdown");

    npc_init();
    npc_shutdown();

    TEST_ASSERT_EQ(g_npc.shop_items, NULL);
    TEST_ASSERT_EQ(g_npc.shop_count, 0);

    TEST_END();
}

/* ========================================
 * Test Cases for Field Parsing
 * ======================================== */

/* Test field parsing - basic */
static void test_parse_field_basic(void) {
    TEST_BEGIN("Parse field - basic");

    char output[64];

    /* Parse first field */
    TEST_ASSERT_EQ(npc_parse_field("a|b|c", '|', 0, output, sizeof(output)), 1);
    TEST_ASSERT_STR_EQ(output, "a");

    /* Parse second field */
    TEST_ASSERT_EQ(npc_parse_field("a|b|c", '|', 1, output, sizeof(output)), 1);
    TEST_ASSERT_STR_EQ(output, "b");

    /* Parse third field */
    TEST_ASSERT_EQ(npc_parse_field("a|b|c", '|', 2, output, sizeof(output)), 1);
    TEST_ASSERT_STR_EQ(output, "c");

    TEST_END();
}

/* Test field parsing - empty fields */
static void test_parse_field_empty(void) {
    TEST_BEGIN("Parse field - empty");

    char output[64];

    /* Empty string */
    TEST_ASSERT_EQ(npc_parse_field("", '|', 0, output, sizeof(output)), -1);

    /* Field beyond range */
    TEST_ASSERT_EQ(npc_parse_field("a|b", '|', 5, output, sizeof(output)), -1);

    TEST_END();
}

/* Test field parsing - long fields */
static void test_parse_field_long(void) {
    TEST_BEGIN("Parse field - long");

    char output[16];

    /* Field longer than buffer */
    int len = npc_parse_field("abcdefghijklmnopqrstuvwxyz", '|', 0, output, sizeof(output));
    TEST_ASSERT_EQ(len, 15);  /* max_len - 1 */
    TEST_ASSERT_STR_EQ(output, "abcdefghijklmno");

    TEST_END();
}

/* Test field parsing - integer */
static void test_parse_field_int(void) {
    TEST_BEGIN("Parse field - integer");

    TEST_ASSERT_EQ(npc_parse_field_int("123|456|789", '|', 0), 123);
    TEST_ASSERT_EQ(npc_parse_field_int("123|456|789", '|', 1), 456);
    TEST_ASSERT_EQ(npc_parse_field_int("123|456|789", '|', 2), 789);
    TEST_ASSERT_EQ(npc_parse_field_int("abc|def", '|', 0), 0);  /* Invalid number */

    TEST_END();
}

/* Test field parsing - protocol format */
static void test_parse_field_protocol(void) {
    TEST_BEGIN("Parse field - protocol format");

    /* From FUN_00462f60 protocol format:
     * NPC_ID|Dialog_Type|Param1|Action_Type|Param2|Param3|[Name]|Text...
     */
    const char* packet = "12345|1|100|2|200|300|NPCName|Hello World";

    char output[64];

    /* Parse NPC ID */
    TEST_ASSERT_EQ(npc_parse_field_int(packet, '|', 0), 12345);

    /* Parse Dialog Type */
    TEST_ASSERT_EQ(npc_parse_field_int(packet, '|', 1), 1);

    /* Parse Param1 */
    TEST_ASSERT_EQ(npc_parse_field_int(packet, '|', 2), 100);

    /* Parse Action Type */
    TEST_ASSERT_EQ(npc_parse_field_int(packet, '|', 3), 2);

    /* Parse Param2 */
    TEST_ASSERT_EQ(npc_parse_field_int(packet, '|', 4), 200);

    /* Parse Param3 */
    TEST_ASSERT_EQ(npc_parse_field_int(packet, '|', 5), 300);

    /* Parse Name */
    npc_parse_field(packet, '|', 6, output, sizeof(output));
    TEST_ASSERT_STR_EQ(output, "NPCName");

    /* Parse Text */
    npc_parse_field(packet, '|', 7, output, sizeof(output));
    TEST_ASSERT_STR_EQ(output, "Hello World");

    TEST_END();
}

/* ========================================
 * Test Cases for NPC Lookup
 * ======================================== */

/* Test find NPC by ID */
static void test_find_npc_by_id(void) {
    TEST_BEGIN("Find NPC by ID");

    memset(&g_npc, 0, sizeof(NPCContext));

    /* Add some seen NPCs */
    g_npc.seen_npcs[0] = 100;
    g_npc.seen_npcs[1] = 200;
    g_npc.seen_npcs[2] = 300;
    g_npc.seen_count = 3;

    /* Find existing NPC */
    TEST_ASSERT_EQ(npc_find_by_id(200), 200);

    /* Find non-existing NPC */
    TEST_ASSERT_EQ(npc_find_by_id(999), 0);

    TEST_END();
}

/* Test find shop item */
static void test_find_shop_item(void) {
    TEST_BEGIN("Find shop item");

    memset(&g_npc, 0, sizeof(NPCContext));

    /* Allocate shop items */
    g_npc.shop_items = (ShopItem*)malloc(MAX_SHOP_ITEMS * sizeof(ShopItem));
    memset(g_npc.shop_items, 0, MAX_SHOP_ITEMS * sizeof(ShopItem));

    /* Add some items */
    g_npc.shop_items[0].id = 1001;
    g_npc.shop_items[1].id = 1002;
    g_npc.shop_items[2].id = 1003;
    g_npc.shop_count = 3;

    /* Find existing item */
    TEST_ASSERT_EQ(npc_find_shop_item(1002), 1);

    /* Find non-existing item */
    TEST_ASSERT_EQ(npc_find_shop_item(9999), -1);

    free(g_npc.shop_items);
    g_npc.shop_items = NULL;

    TEST_END();
}

/* ========================================
 * Test Cases for Shop Operations
 * ======================================== */

/* Test open shop */
static void test_open_shop(void) {
    TEST_BEGIN("Open shop");

    memset(&g_npc, 0, sizeof(NPCContext));

    int result = npc_open_shop(12345, "TestShop");

    TEST_ASSERT_EQ(result, 1);
    TEST_ASSERT_EQ(g_npc.shop_open, 1);
    TEST_ASSERT_EQ(g_npc.current_npc, 12345);
    TEST_ASSERT_STR_EQ(g_npc.shop_name, "TestShop");

    npc_close_shop();

    TEST_END();
}

/* Test close shop */
static void test_close_shop(void) {
    TEST_BEGIN("Close shop");

    memset(&g_npc, 0, sizeof(NPCContext));

    npc_open_shop(12345, "TestShop");
    npc_close_shop();

    TEST_ASSERT_EQ(g_npc.shop_open, 0);
    TEST_ASSERT_EQ(g_npc.shop_count, 0);
    TEST_ASSERT_EQ(g_npc.current_npc, 0);

    TEST_END();
}

/* Test add shop item */
static void test_add_shop_item(void) {
    TEST_BEGIN("Add shop item");

    memset(&g_npc, 0, sizeof(NPCContext));

    npc_open_shop(12345, "TestShop");

    int result = npc_add_shop_item(1001, 100, 10);

    TEST_ASSERT_EQ(result, 1);
    TEST_ASSERT_EQ(g_npc.shop_count, 1);
    TEST_ASSERT_EQ(g_npc.shop_items[0].id, 1001);
    TEST_ASSERT_EQ(g_npc.shop_items[0].price, 100);
    TEST_ASSERT_EQ(g_npc.shop_items[0].stock, 10);

    npc_close_shop();

    TEST_END();
}

/* Test shop mode values */
static void test_shop_mode_values(void) {
    TEST_BEGIN("Shop mode values");

    memset(&g_npc, 0, sizeof(NPCContext));

    /* Initialize shop */
    npc_open_shop(1, "Test");

    /* Test shop mode 1 - Buy */
    DialogContext ctx = {0};
    strncpy(ctx.text, "Test", MAX_DIALOGUE_TEXT - 1);
    npc_process_dialog_action(&ctx, DIALOG_ACTION_MENU_BUY);
    TEST_ASSERT_EQ(g_npc.shop_mode, 1);

    /* Test shop mode 2 - Sell */
    npc_process_dialog_action(&ctx, DIALOG_ACTION_MENU_SELL);
    TEST_ASSERT_EQ(g_npc.shop_mode, 2);

    /* Test shop mode 3 - Trade */
    npc_process_dialog_action(&ctx, DIALOG_ACTION_MENU_TRADE);
    TEST_ASSERT_EQ(g_npc.shop_mode, 3);

    /* Test shop mode 4 - Storage */
    npc_process_dialog_action(&ctx, DIALOG_ACTION_MENU_STORAGE);
    TEST_ASSERT_EQ(g_npc.shop_mode, 4);

    /* Test shop mode 5 - Heal */
    npc_process_dialog_action(&ctx, DIALOG_ACTION_MENU_HEAL);
    TEST_ASSERT_EQ(g_npc.shop_mode, 5);

    /* Test shop mode 6 - Teleport */
    npc_process_dialog_action(&ctx, DIALOG_ACTION_MENU_TELEPORT);
    TEST_ASSERT_EQ(g_npc.shop_mode, 6);

    npc_close_shop();

    TEST_END();
}

/* ========================================
 * Test Cases for Dialog Operations
 * ======================================== */

/* Test set dialog position */
static void test_set_dialog_position(void) {
    TEST_BEGIN("Set dialog position");

    memset(&g_npc, 0, sizeof(NPCContext));

    DialogContext ctx = {0};
    ctx.param1 = 100;
    ctx.param2 = 200;
    strncpy(ctx.text, "Test text", MAX_DIALOGUE_TEXT - 1);

    npc_process_dialog_action(&ctx, DIALOG_ACTION_SET_POSITION);

    TEST_ASSERT_EQ(g_npc.dialog.pos_x, 100);
    TEST_ASSERT_EQ(g_npc.dialog.pos_y, 200);

    TEST_END();
}

/* Test dialog mode setting */
static void test_dialog_mode_setting(void) {
    TEST_BEGIN("Dialog mode setting");

    memset(&g_npc, 0, sizeof(NPCContext));

    DialogContext ctx = {0};
    strncpy(ctx.text, "Test text", MAX_DIALOGUE_TEXT - 1);

    /* Test mode 0 - SET_TEXT */
    npc_process_dialog_action(&ctx, DIALOG_ACTION_SET_TEXT);
    TEST_ASSERT_EQ(g_npc.dialog.mode, 0);

    /* Test mode 3 - CLOSE */
    npc_process_dialog_action(&ctx, DIALOG_ACTION_CLOSE);
    TEST_ASSERT_EQ(g_npc.dialog.mode, 3);

    /* Test mode 4 - OPEN */
    npc_process_dialog_action(&ctx, DIALOG_ACTION_OPEN);
    TEST_ASSERT_EQ(g_npc.dialog.mode, 4);

    TEST_END();
}

/* Test dialog options */
static void test_dialog_options(void) {
    TEST_BEGIN("Dialog options");

    memset(&g_npc, 0, sizeof(NPCContext));

    DialogContext ctx = {0};
    strncpy(ctx.text, "Option text", MAX_DIALOGUE_TEXT - 1);

    /* Test SET_MENU - mode 12 */
    npc_process_dialog_action(&ctx, DIALOG_ACTION_SET_MENU);
    TEST_ASSERT_EQ(g_npc.dialog.mode, 12);
    TEST_ASSERT_EQ(g_npc.dialog.has_options, 1);

    /* Test SET_OPTIONS - mode 1 */
    g_npc.dialog.has_options = 0;
    npc_process_dialog_action(&ctx, DIALOG_ACTION_SET_OPTIONS);
    TEST_ASSERT_EQ(g_npc.dialog.mode, 1);
    TEST_ASSERT_EQ(g_npc.dialog.has_options, 1);

    TEST_END();
}

/* Test confirm dialog */
static void test_confirm_dialog(void) {
    TEST_BEGIN("Confirm dialog");

    memset(&g_npc, 0, sizeof(NPCContext));

    DialogContext ctx = {0};
    ctx.param1 = 100;
    ctx.param2 = 200;
    ctx.param3 = 1;
    strncpy(ctx.text, "Confirm?", MAX_DIALOGUE_TEXT - 1);

    npc_process_dialog_action(&ctx, DIALOG_ACTION_CONFIRM);

    TEST_ASSERT_EQ(g_npc.dialog.pos_x, 100);
    TEST_ASSERT_EQ(g_npc.dialog.pos_y, 200);
    TEST_ASSERT_EQ(g_npc.dialog.show_confirm, 1);

    /* Test with param3 = 0 */
    ctx.param3 = 0;
    npc_process_dialog_action(&ctx, DIALOG_ACTION_CONFIRM);
    TEST_ASSERT_EQ(g_npc.dialog.show_confirm, 0);

    TEST_END();
}

/* Test guild display */
static void test_guild_display(void) {
    TEST_BEGIN("Guild display");

    memset(&g_npc, 0, sizeof(NPCContext));

    DialogContext ctx = {0};
    ctx.param1 = 100;
    ctx.param2 = 200;
    ctx.param3 = 1;
    strncpy(ctx.text, "Guild text", MAX_DIALOGUE_TEXT - 1);

    npc_process_dialog_action(&ctx, DIALOG_ACTION_SHOW_GUILD);

    TEST_ASSERT_EQ(g_npc.dialog.pos_x, 100);
    TEST_ASSERT_EQ(g_npc.dialog.pos_y, 200);
    TEST_ASSERT_EQ(g_npc.dialog.show_guild, 1);

    TEST_END();
}

/* ========================================
 * Test Cases for Entity Operations
 * ======================================== */

/* Test create dialog entity */
static void test_create_dialog_entity(void) {
    TEST_BEGIN("Create dialog entity");

    memset(&g_npc, 0, sizeof(NPCContext));

    npc_create_dialog_entity(1001, 100, 200, 5);

    TEST_ASSERT_EQ(g_npc.entity_count, 1);
    TEST_ASSERT_EQ(g_npc.entities[0].sprite_id, 1001);
    TEST_ASSERT_EQ(g_npc.entities[0].x, 100);
    TEST_ASSERT_EQ(g_npc.entities[0].y, 200);
    TEST_ASSERT_EQ(g_npc.entities[0].action_type, 5);
    TEST_ASSERT_EQ(g_npc.entities[0].visible, 1);

    TEST_END();
}

/* Test clear dialog entities */
static void test_clear_dialog_entities(void) {
    TEST_BEGIN("Clear dialog entities");

    memset(&g_npc, 0, sizeof(NPCContext));

    npc_create_dialog_entity(1001, 100, 200, 5);
    npc_create_dialog_entity(1002, 150, 250, 3);

    TEST_ASSERT_EQ(g_npc.entity_count, 2);

    npc_clear_dialog_entities();

    TEST_ASSERT_EQ(g_npc.entity_count, 0);

    TEST_END();
}

/* Test max dialog entities */
static void test_max_dialog_entities(void) {
    TEST_BEGIN("Max dialog entities");

    memset(&g_npc, 0, sizeof(NPCContext));

    /* Add max entities (4) */
    npc_create_dialog_entity(1, 0, 0, 0);
    npc_create_dialog_entity(2, 0, 0, 0);
    npc_create_dialog_entity(3, 0, 0, 0);
    npc_create_dialog_entity(4, 0, 0, 0);

    TEST_ASSERT_EQ(g_npc.entity_count, 4);

    /* Try to add one more - should fail silently */
    npc_create_dialog_entity(5, 0, 0, 0);

    /* Still 4 */
    TEST_ASSERT_EQ(g_npc.entity_count, 4);

    TEST_END();
}

/* ========================================
 * Test Cases for Quest Operations
 * ======================================== */

/* Test quest handling */
static void test_quest_handling(void) {
    TEST_BEGIN("Quest handling");

    memset(&g_npc, 0, sizeof(NPCContext));

    npc_handle_quest(12345, 100);

    TEST_ASSERT_EQ(g_npc.quest_npc, 12345);
    TEST_ASSERT_EQ(g_npc.current_quest, 100);
    TEST_ASSERT_EQ(g_npc.in_quest, 1);

    npc_end_quest();

    TEST_ASSERT_EQ(g_npc.quest_npc, 0);
    TEST_ASSERT_EQ(g_npc.current_quest, 0);
    TEST_ASSERT_EQ(g_npc.in_quest, 0);

    TEST_END();
}

/* Test quest complete action */
static void test_quest_complete_action(void) {
    TEST_BEGIN("Quest complete action");

    memset(&g_npc, 0, sizeof(NPCContext));

    g_npc.in_quest = 1;
    g_npc.current_quest = 100;

    DialogContext ctx = {0};
    ctx.param1 = 100;
    ctx.param2 = 200;
    ctx.param3 = 1;  /* Complete quest */
    strncpy(ctx.text, "Quest complete!", MAX_DIALOGUE_TEXT - 1);

    npc_process_dialog_action(&ctx, DIALOG_ACTION_QUEST_COMPLETE);

    TEST_ASSERT_EQ(g_npc.in_quest, 0);
    TEST_ASSERT_EQ(g_npc.current_quest, 0);

    TEST_END();
}

/* ========================================
 * Test Cases for NPC Interaction
 * ======================================== */

/* Test NPC interact */
static void test_npc_interact(void) {
    TEST_BEGIN("NPC interact");

    memset(&g_npc, 0, sizeof(NPCContext));

    npc_interact(12345);

    TEST_ASSERT_EQ(g_npc.current_npc, 12345);

    TEST_END();
}

/* Test set player data */
static void test_set_player_data(void) {
    TEST_BEGIN("Set player data");

    memset(&g_npc, 0, sizeof(NPCContext));

    char player_data[100] = {0};
    npc_set_player_data(player_data);

    TEST_ASSERT_EQ(g_npc.player_data, player_data);

    TEST_END();
}

/* ========================================
 * Test Cases for Special Actions
 * ======================================== */

/* Test special action handler */
static void test_special_action(void) {
    TEST_BEGIN("Special action");

    /* Should not crash with various params */
    npc_handle_special_action(0);
    npc_handle_special_action(1);
    npc_handle_special_action(2);
    npc_handle_special_action(99);

    TEST_ASSERT(1);  /* If we get here, no crash */

    TEST_END();
}

/* Test multi-spawn */
static void test_multi_spawn(void) {
    TEST_BEGIN("Multi-spawn");

    memset(&g_npc, 0, sizeof(NPCContext));

    /* Allocate entity for multi-spawn */
    char entity[0x200] = {0};
    g_npc.current_entity = entity;
    g_npc.base_x = 100;
    g_npc.base_y = 100;

    DialogContext ctx = {0};
    npc_handle_multi_spawn(&ctx);

    /* Multi-spawn creates 4 entities in slots at offset 0x24 */
    void** entity_slots = (void**)((char*)g_npc.current_entity + 0x24);

    /* Check slots are allocated */
    int allocated = 0;
    for (int i = 0; i < 4; i++) {
        if (entity_slots[i] != NULL) {
            allocated++;
            free(entity_slots[i]);
        }
    }

    TEST_ASSERT(allocated > 0);  /* At least some entities should be created */

    TEST_END();
}

/* ========================================
 * Test Suite
 * ======================================== */

TEST_SUITE(npc) {
    /* Dialog action types */
    test_dialog_action_values();
    test_npc_flags();
    test_npc_max_constants();

    /* Structure sizes */
    test_shop_item_size();
    test_dialog_option_size();
    test_dialog_entity_size();
    test_dialog_context_size();
    test_npc_data_size();
    test_npc_context_size();

    /* Context */
    test_context_init();
    test_npc_init();
    test_npc_shutdown();

    /* Field parsing */
    test_parse_field_basic();
    test_parse_field_empty();
    test_parse_field_long();
    test_parse_field_int();
    test_parse_field_protocol();

    /* NPC lookup */
    test_find_npc_by_id();
    test_find_shop_item();

    /* Shop operations */
    test_open_shop();
    test_close_shop();
    test_add_shop_item();
    test_shop_mode_values();

    /* Dialog operations */
    test_set_dialog_position();
    test_dialog_mode_setting();
    test_dialog_options();
    test_confirm_dialog();
    test_guild_display();

    /* Entity operations */
    test_create_dialog_entity();
    test_clear_dialog_entities();
    test_max_dialog_entities();

    /* Quest operations */
    test_quest_handling();
    test_quest_complete_action();

    /* NPC interaction */
    test_npc_interact();
    test_set_player_data();

    /* Special actions */
    test_special_action();
    test_multi_spawn();
}

/* ========================================
 * Main
 * ======================================== */

int main(int argc, char** argv) {
    printf("========================================\n");
    printf("Stone Age Client - NPC System Tests\n");
    printf("========================================\n\n");

    RUN_TEST_SUITE(npc);

    test_summary();

    return test_all_passed() ? 0 : 1;
}
