/*
 * Stone Age Client - Guild System Core
 * Reverse engineered from sa_9061.exe
 *
 * Core guild functionality:
 * - Initialization and shutdown
 * - Guild operations (create, disband, invite, etc.)
 * - Member management
 * - Utility functions
 *
 * Protocol handlers: guild_protocol.c
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "types.h"
#include "guild.h"
#include "network.h"
#include "character.h"
#include "logger.h"

/* Global guild context */
GuildContext g_guild = {0};

/* Forward declarations */
static void guild_send_packet(const char* cmd, const char* params);
static void guild_send_formatted(const char* fmt, ...);

/* Default rank names */
static const char* s_default_rank_names[MAX_GUILD_RANK] = {
    GUILD_DEFAULT_RANK_MEMBER,
    GUILD_DEFAULT_RANK_ELDER,
    GUILD_DEFAULT_RANK_VICE_LEADER,
    GUILD_DEFAULT_RANK_LEADER
};

/* Default rank permissions */
static const u8 s_default_rank_permissions[MAX_GUILD_RANK] = {
    GUILD_PERM_NONE,
    GUILD_PERM_INVITE,
    GUILD_PERM_INVITE | GUILD_PERM_KICK | GUILD_PERM_NOTICE,
    GUILD_PERM_ALL
};

/*
 * Send guild protocol packet - helper function
 */
static void guild_send_packet(const char* cmd, const char* params) {
    char packet[512];

    if (params && params[0]) {
        _snprintf(packet, sizeof(packet), "%s|%s", cmd, params);
    } else {
        strncpy(packet, cmd, sizeof(packet) - 1);
    }

    network_send(packet, strlen(packet));
    network_send("\n", 1);
}

/*
 * Send formatted guild packet - helper function
 */
static void guild_send_formatted(const char* fmt, ...) {
    char packet[1024];
    va_list args;

    va_start(args, fmt);
    _vsnprintf(packet, sizeof(packet), fmt, args);
    va_end(args);

    network_send(packet, strlen(packet));
    network_send("\n", 1);
}

/*
 * Initialize guild system
 */
int guild_init(void) {
    int i;

    memset(&g_guild, 0, sizeof(GuildContext));

    /* Initialize default ranks */
    for (i = 0; i < 4; i++) {
        strncpy(g_guild.guild.ranks[i].name, s_default_rank_names[i], 15);
        g_guild.guild.ranks[i].permissions = s_default_rank_permissions[i];
    }

    LOG_INFO("Guild system initialized");
    return 1;
}

/*
 * Shutdown guild system
 */
void guild_shutdown(void) {
    memset(&g_guild, 0, sizeof(GuildContext));
    LOG_INFO("Guild system shutdown");
}

/*
 * Create guild
 */
int guild_create(const char* name) {
    if (!name || strlen(name) == 0) {
        return 0;
    }

    if (g_guild.in_guild) {
        LOG_WARN("Already in a guild");
        return 0;
    }

    strncpy(g_guild.create_name, name, MAX_GUILD_NAME - 1);
    g_guild.create_pending = 1;

    /* Send guild create packet: GC|<name> */
    guild_send_packet(GUILD_CMD_CREATE, name);

    LOG_DEBUG("Creating guild: %s", name);
    return 1;
}

/*
 * Disband guild
 */
int guild_disband(void) {
    if (!g_guild.in_guild) {
        return 0;
    }

    if (!guild_is_leader()) {
        LOG_WARN("Only guild leader can disband");
        return 0;
    }

    /* Send guild disband packet: GD| */
    guild_send_packet(GUILD_CMD_DISBAND, "");

    LOG_DEBUG("Disbanding guild: %s", g_guild.guild.name);
    return 1;
}

/*
 * Request guild info
 */
void guild_request_info(void) {
    /* Send guild info request packet: GI| */
    guild_send_packet(GUILD_CMD_INFO, "");

    LOG_DEBUG("Requesting guild info");
}

/*
 * Request guild members
 */
void guild_request_members(void) {
    /* Send guild member list request: GM| */
    guild_send_packet(GUILD_CMD_MEMBERS, "");

    LOG_DEBUG("Requesting guild members");
}

/*
 * Invite player to guild
 */
int guild_invite(const char* name) {
    if (!g_guild.in_guild) {
        return 0;
    }

    if (!guild_can_invite()) {
        LOG_WARN("No permission to invite");
        return 0;
    }

    /* Send guild invite packet: GI|<name> */
    guild_send_packet(GUILD_CMD_INFO, name);

    LOG_DEBUG("Inviting %s to guild", name);
    return 1;
}

/*
 * Accept guild invite
 */
int guild_accept_invite(void) {
    if (!g_guild.invite_pending) {
        return 0;
    }

    /* Send accept packet: GA| */
    guild_send_packet(GUILD_CMD_ACCEPT, "");

    g_guild.invite_pending = 0;
    LOG_DEBUG("Accepted guild invite");
    return 1;
}

/*
 * Decline guild invite
 */
int guild_decline_invite(void) {
    if (!g_guild.invite_pending) {
        return 0;
    }

    /* Send decline packet: GR| */
    guild_send_packet(GUILD_CMD_REJECT, "");

    g_guild.invite_pending = 0;
    LOG_DEBUG("Declined guild invite");
    return 1;
}

/*
 * Leave guild
 */
int guild_leave(void) {
    if (!g_guild.in_guild) {
        return 0;
    }

    if (guild_is_leader()) {
        LOG_WARN("Guild leader cannot leave, must disband or transfer");
        return 0;
    }

    /* Send leave packet: GL| */
    guild_send_packet(GUILD_CMD_LEAVE, "");

    LOG_DEBUG("Leaving guild: %s", g_guild.guild.name);
    return 1;
}

/*
 * Kick member from guild
 */
int guild_kick(const char* name) {
    GuildMember* member;

    if (!g_guild.in_guild) {
        return 0;
    }

    if (!guild_can_kick()) {
        LOG_WARN("No permission to kick");
        return 0;
    }

    member = guild_find_member(name);
    if (!member) {
        return 0;
    }

    if (member->rank >= GUILD_RANK_LEADER) {
        LOG_WARN("Cannot kick guild leader");
        return 0;
    }

    /* Send kick packet: GK|<name> */
    guild_send_packet(GUILD_CMD_KICK, name);

    LOG_DEBUG("Kicking %s from guild", name);
    return 1;
}

/*
 * Set member rank
 */
int guild_set_rank(u32 member_id, u8 rank) {
    GuildMember* member;
    char params[64];

    if (!g_guild.in_guild || rank >= MAX_GUILD_RANK) {
        return 0;
    }

    member = guild_find_member_by_id(member_id);
    if (!member) {
        return 0;
    }

    /* Send rank change packet: GR|<member_id>|<rank> */
    _snprintf(params, sizeof(params), "%u|%d", member_id, rank);
    guild_send_packet(GUILD_CMD_REJECT, params);

    LOG_DEBUG("Setting rank for %s to %d", member->name, rank);
    return 1;
}

/*
 * Set guild notice
 */
int guild_set_notice(const char* notice) {
    if (!g_guild.in_guild) {
        return 0;
    }

    if (!guild_can_set_notice()) {
        LOG_WARN("No permission to set notice");
        return 0;
    }

    /* Send notice change packet: GN|<notice> */
    guild_send_packet(GUILD_CMD_NOTICE, notice);

    LOG_DEBUG("Setting guild notice");
    return 1;
}

/*
 * Send guild chat
 */
int guild_chat(const char* message) {
    char packet[512];

    if (!g_guild.in_guild || !message) {
        return 0;
    }

    /* Send guild chat packet: P|/G<message> */
    _snprintf(packet, sizeof(packet), "P|/G%s", message);
    network_send(packet, strlen(packet));
    network_send("\n", 1);

    return 1;
}

/*
 * Transfer guild leadership
 */
int guild_transfer_leader(u32 new_leader_id) {
    GuildMember* member;
    char params[32];

    if (!g_guild.in_guild) {
        return 0;
    }

    if (!guild_is_leader()) {
        LOG_WARN("Only guild leader can transfer leadership");
        return 0;
    }

    member = guild_find_member_by_id(new_leader_id);
    if (!member) {
        return 0;
    }

    /* Send transfer packet: GT|<new_leader_id> */
    _snprintf(params, sizeof(params), "%u", new_leader_id);
    guild_send_packet(GUILD_CMD_TRANSFER, params);

    LOG_DEBUG("Transferring leadership to %s", member->name);
    return 1;
}

/*
 * Contribute to guild
 */
int guild_contribute(u32 gold) {
    char params[32];

    if (!g_guild.in_guild || gold == 0) {
        return 0;
    }

    /* Send contribution packet: GF|<amount> */
    _snprintf(params, sizeof(params), "%u", gold);
    guild_send_packet(GUILD_CMD_CONTRIBUTE, params);

    LOG_DEBUG("Contributing %u gold to guild", gold);
    return 1;
}

/*
 * Find member by name
 */
GuildMember* guild_find_member(const char* name) {
    int i;

    for (i = 0; i < g_guild.member_count; i++) {
        if (strcmp(g_guild.members[i].name, name) == 0) {
            return &g_guild.members[i];
        }
    }

    return NULL;
}

/*
 * Find member by ID
 */
GuildMember* guild_find_member_by_id(u32 id) {
    int i;

    for (i = 0; i < g_guild.member_count; i++) {
        if (g_guild.members[i].id == id) {
            return &g_guild.members[i];
        }
    }

    return NULL;
}

/*
 * Get member by index
 */
GuildMember* guild_get_member_by_index(int index) {
    if (index < 0 || index >= g_guild.member_count) {
        return NULL;
    }
    return &g_guild.members[index];
}

/*
 * Get online member count
 */
int guild_get_online_count(void) {
    int i;
    int count = 0;

    for (i = 0; i < g_guild.member_count; i++) {
        if (g_guild.members[i].online) {
            count++;
        }
    }

    return count;
}

/*
 * Get member count
 */
int guild_get_member_count(void) {
    return g_guild.member_count;
}

/*
 * Check if member has permission
 */
int guild_has_permission(u32 member_id, GuildPermission perm) {
    GuildMember* member = guild_find_member_by_id(member_id);

    if (!member) {
        return 0;
    }

    return (g_guild.guild.ranks[member->rank].permissions & perm) != 0;
}

/*
 * Check if can invite
 */
int guild_can_invite(void) {
    return guild_has_permission(g_guild.guild.leader_id, GUILD_PERM_INVITE);
}

/*
 * Check if can kick
 */
int guild_can_kick(void) {
    return guild_has_permission(g_guild.guild.leader_id, GUILD_PERM_KICK);
}

/*
 * Check if can set notice
 */
int guild_can_set_notice(void) {
    return guild_has_permission(g_guild.guild.leader_id, GUILD_PERM_NOTICE);
}

/*
 * Check if is guild leader
 */
int guild_is_leader(void) {
    Character* player = character_get_player();
    if (!player) return 0;
    return player->id == g_guild.guild.leader_id;
}

/*
 * Get guild ID
 */
u32 guild_get_id(void) {
    return g_guild.guild.id;
}

/*
 * Get guild name
 */
const char* guild_get_name(void) {
    return g_guild.guild.name;
}

/*
 * Get guild notice
 */
const char* guild_get_notice(void) {
    return g_guild.guild.notice;
}

/*
 * Get guild level
 */
u32 guild_get_level(void) {
    return g_guild.guild.level;
}

/*
 * Get guild funds
 */
u32 guild_get_funds(void) {
    return g_guild.guild.funds;
}

/*
 * Check if in guild
 */
int guild_is_in_guild(void) {
    return g_guild.in_guild;
}

/*
 * Select member by index
 */
void guild_select_member(int index) {
    if (index >= 0 && index < g_guild.member_count) {
        g_guild.selected_index = index;
        g_guild.selected_member_id = g_guild.members[index].id;
    }
}

/*
 * Get selected member
 */
GuildMember* guild_get_selected(void) {
    if (g_guild.selected_index >= 0 && g_guild.selected_index < g_guild.member_count) {
        return &g_guild.members[g_guild.selected_index];
    }
    return NULL;
}
