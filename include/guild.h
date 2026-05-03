/*
 * Stone Age Client - Guild System Header
 * Reverse engineered from sa_9061.exe
 *
 * Protocol Commands:
 * - GC|<name>: Create guild
 * - GD|: Disband guild
 * - GL|: Leave guild
 * - GK|<name>: Kick member
 * - GI|: Request guild info
 * - GI|<name>: Invite player
 * - GA|: Accept invite
 * - GR|: Reject invite
 * - GN|<notice>: Set notice
 * - GT|<player_id>: Transfer leadership
 * - GF|<amount>: Contribute funds
 * - GM|: Request member list
 * - P|/G<message>: Guild chat
 */

#ifndef GUILD_H
#define GUILD_H

#include "types.h"

/* Protocol commands */
#define GUILD_CMD_CREATE        "GC"    /* Create guild */
#define GUILD_CMD_DISBAND       "GD"    /* Disband guild */
#define GUILD_CMD_LEAVE         "GL"    /* Leave guild */
#define GUILD_CMD_KICK          "GK"    /* Kick member */
#define GUILD_CMD_INFO          "GI"    /* Request guild info / Invite */
#define GUILD_CMD_ACCEPT        "GA"    /* Accept invite */
#define GUILD_CMD_REJECT        "GR"    /* Reject invite / Set rank */
#define GUILD_CMD_NOTICE        "GN"    /* Set notice */
#define GUILD_CMD_TRANSFER      "GT"    /* Transfer leadership */
#define GUILD_CMD_CONTRIBUTE    "GF"    /* Contribute funds */
#define GUILD_CMD_MEMBERS       "GM"    /* Request member list */

/* Constants */
#define MAX_GUILD_MEMBERS       50      /* Max members per guild */
#define MAX_GUILD_NAME          32      /* Max guild name length */
#define MAX_GUILD_NOTICE        128     /* Max notice length */
#define MAX_GUILD_RANK          4       /* Number of ranks */

/* Guild ranks */
typedef enum {
    GUILD_RANK_MEMBER = 0,
    GUILD_RANK_ELDER = 1,
    GUILD_RANK_VICE_LEADER = 2,
    GUILD_RANK_LEADER = 3
} GuildRank;

/* Guild permissions */
typedef enum {
    GUILD_PERM_NONE = 0,
    GUILD_PERM_INVITE = (1 << 0),
    GUILD_PERM_KICK = (1 << 1),
    GUILD_PERM_NOTICE = (1 << 2),
    GUILD_PERM_RANK = (1 << 3),
    GUILD_PERM_ALL = 0xFF
} GuildPermission;

/* Guild member entry */
typedef struct {
    u32 id;
    char name[24];
    u16 level;
    u16 job;
    u8 rank;
    u8 online;
    u32 contribution;
} GuildMember;

/* Guild rank info */
typedef struct {
    char name[16];
    u8 permissions;
} GuildRankInfo;

/* Default rank names */
#define GUILD_DEFAULT_RANK_MEMBER      "Member"
#define GUILD_DEFAULT_RANK_ELDER       "Elder"
#define GUILD_DEFAULT_RANK_VICE_LEADER "Vice Leader"
#define GUILD_DEFAULT_RANK_LEADER      "Leader"

/* Guild info */
typedef struct {
    u32 id;
    char name[MAX_GUILD_NAME];
    char notice[MAX_GUILD_NOTICE];
    u32 leader_id;
    u32 create_time;
    u32 member_count;
    u32 max_members;
    u32 level;
    u32 exp;
    u32 funds;

    /* Ranks */
    GuildRankInfo ranks[MAX_GUILD_RANK];

} GuildInfo;

/* Guild context */
typedef struct {
    /* Guild info */
    GuildInfo guild;
    int in_guild;
    int guild_loaded;

    /* Members */
    GuildMember members[MAX_GUILD_MEMBERS];
    int member_count;
    int member_loaded;

    /* Current selection */
    int selected_index;
    u32 selected_member_id;

    /* Guild creation */
    char create_name[MAX_GUILD_NAME];
    int create_pending;

    /* Guild invite */
    u32 invite_guild_id;
    char invite_guild_name[MAX_GUILD_NAME];
    char invite_leader_name[24];
    int invite_pending;

    /* Flags */
    int dirty;
    int notice_dirty;

} GuildContext;

/* Global guild context */
extern GuildContext g_guild;

/* Initialization */
int guild_init(void);
void guild_shutdown(void);

/* Guild info */
int guild_create(const char* name);
void guild_handle_create_result(void* data, u32 size);
int guild_disband(void);
void guild_handle_disband(void* data, u32 size);

/* Guild info request */
void guild_request_info(void);
void guild_handle_info(void* data, u32 size);
void guild_request_members(void);
void guild_handle_members(void* data, u32 size);

/* Membership */
int guild_invite(const char* name);
void guild_handle_invite(void* data, u32 size);
void guild_handle_invite_request(void* data, u32 size);
int guild_accept_invite(void);
int guild_decline_invite(void);
int guild_leave(void);
void guild_handle_leave(void* data, u32 size);
int guild_kick(const char* name);
void guild_handle_kick(void* data, u32 size);

/* Rank management */
int guild_set_rank(u32 member_id, u8 rank);
void guild_handle_rank_change(void* data, u32 size);
int guild_set_rank_name(u8 rank_index, const char* name);
int guild_set_rank_permissions(u8 rank_index, u8 permissions);

/* Notice */
int guild_set_notice(const char* notice);
void guild_handle_notice_change(void* data, u32 size);

/* Chat */
int guild_chat(const char* message);
void guild_handle_chat(void* data, u32 size);

/* Leader transfer */
int guild_transfer_leader(u32 new_leader_id);
void guild_handle_transfer(void* data, u32 size);

/* Contribution */
int guild_contribute(u32 gold);
void guild_handle_contribution(void* data, u32 size);

/* Member query */
GuildMember* guild_find_member(const char* name);
GuildMember* guild_find_member_by_id(u32 id);
GuildMember* guild_get_member_by_index(int index);
int guild_get_online_count(void);
int guild_get_member_count(void);

/* Permission check */
int guild_has_permission(u32 member_id, GuildPermission perm);
int guild_can_invite(void);
int guild_can_kick(void);
int guild_can_set_notice(void);
int guild_is_leader(void);

/* Guild info query */
u32 guild_get_id(void);
const char* guild_get_name(void);
const char* guild_get_notice(void);
u32 guild_get_level(void);
u32 guild_get_funds(void);
int guild_is_in_guild(void);

/* Selection */
void guild_select_member(int index);
GuildMember* guild_get_selected(void);

#endif /* GUILD_H */
