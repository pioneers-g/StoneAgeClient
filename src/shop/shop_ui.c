/*
 * Stone Age Client - Shop UI Implementation
 * Reverse engineered from sa_9061.exe (FUN_00416be0)
 *
 * Shop UI rendering, interaction, item display
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "shop.h"
#include "shop_ui.h"
#include "shop_protocol.h"
#include "item.h"
#include "network.h"
#include "render.h"
#include "logger.h"

/* External references */
extern u32 g_mouse_state;
extern s32 g_hover_sprite;

/* Shop UI context - from FUN_00416be0 analysis */
static ShopUIContext g_shop_ui = {0};

/* Shop detailed item data - from DAT_0054dbe0 region */
/* Each entry is 0x15a (346) bytes for basic, 0x568 (1384) bytes for detailed */
static ShopDetailedItem g_shop_items[MAX_SHOP_UI_ITEMS];
u32 g_shop_item_count = 0;

/* UI element IDs for clickable areas - DAT_0054cce8 through DAT_0054ccf8 */
static u32 s_ui_buttons[5] = {0};

/* Scroll position - DAT_0054ccfc */
static int s_scroll_pos = 0;

/* Selected item index - DAT_0054cd10 */
static u32 s_selected_item = 0;

/* Buy/sell mode toggle - DAT_00564e58 */
static int s_sell_mode = 0;

/* Tooltip window - DAT_00564e48 */
static void* s_tooltip_window = NULL;

/* Pending item for detail view - DAT_004aa044 */
static int s_pending_item_index = -1;

/* Forward declarations */
static void shop_ui_render_item_details(int x, int y, ShopDetailedItem* item);
static void shop_ui_handle_item_click(int x, int y);

/*
 * Initialize shop UI - FUN_00416be0 initialization block
 */
int shop_ui_init(void) {
    u32 empty_slots;
    u32 pet_slots;

    if (g_shop_ui.is_initialized) {
        return 1;
    }

    /* Create shop window - FUN_00448610 pattern */
    /* Window dimensions: 0x269 x 0x195 (617 x 405) */
    g_shop_ui.window = NULL;  /* Would call window_create() */

    /* Clear shop item data - DAT_0054dbe0 region (0x1b08 bytes) */
    memset(g_shop_items, 0, sizeof(g_shop_items));
    g_shop_item_count = 0;

    /* Clear UI element array - DAT_0054c88c region */
    memset(g_shop_ui.ui_elements, 0, sizeof(g_shop_ui.ui_elements));

    /* Initialize state */
    s_scroll_pos = 0;
    s_selected_item = 0;
    s_sell_mode = 0;
    s_pending_item_index = -1;

    /* Count empty inventory/pet slots - from FUN_00416be0 */
    empty_slots = 0;
    pet_slots = 0;
    /* This would check DAT_0462cdcc for character slots
     * and DAT_046274ce for pet slots */

    g_shop_ui.is_initialized = 1;
    g_shop_ui.player_gold = 0;  /* Would get from g_inventory.gold */
    g_shop_ui.pending_gold = 0;

    LOG_DEBUG("Shop UI initialized");
    return 1;
}

/*
 * Shutdown shop UI
 */
void shop_ui_shutdown(void) {
    if (s_tooltip_window) {
        /* Would call window_destroy(s_tooltip_window) */
        s_tooltip_window = NULL;
    }

    if (g_shop_ui.window) {
        /* Would call window_destroy(g_shop_ui.window) */
        g_shop_ui.window = NULL;
    }

    memset(&g_shop_ui, 0, sizeof(ShopUIContext));
    memset(g_shop_items, 0, sizeof(g_shop_items));
    g_shop_item_count = 0;

    LOG_DEBUG("Shop UI shutdown");
}

/*
 * Clear shop items
 */
void shop_ui_clear_items(void) {
    memset(g_shop_items, 0, sizeof(g_shop_items));
    g_shop_item_count = 0;
    s_scroll_pos = 0;
    s_selected_item = 0;
}

/*
 * Add item to shop UI
 */
void shop_ui_add_item(const ShopDetailedItem* item) {
    if (g_shop_item_count >= MAX_SHOP_UI_ITEMS) {
        return;
    }

    memcpy(&g_shop_items[g_shop_item_count], item, sizeof(ShopDetailedItem));
    g_shop_item_count++;
}

/*
 * Get shop item
 */
ShopDetailedItem* shop_ui_get_item(u32 index) {
    if (index >= g_shop_item_count) {
        return NULL;
    }
    return &g_shop_items[index];
}

/*
 * Update shop UI - Main update from FUN_00416be0
 * Creates button sprites and handles mouse interaction
 */
void shop_ui_update(void) {
    int i;
    int base_x, base_y;
    int item_y;
    u32 sprite_id;
    int scroll_end;
    static u32 close_sprite = 0;
    static u32 confirm_sprite = 0;
    static u32 scroll_up_sprite = 0;
    static u32 scroll_down_sprite = 0;
    static u32 mode_sprite = 0;

    if (!g_shop_ui.is_initialized) {
        shop_ui_init();
    }

    /* Get window position from DAT_00564e38 widget */
    base_x = 0xc;
    base_y = 0;

    /* Create UI button sprites - FUN_0047e210 calls from FUN_00416be0
     * Button sprite constants:
     *   0x66b1 - Close button
     *   0x66b3 - Confirm button
     *   0x66db - Scroll up
     *   0x66dc - Scroll down
     *   0x6767 - Mode toggle (buy)
     *   0x6768 - Mode toggle (sell)
     */
    close_sprite = render_queue_add_sprite(base_x + 0x19a, base_y + 0x17a, 0x69, 0x66b1, 2);
    confirm_sprite = render_queue_add_sprite(base_x + 0x217, base_y + 0x17a, 0x69, 0x66b3, 2);
    scroll_up_sprite = render_queue_add_sprite(base_x + 300, base_y + 0x22, 0x69, 0x66db, 2);
    scroll_down_sprite = render_queue_add_sprite(base_x + 300, base_y + 0x176, 0x69, 0x66dc, 2);

    /* Mode toggle sprite - 0x6767 for buy, 0x6768 for sell */
    sprite_id = s_sell_mode ? 0x6768 : 0x6767;
    mode_sprite = render_queue_add_sprite(base_x + 0x230, base_y + 0x13e, 0x69, sprite_id, 2);

    /* Check for mouse hover on buttons - DAT_045f1bc4 & 1 check */
    /* Button click handling from FUN_00416de1 switch cases */
    if (g_mouse_state & 1) {
        if (g_hover_sprite == (s32)close_sprite) {
            /* Case 0: Close button - send cancel packet */
            network_send("B\n", 2);
            shop_close();
            return;
        }
        if (g_hover_sprite == (s32)confirm_sprite) {
            /* Case 1: Confirm button */
            shop_ui_confirm_transaction();
            return;
        }
        if (g_hover_sprite == (s32)scroll_up_sprite) {
            /* Case 2: Scroll up */
            shop_ui_scroll_up();
        }
        if (g_hover_sprite == (s32)scroll_down_sprite) {
            /* Case 3: Scroll down */
            shop_ui_scroll_down();
        }
        if (g_hover_sprite == (s32)mode_sprite) {
            /* Case 4: Toggle mode */
            s_sell_mode = !s_sell_mode;
        }
    }

    /* Render scroll area items with click detection */
    scroll_end = s_scroll_pos + 13;
    if (scroll_end > (int)g_shop_item_count) {
        scroll_end = g_shop_item_count;
    }

    if (s_scroll_pos < scroll_end) {
        item_y = base_y + 22;
        for (i = s_scroll_pos; i < scroll_end; i++) {
            ShopDetailedItem* item = &g_shop_items[i];

            /* Check for item click - DAT_045f1bc4 & 2 check */
            if (g_mouse_state & 2) {
                /* Check if click is on this item row */
                /* Would need proper hit testing here */
            }

            item_y += 28;
        }
    }

    /* Handle pending item detail display */
    if (s_pending_item_index >= 0 && s_pending_item_index < (int)g_shop_item_count) {
        ShopDetailedItem* item = &g_shop_items[s_pending_item_index];
        if (item->has_details) {
            /* Show detailed stats - handled in render */
        }
    }
}

/*
 * Render shop UI - FUN_00416be0 rendering implementation
 * Uses render_queue_add_sprite (FUN_0047e210) and text_queue_add (FUN_0041d860)
 */
void shop_ui_render(void) {
    int i;
    int base_x, base_y;
    int item_y;
    char text_buf[128];
    int scroll_end;

    if (!g_shop_ui.is_initialized) {
        return;
    }

    /* Get window base position from window widget */
    base_x = 0xc;   /* Default X from FUN_00416be0: FUN_00448610(0xc,...) */
    base_y = 0;

    /* Render scroll area items - up to 13 visible from DAT_0054ccfc + 13 */
    scroll_end = s_scroll_pos + 13;
    if (scroll_end > (int)g_shop_item_count) {
        scroll_end = g_shop_item_count;
    }

    if (s_scroll_pos < scroll_end) {
        item_y = base_y + 22;  /* Start at Y offset 0x16 */
        for (i = s_scroll_pos; i < scroll_end; i++) {
            ShopDetailedItem* item = &g_shop_items[i];

            /* Check if item has data - from FUN_00416be0 piVar11[0xe] check */
            if (!item->has_details && item->name[0] == '\0') {
                /* Empty slot - render placeholder from DAT_004e220c */
                text_queue_add_simple(base_x + 20, item_y, 1, 0, "", 0);
            } else {
                /* Render item row */
                /* Format from FUN_00416be0:
                 * "%20s = %-13s" or "%20s = %-13d" for items
                 * "%17s %12s %8s" or "%17s %12s %8d" for pets
                 */
                if (item->is_pet) {
                    if (item->item_id == 0) {
                        snprintf(text_buf, sizeof(text_buf), "%17s %12s %8s",
                                 item->name, "", "");
                    } else {
                        snprintf(text_buf, sizeof(text_buf), "%17s %12s %8d",
                                 item->name, "", item->item_id);
                    }
                } else {
                    if (item->item_id == 0) {
                        snprintf(text_buf, sizeof(text_buf), "%20s = %-13s",
                                 item->name, item->description);
                    } else {
                        snprintf(text_buf, sizeof(text_buf), "%20s = %-13d",
                                 item->name, item->item_id);
                    }
                }
                text_queue_add_simple(base_x + 20, item_y, 1, 0, text_buf, 2);

                /* Render selection highlight sprite if has special flag */
                if (item->selected) {
                    /* FUN_0047e640 call for selection highlight */
                    render_queue_add_fade(base_x + 18, item_y - 3,
                                         base_x + 280, item_y + 17,
                                         0x6b, 0xf9, 0);
                }
            }

            item_y += 28;  /* 0x1c spacing between items */
        }
    }

    /* Render player gold - from DAT_0462bee8 */
    snprintf(text_buf, sizeof(text_buf), "Gold: %u", g_shop_ui.player_gold);
    text_queue_add_simple(base_x + 380, base_y + 336, 1, 0, text_buf, 0);

    /* Render pending transaction gold - from DAT_0054c908 */
    snprintf(text_buf, sizeof(text_buf), "Pending: %u", g_shop_ui.pending_gold);
    text_queue_add_simple(base_x + 525, base_y + 336, 1, 0, text_buf, 0);

    /* Render selected item details */
    if (s_selected_item < g_shop_item_count) {
        ShopDetailedItem* item = &g_shop_items[s_selected_item];
        if (item->has_details) {
            shop_ui_render_item_details(base_x + 100, base_y + 50, item);
        }
    }
}

/*
 * Render item details panel - FUN_00416be0 detail rendering
 * Shows item/pet stats when selected
 */
static void shop_ui_render_item_details(int x, int y, ShopDetailedItem* item) {
    char text_buf[64];
    int line_y;
    u32 i;

    if (!item || !item->has_details) {
        return;
    }

    /* Check if this is a pet (DAT_0054dd71[offset] check) */
    if (item->is_pet) {
        /* Pet rendering - render pet sprite and stats */
        /* FUN_0047e210 calls with sprite layer 0x6c for pet image */
        if (item->sprite_id != 0) {
            render_queue_add_sprite(x + 159, y + 100, 0x6c, item->sprite_id, 0);
            render_queue_add_sprite(x + 159, y + 100, 0x69, 0x669d, 0);
        }

        /* Pet name - from DAT_0054dc23 + offset */
        text_queue_add_simple(x + 0x145, y + 150, 1, item->level, item->name, 0);

        /* Pet description - from DAT_0054dcb2 + offset */
        snprintf(text_buf, sizeof(text_buf), "%s", item->description);
        text_queue_add_simple(x + 0x145, y + 174, 1, item->level, text_buf, 0);

        /* Multi-line description - up to 28 chars per line (0x1c) */
        line_y = y + 198;  /* 0xc6 */
        for (i = 0; i < 4 && item->description[i * 28] != '\0'; i++) {
            char line[32];
            strncpy(line, &item->description[i * 28], 28);
            line[28] = '\0';
            text_queue_add_simple(x + 0x145, line_y, 1, 0, line, 0);
            line_y += 24;  /* 0x18 */
        }
    } else {
        /* Item rendering - render item details */
        /* Item name */
        text_queue_add_simple(x + 0x145, y + 35, 1, 0, item->name, 0);

        /* Level indicator - from DAT_0054dd72 */
        snprintf(text_buf, sizeof(text_buf), "Lv: %d", item->level);
        text_queue_add_simple(x + 0x23a, y + 35, 1, 0, text_buf, 0);

        /* Stats - from DAT_0054dc40 array */
        text_queue_add_simple(x + 0x145, y + 55, 1, 6, "HP:", 0);
        snprintf(text_buf, sizeof(text_buf), "%d", item->hp);
        text_queue_add_simple(x + 0x180, y + 55, 1, 0, text_buf, 0);

        text_queue_add_simple(x + 0x145, y + 80, 1, 0, "MP:", 0);
        snprintf(text_buf, sizeof(text_buf), "%d", item->mp);
        text_queue_add_simple(x + 0x180, y + 80, 1, 0, text_buf, 0);

        text_queue_add_simple(x + 0x145, y + 100, 1, 0, "ATK:", 0);
        snprintf(text_buf, sizeof(text_buf), "%d", item->attack);
        text_queue_add_simple(x + 0x180, y + 100, 1, 0, text_buf, 0);

        text_queue_add_simple(x + 0x145, y + 120, 1, 0, "DEF:", 0);
        snprintf(text_buf, sizeof(text_buf), "%d", item->defense);
        text_queue_add_simple(x + 0x180, y + 120, 1, 0, text_buf, 0);

        text_queue_add_simple(x + 0x145, y + 140, 1, 0, "AGI:", 0);
        snprintf(text_buf, sizeof(text_buf), "%d", item->agility);
        text_queue_add_simple(x + 0x180, y + 140, 1, 0, text_buf, 0);

        text_queue_add_simple(x + 0x145, y + 160, 1, 0, "SPI:", 0);
        snprintf(text_buf, sizeof(text_buf), "%d", item->spirit);
        text_queue_add_simple(x + 0x180, y + 160, 1, 0, text_buf, 0);

        /* Calculated stats from DAT_0054dc04-0x10 - divided by 10 */
        text_queue_add_simple(x + 0x186, y + 80, 1, 5, "Hit:", 0);
        snprintf(text_buf, sizeof(text_buf), "%d", item->hit_rate / 10);
        text_queue_add_simple(x + 0x1b0, y + 80, 1, 0, text_buf, 0);

        text_queue_add_simple(x + 0x186, y + 100, 1, 3, "Dodge:", 0);
        snprintf(text_buf, sizeof(text_buf), "%d", item->dodge / 10);
        text_queue_add_simple(x + 0x1b0, y + 100, 1, 0, text_buf, 0);

        text_queue_add_simple(x + 0x186, y + 120, 1, 6, "Crit:", 0);
        snprintf(text_buf, sizeof(text_buf), "%d", item->critical / 10);
        text_queue_add_simple(x + 0x1b0, y + 120, 1, 0, text_buf, 0);

        text_queue_add_simple(x + 0x186, y + 140, 1, 4, "Luck:", 0);
        snprintf(text_buf, sizeof(text_buf), "%d", item->luck / 10);
        text_queue_add_simple(x + 0x1b0, y + 140, 1, 0, text_buf, 0);

        /* Charm - from DAT_0054dc14 */
        text_queue_add_simple(x + 0x186, y + 160, 1, 0, "Charm:", 0);
        snprintf(text_buf, sizeof(text_buf), "%d", item->charm);
        text_queue_add_simple(x + 0x1b0, y + 160, 1, 0, text_buf, 0);

        /* Skills - from DAT_0054dcc2 array, up to 7 skills */
        if (item->skill_count > 0) {
            line_y = y + 190;  /* 0xbe */
            for (i = 0; i < item->skill_count && i < MAX_SHOP_SKILLS; i++) {
                snprintf(text_buf, sizeof(text_buf), "Skill %d: %s",
                         (int)(i + 1), item->skill_names[i]);
                text_queue_add_simple(x + 0x145, line_y, 1, 0, text_buf, 0);
                line_y += 20;  /* 0x14 */
            }
        }
    }
}

/*
 * Handle shop UI click - FUN_00416be0 click detection
 */
void shop_ui_handle_click(int x, int y) {
    int i;
    int button_index = -1;

    /* Check which button was clicked - DAT_0054cce8 check pattern */
    for (i = 0; i < 5; i++) {
        /* Would check if x,y is within button bounds */
        /* if (is_point_in_button(x, y, s_ui_buttons[i])) {
         *     button_index = i;
         *     break;
         * }
         */
    }

    switch (button_index) {
        case 0:  /* Buy/confirm button */
            shop_ui_confirm_transaction();
            break;

        case 1:  /* Close button */
            shop_ui_shutdown();
            break;

        case 2:  /* Scroll up */
            shop_ui_scroll_up();
            break;

        case 3:  /* Scroll down */
            shop_ui_scroll_down();
            break;

        case 4:  /* Toggle buy/sell mode */
            shop_ui_toggle_mode();
            break;

        default:
            /* Check if clicking on item row */
            shop_ui_handle_item_click(x, y);
            break;
    }
}

/*
 * Handle item row click
 */
static void shop_ui_handle_item_click(int x, int y) {
    int item_index = 0;
    ShopDetailedItem* item;

    /* Calculate which item row was clicked */
    /* row_y = (y - 122) / 28;  // Assuming items start at y=122 with 28px rows */
    /* item_index = s_scroll_pos + row_y; */

    /* For now, use simple selection */
    if (s_selected_item < g_shop_item_count) {
        item = &g_shop_items[s_selected_item];

        if (item->has_details) {
            /* Already selected - toggle into shopping cart */
            /* Check if player has enough gold */
            if (g_shop_ui.player_gold >= item->price) {
                g_shop_ui.pending_gold += item->price;
                /* Send buy packet */
                shop_buy_network(item->item_id, item->price);
            }
        } else {
            /* Request item details */
            s_pending_item_index = item_index;
        }

        s_selected_item = item_index;
        s_sell_mode = 0;
    }
}

/*
 * Scroll up
 */
void shop_ui_scroll_up(void) {
    if (s_scroll_pos > 0) {
        s_scroll_pos--;
    }
}

/*
 * Scroll down
 */
void shop_ui_scroll_down(void) {
    if (s_scroll_pos + 14 < (int)g_shop_item_count) {
        s_scroll_pos++;
    }
}

/*
 * Set scroll position
 */
void shop_ui_set_scroll(int pos) {
    if (pos < 0) pos = 0;
    if (pos + 14 > (int)g_shop_item_count) {
        pos = g_shop_item_count - 14;
        if (pos < 0) pos = 0;
    }
    s_scroll_pos = pos;
}

/*
 * Toggle buy/sell mode
 */
void shop_ui_toggle_mode(void) {
    s_sell_mode = !s_sell_mode;
    LOG_DEBUG("Shop mode: %s", s_sell_mode ? "Sell" : "Buy");
}

/*
 * Get current mode
 */
int shop_ui_get_mode(void) {
    return s_sell_mode;
}

/*
 * Buy selected item
 */
int shop_ui_buy_selected(void) {
    ShopDetailedItem* item;

    if (s_selected_item >= g_shop_item_count) {
        return 0;
    }

    item = &g_shop_items[s_selected_item];
    if (!item->has_details) {
        return 0;
    }

    return shop_buy_item(item->item_id, item->price);
}

/*
 * Confirm transaction - FUN_00416be0 case 0
 */
int shop_ui_confirm_transaction(void) {
    char packet[256];
    u32 item_count;
    u32 i;

    /* Build transaction packet */
    item_count = 0;
    for (i = 0; i < g_shop_item_count; i++) {
        if (g_shop_items[i].has_details) {
            item_count++;
        }
    }

    if (item_count == 0) {
        /* Send cancel packet */
        network_send("B\n", 2);
    } else {
        /* Send buy packet with item list */
        snprintf(packet, sizeof(packet), "B|%u", item_count);
        /* Would append item IDs */
        network_send(packet, strlen(packet));
    }

    /* Reset state */
    g_shop_ui.pending_gold = 0;

    if (s_tooltip_window) {
        /* window_destroy(s_tooltip_window); */
        s_tooltip_window = NULL;
    }

    return 1;
}

/*
 * Process shop packet - FUN_00416be0 packet handling
 * Handles 'B' and 'D' prefixed packets
 */
void shop_ui_process_packet(const char* data, u32 size) {
    char field[256];
    u32 item_count;
    u32 item_id;
    u32 i;
    int field_idx;

    if (size == 0 || data[0] == '\0') {
        return;
    }

    /* Parse packet type */
    if (data[0] == 'B') {
        /* Item list packet: B|<count>|<items>... */
        protocol_parse_field(data, '|', 1, field, sizeof(field));
        item_count = atoi(field);

        shop_ui_clear_items();

        for (i = 0; i < item_count && i < MAX_SHOP_UI_ITEMS; i++) {
            ShopDetailedItem* item = &g_shop_items[i];

            /* Parse item fields - from FUN_00416be0 packet parsing:
             * Field 0: type (0 = item, 1 = pet)
             * Field 1: item ID
             * Field 2: item name
             * Field 3: description (if type 0)
             * Field 4: price
             */
            field_idx = 2 + i * 4;

            protocol_parse_field(data, '|', field_idx, field, sizeof(field));
            item->is_pet = (field[0] == '1');

            protocol_parse_field(data, '|', field_idx + 1, field, sizeof(field));
            item->item_id = atoi(field);

            protocol_parse_field(data, '|', field_idx + 2, item->name, sizeof(item->name));

            if (!item->is_pet) {
                protocol_parse_field(data, '|', field_idx + 3, item->description,
                                     sizeof(item->description));
            }

            protocol_parse_field(data, '|', field_idx + 4, field, sizeof(field));
            item->price = atoi(field);

            item->has_details = 1;
            g_shop_item_count++;
        }

        LOG_DEBUG("Shop received %u items", item_count);

    } else if (data[0] == 'D') {
        /* Item detail packet: D|<item_id>|<details>... */
        protocol_parse_field(data, '|', 1, field, sizeof(field));
        item_id = atoi(field);

        /* Find item by ID */
        for (i = 0; i < g_shop_item_count; i++) {
            if (g_shop_items[i].item_id == item_id) {
                ShopDetailedItem* item = &g_shop_items[i];

                /* Parse detailed stats - from FUN_00416be0 D packet parsing:
                 * Field 2: description lines
                 * Field 3: level
                 * Fields 4-13: stats (HP, MP, ATK, DEF, AGI, SPI, LUK, etc.)
                 * Fields 14+: skills
                 */
                protocol_parse_field(data, '|', 2, item->description,
                                     sizeof(item->description));

                protocol_parse_field(data, '|', 3, field, sizeof(field));
                item->level = atoi(field);

                protocol_parse_field(data, '|', 4, field, sizeof(field));
                item->hp = atoi(field);

                protocol_parse_field(data, '|', 5, field, sizeof(field));
                item->mp = atoi(field);

                protocol_parse_field(data, '|', 6, field, sizeof(field));
                item->attack = atoi(field);

                protocol_parse_field(data, '|', 7, field, sizeof(field));
                item->defense = atoi(field);

                protocol_parse_field(data, '|', 8, field, sizeof(field));
                item->agility = atoi(field);

                protocol_parse_field(data, '|', 9, field, sizeof(field));
                item->spirit = atoi(field);

                protocol_parse_field(data, '|', 10, field, sizeof(field));
                item->luck = atoi(field);

                item->has_details = 1;
                s_pending_item_index = i;
                break;
            }
        }
    }
}
