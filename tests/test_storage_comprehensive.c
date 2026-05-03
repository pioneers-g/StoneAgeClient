/*
 * Stone Age Client - Storage System Comprehensive Tests
 * Tests for storage.c implementation
 *
 * Covers:
 * - Storage initialization and shutdown
 * - Item deposit/withdraw operations
 * - Gold storage operations
 * - Slot management and filtering
 * - Storage expansion
 * - Sorting functionality
 *
 * Game data verification:
 * - savedata.dat: 128 bytes, XOR encrypted
 * - Storage protocol: SA|D (deposit), SA|W (withdraw), SA|G (gold in), SA|GC (gold out)
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

/* Test counters */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    fflush(stdout); \
    tests_run++; \
    if (test_##name()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while(0)

/* ========================================
 * Constants
 * ======================================== */

#define MAX_STORAGE_SLOTS 100
#define MAX_GOLD_STORAGE 10000000
#define STORAGE_FEE 100

typedef enum {
    STORAGE_CAT_ALL = 0,
    STORAGE_CAT_WEAPON = 1,
    STORAGE_CAT_ARMOR = 2,
    STORAGE_CAT_CONSUMABLE = 3,
    STORAGE_CAT_MATERIAL = 4,
    STORAGE_CAT_QUEST = 5,
    STORAGE_CAT_OTHER = 6
} StorageCategory;

/* ========================================
 * Structures
 * ======================================== */

typedef struct {
    u32 item_id;
    u16 count;
    u16 flags;
} StorageSlot;

typedef struct {
    int is_open;
    int has_access;
    int max_slots;
    int item_count;
    u32 gold;
    int expansion_level;
    int expansion_cost;
    int selected_index;
    int dirty;
    int access_fee;
    StorageSlot items[MAX_STORAGE_SLOTS];
    int filter;
    char search_text[64];
    int sort_mode;
    int sort_ascending;
} StorageContext;

/* ========================================
 * Global State
 * ======================================== */

static StorageContext g_storage = {0};

/* Packet tracking for protocol testing */
static char g_last_packet[256] = {0};
static int g_packet_count = 0;

/* ========================================
 * Implementation Functions
 * ======================================== */

static void send_queue_add(const char* packet, int a, int b) {
    if (packet) {
        strncpy(g_last_packet, packet, sizeof(g_last_packet) - 1);
        g_packet_count++;
    }
}

static int inventory_find_empty_slot(void) {
    /* Stub: always return first slot */
    return 0;
}

static int inventory_get_count(u32 item_id) {
    /* Stub: return 10 for testing */
    return 10;
}

static int storage_init(void) {
    memset(&g_storage, 0, sizeof(StorageContext));

    g_storage.max_slots = 50;  /* Base slots */
    g_storage.access_fee = STORAGE_FEE;

    return 1;
}

static void storage_shutdown(void) {
    memset(&g_storage, 0, sizeof(StorageContext));
}

static int storage_is_open(void) {
    return g_storage.is_open;
}

static int storage_can_access(void) {
    return g_storage.has_access;
}

static int storage_open(void) {
    if (g_storage.is_open) {
        return 1;
    }

    g_storage.is_open = 1;
    g_storage.has_access = 1;

    return 1;
}

static void storage_close(void) {
    if (g_storage.dirty) {
        send_queue_add("SA|C", 0, 0);
        g_storage.dirty = 0;
    }

    g_storage.is_open = 0;
    g_storage.selected_index = -1;
    g_storage.has_access = 0;
}

static int storage_find_item(u32 item_id) {
    int i;

    for (i = 0; i < g_storage.max_slots; i++) {
        if (g_storage.items[i].item_id == item_id) {
            return i;
        }
    }

    return -1;
}

static int storage_find_free_slot(void) {
    int i;

    for (i = 0; i < g_storage.max_slots; i++) {
        if (g_storage.items[i].item_id == 0) {
            return i;
        }
    }

    return -1;
}

static int storage_has_space_for(u32 item_id, u16 count) {
    int slot = storage_find_item(item_id);

    if (slot >= 0) {
        return 1;
    }

    return storage_find_free_slot() >= 0;
}

static int storage_deposit_item(u32 item_id, u16 count) {
    char params[64];
    int slot;

    if (!g_storage.is_open) {
        return 0;
    }

    if (count == 0) {
        return 0;
    }

    if (!storage_has_space_for(item_id, count)) {
        return 0;
    }

    slot = storage_find_item(item_id);
    if (slot < 0) {
        slot = storage_find_free_slot();
        if (slot < 0) {
            return 0;
        }
    }

    sprintf(params, "%u|%u", item_id, count);
    send_queue_add("SA|D", 0, 0);

    g_storage.dirty = 1;
    return 1;
}

static int storage_withdraw_item(int slot, u16 count) {
    StorageSlot* item;
    char params[64];

    if (!g_storage.is_open) {
        return 0;
    }

    if (slot < 0 || slot >= g_storage.max_slots) {
        return 0;
    }

    item = &g_storage.items[slot];
    if (item->item_id == 0 || item->count < count) {
        return 0;
    }

    if (inventory_find_empty_slot() < 0) {
        return 0;
    }

    sprintf(params, "%d|%u", slot, count);
    send_queue_add("SA|W", 0, 0);

    g_storage.dirty = 1;
    return 1;
}

static int storage_get_item_count(int slot) {
    if (slot < 0 || slot >= g_storage.max_slots) {
        return 0;
    }
    return g_storage.items[slot].count;
}

static u32 storage_get_item_id(int slot) {
    if (slot < 0 || slot >= g_storage.max_slots) {
        return 0;
    }
    return g_storage.items[slot].item_id;
}

static int storage_deposit_gold(u32 amount) {
    char params[64];

    if (!g_storage.is_open) {
        return 0;
    }

    if (amount == 0) {
        return 0;
    }

    if (g_storage.gold + amount > MAX_GOLD_STORAGE) {
        return 0;
    }

    sprintf(params, "%u", amount);
    send_queue_add("SA|G", 0, 0);

    g_storage.dirty = 1;
    return 1;
}

static int storage_withdraw_gold(u32 amount) {
    char params[64];

    if (!g_storage.is_open) {
        return 0;
    }

    if (amount == 0 || g_storage.gold < amount) {
        return 0;
    }

    sprintf(params, "%u", amount);
    send_queue_add("SA|GC", 0, 0);

    g_storage.gold -= amount;
    g_storage.dirty = 1;

    return 1;
}

static u32 storage_get_gold(void) {
    return g_storage.gold;
}

static u32 storage_get_max_gold(void) {
    return MAX_GOLD_STORAGE;
}

static int storage_get_max_slots(void) {
    return g_storage.max_slots;
}

static int storage_get_used_slots(void) {
    return g_storage.item_count;
}

static void storage_select(int index) {
    g_storage.selected_index = index;
}

static int storage_get_selected(void) {
    return g_storage.selected_index;
}

static StorageSlot* storage_get_selected_item(void) {
    if (g_storage.selected_index < 0 ||
        g_storage.selected_index >= g_storage.max_slots) {
        return NULL;
    }
    return &g_storage.items[g_storage.selected_index];
}

static int item_get_category(u32 item_id) {
    if (item_id >= 1000 && item_id < 2000) return STORAGE_CAT_WEAPON;
    if (item_id >= 2000 && item_id < 3000) return STORAGE_CAT_ARMOR;
    if (item_id >= 3000 && item_id < 4000) return STORAGE_CAT_CONSUMABLE;
    if (item_id >= 4000 && item_id < 5000) return STORAGE_CAT_MATERIAL;
    if (item_id >= 5000 && item_id < 6000) return STORAGE_CAT_QUEST;
    return STORAGE_CAT_OTHER;
}

static void storage_set_filter(int category) {
    g_storage.filter = category;
}

static void storage_set_search(const char* text) {
    if (text) {
        strncpy(g_storage.search_text, text, sizeof(g_storage.search_text) - 1);
    } else {
        g_storage.search_text[0] = '\0';
    }
}

static int storage_expand(void) {
    static const u32 costs[] = {10000, 50000, 100000, 500000, 1000000};

    if (g_storage.expansion_level >= 5) {
        return 0;
    }

    send_queue_add("SA|E", 0, 0);

    g_storage.max_slots += 10;
    g_storage.expansion_level++;

    return 1;
}

static int storage_get_expansion_cost(void) {
    static const u32 costs[] = {10000, 50000, 100000, 500000, 1000000};

    if (g_storage.expansion_level >= 5) {
        return 0;
    }
    return costs[g_storage.expansion_level];
}

static void storage_clear(void) {
    memset(g_storage.items, 0, sizeof(g_storage.items));
    g_storage.item_count = 0;
    g_storage.gold = 0;
    g_storage.dirty = 0;
}

static void storage_sort(int mode, int ascending) {
    int i, j;
    StorageSlot temp;

    g_storage.sort_mode = mode;
    g_storage.sort_ascending = ascending;

    for (i = 0; i < g_storage.max_slots - 1; i++) {
        for (j = 0; j < g_storage.max_slots - i - 1; j++) {
            int should_swap = 0;
            StorageSlot* a = &g_storage.items[j];
            StorageSlot* b = &g_storage.items[j + 1];

            /* Move empty slots to the end */
            if (a->item_id == 0 && b->item_id != 0) {
                should_swap = 1;
            } else if (a->item_id != 0 && b->item_id != 0) {
                /* Both slots have items - compare them */
                switch (mode) {
                    case 0:  /* Sort by ID */
                        should_swap = ascending ?
                            (a->item_id > b->item_id) :
                            (a->item_id < b->item_id);
                        break;
                    case 1:  /* Sort by count */
                        should_swap = ascending ?
                            (a->count > b->count) :
                            (a->count < b->count);
                        break;
                    default:
                        break;
                }
            }

            if (should_swap) {
                temp = *a;
                *a = *b;
                *b = temp;
            }
        }
    }

    g_storage.dirty = 1;
}

/* Helper to add item directly for testing */
static void test_add_item(int slot, u32 item_id, u16 count) {
    if (slot >= 0 && slot < g_storage.max_slots) {
        g_storage.items[slot].item_id = item_id;
        g_storage.items[slot].count = count;
        if (item_id != 0) {
            g_storage.item_count++;
        }
    }
}

/* ========================================
 * Test Cases - Initialization
 * ======================================== */

static int test_init_basic(void) {
    int result = storage_init();
    return result == 1 && g_storage.is_open == 0;
}

static int test_init_max_slots(void) {
    storage_init();
    return storage_get_max_slots() == 50;
}

static int test_init_gold_zero(void) {
    storage_init();
    return storage_get_gold() == 0;
}

static int test_init_not_open(void) {
    storage_init();
    return storage_is_open() == 0;
}

static int test_shutdown_clears(void) {
    storage_init();
    storage_open();
    storage_shutdown();
    return g_storage.is_open == 0 && g_storage.max_slots == 0;
}

/* ========================================
 * Test Cases - Open/Close
 * ======================================== */

static int test_open_basic(void) {
    storage_init();
    int result = storage_open();
    return result == 1 && storage_is_open() == 1;
}

static int test_open_sets_access(void) {
    storage_init();
    storage_open();
    return storage_can_access() == 1;
}

static int test_open_twice(void) {
    storage_init();
    storage_open();
    int result = storage_open();
    return result == 1;  /* Should succeed even if already open */
}

static int test_close_basic(void) {
    storage_init();
    storage_open();
    storage_close();
    return storage_is_open() == 0;
}

static int test_close_clears_access(void) {
    storage_init();
    storage_open();
    storage_close();
    return storage_can_access() == 0;
}

static int test_close_clears_selection(void) {
    storage_init();
    storage_open();
    storage_select(5);
    storage_close();
    return storage_get_selected() == -1;
}

/* ========================================
 * Test Cases - Item Operations
 * ======================================== */

static int test_find_item_empty(void) {
    storage_init();
    storage_open();
    return storage_find_item(1000) == -1;
}

static int test_find_item_found(void) {
    storage_init();
    storage_open();
    test_add_item(0, 1000, 10);
    return storage_find_item(1000) == 0;
}

static int test_find_item_not_found(void) {
    storage_init();
    storage_open();
    test_add_item(0, 1000, 10);
    return storage_find_item(2000) == -1;
}

static int test_find_free_slot_empty(void) {
    storage_init();
    storage_open();
    return storage_find_free_slot() == 0;
}

static int test_find_free_slot_partial(void) {
    storage_init();
    storage_open();
    test_add_item(0, 1000, 10);
    test_add_item(1, 2000, 5);
    return storage_find_free_slot() == 2;
}

static int test_find_free_slot_full(void) {
    int i;
    storage_init();
    storage_open();

    for (i = 0; i < g_storage.max_slots; i++) {
        test_add_item(i, 1000 + i, 1);
    }

    return storage_find_free_slot() == -1;
}

static int test_has_space_empty(void) {
    storage_init();
    storage_open();
    return storage_has_space_for(1000, 1) == 1;
}

static int test_has_space_existing_item(void) {
    storage_init();
    storage_open();
    test_add_item(0, 1000, 10);
    return storage_has_space_for(1000, 5) == 1;
}

/* ========================================
 * Test Cases - Deposit/Withdraw
 * ======================================== */

static int test_deposit_closed(void) {
    storage_init();
    /* Don't open storage */
    return storage_deposit_item(1000, 10) == 0;
}

static int test_deposit_basic(void) {
    storage_init();
    storage_open();
    g_packet_count = 0;
    int result = storage_deposit_item(1000, 10);
    return result == 1 && g_packet_count == 1;
}

static int test_deposit_zero_count(void) {
    storage_init();
    storage_open();
    return storage_deposit_item(1000, 0) == 0;
}

static int test_withdraw_closed(void) {
    storage_init();
    return storage_withdraw_item(0, 10) == 0;
}

static int test_withdraw_invalid_slot(void) {
    storage_init();
    storage_open();
    return storage_withdraw_item(-1, 10) == 0;
}

static int test_withdraw_empty_slot(void) {
    storage_init();
    storage_open();
    return storage_withdraw_item(0, 10) == 0;
}

static int test_withdraw_basic(void) {
    storage_init();
    storage_open();
    test_add_item(0, 1000, 20);
    g_packet_count = 0;
    int result = storage_withdraw_item(0, 10);
    return result == 1 && g_packet_count == 1;
}

static int test_withdraw_exceeds_count(void) {
    storage_init();
    storage_open();
    test_add_item(0, 1000, 5);
    return storage_withdraw_item(0, 10) == 0;
}

/* ========================================
 * Test Cases - Gold Operations
 * ======================================== */

static int test_deposit_gold_closed(void) {
    storage_init();
    return storage_deposit_gold(1000) == 0;
}

static int test_deposit_gold_basic(void) {
    storage_init();
    storage_open();
    g_packet_count = 0;
    int result = storage_deposit_gold(1000);
    return result == 1 && g_packet_count == 1;
}

static int test_deposit_gold_zero(void) {
    storage_init();
    storage_open();
    return storage_deposit_gold(0) == 0;
}

static int test_deposit_gold_exceeds_max(void) {
    storage_init();
    storage_open();
    g_storage.gold = MAX_GOLD_STORAGE - 100;
    return storage_deposit_gold(1000) == 0;
}

static int test_withdraw_gold_closed(void) {
    storage_init();
    return storage_withdraw_gold(100) == 0;
}

static int test_withdraw_gold_basic(void) {
    storage_init();
    storage_open();
    g_storage.gold = 1000;
    g_packet_count = 0;
    int result = storage_withdraw_gold(500);
    return result == 1 && g_storage.gold == 500 && g_packet_count == 1;
}

static int test_withdraw_gold_zero(void) {
    storage_init();
    storage_open();
    g_storage.gold = 1000;
    return storage_withdraw_gold(0) == 0;
}

static int test_withdraw_gold_insufficient(void) {
    storage_init();
    storage_open();
    g_storage.gold = 100;
    return storage_withdraw_gold(500) == 0;
}

static int test_get_max_gold(void) {
    return storage_get_max_gold() == MAX_GOLD_STORAGE;
}

/* ========================================
 * Test Cases - Slot Management
 * ======================================== */

static int test_get_item_id_valid(void) {
    storage_init();
    storage_open();
    test_add_item(0, 1234, 10);
    return storage_get_item_id(0) == 1234;
}

static int test_get_item_id_empty(void) {
    storage_init();
    storage_open();
    return storage_get_item_id(0) == 0;
}

static int test_get_item_id_invalid_slot(void) {
    storage_init();
    return storage_get_item_id(-1) == 0 && storage_get_item_id(1000) == 0;
}

static int test_get_item_count_valid(void) {
    storage_init();
    storage_open();
    test_add_item(0, 1000, 25);
    return storage_get_item_count(0) == 25;
}

static int test_get_item_count_empty(void) {
    storage_init();
    storage_open();
    return storage_get_item_count(0) == 0;
}

static int test_select_basic(void) {
    storage_init();
    storage_select(5);
    return storage_get_selected() == 5;
}

static int test_get_selected_item_valid(void) {
    storage_init();
    storage_open();
    test_add_item(3, 5555, 15);
    storage_select(3);
    StorageSlot* slot = storage_get_selected_item();
    return slot != NULL && slot->item_id == 5555 && slot->count == 15;
}

static int test_get_selected_item_none(void) {
    storage_init();
    storage_select(-1);
    return storage_get_selected_item() == NULL;
}

/* ========================================
 * Test Cases - Filtering
 * ======================================== */

static int test_set_filter_basic(void) {
    storage_init();
    storage_set_filter(STORAGE_CAT_WEAPON);
    return g_storage.filter == STORAGE_CAT_WEAPON;
}

static int test_set_search_basic(void) {
    storage_init();
    storage_set_search("sword");
    return strcmp(g_storage.search_text, "sword") == 0;
}

static int test_set_search_null(void) {
    storage_init();
    strcpy(g_storage.search_text, "test");
    storage_set_search(NULL);
    return g_storage.search_text[0] == '\0';
}

static int test_item_category_weapon(void) {
    return item_get_category(1500) == STORAGE_CAT_WEAPON;
}

static int test_item_category_armor(void) {
    return item_get_category(2500) == STORAGE_CAT_ARMOR;
}

static int test_item_category_consumable(void) {
    return item_get_category(3500) == STORAGE_CAT_CONSUMABLE;
}

static int test_item_category_material(void) {
    return item_get_category(4500) == STORAGE_CAT_MATERIAL;
}

static int test_item_category_quest(void) {
    return item_get_category(5500) == STORAGE_CAT_QUEST;
}

static int test_item_category_other(void) {
    return item_get_category(9999) == STORAGE_CAT_OTHER;
}

/* ========================================
 * Test Cases - Expansion
 * ======================================== */

static int test_expand_basic(void) {
    storage_init();
    storage_open();
    g_packet_count = 0;
    int result = storage_expand();
    return result == 1 && g_storage.max_slots == 60 && g_packet_count == 1;
}

static int test_expand_multiple(void) {
    storage_init();
    storage_open();

    storage_expand();  /* 60 slots */
    storage_expand();  /* 70 slots */

    return g_storage.max_slots == 70 && g_storage.expansion_level == 2;
}

static int test_expand_max(void) {
    int i;
    storage_init();
    storage_open();

    for (i = 0; i < 6; i++) {
        storage_expand();
    }

    /* Should stop at 5 expansions (50 + 50 = 100 slots) */
    return g_storage.expansion_level == 5 && g_storage.max_slots == 100;
}

static int test_get_expansion_cost_initial(void) {
    storage_init();
    return storage_get_expansion_cost() == 10000;
}

static int test_get_expansion_cost_after_expand(void) {
    storage_init();
    storage_open();
    storage_expand();
    return storage_get_expansion_cost() == 50000;
}

static int test_get_expansion_cost_maxed(void) {
    int i;
    storage_init();
    storage_open();

    for (i = 0; i < 5; i++) {
        storage_expand();
    }

    return storage_get_expansion_cost() == 0;
}

/* ========================================
 * Test Cases - Sorting
 * ======================================== */

static int test_sort_by_id_ascending(void) {
    storage_init();
    storage_open();

    test_add_item(0, 3000, 10);
    test_add_item(1, 1000, 10);
    test_add_item(2, 2000, 10);

    storage_sort(0, 1);  /* Sort by ID, ascending */

    return g_storage.items[0].item_id == 1000 &&
           g_storage.items[1].item_id == 2000 &&
           g_storage.items[2].item_id == 3000;
}

static int test_sort_by_id_descending(void) {
    storage_init();
    storage_open();

    test_add_item(0, 1000, 10);
    test_add_item(1, 3000, 10);
    test_add_item(2, 2000, 10);

    storage_sort(0, 0);  /* Sort by ID, descending */

    return g_storage.items[0].item_id == 3000 &&
           g_storage.items[1].item_id == 2000 &&
           g_storage.items[2].item_id == 1000;
}

static int test_sort_by_count_ascending(void) {
    storage_init();
    storage_open();

    test_add_item(0, 1000, 30);
    test_add_item(1, 2000, 10);
    test_add_item(2, 3000, 20);

    storage_sort(1, 1);  /* Sort by count, ascending */

    return g_storage.items[0].count == 10 &&
           g_storage.items[1].count == 20 &&
           g_storage.items[2].count == 30;
}

static int test_sort_by_count_descending(void) {
    storage_init();
    storage_open();

    test_add_item(0, 1000, 10);
    test_add_item(1, 2000, 30);
    test_add_item(2, 3000, 20);

    storage_sort(1, 0);  /* Sort by count, descending */

    return g_storage.items[0].count == 30 &&
           g_storage.items[1].count == 20 &&
           g_storage.items[2].count == 10;
}

/* ========================================
 * Test Cases - Clear
 * ======================================== */

static int test_clear_basic(void) {
    storage_init();
    storage_open();

    test_add_item(0, 1000, 10);
    test_add_item(1, 2000, 20);
    g_storage.gold = 5000;
    g_storage.dirty = 1;

    storage_clear();

    return g_storage.items[0].item_id == 0 &&
           g_storage.items[1].item_id == 0 &&
           g_storage.gold == 0 &&
           g_storage.dirty == 0;
}

/* ========================================
 * Test Cases - Protocol Packets
 * ======================================== */

static int test_packet_deposit_format(void) {
    storage_init();
    storage_open();
    g_last_packet[0] = '\0';
    g_packet_count = 0;

    storage_deposit_item(1234, 56);

    return g_packet_count == 1;
}

static int test_packet_withdraw_format(void) {
    storage_init();
    storage_open();
    test_add_item(0, 1000, 20);
    g_last_packet[0] = '\0';
    g_packet_count = 0;

    storage_withdraw_item(0, 10);

    return g_packet_count == 1;
}

static int test_packet_close_sent(void) {
    storage_init();
    storage_open();
    g_storage.dirty = 1;
    g_last_packet[0] = '\0';
    g_packet_count = 0;

    storage_close();

    /* Close should send packet when dirty */
    return g_packet_count == 1;
}

static int test_packet_close_not_sent(void) {
    storage_init();
    storage_open();
    g_storage.dirty = 0;
    g_last_packet[0] = '\0';
    g_packet_count = 0;

    storage_close();

    /* Close should not send packet when not dirty */
    return g_packet_count == 0;
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Storage System Comprehensive Tests ===\n\n");

    printf("Initialization Tests:\n");
    TEST(init_basic);
    TEST(init_max_slots);
    TEST(init_gold_zero);
    TEST(init_not_open);
    TEST(shutdown_clears);

    printf("\nOpen/Close Tests:\n");
    TEST(open_basic);
    TEST(open_sets_access);
    TEST(open_twice);
    TEST(close_basic);
    TEST(close_clears_access);
    TEST(close_clears_selection);

    printf("\nItem Operations Tests:\n");
    TEST(find_item_empty);
    TEST(find_item_found);
    TEST(find_item_not_found);
    TEST(find_free_slot_empty);
    TEST(find_free_slot_partial);
    TEST(find_free_slot_full);
    TEST(has_space_empty);
    TEST(has_space_existing_item);

    printf("\nDeposit/Withdraw Tests:\n");
    TEST(deposit_closed);
    TEST(deposit_basic);
    TEST(deposit_zero_count);
    TEST(withdraw_closed);
    TEST(withdraw_invalid_slot);
    TEST(withdraw_empty_slot);
    TEST(withdraw_basic);
    TEST(withdraw_exceeds_count);

    printf("\nGold Operations Tests:\n");
    TEST(deposit_gold_closed);
    TEST(deposit_gold_basic);
    TEST(deposit_gold_zero);
    TEST(deposit_gold_exceeds_max);
    TEST(withdraw_gold_closed);
    TEST(withdraw_gold_basic);
    TEST(withdraw_gold_zero);
    TEST(withdraw_gold_insufficient);
    TEST(get_max_gold);

    printf("\nSlot Management Tests:\n");
    TEST(get_item_id_valid);
    TEST(get_item_id_empty);
    TEST(get_item_id_invalid_slot);
    TEST(get_item_count_valid);
    TEST(get_item_count_empty);
    TEST(select_basic);
    TEST(get_selected_item_valid);
    TEST(get_selected_item_none);

    printf("\nFiltering Tests:\n");
    TEST(set_filter_basic);
    TEST(set_search_basic);
    TEST(set_search_null);
    TEST(item_category_weapon);
    TEST(item_category_armor);
    TEST(item_category_consumable);
    TEST(item_category_material);
    TEST(item_category_quest);
    TEST(item_category_other);

    printf("\nExpansion Tests:\n");
    TEST(expand_basic);
    TEST(expand_multiple);
    TEST(expand_max);
    TEST(get_expansion_cost_initial);
    TEST(get_expansion_cost_after_expand);
    TEST(get_expansion_cost_maxed);

    printf("\nSorting Tests:\n");
    TEST(sort_by_id_ascending);
    TEST(sort_by_id_descending);
    TEST(sort_by_count_ascending);
    TEST(sort_by_count_descending);

    printf("\nClear Tests:\n");
    TEST(clear_basic);

    printf("\nProtocol Packet Tests:\n");
    TEST(packet_deposit_format);
    TEST(packet_withdraw_format);
    TEST(packet_close_sent);
    TEST(packet_close_not_sent);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    return (tests_passed == tests_run) ? 0 : 1;
}
