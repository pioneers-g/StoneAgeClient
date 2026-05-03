/*
 * Stone Age Client - Text Protocol Handlers
 * Command handler implementations for text protocol
 * Split from text_protocol.c
 */

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "text_protocol.h"
#include "text_protocol_internal.h"
#include "network.h"
#include "protocol.h"
#include "character.h"
#include "character_protocol.h"
#include "item_protocol.h"
#include "battle.h"
#include "npc.h"
#include "npc_dialog.h"
#include "shop.h"
#include "shop_ui.h"
#include "party.h"
#include "trade.h"
#include "chat.h"
#include "logger.h"

/* External state variables */
extern int g_skip_enter_game;
extern int g_dialog_skip_flag;
extern u32 g_player_char_id;
extern u32 g_current_npc_id;
extern void* g_current_npc_context;
extern u32 g_protocol_mode;
extern int g_in_battle;
extern int g_game_sub_state;
extern int g_game_state_flag;
extern int g_state_changed;

/* Forward declarations */
static void npc_send_dialog_ack(u32 npc_id);
static void npc_send_dialog_ack_binary(u32 npc_id);
static void npc_set_context_name(void* context, const char* name);

/* Authentication handlers */
void handle_L(void) { LOG_DEBUG("L: logout - reason %d", get_int_param(1)); }

void handle_ClientLogin(void) {
    LOG_DEBUG("ClientLogin: '%s' result %d", get_unescaped_param(1), get_int_param(2));
}

void handle_CreateNewChar(void) {
    LOG_DEBUG("CreateNewChar: '%s' result %d", get_unescaped_param(2), get_int_param(1));
}

void handle_CharDelete(void) {
    LOG_DEBUG("CharDelete: '%s' result %d", get_unescaped_param(1), get_int_param(2));
}

void handle_CharLogin(void) {
    LOG_DEBUG("CharLogin: '%s' result %d", get_unescaped_param(2), get_int_param(1));
}

void handle_CharList(void) {
    const char* raw = s_proto_ctx.line_copy;
    if (raw) character_parse_list(raw);
    LOG_DEBUG("CharList: %u fields", s_proto_ctx.field_count);
}

void handle_CharLogout(void) { LOG_DEBUG("CharLogout: %d", get_int_param(1)); }
void handle_ProcGet(void) { LOG_DEBUG("ProcGet: %s", get_unescaped_param(1)); }
void handle_PlayerNumGet(void) { LOG_DEBUG("PlayerNumGet: %d, %d", get_int_param(1), get_int_param(2)); }

/* Item handlers */
void handle_XY(void) { item_handle_text((u32)get_int_param(1)); LOG_DEBUG("XY: item"); }
void handle_XYC(void) { LOG_DEBUG("XYC: drop %d reason %d", get_int_param(1), get_int_param(2)); }
void handle_XYD(void) { LOG_DEBUG("XYD: delete '%s' %d", get_unescaped_param(1), get_int_param(2)); }
void handle_XYE(void) { LOG_DEBUG("XYE: logout %d", get_int_param(1)); }

void handle_XYF(void) {
    const char* raw = get_string_param(1);
    const char* unesc = get_unescaped_param(1);
    size_t len;
    u32 idx;
    char subtype;

    if (!raw || !raw[0]) return;
    subtype = raw[0];

    if (subtype == 'C') {
        const char* chat = (raw[1] == '|') ? raw + 2 : raw + 1;
        len = strlen(chat); if (len >= MSG_LOG_SIZE) len = MSG_LOG_SIZE - 1;
        idx = s_chat_log_index;
        memcpy(s_chat_log[idx], chat, len);
        s_chat_log[idx][len] = '\0';
        s_chat_log_index = (s_chat_log_index + 1) & 3;
        LOG_DEBUG("XYF C: chat %u", idx);
    } else if (subtype == 'P') {
        const char* party = (raw[1] == '|') ? raw + 2 : raw + 1;
        if (party && party[0]) chat_receive(CHAT_TYPE_PARTY, "", NULL, party, 0x00FF00);
        LOG_DEBUG("XYF P: party");
    } else if (subtype == 'A') {
        s_ack_value = (u32)get_int_param(2);
        if (s_ack_flag == 1) { s_ack_flag = 0; LOG_DEBUG("XYF A: ack %u", s_ack_value); }
    } else if (subtype == 'U') {
        s_update_flag = 1;
        LOG_DEBUG("XYF U: update");
    } else {
        len = strlen(unesc); if (len >= MSG_LOG_SIZE) len = MSG_LOG_SIZE - 1;
        idx = s_message_log_index;
        memcpy(s_message_log[idx], unesc, len);
        s_message_log[idx][len] = '\0';
        s_message_log_index = (s_message_log_index + 1) & 3;
        LOG_DEBUG("XYF: msg %u", idx);
    }
}

/* Map/movement handlers */
void handle_XYG(void) {
    if (s_proto_ctx.line_copy) character_parse_list(s_proto_ctx.line_copy);
    LOG_DEBUG("XYG: map enter");
}

void handle_XYH(void) { LOG_DEBUG("XYH: map %d (%d,%d)", get_int_param(1), get_int_param(2), get_int_param(3)); }

void handle_XYJ(void) {
    int result = get_int_param(1);
    if (result >= 0) character_handle_move_result((u32)result, s_proto_ctx.line_copy);
    LOG_DEBUG("XYJ: move %d", result);
}

void handle_XYK(void) {
    int p1 = get_int_param(1);
    if (p1 >= 0 && p1 < MAX_CHARACTERS)
        character_handle_position_text((u32)p1, (u16)get_int_param(3), (u16)get_int_param(4),
                                        (u16)get_int_param(5), 0, (u16)get_int_param(6),
                                        (u16)get_int_param(7), (u16)get_int_param(8));
    LOG_DEBUG("XYK: pos %d", p1);
}

void handle_XYL(void) { LOG_DEBUG("XYL: map '%s' (%d,%d)", get_unescaped_param(1), get_int_param(2), get_int_param(3)); }

void handle_XYM(void) {
    int idx = get_int_param(1);
    if (idx >= 0 && idx < MAX_CHARACTERS) character_teleport((u32)idx, s_proto_ctx.line_copy);
    LOG_DEBUG("XYM: teleport %d", idx);
}

void handle_XYN(void) {
    const char* type = get_string_param(1);
    if (!type || !type[0]) return;
    if (type[0] == 'P') character_parse_list(s_proto_ctx.line_copy);
    else if (type[0] == 'M') LOG_DEBUG("XYN: NPC %s", get_unescaped_param(3));
    else LOG_DEBUG("XYN: type '%c'", type[0]);
}

void handle_XYO(void) {
    u32 char_id = (u32)get_int_param(1);
    u16 x = (u16)get_int_param(3), y = (u16)get_int_param(4);
    u16 dir = (u16)get_int_param(5), sprite = (u16)get_int_param(6);
    const char* name = get_unescaped_param(9);
    static char s_name[32];
    Character* player;

    if (g_skip_enter_game) return;
    if (name && name[0]) { strncpy(s_name, name, 25); s_name[24] = '\0'; }

    player = character_get_player();
    if (player) {
        int is_player = (g_player_char_id == char_id);
        player->x = x; player->y = y;
        player->direction = (u8)dir; player->sprite_id = sprite;
        strncpy(player->name, s_name, 23); player->name[23] = '\0';
        player->action = ACTION_STAND; player->is_moving = 0; player->visible = 1;
        LOG_DEBUG("XYO: '%s' at (%u,%u)", player->name, x, y);
        if (is_player) {
            int sub = -1;
            const char* sf = get_string_param(10);
            if (sf && sf[0]) sub = protocol_decode_int(sf);
            g_game_sub_state = (sub < 0) ? -1 : sub;
            g_game_state_flag = 0;
            g_state_changed = (sub < 0);
        }
    }
    character_setup_position(x, y);
}

void handle_XYP(void) {
    character_handle_action_result((u32)get_int_param(1), (u16)get_int_param(2),
                                    (u16)get_int_param(3), (u16)get_int_param(4),
                                    (u16)get_int_param(5), (u32)get_int_param(6));
    LOG_DEBUG("XYP: action");
}

void handle_XYQ(void) { LOG_DEBUG("XYQ: talk %d", get_int_param(1)); }

/* NPC handlers */
void handle_XYR(void) {
    u32 npc_id = (u32)get_int_param(1);
    const char* dialog = get_unescaped_param(2);
    int type = get_int_param(4);

    if (g_skip_enter_game || g_dialog_skip_flag) return;
    if (g_current_npc_id == npc_id) {
        if (!g_current_npc_context) {
            npc_show_dialog(npc_id, dialog);
            if (g_protocol_mode == 0) npc_send_dialog_ack(npc_id);
            else npc_send_dialog_ack_binary(npc_id);
        } else {
            DialogContext ctx = {0};
            ctx.npc_id = npc_id;
            if (type == 0x29 && g_in_battle == 1) npc_set_context_name(g_current_npc_context, dialog);
            npc_process_dialog_action(&ctx, (DialogAction)type);
        }
    } else npc_show_dialog(npc_id, dialog);
    LOG_DEBUG("XYR: npc %u type %d", npc_id, type);
}

void handle_XYS(void) {
    u32 shop_id = (u32)get_int_param(1);
    const char* data = get_unescaped_param(2);
    char items[512];
    int count = 0;

    if (g_skip_enter_game) return;
    g_shop.current_shop_id = shop_id; g_shop.is_open = 1;
    if (data && data[0]) {
        char* tok;
        strncpy(items, data, sizeof(items) - 1);
        items[sizeof(items) - 1] = '\0';
        tok = strtok(items, ",");
        while (tok && count < MAX_SHOP_ITEMS) {
            u32 id = (u32)atoi(tok);
            if (id > 0) { g_shop.items[count].item_id = id; g_shop.items[count].available = 1; count++; }
            tok = strtok(NULL, ",");
        }
    }
    g_shop.item_count = count;
    shop_ui_init();
    LOG_DEBUG("XYS: shop %u items %d", shop_id, count);
}

void handle_XYT(void) { npc_clear_dialog_entities(); LOG_DEBUG("XYT: close"); }
void handle_XYU(void) { if (s_proto_ctx.line_copy) character_parse_detailed_stats(s_proto_ctx.line_copy); LOG_DEBUG("XYU: stats"); }

void handle_XYV(void) {
    if (get_int_param(3) == 0) item_handle_quantity_text((u32)get_int_param(1), (u32)get_int_param(2));
    LOG_DEBUG("XYV: pickup");
}

void handle_XYW(void) { item_parse_inventory_list(s_proto_ctx.line_copy, 0); LOG_DEBUG("XYW: inv"); }
void handle_XYX(void) { item_handle_use_text((u32)get_int_param(1)); LOG_DEBUG("XYX: use"); }
void handle_XYY(void) { item_handle_equip_text((u32)get_int_param(2), (u32)get_int_param(1)); LOG_DEBUG("XYY: equip"); }
void handle_XYZ(void) { LOG_DEBUG("XYZ: unequip %u", get_int_param(1)); }

/* Battle handlers */
void handle_XYA(void) { battle_handle_text_start((u32)get_int_param(1), (u32)get_int_param(2), (u32)get_int_param(3)); LOG_DEBUG("XYA: battle"); }
void handle_XYB(void) { battle_handle_turn_text((u32)get_int_param(1), 0, 0, 0); LOG_DEBUG("XYB: turn"); }
void handle_XYC2(void) { battle_handle_action_text((u32)get_int_param(1), (u32)get_int_param(2), (u32)get_int_param(3), (u32)get_int_param(4), get_unescaped_param(5)); LOG_DEBUG("XYC2: action"); }
void handle_XYD2(void) { battle_handle_end_text((u32)get_int_param(1), (u32)get_int_param(2), 0, 0); LOG_DEBUG("XYD2: result"); }
void handle_XYE2(void) { battle_handle_text_command((u32)get_int_param(1), (u32)get_int_param(2)); LOG_DEBUG("XYE2: end"); }

/* Party/trade handlers */
void handle_XYE3(void) {
    const char* msg = get_unescaped_param(1);
    int color = get_int_param(2);
    if (msg && msg[0]) chat_system_color(msg, color > 0 ? (u32)color : 0xFFFFFF);
    LOG_DEBUG("XYE3: sysmsg");
}

void handle_XYF2(void) {
    u32 member = (u32)get_int_param(1);
    int i;
    if (!g_party.is_in_party) return;
    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id == member) {
            memset(&g_party.members[i], 0, sizeof(PartyMember));
            g_party.member_count--;
            break;
        }
    }
    if (member == g_party.leader_id) {
        for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
            if (g_party.members[i].id) { g_party.leader_id = g_party.members[i].id; break; }
        }
    }
    if (g_party.member_count <= 0) party_disband();
    LOG_DEBUG("XYF2: leave %u", member);
}

void handle_XYG2(void) {
    const char* name = get_unescaped_param(2);
    if (g_trade.state != TRADE_STATE_NONE) return;
    if (s_proto_ctx.line_copy) trade_handle_packet(s_proto_ctx.line_copy);
    else if (name && name[0]) {
        strncpy(g_trade.partner.name, name, sizeof(g_trade.partner.name) - 1);
        g_trade.state = TRADE_STATE_INVITED;
        LOG_INFO("Trade invite: %s", g_trade.partner.name);
    }
    g_trade.bit_flags |= TRADE_BIT_WINDOW;
}

void handle_XYH2(void) {
    const char* name = get_unescaped_param(1);
    int result = get_int_param(2);
    if (result == 0) {
        g_trade.state = TRADE_STATE_TRADING;
        if (name && name[0]) strncpy(g_trade.partner.name, name, sizeof(g_trade.partner.name) - 1);
        g_trade.bit_flags |= TRADE_BIT_WINDOW;
        g_trade.window_visible = 1;
        LOG_INFO("Trade accepted: %s", name);
    } else if (result == 1 || result == 2) {
        trade_reset();
        LOG_INFO("Trade declined/busy: %s", name);
    } else if (result == 3) {
        trade_handle_cancel();
    }
}

/* NPC dialog helper functions */
static void npc_send_dialog_ack(u32 npc_id) {
    char packet[64];
    snprintf(packet, sizeof(packet), "T|%u\n", npc_id);
    network_send(packet, strlen(packet));
}

static void npc_send_dialog_ack_binary(u32 npc_id) {
    extern void npc_send_ack_binary_impl(u32 npc_id);
    npc_send_ack_binary_impl(npc_id);
}

static void npc_set_context_name(void* context, const char* name) {
    if (context && name) strncpy((char*)context + 0x1d4, name, 20);
}
