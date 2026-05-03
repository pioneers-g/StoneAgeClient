/*
 * Stone Age Client - Party System Header
 * Reverse engineered from sa_9061.exe (FUN_0043bf90, FUN_00465440)
 *
 * Protocol Commands:
 * - PI: Party invite
 * - PA: Party accept
 * - PD: Party decline
 * - PL: Party leave (DAT_004b9fb0)
 * - PK: Party kick
 * - PM: Party member update
 * - PLIST: Party list
 */

#ifndef PARTY_H
#define PARTY_H

#include "types.h"

/* Constants */
#define MAX_PARTY_MEMBERS       5
#define MAX_PARTY_NAME          24
#define PARTY_INVITE_TIMEOUT    60000   /* 60 seconds */

/* Party protocol commands - from FUN_0043bf90 analysis */
#define PARTY_CMD_INVITE        "PI"
#define PARTY_CMD_ACCEPT        "PA"
#define PARTY_CMD_DECLINE       "PD"
#define PARTY_CMD_LEAVE         "PL"    /* DAT_004b9fb0 */
#define PARTY_CMD_KICK          "PK"
#define PARTY_CMD_UPDATE        "PM"
#define PARTY_CMD_LIST          "PLIST"
#define PARTY_CMD_CHAT          "PC"

/* Experience share modes */
typedef enum {
    EXP_SHARE_EQUAL = 0,
    EXP_SHARE_LEVEL = 1,
    EXP_SHARE_LEADER = 2
} ExpShareMode;

/* Item share modes */
typedef enum {
    ITEM_SHARE_RANDOM = 0,
    ITEM_SHARE_LEADER = 1,
    ITEM_SHARE_FINDER = 2
} ItemShareMode;

/* Party member status flags */
typedef enum {
    PARTY_STATUS_ONLINE    = 0x01,
    PARTY_STATUS_IN_BATTLE = 0x02,
    PARTY_STATUS_DEAD      = 0x04,
    PARTY_STATUS_AWAY      = 0x08,
    PARTY_STATUS_LEADER    = 0x10
} PartyMemberStatus;

/* Party member structure */
typedef struct {
    u32 id;
    char name[MAX_PARTY_NAME];
    u16 level;
    u16 hp;
    u16 max_hp;
    u16 mp;
    u16 max_mp;
    u16 x;
    u16 y;
    u16 sprite_id;
    u8  status;         /* PartyMemberStatus flags */
    u8  slot;           /* Party slot index (0-4) */
    u8  direction;
    u8  action;
    u32 last_update;
} PartyMember;

/* Party invite structure */
typedef struct {
    u32 party_id;
    u32 leader_id;
    char leader_name[MAX_PARTY_NAME];
    u32 timestamp;
    u32 expire_time;
    u8  valid;
    u8  reserved[3];
} PartyInvite;

/* Party action types */
typedef enum {
    PARTY_ACTION_NONE = 0,
    PARTY_ACTION_INVITE = 1,
    PARTY_ACTION_JOIN = 2,
    PARTY_ACTION_LEAVE = 3,
    PARTY_ACTION_KICK = 4,
    PARTY_ACTION_DISBAND = 5,
    PARTY_ACTION_TRANSFER = 6
} PartyAction;

/* Party notification callback */
typedef void (*PartyCallback)(PartyAction action, u32 player_id, const char* name);

/* Party context */
typedef struct {
    /* State */
    u8  is_in_party;
    u8  is_leader;
    u8  member_count;
    u8  max_members;

    /* Leader */
    u32 leader_id;
    char leader_name[MAX_PARTY_NAME];

    /* Members */
    PartyMember members[MAX_PARTY_MEMBERS];

    /* Settings */
    ExpShareMode exp_share;
    ItemShareMode item_share;

    /* Pending invite */
    PartyInvite pending_invite;
    u32 invite_received_time;

    /* Party statistics */
    u32 total_exp_gained;
    u32 total_items_found;
    u32 formation_time;
    u32 last_action_time;

    /* Callbacks */
    PartyCallback callbacks[4];
    int callback_count;

    /* Window state */
    void* party_window;
    u8  window_visible;
    u8  window_dirty;

} PartyContext;

/* Global party context */
extern PartyContext g_party;

/* Initialization */
int party_init(void);
void party_shutdown(void);

/* Party creation/disband */
int party_create(void);
void party_disband(void);

/* Invitation */
int party_invite(u32 player_id);
void party_handle_invite(void* data, u32 size);
int party_accept_invite(void);
void party_decline_invite(void);

/* Join/Leave */
void party_handle_join(void* data, u32 size);
void party_leave(void);
void party_handle_leave(void* data, u32 size);

/* Member management */
int party_kick(u32 player_id);
int party_change_leader(u32 new_leader_id);
void party_update_member(u32 player_id, u16 hp, u16 max_hp, u16 mp, u16 max_mp);

/* Share settings */
void party_set_exp_share(ExpShareMode mode);
void party_set_item_share(ItemShareMode mode);

/* Experience distribution */
void party_distribute_exp(u32 base_exp);

/* Query functions */
int party_is_in_party(void);
int party_is_leader(void);
int party_get_member_count(void);
int party_is_full(void);

PartyMember* party_get_member(int index);
PartyMember* party_find_member(u32 player_id);
PartyMember* party_find_member_by_name(const char* name);
PartyMember* party_get_leader(void);

/* Chat */
int party_chat(const char* message);

/* Position */
void party_update_position(u32 player_id, u16 x, u16 y);

/* List */
void party_request_list(void);

/* Death/Revive */
void party_member_died(u32 player_id);
void party_member_revived(u32 player_id, u16 hp);

/* Callbacks */
int party_register_callback(PartyCallback callback);

/* Protocol handlers */
void party_handle_member_update(void* data, u32 size);
void party_handle_list(void* data, u32 size);

#endif /* PARTY_H */
