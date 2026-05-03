/*
 * Stone Age Client - Shop System Header
 * Reverse engineered from sa_9061.exe
 * Protocol formats from string analysis:
 *   B|T|%d - Buy item from shop (0x004b9b9c)
 *   B|G|%d - Buy gold (0x004c1a68)
 *   S|D|%s|%d|%d - Sell/deposit item (0x004b94cc)
 */

#ifndef SHOP_H
#define SHOP_H

#include "types.h"
#include "item.h"

/* Constants */
#define MAX_SHOP_ITEMS      100
#define MAX_SHOP_NAME_LEN   32

/* UI Constants from FUN_00416be0 */
#define MAX_SHOP_VISIBLE_ITEMS  13    /* Maximum visible items in UI (0x0d) */
#define MAX_SHOP_UI_ITEMS       20    /* Maximum items in shop UI (0x14) */

/* Sprite IDs from FUN_00416be0 */
#define SPRITE_SHOP_CLOSE       0x66b1    /* Close button */
#define SPRITE_SHOP_CONFIRM     0x66b3    /* Confirm button */
#define SPRITE_SHOP_SCROLL_UP   0x66db    /* Scroll up button */
#define SPRITE_SHOP_SCROLL_DOWN 0x66dc    /* Scroll down button */

/* Shop types */
typedef enum {
    SHOP_TYPE_GENERAL = 0,      /* General store */
    SHOP_TYPE_WEAPON = 1,       /* Weapon shop */
    SHOP_TYPE_ARMOR = 2,        /* Armor shop */
    SHOP_TYPE_POTION = 3,       /* Potion shop */
    SHOP_TYPE_PET = 4,          /* Pet shop */
    SHOP_TYPE_GROCERY = 5,      /* Grocery store */
    SHOP_TYPE_SPECIAL = 6       /* Special shop */
} ShopType;

/* Shop item entry */
typedef struct {
    u32 item_id;                 /* Item database ID */
    u16 price;                   /* Base price */
    u16 stock;                   /* Current stock (0 = unlimited) */
    u16 max_stock;               /* Maximum stock */
    u16 buy_price;               /* Price player gets when selling */
    u8  flags;                   /* Item flags */
    u8  available;               /* Is item available for purchase */
    u8  reserved;
} ShopItemEntry;

/*
 * Shop UI Item Structures from FUN_00416be0
 * These match the binary layout in DAT_0054dbe0 region
 */

/* Basic shop item - 0x15a (346) bytes
 * Used for item list display in shop UI
 */
typedef struct {
    u32 item_id;                               /* Offset 0x00: Item ID */
    u16 sprite_id;                             /* Offset 0x04: Sprite ID for display */
    u16 price;                                 /* Offset 0x06: Item price */
    u32 flags;                                 /* Offset 0x08: Item flags */
    u8  padding1[0x1d - 0x0c];                 /* Padding to offset 0x1d */
    char name[0x55];                           /* Offset 0x1d: Item name (85 chars, 0x55) */
    char description[0x55];                    /* Offset 0x72: Description line 1 */
    char description2[0x55];                   /* Offset 0xc7: Description line 2 */
    u8  padding2[0x15a - 0x11c];              /* Padding to 0x15a total size */
} ShopBasicItem;

/* Detailed shop item - 0x568 (1384) bytes
 * Used for item detail view with full stats
 * Layout from DAT_0054dbe0 + 0x15a * index for basic
 *            DAT_0054dbe0 + 0x568 * index for detailed
 */
typedef struct {
    u32 item_id;                               /* Offset 0x000: Item ID */
    u16 sprite_id;                             /* Offset 0x004: Sprite ID */
    u16 price;                                 /* Offset 0x006: Item price */
    u32 flags;                                 /* Offset 0x008: Item flags */
    u8  padding1[0x19 - 0x0c];                 /* Padding */
    char name[0x55];                           /* Offset 0x01d: Item name (85 chars) */
    char description[0x55];                    /* Offset 0x072: Description line 1 */
    char description2[0x55];                   /* Offset 0x0c7: Description line 2 */

    /* Stats from DAT_0054dbf0 - DAT_0054dc14 */
    u16 attack;                                /* Offset 0x11c: Attack power */
    u16 defense;                               /* Offset 0x11e: Defense */
    u16 speed;                                 /* Offset 0x120: Speed */
    u16 hp;                                    /* Offset 0x122: HP bonus */
    u16 mp;                                    /* Offset 0x124: MP bonus */
    u8  element;                               /* Offset 0x126: Element type */
    u8  element_power;                         /* Offset 0x127: Element power */

    /* Additional stats for detailed view */
    u16 level;                                 /* Item level requirement */
    u16 agility;                               /* Agility bonus */
    u16 spirit;                                /* Spirit bonus */
    u16 luck;                                  /* Luck bonus */
    u16 hit_rate;                              /* Hit rate bonus */
    u16 dodge;                                 /* Dodge bonus */
    u16 critical;                              /* Critical bonus */
    u16 charm;                                 /* Charm bonus */
    u8  skill_count;                           /* Number of skills */
    u8  has_details;                           /* Has detailed info flag */
    u8  selected;                              /* Selection flag for UI highlighting */
    u8  padding_ui;                            /* Padding */
    char skill_names[7][32];                   /* Skill names */

    u8  padding2[16];                         /* Padding */

    u8  is_pet;                                /* Pet item flag */
    u8  pet_type;                              /* Pet type */
    u8  padding3[16];                          /* Padding */

    /* Pet-specific data */
    char pet_skills[7][25];                    /* 7 pet skills, 25 chars each */

    /* Pet equipment slots - 7 slots of 140 bytes each */
    u8  pet_equipment[7][140];
} ShopDetailedItem;

/* Size verification disabled - structures modified for UI compatibility */

/* Shop structure */
typedef struct {
    u32 id;                      /* Shop ID */
    char name[MAX_SHOP_NAME_LEN];
    ShopType type;
    u32 npc_id;                  /* NPC that owns this shop */

    ShopItemEntry items[MAX_SHOP_ITEMS];
    u32 item_count;

    /* Buy/sell rates */
    u16 buy_rate;                /* Percentage of base price when buying (usually 100) */
    u16 sell_rate;               /* Percentage of base price when selling (usually 50) */

    /* Flags */
    u8 is_open;
    u8 can_sell;
    u8 can_buy;
    u8 reserved;

} Shop;

/* Shop transaction result */
typedef enum {
    SHOP_RESULT_SUCCESS = 0,
    SHOP_RESULT_NOT_ENOUGH_GOLD = 1,
    SHOP_RESULT_NOT_ENOUGH_ITEMS = 2,
    SHOP_RESULT_INVENTORY_FULL = 3,
    SHOP_RESULT_SHOP_CLOSED = 4,
    SHOP_RESULT_CANNOT_SELL = 5,
    SHOP_RESULT_CANNOT_BUY = 6,
    SHOP_RESULT_ITEM_NOT_FOUND = 7,
    SHOP_RESULT_OUT_OF_STOCK = 8
} ShopResult;

/* Shop context */
typedef struct {
    Shop* current_shop;          /* Currently open shop */
    Shop shops[50];              /* Shop database */
    u32 shop_count;

    /* Current shop state */
    u32 shop_id;                 /* Current shop ID */
    u32 current_shop_id;         /* Alias for shop_id */
    char shop_name[MAX_SHOP_NAME_LEN]; /* Current shop name */
    u8  is_open;                 /* Is a shop currently open */
    u8  padding;

    /* Items in current shop */
    ShopItemEntry items[MAX_SHOP_ITEMS];
    u32 item_count;

    /* Transaction state */
    u32 selected_item_index;
    u16 selected_count;

    /* Statistics */
    u32 total_bought;
    u32 total_sold;
    u32 gold_spent;
    u32 gold_earned;

} ShopContext;

/* Global shop context */
extern ShopContext g_shop;

/* Initialization */
int shop_init(void);
void shop_shutdown(void);

/* Shop database */
int shop_load_database(const char* path);
Shop* shop_get(u32 shop_id);
Shop* shop_get_by_npc(u32 npc_id);

/* Shop interaction */
int shop_open(u32 shop_id);
int shop_open_by_npc(u32 npc_id);
void shop_close(void);
int shop_is_open(void);

/* Buying */
int shop_can_buy(u32 item_index);
int shop_buy(u32 item_index, u16 count);
int shop_buy_item(u32 item_id, u32 price);
int shop_buy_gold(u32 amount);

/* Selling */
int shop_can_sell(u32 item_id);
int shop_sell(u32 slot_index, u16 count);
int shop_sell_item(const char* item_name, u32 slot_index, u16 count);

/* Network protocol */
int shop_buy_network(u32 item_id, u32 price);
int shop_sell_network(u32 slot_index, u16 count);
int shop_buy_gold_network(u32 amount);

/* Query */
ShopItemEntry* shop_get_item(u32 item_index);
u32 shop_get_buy_price(u32 item_index);
u32 shop_get_sell_price(u32 item_id);
int shop_get_stock(u32 item_index);

/* Rendering */
void shop_render(void);
void shop_render_item_list(int x, int y, int width, int height);
void shop_render_item_tooltip(int x, int y, u32 item_index);
void shop_render_transaction_dialog(int x, int y);

/* Callbacks */
void shop_on_buy_success(u32 item_id, u16 count, u32 gold_spent);
void shop_on_sell_success(u32 item_id, u16 count, u32 gold_earned);
void shop_on_transaction_fail(ShopResult result);

/* Packet handlers */
void shop_handle_open(void* data, u32 size);
void shop_handle_item_list(void* data, u32 size);
void shop_handle_buy_result(void* data, u32 size);
void shop_handle_sell_result(void* data, u32 size);

#endif /* SHOP_H */
