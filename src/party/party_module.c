/*
 * Stone Age Client - Party Module Functions
 * Split from party.c - Settings, exp distribution, utilities, callbacks
 * Reverse engineered from sa_9061.exe
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

/* External globals */
extern PartyContext g_party;

/* Forward declarations */
extern int party_send_command(const char* cmd, const char* params);
extern void party_notify_callbacks(PartyAction action, u32 player_id, const char* name);

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
            break;
    }

    /* Apply party bonus for equal distribution */
    if (active_count > 1) {
        share_exp += share_exp * (active_count - 1) / 10;
    }

    /* Apply to all active members */
    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id != 0 && g_party.members[i].hp > 0) {
            g_party.total_exp_gained += share_exp;
            LOG_DEBUG("Member %s gained %u exp", g_party.members[i].name, share_exp);
        }
    }
}

/*
 * Get member by index
 */
PartyMember* party_get_member_by_index(int index) {
    if (index < 0 || index >= MAX_PARTY_MEMBERS) {
        return NULL;
    }

    return &g_party.members[index];
}

/*
 * Find member by name
 */
PartyMember* party_find_member_by_name(const char* name) {
    int i;

    if (!name) return NULL;

    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id != 0 &&
            strncmp(g_party.members[i].name, name, MAX_PARTY_NAME) == 0) {
            return &g_party.members[i];
        }
    }

    return NULL;
}

/*
 * Find member by ID (returns pointer, not index)
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
 * Send party chat message
 */
int party_chat(const char* message) {
    char params[256];

    if (!g_party.is_in_party) {
        LOG_WARN("Not in a party");
        return 0;
    }

    if (!message || !message[0]) {
        return 0;
    }

    /* Send: PC|<message> */
    _snprintf(params, sizeof(params), "%s", message);
    party_send_command("PC", params);

    LOG_DEBUG("Party chat: %s", message);
    return 1;
}

/*
 * Update member position
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
 * Request party list from server
 */
void party_request_list(void) {
    party_send_command("PLIST", NULL);
}

/*
 * Handle member death
 */
void party_member_died(u32 player_id) {
    int i;

    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id == player_id) {
            g_party.members[i].hp = 0;
            party_notify_callbacks(PARTY_ACTION_LEAVE, player_id, g_party.members[i].name);
            LOG_INFO("Party member %s died", g_party.members[i].name);
            break;
        }
    }
}

/*
 * Handle member revival
 */
void party_member_revived(u32 player_id, u16 hp) {
    int i;

    for (i = 0; i < MAX_PARTY_MEMBERS; i++) {
        if (g_party.members[i].id == player_id) {
            g_party.members[i].hp = hp;
            party_notify_callbacks(PARTY_ACTION_JOIN, player_id, g_party.members[i].name);
            LOG_INFO("Party member %s revived with %u HP", g_party.members[i].name, hp);
            break;
        }
    }
}

/*
 * Register callback
 */
int party_register_callback(PartyCallback callback) {
    if (g_party.callback_count >= 4) {
        LOG_WARN("Party callback limit reached");
        return 0;
    }

    g_party.callbacks[g_party.callback_count++] = callback;
    return 1;
}

/*
 * Handle member update from server
 */
void party_handle_member_update(void* data, u32 size) {
    /* Parse update data: player_id|hp|max_hp|mp|max_mp|x|y */
    char* parse = (char*)data;
    u32 player_id;
    u16 hp, max_hp, mp, max_mp;
    u16 x = 0, y = 0;
    char* next;

    if (!data || size == 0) return;

    /* Player ID */
    next = strchr(parse, '|');
    if (next) {
        *next = '\0';
        player_id = (u32)atoi(parse);
        parse = next + 1;
    } else return;

    /* HP */
    next = strchr(parse, '|');
    if (next) {
        *next = '\0';
        hp = (u16)atoi(parse);
        parse = next + 1;
    } else return;

    /* Max HP */
    next = strchr(parse, '|');
    if (next) {
        *next = '\0';
        max_hp = (u16)atoi(parse);
        parse = next + 1;
    } else return;

    /* MP */
    next = strchr(parse, '|');
    if (next) {
        *next = '\0';
        mp = (u16)atoi(parse);
        parse = next + 1;
    } else return;

    /* Max MP */
    next = strchr(parse, '|');
    if (next) {
        *next = '\0';
        max_mp = (u16)atoi(parse);
        parse = next + 1;
    } else return;

    /* X */
    next = strchr(parse, '|');
    if (next) {
        *next = '\0';
        x = (u16)atoi(parse);
        parse = next + 1;
    }

    /* Y */
    y = (u16)atoi(parse);

    /* Update member */
    party_update_member(player_id, hp, max_hp, mp, max_mp);
    party_update_position(player_id, x, y);
}

/*
 * Handle party list from server
 */
void party_handle_list(void* data, u32 size) {
    /* Parse list: count|member1_data|member2_data|... */
    char* parse = (char*)data;
    int count, i;
    char* next;

    if (!data || size == 0) return;

    /* Clear current members */
    memset(g_party.members, 0, sizeof(g_party.members));
    g_party.member_count = 0;

    /* Count */
    next = strchr(parse, '|');
    if (next) {
        *next = '\0';
        count = atoi(parse);
        parse = next + 1;
    } else return;

    /* Parse each member */
    for (i = 0; i < count && i < MAX_PARTY_MEMBERS; i++) {
        /* Each member is separated by '|' */
        /* Member format: name|id|level|hp|max_hp|mp|max_mp */
        /* ... simplified parsing */
    }

    g_party.member_count = count;
    g_party.is_in_party = (count > 0);

    party_notify_callbacks(PARTY_ACTION_NONE, 0, NULL);
    LOG_DEBUG("Party list received: %d members", count);
}

/*
 * Send command helper (exported for module use)
 */
int party_send_command(const char* cmd, const char* params) {
    char packet[256];
    int len;

    if (!cmd) return 0;

    if (params && params[0]) {
        len = _snprintf(packet, sizeof(packet), "%s|%s", cmd, params);
    } else {
        len = _snprintf(packet, sizeof(packet), "%s", cmd);
    }

    if (len < 0 || len >= (int)sizeof(packet)) {
        return 0;
    }

    send_queue_add(packet, 0, 0);
    return 1;
}

/*
 * Notify callbacks helper (exported for module use)
 */
void party_notify_callbacks(PartyAction action, u32 player_id, const char* name) {
    int i;
    for (i = 0; i < g_party.callback_count && i < 4; i++) {
        if (g_party.callbacks[i]) {
            g_party.callbacks[i](action, player_id, name);
        }
    }
}
