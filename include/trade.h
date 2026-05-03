/*
 * Stone Age Client - Trade System Header
 * Reverse engineered from sa_9061.exe (FUN_0045a9a0, FUN_0045bb00)
 */

#ifndef TRADE_H
#define TRADE_H

#include "types.h"

/* Constants */
#define MAX_TRADE_ITEMS  12

/* Trade bit flags - from DAT_045e8ce0 */
#define TRADE_BIT_WINDOW    0x400000

/* Trade states - from DAT_04630b22 values */
typedef enum {
    TRADE_STATE_NONE = 0,
    TRADE_STATE_REQUESTING = 1,
    TRADE_STATE_INVITED = 2,
    TRADE_STATE_INVITING = 3,     /* Sent invite, waiting response */
    TRADE_STATE_TRADING = 4,
    TRADE_STATE_CONFIRMING = 5,
    TRADE_STATE_COMPLETE = 6,
    TRADE_STATE_CANCELLED = 7
} TradeState;

/* Trade sub-states - from DAT_04630878 values */
typedef enum {
    TRADE_SUB_NONE = 0,
    TRADE_SUB_INIT = 1,
    TRADE_SUB_PENDING = 2,
    TRADE_SUB_ACTIVE = 3,
    TRADE_SUB_CONFIRMED = 4
} TradeSubState;

/* Trade item structure - from DAT_045e7c50 region */
typedef struct {
    u32 id;
    u32 count;
    char name[64];
    char desc[128];
    u16 icon_id;
    u16 type;
    u32 flags;
} TradeItem;

/* Trade partner data - from DAT_045f1970 region */
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

/* Trade window handle - from DAT_045e19b8, DAT_04583240 */
typedef void* TradeWindowHandle;

/* Trade context - matches DAT_045exxxx region */
typedef struct {
    /* State - DAT_04630b22 */
    TradeState state;
    TradeSubState sub_state;     /* DAT_04630878 */
    u32 request_time;

    /* Partner info - DAT_045f1970 region */
    TradePartnerData partner;

    /* Window handles - DAT_045e19b8, DAT_04583240 */
    TradeWindowHandle my_window;
    TradeWindowHandle partner_window;

    /* Bit flags - DAT_045e8ce0 */
    u32 bit_flags;

    /* Partner info */
    u32 partner_id;
    char partner_name[32];

    /* My items - DAT_045e7c50 region */
    TradeItem my_items[MAX_TRADE_ITEMS];
    int my_item_count;
    u32 my_gold;
    int my_locked;
    int selected_index;

    /* Partner items - DAT_045e8058 region */
    TradeItem partner_items[MAX_TRADE_ITEMS];
    int partner_item_count;
    u32 partner_gold;
    int partner_locked;

    /* Trade window visibility */
    int window_visible;

} TradeContext;

/* Global trade context */
extern TradeContext g_trade;

/* Initialization */
int trade_init(void);
void trade_shutdown(void);

/* Trade request */
int trade_request(u32 player_id);
void trade_handle_request(void* data, u32 size);
int trade_accept(void);
void trade_decline(void);
void trade_cancel(void);
void trade_reset(void);

/* Item management */
int trade_add_item(u32 item_id, u32 count);
int trade_remove_item(int slot);
int trade_set_gold(u32 gold);

/* Lock/confirm */
int trade_lock(void);
int trade_unlock(void);

/* Partner notifications */
void trade_handle_partner_lock(int locked);
void trade_handle_partner_add_item(void* data, u32 size);
void trade_handle_partner_remove_item(void* data, u32 size);
void trade_handle_partner_gold(void* data, u32 size);

/* Complete/cancel */
void trade_handle_complete(void* data, u32 size);
void trade_handle_cancel(void);

/* Query */
TradeState trade_get_state(void);
int trade_is_active(void);
const char* trade_get_partner_name(void);

TradeItem* trade_get_my_item(int slot);
TradeItem* trade_get_partner_item(int slot);

u32 trade_get_my_gold(void);
u32 trade_get_partner_gold(void);

int trade_is_my_locked(void);
int trade_is_partner_locked(void);
int trade_is_ready(void);

int trade_get_my_item_count(void);
int trade_get_partner_item_count(void);

/* Enhanced handling - FUN_0045a9a0 */
void trade_handle_packet(const char* packet_data);

/* Additional query functions */
int trade_is_window_visible(void);
u32 trade_get_bit_flags(void);
u32 trade_get_partner_id(void);

#endif /* TRADE_H */
