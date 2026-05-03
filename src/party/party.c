/*
 * Stone Age Client - Party System Implementation
 * Reverse engineered from sa_9061.exe (FUN_0043bf90, FUN_00465440, FUN_00465460)
 *
 * Protocol Commands:
 * - PI: Party invite
 * - PA: Party accept
 * - PD: Party decline
 * - PL: Party leave (DAT_004b9fb0)
 * - PK: Party kick
 * - PM: Party member update
 * - PLIST: Party list
 * - PC: Party chat
 */

#include <windows.h>
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

    /* Send via text protocol - FUN_0043bf90 routing */
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

    LOG_INFO("Party created");
    return 1;
}

/*
 * Disband party
 */
void party_disband(void) {
    int i;

    if (!g_party.is_in_party) {
        return;
    }

    /* Clear all members */
    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        memset(&g_party.members[i], 0, sizeof(PartyMember));
    }

    g_party.is_in_party = 0;
    g_party.is_leader = 0;
    g_party.member_count = 0;
    g_party.leader_id = 0;

    LOG_INFO("Party disbanded");
}

/*
 * Invite player to party
 */
int party_invite(u32 player_id) {
    char params[64];

    if (!g_party.is_in_party) {
        /* Auto-create party if not in one */
        if (!party_create()) {
            return 0;
        }
    }

    if (!g_party.is_leader) {
        LOG_WARN("Only party leader can invite");
        return 0;
    }

    if (g_party.member_count >= MAX_PARTY_MEMBERS) {
        LOG_WARN("Party is full");
        return 0;
    }

    /* Send invite packet - PI|player_id */
    _snprintf(params, sizeof(params), "%u", player_id);
    if (!party_send_command(PARTY_CMD_INVITE, params)) {
        return 0;
    }

    LOG_DEBUG("Invited player %u to party", player_id);
    return 1;
}

/*
 * Handle party invite packet
 */
void party_handle_invite(void* data, u32 size) {
    u8* ptr = (u8*)data;
    PartyInvite invite;

    if (size < sizeof(PartyInvite)) {
        return;
    }

    memcpy(&invite, ptr, sizeof(PartyInvite));

    /* Store invite for player response */
    g_party.pending_invite = invite;
    g_party.invite_received_time = timeGetTime();

    LOG_DEBUG("Received party invite from %s", invite.leader_name);
}

/*
 * Accept party invite
 */
int party_accept_invite(void) {
    char params[64];

    if (!g_party.pending_invite.valid) {
        return 0;
    }

    /* Check invite timeout */
    if (timeGetTime() - g_party.invite_received_time > PARTY_INVITE_TIMEOUT) {
        memset(&g_party.pending_invite, 0, sizeof(PartyInvite));
        LOG_WARN("Party invite expired");
        return 0;
    }

    /* Send accept packet - PA|party_id */
    _snprintf(params, sizeof(params), "%u", g_party.pending_invite.party_id);
    if (!party_send_command(PARTY_CMD_ACCEPT, params)) {
        return 0;
    }

    g_party.is_in_party = 1;
    g_party.leader_id = g_party.pending_invite.leader_id;
    strncpy(g_party.leader_name, g_party.pending_invite.leader_name, MAX_PARTY_NAME - 1);

    memset(&g_party.pending_invite, 0, sizeof(PartyInvite));
    LOG_INFO("Accepted party invite");
    return 1;
}

/*
 * Decline party invite
 */
void party_decline_invite(void) {
    char params[64];

    if (!g_party.pending_invite.valid) {
        return;
    }

    /* Send decline packet - PD|party_id */
    _snprintf(params, sizeof(params), "%u", g_party.pending_invite.party_id);
    party_send_command(PARTY_CMD_DECLINE, params);

    memset(&g_party.pending_invite, 0, sizeof(PartyInvite));
    LOG_DEBUG("Declined party invite");
}

/*
 * Handle join notification
 */
void party_handle_join(void* data, u32 size) {
    u8* ptr = (u8*)data;
    PartyMember* member;
    int slot;

    if (!g_party.is_in_party) {
        g_party.is_in_party = 1;
        g_party.member_count = 0;
    }

    /* Find empty slot */
    for (slot = 0; slot < MAX_PARTY_MEMBERS; slot++) {
        if (g_party.members[slot].id == 0) {
            break;
        }
    }

    if (slot >= MAX_PARTY_MEMBERS) {
        LOG_WARN("No empty party slot");
        return;
    }

    member = &g_party.members[slot];

    /* Parse member data - PM protocol format */
    member->id = *(u32*)ptr; ptr += 4;
    member->hp = *(u16*)ptr; ptr += 2;
    member->max_hp = *(u16*)ptr; ptr += 2;
    member->mp = *(u16*)ptr; ptr += 2;
    member->max_mp = *(u16*)ptr; ptr += 2;
    member->level = *(u16*)ptr; ptr += 2;
    strncpy(member->name, (char*)ptr, sizeof(member->name) - 1);
    member->slot = (u8)slot;
    member->status = PARTY_STATUS_ONLINE;
    member->last_update = timeGetTime();

    g_party.member_count++;

    party_notify_callbacks(PARTY_ACTION_JOIN, member->id, member->name);
    LOG_INFO("%s joined the party", member->name);
}

/*
 * Leave party
 */
void party_leave(void) {
    if (!g_party.is_in_party) {
        return;
    }

    /* Send leave packet - PL (DAT_004b9fb0) */
    party_send_command(PARTY_CMD_LEAVE, NULL);

    party_disband();
    LOG_INFO("Left party");
}

/*
 * Handle leave notification
 */
void party_handle_leave(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 player_id;
    int i;

    player_id = *(u32*)ptr;

    /* Find and remove member */
    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id == player_id) {
            LOG_INFO("%s left the party", g_party.members[i].name);
            memset(&g_party.members[i], 0, sizeof(PartyMember));
            g_party.member_count--;
            break;
        }
    }

    /* Check if leader left */
    if (player_id == g_party.leader_id) {
        /* Transfer leadership to next member */
        for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
            if (g_party.members[i].id != 0) {
                g_party.leader_id = g_party.members[i].id;
                if (g_party.members[i].id == 0) { /* Player ID check */
                    g_party.is_leader = 1;
                }
                LOG_INFO("Party leadership transferred to %s", g_party.members[i].name);
                break;
            }
        }
    }

    /* If no more members, disband */
    if (g_party.member_count == 0) {
        party_disband();
    }
}

/*
 * Kick member from party
 */
int party_kick(u32 player_id) {
    char params[64];

    if (!g_party.is_leader) {
        LOG_WARN("Only party leader can kick members");
        return 0;
    }

    /* Send kick packet - PK|player_id */
    _snprintf(params, sizeof(params), "%u", player_id);
    if (!party_send_command(PARTY_CMD_KICK, params)) {
        return 0;
    }

    LOG_DEBUG("Kicking player %u from party", player_id);
    return 1;
}

/*
 * Change party leader
 */
int party_change_leader(u32 new_leader_id) {
    char params[64];
    int i;

    if (!g_party.is_leader) {
        LOG_WARN("Only party leader can transfer leadership");
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

    /* Send change leader packet - PT|new_leader_id */
    _snprintf(params, sizeof(params), "%u", new_leader_id);
    if (!party_send_command("PT", params)) {
        return 0;
    }

    g_party.is_leader = 0;
    g_party.leader_id = new_leader_id;

    party_notify_callbacks(PARTY_ACTION_TRANSFER, new_leader_id, g_party.members[i].name);
    LOG_INFO("Transferred party leadership to %s", g_party.members[i].name);
    return 1;
}

/*
 * Update member status
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
 * Set experience share mode
 */
void party_set_exp_share(ExpShareMode mode) {
    if (!g_party.is_leader) {
        return;
    }

    g_party.exp_share = mode;
    LOG_DEBUG("Experience share mode set to %d", mode);
}

/*
 * Set item share mode
 */
void party_set_item_share(ItemShareMode mode) {
    if (!g_party.is_leader) {
        return;
    }

    g_party.item_share = mode;
    LOG_DEBUG("Item share mode set to %d", mode);
}

/*
 * Calculate experience distribution
 */
void party_distribute_exp(u32 base_exp) {
    int i;
    int active_count = 0;
    u32 share_exp;
    u32 total_levels = 0;

    if (g_party.member_count == 0) {
        return;
    }

    /* Count active members (alive and nearby) and sum levels */
    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id != 0 && g_party.members[i].hp > 0) {
            active_count++;
            total_levels += g_party.members[i].level;
        }
    }

    if (active_count == 0) {
        return;
    }

    switch (g_party.exp_share) {
        case EXP_SHARE_EQUAL:
            share_exp = base_exp / active_count;
            break;

        case EXP_SHARE_LEVEL:
            /* Higher level members get proportionally more */
            if (total_levels > 0) {
                for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
                    if (g_party.members[i].id != 0 && g_party.members[i].hp > 0) {
                        u32 member_exp = (base_exp * g_party.members[i].level) / total_levels;

                        /* Apply party bonus */
                        if (active_count > 1) {
                            member_exp += member_exp * (active_count - 1) / 10;
                        }

                        g_party.total_exp_gained += member_exp;
                        LOG_DEBUG("Member %s gained %u exp (level-based)",
                                 g_party.members[i].name, member_exp);
                    }
                }
                return;
            }
            share_exp = base_exp / active_count;
            break;

        case EXP_SHARE_LEADER:
            /* Leader gets all */
            {
                PartyMember* leader = party_get_leader();
                if (leader) {
                    share_exp = base_exp;
                    if (active_count > 1) {
                        share_exp += share_exp * (active_count - 1) / 10;
                    }
                    g_party.total_exp_gained += share_exp;
                    LOG_DEBUG("Leader %s gained %u exp", leader->name, share_exp);
                }
                return;
            }

        default:
            share_exp = base_exp / active_count;
    }

    /* Apply bonus for party */
    if (active_count > 1) {
        /* Party bonus: 10% per additional member */
        u32 bonus = share_exp * (active_count - 1) / 10;
        share_exp += bonus;
    }

    /* Distribute to each member - update local tracking */
    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id != 0 && g_party.members[i].hp > 0) {
            g_party.total_exp_gained += share_exp;
            LOG_DEBUG("Member %s gained %u exp", g_party.members[i].name, share_exp);
        }
    }
}

/*
 * Check if player is in party
 */
int party_is_in_party(void) {
    return g_party.is_in_party;
}

/*
 * Check if player is party leader
 */
int party_is_leader(void) {
    return g_party.is_leader;
}

/*
 * Get party member count
 */
int party_get_member_count(void) {
    return g_party.member_count;
}

/*
 * Get party member by index
 */
PartyMember* party_get_member(int index) {
    if (index < 0 || index >= MAX_PARTY_MEMBERS) {
        return NULL;
    }
    return &g_party.members[index];
}

/*
 * Find party member by ID
 */
PartyMember* party_find_member(u32 player_id) {
    int i;

    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id == player_id) {
            return &g_party.members[i];
        }
    }

    return NULL;
}

/*
 * Get party member by name
 */
PartyMember* party_find_member_by_name(const char* name) {
    int i;

    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (strcmp(g_party.members[i].name, name) == 0) {
            return &g_party.members[i];
        }
    }

    return NULL;
}

/*
 * Send party chat
 */
int party_chat(const char* message) {
    if (!g_party.is_in_party) {
        return 0;
    }

    if (!message || !message[0]) {
        return 0;
    }

    /* Send party chat packet - PC|message */
    if (!party_send_command(PARTY_CMD_CHAT, message)) {
        return 0;
    }

    LOG_DEBUG("Party chat: %s", message);
    return 1;
}

/*
 * Update party member position
 */
void party_update_position(u32 player_id, u16 x, u16 y) {
    int i;

    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id == player_id) {
            g_party.members[i].x = x;
            g_party.members[i].y = y;
            break;
        }
    }
}

/*
 * Check if party is full
 */
int party_is_full(void) {
    return g_party.member_count >= MAX_PARTY_MEMBERS;
}

/*
 * Get party leader
 */
PartyMember* party_get_leader(void) {
    return party_find_member(g_party.leader_id);
}

/*
 * Request party list
 */
void party_request_list(void) {
    /* Send party list request - PLIST */
    party_send_command(PARTY_CMD_LIST, NULL);
}

/*
 * Party member died
 */
void party_member_died(u32 player_id) {
    int i;

    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id == player_id) {
            g_party.members[i].hp = 0;
            g_party.members[i].status |= PARTY_STATUS_DEAD;
            LOG_DEBUG("Party member %s died", g_party.members[i].name);
            break;
        }
    }
}

/*
 * Party member revived
 */
void party_member_revived(u32 player_id, u16 hp) {
    int i;

    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id == player_id) {
            g_party.members[i].hp = hp;
            g_party.members[i].status &= ~PARTY_STATUS_DEAD;
            LOG_DEBUG("Party member %s revived", g_party.members[i].name);
            break;
        }
    }
}

/*
 * Register party callback
 */
int party_register_callback(PartyCallback callback) {
    if (!callback) return 0;
    if (g_party.callback_count >= 4) return 0;

    g_party.callbacks[g_party.callback_count++] = callback;
    return 1;
}

/*
 * Handle party member update (PM command)
 */
void party_handle_member_update(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 player_id;
    u16 hp, max_hp, mp, max_mp;
    u16 x, y;

    if (size < 16) return;

    player_id = *(u32*)ptr; ptr += 4;
    hp = *(u16*)ptr; ptr += 2;
    max_hp = *(u16*)ptr; ptr += 2;
    mp = *(u16*)ptr; ptr += 2;
    max_mp = *(u16*)ptr; ptr += 2;
    x = *(u16*)ptr; ptr += 2;
    y = *(u16*)ptr;

    party_update_member(player_id, hp, max_hp, mp, max_mp);
    party_update_position(player_id, x, y);
}

/*
 * Handle party list response
 */
void party_handle_list(void* data, u32 size) {
    u8* ptr = (u8*)data;
    u32 leader_id;
    u8 member_count;
    int i;

    if (size < 5) return;

    leader_id = *(u32*)ptr; ptr += 4;
    member_count = *ptr++;

    g_party.leader_id = leader_id;
    g_party.is_in_party = 1;
    g_party.member_count = 0;

    /* Clear existing members */
    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        memset(&g_party.members[i], 0, sizeof(PartyMember));
    }

    /* Parse member entries */
    for (i = 0; i < member_count && i < MAX_PARTY_MEMBERS; i++) {
        PartyMember* member = &g_party.members[i];
        u32 entry_size;

        if (ptr + 2 > (u8*)data + size) break;

        entry_size = *(u16*)ptr; ptr += 2;
        if (ptr + entry_size > (u8*)data + size) break;

        /* Parse member entry */
        member->id = *(u32*)ptr; ptr += 4;
        member->level = *(u16*)ptr; ptr += 2;
        member->hp = *(u16*)ptr; ptr += 2;
        member->max_hp = *(u16*)ptr; ptr += 2;
        member->mp = *(u16*)ptr; ptr += 2;
        member->max_mp = *(u16*)ptr; ptr += 2;
        member->x = *(u16*)ptr; ptr += 2;
        member->y = *(u16*)ptr; ptr += 2;
        member->sprite_id = *(u16*)ptr; ptr += 2;
        member->status = *ptr++;
        member->slot = (u8)i;
        strncpy(member->name, (char*)ptr, sizeof(member->name) - 1);

        /* Check if this is the leader */
        if (member->id == leader_id) {
            member->status |= PARTY_STATUS_LEADER;
            strncpy(g_party.leader_name, member->name, MAX_PARTY_NAME - 1);
        }

        g_party.member_count++;
        ptr += entry_size - 17; /* Skip to next entry */
    }

    LOG_DEBUG("Party list received: %d members", g_party.member_count);
}
