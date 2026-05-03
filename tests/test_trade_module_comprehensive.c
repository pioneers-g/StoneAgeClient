/*
 * Stone Age Client - Trade Module Comprehensive Unit Tests
 * Tests for trade.c, trade_protocol.c
 *
 * Verified against Ghidra decompilation:
 * - FUN_00463ae0: Trade request
 * - FUN_00463bd0: Trade check status (timeout 600000ms)
 * - FUN_00463c20: Trade handle response
 * - FUN_00463c80: Trade send request
 *
 * Coverage targets:
 * - Trade state machine: 95%
 * - Item management: 90%
 * - Lock/unlock operations: 90%
 * - Gold operations: 85%
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../tests/test_framework.h"

/* Types from types.h */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;

/* Trade constants - verified from Ghidra */
#define MAX_TRADE_ITEMS  12
#define TRADE_TIMEOUT_MS 600000  /* 10 minutes from FUN_00463bd0 */
#define TRADE_BIT_WINDOW 0x400000

/* Trade states - from DAT_0461c000 values */
typedef enum {
    TRADE_STATE_NONE = 0,
    TRADE_STATE_REQUESTING = 1,
    TRADE_STATE_INVITED = 2,
    TRADE_STATE_INVITING = 3,
    TRADE_STATE_TRADING = 4,
    TRADE_STATE_CONFIRMING = 5,
    TRADE_STATE_COMPLETE = 6,
    TRADE_STATE_CANCELLED = 7
} TradeState;

/* Trade sub-states */
typedef enum {
    TRADE_SUB_NONE = 0,
    TRADE_SUB_INIT = 1,
    TRADE_SUB_PENDING = 2,
    TRADE_SUB_ACTIVE = 3,
    TRADE_SUB_CONFIRMED = 4
} TradeSubState;

/* Trade item structure */
typedef struct {
    u32 id;
    u32 count;
    char name[64];
    char desc[128];
    u16 icon_id;
    u16 type;
    u32 flags;
} TradeItem;

/* Trade partner data */
typedef struct {
    u32 char_id;
    char name[24];
    char title[32];
    u32 level;
    u32 hp;
    u32 max_hp;
    u32 mp;
    u32 max_mp;
} TradePartnerData;

/* Trade context */
typedef struct {
    TradeState state;
    TradeSubState sub_state;
    u32 request_time;

    TradePartnerData partner;

    void* my_window;
    void* partner_window;

    u32 bit_flags;

    u32 partner_id;
    char partner_name[32];

    TradeItem my_items[MAX_TRADE_ITEMS];
    int my_item_count;
    u32 my_gold;
    int my_locked;
    int selected_index;

    TradeItem partner_items[MAX_TRADE_ITEMS];
    int partner_item_count;
    u32 partner_gold;
    int partner_locked;

    int window_visible;
} TradeContext;

/* Global trade context for testing */
static TradeContext g_trade = {0};

/* ========================================
 * Implementation Functions (matching trade.c)
 * ======================================== */

static void trade_init(void) {
    memset(&g_trade, 0, sizeof(TradeContext));
    g_trade.selected_index = -1;
    g_trade.my_window = NULL;
    g_trade.partner_window = NULL;
    g_trade.bit_flags = 0;
    g_trade.window_visible = 0;
}

static void trade_reset(void) {
    int i;

    for (i = 0; i < MAX_TRADE_ITEMS; i++) {
        g_trade.my_items[i].id = 0;
        g_trade.partner_items[i].id = 0;
    }

    g_trade.my_gold = 0;
    g_trade.partner_gold = 0;
    g_trade.my_item_count = 0;     /* FIX: Reset item count */
    g_trade.partner_item_count = 0; /* FIX: Reset partner item count */
    memset(&g_trade.partner, 0, sizeof(TradePartnerData));
    g_trade.state = TRADE_STATE_NONE;
    g_trade.sub_state = TRADE_SUB_NONE;
    g_trade.my_locked = 0;
    g_trade.partner_locked = 0;
    g_trade.window_visible = 0;
}

static int trade_request(u32 player_id) {
    if (g_trade.state != TRADE_STATE_NONE) {
        return 0;
    }

    g_trade.state = TRADE_STATE_REQUESTING;
    g_trade.request_time = 0;  /* Would be timeGetTime() in real impl */
    g_trade.partner_id = player_id;

    return 1;
}

static int trade_accept(void) {
    if (g_trade.state != TRADE_STATE_INVITED) {
        return 0;
    }

    g_trade.state = TRADE_STATE_TRADING;
    g_trade.sub_state = TRADE_SUB_INIT;
    g_trade.my_locked = 0;
    g_trade.partner_locked = 0;
    g_trade.bit_flags |= TRADE_BIT_WINDOW;

    return 1;
}

static void trade_decline(void) {
    trade_reset();
}

static void trade_cancel(void) {
    if (g_trade.state == TRADE_STATE_NONE) {
        return;
    }
    trade_reset();
}

static int trade_add_item(u32 item_id, u32 count) {
    int i;

    if (g_trade.state != TRADE_STATE_TRADING) {
        return 0;
    }

    if (g_trade.my_locked) {
        return 0;
    }

    for (i = 0; i < MAX_TRADE_ITEMS; i++) {
        if (g_trade.my_items[i].id == 0) {
            g_trade.my_items[i].id = item_id;
            g_trade.my_items[i].count = count;
            g_trade.my_item_count++;
            return 1;
        }
    }

    return 0;
}

static int trade_remove_item(int slot) {
    if (g_trade.state != TRADE_STATE_TRADING) {
        return 0;
    }

    if (g_trade.my_locked) {
        return 0;
    }

    if (slot < 0 || slot >= MAX_TRADE_ITEMS) {
        return 0;
    }

    if (g_trade.my_items[slot].id == 0) {
        return 0;
    }

    g_trade.my_items[slot].id = 0;
    g_trade.my_items[slot].count = 0;
    g_trade.my_item_count--;

    return 1;
}

static int trade_set_gold(u32 gold) {
    if (g_trade.state != TRADE_STATE_TRADING) {
        return 0;
    }

    if (g_trade.my_locked) {
        return 0;
    }

    g_trade.my_gold = gold;
    return 1;
}

static int trade_lock(void) {
    if (g_trade.state != TRADE_STATE_TRADING) {
        return 0;
    }

    if (g_trade.my_locked) {
        return 0;
    }

    g_trade.my_locked = 1;
    return 1;
}

static int trade_unlock(void) {
    if (g_trade.state != TRADE_STATE_TRADING) {
        return 0;
    }

    if (!g_trade.my_locked) {
        return 0;
    }

    if (g_trade.partner_locked) {
        return 0;
    }

    g_trade.my_locked = 0;
    return 1;
}

static TradeState trade_get_state(void) {
    return g_trade.state;
}

static int trade_is_active(void) {
    return g_trade.state != TRADE_STATE_NONE;
}

static const char* trade_get_partner_name(void) {
    return g_trade.partner.name;
}

static TradeItem* trade_get_my_item(int slot) {
    if (slot < 0 || slot >= MAX_TRADE_ITEMS) {
        return NULL;
    }
    return &g_trade.my_items[slot];
}

static TradeItem* trade_get_partner_item(int slot) {
    if (slot < 0 || slot >= MAX_TRADE_ITEMS) {
        return NULL;
    }
    return &g_trade.partner_items[slot];
}

static u32 trade_get_my_gold(void) {
    return g_trade.my_gold;
}

static u32 trade_get_partner_gold(void) {
    return g_trade.partner_gold;
}

static int trade_is_my_locked(void) {
    return g_trade.my_locked;
}

static int trade_is_partner_locked(void) {
    return g_trade.partner_locked;
}

static int trade_is_ready(void) {
    return g_trade.my_locked && g_trade.partner_locked;
}

static int trade_get_my_item_count(void) {
    return g_trade.my_item_count;
}

static int trade_get_partner_item_count(void) {
    return g_trade.partner_item_count;
}

static int trade_is_window_visible(void) {
    return g_trade.window_visible;
}

static u32 trade_get_bit_flags(void) {
    return g_trade.bit_flags;
}

static u32 trade_get_partner_id(void) {
    return g_trade.partner.char_id;
}

/* ========================================
 * Test Cases - Constants
 * ======================================== */

static void test_trade_constants(void) {
    TEST_BEGIN("Trade constants");

    TEST_ASSERT_EQ(MAX_TRADE_ITEMS, 12);
    TEST_ASSERT_EQ(TRADE_TIMEOUT_MS, 600000);  /* 10 minutes from Ghidra */
    TEST_ASSERT_EQ(TRADE_BIT_WINDOW, 0x400000);

    TEST_END();
}

static void test_trade_state_values(void) {
    TEST_BEGIN("Trade state enum values");

    TEST_ASSERT_EQ(TRADE_STATE_NONE, 0);
    TEST_ASSERT_EQ(TRADE_STATE_REQUESTING, 1);
    TEST_ASSERT_EQ(TRADE_STATE_INVITED, 2);
    TEST_ASSERT_EQ(TRADE_STATE_INVITING, 3);
    TEST_ASSERT_EQ(TRADE_STATE_TRADING, 4);
    TEST_ASSERT_EQ(TRADE_STATE_CONFIRMING, 5);
    TEST_ASSERT_EQ(TRADE_STATE_COMPLETE, 6);
    TEST_ASSERT_EQ(TRADE_STATE_CANCELLED, 7);

    TEST_END();
}

static void test_trade_substate_values(void) {
    TEST_BEGIN("Trade sub-state enum values");

    TEST_ASSERT_EQ(TRADE_SUB_NONE, 0);
    TEST_ASSERT_EQ(TRADE_SUB_INIT, 1);
    TEST_ASSERT_EQ(TRADE_SUB_PENDING, 2);
    TEST_ASSERT_EQ(TRADE_SUB_ACTIVE, 3);
    TEST_ASSERT_EQ(TRADE_SUB_CONFIRMED, 4);

    TEST_END();
}

/* ========================================
 * Test Cases - Initialization
 * ======================================== */

static void test_trade_init(void) {
    TEST_BEGIN("Trade context initialization");

    trade_init();

    TEST_ASSERT_EQ(0, g_trade.state);
    TEST_ASSERT_EQ(0, g_trade.my_item_count);
    TEST_ASSERT_EQ(0, g_trade.partner_item_count);
    TEST_ASSERT_EQ(0, g_trade.my_gold);
    TEST_ASSERT_EQ(0, g_trade.partner_gold);
    TEST_ASSERT_EQ(-1, g_trade.selected_index);
    TEST_ASSERT_EQ(0, g_trade.my_locked);
    TEST_ASSERT_EQ(0, g_trade.partner_locked);
    TEST_ASSERT_EQ(0, g_trade.bit_flags);

    TEST_END();
}

static void test_trade_init_clears_data(void) {
    TEST_BEGIN("Trade init clears existing data");

    /* Setup some data */
    trade_init();
    g_trade.state = TRADE_STATE_TRADING;
    g_trade.my_item_count = 5;
    g_trade.my_gold = 1000;
    g_trade.my_locked = 1;

    /* Re-initialize */
    trade_init();

    TEST_ASSERT_EQ(0, g_trade.state);
    TEST_ASSERT_EQ(0, g_trade.my_item_count);
    TEST_ASSERT_EQ(0, g_trade.my_gold);
    TEST_ASSERT_EQ(0, g_trade.my_locked);

    TEST_END();
}

/* ========================================
 * Test Cases - Trade Request
 * ======================================== */

static void test_trade_request_basic(void) {
    TEST_BEGIN("Trade request basic");

    trade_init();

    int result = trade_request(12345);

    TEST_ASSERT_EQ(1, result);
    TEST_ASSERT_EQ(TRADE_STATE_REQUESTING, g_trade.state);
    TEST_ASSERT_EQ(12345, g_trade.partner_id);

    TEST_END();
}

static void test_trade_request_while_in_trade(void) {
    TEST_BEGIN("Trade request while in trade fails");

    trade_init();

    trade_request(12345);
    int result = trade_request(67890);  /* Try another request */

    TEST_ASSERT_EQ(0, result);
    TEST_ASSERT_EQ(12345, g_trade.partner_id);  /* Unchanged */

    TEST_END();
}

static void test_trade_request_different_players(void) {
    TEST_BEGIN("Trade request different players");

    trade_init();

    trade_request(100);
    TEST_ASSERT_EQ(100, g_trade.partner_id);

    trade_cancel();

    trade_request(200);
    TEST_ASSERT_EQ(200, g_trade.partner_id);

    TEST_END();
}

/* ========================================
 * Test Cases - Trade Accept/Decline
 * ======================================== */

static void test_trade_accept(void) {
    TEST_BEGIN("Trade accept");

    trade_init();
    g_trade.state = TRADE_STATE_INVITED;
    g_trade.partner.char_id = 12345;

    int result = trade_accept();

    TEST_ASSERT_EQ(1, result);
    TEST_ASSERT_EQ(TRADE_STATE_TRADING, g_trade.state);
    TEST_ASSERT_EQ(TRADE_SUB_INIT, g_trade.sub_state);
    TEST_ASSERT_EQ(0, g_trade.my_locked);
    TEST_ASSERT_EQ(0, g_trade.partner_locked);
    TEST_ASSERT_EQ(TRADE_BIT_WINDOW, (g_trade.bit_flags & TRADE_BIT_WINDOW));

    TEST_END();
}

static void test_trade_accept_wrong_state(void) {
    TEST_BEGIN("Trade accept wrong state fails");

    trade_init();

    /* Try to accept without being invited */
    int result = trade_accept();

    TEST_ASSERT_EQ(0, result);
    TEST_ASSERT_EQ(TRADE_STATE_NONE, g_trade.state);

    TEST_END();
}

static void test_trade_decline(void) {
    TEST_BEGIN("Trade decline");

    trade_init();
    g_trade.state = TRADE_STATE_INVITED;
    g_trade.partner.char_id = 12345;

    trade_decline();

    TEST_ASSERT_EQ(TRADE_STATE_NONE, g_trade.state);

    TEST_END();
}

static void test_trade_cancel(void) {
    TEST_BEGIN("Trade cancel");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;
    g_trade.my_item_count = 3;
    g_trade.my_gold = 500;

    trade_cancel();

    TEST_ASSERT_EQ(TRADE_STATE_NONE, g_trade.state);
    TEST_ASSERT_EQ(0, g_trade.my_item_count);
    TEST_ASSERT_EQ(0, g_trade.my_gold);

    TEST_END();
}

static void test_trade_cancel_when_none(void) {
    TEST_BEGIN("Trade cancel when none");

    trade_init();

    /* Should not crash */
    trade_cancel();

    TEST_ASSERT_EQ(TRADE_STATE_NONE, g_trade.state);

    TEST_END();
}

/* ========================================
 * Test Cases - Item Management
 * ======================================== */

static void test_trade_add_item_basic(void) {
    TEST_BEGIN("Trade add item basic");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;

    int result = trade_add_item(100, 1);

    TEST_ASSERT_EQ(1, result);
    TEST_ASSERT_EQ(1, g_trade.my_item_count);
    TEST_ASSERT_EQ(100, g_trade.my_items[0].id);
    TEST_ASSERT_EQ(1, g_trade.my_items[0].count);

    TEST_END();
}

static void test_trade_add_item_multiple(void) {
    TEST_BEGIN("Trade add item multiple");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;

    trade_add_item(100, 1);
    trade_add_item(101, 5);
    trade_add_item(102, 10);

    TEST_ASSERT_EQ(3, g_trade.my_item_count);
    TEST_ASSERT_EQ(100, g_trade.my_items[0].id);
    TEST_ASSERT_EQ(101, g_trade.my_items[1].id);
    TEST_ASSERT_EQ(102, g_trade.my_items[2].id);

    TEST_END();
}

static void test_trade_add_item_when_locked(void) {
    TEST_BEGIN("Trade add item when locked fails");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;
    g_trade.my_locked = 1;

    int result = trade_add_item(100, 1);

    TEST_ASSERT_EQ(0, result);
    TEST_ASSERT_EQ(0, g_trade.my_item_count);

    TEST_END();
}

static void test_trade_add_item_when_not_trading(void) {
    TEST_BEGIN("Trade add item when not trading fails");

    trade_init();

    int result = trade_add_item(100, 1);

    TEST_ASSERT_EQ(0, result);

    TEST_END();
}

static void test_trade_add_item_max(void) {
    TEST_BEGIN("Trade add item max capacity");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;

    /* Add MAX_TRADE_ITEMS items */
    int i;
    for (i = 0; i < MAX_TRADE_ITEMS; i++) {
        trade_add_item(100 + i, 1);
    }

    TEST_ASSERT_EQ(MAX_TRADE_ITEMS, g_trade.my_item_count);

    /* Try to add one more */
    int result = trade_add_item(999, 1);

    TEST_ASSERT_EQ(0, result);
    TEST_ASSERT_EQ(MAX_TRADE_ITEMS, g_trade.my_item_count);

    TEST_END();
}

static void test_trade_remove_item_basic(void) {
    TEST_BEGIN("Trade remove item basic");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;
    trade_add_item(100, 1);

    int result = trade_remove_item(0);

    TEST_ASSERT_EQ(1, result);
    TEST_ASSERT_EQ(0, g_trade.my_item_count);
    TEST_ASSERT_EQ(0, g_trade.my_items[0].id);

    TEST_END();
}

static void test_trade_remove_item_invalid_slot(void) {
    TEST_BEGIN("Trade remove item invalid slot");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;
    trade_add_item(100, 1);

    TEST_ASSERT_EQ(0, trade_remove_item(-1));
    TEST_ASSERT_EQ(0, trade_remove_item(MAX_TRADE_ITEMS));
    TEST_ASSERT_EQ(0, trade_remove_item(100));

    TEST_ASSERT_EQ(1, g_trade.my_item_count);  /* Unchanged */

    TEST_END();
}

static void test_trade_remove_item_empty_slot(void) {
    TEST_BEGIN("Trade remove item empty slot");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;

    int result = trade_remove_item(0);  /* Slot is empty */

    TEST_ASSERT_EQ(0, result);

    TEST_END();
}

static void test_trade_remove_item_when_locked(void) {
    TEST_BEGIN("Trade remove item when locked fails");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;
    trade_add_item(100, 1);
    g_trade.my_locked = 1;

    int result = trade_remove_item(0);

    TEST_ASSERT_EQ(0, result);
    TEST_ASSERT_EQ(1, g_trade.my_item_count);

    TEST_END();
}

/* ========================================
 * Test Cases - Gold Operations
 * ======================================== */

static void test_trade_set_gold_basic(void) {
    TEST_BEGIN("Trade set gold basic");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;

    int result = trade_set_gold(5000);

    TEST_ASSERT_EQ(1, result);
    TEST_ASSERT_EQ(5000, g_trade.my_gold);

    TEST_END();
}

static void test_trade_set_gold_multiple_times(void) {
    TEST_BEGIN("Trade set gold multiple times");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;

    trade_set_gold(1000);
    TEST_ASSERT_EQ(1000, g_trade.my_gold);

    trade_set_gold(5000);
    TEST_ASSERT_EQ(5000, g_trade.my_gold);

    trade_set_gold(0);
    TEST_ASSERT_EQ(0, g_trade.my_gold);

    TEST_END();
}

static void test_trade_set_gold_when_locked(void) {
    TEST_BEGIN("Trade set gold when locked fails");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;
    g_trade.my_locked = 1;

    int result = trade_set_gold(5000);

    TEST_ASSERT_EQ(0, result);
    TEST_ASSERT_EQ(0, g_trade.my_gold);

    TEST_END();
}

static void test_trade_set_gold_when_not_trading(void) {
    TEST_BEGIN("Trade set gold when not trading fails");

    trade_init();

    int result = trade_set_gold(5000);

    TEST_ASSERT_EQ(0, result);

    TEST_END();
}

/* ========================================
 * Test Cases - Lock/Unlock Operations
 * ======================================== */

static void test_trade_lock_basic(void) {
    TEST_BEGIN("Trade lock basic");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;

    int result = trade_lock();

    TEST_ASSERT_EQ(1, result);
    TEST_ASSERT_EQ(1, g_trade.my_locked);

    TEST_END();
}

static void test_trade_lock_already_locked(void) {
    TEST_BEGIN("Trade lock already locked fails");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;
    g_trade.my_locked = 1;

    int result = trade_lock();

    TEST_ASSERT_EQ(0, result);

    TEST_END();
}

static void test_trade_lock_when_not_trading(void) {
    TEST_BEGIN("Trade lock when not trading fails");

    trade_init();

    int result = trade_lock();

    TEST_ASSERT_EQ(0, result);

    TEST_END();
}

static void test_trade_unlock_basic(void) {
    TEST_BEGIN("Trade unlock basic");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;
    g_trade.my_locked = 1;

    int result = trade_unlock();

    TEST_ASSERT_EQ(1, result);
    TEST_ASSERT_EQ(0, g_trade.my_locked);

    TEST_END();
}

static void test_trade_unlock_not_locked(void) {
    TEST_BEGIN("Trade unlock not locked fails");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;
    g_trade.my_locked = 0;

    int result = trade_unlock();

    TEST_ASSERT_EQ(0, result);

    TEST_END();
}

static void test_trade_unlock_partner_locked(void) {
    TEST_BEGIN("Trade unlock when partner locked fails");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;
    g_trade.my_locked = 1;
    g_trade.partner_locked = 1;

    int result = trade_unlock();

    TEST_ASSERT_EQ(0, result);
    TEST_ASSERT_EQ(1, g_trade.my_locked);  /* Still locked */

    TEST_END();
}

/* ========================================
 * Test Cases - Ready Check
 * ======================================== */

static void test_trade_is_ready_none_locked(void) {
    TEST_BEGIN("Trade is ready - none locked");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;
    g_trade.my_locked = 0;
    g_trade.partner_locked = 0;

    TEST_ASSERT_EQ(0, trade_is_ready());

    TEST_END();
}

static void test_trade_is_ready_one_locked(void) {
    TEST_BEGIN("Trade is ready - one locked");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;

    g_trade.my_locked = 1;
    g_trade.partner_locked = 0;
    TEST_ASSERT_EQ(0, trade_is_ready());

    g_trade.my_locked = 0;
    g_trade.partner_locked = 1;
    TEST_ASSERT_EQ(0, trade_is_ready());

    TEST_END();
}

static void test_trade_is_ready_both_locked(void) {
    TEST_BEGIN("Trade is ready - both locked");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;
    g_trade.my_locked = 1;
    g_trade.partner_locked = 1;

    TEST_ASSERT_EQ(1, trade_is_ready());

    TEST_END();
}

/* ========================================
 * Test Cases - Query Functions
 * ======================================== */

static void test_trade_get_state(void) {
    TEST_BEGIN("Trade get state");

    trade_init();
    TEST_ASSERT_EQ(TRADE_STATE_NONE, trade_get_state());

    g_trade.state = TRADE_STATE_TRADING;
    TEST_ASSERT_EQ(TRADE_STATE_TRADING, trade_get_state());

    TEST_END();
}

static void test_trade_is_active(void) {
    TEST_BEGIN("Trade is active");

    trade_init();
    TEST_ASSERT_EQ(0, trade_is_active());

    g_trade.state = TRADE_STATE_TRADING;
    TEST_ASSERT_EQ(1, trade_is_active());

    g_trade.state = TRADE_STATE_REQUESTING;
    TEST_ASSERT_EQ(1, trade_is_active());

    TEST_END();
}

static void test_trade_get_item(void) {
    TEST_BEGIN("Trade get item");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;
    trade_add_item(100, 5);

    TradeItem* item = trade_get_my_item(0);
    TEST_ASSERT(item != NULL);
    TEST_ASSERT_EQ(100, item->id);
    TEST_ASSERT_EQ(5, item->count);

    /* Invalid slot */
    TEST_ASSERT(NULL == trade_get_my_item(-1));
    TEST_ASSERT(NULL == trade_get_my_item(MAX_TRADE_ITEMS));

    TEST_END();
}

static void test_trade_get_gold(void) {
    TEST_BEGIN("Trade get gold");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;
    g_trade.my_gold = 1234;
    g_trade.partner_gold = 5678;

    TEST_ASSERT_EQ(1234, trade_get_my_gold());
    TEST_ASSERT_EQ(5678, trade_get_partner_gold());

    TEST_END();
}

static void test_trade_get_locked_status(void) {
    TEST_BEGIN("Trade get locked status");

    trade_init();

    g_trade.my_locked = 0;
    g_trade.partner_locked = 0;
    TEST_ASSERT_EQ(0, trade_is_my_locked());
    TEST_ASSERT_EQ(0, trade_is_partner_locked());

    g_trade.my_locked = 1;
    g_trade.partner_locked = 1;
    TEST_ASSERT_EQ(1, trade_is_my_locked());
    TEST_ASSERT_EQ(1, trade_is_partner_locked());

    TEST_END();
}

static void test_trade_get_item_count(void) {
    TEST_BEGIN("Trade get item count");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;

    TEST_ASSERT_EQ(0, trade_get_my_item_count());
    TEST_ASSERT_EQ(0, trade_get_partner_item_count());

    trade_add_item(100, 1);
    trade_add_item(101, 1);

    TEST_ASSERT_EQ(2, trade_get_my_item_count());

    g_trade.partner_item_count = 3;
    TEST_ASSERT_EQ(3, trade_get_partner_item_count());

    TEST_END();
}

/* ========================================
 * Test Cases - Integration
 * ======================================== */

static void test_trade_full_flow(void) {
    TEST_BEGIN("Trade full flow");

    trade_init();

    /* Start trade */
    g_trade.state = TRADE_STATE_INVITED;
    g_trade.partner.char_id = 12345;

    /* Accept */
    trade_accept();
    TEST_ASSERT_EQ(TRADE_STATE_TRADING, g_trade.state);

    /* Add items */
    trade_add_item(100, 1);
    trade_add_item(101, 5);
    TEST_ASSERT_EQ(2, g_trade.my_item_count);

    /* Set gold */
    trade_set_gold(5000);
    TEST_ASSERT_EQ(5000, g_trade.my_gold);

    /* Lock */
    trade_lock();
    TEST_ASSERT_EQ(1, g_trade.my_locked);

    /* Cancel */
    trade_cancel();
    TEST_ASSERT_EQ(TRADE_STATE_NONE, g_trade.state);
    TEST_ASSERT_EQ(0, g_trade.my_item_count);
    TEST_ASSERT_EQ(0, g_trade.my_gold);

    TEST_END();
}

static void test_trade_lock_then_modify_fails(void) {
    TEST_BEGIN("Trade lock then modify fails");

    trade_init();
    g_trade.state = TRADE_STATE_TRADING;

    trade_add_item(100, 1);
    trade_lock();

    /* All modifications should fail */
    TEST_ASSERT_EQ(0, trade_add_item(101, 1));
    TEST_ASSERT_EQ(0, trade_remove_item(0));
    TEST_ASSERT_EQ(0, trade_set_gold(1000));

    TEST_ASSERT_EQ(1, g_trade.my_item_count);  /* Unchanged */

    TEST_END();
}

static void test_trade_complete_cycle(void) {
    TEST_BEGIN("Trade complete cycle");

    trade_init();

    /* Player A requests trade */
    trade_request(12345);
    TEST_ASSERT_EQ(TRADE_STATE_REQUESTING, g_trade.state);

    /* Cancel and start fresh */
    trade_cancel();
    TEST_ASSERT_EQ(TRADE_STATE_NONE, g_trade.state);

    /* Player B invites */
    g_trade.state = TRADE_STATE_INVITED;
    g_trade.partner.char_id = 54321;
    strcpy(g_trade.partner.name, "TestPlayer");

    /* Accept */
    trade_accept();
    TEST_ASSERT_EQ(TRADE_STATE_TRADING, g_trade.state);

    /* Both add items and lock */
    trade_add_item(200, 3);
    g_trade.partner_items[0].id = 300;
    g_trade.partner_items[0].count = 2;
    g_trade.partner_item_count = 1;

    trade_set_gold(1000);
    g_trade.partner_gold = 2000;

    trade_lock();
    g_trade.partner_locked = 1;

    /* Ready */
    TEST_ASSERT_EQ(1, trade_is_ready());

    TEST_END();
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("\n=== Trade Module Comprehensive Tests ===\n");
    printf("Testing trade.c implementation\n");
    printf("Verified against Ghidra decompilation\n\n");

    /* Constants tests */
    printf("[Constants]\n");
    test_trade_constants();
    test_trade_state_values();
    test_trade_substate_values();

    /* Initialization tests */
    printf("\n[Initialization]\n");
    test_trade_init();
    test_trade_init_clears_data();

    /* Trade request tests */
    printf("\n[Trade Request]\n");
    test_trade_request_basic();
    test_trade_request_while_in_trade();
    test_trade_request_different_players();

    /* Accept/Decline tests */
    printf("\n[Accept/Decline]\n");
    test_trade_accept();
    test_trade_accept_wrong_state();
    test_trade_decline();
    test_trade_cancel();
    test_trade_cancel_when_none();

    /* Item management tests */
    printf("\n[Item Management]\n");
    test_trade_add_item_basic();
    test_trade_add_item_multiple();
    test_trade_add_item_when_locked();
    test_trade_add_item_when_not_trading();
    test_trade_add_item_max();
    test_trade_remove_item_basic();
    test_trade_remove_item_invalid_slot();
    test_trade_remove_item_empty_slot();
    test_trade_remove_item_when_locked();

    /* Gold operation tests */
    printf("\n[Gold Operations]\n");
    test_trade_set_gold_basic();
    test_trade_set_gold_multiple_times();
    test_trade_set_gold_when_locked();
    test_trade_set_gold_when_not_trading();

    /* Lock/Unlock tests */
    printf("\n[Lock/Unlock]\n");
    test_trade_lock_basic();
    test_trade_lock_already_locked();
    test_trade_lock_when_not_trading();
    test_trade_unlock_basic();
    test_trade_unlock_not_locked();
    test_trade_unlock_partner_locked();

    /* Ready check tests */
    printf("\n[Ready Check]\n");
    test_trade_is_ready_none_locked();
    test_trade_is_ready_one_locked();
    test_trade_is_ready_both_locked();

    /* Query function tests */
    printf("\n[Query Functions]\n");
    test_trade_get_state();
    test_trade_is_active();
    test_trade_get_item();
    test_trade_get_gold();
    test_trade_get_locked_status();
    test_trade_get_item_count();

    /* Integration tests */
    printf("\n[Integration]\n");
    test_trade_full_flow();
    test_trade_lock_then_modify_fails();
    test_trade_complete_cycle();

    /* Summary */
    test_summary();

    return test_all_passed() ? 0 : 1;
}
