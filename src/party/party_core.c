/*
 * Stone Age Client - Party System Core Functions
 * Split from party.c - Core party management
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "party.h"
#include "network.h"
#include "network_protocol.h"
#include "logger.h"

/* Global party context */
PartyContext g_party = {0};

/* Internal helper: Send party protocol command */
static int party_send_command(const char* cmd, const char* params) {
    char packet[256];
    int len;

    if (!cmd) return 0;

    if (params && params[0]) {
        len = _snprintf(packet, sizeof(packet), "%s|%s", cmd, params);
    } else {
        len = _snprintf(packet, sizeof(packet), "%s", cmd);
    }

    if (len < 0 || len >= (int)sizeof(packet)) {
        LOG_ERROR("Party packet too long");
        return 0;
    }

    /* Send via text protocol */
    send_queue_add(packet, 0, 0);

    LOG_DEBUG("Party send: %s", packet);
    return 1;
}

/* Internal helper: Notify callbacks */
static void party_notify_callbacks(PartyAction action, u32 player_id, const char* name) {
    int i;
    for (i = 0; i < g_party.callback_count && i < 4; i++) {
        if (g_party.callbacks[i]) {
            g_party.callbacks[i](action, player_id, name);
        }
    }
}

/*
 * Initialize party system
 */
int party_init(void) {
    memset(&g_party, 0, sizeof(PartyContext));
    g_party.max_members = MAX_PARTY_MEMBERS;
    g_party.exp_share = EXP_SHARE_EQUAL;
    g_party.item_share = ITEM_SHARE_LEADER;

    LOG_INFO("Party system initialized");
    return 1;
}

/*
 * Shutdown party system
 */
void party_shutdown(void) {
    party_disband();
    memset(&g_party, 0, sizeof(PartyContext));
    LOG_INFO("Party system shutdown");
}

/*
 * Create party (become leader)
 */
int party_create(void) {
    if (g_party.is_in_party) {
        LOG_WARN("Already in a party");
        return 0;
    }

    g_party.is_in_party = 1;
    g_party.is_leader = 1;
    g_party.member_count = 1;
    g_party.leader_id = 0; /* Player ID would be set here */

    /* Send create packet: PC|CREATE */
    party_send_command("PC", "CREATE");

    party_notify_callbacks(PARTY_ACTION_NONE, 0, NULL);
    LOG_INFO("Party created");
    return 1;
}

/*
 * Disband party
 */
void party_disband(void) {
    if (!g_party.is_in_party) {
        return;
    }

    /* Send disband packet if leader */
    if (g_party.is_leader) {
        party_send_command("PC", "DISBAND");
    }

    memset(g_party.members, 0, sizeof(g_party.members));
    g_party.member_count = 0;
    g_party.is_in_party = 0;
    g_party.is_leader = 0;
    g_party.leader_id = 0;

    party_notify_callbacks(PARTY_ACTION_DISBAND, 0, NULL);
    LOG_INFO("Party disbanded");
}

/*
 * Invite player to party
 */
int party_invite(u32 player_id) {
    char params[32];

    if (!g_party.is_in_party) {
        /* Create party first */
        party_create();
    }

    if (!g_party.is_leader) {
        LOG_WARN("Only leader can invite");
        return 0;
    }

    if (g_party.member_count >= MAX_PARTY_MEMBERS) {
        LOG_WARN("Party is full");
        return 0;
    }

    /* Send invite: PI|<player_id> */
    _snprintf(params, sizeof(params), "%u", player_id);
    party_send_command("PI", params);

    LOG_DEBUG("Invited player %u to party", player_id);
    return 1;
}

/*
 * Handle incoming invite notification
 */
void party_handle_invite(void* data, u32 size) {
    /* Parse invite data: leader_name|leader_id */
    char* leader_name = (char*)data;
    u32 leader_id = 0;
    char* id_str;

    if (!data || size == 0) return;

    /* Find separator */
    id_str = strchr(leader_name, '|');
    if (id_str) {
        *id_str = '\0';
        leader_id = (u32)atoi(id_str + 1);
    }

    g_party.pending_invite.valid = 1;
    g_party.pending_invite.leader_id = leader_id;
    strncpy(g_party.pending_invite.leader_name, leader_name, MAX_PARTY_NAME - 1);
    g_party.pending_invite.timestamp = timeGetTime();
    g_party.pending_invite.expire_time = PARTY_INVITE_TIMEOUT;

    party_notify_callbacks(PARTY_ACTION_INVITE, leader_id, leader_name);
    LOG_INFO("Party invite from %s (ID: %u)", leader_name, leader_id);
}

/*
 * Accept pending invite
 */
int party_accept_invite(void) {
    if (!g_party.pending_invite.valid) {
        LOG_WARN("No pending invite");
        return 0;
    }

    /* Send accept: PA */
    party_send_command("PA", NULL);

    g_party.pending_invite.valid = 0;
    g_party.is_in_party = 1;

    party_notify_callbacks(PARTY_ACTION_JOIN, g_party.pending_invite.leader_id, g_party.pending_invite.leader_name);
    LOG_INFO("Joined party");
    return 1;
}

/*
 * Decline pending invite
 */
void party_decline_invite(void) {
    if (!g_party.pending_invite.valid) {
        return;
    }

    /* Send decline: PD */
    party_send_command("PD", NULL);

    g_party.pending_invite.valid = 0;
    party_notify_callbacks(PARTY_ACTION_LEAVE, g_party.pending_invite.leader_id, NULL);
    LOG_INFO("Declined party invite");
}

/*
 * Handle join notification
 */
void party_handle_join(void* data, u32 size) {
    /* Parse join data: player_name|player_id|level|hp|max_hp|mp|max_mp */
    char* parse = (char*)data;
    PartyMember* member;
    int slot;

    if (!data || size == 0) return;

    /* Find empty slot */
    for (slot = 0; slot < MAX_PARTY_MEMBERS; slot++) {
        if (g_party.members[slot].id == 0) {
            member = &g_party.members[slot];
            break;
        }
    }

    if (slot >= MAX_PARTY_MEMBERS) {
        LOG_WARN("No slot for new party member");
        return;
    }

    /* Parse member info */
    memset(member, 0, sizeof(PartyMember));

    /* Name */
    char* next = strchr(parse, '|');
    if (next) {
        *next = '\0';
        strncpy(member->name, parse, sizeof(member->name) - 1);
        parse = next + 1;
    }

    /* ID */
    next = strchr(parse, '|');
    if (next) {
        *next = '\0';
        member->id = (u32)atoi(parse);
        parse = next + 1;
    }

    /* Level */
    next = strchr(parse, '|');
    if (next) {
        *next = '\0';
        member->level = (u16)atoi(parse);
        parse = next + 1;
    }

    /* HP */
    next = strchr(parse, '|');
    if (next) {
        *next = '\0';
        member->hp = (u16)atoi(parse);
        parse = next + 1;
    }

    /* Max HP */
    next = strchr(parse, '|');
    if (next) {
        *next = '\0';
        member->max_hp = (u16)atoi(parse);
        parse = next + 1;
    }

    /* MP */
    next = strchr(parse, '|');
    if (next) {
        *next = '\0';
        member->mp = (u16)atoi(parse);
        parse = next + 1;
    }

    /* Max MP */
    member->max_mp = (u16)atoi(parse);

    g_party.member_count++;

    party_notify_callbacks(PARTY_ACTION_JOIN, member->id, member->name);
    LOG_INFO("%s joined party (ID: %u, Level: %u)", member->name, member->id, member->level);
}

/*
 * Leave party
 */
void party_leave(void) {
    if (!g_party.is_in_party) {
        return;
    }

    /* Send leave: PL */
    party_send_command("PL", NULL);

    /* If leader leaves, disband */
    if (g_party.is_leader) {
        party_disband();
        return;
    }

    /* Remove self from member list */
    g_party.is_in_party = 0;
    g_party.member_count = 0;
    memset(g_party.members, 0, sizeof(g_party.members));

    party_notify_callbacks(PARTY_ACTION_LEAVE, 0, NULL);
    LOG_INFO("Left party");
}

/*
 * Handle leave notification
 */
void party_handle_leave(void* data, u32 size) {
    u32 player_id;
    int i;
    char name[32] = {0};

    if (!data || size < 4) return;

    player_id = *(u32*)data;

    /* Find and remove member */
    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id == player_id) {
            strncpy(name, g_party.members[i].name, sizeof(name) - 1);
            memset(&g_party.members[i], 0, sizeof(PartyMember));
            g_party.member_count--;

            /* If leader left, promote next member */
            if (player_id == g_party.leader_id && g_party.member_count > 0) {
                for (int j = 0; j < MAX_PARTY_MEMBERS; j++) {
                    if (g_party.members[j].id != 0) {
                        g_party.leader_id = g_party.members[j].id;
                        if (g_party.members[j].id == 0 /* player_id */) {
                            g_party.is_leader = 1;
                        }
                        break;
                    }
                }
            }

            break;
        }
    }

    party_notify_callbacks(PARTY_ACTION_LEAVE, player_id, name);
    LOG_INFO("%s left party", name);
}

/*
 * Kick member from party
 */
int party_kick(u32 player_id) {
    char params[32];

    if (!g_party.is_leader) {
        LOG_WARN("Only leader can kick");
        return 0;
    }

    /* Send kick: PK|<player_id> */
    _snprintf(params, sizeof(params), "%u", player_id);
    party_send_command("PK", params);

    LOG_DEBUG("Kicked player %u from party", player_id);
    return 1;
}

/*
 * Change party leader
 */
int party_change_leader(u32 new_leader_id) {
    char params[32];
    int i;

    if (!g_party.is_leader) {
        LOG_WARN("Only leader can change leader");
        return 0;
    }

    /* Verify new leader is in party */
    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id == new_leader_id) {
            break;
        }
    }

    if (i >= MAX_PARTY_MEMBERS) {
        LOG_WARN("Player not in party");
        return 0;
    }

    /* Send change leader: PC|LEADER|<new_leader_id> */
    _snprintf(params, sizeof(params), "LEADER|%u", new_leader_id);
    party_send_command("PC", params);

    g_party.leader_id = new_leader_id;
    g_party.is_leader = (new_leader_id == 0 /* player_id */);

    party_notify_callbacks(PARTY_ACTION_TRANSFER, new_leader_id, NULL);
    LOG_INFO("Party leader changed to %u", new_leader_id);
    return 1;
}

/*
 * Update member stats
 */
void party_update_member(u32 player_id, u16 hp, u16 max_hp, u16 mp, u16 max_mp) {
    int i;

    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id == player_id) {
            g_party.members[i].hp = hp;
            g_party.members[i].max_hp = max_hp;
            g_party.members[i].mp = mp;
            g_party.members[i].max_mp = max_mp;
            break;
        }
    }
}

/*
 * Get party leader
 */
PartyMember* party_get_leader(void) {
    int i;

    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id == g_party.leader_id) {
            return &g_party.members[i];
        }
    }

    return NULL;
}

/*
 * Get member by ID
 */
PartyMember* party_get_member(u32 player_id) {
    int i;

    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id == player_id) {
            return &g_party.members[i];
        }
    }

    return NULL;
}

/*
 * Check if in party
 */
int party_is_in_party(void) {
    return g_party.is_in_party;
}

/*
 * Check if is leader
 */
int party_is_leader(void) {
    return g_party.is_leader;
}

/*
 * Get member count
 */
int party_get_member_count(void) {
    return g_party.member_count;
}
