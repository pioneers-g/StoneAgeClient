/*
 * Stone Age Client - NPC Dialogue Script System Header
 * Reverse engineered from sa_9061.exe (FUN_00462f60, FUN_00463380, FUN_004781f0)
 */

#ifndef NPC_H
#define NPC_H

#include "types.h"

/* Maximum constants */
#define MAX_DIALOGUE_ENTRIES    40
#define MAX_SHOP_ITEMS          100
#define MAX_SEEN_NPCS           4096
#define MAX_DIALOGUE_TEXT       256
#define MAX_SHOP_NAME           32
#define MAX_NPC_NAME            21      /* 0x15 from FUN_00462f60 */
#define MAX_DIALOG_OPTIONS      4

/* NPC flags */
#define NPC_FLAG_SHOP           0x01
#define NPC_FLAG_QUEST          0x02
#define NPC_FLAG_HEALER         0x04
#define NPC_FLAG_STORAGE        0x08
#define NPC_FLAG_TELEPORT       0x10

/* Dialog action types - from FUN_004781f0 switch cases
 * TODO: Verify all action type values match binary FUN_004781f0
 * Some values are interpolated from switch case patterns
 */
typedef enum {
    DIALOG_ACTION_CLOSE = 0,        /* FUN_00477d90(3) - Close dialog */
    DIALOG_ACTION_OPEN = 1,         /* FUN_00477d90(4) - Open dialog */
    DIALOG_ACTION_SET_TEXT = 2,     /* FUN_00477d70 + FUN_00477d90(0) - Set text, mode 0 */
    DIALOG_ACTION_SET_MENU = 3,     /* FUN_00477d70 + FUN_00477d90(0xc) - Set menu options */
    DIALOG_ACTION_SET_OPTIONS = 4,  /* FUN_00477d70 + FUN_00477d90(1) - Set option buttons */
    DIALOG_ACTION_SET_VAR = 5,      /* FUN_00477d70 + FUN_00477d90(2) - Set variable */
    /* Cases 6-9 not present in switch - reserved */
    DIALOG_ACTION_SHOW_ITEM = 10,   /* FUN_00477d70 + FUN_00477d90(2) - Show item info (0x0a) */
    DIALOG_ACTION_MENU_BUY = 11,    /* FUN_00477d70 + FUN_00477d90(5) - Buy menu (0x0b) */
    DIALOG_ACTION_MENU_SELL = 12,   /* FUN_00477d70 + FUN_00477d90(6) - Sell menu (0x0c) */
    DIALOG_ACTION_MENU_TRADE = 13,  /* FUN_00477d70 + FUN_00477d90(7) - Trade menu (0x0d) */
    DIALOG_ACTION_MENU_STORAGE = 14,/* FUN_00477d70 + FUN_00477d90(8) - Storage menu (0x0e) */
    DIALOG_ACTION_MENU_HEAL = 15,   /* FUN_00477d70 + FUN_00477d90(9) - Heal menu (0x0f) */
    DIALOG_ACTION_MENU_TELEPORT = 16,/* FUN_00477d70 + FUN_00477d90(10) - Teleport menu (0x10) */
    DIALOG_ACTION_MENU_QUEST = 17,  /* FUN_00477d70 + FUN_00477d90(4) - Quest menu (0x11) */
    DIALOG_ACTION_SHOW_STATUS = 18, /* FUN_00477d70 + FUN_00477d90(0xb) - Show status (0x12) */
    DIALOG_ACTION_SHOW_QUEST = 19,  /* FUN_00477d70 + FUN_00477d90(3) - Show quest info (0x13) */
    DIALOG_ACTION_SET_POSITION = 20,/* FUN_00477cb0 + FUN_00477d70 - Set position + text (0x14) */
    DIALOG_ACTION_CONFIRM = 21,     /* Position + text + conditional show (0x15) */
    DIALOG_ACTION_SHOW_GUILD = 22,  /* Position + text + conditional show (0x16) */
    DIALOG_ACTION_SPECIAL_PARAM = 23,/* FUN_00478190(param_5) - Special param action (0x17) */
    /* Cases 24-29 not present in switch - reserved */
    DIALOG_ACTION_HIDE = 30,        /* FUN_00477d70 - Just set text (0x1e) */
    DIALOG_ACTION_SHOW_PARTY = 31,  /* Position + text + mode 3 (0x1f) */
    /* Cases 32-33 not present in switch - reserved */
    DIALOG_ACTION_QUEST_COMPLETE = 34,/* Position + text + conditional (0x22) */
    /* Cases 35-40 not present in switch - reserved */
    DIALOG_ACTION_ENTITY_EFFECT = 41,/* Entity effect at offset 0x14 (0x29) */
    DIALOG_ACTION_CLEAR_ENTITY = 42,/* Clear entity effect at offset 0x14 (0x2a) */
    DIALOG_ACTION_CREATE_ENTITY = 43,/* Create entity with name storage (0x2b) */
    /* Cases 43-50 not present in switch - reserved */
    DIALOG_ACTION_CLEAR_EFFECT = 51,/* Clear effects at 0x1c/0x20 (0x33) */
    /* Cases 52-59 not present in switch - reserved */
    DIALOG_ACTION_MULTI_SPAWN = 60  /* Multi-element spawn (0x3c) */
} DialogAction;

/* Dialogue entry structure */
typedef struct DialogueEntry {
    u32 npc_id;
    u16 portrait_id;
    u16 flags;
    u32 timestamp;
    char text[MAX_DIALOGUE_TEXT];
    struct DialogueEntry* next;
    struct DialogueEntry* prev;
} DialogueEntry;

/* Shop item structure */
typedef struct {
    u32 id;
    u32 price;
    u32 stock;
    u32 flags;
    char name[32];
} ShopItem;

/* Dialog option structure */
typedef struct {
    char text[64];
    u16 action;
    u16 param;
    u8 enabled;
    u8 selected;
} DialogOption;

/* Dialog entity structure - for spawned entities during dialog */
typedef struct {
    u32 sprite_id;
    s32 x;
    s32 y;
    u16 action_type;
    u16 anim_frame;
    u8 visible;
    u8 reserved[3];
} DialogEntity;

/* Dialog context - maintains current dialog state */
typedef struct {
    u32 npc_id;
    char npc_name[MAX_NPC_NAME];
    char text[MAX_DIALOGUE_TEXT];
    DialogOption options[MAX_DIALOG_OPTIONS];
    DialogEntity entities[4];
    u16 action_type;
    u16 sub_action;
    u16 mode;               /* Dialog mode */
    u16 padding_mode;
    s32 pos_x;              /* Dialog position X */
    s32 pos_y;              /* Dialog position Y */
    u32 param1;
    u32 param2;
    u32 param3;
    u8 has_options;
    u8 entity_count;
    u8 wait_input;
    u8 show_confirm;        /* Show confirm button */
    u8 show_guild;          /* Show guild options */
    u8 reserved[2];
} DialogContext;

/* Map NPC data - for NPCs on the field */
#define MAX_NPCS            256

typedef struct {
    u32 id;
    u32 type_id;
    u16 sprite_id;
    u16 portrait_id;
    s16 x;
    s16 y;
    u8 direction;
    u8 flags;
    char name[MAX_NPC_NAME];
} NPCData;

/* Talk window structure */
typedef struct {
    HWND hWnd;
    HWND hMainWnd;
    int initialized;

    /* Device contexts for rendering */
    HDC hdc[5];
    HBITMAP hBmp[5];
    HGDIOBJ hOldBmp[5];

    /* Buffer DC */
    HDC hdcBuffer;
    HBITMAP hBufferBmp;
    HGDIOBJ hOldBufferBmp;

    /* Dialogue entries */
    DialogueEntry* entries;
    DialogueEntry* current;
    DialogueEntry* head;
    DialogueEntry* tail;
    u32 entry_count;

    /* Active dialogue state */
    u32 active_npc;
    u32 npc_id;
    int scroll_offset;
} TalkWindow;

/* Player data reference */
typedef struct PlayerData {
    u32 id;
    char name[24];
    u16 x, y;
    u8  in_battle;
    u8  padding;
} PlayerData;

/* NPC context structure */
typedef struct {
    /* Current NPC state */
    u32 current_npc;
    u32 active_dialog_npc;
    int dialog_open;
    int shop_open;
    int in_quest;
    int in_event;
    int in_dialog;

    /* Debug mode flag */
    int debug_mode;

    /* Dialog mode - from FUN_00477d90 */
    int dialog_mode;

    /* Shop data */
    char shop_name[MAX_SHOP_NAME];
    ShopItem* shop_items;
    int shop_count;
    int shop_mode;

    /* Quest data */
    u32 quest_npc;
    u32 current_quest;

    /* Seen NPCs tracking */
    u32 seen_npcs[MAX_SEEN_NPCS];
    int seen_count;

    /* Map NPCs */
    NPCData npcs[MAX_NPCS];
    u32 npc_count;

    /* Current dialog context */
    DialogContext dialog;

    /* Player data reference */
    PlayerData* player_data;

    /* Entity reference for dialog - DAT_0462e3ac pattern */
    void* current_entity;

    /* Entity list for NPC spawning */
    void* entities[MAX_NPCS];
    u32 entity_count;

    /* Base position for entity spawning */
    s32 base_x;
    s32 base_y;
} NPCContext;

/* Global NPC context */
extern NPCContext g_npc;

/* Initialization and shutdown */
int npc_init(void);
void npc_shutdown(void);

/* Skin loading */
int npc_load_skin(const char* path);

/* Talk window management */
int npc_show_talk_window(HINSTANCE hInstance);
void npc_hide_talk_window(void);
void npc_init_ime(void);

/* Dialogue management */
int npc_add_dialogue(u32 npc_id, const char* text, u16 portrait_id, u8 flags);
void npc_clear_dialogue(void);
void npc_set_dialogue(u32 npc_id, const char* text);
void npc_advance_dialogue(void);

/* Packet handlers */
void npc_handle_dialog_packet(void* data, u32 size);
void npc_handle_shop_packet(void* data, u32 size);

/* NPC interaction */
void npc_send_response(u32 npc_id);
NPCData* npc_find_by_id(u32 npc_id);
void npc_interact(u32 npc_id);

/* Shop functions */
int npc_open_shop(u32 npc_id, const char* shop_name);
void npc_close_shop(void);
int npc_add_shop_item(u32 item_id, u32 price, u32 stock);
int npc_find_shop_item(u32 item_id);
int npc_buy_item(u32 item_id, u32 count);
int npc_sell_item(u32 item_id, u32 count);
void npc_update_shop(void);

/* Quest functions */
void npc_handle_quest(u32 npc_id, u32 quest_id);
void npc_end_quest(void);

/* Rendering */
void npc_render_dialog(void);
void npc_render_talk_window(HWND hWnd);

/* Input handling */
void npc_handle_click(int x, int y);
void npc_select_option(int option);
void npc_send_option(int option);

/* Player data */
void npc_set_player_data(void* player_data);

/* Window access */
HWND npc_get_talk_window(void);
int npc_is_talk_visible(void);

/* Dialog action processing - FUN_004781f0 */
void npc_process_dialog_action(DialogContext* ctx, DialogAction action);
void npc_set_dialog_position(int x, int y);
void npc_create_dialog_entity(u32 sprite_id, int x, int y, u16 action);
void npc_spawn_dialog_effect(u32 effect_id, int x, int y, u8 effect_type);
void npc_clear_dialog_entities(void);
void npc_show_dialog(u32 npc_id, const char* text);
void npc_handle_special_action(int param);
void npc_handle_multi_spawn(DialogContext* ctx);

/* Protocol parsing helpers */
int npc_parse_field(const char* str, char delimiter, int field_index, char* output, int max_len);
int npc_parse_field_int(const char* str, char delimiter, int field_index);

/* Extended packet handlers */
void npc_handle_dialog_action(void* data, u32 size);
void npc_handle_object_spawn(void* data, u32 size);

/* Dialog acknowledgment functions - FUN_0043b490, FUN_0048f900 patterns */
void npc_send_dialog_ack(u32 npc_id);
void npc_send_ack_binary_impl(u32 npc_id);
void npc_set_context_name(void* context, const char* name);

/* Window procedure */
LRESULT CALLBACK npc_window_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif /* NPC_H */
