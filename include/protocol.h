/*
 * Stone Age Client - Network Protocol
 * Reverse engineered from sa_9061.exe (FUN_0048d3e0)
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <winsock2.h>
#include "types.h"

/* Packet opcodes - from FUN_0048d3e0 dispatcher */
typedef enum {
    /* Login/Account packets (0x02-0x07) */
    PKT_SV_LOGIN_RESULT      = 0x02,  /* FUN_00465400 */
    PKT_CLIENT_LOGIN         = 0x03,
    PKT_SV_CHAR_LIST         = 0x04,  /* FUN_00463e70 */
    PKT_CHAR_LOGIN           = 0x05,
    PKT_CHAR_CREATE          = 0x06,
    PKT_SV_CHAR_CREATE_RESULT= 0x07,  /* FUN_00464e10 */

    /* Character management (0x0C-0x0F) */
    PKT_SV_CHAR_DELETE_RESULT= 0x0C,  /* FUN_004643f0 */
    PKT_SV_CHAR_LOGOUT_RESULT= 0x0D,  /* FUN_00464610 */
    PKT_CHAR_DELETE          = 0x0E,
    PKT_SV_LOGIN_COMPLETE    = 0x0F,  /* FUN_00465460, FUN_00464ef0 */

    /* Map/Movement (0x16-0x2C) */
    PKT_SV_MAP_ENTER         = 0x16,  /* FUN_00464670 */
    PKT_CLIENT_MAP_READY     = 0x17,
    PKT_SV_MAP_DATA          = 0x18,  /* FUN_00464650 */
    PKT_CLIENT_MOVE          = 0x19,
    PKT_SV_MOVE_RESULT       = 0x1A,  /* FUN_00465170 */
    PKT_SV_CHAR_POSITIONS    = 0x1C,  /* FUN_00464af0 - multiple positions */
    PKT_SV_MAP_CHANGE        = 0x1E,  /* FUN_00463f00 */
    PKT_SV_TELEPORT          = 0x1F,  /* FUN_00464190 */
    PKT_CLIENT_WALK          = 0x20,
    PKT_CLIENT_RUN           = 0x21,
    PKT_SV_WALK_RESULT       = 0x22,
    PKT_SV_RUN_RESULT        = 0x23,
    PKT_SV_OBJECT_SPAWN      = 0x24,  /* FUN_00463790 */
    PKT_SV_ENTER_GAME        = 0x25,  /* FUN_0047cfe0 - main enter handler */
    PKT_CLIENT_ACTION        = 0x26,
    PKT_SV_ACTION_RESULT     = 0x27,  /* FUN_00465460, FUN_00462200 */
    PKT_CLIENT_TALK          = 0x28,
    PKT_SV_TALK_RESULT       = 0x29,  /* FUN_00462590 */
    PKT_SV_NPC_DIALOG        = 0x2A,  /* FUN_00462f60 */
    PKT_SV_NPC_SHOP          = 0x2B,  /* FUN_00463380 */
    PKT_SV_DIALOG_CLOSE      = 0x2C,  /* FUN_00465460 */

    /* Item/Inventory (0x2E-0x37) */
    PKT_SV_ITEM_PICKUP       = 0x2E,  /* FUN_00465460, FUN_0045ffb0 */
    PKT_SV_ITEM_DROP         = 0x2F,  /* FUN_00465460 */
    PKT_CLIENT_PICKUP        = 0x30,
    PKT_SV_INVENTORY_UPDATE  = 0x31,  /* FUN_00463ee0 */
    PKT_CLIENT_USE_ITEM      = 0x32,
    PKT_SV_ITEM_USE_RESULT   = 0x33,  /* FUN_00464ee0 */
    PKT_CLIENT_EQUIP         = 0x34,
    PKT_SV_EQUIP_RESULT      = 0x35,  /* FUN_00463d80 */
    PKT_CLIENT_UNEQUIP       = 0x36,
    PKT_SV_UNEQUIP_RESULT    = 0x37,  /* FUN_00465060 */

    /* Battle system (0x3B-0x48) */
    PKT_SV_BATTLE_START      = 0x3B,  /* FUN_00465390 */
    PKT_CLIENT_BATTLE_ACTION = 0x3C,
    PKT_CLIENT_BATTLE_FLEE   = 0x3D,
    PKT_CLIENT_BATTLE_SKILL  = 0x3E,
    PKT_SV_BATTLE_TURN       = 0x3F,  /* FUN_00465160 */
    PKT_CLIENT_BATTLE_ITEM   = 0x40,
    PKT_SV_BATTLE_RESULT     = 0x42,  /* FUN_00464ac0 */
    PKT_CLIENT_BATTLE_CAPTURE= 0x43,
    PKT_SV_BATTLE_CAPTURE    = 0x44,  /* FUN_00464db0 */
    PKT_SV_BATTLE_END        = 0x45,  /* FUN_004653d0 */
    PKT_SV_BATTLE_EXP        = 0x48,  /* FUN_0045fa40 */

    /* Pet system (0x4A-0x54) */
    PKT_SV_PET_LIST          = 0x4A,  /* FUN_00463c20 */
    PKT_CLIENT_PET_COMMAND   = 0x4B,
    PKT_SV_PET_STATUS        = 0x4C,  /* FUN_00463d20 */
    PKT_CLIENT_PET_SKILL     = 0x4D,
    PKT_SV_PET_SKILL_LIST    = 0x4E,  /* FUN_0045fdc0 */
    PKT_CLIENT_PET_NAME      = 0x4F,
    PKT_SV_PET_NAME_RESULT   = 0x50,  /* FUN_0045fb80 */
    PKT_CLIENT_PET_RELEASE   = 0x51,
    PKT_SV_PET_RELEASE_RESULT= 0x52,  /* FUN_0045ff50 */
    PKT_SV_PET_CAPTURED      = 0x54,  /* FUN_00465460 */

    /* Party/Trade (0x56-0x5D) */
    PKT_SV_PARTY_INVITE      = 0x56,  /* FUN_00465460 */
    PKT_CLIENT_PARTY_ACCEPT  = 0x57,
    PKT_SV_PARTY_JOIN        = 0x58,  /* FUN_00465460 */
    PKT_CLIENT_PARTY_LEAVE   = 0x59,
    PKT_SV_PARTY_LEAVE       = 0x5A,  /* FUN_00465440 */
    PKT_CLIENT_TRADE_REQUEST = 0x5B,
    PKT_SV_TRADE_REQUEST     = 0x5C,  /* FUN_0045a9a0 */
    PKT_SV_TRADE_START       = 0x5D,  /* FUN_00465460, FUN_00465470 */

    /* Skills (0x5F-0x6B) */
    PKT_SV_SKILL_LIST        = 0x5F,  /* FUN_00465460 */
    PKT_CLIENT_SKILL_USE     = 0x60,
    PKT_CLIENT_SKILL_LEARN   = 0x61,
    PKT_SV_SKILL_LEARN_RESULT= 0x62,
    PKT_CLIENT_SKILL_FORGET  = 0x63,
    PKT_SV_SKILL_USE_RESULT  = 0x64,  /* FUN_00465b80 */
    PKT_SV_SKILL_COOLDOWN    = 0x65,  /* FUN_00465bb0 */

    /* Mail/System (0x6D-0x73) */
    PKT_SV_MAIL_LIST         = 0x6D,  /* FUN_0045a900 */
    PKT_CLIENT_MAIL_READ     = 0x6E,
    PKT_SV_MAIL_READ         = 0x6F,  /* FUN_0045a980 */
    PKT_CLIENT_MAIL_SEND     = 0x70,
    PKT_SV_MAIL_SEND_RESULT  = 0x71,  /* FUN_0045d6f0 */
    PKT_CLIENT_MAIL_DELETE   = 0x72,
    PKT_SV_SYSTEM_MSG        = 0x73,  /* FUN_00465110 */

    /* Additional features (0x75-0x89) */
    PKT_SV_QUEST_LIST        = 0x75,  /* FUN_00465d20 */
    PKT_SV_QUEST_UPDATE      = 0x78,  /* FUN_00465de0 */
    PKT_SV_ACHIEVEMENT       = 0x7B,  /* FUN_00465f70 */
    PKT_SV_EVENT_START       = 0x7D,  /* FUN_00465460, FUN_00466030 */
    PKT_SV_EVENT_UPDATE      = 0x7E,  /* FUN_00465460, FUN_004660c0 */
    PKT_SV_EVENT_END         = 0x80,  /* FUN_00465460, FUN_00466110 */
    PKT_CLIENT_EVENT_JOIN    = 0x81,
    PKT_SV_RANKING           = 0x83,  /* FUN_00465460, FUN_00466150 */
    PKT_SV_NOTICE            = 0x85,  /* FUN_004665b0 */
    PKT_SV_SERVER_TIME       = 0x86,  /* FUN_00466550 */
    PKT_SV_PING              = 0x88,  /* FUN_0045a930 */
    PKT_CLIENT_PONG          = 0x89,
    PKT_SV_PONG              = 0x8A,  /* FUN_00466740 */

    /* System packets */
    PKT_HEARTBEAT            = 0xFF,
    PKT_DISCONNECT           = 0xFE

} PacketOpcode;

/* Packet header structure */
#pragma pack(push, 1)
typedef struct {
    u16 size;
    u16 opcode;
    u16 checksum;
    u8  data[0];
} PacketHeader;
#pragma pack(pop)

/* Login request structure */
#pragma pack(push, 1)
typedef struct {
    u16 size;
    u16 opcode;
    u16 checksum;
    char username[32];
    char password[32];
    char version[16];
    u32 flags;
} PacketLogin;
#pragma pack(pop)

/* Character stats structure - for protocol data */
#pragma pack(push, 1)
typedef struct {
    u32 char_id;
    char name[24];
    u16 level;
    u16 hp;
    u16 max_hp;
    u16 mp;
    u16 max_mp;
    u16 str;
    u16 dex;
    u16 vit;
    u16 agi;
    u16 intel;
    u16 luck;
    u16 x;
    u16 y;
    u16 direction;
    u32 experience;
    u32 gold;
    u8  sprite_id;
    u8  job;
    u16 flags;
} ProtocolCharacterStats;
#pragma pack(pop)

/* Position structure */
#pragma pack(push, 1)
typedef struct {
    u16 x;
    u16 y;
    u8  direction;
    u8  action;
} PositionData;
#pragma pack(pop)

/* Packet reader context */
typedef struct {
    u8* data;
    u32 size;
    u32 offset;
    u16 checksum_calc;
    u16 checksum_recv;
} PacketReader;

/* Protocol functions */
void protocol_init(void);
void protocol_shutdown(void);

/* Packet building */
int packet_build_login(char* buffer, const char* username, const char* password);
int packet_build_char_select(char* buffer, u32 char_id);
int packet_build_move(char* buffer, u16 x, u16 y, u8 direction);
int packet_build_action(char* buffer, u8 action, u32 target_id);
int packet_build_talk(char* buffer, const char* message);
int packet_build_heartbeat(char* buffer);

/* Packet parsing */
int packet_parse(char* buffer, int len, void (*handler)(u16 opcode, void* data, int size));
u16 packet_checksum(void* data, int len);

/* Packet reader functions */
void packet_reader_init(PacketReader* reader, void* data, u32 size);
u16 packet_read_u16(PacketReader* reader);
u32 packet_read_u32(PacketReader* reader);
int packet_validate_checksum(PacketReader* reader);

/* Protocol encoding/decoding - FUN_0043efe0 */
int protocol_decode_int(const char* str);
void protocol_encode_int(int value, char* buffer, int max_len);

/* String unescaping - FUN_0043ded0 */
char* protocol_unescape_string(const char* str);
void protocol_unescape_string_to(const char* src, char* dst, int max_len);

/* Text protocol dispatcher - FUN_0043bf90 */
int protocol_dispatch_text(char* data, u32 len);

/* Text protocol sending */
int protocol_send_text_command(SOCKET sock, const char* cmd, const char* params);

/* Packet dispatcher */
int packet_dispatch(u16 opcode, void* data, u32 size);

/* Binary packet dispatcher - FUN_0045ffb0 */
void packet_dispatch_binary(SOCKET sock);

/* Packet handlers */
void packet_handle_login_ok(void* data, int size);
void packet_handle_login_fail(void* data, int size);
void packet_handle_char_list(void* data, int size);
void packet_handle_map_data(void* data, int size);
void packet_handle_battle_start(void* data, int size);

#endif /* PROTOCOL_H */
