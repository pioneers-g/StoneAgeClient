/*
 * Stone Age Client - Item System
 * Reverse engineered from sa_9061.exe (FUN_0045ffb0)
 */

#ifndef ITEM_H
#define ITEM_H

#include "types.h"

/* Constants from binary analysis */
#define MAX_INVENTORY_SLOTS      7    /* 7 items per character from case 0x42 loop */
#define MAX_ITEM_NAME_LEN        29   /* 0x1d from field size check */
#define MAX_ITEM_DESC_LEN        85   /* 0x55 from field size check */
#define MAX_ITEM_FIELD_LEN       17   /* 0x11 from field size check */

/* Item types */
typedef enum {
    ITEM_TYPE_NONE = 0,
    ITEM_TYPE_WEAPON = 1,
    ITEM_TYPE_ARMOR = 2,
    ITEM_TYPE_HELMET = 3,
    ITEM_TYPE_ACCESSORY = 4,
    ITEM_TYPE_CONSUMABLE = 5,
    ITEM_TYPE_MATERIAL = 6,
    ITEM_TYPE_QUEST = 7,
    ITEM_TYPE_PET_ITEM = 8
} ItemType;

/* Item rarity */
typedef enum {
    ITEM_RARITY_COMMON = 0,
    ITEM_RARITY_UNCOMMON = 1,
    ITEM_RARITY_RARE = 2,
    ITEM_RARITY_EPIC = 3,
    ITEM_RARITY_LEGENDARY = 4
} ItemRarity;

/*
 * Inventory item structure - 0x184 (388) bytes per entry
 * From FUN_0045ffb0 case 0x42 analysis
 * Array at DAT_046274d4, indexed by (char_index * 0xb18) + (item_index * 0x184)
 *
 * Memory layout from Ghidra decompilation:
 * - active (DAT_046275b0): 2 bytes at offset 0xDC
 * - name (DAT_046275ba): max 0x1d bytes at offset 0xE6
 * - field_0x23 (DAT_046275d7): max 0x11 bytes at offset 0x103
 * - description (DAT_046275e8): max 0x55 bytes at offset 0x114
 * - level (DAT_046275b4): 2 bytes at offset 0x124
 * - has_level (DAT_046275b6): 2 bytes at offset 0x126
 * - count (DAT_046275b2): 2 bytes at offset 0x122
 * - sprite_id (DAT_046275b8): 2 bytes at offset 0x128
 */
#pragma pack(push, 1)
typedef struct {
    /* Offset 0x00-0xD7: Unknown fields (first part) */
    u32 field_0x00;              /* Field 3 (integer from param) */
    u32 field_0x04;              /* Field 5 (integer) */
    u32 field_0x08;              /* Field 7 (integer) */
    u32 field_0x0c;              /* Reserved */
    u32 field_0x10;              /* Reserved */
    u32 field_0x14;              /* Reserved */
    u32 field_0x18;              /* Reserved */
    u32 field_0x1c;              /* Reserved */
    u32 field_0x20;              /* Reserved */
    u32 field_0x24;              /* Reserved */
    u32 field_0x28;              /* Reserved */
    u32 field_0x2c;              /* Reserved */
    u32 field_0x30;              /* Reserved */
    u32 field_0x34;              /* Reserved */
    u32 field_0x38;              /* Reserved */
    u32 field_0x3c;              /* Reserved */
    u32 field_0x40;              /* Reserved */
    u32 field_0x44;              /* Reserved */
    u32 field_0x48;              /* Reserved */
    u32 field_0x4c;              /* Reserved */
    u32 field_0x50;              /* Reserved */
    u32 field_0x54;              /* Reserved */
    u32 field_0x58;              /* Reserved */
    u32 field_0x5c;              /* Reserved */
    u32 field_0x60;              /* Reserved */
    u32 field_0x64;              /* Reserved */
    u32 field_0x68;              /* Reserved */
    u32 field_0x6c;              /* Reserved */
    u32 field_0x70;              /* Reserved */
    u32 field_0x74;              /* Reserved */
    u32 field_0x78;              /* Reserved */
    u32 field_0x7c;              /* Reserved */
    u32 field_0x80;              /* Reserved */
    u32 field_0x84;              /* Reserved */
    u32 field_0x88;              /* Reserved */
    u32 field_0x8c;              /* Reserved */
    u32 field_0x90;              /* Reserved */
    u32 field_0x94;              /* Reserved */
    u32 field_0x98;              /* Reserved */
    u32 field_0x9c;              /* Reserved */
    u32 field_0xa0;              /* Reserved */
    u32 field_0xa4;              /* Reserved */
    u32 field_0xa8;              /* Reserved */
    u32 field_0xac;              /* Reserved */
    u32 field_0xb0;              /* Reserved */
    u32 field_0xb4;              /* Reserved */
    u32 field_0xb8;              /* Reserved */
    u32 field_0xbc;              /* Reserved */
    u32 field_0xc0;              /* Reserved */
    u32 field_0xc4;              /* Reserved */
    u32 field_0xc8;              /* Reserved */
    u32 field_0xcc;              /* Reserved */
    u32 field_0xd0;              /* Reserved */
    u32 field_0xd4;              /* Reserved */
    u32 field_0xd8;              /* Reserved */

    /* Offset 0xDC: active flag from *(undefined2 *)(&DAT_046275b0 + index) = 1 */
    u16 active;                  /* 0 = empty, 1 = has item */

    /* Offset 0xDE: padding */
    u16 padding1;

    /* Offset 0xE0-0xE4: Reserved */
    u32 field_0xe0;

    /* Offset 0xE6: item name (max 0x1d = 29 bytes) - stored at DAT_046275ba */
    char name[MAX_ITEM_NAME_LEN];

    /* Offset 0x103: description field (max 0x11 = 17 bytes) - stored at DAT_046275d7 */
    char short_field[MAX_ITEM_FIELD_LEN];

    /* Offset 0x114: extended description (max 0x55 = 85 bytes) - stored at DAT_046275e8 */
    char description[MAX_ITEM_DESC_LEN];

    /* Offset 0x122: item count - stored at DAT_046275b2 */
    u16 count;

    /* Offset 0x124: level or sprite - stored at DAT_046275b4 */
    u16 level;

    /* Offset 0x126: has_level flag - stored at DAT_046275b6 */
    u16 has_level;               /* 1 if level >= 100 */

    /* Offset 0x128: sprite ID - stored at DAT_046275b8 */
    u16 sprite_id;

    /* Offset 0x12A-0x13C: more fields */
    u16 field_0x12a;
    u16 field_0x12c;
    u32 field_0x12e;
    u32 field_0x132;
    u32 field_0x136;
    u32 field_0x13a;

    /* Offset 0x13D: field 8 string (max 0x11 = 17 bytes) - at DAT_0462763d */
    char field_8[MAX_ITEM_FIELD_LEN];

    /* Offset 0x150-0x160: more fields */
    u32 field_0x150;
    u32 field_0x154;
    u32 field_0x158;
    u32 field_0x15c;
    u32 field_0x160;

    /* Offset 0x164: field 10 string - at DAT_046274e4 */
    char field_10[32];

    /* Offset 0x184: end */
} InventoryItem;
#pragma pack(pop)

/*
 * Character inventory block - 0xb18 (2840) bytes
 * Contains 7 inventory slots (7 * 0x184 = 0xb14) + 4 bytes header
 */
#pragma pack(push, 1)
typedef struct {
    u32 char_index;              /* Character index */
    InventoryItem items[MAX_INVENTORY_SLOTS];
    u32 padding;
} CharacterInventory;
#pragma pack(pop)

/* Item database entry */
typedef struct {
    u32 id;
    char name[32];
    char description[128];
    ItemType type;
    u16 sub_type;
    u16 rarity;
    u16 sprite_id;
    u16 max_stack;
    u32 price;

    /* Stats */
    s16 attack;
    s16 defense;
    s16 magic_attack;
    s16 magic_defense;
    s16 agility;
    s16 hp_bonus;
    s16 mp_bonus;

    /* Requirements */
    u16 req_level;
    u16 req_strength;
    u16 req_dexterity;

    /* Effect (for consumables) */
    u16 effect_type;
    s16 effect_value;
    u16 effect_duration;

    /* Flags */
    u32 flags;
} Item;

/* Inventory slot (simplified for player use) */
typedef struct {
    u32 item_id;
    u16 count;
    u8  equipped;
    u8  flags;
} InventorySlot;

/* Equipment slots */
typedef enum {
    EQUIP_SLOT_WEAPON = 0,
    EQUIP_SLOT_ARMOR = 1,
    EQUIP_SLOT_HELMET = 2,
    EQUIP_SLOT_ACCESSORY1 = 3,
    EQUIP_SLOT_ACCESSORY2 = 4,
    EQUIP_SLOT_MAX = 5
} EquipSlot;

/* Player inventory structure */
typedef struct {
    InventorySlot slots[100];
    u32 capacity;
    u32 count;
    u32 gold;

    /* Equipment */
    InventorySlot equipment[EQUIP_SLOT_MAX];

    /* Bank storage */
    InventorySlot bank[200];
    u32 bank_gold;
} Inventory;

/* Item database */
#define MAX_ITEMS 5000
extern Item g_items[MAX_ITEMS];
extern u32 g_item_count;

/* Global inventory */
extern Inventory g_inventory;

/* ========================================
 * Core Item Functions (item.c)
 * ======================================== */

int item_init(void);
void item_shutdown(void);
Item* item_get(u32 item_id);
int item_load_database(const char* path);

/* ========================================
 * Inventory Management (inventory.c)
 * ======================================== */

int inventory_add_item(u32 item_id, u16 count);
int inventory_remove_item(u32 item_id, u16 count);
int inventory_has_item(u32 item_id, u16 count);
u16 inventory_get_count(u32 item_id);
int inventory_find_slot(u32 item_id);
int inventory_find_empty_slot(void);

/* Equipment */
int inventory_equip(u32 slot_index, EquipSlot equip_slot);
int inventory_unequip(EquipSlot equip_slot);
Item* inventory_get_equipped(EquipSlot slot);
int inventory_can_equip(u32 item_id, EquipSlot slot);

/* Sorting */
void inventory_sort_by_type(void);

/* Bank */
int inventory_bank_deposit(u32 item_id, u16 count);
int inventory_bank_withdraw(u32 item_id, u16 count);
int inventory_bank_deposit_gold(u32 amount);
int inventory_bank_withdraw_gold(u32 amount);

/* ========================================
 * Item Rendering (item.c)
 * ======================================== */

void inventory_render(void);
void inventory_render_slot(int x, int y, InventorySlot* slot);
void inventory_render_tooltip(int x, int y, u32 item_id);
void inventory_sort_by_name(void);
void inventory_sort_by_rarity(void);

/* Trading */
int inventory_can_trade(u32 item_id);
int inventory_trade_add(u32 item_id, u16 count);
void inventory_trade_clear(void);

/* ========================================
 * Item Flags & Effects
 * ======================================== */

/* Item flags */
#define ITEM_FLAG_TRADEABLE    0x0001
#define ITEM_FLAG_DROPPABLE    0x0002
#define ITEM_FLAG_QUEST        0x0004
#define ITEM_FLAG_UNIQUE       0x0008
#define ITEM_FLAG_CRAFTABLE    0x0010

/* Item effect types */
#define EFFECT_HEAL_HP         1
#define EFFECT_HEAL_MP         2
#define EFFECT_BUFF_ATTACK     3
#define EFFECT_BUFF_DEFENSE    4
#define EFFECT_BUFF_SPEED      5
#define EFFECT_CURE_POISON     10
#define EFFECT_CURE_STUN       11

/* Include sub-module headers */
#include "item_effect.h"
#include "item_protocol.h"

#endif /* ITEM_H */
