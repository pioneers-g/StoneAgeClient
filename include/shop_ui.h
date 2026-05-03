/*
 * Stone Age Client - Shop UI System Header
 * Reverse engineered from FUN_00416be0
 *
 * Shop interface with scrolling, buy/sell mode, item details
 */

#ifndef SHOP_UI_H
#define SHOP_UI_H

#include "types.h"
#include "shop.h"  /* For ShopDetailedItem */

/* Maximum items displayable in shop UI at once */
#define MAX_SHOP_UI_ITEMS       20
#define MAX_SHOP_DESC_LINES     6
#define MAX_SHOP_SKILLS         7

/* Shop UI context */
typedef struct {
    void* window;                   /* Main shop window handle */
    void* tooltip_window;           /* Tooltip/detail window */
    u32  ui_elements[20];           /* UI element IDs */
    int  is_initialized;
    int  sell_mode;                 /* 0 = buy mode, 1 = sell mode */
    int  scroll_pos;                /* Current scroll position */
    u32  selected_index;            /* Currently selected item */
    u32  player_gold;               /* Player's current gold */
    u32  pending_gold;              /* Gold pending transaction */
} ShopUIContext;

/* Shop UI functions */
int shop_ui_init(void);
void shop_ui_shutdown(void);
void shop_ui_update(void);
void shop_ui_render(void);
void shop_ui_handle_click(int x, int y);
void shop_ui_process_packet(const char* data, u32 size);

/* Item management */
void shop_ui_clear_items(void);
void shop_ui_add_item(const ShopDetailedItem* item);
ShopDetailedItem* shop_ui_get_item(u32 index);

/* Scroll management */
void shop_ui_scroll_up(void);
void shop_ui_scroll_down(void);
void shop_ui_set_scroll(int pos);

/* Mode toggle */
void shop_ui_toggle_mode(void);
int shop_ui_get_mode(void);

/* Transaction */
int shop_ui_buy_selected(void);
int shop_ui_confirm_transaction(void);

#endif /* SHOP_UI_H */
