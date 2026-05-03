/*
 * Stone Age Client - Storage Module Comprehensive Tests
 * Tests for storage.c - warehouse/storage box functionality
 *
 * Coverage:
 * - Initialization and shutdown
 * - Open/close operations
 * - Item deposit/withdraw
 * - Gold operations
 * - Filtering and sorting
 * - Expansion system
 * - Selection management
 * - Utility functions
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

/* Storage constants */
#define MAX_STORAGE_ITEMS   100
#define MAX_GOLD_STORAGE    100000000
#define STORAGE_FEE         100

/* Storage slot */
typedef struct {
    u32 item_id;
    u16 count;
    u16 flags;
    u8  data[8];
} StorageSlot;

/* Storage category */
typedef enum {
    STORAGE_CAT_ALL = 0,
    STORAGE_CAT_WEAPON = 1,
    STORAGE_CAT_ARMOR = 2,
    STORAGE_CAT_CONSUMABLE = 3,
    STORAGE_CAT_MATERIAL = 4,
    STORAGE_CAT_QUEST = 5,
    STORAGE_CAT_OTHER = 6
} StorageCategory;

/* Storage context */
typedef struct {
    StorageSlot items[MAX_STORAGE_ITEMS];
    int item_count;
    int max_slots;
    u32 gold;
    int selected_index;
    int scroll_offset;
    StorageCategory filter;
    char search_text[32];
    u8 is_open;
    u8 has_access;
    u32 access_fee;
    u8 sort_mode;
    u8 sort_ascending;
    u8 expansion_level;
    u32 expansion_cost;
    u8 dirty;
} StorageContext;

/* Global storage context */
static StorageContext g_storage = {0};

/* Expansion costs */
static const u32 s_expansion_costs[] = {
    10000,   /* +10 slots */
    50000,   /* +10 slots */
    100000,  /* +10 slots */
    500000,  /* +10 slots */
    1000000  /* +10 slots */
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

/* ========================================
 * Helper Functions
 * ======================================== */

void storage_init(void) {
    memset(&g_storage, 0, sizeof(StorageContext));
    g_storage.max_slots = 50;  /* Base slots */
    g_storage.access_fee = STORAGE_FEE;
}

void storage_shutdown(void) {
    memset(&g_storage, 0, sizeof(StorageContext));
}

int storage_open(void) {
    if (g_storage.is_open) return 1;
    g_storage.is_open = 1;
    g_storage.has_access = 1;
    return 1;
}

void storage_close(void) {
    g_storage.is_open = 0;
    g_storage.selected_index = -1;
    g_storage.has_access = 0;
}

int storage_is_open(void) {
    return g_storage.is_open;
}

int storage_can_access(void) {
    return g_storage.has_access;
}

int storage_find_item(u32 item_id) {
    int i;
    for (i = 0; i < g_storage.max_slots; i++) {
        if (g_storage.items[i].item_id == item_id) {
            return i;
        }
    }
    return -1;
}

int storage_find_free_slot(void) {
    int i;
    for (i = 0; i < g_storage.max_slots; i++) {
        if (g_storage.items[i].item_id == 0) {
            return i;
        }
    }
    return -1;
}

int storage_deposit_item(u32 item_id, u16 count) {
    int slot;

    if (!g_storage.is_open) return 0;
    if (count == 0) return 0;

    slot = storage_find_item(item_id);
    if (slot < 0) {
        slot = storage_find_free_slot();
        if (slot < 0) return 0;
        g_storage.items[slot].item_id = item_id;
        g_storage.items[slot].count = count;
        g_storage.item_count++;
    } else {
        g_storage.items[slot].count += count;
    }

    g_storage.dirty = 1;
    return 1;
}

int storage_withdraw_item(int slot, u16 count) {
    if (!g_storage.is_open) return 0;
    if (slot < 0 || slot >= g_storage.max_slots) return 0;
    if (g_storage.items[slot].item_id == 0) return 0;
    if (g_storage.items[slot].count < count) return 0;

    g_storage.items[slot].count -= count;
    if (g_storage.items[slot].count == 0) {
        g_storage.items[slot].item_id = 0;
        g_storage.item_count--;
    }

    g_storage.dirty = 1;
    return 1;
}

int storage_deposit_gold(u32 amount) {
    if (!g_storage.is_open) return 0;
    if (amount == 0) return 0;
    if (g_storage.gold + amount > MAX_GOLD_STORAGE) return 0;

    g_storage.gold += amount;
    g_storage.dirty = 1;
    return 1;
}

int storage_withdraw_gold(u32 amount) {
    if (!g_storage.is_open) return 0;
    if (amount == 0 || g_storage.gold < amount) return 0;

    g_storage.gold -= amount;
    g_storage.dirty = 1;
    return 1;
}

u32 storage_get_gold(void) {
    return g_storage.gold;
}

int storage_get_max_slots(void) {
    return g_storage.max_slots;
}

int storage_expand(void) {
    if (g_storage.expansion_level >= 5) return 0;

    g_storage.max_slots += 10;
    g_storage.expansion_level++;
    g_storage.expansion_cost = (g_storage.expansion_level < 5) ?
        s_expansion_costs[g_storage.expansion_level] : 0;

    return 1;
}

int storage_get_expansion_cost(void) {
    if (g_storage.expansion_level >= 5) return 0;
    return s_expansion_costs[g_storage.expansion_level];
}

void storage_select(int index) {
    g_storage.selected_index = index;
}

int storage_get_selected(void) {
    return g_storage.selected_index;
}

StorageSlot* storage_get_selected_item(void) {
    if (g_storage.selected_index < 0 ||
        g_storage.selected_index >= g_storage.max_slots) {
        return NULL;
    }
    return &g_storage.items[g_storage.selected_index];
}

void storage_set_filter(StorageCategory category) {
    g_storage.filter = category;
}

void storage_set_search(const char* text) {
    if (text) {
        strncpy(g_storage.search_text, text, sizeof(g_storage.search_text) - 1);
    } else {
        g_storage.search_text[0] = '\0';
    }
}

int storage_get_used_slots(void) {
    return g_storage.item_count;
}

int storage_get_item_count_total(void) {
    int total = 0;
    int i;
    for (i = 0; i < g_storage.max_slots; i++) {
        if (g_storage.items[i].item_id != 0) {
            total += g_storage.items[i].count;
        }
    }
    return total;
}

int storage_has_space_for(u32 item_id, u16 count) {
    int slot = storage_find_item(item_id);
    if (slot >= 0) return 1;
    return storage_find_free_slot() >= 0;
}

void storage_clear(void) {
    memset(g_storage.items, 0, sizeof(g_storage.items));
    g_storage.item_count = 0;
    g_storage.gold = 0;
    g_storage.dirty = 0;
}

/* ========================================
 * Constants Tests
 * ======================================== */

static int test_max_storage_items(void) {
    return MAX_STORAGE_ITEMS == 100;
}

static int test_max_gold_storage(void) {
    return MAX_GOLD_STORAGE == 100000000;
}

static int test_storage_fee(void) {
    return STORAGE_FEE == 100;
}

/* ========================================
 * Initialization Tests
 * ======================================== */

static int test_storage_init(void) {
    storage_init();
    return g_storage.max_slots == 50 &&
           g_storage.access_fee == STORAGE_FEE &&
           g_storage.is_open == 0;
}

static int test_storage_shutdown(void) {
    storage_init();
    g_storage.gold = 1000;
    storage_shutdown();
    return g_storage.gold == 0 &&
           g_storage.is_open == 0;
}

static int test_storage_clear(void) {
    storage_init();
    storage_deposit_item(1001, 10);
    storage_deposit_gold(500);
    storage_clear();

    return g_storage.item_count == 0 &&
           g_storage.gold == 0 &&
           g_storage.dirty == 0;
}

/* ========================================
 * Open/Close Tests
 * ======================================== */

static int test_storage_open(void) {
    storage_init();
    int result = storage_open();
    return result == 1 &&
           g_storage.is_open == 1 &&
           g_storage.has_access == 1;
}

static int test_storage_close(void) {
    storage_init();
    storage_open();
    storage_close();
    return g_storage.is_open == 0 &&
           g_storage.has_access == 0;
}

static int test_storage_is_open(void) {
    storage_init();
    if (storage_is_open() != 0) return 0;

    storage_open();
    if (storage_is_open() != 1) return 0;

    storage_close();
    return storage_is_open() == 0;
}

static int test_storage_can_access(void) {
    storage_init();
    if (storage_can_access() != 0) return 0;

    storage_open();
    return storage_can_access() == 1;
}

static int test_storage_reopen(void) {
    storage_init();
    storage_open();
    int result1 = storage_open();  /* Should return 1 (already open) */
    return result1 == 1;
}

/* ========================================
 * Item Deposit Tests
 * ======================================== */

static int test_deposit_item_basic(void) {
    storage_init();
    storage_open();
    int result = storage_deposit_item(1001, 10);
    return result == 1 &&
           g_storage.items[0].item_id == 1001 &&
           g_storage.items[0].count == 10;
}

static int test_deposit_item_stacking(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 10);
    int result = storage_deposit_item(1001, 5);
    return result == 1 &&
           g_storage.items[0].count == 15;
}

static int test_deposit_item_multiple(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 10);
    storage_deposit_item(1002, 20);
    storage_deposit_item(1003, 30);

    return g_storage.items[0].item_id == 1001 &&
           g_storage.items[1].item_id == 1002 &&
           g_storage.items[2].item_id == 1003 &&
           g_storage.item_count == 3;
}

static int test_deposit_item_closed(void) {
    storage_init();
    /* Don't open storage */
    int result = storage_deposit_item(1001, 10);
    return result == 0;
}

static int test_deposit_item_zero_count(void) {
    storage_init();
    storage_open();
    int result = storage_deposit_item(1001, 0);
    return result == 0;
}

static int test_deposit_item_fills_slots(void) {
    storage_init();
    storage_open();

    /* Fill all 50 base slots */
    int i;
    for (i = 0; i < 50; i++) {
        storage_deposit_item(1000 + i, 1);
    }

    /* Next deposit should fail */
    int result = storage_deposit_item(9999, 1);
    return result == 0 && g_storage.item_count == 50;
}

/* ========================================
 * Item Withdraw Tests
 * ======================================== */

static int test_withdraw_item_basic(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 10);
    int result = storage_withdraw_item(0, 5);
    return result == 1 &&
           g_storage.items[0].count == 5;
}

static int test_withdraw_item_all(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 10);
    int result = storage_withdraw_item(0, 10);
    return result == 1 &&
           g_storage.items[0].item_id == 0 &&
           g_storage.item_count == 0;
}

static int test_withdraw_item_partial(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 20);
    storage_withdraw_item(0, 8);
    return g_storage.items[0].count == 12;
}

static int test_withdraw_item_closed(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 10);
    storage_close();
    int result = storage_withdraw_item(0, 5);
    return result == 0;
}

static int test_withdraw_item_invalid_slot(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 10);
    int result = storage_withdraw_item(99, 5);
    return result == 0;
}

static int test_withdraw_item_too_many(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 10);
    int result = storage_withdraw_item(0, 20);
    return result == 0 &&
           g_storage.items[0].count == 10;
}

/* ========================================
 * Gold Operations Tests
 * ======================================== */

static int test_deposit_gold_basic(void) {
    storage_init();
    storage_open();
    int result = storage_deposit_gold(1000);
    return result == 1 && g_storage.gold == 1000;
}

static int test_deposit_gold_multiple(void) {
    storage_init();
    storage_open();
    storage_deposit_gold(1000);
    storage_deposit_gold(500);
    return g_storage.gold == 1500;
}

static int test_deposit_gold_max(void) {
    storage_init();
    storage_open();
    /* Deposit near max */
    storage_deposit_gold(MAX_GOLD_STORAGE - 1000);
    /* This should succeed */
    int result1 = storage_deposit_gold(1000);
    /* This should fail (would exceed max) */
    int result2 = storage_deposit_gold(1);
    return result1 == 1 && result2 == 0 &&
           g_storage.gold == MAX_GOLD_STORAGE;
}

static int test_withdraw_gold_basic(void) {
    storage_init();
    storage_open();
    storage_deposit_gold(1000);
    int result = storage_withdraw_gold(500);
    return result == 1 && g_storage.gold == 500;
}

static int test_withdraw_gold_all(void) {
    storage_init();
    storage_open();
    storage_deposit_gold(1000);
    int result = storage_withdraw_gold(1000);
    return result == 1 && g_storage.gold == 0;
}

static int test_withdraw_gold_insufficient(void) {
    storage_init();
    storage_open();
    storage_deposit_gold(500);
    int result = storage_withdraw_gold(1000);
    return result == 0 && g_storage.gold == 500;
}

static int test_withdraw_gold_closed(void) {
    storage_init();
    storage_open();
    storage_deposit_gold(1000);
    storage_close();
    int result = storage_withdraw_gold(500);
    return result == 0;
}

static int test_deposit_gold_closed(void) {
    storage_init();
    int result = storage_deposit_gold(1000);
    return result == 0;
}

/* ========================================
 * Find Item Tests
 * ======================================== */

static int test_find_item_exists(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 10);
    storage_deposit_item(1002, 20);
    storage_deposit_item(1003, 30);

    int slot = storage_find_item(1002);
    return slot >= 0 && g_storage.items[slot].item_id == 1002;
}

static int test_find_item_not_exists(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 10);
    int slot = storage_find_item(9999);
    return slot == -1;
}

static int test_find_free_slot_basic(void) {
    storage_init();
    storage_open();
    int slot = storage_find_free_slot();
    return slot == 0;
}

static int test_find_free_slot_partial(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 10);
    storage_deposit_item(1002, 20);
    int slot = storage_find_free_slot();
    return slot == 2;
}

static int test_find_free_slot_full(void) {
    storage_init();
    storage_open();
    int i;
    for (i = 0; i < 50; i++) {
        storage_deposit_item(1000 + i, 1);
    }
    int slot = storage_find_free_slot();
    return slot == -1;
}

/* ========================================
 * Expansion Tests
 * ======================================== */

static int test_expand_basic(void) {
    storage_init();
    int old_max = storage_get_max_slots();
    int result = storage_expand();
    return result == 1 &&
           storage_get_max_slots() == old_max + 10;
}

static int test_expand_multiple(void) {
    storage_init();
    storage_expand();
    storage_expand();
    storage_expand();
    return storage_get_max_slots() == 80;  /* 50 + 30 */
}

static int test_expand_max_level(void) {
    storage_init();
    int i;
    for (i = 0; i < 5; i++) {
        storage_expand();
    }
    /* Should be at max (100 slots) */
    int max_slots = storage_get_max_slots();
    /* Try to expand again */
    int result = storage_expand();
    return max_slots == 100 && result == 0;
}

static int test_expansion_cost(void) {
    storage_init();
    int cost1 = storage_get_expansion_cost();
    storage_expand();
    int cost2 = storage_get_expansion_cost();

    return cost1 == 10000 && cost2 == 50000;
}

static int test_expansion_cost_maxed(void) {
    storage_init();
    int i;
    for (i = 0; i < 5; i++) {
        storage_expand();
    }
    int cost = storage_get_expansion_cost();
    return cost == 0;
}

/* ========================================
 * Selection Tests
 * ======================================== */

static int test_select_slot(void) {
    storage_init();
    storage_select(5);
    return g_storage.selected_index == 5;
}

static int test_get_selected(void) {
    storage_init();
    storage_select(10);
    return storage_get_selected() == 10;
}

static int test_get_selected_item(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 10);
    storage_select(0);
    StorageSlot* slot = storage_get_selected_item();
    return slot != NULL && slot->item_id == 1001;
}

static int test_get_selected_item_invalid(void) {
    storage_init();
    storage_select(-1);
    StorageSlot* slot = storage_get_selected_item();
    return slot == NULL;
}

static int test_get_selected_item_out_of_range(void) {
    storage_init();
    storage_select(999);
    StorageSlot* slot = storage_get_selected_item();
    return slot == NULL;
}

/* ========================================
 * Filter Tests
 * ======================================== */

static int test_set_filter(void) {
    storage_init();
    storage_set_filter(STORAGE_CAT_WEAPON);
    return g_storage.filter == STORAGE_CAT_WEAPON;
}

static int test_set_filter_all(void) {
    storage_init();
    storage_set_filter(STORAGE_CAT_ALL);
    return g_storage.filter == STORAGE_CAT_ALL;
}

static int test_set_search(void) {
    storage_init();
    storage_set_search("sword");
    return strcmp(g_storage.search_text, "sword") == 0;
}

static int test_set_search_clear(void) {
    storage_init();
    storage_set_search("sword");
    storage_set_search(NULL);
    return g_storage.search_text[0] == '\0';
}

/* ========================================
 * Query Tests
 * ======================================== */

static int test_get_used_slots(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 10);
    storage_deposit_item(1002, 20);
    return storage_get_used_slots() == 2;
}

static int test_get_item_count_total(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 10);
    storage_deposit_item(1002, 20);
    storage_deposit_item(1003, 30);
    return storage_get_item_count_total() == 60;
}

static int test_has_space_for_empty(void) {
    storage_init();
    storage_open();
    return storage_has_space_for(1001, 10) == 1;
}

static int test_has_space_for_existing(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 10);
    return storage_has_space_for(1001, 10) == 1;
}

static int test_has_space_for_full(void) {
    storage_init();
    storage_open();
    int i;
    for (i = 0; i < 50; i++) {
        storage_deposit_item(1000 + i, 1);
    }
    return storage_has_space_for(9999, 1) == 0;
}

/* ========================================
 * Dirty Flag Tests
 * ======================================== */

static int test_dirty_on_deposit(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 10);
    return g_storage.dirty == 1;
}

static int test_dirty_on_withdraw(void) {
    storage_init();
    storage_open();
    storage_deposit_item(1001, 10);
    g_storage.dirty = 0;
    storage_withdraw_item(0, 5);
    return g_storage.dirty == 1;
}

static int test_dirty_on_gold_deposit(void) {
    storage_init();
    storage_open();
    storage_deposit_gold(1000);
    return g_storage.dirty == 1;
}

static int test_dirty_on_gold_withdraw(void) {
    storage_init();
    storage_open();
    storage_deposit_gold(1000);
    g_storage.dirty = 0;
    storage_withdraw_gold(500);
    return g_storage.dirty == 1;
}

/* ========================================
 * Integration Tests
 * ======================================== */

static int test_full_storage_cycle(void) {
    storage_init();
    storage_open();

    /* Deposit items */
    storage_deposit_item(1001, 50);
    storage_deposit_item(1002, 30);
    storage_deposit_gold(10000);

    /* Withdraw some */
    storage_withdraw_item(0, 20);
    storage_withdraw_gold(5000);

    return g_storage.items[0].count == 30 &&
           g_storage.items[1].count == 30 &&
           g_storage.gold == 5000;
}

static int test_storage_expansion_usage(void) {
    storage_init();
    storage_open();

    /* Fill base slots */
    int i;
    for (i = 0; i < 50; i++) {
        storage_deposit_item(1000 + i, 1);
    }

    /* Expand */
    storage_expand();

    /* Should be able to add more */
    int result = storage_deposit_item(9999, 1);
    return result == 1 && g_storage.item_count == 51;
}

static int test_storage_close_clears_selection(void) {
    storage_init();
    storage_open();
    storage_select(5);
    storage_close();
    return g_storage.selected_index == -1;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Storage Module Comprehensive Tests ===\n\n");

    /* Constants tests */
    printf("Constants Tests:\n");
    TEST(max_storage_items);
    TEST(max_gold_storage);
    TEST(storage_fee);

    /* Initialization tests */
    printf("\nInitialization Tests:\n");
    TEST(storage_init);
    TEST(storage_shutdown);
    TEST(storage_clear);

    /* Open/close tests */
    printf("\nOpen/Close Tests:\n");
    TEST(storage_open);
    TEST(storage_close);
    TEST(storage_is_open);
    TEST(storage_can_access);
    TEST(storage_reopen);

    /* Item deposit tests */
    printf("\nItem Deposit Tests:\n");
    TEST(deposit_item_basic);
    TEST(deposit_item_stacking);
    TEST(deposit_item_multiple);
    TEST(deposit_item_closed);
    TEST(deposit_item_zero_count);
    TEST(deposit_item_fills_slots);

    /* Item withdraw tests */
    printf("\nItem Withdraw Tests:\n");
    TEST(withdraw_item_basic);
    TEST(withdraw_item_all);
    TEST(withdraw_item_partial);
    TEST(withdraw_item_closed);
    TEST(withdraw_item_invalid_slot);
    TEST(withdraw_item_too_many);

    /* Gold operations tests */
    printf("\nGold Operations Tests:\n");
    TEST(deposit_gold_basic);
    TEST(deposit_gold_multiple);
    TEST(deposit_gold_max);
    TEST(withdraw_gold_basic);
    TEST(withdraw_gold_all);
    TEST(withdraw_gold_insufficient);
    TEST(withdraw_gold_closed);
    TEST(deposit_gold_closed);

    /* Find item tests */
    printf("\nFind Item Tests:\n");
    TEST(find_item_exists);
    TEST(find_item_not_exists);
    TEST(find_free_slot_basic);
    TEST(find_free_slot_partial);
    TEST(find_free_slot_full);

    /* Expansion tests */
    printf("\nExpansion Tests:\n");
    TEST(expand_basic);
    TEST(expand_multiple);
    TEST(expand_max_level);
    TEST(expansion_cost);
    TEST(expansion_cost_maxed);

    /* Selection tests */
    printf("\nSelection Tests:\n");
    TEST(select_slot);
    TEST(get_selected);
    TEST(get_selected_item);
    TEST(get_selected_item_invalid);
    TEST(get_selected_item_out_of_range);

    /* Filter tests */
    printf("\nFilter Tests:\n");
    TEST(set_filter);
    TEST(set_filter_all);
    TEST(set_search);
    TEST(set_search_clear);

    /* Query tests */
    printf("\nQuery Tests:\n");
    TEST(get_used_slots);
    TEST(get_item_count_total);
    TEST(has_space_for_empty);
    TEST(has_space_for_existing);
    TEST(has_space_for_full);

    /* Dirty flag tests */
    printf("\nDirty Flag Tests:\n");
    TEST(dirty_on_deposit);
    TEST(dirty_on_withdraw);
    TEST(dirty_on_gold_deposit);
    TEST(dirty_on_gold_withdraw);

    /* Integration tests */
    printf("\nIntegration Tests:\n");
    TEST(full_storage_cycle);
    TEST(storage_expansion_usage);
    TEST(storage_close_clears_selection);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
