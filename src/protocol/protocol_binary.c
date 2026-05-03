/*
 * Stone Age Client - Binary Protocol Dispatcher
 * Reverse engineered from sa_9061.exe (FUN_0048d3e0)
 */

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "types.h"
#include "protocol.h"
#include "protocol_binary.h"
#include "network.h"
#include "logger.h"
#include "login.h"
#include "character.h"
#include "battle.h"
#include "pet.h"
#include "skill.h"
#include "item.h"
#include "map.h"
#include "npc.h"
#include "quest.h"
#include "mail.h"
#include "trade.h"
#include "chat.h"
#include "gamestate.h"

/* Packet processing state */
static u32 g_packet_state = 0;

/*
 * Packet dispatcher - FUN_0048d3e0 pattern
 */
int packet_dispatch(u16 opcode, void* data, u32 size) {
    PacketReader reader;
    u16 val1, val2, val3, val4, val5, val6, val7;
    u32 uval1, uval2;

    packet_reader_init(&reader, data, size);

    switch (opcode) {
        case PKT_SV_LOGIN_RESULT:  /* 0x02 */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            val3 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_INFO("Login result received");
            login_handle_result(data, size);
            break;

        case PKT_SV_CHAR_LIST:  /* 0x04 */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_INFO("Character list received");
            login_handle_characters(data, size);
            break;

        case PKT_SV_CHAR_CREATE_RESULT:  /* 0x07 */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_INFO("Character create result");
            login_handle_create_result(data, size);
            break;

        case PKT_SV_CHAR_DELETE_RESULT:  /* 0x0C */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_INFO("Character delete result: %u", uval1);
            login_handle_delete_result(data, size);
            break;

        case PKT_SV_CHAR_LOGOUT_RESULT:  /* 0x0D */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_INFO("Character logout result: %u", uval1);
            gamestate_set_state(GAME_STATE_CHAR_SELECT, 0);
            break;

        case PKT_SV_LOGIN_COMPLETE:  /* 0x0F */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_INFO("Login complete: %u", uval1);
            gamestate_set_state(GAME_STATE_PLAYING, 0);
            break;

        case PKT_SV_MAP_ENTER:  /* 0x16 */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_INFO("Map enter: %u", uval1);
            character_parse_list(data);
            break;

        case PKT_SV_MAP_DATA:  /* 0x18 */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Map data received");
            map_handle_data(data, size);
            break;

        case PKT_SV_MOVE_RESULT:  /* 0x1A */
            val1 = packet_read_u16(&reader);
            uval1 = packet_read_u32(&reader);
            val2 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Move result: pos(%d, %u, %d)", val1, uval1, val2);
            character_handle_move_result(val1, data);
            break;

        case PKT_SV_CHAR_POSITIONS:  /* 0x1C */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            val3 = packet_read_u16(&reader);
            val4 = packet_read_u16(&reader);
            val5 = packet_read_u16(&reader);
            val6 = packet_read_u16(&reader);
            val7 = packet_read_u16(&reader);
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Character positions");
            character_update_positions(data, size);
            break;

        case PKT_SV_MAP_CHANGE:  /* 0x1E */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_INFO("Map change: %u", uval1);
            map_change(uval1);
            break;

        case PKT_SV_TELEPORT:  /* 0x1F */
            val1 = packet_read_u16(&reader);
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_INFO("Teleport to: %d, %u", val1, uval1);
            character_teleport(val1, data);
            break;

        case PKT_SV_OBJECT_SPAWN:  /* 0x24 */
            val1 = packet_read_u16(&reader);
            uval1 = packet_read_u32(&reader);
            val2 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Object spawn: %d, %u, %d", val1, uval1, val2);
            npc_spawn_object(val1, uval1, val2);
            break;

        case PKT_SV_ENTER_GAME:  /* 0x25 */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            val3 = packet_read_u16(&reader);
            val4 = packet_read_u16(&reader);
            val5 = packet_read_u16(&reader);
            val6 = packet_read_u16(&reader);
            val7 = packet_read_u16(&reader);
            val1 = packet_read_u16(&reader);
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_INFO("Enter game");
            gamestate_set_state(GAME_STATE_PLAYING, 0);
            character_setup_position(val2, val3);
            break;

        case PKT_SV_ACTION_RESULT:  /* 0x27 */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            val3 = packet_read_u16(&reader);
            val4 = packet_read_u16(&reader);
            val5 = packet_read_u16(&reader);
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Action result");
            character_handle_action_result(val1, val2, val3, val4, val5, uval1);
            break;

        case PKT_SV_TALK_RESULT:  /* 0x29 */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Talk result: %u", uval1);
            npc_handle_talk_result(data, size);
            break;

        case PKT_SV_NPC_DIALOG:  /* 0x2A */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("NPC dialog: %u", uval1);
            npc_handle_dialog(data, size);
            break;

        case PKT_SV_NPC_SHOP:  /* 0x2B */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("NPC shop: %u", uval1);
            npc_handle_shop(data, size);
            break;

        case PKT_SV_DIALOG_CLOSE:  /* 0x2C */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Dialog close: %u", uval1);
            npc_close_dialog();
            break;

        case PKT_SV_ITEM_PICKUP:  /* 0x2E */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Item pickup: %u", uval1);
            item_handle_pickup(data, size);
            break;

        case PKT_SV_ITEM_DROP:  /* 0x2F */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            val3 = packet_read_u16(&reader);
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Item drop");
            item_handle_drop(data, size);
            break;

        case PKT_SV_INVENTORY_UPDATE:  /* 0x31 */
            val1 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Inventory update: %d", val1);
            item_handle_inventory_update(data, size);
            break;

        case PKT_SV_ITEM_USE_RESULT:  /* 0x33 */
            val1 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Item use result: %d", val1);
            item_handle_use_result(data, size);
            break;

        case PKT_SV_EQUIP_RESULT:  /* 0x35 */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Equip result: %d, %d", val1, val2);
            item_handle_equip_result(data, size);
            break;

        case PKT_SV_UNEQUIP_RESULT:  /* 0x37 */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Unequip result: %d, %d", val1, val2);
            item_handle_unequip_result(data, size);
            break;

        case PKT_SV_BATTLE_START:  /* 0x3B */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            val3 = packet_read_u16(&reader);
            val4 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_INFO("Battle start");
            battle_on_start(val1, val2);
            gamestate_set_state(GAME_STATE_BATTLE, 0);
            break;

        case PKT_SV_BATTLE_TURN:  /* 0x3F */
            val1 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Battle turn: %d", val1);
            battle_set_turn(val1);
            break;

        case PKT_SV_BATTLE_RESULT:  /* 0x42 */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            val3 = packet_read_u16(&reader);
            val4 = packet_read_u16(&reader);
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Battle result");
            battle_handle_result(val1, val2, val3, val4, uval1);
            break;

        case PKT_SV_BATTLE_CAPTURE:  /* 0x44 */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Battle capture");
            battle_handle_capture(val1, val2);
            break;

        case PKT_SV_BATTLE_END:  /* 0x45 */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            val3 = packet_read_u16(&reader);
            val4 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_INFO("Battle end");
            battle_on_end(val1, val2, val3, val4, val1);
            gamestate_set_state(GAME_STATE_PLAYING, 0);
            break;

        case PKT_SV_BATTLE_EXP:  /* 0x48 */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Battle exp: %u", uval1);
            character_gain_exp(uval1);
            break;

        case PKT_SV_PET_LIST:  /* 0x4A */
            uval1 = packet_read_u32(&reader);
            uval2 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Pet list");
            pet_handle_list_packet(data, size);
            break;

        case PKT_SV_PET_STATUS:  /* 0x4C */
            uval1 = packet_read_u32(&reader);
            uval2 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Pet status");
            pet_handle_status_packet(data, size);
            break;

        case PKT_SV_PET_SKILL_LIST:  /* 0x4E */
            uval1 = packet_read_u32(&reader);
            uval2 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Pet skill list");
            pet_handle_skill_list(data, size);
            break;

        case PKT_SV_PET_NAME_RESULT:  /* 0x50 */
            uval1 = packet_read_u32(&reader);
            uval2 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Pet name result");
            pet_handle_name_result(data, size);
            break;

        case PKT_SV_PET_RELEASE_RESULT:  /* 0x52 */
            uval1 = packet_read_u32(&reader);
            uval2 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Pet release result");
            pet_handle_release_result(data, size);
            break;

        case PKT_SV_PET_CAPTURED:  /* 0x54 */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_INFO("Pet captured: %u", uval1);
            pet_handle_capture_result(uval1, 0);
            break;

        case PKT_SV_PARTY_INVITE:  /* 0x56 */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Party invite");
            party_handle_invite(data, size);
            break;

        case PKT_SV_PARTY_JOIN:  /* 0x58 */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Party join: %u", uval1);
            party_handle_join(data, size);
            break;

        case PKT_SV_PARTY_LEAVE:  /* 0x5A */
            val1 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Party leave: %d", val1);
            party_handle_leave(val1);
            break;

        case PKT_SV_TRADE_REQUEST:  /* 0x5C */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Trade request: %u", uval1);
            trade_handle_request(data, size);
            break;

        case PKT_SV_TRADE_START:  /* 0x5D */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Trade start: %u", uval1);
            trade_handle_start(data, size);
            break;

        case PKT_SV_SKILL_LIST:  /* 0x5F */
            val1 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Skill list: %d", val1);
            skill_handle_list(data, size);
            break;

        case PKT_SV_SKILL_USE_RESULT:  /* 0x64 */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Skill use result");
            skill_handle_result(data, size);
            break;

        case PKT_SV_SKILL_COOLDOWN:  /* 0x65 */
            val1 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Skill cooldown: %d", val1);
            skill_start_cooldown(val1);
            break;

        case PKT_SV_SYSTEM_MSG:  /* 0x73 */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("System message");
            chat_handle_system_message(data, size);
            break;

        case PKT_SV_MAIL_LIST:  /* 0x6D */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Mail list: %u", uval1);
            mail_handle_list(data, size);
            break;

        case PKT_SV_MAIL_READ:  /* 0x6F */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Mail read: %u", uval1);
            mail_handle_read(data, size);
            break;

        case PKT_SV_MAIL_SEND_RESULT:  /* 0x71 */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Mail send result: %u", uval1);
            mail_handle_send_result(data, size);
            break;

        case PKT_SV_QUEST_LIST:  /* 0x75 */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Quest list: %u", uval1);
            quest_handle_list(data, size);
            break;

        case PKT_SV_QUEST_UPDATE:  /* 0x78 */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Quest update: %u", uval1);
            quest_handle_update(data, size);
            break;

        case PKT_SV_ACHIEVEMENT:  /* 0x7B */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Achievement: %u", uval1);
            achievement_handle(data, size);
            break;

        case PKT_SV_EVENT_START:  /* 0x7D */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Event start: %u", uval1);
            break;

        case PKT_SV_EVENT_UPDATE:  /* 0x7E */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            val3 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Event update");
            break;

        case PKT_SV_EVENT_END:  /* 0x80 */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Event end");
            break;

        case PKT_SV_RANKING:  /* 0x83 */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Ranking: %u", uval1);
            break;

        case PKT_SV_NOTICE:  /* 0x85 */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Notice: %u", uval1);
            chat_handle_notice(data, size);
            break;

        case PKT_SV_SERVER_TIME:  /* 0x86 */
            val1 = packet_read_u16(&reader);
            val2 = packet_read_u16(&reader);
            val3 = packet_read_u16(&reader);
            val4 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Server time");
            game_set_server_time(val1, val2, val3, val4);
            break;

        case PKT_SV_PING:  /* 0x88 */
            uval1 = packet_read_u32(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Ping: %u", uval1);
            network_send_pong(uval1);
            break;

        case PKT_SV_PONG:  /* 0x89 */
            val1 = packet_read_u16(&reader);
            packet_read_u16(&reader);
            LOG_DEBUG("Pong: %d", val1);
            network_update_ping_time(val1);
            break;

        default:
            LOG_WARN("Unknown packet opcode: 0x%04X", opcode);
            break;
    }

    g_packet_state = 0;
    return 1;
}

/*
 * Binary protocol dispatcher - FUN_0048d3e0 pattern
 * Handles binary packets from server
 */
void packet_dispatch_binary(SOCKET sock) {
    extern NetworkContext g_net;
    u16 size, opcode, checksum;
    u8* data;

    while (g_net.recv_len >= 6) {
        /* Read packet header */
        data = g_net.recv_buffer;
        size = *(u16*)(data);
        opcode = *(u16*)(data + 2);
        checksum = *(u16*)(data + 4);

        /* Check if complete packet is available */
        if (g_net.recv_len < size) {
            break;
        }

        /* Validate checksum */
        u16 calc_checksum = packet_checksum(data + 6, size - 6);
        if (calc_checksum != checksum) {
            LOG_DEBUG("Packet 0x%04X checksum mismatch: calc=0x%04X recv=0x%04X",
                     opcode, calc_checksum, checksum);
            /* Continue processing anyway for compatibility */
        }

        /* Dispatch packet */
        packet_dispatch(opcode, data + 6, size - 6);

        /* Remove processed packet */
        network_buffer_remove(size);
    }
}

/*
 * Handle login success
 */
void packet_handle_login_ok(void* data, int size) {
    LOG_INFO("Login successful");
}

/*
 * Handle login failure
 */
void packet_handle_login_fail(void* data, int size) {
    LOG_WARN("Login failed");
}

/*
 * Handle character list
 */
void packet_handle_char_list(void* data, int size) {
    ProtocolCharacterStats* chars = (ProtocolCharacterStats*)data;
    int count = size / sizeof(ProtocolCharacterStats);

    LOG_INFO("Received %d characters", count);
}

/*
 * Handle map data
 */
void packet_handle_map_data(void* data, int size) {
    LOG_DEBUG("Received map data: %d bytes", size);
}

/*
 * Handle battle start
 */
void packet_handle_battle_start(void* data, int size) {
    LOG_INFO("Battle started");
}
